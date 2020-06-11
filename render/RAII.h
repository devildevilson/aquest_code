#ifndef RAII_H
#define RAII_H

#include <string>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include "Internal.h"

namespace yavf {
  class Device;
  
  namespace raii {
    struct vkCreateShaderModule {
      VkResult operator()(VkDevice device, const VkShaderModuleCreateInfo* info, const VkAllocationCallbacks* allocator, VkShaderModule* module) const;
    };
    
    struct vkCreateBufferView {
      VkResult operator()(VkDevice device, const VkBufferViewCreateInfo* info, const VkAllocationCallbacks* allocator, VkBufferView* bufferView) const;
    };
    
    struct vkCreateCommandPool {
      VkResult operator()(VkDevice device, const VkCommandPoolCreateInfo* info, const VkAllocationCallbacks* allocator, VkCommandPool* commandPool) const;
    };
    
    struct vkCreateComputePipelines {
      VkResult operator()(VkDevice device, VkPipelineCache cache, const uint32_t &infoCount, const VkComputePipelineCreateInfo* infos, const VkAllocationCallbacks* allocator, VkPipeline* pipelines) const;
    };
    
    struct vkCreateDescriptorPool {
      VkResult operator()(VkDevice device, const VkDescriptorPoolCreateInfo* info, const VkAllocationCallbacks* allocator, VkDescriptorPool* pool) const;
    };
    
    struct vkCreateDescriptorSetLayout {
      VkResult operator()(VkDevice device, const VkDescriptorSetLayoutCreateInfo* info, const VkAllocationCallbacks* allocator, VkDescriptorSetLayout* setLayout) const;
    };
    
    struct vkCreateDescriptorUpdateTemplate {
      VkResult operator()(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* info, const VkAllocationCallbacks* allocator, VkDescriptorUpdateTemplate* updateTemplate) const;
    };
    
    struct vkCreateDevice {
      VkResult operator()(VkPhysicalDevice physDev, const VkDeviceCreateInfo* info, const VkAllocationCallbacks* allocator, VkDevice* device) const;
    };
    
    struct vkCreateDisplayPlaneSurfaceKHR {
      VkResult operator()(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* info, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface) const;
    };
    
    struct vkCreateEvent {
      VkResult operator()(VkDevice device, const VkEventCreateInfo* info, const VkAllocationCallbacks* allocator, VkEvent* event) const;
    };
    
    struct vkCreateFence {
      VkResult operator()(VkDevice device, const VkFenceCreateInfo* info, const VkAllocationCallbacks* allocator, VkFence* fence) const;
    };
    
    struct vkCreateFramebuffer {
      VkResult operator()(VkDevice device, const VkFramebufferCreateInfo* info, const VkAllocationCallbacks* allocator, VkFramebuffer* framebuffer) const;
    };
    
    struct vkCreateGraphicsPipelines {
      VkResult operator()(VkDevice device, VkPipelineCache cache, const uint32_t &infoCount, const VkGraphicsPipelineCreateInfo* infos, const VkAllocationCallbacks* allocator, VkPipeline* pipelines) const;
    };
    
    struct vkCreateImageView {
      VkResult operator()(VkDevice device, const VkImageViewCreateInfo* info, const VkAllocationCallbacks* allocator, VkImageView* imageView) const;
    };
    
    struct vkCreateInstance {
      VkResult operator()(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance) const;
    };
    
    struct vkCreatePipelineCache {
      VkResult operator()(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache) const;
    };
    
    struct vkCreatePipelineLayout {
      VkResult operator()(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout) const;
    };
    
    struct vkCreateQueryPool {
      VkResult operator()(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool) const;
    };
    
    struct vkCreateRenderPass {
      VkResult operator()(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) const;
    };
    
    struct vkCreateSampler {
      VkResult operator()(VkDevice device, const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSampler* pSampler) const;
    };
    
    struct vkCreateSamplerYcbcrConversion {
      VkResult operator()(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSamplerYcbcrConversion* pYcbcrConversion) const;
    };
    
    struct vkCreateSemaphore {
      VkResult operator()(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore) const;
    };
    
    struct vkCreateSwapchainKHR {
      VkResult operator()(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) const;
    };
    
