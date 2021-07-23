#ifndef STAGES_H
#define STAGES_H

#include <atomic>
#include <mutex>
#include <unordered_set>

#include "utils/utility.h"
#include "utils/frustum.h"

#include "stage.h"
#include "render.h"
#include "shared_structures.h"
#include "shared_render_utility.h"
#include "vulkan_hpp_header.h"

namespace devils_engine {
  namespace render {
    class deffered;
    struct images;
    struct window;
    struct particles;
    struct world_map_buffers;
    struct container;

    class window_next_frame : public stage {
    public:
      window_next_frame();
      void begin() override;
      void proccess(container* ctx) override;
      void clear() override;
    };

    class task_begin : public stage {
    public:
      void begin() override;
      void proccess(container* ctx) override;
      void clear() override;
    };

    class task_end : public stage {
    public:
      void begin() override;
      void proccess(container* ctx) override;
      void clear() override;
    };

    class render_pass_begin : public stage {
    public:
      render_pass_begin(const uint32_t &index);
      void begin() override;
      void proccess(container* ctx) override;
      void clear() override;
    private:
      uint32_t index;
    };

    class render_pass_end : public stage {
    public:
      void begin() override;
      void proccess(container* ctx) override;
      void clear() override;
    };
    
    class tile_optimizer : public stage {
    public:
      static const uint32_t work_group_size = 256;
      
      struct indirect_buffer {
//         VkDrawIndirectCommand pentagon_command;
//         VkDrawIndirectCommand hexagon_command;
        vk::DrawIndexedIndirectCommand pen_tiles_command;
        uint32_t padding1[3];
        vk::DrawIndexedIndirectCommand hex_tiles_command;
        uint32_t padding_hex[3];
        vk::DrawIndirectCommand borders_command;
        uint32_t padding2[4];
        vk::DrawIndirectCommand walls_command;
        uint32_t padding3[4];
        vk::DrawIndirectCommand structures_command;
        uint32_t padding4[4];
        vk::DrawIndirectCommand heraldies_command;
        uint32_t padding5[4];
        glm::uvec4 dispatch_indirect_command;
        
        utils::frustum frustum;
        aabb_t selection_box;
        utils::frustum selection_frustum;
        
        biome_objects_data_t biome_data[MAX_BIOMES_COUNT];
      };
      
      static_assert(sizeof(indirect_buffer) % (sizeof(uint32_t)*4) == 0);
      
      struct create_info {
        vk::Device device;
        vma::Allocator allocator;
        vk::DescriptorSetLayout tiles_data_layout;
        vk::DescriptorSetLayout uniform_layout;
      };
      tile_optimizer(const create_info &info);
      ~tile_optimizer();
      void begin() override;
      void proccess(container* ctx) override;
      void clear() override;
      
      vk::Buffer indirect_buffer() const;
      vk::Buffer tiles_index_buffer() const;
      vk::Buffer borders_index_buffer() const;
      vk::Buffer walls_index_buffer() const;
      vk::Buffer objects_index_buffer() const;
      vk::Buffer structures_index_buffer() const;
      vk::Buffer heraldy_index_buffer() const;
      vk::DescriptorSet get_buffers_set() const;
      vk::DescriptorSetLayout get_buffers_layout() const;
      
      void set_borders_count(const uint32_t &count);
      void set_connections_count(const uint32_t &count);
      void set_max_structures_count(const uint32_t &count); 
      void set_max_heraldy_count(const uint32_t &count); 
      // армейская символика потребует неограниченного количества геральдик...
      // естественно вряд ли это число будет слишком большим (раза в 3-4 больше чем титульных геральдик)
      
      void set_biome_tile_count(const std::array<std::pair<uint32_t, uint32_t>, MAX_BIOMES_COUNT> &data);
      
      void set_border_rendering(const bool value);
      bool is_rendering_border() const;
      
      void set_selection_box(const aabb_t &box);
      void set_selection_frustum(const utils::frustum &frustum);
      
      uint32_t get_borders_indices_count() const;
      uint32_t get_connections_indices_count() const;
      uint32_t get_structures_indices_count() const;
      uint32_t get_heraldies_indices_count() const;
      
      uint32_t get_selection_count() const;
      const uint32_t* get_selection_data() const;
    private:
      vk::Device device;
      vma::Allocator allocator;
      
