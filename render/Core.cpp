#include "Core.h"

#include <iostream>
#include <unordered_set>
#include "Internal.h"

// #define VMA_IMPLEMENTATION
// #include <vk_mem_alloc.h>

#include "Makers.h"

#include <cstring>

#ifndef _WIN32
#include <execinfo.h>
#include <signal.h>
#include <unistd.h>
//#else
#endif

PFN_vkSetDebugUtilsObjectNameEXT yavfSetDebugUtilsObjectNameEXT = nullptr;

VkResult CreateDebugReportCallbackEXT(VkInstance instance,
                                      const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator,
                                      VkDebugReportCallbackEXT* pCallback) {
  auto func = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pCallback);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void DestroyDebugReportCallbackEXT(VkInstance instance,
                                   VkDebugReportCallbackEXT callback,
                                   const VkAllocationCallbacks* pAllocator) {
  auto func = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
  if (func != nullptr) {
    func(instance, callback, pAllocator);
  }
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
                                      const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator,
                                      VkDebugUtilsMessengerEXT* pCallback) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
    VkResult res = func(instance, pCreateInfo, pAllocator, pCallback);

    yavfSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT");

    return res;
  }

  return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT callback,
                                   const VkAllocationCallbacks* pAllocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr) {
    func(instance, callback, pAllocator);
  }
}

VkBool32 debugCallback_f(VkDebugReportFlagsEXT flags,
                         VkDebugReportObjectTypeEXT objType,
                         uint64_t obj,
                         size_t location,
                         int32_t code,
                         const char* layerPrefix,
                         const char* msg,
                         void* userData) {
  (void)obj;
  (void)location;
  (void)code;
  (void)userData;
  (void)flags;

  const char* objString = nullptr;

  switch(objType) {
    case VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT:        objString = "VkInstance"; break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT: objString = "VkPhysicalDevice"; break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT:          objString = "VkDevice"; break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT:           objString = "VkQueue"; break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT:       objString = "VkSemaphore"; break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT:  objString = "VkCommandBuffer"; break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT:           objString = "VkFence"; break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT:   objString = "VkDeviceMemory"; break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT:          objString = "VkBuffer"; break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT:           objString = "VkImage"; break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT:           objString = "VkEvent"; break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT:      objString = "VkQueryPool"; break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT:     objString = "VkBufferView"; break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT:      objString = "VkImageView"; break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT:   objString = "VkShaderModule"; break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT:  objString = "VkPipelineCache"; break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT: objString = "VkPipelineLayout"; break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT:     objString = "VkRenderPass"; break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT:        objString = "VkPipeline"; break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT: objString = "VkDescriptorSetLayout"; break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT:         objString = "VkSampler"; break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT: objString = "VkDescriptorPool"; break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT:  objString = "VkDescriptorSet"; break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT:     objString = "VkFramebuffer"; break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT:    objString = "VkCommandPool"; break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT:     objString = "VkSurfaceKHR"; break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT:   objString = "VkSurfaceKHR"; break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_DEBUG_REPORT_EXT:    objString = "VkDebugReportCallbackEXT"; break;
    default: objString = "Unknown"; break;
  };

  std::cout << "[Vulkan : DEBUG]" << "\n";
  std::cout << "Layer: " << layerPrefix << "\n";
  std::cout << "Object: " << objString << "\n";
  std::cout << "Message: " << msg << "\n";

  return VK_FALSE;
}

VkBool32 utilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                VkDebugUtilsMessageTypeFlagsEXT             messageType,
                                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                void*                                       pUserData) {
  (void)pUserData;

  std::string severity;
  std::string type;

  switch(messageSeverity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: severity = "VERBOSE"; break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: severity = "INFO"; break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: severity = "WARNING"; break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: severity = "ERROR"; break;
    default: severity = "COMBINED"; break;
  }

  switch (messageType) {
    case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT: type = "GENERAL"; break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT: type = "VALIDATION"; break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: type = "PERFORMANCE"; break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT: type = "PERFORMANCE & VALIDATION"; break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT: type = "PERFORMANCE & GENERAL"; break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT: type = "VALIDATION & GENERAL"; break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
      type = "GENERAL & VALIDATION & PERFORMANCE"; break;
    default: type = "COMBINED"; break;
  }

  std::cout << "[Vulkan : DEBUG]" << "\n";
  std::cout << "Message type: " << type << " severity: " << severity << "\n";

  if (pCallbackData == nullptr) return VK_FALSE;

  if (pCallbackData->pMessageIdName != nullptr) std::cout << "Message ID: " << pCallbackData->pMessageIdName << "\n";
  if (pCallbackData->pMessage != nullptr) std::cout << "Message: " << pCallbackData->pMessage << "\n";

  if (pCallbackData->objectCount > 0) {
    for (uint32_t i = 0; i < pCallbackData->objectCount; ++i) {
      std::string objectType;

      switch(pCallbackData->pObjects[i].objectType) {
        case VK_OBJECT_TYPE_INSTANCE: objectType = "VK_OBJECT_TYPE_INSTANCE"; break;
        case VK_OBJECT_TYPE_PHYSICAL_DEVICE: objectType = "VK_OBJECT_TYPE_PHYSICAL_DEVICE"; break;
        case VK_OBJECT_TYPE_DEVICE: objectType = "VK_OBJECT_TYPE_DEVICE"; break;
        case VK_OBJECT_TYPE_QUEUE: objectType = "VK_OBJECT_TYPE_QUEUE"; break;
        case VK_OBJECT_TYPE_SEMAPHORE: objectType = "VK_OBJECT_TYPE_SEMAPHORE"; break;
        case VK_OBJECT_TYPE_COMMAND_BUFFER: objectType = "VK_OBJECT_TYPE_COMMAND_BUFFER"; break;
        case VK_OBJECT_TYPE_FENCE: objectType = "VK_OBJECT_TYPE_FENCE"; break;
        case VK_OBJECT_TYPE_DEVICE_MEMORY: objectType = "VK_OBJECT_TYPE_DEVICE_MEMORY"; break;
        case VK_OBJECT_TYPE_BUFFER: objectType = "VK_OBJECT_TYPE_BUFFER"; break;
        case VK_OBJECT_TYPE_IMAGE: objectType = "VK_OBJECT_TYPE_IMAGE"; break;
        case VK_OBJECT_TYPE_EVENT: objectType = "VK_OBJECT_TYPE_EVENT"; break;
        case VK_OBJECT_TYPE_QUERY_POOL: objectType = "VK_OBJECT_TYPE_QUERY_POOL"; break;
        case VK_OBJECT_TYPE_BUFFER_VIEW: objectType = "VK_OBJECT_TYPE_BUFFER_VIEW"; break;
        case VK_OBJECT_TYPE_IMAGE_VIEW: objectType = "VK_OBJECT_TYPE_IMAGE_VIEW"; break;
        case VK_OBJECT_TYPE_SHADER_MODULE: objectType = "VK_OBJECT_TYPE_SHADER_MODULE"; break;
        case VK_OBJECT_TYPE_PIPELINE_CACHE: objectType = "VK_OBJECT_TYPE_PIPELINE_CACHE"; break;
        case VK_OBJECT_TYPE_PIPELINE_LAYOUT: objectType = "VK_OBJECT_TYPE_PIPELINE_LAYOUT"; break;
        case VK_OBJECT_TYPE_RENDER_PASS: objectType = "VK_OBJECT_TYPE_RENDER_PASS"; break;
        case VK_OBJECT_TYPE_PIPELINE: objectType = "VK_OBJECT_TYPE_PIPELINE"; break;
        case VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT: objectType = "VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT"; break;
        case VK_OBJECT_TYPE_SAMPLER: objectType = "VK_OBJECT_TYPE_SAMPLER"; break;
        case VK_OBJECT_TYPE_DESCRIPTOR_POOL: objectType = "VK_OBJECT_TYPE_DESCRIPTOR_POOL"; break;
        case VK_OBJECT_TYPE_DESCRIPTOR_SET: objectType = "VK_OBJECT_TYPE_DESCRIPTOR_SET"; break;
        case VK_OBJECT_TYPE_FRAMEBUFFER: objectType = "VK_OBJECT_TYPE_FRAMEBUFFER"; break;
        case VK_OBJECT_TYPE_COMMAND_POOL: objectType = "VK_OBJECT_TYPE_COMMAND_POOL"; break;
        case VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION: objectType = "VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION"; break;
        case VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE: objectType = "VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE"; break;
        case VK_OBJECT_TYPE_SURFACE_KHR: objectType = "VK_OBJECT_TYPE_SURFACE_KHR"; break;
        case VK_OBJECT_TYPE_SWAPCHAIN_KHR: objectType = "VK_OBJECT_TYPE_SWAPCHAIN_KHR"; break;
        case VK_OBJECT_TYPE_DISPLAY_KHR: objectType = "VK_OBJECT_TYPE_DISPLAY_KHR"; break;
        case VK_OBJECT_TYPE_DISPLAY_MODE_KHR: objectType = "VK_OBJECT_TYPE_DISPLAY_MODE_KHR"; break;
        case VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT: objectType = "VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT"; break;
//         case VK_OBJECT_TYPE_OBJECT_TABLE_NVX: objectType = "VK_OBJECT_TYPE_OBJECT_TABLE_NVX"; break;
        case VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV: objectType = "VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV"; break;
        case VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT: objectType = "VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT"; break;
        case VK_OBJECT_TYPE_VALIDATION_CACHE_EXT: objectType = "VK_OBJECT_TYPE_VALIDATION_CACHE_EXT"; break;
        case VK_OBJECT_TYPE_UNKNOWN:
        default: objectType = "VK_OBJECT_TYPE_UNKNOWN"; break;
      }

      std::cout << "Object type: " << objectType << " handle: " << (void*)pCallbackData->pObjects[i].objectHandle << "\n";
      if (pCallbackData->pObjects[i].pObjectName != nullptr) std::cout << "Object name: " << pCallbackData->pObjects[i].pObjectName << '\n';
    }
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
    void *array[200];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 200);
    // print out all the frames to stderr
//     fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);

    throw std::runtime_error("Vulkan error");
  }

  return VK_FALSE;
}

bool checkValidationLayers(const std::vector<const char*> &layers) {
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  std::unordered_set<std::string> intersection(layers.begin(), layers.end());
  std::vector<VkLayerProperties> finalLayers;

  for (const auto &layer : availableLayers) {
    intersection.erase(layer.layerName);
  }

  return intersection.empty();
}

bool checkInstanceExtencions(const std::vector<const char*> &layers, const std::vector<const char*> &extensions) {
  std::unordered_set<std::string> intersection(extensions.begin(), extensions.end());

  uint32_t count = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
  std::vector<VkExtensionProperties> avaiableExtensions(count);
  vkEnumerateInstanceExtensionProperties(nullptr, &count, avaiableExtensions.data());

  for (const auto &extension : avaiableExtensions) {
    intersection.erase(extension.extensionName);
  }

  if (intersection.empty()) return true;

  for (auto layer : layers) {
    uint32_t count = 0;
    vkEnumerateInstanceExtensionProperties(layer, &count, nullptr);
    std::vector<VkExtensionProperties> avaiableExtensions(count);
    vkEnumerateInstanceExtensionProperties(layer, &count, avaiableExtensions.data());

    for (const auto &extension : avaiableExtensions) {
      intersection.erase(extension.extensionName);
    }

    if (intersection.empty()) return true;
  }

  return false;
}

std::vector<VkExtensionProperties> getRequiredExtensions(const std::vector<const char*> &layers, const std::vector<const char*> &extensions, const bool addDebugExtension) {
  std::unordered_set<std::string> intersection(extensions.begin(), extensions.end());

  std::vector<VkExtensionProperties> finalExtensions;

  uint32_t count = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
  std::vector<VkExtensionProperties> avaiableExtensions(count);
  vkEnumerateInstanceExtensionProperties(nullptr, &count, avaiableExtensions.data());

  std::string debugUtils = "VK_EXT_debug_utils";

  if (addDebugExtension) {
    intersection.insert(debugUtils);
  }

  for (const auto &extension : avaiableExtensions) {
    auto itr = intersection.find(std::string(extension.extensionName));
    if (itr != intersection.end()) finalExtensions.push_back(extension);
  }

  for (auto layer : layers) {
    uint32_t count = 0;
    vkEnumerateInstanceExtensionProperties(layer, &count, nullptr);
    std::vector<VkExtensionProperties> avaiableExtensions(count);
    vkEnumerateInstanceExtensionProperties(layer, &count, avaiableExtensions.data());

    for (const auto &extension : avaiableExtensions) {
      auto itr = intersection.find(std::string(extension.extensionName));
      if (itr != intersection.end()) finalExtensions.push_back(extension);
    }
  }

  return finalExtensions;
}

