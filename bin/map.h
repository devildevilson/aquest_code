#ifndef MAP_H
#define MAP_H

#include <cstddef>
#include <cstdint>
#include <vector>
#include "utils/utility.h"
#include "render/shared_structures.h"
#include "utils/ray.h"

// все данные карты должны хранится здесь
// точки, гексы, треугольники, биомы, провинции, религии, культуры (это из основных)
// + несколько буферов неосновных данных (возможно пригодятся разные данные полученные при генерации)
// здесь должны быть только данные + несколько алгоритмов (каст луча, поиск и проч)
// эти данные мы должны использовать при отрисовки карты

namespace yavf {
  class Buffer;
  class Device;
  class DescriptorSet;
}

constexpr size_t power4(const uint32_t &pow) {
  return 1 << pow*2;
}

constexpr size_t div4(const size_t &num) {
  return num >> 2;
}

namespace devils_engine {
  namespace map {
    struct tile;
  }
  
  namespace core {
    struct map {
      enum class status {
        initial,
        valid
      };
      
      // уровень детализации карты лучше задавать через константы
      static const uint32_t detail_level = 7;
      static const uint32_t accel_struct_detail_level = 4;
      
      static constexpr size_t tri_count_d(const size_t &detail_level) {
        return 20 * power4(detail_level);
      }
      
      static constexpr size_t icosahedron_points_count_t(const size_t &tri_count) {
        return (tri_count*3-12*5)/6+12;
      }
      
      static constexpr size_t icosahedron_points_count_d(const size_t &detail_level) {
        return icosahedron_points_count_t(tri_count_d(detail_level));
      }
      
      static constexpr size_t hex_count_t(const size_t &tri_count) {
        return icosahedron_points_count_t(tri_count) + tri_count;
      }
      
      static constexpr size_t hex_count_d(const size_t &detail_level) {
        return hex_count_t(tri_count_d(detail_level));
      }
      
      static constexpr size_t points_count_t(const size_t &tri_count) {
        return icosahedron_points_count_t(tri_count) + tri_count + tri_count*6/2;
      }
      
      static constexpr size_t points_count_d(const size_t &detail_level) {
        return points_count_t(tri_count_d(detail_level));
      }
      
      struct triangle {
        uint32_t points[3];
        uint32_t current_level;
        uint32_t upper_level_index;
        uint32_t next_level[4];
      };
      
      yavf::Device* device;
      yavf::Buffer* points;
      yavf::Buffer* tiles;
      yavf::Buffer* accel_triangles;
      yavf::Buffer* tile_indices;
      yavf::Buffer* biomes;
      yavf::Buffer* provinces;
      yavf::Buffer* faiths;
      yavf::Buffer* cultures;
      yavf::DescriptorSet* tiles_set;
      std::vector<triangle> triangles;
      float max_triangle_size; // 23.8457f
      
      status s;
      
      struct create_info {
        yavf::Device* device;
      };
      map(const create_info &info);
      ~map();
      
      uint32_t cast_ray(const utils::ray &ray) const;
      bool intersect_container(const uint32_t &tri_index, const utils::ray &ray) const;
      bool intersect_tri(const glm::vec4 &v0, const glm::vec4 &v1, const glm::vec4 &v2, const utils::ray &ray) const;
      bool intersect_tile(const uint32_t &tile_index, const utils::ray &ray) const;
      
      const render::light_map_tile_t & get_tile(const uint32_t &index) const;
      const glm::vec4 & get_point(const uint32_t &index) const;
      
      uint32_t points_count() const;
      uint32_t tiles_count() const;
      uint32_t accel_triangles_count() const;
      uint32_t triangles_count() const;
      
      void set_tile_data(const devils_engine::map::tile* tile, const uint32_t &index);
      void set_point_data(const glm::vec3 &point, const uint32_t &index);
      void set_tile_indices(const uint32_t &triangle_index, const glm::uvec3 &points, const std::vector<uint32_t> &indices, const uint32_t &offset, const uint32_t &count, const bool has_pentagon);
      void flush_data();
      void set_tile_biom(const uint32_t &tile_index, const uint32_t &biom_index);
      void set_tile_tectonic_plate(const uint32_t &tile_index, const uint32_t &tectonic_plate_index);
      void set_tile_height(const uint32_t &tile_index, const float &tile_hight);
      
      enum status status() const;
      void set_status(const enum status s);
    };
  }
}

#endif
