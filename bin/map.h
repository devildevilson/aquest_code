#ifndef MAP_H
#define MAP_H

#include <cstddef>
#include <cstdint>
#include <vector>
#include <atomic>
#include <mutex>
#include "utils/utility.h"
#include "render/shared_structures.h"
#include "utils/ray.h"
#include "utils/frustum.h"

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
  return size_t(1) << pow*2;
}

constexpr size_t div4(const size_t &num) {
  return num >> 2;
}

namespace devils_engine {
  namespace map {
    struct tile;
  }
  
  namespace core {
    struct seasons;
    
    struct map {
      enum class status {
        initial,
        rendering,
        valid
      };
      
      // уровень детализации карты лучше задавать через константы
      static const uint32_t detail_level = 7;
      static const uint32_t accel_struct_detail_level = 4;
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
      
      struct aabb {
        glm::vec4 pos;
        glm::vec4 extents;
      };
      
      struct user_data {
        enum class type {
          army,
          city,
          structure,
          title,
          count
        };
        enum type type;
        uint32_t index;
      };
      
      struct object {
        struct aabb aabb;
        void* user_data;
      };
      
      // теперь нужно определиться с тем где будут храниться указатели на объекты на карте
      // либо на последнем уровне разбиения, либо могут храниться на каждом в зависимости от того попадают ли объекты полностью
      // последнее требует функции проверки нахождения внутри фигуры состоящей из 5 плоскостей, 
      // по идее должно быть по схожему алгоритму с фрустумом
      struct triangle {
        uint32_t points[3];
        uint32_t current_level;
        uint32_t upper_level_index;
        uint32_t next_level[4];
        // тут должна быть возможность добавлять произвольные данные (какие? юзердата указатель на что? может быть два индекса?)
        // нужно учесть: армии, города, структуры, знамена, скорее всего что то еще
        // дело в том что данные у нас копируются и не стоит тут добавлять еще данные
      };
      
      // почти все мы тут можем сгенерировать
      // кроме высот и матрицы поворота
      yavf::Device* device;
      yavf::Buffer* points;
      yavf::Buffer* tiles;
      yavf::Buffer* accel_triangles;
      yavf::Buffer* tile_indices;
      // биомы: это текстура + цвет, мы это дело можем записать в данные тайла
      // + какие то объекты на карте, что с ними? нужно придумать какой то способ нарисовать спрайт так чтобы он выглядел максимально 3дшно
      // у нас есть жесткие ограничения по тому как мы видим объекты на карте, мы их видим в основном сверху и чуть с юга
      yavf::Buffer* biomes;
      yavf::Buffer* structures;
      yavf::Buffer* tile_object_indices; // тут по идее мы будем хранить кучу всяких индексов
      yavf::Buffer* army_data_buffer;
      yavf::Buffer* provinces; // скорее всего не пригодится (и далее)
      yavf::Buffer* faiths;
      yavf::Buffer* cultures;
      yavf::DescriptorSet* tiles_set;
      std::vector<triangle> triangles;
      std::vector<std::vector<object>> triangles_data;
      float max_triangle_size; // 23.8457f
      uint32_t free_army_slot;
      uint32_t armies_count;
      
      glm::mat4 world_matrix;
      
      std::atomic<status> s;
      mutable std::mutex mutex;
      
      struct create_info {
        yavf::Device* device;
      };
      map(const create_info &info);
      ~map();
      
      uint32_t cast_ray(const utils::ray &ray) const;
      bool intersect_container(const uint32_t &tri_index, const utils::ray &ray) const;
      bool intersect_tri(const glm::vec4 &v0, const glm::vec4 &v1, const glm::vec4 &v2, const utils::ray &ray, float &t) const;
      bool intersect_tile(const uint32_t &tile_index, const utils::ray &ray) const;
      
      // добавляем объект, нужно их добавлять каждый ход? нужно сделать как можно меньше добавлений
      // нужно запомнить какую то информацию для быстрого удаления
      size_t add_object(const object &obj);
      void remove_object(const size_t &place);
      void* cast_ray_object(const utils::ray &ray, float &dist) const;
      bool test_triangle_frustum(const uint32_t &tri_index, const utils::frustum &frustum) const;
      int32_t frustum_culling(const utils::frustum &frustum, std::vector<void*> &objects) const;
      
      const render::light_map_tile_t get_tile(const uint32_t &index) const;
      const render::map_tile_t* get_tile_ptr(const uint32_t &index) const;
      render::map_tile_t* get_tile_ptr(const uint32_t &index);
      const render::map_tile_t* get_tile_ptr_lua(const uint32_t &index) const;
      const glm::vec4 get_point(const uint32_t &index) const;
      
      uint32_t points_count() const;
      uint32_t tiles_count() const;
      uint32_t accel_triangles_count() const;
      uint32_t triangles_count() const;
      
      void set_tile_data(const devils_engine::map::tile* tile, const uint32_t &index);
      void set_point_data(const glm::vec3 &point, const uint32_t &index);
      void set_tile_indices(const uint32_t &triangle_index, const glm::uvec3 &points, const std::vector<uint32_t> &indices, const uint32_t &offset, const uint32_t &count, const bool has_pentagon);
      void flush_data();
      void flush_points();
      void flush_structures();
      //void set_tile_biom(const uint32_t &tile_index, const uint32_t &biom_index);
      void set_tile_color(const uint32_t &tile_index, const render::color_t &color);
      void set_tile_texture(const uint32_t &tile_index, const render::image_t &texture);
      //void set_tile_tectonic_plate(const uint32_t &tile_index, const uint32_t &tectonic_plate_index);
      void set_tile_height(const uint32_t &tile_index, const float &tile_hight);
      void set_tile_height_lua(const uint32_t &tile_index, const float &tile_hight);
      void set_tile_border_data(const uint32_t &tile_index, const uint32_t &offset, const uint32_t &size);
      void set_tile_connections_data(const uint32_t &tile_index, const uint32_t &offset, const uint32_t &size);
      
      uint32_t get_tile_objects_index(const uint32_t &tile_index, const uint32_t &data_index) const;
      void set_tile_objects_index(const uint32_t &tile_index, const uint32_t &data_index, const uint32_t &data);
      bool tile_objects_index_comp_swap(const uint32_t &tile_index, const uint32_t &data_index, uint32_t &comp, const uint32_t &data);
      
      uint32_t allocate_army_data();
      void release_army_data(const uint32_t &index);
      void set_army_pos(const uint32_t &index, const glm::vec3 &pos);
      void set_army_image(const uint32_t &index, const render::image_t &img);
      glm::vec3 get_army_pos(const uint32_t &index) const;
      render::image_t get_army_image(const uint32_t &index) const;
      
      float get_tile_height(const uint32_t &tile_index) const;
      float get_tile_height_lua(const uint32_t &tile_index) const;
      
      void copy_biomes(const seasons* s);
      void set_tile_biome(const seasons* s);
      
      void set_tile_structure_index(const uint32_t &tile_index, const uint32_t &struct_index);
      
      enum status status() const;
      void set_status(const enum status s);
      size_t memory_size() const;
    };
  }
}

#endif
