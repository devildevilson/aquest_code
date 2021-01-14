#ifndef BATTLE_RENDER_STAGES_H
#define BATTLE_RENDER_STAGES_H

#include "stage.h"
#include "yavf.h"
#include "utils/utility.h"
#include "utils/frustum.h"
#include "shared_structures.h"
#include "shared_battle_structures.h"
#include <array>

namespace devils_engine {
  namespace render {
    namespace battle {
      class tile_optimizer : public stage {
      public:
        static const size_t work_group_size = 256;
        
        // наверное в принципе все данные лучше хранить в гпу буфере, другое дело как обновлять? копировать перед началом... в будущем можно так сделать
        struct indirect_buffer_data {
//           struct indexed_indirect_command {
//             VkDrawIndexedIndirectCommand indirect;
//             glm::uvec3 data;
//           };
          
          VkDrawIndexedIndirectCommand tiles_indirect;
          glm::uvec3 sizes_data;
          
          utils::frustum frustum;
          
          // добавится еще фрустум выделения (выделить мы можем много отрядов за раз (только отрядов?)) (что делать с воротами?)
          // и луч от курсора (луч направляем в тайлы, нужно проверить луч со сферой + луч с плоскостью)
          utils::frustum selection_frustum;
          glm::vec4 ray_pos;
          glm::vec4 ray_dir;
          
          biome_objects_data_t biomes_indirect[BATTLE_BIOMES_MAX_COUNT];
        };
        
        static_assert(sizeof(indirect_buffer_data) % 16 == 0);
        static_assert(sizeof(biome_objects_data_t) % 16 == 0);
        
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
        yavf::Buffer* get_biomes_indices() const;
        
        void update_containers();
        void update_selection_frustum(const utils::frustum &fru);
        void update_biome_data(const std::array<std::pair<uint32_t, uint32_t>, BATTLE_BIOMES_MAX_COUNT> &data);
      private:
        yavf::Device* device;
        yavf::Pipeline pipe;
        
        yavf::DescriptorSet* set;
        yavf::Buffer* indirect_buffer;
        yavf::Buffer* tiles_indices;
        yavf::Buffer* biomes_indices;
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
      
      class biome_render : public stage {
      public:
        struct create_info {
          yavf::Device* device;
          tile_optimizer* opt;
        };
        
        biome_render(const create_info &info);
        ~biome_render();
        
        void begin() override;
        void proccess(context * ctx) override;
        void clear() override;
      private:
        yavf::Device* device;
        tile_optimizer* opt;
        yavf::Pipeline pipe;
        
        yavf::DescriptorSet* images_set;
        
        const bool multidraw;
      };
    }
  }
}

#endif