VkResult createSwapChain(const yavf::Internal::SurfaceData &surfaceData,
                         const VkSurfaceKHR surface,
                         yavf::Device* device,
                         //yavf::Internal::SwapchainData &swapchainData
                         yavf::Swapchain &swapchain,
                         std::vector<yavf::Image*> &swapchainImages) {
  uint32_t imageCount = surfaceData.surfaceCapabilities.minImageCount + 1;
  if (surfaceData.surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceData.surfaceCapabilities.maxImageCount) {
    imageCount = surfaceData.surfaceCapabilities.maxImageCount;
  }

  yavf::Swapchain old = swapchain;

  const VkSwapchainCreateInfoKHR info{
    VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    nullptr,
    0,
    surface,
    imageCount,
    surfaceData.format.format,
    surfaceData.format.colorSpace,
    surfaceData.extent,
    1,
    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
    VK_SHARING_MODE_EXCLUSIVE,
    0,
    nullptr,
    surfaceData.surfaceCapabilities.currentTransform,
    VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    surfaceData.presentMode,
    VK_TRUE,
    old
  };

  swapchain = device->recreate(info, "swapchain_for_simplewindow");

  //VkResult res = vkCreateSwapchainKHR(device, &info, nullptr, &swapchainData.handle);
  //if (res != VK_SUCCESS) return res;

//   if (old != VK_NULL_HANDLE) {
//     //vkDestroySwapchainKHR(device, old, nullptr);
//     device->destroy(old);
//   }

  swapchainImages = device->getSwapchainImages(swapchain);

//   vkGetSwapchainImagesKHR(device, swapchainData.handle, &imageCount, nullptr);
//   swapchainData.images.resize(imageCount);
//   vkGetSwapchainImagesKHR(device, swapchainData.handle, &imageCount, swapchainData.images.data());

  return VK_SUCCESS;
}

// void getBarrierData(const VkImageLayout &old, const VkImageLayout &New, VkAccessFlags &srcFlags, VkAccessFlags &dstFlags, VkPipelineStageFlags &srcStage, VkPipelineStageFlags &dstStage) {
//   switch (old) {
//     case VK_IMAGE_LAYOUT_UNDEFINED:
//       srcFlags = 0;
//       srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
//
//       break;
//     case VK_IMAGE_LAYOUT_PREINITIALIZED:
//       srcFlags = VK_ACCESS_HOST_WRITE_BIT;
//       srcStage = VK_PIPELINE_STAGE_HOST_BIT;
//
//       break;
//     case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
//       srcFlags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//       srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//
//       break;
//     case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
//       srcFlags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
//       srcStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
//
//       break;
//     case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
//       srcFlags = VK_ACCESS_TRANSFER_READ_BIT;
//       srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
//
//       break;
//     case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
//       srcFlags = VK_ACCESS_TRANSFER_WRITE_BIT;
//       srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
//
//       break;
//     case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
//       srcFlags = VK_ACCESS_SHADER_READ_BIT;
//       srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
//
//       break;
//     case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
//       srcFlags = VK_ACCESS_MEMORY_READ_BIT;
//       srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//
//       break;
//     default:
//       YAVF_WARNING_REPORT("This layout is not supported yet")
//       break;
//   }
//
//   switch (New) {
//     case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
//       dstFlags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//       dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//
//       break;
//     case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
//       dstFlags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
//       dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
//
//       break;
//     case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
//       dstFlags = VK_ACCESS_TRANSFER_READ_BIT;
//       dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
//
//       break;
//     case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
//       dstFlags = VK_ACCESS_TRANSFER_WRITE_BIT;
//       dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
//
//       break;
//     case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
//       if (srcFlags == 0) srcFlags = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
//       dstFlags = VK_ACCESS_SHADER_READ_BIT;
//
//       srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_HOST_BIT;
//       dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
//
//       break;
//     case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
//       dstFlags = VK_ACCESS_MEMORY_READ_BIT;
//       dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//
//       break;
//     default:
//       YAVF_WARNING_REPORT("This layout is not supported yet")
//       break;
//   }
// }

namespace yavf {
  bool checkDeviceExtensions(VkPhysicalDevice device, const std::vector<const char*> &layers, const std::vector<const char*> &extensions) {
    uint32_t count = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> ext(count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, ext.data());

    std::unordered_set<std::string> intersection(extensions.begin(), extensions.end());
    for (const auto &extension : ext) {
      intersection.erase(extension.extensionName);
    }

    if (intersection.empty()) return true;

    for (auto layer : layers) {
      uint32_t count = 0;
      vkEnumerateDeviceExtensionProperties(device, layer, &count, nullptr);
      std::vector<VkExtensionProperties> ext(count);
      vkEnumerateDeviceExtensionProperties(device, layer, &count, ext.data());

      for (const auto &extension : ext) {
        intersection.erase(extension.extensionName);
      }

      if (intersection.empty()) return true;
    }

    return false;
  }

  std::vector<VkExtensionProperties> getRequiredDeviceExtensions(VkPhysicalDevice device, const std::vector<const char*> &layers, const std::vector<const char*> &extensions) {
    std::vector<VkExtensionProperties> finalExtensions;

    uint32_t count = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> ext(count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, ext.data());

    std::unordered_set<std::string> intersection(extensions.begin(), extensions.end());

    for (const auto &extension : ext) {
      if (intersection.find(extension.extensionName) != intersection.end()) finalExtensions.push_back(extension);
    }

    for (auto layer : layers) {
      uint32_t count = 0;
      vkEnumerateDeviceExtensionProperties(device, layer, &count, nullptr);
      std::vector<VkExtensionProperties> ext(count);
      vkEnumerateDeviceExtensionProperties(device, layer, &count, ext.data());

      for (const auto &extension : ext) {
        if (intersection.find(extension.extensionName) != intersection.end()) finalExtensions.push_back(extension);
      }
    }

    return finalExtensions;
  }

  VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &formats) {
    if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
      return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    }

    for (const auto &availableFormat : formats) {
      if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
        return availableFormat;
      }
    }

    // если вдруг внезапно первые два условия провалились, то можно ранжировать доступные форматы
    // но, в принципе в большинстве случаев, подойдет и первый попавшийся
    return formats[0];
  }

  size_t alignMemorySize(const size_t &alignment, const size_t &size) {
    return size_t((size - 1) / alignment + 1) * alignment;
  }

  VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &presentModes) {
    for (const auto &availablePresentMode : presentModes) {
      if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
        return availablePresentMode;
      }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
  }

  bool checkSwapchainPresentMode(const std::vector<VkPresentModeKHR> &presentModes, const VkPresentModeKHR &mode) {
    for (const auto &availablePresentMode : presentModes) {
      if (mode == availablePresentMode) {
        return true;
      }
    }

    return false;
  }

  VkExtent2D chooseSwapchainExtent(const uint32_t &width, const uint32_t &height, const VkSurfaceCapabilitiesKHR& capabilities) {
//     if (capabilities.currentExtent.width != UINT32_MAX) return capabilities.currentExtent;

    VkExtent2D actualExtent = {(uint32_t) width, (uint32_t) height};

    actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

    return actualExtent;
  }

  VkFormat findSupportedFormat(VkPhysicalDevice phys, const std::vector<VkFormat> &candidates, const VkImageTiling &tiling, const VkFormatFeatureFlags &features) {
    for (VkFormat format : candidates) {
      VkFormatProperties props;
      vkGetPhysicalDeviceFormatProperties(phys, format, &props);

      if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) return format;
      else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) return format;
    }

    return VK_FORMAT_UNDEFINED;
  }

//   ImageCreateInfo::ImageCreateInfo(VkImageCreateFlags flags,
//                                    VkImageType imageType,
//                                    VkFormat format,
//                                    VkExtent3D extent,
//                                    uint32_t mipLevels,
//                                    uint32_t arrayLayers,
//                                    VkSampleCountFlagBits samples,
//                                    VkImageTiling tiling,
//                                    VkImageUsageFlags usage,
//                                    VkImageAspectFlags aspect,
//                                    VmaMemoryUsage memoryUsage) {
//     this->flags = flags;
//     this->imageType = imageType;
//     this->format = format;
//     this->extent = extent;
//     this->mipLevels = mipLevels;
//     this->arrayLayers = arrayLayers;
//     this->samples = samples;
//     this->tiling = tiling;
//     this->usage = usage;
//     this->aspect = aspect;
//     this->memoryUsage = memoryUsage;
//   }

//   BufferCreateInfo::BufferCreateInfo(VkBufferCreateFlags flags,
//                                      VkDeviceSize size,
//                                      VkBufferUsageFlags usage,
//                                      uint32_t dataCount,
//                                      VmaMemoryUsage memoryUsage) {
//     this->flags = flags;
//     this->size = size;
//     this->usage = usage;
//     this->dataCount = dataCount;
//     this->memoryUsage = memoryUsage;
//   }

  SimpleRenderTarget::SimpleRenderTarget() : buffer(VK_NULL_HANDLE), pass(VK_NULL_HANDLE) {}
  SimpleRenderTarget::SimpleRenderTarget(const CreateInfo &info) : frameSize(info.frameSize), buffer(info.buffer), pass(info.pass) {}
  SimpleRenderTarget::~SimpleRenderTarget() {}

  std::vector<VkClearValue> SimpleRenderTarget::clearValues() const {
    std::vector<VkClearValue> values(2);
    values[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    // values[0].depthStencil = {1.0f, 0};
    // values[1].color = {0.0f, 0.0f, 0.0f, 1.0f};
    values[1].depthStencil = {1.0f, 0};

    return values;
  }

  VkRect2D SimpleRenderTarget::size() const  {
    return frameSize;
  }

  RenderPass SimpleRenderTarget::renderPass() const  {
    return pass;
  }

  VkViewport SimpleRenderTarget::viewport() const  {
    return {
      0.0f, 0.0f,
      static_cast<float>(frameSize.extent.width),
      static_cast<float>(frameSize.extent.height),
      0.0f, 1.0f
    };
  }

  VkRect2D SimpleRenderTarget::scissor() const  {
    return frameSize;
  }

  Framebuffer SimpleRenderTarget::framebuffer() const  {
    return buffer;
  }

  void SimpleRenderTarget::setFramebuffer(const Framebuffer &f) {
    buffer = f;
  }

  void SimpleRenderTarget::setSize(const VkRect2D &size) {
    frameSize = size;
  }

//   void SimpleRenderTarget::resize(const VkRect2D &newSize, const std::vector<VkImageView> &views) {
//
//   }

  SimpleWindow::SimpleWindow(Instance* instance, Device* device, VkSurfaceKHR surface, const uint32_t &width, const uint32_t &height) {
    this->device = device;
    this->instance = instance;
    // this->renderPassHandle = renderPass;
    this->surface = surface;

    for (size_t i = 0; i < device->getFamiliesCount(); ++i){
      VkBool32 present;
      vkGetPhysicalDeviceSurfaceSupportKHR(device->physicalHandle(), i, surface, &present);

      if (present == VK_TRUE) {
        family = i;
        break;
      }
    }

    if (family == UINT32_MAX) return;

    if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->physicalHandle(), surface, &surfaceData.surfaceCapabilities) != VK_SUCCESS) return;

    uint32_t count = 0;
    if (vkGetPhysicalDeviceSurfaceFormatsKHR(device->physicalHandle(), surface, &count, nullptr) != VK_SUCCESS) return;

    std::vector<VkSurfaceFormatKHR> formats(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device->physicalHandle(), surface, &count, formats.data());

    if (vkGetPhysicalDeviceSurfacePresentModesKHR(device->physicalHandle(), surface, &count, nullptr) != VK_SUCCESS) return;

    std::vector<VkPresentModeKHR> presentModes(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device->physicalHandle(), surface, &count, presentModes.data());

    surfaceData.format = chooseSwapSurfaceFormat(formats);
    surfaceData.presentMode = chooseSwapPresentMode(presentModes);
    surfaceData.extent = chooseSwapchainExtent(width, height, surfaceData.surfaceCapabilities);

    if (createSwapChain(surfaceData, surface, device, swapchain, swapchainImages) != VK_SUCCESS) return;

