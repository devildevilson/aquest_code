#ifndef DEVILS_ENGINE_RENDER_TARGETS_H
#define DEVILS_ENGINE_RENDER_TARGETS_H

#include "target.h"
// #include "stage.h"
#include "interfaces.h"
#include "utils/utility.h"
#include "vulkan_hpp_header.h"
#include "parallel_hashmap/phmap.h"

namespace devils_engine {
  namespace core {
    struct titulus;
    struct dynasty;
  }
  
  namespace render {
    struct container;
    class resource_provider;
    
    struct buffers : public target {
      container* c;
      vk_buffer_data uniform; // еще было бы неплохо положить дополнительное случайное число
//       vk_buffer_data matrices;
//       vk_buffer_data common;
      
      void* uniform_camera;
      void* uniform_matrices;
      void* uniform_common;
      
      // тут мы должны расположить индексы следующих слоев геральдики
      // то есть это что, пачка массивов оканчивающаяся MAX? можно даже сделать размер массива + сам массив
      // как указывать геральдики в титулах и династиях? таблица с индексами, хотя возможно даже лучше 
      // указать функцию генерации последовательности
      // по идее индексы геральдики можно перегенерировать при загрузке сохранения, 
      // по дополнительным династиям и по дополнительным титулам
      vk_buffer_data heraldy; // данные о геральдике нам много где нужны (везде кроме меню)
      vk_buffer_data heraldy_indices;
      
      vk::DescriptorSet uniform_set;
      
      vk::DescriptorSetLayout heraldy_layout;
      vk::DescriptorSet heraldy_set;
      
//       size_t current_indices_size;
      phmap::flat_hash_map<std::string, uint32_t> heraldy_layers_map; // было бы неплохо сделать отдельный буфер
//       std::vector<new_heraldy_indices> new_indices;

      buffers(container* c);
      ~buffers();

      void update_projection_matrix(const glm::mat4 &matrix);
      void update_view_matrix(const glm::mat4 &matrix);
      void update_pos(const glm::vec3 &pos);
      void update_dir(const glm::vec3 &dir);
      void update_zoom(const float &zoom);
      void update_cursor_dir(const glm::vec4 &cursor_dir);
      void update_dimensions(const uint32_t &width, const uint32_t &height);
      void update_time(const uint32_t &time);
      void update_persistent_state(const uint32_t &state);
      void update_application_state(const uint32_t &state);
      void update_turn_state(const uint32_t &state);
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
      glm::vec4 get_cursor_dir() const;
      float get_zoom() const;
      
      void resize_heraldy_buffer(const size_t &heraldy_layers_buffer_size);
      // мне нужно скопировать старую инфу обратно + добавить новуюб не хочется делать индексы в памяти хоста, 
      // другое дело что во время игры может быть что мы создадим много дворян и еще несколько титулов
      // все довольно просто во время хода компа, хотя в общем то какая разница, нужно просто придумать еще один класс который будет этим заниматься
//       void resize_heraldy_indices_buffer(const size_t &heraldy_indices_size, const size_t &heraldy_indices_count);
    };
    
    // в вулкане у нас есть новая перменная gl_VertexIndex
    // она по идее заполняется тем что у нас лежит в индексном буфере
    // а значит мы можем не использовать вершинный буфер, а только индексный
    // + нам не нужно делать оптимизаторы для границ и соединений
    // их мы можем записать в тайлдата, в виде оффсет + количество (не больше 6) (3 бита на количество 29 на оффсет)
    // границы и соединения максимум 6 на один тайл, мы можем нарисовать их вместе
    struct world_map_buffers : public target, public copy_stage {
      container* c;
      vk_buffer_data border_buffer; // вообще эти вещи после создания могут пойти в гпу спокойно (границы мы будем все же переделывать довольно часто)
      vk_buffer_data border_types;
//       vk_buffer_data tiles_connections; // а вот соединения не будем
//       vk_buffer_data structure_buffer;  // эти штуки могут быть постоянно в гпу памяти, но совсем иногда их нужно обновлять
      
      vk_buffer_data tiles_renderable;
      vk_buffer_data gpu_tiles_renderable;
//       vk_buffer_data tiles_exploration;
//       vk_buffer_data tiles_visibility;
      
      vk::DescriptorPool pool;
      vk::DescriptorSetLayout borders_data_layout;
      vk::DescriptorSet border_set;
//       vk::DescriptorSet types_set;
      vk::DescriptorSetLayout tiles_rendering_data_layout;
      vk::DescriptorSet tiles_rendering_data;
      
      world_map_buffers(container* c);
      ~world_map_buffers();
      
      void recreate(const uint32_t &width, const uint32_t &height) override;
      void copy(resource_provider* ctx, vk::CommandBuffer task) const override;
      
      //void set_structure_data(const uint32_t &size, core::city_type* data); 
      // скопировать наверное нужно отдельно, нам потребуется скорпировать еще и данные структур
      
      void resize_border_buffer(const size_t &size);
      void resize_border_types(const size_t &size);
      
      void set_map_exploration(const uint32_t &tile_index, const bool explored);
      void set_map_visibility(const uint32_t &tile_index, const bool visible);
      bool get_map_exploration(const uint32_t &tile_index) const;
      bool get_map_visibility(const uint32_t &tile_index) const;
      bool get_map_renderable(const uint32_t &tile_index) const;
      void clear_renderable();
    };
  }
}

#endif
