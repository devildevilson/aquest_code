#include "RAII.h"

#include "Internal.h"
#include "Core.h"
#include <fstream>

namespace yavf {
  namespace raii {
    ShaderModule::ShaderModule() : RAIIType1() {}
    ShaderModule::ShaderModule(Device* d, const VkShaderModuleCreateInfo &info) : RAIIType1(d->handle(), info, "vkCreateShaderModule") {}
    
    ShaderModule::ShaderModule(Device* d, const char* path) : RAIIType1(d->handle()) {
      auto file = std::ifstream(std::string(path), std::ios::binary);
      
      if (!file) {
        YAVF_ERROR_REPORT("addShader", "bad file path", 0)
      }
      
      file.seekg(0, std::ios::end);
      size_t length = file.tellg();
      
      std::vector<char> opcode(length);
      file.seekg(0, std::ios::beg);
      file.read(opcode.data(), opcode.size());
      
      const VkShaderModuleCreateInfo info{
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        nullptr,
        0,
        opcode.size(),
        (uint32_t*)opcode.data()
      };
      
      vkCheckError("vkCreateShaderModule", nullptr, 
      construct(d->handle(), info));
    }
    
    ShaderModule::ShaderModule(Device* d, const std::string &path) : RAIIType1(d->handle()) {
      auto file = std::ifstream(path, std::ios::binary);
      
      if (!file) {
        YAVF_ERROR_REPORT("addShader", "bad file path", 0)
      }
      
      file.seekg(0, std::ios::end);
      size_t length = file.tellg();
      
      std::vector<char> opcode(length);
      file.seekg(0, std::ios::beg);
      file.read(opcode.data(), opcode.size());
      
      const VkShaderModuleCreateInfo info{
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        nullptr,
        0,
        opcode.size(),
        (uint32_t*)opcode.data()
      };
      
      vkCheckError("vkCreateShaderModule", nullptr, 
      construct(d->handle(), info));
    }
  }
}
  

