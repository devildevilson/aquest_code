#include "map.h"

#include "render/map_data.h"

namespace devils_engine {
  namespace generator {
    map::map(const create_info &info) : render_container(info.render_container) {}
    map::~map() {}
    
    uint32_t map::points_count() const {
      return points_count_d(detail_level);
    }
    
    uint32_t map::tiles_count() const {
      return hex_count_d(detail_level);
    }
    
    uint32_t map::accel_triangles_count() const {
      return tri_count_d(accel_struct_detail_level);
    }
    
    uint32_t map::triangles_count() const {
      return tri_count_d(detail_level);
    }
    
    void map::set_tile_color(const uint32_t &tile_index, const render::color_t &color) {
      ASSERT(tile_index < tiles_count());
      std::unique_lock<std::mutex> lock(mutex);
      auto tiles_arr = reinterpret_cast<render::map_tile_t*>(data->tiles.ptr);
      tiles_arr[tile_index].color = color;
    }
    
    void map::set_tile_height(const uint32_t &tile_index, const float &tile_height) {
      std::unique_lock<std::mutex> lock(mutex);
      auto tiles_arr = reinterpret_cast<render::map_tile_t*>(data->tiles.ptr);
      tiles_arr[tile_index].height = tile_height;
    }
    
    float map::get_tile_height(const uint32_t &tile_index) const {
      std::unique_lock<std::mutex> lock(mutex);
      auto tiles_arr = reinterpret_cast<render::map_tile_t*>(data->tiles.ptr);
      return tiles_arr[tile_index].height;
    }
    
    const render::map_tile_t* map::get_tile_ptr(const uint32_t &index) const {
      std::unique_lock<std::mutex> lock(mutex);
      auto tiles_arr = reinterpret_cast<const render::map_tile_t*>(data->tiles.ptr);
      return &tiles_arr[index];
    }
    