    struct vmaCreateBuffer {
      VkResult operator()(VmaAllocator allocator, const VkBufferCreateInfo* pBufferCreateInfo, const VmaAllocationCreateInfo* pAllocationCreateInfo, VkBuffer* pBuffer, VmaAllocation* pAllocation, VmaAllocationInfo* pAllocationInfo) const;
    };
    
    struct vmaCreateImage {
      VkResult operator()(VmaAllocator allocator, const VkImageCreateInfo* pImageCreateInfo, const VmaAllocationCreateInfo* pAllocationCreateInfo, VkImage* pImage, VmaAllocation* pAllocation, VmaAllocationInfo* pAllocationInfo) const;
    };
    
    struct vkDestroyShaderModule {
      void operator()(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator) const;
    };
    
    struct vkDestroyBufferView {
      void operator()(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator) const;
    };
    
    struct vkDestroyCommandPool {
      void operator()(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator) const;
    };
    
    struct vkDestroyPipeline {
      void operator()(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator) const;
    };
    
    struct vkDestroyDescriptorPool {
      void operator()(VkDevice device, VkDescriptorPool pool, const VkAllocationCallbacks* pAllocator) const;
    };
    
    struct vkDestroyDescriptorSetLayout {
      void operator()(VkDevice device, VkDescriptorSetLayout setLayout, const VkAllocationCallbacks* pAllocator) const;
    };
    
    struct vkDestroyDescriptorUpdateTemplate {
      void operator()(VkDevice device, VkDescriptorUpdateTemplate updateTemplate, const VkAllocationCallbacks* pAllocator) const;
    };
    
    struct vkDestroyDevice {
      void operator()(VkDevice device, const VkAllocationCallbacks* pAllocator) const;
    };
    
    struct vkDestroySurfaceKHR {
      void operator()(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator) const;
    };
    
    struct vkDestroyEvent {
      void operator()(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator) const;
    };
    
    struct vkDestroyFence {
      void operator()(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator) const;
    };
    
    struct vkDestroyFramebuffer {
      void operator()(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator) const;
    };
    
    struct vkDestroyImageView {
      void operator()(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator) const;
    };
    
    struct vkDestroyInstance {
      void operator()(VkInstance instance, const VkAllocationCallbacks* pAllocator) const;
    };
    
    struct vkDestroyPipelineCache {
      void operator()(VkDevice device, VkPipelineCache cache, const VkAllocationCallbacks* pAllocator) const;
    };
    
    struct vkDestroyPipelineLayout {
      void operator()(VkDevice device, VkPipelineLayout layout, const VkAllocationCallbacks* pAllocator) const;
    };
    
    struct vkDestroyQueryPool {
      void operator()(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator) const;
    };
    
    struct vkDestroyRenderPass {
      void operator()(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator) const;
    };
    
    struct vkDestroySampler {
      void operator()(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator) const;
    };
    
    struct vkDestroySamplerYcbcrConversion {
      void operator()(VkDevice device, VkSamplerYcbcrConversion samplerYcbcrConversion, const VkAllocationCallbacks* pAllocator) const;
    };
    
    struct vkDestroySemaphore {
      void operator()(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator) const;
    };
    
    struct vkDestroySwapchainKHR {
      void operator()(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator) const;
    };
    
    struct vmaDestroyBuffer {
      void operator()(VmaAllocator allocator, VkBuffer buffer, VmaAllocation allocation) const;
    };
    
    struct vmaDestroyImage {
      void operator()(VmaAllocator allocator, VkImage image, VmaAllocation allocation) const;
    };
    
    template <typename ObjType>
    class Copyable {
    public:
      Copyable(ObjType obj) : obj(obj) {}
      Copyable(const Copyable<ObjType> &copy) : obj(copy.obj) {}
      ~Copyable() {}
      
      Copyable & operator=(const Copyable<ObjType> &copy) {
        obj = copy.obj;
        return *this;
      }
      
      operator ObjType() const {
        return obj;
      }
      
      bool operator==(const Copyable<ObjType> &copy) const {
        return obj == copy.obj;
      }
      
      bool operator!=(const Copyable<ObjType> &copy) const {
        return obj != copy.obj;
      }
    protected:
      ObjType obj;
    };
    