//   #define FUNC_NAME_ENUM(func) e##func
//   #define FUNC_POINTER(func) reinterpret_cast<void*>(func)
//   #define GET_FUNC_POINTER(type, func) (reinterpret_cast<type>(func))
//     
//     enum VulkanFuncEnum {
//       FUNC_NAME_ENUM(vkCreateShaderModule),
//       FUNC_NAME_ENUM(vkCreateBufferView),
//       FUNC_NAME_ENUM(vkCreateCommandPool),
//       FUNC_NAME_ENUM(vkCreateComputePipelines),
//       FUNC_NAME_ENUM(vkCreateDescriptorPool),
//       FUNC_NAME_ENUM(vkCreateDescriptorSetLayout),
//       FUNC_NAME_ENUM(vkCreateDescriptorUpdateTemplate),
//       FUNC_NAME_ENUM(vkCreateDevice),
//   //     FUNC_NAME_ENUM(vkCreateDisplayModeKHR),
//       FUNC_NAME_ENUM(vkCreateDisplayPlaneSurfaceKHR),
//       FUNC_NAME_ENUM(vkCreateEvent),
//       FUNC_NAME_ENUM(vkCreateFence),
//       FUNC_NAME_ENUM(vkCreateFramebuffer),
//       FUNC_NAME_ENUM(vkCreateGraphicsPipelines),
//       FUNC_NAME_ENUM(vkCreateImageView),
//   //     FUNC_NAME_ENUM(vkCreateIndirectCommandsLayoutNVX),
//       FUNC_NAME_ENUM(vkCreateInstance),
//   //     FUNC_NAME_ENUM(vkCreateObjectTableNVX),
//       FUNC_NAME_ENUM(vkCreatePipelineCache),
//       FUNC_NAME_ENUM(vkCreatePipelineLayout),
//       FUNC_NAME_ENUM(vkCreateQueryPool),
//   //     FUNC_NAME_ENUM(vkCreateRayTracingPipelinesNV),
//       FUNC_NAME_ENUM(vkCreateRenderPass),
// //       FUNC_NAME_ENUM(vkCreateRenderPass2KHR),
//       FUNC_NAME_ENUM(vkCreateSampler),
//       FUNC_NAME_ENUM(vkCreateSamplerYcbcrConversion),
//       FUNC_NAME_ENUM(vkCreateSemaphore),
//   //     FUNC_NAME_ENUM(vkCreateSharedSwapchainsKHR),
//       FUNC_NAME_ENUM(vkCreateSwapchainKHR),
//   //     FUNC_NAME_ENUM(vkCreateValidationCacheEXT),     // вроде как для EXT надо загрузить указатели на функцию
//   //     FUNC_NAME_ENUM(vkCreateDebugReportCallbackEXT), // и поэтому здесь это не подходит
//   //     FUNC_NAME_ENUM(vkCreateDebugUtilsMessengerEXT),
//       FUNC_NAME_ENUM(vmaCreateBuffer),
//       FUNC_NAME_ENUM(vmaCreateImage),
//       VULKAN_FUNC_ENUM_COUNT
//     };
//     
//     struct FuncPointers {void* create; void* destroy;};
//     static const FuncPointers vulkanFuncPtr[] = {
//       {FUNC_POINTER(vkCreateShaderModule), FUNC_POINTER(vkDestroyShaderModule)},
//       {FUNC_POINTER(vkCreateBufferView), FUNC_POINTER(vkDestroyBufferView)},
//       {FUNC_POINTER(vkCreateCommandPool), FUNC_POINTER(vkDestroyCommandPool)},
//       {FUNC_POINTER(vkCreateComputePipelines), FUNC_POINTER(vkDestroyPipeline)},
//       {FUNC_POINTER(vkCreateDescriptorPool), FUNC_POINTER(vkDestroyDescriptorPool)},
//       {FUNC_POINTER(vkCreateDescriptorSetLayout), FUNC_POINTER(vkDestroyDescriptorSetLayout)},
//       {FUNC_POINTER(vkCreateDescriptorUpdateTemplate), FUNC_POINTER(vkDestroyDescriptorUpdateTemplate)},
//       {FUNC_POINTER(vkCreateDevice), FUNC_POINTER(vkDestroyDevice)},
//   //     {FUNC_POINTER(vkCreateDisplayModeKHR), FUNC_POINTER(vkDestroyDisplayModeKHR)},
//       {FUNC_POINTER(vkCreateDisplayPlaneSurfaceKHR), FUNC_POINTER(vkDestroySurfaceKHR)},
//       {FUNC_POINTER(vkCreateEvent), FUNC_POINTER(vkDestroyEvent)},
//       {FUNC_POINTER(vkCreateFence), FUNC_POINTER(vkDestroyFence)},
//       {FUNC_POINTER(vkCreateFramebuffer), FUNC_POINTER(vkDestroyFramebuffer)},
//       {FUNC_POINTER(vkCreateGraphicsPipelines), FUNC_POINTER(vkDestroyPipeline)},
//       {FUNC_POINTER(vkCreateImageView), FUNC_POINTER(vkDestroyImageView)},
//       {FUNC_POINTER(vkCreateInstance), FUNC_POINTER(vkDestroyInstance)},
//       {FUNC_POINTER(vkCreatePipelineCache), FUNC_POINTER(vkDestroyPipelineCache)},
//       {FUNC_POINTER(vkCreatePipelineLayout), FUNC_POINTER(vkDestroyPipelineLayout)},
//       {FUNC_POINTER(vkCreateQueryPool), FUNC_POINTER(vkDestroyQueryPool)},
//       {FUNC_POINTER(vkCreateRenderPass), FUNC_POINTER(vkDestroyRenderPass)},
// //       {FUNC_POINTER(vkCreateRenderPass2KHR), FUNC_POINTER(vkDestroyRenderPass)},
//       {FUNC_POINTER(vkCreateSampler), FUNC_POINTER(vkDestroySampler)},
//       {FUNC_POINTER(vkCreateSamplerYcbcrConversion), FUNC_POINTER(vkDestroySamplerYcbcrConversion)},
//       {FUNC_POINTER(vkCreateSemaphore), FUNC_POINTER(vkDestroySemaphore)},
//   //     {FUNC_POINTER(vkCreateSharedSwapchainsKHR), FUNC_POINTER(vkDestroySwapchainKHR)},
//       {FUNC_POINTER(vkCreateSwapchainKHR), FUNC_POINTER(vkDestroySwapchainKHR)},
//       {FUNC_POINTER(vmaCreateBuffer), FUNC_POINTER(vmaDestroyBuffer)},
//       {FUNC_POINTER(vmaCreateImage), FUNC_POINTER(vmaDestroyImage)}
//     };
//     
//     typedef VkResult (*vmaCreateImageType)(VmaAllocator, const VkImageCreateInfo*, const VmaAllocationCreateInfo*, VkImage*, VmaAllocation*, VmaAllocationInfo*);
//     typedef void (*vmaDestroyImageType)(VmaAllocator, VkImage, VmaAllocation);
//     typedef VkResult (*vmaCreateBufferType)(VmaAllocator, const VkBufferCreateInfo*, const VmaAllocationCreateInfo*, VkBuffer*, VmaAllocation*, VmaAllocationInfo*);
//     typedef void (*vmaDestroyBufferType)(VmaAllocator, VkBuffer, VmaAllocation);
