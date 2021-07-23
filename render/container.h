#ifndef CONTAINER_H
#define CONTAINER_H

#include "utils/typeless_container.h"
#include "utils/bit_field.h"
#include "context.h"
#include "target.h"
#include "utils/constexpr_funcs.h"
#include <vector>
#include <string>
#include "vulkan_declarations.h"

// namespace yavf {
//   class Instance;
//   class Device;
//   class TaskInterface;
//   class CombinedTask;
//   class ComputeTask;
//   class GraphicTask;
//   class TransferTask;
// }

namespace devils_engine {
  namespace render {
    class stage_container;
  }
  
  namespace render {
    struct vulkan_container;
    struct vulkan_window;
    
    struct container : public target { // public context,
      enum physical_device_properties {
        physical_device_sampler_anisotropy,
        physical_device_multidraw_indirect,
        physical_device_properties_count
      };
      
      struct application_info {
        std::string app_name;
        uint32_t app_version;
        std::string engine_name;
        uint32_t engine_version;
        uint32_t api_version;
      };
      
      struct window_info {
        uint32_t width;
        uint32_t height;
        uint32_t video_mode; // это относится к фуллскрину
        bool fullscreen;
      };
      
      static const size_t bit_field_size = ceil(double(physical_device_properties_count) / double(SIZE_WIDTH));
      
      utils::typeless_container mem;
      struct window* window;
      vulkan_container* vulkan;
      vulkan_window* vlk_window;
      render::stage_container* render;
      utils::bit_field<bit_field_size> device_properties;

      container();
      ~container();

      struct window* create_window(const window_info &info);
      vk::Instance* create_instance(const std::vector<const char*> &extensions, const application_info* app_info);
      vk::Device* create_device();
      void create_vlk_window();
      void create_swapchain();
      void create_render_pass();
      render::stage_container* create_system(const size_t &system_container_size);
      void create_tasks();
      bool is_properties_presented(const uint32_t &index) const;
      
      void next_frame();
      void present();
      vk::Image* image() const;
      vk::Image* depth() const;
      uint32_t swapchain_frames_count() const;
      uint32_t swapchain_current_image() const;
      uint32_t swapchain_current_frame() const;
      void toggle_vsync();
      
      std::tuple<clear_value, clear_value> clear_values() const;
      rect2d size() const;
      struct viewport viewport() const;
      rect2d scissor() const;
      vk::RenderPass* render_pass() const;
      vk::RenderPass* render_pass_objects() const;
      vk::Framebuffer* current_buffer() const;
      vk::Fence* current_frame_fence() const;
      vk::Semaphore* image_wait_semaphore() const;
      vk::Semaphore* finish_semaphore() const;
      vk::Queue* queue() const;
      uint32_t wait_pipeline_stage() const;

//       yavf::TaskInterface* interface() const override;
//       yavf::CombinedTask* combined() const override;
//       yavf::ComputeTask* compute() const override;
//       yavf::GraphicTask* graphics() const override;
//       yavf::TransferTask* transfer() const override;
      vk::CommandBuffer* command_buffer() const;
      void recreate(const uint32_t &width, const uint32_t &height) override;
      
      void print_memory_info() const;
    };
  }
}

#endif
