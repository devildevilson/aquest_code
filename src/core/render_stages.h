#ifndef DEVILS_ENGINE_CORE_RENDER_STAGES_H
#define DEVILS_ENGINE_CORE_RENDER_STAGES_H

#include <atomic>
#include "shared_structures.h"
#include "render/shared_render_utility.h"
#include "render/interfaces.h"
#include "render/targets.h"
#include "utils/frustum.h"

namespace devils_engine {
  namespace core {
    struct map;
    class context;
  }
  
  namespace render {
    // тут следовало бы разделить обновляемые данные и необновляемые данные
    // и тогда update сведется просто к копированию буфера
    class tile_updater final : public stage {
    public:
      struct create_info {
        vk::Device device;
        vma::Allocator allocator;
        core::map* map;
      };
      tile_updater(const create_info &info);
      ~tile_updater();
      void begin(resource_provider* ctx) override;
      bool process(resource_provider* ctx, vk::CommandBuffer task) override;
      void clear() override;
      
      void update_texture(const uint32_t tile_index, const render::image_t texture);
      void update_color(const uint32_t tile_index, const render::color_t color);
      void update_borders_data(const uint32_t tile_index, const uint32_t borders_data);
      void update_biome_index(const uint32_t tile_index, const uint32_t biome_index);
      void update_all(const core::context* ctx);
      
      void setup_default(container* ctx);
    private:
      struct update_data_t {
        render::image_t texture;
        render::color_t color;
        uint32_t borders_data;
        uint32_t biome_index;
      };
      
      std::atomic_bool need_copy;
      vk::Device device;
      vma::Allocator allocator;
      vk::Event ev;
      vk_buffer_data updates;
      vk::BufferCopy* copies;
      core::map* map;
    };
    
    class tile_optimizer final : public stage {
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
        vk::DescriptorSetLayout tiles_rendering_data_layout;
      };
      tile_optimizer(const create_info &info);
      ~tile_optimizer();
      void begin(resource_provider* ctx) override;
      bool process(resource_provider* ctx, vk::CommandBuffer task) override;
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
      vk::Pipeline tile_pipe;
      vk::Pipeline counter_pipe;
      
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
    
    class tile_objects_optimizer final : public stage {
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
      void begin(resource_provider* ctx) override;
      bool process(resource_provider* ctx, vk::CommandBuffer task) override;
      void clear() override;
    private:
      vk::Device device;
      vma::Allocator allocator;
      tile_optimizer* opt;
      vk::PipelineLayout p_layout;
      vk::Pipeline pipe;
    };
    
    class barriers final : public stage {
    public:
      void begin(resource_provider* ctx) override;
      bool process(resource_provider* ctx, vk::CommandBuffer task) override;
      void clear() override;
    };

    class tile_render : public stage {
    public:
      struct create_info {
        container* cont;
        vk::DescriptorSetLayout tiles_data_layout;
        vk::DescriptorSetLayout images_layout;
        tile_optimizer* opt;
        class pass* pass;
        uint32_t subpass_index;
      };
      tile_render(const create_info &info);
      ~tile_render();

      void begin(resource_provider* ctx) override;
      bool process(resource_provider* ctx, vk::CommandBuffer task) override;
      void clear() override;
      
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
    
    class tile_border_render final : public stage {
    public:
      struct create_info {
        container* cont;
        vk::DescriptorSetLayout tiles_data_layout;
        vk::DescriptorSetLayout images_layout;
        tile_optimizer* opt;
        world_map_buffers* map_buffers;
        class pass* pass;
        uint32_t subpass_index;
      };
      tile_border_render(const create_info &info);
      ~tile_border_render();

      void begin(resource_provider* ctx) override;
      bool process(resource_provider* ctx, vk::CommandBuffer task) override;
      void clear() override;
    private:
      vk::Device device;
      vma::Allocator allocator;
      tile_optimizer* opt;
      vk::PipelineLayout p_layout;
      vk::Pipeline pipe;
      vk::DescriptorSet images_set;
      world_map_buffers* map_buffers;
    };
    
    class tile_object_render final : public stage {
    public:
      struct create_info {
        container* cont;
        vk::DescriptorSetLayout tiles_data_layout;
        vk::DescriptorSetLayout images_layout;
        tile_optimizer* opt;
        world_map_buffers* map_buffers;
        class pass* pass;
        uint32_t subpass_index;
      };
      tile_object_render(const create_info &info);
      ~tile_object_render();

      void begin(resource_provider* ctx) override;
      bool process(resource_provider* ctx, vk::CommandBuffer task) override;
      void clear() override;
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
    
    class tile_highlights_render final : public stage {
    public:
      struct create_info {
        container* cont;
        vk::DescriptorSetLayout tiles_data_layout;
        world_map_buffers* map_buffers;
        class pass* pass;
        uint32_t subpass_index;
      };
      tile_highlights_render(const create_info &info);
      ~tile_highlights_render();
      
      void begin(resource_provider* ctx) override;
      bool process(resource_provider* ctx, vk::CommandBuffer task) override;
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
    
    class tile_structure_render final : public stage, public copy_stage {
    public:
      struct structure_data {
        uint32_t tile_index;
        image_t img; // одно изображение? чет я сомневаюсь в том что имеет смысл делить вид сверху и нет
        float scale;
      };
      
