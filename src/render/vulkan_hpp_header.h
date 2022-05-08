// может добавиться пару define, например эксепшоны
// вообще было бы неплохо сделать вообще все без эксепшонов
// почему мне кажется что эксепшоны лучше чем ассерты?
// ну покрайней мере потому что я могу их перехватить если вдруг мне это в итоге потребуется
#include "vulkan/vulkan.hpp"
#include "vk_mem_alloc/vk_mem_alloc.hpp"

#ifndef DEVILS_ENGINE_RENDER_VULKAN_HPP_HEADER_H
#define DEVILS_ENGINE_RENDER_VULKAN_HPP_HEADER_H

#include <functional>
#include "vulkan_declarations.h"

namespace devils_engine {
  namespace render {
    // эти структуры нужны чтобы спрятать vk неймспейс от всей остальной программы
    struct vulkan_container {
      vk::UniqueInstance instance;
      VkDebugUtilsMessengerEXT callback;
      VkSurfaceKHR surface; // удаляется в vulkan_window !!!
      vk::PhysicalDevice physical_device;
      vk::Device device;
      
      vma::Allocator buffer_allocator;
      
      vk::DescriptorPool descriptor_pool;
      vk::DescriptorSetLayout uniform_layout;
      vk::DescriptorSetLayout storage_layout;
      
      uint32_t graphics_family;
      uint32_t compute_family;
      uint32_t present_family;
      
      vk::Queue graphics;
      vk::Queue compute;
      vk::Queue present;
      
      vk::CommandPool command_pool;
      vk::CommandPool transfer_command_pool;
      //std::vector<vk::CommandBuffer> command_buffers;
      
      vk::Fence transfer_fence;
      
      vk::PhysicalDeviceLimits limits;
      
      vulkan_container();
      ~vulkan_container();
    };
    
    struct swapchain {
      // эти вещи должны быть отдельно, не помню правда почему именно
      struct frame {
        vk::Semaphore image_available;
        vk::PipelineStageFlags flags;
        vk::Semaphore finish_rendering;
        vk::Fence fence;
      };
      
      struct image {
        vk::Image handle;
        vk::Image depth;
        vk::ImageView view;
        vk::ImageView depth_view;
        vk::Framebuffer buffer;
      };
      
      vk::SwapchainKHR handle;
      vk::DeviceMemory depth_memory;
      uint32_t image_index;
      uint32_t current_frame;
      vk::Format depth_format;
      std::vector<image> images;  
      std::vector<frame> frames;
//       std::vector<vk::DescriptorSet> sets; // ???
      
      swapchain();
    };
    
    struct surface {
      vk::SurfaceKHR handle;
      vk::SurfaceCapabilitiesKHR capabilities;
      vk::SurfaceFormatKHR format;
      vk::PresentModeKHR presentMode;
      vk::Extent2D offset;
      vk::Extent2D extent;
      
      surface();
    };
    
    struct vulkan_window {
      vk::Instance* instance;
      vk::Device* device;
      vk::PhysicalDevice* physical_device;
      // дескриптор пул?
      vk::RenderPass render_pass;
      vk::RenderPass render_pass_objects;
      struct swapchain swapchain;
      struct surface surface;
      
      vulkan_window(vk::Instance* instance, vk::Device* device, vk::PhysicalDevice* physical_device, VkSurfaceKHR surface);
      ~vulkan_window();
    };
    
    struct vk_buffer_data {
      vk::Buffer handle;
      vma::Allocation allocation;
      void* ptr;
      
      vk_buffer_data();
      vk_buffer_data(vk::Buffer handle, vma::Allocation allocation, void* ptr);
      void create(vma::Allocator allocator, const vk::BufferCreateInfo &info, const vma::MemoryUsage &mem_usage, const std::string &name = "");
      void destroy(vma::Allocator allocator);
      
      vk_buffer_data(const vk_buffer_data &copy) = default;
      vk_buffer_data(vk_buffer_data &&move) = default;
      vk_buffer_data & operator=(const vk_buffer_data &copy) = default;
      vk_buffer_data & operator=(vk_buffer_data &&move) = default;
    };
    
    struct vk_buffer_data_unique : public vk_buffer_data {
      vma::Allocator allocator;
      
