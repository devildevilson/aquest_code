#ifndef STAGES_H
#define STAGES_H

#include "stage.h"
#include "yavf.h"
#include "utils/utility.h"
#include "bin/figures.h"
#include "bin/map.h"
#include "render.h"
// #include "targets.h"
//#include "shared_structures.h"
#include "shared_render_utility.h"
#include <atomic>
#include <mutex>
#include <unordered_set>

namespace devils_engine {
  namespace render {
    class deffered;
    struct images;
    struct window;
    struct particles;
    struct world_map_buffers;

    class window_next_frame : public stage {
    public:
      struct create_info {
        window* w;
      };
      window_next_frame(const create_info &info);
      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;
    private:
      window* w;
    };

    class task_begin : public stage {
    public:
      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;
    };

    class task_end : public stage {
    public:
      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;
    };

    class render_pass_begin : public stage {
    public:
      render_pass_begin(const uint32_t &index);
      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;
    private:
      uint32_t index;
    };

    class render_pass_end : public stage {
    public:
      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;
    };
    
    class tile_optimizer : public stage {
    public:
      static const uint32_t work_group_size = 256;
      
      struct indirect_buffer {
//         VkDrawIndirectCommand pentagon_command;
//         VkDrawIndirectCommand hexagon_command;
        VkDrawIndexedIndirectCommand pen_tiles_command;
        uint32_t padding1[3];
        VkDrawIndexedIndirectCommand hex_tiles_command;
        uint32_t padding_hex[3];
        VkDrawIndexedIndirectCommand borders_command;
        uint32_t padding2[3];
        VkDrawIndexedIndirectCommand walls_command;
        uint32_t padding3[3];
        VkDrawIndexedIndirectCommand structures_command;
        uint32_t padding4[3];
        VkDrawIndexedIndirectCommand heraldies_command;
        uint32_t padding5[3];
        glm::uvec4 dispatch_indirect_command;
        
        utils::frustum frustum;
        aabb_t selection_box;
        utils::frustum selection_frustum;
        
        biome_objects_data_t biome_data[MAX_BIOMES_COUNT];
      };
      
      static_assert(sizeof(indirect_buffer) % (sizeof(uint32_t)*4) == 0);
      
      struct create_info {
        yavf::Device* device;
      };
      tile_optimizer(const create_info &info);
      ~tile_optimizer();
      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;
      
      yavf::Buffer* indirect_buffer() const;
      yavf::Buffer* tiles_index_buffer() const;
      yavf::Buffer* borders_index_buffer() const;
      yavf::Buffer* walls_index_buffer() const;
      yavf::Buffer* objects_index_buffer() const;
      yavf::Buffer* structures_index_buffer() const;
      yavf::Buffer* heraldy_index_buffer() const;
      yavf::DescriptorSet* buffers_set() const;
      
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
    private:
      yavf::Device* device;
      yavf::Buffer* indirect;
      yavf::Buffer* tiles_indices;
      yavf::Buffer* borders_indices;
      yavf::Buffer* walls_indices;
      yavf::Buffer* objects_indices;
      yavf::Buffer* structures_indices;
      yavf::Buffer* heraldy_indices;
      yavf::DescriptorSet* set;
      yavf::Pipeline pipe;
    };
    
    class tile_objects_optimizer : public stage {
    public:
      struct create_info {
        yavf::Device* device;
        tile_optimizer* opt;
      };
      tile_objects_optimizer(const create_info &info);
      ~tile_objects_optimizer();
      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;      
      
    private:
      yavf::Device* device;
      tile_optimizer* opt;
      yavf::Pipeline pipe;
    };
    
    class tile_borders_optimizer : public stage {
    public:
      static const uint32_t work_group_size = 256;
      
      struct indirect_buffer {
        VkDrawIndirectCommand border_command;
        glm::uvec4 data;
        utils::frustum frustum;
      };
      
      static_assert(sizeof(indirect_buffer) % (sizeof(uint32_t)*4) == 0);
      
      struct create_info {
        yavf::Device* device;
        world_map_buffers* map_buffers;
      };
      tile_borders_optimizer(const create_info &info);
      ~tile_borders_optimizer();
      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;
      