    template <typename ObjType, typename ObjCreateInfo, typename CreateFuncType, typename DestroyFuncType>
    class RAIIType1 {
    public:
      RAIIType1() : d(VK_NULL_HANDLE), obj(VK_NULL_HANDLE) {}
      RAIIType1(VkDevice d) : d(d), obj(VK_NULL_HANDLE) {}
      
      RAIIType1(VkDevice d, const ObjCreateInfo &info, const char* funcName) 
      :
//        cfn(CreateFuncType()), dfn(DestroyFuncType()),
        d(d), obj(VK_NULL_HANDLE) {

        const auto fn = CreateFuncType();
        vkCheckError(funcName, nullptr,
                     fn(d, &info, nullptr, &obj));
      }
      
      RAIIType1(const RAIIType1 &another) = delete;
      RAIIType1(RAIIType1 &&another) {
//        cfn = another.cfn;
//        dfn = another.dfn;
        obj = another.obj;
        d = another.d;
        another.obj = VK_NULL_HANDLE;
      }
      
      ~RAIIType1() {
        const auto fn = DestroyFuncType();
        if (obj != VK_NULL_HANDLE) fn(d, obj, nullptr);
      }
      
      RAIIType1 & operator=(const RAIIType1 &another) = delete;
      RAIIType1 & operator=(RAIIType1 &&another) {
        if (this == &another) return *this;
        
//        cfn = another.cfn;
//        dfn = another.dfn;
        obj = another.obj;
        d = another.d;
        another.obj = VK_NULL_HANDLE;
        return *this;
      }
      
      VkResult construct(VkDevice d, const ObjCreateInfo &info) {
        this->d = d;
        const auto fn = CreateFuncType();
        return fn(d, &info, nullptr, &obj);
      }
      
      VkResult construct(const ObjCreateInfo &info) {
        const auto fn = CreateFuncType();
        return fn(d, &info, nullptr, &obj);
      }
      
      operator ObjType() const {
        return obj;
      }
    private:
//      CreateFuncType cfn;
//      DestroyFuncType dfn;
    protected:
      VkDevice d;
      ObjType obj;
    };
    
    template <typename ObjType, typename ObjCreateInfo, typename CreateFuncType, typename DestroyFuncType>
    class RAIIType2 {
    public:
      RAIIType2() : d(VK_NULL_HANDLE), cache(VK_NULL_HANDLE), obj(VK_NULL_HANDLE) {}
      RAIIType2(VkDevice d) : d(d), cache(VK_NULL_HANDLE), obj(VK_NULL_HANDLE) {}
      RAIIType2(VkDevice d, VkPipelineCache cache) : d(d), cache(cache), obj(VK_NULL_HANDLE) {}
      
      RAIIType2(VkDevice d, VkPipelineCache cache, const ObjCreateInfo &info, const char* funcName) 
      : d(d), cache(cache), obj(VK_NULL_HANDLE) {
        const auto fn = CreateFuncType();
        vkCheckError(funcName, nullptr,
                     fn(d, cache, 1, &info, nullptr, &obj));
      }
      
      RAIIType2(const RAIIType2 &another) = delete;
      RAIIType2(RAIIType2 &&another) {
//        cfn = another.cfn;
//        dfn = another.dfn;
        obj = another.obj;
        cache = another.cache;
        d = another.d;
        another.obj = VK_NULL_HANDLE;
      }
      
      ~RAIIType2() {
        const auto fn = DestroyFuncType();
        if (obj != VK_NULL_HANDLE) fn(d, obj, nullptr);
      }
      
      RAIIType2 & operator=(const RAIIType2 &another) = delete;
      RAIIType2 & operator=(RAIIType2 &&another) {
        if (this == &another) return *this;
        
//        cfn = another.cfn;
//        dfn = another.dfn;
        obj = another.obj;
        cache = another.cache;
        d = another.d;
        another.obj = VK_NULL_HANDLE;
        return *this;
      }
      
      VkResult construct(VkDevice d, const ObjCreateInfo &info) {
        this->d = d;
        const auto fn = CreateFuncType();
        return fn(d, cache, 1, &info, nullptr, &obj);
      }
      
      VkResult construct(VkDevice d, VkPipelineCache cache, const ObjCreateInfo &info) {
        this->d = d;
        this->cache = cache;
        const auto fn = CreateFuncType();
        return fn(d, cache, 1, &info, nullptr, &obj);
      }
      
