#ifndef BATTLE_RENDER_STAGES_H
#define BATTLE_RENDER_STAGES_H

#include "stage.h"
#include "yavf.h"
#include "utils/utility.h"
#include "utils/frustum.h"

namespace devils_engine {
  namespace render {
    namespace battle {
      class tile_optimizer : public stage {
      public:
        static const size_t work_group_size = 256;
        
        struct indirect_buffer_data {
          glm::uvec4 tiles_indirect;
          glm::uvec4 sizes_data;
          
          utils::frustum frustum;
        };
        
        struct create_info {
          yavf::Device* device;
          
        };
        tile_optimizer(const create_info &info);
        ~tile_optimizer();
        
        void begin() override;
        void proccess(context * ctx) override;
        void clear() override;
        
        yavf::Buffer* get_indirect_buffer() const;
        yavf::Buffer* get_tiles_indices() const;
      private:
        yavf::Device* device;
        yavf::Pipeline pipe;
        
        yavf::DescriptorSet* set;
        yavf::Buffer* indirect_buffer;
        yavf::Buffer* tiles_indices;
      };
      
      class tile_render : public stage {
      public:
        struct create_info {
          yavf::Device* device;
          tile_optimizer* opt;
        };
        
        tile_render(const create_info &info);
        ~tile_render();
        
        void begin() override;
        void proccess(context * ctx) override;
        void clear() override;
      private:
        yavf::Device* device;
        tile_optimizer* opt;
        yavf::Pipeline pipe;
        
        yavf::Buffer* points_indices;
        yavf::DescriptorSet* images_set;
      };
    }
  }
}

#endif
