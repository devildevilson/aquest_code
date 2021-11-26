#include "window.h"

#include "utils/globals.h"
//#include "settings.h"
#include "targets.h"
#include "utils/utility.h"

// #include "vulkan_hpp_header.h"
// #include "makers.h"
#include <stdexcept>

// #define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#define REFRESH_RATE_TO_MCS(rate) (1000000.0f / float(rate))

enum FLAGS_CONSTS {
  FLAG_VSYNC                  = (1 << 0),
  FLAG_IMMEDIATE_PRESENT_MODE = (1 << 1),
  FLAG_ICONIFIED              = (1 << 2),
  FLAG_FOCUSED                = (1 << 3),
  FLAG_FULLSCREEN             = (1 << 4),
  FLAG_COUNT
};

namespace devils_engine {
  namespace render {
    window::flags::flags() : container(0) {}
    bool window::flags::vsync() const {
      return (container & FLAG_VSYNC) == FLAG_VSYNC;
    }

    bool window::flags::immediate_present_mode() const {
      return (container & FLAG_IMMEDIATE_PRESENT_MODE) == FLAG_IMMEDIATE_PRESENT_MODE;
    }

    bool window::flags::iconified() const {
      return (container & FLAG_ICONIFIED) == FLAG_ICONIFIED;
    }

    bool window::flags::focused() const {
      return (container & FLAG_FOCUSED) == FLAG_FOCUSED;
    }

    bool window::flags::fullscreen() const {
      return (container & FLAG_FULLSCREEN) == FLAG_FULLSCREEN;
    }

    void window::flags::set_vsync(const bool value) {
      container = value ? container | FLAG_VSYNC : container & (~FLAG_VSYNC);
    }

    void window::flags::immediate_present_mode(const bool value) {
      container = value ? container | FLAG_IMMEDIATE_PRESENT_MODE : container & (~FLAG_IMMEDIATE_PRESENT_MODE);
    }

    void window::flags::set_iconified(const bool value) {
      container = value ? container | FLAG_ICONIFIED : container & (~FLAG_ICONIFIED);
    }

    void window::flags::set_focused(const bool value) {
      container = value ? container | FLAG_FOCUSED : container & (~FLAG_FOCUSED);
    }

    void window::flags::set_fullscreen(const bool value) {
      container = value ? container | FLAG_FULLSCREEN : container & (~FLAG_FULLSCREEN);
    }

    //window::surface::surface() : handle(VK_NULL_HANDLE), presentMode(VK_PRESENT_MODE_MAX_ENUM_KHR), offset{0, 0}, extent{0, 0} {}

    window::window(const create_info &info) : monitor(nullptr), handle(nullptr), recreation_target(info.recreation_target) {
      // const bool fullscreen = Global::get<utils::settings>()->graphics.fullscreen;
      // uint32_t width = Global::get<utils::settings>()->graphics.width;
      // uint32_t height = Global::get<utils::settings>()->graphics.height;
      // const float fov = Global::get<utils::settings>()->graphics.fov;
      const bool fullscreen = info.fullscreen;
      uint32_t width = info.width;
      uint32_t height = info.height;
      const float fov = 75.0f;
      this->fov = fov;

    //   int32_t count;
    //   auto monitors = glfwGetMonitors(&count);
    //   for (int32_t i = 0; i < count; ++i) {
    //     std::cout << "Monitor name: " << glfwGetMonitorName(monitors[i]) << "\n";
    //     int32_t x, y;
    //     glfwGetMonitorPhysicalSize(monitors[i], &x, &y);
    //     std::cout << "Monitor phys size: x " << x << " y " << y << "\n";
    //     glfwGetMonitorPos(monitors[i], &x, &y);
    //     std::cout << "Monitor pos: x " << x << " y " << y << "\n";
    //   }

      if (fullscreen) {
        monitor = glfwGetPrimaryMonitor();

        const auto data = glfwGetVideoMode(monitor);
        width = data->width;
        height = data->height;

        // Global::get<utils::settings>()->graphics.width = data->width;
        // Global::get<utils::settings>()->graphics.height = data->height;
      }

      glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
      glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
      glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_FALSE);
      glfwWindowHint(GLFW_REFRESH_RATE, GLFW_DONT_CARE);
      glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
      glfwWindowHint(GLFW_FOCUSED, GLFW_FALSE);
      glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_TRUE);
      if (fullscreen) {
        handle = glfwCreateWindow(width, height, APP_NAME, monitor, nullptr);
      } else {
        handle = glfwCreateWindow(width, height, APP_NAME, nullptr, nullptr);
      }

