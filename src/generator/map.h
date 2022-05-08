#ifndef DEVILS_ENGINE_GENERATOR_MAP_H
#define DEVILS_ENGINE_GENERATOR_MAP_H

#include <cstdint>
#include <memory>
#include <vector>
#include <mutex>
#include "utils/utility.h"
#include "utils/ray.h"
#include "render/shared_structures.h"

// а нужен ли мне generator::map? не уверен
// ну хотя явно нужно подчистить структуру карты

namespace devils_engine {
  namespace render {
    struct container;
    struct map_data;
  }
  
  namespace generator {
    struct map {
//       struct aabb {
//         glm::vec4 pos;
//         glm::vec4 extents;
//       };
      
//       struct object {
//         struct aabb aabb;
//         void* user_data;
//       };
      
      struct triangle {
        uint32_t points[3];
        uint32_t current_level;
        uint32_t upper_level_index;
        uint32_t next_level[4];
      };
      
      // уровень детализации карты лучше задавать через константы
      constexpr static uint32_t detail_level = 7;
      constexpr static uint32_t accel_struct_detail_level = 4;
      static const size_t maximum_army_count = 10000;
      constexpr static const float world_radius = WORLD_RADIUS_CONSTANT;
      constexpr static const float maximum_world_elevation = 15.0f; // кажется этого должно хватить для 
      
      constexpr static size_t tri_count_d(const size_t &detail_level) {
        return 20 * power4(detail_level);
      }
      
      constexpr static size_t icosahedron_points_count_t(const size_t &tri_count) {
        return (tri_count*3-12*5)/6+12;
      }
      
      constexpr static size_t icosahedron_points_count_d(const size_t &detail_level) {
        return icosahedron_points_count_t(tri_count_d(detail_level));
      }
      
      constexpr static size_t hex_count_t(const size_t &tri_count) {
        return icosahedron_points_count_t(tri_count) + tri_count;
      }
      
      constexpr static size_t hex_count_d(const size_t &detail_level) {
        return hex_count_t(tri_count_d(detail_level));
      }
      
      constexpr static size_t points_count_t(const size_t &tri_count) {
        return icosahedron_points_count_t(tri_count) + tri_count + tri_count*6/2;
      }
      
      constexpr static size_t points_count_d(const size_t &detail_level) {
        return points_count_t(tri_count_d(detail_level));
      }
      
      render::container* render_container;
      std::unique_ptr<render::map_data> data;
//       std::vector<glm::vec4> points;
      std::vector<triangle> triangles;
//       std::vector<std::vector<object>> triangles_data;
      float max_triangle_size; // 23.8457f
      glm::mat4 world_matrix;
      
      mutable std::mutex mutex;
      
      struct create_info {
        render::container* render_container;
      };
      map(const create_info &info);
      ~map();
      
      uint32_t points_count() const;
      uint32_t tiles_count() const;
      uint32_t accel_triangles_count() const;
      uint32_t triangles_count() const;
      
      void set_tile_color(const uint32_t &tile_index, const render::color_t &color);
      void set_tile_height(const uint32_t &tile_index, const float &tile_hight);
      float get_tile_height(const uint32_t &tile_index) const;
      const render::map_tile_t* get_tile_ptr(const uint32_t &index) const;
      glm::vec4 get_point(const uint32_t &index) const;
      
      uint32_t cast_ray(const utils::ray &ray, float &ray_dist) const;
      bool intersect_container(const uint32_t &tri_index, const utils::ray &ray) const;
      bool intersect_tri(const glm::vec4 &v0, const glm::vec4 &v1, const glm::vec4 &v2, const utils::ray &ray, float &t) const;
      bool intersect_tile(const uint32_t &tile_index, const utils::ray &ray) const;
    };
  }
}

#endif