//     swapchain.imageViews.resize(swapchain.images.size(), VK_NULL_HANDLE);
//     for (size_t i = 0; i < swapchain.imageViews.size(); ++i) {
//       const VkImageViewCreateInfo info{
//         VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
//         nullptr,
//         0,
//         swapchain.images[i],
//         VK_IMAGE_VIEW_TYPE_2D,
//         surfaceData.format.format,
//         YAVF_DEFAULT_SWIZZLE,
//         {
//           VK_IMAGE_ASPECT_COLOR_BIT,
//           0, 1, 0, 1
//         }
//       };
//
//       if (vkCreateImageView(device->handle(), &info, nullptr, &swapchain.imageViews[i]) != VK_SUCCESS) {
//         for (size_t i = 0; i < swapchain.imageViews.size(); ++i) {
//           if (swapchain.imageViews[i] != VK_NULL_HANDLE) {
//             vkDestroyImageView(device->handle(), swapchain.imageViews[i], nullptr);
//           }
//         }
//
//         vkDestroySwapchainKHR(device->handle(), swapchain.handle, nullptr);
//         return;
//       }
//     }

    for (size_t i = 0; i < swapchainImages.size(); ++i) {
      swapchainImages[i]->createView(VK_IMAGE_VIEW_TYPE_2D, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}, surfaceData.format.format);
    }

    VkFormat depth = findSupportedFormat(device->physicalHandle(),
                                        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                                         VK_IMAGE_TILING_OPTIMAL,
                                         VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

//     const yavf::ImageCreateInfo info{
//       0,
//       VK_IMAGE_TYPE_2D,
//       depth,
//       {surfaceData.extent.width, surfaceData.extent.height, 1},
//       1, 1,
//       VK_SAMPLE_COUNT_1_BIT,
//       VK_IMAGE_TILING_OPTIMAL,
//       VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
//       VK_IMAGE_ASPECT_DEPTH_BIT,
//       VMA_MEMORY_USAGE_GPU_ONLY
//     };

    depthImage = device->create(ImageCreateInfo::texture2D(
      {surfaceData.extent.width, surfaceData.extent.height},
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      depth
    ), VMA_MEMORY_USAGE_GPU_ONLY);

    depthImage->createView(VK_IMAGE_VIEW_TYPE_2D, {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1});

    TransferTask* task = device->allocateTransferTask();

    task->begin();
    task->setBarrier(depthImage, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    task->end();

    task->start();
    task->wait();

    device->deallocate(task);

    createRenderPass();

//     VkSemaphoreCreateInfo semaforeInfo{
//       VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
//       nullptr,
//       0
//     };

    // VkFenceCreateInfo fenceInfo{
    //   VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    //   nullptr,
    //   VK_FENCE_CREATE_SIGNALED_BIT
    // };

//     waitSemaphore = device->createSemaphoreProxy();

    frames.resize(swapchainImages.size());
    framebuffers.resize(swapchainImages.size());
    for (size_t i = 0; i < swapchainImages.size(); ++i) {
//       vkCheckError("vkCreateSemaphore", nullptr,
//       vkCreateSemaphore(device->handle(), &semaforeInfo, nullptr, &frames[i].imageAvailableSemaphore));
//       vkCheckError("vkCreateSemaphore", nullptr,
//       vkCreateSemaphore(device->handle(), &semaforeInfo, nullptr, &frames[i].finishedRenderingSemaphore));
      // vkCreateFence(device->handle(), &fenceInfo, nullptr, &frames[i].fence);

      frames[i].imageAvailable  = device->createSemaphore();
      frames[i].finishRendering = device->createSemaphore();
//       frames[i].flag = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

      const std::vector<VkImageView> attachments = {
        swapchainImages[i]->view(),
        depthImage->view()
      };

      const VkFramebufferCreateInfo frameInfo{
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        nullptr,
        0,
        renderPassHandle,
        static_cast<uint32_t>(attachments.size()),
        attachments.data(),
        surfaceData.extent.width,
        surfaceData.extent.height,
        1
      };
      Framebuffer fb = device->create(frameInfo, "framebuffer_for_simplewindow_" + std::to_string(i));

      const SimpleRenderTarget::CreateInfo info{
        {{0, 0}, surfaceData.extent},
        fb,
        renderPassHandle
      };
      framebuffers[i] = SimpleRenderTarget(info);
    }

    valid = true;
  }

  SimpleWindow::~SimpleWindow() {
    for (size_t i = 0; i < frames.size(); ++i) {
      //vkDestroySemaphore(device->handle(), frames[i].imageAvailableSemaphore, nullptr);
      //vkDestroySemaphore(device->handle(), frames[i].finishedRenderingSemaphore, nullptr);

      device->destroy(frames[i].imageAvailable);
      device->destroy(frames[i].finishRendering);

      // vkDestroyFence(device->handle(), frames[i].fence, nullptr);
//       if (frames[i].framebuffer != VK_NULL_HANDLE) {
//         vkDestroyFramebuffer(device->handle(), frames[i].framebuffer, nullptr);
//       }
    }

    for (size_t i = 0; i < framebuffers.size(); ++i) {
      device->destroy(framebuffers[i].framebuffer());
    }

    device->destroy(depthImage);
    device->destroy(renderPassHandle);

//     device->destroySemaphoreProxy(waitSemaphore);

//     for (size_t i = 0; i < swapchain.imageViews.size(); ++i) {
//       if (swapchain.imageViews[i] != VK_NULL_HANDLE) {
//         vkDestroyImageView(device->handle(), swapchain.imageViews[i], nullptr);
//       }
//     }

//     vkDestroySwapchainKHR(device->handle(), swapchain.handle, nullptr);

    device->destroy(swapchain);

    vkDestroySurfaceKHR(instance->handle(), surface, nullptr);
  }

  uint32_t SimpleWindow::framesCount() const {
    return swapchainImages.size();
  }

  VirtualFrame & SimpleWindow::operator[](const uint32_t &index) {
    return frames[index];
  }

  const VirtualFrame & SimpleWindow::operator[](const uint32_t &index) const {
    return frames[index];
  }

  VkResult SimpleWindow::nextFrame() {
    currentVirtualFrame = (currentVirtualFrame + 1) % frames.size();

    VkResult res = swapchain.acquireNextImage(UINT64_MAX, frames[currentVirtualFrame].imageAvailable, VK_NULL_HANDLE, &imageIndex);
//     VkResult res = vkAcquireNextImageKHR(device->handle(),
//                                  swapchain.handle(),
//                                  UINT64_MAX,
//                                  frames[currentVirtualFrame].imageAvailable,
//                                  VK_NULL_HANDLE,
//                                  &imageIndex);

//     waitSemaphore->changeSemaphore(frames[currentFrame].imageAvailableSemaphore, frames[currentFrame].flag);
//     updateFramebuffer();

    return res;
  }

  uint32_t SimpleWindow::currentFrame() const {
    return currentVirtualFrame;
  }

  uint32_t SimpleWindow::currentImage() const {
    return imageIndex;
  }

  RenderTarget* SimpleWindow::currentRenderTarget() {
    return &framebuffers[imageIndex];
  }

//   std::vector<VkClearValue> SimpleWindow::clearValues() const {
//     std::vector<VkClearValue> values(2);
//     values[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
//     // values[0].depthStencil = {1.0f, 0};
//     // values[1].color = {0.0f, 0.0f, 0.0f, 1.0f};
//     values[1].depthStencil = {1.0f, 0};
//
//     return values;
//   }
//
// //   VkImage Window::image() const {
// //     return swapchain.images[currentFrame];
// //   }
//
//   VkRect2D SimpleWindow::size() const {
//     const VkRect2D sizeR{
//       {0, 0},
//       surfaceData.extent
//     };
//
//     return sizeR;
//   }
//
//   VkRenderPass SimpleWindow::renderPass() const {
//     return renderPassHandle;
//   }
//
//   VkViewport SimpleWindow::viewport() const {
//     const VkViewport view{
//       0.0f, 0.0f,
//       static_cast<float>(surfaceData.extent.width),
//       static_cast<float>(surfaceData.extent.height),
//       0.0f, 1.0f
//     };
//
//     return view;
//   }
//
//   VkRect2D SimpleWindow::scissor() const {
//     const VkRect2D sizeR{
//       {0, 0},
//       surfaceData.extent
//     };
//
//     return sizeR;
//   }
//
//   VkFramebuffer SimpleWindow::framebuffer() const {
//     return frames[currentFrame].framebuffer;
//   }
//
// //   SemaphoreProxy* Window::createSemaphoreProxy(const VkPipelineStageFlags &flag) {
// //     SemaphoreOwner* so = device->createSemaphoreProxy();
// //     waitSemaphores.push_back(so);
// //     for (uint32_t i = 0; i < frames.size(); ++i) {
// //       VkSemaphore s = device->createSemaphore();
// //       frames[i].imageAvailable.push_back(s);
// //       frames[i].flags.push_back(flag);
// //     }
// //
// //     return so;
// //   }
//
//   SemaphoreProxy* SimpleWindow::getSemaphoreProxy() const {
//     return waitSemaphore;
//   }
//
//   void SimpleWindow::addSemaphoreProxy(SemaphoreProxy* proxy) {
//     signalSemaphores.push_back(proxy);
//   }

//   VkSemaphore Window::imageAvailable() const {
//     return frames[currentFrame].imageAvailableSemaphore;
//   }
//
//   VkSemaphore Window::finishRendering() const {
//     return frames[currentFrame].finishedRenderingSemaphore;
//   }

//   void SimpleWindow::updateFramebuffer() {
//     if(frames[currentFrame].framebuffer != VK_NULL_HANDLE) {
//       vkDestroyFramebuffer(device->handle(), frames[currentFrame].framebuffer, nullptr);
//     }
//
//     const std::array<VkImageView, 2> attachments = {
//       swapchain.imageViews[imageIndex],
//       depthImage->view().handle
//     };
//
//     const VkFramebufferCreateInfo framebufferCreateInfo{
//       VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
//       nullptr,
//       0,
//       renderPassHandle,
//       attachments.size(),
//       attachments.data(),
//       surfaceData.extent.width,
//       surfaceData.extent.height,
//       1
//     };
//
//     vkCheckError("vkCreateFramebuffer", nullptr,
//     vkCreateFramebuffer(device->handle(), &framebufferCreateInfo, nullptr, &frames[currentFrame].framebuffer));
//   }

  VkResult SimpleWindow::present(const Internal::Queue &queue) {
//     VkSemaphore s[signalSemaphores.size()];
//     for (uint32_t i = 0; i < signalSemaphores.size(); ++i) {
//       s[i] = signalSemaphores[i]->get();
//       //std::cout << "Present semaphore " << s[i] << '\n';
//     }

//     const VkPresentInfoKHR info{
//       VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
//       nullptr,
//       1,
//       &frames[currentFrame].finishedRenderingSemaphore,
//       1,
//       &swapchain.handle,
//       &imageIndex,
//       nullptr
//     };

    const VkSwapchainKHR s = swapchain.handle();

    const VkPresentInfoKHR info{
      VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      nullptr,
      //static_cast<uint32_t>(signalSemaphores.size()),
      1,
      &frames[currentVirtualFrame].finishRendering,
      1,
      &s,
      &imageIndex,
      nullptr
    };

    // auto queue = device->getQueue(family);

    // if (vkWaitForFences(device->handle(), 1, &queue.fence, VK_FALSE, 0) != VK_SUCCESS) {
    //   vkWaitForFences(device->handle(), 1, &queue.fence, VK_FALSE, 1000000000);
    // }

    return vkQueuePresentKHR(queue.handle, &info);
  }

  VkResult SimpleWindow::present() {
//     VkSemaphore s[signalSemaphores.size()];
//     for (uint32_t i = 0; i < signalSemaphores.size(); ++i) s[i] = signalSemaphores[i]->get();

    const VkSwapchainKHR s = swapchain.handle();

    const VkPresentInfoKHR info{
      VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      nullptr,
      1,
      &frames[currentVirtualFrame].finishRendering,
      1,
      &s,
      &imageIndex,
      nullptr
    };

    auto queue = device->getQueue(family);

    // if (vkWaitForFences(device->handle(), 1, &queue.fence, VK_FALSE, 0) != VK_SUCCESS) {
    //   vkWaitForFences(device->handle(), 1, &queue.fence, VK_FALSE, 1000000000);
    // }

    return vkQueuePresentKHR(queue.handle, &info);
  }

  void SimpleWindow::recreate(const uint32_t &width, const uint32_t &height) {
    device->wait();

    if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->physicalHandle(), surface, &surfaceData.surfaceCapabilities) != VK_SUCCESS) return;

    surfaceData.extent = chooseSwapchainExtent(width, height, surfaceData.surfaceCapabilities);

    createSwapChain(surfaceData, surface, device, swapchain, swapchainImages);

