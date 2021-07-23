#include "container.h"

#include "utils/globals.h"
#include "shared_structures.h"
#include "render.h"
#include "window.h"
#include "vulkan_hpp_header.h"
#include "makers.h"

#include <iostream>

#define VMA_DEFAULT_MEMORY_SIZE (32 * 1024 * 1024)

static const std::vector<const char*> instanceLayers = {
  //"VK_LAYER_LUNARG_standard_validation",
  "VK_LAYER_KHRONOS_validation",
//   "VK_LAYER_LUNARG_api_dump",
//   "VK_LAYER_LUNARG_object_tracker",
//   "VK_LAYER_GOOGLE_unique_objects",
//   "VK_LAYER_LUNARG_parameter_validation",
//   "VK_LAYER_LUNARG_assistant_layer"
};

namespace devils_engine {
  namespace render {
    vk::Result createSwapChain(const surface &surfaceData, vk::Device device, struct swapchain &swapchainData) {
      uint32_t imageCount = surfaceData.capabilities.minImageCount + 1;
      if (surfaceData.capabilities.maxImageCount > 0 && imageCount > surfaceData.capabilities.maxImageCount) {
        imageCount = surfaceData.capabilities.maxImageCount;
      }
      
      for (const auto &img : swapchainData.images) {
        device.destroy(img.buffer);
        device.destroy(img.view);
        device.destroy(img.depth_view);
        device.destroy(img.depth);
      }
      
      swapchainData.images.clear();

      auto old = swapchainData.handle;

      const vk::SwapchainCreateInfoKHR info(
        {},
        surfaceData.handle,
        imageCount,
        surfaceData.format.format,
        surfaceData.format.colorSpace,
        surfaceData.extent,
        1,
        vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst,
        vk::SharingMode::eExclusive,
        0,
        nullptr,
        surfaceData.capabilities.currentTransform,
        vk::CompositeAlphaFlagBitsKHR::eOpaque,
        surfaceData.presentMode,
        VK_TRUE,
        old
      );

      swapchainData.handle = device.createSwapchainKHR(info);
      device.destroy(old);
      set_name(device, swapchainData.handle, "window_swapchain");
      const auto images = device.getSwapchainImagesKHR(swapchainData.handle);
      swapchainData.images.resize(images.size());
      for (size_t i = 0; i < images.size(); ++i) {
        swapchainData.images[i].handle = images[i];
        set_name(device, images[i], "swapchain image " + std::to_string(i));
      }

      return vk::Result::eSuccess;
    }
    
    container::container() : 
      mem(sizeof(vulkan_container) + sizeof(vulkan_window) + sizeof(struct window) + sizeof(render::stage_container)), 
      window(nullptr), 
      vulkan(nullptr), 
      vlk_window(nullptr), 
      render(nullptr) 
    {}
    container::~container() {
      vulkan->device.waitIdle();

      mem.destroy(render);
      mem.destroy(window);
      mem.destroy(vlk_window);
      mem.destroy(vulkan);
    }

    vk::Instance* container::create_instance(const std::vector<const char*> &extensions, const application_info* app_info) {
      const vk::ApplicationInfo info(
        app_info->app_name.c_str(),
        app_info->app_version,
        app_info->engine_name.c_str(),
        app_info->engine_version,
        app_info->api_version
      );
      
      auto copy_ext = extensions;
#ifndef _NDEBUG
//       copy_ext.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME); // это старый походу
      copy_ext.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
      
      const vk::InstanceCreateInfo inst_info(
        {},
        &info,
#ifndef _NDEBUG
        instanceLayers, 
#else
        nullptr,
#endif
        copy_ext
      );
      
      vulkan = mem.create<vulkan_container>();
      vulkan->instance = vk::createInstanceUnique(inst_info);
      
#ifndef _NDEBUG
      debug_container_pointer = vulkan;
      load_debug_extensions_functions(vulkan->instance.get());
#endif
      
      setupDebugCallback(vulkan->instance.get(), &vulkan->callback);
      
      return &vulkan->instance.get();
    }

    struct window* container::create_window(const window_info &info) {
      window = mem.create<struct window>(window::create_info{this, info.fullscreen, info.width, info.height, 0.0f, info.video_mode});
      return window;
    }

    vk::Device* container::create_device() {
      const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
// #ifndef _NDEBUG
//         "VK_EXT_debug_marker", // старое расширение
// #endif
      };