      vk_buffer_data_unique(vma::Allocator allocator, vk::Buffer handle, vma::Allocation allocation, void* ptr);
      vk_buffer_data_unique(vk_buffer_data_unique &&move);
      ~vk_buffer_data_unique();
      vk_buffer_data_unique & operator=(vk_buffer_data_unique && move);
      vk_buffer_data_unique(const vk_buffer_data_unique &copy) = delete;
      vk_buffer_data_unique & operator=(const vk_buffer_data_unique &copy) = delete;
    };
    
    struct vk_image_data {
      vk::Image handle;
      vma::Allocation allocation;
      void* ptr;
      
      vk_image_data();
      vk_image_data(vk::Image handle, vma::Allocation allocation, void* ptr);
      void create(vma::Allocator allocator, const vk::ImageCreateInfo &info, const vma::MemoryUsage &mem_usage, const std::string &name = "");
      void destroy(vma::Allocator allocator);
      
      vk_image_data(const vk_image_data &copy) = default;
      vk_image_data(vk_image_data &&move) = default;
      vk_image_data & operator=(const vk_image_data &copy) = default;
      vk_image_data & operator=(vk_image_data &&move) = default;
    };
    
    struct vk_image_data_unique : public vk_image_data {
      vma::Allocator allocator;
      
      vk_image_data_unique(vma::Allocator allocator, vk::Image handle, vma::Allocation allocation, void* ptr);
      vk_image_data_unique(vk_image_data_unique &&move);
      ~vk_image_data_unique();
      vk_image_data_unique & operator=(vk_image_data_unique && move);
      vk_image_data_unique(const vk_image_data_unique &copy) = delete;
      vk_image_data_unique & operator=(const vk_image_data_unique &copy) = delete;
    };
    
    void setupDebugCallback(vk::Instance instance, VkDebugUtilsMessengerEXT* callback);
    void destroyDebugCallback(vk::Instance instance, VkDebugUtilsMessengerEXT callback);
    bool checkDeviceExtensions(vk::PhysicalDevice device, const std::vector<const char*> &layers, const std::vector<const char*> &extensions);
    std::vector<vk::LayerProperties> getRequiredValidationLayers(const std::vector<const char*> &layers);
    std::vector<vk::ExtensionProperties> getRequiredDeviceExtensions(vk::PhysicalDevice device, const std::vector<const char*> &layers, const std::vector<const char*> &extensions);
    // graphics, compute, present
    std::tuple<uint32_t, uint32_t, uint32_t> findDeviceQueueFamiliesIndices(vk::PhysicalDevice device, VkSurfaceKHR surface);
    uint32_t findDeviceQueueFamilyIndex(vk::PhysicalDevice device, const vk::QueueFlags &flags);
    bool checkDeviceQueueFamilyPresentCapability(vk::PhysicalDevice device, const uint32_t &index, VkSurfaceKHR surface);
    vk::PresentModeKHR chooseSwapchainPresentMode(const std::vector<vk::PresentModeKHR> &modes);
    vk::SurfaceFormatKHR chooseSwapchainSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &formats);
    vk::Extent2D chooseSwapchainExtent(const uint32_t &width, const uint32_t &height, const vk::SurfaceCapabilitiesKHR& capabilities);
    bool checkSwapchainPresentMode(const std::vector<vk::PresentModeKHR> &modes, const vk::PresentModeKHR &mode);
    vk::Format findSupportedFormat(vk::PhysicalDevice phys, const std::vector<vk::Format> &candidates, const vk::ImageTiling &tiling, const vk::FormatFeatureFlags &features);
    
    vk::ImageCreateInfo texture2D(
      const vk::Extent2D &size, 
      const vk::ImageUsageFlags &usage, 
      const vk::Format &format = vk::Format::eR8G8B8A8Unorm, 
      const uint32_t &arrayLayers = 1,
      const uint32_t &mipLevels = 1,
      const vk::SampleCountFlagBits &samples = vk::SampleCountFlagBits::e1,
      const vk::ImageCreateFlags &flags = {}
    );
    
    vk::ImageCreateInfo texture2D_staging(
      const vk::Extent2D &size,
      const vk::ImageUsageFlags &usage = vk::ImageUsageFlagBits::eTransferSrc,
      const vk::Format &format = vk::Format::eR8G8B8A8Unorm,
      const vk::ImageCreateFlags &flags = {}
    );
    