      flags.set_fullscreen(fullscreen);
      flags.set_focused(false);
    }

    window::~window() {
      glfwDestroyWindow(handle);
    }

    VkSurfaceKHR window::create_surface(VkInstance instance) const {
      VkSurfaceKHR s = VK_NULL_HANDLE;
      const auto res = glfwCreateWindowSurface(instance, handle, nullptr, &s);
      if (res != VK_SUCCESS) throw std::runtime_error("Could not create window surface");
      return s;
    }
    
    void window::resize() {
      int w, h;
      glfwGetWindowSize(handle, &w, &h);
      recreation_target->recreate(w, h);
    }

    uint32_t window::refresh_rate() const {
      auto mon = monitor;
      if (mon == nullptr) {
        mon = glfwGetPrimaryMonitor();
      }

      auto mode = glfwGetVideoMode(mon);
      return mode->refreshRate;
    }

    uint32_t window::refresh_rate_mcs() const {
      // как определить частоту кадров в оконном режиме? по всей видимости никак
      const uint32_t rate = refresh_rate();
      return rate == 0 ? 0 : REFRESH_RATE_TO_MCS(rate);
    }

    void window::screenshot(vk::Image* container) const {
      (void)container;
      // пока непонятно что нужно делать толком, скорее всего нужно в стейджах создать специальный стейдж, который будет копировать текущую картинку
//       if (container->info().extent.width != surface.extent.width || container->info().extent.height != surface.extent.height) {
//         container->recreate({surface.extent.width, surface.extent.height, 1});
//       }
// 
//       static const VkImageSubresourceRange range{
//         VK_IMAGE_ASPECT_COLOR_BIT,
//         0, 1, 0, 1
//       };
// 
//       const VkImageBlit blit{
//         {
//           VK_IMAGE_ASPECT_COLOR_BIT,
//           0, 0, 1
//         },
//         {
//           {0, 0, 0},
//           {static_cast<int32_t>(surface.extent.width), static_cast<int32_t>(surface.extent.height), 1}
//         },
//         {
//           VK_IMAGE_ASPECT_COLOR_BIT,
//           0, 0, 1
//         },
//         {
//           {0, 0, 0},
//           {static_cast<int32_t>(surface.extent.width), static_cast<int32_t>(surface.extent.height), 1}
//         }
//       };
// 
//       const uint32_t index = swapchain.image_index == 0 ? swapchain.images.size()-1 : swapchain.image_index-1;
//       yavf::Image* img = swapchain.images[index];
// 
//       auto trans = device->allocateGraphicTask();
//       trans->begin();
//       trans->setBarrier(img, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, range, present_family, VK_QUEUE_FAMILY_IGNORED);
//       trans->setBarrier(container, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
//       trans->copyBlit(img, container, blit);
//       trans->setBarrier(img, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, range, VK_QUEUE_FAMILY_IGNORED, present_family);
//       trans->setBarrier(container, VK_IMAGE_LAYOUT_GENERAL);
//       trans->end();
//       device->deallocate(trans);
    }

    void window::show() const {
      glfwShowWindow(handle);
    }

    void window::hide() const {
      glfwHideWindow(handle);
    }

    bool window::close() const {
      return glfwWindowShouldClose(handle);
    }
    
    void window::toggle_fullscreen() {
      flags.set_fullscreen(!flags.fullscreen());
      if (flags.fullscreen()) {
        ASSERT(monitor == nullptr);
        monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(handle, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
      } else {
        ASSERT(monitor != nullptr);
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        monitor = nullptr;
        glfwSetWindowMonitor(handle, nullptr, 0, 0, mode->width, mode->height, mode->refreshRate);
      }
    }
    
    std::tuple<int32_t, int32_t> window::framebuffer_size() const {
      int32_t w, h;
      glfwGetFramebufferSize(handle, &w, &h);
      return std::make_tuple(w, h);
    }
    
    // это возвращает не то что я хочу, должен быть способ вернуть именно пиксели рендерящейся области
    std::tuple<int32_t, int32_t> window::size() const {
      int32_t w, h;
      glfwGetWindowSize(handle, &w, &h);
      return std::make_tuple(w, h);
    }
    
    std::tuple<float, float> window::content_scale() const {
      float xscale, yscale;
      glfwGetWindowContentScale(handle, &xscale, &yscale);
      return std::make_tuple(xscale, yscale);
    }
    
    std::tuple<int32_t, int32_t> window::monitor_physical_size() const {
      int32_t w = 0, h = 0;
      if (monitor != nullptr) glfwGetMonitorPhysicalSize(monitor, &w, &h);
      return std::make_tuple(w, h);
    }
    
    std::tuple<float, float> window::monitor_content_scale() const {
      float xscale = 0.0f, yscale = 0.0f;
      if (monitor != nullptr) glfwGetMonitorContentScale(monitor, &xscale, &yscale);
      return std::make_tuple(xscale, yscale);
    }
    
    std::tuple<double, double> window::get_cursor_pos() const {
      double x,y;
      glfwGetCursorPos(handle, &x, &y);
      return std::tie(x, y);
    }
  }
}