      VkResult construct(const ObjCreateInfo &info) {
        const auto fn = CreateFuncType();
        return fn(d, cache, 1, &info, nullptr, &obj);
      }
      
      operator ObjType() const {
        return obj;
      }
    private:
//      CreateFuncType cfn;
//      DestroyFuncType dfn;
    protected:
      VkDevice d;
      VkPipelineCache cache;
      ObjType obj;
    };
    
    template <typename ObjType, typename ObjCreateInfo, typename CreateFuncType, typename DestroyFuncType>
    class RAIIType3 {
    public:
      RAIIType3() : obj(VK_NULL_HANDLE) {}
      
      RAIIType3(const ObjCreateInfo &info, const char* funcName) 
      : obj(VK_NULL_HANDLE) { //, createFuncPtr(createFuncPtr), destroyFuncPtr(destroyFuncPtr) {
        const auto fn = CreateFuncType();
        vkCheckError(funcName, nullptr, 
        fn(&info, nullptr, &obj));
      }
      
      RAIIType3(const RAIIType3 &another) = delete;
      RAIIType3(RAIIType3 &&another) {
//        cfn = another.cfn;
//        dfn = another.dfn;
        obj = another.obj;
        another.obj = VK_NULL_HANDLE;
      }
      
      ~RAIIType3() {
        const auto fn = DestroyFuncType();
        if (obj != VK_NULL_HANDLE) fn(obj, nullptr);
      }
      
      RAIIType3 & operator=(const RAIIType3 &another) = delete;
      RAIIType3 & operator=(RAIIType3 &&another) {
        if (this == &another) return *this;
        
//        cfn = another.cfn;
//        dfn = another.dfn;
        obj = another.obj;
        another.obj = VK_NULL_HANDLE;
        return *this;
      }
      
      VkResult construct(const ObjCreateInfo &info) {
        const auto fn = CreateFuncType();
        return fn(&info, nullptr, &obj);
      }
      
      operator ObjType() const {
        return obj;
      }
    private:
//      CreateFuncType cfn;
//      DestroyFuncType dfn;
    protected:
      ObjType obj;
    };
    
    template <typename ObjType, typename ObjCreateInfo, typename CreateFuncType, typename DestroyFuncType>
    class RAIIType4 {
    public:
      RAIIType4() : allocator(VK_NULL_HANDLE), allocation(VK_NULL_HANDLE), obj(VK_NULL_HANDLE) {}
      RAIIType4(VmaAllocator allocator) : allocator(allocator), allocation(VK_NULL_HANDLE), obj(VK_NULL_HANDLE) {}
      
      RAIIType4(VmaAllocator allocator, const ObjCreateInfo &info, const VmaAllocationCreateInfo &pAllocationCreateInfo, const char* funcName) 
      : allocator(allocator), allocation(VK_NULL_HANDLE), obj(VK_NULL_HANDLE) { //, createFuncPtr(createFuncPtr), destroyFuncPtr(destroyFuncPtr) {
        const auto fn = CreateFuncType();
        vkCheckError(funcName, nullptr, 
                     fn(allocator,  &info, &pAllocationCreateInfo, &obj, &allocation, nullptr));
      }
      
      RAIIType4(const RAIIType4 &another) = delete;
      RAIIType4(RAIIType4 &&another) {
//        cfn = another.cfn;
//        dfn = another.dfn;
        obj = another.obj;
        allocation = another.allocation;
        allocator = another.allocator;
        another.obj = VK_NULL_HANDLE;
      }
      
      ~RAIIType4() {
        const auto fn = DestroyFuncType();
        if (obj != VK_NULL_HANDLE) fn(allocator, obj, allocation);
      }
      
      RAIIType4 & operator=(const RAIIType4 &another) = delete;
      RAIIType4 & operator=(RAIIType4 &&another) {
        if (this == &another) return *this;
        
//        cfn = another.cfn;
//        dfn = another.dfn;
        obj = another.obj;
        allocation = another.allocation;
        allocator = another.allocator;
        another.obj = VK_NULL_HANDLE;
        return *this;
      }
      
      VkResult construct(VmaAllocator allocator, const ObjCreateInfo &info, const VmaAllocationCreateInfo &pAllocationCreateInfo, VmaAllocationInfo *pAllocationInfo) {
        this->allocator = allocator;
        const auto fn = CreateFuncType();
        return fn(allocator,  &info, &pAllocationCreateInfo, &obj, &allocation, pAllocationInfo);
      }
      
