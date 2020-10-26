#ifndef TARGETS_H
#define TARGETS_H

#include "target.h"
#include "utils/utility.h"
// #include "bin/figures.h"
// #include <atomic>

namespace yavf {
  class Buffer;
  class Device;
}

namespace devils_engine {
  namespace core {
    struct city_type;
  }
  
  namespace map {
    struct container;
  }

  namespace render {
    struct buffers : public target {
      yavf::Device* device;
      yavf::Buffer* uniform;
      yavf::Buffer* matrices;

      buffers(yavf::Device* device);
      ~buffers();

      void update_projection_matrix(const glm::mat4 &matrix);
      void update_view_matrix(const glm::mat4 &matrix);
      void update_pos(const glm::vec3 &pos);
      void update_dir(const glm::vec3 &dir);
      void update_zoom(const float &zoom);
//       void set_tile_data(const map::tile &tile, const uint32_t &index);
//       void set_point_data(const glm::vec3 &point, const uint32_t &index);
//       void set_tile_indices(const uint32_t &triangle_index, const std::vector<uint32_t> &indices, const uint32_t &offset, const uint32_t &count, const bool has_pentagon);
      void recreate(const uint32_t &width, const uint32_t &height) override;
      glm::mat4 get_matrix() const;
      glm::mat4 get_proj() const;
      glm::mat4 get_view() const;
      glm::mat4 get_inv_proj() const;
      glm::mat4 get_inv_view() const;
      glm::mat4 get_inv_view_proj() const;
      glm::vec4 get_pos() const;
      glm::vec4 get_dir() const;
    };
    
    // в вулкане у нас есть новая перменная gl_VertexIndex
    // она по идее заполняется тем что у нас лежит в индексном буфере
    // а значит мы можем не использовать вершинный буфер, а только индексный
    // + нам не нужно делать оптимизаторы для границ и соединений
    // их мы можем записать в тайлдата, в виде оффсет + количество (не больше 6) (3 бита на количество 29 на оффсет)
    // границы и соединения максимум 6 на один тайл, мы можем нарисовать их вместе
    struct world_map_buffers : public target {
      yavf::Device* device;
      yavf::Buffer* border_buffer; // вообще эти вещи после создания могут пойти в гпу спокойно (границы мы будем все же переделывать довольно часто)
      yavf::Buffer* border_types;
      yavf::Buffer* tiles_connections; // а вот соединения не будем
      yavf::Buffer* structure_buffer;  // эти штуки могут быть постоянно в гпу памяти, но совсем иногда их нужно обновлять
      
      world_map_buffers(yavf::Device* device);
      ~world_map_buffers();
      
      void recreate(const uint32_t &width, const uint32_t &height) override;
      void set_structure_data(const uint32_t &size, core::city_type* data); 
      // скопировать наверное нужно отдельно, нам потребуется скорпировать еще и данные структур
    };
  }
}

#endif
