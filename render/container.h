#ifndef DEVILS_ENGINE_RENDER_CONTAINER_H
#define DEVILS_ENGINE_RENDER_CONTAINER_H

#include "utils/typeless_container.h"
#include "utils/bit_field.h"
// #include "context.h"
// #include "target.h"
// #include "interfaces.h"
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
    class command_buffer;
    class queue;
    class container_view;
    class stage;
    class pass;
    
    struct container final { //  : public target, public semaphore_provider, public context, 
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
      
      utils::typeless_container mem;
      struct window* window;
      vulkan_container* vulkan;
      vulkan_window* vlk_window;
      class queue* render_queue;  
      command_buffer* command_buffers;
      command_buffer* secondary_command_buffers;
      render::stage_container* render;
      container_view* view;
      utils::bit_field<physical_device_properties_count> device_properties;

      container();
      ~container();

      struct window* create_window(const window_info &info);
      vk::Instance* create_instance(const std::vector<const char*> &extensions, const application_info* app_info);
      vk::Device* create_device();
      void create_vlk_window();
      void create_swapchain();
//       void create_render_pass();
      render::stage_container* create_system(const size_t &system_container_size);
      void create_tasks();
      bool is_properties_presented(const uint32_t &index) const;
      
      void begin();
      void next_frame();
      void draw();
      void present();
      void wait();
      void clear();
      vk::Image* image() const;
      vk::Image* depth() const;
      uint32_t swapchain_frames_count() const;
      uint32_t swapchain_current_image() const;
      uint32_t swapchain_current_frame() const;
      void toggle_vsync();
      
      void set_command_buffer_childs(stage* childs);
      void set_secondary_command_buffer_childs(stage* childs);
      void set_secondary_command_buffer_renderpass(pass* renderpass);
      
      std::tuple<clear_value, clear_value> clear_values() const;
      rect2d size() const;
      struct viewport viewport() const;
      rect2d scissor() const;
//       vk::RenderPass* render_pass() const; // отсюда уйдет
//       vk::RenderPass* render_pass_objects() const;
      vk::Framebuffer* current_buffer() const;
      vk::Fence* current_frame_fence() const;
      vk::Semaphore* image_wait_semaphore() const;
      vk::Semaphore* finish_semaphore() const;
      vk::Queue* queue() const;
      uint32_t wait_pipeline_stage() const;
      vk::PhysicalDeviceLimits* limits() const;
      uint32_t get_surface_format() const;
      uint32_t get_depth_format() const;

//       yavf::TaskInterface* interface() const override;
//       yavf::CombinedTask* combined() const override;
//       yavf::ComputeTask* compute() const override;
//       yavf::GraphicTask* graphics() const override;
//       yavf::TransferTask* transfer() const override;
//       vk::CommandBuffer* command_buffer() const;
      void recreate(const uint32_t &width, const uint32_t &height);
      //size_t get_signaling(const size_t &max, vk::Semaphore* semaphores_arr, vk::PipelineStageFlags* flags_arr) const override;
      
      void print_memory_info() const;
    };
  }
}

#endif