//     for (size_t i = 0; i < swapchainImages.size(); ++i) {
//       vkDestroyImageView(device->handle(), swapchain.imageViews[i], nullptr);
//     }

    swapchainImages = device->getSwapchainImages(swapchain);
    for (size_t i = 0; i < swapchainImages.size(); ++i) {
      swapchainImages[i]->createView(VK_IMAGE_VIEW_TYPE_2D, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}, surfaceData.format.format);

      device->destroy(framebuffers[i].framebuffer());

      const std::vector<VkImageView> attachments = {
        swapchainImages[i]->view(),
        depthImage->view()
      };

      const VkFramebufferCreateInfo frameInfo{
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        nullptr,
        0,
        renderPassHandle,
        static_cast<uint32_t>(attachments.size()),
        attachments.data(),
        surfaceData.extent.width,
        surfaceData.extent.height,
        1
      };
      Framebuffer fb = device->create(frameInfo, "framebuffer_for_simplewindow_" + std::to_string(i));

      framebuffers[i].setFramebuffer(fb);
      framebuffers[i].setSize({{0, 0}, surfaceData.extent});
    }

//     swapchain.imageViews.resize(swapchain.images.size(), VK_NULL_HANDLE);
//     for (size_t i = 0; i < swapchain.imageViews.size(); ++i) {
//       VkImageViewCreateInfo info{
//         VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
//         nullptr,
//         0,
//         swapchain.images[i],
//         VK_IMAGE_VIEW_TYPE_2D,
//         surfaceData.format.format,
//         YAVF_DEFAULT_SWIZZLE,
//         {
//           VK_IMAGE_ASPECT_COLOR_BIT,
//           0, 1, 0, 1
//         }
//       };
//
//       if (vkCreateImageView(device->handle(), &info, nullptr, &swapchain.imageViews[i]) != VK_SUCCESS) return;
//     }

    depthImage->recreate({surfaceData.extent.width, surfaceData.extent.height, 1});

//     depthImage->createView(VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_DEPTH_BIT);

    TransferTask* task = device->allocateTransferTask();

    task->begin();
    task->setBarrier(depthImage, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    task->end();

    task->start();
    task->wait();

    device->deallocate(task);

    // изменилось ли количество картинок свопчейна? вряд ли они меняются
  }

  bool SimpleWindow::isValid() const {
    return valid;
  }

  uint32_t SimpleWindow::getFamily() const {
    return family;
  }

  void SimpleWindow::createRenderPass() {
    RenderPassMaker rpm(device);

    renderPassHandle = rpm.attachmentBegin(surfaceData.format.format)
                            .attachmentLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
                            .attachmentStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
                            .attachmentFinalLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
                          .attachmentBegin(depthImage->info().format)
                            .attachmentLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
                            .attachmentStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE)
                            .attachmentInitialLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
                            .attachmentFinalLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
                          .subpassBegin(VK_PIPELINE_BIND_POINT_GRAPHICS)
                            .subpassColorAttachment(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0)
                            .subpassDepthStencilAttachment(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1)
                          .dependencyBegin(VK_SUBPASS_EXTERNAL, 0)
                            .dependencySrcStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
                            .dependencyDstStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
                            .dependencyDstAccessMask(VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
                          .create("default render pass");
  }

  PhysicalDevice::PhysicalDevice() : physDevice(VK_NULL_HANDLE) {}
  PhysicalDevice::PhysicalDevice(const PhysicalDevice &device) : physDevice(device.handle()) {}
  PhysicalDevice::PhysicalDevice(VkPhysicalDevice device) : physDevice(device) {}
  PhysicalDevice::~PhysicalDevice() {}

