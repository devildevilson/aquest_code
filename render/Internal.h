#ifndef YAVF_INTERNAL_H
#define YAVF_INTERNAL_H

#include <vector>
#include <vulkan/vulkan.h>

#ifndef YAVF_INFO_REPORT
#define YAVF_INFO_REPORT(msg) \
  std::cout << msg << "\n";
#endif

#ifndef YAVF_WARNING_REPORT
#define YAVF_WARNING_REPORT(msg) \
  std::cout << "[Vulkan : WARNING] " << msg << "\n";
#endif

#ifndef YAVF_ERROR_REPORT
#define YAVF_ERROR_REPORT(func, err, name)                                                                                                          \
  {                                                                                                                                                 \
    if (name != nullptr) {                                                                                                                          \
      throw std::runtime_error(std::string("[Vulkan : ERROR] ") + (func != nullptr ? func : "") + " returned " + err + " when creating " + (name != nullptr ? name : "") + "!\n");          \
    } else if (func != nullptr) throw std::runtime_error(std::string("[Vulkan : ERROR] ") + (func != nullptr ? func : "") + " returned " + err + "!\n");                                    \
    else throw std::runtime_error(std::string("[Vulkan : ERROR] ") + err);                                                                          \
  }
#endif

// if (name != nullptr) {                                                                                                                            
//   std::cout << "[Vulkan : ERROR] " << func << " returned " << err << " when creating " << (name != nullptr ? name : "") << "!\n";                 
// } else std::cout << "[Vulkan : ERROR] " << func << " returned " << err << "!\n";                                                                  
// exit(1);

namespace yavf {
  void vkCheckError(const char* vulkanFunctionName, const char* userName, const VkResult &res);
  
  std::vector<VkLayerProperties> getRequiredValidationLayers(const std::vector<const char*> &layers);

  namespace Internal {
//     struct BufferParameters {
//       VkBufferCreateFlags flags = 0;
//       uint32_t dataCount        = 0;
//       VkDeviceSize size         = 0;
//       VkDeviceSize offset       = 0;
//       VkBufferUsageFlags usage  = 0;
//     };

    struct BufferInfo {
      union {
        struct {
          VkStructureType        sType;
          const void*            pNext;
          VkBufferCreateFlags    flags;
          VkDeviceSize           size;
          VkBufferUsageFlags     usage;
          VkSharingMode          sharingMode;
          uint32_t               queueFamilyIndexCount;
          const uint32_t*        pQueueFamilyIndices;
        };
        
        VkBufferCreateInfo _info;
      };
      
      BufferInfo();
      BufferInfo(const VkBufferCreateInfo &info);
      
      BufferInfo & operator=(const VkBufferCreateInfo &info);
      BufferInfo & operator=(const BufferInfo &info);
      
      operator VkBufferCreateInfo() const;
    };
    
    // struct BufferMeta {
    //   VkBufferUsageFlags usage = 0;
    //   VkFormat format          = VK_FORMAT_UNDEFINED;
    //   void* ptr                = nullptr;
    //   VkBufferView view        = VK_NULL_HANDLE;
    //   Descriptor descriptor;
    // };

//     struct ImageParameters {
//       uint32_t              mipmaps     = 1;
//       uint32_t              layers      = 1;
//       VkImageCreateFlags    createFlags = 0;
//       VkImageType           type        = VK_IMAGE_TYPE_2D;
//       VkBufferUsageFlags    usage       = 0;
//       VkSampleCountFlagBits samples     = VK_SAMPLE_COUNT_1_BIT;
//       VkFormat              format      = VK_FORMAT_R8G8B8A8_UNORM;
//       VkImageTiling         tiling      = VK_IMAGE_TILING_OPTIMAL;
//       VkImageLayout         layout      = VK_IMAGE_LAYOUT_UNDEFINED;
//       VkExtent3D            size        = {0, 0, 0};
//     };
    
    struct ImageInfo {
      union {
        struct {
          VkStructureType          sType;
          const void*              pNext;
          VkImageCreateFlags       flags;
          VkImageType              imageType;
          VkFormat                 format;
          VkExtent3D               extent;
          uint32_t                 mipLevels;
          uint32_t                 arrayLayers;
          VkSampleCountFlagBits    samples;
          VkImageTiling            tiling;
          VkImageUsageFlags        usage;
          VkSharingMode            sharingMode;
          uint32_t                 queueFamilyIndexCount;
          const uint32_t*          pQueueFamilyIndices;
          VkImageLayout            initialLayout;
        };
        
        VkImageCreateInfo _info;
      };
      
      ImageInfo();
      ImageInfo(const VkImageCreateInfo &info);
      
      ImageInfo & operator=(const VkImageCreateInfo &info);
      ImageInfo & operator=(const ImageInfo &info);
      
      operator VkImageCreateInfo() const;
    };

    // struct ImageMeta {
    //   VkImageViewType viewType   = VK_IMAGE_VIEW_TYPE_2D;
    //   VkImageUsageFlags usage    = 0;
    //   void*           ptr        = nullptr;
    //   VkImageView     view       = VK_NULL_HANDLE;
    //   Descriptor      descriptor;
    // };

    struct VirtualFrame {
      VkFramebuffer   framebuffer                = VK_NULL_HANDLE;
      VkSemaphore     imageAvailableSemaphore    = VK_NULL_HANDLE;
      VkPipelineStageFlags flag;
      //VkSemaphore     finishedRenderingSemaphore = VK_NULL_HANDLE;
      //std::vector<VkSemaphore> imageAvailable;
      //std::vector<VkPipelineStageFlags> flags;
      //VkFence         fence                      = VK_NULL_HANDLE;
    };

    struct JobFrames {
      VkCommandBuffer buffer = VK_NULL_HANDLE;
      VkFence fence = VK_NULL_HANDLE;
    };

    struct SwapchainData {
      VkSwapchainKHR handle = VK_NULL_HANDLE;
      std::vector<VkImage> images;
      std::vector<VkImageView> imageViews;
    };

    struct SurfaceData {
      VkSurfaceCapabilitiesKHR surfaceCapabilities;
      VkSurfaceFormatKHR format;
      VkPresentModeKHR presentMode;
      VkExtent2D extent;
    };
    
    struct QueueInfo {
      uint32_t count = 0;
      VkQueueFlags flags = 0;
    };

    struct Queue {
      VkQueue handle;
      VkFence fence;
    };

    class QueueFamily {
    public:
      QueueFamily(const uint32_t &queueFamilyIndex, const uint32_t &queueCount, const VkQueueFlags &queueFlags, const VkDevice &device);
      ~QueueFamily();

      uint32_t index() const;
      VkQueueFlags flags() const;

      uint32_t size() const;

      Queue submitCommands(const uint32_t &submitCount, const VkSubmitInfo* submitInfo);
      Queue queue() const;

      void clear();
      QueueFamily & operator=(QueueFamily &other);
    private:
      VkDevice pDevice;
      uint32_t queueFamilyIndex;
      VkQueueFlags queueFlags;

      std::vector<Queue> queues;
    };
  }
}

#endif
