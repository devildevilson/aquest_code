#include "vulkan_hpp_header.h"

#include <cstring>
#include <iostream>
#include <fstream>

#include "parallel_hashmap/phmap.h"
#include "utils/constexpr_funcs.h"

// для бектрейсинга
#ifndef _WIN32
  #include <execinfo.h>
  #include <signal.h>
  #include <unistd.h>
//#else
#endif

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
  VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
  VkDebugUtilsMessageTypeFlagsEXT messageType, 
  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
  void* pUserData
) {
  auto hpp_messageSeverity = static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(messageSeverity);
  const auto severity_str = vk::to_string(hpp_messageSeverity);
  
  auto hpp_messageType = vk::DebugUtilsMessageTypeFlagsEXT(messageType);
  const auto type_str = vk::to_string(hpp_messageType);
  
  std::cout << "[Vulkan : DEBUG]" << "\n";
  std::cout << "Message type: " << type_str << " severity: " << severity_str << "\n";
  
  if (pCallbackData == nullptr) return VK_FALSE;
  
  if (pCallbackData->pMessageIdName != nullptr) std::cout << "Message ID: " << pCallbackData->pMessageIdName << "\n";
  if (pCallbackData->pMessage != nullptr) std::cout << "Message: " << pCallbackData->pMessage << "\n";
  
  for (uint32_t i = 0; i < pCallbackData->objectCount; ++i) {
    auto type = static_cast<vk::ObjectType>(pCallbackData->pObjects[i].objectType);
    const auto type_str = vk::to_string(type);
    
    std::cout << "Object type: " << type_str << " handle: " << (void*)pCallbackData->pObjects[i].objectHandle << "\n";
    if (pCallbackData->pObjects[i].pObjectName != nullptr) std::cout << "Object name: " << pCallbackData->pObjects[i].pObjectName << '\n';
  }
  
  if (pCallbackData->queueLabelCount > 0) {
    std::cout << "Queue labels: " << "\n";
    for (uint32_t i = 0; i < pCallbackData->queueLabelCount; ++i) {
      std::cout << "Label " << i << " name: " << pCallbackData->pQueueLabels[i].pLabelName << "\n";
    }
  }

  if (pCallbackData->cmdBufLabelCount > 0) {
    std::cout << "Command buffer labels: " << "\n";
    for (uint32_t i = 0; i < pCallbackData->cmdBufLabelCount; ++i) {
      std::cout << "Label " << i << " name: " << pCallbackData->pCmdBufLabels[i].pLabelName << "\n";
    }
  }

  std::cout << "\n";
  
  if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
#ifndef _WIN32
    void *array[200];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 200);
    // print out all the frames to stderr
//     fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
#endif
  }
  
  (void)pUserData;

  return VK_FALSE;
}

namespace devils_engine {
  namespace render {
    void setupDebugCallback(vk::Instance instance, VkDebugUtilsMessengerEXT* callback) {
      if (pfnCreateDebugUtilsMessenger == nullptr) return;

      // vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | 
      auto createInfo = vk::DebugUtilsMessengerCreateInfoEXT(
        vk::DebugUtilsMessengerCreateFlagsEXT(),
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        debugCallback,
        nullptr
      );

      // Vulkan-hpp has methods for this, but they trigger linking errors...
      //instance->createDebugUtilsMessengerEXT(createInfo);
      //instance->createDebugUtilsMessengerEXTUnique(createInfo);

      // reinterpret_cast is also used by vulkan.hpp internally for all these structs
      const auto res = pfnCreateDebugUtilsMessenger(instance, reinterpret_cast<const VkDebugUtilsMessengerCreateInfoEXT*>(&createInfo), nullptr, callback);
      if (res != VK_SUCCESS) throw std::runtime_error("failed to set up debug callback!");
    }
    
    void destroyDebugCallback(vk::Instance instance, VkDebugUtilsMessengerEXT callback) {
      if (pfnDestroyDebugUtilsMessenger == nullptr && callback != VK_NULL_HANDLE) throw std::runtime_error("There is no destroy debug function");
      if (pfnDestroyDebugUtilsMessenger == nullptr) return;
      
      pfnDestroyDebugUtilsMessenger(instance, callback, nullptr);
    }
    
    vulkan_container::vulkan_container() : callback(nullptr), surface(nullptr), graphics_family(UINT32_MAX), compute_family(UINT32_MAX), present_family(UINT32_MAX) {}
    vulkan_container::~vulkan_container() {
      device.waitIdle();
      device.destroy(transfer_command_pool);
      device.destroy(command_pool);
      device.destroy(uniform_layout);
      device.destroy(storage_layout);
      device.destroy(descriptor_pool);
      device.destroy(transfer_fence);
      buffer_allocator.destroy();
      device.destroy();
      destroyDebugCallback(instance.get(), callback);
    }
    
    swapchain::swapchain() : image_index(0), current_frame(0) {}
    surface::surface() : presentMode(vk::PresentModeKHR::eImmediate), offset{0, 0}, extent{0, 0} {}
    
    vulkan_window::vulkan_window(vk::Instance* instance, vk::Device* device, vk::PhysicalDevice* physical_device, VkSurfaceKHR surface) : 
      instance(instance), device(device), physical_device(physical_device) 
    {
      this->surface.handle = surface;
    }
    