//   void PhysicalDevice::enumerateDeviceGroups( uint32_t* pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) {
//     vkEnumeratePhysicalDeviceGroups(physDevice, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
//   }

  void PhysicalDevice::getProperties(VkPhysicalDeviceProperties* pProperties) {
    vkGetPhysicalDeviceProperties(physDevice, pProperties);
  }

  void PhysicalDevice::getProperties2(VkPhysicalDeviceProperties2* pProperties) {
    vkGetPhysicalDeviceProperties2(physDevice, pProperties);
  }

  void PhysicalDevice::getQueueFamilyProperties(uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties* pQueueFamilyProperties) {
    vkGetPhysicalDeviceQueueFamilyProperties(physDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
  }

  void PhysicalDevice::getQueueFamilyProperties2(uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties2* pQueueFamilyProperties) {
    vkGetPhysicalDeviceQueueFamilyProperties2(physDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
  }

  void PhysicalDevice::getMemoryProperties(VkPhysicalDeviceMemoryProperties* pMemoryProperties) {
    vkGetPhysicalDeviceMemoryProperties(physDevice, pMemoryProperties);
  }

  void PhysicalDevice::getMemoryProperties2(VkPhysicalDeviceMemoryProperties2* pMemoryProperties) {
    vkGetPhysicalDeviceMemoryProperties2(physDevice, pMemoryProperties);
  }

  void PhysicalDevice::getFeatures(VkPhysicalDeviceFeatures* features) {
    vkGetPhysicalDeviceFeatures(physDevice, features);
  }

  VkPhysicalDevice PhysicalDevice::handle() const {
    return physDevice;
  }

  PhysicalDevice & PhysicalDevice::operator=(const PhysicalDevice &another) {
    physDevice = another.physDevice;
    return *this;
  }

  PhysicalDevice & PhysicalDevice::operator=(const VkPhysicalDevice &another) {
    physDevice = another;
    return *this;
  }

  PhysicalDevice::operator VkPhysicalDevice() const {
    return physDevice;
  }

  bool PhysicalDevice::operator==(const PhysicalDevice &another) const {
    return physDevice == another.physDevice;
  }

  bool PhysicalDevice::operator!=(const PhysicalDevice &another) const {
    return physDevice != another.physDevice;
  }

  Device::Device(const CreateInfo &info) {
    this->inst = info.inst;
    this->h = info.device;
    this->phys = info.phys;

//     vkGetPhysicalDeviceProperties(phys, props);
    VkPhysicalDeviceProperties props;
//     vkGetPhysicalDeviceProperties(phys, &props);
    phys.getProperties(&props);

    this->minMemoryMapAlignment = props.limits.minMemoryMapAlignment;
    this->nonCoherentAtomSize = props.limits.nonCoherentAtomSize;

    {
      const VmaAllocatorCreateInfo allocInfo{
//        VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT,
  0,
        phys,
        h,
        info.bufferSizeBlock, //1000000,  // 1 mib
        nullptr,
        nullptr,
        0,
        nullptr,
        nullptr,
        nullptr,
        inst->handle(),
        VK_MAKE_VERSION(1, 0, 0)
      };

      vkCheckError("vmaCreateAllocator", nullptr,
      vmaCreateAllocator(&allocInfo, &bufferAllocatorHandle));
    }

    {
      const VmaAllocatorCreateInfo allocInfo{
//        VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT,
        0,
        phys,
        h,
        info.imageSizeBlock, //1000000,  // 1 mib
        nullptr,
        nullptr,
        0,
        nullptr,
        nullptr,
        nullptr,
        inst->handle(),
        VK_MAKE_VERSION(1, 0, 0)
      };

      vkCheckError("vmaCreateAllocator", nullptr,
      vmaCreateAllocator(&allocInfo, &imageAllocatorHandle));
    }

    commandPools.resize(4, VK_NULL_HANDLE);

    for (size_t i = 0; i < info.familiesCount; ++i) {
      this->families.emplace_back(i, info.families[i].count, info.families[i].flags, h);
    }

    {
      uint32_t index = getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);

      VkCommandPoolCreateInfo commandInfo{
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        nullptr,
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        static_cast<uint32_t>(index)
      };

      VkCommandPool pool = VK_NULL_HANDLE;
      vkCheckError("vkCreateCommandPool", nullptr,
      vkCreateCommandPool(h, &commandInfo, nullptr, &pool));

      commandPools[0] = pool;
    }

    {
      uint32_t index = getQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);

      VkCommandPoolCreateInfo commandInfo{
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        nullptr,
        VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        static_cast<uint32_t>(index)
      };

      VkCommandPool pool = VK_NULL_HANDLE;
      vkCheckError("vkCreateCommandPool", nullptr,
      vkCreateCommandPool(h, &commandInfo, nullptr, &pool));

      commandPools[1] = pool;
    }

    {
      uint32_t index = getQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);

      VkCommandPoolCreateInfo commandInfo{
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        nullptr,
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        index
      };

      VkCommandPool pool = VK_NULL_HANDLE;
      vkCheckError("vkCreateCommandPool", nullptr,
      vkCreateCommandPool(h, &commandInfo, nullptr, &pool));

      commandPools[2] = pool;
    }

    {
      uint32_t index = getQueueFamilyIndex(YAVF_ANY_QUEUE);

      VkCommandPoolCreateInfo commandInfo{
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        nullptr,
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        index
      };

      VkCommandPool pool = VK_NULL_HANDLE;
      vkCheckError("vkCreateCommandPool", nullptr,
      vkCreateCommandPool(h, &commandInfo, nullptr, &pool));

      commandPools[3] = pool;
    }
  }

  Device::~Device() {
    wait();

//     for (auto sample : samplerContainer) {
//       vkDestroySampler(h, sample, nullptr);
//     }
//     samplerContainer.clear();

    for (auto &sampler : samplers) {
      vkDestroySampler(h, sampler.second.handle(), nullptr);
    }
    samplers.clear();

    for (auto buffer : buffers) {
      bufferPool.deleteElement(buffer);
    }
    buffers.clear();

    for (auto bufferView : bufferViews) {
      bufferViewPool.deleteElement(bufferView);
    }
    bufferViews.clear();

    for (auto image : images) {
      imagePool.deleteElement(image);
    }
    images.clear();

    for (auto imageView : imageViews) {
      imageViewPool.deleteElement(imageView);
    }
    imageViews.clear();

    vmaDestroyAllocator(bufferAllocatorHandle);
    vmaDestroyAllocator(imageAllocatorHandle);

    for (auto task : graphicTasks) {
//       delete task;
      graphicsPool.deleteElement(task);
    }
    graphicTasks.clear();

    for (auto task : combinedTasks) {
//       delete task;
      combinedPool.deleteElement(task);
    }
    combinedTasks.clear();

    for (auto task : computeTasks) {
//       delete task;
      computePool.deleteElement(task);
    }
    computeTasks.clear();

    for (auto task : transferTasks) {
      copyPool.deleteElement(task);
    }
    transferTasks.clear();

    for (auto &family : families) {
      family.clear();
    }
    families.clear();

    for (auto semaphore : semaphores) {
      vkDestroySemaphore(h, semaphore, nullptr);
    }
    semaphores.clear();

    for (auto fence : fences) {
      vkDestroyFence(h, fence, nullptr);
    }
    fences.clear();

    for (auto commandPool : commandPools) {
      vkDestroyCommandPool(h, commandPool, nullptr);
    }
    commandPools.clear();

    for (auto &pass : passes) {
      vkDestroyRenderPass(h, pass.second, nullptr);
    }
    passes.clear();

    for (auto &layout : layouts) {
      vkDestroyPipelineLayout(h, layout.second, nullptr);
    }
    layouts.clear();

    for (auto &pipeline : pipelines) {
      vkDestroyPipeline(h, pipeline.second.handle(), nullptr);
    }
    pipelines.clear();

    for (auto &setLayout : setLayouts) {
      vkDestroyDescriptorSetLayout(h, setLayout.second, nullptr);
    }
    setLayouts.clear();

    for (auto &pool : pools) {
      vkDestroyDescriptorPool(h, pool.second.pool, nullptr);
      for (size_t i = 0; i < pool.second.descriptorSetArray.size(); ++i) {
        descriptorSetPool.deleteElement(pool.second.descriptorSetArray[i]);
      }
    }
    pools.clear();

    for (auto &swapdata : swapchains) {
      vkDestroySwapchainKHR(h, swapdata.second.swap, nullptr);
      for (size_t i = 0; i < swapdata.second.images.size(); ++i) {
        Image* image = swapdata.second.images[i];
        imagePool.deleteElement(image);
      }
    }
    swapchains.clear();

    for (auto &framebuffer : framebuffers) {
      vkDestroyFramebuffer(h, framebuffer.second, nullptr);
    }
    framebuffers.clear();

    vkDestroyDevice(h, nullptr);
  }

  Buffer* Device::create(const BufferCreateInfo &info, const VmaMemoryUsage &usage) {
    YAVF_LOCK_MUTEX(buffer_mutex)

    Buffer* b = bufferPool.newElement(this, info, usage);

    b->internalIndex = buffers.size();
    buffers.push_back(b);

    return b;
  }

  Image* Device::create(const ImageCreateInfo &info, const VmaMemoryUsage &usage) {
    YAVF_LOCK_MUTEX(image_mutex)

    Image* i = imagePool.newElement(this, info, usage);

    i->internalIndex = images.size();
    images.push_back(i);

    return i;
  }

  BufferView* Device::create(const VkBufferViewCreateInfo &info, Buffer* relatedBuffer) {
    BufferView* v = bufferViewPool.newElement(this, info, relatedBuffer);

    v->internalIndex = bufferViews.size();
    bufferViews.push_back(v);

    return v;
  }

  ImageView* Device::create(const VkImageViewCreateInfo &info, Image* relatedImage) {
    ImageView* v = imageViewPool.newElement(this, info, relatedImage);

    v->internalIndex = imageViews.size();
    imageViews.push_back(v);

    return v;
  }

  DescriptorPool Device::create(const VkDescriptorPoolCreateInfo &info, const std::string &name) {
    auto itr = pools.find(name);
    if (itr != pools.end()) YAVF_ERROR_REPORT(nullptr, ("DescriptorPool with name " + name + " is already exist!").c_str(), nullptr);

    VkDescriptorPool pool = VK_NULL_HANDLE;
    vkCheckError("vkCreateDescriptorPool", name.c_str(),
    vkCreateDescriptorPool(h, &info, nullptr, &pool));

    pools[name].pool = pool;

    if (inst->debugSupport()) {
      // const VkDebugMarkerObjectNameInfoEXT nameInfo{
      //   VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
      //   nullptr,
      //   VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT,
      //   uint64_t(pool),
      //   name.c_str()
      // };

      // vkCheckError("vkDebugMarkerSetObjectNameEXT", name.c_str(),
      // vkDebugMarkerSetObjectNameEXT(h, &nameInfo));

      const VkDebugUtilsObjectNameInfoEXT nameInfo{
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        nullptr,
        VK_OBJECT_TYPE_DESCRIPTOR_POOL,
        uint64_t(pool),
        name.c_str()
      };

      vkCheckError("vkSetDebugUtilsObjectNameEXT", name.c_str(),
      yavfSetDebugUtilsObjectNameEXT(h, &nameInfo));
    }

    return pool;
  }

  Sampler Device::create(const VkSamplerCreateInfo &info, const std::string &name) {
    auto itr = samplers.find(name);
    if (itr != samplers.end()) YAVF_ERROR_REPORT(nullptr, ("Sampler with name " + name + " is already exist!").c_str(), nullptr)
    // throw std::runtime_error("Sampler with name " + name + " is already exist!");

    VkSampler sampler = VK_NULL_HANDLE;
    vkCheckError("vkCreateSampler", name.c_str(),
    vkCreateSampler(h, &info, nullptr, &sampler));

    Sampler s(sampler);
    samplers[name] = s;

    if (inst->debugSupport()) {
      // const VkDebugMarkerObjectNameInfoEXT nameInfo{
      //   VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
      //   nullptr,
      //   VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT,
      //   uint64_t(sampler),
      //   name.c_str()
      // };

      // vkCheckError("vkDebugMarkerSetObjectNameEXT", name.c_str(),
      // vkDebugMarkerSetObjectNameEXT(h, &nameInfo));

      const VkDebugUtilsObjectNameInfoEXT nameInfo{
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        nullptr,
        VK_OBJECT_TYPE_SAMPLER,
        uint64_t(sampler),
        name.c_str()
      };

      vkCheckError("vkSetDebugUtilsObjectNameEXT", name.c_str(),
      yavfSetDebugUtilsObjectNameEXT(h, &nameInfo));
    }

    return s;
  }

  DescriptorSetLayout Device::create(const VkDescriptorSetLayoutCreateInfo &info, const std::string &name) {
    auto itr = setLayouts.find(name);
    if (itr != setLayouts.end()) YAVF_ERROR_REPORT(nullptr, ("DescriptorSetLayout with name " + name + " is already exist!").c_str(), nullptr);

    VkDescriptorSetLayout setLayout = VK_NULL_HANDLE;
    vkCheckError("vkCreateDescriptorSetLayout", name.c_str(),
    vkCreateDescriptorSetLayout(h, &info, nullptr, &setLayout));

    setLayouts[name] = setLayout;

    if (inst->debugSupport()) {
      // const VkDebugMarkerObjectNameInfoEXT nameInfo{
      //   VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
      //   nullptr,
      //   VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT,
      //   uint64_t(setLayout),
      //   name.c_str()
      // };

      // vkCheckError("vkDebugMarkerSetObjectNameEXT", name.c_str(),
      // vkDebugMarkerSetObjectNameEXT(h, &nameInfo));

      const VkDebugUtilsObjectNameInfoEXT nameInfo{
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        nullptr,
        VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,
        uint64_t(setLayout),
        name.c_str()
      };

      vkCheckError("vkSetDebugUtilsObjectNameEXT", name.c_str(),
      yavfSetDebugUtilsObjectNameEXT(h, &nameInfo));
    }

    return setLayout;
  }

  PipelineLayout Device::create(const VkPipelineLayoutCreateInfo &info, const std::string &name) {
    auto itr = layouts.find(name);
    if (itr != layouts.end()) YAVF_ERROR_REPORT(nullptr, ("PipelineLayout with name " + name + " is already exist!").c_str(), nullptr);

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    vkCheckError("vkCreatePipelineLayout", name.c_str(),
    vkCreatePipelineLayout(h, &info, nullptr, &pipelineLayout));

    layouts[name] = pipelineLayout;

    if (inst->debugSupport()) {
      // const VkDebugMarkerObjectNameInfoEXT nameInfo{
      //   VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
      //   nullptr,
      //   VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT,
      //   uint64_t(pipelineLayout),
      //   name.c_str()
      // };

      // vkCheckError("vkDebugMarkerSetObjectNameEXT", name.c_str(),
      // vkDebugMarkerSetObjectNameEXT(h, &nameInfo));

      const VkDebugUtilsObjectNameInfoEXT nameInfo{
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        nullptr,
        VK_OBJECT_TYPE_PIPELINE_LAYOUT,
        uint64_t(pipelineLayout),
        name.c_str()
      };

      vkCheckError("vkSetDebugUtilsObjectNameEXT", name.c_str(),
      yavfSetDebugUtilsObjectNameEXT(h, &nameInfo));
    }

    return pipelineLayout;
  }

  Pipeline Device::create(const VkPipelineCache &cache, const VkGraphicsPipelineCreateInfo &info, const std::string &name) {
    auto itr = pipelines.find(name);
    if (itr != pipelines.end()) YAVF_ERROR_REPORT(nullptr, ("Pipeline with name " + name + " is already exist!").c_str(), nullptr);

    // тут можно создать сразу несколько пайплайнов, но я честно говоря не вижу в этом никакого смысла
    VkPipeline pipeline = VK_NULL_HANDLE;
    vkCheckError("vkCreateGraphicsPipelines", name.c_str(),
    vkCreateGraphicsPipelines(h, cache, 1, &info, nullptr, &pipeline));

    Pipeline p(pipeline, info.layout);
    pipelines[name] = p;

    if (inst->debugSupport()) {
      // const VkDebugMarkerObjectNameInfoEXT nameInfo{
      //   VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
      //   nullptr,
      //   VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
      //   uint64_t(pipeline),
      //   name.c_str()
      // };

      // vkCheckError("vkDebugMarkerSetObjectNameEXT", name.c_str(),
      // vkDebugMarkerSetObjectNameEXT(h, &nameInfo));

      const VkDebugUtilsObjectNameInfoEXT nameInfo{
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        nullptr,
        VK_OBJECT_TYPE_PIPELINE,
        uint64_t(pipeline),
        name.c_str()
      };

      vkCheckError("vkSetDebugUtilsObjectNameEXT", name.c_str(),
      yavfSetDebugUtilsObjectNameEXT(h, &nameInfo));
    }

    return p;
  }

  Pipeline Device::create(const VkPipelineCache &cache, const VkComputePipelineCreateInfo &info, const std::string &name) {
    auto itr = pipelines.find(name);
    if (itr != pipelines.end()) YAVF_ERROR_REPORT(nullptr, ("Pipeline with name " + name + " is already exist!").c_str(), nullptr);

    // тут можно создать сразу несколько пайплайнов, но я честно говоря не вижу в этом никакого смысла
    VkPipeline pipeline = VK_NULL_HANDLE;
    vkCheckError("vkCreateComputePipelines", name.c_str(),
    vkCreateComputePipelines(h, cache, 1, &info, nullptr, &pipeline));

    Pipeline p(pipeline, info.layout);
    pipelines[name] = p;

    if (inst->debugSupport()) {
      // const VkDebugMarkerObjectNameInfoEXT nameInfo{
      //   VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
      //   nullptr,
      //   VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
      //   uint64_t(pipeline),
      //   name.c_str()
      // };

      // vkCheckError("vkDebugMarkerSetObjectNameEXT", name.c_str(),
      // vkDebugMarkerSetObjectNameEXT(h, &nameInfo));

      const VkDebugUtilsObjectNameInfoEXT nameInfo{
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        nullptr,
        VK_OBJECT_TYPE_PIPELINE,
        uint64_t(pipeline),
        name.c_str()
      };

      vkCheckError("vkSetDebugUtilsObjectNameEXT", name.c_str(),
      yavfSetDebugUtilsObjectNameEXT(h, &nameInfo));
    }

    return p;
  }

  RenderPass Device::create(const VkRenderPassCreateInfo &info, const std::string &name) {
    auto itr = passes.find(name);
    if (itr != passes.end()) YAVF_ERROR_REPORT(nullptr, ("RenderPass with name " + name + " is already exist!").c_str(), nullptr);

    VkRenderPass pass = VK_NULL_HANDLE;
    vkCheckError("vkCreateRenderPass", name.c_str(),
    vkCreateRenderPass(h, &info, nullptr, &pass));

    passes[name] = pass;

    if (inst->debugSupport()) {
      // const VkDebugMarkerObjectNameInfoEXT nameInfo{
      //   VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
      //   nullptr,
      //   VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
      //   uint64_t(pass),
      //   name.c_str()
      // };

      // vkCheckError("vkDebugMarkerSetObjectNameEXT", name.c_str(),
      // vkDebugMarkerSetObjectNameEXT(h, &nameInfo));

      const VkDebugUtilsObjectNameInfoEXT nameInfo{
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        nullptr,
        VK_OBJECT_TYPE_RENDER_PASS,
        uint64_t(pass),
        name.c_str()
      };

      vkCheckError("vkSetDebugUtilsObjectNameEXT", name.c_str(),
      yavfSetDebugUtilsObjectNameEXT(h, &nameInfo));
    }

    return pass;
  }

  Framebuffer Device::create(const FramebufferCreateInfo &info, const std::string &name) {
    auto itr = framebuffers.find(name);
    if (itr != framebuffers.end()) YAVF_ERROR_REPORT(nullptr, ("Framebuffer with name " + name + " is already exist!").c_str(), nullptr);

    VkFramebuffer f = VK_NULL_HANDLE;
    vkCheckError("vkCreateFramebuffer", name.c_str(),
    vkCreateFramebuffer(h, &info._info, nullptr, &f));

    Framebuffer framebuffer(f);

    framebuffers[name] = framebuffer;

    if (inst->debugSupport()) {
      const VkDebugUtilsObjectNameInfoEXT nameInfo{
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        nullptr,
        VK_OBJECT_TYPE_FRAMEBUFFER,
        uint64_t(f),
        name.c_str()
      };

      vkCheckError("vkSetDebugUtilsObjectNameEXT", name.c_str(),
      yavfSetDebugUtilsObjectNameEXT(h, &nameInfo));
    }

    return framebuffer;
  }

  Swapchain Device::create(const VkSwapchainCreateInfoKHR &info, const std::string &name) {
    auto itr = swapchains.find(name);
    if (itr != swapchains.end()) YAVF_ERROR_REPORT(nullptr, ("Swapchain with name " + name + " is already exist!").c_str(), nullptr);

    VkSwapchainKHR s = VK_NULL_HANDLE;
    vkCheckError("vkCreateSwapchainKHR", name.c_str(),
    vkCreateSwapchainKHR(h, &info, nullptr, &s));

    Swapchain swapchain(info.imageExtent, this, s);

    swapchains[name].swap = swapchain;

    if (inst->debugSupport()) {
      const VkDebugUtilsObjectNameInfoEXT nameInfo{
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        nullptr,
        VK_OBJECT_TYPE_SWAPCHAIN_KHR,
        uint64_t(s),
        name.c_str()
      };

      vkCheckError("vkSetDebugUtilsObjectNameEXT", name.c_str(),
      yavfSetDebugUtilsObjectNameEXT(h, &nameInfo));
    }

    return swapchain;
  }

  Swapchain Device::recreate(const VkSwapchainCreateInfoKHR &info, const std::string &name) {
    VkSwapchainKHR s = VK_NULL_HANDLE;
    vkCheckError("vkCreateSwapchainKHR", name.c_str(),
    vkCreateSwapchainKHR(h, &info, nullptr, &s));

    Swapchain swapchain(info.imageExtent, this, s);

    auto itr = swapchains.find(name);
    if (itr == swapchains.end()) {
      swapchains[name].swap = swapchain;
    } else {
      vkDestroySwapchainKHR(h, itr->second.swap, nullptr);
      itr->second.swap = swapchain;

      for (size_t i = 0; i < itr->second.images.size(); ++i) {
        imagePool.deleteElement(itr->second.images[i]);
      }
      itr->second.images.clear();
    }

    if (inst->debugSupport()) {
      const VkDebugUtilsObjectNameInfoEXT nameInfo{
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        nullptr,
        VK_OBJECT_TYPE_SWAPCHAIN_KHR,
        uint64_t(s),
        name.c_str()
      };

      vkCheckError("vkSetDebugUtilsObjectNameEXT", name.c_str(),
      yavfSetDebugUtilsObjectNameEXT(h, &nameInfo));
    }

    return swapchain;
  }

  DescriptorSet* Device::create(const DescriptorPool &pool, const VkDescriptorSet &set) {
    DescriptorSet* base = descriptorSetPool.newElement(this, set);

    for (auto &data : pools) {
      if (data.second.pool == pool) {
        data.second.descriptorSetArray.push_back(base);
        break;
      }
    }

    return base;
  }

  std::vector<Image*> Device::getSwapchainImages(const Swapchain &swapchain) {
    for (auto itr = swapchains.begin(); itr != swapchains.end(); ++itr) {
      if (itr->second.swap == swapchain) {
        if (!itr->second.images.empty()) return itr->second.images;

        uint32_t count = 0;
        vkCheckError("vkGetSwapchainImagesKHR", nullptr,
        vkGetSwapchainImagesKHR(h, swapchain, &count, nullptr));

        // так как мы к этому моменту можем (и скорее всего) переделать свопчеин
        // это так не работает, наверное придется каждый раз память перевыделять
        //if (itr->second.images.size() == count) return itr->second.images;

        VkImage imgs[count];
        vkCheckError("vkGetSwapchainImagesKHR", nullptr,
        vkGetSwapchainImagesKHR(h, swapchain, &count, imgs));

        itr->second.images.resize(count, nullptr);

        for (size_t i = 0; i < count; ++i) {
          Image* img = imagePool.newElement(this, imgs[i], itr->second.swap.getImageExtent());
          itr->second.images[i] = img;
        }

        return itr->second.images;

        break;
      }
    }

    return std::vector<Image*>();
  }

  std::vector<Image*> Device::getSwapchainImages(const std::string &name) {
    auto itr = swapchains.find(name);
    if (itr == swapchains.end()) return std::vector<Image*>();

    if (!itr->second.images.empty()) return itr->second.images;

    uint32_t count = 0;
    vkCheckError("vkGetSwapchainImagesKHR", nullptr,
    vkGetSwapchainImagesKHR(h, itr->second.swap, &count, nullptr));

    itr->second.images.resize(count, nullptr);

    // так как мы к этому моменту можем (и скорее всего) переделать свопчеин
    // это так не работает, наверное придется каждый раз память перевыделять
    //if (itr->second.images.size() == count) return itr->second.images;

    VkImage imgs[count];
    vkCheckError("vkGetSwapchainImagesKHR", nullptr,
    vkGetSwapchainImagesKHR(h, itr->second.swap, &count, imgs));

    for (size_t i = 0; i < count; ++i) {
      Image* img = imagePool.newElement(this, imgs[i], itr->second.swap.getImageExtent());
      itr->second.images[i] = img;
    }

    return itr->second.images;
  }

  void Device::destroy(Buffer* buffer) {
    YAVF_LOCK_MUTEX(buffer_mutex)

    buffers.back()->internalIndex = buffer->internalIndex;
    std::swap(buffers.back(), buffers[buffer->internalIndex]);
    buffers.pop_back();

    bufferPool.deleteElement(buffer);
  }

  void Device::destroy(Image* image) {
    YAVF_LOCK_MUTEX(image_mutex)

    images.back()->internalIndex = image->internalIndex;
    std::swap(images.back(), images[image->internalIndex]);
    images.pop_back();

    imagePool.deleteElement(image);
  }

  void Device::destroy(BufferView* bufferView) {
    bufferViews.back()->internalIndex = bufferView->internalIndex;
    std::swap(bufferViews.back(), bufferViews[bufferView->internalIndex]);
    bufferViews.pop_back();

    bufferViewPool.deleteElement(bufferView);
  }

  void Device::destroy(ImageView* imageView) {
    imageViews.back()->internalIndex = imageView->internalIndex;
    std::swap(imageViews.back(), imageViews[imageView->internalIndex]);
    imageViews.pop_back();

    imageViewPool.deleteElement(imageView);
  }

  //const uint32_t &frameCount
  GraphicTask* Device::allocateGraphicTask(const bool &primary) {
    //GraphicTask* task = new GraphicTask(this, primary);
    GraphicTask* task = graphicsPool.newElement(this, primary);
    graphicTasks.push_back(task);

    return task;
  }

  ComputeTask* Device::allocateComputeTask(const bool &primary) {
//     ComputeTask* task = new ComputeTask(this, primary);
    ComputeTask* task = computePool.newElement(this, primary);
    computeTasks.push_back(task);

    return task;
  }

  CombinedTask* Device::allocateCombinedTask(const bool &primary) {
    //CombinedTask* task = new CombinedTask(this, primary);
    CombinedTask* task = combinedPool.newElement(this, primary);
    combinedTasks.push_back(task);

    return task;
  }

  TransferTask* Device::allocateTransferTask(const bool &primary) {
    TransferTask* task = copyPool.newElement(this, primary);
    transferTasks.push_back(task);

    return task;
  }

  void Device::deallocate(GraphicTask* task) {
    task->wait();

    for (size_t i = 0; i < graphicTasks.size(); ++i) {
      if (task == graphicTasks[i]) {
        std::swap(graphicTasks[i], graphicTasks.back());
        graphicTasks.pop_back();
//         delete task;
        graphicsPool.deleteElement(task);
        break;
      }
    }
  }

  void Device::deallocate(ComputeTask* task) {
    task->wait();

    for (size_t i = 0; i < computeTasks.size(); ++i) {
      if (task == computeTasks[i]) {
        std::swap(computeTasks[i], computeTasks.back());
        computeTasks.pop_back();
//         delete task;
        computePool.deleteElement(task);
        break;
      }
    }
  }

  void Device::deallocate(CombinedTask* task) {
    task->wait();

    for (size_t i = 0; i < combinedTasks.size(); ++i) {
      if (task == combinedTasks[i]) {
        std::swap(combinedTasks[i], combinedTasks.back());
        combinedTasks.pop_back();
        //delete task;
        combinedPool.deleteElement(task);
        break;
      }
    }
  }

  void Device::deallocate(TransferTask* task) {
    task->wait();

    for (size_t i = 0; i < transferTasks.size(); ++i) {
      if (task == transferTasks[i]) {
        std::swap(transferTasks[i], transferTasks.back());
        transferTasks.pop_back();
        copyPool.deleteElement(task);
        break;
      }
    }
  }