      VkResult construct(const ObjCreateInfo &info, const VmaAllocationCreateInfo &pAllocationCreateInfo, VmaAllocationInfo *pAllocationInfo) {
        const auto fn = CreateFuncType();
        return fn(allocator,  &info, &pAllocationCreateInfo, &obj, &allocation, pAllocationInfo);
      }
      
      operator ObjType() const {
        return obj;
      }
    private:
//      CreateFuncType cfn;
//      DestroyFuncType dfn;
    protected:
      VmaAllocator allocator;
      VmaAllocation allocation;
      ObjType obj;
    };
    
    template <typename ObjType, typename DestroyFuncType>
    class RAIIType5 {
    public:
      RAIIType5(VkDevice d, ObjType obj) : d(d), obj(obj) {}
      RAIIType5(const RAIIType5 &another) = delete;
      RAIIType5(RAIIType5 &&another) {
//        dfn = another.dfn;
        obj = another.obj;
        another.obj = VK_NULL_HANDLE;
      }
      
      ~RAIIType5() {
        const auto fn = DestroyFuncType();
        if (obj != VK_NULL_HANDLE) fn(d, obj, nullptr);
      }
      
      RAIIType5 & operator=(const RAIIType5 &another) = delete;
      RAIIType5 & operator=(RAIIType5 &&another) {
        if (this == &another) return *this;
        
//        dfn = another.dfn;
        obj = another.obj;
        d = another.d;
        another.obj = VK_NULL_HANDLE;
        return *this;
      }
      
      operator ObjType() const {
        return obj;
      }
    private:
//      DestroyFuncType dfn;
    protected:
      VkDevice d;
      ObjType obj;
    };
    
    class ShaderModule : public RAIIType1<VkShaderModule, VkShaderModuleCreateInfo, vkCreateShaderModule, vkDestroyShaderModule> {
    public:
      ShaderModule();
      ShaderModule(Device* d, const VkShaderModuleCreateInfo &info);
      ShaderModule(Device* d, const char* path);
      ShaderModule(Device* d, const std::string &path);
    };
    
    inline VkResult vkCreateShaderModule::operator()(VkDevice device, const VkShaderModuleCreateInfo* info, const VkAllocationCallbacks* allocator, VkShaderModule* module) const {
      return ::vkCreateShaderModule(device, info, allocator, module);
    }
    
    inline VkResult vkCreateBufferView::operator()(VkDevice device, const VkBufferViewCreateInfo* info, const VkAllocationCallbacks* allocator, VkBufferView* bufferView) const {
      return ::vkCreateBufferView(device, info, allocator, bufferView);
    }
  
    inline VkResult vkCreateCommandPool::operator()(VkDevice device, const VkCommandPoolCreateInfo* info, const VkAllocationCallbacks* allocator, VkCommandPool* commandPool) const {
      return ::vkCreateCommandPool(device, info, allocator, commandPool);
    }
  
    inline VkResult vkCreateComputePipelines::operator()(VkDevice device, VkPipelineCache cache, const uint32_t &infoCount, const VkComputePipelineCreateInfo* infos, const VkAllocationCallbacks* allocator, VkPipeline* pipelines) const {
      return ::vkCreateComputePipelines(device, cache, infoCount, infos, allocator, pipelines);
    }
  
    inline VkResult vkCreateDescriptorPool::operator()(VkDevice device, const VkDescriptorPoolCreateInfo* info, const VkAllocationCallbacks* allocator, VkDescriptorPool* pool) const {
      return ::vkCreateDescriptorPool(device, info, allocator, pool);
    }
  
    inline VkResult vkCreateDescriptorSetLayout::operator()(VkDevice device, const VkDescriptorSetLayoutCreateInfo* info, const VkAllocationCallbacks* allocator, VkDescriptorSetLayout* setLayout) const {
      return ::vkCreateDescriptorSetLayout(device, info, allocator, setLayout);
    }
    
    inline VkResult vkCreateDescriptorUpdateTemplate::operator()(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* info, const VkAllocationCallbacks* allocator, VkDescriptorUpdateTemplate* updateTemplate) const {
      return ::vkCreateDescriptorUpdateTemplate(device, info, allocator, updateTemplate);
    }
  
