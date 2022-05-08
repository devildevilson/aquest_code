#ifndef DEVILS_ENGINE_RENDER_STAGES_H
#define DEVILS_ENGINE_RENDER_STAGES_H

#include <atomic>
#include <mutex>
#include <unordered_set>

#include "utils/utility.h"
#include "utils/frustum.h"

//#include "stage.h"
#include "interfaces.h"
#include "render.h"
#include "shared_structures.h"
#include "shared_render_utility.h"
#include "heraldy.h"
#include "vulkan_hpp_header.h"

namespace devils_engine {
  namespace core {
    struct map;
    class context;
  }
  
  namespace render {
    class deffered;
    class pass;
    struct images;
    struct window;
    struct particles;
    struct world_map_buffers;
    struct container;
    
    void do_copy_tasks(vk::CommandBuffer task, vk::Event event, const size_t &size, const copy_stage* const* stages);
    void set_event_cmd(vk::CommandBuffer task, vk::Event event);
    
    template <size_t N>
    class static_copy_array final : public stage, public copy_stage {
    public:
      static_copy_array(vk::Device device) : device(device), size(0) { 
        memset(stages.data(), 0, stages.size() * sizeof(stages[0])); 
        event = device.createEvent(vk::EventCreateInfo());
        device.setEvent(event);
      }
      ~static_copy_array() { device.destroy(event); }
      void begin(resource_provider*) override { device.resetEvent(event); }
      bool process(resource_provider* ctx, vk::CommandBuffer task) override { 
        copy(ctx, task);
        set_event_cmd(task, event);
        return true;
      }
      void clear() override {}
      void copy(resource_provider* ctx, vk::CommandBuffer task) const override { for (size_t i = 0; i < size; ++i) { stages[i]->copy(ctx, task); } }
      
      void add(const copy_stage* stage) {
        if (size >= N) throw std::runtime_error("Small copy array size " + std::to_string(N));
        stages[size] = stage;
        ++size;
      }
      
      bool end() const {
        const auto res = device.getEventStatus(event);
        return res == vk::Result::eEventSet;
      }
    private:
      vk::Device device;
      vk::Event event;
      size_t size;
      std::array<const copy_stage*, N> stages;
    };
    
    class interface_stage final : public stage { // , public pipeline_stage
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
        class pass* pass;
        uint32_t subpass_index;
      };
      interface_stage(const create_info &info);
      ~interface_stage();
      
      void begin(resource_provider* ctx) override;
      bool process(resource_provider* ctx, vk::CommandBuffer task) override;
      void clear() override;
      
      uint32_t add_layers(const size_t &count, const render::heraldy_layer_t* layers);
    private:
      vk::Device device;
      vma::Allocator allocator;
      vk::PipelineLayout p_layout;
      vk::Pipeline pipe;
      vk_buffer_data vertex_gui;
      vk_buffer_data index_gui;
      vk_buffer_data matrix;
      // изображение по слоям тут? нужно ли оно где то еще?
      vk_buffer_data image_layers_buffer;
      vk::DescriptorSetLayout sampled_image_layout;
      vk::DescriptorSet matrix_set;
      vk::DescriptorSet images_set;
      vk::DescriptorSet layers_set;
      
      uint32_t maximum_layers;
      std::atomic<uint32_t> layers_counter;
      std::vector<interface_draw_command> commands;
    };
    
    class skybox final : public stage {
    public:
      skybox(container* cont, class pass* pass, const uint32_t &subpass_index);
      ~skybox();
      void begin(resource_provider* ctx) override;
      bool process(resource_provider* ctx, vk::CommandBuffer task) override;
      void clear() override;
    private:
      vk::Device device;
      vk::PipelineLayout p_layout;
      vk::Pipeline pipe;
    };
    
    class secondary_buffer_subpass final : public stage {
    public:
      void begin(resource_provider* ctx) override;
      bool process(resource_provider* ctx, vk::CommandBuffer task) override;
      void clear() override;
    };
  }
}

#endif