    vulkan_window::~vulkan_window() {
      for (const auto &img : swapchain.images) {
        device->destroy(img.depth_view);
        device->destroy(img.view);
        device->destroy(img.depth);
        device->destroy(img.buffer);
      }
      
      for (const auto &frame : swapchain.frames) {
        device->destroy(frame.finish_rendering);
        device->destroy(frame.image_available);
        device->destroy(frame.fence);
      }
      
      device->destroy(swapchain.handle);
      device->free(swapchain.depth_memory);
      device->destroy(render_pass);
      device->destroy(render_pass_objects);
      vkDestroySurfaceKHR(*instance, surface.handle, nullptr);
    }
    
    vk_buffer_data::vk_buffer_data() : handle(nullptr), allocation(nullptr), ptr(nullptr) {}
    vk_buffer_data::vk_buffer_data(vk::Buffer handle, vma::Allocation allocation, void* ptr) : handle(handle), allocation(allocation), ptr(ptr) {}
    void vk_buffer_data::create(vma::Allocator allocator, const vk::BufferCreateInfo &info, const vma::MemoryUsage &mem_usage, const std::string &name) {
      if (handle) throw std::runtime_error("Buffer is already created");
      const auto [buf, mem] = create_buffer(allocator, info, mem_usage, &ptr, name);
      handle = buf;
      allocation = mem;
    }
    
    void vk_buffer_data::destroy(vma::Allocator allocator) {
      if (handle) allocator.destroyBuffer(handle, allocation);
      handle = nullptr;
      allocation = nullptr;
      ptr = nullptr;
    }
    
    vk_buffer_data_unique::vk_buffer_data_unique(vma::Allocator allocator, vk::Buffer handle, vma::Allocation allocation, void* ptr) : 
      vk_buffer_data(handle, allocation, ptr), allocator(allocator) {}
    vk_buffer_data_unique::vk_buffer_data_unique(vk_buffer_data_unique &&move) :
      vk_buffer_data(move.handle, move.allocation, move.ptr), allocator(move.allocator)
    {
      move.allocator = nullptr; move.handle = nullptr; move.allocation = nullptr; move.ptr = nullptr;
    }
    vk_buffer_data_unique::~vk_buffer_data_unique() { if (handle) destroy(allocator); allocator = nullptr; }
    vk_buffer_data_unique & vk_buffer_data_unique::operator=(vk_buffer_data_unique && move) {
      if (handle) destroy(allocator);
      allocator = move.allocator; handle = move.handle; allocation = move.allocation; ptr = move.ptr;
      move.allocator = nullptr; move.handle = nullptr; move.allocation = nullptr; move.ptr = nullptr;
      return *this;
    }
    
    vk_image_data::vk_image_data() : handle(nullptr), allocation(nullptr), ptr(nullptr) {}
    vk_image_data::vk_image_data(vk::Image handle, vma::Allocation allocation, void* ptr) : handle(handle), allocation(allocation), ptr(ptr) {}
    void vk_image_data::create(vma::Allocator allocator, const vk::ImageCreateInfo &info, const vma::MemoryUsage &mem_usage, const std::string &name) {
      if (handle) throw std::runtime_error("Buffer is already created");
      const auto [img, mem] = create_image(allocator, info, mem_usage, &ptr, name);
      handle = img;
      allocation = mem;
    }
    
    void vk_image_data::destroy(vma::Allocator allocator) {
      if (handle) allocator.destroyImage(handle, allocation);
      handle = nullptr;
      allocation = nullptr;
      ptr = nullptr;
    }
    
    vk_image_data_unique::vk_image_data_unique(vma::Allocator allocator, vk::Image handle, vma::Allocation allocation, void* ptr) : 
      vk_image_data(handle, allocation, ptr), allocator(allocator) {}
    vk_image_data_unique::vk_image_data_unique(vk_image_data_unique &&move) :
      vk_image_data(move.handle, move.allocation, move.ptr), allocator(move.allocator) 
    {
      move.allocator = nullptr; move.handle = nullptr; move.allocation = nullptr; move.ptr = nullptr;
    }
    vk_image_data_unique::~vk_image_data_unique() { if (handle) destroy(allocator); allocator = nullptr; }
    vk_image_data_unique & vk_image_data_unique::operator=(vk_image_data_unique && move) {
      if (handle) destroy(allocator);
      allocator = move.allocator; handle = move.handle; allocation = move.allocation; ptr = move.ptr;
      move.allocator = nullptr; move.handle = nullptr; move.allocation = nullptr; move.ptr = nullptr;
      return *this;
    }
    