    inline VkResult vkCreateDevice::operator()(VkPhysicalDevice physDev, const VkDeviceCreateInfo* info, const VkAllocationCallbacks* allocator, VkDevice* device) const {
      return ::vkCreateDevice(physDev, info, allocator, device);
    }
  
    inline VkResult vkCreateDisplayPlaneSurfaceKHR::operator()(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* info, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface) const {
      return ::vkCreateDisplayPlaneSurfaceKHR(instance, info, allocator, surface);
    }
  
    inline VkResult vkCreateEvent::operator()(VkDevice device, const VkEventCreateInfo* info, const VkAllocationCallbacks* allocator, VkEvent* event) const {
      return ::vkCreateEvent(device, info, allocator, event);
    }
  
    inline VkResult vkCreateFence::operator()(VkDevice device, const VkFenceCreateInfo* info, const VkAllocationCallbacks* allocator, VkFence* fence) const {
      return ::vkCreateFence(device, info, allocator, fence);
    }
  
    inline VkResult vkCreateFramebuffer::operator()(VkDevice device, const VkFramebufferCreateInfo* info, const VkAllocationCallbacks* allocator, VkFramebuffer* framebuffer) const {
      return ::vkCreateFramebuffer(device, info, allocator, framebuffer);
    }
  
    inline VkResult vkCreateGraphicsPipelines::operator()(VkDevice device, VkPipelineCache cache, const uint32_t &infoCount, const VkGraphicsPipelineCreateInfo* infos, const VkAllocationCallbacks* allocator, VkPipeline* pipelines) const {
      return ::vkCreateGraphicsPipelines(device, cache, infoCount, infos, allocator, pipelines);
    }
  
    inline VkResult vkCreateImageView::operator()(VkDevice device, const VkImageViewCreateInfo* info, const VkAllocationCallbacks* allocator, VkImageView* imageView) const {
      return ::vkCreateImageView(device, info, allocator, imageView);
    }
  
    inline VkResult vkCreateInstance::operator()(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance) const {
      return ::vkCreateInstance(pCreateInfo, pAllocator, pInstance);
    }

    inline VkResult vkCreatePipelineCache::operator()(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache) const {
      return ::vkCreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);
    }
  