//   SemaphoreOwner* Device::createSemaphoreProxy() {
//     return proxies.newElement();
//   }
//
//   void Device::destroySemaphoreProxy(SemaphoreOwner* proxy) {
//     proxies.deleteElement(proxy);
//   }

  //uint32_t* index
  Semaphore Device::createSemaphore() {
    VkSemaphore s = VK_NULL_HANDLE;

    const VkSemaphoreCreateInfo i{
      VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
      nullptr,
      0
    };

    vkCheckError("vkCreateSemaphore", nullptr,
    vkCreateSemaphore(h, &i, nullptr, &s));

    semaphores.push_back(s);

    return s;
  }

  void Device::destroy(const Semaphore &s) {
//     for (int32_t i = semaphores.size()-1; i >= 0; ++i) {
//       if (semaphores[i] == s) {
//         vkDestroySemaphore(h, semaphores[i], nullptr);
//         semaphores.erase(semaphores.begin()+i);
//         return;
//       }
//     }

    size_t index = SIZE_MAX;
    for (size_t i = 0; i < semaphores.size(); ++i) {
      if (s == semaphores[i]) index = i;
    }

    if (index == SIZE_MAX) YAVF_ERROR_REPORT("Device::destroy Semaphore", "Could not find semaphore" , nullptr);

    vkDestroySemaphore(h, s, nullptr);
    std::swap(semaphores[index], semaphores.back());
    semaphores.pop_back();

//     for (auto it = semaphores.begin(); it != semaphores.end(); ++it) {
//       if (*it == s) {
//         vkDestroySemaphore(h, s, nullptr);
//         semaphores.erase(it);
//         return;
//       }
//     }

    // тут по идее должна быть ошибка
  }

//   void Device::destroySemaphore(const uint32_t &index) {
//     vkDestroySemaphore(h, semaphores[index], nullptr);
//     semaphores.erase(semaphores.begin()+index);
//   }

  Fence Device::createFence(const VkFenceCreateFlags &flags) {
    VkFence f = VK_NULL_HANDLE;

    const VkFenceCreateInfo i{
      VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      nullptr,
      flags
    };

    vkCheckError("vkCreateSemaphore", nullptr,
    vkCreateFence(h, &i, nullptr, &f));

    fences.push_back(f);

    return f;
  }

  void Device::destroy (const Fence &f) {
//     for (int32_t i = fences.size()-1; i >= 0; ++i) {
//       if (fences[i] == f) {
//         vkDestroyFence(h, fences[i], nullptr);
//         fences.erase(fences.begin()+i);
//         return;
//       }
//     }

    size_t index = SIZE_MAX;
    for (size_t i = 0; i < fences.size(); ++i) {
      if (f == fences[i]) index = i;
    }

    if (index == SIZE_MAX) YAVF_ERROR_REPORT("Device::destroy Fence", "Could not find fence" , nullptr);

    vkDestroyFence(h, f, nullptr);
    std::swap(fences[index], fences.back());
    fences.pop_back();

//     for (auto it = fences.begin(); it != fences.end(); ++it) {
//       if (*it == f) {
//         vkDestroyFence(h, f, nullptr);
//         fences.erase(it);
//         return;
//       }
//     }

    // тут по идее должна быть ошибка
  }