      yavf::Buffer* indirect_buffer() const;
      yavf::Buffer* vertices_buffer() const;
      
      void set_borders_count(const uint32_t &count);
    private:
      yavf::Device* device;
      yavf::Buffer* indirect;
      yavf::Buffer* borders_indices;
      yavf::DescriptorSet* set;
      yavf::Pipeline pipe;
      world_map_buffers* map_buffers;
    };
    
    class tile_walls_optimizer : public stage {
    public:
      static const uint32_t work_group_size = 256;
      
      struct indirect_buffer {
        VkDrawIndirectCommand walls_command;
        glm::uvec4 data;
        utils::frustum frustum;
      };
      
      static_assert(sizeof(indirect_buffer) % (sizeof(uint32_t)*4) == 0);
      
      struct create_info {
        yavf::Device* device;
        world_map_buffers* map_buffers;
      };
      tile_walls_optimizer(const create_info &info);
      ~tile_walls_optimizer();
      
      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;
      
      yavf::Buffer* indirect_buffer() const;
      yavf::Buffer* vertices_buffer() const;
      
      void set_connections_count(const uint32_t &count);
    private:
      yavf::Device* device;
      yavf::Buffer* indirect;
      yavf::Buffer* walls_indices;
      yavf::DescriptorSet* set;
      yavf::Pipeline pipe;
      world_map_buffers* map_buffers;
    };
    
    class barriers : public stage {
    public:
      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;
    };

    class tile_render : public stage, public pipeline_stage {
    public:
      struct create_info {
        yavf::Device* device;
        tile_optimizer* opt;
      };
      tile_render(const create_info &info);
      ~tile_render();

      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;

      void recreate_pipelines(const game::image_resources_t* resource) override;
      void change_rendering_mode(const uint32_t &render_mode, const uint32_t &water_mode, const uint32_t &render_slot, const uint32_t &water_slot, const glm::vec3 &color);
      void add(const uint32_t &tile_index); // кажется только индекс тайла нам нужен
      void picked_tile(const uint32_t &tile_index);
      
      yavf::Buffer* vertex_indices() const;
    private:
      yavf::Device* device;
      tile_optimizer* opt;
      yavf::Pipeline pipe;
      yavf::Pipeline one_tile_pipe;
//       yavf::Buffer* indices;
      yavf::Buffer* points_indices;
      yavf::DescriptorSet* images_set;
      uint32_t picked_tile_index;

      void create_render_pass();
    };
    
    class tile_border_render : public stage, public pipeline_stage {
    public:
      struct create_info {
        yavf::Device* device;
//         tile_borders_optimizer* opt;
        tile_optimizer* opt;
        world_map_buffers* map_buffers;
      };
      tile_border_render(const create_info &info);
      ~tile_border_render();

      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;

      void recreate_pipelines(const game::image_resources_t* resource) override;
//       void add(const uint32_t &tile_index); // кажется только индекс тайла нам нужен
    private:
      yavf::Device* device;
//       tile_borders_optimizer* opt;
      tile_optimizer* opt;
//       tile_render* render;
      yavf::Pipeline pipe;
      world_map_buffers* map_buffers;
    };
    
    class tile_connections_render : public stage, public pipeline_stage {
    public:
      struct create_info {
        yavf::Device* device;
//         tile_walls_optimizer* opt;
        tile_optimizer* opt;
        world_map_buffers* map_buffers;
      };
      tile_connections_render(const create_info &info);
      ~tile_connections_render();

      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;

      void recreate_pipelines(const game::image_resources_t* resource) override;
      void change_rendering_mode(const uint32_t &render_mode, const uint32_t &water_mode, const uint32_t &render_slot, const uint32_t &water_slot, const glm::vec3 &color);
    private:
      yavf::Device* device;
//       tile_walls_optimizer* opt;
      tile_optimizer* opt;
      yavf::Pipeline pipe;
      world_map_buffers* map_buffers;
      yavf::DescriptorSet* images_set;
    };
    