      VkSurfaceKHR s = window->create_surface(vulkan->instance.get());
      vulkan->surface = s;

      auto physDevices = vulkan->instance->enumeratePhysicalDevices();

      // как выбирать устройство?
      size_t maxMem = 0;

      vk::PhysicalDevice choosen;
      for (size_t i = 0; i < physDevices.size(); ++i) {
        vk::PhysicalDeviceProperties deviceProperties;
        vk::PhysicalDeviceFeatures deviceFeatures;
        vk::PhysicalDeviceMemoryProperties memProp;
        physDevices[i].getProperties(&deviceProperties);
        physDevices[i].getFeatures(&deviceFeatures);
        physDevices[i].getMemoryProperties(&memProp);

        size_t a = 0;
        for (uint32_t j = 0; j < memProp.memoryHeapCount; ++j) {
          //VK_MEMORY_HEAP_DEVICE_LOCAL_BIT
          if ((memProp.memoryHeaps[i].flags & vk::MemoryHeapFlagBits::eDeviceLocal) == vk::MemoryHeapFlagBits::eDeviceLocal) {
            a = std::max(memProp.memoryHeaps[i].size, a);
          }
        }

    //     std::cout << "Device name: " << deviceProperties.deviceName << "\n";

        bool extSupp = checkDeviceExtensions(physDevices[i], instanceLayers, deviceExtensions);

        uint32_t count = 0;
        physDevices[i].getQueueFamilyProperties(&count, nullptr);

        bool presentOk = false;
        for (uint32_t i = 0; i < count; ++i) {
          VkBool32 present;
          const auto ret = physDevices[i].getSurfaceSupportKHR(i, s, &present);
          if (ret != vk::Result::eSuccess) throw std::runtime_error("getSurfaceSupportKHR failed");

          if (present) {
            presentOk = true;
            break;
          }
        }

        if (extSupp && presentOk && maxMem < a) {
          maxMem = a;
          choosen = physDevices[i];
          //break;
        }
      }

      if (choosen == vk::PhysicalDevice(nullptr)) throw std::runtime_error("choosen.handle() == VK_NULL_HANDLE");
      
      vk::PhysicalDeviceProperties deviceProperties;
      choosen.getProperties(&deviceProperties);
      std::cout << "Using device: " << deviceProperties.deviceName << '\n';
      
      vk::PhysicalDeviceFeatures f_test;
      choosen.getFeatures(&f_test);
      device_properties.set(physical_device_sampler_anisotropy, f_test.samplerAnisotropy == VK_TRUE);
      device_properties.set(physical_device_multidraw_indirect, f_test.multiDrawIndirect == VK_TRUE);

      device_maker dm(&vulkan->instance.get());
      vk::PhysicalDeviceFeatures f = {};
      f.samplerAnisotropy = is_properties_presented(physical_device_sampler_anisotropy);
      f.multiDrawIndirect = is_properties_presented(physical_device_multidraw_indirect);
      //f.geometryShader = VK_TRUE;
    //   f.multiDrawIndirect = VK_TRUE;
    //   f.drawIndirectFirstInstance = VK_TRUE;
    //   f.fragmentStoresAndAtomics = VK_TRUE;
//       f.wideLines = VK_TRUE;
      auto dev = dm.beginDevice(choosen).setExtensions(deviceExtensions).createQueues(1).features(f).create(instanceLayers, "Graphics device");
      vulkan->physical_device = choosen;
      vulkan->device = dev;
      
#ifndef _NDEBUG
      load_debug_extensions_functions(vulkan->device);
#endif
      
      {
        vma::AllocatorCreateInfo alloc_info(
          {},
          vulkan->physical_device,
          vulkan->device,
          VMA_DEFAULT_MEMORY_SIZE
        );
        
        alloc_info.instance = vulkan->instance.get();
        alloc_info.vulkanApiVersion = VK_API_VERSION_1_0;
        
        vulkan->buffer_allocator = vma::createAllocator(alloc_info);
      }

      {
        descriptor_pool_maker dpm(&dev);

        vulkan->descriptor_pool = 
          dpm.poolSize(vk::DescriptorType::eStorageBuffer, 20)
             .poolSize(vk::DescriptorType::eUniformBuffer, 10)
             .poolSize(vk::DescriptorType::eStorageImage, 10)
             .poolSize(vk::DescriptorType::eCombinedImageSampler, 10)
             .create(DEFAULT_DESCRIPTOR_POOL_NAME);
      }