//   void Device::destroyFence(const uint32_t &index) {
//     vkDestroyFence(h, fences[index], nullptr);
//     fences.erase(fences.begin()+index);
//   }

  Internal::Queue Device::submit(const uint32_t &family, const uint32_t &submitCount, const VkSubmitInfo* submitInfo) {
//     std::cout << "families.size() " << families.size() << '\n';

    return families[family].submitCommands(submitCount, submitInfo);
  }

  Internal::Queue Device::submit(const std::vector<TaskInterface*> &tasks) {
    std::vector<VkSubmitInfo> infos(tasks.size());
    VkQueueFlags f = 0;

    /*std::vector<std::vector<VkSemaphore>> waits(tasks.size());
    std::vector<std::vector<VkPipelineStageFlags>> flags(tasks.size());
    std::vector<std::vector<VkSemaphore>> signal(tasks.size())*/;

//     for (uint32_t i = 0; i < tasks.size(); ++i) {
//       const uint32_t waitCount = tasks[i]->waitSemaphoresCount();
//       const uint32_t signalCount = tasks[i]->signalSemaphoresCount();
//
//       waits[i].resize(waitCount);
//       flags[i].resize(waitCount);
//       signal[i].resize(waitCount);
//
//       tasks[i]->getWaitSemaphores(waits[i].data(), flags[i].data());
//
//       tasks[i]->getSignalSemaphores(signal[i].data());
//
//       VkCommandBuffer c = tasks[i]->getCommandBuffer();
//
//       const VkSubmitInfo info{
//         VK_STRUCTURE_TYPE_SUBMIT_INFO,
//         nullptr,
//         waitCount,
//         waits[i].data(),
//         flags[i].data(),
//         1,
//         &c,
//         signalCount,
//         signal[i].data()
//       };
//
//       infos.push_back(info);
//
//       f |= tasks[i]->getQueueFlags();
//     }

    for (uint32_t i = 0; i < tasks.size(); ++i) {
      infos[i] = tasks[i]->getSubmitInfo();

      f |= tasks[i]->getQueueFlags();
    }

    uint32_t index = getQueueFamilyIndex(f);

    vkCheckError("getQueueFamilyIndex", nullptr, index == UINT32_MAX ? VK_ERROR_INITIALIZATION_FAILED : VK_SUCCESS);

    return families[index].submitCommands(infos.size(), infos.data());
  }

  uint32_t Device::getQueueFamilyIndex(const VkQueueFlags &flags) const {
    for (uint16_t i = 0; i < families.size(); ++i) {
      if (families[i].flags() & flags) return i;
    }

    return UINT32_MAX;
  }

  Internal::Queue Device::getQueue(const uint32_t &familyIndex) const {
    return families[familyIndex].queue();
  }

  uint32_t Device::getFamiliesCount() const {
    return families.size();
  }

  void Device::wait() const {
    vkCheckError("vkDeviceWaitIdle", nullptr,
    vkDeviceWaitIdle(h));
  }

  VkDevice Device::handle() const {
    return h;
  }

  VkPhysicalDevice Device::physicalHandle() const {
    return phys;
  }

  VmaAllocator Device::bufferAllocator() const {
    return bufferAllocatorHandle;
  }

  VmaAllocator Device::imageAllocator()  const {
    return imageAllocatorHandle;
  }

  VkCommandPool Device::commandPool(const uint32_t &index) const {
    return commandPools[index];
  }

  VkRenderPass Device::renderpass(const std::string &name) const {
    auto itr = passes.find(name);
    if (itr != passes.end()) return itr->second;

    return VK_NULL_HANDLE;
  }

  Pipeline Device::pipeline(const std::string &name) const {
    auto itr = pipelines.find(name);
    if (itr != pipelines.end()) return itr->second;

    return Pipeline();
  }

  VkPipelineLayout Device::layout(const std::string &name) const {
    auto itr = layouts.find(name);
    if (itr != layouts.end()) return itr->second;

    return VK_NULL_HANDLE;
  }

  VkDescriptorSetLayout Device::setLayout(const std::string &name) const {
    auto itr = setLayouts.find(name);
    if (itr != setLayouts.end()) return itr->second;

    return VK_NULL_HANDLE;
  }

  Sampler Device::sampler(const std::string &name) const {
    auto itr = samplers.find(name);
    if (itr != samplers.end()) return itr->second;

    return Sampler(VK_NULL_HANDLE);
  }

  VkDescriptorPool Device::descriptorPool(const std::string &name) const {
    auto itr = pools.find(name);
    if (itr != pools.end()) return itr->second.pool;

    return VK_NULL_HANDLE;
  }

  void Device::destroy(const Framebuffer &framebuffer) {
//     for (uint32_t i = 0; i < framebuffers.size(); ++i) {
//       if (framebuffer == framebuffers[i]) {
//         std::swap(framebuffers[i], framebuffers.back());
//         framebuffers.pop_back();
//         vkDestroyFramebuffer(h, framebuffer, nullptr);
//         break;
//       }
//     }

    for (auto itr = framebuffers.begin(); itr != framebuffers.end(); ++itr) {
      if (itr->second == framebuffer) {
        vkDestroyFramebuffer(h, framebuffer, nullptr);
        framebuffers.erase(itr);
        break;
      }
    }
  }

  void Device::destroy(const Swapchain &swapchain) {
    for (auto itr = swapchains.begin(); itr != swapchains.end(); ++itr) {
      if (itr->second.swap == swapchain) {
        vkDestroySwapchainKHR(h, swapchain, nullptr);

        for (size_t i = 0; i < itr->second.images.size(); ++i) {
          Image* image = itr->second.images[i];
          imagePool.deleteElement(image);
        }

        swapchains.erase(itr);
        break;
      }
    }
  }

  void Device::destroy(const RenderPass &pass) {
    for (auto itr = passes.begin(); itr != passes.end(); ++itr) {
      if (itr->second == pass) {
        vkDestroyRenderPass(h, itr->second, nullptr);
        passes.erase(itr);
        break;
      }
    }
  }

  void Device::destroy(const Pipeline &pipeline) {
    for (auto itr = pipelines.begin(); itr != pipelines.end(); ++itr) {
      if (itr->second == pipeline) {
        vkDestroyPipeline(h, itr->second, nullptr);
        pipelines.erase(itr);
        break;
      }
    }
  }

  void Device::destroy(const PipelineLayout &pipelineLayout) {
    for (auto itr = layouts.begin(); itr != layouts.end(); ++itr) {
      if (itr->second == pipelineLayout) {
        vkDestroyPipelineLayout(h, itr->second, nullptr);
        layouts.erase(itr);
        break;
      }
    }
  }

  void Device::destroy(const DescriptorSetLayout &setLayout) {
    for (auto itr = setLayouts.begin(); itr != setLayouts.end(); ++itr) {
      if (itr->second == setLayout) {
        vkDestroyDescriptorSetLayout(h, itr->second, nullptr);
        setLayouts.erase(itr);
        break;
      }
    }
  }

  void Device::destroy(const Sampler &sampler) {
    for (auto itr = samplers.begin(); itr != samplers.end(); ++itr) {
      if (itr->second.handle() == sampler.handle()) {
        vkDestroySampler(h, itr->second.handle(), nullptr);
        samplers.erase(itr);
        break;
      }
    }
  }

  void Device::destroy(const DescriptorPool &descriptorPool) {
    for (auto itr = pools.begin(); itr != pools.end(); ++itr) {
      if (itr->second.pool == descriptorPool) {
        vkDestroyDescriptorPool(h, itr->second.pool, nullptr);

        for (size_t i = 0; i < itr->second.descriptorSetArray.size(); ++i) {
          descriptorSetPool.deleteElement(itr->second.descriptorSetArray[i]);
        }

        pools.erase(itr);
        break;
      }
    }
  }

  void Device::destroyRenderPass(const std::string &name) {
    auto it = passes.find(name);
    if (it == passes.end()) return;

    vkDestroyRenderPass(h, it->second, nullptr);

    passes.erase(it);
  }

  void Device::destroyPipeline(const std::string &name) {
    auto it = pipelines.find(name);
    if (it == pipelines.end()) return;

    vkDestroyPipeline(h, it->second.handle(), nullptr);

    pipelines.erase(it);
  }

  void Device::destroyLayout(const std::string &name) {
    auto it = layouts.find(name);
    if (it == layouts.end()) return;

    vkDestroyPipelineLayout(h, it->second, nullptr);

    layouts.erase(it);
  }

  void Device::destroySetLayout(const std::string &name) {
    auto it = setLayouts.find(name);
    if (it == setLayouts.end()) return;

    vkDestroyDescriptorSetLayout(h, it->second, nullptr);

    setLayouts.erase(it);
  }

  void Device::destroySampler(const std::string &name) {
    auto it = samplers.find(name);
    if (it == samplers.end()) return;

    vkDestroySampler(h, it->second.handle(), nullptr);

    samplers.erase(it);
  }

  void Device::destroyDescriptorPool(const std::string &name) {
    auto it = pools.find(name);
    if (it == pools.end()) return;

    vkDestroyDescriptorPool(h, it->second.pool, nullptr);
    for (size_t i = 0; i < it->second.descriptorSetArray.size(); ++i) {
      descriptorSetPool.deleteElement(it->second.descriptorSetArray[i]);
    }

    pools.erase(it);
  }

  void Device::destroySwapchain(const std::string &name) {
    auto it = swapchains.find(name);
    if (it == swapchains.end()) return;

    vkDestroySwapchainKHR(h, it->second.swap, nullptr);
    for (size_t i = 0; i < it->second.images.size(); ++i) {
      Image* image = it->second.images[i];
      imagePool.deleteElement(image);
    }

    swapchains.erase(it);
  }

  void Device::destroyFramebuffer(const std::string &name) {
    auto it = framebuffers.find(name);
    if (it == framebuffers.end()) return;

    vkDestroyFramebuffer(h, it->second, nullptr);

    framebuffers.erase(it);
  }

  VmaStats Device::getBufferStatistic() const {
    VmaStats stats;
    vmaCalculateStats(bufferAllocatorHandle, &stats);

    return stats;
  }

  VmaStats Device::getImageStatistic() const {
    VmaStats stats;
    vmaCalculateStats(imageAllocatorHandle, &stats);

    return stats;
  }

  void Device::getProperties(VkPhysicalDeviceProperties* props) const {
    vkGetPhysicalDeviceProperties(phys, props);
  }

  size_t Device::getMinMemoryMapAlignment() const {
    return minMemoryMapAlignment;
  }

  size_t Device::getNonCoherentAtomSize() const {
    return nonCoherentAtomSize;
  }

//   void Instance::addLayer(const char* layerName) {
//     layers.push_back(layerName);
//   }
//
//   void Instance::addExtension(const char* extensionName) {
//     extensions.push_back(extensionName);
//   }
//
//   void Instance::setLayers(const std::vector<const char*> &layers) {
//     Instance::layers = layers;
//   }
//
//   void Instance::setExtensions(const std::vector<const char*> &extensions) {
//     Instance::extensions = extensions;
//   }
//
//   std::vector<const char*> & Instance::getLayers() {
//     return layers;
//   }
//
//   std::vector<const char*> & Instance::getExtensions() {
//     return extensions;
//   }

