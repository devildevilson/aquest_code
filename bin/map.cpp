#include "map.h"
#include "render/yavf.h"
#include "render/shared_structures.h"
#include "figures.h"
#include <thread>
#include <chrono>
#include "seasons.h"
#include "utils/thread_pool.h"
#include "utils/works_utils.h"
#include "seasons.h"
#include "utils/globals.h"
#include "utils/utility.h"
#include <atomic>

#define MAP_CONTAINER_DESCRIPTOR_POOL_NAME "map_container_descriptor_pool"

namespace devils_engine {
  namespace core {
    static_assert(sizeof(std::atomic<uint32_t>) == sizeof(uint32_t));
    static_assert(alignof(std::atomic<uint32_t>) == alignof(uint32_t));
    
    const size_t seasons::maximum_biomes;
    const size_t seasons::maximum_seasons;
    const int32_t seasons::invalid_biome;
    
    struct c_tile_data {
      std::atomic<uint32_t> data[2][4];
      
      c_tile_data() : data{{GPU_UINT_MAX, GPU_UINT_MAX, GPU_UINT_MAX, GPU_UINT_MAX}, {GPU_UINT_MAX, GPU_UINT_MAX, GPU_UINT_MAX, GPU_UINT_MAX}} {}
    };
    
    static_assert(sizeof(c_tile_data) == sizeof(render::additional_data_t));
    static_assert(alignof(c_tile_data) == alignof(render::additional_data_t));
    