    vk::Extent2D cast(const extent2d &val) { return { val.width, val.height }; }
    vk::Extent3D cast(const extent3d &val) { return { val.width, val.height, val.depth }; }
    vk::Offset2D cast(const offset2d &val) { return { val.x, val.y }; }
    vk::Offset3D cast(const offset3d &val) { return { val.x, val.y, val.z }; }
    vk::Rect2D cast(const rect2d &val) { return { vk::Offset2D{ val.offset.x, val.offset.x }, vk::Extent2D{ val.extent.width, val.extent.height } }; }
    vk::Viewport cast(const viewport &val) { return { val.x, val.y, val.width, val.height, val.minDepth, val.maxDepth }; }
    static_assert(sizeof(vk::ClearColorValue) == sizeof(clear_color_value));
    vk::ClearColorValue cast(const clear_color_value &val) { vk::ClearColorValue v; for (int i = 0; i < 4; ++i) { v.uint32[i] = val.uint32[i]; } return v; }
    vk::ClearDepthStencilValue cast(const clear_depth_stencil_value &val) { return { val.depth, val.stencil }; }
    static_assert(sizeof(vk::ClearValue) == sizeof(clear_value));
    vk::ClearValue cast(const clear_value &val) { vk::ClearValue v; v.color = cast(val.color); return v; }
    
    extent2d cast(const vk::Extent2D &val) { return { val.width, val.height }; }
    extent3d cast(const vk::Extent3D &val) { return { val.width, val.height, val.depth }; }
    offset2d cast(const vk::Offset2D &val) { return { val.x, val.y }; }
    offset3d cast(const vk::Offset3D &val) { return { val.x, val.y, val.z }; }
    rect2d cast(const vk::Rect2D &val) { return { offset2d{ val.offset.x, val.offset.x }, extent2d{ val.extent.width, val.extent.height } }; }
    viewport cast(const vk::Viewport &val) { return { val.x, val.y, val.width, val.height, val.minDepth, val.maxDepth }; }
    clear_color_value cast(const vk::ClearColorValue &val) { clear_color_value v; for (int i = 0; i < 4; ++i) { v.uint32[i] = val.uint32[i]; } return v; }
    clear_depth_stencil_value cast(const vk::ClearDepthStencilValue &val) { return { val.depth, val.stencil }; }
    clear_value cast(const vk::ClearValue &val) { clear_value v; v.color = cast(val.color); return v; }
    
    bool checkDeviceExtensions(vk::PhysicalDevice device, const std::vector<const char*> &layers, const std::vector<const char*> &extensions) {
      const auto &ext = device.enumerateDeviceExtensionProperties();

      phmap::flat_hash_set<std::string> intersection(extensions.begin(), extensions.end());
      for (const auto &extension : ext) {
        intersection.erase(extension.extensionName);
      }

      if (intersection.empty()) return true;

      for (auto layer : layers) {
        const auto &ext = device.enumerateDeviceExtensionProperties(std::string(layer));

        for (const auto &extension : ext) {
          intersection.erase(extension.extensionName);
        }

        if (intersection.empty()) return true;
      }

      return false;
    }
    
    std::vector<vk::LayerProperties> getRequiredValidationLayers(const std::vector<const char*> &layers) {
      const auto &availableLayers = vk::enumerateInstanceLayerProperties();

      phmap::flat_hash_set<std::string> intersection(layers.begin(), layers.end());
      std::vector<vk::LayerProperties> finalLayers;

      for (const auto &layer : availableLayers) {
        auto itr = intersection.find(std::string(layer.layerName));
        if (itr != intersection.end()) finalLayers.push_back(layer);
      }

      return finalLayers;
    }
    
    std::vector<vk::ExtensionProperties> getRequiredDeviceExtensions(vk::PhysicalDevice device, const std::vector<const char*> &layers, const std::vector<const char*> &extensions) {
      std::vector<vk::ExtensionProperties> finalExtensions;

      const auto ext = device.enumerateDeviceExtensionProperties(nullptr);

      phmap::flat_hash_set<std::string> intersection(extensions.begin(), extensions.end());

      for (const auto &extension : ext) {
        if (intersection.find(extension.extensionName) != intersection.end()) finalExtensions.push_back(extension);
      }

      for (auto layer : layers) {
        const auto ext = device.enumerateDeviceExtensionProperties(std::string(layer));

        for (const auto &extension : ext) {
          if (intersection.find(extension.extensionName) != intersection.end()) finalExtensions.push_back(extension);
        }
      }

      return finalExtensions;
    }
    
    std::tuple<uint32_t, uint32_t, uint32_t> findDeviceQueueFamiliesIndices(vk::PhysicalDevice device, VkSurfaceKHR surface) {
      const auto &queue_families = device.getQueueFamilyProperties();
      uint32_t present = UINT32_MAX, graphics = UINT32_MAX, compute = UINT32_MAX;
      
      for (size_t i = 0; i < queue_families.size(); ++i) {
        const bool is_graphics = queue_families[i].queueCount > 0 && queue_families[i].queueFlags & vk::QueueFlagBits::eGraphics;
        const bool is_compute = queue_families[i].queueCount > 0 && queue_families[i].queueFlags & vk::QueueFlagBits::eCompute;
        const bool is_present = queue_families[i].queueCount > 0 && device.getSurfaceSupportKHR(i, surface);
        graphics = is_graphics ? i : graphics;
        present = is_present ? i : present;
        compute = is_compute ? i : compute;
      }
      
      return std::make_tuple(graphics, compute, present);
    }
    
    uint32_t findDeviceQueueFamilyIndex(vk::PhysicalDevice device, const vk::QueueFlags &flags) {
      const auto &queue_families = device.getQueueFamilyProperties();
      for (size_t i = 0; i < queue_families.size(); ++i) {
        const bool valid = queue_families[i].queueCount > 0 && ((queue_families[i].queueFlags & flags) == flags);
        if (valid) return i;
      }
      
      return UINT32_MAX;
    }
    
