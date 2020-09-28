#include "map.h"
#include "render/yavf.h"
#include "render/shared_structures.h"
#include "figures.h"
#include <thread>
#include <chrono>

#define MAP_CONTAINER_DESCRIPTOR_POOL_NAME "map_container_descriptor_pool"

namespace devils_engine {
  namespace core {
    map::map(const create_info &info) : 
      device(info.device),
      points(nullptr),
      tiles(nullptr),
      accel_triangles(nullptr),
      biomes(nullptr),
      provinces(nullptr),
      faiths(nullptr),
      cultures(nullptr),
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
      biomes = device->create(yavf::BufferCreateInfo::buffer(sizeof(render::packed_biom_data_t)*10, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
      
      const uint32_t tile_indices_count = std::ceil(float(tiles_count)/float(4));
      tile_indices = device->create(yavf::BufferCreateInfo::buffer(
        sizeof(glm::uvec4)*tile_indices_count, 
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT
      ), VMA_MEMORY_USAGE_CPU_ONLY); // это можно засунуть сразу в гпу память
      
      memset(tile_indices->ptr(), 0, tile_indices->info().size);
      
      size_t accum = 0;
      for (uint32_t i = 0; i < detail_level; ++i) {
        accum += tri_count_d(i);
      }
      accum += tri_count_d(detail_level);
      
      triangles.resize(accum);
      
      yavf::DescriptorSetLayout tiles_data_layout = device->setLayout(TILES_DATA_LAYOUT_NAME);
      if (tiles_data_layout == VK_NULL_HANDLE) {
        yavf::DescriptorLayoutMaker dlm(device);
        tiles_data_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL)
                               .binding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL)
                               .binding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL)
                               .binding(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL)
                               .binding(4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL)
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
      device->destroyDescriptorPool(MAP_CONTAINER_DESCRIPTOR_POOL_NAME);
//       device->destroy(provinces);
//       device->destroy(faiths);
//       device->destroy(cultures);
    }
    
    bool test_intersect_func(const glm::vec4 &v0, const glm::vec4 &v1, const glm::vec4 &v2, const utils::ray &ray) {
      const glm::vec3 v0v1 = v1 - v0;
      const glm::vec3 v0v2 = v2 - v0;
      const glm::vec3 pvec = glm::cross(glm::vec3(ray.dir), v0v2);
      const float det = glm::dot(v0v1, pvec);

      //if (det > -EPSILON) return false;
      if (det < EPSILON) return false;
      //if (glm::abs(det) < EPSILON) return false;

      std::cout << "normal ok" << '\n';

      const float invDet = 1 / det;

      float u,v,t;

      const glm::vec3 tvec = ray.pos - v0;
      u = glm::dot(tvec, pvec) * invDet;

      std::cout << "u " << u << '\n';
      //if (u <= 0.0f || u >= 1.0f) return false;
      if (u < -EPSILON || u > 1+EPSILON) return false;

      const glm::vec3 qvec = glm::cross(tvec, v0v1);
      v = glm::dot(glm::vec3(ray.dir), qvec) * invDet;
      std::cout << "v " << v << '\n';
      //if (v <= 0.0f || u + v >= 1.0f) return false;
      if (v < -EPSILON || u + v > 1+EPSILON) return false;

      t = glm::dot(v0v2, qvec) * invDet;

      (void)t;
      
      std::cout << "triangle ok" << "\n";
      
      return true;
    }
    