      vk_buffer_data indirect;
      vk_buffer_data tiles_indices;
      vk_buffer_data borders_indices;
      vk_buffer_data walls_indices;
      vk_buffer_data objects_indices;
      vk_buffer_data structures_indices;
      vk_buffer_data heraldy_indices;
      // лайоут для сета? дескриптор пул?
      vk::DescriptorPool pool;
      vk::DescriptorSetLayout layout;
      vk::DescriptorSet buffers_set;
      vk::PipelineLayout pipe_layout;
      vk::Pipeline pipe;
      
      size_t borders_buffer_size;
      size_t connections_buffer_size;
      size_t structures_buffer_size;
      size_t heraldies_buffer_size;
      
      uint32_t borders_indices_count;
      uint32_t connections_indices_count;
      uint32_t structures_indices_count;
      uint32_t heraldies_indices_count;
      
      bool render_borders;
    };
    
    class tile_objects_optimizer : public stage {
    public:
      struct create_info {
        vk::Device device;
        vma::Allocator allocator;
        vk::DescriptorSetLayout tiles_data_layout;
        vk::DescriptorSetLayout uniform_layout;
        tile_optimizer* opt;
      };
      tile_objects_optimizer(const create_info &info);
      ~tile_objects_optimizer();
      void begin() override;
      void proccess(container* ctx) override;
      void clear() override;
    private:
      vk::Device device;
      vma::Allocator allocator;
      tile_optimizer* opt;
      vk::PipelineLayout p_layout;
      vk::Pipeline pipe;
    };
    
    // вообще надо бы это удалить
//     class tile_borders_optimizer : public stage {
//     public:
//       static const uint32_t work_group_size = 256;
//       
//       struct indirect_buffer {
//         vk::DrawIndirectCommand border_command;
//         glm::uvec4 data;
//         utils::frustum frustum;
//       };
//       
//       static_assert(sizeof(indirect_buffer) % (sizeof(uint32_t)*4) == 0);
//       
//       struct create_info {
//         vk::Device device;
//         vma::Allocator allocator;
//         vk::DescriptorSetLayout tiles_data_layout;
//         vk::DescriptorSetLayout uniform_layout;
//         world_map_buffers* map_buffers;
//       };
//       tile_borders_optimizer(const create_info &info);
//       ~tile_borders_optimizer();
//       void begin() override;
//       void proccess(container* ctx) override;
//       void clear() override;
//       
//       vk::Buffer indirect_buffer() const;
//       vk::Buffer vertices_buffer() const;
//       
//       void set_borders_count(const uint32_t &count);
//     private:
//       vk::Device device;
//       vma::Allocator allocator;
//       vk_buffer_data indirect;
//       vk_buffer_data borders_indices;
//       
//       vk::DescriptorSet set;
//       vk::PipelineLayout pipe_layout;
//       vk::Pipeline pipe;
//       world_map_buffers* map_buffers;
//     };
//     
//     class tile_walls_optimizer : public stage {
//     public:
//       static const uint32_t work_group_size = 256;
//       
//       struct indirect_buffer {
//         VkDrawIndirectCommand walls_command;
//         glm::uvec4 data;
//         utils::frustum frustum;
//       };
//       
//       static_assert(sizeof(indirect_buffer) % (sizeof(uint32_t)*4) == 0);
//       
//       struct create_info {
//         vk::Device device;
//         vk::PhysicalDevice physical_device;
//         world_map_buffers* map_buffers;
//       };
//       tile_walls_optimizer(const create_info &info);
//       ~tile_walls_optimizer();
//       
//       void begin() override;
//       void proccess(container* ctx) override;
//       void clear() override;
//       
//       yavf::Buffer* indirect_buffer() const;
//       yavf::Buffer* vertices_buffer() const;
//       
//       void set_connections_count(const uint32_t &count);
//     private:
//       vk::Device device;
//       vk::PhysicalDevice physical_device;
//       vk::Buffer walls_indices;
//       vk::DescriptorSet set;
//       vk::Pipeline pipe;
//       world_map_buffers* map_buffers;
//     };
    
    class barriers : public stage {
    public:
      void begin() override;
      void proccess(container* ctx) override;
      void clear() override;
    };

    class tile_render : public stage {
    public:
      struct create_info {
        container* cont;
        vk::DescriptorSetLayout tiles_data_layout;
        vk::DescriptorSetLayout images_layout;
        tile_optimizer* opt;
      };
      tile_render(const create_info &info);
      ~tile_render();

