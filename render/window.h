#ifndef WINDOW_H
#define WINDOW_H

#include <cstdint>
#include <vector>
#include <tuple>
#include "target.h"
#include "vulkan_declarations.h"

typedef struct GLFWmonitor GLFWmonitor;
typedef struct GLFWwindow GLFWwindow;

// namespace yavf {
//   class Instance;
//   class Device;
// }

#define FAR_CLIPPING 256.0f

namespace devils_engine {
  namespace render {
    struct vulkan_window;
    
    struct window {
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
      
      struct create_info {
        target* recreation_target;
        bool fullscreen;
        uint32_t width;
        uint32_t height;
        float fov;
        uint32_t video_mode;
      };
      
      // вообще нам не нужен вулкан в окне
      // окно можно и нужно создать до вулкана
      // а весь вулкан вынести в контейнер
      GLFWmonitor* monitor;
      GLFWwindow* handle;
      extent2d extent;
      struct flags flags;
      float fov;
      target* recreation_target;
      
      window(const create_info &info);
      ~window();
      
      VkSurfaceKHR create_surface(VkInstance instance) const;
      void resize();
      uint32_t refresh_rate() const;
      uint32_t refresh_rate_mcs() const;
      
      void screenshot(vk::Image* container) const;
      void show() const;
      void hide() const;
      bool close() const;
      void toggle_fullscreen();
      std::tuple<int32_t, int32_t> framebuffer_size() const;
      std::tuple<int32_t, int32_t> size() const;
      std::tuple<float, float> content_scale() const;
      std::tuple<int32_t, int32_t> monitor_physical_size() const;
      std::tuple<float, float> monitor_content_scale() const;
      std::tuple<double, double> get_cursor_pos() const;
    };
  }
}

#endif
