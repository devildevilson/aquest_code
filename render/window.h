#ifndef WINDOW_H
#define WINDOW_H

#include <cstdint>
#include "yavf.h"
#include "target.h"

typedef struct GLFWmonitor GLFWmonitor;
typedef struct GLFWwindow GLFWwindow;

namespace yavf {
  class Instance;
  class Device;
}

#define FAR_CLIPPING 256.0f

namespace devils_engine {
  namespace render {
    struct window : public target, public yavf::RenderTarget {
      struct virtual_frame {
        yavf::Semaphore image_available;
        VkPipelineStageFlags flags;
        yavf::Semaphore finish_rendering;
      };
      
      struct flags {
        uint32_t container;
        
        flags();
        bool vsync() const;
        bool immediate_present_mode() const;
        bool iconified() const;
        bool focused() const;
        bool fullscreen() const;
        
        void set_vsync(const bool value);
        void immediate_present_mode(const bool value);
        void set_iconified(const bool value);
        void set_focused(const bool value);
        void set_fullscreen(const bool value);
      };
      
      struct swapchain {
        yavf::Swapchain handle;
        uint32_t image_index;
        std::vector<yavf::Image*> images;
        std::vector<yavf::Image*> depths;
        std::vector<yavf::Framebuffer> buffers;
        std::vector<yavf::DescriptorSet*> sets;
      };
      
      struct offset {
        uint32_t x;
        uint32_t y;
      };
      
      struct surface {
        VkSurfaceKHR handle;
        VkSurfaceCapabilitiesKHR capabilities;
        VkSurfaceFormatKHR format;
        VkPresentModeKHR presentMode;
        struct offset offset;
        VkExtent2D extent;
        
        surface();
      };
      
      struct create_info {
        yavf::Instance* inst;
        bool fullscreen;
        uint32_t width;
        uint32_t height;
        float fov;
        uint32_t video_mode;
      };
      
      yavf::Instance* inst;
      yavf::Device* device;
      GLFWmonitor* monitor;
      GLFWwindow* handle;
      struct surface surface;
      struct swapchain swapchain;
      struct flags flags;
      float fov;
      VkRenderPass render_pass;
      std::vector<virtual_frame> frames;
      uint32_t current_frame;
      uint32_t present_family;
      
      window(const create_info &info);
      ~window();
      
      void create_swapchain(yavf::Device* device);
      
      std::vector<VkClearValue> clearValues() const override;
      VkRect2D size() const override;
      yavf::RenderPass renderPass() const override;
      VkViewport viewport() const override;
      VkRect2D scissor() const override;
      yavf::Framebuffer framebuffer() const override;
      void recreate(const uint32_t &width, const uint32_t &height) override;
      
      void resize();
      void next_frame();
      void present();
      void present(const yavf::Internal::Queue &queue);
      uint32_t refresh_rate() const;
      uint32_t refresh_rate_mcs() const;
      void create_render_pass();
      yavf::Image* image() const;
      yavf::Image* depth() const;
      
      void screenshot(yavf::Image* container) const;
      void show() const;
      void hide() const;
      bool close() const;
      void toggle_vsync();
      void update_buffers() const;
    };
  }
}

#endif