    bool checkDeviceQueueFamilyPresentCapability(vk::PhysicalDevice device, const uint32_t &index, VkSurfaceKHR surface) {
      return index != UINT32_MAX && device.getSurfaceSupportKHR(index, surface);
    }
    
    vk::PresentModeKHR chooseSwapchainPresentMode(const std::vector<vk::PresentModeKHR> &modes) {
      for (const auto &availablePresentMode : modes) {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
          return availablePresentMode;
        }
      }

      return vk::PresentModeKHR::eFifo;
    }
    
    vk::SurfaceFormatKHR chooseSwapchainSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &formats) {
      if (formats.size() == 1 && formats[0].format == vk::Format::eUndefined) {
        return {vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear};
      }

      for (const auto &availableFormat : formats) {
        if (availableFormat.format == vk::Format::eB8G8R8A8Unorm && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
          return availableFormat;
        }
      }

      // если вдруг внезапно первые два условия провалились, то можно ранжировать доступные форматы
      // но, в принципе в большинстве случаев, подойдет и первый попавшийся
      return formats[0];
    }
    
    vk::Extent2D chooseSwapchainExtent(const uint32_t &width, const uint32_t &height, const vk::SurfaceCapabilitiesKHR& capabilities) {
      vk::Extent2D actualExtent = {(uint32_t) width, (uint32_t) height};

      actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
      actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

      return actualExtent;
    }
    
    bool checkSwapchainPresentMode(const std::vector<vk::PresentModeKHR> &modes, const vk::PresentModeKHR &mode) {
      for (const auto &availablePresentMode : modes) {
        if (mode == availablePresentMode) return true;
      }

      return false;
    }
    
    vk::Format findSupportedFormat(vk::PhysicalDevice phys, const std::vector<vk::Format> &candidates, const vk::ImageTiling &tiling, const vk::FormatFeatureFlags &features) {
      for (const auto &format : candidates) {
        vk::FormatProperties props;
        phys.getFormatProperties(format, &props);

        if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) return format;
        else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) return format;
      }

      return vk::Format::eUndefined;
    }
    
    vk::ImageCreateInfo texture2D(
      const vk::Extent2D &size, 
      const vk::ImageUsageFlags &usage, 
      const vk::Format &format, 
      const uint32_t &arrayLayers,
      const uint32_t &mipLevels,
      const vk::SampleCountFlagBits &samples,
      const vk::ImageCreateFlags &flags
    ) {
      return vk::ImageCreateInfo(
        flags, 
        vk::ImageType::e2D, 
        format, 
        {size.width, size.height, 1}, 
        mipLevels, arrayLayers,
        samples, 
        vk::ImageTiling::eOptimal,
        usage,
        vk::SharingMode::eExclusive,
        nullptr,
        vk::ImageLayout::eUndefined
      );
    }
    
    vk::ImageCreateInfo texture2D_staging(
      const vk::Extent2D &size,
      const vk::ImageUsageFlags &usage,
      const vk::Format &format,
      const vk::ImageCreateFlags &flags
    ) {
      return vk::ImageCreateInfo(
        flags, 
        vk::ImageType::e2D, 
        format, 
        {size.width, size.height, 1}, 
        1, 1,
        vk::SampleCountFlagBits::e1, 
        vk::ImageTiling::eLinear,
        usage,
        vk::SharingMode::eExclusive,
        nullptr,
        vk::ImageLayout::eUndefined
      );
    }
    
    vk::ImageViewCreateInfo make_view_info(
      vk::Image            image,
      vk::Format           format,
      vk::ImageViewType    viewType,
      vk::ImageSubresourceRange subresourceRange,
      vk::ComponentMapping components,
      vk::ImageViewCreateFlags flags
    ) {
      return vk::ImageViewCreateInfo(flags, image, viewType, format, components, subresourceRange);
    }
    
    vk::BufferCreateInfo buffer(const vk::DeviceSize &size, const vk::BufferUsageFlags &usage, const vk::BufferCreateFlags &flags) {
      return vk::BufferCreateInfo(flags, size, usage, vk::SharingMode::eExclusive, nullptr);
    }
    
    uint32_t findMemoryType(vk::PhysicalDevice physical_device, uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
      const auto memProperties = physical_device.getMemoryProperties();

      for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
        if ((typeFilter & (1 << i)) && 
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) 
          return i;
      }

      throw std::runtime_error("failed to find suitable memory type!");
    }
    
    vk::DeviceMemory create_memory(
      vk::Device device, 
      vk::PhysicalDevice physical_device,
      vk::Buffer buf, 
      const vk::DeviceSize &mem_size, 
      const vk::MemoryPropertyFlags &properties
    ) {
      const auto memRequirements = device.getBufferMemoryRequirements(buf);
      const uint32_t mem_type_index = findMemoryType(physical_device, memRequirements.memoryTypeBits, properties);
      
      const vk::MemoryAllocateInfo inf(mem_size, mem_type_index);
      return device.allocateMemory(inf);
    }
    
    vk::DeviceMemory create_memory(
      vk::Device device, 
      vk::PhysicalDevice physical_device,
      vk::Image img, 
      const vk::DeviceSize &mem_size, 
      const vk::MemoryPropertyFlags &properties
    ) {
      const auto memRequirements = device.getImageMemoryRequirements(img);
      const uint32_t mem_type_index = findMemoryType(physical_device, memRequirements.memoryTypeBits, properties);
      
      const vk::MemoryAllocateInfo inf(mem_size, mem_type_index);
      return device.allocateMemory(inf);
    }
    
    std::tuple<vk::Buffer, vk::DeviceMemory> create_buffer(
      vk::Device device, 
      vk::PhysicalDevice physical_device, 
      const vk::BufferCreateInfo &info,
      const vk::MemoryPropertyFlags &properties
    ) {
      const auto buf = device.createBuffer(info);
      const auto memRequirements = device.getBufferMemoryRequirements(buf);
      const uint32_t mem_type_index = findMemoryType(physical_device, memRequirements.memoryTypeBits, properties);
      const vk::MemoryAllocateInfo inf(memRequirements.size, mem_type_index);
      const auto mem = device.allocateMemory(inf);
      device.bindBufferMemory(buf, mem, 0);
      return std::make_tuple(buf, mem);
    }
    
    std::tuple<vk::UniqueBuffer, vk::UniqueDeviceMemory> create_buffer_unique(
      vk::Device device, 
      vk::PhysicalDevice physical_device, 
      const vk::BufferCreateInfo &info,
      const vk::MemoryPropertyFlags &properties
    ) {
      auto buf = device.createBufferUnique(info);
      const auto memRequirements = device.getBufferMemoryRequirements(buf.get());
      const uint32_t mem_type_index = findMemoryType(physical_device, memRequirements.memoryTypeBits, properties);
      const vk::MemoryAllocateInfo inf(memRequirements.size, mem_type_index);
      auto mem = device.allocateMemoryUnique(inf);
      device.bindBufferMemory(buf.get(), mem.get(), 0);
      return std::make_tuple(std::move(buf), std::move(mem));
    }
    
    std::tuple<vk::Buffer, vma::Allocation> create_buffer(
      vma::Allocator allocator, 
      const vk::BufferCreateInfo &info,
      const vma::MemoryUsage &mem_usage,
      void** pData,
      const std::string &name
    ) {
      const bool need_memory_map = mem_usage == vma::MemoryUsage::eCpuOnly || mem_usage == vma::MemoryUsage::eCpuCopy || mem_usage == vma::MemoryUsage::eCpuToGpu;
      const auto fl = need_memory_map ? vma::AllocationCreateFlagBits::eMapped : vma::AllocationCreateFlags();
      const vma::AllocationCreateInfo alloc_info(fl, mem_usage);
      std::pair<vk::Buffer, vma::Allocation> p;
      if (pData == nullptr) {
        p = allocator.createBuffer(info, alloc_info);
      } else {
        vma::AllocationInfo i;
        p = allocator.createBuffer(info, alloc_info, &i);
        *pData = i.pMappedData;
      }
      auto dev = allocator_device(allocator);
      set_name(dev, p.first, name);
      return std::make_tuple(p.first, p.second);
    }
    
    vk_buffer_data_unique create_buffer_unique(
      vma::Allocator allocator, 
      const vk::BufferCreateInfo &info,
      const vma::MemoryUsage &mem_usage,
      const std::string &name
    ) {
      const bool need_memory_map = mem_usage == vma::MemoryUsage::eCpuOnly || mem_usage == vma::MemoryUsage::eCpuCopy || mem_usage == vma::MemoryUsage::eCpuToGpu;
      const auto fl = need_memory_map ? vma::AllocationCreateFlagBits::eMapped : vma::AllocationCreateFlags();
      const vma::AllocationCreateInfo alloc_info(fl, mem_usage);
      vma::AllocationInfo i;
      const auto p = allocator.createBuffer(info, alloc_info, &i);
      auto dev = allocator_device(allocator);
      set_name(dev, p.first, name);
      return vk_buffer_data_unique(allocator, p.first, p.second, i.pMappedData);
    }
    
    std::tuple<vk::Image, vk::DeviceMemory> create_image(
      vk::Device device, 
      vk::PhysicalDevice physical_device, 
      const vk::ImageCreateInfo &info,
      const vk::MemoryPropertyFlags &properties
    ) {
      const auto img = device.createImage(info);
      const auto memRequirements = device.getImageMemoryRequirements(img);
      const uint32_t mem_type_index = findMemoryType(physical_device, memRequirements.memoryTypeBits, properties);
      const vk::MemoryAllocateInfo inf(memRequirements.size, mem_type_index);
      const auto mem = device.allocateMemory(inf);
      device.bindImageMemory(img, mem, 0);
      return std::make_tuple(img, mem);
    }
    
    std::tuple<vk::Image, vma::Allocation> create_image(
      vma::Allocator allocator, 
      const vk::ImageCreateInfo &info,
      const vma::MemoryUsage &mem_usage,
      void** pData,
      const std::string &name
    ) {
      const bool need_memory_map = mem_usage == vma::MemoryUsage::eCpuOnly || mem_usage == vma::MemoryUsage::eCpuCopy || mem_usage == vma::MemoryUsage::eCpuToGpu;
      const auto fl = need_memory_map ? vma::AllocationCreateFlagBits::eMapped : vma::AllocationCreateFlags();
      const vma::AllocationCreateInfo alloc_info(fl, mem_usage);
      std::pair<vk::Image, vma::Allocation> p;
      if (pData == nullptr) {
        p = allocator.createImage(info, alloc_info);
      } else {
        vma::AllocationInfo i;
        p = allocator.createImage(info, alloc_info, &i);
        *pData = i.pMappedData;
      }
      auto dev = allocator_device(allocator);
      set_name(dev, p.first, name);
      return std::make_tuple(p.first, p.second);
    }
    
    std::tuple<std::vector<vk::Image>, vk::DeviceMemory> create_images(
      vk::Device device, 
      vk::PhysicalDevice physical_device, 
      const vk::ImageCreateInfo &info,
      const vk::MemoryPropertyFlags &properties,
      const uint32_t &count
    ) {
      std::vector<vk::Image> imgs(count);
      for (size_t i = 0; i < imgs.size(); ++i) {
        imgs[i] = device.createImage(info);
      }
      
      const auto memRequirements = device.getImageMemoryRequirements(imgs[0]);
      const uint32_t mem_type_index = findMemoryType(physical_device, memRequirements.memoryTypeBits, properties);
      const vk::MemoryAllocateInfo inf(align_to(memRequirements.size, memRequirements.alignment) * imgs.size(), mem_type_index);
      const auto mem = device.allocateMemory(inf);
      size_t offset = 0;
      for (size_t i = 0; i < imgs.size(); ++i) {
        device.bindImageMemory(imgs[i], mem, offset);
        offset += align_to(memRequirements.size, memRequirements.alignment);
      }
      
      return std::make_tuple(std::move(imgs), mem);
    }
    
    std::tuple<vk::UniqueImage, vk::UniqueDeviceMemory> create_image_unique(
      vk::Device device, 
      vk::PhysicalDevice physical_device, 
      const vk::ImageCreateInfo &info,
      const vk::MemoryPropertyFlags &properties
    ) {
      auto img = device.createImageUnique(info);
      const auto memRequirements = device.getImageMemoryRequirements(img.get());
      const uint32_t mem_type_index = findMemoryType(physical_device, memRequirements.memoryTypeBits, properties);
      const vk::MemoryAllocateInfo inf(memRequirements.size, mem_type_index);
      auto mem = device.allocateMemoryUnique(inf);
      device.bindImageMemory(img.get(), mem.get(), 0);
      return std::make_tuple(std::move(img), std::move(mem));
    }
    
    vk_image_data_unique create_image_unique(
      vma::Allocator allocator, 
      const vk::ImageCreateInfo &info,
      const vma::MemoryUsage &mem_usage,
      const std::string &name
    ) {
      const bool need_memory_map = mem_usage == vma::MemoryUsage::eCpuOnly || mem_usage == vma::MemoryUsage::eCpuCopy || mem_usage == vma::MemoryUsage::eCpuToGpu;
      const auto fl = need_memory_map ? vma::AllocationCreateFlagBits::eMapped : vma::AllocationCreateFlags();
      const vma::AllocationCreateInfo alloc_info(fl, mem_usage);
      vma::AllocationInfo i;
      const auto p = allocator.createImage(info, alloc_info, &i);
      // то что находится в типе VmaAllocator я вижу потому что здесь находится имплементация VMA
      // мне кажется это неаккуратный хак, но полезный
      auto dev = allocator_device(allocator);
      set_name(dev, p.first, name);
      return vk_image_data_unique(allocator, p.first, p.second, i.pMappedData);
    }
    
    std::tuple<vk::AccessFlags, vk::AccessFlags, vk::PipelineStageFlags, vk::PipelineStageFlags> getBarrierData(const vk::ImageLayout &old, const vk::ImageLayout &New) {
      vk::AccessFlags srcFlags, dstFlags; 
      vk::PipelineStageFlags srcStage, dstStage;
      
      switch (old) {
        case vk::ImageLayout::eUndefined:
          srcFlags = vk::AccessFlags(0);
          srcStage = vk::PipelineStageFlagBits::eTopOfPipe;

          break;
        case vk::ImageLayout::ePreinitialized:
          srcFlags = vk::AccessFlagBits::eHostWrite;
          srcStage = vk::PipelineStageFlagBits::eHost;

          break;
        case vk::ImageLayout::eGeneral:
          srcFlags = vk::AccessFlagBits::eShaderWrite;
          srcStage = vk::PipelineStageFlagBits::eComputeShader;

          break;
        case vk::ImageLayout::eColorAttachmentOptimal:
          srcFlags = vk::AccessFlagBits::eColorAttachmentWrite;
          srcStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;

          break;
        case vk::ImageLayout::eDepthStencilAttachmentOptimal:
          srcFlags = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
          srcStage = vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;

          break;
        case vk::ImageLayout::eTransferSrcOptimal:
          srcFlags = vk::AccessFlagBits::eTransferRead;
          srcStage = vk::PipelineStageFlagBits::eTransfer;

          break;
        case vk::ImageLayout::eTransferDstOptimal:
          srcFlags = vk::AccessFlagBits::eTransferWrite;
          srcStage = vk::PipelineStageFlagBits::eTransfer;

          break;
        case vk::ImageLayout::eShaderReadOnlyOptimal:
          srcFlags = vk::AccessFlagBits::eShaderRead;
          srcStage = vk::PipelineStageFlagBits::eFragmentShader;

          break;
        case vk::ImageLayout::ePresentSrcKHR:
          srcFlags = vk::AccessFlagBits::eMemoryRead;
          srcStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;

          break;
        default: throw std::runtime_error("The layout " + vk::to_string(old) + " is not supported yet"); break;
      }

      switch (New) {
        case vk::ImageLayout::eColorAttachmentOptimal:
          dstFlags = vk::AccessFlagBits::eColorAttachmentWrite;
          dstStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;

          break;
        case vk::ImageLayout::eDepthStencilAttachmentOptimal:
          dstFlags = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
          dstStage = vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;

          break;
        case vk::ImageLayout::eTransferSrcOptimal:
          dstFlags = vk::AccessFlagBits::eTransferRead;
          dstStage = vk::PipelineStageFlagBits::eTransfer;

          break;
        case vk::ImageLayout::eTransferDstOptimal:
          dstFlags = vk::AccessFlagBits::eTransferWrite;
          dstStage = vk::PipelineStageFlagBits::eTransfer;

          break;
        case vk::ImageLayout::eShaderReadOnlyOptimal:
          if (srcFlags == vk::AccessFlags(0)) {
            srcFlags = vk::AccessFlagBits::eHostWrite | vk::AccessFlagBits::eTransferWrite;
            srcStage = vk::PipelineStageFlagBits::eTransfer | vk::PipelineStageFlagBits::eHost;
          }

          dstFlags = vk::AccessFlagBits::eShaderRead;
          dstStage = vk::PipelineStageFlagBits::eFragmentShader;

          break;
        case vk::ImageLayout::ePresentSrcKHR:
          dstFlags = vk::AccessFlagBits::eMemoryRead;
          dstStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;

          break;
        case vk::ImageLayout::eGeneral:
          dstFlags = vk::AccessFlagBits::eShaderWrite;
          dstStage = vk::PipelineStageFlagBits::eComputeShader;

          break;
        default: throw std::runtime_error("The layout " + vk::to_string(New) + " is not supported yet"); break;
      }
      
      return std::make_tuple(srcFlags, dstFlags, srcStage, dstStage);
    }
    
    std::tuple<vk::ImageMemoryBarrier, vk::PipelineStageFlags, vk::PipelineStageFlags> make_image_memory_barrier(
      vk::Image image, const vk::ImageLayout &old_layout, const vk::ImageLayout &new_layout, const vk::ImageSubresourceRange &range
    ) {
      vk::ImageMemoryBarrier b({}, {}, old_layout, new_layout, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, image, range);
      const auto [srcFlags, dstFlags, srcStage, dstStage] = getBarrierData(old_layout, new_layout);
      b.srcAccessMask = srcFlags;
      b.dstAccessMask = dstFlags;
      
      return std::make_tuple(b, srcStage, dstStage);
    }
    
    void change_image_layout(
      vk::Device device, 
      vk::Image image, 
      vk::CommandPool transfer_pool, 
      vk::Queue transfer_queue, 
      vk::Fence fence, 
      const vk::ImageLayout &old_layout, 
      const vk::ImageLayout &new_layout, 
      const vk::ImageSubresourceRange &range
    ) {
      do_command(device, transfer_pool, transfer_queue, fence, [&] (vk::CommandBuffer task) {
        const auto [b_info, srcStage, dstStage] = make_image_memory_barrier(image, old_layout, new_layout, range);
        const vk::CommandBufferBeginInfo binfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        task.begin(binfo);
        task.pipelineBarrier(srcStage, dstStage, vk::DependencyFlagBits::eByRegion, nullptr, nullptr, b_info);
        task.end();
      });
    }
    
    void do_command(
      vk::Device device, 
      vk::CommandPool pool, 
      vk::Queue queue, 
      vk::Fence fence,
      std::function<void(vk::CommandBuffer)> action
    ) {
      const vk::CommandBufferAllocateInfo alloc_info(pool, vk::CommandBufferLevel::ePrimary, 1);
      auto task = device.allocateCommandBuffers(alloc_info)[0];
      
      action(task);
      
      const vk::SubmitInfo submit(nullptr, nullptr, task, nullptr);
      queue.submit(submit, fence);
      
      const auto res = device.waitForFences(fence, VK_TRUE, SIZE_MAX);
      if (res != vk::Result::eSuccess) throw std::runtime_error("waitForFences failed");
      
      device.freeCommandBuffers(pool, task);
      device.resetFences(fence);
      
#ifndef _NDEBUG
      // вообще полезно держать вот такие глобальные указатели, другое дело что не знаю что полезного я могу тут проверить
//       if (debug_window_pointer != nullptr) {
//         assert(debug_window_pointer->swapchain.current_frame < debug_window_pointer->swapchain.images.size());
//       }
#endif
    }
    
    vk::UniqueShaderModule create_shader_module(vk::Device device, const std::string &path) {
      auto file = std::ifstream(path, std::ios::binary);
      if (!file) throw std::runtime_error("Could not load shader file " + path);
      
      file.seekg(0, std::ios::end);
      size_t length = file.tellg();
      
      std::vector<char> opcode(length);
      file.seekg(0, std::ios::beg);
      file.read(opcode.data(), opcode.size());
      
      const vk::ShaderModuleCreateInfo info({}, opcode.size(), (uint32_t*)(opcode.data()));
      return device.createShaderModuleUnique(info);
    }
    
    PFN_vkCreateDebugUtilsMessengerEXT pfnCreateDebugUtilsMessenger = nullptr;
    PFN_vkDestroyDebugUtilsMessengerEXT pfnDestroyDebugUtilsMessenger = nullptr;
    PFN_vkCmdBeginDebugUtilsLabelEXT pfnCmdBeginDebugUtilsLabelEXT = nullptr;
    PFN_vkCmdEndDebugUtilsLabelEXT pfnCmdEndDebugUtilsLabelEXT = nullptr;
    PFN_vkCmdInsertDebugUtilsLabelEXT pfnCmdInsertDebugUtilsLabelEXT = nullptr;
    PFN_vkQueueBeginDebugUtilsLabelEXT pfnQueueBeginDebugUtilsLabelEXT = nullptr;
    PFN_vkQueueEndDebugUtilsLabelEXT pfnQueueEndDebugUtilsLabelEXT = nullptr;
    PFN_vkQueueInsertDebugUtilsLabelEXT pfnQueueInsertDebugUtilsLabelEXT = nullptr;
    PFN_vkSetDebugUtilsObjectNameEXT pfnSetDebugUtilsObjectNameEXT = nullptr;
    PFN_vkSetDebugUtilsObjectTagEXT pfnSetDebugUtilsObjectTagEXT = nullptr;
    PFN_vkSubmitDebugUtilsMessageEXT pfnSubmitDebugUtilsMessageEXT = nullptr;
    
    vulkan_container* debug_container_pointer = nullptr;
    vulkan_window* debug_window_pointer = nullptr;
    
