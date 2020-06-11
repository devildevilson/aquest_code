#include "Internal.h"

#include <iostream>
#include <string>
#include <unordered_set>

namespace yavf {
  void vkCheckError(const char* vulkanFunctionName, const char* userName, const VkResult &res) {
    if (res >= VK_SUCCESS) return;
    const char* errorString = nullptr;

    switch(res) {
      case VK_ERROR_OUT_OF_HOST_MEMORY: errorString = "VK_ERROR_OUT_OF_HOST_MEMORY"; break;
      case VK_ERROR_OUT_OF_DEVICE_MEMORY: errorString = "VK_ERROR_OUT_OF_DEVICE_MEMORY"; break;
      case VK_ERROR_INITIALIZATION_FAILED: errorString = "VK_ERROR_INITIALIZATION_FAILED"; break;
      case VK_ERROR_DEVICE_LOST: errorString = "VK_ERROR_DEVICE_LOST"; break;
      case VK_ERROR_MEMORY_MAP_FAILED: errorString = "VK_ERROR_MEMORY_MAP_FAILED"; break;
      case VK_ERROR_LAYER_NOT_PRESENT: errorString = "VK_ERROR_LAYER_NOT_PRESENT"; break;
      case VK_ERROR_EXTENSION_NOT_PRESENT: errorString = "VK_ERROR_EXTENSION_NOT_PRESENT"; break;
      case VK_ERROR_FEATURE_NOT_PRESENT: errorString = "VK_ERROR_FEATURE_NOT_PRESENT"; break;
      case VK_ERROR_INCOMPATIBLE_DRIVER: errorString = "VK_ERROR_INCOMPATIBLE_DRIVER"; break;
      case VK_ERROR_TOO_MANY_OBJECTS: errorString = "VK_ERROR_TOO_MANY_OBJECTS"; break;
      case VK_ERROR_FORMAT_NOT_SUPPORTED: errorString = "VK_ERROR_FORMAT_NOT_SUPPORTED"; break;
      case VK_ERROR_FRAGMENTED_POOL: errorString = "VK_ERROR_FRAGMENTED_POOL"; break;
      default: errorString = "unknown error"; break;
    };

    YAVF_ERROR_REPORT(vulkanFunctionName, errorString, userName)
  }
  
  std::vector<VkLayerProperties> getRequiredValidationLayers(const std::vector<const char*> &layers) {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    std::unordered_set<std::string> intersection(layers.begin(), layers.end());
    std::vector<VkLayerProperties> finalLayers;

    for (const auto &layer : availableLayers) {
      auto itr = intersection.find(std::string(layer.layerName));
      if (itr != intersection.end()) finalLayers.push_back(layer);
    }

    return finalLayers;
  }

  namespace Internal {
    BufferInfo::BufferInfo() : _info{
      VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      nullptr,
      0,
      0,
      0,
      VK_SHARING_MODE_EXCLUSIVE,
      0,
      nullptr
    } {}
    
    BufferInfo::BufferInfo(const VkBufferCreateInfo &info) : _info(info) {}
    
    BufferInfo & BufferInfo::operator=(const VkBufferCreateInfo &info) {
      _info = info;
      return *this;
    }
    
    BufferInfo & BufferInfo::operator=(const BufferInfo &info) {
      _info = info._info;
      return *this;
    }
    
    BufferInfo::operator VkBufferCreateInfo() const {
      return _info;
    }
    
    ImageInfo::ImageInfo() : _info{
    VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      nullptr,
      0,
      VK_IMAGE_TYPE_2D,
      VK_FORMAT_UNDEFINED,
      {0, 0, 0},
      1,
      1,
      VK_SAMPLE_COUNT_1_BIT,
      VK_IMAGE_TILING_LINEAR,
      0,
      VK_SHARING_MODE_EXCLUSIVE,
      1,
      nullptr,
      VK_IMAGE_LAYOUT_UNDEFINED
    } {}
    
    ImageInfo::ImageInfo(const VkImageCreateInfo &info) : _info(info) {}
    
    ImageInfo & ImageInfo::operator=(const VkImageCreateInfo &info) {
      _info = info;
      return *this;
    }
    
    ImageInfo & ImageInfo::operator=(const ImageInfo &info) {
      _info = info._info;
      return *this;
    }
    
    ImageInfo::operator VkImageCreateInfo() const {
      return _info;
    }
    
    QueueFamily::QueueFamily(const uint32_t &queueFamilyIndex, const uint32_t &queueCount, const VkQueueFlags &queueFlags, const VkDevice &device) {
      this->queueFamilyIndex = queueFamilyIndex;
      this->queueFlags = queueFlags;
      this->pDevice = device;
      
      queues.resize(queueCount);

      VkFenceCreateInfo info{
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        nullptr,
        VK_FENCE_CREATE_SIGNALED_BIT
      };
      
      for (uint32_t i = 0; i < queues.size(); ++i) {
        queues[i].handle = VK_NULL_HANDLE;
        queues[i].fence = VK_NULL_HANDLE;
        vkGetDeviceQueue(this->pDevice, this->queueFamilyIndex, i, &queues[i].handle);
        
        vkCheckError("vkCreateFence", nullptr, 
        vkCreateFence(this->pDevice, &info, nullptr, &queues[i].fence));
      }
    }

    QueueFamily::~QueueFamily() {
//       clear();
    }

    uint32_t QueueFamily::index() const {
      return queueFamilyIndex;
    }

    VkQueueFlags QueueFamily::flags() const {
      return queueFlags;
    }

    uint32_t QueueFamily::size() const {
      return queues.size();
    }

    Queue QueueFamily::submitCommands(const uint32_t &submitCount, const VkSubmitInfo* submitInfo) {
      for (size_t i = 0; i < queues.size(); ++i) {
        if (vkGetFenceStatus(pDevice, queues[i].fence) == VK_SUCCESS) {
          vkResetFences(pDevice, 1, &queues[i].fence);
          vkCheckError("vkQueueSubmit", nullptr, 
          vkQueueSubmit(queues[i].handle, submitCount, submitInfo, queues[i].fence));
          
          return queues[i];
        }
      }
      
      // что делать в этом случае? пока буду ждать первую
      vkWaitForFences(pDevice, 1, &queues[0].fence, VK_FALSE, 1000000000);
      vkResetFences(pDevice, 1, &queues[0].fence);
      
      vkCheckError("vkQueueSubmit", nullptr, 
      vkQueueSubmit(queues[0].handle, submitCount, submitInfo, queues[0].fence));
      
      return queues[0];
    }

    Queue QueueFamily::queue() const {
      for (size_t i = 0; i < queues.size(); ++i) {
        if (vkGetFenceStatus(pDevice, queues[i].fence) == VK_SUCCESS) return queues[i];
      }
      
      return queues[0];
    }

    void QueueFamily::clear() {
      for (uint32_t i = 0; i < queues.size(); ++i) {
        if (queues[i].fence != VK_NULL_HANDLE) {
          vkDestroyFence(pDevice, queues[i].fence, nullptr);
          queues[i].fence = VK_NULL_HANDLE;
        }
      }
      
      queues.clear();
    }

    QueueFamily & QueueFamily::operator=(QueueFamily &other) {
      std::cout << "Copyed!" << "\n";

      this->pDevice = other.pDevice;
      this->queueFlags = other.queueFlags;
      this->queueFamilyIndex = other.queueFamilyIndex;

      this->queues = other.queues;
      other.queues.clear();

      return *this;
    }
  }
}