    inline VkResult vkCreatePipelineLayout::operator()(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout) const {
      return ::vkCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout);
    }
  
    inline VkResult vkCreateQueryPool::operator()(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool) const {
      return ::vkCreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
    }

    inline VkResult vkCreateRenderPass::operator()(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) const {
      return ::vkCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
    }
  
    inline VkResult vkCreateSampler::operator()(VkDevice device, const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSampler* pSampler) const {
      return ::vkCreateSampler(device, pCreateInfo, pAllocator, pSampler);
    }
  
    inline VkResult vkCreateSamplerYcbcrConversion::operator()(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSamplerYcbcrConversion* pYcbcrConversion) const {
      return ::vkCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);
    }
  
    inline VkResult vkCreateSemaphore::operator()(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore) const {
      return ::vkCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
    }
  
    inline VkResult vkCreateSwapchainKHR::operator()(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) const {
      return ::vkCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);
    }
  
    inline VkResult vmaCreateBuffer::operator()(VmaAllocator allocator, const VkBufferCreateInfo* pBufferCreateInfo, const VmaAllocationCreateInfo* pAllocationCreateInfo, VkBuffer* pBuffer, VmaAllocation* pAllocation, VmaAllocationInfo* pAllocationInfo) const {
      return ::vmaCreateBuffer(allocator, pBufferCreateInfo, pAllocationCreateInfo, pBuffer, pAllocation, pAllocationInfo);
    }
  
    inline VkResult vmaCreateImage::operator()(VmaAllocator allocator, const VkImageCreateInfo* pImageCreateInfo, const VmaAllocationCreateInfo* pAllocationCreateInfo, VkImage* pImage, VmaAllocation* pAllocation, VmaAllocationInfo* pAllocationInfo) const {
      return ::vmaCreateImage(allocator, pImageCreateInfo, pAllocationCreateInfo, pImage, pAllocation, pAllocationInfo);
    }
  
    inline void vkDestroyShaderModule::operator()(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator) const {
      ::vkDestroyShaderModule(device, shaderModule, pAllocator);
    }
  
    inline void vkDestroyBufferView::operator()(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator) const {
      ::vkDestroyBufferView(device, bufferView, pAllocator);
    }
  
    inline void vkDestroyCommandPool::operator()(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator) const {
      ::vkDestroyCommandPool(device, commandPool, pAllocator);
    }
  
    inline void vkDestroyPipeline::operator()(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator) const {
      ::vkDestroyPipeline(device, pipeline, pAllocator);
    }
  
    inline void vkDestroyDescriptorPool::operator()(VkDevice device, VkDescriptorPool pool, const VkAllocationCallbacks* pAllocator) const {
      ::vkDestroyDescriptorPool(device, pool, pAllocator);
    }
  
    inline void vkDestroyDescriptorSetLayout::operator()(VkDevice device, VkDescriptorSetLayout setLayout, const VkAllocationCallbacks* pAllocator) const {
      ::vkDestroyDescriptorSetLayout(device, setLayout, pAllocator);
    }

    inline void vkDestroyDescriptorUpdateTemplate::operator()(VkDevice device, VkDescriptorUpdateTemplate updateTemplate, const VkAllocationCallbacks* pAllocator) const {
      ::vkDestroyDescriptorUpdateTemplate(device, updateTemplate, pAllocator);
    }
  
    inline void vkDestroyDevice::operator()(VkDevice device, const VkAllocationCallbacks* pAllocator) const {
      ::vkDestroyDevice(device, pAllocator);
    }
  
    inline void vkDestroySurfaceKHR::operator()(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator) const {
      ::vkDestroySurfaceKHR(instance, surface, pAllocator);
    }
  
    inline void vkDestroyEvent::operator()(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator) const {
      ::vkDestroyEvent(device, event, pAllocator);
    }
  
    inline void vkDestroyFence::operator()(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator) const {
      ::vkDestroyFence(device, fence, pAllocator);
    }
  
    inline void vkDestroyFramebuffer::operator()(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator) const {
      ::vkDestroyFramebuffer(device, framebuffer, pAllocator);
    }
  
    inline void vkDestroyImageView::operator()(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator) const {
      ::vkDestroyImageView(device, imageView, pAllocator);
    }
  
    inline void vkDestroyInstance::operator()(VkInstance instance, const VkAllocationCallbacks* pAllocator) const {
      ::vkDestroyInstance(instance, pAllocator);
    }
  
    inline void vkDestroyPipelineCache::operator()(VkDevice device, VkPipelineCache cache, const VkAllocationCallbacks* pAllocator) const {
      ::vkDestroyPipelineCache(device, cache, pAllocator);
    }
  
    inline void vkDestroyPipelineLayout::operator()(VkDevice device, VkPipelineLayout layout, const VkAllocationCallbacks* pAllocator) const {
      ::vkDestroyPipelineLayout(device, layout, pAllocator);
    }
  
    inline void vkDestroyQueryPool::operator()(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator) const {
      ::vkDestroyQueryPool(device, queryPool, pAllocator);
    }
  
    inline void vkDestroyRenderPass::operator()(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator) const {
      ::vkDestroyRenderPass(device, renderPass, pAllocator);
    }
  
    inline void vkDestroySampler::operator()(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator) const {
      ::vkDestroySampler(device, sampler, pAllocator);
    }
  
    inline void vkDestroySamplerYcbcrConversion::operator()(VkDevice device, VkSamplerYcbcrConversion samplerYcbcrConversion, const VkAllocationCallbacks* pAllocator) const {
      ::vkDestroySamplerYcbcrConversion(device, samplerYcbcrConversion, pAllocator);
    }
  
    inline void vkDestroySemaphore::operator()(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator) const {
      ::vkDestroySemaphore(device, semaphore, pAllocator);
    }
  
    inline void vkDestroySwapchainKHR::operator()(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator) const {
      ::vkDestroySwapchainKHR(device, swapchain, pAllocator);
    }
  
    inline void vmaDestroyBuffer::operator()(VmaAllocator allocator, VkBuffer buffer, VmaAllocation allocation) const {
      ::vmaDestroyBuffer(allocator, buffer, allocation);
    }
  
    inline void vmaDestroyImage::operator()(VmaAllocator allocator, VkImage image, VmaAllocation allocation) const {
      ::vmaDestroyImage(allocator, image, allocation);
    }
  }
}
  
#endif