    glm::vec4 map::get_point(const uint32_t &index) const {
      ASSERT(index < points_count());
      std::unique_lock<std::mutex> lock(mutex);
      auto points_arr = reinterpret_cast<const glm::vec4*>(data->points.ptr);
      return points_arr[index];
    }
    
#define MAX_VALUE 1000000.0f
    uint32_t cast_ray_reck(const map* m, const utils::ray &ray, const uint32_t &tri_index, float &distance) {
      const bool ret = m->intersect_container(tri_index, ray);
      if (!ret) return UINT32_MAX;
      
//       auto ctx = global::get<systems::map_t>()->core_context;
      
      const map::triangle &tri = m->triangles[tri_index];
      
      const uint32_t level = tri.current_level;
      if (level == map::detail_level) {
        uint32_t final_tile_index = UINT32_MAX;
        float final_tile_dist = MAX_VALUE;
        for (size_t i = 0; i < 4; ++i) {
          const uint32_t tile_index = tri.next_level[i];
          // тут нужно проверить дальность до тайла + проверить пересечение со стенками
          // нужно ли чекать ближайший треугольник? не уверен что это необходимо
          
          //const auto tile = ctx->get_entity<core::tile>(tile_index);
          const auto tile = m->get_tile_ptr(tile_index);
          
          const uint32_t p_count = tile->points[5] == UINT32_MAX ? 5 : 6;
          //const uint32_t p_count = tile->neighbors_count();
//           const uint32_t point_a_index = tile->center;
          const float height = tile->height;
//           const uint32_t height_layer = render::compute_height_layer(height);
//           const float final_height = render::layer_height * height_layer;
//           const float computed_height = final_height * render::render_tile_height;
          const float computed_height = height;
          
//           glm::vec4 center = m->get_point(point_a_index);
//           glm::vec4 center_height = center + glm::normalize(glm::vec4(glm::vec3(center), 0.0f)) * (computed_height);
          glm::vec4 local_points[6];
          glm::vec4 local_points_height[6];
          for (uint32_t j = 0; j < p_count; ++j) {
            const uint32_t point1_index = tile->points[j];
            const glm::vec4 point = m->get_point(point1_index);
            const glm::vec4 point_normal = glm::vec4(glm::vec3(point) / map::world_radius, 0.0f);
            const glm::vec4 point_height = point + point_normal * (computed_height);
            local_points[j] = point;
            local_points_height[j] = point_height;
          }
          
          // проверяем теугольники тайла, возможно имеет смысл брать треугольники как при рендеринге - меньше треугольников
          static const uint32_t indices_hex[] = {0, 1, 5, 2, 4, 3};
          static const uint32_t indices_pen[] = {0, 1, 4, 2, 3};
          uint32_t index1 = 0;
          uint32_t index2 = 1;
          uint32_t index3 = 2;
          for (uint32_t j = 0; j < p_count-2; ++j) {
            const uint32_t final_index1 = p_count == 6 ? indices_hex[index1] : indices_pen[index1];
            const uint32_t final_index2 = p_count == 6 ? indices_hex[index2] : indices_pen[index2];
            const uint32_t final_index3 = p_count == 6 ? indices_hex[index3] : indices_pen[index3];
            
            const auto point1 = local_points_height[final_index1];
            const auto point2 = local_points_height[final_index2];
            const auto point3 = local_points_height[final_index3];
            
            float dist = MAX_VALUE;
            const bool ret = m->intersect_tri(point1, point2, point3, ray, dist);
            if (ret && dist < final_tile_dist) {
              final_tile_index = tile_index;
              final_tile_dist = dist;
              break;
            }
            
            index1 = index2;
            index2 = index3;
            index3 = (index3+1)%p_count;
          }
          
//           for (uint32_t j = 0; j < p_count; ++j) {
//             const uint32_t b_index = j;
//             const uint32_t c_index = (j+1)%p_count;
//             
//             ASSERT(c_index < p_count);
//             
//             float dist = MAX_VALUE;
// //               const bool ret = intersect_tri(get_point(point_a_index), get_point(point_b_index), get_point(point_c_index), ray, dist);
//             const bool ret = m->intersect_tri(center_height, local_points_height[b_index], local_points_height[c_index], ray, dist);
//             if (ret && dist < final_tile_dist) {
//               final_tile_index = tile_index;
//               final_tile_dist = dist;
//               //break;
//             }
//           }
          
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
            const bool ret1 = m->intersect_tri(std::get<0>(wall_triangle[0]), std::get<1>(wall_triangle[0]), std::get<2>(wall_triangle[0]), ray, dist1);
            const bool ret2 = m->intersect_tri(std::get<0>(wall_triangle[1]), std::get<1>(wall_triangle[1]), std::get<2>(wall_triangle[1]), ray, dist2);
            
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
        const uint32_t index = cast_ray_reck(m, ray, tri_index, dist); // может я тут что то не так делаю
        
        if (index == UINT32_MAX) continue;
        
        if (dist < global_dist) {
          global_dist = dist;
          global_index = index;
        }
      }
      
      distance = global_dist;
      return global_index;
    }

    uint32_t map::cast_ray(const utils::ray &ray, float &ray_dist) const {      
      size_t current_detail_level = 0;
      
      float dist = MAX_VALUE;
      uint32_t final_tile = UINT32_MAX;
      for (size_t i = 0; i < 20*power4(current_detail_level); ++i) {
        float local_dist = MAX_VALUE;
        const uint32_t tile_index = cast_ray_reck(this, ray, i, local_dist);
        
        if (local_dist < dist) {
          dist = local_dist;
          final_tile = tile_index;
        }
      }
      
      ray_dist = dist;
      return final_tile;
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
      //auto points_arr = reinterpret_cast<glm::vec4*>(data->points.ptr);
//       const auto points_arr = points.data();
      
      // короче я что то тут упускаю серьезно и не проверяю
      // не могу добиться чтобы луч попадал во все треугольники
      // может с лучем какая то беда? хотя вроде бы я кучу времени исправлял все это дело
      // я более чем уверен что проблема лежит где то в этих переделах
      // кажется проблема лежит в переходе от одного треугольника к другому
      
      using local_vec4 = glm::vec4;
      using local_vec3 = glm::vec3;
      using local_float = float;
      
      const local_vec4 point1 = get_point(tri.points[0]);
      const local_vec4 point2 = get_point(tri.points[1]);
      const local_vec4 point3 = get_point(tri.points[2]);
      
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

      //if (det > -LOCAL_EPSILON && det < LOCAL_EPSILON) return false;
      if (std::abs(det) < LOCAL_EPSILON) return false;

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
      auto tiles_arr = reinterpret_cast<render::light_map_tile_t*>(data->tiles.ptr);
      const auto &tile_data = render::unpack_data(tiles_arr[tile_index]);
      const uint32_t points_count = render::is_pentagon(tile_data) ? 5 : 6;
      //const uint32_t point_a_index = tile_data.points[0];
      const uint32_t point_a_index = tile_data.center;
      for (uint32_t i = 0; i < points_count; ++i) {
        const uint32_t point_b_index = tile_data.points[i];
        const uint32_t point_c_index = tile_data.points[(i+1)%points_count];
        
        float dist = MAX_VALUE;
        const bool ret = intersect_tri(get_point(point_a_index), get_point(point_b_index), get_point(point_c_index), ray, dist);
        if (ret) return true;
      }

      return false;
    }
  }
}