      void begin() override;
      void proccess(container* ctx) override;
      void clear() override;

//       void recreate_pipelines(const game::image_resources_t* resource) override;
//       void change_rendering_mode(const uint32_t &render_mode, const uint32_t &water_mode, const uint32_t &render_slot, const uint32_t &water_slot, const glm::vec3 &color);
      
      vk::Buffer vertex_indices() const;
    private:
      vk::Device device;
      vma::Allocator allocator;
      tile_optimizer* opt;
      vk::PipelineLayout p_layout;
      vk::Pipeline pipe;
      vk_buffer_data points_indices;
      vk::DescriptorSet images_set;
    };
    
    class tile_border_render : public stage {
    public:
      struct create_info {
        container* cont;
        vk::DescriptorSetLayout tiles_data_layout;
        vk::DescriptorSetLayout images_layout;
        tile_optimizer* opt;
        world_map_buffers* map_buffers;
      };
      tile_border_render(const create_info &info);
      ~tile_border_render();

      void begin() override;
      void proccess(container* ctx) override;
      void clear() override;

//       void recreate_pipelines(const game::image_resources_t* resource) override;
//       void add(const uint32_t &tile_index); // кажется только индекс тайла нам нужен
    private:
      vk::Device device;
      vma::Allocator allocator;
      tile_optimizer* opt;
      vk::PipelineLayout p_layout;
      vk::Pipeline pipe;
      vk::DescriptorSet images_set;
      world_map_buffers* map_buffers;
    };
    
//     class tile_connections_render : public stage {
//     public:
//       struct create_info {
//         container* cont;
//         vk::DescriptorSetLayout tiles_data_layout;
//         vk::DescriptorSetLayout images_layout;
//         tile_optimizer* opt;
//         world_map_buffers* map_buffers;
//       };
//       tile_connections_render(const create_info &info);
//       ~tile_connections_render();
// 
//       void begin() override;
//       void proccess(container* ctx) override;
//       void clear() override;
// 
// //       void recreate_pipelines(const game::image_resources_t* resource) override;
//       void change_rendering_mode(const uint32_t &render_mode, const uint32_t &water_mode, const uint32_t &render_slot, const uint32_t &water_slot, const glm::vec3 &color);
//     private:
//       vk::Device device;
//       vma::Allocator allocator;
//       tile_optimizer* opt;
//       vk::PipelineLayout p_layout;
//       vk::Pipeline pipe;
//       world_map_buffers* map_buffers;
//       vk::DescriptorSet images_set;
//     };
    
    class tile_object_render : public stage {
    public:
      struct create_info {
        container* cont;
        vk::DescriptorSetLayout tiles_data_layout;
        vk::DescriptorSetLayout images_layout;
        tile_optimizer* opt;
        world_map_buffers* map_buffers;
      };
      tile_object_render(const create_info &info);
      ~tile_object_render();

      void begin() override;
      void proccess(container* ctx) override;
      void clear() override;

//       void recreate_pipelines(const game::image_resources_t* resource) override;
//       void change_rendering_mode(const uint32_t &render_mode, const uint32_t &water_mode, const uint32_t &render_slot, const uint32_t &water_slot, const glm::vec3 &color);
    private:
      vk::Device device;
      vma::Allocator allocator;
      tile_optimizer* opt;
      vk::PipelineLayout p_layout;
      vk::Pipeline pipe;
      world_map_buffers* map_buffers;
      vk::DescriptorSet images_set;
      
      const bool multi_draw_indirect;
    };
    
    // новый класс для выделения?
    // как добавлять тайлы? просто закидывать их в буфер? почему бы и нет
    // как их рисовать? прозрачный слой над обычными тайлами? наверное можно будет придумать что то еще
    // но в остальных стратегиях примерно так и было
    
    class tile_highlights_render : public stage {
    public:
      struct create_info {
        container* cont;
        vk::DescriptorSetLayout tiles_data_layout;
        world_map_buffers* map_buffers;
      };
      tile_highlights_render(const create_info &info);
      ~tile_highlights_render();
      
      void begin() override;
      void proccess(container* ctx) override;
      void clear() override;
      
