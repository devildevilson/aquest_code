#ifndef RENDER_VULKAN_DECLARATIONS_H
#define RENDER_VULKAN_DECLARATIONS_H

#include <cstdint>

#define VK_NULL_HANDLE nullptr
#define DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef struct object##_T *object;
DEFINE_NON_DISPATCHABLE_HANDLE(VkInstance)
DEFINE_NON_DISPATCHABLE_HANDLE(VkDevice)
DEFINE_NON_DISPATCHABLE_HANDLE(VkSurfaceKHR)
DEFINE_NON_DISPATCHABLE_HANDLE(VkBuffer)
DEFINE_NON_DISPATCHABLE_HANDLE(VkImage)
DEFINE_NON_DISPATCHABLE_HANDLE(VkImageView)
DEFINE_NON_DISPATCHABLE_HANDLE(VkDeviceMemory)

namespace vk {
  class Instance;
  class Device;
  class PhysicalDevice;
  class Image;
  class Buffer;
  class Framebuffer;
  class RenderPass;
  class DescriptorPool;
  class DescriptorSet;
  class DescriptorSetLayout;
  class CommandPool;
  class CommandBuffer;
  class Queue;
  class Fence;
  class Semaphore;
  struct PhysicalDeviceLimits;
  typedef uint64_t DeviceSize;
}

namespace devils_engine {
  namespace render {
    struct extent2d {
      uint32_t width;
      uint32_t height;
    };
    
    struct extent3d {
      uint32_t width;
      uint32_t height;
      uint32_t depth;
    };
    
    struct offset2d {
      int32_t x;
      int32_t y;
    };
    
    struct offset3d {
      int32_t x;
      int32_t y;
      int32_t z;
    };
    
    struct rect2d {
      offset2d offset;
      extent2d extent;
    };
    
    struct rect2df {
      float x;
      float y;
      float w;
      float h;
    };
    
    struct viewport {
      float x;
      float y;
      float width;
      float height;
      float minDepth;
      float maxDepth;
    };
    
    union clear_color_value {
      float    float32[4];
      int32_t  int32[4];
      uint32_t uint32[4];
    };
    
    struct clear_depth_stencil_value {
      float    depth;
      uint32_t stencil;
    };
    
    union clear_value {
      clear_color_value         color;
      clear_depth_stencil_value depthStencil;
    };
  }
}

#endif
