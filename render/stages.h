#ifndef STAGES_H
#define STAGES_H

#include "stage.h"
#include "yavf.h"
#include "utils/utility.h"
#include "bin/figures.h"
#include "bin/map.h"
#include <atomic>
#include <mutex>
#include <unordered_set>

namespace devils_engine {
  namespace render {
    class deffered;
    struct images;
    struct window;
    struct particles;

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
      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;
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
        VkDrawIndirectCommand pentagon_command;
        VkDrawIndirectCommand hexagon_command;
        glm::uvec4 data;
        utils::frustum frustum;
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
      yavf::Buffer* instances_buffer() const;
    private:
      yavf::Device* device;
      yavf::Buffer* indirect;
      yavf::Buffer* tiles_indices;
      yavf::Pipeline pipe;
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
      void add(const uint32_t &tile_index); // кажется только индекс тайла нам нужен
      
      yavf::Buffer* vertex_indices() const;
    private:
      yavf::Device* device;
      tile_optimizer* opt;
      yavf::Pipeline pipe;
//       yavf::Buffer* indices;
      yavf::Buffer* points_indices;

      void create_render_pass();
    };
    
    class tile_border_render : public stage, public pipeline_stage {
    public:
      struct create_info {
        yavf::Device* device;
        tile_optimizer* opt;
        tile_render* render;
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
      tile_optimizer* opt;
      tile_render* render;
      yavf::Pipeline pipe;
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
    };

    class task_start : public stage {
    public:
      task_start(yavf::Device* device);
      void begin() override;
      void proccess(context* ctx) override;
      void clear() override;
      void wait();
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