      {
        descriptor_set_layout_maker dlm(&dev);

        vulkan->uniform_layout = 
          dlm.binding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eAll)
             .binding(1, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eAll)
             .create(UNIFORM_BUFFER_LAYOUT_NAME);
        vulkan->storage_layout = dlm.binding(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eAll).create(STORAGE_BUFFER_LAYOUT_NAME);
      }
      
      const uint32_t common_family = findDeviceQueueFamilyIndex(vulkan->physical_device, vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute);
      if (common_family != UINT32_MAX) {
        assert(checkDeviceQueueFamilyPresentCapability(vulkan->physical_device, common_family, s));
        vulkan->graphics_family = vulkan->compute_family = vulkan->present_family = common_family;
      } else {
        //const auto [graphics_family, compute_family, present_family] = findDeviceQueueFamiliesIndices(vulkan->physical_device, s);
        vulkan->graphics_family = findDeviceQueueFamilyIndex(vulkan->physical_device, vk::QueueFlagBits::eGraphics);
        vulkan->compute_family = findDeviceQueueFamilyIndex(vulkan->physical_device, vk::QueueFlagBits::eGraphics);
        vulkan->present_family = checkDeviceQueueFamilyPresentCapability(vulkan->physical_device, vulkan->graphics_family, s) ? vulkan->graphics_family : vulkan->compute_family;
        assert(checkDeviceQueueFamilyPresentCapability(vulkan->physical_device, vulkan->present_family, s));
      }
      
      vulkan->graphics = dev.getQueue(vulkan->graphics_family, 0);
      vulkan->compute  = dev.getQueue(vulkan->compute_family,  0);
      vulkan->present  = dev.getQueue(vulkan->present_family,  0);
      set_name(vulkan->device, vulkan->graphics, "graphics queue");
      if (vulkan->compute != vulkan->graphics) set_name(vulkan->device, vulkan->graphics, "compute queue");
      if (vulkan->present != vulkan->graphics && vulkan->present != vulkan->compute) set_name(vulkan->device, vulkan->graphics, "present queue");
      
      // если у нас аж 3 разных очереди, то я понятие не имею что делать
      // создам пока что из одной командный пул
      
      {
        const vk::CommandPoolCreateInfo ci(
          vk::CommandPoolCreateFlagBits::eResetCommandBuffer | vk::CommandPoolCreateFlagBits::eTransient,
          vulkan->graphics_family
        );
        vulkan->command_pool = dev.createCommandPool(ci);
        set_name(vulkan->device, vulkan->command_pool, "graphics command pool");
      }
      
      {
        const vk::CommandPoolCreateInfo ci(
          vk::CommandPoolCreateFlagBits::eTransient, // если резетится тоже помогает
          vulkan->graphics_family
        );
        vulkan->transfer_command_pool = dev.createCommandPool(ci);
        set_name(vulkan->device, vulkan->command_pool, "transfer command pool");
      }
      
      //const vk::FenceCreateInfo finfo(vk::FenceCreateFlagBits::eSignaled);
      const vk::FenceCreateInfo finfo;
      vulkan->transfer_fence = vulkan->device.createFence(finfo);
      set_name(vulkan->device, vulkan->transfer_fence, "transfer fence");