//     PFN_vkDebugMarkerSetObjectTagEXT pfnDebugMarkerSetObjectTag = nullptr;
//     PFN_vkDebugMarkerSetObjectNameEXT pfnDebugMarkerSetObjectName = nullptr;
//     PFN_vkCmdDebugMarkerBeginEXT pfnCmdDebugMarkerBegin = nullptr;
//     PFN_vkCmdDebugMarkerEndEXT pfnCmdDebugMarkerEnd = nullptr;
//     PFN_vkCmdDebugMarkerInsertEXT pfnCmdDebugMarkerInsert = nullptr;
    
    void load_debug_extensions_functions(vk::Instance instance) {
      pfnCreateDebugUtilsMessenger     = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
      pfnDestroyDebugUtilsMessenger    = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
      pfnCmdBeginDebugUtilsLabelEXT    = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkCmdBeginDebugUtilsLabelEXT");
      pfnCmdEndDebugUtilsLabelEXT      = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkCmdEndDebugUtilsLabelEXT");
      pfnCmdInsertDebugUtilsLabelEXT   = (PFN_vkCmdInsertDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkCmdInsertDebugUtilsLabelEXT");
      pfnQueueBeginDebugUtilsLabelEXT  = (PFN_vkQueueBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkQueueBeginDebugUtilsLabelEXT");
      pfnQueueEndDebugUtilsLabelEXT    = (PFN_vkQueueEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkQueueEndDebugUtilsLabelEXT");
      pfnQueueInsertDebugUtilsLabelEXT = (PFN_vkQueueInsertDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkQueueInsertDebugUtilsLabelEXT");
      pfnSetDebugUtilsObjectNameEXT    = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT");
      pfnSetDebugUtilsObjectTagEXT     = (PFN_vkSetDebugUtilsObjectTagEXT)vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectTagEXT");
      pfnSubmitDebugUtilsMessageEXT    = (PFN_vkSubmitDebugUtilsMessageEXT)vkGetInstanceProcAddr(instance, "vkSubmitDebugUtilsMessageEXT");
      
      assert(pfnCreateDebugUtilsMessenger != nullptr);
    }
    
    void load_debug_extensions_functions(vk::Device device) {
//       pfnDebugMarkerSetObjectTag = (PFN_vkDebugMarkerSetObjectTagEXT)vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectTagEXT");
//       pfnDebugMarkerSetObjectName = (PFN_vkDebugMarkerSetObjectNameEXT)vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectNameEXT");
//       pfnCmdDebugMarkerBegin = (PFN_vkCmdDebugMarkerBeginEXT)vkGetDeviceProcAddr(device, "vkCmdDebugMarkerBeginEXT");
//       pfnCmdDebugMarkerEnd = (PFN_vkCmdDebugMarkerEndEXT)vkGetDeviceProcAddr(device, "vkCmdDebugMarkerEndEXT");
//       pfnCmdDebugMarkerInsert = (PFN_vkCmdDebugMarkerInsertEXT)vkGetDeviceProcAddr(device, "vkCmdDebugMarkerInsertEXT");
      (void)device;
    }
  }
}