      void add(const uint32_t &tile_index, const color_t &color);
    private:
      vk::Device device;
      vma::Allocator allocator;
      vk::PipelineLayout p_layout;
      vk::Pipeline pipe;
      world_map_buffers* map_buffers;
      // кажется текстуры здесь не нужны, нужно ли здесь время, чтобы сделать мигание выделения? 
      // какой максимум? 
      vk_buffer_data tiles_indices;
      std::atomic<uint32_t> hex_tiles_count;
      std::atomic<uint32_t> pen_tiles_count;
    };
    
    class tile_structure_render : public stage {
    public:
      struct create_info {
        container* cont;
        vk::DescriptorSetLayout tiles_data_layout;
        vk::DescriptorSetLayout images_layout;
        tile_optimizer* opt;
        world_map_buffers* map_buffers;
      };
      
      tile_structure_render(const create_info &info);
      ~tile_structure_render();
      
      void begin() override;
      void proccess(container* ctx) override;
      void clear() override;
    private:
      vk::Device device;
      vma::Allocator allocator;
      tile_optimizer* opt;
      world_map_buffers* map_buffers;
      vk::PipelineLayout p_layout;
      vk::Pipeline pipe;
      vk::DescriptorSet images_set;
    };
    
    class heraldies_render : public stage {
    public:
      struct create_info {
        container* cont;
        vk::DescriptorSetLayout tiles_data_layout;
        vk::DescriptorSetLayout images_layout;
        tile_optimizer* opt;
        world_map_buffers* map_buffers;
      };
      
      heraldies_render(const create_info &info);
      ~heraldies_render();
      
      void begin() override;
      void proccess(container* ctx) override;
      void clear() override;
    private:
      vk::Device device;
      vma::Allocator allocator;
      tile_optimizer* opt;
      world_map_buffers* map_buffers;
      vk::PipelineLayout p_layout;
      vk::Pipeline pipe;
      vk::DescriptorSet images_set;
    };
    
    class armies_render : public stage {
    public:
      struct create_info {
        container* cont;
        vk::DescriptorSetLayout tiles_data_layout;
        vk::DescriptorSetLayout images_layout;
        tile_optimizer* opt;
        world_map_buffers* map_buffers;
      };
      
      armies_render(const create_info &info);
      ~armies_render();
      
      void begin() override;
      void proccess(container* ctx) override;
      void clear() override;
    private:
      vk::Device device;
      vma::Allocator allocator;
      tile_optimizer* opt;
      world_map_buffers* map_buffers;
      vk::PipelineLayout p_layout;
      vk::Pipeline pipe;
      vk::DescriptorSet images_set;
    };
    
    class world_map_render : public stage_container { // public pipeline_stage, 
    public:
      struct create_info {
        size_t container_size;
      };
      world_map_render(const create_info &info);
      
      void begin() override;
      void proccess(struct container* ctx) override;
      void clear() override;
      
//       void recreate_pipelines(const game::image_resources_t* resource) override;
    };
    
    class interface_stage : public stage { // , public pipeline_stage
    public:
      struct interface_draw_command {
        unsigned int elem_count;
        rect2df clip_rect;
        void* texture;  // по размеру должен совпадать с nk_handle
        void* userdata; // по размеру должен совпадать с nk_handle
      };
      
      struct create_info {
        container* cont;
        vk::DescriptorSetLayout images_layout;
      };
      interface_stage(const create_info &info);
      ~interface_stage();
      
      void begin() override;
      void proccess(container* ctx) override;
      void clear() override;

//       void recreate_pipelines(const game::image_resources_t* resource) override;
    private:
      vk::Device device;
      vma::Allocator allocator;
      vk::PipelineLayout p_layout;
      vk::Pipeline pipe;
      vk_buffer_data vertex_gui;
      vk_buffer_data index_gui;
      vk_buffer_data matrix;
      vk::DescriptorSetLayout sampled_image_layout;
      vk::DescriptorSet matrix_set;
      vk::DescriptorSet images_set;
      
      std::vector<interface_draw_command> commands;
    };

    class task_start : public stage {
    public:
      task_start(vk::Device device);
      void begin() override;
      void proccess(container* ctx) override;
      void clear() override;
      void wait();
      bool status() const; // быстро ли будет работать?
    private:
      vk::Device device;
      vk::Fence rendering_fence;
    };

    class window_present : public stage {
    public:
      window_present();
      void begin() override;
      void proccess(container* ctx) override;
      void clear() override;
    };
  }
}

#endif