      return &vulkan->device;
    }
    
    void container::create_vlk_window() {
      vlk_window = mem.create<vulkan_window>(&vulkan->instance.get(), &vulkan->device, &vulkan->physical_device, vulkan->surface);
      set_name(vulkan->device, vlk_window->surface.handle, "window surface");
#ifndef _NDEBUG
      debug_window_pointer = vlk_window;
#endif
    }
    
    void container::create_swapchain() {
      vlk_window->surface.capabilities = vulkan->physical_device.getSurfaceCapabilitiesKHR(vlk_window->surface.handle);
      // возможно нужно проверить на наличие hdr вывода
      const auto formats = vulkan->physical_device.getSurfaceFormatsKHR(vlk_window->surface.handle);
      // в будущем понадобится переделывать под разные презент моды
      const auto presents = vulkan->physical_device.getSurfacePresentModesKHR(vlk_window->surface.handle);

      const auto [w, h] = window->size();

      vlk_window->surface.format = chooseSwapchainSurfaceFormat(formats);
      vlk_window->surface.presentMode = chooseSwapchainPresentMode(presents);
      vlk_window->surface.extent = chooseSwapchainExtent(w, h, vlk_window->surface.capabilities);

      auto immediatePresentMode = checkSwapchainPresentMode(presents, vk::PresentModeKHR::eImmediate);
      window->flags.immediate_present_mode(immediatePresentMode);
      window->flags.set_vsync(true);
      window->flags.set_focused(true);

      createSwapChain(vlk_window->surface, vulkan->device, vlk_window->swapchain);

      const vk::Format depth = findSupportedFormat(
        vulkan->physical_device,
        {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment
      );

//       swapchain.depths.resize(swapchain.images.size(), nullptr);
      vlk_window->swapchain.frames.resize(vlk_window->swapchain.images.size());
      vlk_window->swapchain.depth_format = depth;
//       swapchain.buffers.resize(swapchain.images.size());

      ASSERT(vlk_window->swapchain.images.size() != 0);
      
      const size_t images_count = vlk_window->swapchain.images.size();
//       size_t depth_image_size = 0; // align_to(vulkan->surface.extent.width * vulkan->surface.extent.height * 4, 16)
//       size_t depth_images_size = 0; // depth_image_size * images_count
      
      const auto depth_info = texture2D(vlk_window->surface.extent, vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferDst, depth);
      const auto [depth_images, depth_mem] = create_images(vulkan->device, vulkan->physical_device, depth_info, vk::MemoryPropertyFlagBits::eDeviceLocal, images_count);
      vlk_window->swapchain.depth_memory = depth_mem;
      set_name(vulkan->device, depth_mem, "swapchain depth image memory");
      
      for (uint32_t i = 0; i < vlk_window->swapchain.images.size(); ++i) {
        // view для основной картинки
        {
          const vk::ImageViewCreateInfo view_info = make_view_info(vlk_window->swapchain.images[i].handle, vlk_window->surface.format.format);
          vlk_window->swapchain.images[i].view = vulkan->device.createImageView(view_info);
          set_name(vulkan->device, vlk_window->swapchain.images[i].view, "swapchain image " + std::to_string(i) + " view");
        }
        
        // глубина
        vlk_window->swapchain.images[i].depth = depth_images[i];
        set_name(vulkan->device, vlk_window->swapchain.images[i].depth, "swapchain depth image " + std::to_string(i));
        
        // VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED - ВМА говорит что этот флаг существует только на мобилках, и с него никакого толку на компах
        // делаем вью для глубины
        {
          const vk::ImageViewCreateInfo view_info = make_view_info(vlk_window->swapchain.images[i].depth, depth, vk::ImageViewType::e2D, {vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1});
          vlk_window->swapchain.images[i].depth_view = vulkan->device.createImageView(view_info);
          set_name(vulkan->device, vlk_window->swapchain.images[i].depth_view, "swapchain depth image " + std::to_string(i) + " view");
        }
        
        const vk::SemaphoreCreateInfo si;
        vlk_window->swapchain.frames[i].image_available = vulkan->device.createSemaphore(si);
        vlk_window->swapchain.frames[i].flags = vk::PipelineStageFlagBits::eTopOfPipe; // VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        vlk_window->swapchain.frames[i].finish_rendering = vulkan->device.createSemaphore(si);
        const vk::FenceCreateInfo fi(vk::FenceCreateFlagBits::eSignaled);
        vlk_window->swapchain.frames[i].fence = vulkan->device.createFence(fi);
        
        set_name(vulkan->device, vlk_window->swapchain.frames[i].image_available, "frame image availability semaphore " + std::to_string(i));
        set_name(vulkan->device, vlk_window->swapchain.frames[i].finish_rendering, "frame finish rendering semaphore " + std::to_string(i));
        set_name(vulkan->device, vlk_window->swapchain.frames[i].fence, "frame fence " + std::to_string(i));
      }

      create_render_pass();

      for (uint32_t i = 0; i < vlk_window->swapchain.images.size(); ++i) {
        const std::initializer_list<vk::ImageView> views = {
          vlk_window->swapchain.images[i].view,
          vlk_window->swapchain.images[i].depth_view
        };
        vlk_window->swapchain.images[i].buffer = vulkan->device.createFramebuffer(
          vk::FramebufferCreateInfo(
            {},
            vlk_window->render_pass,
            views,
            vlk_window->surface.extent.width,
            vlk_window->surface.extent.height,
            1
          )
        );
        set_name(vulkan->device, vlk_window->swapchain.images[i].buffer, "window_framebuffer_"+std::to_string(i));
      }
      
      // на всякий случай
      ASSERT(vlk_window->swapchain.images[0].handle != vlk_window->swapchain.images[1].handle && 
             vlk_window->swapchain.images[0].handle != vlk_window->swapchain.images[2].handle && 
             vlk_window->swapchain.images[1].handle != vlk_window->swapchain.images[2].handle);
      
      // вообще по идее нам не нужно менять лайоут у картинки потому что он смениться автоматически в рендерпасе
//       const vk::CommandBufferAllocateInfo info(vulkan->transfer_command_pool, vk::CommandBufferLevel::ePrimary, 1);
//       auto task = vulkan->device.allocateCommandBuffers(info)[0];
//       
//       const vk::CommandBufferBeginInfo binfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
//       task.begin(binfo);
//       for (size_t i = 0; i < vlk_window->swapchain.images.size(); ++i) {
//         // барьер
//       }
//       
//       task.end();
//       
//       const vk::SubmitInfo submit(nullptr, nullptr, task, nullptr);
//       vulkan->graphics.submit(submit, vulkan->transfer_fence);
//       
//       const auto res = vulkan->device.waitForFences(vulkan->transfer_fence, VK_TRUE, SIZE_MAX);
//       if (res != vk::Result::eSuccess) throw std::runtime_error("waitForFences failed");
//       
//       vulkan->device.freeCommandBuffers(vulkan->transfer_command_pool, task);
    }
    
    void container::create_render_pass() {
      render_pass_maker rpm(&vulkan->device);
      
      vlk_window->render_pass = 
        rpm.attachmentBegin(vlk_window->surface.format.format)
             //.attachmentLoadOp(VK_ATTACHMENT_LOAD_OP_LOAD)
             .attachmentLoadOp(vk::AttachmentLoadOp::eClear)
             .attachmentStoreOp(vk::AttachmentStoreOp::eStore)
             //.attachmentInitialLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
             .attachmentInitialLayout(vk::ImageLayout::eUndefined)
             .attachmentFinalLayout(vk::ImageLayout::ePresentSrcKHR)
           .attachmentBegin(vlk_window->swapchain.depth_format)
             //.attachmentLoadOp(VK_ATTACHMENT_LOAD_OP_LOAD)
             .attachmentLoadOp(vk::AttachmentLoadOp::eClear)
             .attachmentStoreOp(vk::AttachmentStoreOp::eStore)
             //.attachmentInitialLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
             .attachmentInitialLayout(vk::ImageLayout::eUndefined)
             .attachmentFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
           .subpassBegin(vk::PipelineBindPoint::eGraphics)
             .subpassColorAttachment(0, vk::ImageLayout::eColorAttachmentOptimal)
             .subpassDepthStencilAttachment(1, vk::ImageLayout::eDepthStencilAttachmentOptimal)
           .dependencyBegin(VK_SUBPASS_EXTERNAL, 0)
    //                          .dependencySrcStageMask(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT) // VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
    //                          .dependencyDstStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT) // VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT
    //                          .dependencySrcAccessMask(VK_ACCESS_MEMORY_WRITE_BIT) // VK_ACCESS_SHADER_WRITE_BIT
    //                          .dependencyDstAccessMask(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT) // VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_INDIRECT_COMMAND_READ_BIT
             .dependencySrcStageMask(vk::PipelineStageFlagBits::eComputeShader)
             .dependencyDstStageMask(vk::PipelineStageFlagBits::eDrawIndirect)
             .dependencySrcAccessMask(vk::AccessFlagBits::eShaderWrite)
             .dependencyDstAccessMask(vk::AccessFlagBits::eIndirectCommandRead)
           .dependencyBegin(0, VK_SUBPASS_EXTERNAL)
    //                          .dependencySrcStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
    //                          .dependencyDstStageMask(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT) // VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT
    //                          .dependencySrcAccessMask(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT) //VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | 
    //                          .dependencyDstAccessMask(0)
             .dependencySrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
             .dependencyDstStageMask(vk::PipelineStageFlagBits::eTransfer) //VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
             // трансфер еще отвечает за задание значений через кмд филл баффер
             .dependencySrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite) //VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | 
             .dependencyDstAccessMask(vk::AccessFlagBits::eTransferWrite)
           .create("default render pass");
                       
       vlk_window->render_pass_objects = 
         rpm.attachmentBegin(vlk_window->surface.format.format)
              .attachmentLoadOp(vk::AttachmentLoadOp::eLoad)
              .attachmentStoreOp(vk::AttachmentStoreOp::eStore)
              //.attachmentInitialLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
              .attachmentInitialLayout(vk::ImageLayout::ePresentSrcKHR)
              .attachmentFinalLayout(vk::ImageLayout::ePresentSrcKHR)
            .attachmentBegin(vlk_window->swapchain.depth_format)
              .attachmentLoadOp(vk::AttachmentLoadOp::eLoad)
              .attachmentStoreOp(vk::AttachmentStoreOp::eDontCare)
              //.attachmentInitialLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
              .attachmentInitialLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
              .attachmentFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
            .subpassBegin(vk::PipelineBindPoint::eGraphics)
              .subpassColorAttachment(0, vk::ImageLayout::eColorAttachmentOptimal)
              .subpassDepthStencilAttachment(1, vk::ImageLayout::eDepthStencilAttachmentOptimal)
            .dependencyBegin(VK_SUBPASS_EXTERNAL, 0)
              .dependencySrcStageMask(vk::PipelineStageFlagBits::eComputeShader) // VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
              .dependencyDstStageMask(vk::PipelineStageFlagBits::eDrawIndirect) // VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT
              .dependencySrcAccessMask(vk::AccessFlagBits::eShaderWrite) // VK_ACCESS_SHADER_WRITE_BIT
              .dependencyDstAccessMask(vk::AccessFlagBits::eIndirectCommandRead) // VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_INDIRECT_COMMAND_READ_BIT
  //                                 .dependencySrcStageMask(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT) // VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
  //                                 .dependencyDstStageMask(VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT) // VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT
  //                                 .dependencySrcAccessMask(VK_ACCESS_SHADER_WRITE_BIT) // VK_ACCESS_SHADER_WRITE_BIT
  //                                 .dependencyDstAccessMask(VK_ACCESS_INDIRECT_COMMAND_READ_BIT) // VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_INDIRECT_COMMAND_READ_BIT
            .dependencyBegin(0, VK_SUBPASS_EXTERNAL)
              .dependencySrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
              .dependencyDstStageMask(vk::PipelineStageFlagBits::eTopOfPipe) // VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT
              .dependencySrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite) //VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | 
              .dependencyDstAccessMask({})
            .create("default render pass objects");
    }

    render::stage_container* container::create_system(const size_t &system_container_size) {
      render = mem.create<render::stage_container>(system_container_size);
      return render;
    }

    void container::create_tasks() {
      //vulkan->command_buffers.resize(window->swapchain_frames_count(), nullptr);
      //vulkan->command_buffers.shrink_to_fit();
      
      const vk::CommandBufferAllocateInfo al_info(vulkan->command_pool, vk::CommandBufferLevel::ePrimary, swapchain_frames_count());
      vulkan->command_buffers = vulkan->device.allocateCommandBuffers(al_info);
      for (size_t i = 0; i < vulkan->command_buffers.size(); ++i) {
        set_name(vulkan->device, vulkan->command_buffers[i], "frame command buffer " + std::to_string(i));
      }

//       for (size_t i = 0 ; i < vulkan->command_buffers.size(); ++i) {
//         vulkan->command_buffers[i] = vulkan->device.allocateCommandBuffers(al_info);
//         tasks[i]->pushWaitSemaphore(window->frames[i].image_available, window->frames[i].flags);
//         tasks[i]->pushSignalSemaphore(window->frames[i].finish_rendering);
//       }
    }
    
    bool container::is_properties_presented(const uint32_t &index) const {
      return device_properties.get(index);
    }
    
    void container::next_frame() {
      vlk_window->swapchain.current_frame = (vlk_window->swapchain.current_frame + 1) % vlk_window->swapchain.frames.size();
      
      const auto &cur_index = vlk_window->swapchain.current_frame;
      const auto &frames = vlk_window->swapchain.frames;
      const auto [res, index] = vulkan->device.acquireNextImageKHR(vlk_window->swapchain.handle, SIZE_MAX, frames[cur_index].image_available);
      vlk_window->swapchain.image_index = index;

      switch(res) {
        case vk::Result::eSuccess:
        case vk::Result::eSuboptimalKHR: break;
        case vk::Result::eErrorOutOfDateKHR: window->resize(); break;
        default: throw std::runtime_error("Problem occurred during swap chain image acquisition!");
      }
    }
    
    void container::present() {
      vk::Result res;
      {
        // RegionLog rl("vkQueuePresent");

        const auto &s = vlk_window->swapchain.handle;
        const auto &index = vlk_window->swapchain.image_index;
        const auto &cur_index = vlk_window->swapchain.current_frame;
        const auto &frames = vlk_window->swapchain.frames;
        const vk::PresentInfoKHR info(frames[cur_index].finish_rendering, s, index);
        
        //PRINT_VAR("current_frame        ", current_frame)
        //PRINT_VAR("swapchain.image_index", swapchain.image_index)
//         PRINT_VAR("present_family", present_family)
        
        res = vulkan->present.presentKHR(&info);

        //vkQueueWaitIdle(queue.handle);
        //vkDeviceWaitIdle(device->handle());
      }

      // RegionLog rl("switch(res)");

      switch(res) {
        case vk::Result::eSuccess: break;
        case vk::Result::eSuboptimalKHR:
        case vk::Result::eErrorOutOfDateKHR: window->resize(); break;
        default: throw std::runtime_error("Problem occurred during image presentation!");
      }
    }
    
    vk::Image* container::image() const {
      return &vlk_window->swapchain.images[vlk_window->swapchain.image_index].handle;
    }
    
    vk::Image* container::depth() const {
      return &vlk_window->swapchain.images[vlk_window->swapchain.image_index].depth;
    }
    
    uint32_t container::swapchain_frames_count() const {
      return vlk_window->swapchain.images.size();
    }
    
    uint32_t container::swapchain_current_image() const {
      return vlk_window->swapchain.image_index;
    }
    
    uint32_t container::swapchain_current_frame() const {
      return vlk_window->swapchain.current_frame;
    }
    
    void container::toggle_vsync() {
      if (!window->flags.immediate_present_mode()) return;

      window->flags.set_vsync(!window->flags.vsync());
      if (window->flags.vsync()) {
        const auto modes = vulkan->physical_device.getSurfacePresentModesKHR(vlk_window->surface.handle);
        vlk_window->surface.presentMode = chooseSwapchainPresentMode(modes);
      } else {
        vlk_window->surface.presentMode = vk::PresentModeKHR::eImmediate;
      }

      window->resize();
    }
    
    std::tuple<clear_value, clear_value> container::clear_values() const {
      return std::make_tuple(clear_value{0.0f, 0.0f, 0.0f, 1.0f}, clear_value{1.0f, 0});
    }
    
    rect2d container::size() const {
      return { {0, 0}, cast(vlk_window->surface.extent) };
    }
    
    struct viewport container::viewport() const {
      return {
        0.0f, 0.0f,
        static_cast<float>(vlk_window->surface.extent.width),
        static_cast<float>(vlk_window->surface.extent.height),
        0.0f, 1.0f
      };
    }
    
    rect2d container::scissor() const {
      return { {0, 0}, cast(vlk_window->surface.extent) };
    }
    
    vk::RenderPass* container::render_pass() const {
      return &vlk_window->render_pass;
    }
    
    vk::RenderPass* container::render_pass_objects() const {
      return &vlk_window->render_pass_objects;
    }
    
    vk::Framebuffer* container::current_buffer() const {
      return &vlk_window->swapchain.images[swapchain_current_image()].buffer;
    }
    
    vk::Fence* container::current_frame_fence() const {
      return &vlk_window->swapchain.frames[swapchain_current_frame()].fence;
    }
    
    vk::Semaphore* container::image_wait_semaphore() const {
      return &vlk_window->swapchain.frames[swapchain_current_frame()].image_available;
    }
    
    vk::Semaphore* container::finish_semaphore() const {
      return &vlk_window->swapchain.frames[swapchain_current_frame()].finish_rendering;
    }
    
    vk::Queue* container::queue() const {
      return &vulkan->graphics;
    }
    
    uint32_t container::wait_pipeline_stage() const {
      return uint32_t(vlk_window->swapchain.frames[swapchain_current_frame()].flags);
    }