//   struct cmp_str {
//     bool operator()(char const *a, char const *b) const {
//       return std::strcmp(a, b) < 0;
//     }
//   };

  //, callback(VK_NULL_HANDLE)
  Instance::Instance() : inst(VK_NULL_HANDLE), messenger(VK_NULL_HANDLE) {}
  Instance::Instance(const CreateInfo &info) : inst(VK_NULL_HANDLE), messenger(VK_NULL_HANDLE) {
    construct(info);
  }

  Instance::~Instance() {
    for (auto &device : devices) {
      delete device.second;
    }

    if (messenger != VK_NULL_HANDLE) {
      DestroyDebugUtilsMessengerEXT(inst, messenger, nullptr);
      messenger = VK_NULL_HANDLE;
    }

    if (inst != VK_NULL_HANDLE) {
      vkDestroyInstance(inst, nullptr);
      inst = VK_NULL_HANDLE;
    }
  }

  void Instance::construct(const CreateInfo &info) {
    std::vector<VkLayerProperties> lay;
    std::vector<VkExtensionProperties> ext;

//     if (info.createDebugLayer) {
//       std::unordered_set<const char*> tmp(info.extensionNames.begin(), info.extensionNames.end());
//       tmp.insert("VK_EXT_debug_utils");
//       extensionNames = std::vector<const char*>(tmp.begin(), tmp.end());
//     } else {
//       extensionNames = std::vector<const char*>(info.extensionNames.begin(), info.extensionNames.end());
//     }

    bool createDebugLayer = info.createDebugLayer;
//     bool hasExt = false;
    for (size_t i = 0; i < info.extensionNames.size(); ++i) {
      if (strcmp(info.extensionNames[i], "VK_EXT_debug_utils") == 0) {
//       if (std::string(info.extensionNames[i]).compare("VK_EXT_debug_utils") == 0) {
        createDebugLayer = true;
//         hasExt = true;
        break;
      }
    }
/*
    if (!hasExt && createDebugLayer) {

    }
    */

    lay = getRequiredValidationLayers(info.layerNames);
    ext = getRequiredExtensions(info.layerNames, info.extensionNames, createDebugLayer);

    std::vector<const char*> layerNames(lay.size(), nullptr);
    std::vector<const char*> extensionNames(ext.size(), nullptr);

    for (size_t i = 0; i < lay.size(); ++i) {
//       const size_t len = strlen(lay[i].layerName)+1;
//       layerNames[i] = new char[len];
//       memcpy(layerNames[i], lay[i].layerName, len);
      layerNames[i] = lay[i].layerName;
    }

    for (size_t i = 0; i < ext.size(); ++i) {
      extensionNames[i] = ext[i].extensionName;
    }

    if (info.layerNames.size() != layerNames.size()) {
      for (size_t i = 0; i < info.layerNames.size(); ++i) {
        bool present = false;
        for (size_t j = 0; j < layerNames.size(); ++j) {
          if (strcmp(info.layerNames[i], layerNames[j]) == 0) {
//           if (std::string(info.layerNames[i]).compare(layerNames[j]) == 0) {
            present = true;
          }
        }

        if (!present) {
          YAVF_WARNING_REPORT("Layer: " + std::string(info.layerNames[i]) + " is not present")
        }
      }
    }

    if (info.extensionNames.size() != extensionNames.size()) {
      for (size_t i = 0; i < info.extensionNames.size(); ++i) {
        bool present = false;
        for (size_t j = 0; j < extensionNames.size(); ++j) {
          if (strcmp(info.extensionNames[i], extensionNames[j]) == 0) {
//           if (std::string(info.extensionNames[i]).compare(extensionNames[j]) == 0) {
            present = true;
          }
        }

        if (!present) {
          YAVF_WARNING_REPORT("Extension: " + std::string(info.extensionNames[i]) + " is not present")
        }
      }
    }

    const VkApplicationInfo appInfo{
      VK_STRUCTURE_TYPE_APPLICATION_INFO,
      nullptr,
      (info.appInfoPtr != nullptr ? info.appInfoPtr->appName.c_str() : nullptr),
      (info.appInfoPtr != nullptr ? info.appInfoPtr->appVersion : 0),
      (info.appInfoPtr != nullptr ? info.appInfoPtr->engineName.c_str() : nullptr),
      (info.appInfoPtr != nullptr ? info.appInfoPtr->engineVersion : 0),
      (info.appInfoPtr != nullptr ? info.appInfoPtr->apiVersion : 0)
    };

    const VkInstanceCreateInfo instInfo{
      VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      info.creationExtension,
      0,
      info.appInfoPtr != nullptr ? &appInfo : nullptr,
      static_cast<uint32_t>(layerNames.size()), layerNames.data(),
      static_cast<uint32_t>(extensionNames.size()), extensionNames.data()
    };

    vkCheckError("vkCreateInstance", nullptr,
    vkCreateInstance(&instInfo, nullptr, &inst));

// #ifdef YAVF_DEBUG_REPORT_EXTENSION
//       extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
//       extensions.push_back("VK_EXT_debug_utils");
// #endif

    if (info.printLayerInfo) {
      YAVF_INFO_REPORT("Using layers:")
      for (const auto &layer : lay) {
//         layerNames.push_back(layer.layerName);
        YAVF_INFO_REPORT(("Name: "+std::string(layer.layerName)).c_str())
        YAVF_INFO_REPORT(("Description: "+std::string(layer.description)).c_str())
        YAVF_INFO_REPORT(("Version: "+VK_VERSION_TO_STRING(layer.specVersion)).c_str())
        YAVF_INFO_REPORT(("Implementation version: "+VK_VERSION_TO_STRING(layer.implementationVersion)).c_str())
      }
    }

    if (info.printExtensionInfo) {
      if (info.printLayerInfo) YAVF_INFO_REPORT("")
      YAVF_INFO_REPORT("Using extensions:")
      for (const auto &extension : ext) {
//         extensionNames.push_back(extension.extensionName);
        YAVF_INFO_REPORT(("Name: "+std::string(extension.extensionName)).c_str())
        YAVF_INFO_REPORT(("Version: "+VK_VERSION_TO_STRING(extension.specVersion)).c_str())
      }
    }

//     info.enabledExtensionCount = extensionNames.size();
//     info.ppEnabledExtensionNames = extensionNames.data();
//
//     info.enabledLayerCount = layerNames.size();
//     info.ppEnabledLayerNames = layerNames.data();

//     vkCheckError("vkCreateInstance", nullptr,
//     vkCreateInstance(&info, nullptr, &inst));

    if (info.createDebugLayer) {
      const VkDebugUtilsMessengerCreateInfoEXT messengerinfo{
        VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        nullptr,
        0,
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
//         VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
//         VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        (PFN_vkDebugUtilsMessengerCallbackEXT) &utilsMessengerCallback,
        nullptr
      };

//       vkCheckError("vkCreateDebugReportCallbackEXT", nullptr,
//       CreateDebugReportCallbackEXT(inst, &createInfo, nullptr, &callback));

      vkCheckError("vkCreateDebugUtilsMessengerEXT", nullptr,
      CreateDebugUtilsMessengerEXT(inst, &messengerinfo, nullptr, &messenger));
    }
  }

  //std::vector<VkPhysicalDevice> getDevices(const std::function<bool(const VkPhysicalDevice)> &func);
  std::vector<PhysicalDevice> Instance::getPhysicalDevices() {
    uint32_t count;
    vkEnumeratePhysicalDevices(inst, &count, nullptr);
    std::vector<VkPhysicalDevice> physDevices(count);
    std::vector<PhysicalDevice> physDevices2(count);
    vkEnumeratePhysicalDevices(inst, &count, physDevices.data());

    for (uint32_t i = 0; i < physDevices.size(); ++i) {
      physDevices2[i] = physDevices[i];
    }

    return physDevices2;
  }

  Device* Instance::getDevice(const std::string &name) const {
    auto itr = devices.find(name);
    if (itr == devices.end()) return nullptr;

    return itr->second;
  }

//     void create(const uint32_t &apiVersion, const std::string &appName, const uint32_t &appVersion, const std::string &enName, const uint32_t &engineVersion);
  Device* Instance::create(const Device::CreateInfo &info, const std::string &name) {
    auto itr = devices.find(name);
    if (itr != devices.end()) YAVF_ERROR_REPORT("Instance::create", "Device with this name is already present", name.c_str());

    Device* newDevice = new Device(info);
    devices[name] = newDevice;

    return newDevice;
  }

  bool Instance::debugSupport() const {
    return messenger != VK_NULL_HANDLE;
  }

  VkInstance Instance::handle() const {
    return inst;
  }

  Instance::operator VkInstance() const {
    return inst;
  }

  bool Instance::operator==(const Instance &another) const {
    return inst == another.inst;
  }

  bool Instance::operator!=(const Instance &another) const {
    return inst == another.inst;
  }
}

//   Instance::Instance() {}
//
//   Instance::~Instance() {
//     for (auto device : devices) {
//       delete device.second;
//     }
//
// //     DestroyDebugReportCallbackEXT(inst, callback, nullptr);
//     DestroyDebugUtilsMessengerEXT(inst, messenger, nullptr);
//     vkDestroyInstance(inst, nullptr);
//   }
//
//   std::vector<VkPhysicalDevice> Instance::getDevices(const std::function<bool(VkPhysicalDevice)> &func) {
//     uint32_t count;
//     vkEnumeratePhysicalDevices(inst, &count, nullptr);
//     std::vector<VkPhysicalDevice> physDevices(count);
//     vkEnumeratePhysicalDevices(inst, &count, physDevices.data());
//
//     for(auto itr = physDevices.begin(); itr != physDevices.end();) {
//       if(!func(*itr)) {
//         itr = physDevices.erase(itr);
//       } else {
//         ++itr;
//       }
//     }
//
//     return physDevices;
//   }
//
//   Device* Instance::getDevice(const std::string &name) const {
//     auto itr = devices.find(name);
//     if (itr != devices.end()) return itr->second;
//
//     return nullptr;
//   }
//
//   void Instance::create(const uint32_t &apiVersion, const std::string &appName, const uint32_t &appVersion, const std::string &enName, const uint32_t &engineVersion) {
//     VkApplicationInfo appInfo{
//       VK_STRUCTURE_TYPE_APPLICATION_INFO,
//       nullptr,
//       appName.c_str(),
//       appVersion,
//       enName.c_str(),
//       engineVersion,
//       apiVersion
//     };
//
//     VkInstanceCreateInfo info{
//       VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
//       nullptr,
//       0,
//       &appInfo,
//       0, nullptr,
//       0, nullptr
//     };
//
// // #ifdef YAVF_DEBUG_REPORT_EXTENSION
// //       extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
//       extensions.push_back("VK_EXT_debug_utils");
// // #endif
//
//     std::vector<VkLayerProperties> lay;
//     std::vector<const char*> layerNames;
//     std::vector<VkExtensionProperties> ext;
//     std::vector<const char*> extensionNames;
//
//     lay = getRequiredValidationLayers(layers);
//     ext = getRequiredExtensions(layers, extensions);
//
//     YAVF_INFO_REPORT("Using layers:")
//     for (const auto &layer : lay) {
//       layerNames.push_back(layer.layerName);
//       YAVF_INFO_REPORT(("Name: "+std::string(layer.layerName)).c_str())
//       YAVF_INFO_REPORT(("Description: "+std::string(layer.description)).c_str())
//       YAVF_INFO_REPORT(("Version: "+VK_VERSION_TO_STRING(layer.specVersion)).c_str())
//       YAVF_INFO_REPORT(("Implementation version: "+VK_VERSION_TO_STRING(layer.implementationVersion)).c_str())
//     }
//
//     YAVF_INFO_REPORT("")
//     YAVF_INFO_REPORT("Using extensions:")
//     for (const auto &extension : ext) {
//       extensionNames.push_back(extension.extensionName);
//       YAVF_INFO_REPORT(("Name: "+std::string(extension.extensionName)).c_str())
//       YAVF_INFO_REPORT(("Version: "+VK_VERSION_TO_STRING(extension.specVersion)).c_str())
//     }
//
//     info.enabledExtensionCount = extensionNames.size();
//     info.ppEnabledExtensionNames = extensionNames.data();
//
//     info.enabledLayerCount = layerNames.size();
//     info.ppEnabledLayerNames = layerNames.data();
//
//     vkCheckError("vkCreateInstance", nullptr,
//     vkCreateInstance(&info, nullptr, &inst));
//
// // #ifdef YAVF_DEBUG_REPORT_EXTENSION
// //       VkDebugReportCallbackCreateInfoEXT createInfo{
// //         VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
// //         nullptr,
// //         VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
// //         (PFN_vkDebugReportCallbackEXT) &debugCallback_f,
// //         nullptr
// //       };
//
//       VkDebugUtilsMessengerCreateInfoEXT messengerinfo{
//         VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
//         nullptr,
//         0,
//         VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
//         VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
// //         VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
// //         VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
//         VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
//         VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
//         VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
//         (PFN_vkDebugUtilsMessengerCallbackEXT) &utilsMessengerCallback,
//         nullptr
//       };
//
// //       vkCheckError("vkCreateDebugReportCallbackEXT", nullptr,
// //       CreateDebugReportCallbackEXT(inst, &createInfo, nullptr, &callback));
//
//       vkCheckError("vkCreateDebugUtilsMessengerEXT", nullptr,
//       CreateDebugUtilsMessengerEXT(inst, &messengerinfo, nullptr, &messenger));
// // #endif
//   }
//
//   VkInstance Instance::handle() const {
//     return inst;
//   }
//
//   std::vector<const char*> Instance::layers;
//   std::vector<const char*> Instance::extensions;
// }