    class tile_object_render : public stage {
    public:
      struct create_info {
        yavf::Device* device;
        tile_optimizer* opt;
        world_map_buffers* map_buffers;
      };
      tile_object_render(const create_info &info);
      ~tile_object_render();

      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;

//       void recreate_pipelines(const game::image_resources_t* resource) override;
//       void change_rendering_mode(const uint32_t &render_mode, const uint32_t &water_mode, const uint32_t &render_slot, const uint32_t &water_slot, const glm::vec3 &color);
    private:
      yavf::Device* device;
      tile_optimizer* opt;
      yavf::Pipeline pipe;
      world_map_buffers* map_buffers;
      yavf::DescriptorSet* images_set;
    };
    
    // новый класс для выделения?
    // как добавлять тайлы? просто закидывать их в буфер? почему бы и нет
    // как их рисовать? прозрачный слой над обычными тайлами? наверное можно будет придумать что то еще
    // но в остальных стратегиях примерно так и было
    
    class tile_highlights_render : public stage {
    public:
      struct create_info {
        yavf::Device* device;
//         tile_optimizer* opt;
        world_map_buffers* map_buffers;
      };
      tile_highlights_render(const create_info &info);
      ~tile_highlights_render();
      
      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;
      
      void add(const uint32_t &tile_index, const color_t &color);
    private:
      yavf::Device* device;
//       tile_optimizer* opt;
      yavf::Pipeline pipe;
      world_map_buffers* map_buffers;
      // кажется текстуры здесь не нужны, нужно ли здесь время, чтобы сделать мигание выделения? 
      // какой максимум? 
      yavf::Buffer* tiles_indices;
      std::atomic<uint32_t> hex_tiles_count;
      std::atomic<uint32_t> pen_tiles_count;
    };
    
    class tile_structure_render : public stage {
    public:
      struct create_info {
        yavf::Device* device;
        tile_optimizer* opt;
        world_map_buffers* map_buffers;
      };
      
      tile_structure_render(const create_info &info);
      ~tile_structure_render();
      
      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;
    private:
      yavf::Device* device;
      tile_optimizer* opt;
      world_map_buffers* map_buffers;
      yavf::Pipeline pipe;
      yavf::DescriptorSet* images_set;
    };
    
    class heraldies_render : public stage {
    public:
      struct create_info {
        yavf::Device* device;
        tile_optimizer* opt;
        world_map_buffers* map_buffers;
      };
      
      heraldies_render(const create_info &info);
      ~heraldies_render();
      
      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;
    private:
      yavf::Device* device;
      tile_optimizer* opt;
      world_map_buffers* map_buffers;
      yavf::Pipeline pipe;
      yavf::DescriptorSet* images_set;
    };
    
    class armies_render : public stage {
    public:
      struct create_info {
        yavf::Device* device;
        tile_optimizer* opt;
        world_map_buffers* map_buffers;
      };
      
      armies_render(const create_info &info);
      ~armies_render();
      
      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;
    private:
      yavf::Device* device;
      tile_optimizer* opt;
      world_map_buffers* map_buffers;
      yavf::Pipeline pipe;
      yavf::DescriptorSet* images_set;
    };
    
    class world_map_render : public pipeline_stage, public stage_container {
    public:
      struct create_info {
//         yavf::Device* device;
        size_t container_size;
      };
      world_map_render(const create_info &info);
      
      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;
      
      void recreate_pipelines(const game::image_resources_t* resource) override;
    };
    
    class interface_stage : public stage, public pipeline_stage {
    public:
      struct create_info {
        yavf::Device* device;
      };
      interface_stage(const create_info &info);
      ~interface_stage();
      
      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;

      void recreate_pipelines(const game::image_resources_t* resource) override;
    private:
      yavf::Device* device;
      yavf::Pipeline pipe;
      yavf::Buffer vertex_gui;
      yavf::Buffer index_gui;
      yavf::Buffer matrix;
      yavf::DescriptorSet* images_set;
    };

    class task_start : public stage {
    public:
      task_start(yavf::Device* device);
      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;
      void wait();
      bool status() const; // быстро ли будет работать?
    private:
      yavf::Device* device;
      yavf::Internal::Queue wait_fence;
    };

    class window_present : public stage {
    public:
      struct create_info {
        window* w;
      };
      window_present(const create_info &info);
      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;
    private:
      window* w;
    };
  }
}

#endif
