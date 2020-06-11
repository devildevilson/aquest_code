#include "window.h"

#include "utils/globals.h"
//#include "settings.h"
#include "targets.h"
#include "utils/utility.h"

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
    VkResult createSwapChain(const struct window::surface &surfaceData, yavf::Device* device, struct window::swapchain &swapchainData) {
      uint32_t imageCount = surfaceData.capabilities.minImageCount + 1;
      if (surfaceData.capabilities.maxImageCount > 0 && imageCount > surfaceData.capabilities.maxImageCount) {
        imageCount = surfaceData.capabilities.maxImageCount;
      }

      yavf::Swapchain old = swapchainData.handle;

//       if (old != VK_NULL_HANDLE) {
//         for (auto img : swapchainData.images) {
//           if (img->view() == nullptr) continue;
//           device->destroy(img->view());
//         }
//       }

      const VkSwapchainCreateInfoKHR info{
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        nullptr,
        0,
        surfaceData.handle,
        imageCount,
        surfaceData.format.format,
        surfaceData.format.colorSpace,
        surfaceData.extent,
        1,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr,
        surfaceData.capabilities.currentTransform,
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        surfaceData.presentMode,
        VK_TRUE,
        old
      };

      swapchainData.handle = device->recreate(info, "window_swapchain");
      swapchainData.images = device->getSwapchainImages(swapchainData.handle);

      return VK_SUCCESS;
    }

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

    window::surface::surface() : handle(VK_NULL_HANDLE), presentMode(VK_PRESENT_MODE_MAX_ENUM_KHR), offset{0, 0}, extent{0, 0} {}

    window::window(const create_info &info) : inst(info.inst), device(nullptr), monitor(nullptr), handle(nullptr) {
      // const bool fullscreen = Global::get<utils::settings>()->graphics.fullscreen;
      // uint32_t width = Global::get<utils::settings>()->graphics.width;
      // uint32_t height = Global::get<utils::settings>()->graphics.height;
      // const float fov = Global::get<utils::settings>()->graphics.fov;
      const bool fullscreen = false;
      uint32_t width = 1280;
      uint32_t height = 720;
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
      if (fullscreen) {
        handle = glfwCreateWindow(width, height, APP_NAME, monitor, nullptr);
      } else {
        handle = glfwCreateWindow(width, height, APP_NAME, nullptr, nullptr);
      }

      yavf::vkCheckError("glfwCreateWindowSurface", nullptr,
      glfwCreateWindowSurface(inst->handle(), handle, nullptr, &surface.handle));

      flags.set_fullscreen(fullscreen);
    }

    window::~window() {
      for (size_t i = 0; i < frames.size(); ++i) {
        device->destroy(frames[i].image_available);
        device->destroy(frames[i].finish_rendering);
      }

      for (auto &buffer : swapchain.buffers) {
        device->destroy(buffer);
      }

      for (auto &depth : swapchain.depths) {
        device->destroy(depth);
      }

      device->destroy(render_pass);
      device->destroy(swapchain.handle);
      vkDestroySurfaceKHR(inst->handle(), surface.handle, nullptr);
      glfwDestroyWindow(handle);
    }

    void window::create_swapchain(yavf::Device* device) {
      this->device = device;

      for (size_t i = 0; i < device->getFamiliesCount(); ++i) {
        VkBool32 present;
        vkGetPhysicalDeviceSurfaceSupportKHR(device->physicalHandle(), i, surface.handle, &present);

        if (present == VK_TRUE) {
          present_family = i;
          break;
        }
      }

      vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->physicalHandle(), surface.handle, &surface.capabilities);

      // возможно нужно проверить на наличие hdr вывода
      uint32_t count = 0;
      vkGetPhysicalDeviceSurfaceFormatsKHR(device->physicalHandle(), surface.handle, &count, nullptr);
      std::vector<VkSurfaceFormatKHR> formats(count);
      vkGetPhysicalDeviceSurfaceFormatsKHR(device->physicalHandle(), surface.handle, &count, formats.data());

    //   std::cout << "Window formats count " << formats.size() << "\n";
    //   for (uint32_t i = 0; i < formats.size(); ++i) {
    //     std::cout << "Window format " << i << " : " << (formats[i].format) << "\n";
    //   }

      // в будущем понадобится переделывать под разные презент моды
      vkGetPhysicalDeviceSurfacePresentModesKHR(device->physicalHandle(), surface.handle, &count, nullptr);
      std::vector<VkPresentModeKHR> presents(count);
      vkGetPhysicalDeviceSurfacePresentModesKHR(device->physicalHandle(), surface.handle, &count, presents.data());

      int w,h;
      glfwGetWindowSize(handle, &w, &h);

      surface.format = yavf::chooseSwapSurfaceFormat(formats);
      surface.presentMode = yavf::chooseSwapPresentMode(presents);
      surface.extent = yavf::chooseSwapchainExtent(w, h, surface.capabilities);

      auto immediatePresentMode = yavf::checkSwapchainPresentMode(presents, VK_PRESENT_MODE_IMMEDIATE_KHR);
      flags.immediate_present_mode(immediatePresentMode);
      flags.set_vsync(true);
      flags.set_focused(true);

      createSwapChain(surface, device, swapchain);

      const VkFormat depth = yavf::findSupportedFormat(device->physicalHandle(),
                                                      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                                                      VK_IMAGE_TILING_OPTIMAL,
                                                      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

      swapchain.depths.resize(swapchain.images.size(), nullptr);
      frames.resize(swapchain.images.size());
      swapchain.buffers.resize(swapchain.images.size());

      ASSERT(swapchain.images.size() != 0);

      for (uint32_t i = 0; i < swapchain.images.size(); ++i) {
        swapchain.images[i]->createView(VK_IMAGE_VIEW_TYPE_2D, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}, surface.format.format);

        swapchain.depths[i] = device->create(yavf::ImageCreateInfo::texture2D(surface.extent,
                                                                              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                                                              depth),
                                             VMA_MEMORY_USAGE_GPU_ONLY);
        swapchain.depths[i]->createView(VK_IMAGE_VIEW_TYPE_2D, {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1});

        frames[i].image_available = device->createSemaphore();
        frames[i].flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT; // VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        frames[i].finish_rendering = device->createSemaphore();
      }

      create_render_pass();

      for (uint32_t i = 0; i < swapchain.images.size(); ++i) {
        const std::vector<VkImageView> views = {
          swapchain.images[i]->view()->handle(),
          swapchain.depths[i]->view()->handle()
        };
        swapchain.buffers[i] = device->create(yavf::FramebufferCreateInfo::framebuffer(render_pass,
                                                                                       views.size(),
                                                                                       views.data(),
                                                                                       surface.extent.width,
                                                                                       surface.extent.height),
                                              "window_framebuffer_"+std::to_string(i));
      }
      
      // на всякий случай
      ASSERT(swapchain.images[0] != swapchain.images[1] && swapchain.images[0] != swapchain.images[2] && swapchain.images[1] != swapchain.images[2]);
      ASSERT(swapchain.images[0]->handle() != swapchain.images[1]->handle() && swapchain.images[0]->handle() != swapchain.images[2]->handle() && swapchain.images[1]->handle() != swapchain.images[2]->handle());

      yavf::TransferTask* task = device->allocateTransferTask();

      task->begin();
      for (uint32_t i = 0; i < swapchain.depths.size(); ++i) {
        task->setBarrier(swapchain.depths[i], VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
      }
      task->end();

      task->start();
      task->wait();

      device->deallocate(task);
    }

    std::vector<VkClearValue> window::clearValues() const {
//       if (swapchain.image_index == 0) return {{0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 0}};
//       if (swapchain.image_index == 1) return {{1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0}};
      return {{0.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0}};
    }

    VkRect2D window::size() const {
      return {
        {0, 0},
        surface.extent
      };
    }

    yavf::RenderPass window::renderPass() const {
      return render_pass;
    }

    VkViewport window::viewport() const {
      return {
        0.0f, 0.0f,
        static_cast<float>(surface.extent.width),
        static_cast<float>(surface.extent.height),
        0.0f, 1.0f
      };
    }

    VkRect2D window::scissor() const {
      return {
        {0, 0},
        surface.extent
      };
    }

    yavf::Framebuffer window::framebuffer() const {
//       PRINT_VAR("swapchain.image_index", swapchain.image_index)
      return swapchain.buffers[swapchain.image_index];
    }

    void window::recreate(const uint32_t &width, const uint32_t &height) {
      if (surface.extent.width == width && surface.extent.height == height) return;
      device->wait();

      vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->physicalHandle(), surface.handle, &surface.capabilities);
      surface.extent = yavf::chooseSwapchainExtent(width, height, surface.capabilities);

      createSwapChain(surface, device, swapchain);

      const VkFormat depth = yavf::findSupportedFormat(device->physicalHandle(),
                                                      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                                                      VK_IMAGE_TILING_OPTIMAL,
                                                      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

      for (uint32_t i = 0; i < swapchain.images.size(); ++i) {
        device->destroy(swapchain.depths[i]);
        device->destroy(swapchain.buffers[i]);

        swapchain.images[i]->createView(VK_IMAGE_VIEW_TYPE_2D, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}, surface.format.format);

        swapchain.depths[i] = device->create(yavf::ImageCreateInfo::texture2D(surface.extent,
                                                                        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                                                        depth),
                                        VMA_MEMORY_USAGE_GPU_ONLY);
        swapchain.depths[i]->createView(VK_IMAGE_VIEW_TYPE_2D, {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1});

        const std::vector<VkImageView> views = {
          swapchain.images[i]->view()->handle(),
          swapchain.depths[i]->view()->handle()
        };
        swapchain.buffers[i] = device->create(yavf::FramebufferCreateInfo::framebuffer(render_pass,
                                                                                       views.size(),
                                                                                       views.data(),
                                                                                       surface.extent.width,
                                                                                       surface.extent.height),
                                              "window_framebuffer_"+std::to_string(i));
      }

      yavf::TransferTask* task = device->allocateTransferTask();

      task->begin();
      for (uint32_t i = 0; i < swapchain.depths.size(); ++i) {
        task->setBarrier(swapchain.depths[i], VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
      }
      task->end();

      task->start();
      task->wait();

      device->deallocate(task);

      update_buffers();
    }

    void window::resize() {
      int w, h;
      glfwGetWindowSize(handle, &w, &h);
      recreate(w, h);
    }

    void window::next_frame() {
      current_frame = (current_frame + 1) % frames.size();

      VkResult res = swapchain.handle.acquireNextImage(UINT64_MAX, frames[current_frame].image_available, VK_NULL_HANDLE, &swapchain.image_index);

      switch(res) {
        case VK_SUCCESS:
        case VK_SUBOPTIMAL_KHR:
          break;
        case VK_ERROR_OUT_OF_DATE_KHR:
          resize();
          break;
        default:
          //Global::console()->print("Problem occurred during swap chain image acquisition!");
          throw std::runtime_error("Problem occurred during swap chain image acquisition!");
      }
    }

    void window::present() {
      VkResult res;
      {
        // RegionLog rl("vkQueuePresent");

        const VkSwapchainKHR s = swapchain.handle;

        const VkPresentInfoKHR info{
          VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
          nullptr,
          1,
          &frames[current_frame].finish_rendering,
          //nullptr,
          1,
          &s,
          &swapchain.image_index,
          nullptr
        };
        
        //PRINT_VAR("current_frame        ", current_frame)
        //PRINT_VAR("swapchain.image_index", swapchain.image_index)
//         PRINT_VAR("present_family", present_family)

        auto queue = device->getQueue(present_family);
        res = vkQueuePresentKHR(queue.handle, &info);
        //vkQueueWaitIdle(queue.handle);
        //vkDeviceWaitIdle(device->handle());
      }

      // RegionLog rl("switch(res)");

      switch(res) {
        case VK_SUCCESS:
          break;
        case VK_SUBOPTIMAL_KHR:
        case VK_ERROR_OUT_OF_DATE_KHR:
          // int width, height;
          resize();
          break;
        default:
          //Global::console()->print("Problem occurred during image presentation!\n");
          throw std::runtime_error("Problem occurred during image presentation!");
      }
    }
    
    void window::present(const yavf::Internal::Queue &queue) {
      VkResult res;
      {
        // RegionLog rl("vkQueuePresent");

        const VkSwapchainKHR s = swapchain.handle;

        const VkPresentInfoKHR info{
          VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
          nullptr,
          1,
          &frames[current_frame].finish_rendering,
          1,
          &s,
          &swapchain.image_index,
          nullptr
        };
        
//         PRINT_VAR("swapchain.image_index", swapchain.image_index)

        res = vkQueuePresentKHR(queue.handle, &info);
      }

      // RegionLog rl("switch(res)");

      switch(res) {
        case VK_SUCCESS:
          break;
        case VK_SUBOPTIMAL_KHR:
        case VK_ERROR_OUT_OF_DATE_KHR:
          // int width, height;
          resize();
          break;
        default:
          //Global::console()->print("Problem occurred during image presentation!\n");
          throw std::runtime_error("Problem occurred during image presentation!");
      }
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

    void window::create_render_pass() {
      yavf::RenderPassMaker rpm(device);

      render_pass = rpm.attachmentBegin(surface.format.format)
                         //.attachmentLoadOp(VK_ATTACHMENT_LOAD_OP_LOAD)
                         .attachmentLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
                         .attachmentStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
                         //.attachmentInitialLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
                         .attachmentInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
                         .attachmentFinalLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
                       .attachmentBegin(swapchain.depths[0]->info().format)
                         //.attachmentLoadOp(VK_ATTACHMENT_LOAD_OP_LOAD)
                         .attachmentLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
                         .attachmentStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE)
                         //.attachmentInitialLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
                         .attachmentInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
                         .attachmentFinalLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
                       .subpassBegin(VK_PIPELINE_BIND_POINT_GRAPHICS)
                         .subpassColorAttachment(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0)
                         .subpassDepthStencilAttachment(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1)
                       .dependencyBegin(VK_SUBPASS_EXTERNAL, 0)
                         .dependencySrcStageMask(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT) // VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
                         .dependencyDstStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT) // VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT
                         .dependencySrcAccessMask(VK_ACCESS_MEMORY_WRITE_BIT) // VK_ACCESS_SHADER_WRITE_BIT
                         .dependencyDstAccessMask(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT) // VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_INDIRECT_COMMAND_READ_BIT
                       .dependencyBegin(0, VK_SUBPASS_EXTERNAL)
                         .dependencySrcStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
                         .dependencyDstStageMask(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT) // VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT
                         .dependencySrcAccessMask(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT) //VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | 
                         .dependencyDstAccessMask(0)
                       .create("default render pass");
    }

    yavf::Image* window::image() const {
      return swapchain.images[swapchain.image_index];
    }

    yavf::Image* window::depth() const {
      return swapchain.depths[swapchain.image_index];
    }

    void window::screenshot(yavf::Image* container) const {
      if (container->info().extent.width != surface.extent.width || container->info().extent.height != surface.extent.height) {
        container->recreate({surface.extent.width, surface.extent.height, 1});
      }

      static const VkImageSubresourceRange range{
        VK_IMAGE_ASPECT_COLOR_BIT,
        0, 1, 0, 1
      };

      const VkImageBlit blit{
        {
          VK_IMAGE_ASPECT_COLOR_BIT,
          0, 0, 1
        },
        {
          {0, 0, 0},
          {static_cast<int32_t>(surface.extent.width), static_cast<int32_t>(surface.extent.height), 1}
        },
        {
          VK_IMAGE_ASPECT_COLOR_BIT,
          0, 0, 1
        },
        {
          {0, 0, 0},
          {static_cast<int32_t>(surface.extent.width), static_cast<int32_t>(surface.extent.height), 1}
        }
      };

      const uint32_t index = swapchain.image_index == 0 ? swapchain.images.size()-1 : swapchain.image_index-1;
      yavf::Image* img = swapchain.images[index];

      auto trans = device->allocateGraphicTask();
      trans->begin();
      trans->setBarrier(img, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, range, present_family, VK_QUEUE_FAMILY_IGNORED);
      trans->setBarrier(container, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
      trans->copyBlit(img, container, blit);
      trans->setBarrier(img, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, range, VK_QUEUE_FAMILY_IGNORED, present_family);
      trans->setBarrier(container, VK_IMAGE_LAYOUT_GENERAL);
      trans->end();
      device->deallocate(trans);
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

    void window::toggle_vsync() {
      if (!flags.immediate_present_mode()) return;

      flags.set_vsync(!flags.vsync());
      if (flags.vsync()) {
        uint32_t count = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device->physicalHandle(), surface.handle, &count, nullptr);
        std::vector<VkPresentModeKHR> presents(count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device->physicalHandle(), surface.handle, &count, presents.data());

        surface.presentMode = yavf::chooseSwapPresentMode(presents);
      } else {
        surface.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
      }

      resize();
    }

    void window::update_buffers() const {
      //std::cout << "width " << surface.extent.width << " height " << surface.extent.height << "\n";
      //const simd::mat4 &perspective = simd::perspective(glm::radians(fov), float(surface.extent.width) / float(surface.extent.height), 0.1f, FAR_CLIPPING);
      //const simd::mat4 &ortho = simd::ortho(0.0f, float(surface.extent.width) / float(surface.extent.height), 0.0f, 1.0f, 0.1f, FAR_CLIPPING);
      // Global::get<render::buffers>()->set_persp(perspective);
      // Global::get<render::buffers>()->set_ortho(ortho);
      // Global::get<render::buffers>()->set_camera_dim(surface.extent.width, surface.extent.height);
      // Global::get<render::buffers>()->update_data();
    }
  }
}