    vk::ImageViewCreateInfo make_view_info(
      vk::Image            image,
      vk::Format           format    = vk::Format::eR8G8B8A8Unorm,
      vk::ImageViewType    viewType  = vk::ImageViewType::e2D,
      vk::ImageSubresourceRange subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1},
      vk::ComponentMapping components            = {},
      vk::ImageViewCreateFlags flags = {}
    );
    
    vk::BufferCreateInfo buffer(const vk::DeviceSize &size, const vk::BufferUsageFlags &usage, const vk::BufferCreateFlags &flags = {});
    
    vk::DeviceMemory create_memory(
      vk::Device device, 
      vk::PhysicalDevice physical_device,
      vk::Buffer buf, 
      const vk::DeviceSize &mem_size, 
      const vk::MemoryPropertyFlags &properties
    );
    
    vk::DeviceMemory create_memory(
      vk::Device device, 
      vk::PhysicalDevice physical_device,
      vk::Image img, 
      const vk::DeviceSize &mem_size, 
      const vk::MemoryPropertyFlags &properties
    );
    
    std::tuple<vk::Buffer, vk::DeviceMemory> create_buffer(
      vk::Device device, 
      vk::PhysicalDevice physical_device, 
      const vk::BufferCreateInfo &info,
      const vk::MemoryPropertyFlags &properties
    );
    
    std::tuple<vk::UniqueBuffer, vk::UniqueDeviceMemory> create_buffer_unique(
      vk::Device device, 
      vk::PhysicalDevice physical_device, 
      const vk::BufferCreateInfo &info,
      const vk::MemoryPropertyFlags &properties
    );
    
    std::tuple<vk::Buffer, vma::Allocation> create_buffer(
      vma::Allocator allocator, 
      const vk::BufferCreateInfo &info,
      const vma::MemoryUsage &mem_usage,
      void** pData = nullptr,
      const std::string &name = ""
    );
    
    vk_buffer_data_unique create_buffer_unique(
      vma::Allocator allocator, 
      const vk::BufferCreateInfo &info,
      const vma::MemoryUsage &mem_usage,
      const std::string &name = ""
    );
    
    std::tuple<vk::Image, vk::DeviceMemory> create_image(
      vk::Device device, 
      vk::PhysicalDevice physical_device, 
      const vk::ImageCreateInfo &info,
      const vk::MemoryPropertyFlags &properties
    );
    
    std::tuple<vk::Image, vma::Allocation> create_image(
      vma::Allocator allocator, 
      const vk::ImageCreateInfo &info,
      const vma::MemoryUsage &mem_usage,
      void** pData = nullptr,
      const std::string &name = ""
    );
    
    std::tuple<std::vector<vk::Image>, vk::DeviceMemory> create_images(
      vk::Device device, 
      vk::PhysicalDevice physical_device, 
      const vk::ImageCreateInfo &info,
      const vk::MemoryPropertyFlags &properties,
      const uint32_t &count
    );
    
    std::tuple<vk::UniqueImage, vk::UniqueDeviceMemory> create_image_unique(
      vk::Device device, 
      vk::PhysicalDevice physical_device, 
      const vk::ImageCreateInfo &info,
      const vk::MemoryPropertyFlags &properties
    );
    
    vk_image_data_unique create_image_unique(
      vma::Allocator allocator, 
      const vk::ImageCreateInfo &info,
      const vma::MemoryUsage &mem_usage,
      const std::string &name = ""
    );
    
    std::tuple<vk::ImageMemoryBarrier, vk::PipelineStageFlags, vk::PipelineStageFlags> make_image_memory_barrier(
      vk::Image image, const vk::ImageLayout &old_layout, const vk::ImageLayout &new_layout, const vk::ImageSubresourceRange &range
    );
    
    void change_image_layout(
      vk::Device device, 
      vk::Image image, 
      vk::CommandPool transfer_pool, 
      vk::Queue transfer_queue, 
      vk::Fence fence, 
      const vk::ImageLayout &old_layout, 
      const vk::ImageLayout &new_layout, 
      const vk::ImageSubresourceRange &range
    );
    
    void do_command(
      vk::Device device, 
      vk::CommandPool pool, 
      vk::Queue queue, 
      vk::Fence fence,
      std::function<void(vk::CommandBuffer)> action
    );
    
    vk::UniqueShaderModule create_shader_module(vk::Device device, const std::string &path);
    
    // vk_mem_alloc_impl.cpp
    vk::Device allocator_device(vma::Allocator allocator);
    vk::Instance allocator_instance(vma::Allocator allocator);
    
    vk::Extent2D cast(const extent2d &val);
    vk::Extent3D cast(const extent3d &val);
    vk::Offset2D cast(const offset2d &val);
    vk::Offset3D cast(const offset3d &val);
    vk::Rect2D cast(const rect2d &val);
    vk::Viewport cast(const viewport &val);
    vk::ClearColorValue cast(const clear_color_value &val);
    vk::ClearDepthStencilValue cast(const clear_depth_stencil_value &val);
    vk::ClearValue cast(const clear_value &val);
    
    extent2d cast(const vk::Extent2D &val);
    extent3d cast(const vk::Extent3D &val);
    offset2d cast(const vk::Offset2D &val);
    offset3d cast(const vk::Offset3D &val);
    rect2d cast(const vk::Rect2D &val);
    viewport cast(const vk::Viewport &val);
    clear_color_value cast(const vk::ClearColorValue &val);
    clear_depth_stencil_value cast(const vk::ClearDepthStencilValue &val);
    clear_value cast(const vk::ClearValue &val);
    
    extern PFN_vkCreateDebugUtilsMessengerEXT pfnCreateDebugUtilsMessenger;
    extern PFN_vkDestroyDebugUtilsMessengerEXT pfnDestroyDebugUtilsMessenger;
    extern PFN_vkCmdBeginDebugUtilsLabelEXT pfnCmdBeginDebugUtilsLabelEXT;
    extern PFN_vkCmdEndDebugUtilsLabelEXT pfnCmdEndDebugUtilsLabelEXT;
    extern PFN_vkCmdInsertDebugUtilsLabelEXT pfnCmdInsertDebugUtilsLabelEXT;
    extern PFN_vkQueueBeginDebugUtilsLabelEXT pfnQueueBeginDebugUtilsLabelEXT;
    extern PFN_vkQueueEndDebugUtilsLabelEXT pfnQueueEndDebugUtilsLabelEXT;
    extern PFN_vkQueueInsertDebugUtilsLabelEXT pfnQueueInsertDebugUtilsLabelEXT;
    extern PFN_vkSetDebugUtilsObjectNameEXT pfnSetDebugUtilsObjectNameEXT;
    extern PFN_vkSetDebugUtilsObjectTagEXT pfnSetDebugUtilsObjectTagEXT;
    extern PFN_vkSubmitDebugUtilsMessageEXT pfnSubmitDebugUtilsMessageEXT;
    
    extern vulkan_container* debug_container_pointer;
    extern vulkan_window* debug_window_pointer;
    
//     extern PFN_vkDebugMarkerSetObjectTagEXT pfnDebugMarkerSetObjectTag;
//     extern PFN_vkDebugMarkerSetObjectNameEXT pfnDebugMarkerSetObjectName;
//     extern PFN_vkCmdDebugMarkerBeginEXT pfnCmdDebugMarkerBegin;
//     extern PFN_vkCmdDebugMarkerEndEXT pfnCmdDebugMarkerEnd;
//     extern PFN_vkCmdDebugMarkerInsertEXT pfnCmdDebugMarkerInsert;
    
    void load_debug_extensions_functions(vk::Instance instance);
    void load_debug_extensions_functions(vk::Device device);
    
    template <typename T>
    void set_name(vk::Device device, T obj, const std::string &name) {
      if (pfnSetDebugUtilsObjectNameEXT == nullptr) return;
      if (name.empty()) return;
      
      const VkDebugUtilsObjectNameInfoEXT info{
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        nullptr,
        static_cast<VkObjectType>(T::objectType),
        uint64_t(typename T::CType(obj)),
        name.c_str()
      };
      
      pfnSetDebugUtilsObjectNameEXT(device, &info);
    }
  }
}

#endif