    map::map(const create_info &info) : 
      device(info.device),
      points(nullptr),
      tiles(nullptr),
      accel_triangles(nullptr),
      biomes(nullptr),
      structures(nullptr),
      provinces(nullptr),
      faiths(nullptr),
      cultures(nullptr),
      free_army_slot(UINT32_MAX),
      armies_count(0),
      s(status::initial)
    {
      const uint32_t accel_triangles_count = tri_count_d(accel_struct_detail_level);
      const uint32_t tiles_count = hex_count_d(detail_level);
      const uint32_t points_count = points_count_d(detail_level);
      
      points = device->create(yavf::BufferCreateInfo::buffer(sizeof(glm::vec4)*points_count, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
      tiles = device->create(yavf::BufferCreateInfo::buffer(sizeof(render::light_map_tile_t)*tiles_count, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
      accel_triangles = device->create(yavf::BufferCreateInfo::buffer(
        sizeof(render::packed_fast_triangle_t)*accel_triangles_count, 
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT
      ), VMA_MEMORY_USAGE_CPU_ONLY);
      biomes = device->create(yavf::BufferCreateInfo::buffer(
        sizeof(render::packed_biome_data_t)*MAX_BIOMES_COUNT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
      ), VMA_MEMORY_USAGE_GPU_ONLY);
      
      const uint32_t tile_indices_count = std::ceil(float(tiles_count)/float(4));
      tile_indices = device->create(yavf::BufferCreateInfo::buffer(
        sizeof(glm::uvec4)*tile_indices_count, 
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT
      ), VMA_MEMORY_USAGE_CPU_ONLY); // это можно засунуть сразу в гпу память
      
      structures = device->create(yavf::BufferCreateInfo::buffer(
        sizeof(glm::uvec4), 
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
      ), VMA_MEMORY_USAGE_GPU_ONLY);
      
      tile_object_indices = device->create(yavf::BufferCreateInfo::buffer(
        sizeof(c_tile_data) * tiles_count,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
      ), VMA_MEMORY_USAGE_CPU_ONLY);
      
      army_data_buffer = device->create(yavf::BufferCreateInfo::buffer(
        sizeof(render::army_data_t) * 16,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
      ), VMA_MEMORY_USAGE_CPU_ONLY);
      
      memset(tile_indices->ptr(), 0, tile_indices->info().size);
      using array_data_type = c_tile_data;
      auto object_indices = reinterpret_cast<array_data_type*>(tile_object_indices->ptr());
      const uint32_t vec_count = sizeof(array_data_type) / sizeof(glm::uvec4);
      for (size_t i = 0; i < tiles_count; ++i) {
        for (uint32_t j = 0; j < vec_count; ++j) {
          for (uint32_t k = 0; k < 4; ++k) {
            object_indices[i].data[j][k] = GPU_UINT_MAX;
          }
        }
      }
      
      size_t accum = 0;
      for (uint32_t i = 0; i < detail_level; ++i) {
        accum += tri_count_d(i);
      }
      accum += tri_count_d(detail_level);
      
      triangles.resize(accum);
      triangles_data.resize(accum);
      
      yavf::DescriptorSetLayout tiles_data_layout = device->setLayout(TILES_DATA_LAYOUT_NAME);
      if (tiles_data_layout == VK_NULL_HANDLE) {
        yavf::DescriptorLayoutMaker dlm(device);
        tiles_data_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL)
                               .binding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL)
                               .binding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL)
                               .binding(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL)
                               .binding(4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL)
                               .binding(5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL)
                               .binding(6, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL)
                               .binding(7, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL)
                               .create(TILES_DATA_LAYOUT_NAME);
      }
      
      //auto pool = device->descriptorPool(DEFAULT_DESCRIPTOR_POOL_NAME);
      yavf::DescriptorPool pool = VK_NULL_HANDLE;
      {
        yavf::DescriptorPoolMaker dpm(device);
        pool = dpm.poolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10).create(MAP_CONTAINER_DESCRIPTOR_POOL_NAME);
      }
      
      {
        yavf::DescriptorMaker dm(device);
        tiles_set = dm.layout(tiles_data_layout).create(pool)[0];
        size_t index = tiles_set->add({tiles, 0, tiles->info().size, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
                       tiles_set->add({biomes, 0, biomes->info().size, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
                       tiles_set->add({points, 0, points->info().size, 0, 2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
                       tiles_set->add({accel_triangles, 0, accel_triangles->info().size, 0, 3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
                       tiles_set->add({tile_indices, 0, tile_indices->info().size, 0, 4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
                       tiles_set->add({structures, 0, structures->info().size, 0, 5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
                       tiles_set->add({tile_object_indices, 0, tile_object_indices->info().size, 0, 6, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
                       tiles_set->add({army_data_buffer, 0, army_data_buffer->info().size, 0, 7, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
        tiles_set->update();
        tiles->setDescriptor(tiles_set, index);
      }
    }
    
    map::~map() {
      device->destroy(points);
      device->destroy(tiles);
      device->destroy(accel_triangles);
      device->destroy(biomes);
      device->destroy(tile_indices);
      device->destroy(structures);
      device->destroy(tile_object_indices);
      device->destroy(army_data_buffer);
      device->destroyDescriptorPool(MAP_CONTAINER_DESCRIPTOR_POOL_NAME);
//       device->destroy(provinces);
//       device->destroy(faiths);
//       device->destroy(cultures);
    }
    
    bool test_intersect_func(const glm::vec4 &v0, const glm::vec4 &v1, const glm::vec4 &v2, const utils::ray &ray, float &t) {
      const glm::vec3 v0v1 = v1 - v0;
      const glm::vec3 v0v2 = v2 - v0;
      const glm::vec3 pvec = glm::cross(glm::vec3(ray.dir), v0v2);
      const float det = glm::dot(v0v1, pvec);

      //if (det > -EPSILON) return false;
//       if (det < EPSILON) return false;
//       //if (glm::abs(det) < EPSILON) return false;
// 
//       std::cout << "normal ok" << '\n';

      const float invDet = 1.0f / det;

      float u,v;

      const glm::vec3 tvec = ray.pos - v0;
      u = glm::dot(tvec, pvec) * invDet;

      std::cout << "u " << u << '\n';
      //if (u <= 0.0f || u >= 1.0f) return false;
      if (u < -EPSILON || u > 1.0f+EPSILON) return false;

      const glm::vec3 qvec = glm::cross(tvec, v0v1);
      v = glm::dot(glm::vec3(ray.dir), qvec) * invDet;
      std::cout << "v " << v << '\n';
      //if (v <= 0.0f || u + v >= 1.0f) return false;
      if (v < -EPSILON || u + v > 1.0f+EPSILON) return false;

      t = glm::dot(v0v2, qvec) * invDet;
      
      std::cout << "triangle ok" << "\n";
      
      return true;
    }
    
    bool test_intersect_funcd(const glm::dvec4 &v0, const glm::dvec4 &v1, const glm::dvec4 &v2, const utils::ray &ray, double &t) {
      const glm::dvec3 ray_pos = ray.pos;
//       const glm::dvec3 ray_dir = ray.dir;
      const glm::dvec3 v0v1 = v1 - v0;
      const glm::dvec3 v0v2 = v2 - v0;
//       const glm::dvec3 N = glm::cross(v0v1, v0v2);
      const glm::dvec3 pvec = glm::cross(glm::dvec3(ray.dir), v0v2);
      const double det = glm::dot(v0v1, pvec);
//       const double area2 = glm::length(N);
      
      //if (det > -EPSILON) return false;
      //if (det < double(EPSILON)) return false;
      if (det > -double(EPSILON) && det < double(EPSILON)) return false;
      //if (glm::abs(det) < EPSILON) return false;
      
      // НЕРАБОТАЕТ =(((
//       const double not_ray_dir = glm::dot(N, ray_dir);
//       if (glm::abs(not_ray_dir) < EPSILON) return false;
//       std::cout << "normal ok" << '\n';
      
//       const double d = glm::dot(N, glm::dvec3(v0));
//       t = (glm::dot(N, ray_pos) + d) / not_ray_dir;
//       if (t < 0.0) return false;
//       PRINT_VAR("t", t)
      
//       const glm::dvec3 P = ray_pos + t * ray_dir; 
//       
//       glm::dvec3 C;
//       glm::dvec3 edge0 = v1 - v0;
//       glm::dvec3 vp0 = P - glm::dvec3(v0); 
//       C = glm::cross(edge0, vp0); 
//       if (glm::dot(N, C) < 0.0) return false; // P is on the right side 
//   
//       // edge 1
//       glm::dvec3 edge1 = v2 - v1; 
//       glm::dvec3 vp1 = P - glm::dvec3(v1); 
//       C = glm::cross(edge1, vp1); 
//       if (glm::dot(N, C) < 0.0)  return false; // P is on the right side 
//   
//       // edge 2
//       glm::dvec3 edge2 = v0 - v2; 
//       glm::dvec3 vp2 = P - glm::dvec3(v2); 
//       C = glm::cross(edge2, vp2); 
//       if (glm::dot(N, C) < 0.0) return false; // P is on the right side; 
//   
//       return true; // this ray hits the triangle 

      const double invDet = 1.0f / det;

      double u,v;

      const glm::dvec3 tvec = ray_pos - glm::dvec3(v0);
      u = glm::dot(tvec, pvec) * invDet;

//       std::cout << "u " << u << '\n';
      if (u < 0.0 || u > 1.0) return false;
      //if (u < -EPSILON || u > 1.0f+EPSILON) return false;

      const glm::dvec3 qvec = glm::cross(tvec, v0v1);
      v = glm::dot(glm::dvec3(ray.dir), qvec) * invDet;
//       std::cout << "v " << v << '\n';
      if (v < 0.0 || u + v > 1.0) return false;
      //if (v < -EPSILON || u + v > 1.0+EPSILON) return false;

      t = glm::dot(v0v2, qvec) * invDet;
      
//       std::cout << "triangle ok" << "\n";
      
      return true;
    }
    
#define MAX_VALUE 1000000.0f
    uint32_t map::cast_ray(const utils::ray &ray, float &ray_dist) const {
      static const std::function<uint32_t(const utils::ray &ray, const uint32_t &tri_index, float &distance)> reck = [this] (const utils::ray &ray, const uint32_t &tri_index, float &distance) {
        const bool ret = intersect_container(tri_index, ray);
        if (!ret) return UINT32_MAX;
        
        const triangle &tri = triangles[tri_index];
        
        const uint32_t level = tri.current_level;
        if (level == detail_level) {
          uint32_t final_tile_index = UINT32_MAX;
          float final_tile_dist = MAX_VALUE;
          for (size_t i = 0; i < 4; ++i) {
            const uint32_t tile_index = tri.next_level[i];
            // тут нужно проверить дальность до тайла + проверить пересечение со стенками
            // нужно ли чекать ближайший треугольник? не уверен что это необходимо
            
            const auto &tile_data = render::unpack_data(get_tile(tile_index));
            const float height = tile_data.height;
            const uint32_t p_count = render::is_pentagon(tile_data) ? 5 : 6;
            const uint32_t point_a_index = tile_data.center;
            const uint32_t height_layer = render::compute_height_layer(height);
            const float final_height = render::layer_height * height_layer;
            const float computed_height = final_height * render::render_tile_height;
            
            glm::vec4 center = get_point(point_a_index);
            glm::vec4 center_height = center + glm::normalize(glm::vec4(glm::vec3(center), 0.0f)) * (computed_height);
            glm::vec4 local_points[6];
            glm::vec4 local_points_height[6];
            for (uint32_t j = 0; j < p_count; ++j) {
              const uint32_t point1_index = tile_data.points[j];
              const glm::vec4 point = get_point(point1_index);
              const glm::vec4 point_normal = glm::vec4(glm::vec3(point) / world_radius, 0.0f);
              const glm::vec4 point_height = point + point_normal * (computed_height);
              local_points[j] = point;
              local_points_height[j] = point_height;
            }
            
            for (uint32_t j = 0; j < p_count; ++j) {
              const uint32_t b_index = j;
              const uint32_t c_index = (j+1)%p_count;
              
              ASSERT(c_index < p_count);
              
              float dist = MAX_VALUE;
//               const bool ret = intersect_tri(get_point(point_a_index), get_point(point_b_index), get_point(point_c_index), ray, dist);
              const bool ret = intersect_tri(center_height, local_points_height[b_index], local_points_height[c_index], ray, dist);
              if (ret && dist < final_tile_dist) {
                final_tile_index = tile_index;
                final_tile_dist = dist;
                //break;
              }
            }
            
            if (final_tile_index != UINT32_MAX) {
              distance = final_tile_dist;
              return final_tile_index;
            }

            // почему то по приоритету берутся стенки
            for (uint32_t j = 0; j < p_count; ++j) {
              const uint32_t b_index = j;
              const uint32_t c_index = (j+1)%p_count;
              const glm::vec4 point1 = local_points[b_index];
              const glm::vec4 point2 = local_points[c_index];
              const glm::vec4 point3 = local_points_height[b_index];
              const glm::vec4 point4 = local_points_height[c_index];
              // две стенки
              const std::tuple<glm::vec4, glm::vec4, glm::vec4> wall_triangle[] = {
                std::tie(point1, point2, point3),
                std::tie(point4, point3, point2)
              };
              
              float dist1 = MAX_VALUE;
              float dist2 = MAX_VALUE;
              const bool ret1 = intersect_tri(std::get<0>(wall_triangle[0]), std::get<1>(wall_triangle[0]), std::get<2>(wall_triangle[0]), ray, dist1);
              const bool ret2 = intersect_tri(std::get<0>(wall_triangle[1]), std::get<1>(wall_triangle[1]), std::get<2>(wall_triangle[1]), ray, dist2);
              
              if (ret1 && dist1 < final_tile_dist) {
                final_tile_dist = dist1;
                final_tile_index = tile_index;
              }
              
              if (ret2 && dist2 < final_tile_dist) {
                final_tile_dist = dist2;
                final_tile_index = tile_index;
              }
            }
          }
          
          distance = final_tile_dist;
          return final_tile_index;
        }
        
        float global_dist = MAX_VALUE;
        uint32_t global_index = UINT32_MAX;
        for (size_t i = 0; i < 4; ++i) {
          const uint32_t tri_index = tri.next_level[i];
          
          float dist = MAX_VALUE;
          const uint32_t index = reck(ray, tri_index, dist); // может я тут что то не так делаю
          
          if (index == UINT32_MAX) continue;
          
          if (dist < global_dist) {
            global_dist = dist;
            global_index = index;
          }
        }
        
        distance = global_dist;
        return global_index;
      };
      
      size_t current_detail_level = 0;
      
      float dist = MAX_VALUE;
      uint32_t final_tile = UINT32_MAX;
      for (size_t i = 0; i < 20*power4(current_detail_level); ++i) {
        float local_dist = MAX_VALUE;
        const uint32_t tile_index = reck(ray, i, local_dist);
        
        if (local_dist < dist) {
          dist = local_dist;
          final_tile = tile_index;
        }
      }
      
      ray_dist = dist;
      return final_tile;
      
//       //ASSERT(current_tri_index != UINT32_MAX); // по идее это условие всегда должно выполняться
//       // у нас еще не создана карта
//       if (current_tri_index == UINT32_MAX) return UINT32_MAX;
//       
//       while (current_detail_level <= detail_level) {
//         const triangle &tri = triangles[current_tri_index];
// //         const uint32_t tri_index_test = current_tri_index;
//         current_tri_index = UINT32_MAX;
//         for (size_t i = 0; i < 4; ++i) {
//           const uint32_t tri_index = tri.next_level[i];
//           if (intersect_container(tri_index, ray)) {
//             current_tri_index = tri_index;
//             //ASSERT(triangles[current_tri_index].current_level == current_detail_level);
//             ++current_detail_level;
//             //std::cout << "current_tri_index " << current_tri_index << '\n';            
//             break;
//           }
//           
// // #ifndef _NDEBUG
// //           size_t count = 0;
// //           for (size_t i = 0; i < triangles[tri_index].current_level; ++i) {
// //             count += 20*power4(i);
// //           }
// //           
// //           const size_t offset = count;
// //           const size_t last = offset + 20*power4(triangles[tri_index].current_level);
// //           
// //           ASSERT(tri_index >= offset && tri_index < last);
// // #endif
// //           
// //         }
// // 
// //         if (current_tri_index == UINT32_MAX) {
// // //           PRINT_VAR("detail_level", detail_level)
// // //           PRINT_VAR("current_detail_level", current_detail_level)
// //           PRINT_VAR("tri_index_test", tri_index_test)
// //           PRINT_VAR("tri.current_level", tri.current_level)
// // //           PRINT_VAR("triangles count", (20*power4(current_detail_level-1)))
// // //           PRINT_VAR("next level 0", tri.next_level[0])
// // //           PRINT_VEC4("ray dir", ray.dir)
// // //           auto points_arr = reinterpret_cast<glm::vec4*>(points->ptr());
// // //           PRINT_VEC4("p 0", points_arr[tri.points[0]])
// // //           PRINT_VEC4("p 1", points_arr[tri.points[1]])
// // //           PRINT_VEC4("p 2", points_arr[tri.points[2]])
// // //           const glm::vec4 test_point = ray.pos + ray.dir * 20.0f;
// // //           PRINT_VEC4("test point", test_point)
// //           
// //           PRINT_VEC4("ray pos", ray.pos)
// //           PRINT_VEC4("ray dir", ray.dir)
// //           PRINT_VEC4("ray p  ", (ray.pos + ray.dir * 20.0f))
// //           auto points_arr = reinterpret_cast<glm::vec4*>(points->ptr());
// //           for (uint32_t i = 0; i < 4; ++i) {
// //             const triangle &next_tri = triangles[tri.next_level[i]];
// //             const glm::vec4 p0 = points_arr[next_tri.points[0]];
// //             const glm::vec4 p1 = points_arr[next_tri.points[1]];
// //             const glm::vec4 p2 = points_arr[next_tri.points[2]];
// //             PRINT_VAR("index", tri.next_level[i])
// //             PRINT_VEC4("p0", p0)
// //             PRINT_VEC4("p1", p1)
// //             PRINT_VEC4("p2", p2)
// //             test_intersect_func(p0, p1, p2, ray);
// //           }
//         }
//         ASSERT(current_tri_index != UINT32_MAX);
//       }
//       
//       ASSERT(triangles[current_tri_index].current_level == detail_level);
//       
//       for (size_t i = 0; i < 4; ++i) {
//         const uint32_t tile_index = triangles[current_tri_index].next_level[i];
//         //std::cout << "tile_index " << tile_index << '\n';
//         if (intersect_tile(tile_index, ray)) return tile_index;
//       }
// 
//       return UINT32_MAX;
      
//       auto fast_triangles_arr = reinterpret_cast<render::packed_fast_triangle_t*>(accel_triangles->ptr());
//       const size_t accel_triangles_count = tri_count_d(accel_struct_detail_level);
//       auto points_arr = reinterpret_cast<glm::vec4*>(points->ptr());
//       
//       for (size_t i = 0; i < accel_triangles_count; ++i) {
//         const auto &fast_triangle = fast_triangles_arr[i];
//         const glm::vec4 a = points_arr[fast_triangle.points.x];
//         const glm::vec4 b = points_arr[fast_triangle.points.y];
//         const glm::vec4 c = points_arr[fast_triangle.points.z];
//         
//         const bool intersect = intersect_tri(a, b, c, ray);
//         if (!intersect) continue;
//         
//         
//       }
    }
    
    glm::vec4 surface_normal(const std::tuple<glm::vec4, glm::vec4, glm::vec4> &points) {
      const glm::vec4 u_vec = std::get<1>(points) - std::get<0>(points);
      const glm::vec4 v_vec = std::get<2>(points) - std::get<0>(points);
      
      const glm::vec3 norm = glm::normalize(glm::cross(glm::vec3(u_vec), glm::vec3(v_vec)));
      return glm::vec4(norm, 0.0f);
    }
    
    glm::dvec4 surface_normald(const std::tuple<glm::dvec4, glm::dvec4, glm::dvec4> &points) {
      const glm::dvec4 u_vec = std::get<1>(points) - std::get<0>(points);
      const glm::dvec4 v_vec = std::get<2>(points) - std::get<0>(points);
      
      const glm::dvec3 norm = glm::normalize(glm::cross(glm::dvec3(u_vec), glm::dvec3(v_vec)));
      return glm::dvec4(norm, 0.0);
    }
    
    bool map::intersect_container(const uint32_t &tri_index, const utils::ray &ray) const {
      ASSERT(tri_index < triangles.size());
      const triangle &tri = triangles[tri_index];
      ASSERT(!triangles.empty());
      auto points_arr = reinterpret_cast<glm::vec4*>(points->ptr());
      
      // короче я что то тут упускаю серьезно и не проверяю
      // не могу добиться чтобы луч попадал во все треугольники
      // может с лучем какая то беда? хотя вроде бы я кучу времени исправлял все это дело
      // я более чем уверен что проблема лежит где то в этих переделах
      // кажется проблема лежит в переходе от одного треугольника к другому
      
      using local_vec4 = glm::vec4;
      using local_vec3 = glm::vec3;
      using local_float = float;
      
      const local_vec4 point1 = points_arr[tri.points[0]];
      const local_vec4 point2 = points_arr[tri.points[1]];
      const local_vec4 point3 = points_arr[tri.points[2]];
      
      const local_vec4 point1_normal = local_vec4(local_vec3(point1) / local_float(world_radius), 0.0f);
      const local_vec4 point2_normal = local_vec4(local_vec3(point2) / local_float(world_radius), 0.0f);
      const local_vec4 point3_normal = local_vec4(local_vec3(point3) / local_float(world_radius), 0.0f);
      const local_vec4 point4 = local_vec4(local_vec3(point1_normal) * local_float(world_radius + local_float(maximum_world_elevation) * 20.0), 1.0f); // так по каким то причинам получше
      const local_vec4 point5 = local_vec4(local_vec3(point2_normal) * local_float(world_radius + local_float(maximum_world_elevation) * 20.0), 1.0f); // 
      const local_vec4 point6 = local_vec4(local_vec3(point3_normal) * local_float(world_radius + local_float(maximum_world_elevation) * 20.0), 1.0f);
      
      // не все треугольники я могу проверить таким образом, почему?
      // ошибки с float'ом? вряд ли, треугольники были направлены в неверную сторону
      // ошибки с переходом от дого треугольника к другому
      // похоже что когда я смотрю на это дело сверху все более менее работает
      // 
      const std::tuple<local_vec4, local_vec4, local_vec4> triangles[] = {
        std::tie(point1, point2, point3), // я кажется могу не проверять этот треугольник
        std::tie(point4, point5, point6), // вверх
        
        // я вроде бы все проверил
        std::tie(point1, point4, point2), // боковой треугольник -Z низ
        std::tie(point1, point4, point3), // боковой треугольник +Z низ
        std::tie(point4, point5, point2), // боковой треугольник -Z верх
        std::tie(point4, point6, point3), // боковой треугольник +Z верх
        
        std::tie(point2, point3, point5), // нижний треугольник низ
        std::tie(point3, point6, point5), // нижний треугольник верх
      };
      
      const size_t triangle_count = sizeof(triangles) / sizeof(triangles[0]);
      
      // мне нужно получить ближайший треугольник, а хотя нет
      static_assert(triangle_count == 8);
      for (size_t i = 0; i < triangle_count; ++i) {
        local_float tmp;
        const bool ret = intersect_tri(std::get<0>(triangles[i]), std::get<1>(triangles[i]), std::get<2>(triangles[i]), ray, tmp);
        //const bool ret = test_intersect_funcd(std::get<0>(triangles[i]), std::get<1>(triangles[i]), std::get<2>(triangles[i]), ray, tmp);
        if (ret) return true;
      }

      return false;
    }
    
    bool map::intersect_tri(const glm::vec4 &v0, const glm::vec4 &v1, const glm::vec4 &v2, const utils::ray &ray, float &t) const {
      const glm::vec3 v0v1 = v1 - v0;
      const glm::vec3 v0v2 = v2 - v0;
      const glm::vec3 pvec = glm::cross(glm::vec3(ray.dir), v0v2);
      const float det = glm::dot(v0v1, pvec);
      
#define LOCAL_EPSILON EPSILON

      if (det > -LOCAL_EPSILON && det < LOCAL_EPSILON) return false;

      const float invDet = 1.0f / det;

      float u,v;

      const glm::vec3 tvec = ray.pos - v0;
      u = glm::dot(tvec, pvec) * invDet;

      if (u < 0.0f || u > 1.0f) return false;

      const glm::vec3 qvec = glm::cross(tvec, v0v1);
      v = glm::dot(glm::vec3(ray.dir), qvec) * invDet;
      if (v < 0.0f || u + v > 1.0f) return false;

      t = glm::dot(v0v2, qvec) * invDet;

      return true;
    }
    
    bool map::intersect_tile(const uint32_t &tile_index, const utils::ray &ray) const {
      auto tiles_arr = reinterpret_cast<render::light_map_tile_t*>(tiles->ptr());
      auto points_arr = reinterpret_cast<glm::vec4*>(points->ptr());
      const auto &tile_data = render::unpack_data(tiles_arr[tile_index]);
      const uint32_t points_count = render::is_pentagon(tile_data) ? 5 : 6;
      //const uint32_t point_a_index = tile_data.points[0];
      const uint32_t point_a_index = tile_data.center;
      for (uint32_t i = 0; i < points_count; ++i) {
        const uint32_t point_b_index = tile_data.points[i];
        const uint32_t point_c_index = tile_data.points[(i+1)%points_count];
        
        float dist = 124414.0f;
        const bool ret = intersect_tri(points_arr[point_a_index], points_arr[point_b_index], points_arr[point_c_index], ray, dist);
        if (ret) return true;
      }

      return false;
    }
    
#define INTERSECT 2
#define INSIDE 1
#define OUTSIDE 0

    int test_hierarchic_triangle(const std::initializer_list<glm::vec4> &triangles, const map::aabb &box) {
      int result = INSIDE;
      for (size_t i = 0; i < triangles.size(); ++i) {
        const glm::vec4 plane = glm::vec4(triangles.begin()[i][0], triangles.begin()[i][1], triangles.begin()[i][2], 0.0f);
        const float p3 = triangles.begin()[i][3];

        const float d = glm::dot(box.pos,      plane);

        const float r = glm::dot(box.extents, glm::abs(plane));

        const float d_p_r = d + r;
        const float d_m_r = d - r;

        //frustumPlane.w
        if (d_p_r < -p3) {
          result = OUTSIDE;
          break;
        } else if (d_m_r < -p3) result = INTERSECT;
      }

      return result;
    }
    
    size_t map::add_object(const object &obj) {
      static const std::function<size_t(const uint32_t &, const object &)> req_func = [this] (const uint32_t &index, const object &obj) {
        ASSERT(index < triangles.size());
        const auto &tri = triangles[index];
        
        // нужно найти 5 нормалей
        // а хотя нужно проверить 3 треугольника на самом деле
        // что делать в случае если объект стоит на перечении? хороший вопрос, 
        // если с нижними уровнями понятно что мы просто пихаем наверх, то 
        // что делать с объектами которые не попадают в первые треугольники?
        // добавлять в какой нибудь, так как скорее всего и луч и фрустум заденут объект 
        
        auto points_arr = reinterpret_cast<glm::vec4*>(points->ptr());
        
        using local_vec4 = glm::vec4;
        using local_vec3 = glm::vec3;
        using local_float = float;
        
        const local_vec4 point1 = points_arr[tri.points[0]];
        const local_vec4 point2 = points_arr[tri.points[1]];
        const local_vec4 point3 = points_arr[tri.points[2]];
        
        const local_vec4 point1_normal = local_vec4(local_vec3(point1) / local_float(world_radius), 0.0f);
        const local_vec4 point2_normal = local_vec4(local_vec3(point2) / local_float(world_radius), 0.0f);
//         const local_vec4 point3_normal = local_vec4(local_vec3(point3) / local_float(world_radius), 0.0f);
        const local_vec4 point4 = local_vec4(local_vec3(point1_normal) * local_float(world_radius + local_float(maximum_world_elevation) * 20.0), 1.0f); // так по каким то причинам получше
        const local_vec4 point5 = local_vec4(local_vec3(point2_normal) * local_float(world_radius + local_float(maximum_world_elevation) * 20.0), 1.0f); // 
//         const local_vec4 point6 = local_vec4(local_vec3(point3_normal) * local_float(world_radius + local_float(maximum_world_elevation) * 20.0), 1.0f);
        
        // 3 треугольника
        
        ASSERT(glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)) == glm::vec3(0.0f, 0.0f, 1.0f));
        
        if (tri.current_level != detail_level) {
          const auto normal1 = surface_normal(std::tie(point1, point4, point3)); // glm::normalize(
          const auto normal2 = surface_normal(std::tie(point1, point4, point2));
          const auto normal3 = surface_normal(std::tie(point2, point3, point5));
          
          const local_float d1 = glm::dot(normal1, point1);
          const local_float d2 = glm::dot(normal2, point1);
          const local_float d3 = glm::dot(normal3, point2);
          
          const auto normal1d = glm::vec4(glm::vec3(normal1), d1);
          const auto normal2d = glm::vec4(glm::vec3(normal2), d2);
          const auto normal3d = glm::vec4(glm::vec3(normal3), d3);
          
          const int ret = test_hierarchic_triangle({normal1d, normal2d, normal3d}, obj.aabb);
          
          if (ret == OUTSIDE) return SIZE_MAX;
          
          if (ret == INSIDE) {
            for (uint32_t j = 0; j < 4; ++j) {
              const uint32_t triangle_index = tri.next_level[j];
              const size_t ret = req_func(triangle_index, obj);
              if (ret != SIZE_MAX) return ret;
            }
          }
        }
        
        // по идее можно добавить в первый пересекающий треугольник
        
        ASSERT(index < triangles_data.size());
        for (size_t j = 0; j < triangles_data[index].size(); ++j) {
          if (triangles_data[index][j].user_data == nullptr) {
            triangles_data[index][j] = obj;
            const size_t ret = (size_t(index) << 32) | j;
            return ret;
          }
        }
        
        const size_t new_index = triangles_data[index].size();
        triangles_data[index].push_back(obj);
        const size_t ret = (size_t(index) << 32) | new_index;
        return ret;
      };
      
      ASSERT(obj.user_data != nullptr);
      // тут нужно обойти все фигуры
      
      const uint32_t current_detail_level = 0;
      for (size_t i = 0; i < 20*power4(current_detail_level); ++i) {
        const uint32_t triangle_index = i;
        const size_t ret = req_func(triangle_index, obj);
        if (ret != SIZE_MAX) return ret;
      }
      
      return SIZE_MAX;
    }
    
    void map::remove_object(const size_t &place) {
      const uint32_t tri_index = uint32_t(place >> 32);
      const uint32_t obj_index = uint32_t(place);
      
      ASSERT(triangles.size() == triangles_data.size());
      
      if (tri_index >= triangles.size()) throw std::runtime_error("Bad object removing");
      if (obj_index >= triangles_data[tri_index].size()) throw std::runtime_error("Bad object removing");
      if (triangles_data[tri_index][obj_index].user_data == nullptr) throw std::runtime_error("Could not find object at this place");
      
      triangles_data[tri_index][obj_index].user_data = nullptr;
    }
    
    bool test_ray_aabb(const utils::ray &ray, const map::aabb &box, float &dist) {
      const glm::vec4 box_max = box.pos + box.extents;
      const glm::vec4 box_min = box.pos - box.extents;
      
      const auto ray_inv_dir = 1.0f / ray.dir;
      float t1 = (box_min[0] - ray.pos[0])*ray_inv_dir[0];
      float t2 = (box_max[0] - ray.pos[0])*ray_inv_dir[0];

      float tmin = glm::min(t1, t2);
      float tmax = glm::max(t1, t2);

      for (int i = 1; i < 3; ++i) {
          t1 = (box_min[i] - ray.pos[i])*ray_inv_dir[i];
          t2 = (box_max[i] - ray.pos[i])*ray_inv_dir[i];

          tmin = glm::max(tmin, glm::min(glm::min(t1, t2), tmax));
          tmax = glm::min(tmax, glm::max(glm::max(t1, t2), tmin));
      }
      
      if (tmax > glm::max(tmin, 0.0f)) {
        dist = tmin > 0.0f ? tmin : tmax;
        return true;
      }
      
      return false;
    }
    
    void* map::cast_ray_object(const utils::ray &ray, float &dist) const {
      // тут мы делаем то же самое что и в случае с тайлами
      // только проверяем боксы вместо тайлов
      
      static const std::function<void*(const utils::ray &ray, const uint32_t &tri_index, float &distance)> reck = [this] (const utils::ray &ray, const uint32_t &tri_index, float &distance) -> void* {
        const bool ret = intersect_container(tri_index, ray);
        if (!ret) return nullptr;
        
        const triangle &tri = triangles[tri_index];
        
        const uint32_t level = tri.current_level;
        
        float global_dist = MAX_VALUE;
        void* global_obj = nullptr;
        if (level != detail_level) {
          for (size_t i = 0; i < 4; ++i) {
            const uint32_t tri_index = tri.next_level[i];
            
            float dist = MAX_VALUE;
            void* obj = reck(ray, tri_index, dist);
            
            if (obj == nullptr) continue;
            
            if (dist < global_dist) {
              global_dist = dist;
              global_obj = obj;
            }
          }
        }
        
        for (size_t i = 0; i < triangles_data[tri_index].size(); ++i) {
          float dist = 1000000.0f;
          const bool ret = test_ray_aabb(ray, triangles_data[tri_index][i].aabb, dist);
          
          if (ret && dist < global_dist) {
            global_dist = dist;
            global_obj = triangles_data[tri_index][i].user_data;
          }
        }
        
        distance = global_dist;
        return global_obj;
      };
      
      void* object_ptr = nullptr;
      float distance = 1000000.0f;
      for (size_t i = 0; i < 20*power4(0); ++i) {
        float d = 1000000.0f;
        void* ptr = reck(ray, i, d);
        
        if (d < distance) {
          distance = d;
          object_ptr = ptr;
        }
      }
      
      if (distance < dist) {
        dist = distance;
        return object_ptr;
      }
      
      return nullptr;
    }
    
    bool map::test_triangle_frustum(const uint32_t &tri_index, const utils::frustum &frustum) const {
      const auto &tri = triangles[tri_index];
      auto points_arr = reinterpret_cast<glm::vec4*>(points->ptr());
        
      using local_vec4 = glm::vec4;
      using local_vec3 = glm::vec3;
      using local_float = float;
      
      local_vec4 points_vec[6];
      points_vec[0] = points_arr[tri.points[0]];
      points_vec[1] = points_arr[tri.points[1]];
      points_vec[2] = points_arr[tri.points[2]];
      
      const local_vec4 point1_normal = local_vec4(local_vec3(points_vec[0]) / local_float(world_radius), 0.0f);
      const local_vec4 point2_normal = local_vec4(local_vec3(points_vec[1]) / local_float(world_radius), 0.0f);
      const local_vec4 point3_normal = local_vec4(local_vec3(points_vec[2]) / local_float(world_radius), 0.0f);
      points_vec[3] = glm::vec4(glm::vec3(point1_normal) * local_float(world_radius + local_float(maximum_world_elevation) * 20.0), 1.0f); // так по каким то причинам получше
      points_vec[4] = glm::vec4(glm::vec3(point2_normal) * local_float(world_radius + local_float(maximum_world_elevation) * 20.0), 1.0f); // 
      points_vec[5] = glm::vec4(glm::vec3(point3_normal) * local_float(world_radius + local_float(maximum_world_elevation) * 20.0), 1.0f);
      
      local_vec4 min = points_vec[0], max = points_vec[0];
      for (uint32_t i = 1; i < 6; ++i) {
        min = glm::min(min, points_vec[i]);
        max = glm::max(max, points_vec[i]);
      }
      
      const aabb box{
                (max + min) / 2.0f,
        glm::abs(max - min) / 2.0f
      };
      
      ASSERT(box.pos.w == 1.0f);
      ASSERT(box.extents.w == 0.0f);
      
      const int ret = test_hierarchic_triangle({
        frustum.planes[0],
        frustum.planes[1],
        frustum.planes[2],
        frustum.planes[3],
        frustum.planes[4],
        frustum.planes[5]
      }, box);
      
      return ret != OUTSIDE;
    }
    
    // эта функция рискует оказаться довольно тяжелой
    int32_t map::frustum_culling(const utils::frustum &frustum, std::vector<void*> &objects) const {
      // тут нам потребуется мультитрединг, для этого нам всего лишь нужно правильно увеличить размер массива
      
      static const std::function<void(const uint32_t &tri_index, const utils::frustum &frustum, const size_t &max_count, std::atomic<size_t> &counter, std::vector<void*> &objects)> req_func = 
        [this] (const uint32_t &tri_index, const utils::frustum &frustum, const size_t &max_count, std::atomic<size_t> &counter, std::vector<void*> &objects) {
        // 1. проверяем треугольник на фрустум
        // 2. обходим треугольники ниже по иерархии
        // 3. проверяем все объекты
        
        const bool test = test_triangle_frustum(tri_index, frustum); // кажется не работает
        if (!test) return;
        
        const auto &tri = triangles[tri_index];
        if (tri.current_level != detail_level) {
          for (uint32_t i = 0; i < 4; ++i) {
            const uint32_t tri_index = tri.next_level[i];
            req_func(tri_index, frustum, max_count, std::ref(counter), std::ref(objects));
          }
        }
        
        for (size_t i = 0; i < triangles_data[tri_index].size(); ++i) {
          if (triangles_data[tri_index][i].user_data == nullptr) continue;
          
          const int ret = test_hierarchic_triangle({
            frustum.planes[0],
            frustum.planes[1],
            frustum.planes[2],
            frustum.planes[3],
            frustum.planes[4],
            frustum.planes[5]
          }, triangles_data[tri_index][i].aabb);
          
          if (ret == OUTSIDE) continue;
          
          const size_t index = counter.fetch_add(1);
          if (index >= max_count) return;
          objects[index] = triangles_data[tri_index][i].user_data;
        }
      };
      
      auto pool = global::get<dt::thread_pool>();
      std::atomic<size_t> counter(0);
      const size_t max_count = objects.size(); // по идее мы можем взять эту инфу у вектора, а вектор увеличивать извне
      
//       for (size_t i = 0; i < 20*power4(0); ++i) {
//         pool->submitnr(req_func, i, frustum, max_count, std::ref(counter), std::ref(objects));
//       }
//       
//       pool->compute();
//       pool->wait();
      
      // так получше работает, что странно
      utils::submit_works(pool, triangles_data.size(), [&] (const size_t &start, const size_t &count) {
        for (size_t i = start; i < start+count; ++i) {
          for (size_t j = 0; j < triangles_data[i].size(); ++j) {
            const auto &obj = triangles_data[i][j];
            if (obj.user_data == nullptr) continue;
            
            // кажется работает, возможно просто нужно проверить треугольники как в шейдере
            // а хотя зачем вообще я проверяю 2 раза? я же могу использовать данные предыдущего кадра
            const int ret = test_hierarchic_triangle({
              frustum.planes[0],
              frustum.planes[1],
              frustum.planes[2],
              frustum.planes[3],
              frustum.planes[4],
              frustum.planes[5]
            }, obj.aabb);
            
            if (ret == OUTSIDE) continue;
            
            const size_t index = counter.fetch_add(1);
            if (index >= max_count) return;
            objects[index] = obj.user_data;
          }
        }
      });
      
      // неплохо было бы вернуть на сколько необходимо увеличить размер, не, наверное так себе идея
      // нужно выходить если недостаточно места в буфере
      if (counter >= max_count) return -1;
      return counter;
    }
    
    const render::light_map_tile_t map::get_tile(const uint32_t &index) const {
      ASSERT(index < tiles_count());
//       std::unique_lock<std::mutex> lock(mutex);
      auto tiles_arr = reinterpret_cast<render::light_map_tile_t*>(tiles->ptr());
      return tiles_arr[index];
    }
    
    const render::map_tile_t* map::get_tile_ptr(const uint32_t &index) const {
      if (index >= tiles_count()) return nullptr;
      auto tiles_arr = reinterpret_cast<render::map_tile_t*>(tiles->ptr());
      return &tiles_arr[index];
    }
    
    render::map_tile_t* map::get_tile_ptr(const uint32_t &index) {
      if (index >= tiles_count()) return nullptr;
      auto tiles_arr = reinterpret_cast<render::map_tile_t*>(tiles->ptr());
      return &tiles_arr[index];
    }
    
    const render::map_tile_t* map::get_tile_ptr_lua(const uint32_t &index) const {
      return get_tile_ptr(index-1);
    }
    
    const glm::vec4 map::get_point(const uint32_t &index) const {
      ASSERT(index < points_count());
//       std::unique_lock<std::mutex> lock(mutex);
      auto points_arr = reinterpret_cast<glm::vec4*>(points->ptr());
      return points_arr[index];
    }
    
    uint32_t map::points_count() const {
//       std::unique_lock<std::mutex> lock(mutex);
      return points->info().size / sizeof(glm::vec4);
    }
    
    uint32_t map::tiles_count() const {
//       std::unique_lock<std::mutex> lock(mutex);
      return tiles->info().size / sizeof(render::light_map_tile_t);
    }
    
    uint32_t map::accel_triangles_count() const {
//       std::unique_lock<std::mutex> lock(mutex);
      return accel_triangles->info().size / sizeof(render::packed_fast_triangle_t);
    }
    
    uint32_t map::triangles_count() const {
//       std::unique_lock<std::mutex> lock(mutex);
      return triangles.size();
    }
    
    void map::set_tile_data(const devils_engine::map::tile* tile, const uint32_t &index) {
      ASSERT(index < tiles_count());
      std::unique_lock<std::mutex> lock(mutex);
      
      auto tiles_arr = reinterpret_cast<render::light_map_tile_t*>(tiles->ptr());
      const render::map_tile_t map_tile{
        tile->index,
        {GPU_UINT_MAX},
        {GPU_UINT_MAX},
        0.0f,
        {tile->neighbours[0].points[0], tile->neighbours[1].points[0], tile->neighbours[2].points[0], tile->neighbours[3].points[0], tile->neighbours[4].points[0], tile->neighbours[5].points[0]},
        {tile->neighbours[0].index, tile->neighbours[1].index, tile->neighbours[2].index, tile->neighbours[3].index, tile->neighbours[4].index, tile->neighbours[5].index},
        GPU_UINT_MAX,
        GPU_UINT_MAX,
        GPU_UINT_MAX,
        0
      };
      tiles_arr[index] = pack_data(map_tile);
    }
    
    void map::set_point_data(const glm::vec3 &point, const uint32_t &index) {
      ASSERT(index < points_count());
      std::unique_lock<std::mutex> lock(mutex);
      auto points_arr = reinterpret_cast<glm::vec4*>(points->ptr());
      points_arr[index] = glm::vec4(point, 1.0f);
    }
    
    void map::set_tile_indices(const uint32_t &triangle_index, const glm::uvec3 &points, const std::vector<uint32_t> &indices, const uint32_t &offset, const uint32_t &count, const bool has_pentagon) {
      std::unique_lock<std::mutex> lock(mutex);
      auto triangles_arr = reinterpret_cast<render::packed_fast_triangle_t*>(accel_triangles->ptr());
      auto tile_indices_arr = reinterpret_cast<uint32_t*>(tile_indices->ptr()); // важно не забыть как мы храним индексы
      
      ASSERT(indices.size() == count);
      
      triangles_arr[triangle_index].points[0] = points[0];
      triangles_arr[triangle_index].points[1] = points[1];
      triangles_arr[triangle_index].points[2] = points[2];
      triangles_arr[triangle_index].data[0] = offset;
      triangles_arr[triangle_index].data[1] = count;
      triangles_arr[triangle_index].data[2] = uint32_t(has_pentagon);
      
      memcpy(&tile_indices_arr[offset], indices.data(), sizeof(uint32_t)*count);
    }
    
    void map::flush_data() {
      std::unique_lock<std::mutex> lock(mutex);
      auto accel_triangles_gpu = device->create(yavf::BufferCreateInfo::buffer(accel_triangles->info().size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT), VMA_MEMORY_USAGE_GPU_ONLY);
      auto tile_indices_gpu = device->create(yavf::BufferCreateInfo::buffer(tile_indices->info().size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT), VMA_MEMORY_USAGE_GPU_ONLY);
      
      auto task = device->allocateTransferTask();
      task->begin();
      task->copy(accel_triangles, accel_triangles_gpu);
      task->copy(tile_indices, tile_indices_gpu);
      task->end();
      
      task->start();
      task->wait();
      device->deallocate(task);
      
      device->destroy(accel_triangles);
      device->destroy(tile_indices);
      
      accel_triangles = accel_triangles_gpu;
      tile_indices = tile_indices_gpu;
      
//       auto pool = device->descriptorPool(DEFAULT_DESCRIPTOR_POOL_NAME);
      
//       yavf::DescriptorSetLayout tiles_data_layout = device->setLayout(TILES_DATA_LAYOUT_NAME);
//       {
//         yavf::DescriptorLayoutMaker dlm(device);
//         tiles_data_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL)
//                                .binding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL)
//                                .binding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL)
//                                .binding(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL)
//                                .binding(4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL)
//                                .create(TILES_DATA_LAYOUT_NAME);
//       }
      
      {
        tiles_set->at(0) = {tiles, 0, tiles->info().size, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
        tiles_set->at(1) = {biomes, 0, biomes->info().size, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
        tiles_set->at(2) = {points, 0, points->info().size, 0, 2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
        tiles_set->at(3) = {accel_triangles, 0, accel_triangles->info().size, 0, 3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
        tiles_set->at(4) = {tile_indices, 0, tile_indices->info().size, 0, 4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
        tiles_set->at(5) = {structures, 0, structures->info().size, 0, 5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
        tiles_set->at(6) = {tile_object_indices, 0, tile_object_indices->info().size, 0, 6, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
        tiles_set->at(7) = {army_data_buffer, 0, army_data_buffer->info().size, 0, 7, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
        tiles_set->update();
        
//         yavf::DescriptorMaker dm(device);
//         auto desc = dm.layout(tiles_data_layout).create(pool)[0];
//         size_t index = desc->add({tiles, 0, tiles->info().size, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
//                        desc->add({biomes, 0, biomes->info().size, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
//                        desc->add({points, 0, points->info().size, 0, 2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
//                        desc->add({accel_triangles, 0, accel_triangles->info().size, 0, 3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
//                        desc->add({tile_indices, 0, tile_indices->info().size, 0, 4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
//         desc->update();
//         tiles->setDescriptor(desc, index);
      }
    }
    
    void map::flush_points() { // TODO: добавить все ресурсы в gpu память (!)
      auto points_gpu = device->create(yavf::BufferCreateInfo::buffer(sizeof(glm::vec4)*points_count_d(detail_level), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT), VMA_MEMORY_USAGE_GPU_ONLY);
      
      auto task = device->allocateTransferTask();
      task->begin();
      task->copy(points, points_gpu);
      task->end();
      
      task->start();
      task->wait();
      device->deallocate(task);
      
      device->destroy(points);
      
      points = points_gpu;
      
      tiles_set->at(0) = {tiles, 0, tiles->info().size, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
      tiles_set->at(1) = {biomes, 0, biomes->info().size, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
      tiles_set->at(2) = {points, 0, points->info().size, 0, 2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
      tiles_set->at(3) = {accel_triangles, 0, accel_triangles->info().size, 0, 3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
      tiles_set->at(4) = {tile_indices, 0, tile_indices->info().size, 0, 4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
      tiles_set->at(5) = {structures, 0, structures->info().size, 0, 5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
      tiles_set->at(6) = {tile_object_indices, 0, tile_object_indices->info().size, 0, 6, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
      tiles_set->at(7) = {army_data_buffer, 0, army_data_buffer->info().size, 0, 7, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
      tiles_set->update();
    }
    
    void map::flush_structures() {
      tiles_set->at(5) = {structures, 0, structures->info().size, 0, 5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
      tiles_set->update(5);
    }
    
//     void map::set_tile_biom(const uint32_t &tile_index, const uint32_t &biom_index) {
//       auto tiles_arr = reinterpret_cast<render::light_map_tile_t*>(tiles->ptr());
//       tiles_arr[tile_index].tile_indices.y = biom_index;
//     }
//     
//     void map::set_tile_tectonic_plate(const uint32_t &tile_index, const uint32_t &tectonic_plate_index) {
//       auto tiles_arr = reinterpret_cast<render::light_map_tile_t*>(tiles->ptr());
//       tiles_arr[tile_index].tile_indices.z = tectonic_plate_index;
//     }

    void map::set_tile_color(const uint32_t &tile_index, const render::color_t &color) {
      ASSERT(tile_index < tiles_count());
      std::unique_lock<std::mutex> lock(mutex);
      auto tiles_arr = reinterpret_cast<render::light_map_tile_t*>(tiles->ptr());
      tiles_arr[tile_index].tile_indices.z = color.container;
    }
    
    void map::set_tile_texture(const uint32_t &tile_index, const render::image_t &texture) {
      ASSERT(tile_index < tiles_count());
      std::unique_lock<std::mutex> lock(mutex);
      auto tiles_arr = reinterpret_cast<render::light_map_tile_t*>(tiles->ptr());
      tiles_arr[tile_index].tile_indices.y = texture.container;
    }
    
    void map::set_tile_height(const uint32_t &tile_index, const float &tile_hight) {
      ASSERT(tile_index < tiles_count());
      std::unique_lock<std::mutex> lock(mutex);
      auto tiles_arr = reinterpret_cast<render::light_map_tile_t*>(tiles->ptr());
      tiles_arr[tile_index].tile_indices.w = glm::floatBitsToUint(tile_hight);
    }
    
    void map::set_tile_height_lua(const uint32_t &tile_index, const float &tile_hight) {
      set_tile_height(tile_index-1, tile_hight);
    }
    
    void map::set_tile_border_data(const uint32_t &tile_index, const uint32_t &offset, const uint32_t &size) {
      ASSERT(tile_index < tiles_count());
      ASSERT(size <= render::border_size_mask);
      ASSERT(offset < render::border_offset_mask);
      const uint32_t packed_border_data = (size << 28) | (offset & render::border_offset_mask);
      std::unique_lock<std::mutex> lock(mutex);
      auto tiles_arr = reinterpret_cast<render::light_map_tile_t*>(tiles->ptr());
      tiles_arr[tile_index].packed_data4[0] = packed_border_data;
    }
    
    void map::set_tile_connections_data(const uint32_t &tile_index, const uint32_t &offset, const uint32_t &size) {
      ASSERT(tile_index < tiles_count());
      ASSERT(size <= render::connections_size_mask);
      ASSERT(offset < render::connections_offset_mask);
      const uint32_t packed_connections_data = (size << 28) | (offset & render::connections_offset_mask);
      std::unique_lock<std::mutex> lock(mutex);
      auto tiles_arr = reinterpret_cast<render::light_map_tile_t*>(tiles->ptr());
      tiles_arr[tile_index].packed_data4[1] = packed_connections_data;
    }
    
    uint32_t map::get_tile_objects_index(const uint32_t &tile_index, const uint32_t &data_index) const {
      //using array_data_type = render::additional_data_t;
      using array_data_type = c_tile_data;
      
      ASSERT(tile_index < tiles_count());
      const uint32_t data_count = sizeof(array_data_type) / sizeof(uint32_t);
      ASSERT(data_index < data_count);
      UNUSED_VARIABLE(data_count);
      const uint32_t lesser_data_count = sizeof(glm::uvec4) / sizeof(uint32_t);
      auto array = reinterpret_cast<array_data_type*>(tile_object_indices->ptr());
      
      const uint32_t index1 = data_index / lesser_data_count;
      const uint32_t index2 = data_index % lesser_data_count;
      
      return array[tile_index].data[index1][index2];
    }
    
    void map::set_tile_objects_index(const uint32_t &tile_index, const uint32_t &data_index, const uint32_t &data) {
      //using array_data_type = render::additional_data_t;
      using array_data_type = c_tile_data;
      
      ASSERT(tile_index < tiles_count());
      const uint32_t data_count = sizeof(array_data_type) / sizeof(uint32_t);
      ASSERT(data_index < data_count);
      UNUSED_VARIABLE(data_count);
      const uint32_t lesser_data_count = sizeof(glm::uvec4) / sizeof(uint32_t);
      
      auto array = reinterpret_cast<array_data_type*>(tile_object_indices->ptr());
      const uint32_t index1 = data_index / lesser_data_count;
      const uint32_t index2 = data_index % lesser_data_count;
      
      array[tile_index].data[index1][index2] = data;
    }
    
    bool map::tile_objects_index_comp_swap(const uint32_t &tile_index, const uint32_t &data_index, uint32_t &comp, const uint32_t &data) {
      using array_data_type = c_tile_data;
      
      ASSERT(tile_index < tiles_count());
      const uint32_t data_count = sizeof(array_data_type) / sizeof(uint32_t);
      ASSERT(data_index < data_count);
      UNUSED_VARIABLE(data_count);
      const uint32_t lesser_data_count = sizeof(glm::uvec4) / sizeof(uint32_t);
      
      auto array = reinterpret_cast<array_data_type*>(tile_object_indices->ptr());
      const uint32_t index1 = data_index / lesser_data_count;
      const uint32_t index2 = data_index % lesser_data_count;
      
      return array[tile_index].data[index1][index2].compare_exchange_strong(comp, data);
    }
    
    uint32_t map::allocate_army_data() {
      auto data_ptr = reinterpret_cast<render::army_data_t*>(army_data_buffer->ptr());
      if (free_army_slot != UINT32_MAX) {
        const uint32_t next_index = glm::floatBitsToUint(data_ptr[free_army_slot].data.w);
        const uint32_t current_index = free_army_slot;
        free_army_slot = next_index;
        
        data_ptr[current_index].data = glm::vec4(0.0f, 0.0f, 0.0f, glm::uintBitsToFloat(UINT32_MAX));
        return current_index;
      }
      
      const uint32_t current_index = armies_count;
      ++armies_count;
      
      if (current_index * sizeof(render::army_data_t) >= army_data_buffer->info().size) {
        const size_t prev_size = army_data_buffer->info().size;
        auto ptr = new char[prev_size];
        memcpy(ptr, data_ptr, prev_size);
        army_data_buffer->resize(prev_size*2);
        data_ptr = reinterpret_cast<render::army_data_t*>(army_data_buffer->ptr());
        memcpy(data_ptr, ptr, prev_size);
        delete [] ptr;
        
        tiles_set->at(7) = {army_data_buffer, 0, army_data_buffer->info().size, 0, 7, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
        tiles_set->update(7);
      }
      
      data_ptr[current_index].data = glm::vec4(0.0f, 0.0f, 0.0f, glm::uintBitsToFloat(UINT32_MAX));
      return current_index;
    }
    
    void map::release_army_data(const uint32_t &index) {
      ASSERT(index * sizeof(render::army_data_t) < army_data_buffer->info().size); 
      ASSERT(index < armies_count);
      auto data_ptr = reinterpret_cast<render::army_data_t*>(army_data_buffer->ptr());
      data_ptr[index].data.w = glm::uintBitsToFloat(free_army_slot);
      free_army_slot = index;
    }
    
    void map::set_army_pos(const uint32_t &index, const glm::vec3 &pos) {
      ASSERT(index * sizeof(render::army_data_t) < army_data_buffer->info().size); 
      ASSERT(index < armies_count);
      auto data_ptr = reinterpret_cast<render::army_data_t*>(army_data_buffer->ptr());
      data_ptr[index].data.x = pos.x;
      data_ptr[index].data.y = pos.y;
      data_ptr[index].data.z = pos.z;
    }
    
    void map::set_army_image(const uint32_t &index, const render::image_t &img) {
      ASSERT(index * sizeof(render::army_data_t) < army_data_buffer->info().size); 
      ASSERT(index < armies_count);
      auto data_ptr = reinterpret_cast<render::army_data_t*>(army_data_buffer->ptr());
      data_ptr[index].data.w = glm::uintBitsToFloat(img.container);
    }
    
    glm::vec3 map::get_army_pos(const uint32_t &index) const {
      ASSERT(index * sizeof(render::army_data_t) < army_data_buffer->info().size); 
      ASSERT(index < armies_count);
      auto data_ptr = reinterpret_cast<render::army_data_t*>(army_data_buffer->ptr());
      return glm::vec3(data_ptr[index].data.x, data_ptr[index].data.y, data_ptr[index].data.z);
    }
    
    render::image_t map::get_army_image(const uint32_t &index) const {
      ASSERT(index * sizeof(render::army_data_t) < army_data_buffer->info().size); 
      ASSERT(index < armies_count);
      auto data_ptr = reinterpret_cast<render::army_data_t*>(army_data_buffer->ptr());
      return {glm::floatBitsToUint(data_ptr[index].data.w)};
    }
    
    float map::get_tile_height(const uint32_t &tile_index) const {
      ASSERT(tile_index < tiles_count());
      std::unique_lock<std::mutex> lock(mutex);
      auto tiles_arr = reinterpret_cast<render::light_map_tile_t*>(tiles->ptr());
      return glm::uintBitsToFloat(tiles_arr[tile_index].tile_indices.w);
    }
    
    float map::get_tile_height_lua(const uint32_t &tile_index) const {
      return get_tile_height(tile_index-1);
    }
    
    void map::copy_biomes(const seasons* s) {
      yavf::Buffer staging(device, yavf::BufferCreateInfo::buffer(
        sizeof(render::packed_biome_data_t)*MAX_BIOMES_COUNT, VK_BUFFER_USAGE_TRANSFER_SRC_BIT
      ), VMA_MEMORY_USAGE_CPU_ONLY);
      
      static_assert(sizeof(render::packed_biome_data_t)*MAX_BIOMES_COUNT == sizeof(render::biome_data_t)*MAX_BIOMES_COUNT);
      memcpy(staging.ptr(), s->biomes, sizeof(render::biome_data_t)*MAX_BIOMES_COUNT);
      
      auto task = device->allocateTransferTask();
      task->begin();
      task->copy(&staging, biomes);
      task->end();
      
      task->start();
      task->wait();
      device->deallocate(task);
    }
    
    void map::set_tile_biome(const seasons* s) {
      std::unique_lock<std::mutex> lock(mutex);
      auto tiles_arr = reinterpret_cast<render::light_map_tile_t*>(tiles->ptr());
      for (size_t i = 0; i < tiles_count(); ++i) {
        const uint8_t index = s->get_tile_biome(s->current_season, i);
        const uint32_t mask = 0x00ffffff;
        const uint32_t final_container = uint32_t(index) << 24 | mask;
        tiles_arr[i].packed_data4[2] = final_container;
      }
    }
    
    void map::set_tile_structure_index(const uint32_t &tile_index, const uint32_t &struct_index) {
      ASSERT(tile_index < tiles_count());
      ASSERT(struct_index < render::maximum_structure_types);
      auto tiles_arr = reinterpret_cast<render::light_map_tile_t*>(tiles->ptr());
      const uint8_t biome_index = uint8_t(tiles_arr[tile_index].packed_data4[2] >> 24);
      tiles_arr[tile_index].packed_data4[2] = uint32_t(biome_index) << 24 | struct_index;
    }
    
    enum map::status map::status() const {
      return s;
    }
    
    void map::set_status(const enum map::status s) {
      this->s = s;
    }
    
    size_t map::memory_size() const {
      size_t mem = 0;
      mem += points->info().size;
      mem += tiles->info().size;
      mem += accel_triangles->info().size;
      mem += tile_indices->info().size;
      mem += biomes->info().size;
      mem += triangles.size() * sizeof(triangles[0]);
      mem += sizeof(*this);
      return mem;
    }
    
    const uint32_t map::detail_level;
    const uint32_t map::accel_struct_detail_level;
#ifndef _WIN32
    const float map::world_radius;
#endif
  }
}