//     yavf::TaskInterface* container::interface() const {
//       return tasks[window->current_frame];
//     }
// 
//     yavf::CombinedTask* container::combined() const {
//       return tasks[window->current_frame];
//     }
// 
//     yavf::ComputeTask* container::compute() const {
//       return tasks[window->current_frame];
//     }
// 
//     yavf::GraphicTask* container::graphics() const {  
//       return tasks[window->current_frame];
//     }
// 
//     yavf::TransferTask* container::transfer() const {
//       return nullptr;
//     }

    vk::CommandBuffer* container::command_buffer() const {
      return &vulkan->command_buffers[swapchain_current_frame()];
    }
    
    void container::recreate(const uint32_t &width, const uint32_t &height) {
      vulkan->device.waitIdle();
      
      const auto res = vulkan->physical_device.getSurfaceCapabilitiesKHR(vlk_window->surface.handle, &vlk_window->surface.capabilities);
      if (res != vk::Result::eSuccess) throw std::runtime_error("getSurfaceCapabilitiesKHR failed");
      
      vlk_window->surface.extent = chooseSwapchainExtent(width, height, vlk_window->surface.capabilities);

      createSwapChain(vlk_window->surface, vulkan->device, vlk_window->swapchain);

      const vk::Format depth = findSupportedFormat(
        vulkan->physical_device,
        {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment
      );
      
      const size_t images_count = vlk_window->swapchain.images.size();
      vulkan->device.free(vlk_window->swapchain.depth_memory);
      const auto depth_info = texture2D(vlk_window->surface.extent, vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferDst, depth);
      const auto [depth_images, depth_mem] = create_images(vulkan->device, vulkan->physical_device, depth_info, vk::MemoryPropertyFlagBits::eDeviceLocal, images_count);
      vlk_window->swapchain.depth_memory = depth_mem;
      set_name(vulkan->device, depth_mem, "swapchain depth image memory");

      for (uint32_t i = 0; i < vlk_window->swapchain.images.size(); ++i) {
        // view для основной картинки
        {
          const vk::ImageViewCreateInfo view_info = make_view_info(vlk_window->swapchain.images[i].handle, vlk_window->surface.format.format);
          vlk_window->swapchain.images[i].view = vulkan->device.createImageView(view_info);
          set_name(vulkan->device, vlk_window->swapchain.images[i].view, "swapchain image " + std::to_string(i) + " view");
        }
        
        // глубина
        vlk_window->swapchain.images[i].depth = depth_images[i];
        set_name(vulkan->device, vlk_window->swapchain.images[i].depth, "swapchain depth image " + std::to_string(i));
        
        // VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED - ВМА говорит что этот флаг существует только на мобилках, и с него никакого толку на компах
        // делаем вью для глубины
        {
          const vk::ImageViewCreateInfo view_info = make_view_info(vlk_window->swapchain.images[i].depth, depth, vk::ImageViewType::e2D, {vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1});
          vlk_window->swapchain.images[i].depth_view = vulkan->device.createImageView(view_info);
          set_name(vulkan->device, vlk_window->swapchain.images[i].depth_view, "swapchain depth image " + std::to_string(i) + " view");
        }
        
        const std::initializer_list<vk::ImageView> views = {
          vlk_window->swapchain.images[i].view,
          vlk_window->swapchain.images[i].depth_view
        };
        vlk_window->swapchain.images[i].buffer = vulkan->device.createFramebuffer(
          vk::FramebufferCreateInfo(
            {},
            vlk_window->render_pass,
            views,
            vlk_window->surface.extent.width,
            vlk_window->surface.extent.height,
            1
          )
        );
        set_name(vulkan->device, vlk_window->swapchain.images[i].buffer, "window_framebuffer_"+std::to_string(i));
      }

      // тут была смена лайоута, но я не понимаю зачем это делать если лайоут меняется самостоятельно в рендерпассе
    }
    
    void container::print_memory_info() const {
      auto allocator = vulkan->buffer_allocator;
      const auto stats = allocator.calculateStats();
      PRINT_VAR("total           block count", stats.total.blockCount)
      PRINT_VAR("total      allocation count", stats.total.allocationCount)
      PRINT_VAR("total    unused range count", stats.total.unusedRangeCount)
      PRINT_VAR("total            used bytes", stats.total.usedBytes)
      PRINT_VAR("total          unused bytes", stats.total.unusedBytes)
      PRINT_VAR("total   allocation size min", stats.total.allocationSizeMin)
      PRINT_VAR("total   allocation size avg", stats.total.allocationSizeAvg)
      PRINT_VAR("total   allocation size max", stats.total.allocationSizeMax)
      PRINT_VAR("total unused range size min", stats.total.unusedRangeSizeMin)
      PRINT_VAR("total unused range size avg", stats.total.unusedRangeSizeAvg)
      PRINT_VAR("total unused range size max", stats.total.unusedRangeSizeMax)
    }
  }
}