      struct create_info {
        container* cont;
        vk::DescriptorSetLayout tiles_data_layout;
        vk::DescriptorSetLayout images_layout;
        tile_optimizer* opt;
        world_map_buffers* map_buffers;
        class pass* pass;
        uint32_t subpass_index;
      };
      
      tile_structure_render(const create_info &info);
      ~tile_structure_render();
      
      void begin(resource_provider* ctx) override;
      bool process(resource_provider* ctx, vk::CommandBuffer task) override;
      void clear() override;
      
      void copy(resource_provider* ctx, vk::CommandBuffer task) const override;
      
      void add(const structure_data &data);
    private:
      vk::Device device;
      vma::Allocator allocator;
      tile_optimizer* opt;
      world_map_buffers* map_buffers;
      vk::PipelineLayout p_layout;
      vk::Pipeline pipe;
      vk::DescriptorSet images_set;
      
      std::atomic<uint32_t> structures_count;
      uint32_t current_inst_size;
      uint32_t local_structures_count;
      vk_buffer_data structures_instance;
      vk_buffer_data gpu_structures_instance;
    };
    
    class heraldies_render final : public stage, public copy_stage {
    public:
      // или не делать фрейм? просто проскалировать?
      // в цк3 коронками оформлялись геральдики на карте
      // мы можем первый слой заменить на любой другой, но нужно сохранить цвет
      // нам нужно не просто заменить слой, а нужно нарисовать картинку до этого
      // короче говоря картинка + новый первый слой, как мы можем это заменить из конфига?
      struct heraldy_data {
        uint32_t tile_index;
        const uint32_t* array; // массив должен пойти в сторадж буфер
        size_t array_size;
        uint32_t shield_layer;
        image_t frame;
        float scale;
        float frame_scale;
      };
      
      // как рисовать? нужно нарисовать прост светлую область за геральдикой
      // можно силуэт по форме быстренько сделать в гимпе
      struct heraldy_highlight_data {
        uint32_t tile_index;
        float scale;
        uint32_t shield_layer;
      };
      
      struct heraldy_interface_data {
        const uint32_t* array;
        size_t array_size;
      };
      
      struct create_info {
        container* cont;
        vk::DescriptorSetLayout tiles_data_layout;
        vk::DescriptorSetLayout images_layout;
        tile_optimizer* opt;
        world_map_buffers* map_buffers;
        class pass* pass;
        uint32_t subpass_index;
      };
      
      heraldies_render(const create_info &info);
      ~heraldies_render();
      
      void begin(resource_provider* ctx) override;
      bool process(resource_provider* ctx, vk::CommandBuffer task) override;
      void clear() override;
      
      void copy(resource_provider* ctx, vk::CommandBuffer task) const override;
      
      // передаем последовательность слоев геральдики, в массиве только слои? размер мы можем взять из array_size
      size_t add(const heraldy_data &data); // вызывается пару тысяч раз за кадр, мультитрединг?
      size_t add(const heraldy_interface_data &data); // вызывается пару раз за кадр
      size_t add_highlight(const heraldy_highlight_data &data); // вызывается один раз за кадр
    private:
      vk::Device device;
      vma::Allocator allocator;
      tile_optimizer* opt;
      world_map_buffers* map_buffers;
      vk::PipelineLayout p_layout;
      vk::Pipeline pipe;
      vk::DescriptorSet images_set;
      
      std::atomic<uint32_t> chain_count;
      std::atomic<uint32_t> inst_count;
      uint32_t current_buffer_size;
      uint32_t current_inst_size;
      uint32_t local_inst_count;
      uint32_t local_chain_count;
      vk_buffer_data layers_chain;
      vk_buffer_data heraldy_instance;
      //vk_buffer_data gpu_layers_chain; // эти данные лежат в render::buffers
      vk_buffer_data gpu_heraldy_instance;
    };
    
    class armies_render final : public stage, public copy_stage {
    public:
      struct army_data {
        glm::vec3 pos;
        image_t img;
        float scale;
      };
      
      struct create_info {
        container* cont;
        vk::DescriptorSetLayout tiles_data_layout;
        vk::DescriptorSetLayout images_layout;
        tile_optimizer* opt;
        world_map_buffers* map_buffers;
        class pass* pass;
        uint32_t subpass_index;
      };
      
      armies_render(const create_info &info);
      ~armies_render();
      
      void begin(resource_provider* ctx) override;
      bool process(resource_provider* ctx, vk::CommandBuffer task) override;
      void clear() override;
      
      void copy(resource_provider* ctx, vk::CommandBuffer task) const override;
      
      void add(const army_data &data);
    private:
      vk::Device device;
      vma::Allocator allocator;
      tile_optimizer* opt;
      world_map_buffers* map_buffers;
      vk::PipelineLayout p_layout;
      vk::Pipeline pipe;
      vk::DescriptorSet images_set;
      
      std::atomic<uint32_t> armies_count;
      uint32_t local_armies_count;
      uint32_t current_inst_size;
      vk_buffer_data instance;
      vk_buffer_data gpu_instance;
    };
  }
}

#endif
