#ifndef BATTLE_RENDER_STAGES_H
#define BATTLE_RENDER_STAGES_H

#include <array>
#include "stage.h"
#include "utils/utility.h"
#include "utils/frustum.h"
#include "shared_structures.h"
#include "shared_battle_structures.h"
#include "vulkan_hpp_header.h"
#include "utils/constexpr_funcs.h"

namespace devils_engine {
  namespace render {
    struct container;
    
    namespace battle {
      class tile_optimizer : public stage {
      public:
        static const size_t work_group_size = 256;
        static const size_t selection_slots = 512;
        static const size_t selection_buffer_size = align_to(selection_slots*2*sizeof(uint32_t), 16);
        
        // наверное в принципе все данные лучше хранить в гпу буфере, другое дело как обновлять? копировать перед началом... в будущем можно так сделать
        struct indirect_buffer_data {
          vk::DrawIndexedIndirectCommand tiles_indirect;
          glm::uvec3 sizes_data;
          
          vk::DrawIndirectCommand units_indirect;
          
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
          container* cont;
          vk::DescriptorSetLayout map_layout;
        };
        tile_optimizer(const create_info &info);
        ~tile_optimizer();
        
        void begin() override;
        void proccess(container* ctx) override;
        void clear() override;
        
        vk::Buffer get_indirect_buffer() const;
        vk::Buffer get_tiles_indices() const;
        vk::Buffer get_biomes_indices() const;
        vk::Buffer get_units_indices() const;
        vk::Buffer get_selection_indices() const;
        vk::DescriptorSetLayout get_buffer_layout() const;
        vk::DescriptorSet get_set() const;
        
        void update_containers();
        void update_unit_container();
        void update_selection_frustum(const utils::frustum &fru);
        void update_biome_data(const std::array<std::pair<uint32_t, uint32_t>, BATTLE_BIOMES_MAX_COUNT> &data);
        void update_mouse_dir_data(const glm::vec4 &pos, const glm::vec4 &dir);
        
        uint32_t get_selection_count() const;
        const uint32_t* get_selection_data() const;
      private:
        vk::Device device;
        vma::Allocator allocator;
        vk::PipelineLayout p_layout;
        vk::Pipeline pipe;
        
        vk::DescriptorPool pool;
        vk::DescriptorSetLayout buffer_layout;
        vk::DescriptorSet set;
        vk_buffer_data indirect_buffer;
        vk_buffer_data tiles_indices;
        vk_buffer_data biomes_indices;
        vk_buffer_data units_indices;
        vk_buffer_data selection_indices;
        
        size_t tiles_indices_size;
        size_t biomes_indices_size;
        size_t units_indices_size;
      };
      
      class tile_render : public stage {
      public:
        struct create_info {
          container* cont;
          vk::DescriptorSetLayout map_layout;
          tile_optimizer* opt;
        };
        
        tile_render(const create_info &info);
        ~tile_render();
        
        void begin() override;
        void proccess(container * ctx) override;
        void clear() override;
      private:
        vk::Device device;
        vma::Allocator allocator;
        tile_optimizer* opt;
        vk::PipelineLayout p_layout;
        vk::Pipeline pipe;
        
        vk_buffer_data points_indices;
        vk::DescriptorSet images_set;
      };
      
      class biome_render : public stage {
      public:
        struct create_info {
          container* cont;
          vk::DescriptorSetLayout map_layout;
          tile_optimizer* opt;
        };
        
        biome_render(const create_info &info);
        ~biome_render();
        
        void begin() override;
        void proccess(container * ctx) override;
        void clear() override;
      private:
        vk::Device device;
        vma::Allocator allocator;
        tile_optimizer* opt;
        vk::PipelineLayout p_layout;
        vk::Pipeline pipe;
        
        vk::DescriptorSet images_set;
        
        const bool multidraw;
      };
      
      class units_render : public stage {
      public:
        struct create_info {
          container* cont;
          vk::DescriptorSetLayout map_layout;
          tile_optimizer* opt;
        };
        
        units_render(const create_info &info);
        ~units_render();
        
        void begin() override;
        void proccess(container * ctx) override;
        void clear() override;
      private:
        vk::Device device;
        vma::Allocator allocator;
        tile_optimizer* opt;
        vk::PipelineLayout p_layout;
        vk::Pipeline pipe;
        
        vk::DescriptorSet images_set;
      };
    }
  }
}

#endif