    uint32_t map::cast_ray(const utils::ray &ray) const {
      size_t current_detail_level = 0;
      size_t current_tri_index = UINT32_MAX;
      
      ASSERT(20*power4(current_detail_level) == 20);
      for (size_t i = 0; i < 20*power4(current_detail_level); ++i) {
        if (intersect_container(i, ray)) {
          ++current_detail_level;
          current_tri_index = i;
          break;
        }
      }
      
      //ASSERT(current_tri_index != UINT32_MAX); // по идее это условие всегда должно выполняться
      // у нас еще не создана карта
      if (current_tri_index == UINT32_MAX) return UINT32_MAX;
      
      while (current_detail_level <= detail_level) {
        const triangle &tri = triangles[current_tri_index];
//         const uint32_t tri_index_test = current_tri_index;
        current_tri_index = UINT32_MAX;
        for (size_t i = 0; i < 4; ++i) {
          const uint32_t tri_index = tri.next_level[i];
          if (intersect_container(tri_index, ray)) {
            current_tri_index = tri_index;
            //ASSERT(triangles[current_tri_index].current_level == current_detail_level);
            ++current_detail_level;
            //std::cout << "current_tri_index " << current_tri_index << '\n';            
            break;
          }
          
// #ifndef _NDEBUG
//           size_t count = 0;
//           for (size_t i = 0; i < triangles[tri_index].current_level; ++i) {
//             count += 20*power4(i);
//           }
//           
//           const size_t offset = count;
//           const size_t last = offset + 20*power4(triangles[tri_index].current_level);
//           
//           ASSERT(tri_index >= offset && tri_index < last);
// #endif
//           
//         }
// 
//         if (current_tri_index == UINT32_MAX) {
// //           PRINT_VAR("detail_level", detail_level)
// //           PRINT_VAR("current_detail_level", current_detail_level)
//           PRINT_VAR("tri_index_test", tri_index_test)
//           PRINT_VAR("tri.current_level", tri.current_level)
// //           PRINT_VAR("triangles count", (20*power4(current_detail_level-1)))
// //           PRINT_VAR("next level 0", tri.next_level[0])
// //           PRINT_VEC4("ray dir", ray.dir)
// //           auto points_arr = reinterpret_cast<glm::vec4*>(points->ptr());
// //           PRINT_VEC4("p 0", points_arr[tri.points[0]])
// //           PRINT_VEC4("p 1", points_arr[tri.points[1]])
// //           PRINT_VEC4("p 2", points_arr[tri.points[2]])
// //           const glm::vec4 test_point = ray.pos + ray.dir * 20.0f;
// //           PRINT_VEC4("test point", test_point)
//           
//           PRINT_VEC4("ray pos", ray.pos)
//           PRINT_VEC4("ray dir", ray.dir)
//           PRINT_VEC4("ray p  ", (ray.pos + ray.dir * 20.0f))
//           auto points_arr = reinterpret_cast<glm::vec4*>(points->ptr());
//           for (uint32_t i = 0; i < 4; ++i) {
//             const triangle &next_tri = triangles[tri.next_level[i]];
//             const glm::vec4 p0 = points_arr[next_tri.points[0]];
//             const glm::vec4 p1 = points_arr[next_tri.points[1]];
//             const glm::vec4 p2 = points_arr[next_tri.points[2]];
//             PRINT_VAR("index", tri.next_level[i])
//             PRINT_VEC4("p0", p0)
//             PRINT_VEC4("p1", p1)
//             PRINT_VEC4("p2", p2)
//             test_intersect_func(p0, p1, p2, ray);
//           }
        }
        ASSERT(current_tri_index != UINT32_MAX);
      }
      
      ASSERT(triangles[current_tri_index].current_level == detail_level);
      
      for (size_t i = 0; i < 4; ++i) {
        const uint32_t tile_index = triangles[current_tri_index].next_level[i];
        //std::cout << "tile_index " << tile_index << '\n';
        if (intersect_tile(tile_index, ray)) return tile_index;
      }

      return UINT32_MAX;
      
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
    
    bool map::intersect_container(const uint32_t &tri_index, const utils::ray &ray) const {
      const triangle &tri = triangles[tri_index];
      ASSERT(!triangles.empty());
      auto points_arr = reinterpret_cast<glm::vec4*>(points->ptr());
//       PRINT_VAR("p 0", tri.points[0]);
//       PRINT_VAR("p 1", tri.points[1]);
//       PRINT_VAR("p 2", tri.points[2]);
      return intersect_tri(points_arr[tri.points[0]], points_arr[tri.points[1]], points_arr[tri.points[2]], ray);
    }
    
    bool map::intersect_tri(const glm::vec4 &v0, const glm::vec4 &v1, const glm::vec4 &v2, const utils::ray &ray) const {
      const glm::vec3 v0v1 = v1 - v0;
      const glm::vec3 v0v2 = v2 - v0;
      const glm::vec3 pvec = glm::cross(glm::vec3(ray.dir), v0v2);
      const float det = glm::dot(v0v1, pvec);
      
#define LOCAL_EPSILON EPSILON

//       if (det > -EPSILON) return false;
      if (det < LOCAL_EPSILON) return false;
//       const glm::vec3 ray_pos_dir = glm::vec3(-ray.pos);
//       const glm::vec3 pvec2 = glm::cross(ray_pos_dir, v0v2);
//       const float det2 = glm::dot(v0v1, pvec2);
//       if (det2 < LOCAL_EPSILON) return false;
      //if (glm::abs(det) < EPSILON) return false;

//       std::cout << "normal ok" << '\n';

      const float invDet = 1 / det;

      float u,v,t;

      const glm::vec3 tvec = ray.pos - v0;
      u = glm::dot(tvec, pvec) * invDet;

      // std::cout << "u " << u << '\n';
      if (u < 0 || u > 1) return false;
      //if (u < -LOCAL_EPSILON || u > 1+LOCAL_EPSILON) return false;
      //if (u <= 0 || u >= 1) return false;

      const glm::vec3 qvec = glm::cross(tvec, v0v1);
      v = glm::dot(glm::vec3(ray.dir), qvec) * invDet;
      // std::cout << "v " << v << '\n';
      if (v < 0 || u + v > 1) return false;
      //if (v < -LOCAL_EPSILON || u + v > 1+LOCAL_EPSILON) return false;
      //if (v <= 0 || u + v >= 1) return false;

      t = glm::dot(v0v2, qvec) * invDet;

      (void)t;

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

        const bool ret = intersect_tri(points_arr[point_a_index], points_arr[point_b_index], points_arr[point_c_index], ray);
        if (ret) return true;
      }

      return false;
    }
    
    const render::light_map_tile_t map::get_tile(const uint32_t &index) const {
      ASSERT(index < tiles_count());
//       std::unique_lock<std::mutex> lock(mutex);
      auto tiles_arr = reinterpret_cast<render::light_map_tile_t*>(tiles->ptr());
      return tiles_arr[index];
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
        GPU_UINT_MAX
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
      tiles_set->update();
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
    
    float map::get_tile_height(const uint32_t &tile_index) const {
      ASSERT(tile_index < tiles_count());
      std::unique_lock<std::mutex> lock(mutex);
      auto tiles_arr = reinterpret_cast<render::light_map_tile_t*>(tiles->ptr());
      return glm::uintBitsToFloat(tiles_arr[tile_index].tile_indices.w);
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

