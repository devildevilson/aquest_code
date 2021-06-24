#include "Types.h"

#include "Core.h"
#include "Makers.h"

//#define VMA_DEBUG_INITIALIZE_ALLOCATIONS 1
//#define VMA_DEBUG_MARGIN 16
//#define VMA_DEBUG_DETECT_CORRUPTION 1
#define VMA_RECORDING_ENABLED 0
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

int getIntTypeFromDescType(const VkDescriptorType &descType) {
  switch (descType) {
//     case VK_DESCRIPTOR_TYPE_SAMPLER:
//     case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
//     case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
//     case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
//     case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
//       return 0;
//     case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
//     case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
//     case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
//     case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
//       return 1;
//     case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
//     case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
//       return 2;
//     default:
//       throw std::runtime_error("This descriptor type isnot supported");
    
    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
      return 2;
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
      return 1;
    default:
      return 0;
  };
}

namespace yavf {
//   Buffer::Buffer() {
//     desc.handle = VK_NULL_HANDLE;
//     desc.arrayElement = UINT32_MAX;
//     desc.bindingNum = UINT32_MAX;
//     desc.type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
//   }
//   
//   Buffer::~Buffer() {}
//   
//   void Buffer::recreate(const size_t &newSize, const uint32_t &dataCount) {
//     parameters.dataCount = dataCount;
//     if (parameters.size >= newSize) return;
// 
//     bool hasView = false;
//     if (bufferView.handle != VK_NULL_HANDLE) {
//       vkDestroyBufferView(device->handle(), bufferView.handle, nullptr);
//       bufferView.handle = VK_NULL_HANDLE;
//       hasView = true;
//     }
//     
//     vmaDestroyBuffer(device->bufferAllocator(), handle, allocation);
//     
//     VmaAllocationInfo data;
//     
//     const size_t finalSize = memUsage == VMA_MEMORY_USAGE_CPU_ONLY ? 
//       alignMemorySize(std::max(device->getMinMemoryMapAlignment(), device->getNonCoherentAtomSize()), newSize) : 
//       //alignMemorySize(device->getNonCoherentAtomSize(), newSize);
//       newSize;
//       
// //     std::cout << "nonCoherentAtomSize   " << device->getNonCoherentAtomSize() << "\n";
// //     std::cout << "minMemoryMapAlignment " << device->getMinMemoryMapAlignment() << "\n";
// //     std::cout << "finalSize             " << finalSize << "\n";
// //     throw std::runtime_error(";ljvfkdpva;v'vawev'w");
//     
//     const VkBufferCreateInfo info{
//       VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
//       nullptr,
//       parameters.flags,
//       finalSize,
//       parameters.usage,
//       VK_SHARING_MODE_EXCLUSIVE,
//       0,
//       nullptr
//     };
//     
//     VmaAllocationCreateInfo alloc{
//       static_cast<VmaAllocationCreateFlags>(memUsage != VMA_MEMORY_USAGE_GPU_ONLY ? VMA_ALLOCATION_CREATE_MAPPED_BIT : 0),
//       memUsage,
//       0, 0, 0, VK_NULL_HANDLE, nullptr
//     };
//     
//     vkCheckError("vmaCreateBuffer", nullptr, 
//     vmaCreateBuffer(device->bufferAllocator(), &info, &alloc, &handle, &allocation, &data));
// 
//     if (hasView) {
//       createView(bufferView.format);
//     }
// 
//     if (desc.handle != VK_NULL_HANDLE) updateDescriptor();
//     
//     parameters.size = finalSize;
//     pointer = data.pMappedData;
//   }
// 
//   void Buffer::flush() const {
//     VmaAllocationInfo data;
//     
//     vmaGetAllocationInfo(device->bufferAllocator(), allocation, &data);
//     
//     VkMappedMemoryRange memRange{
//       VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
//       nullptr,
//       data.deviceMemory,
//       data.offset,
//       data.size
//     };
//     
//     vkFlushMappedMemoryRanges(device->handle(), 1, &memRange);
//   }
// 
//   void Buffer::createView(const VkFormat &format, const VkDeviceSize &offset, const VkDeviceSize &size) {
//     if (bufferView.handle != VK_NULL_HANDLE) {
//       vkDestroyBufferView(device->handle(), bufferView.handle, nullptr);
//       bufferView.handle = VK_NULL_HANDLE;
//     }
// 
//     VkBufferViewCreateInfo info{
//       VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
//       nullptr,
//       0,
//       handle,
//       format,
//       parameters.offset + offset,
//       size
//     };
//     
//     vkCheckError("vkCreateBufferView", nullptr, 
//     vkCreateBufferView(device->handle(), &info, nullptr, &bufferView.handle));
//   }
//   
//   void Buffer::setDescriptorData(const DescriptorUpdate &desc) {
//     this->desc = desc;
//     
//     updateDescriptor();
//   }
// 
// //   void Buffer::setDescriptor(const DescriptorUpdate &desc) {
// //     this->desc = desc;
// //   }
// 
//   // const Internal::BufferMeta & Buffer_t::meta() const {
//   //   return metaInf;
//   // }
// 
//   const Internal::BufferParameters & Buffer::param() const {
//     return parameters;
//   }
// 
//   VkBuffer Buffer::get() const {
//     return handle;
//   }
// 
//   Descriptor Buffer::descriptor() const {
//     return desc.handle;
//   }
// 
//   BufferView Buffer::view() const {
//     return bufferView;
//   }
// 
//   void* Buffer::ptr() const {
//     return pointer;
//   }
// 
//   void Buffer::updateDescriptor(uint32_t bindingNum, uint32_t arrayElement, VkDescriptorType typeEnum) {
//     if (desc.handle == VK_NULL_HANDLE) return;
//     
//     const VkDescriptorBufferInfo info{
//       handle,
//       parameters.offset,
//       parameters.size
//     };
//     
//     const VkWriteDescriptorSet write{
//       VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
//       nullptr,
//       desc.handle,
//       bindingNum == UINT32_MAX ? desc.bindingNum : bindingNum,
//       arrayElement == UINT32_MAX ? desc.arrayElement : arrayElement,
//       1,
//       typeEnum == VK_DESCRIPTOR_TYPE_MAX_ENUM ? desc.type : typeEnum,
//       nullptr,
//       &info,
//       nullptr
//     };
//     
//     vkUpdateDescriptorSets(device->handle(), 1, &write, 0, nullptr);
// 
//     desc.bindingNum = bindingNum == UINT32_MAX ? desc.bindingNum : bindingNum;
//     desc.arrayElement = arrayElement == UINT32_MAX ? desc.arrayElement : arrayElement;
//     desc.type = typeEnum == VK_DESCRIPTOR_TYPE_MAX_ENUM ? desc.type : typeEnum;
//   }
//   
//   Image::Image() {
//     desc.handle = VK_NULL_HANDLE;
//     desc.arrayElement = UINT32_MAX;
//     desc.bindingNum = UINT32_MAX;
//     desc.type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
//   }
//   
//   Image::~Image() {}
// 
//   void Image::recreate(const VkExtent3D &newSize) {
//     bool hasView = false;
//     if (imageView.handle != VK_NULL_HANDLE) {
//       vkDestroyImageView(device->handle(), imageView.handle, nullptr);
//       imageView.handle = VK_NULL_HANDLE;
//       hasView = true;
//     }
//     
//     vmaDestroyImage(device->imageAllocator(), handle, allocation);
//     
//     VmaAllocationInfo data;
// 
//     VkImageCreateInfo imageInfo{
//       VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
//       nullptr,
//       parameters.createFlags,
//       parameters.type,
//       parameters.format,
//       {newSize.width, newSize.height, newSize.depth > 0 ? newSize.depth : 1},
//       parameters.mipmaps,
//       parameters.layers,
//       parameters.samples,
//       parameters.tiling,
//       parameters.usage,
//       VK_SHARING_MODE_EXCLUSIVE,
//       0, nullptr,
//       VK_IMAGE_LAYOUT_UNDEFINED
//     };
// 
//     VmaAllocationCreateInfo alloc{
//       static_cast<VmaAllocationCreateFlags>(memUsage != VMA_MEMORY_USAGE_GPU_ONLY ? VMA_ALLOCATION_CREATE_MAPPED_BIT : 0),
//       memUsage,
//       0, 0, 0, VK_NULL_HANDLE, nullptr
//     };
//     
//     vkCheckError("vmaCreateImage", nullptr, 
//     vmaCreateImage(device->imageAllocator(), &imageInfo, &alloc, &handle, &allocation, &data));
// 
//     if (hasView) {
//       createView(imageView.type, imageView.aspect);
//     }
// 
//     if (desc.handle != VK_NULL_HANDLE) updateDescriptor();
//     
//     parameters.size = newSize;
//     pointer = data.pMappedData;
//     parameters.layout = VK_IMAGE_LAYOUT_UNDEFINED;
//   }
// 
//   void Image::flush() const {
//     VmaAllocationInfo data;
//     
//     vmaGetAllocationInfo(device->imageAllocator(), allocation, &data);
//     
//     VkMappedMemoryRange memRange{
//       VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
//       nullptr,
//       data.deviceMemory,
//       data.offset,
//       data.size
//     };
//     
//     vkFlushMappedMemoryRanges(device->handle(), 1, &memRange);
//   }
// 
//   void Image::createView(const VkImageViewType &type, const VkImageAspectFlags &flags, const uint32_t &baseArrayLayer, const uint32_t &layerCount,
//                            const VkComponentMapping &mapping, const VkFormat &format) {
//     if (imageView.handle != VK_NULL_HANDLE) {
//       vkDestroyImageView(device->handle(), imageView.handle, nullptr);
//       imageView.handle = VK_NULL_HANDLE;
//     }
//     
//     VkImageViewCreateInfo info{
//       VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
//       nullptr,
//       0,
//       handle,
//       type,
//       format == VK_FORMAT_UNDEFINED ? parameters.format : format,
//       mapping,
//       {
//         flags,
//         0, parameters.mipmaps,
//         baseArrayLayer == UINT32_MAX ? 0 : baseArrayLayer,
//         layerCount == UINT32_MAX ? parameters.layers : layerCount
//       }
//     };
//     
//     vkCheckError("vkCreateImageView", nullptr, 
//     vkCreateImageView(device->handle(), &info, nullptr, &imageView.handle));
//     
//     imageView.aspect = flags;
//     imageView.type = type;
//   }
// 
// //   void Image::setDescriptor(const DescriptorUpdate &desc) {
// //     this->desc = desc;
// //   }
// 
//   void Image::setDescriptorData(const DescriptorUpdate &desc) {
//     this->desc = desc;
//     
//     updateDescriptor();
//   }
// 
//   void Image::setSampler(Sampler sampler) {
//     this->samplerH = sampler.handle();
//   }
// 
//   // const Internal::ImageMeta & Image_t::meta() const {
//   //   return metaInf;
//   // }
// 
//   const Internal::ImageParameters & Image::param() const {
//     return parameters;
//   }
// 
//   VkImage Image::get() const {
//     return handle;
//   }
// 
//   Descriptor Image::descriptor() const {
//     return desc.handle;
//   }
// 
//   ImageView Image::view() const {
//     return imageView;
//   }
// 
//   VkSampler Image::sampler() const {
//     return samplerH;
//   }
// 
//   void* Image::ptr() const {
//     return pointer;
//   }
// 
//   void Image::updateDescriptor(uint32_t bindingNum, uint32_t arrayElement, VkDescriptorType typeEnum) {
// //     if (desc.handle == VK_NULL_HANDLE) return;
// //     if (imageView.handle == VK_NULL_HANDLE) return;
// //     if (parameters.layout == VK_IMAGE_LAYOUT_UNDEFINED) return;
//     
//     const VkDescriptorImageInfo info{
//       samplerH,
//       imageView.handle,
//       parameters.layout
//     };
//     
//     const VkWriteDescriptorSet write{
//       VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
//       nullptr,
//       desc.handle,
//       bindingNum == UINT32_MAX ? desc.bindingNum : bindingNum,
//       arrayElement == UINT32_MAX ? desc.arrayElement : arrayElement,
//       1,
//       typeEnum == VK_DESCRIPTOR_TYPE_MAX_ENUM ? desc.type : typeEnum,
//       &info,
//       nullptr,
//       nullptr
//     };
// 
//     vkUpdateDescriptorSets(device->handle(), 1, &write, 0, nullptr);
// 
//     desc.bindingNum = bindingNum == UINT32_MAX ? desc.bindingNum : bindingNum;
//     desc.arrayElement = arrayElement == UINT32_MAX ? desc.arrayElement : arrayElement;
//     desc.type = typeEnum == VK_DESCRIPTOR_TYPE_MAX_ENUM ? desc.type : typeEnum;
//   }
//   
//   Sampler::Sampler() {
//     desc.handle = VK_NULL_HANDLE;
//     desc.arrayElement = UINT32_MAX;
//     desc.bindingNum = UINT32_MAX;
//     desc.type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
//   }
// 
//   Sampler::Sampler(VkSampler sampler) {
//     this->sampler = sampler;
// 
//     desc.handle = VK_NULL_HANDLE;
//     desc.arrayElement = UINT32_MAX;
//     desc.bindingNum = UINT32_MAX;
//     desc.type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
//   }
//   
//   Sampler::~Sampler() {}
// 
//   VkSampler Sampler::handle() const {
//     return sampler;
//   }
// 
//   Descriptor Sampler::descriptor() const {
//     return desc.handle;
//   }
// 
//   void Sampler::setDescriptorData(const DescriptorUpdate &desc) {
//     this->desc = desc;
// 
//     updateDescriptor();
//   }
// 
//   void Sampler::updateDescriptor(uint32_t bindingNum, uint32_t arrayElement, VkDescriptorType typeEnum) {
//     if (desc.handle == VK_NULL_HANDLE) return;
// 
//     VkDescriptorImageInfo info{
//       sampler,
//       VK_NULL_HANDLE,
//       VK_IMAGE_LAYOUT_MAX_ENUM
//     };
// 
//     VkWriteDescriptorSet write{
//       VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
//       nullptr,
//       desc.handle,
//       bindingNum == UINT32_MAX ? desc.bindingNum : bindingNum,
//       arrayElement == UINT32_MAX ? desc.arrayElement : arrayElement,
//       1,
//       typeEnum == VK_DESCRIPTOR_TYPE_MAX_ENUM ? desc.type : typeEnum,
//       &info,
//       nullptr,
//       nullptr
//     };
// 
//     vkUpdateDescriptorSets(device->handle(), 1, &write, 0, nullptr);
// 
//     desc.bindingNum = bindingNum == UINT32_MAX ? desc.bindingNum : bindingNum;
//     desc.arrayElement = arrayElement == UINT32_MAX ? desc.arrayElement : arrayElement;
//     desc.type = typeEnum == VK_DESCRIPTOR_TYPE_MAX_ENUM ? desc.type : typeEnum;
//   }
  
  FramebufferCreateInfo FramebufferCreateInfo::framebuffer(const RenderPass &renderPass,
                                                           const uint32_t &attachmentCount,
                                                           const VkImageView* pAttachments,
                                                           const uint32_t &width,
                                                           const uint32_t &height,
                                                           const uint32_t &layers) {
    return FramebufferCreateInfo({
      VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
      nullptr,
      0,
      renderPass,
      attachmentCount,
      pAttachments,
      width,
      height,
      layers
    });
  }
  
  FramebufferCreateInfo::FramebufferCreateInfo() 
    : _info{
      VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
      nullptr,
      0,
      VK_NULL_HANDLE,
      0,
      nullptr,
      0,
      0,
      0
  } {}
  
  FramebufferCreateInfo::FramebufferCreateInfo(const VkFramebufferCreateInfo &info) : _info(info) {}
  
  FramebufferCreateInfo & FramebufferCreateInfo::operator=(const VkFramebufferCreateInfo &info) {
    _info = info;
    return *this;
  }
  
  FramebufferCreateInfo & FramebufferCreateInfo::operator=(const FramebufferCreateInfo &info) {
    _info = info._info;
    return *this;
  }
  
  FramebufferCreateInfo::operator VkFramebufferCreateInfo() const {
    return _info;
  }
  
  ImageCreateInfo ImageCreateInfo::texture2D(const VkExtent2D &size,
                                             const VkImageUsageFlags &usage,
                                             const VkFormat &format,
                                             const uint32_t &arrayLayers,
                                             const uint32_t &mipLevels,
                                             const VkSampleCountFlagBits &samples,
                                             const VkImageCreateFlags &flags) {
    return ImageCreateInfo({
      VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      nullptr,
      flags,
      VK_IMAGE_TYPE_2D,
      format,
      {size.width, size.height, 1},
      mipLevels,
      arrayLayers,
      samples,
      VK_IMAGE_TILING_OPTIMAL,
      usage,
      VK_SHARING_MODE_EXCLUSIVE,
      1,
      nullptr,
      VK_IMAGE_LAYOUT_UNDEFINED
    });
  }
  
  ImageCreateInfo ImageCreateInfo::texture2DStaging(const VkExtent2D &size,
                                                    const VkImageUsageFlags &usage,
                                                    const VkFormat &format,
                                                    const VkImageCreateFlags &flags) {
    return ImageCreateInfo({
      VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      nullptr,
      flags,
      VK_IMAGE_TYPE_2D,
      format,
      {size.width, size.height, 1},
      1,
      1,
      VK_SAMPLE_COUNT_1_BIT,
      VK_IMAGE_TILING_LINEAR,
      usage,
      VK_SHARING_MODE_EXCLUSIVE,
      1,
      nullptr,
      VK_IMAGE_LAYOUT_UNDEFINED
    });
  }
  
  ImageCreateInfo::ImageCreateInfo() : _info{
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
    
  ImageCreateInfo::ImageCreateInfo(const VkImageCreateInfo &info) : _info(info) {}
  
  ImageCreateInfo & ImageCreateInfo::operator=(const VkImageCreateInfo &info) {
    _info = info;
    return *this;
  }
  
  ImageCreateInfo & ImageCreateInfo::operator=(const ImageCreateInfo &info) {
    _info = info._info;
    return *this;
  }
  
  ImageCreateInfo::operator VkImageCreateInfo() const {
    return _info;
  }
  
  BufferCreateInfo BufferCreateInfo::buffer(const VkDeviceSize &size, const VkBufferUsageFlags &usage, const VkBufferCreateFlags &flags) {
    return BufferCreateInfo({
      VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      nullptr,
      flags,
      size,
      usage,
      VK_SHARING_MODE_EXCLUSIVE,
      0,
      nullptr
    });
  }
    
  BufferCreateInfo::BufferCreateInfo() 
    : _info{
      VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      nullptr,
      0,
      0,
      0,
      VK_SHARING_MODE_EXCLUSIVE,
      0,
      nullptr
    } {}
            
  BufferCreateInfo::BufferCreateInfo(const VkBufferCreateInfo &info) : _info(info) {}
  
  BufferCreateInfo & BufferCreateInfo::operator=(const VkBufferCreateInfo &info) {
    _info = info;
    return *this;
  }
  
  BufferCreateInfo & BufferCreateInfo::operator=(const BufferCreateInfo &info) {
    _info = info._info;
    return *this;
  }
    
  BufferCreateInfo::operator VkBufferCreateInfo() const {
    return _info;
  }
  
  Swapchain::Swapchain() : imageExtent{0, 0}, device(nullptr), swap(VK_NULL_HANDLE) {}
  Swapchain::Swapchain(const Swapchain &swapchain) : imageExtent(swapchain.getImageExtent()), device(swapchain.device), swap(swapchain.handle()) {}
  Swapchain::Swapchain(const VkExtent2D &extent, Device* device, const VkSwapchainKHR &swap) : imageExtent(extent), device(device), swap(swap) {}
  Swapchain::~Swapchain() {}
  
  VkResult Swapchain::getImages(uint32_t* imagesCount, Image** images) {
    if (imagesCount == nullptr) throw std::runtime_error("Swapchain::getImages imagesCount == nullptr");
    
    const uint32_t userCount = *imagesCount;
    VkResult res = vkGetSwapchainImagesKHR(device->handle(), swap, imagesCount, nullptr);
    if (images == nullptr) return res;
    
    // тут нужно сделать создание изображения без данных
//     throw std::runtime_error("not implemeted yet");
    
    const std::vector<Image*> &imagesTmp = device->getSwapchainImages(*this);
    for (uint32_t i = 0; i < userCount; ++i) {
      images[i] = imagesTmp[i];
    }
    
    if (userCount < imagesTmp.size()) return VK_INCOMPLETE;
    return VK_SUCCESS;
  }
  
  VkResult Swapchain::acquireNextImage(const uint64_t &timeout, const Semaphore &semaphore, const Fence &fence, uint32_t* pImageIndex) {
    return vkAcquireNextImageKHR(device->handle(), swap, timeout, semaphore, fence, pImageIndex);
  }
  
  VkSwapchainKHR Swapchain::handle() const {
    return swap;
  }
  
  Swapchain & Swapchain::operator=(const Swapchain &another) {
    imageExtent = another.imageExtent;
    device = another.device;
    swap = another.swap;
    return *this;
  }
  
  void Swapchain::setImageExtent(const VkExtent2D &imageExtent) {
    this->imageExtent = imageExtent;
  }
  
  VkExtent2D Swapchain::getImageExtent() const {
    return imageExtent;
  }
  
  Swapchain::operator VkSwapchainKHR() const {
    return swap;
  }
  
  bool Swapchain::operator==(const Swapchain &another) const {
    return device == another.device && swap == another.swap;
  }
  
  bool Swapchain::operator!=(const Swapchain &another) const {
    return device != another.device || swap != another.swap;
  }
  
  Framebuffer::Framebuffer() : buffer(VK_NULL_HANDLE) {}
  Framebuffer::Framebuffer(const Framebuffer &framebuffer) : buffer(framebuffer.handle()) {}
  Framebuffer::Framebuffer(VkFramebuffer buffer) : buffer(buffer) {}
  Framebuffer::~Framebuffer() {}
  
  VkFramebuffer Framebuffer::handle() const {
    return buffer;
  }
  
  Framebuffer & Framebuffer::operator=(const Framebuffer &another) {
    buffer = another.buffer;
    return *this;
  }
  
  Framebuffer::operator VkFramebuffer() const {
    return buffer;
  }
  
  bool Framebuffer::operator==(const Framebuffer &another) const {
    return buffer == another.buffer;
  }
  
  bool Framebuffer::operator!=(const Framebuffer &another) const {
    return buffer != another.buffer;
  }
  
  Pipeline::Pipeline() : h(VK_NULL_HANDLE), layoutH(VK_NULL_HANDLE) {}
  
  Pipeline::Pipeline(VkPipeline h, VkPipelineLayout l) : h(h), layoutH(l) {}

  VkPipeline Pipeline::handle() const {
    return h;
  }

  VkPipelineLayout Pipeline::layout() const {
    return layoutH;
  }

  bool Pipeline::operator==(const Pipeline &other) const {
    return this->h == other.h;
  }

  bool Pipeline::operator!=(const Pipeline &other) const {
    return this->h != other.h;
  }
  
  Pipeline::operator VkPipeline() const {
    return h;
  }
  
//   Sampler::Sampler() : ptr(nullptr) {}
//   Sampler::Sampler(Internal::SamplerBase* ptr) : ptr(ptr) {}
//   Sampler::~Sampler() {}
//   
//   bool Sampler::valid() const {
//     return ptr != nullptr;
//   }
//   
//   Internal::SamplerBase* Sampler::operator->() const {
//     return ptr;
//   }
//   
//   Sampler & Sampler::operator=(const Sampler &another) {
//     ptr = another.ptr;
//     return *this;
//   }
//   
//   Sampler::operator VkSampler() const {
//     return ptr->handle();
//   }
//   
//   bool Sampler::operator==(const Sampler &another) const {
//     return ptr == another.ptr;
//   }
//   
//   bool Sampler::operator!=(const Sampler &another) const {
//     return ptr != another.ptr;
//   }

  Sampler::Sampler() : sampler(VK_NULL_HANDLE), setHandle(nullptr), resIndex(SIZE_MAX) {}
  Sampler::Sampler(VkSampler sampler) : sampler(sampler), setHandle(nullptr), resIndex(SIZE_MAX) {}
  Sampler::~Sampler() {}
  
  bool Sampler::valid() const {
    return sampler != VK_NULL_HANDLE;
  }
  
  VkSampler Sampler::handle() const {
    return sampler;
  }

  DescriptorSet* Sampler::descriptorSet() const {
    return setHandle;
  }
  
  size_t Sampler::descriptorSetIndex() const {
    return resIndex;
  }
  
  //const uint32_t &binding, const uint32_t &arrayNum
  void Sampler::setDescriptor(DescriptorSet* set, const size_t &index) {
    setHandle = set;
    resIndex = index;
  }
  
  Sampler::operator VkSampler() const {
    return sampler;
  }
  
  bool Sampler::operator==(const Sampler &another) const {
    return sampler == another.sampler;
  }
  
  bool Sampler::operator!=(const Sampler &another) const {
    return sampler != another.sampler;
  }
  
  BufferView::BufferView() : 
    RAIIType1(), 
    formatVar(VK_FORMAT_UNDEFINED), 
    offsetVar(0), 
    sizeVar(SIZE_MAX), 
    relatedBuffer(nullptr), 
    device(nullptr), 
    set(nullptr), 
    setIndex(SIZE_MAX), 
    nextPtr(nullptr) {}
    
  BufferView::BufferView(Device* device, const VkBufferViewCreateInfo &info, Buffer* buffer) : 
    RAIIType1(device->handle(), info, "vkCreateBufferView"),
    formatVar(info.format), 
    offsetVar(info.offset), 
    sizeVar(info.range), 
    relatedBuffer(buffer), 
    device(device), 
    set(nullptr), 
    setIndex(SIZE_MAX), 
    nextPtr(nullptr) {}
    
  BufferView::~BufferView() {}
  
  void BufferView::construct(Device* device, const VkBufferViewCreateInfo &info, Buffer* buffer) {
    this->formatVar = info.format;
    this->offsetVar = info.offset;
    this->sizeVar = info.range;
    this->relatedBuffer = buffer;
    this->device = device;
    
    vkCheckError("vkCreateBufferView", nullptr, 
    RAIIType1::construct(device->handle(), info));
  }
  
  VkFormat BufferView::format() const {
    return formatVar;
  }
  
  size_t BufferView::offset() const {
    return offsetVar;
  }
  
  size_t BufferView::size() const {
    return sizeVar;
  }
  
  DescriptorSet* BufferView::descriptorSet() const {
    return set;
  }
  
  size_t BufferView::descriptorSetIndex() const {
    return setIndex;
  }
  
  void BufferView::setDescriptor(DescriptorSet* set, const size_t &index) {
    this->set = set;
    this->setIndex = index;
    
    if (set != nullptr) set->update(index);
  }
  
  VkBufferView BufferView::handle() const {
    return obj;
  }
  
  Buffer* BufferView::buffer() const {
    return relatedBuffer;
  }
  
  BufferView* BufferView::next() const {
    return nextPtr;
  }
  
  void BufferView::setNext(BufferView* ptr) {
    nextPtr = ptr;
  }
  
  void BufferView::recreate() {
    vkDestroyBufferView(device->handle(), obj, nullptr);
    obj = VK_NULL_HANDLE;
    
    const VkBufferViewCreateInfo info{
      VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
      nullptr,
      0,
      relatedBuffer->handle(),
      formatVar,
      offsetVar,
      sizeVar
    };
    
    vkCheckError("vkCreateBufferView", nullptr, 
    vkCreateBufferView(device->handle(), &info, nullptr, &obj));
    
    if (set != nullptr) {
      set->update(setIndex);
    }
  }
  
  Buffer::Buffer() : RAIIType4(), device(nullptr), pointer(nullptr), viewSize(0), bufferView(nullptr), set(nullptr), setIndex(0) {}
  
  Buffer::Buffer(Device* device, const VkBufferCreateInfo &info, const VmaMemoryUsage &memUsage) : 
    RAIIType4(device->bufferAllocator()), 
    memUsage(memUsage), 
    device(device), 
    pointer(nullptr), 
    viewSize(0), 
    bufferView(nullptr), 
    set(nullptr), 
    setIndex(0), 
    parameters(info) {
    VmaAllocationInfo data;

    //VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT
    const VmaAllocationCreateInfo alloc{
      static_cast<VmaAllocationCreateFlags>(memUsage != VMA_MEMORY_USAGE_GPU_ONLY ? VMA_ALLOCATION_CREATE_MAPPED_BIT : 0),
      memUsage,
      0, 0, 0, VK_NULL_HANDLE, nullptr
    };
    
    vkCheckError("vmaCreateBuffer", nullptr, 
    RAIIType4::construct(info, alloc, &data));
    
    pointer = data.pMappedData;
  }
  
  Buffer::~Buffer() {
//    if (set != nullptr) {
//      set->erase(setIndex);
//    }

    BufferView* view = bufferView;
    while (view != nullptr) {
      BufferView* toDelete = view;
      view = view->next();
      
      device->destroy(toDelete);
    }
  }
  
  void Buffer::construct(Device* device, const VkBufferCreateInfo &info, const VmaMemoryUsage &memUsage) {
    this->device = device;
    this->memUsage = memUsage;
    
    VmaAllocationInfo data;
    
    const VmaAllocationCreateInfo alloc{
      static_cast<VmaAllocationCreateFlags>(memUsage != VMA_MEMORY_USAGE_GPU_ONLY ? VMA_ALLOCATION_CREATE_MAPPED_BIT : 0),
      memUsage,
      0, 0, 0, VK_NULL_HANDLE, nullptr
    };
    
    vkCheckError("vmaCreateBuffer", nullptr, 
    RAIIType4::construct(device->bufferAllocator(), info, alloc, &data));
    
    pointer = data.pMappedData;
    parameters = info;
  }
  
  void Buffer::recreate(const size_t &newSize) {
    // если парам сайз больше, нужно ли пересоздавать?
    if (parameters.size == newSize) return;
    
    const size_t finalSize = memUsage == VMA_MEMORY_USAGE_CPU_ONLY ? 
      alignMemorySize(std::max(device->getMinMemoryMapAlignment(), device->getNonCoherentAtomSize()), newSize) : 
      //alignMemorySize(device->getNonCoherentAtomSize(), newSize);
      newSize;
      
    parameters.size = finalSize;
    
    vmaDestroyBuffer(allocator, obj, RAIIType4::allocation);
    
    const VmaAllocationCreateInfo alloc{
      static_cast<VmaAllocationCreateFlags>(memUsage != VMA_MEMORY_USAGE_GPU_ONLY ? VMA_ALLOCATION_CREATE_MAPPED_BIT : 0),
      memUsage,
      0, 0, 0, VK_NULL_HANDLE, nullptr
    };
    
    VmaAllocationInfo data;
    
    vkCheckError("vmaCreateBuffer", nullptr, 
    RAIIType4::construct(parameters, alloc, &data));
    
    pointer = data.pMappedData;
    
    BufferView* view = bufferView;
    while (view != nullptr) {
      view->recreate();
      
      view = view->next();
    }
    
    if (set != nullptr) {
      set->at(setIndex).bufferData.range = finalSize;
      set->update(setIndex);
    }
  }

  void Buffer::resize(const size_t &newSize) {
    if (parameters.size == newSize) return;

    const size_t finalSize = memUsage == VMA_MEMORY_USAGE_CPU_ONLY ?
                             alignMemorySize(std::max(device->getMinMemoryMapAlignment(), device->getNonCoherentAtomSize()), newSize) :
                             //alignMemorySize(device->getNonCoherentAtomSize(), newSize);
                             newSize;

    // не знаю нужно ли оставить копирование?
                             
    //const size_t copySize = std::min(parameters.size, finalSize);
    parameters.size = finalSize;

    const VmaAllocationCreateInfo alloc{
      static_cast<VmaAllocationCreateFlags>(memUsage != VMA_MEMORY_USAGE_GPU_ONLY ? VMA_ALLOCATION_CREATE_MAPPED_BIT : 0),
      memUsage,
      0, 0, 0, VK_NULL_HANDLE, nullptr
    };

    VmaAllocationInfo data;
    
    vmaDestroyBuffer(allocator, obj, RAIIType4::allocation);

    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    vmaCreateBuffer(allocator, &parameters._info, &alloc, &buffer, &allocation, &data);

    //if (pointer != nullptr) memcpy(data.pMappedData, pointer, copySize);

    obj = buffer;
    RAIIType4::allocation = allocation;
    pointer = data.pMappedData;

    BufferView* view = bufferView;
    while (view != nullptr) {
      view->recreate();
      view = view->next();
    }

    if (set != nullptr) {
      set->at(setIndex).bufferData.range = finalSize;
      set->update(setIndex);
    }
  }
  
  void Buffer::flush() const {
    VmaAllocationInfo data;
    
    vmaGetAllocationInfo(device->bufferAllocator(), RAIIType4::allocation, &data);
    
    const VkMappedMemoryRange memRange{
      VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
      nullptr,
      data.deviceMemory,
      data.offset,
      data.size
    };
    
    vkCheckError("vkFlushMappedMemoryRanges", nullptr, 
    vkFlushMappedMemoryRanges(device->handle(), 1, &memRange));
  }
  
  size_t Buffer::viewCount() const {
    return viewSize;
  }
  
  BufferView* Buffer::view(const size_t &index) const {
    if (index == 0) return bufferView;
    
    size_t tmp = 0;
    BufferView* view = bufferView;
    while (tmp != index && view != nullptr) {
      ++tmp;
      view = view->next();
    }
    
    return view;
  }
  
  BufferView* Buffer::createView(const VkFormat &format, const size_t &offset, const size_t &size) {
    const VkBufferViewCreateInfo info{
      VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
      nullptr,
      0,
      obj,
      format,
      offset,
      size
    };
    
    BufferView* v = device->create(info, this);
    
    if (viewSize == 0) {
      bufferView = v;
      return v;
    }
    
    BufferView* prev = nullptr;
    BufferView* view = bufferView;
    while (view != nullptr) {
      prev = view;
      view = view->next();
    }
    
    prev->setNext(v);
    
    return v;
  }
  
  DescriptorSet* Buffer::descriptorSet() const {
    return set;
  }
  
  size_t Buffer::descriptorSetIndex() const {
    return setIndex;
  }
  
  void Buffer::setDescriptor(DescriptorSet* set, const size_t &index) {
    if (this->set != nullptr) {
      throw std::runtime_error("trying to change descriptor set");
    }

    this->set = set;
    this->setIndex = index;
    
    if (set != nullptr) set->update(setIndex);
  }
  
  Internal::BufferInfo & Buffer::info() {
    return parameters;
  }
  
  VkBuffer Buffer::handle() const {
    return obj;
  }
  
  VmaAllocation Buffer::allocation() const {
    return RAIIType4::allocation;
  }
  
  void* Buffer::ptr() const {
    return pointer;
  }
  
  ImageView::ImageView() : 
    RAIIType1(), 
    typeVar(VK_IMAGE_VIEW_TYPE_MAX_ENUM),
    formatVar(VK_FORMAT_UNDEFINED),
    componentsVar(YAVF_DEFAULT_SWIZZLE),
    subresourceRangeVar({}),
    imageVar(nullptr),
    device(nullptr),
    set(nullptr),
    setIndex(SIZE_MAX),
    nextPtr(nullptr) {}
    
  ImageView::ImageView(Device* device, const VkImageViewCreateInfo &info, Image* relatedImage) : 
    RAIIType1(device->handle(), info, "vkCreateImageView"),
    typeVar(info.viewType),
    formatVar(info.format),
    componentsVar(info.components),
    subresourceRangeVar(info.subresourceRange),
    imageVar(relatedImage),
    device(device),
    set(nullptr),
    setIndex(SIZE_MAX),
    nextPtr(nullptr) {}
  
  ImageView::~ImageView() {}
  
  void ImageView::construct(Device* device, const VkImageViewCreateInfo &info, Image* relatedImage) {
    this->typeVar = info.viewType;
    this->formatVar = info.format;
    this->componentsVar = info.components;
    this->subresourceRangeVar = info.subresourceRange;
    this->imageVar = relatedImage;
    this->device = device;
    
    vkCheckError("vkCreateImageView", nullptr, 
    RAIIType1::construct(device->handle(), info));
  }
  
  VkImageViewType ImageView::type() const {
    return typeVar;
  }
  
  VkFormat ImageView::format() const {
    return formatVar;
  }
  
  VkComponentMapping ImageView::components() const {
    return componentsVar;
  }
  
  VkImageSubresourceRange ImageView::subresourceRange() const {
    return subresourceRangeVar;
  }
  
  VkImageView ImageView::handle() const {
    return obj;
  }
  
  Image* ImageView::image() const {
    return imageVar;
  }
  
  DescriptorSet* ImageView::descriptorSet() const {
    return set;
  }
  
  size_t ImageView::descriptorSetIndex() const {
    return setIndex;
  }
  
  void ImageView::setDescriptor(DescriptorSet* res, const size_t &resIndex) {
    if (this->set != nullptr) {
      throw std::runtime_error("trying to change descriptor set");
    }

    this->set = res;
    this->setIndex = resIndex;
    
    if (set != nullptr) set->update(setIndex);
  }
  
  ImageView* ImageView::next() const {
    return nextPtr;
  }
  
  void ImageView::setNext(ImageView* ptr) {
    nextPtr = ptr;
  }
  
  void ImageView::recreate() {
    vkDestroyImageView(device->handle(), obj, nullptr);
    obj = VK_NULL_HANDLE;
    
    const VkImageViewCreateInfo info{
      VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      nullptr,
      0,
      imageVar->handle(),
      typeVar,
      formatVar,
      componentsVar,
      subresourceRangeVar
    };
    
    vkCheckError("vkCreateImageView", nullptr, 
    RAIIType1::construct(info)); // тут наверное не нужно использовать метод raii
    
    if (set != nullptr) {
      set->update(setIndex);
    }
  }
  
  Image::Image() : 
    RAIIType4(),
    memUsage(VMA_MEMORY_USAGE_UNKNOWN),
    dev(nullptr),
    pointer(nullptr),
    viewSize(0),
    imageView(nullptr) {}
    
  Image::Image(Device* device, const VkImageCreateInfo &info, const VmaMemoryUsage &memUsage) : 
    RAIIType4(device->imageAllocator()),
    memUsage(memUsage),
    dev(device),
    pointer(nullptr),
    viewSize(0),
    imageView(nullptr),
    parameters(info) {
    VmaAllocationInfo data;
    
    const VmaAllocationCreateInfo alloc{
      static_cast<VmaAllocationCreateFlags>(memUsage != VMA_MEMORY_USAGE_GPU_ONLY ? VMA_ALLOCATION_CREATE_MAPPED_BIT : 0),
      memUsage,
      0, 0, 0, VK_NULL_HANDLE, nullptr
    };
    
    vkCheckError("vmaCreateImage", nullptr, 
    RAIIType4::construct(info, alloc, &data));
    
    pointer = data.pMappedData;
  }
    
  Image::Image(Device* device, VkImage image, const VkExtent2D &size) : 
    RAIIType4(),
    memUsage(VMA_MEMORY_USAGE_UNKNOWN),
    dev(device),
    pointer(nullptr),
    viewSize(0),
    imageView(nullptr) {
    obj = image;
    parameters.extent = {size.width, size.height, 1};
  }
  
  Image::~Image() {
    if (RAIIType4::allocation == VK_NULL_HANDLE) obj = VK_NULL_HANDLE;
    
    ImageView* view = imageView;
    while (view != nullptr) {
      ImageView* toDelete = view;
      view = view->next();
      
      dev->destroy(toDelete);
    }
  }
  
  void Image::construct(Device* device, const VkImageCreateInfo &info, const VmaMemoryUsage &memUsage) {
    this->memUsage = memUsage;
    this->dev = device;
    this->parameters = info;
    
    VmaAllocationInfo data;
    
    const VmaAllocationCreateInfo alloc{
      static_cast<VmaAllocationCreateFlags>(memUsage != VMA_MEMORY_USAGE_GPU_ONLY ? VMA_ALLOCATION_CREATE_MAPPED_BIT : 0),
      memUsage,
      0, 0, 0, VK_NULL_HANDLE, nullptr
    };
    
    vkCheckError("vmaCreateImage", nullptr, 
    RAIIType4::construct(device->imageAllocator(), info, alloc, &data));
    
    pointer = data.pMappedData;
  }
  
  void Image::recreate(const VkExtent3D &newSize) {
    if (parameters.extent.width == newSize.width && parameters.extent.height == newSize.height && parameters.extent.depth == newSize.depth) return;
    
    vmaDestroyImage(allocator, obj, RAIIType4::allocation);
    
    parameters.extent = newSize;
    parameters.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    
    VmaAllocationInfo data;
    
    const VmaAllocationCreateInfo alloc{
      static_cast<VmaAllocationCreateFlags>(memUsage != VMA_MEMORY_USAGE_GPU_ONLY ? VMA_ALLOCATION_CREATE_MAPPED_BIT : 0),
      memUsage,
      0, 0, 0, VK_NULL_HANDLE, nullptr
    };
    
    vkCheckError("vmaCreateImage", nullptr, 
    RAIIType4::construct(parameters, alloc, &data));
    
    pointer = data.pMappedData;
    
    ImageView* view = imageView;
    while (view != nullptr) {
      view->recreate();
      
      view = view->next();
    }
  }
  
  void Image::flush() const {
    VmaAllocationInfo data;
    
    vmaGetAllocationInfo(dev->bufferAllocator(), RAIIType4::allocation, &data);
    
    const VkMappedMemoryRange memRange{
      VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
      nullptr,
      data.deviceMemory,
      data.offset,
      data.size
    };
    
    vkCheckError("vkFlushMappedMemoryRanges", nullptr, 
    vkFlushMappedMemoryRanges(dev->handle(), 1, &memRange));
  }
  
  size_t Image::viewCount() const {
    return viewSize;
  }
  
  ImageView* Image::view(const size_t &index) {
    if (index == 0) return imageView;
    
    size_t tmp = 0;
    ImageView* view = imageView;
    while (tmp != index && view != nullptr) {
      ++tmp;
      view = view->next();
    }
    
    return view;
  }
  
  ImageView* Image::createView(const VkImageViewType &type, const VkImageSubresourceRange &range,
                               const VkFormat &format, const VkComponentMapping &mapping) {
    const VkImageViewCreateInfo info{
      VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      nullptr,
      0,
      obj,
      type,
      (format == VK_FORMAT_UNDEFINED ? parameters.format : format),
      mapping,
      range
    };
    
    ImageView* v = dev->create(info, this);
    
    if (viewSize == 0) {
      imageView = v;
      return v;
    }
    
    ImageView* prev = nullptr;
    ImageView* view = imageView;
    while (view != nullptr) {
      prev = view;
      view = view->next();
    }
    
    prev->setNext(v);
    
    return v;
  }
  
  ImageView* Image::createView(const VkImageViewType &type, const VkImageAspectFlags &aspect,
                               const VkFormat &format, const VkComponentMapping &mapping) {
    const VkImageViewCreateInfo info{
      VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      nullptr,
      0,
      obj,
      type,
      (format == VK_FORMAT_UNDEFINED ? parameters.format : format),
      mapping,
      {
        aspect,
        0, parameters.mipLevels,
        0, parameters.arrayLayers
      }
    };
    
    ImageView* v = dev->create(info, this);
    
    if (viewSize == 0) {
      imageView = v;
      return v;
    }
    
    ImageView* prev = nullptr;
    ImageView* view = imageView;
    while (view != nullptr) {
      prev = view;
      view = view->next();
    }
    
    prev->setNext(v);
    
    return v;
  }

  Internal::ImageInfo & Image::info() {
    return parameters;
  }
  
  VkImage Image::handle() const {
    return obj;
  }
  
  VmaAllocation Image::allocation() const {
    return RAIIType4::allocation;
  }
  
  void* Image::ptr() const {
    return pointer;
  }
  
  DescriptorSet::DescriptorSet(Device* device, VkDescriptorSet h) : freeData(UINT32_MAX), device(device), h(h) {}
  DescriptorSet::~DescriptorSet() {
    // тут наверное неплохо было бы резетить
  }
  
  size_t DescriptorSet::add(const DescriptorSetData &data) {
    size_t index;
      
//     assert(freeData == SIZE_MAX);
    
    if (freeData == UINT32_MAX) {
      index = datas.size();
      datas.push_back(data);
    } else {
      index = freeData;
      freeData = datas[freeData].arrayElement;
      datas[index] = data;
    }
      
    return index;
  }
  
  void DescriptorSet::erase(const size_t &index) {
    datas[index].arrayElement = freeData;
    datas[index].bindingNum = UINT32_MAX;
    freeData = index;
  }
  
  void DescriptorSet::update(const size_t &index) {
    if (index == SIZE_MAX) {
      DescriptorUpdater du(device);

//      std::cout << "size: " << datas.size() << "\n";
      for (size_t i = 0; i < datas.size(); ++i) {
        if (datas[i].bindingNum == UINT32_MAX) continue;

        du.currentSet(h);
        du.begin(datas[i].bindingNum, datas[i].arrayElement, datas[i].descType);
        int a = getIntTypeFromDescType(datas[i].descType);
        
        if (a == 0) {
          du.image(datas[i].imageData.imageView, datas[i].imageData.imageLayout, datas[i].imageData.sampler);
        } else if (a == 1) {
//          std::cout << "buffer: " << datas[i].bufferData.buffer->handle() << "\n";
          du.buffer(datas[i].bufferData.buffer, datas[i].bufferData.offset, datas[i].bufferData.range);
        } else {
          du.texelBuffer(datas[i].view);
        }
        
        du.update();
      }
      
      return;
    }
    
    if (index >= datas.size()) return;
    
    if (datas[index].bindingNum == UINT32_MAX) return;
    
    DescriptorUpdater du(device);
    du.currentSet(h);
    du.begin(datas[index].bindingNum, datas[index].arrayElement, datas[index].descType);
//     std::cout << datas[index].descType << "\n";
    int a = getIntTypeFromDescType(datas[index].descType);
    
    if (a == 0) {
      du.image(datas[index].imageData.imageView, datas[index].imageData.imageLayout, datas[index].imageData.sampler);
    } else if (a == 1) {
      du.buffer(datas[index].bufferData.buffer, datas[index].bufferData.offset, datas[index].bufferData.range);
    } else {
      du.texelBuffer(datas[index].view);
    }
    
    du.update();
  }

  void DescriptorSet::resize(const size_t &size) {
    if (datas.size() == size) return;

    datas.resize(size);
  }
  
  size_t DescriptorSet::size() const {
    return datas.size();
  }
  
  VkDescriptorSet DescriptorSet::handle() const {
    return h;
  }
  
//   VkDescriptorType DescriptorSet::type() const {
//     return descType;
//   }
  
  DescriptorSetData & DescriptorSet::operator[](const size_t &index) {
    return datas[index];
  }
  
  const DescriptorSetData & DescriptorSet::operator[](const size_t &index) const {
    return datas[index];
  }
  
  DescriptorSetData & DescriptorSet::at(const size_t &index) {
    return datas[index];
  }
  
  const DescriptorSetData & DescriptorSet::at(const size_t &index) const {
    return datas[index];
  }
}
  
//   namespace Internal {
//     SamplerBase::SamplerBase(VkSampler sampler) : sampler(sampler) {}
//     SamplerBase::~SamplerBase() {}
//     
//     VkSampler SamplerBase::handle() const {
//       return sampler;
//     }
// 
//     DescriptorSet SamplerBase::descriptorSet() const {
//       return setHandle;
//     }
//     
//     size_t SamplerBase::descriptorSetIndex() const {
//       return resIndex;
//     }
//     
//     void SamplerBase::setDescriptor(const DescriptorSet &set, const size_t &index) {
//       this->setHandle = set;
//       this->resIndex = index;
//     }
//     
//     SamplerBase::operator VkSampler() const {
//       return sampler;
//     }
//     
//     bool SamplerBase::operator==(const SamplerBase &another) const {
//       return sampler == another.sampler;
//     }
//     
//     bool SamplerBase::operator!=(const SamplerBase &another) const {
//       return sampler != another.sampler;
//     }
    
//     BufferViewBase::BufferViewBase(const CreateInfo &info) 
//       : formatVar(info.formatVar),
//         internalIndex(info.internalIndex),
//         offsetVar(info.offsetVar),
//         sizeVar(info.sizeVar),
//         h(info.h),
//         relatedBuffer(info.relatedBuffer),
//         device(info.device) {}
//         
//     BufferViewBase::~BufferViewBase() {}
//     
//     VkFormat BufferViewBase::format() const {
//       return formatVar;
//     }
//     
//     size_t BufferViewBase::offset() const {
//       return offsetVar;
//     }
//     
//     size_t BufferViewBase::size() const {
//       return sizeVar;
//     }
//     
//     DescriptorSet BufferViewBase::descriptorSet() const {
//       return set;
//     }
//     
//     size_t BufferViewBase::descriptorSetIndex() const {
//       return setIndex;
//     }
//     
//     void BufferViewBase::setDescriptor(const DescriptorSet &set, const size_t &index) {
//       this->set = set;
//       this->setIndex = index;
//       
//       if (set.valid()) {
//         set->update(setIndex);
//       }
//     }
//     
//     VkBufferView BufferViewBase::handle() const {
//       return h;
//     }
//     
//     Buffer BufferViewBase::buffer() const {
//       return relatedBuffer;
//     }
//     
//     BufferViewBase* BufferViewBase::next() const {
//       return nextPtr;
//     }
//     
//     void BufferViewBase::setNext(BufferViewBase* ptr) {
//       nextPtr = ptr;
//     }
//     
//     BufferViewBase::operator VkBufferView() const {
//       return h;
//     }
//     
//     bool BufferViewBase::operator==(const BufferViewBase &another) const {
//       return h == another.h;
//     }
//     
//     bool BufferViewBase::operator!=(const BufferViewBase &another) const {
//       return h != another.h;
//     }
//     
//     void BufferViewBase::recreate() {
//       vkDestroyBufferView(device->handle(), h, nullptr);
//       
//       const VkBufferViewCreateInfo info{
//         VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
//         nullptr,
//         0,
//         relatedBuffer->handle(),
//         formatVar,
//         offsetVar,
//         sizeVar
//       };
//       
//       vkCheckError("vkCreateBufferView", nullptr, 
//       vkCreateBufferView(device->handle(), &info, nullptr, &h));
//       
//       if (set.valid()) {
//         set->update(setIndex);
//       }
//     }
//     
//     BufferBase::BufferBase(const CreateInfo &info) 
//       : memUsage(info.memUsage),
//         internalIndex(info.internalIndex), 
//         device(info.device),
//         pointer(info.pointer),
//         h(info.h),
//         a(info.a),
//         viewSize(0),
//         bufferView(nullptr),
//         set(nullptr),
//         setIndex(SIZE_MAX),
//         parameters(info.parameters) {}
//     
//     BufferBase::~BufferBase() {}
//     
//     void BufferBase::recreate(const size_t &newSize) {
//       if (parameters.size >= newSize) return;
//       
//       vmaDestroyBuffer(device->bufferAllocator(), h, a);
//       
//       VmaAllocationInfo data;
//       
//       const size_t finalSize = memUsage == VMA_MEMORY_USAGE_CPU_ONLY ? 
//         alignMemorySize(std::max(device->getMinMemoryMapAlignment(), device->getNonCoherentAtomSize()), newSize) : 
//         //alignMemorySize(device->getNonCoherentAtomSize(), newSize);
//         newSize;
//         
//       parameters.size = finalSize;
//       
//       const VmaAllocationCreateInfo alloc{
//         static_cast<VmaAllocationCreateFlags>(memUsage != VMA_MEMORY_USAGE_GPU_ONLY ? VMA_ALLOCATION_CREATE_MAPPED_BIT : 0),
//         memUsage,
//         0, 0, 0, VK_NULL_HANDLE, nullptr
//       };
//       
//       const VkBufferCreateInfo &info = parameters;
// //       VkBuffer newB = VK_NULL_HANDLE;
// //       VmaAllocation newA = VK_NULL_HANDLE;
//       
//       vkCheckError("vmaCreateBuffer", nullptr, 
//       vmaCreateBuffer(device->bufferAllocator(), &info, &alloc, &h, &a, &data));
//       
//       // тут нужно обойти буфер вью и пересоздать
//       size_t tmp = 0;
//       BufferViewBase* view = bufferView.operator->();
//       while (tmp < viewSize) {
//         view->recreate();
//         
//         view = view->next();
//         ++tmp;
//       }
//       
//       pointer = data.pMappedData;
//       
//       if (set.valid()) {
//         set->update(setIndex);
//       }
//     }
//     
//     void BufferBase::flush() const {
//       VmaAllocationInfo data;
//     
//       vmaGetAllocationInfo(device->bufferAllocator(), allocation, &data);
//       
//       VkMappedMemoryRange memRange{
//         VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
//         nullptr,
//         data.deviceMemory,
//         data.offset,
//         data.size
//       };
//       
//       vkFlushMappedMemoryRanges(device->handle(), 1, &memRange);
//     }
//     
//     // что с вью? их может быть несколько, и их нужно обновлять каждый раз как я переделываю буфер
//     // придется хранить их всех в буфере
//     size_t BufferBase::viewCount() const {
//       return viewSize;
//     }
//     
//     BufferView BufferBase::view(const size_t &index) const {
//       if (index == 0) return bufferView;
//       
//       size_t tmp = 0;
//       BufferViewBase* view = bufferView.operator->();
//       while (tmp < viewSize) {
//         if (tmp == index) return BufferView(view);
//         
//         view = view->next();
//         ++tmp;
//       }
//       
//       return BufferView(nullptr);
//     }
//     
//     BufferView BufferBase::createView(const VkFormat &format, const size_t &offset, const size_t &size) {
//       // создавать вью мы теперь будем видимо с помощью девайса
//       
//       const VkBufferViewCreateInfo info{
//         VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
//         nullptr,
//         0,
//         h,
//         format,
//         offset,
//         size
//       };
//       BufferView v = device->create(info, Buffer(this));
//       
//       if (!bufferView.valid()) {
//         bufferView = v;
//         ++viewSize;
//         return v;
//       }
//       
//       size_t tmp = 0;
//       BufferViewBase* view = bufferView.operator->();
//       while (tmp < viewSize-1) {
//         view = view->next();
//         ++tmp;
//       }
//       
//       view->setNext(v.operator->());
//       ++viewSize;
//       return v;
//     }
//     
//     DescriptorSet BufferBase::descriptorSet() const {
//       return set;
//     }
//     
//     size_t BufferBase::descriptorSetIndex() const {
//       return setIndex;
//     }
//     
//     void BufferBase::setDescriptor(const DescriptorSet &set, const size_t &index) {
//       this->set = set;
//       this->setIndex = index;
//       
//       if (set.valid()) {
//         set->update(setIndex);
//       }
//     }
//     
//     Internal::BufferInfo & BufferBase::info() {
//       return parameters;
//     }
//     
//     VkBuffer BufferBase::handle() const {
//       return h;
//     }
//     
//     VmaAllocation BufferBase::allocation() const {
//       return a;
//     }
//     
//     void* BufferBase::ptr() const {
//       return pointer;
//     }
//     
//     BufferBase::operator VkBuffer() const {
//       return h;
//     }
//     
//     bool BufferBase::operator==(const BufferBase &another) const {
//       return this->h == another.h;
//     }
//     
//     bool BufferBase::operator!=(const BufferBase &another) const {
//       return this->h != another.h;
//     }
//     
//     ImageViewBase::ImageViewBase(const CreateInfo &info) 
//       : typeVar(info.type),
//         formatVar(info.format),
//         componentsVar(info.components),
// //         aspectVar(info.aspect),
//         subresourceRangeVar(info.subresourceRange),
//         h(info.h),
//         internalIndex(info.internalIndex),
//         imageVar(info.image),
//         device(info.device),
// //         sampler(nullptr),
//         res(nullptr),
//         resIndex(SIZE_MAX),
//         nextPtr(nullptr) {}
//         
//     ImageViewBase::~ImageViewBase() {}
//     
//     VkImageViewType ImageViewBase::type() const {
//       return typeVar;
//     }
//     
//     VkFormat ImageViewBase::format() const {
//       return formatVar;
//     }
//     
//     VkComponentMapping ImageViewBase::components() const {
//       return componentsVar;
//     }
//     
// //     VkImageAspectFlags ImageViewBase::aspect() const {
// //       return aspectVar;
// //     }
//     
//     VkImageSubresourceRange ImageViewBase::subresourceRange() const {
//       return subresourceRangeVar;
//     }
//     
//     VkImageView ImageViewBase::handle() const {
//       return h;
//     }
//     
//     Image ImageViewBase::image() const {
//       return imageVar;
//     }
//     
//     DescriptorSet ImageViewBase::descriptorSet() const {
//       return res;
//     }
//     
//     size_t ImageViewBase::descriptorSetIndex() const {
//       return resIndex;
//     }
//     
//     void ImageViewBase::setDescriptor(const DescriptorSet &res, const size_t &resIndex) {
//       this->res = res;
//       this->resIndex = resIndex;
//       
//       if (set.valid()) {
//         set->update(setIndex);
//       }
//     }
//     
//     ImageViewBase* ImageViewBase::next() const {
//       return nextPtr;
//     }
//     
//     void ImageViewBase::setNext(ImageViewBase* ptr) {
//       nextPtr = ptr;
//     }
//     
//     ImageViewBase::operator VkImageView() const {
//       return h;
//     }
//     
//     bool ImageViewBase::operator==(const ImageViewBase &another) const {
//       return h == another.h;
//     }
//     
//     bool ImageViewBase::operator!=(const ImageViewBase &another) const {
//       return h != another.h;
//     }
//     
//     void ImageViewBase::recreate() {
//       vkDestroyImageView(device->handle(), h, nullptr);
//       
//       const VkImageViewCreateInfo info{
//         VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
//         nullptr,
//         0,
//         imageVar->handle(),
//         typeVar,
//         formatVar,
//         componentsVar,
//         subresourceRangeVar
//       };
//       
//       vkCheckError("vkCreateImageView", nullptr, 
//       vkCreateImageView(device->handle(), &info, nullptr, &h));
//       
//       if (res.valid()) {
//         res->update(resIndex);
//       }
//     }
//     
//     ImageBase::ImageBase(const CreateInfo &info)
//       : memUsage(info.memUsage),
//         internalIndex(info.internalIndex),
//         dev(info.dev),
//         pointer(info.pointer),
//         h(info.h),
//         a(info.a),
//         viewSize(0),
//         imageView(nullptr),
//         parameters(info.parameters) {}
//         
//     ImageBase::~ImageBase() {}
//     
//     void ImageBase::recreate(const VkExtent3D &newSize) {
//       vmaDestroyImage(dev->imageAllocator(), h, a);
//       
//       VmaAllocationInfo data;
// 
//       parameters.extent = newSize;
//       parameters.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
// 
//       const VmaAllocationCreateInfo alloc{
//         static_cast<VmaAllocationCreateFlags>(memUsage != VMA_MEMORY_USAGE_GPU_ONLY ? VMA_ALLOCATION_CREATE_MAPPED_BIT : 0),
//         memUsage,
//         0, 0, 0, VK_NULL_HANDLE, nullptr
//       };
//       
//       const VkImageCreateInfo &info = parameters;
//       
//       vkCheckError("vmaCreateImage", nullptr, 
//       vmaCreateImage(dev->imageAllocator(), &info, &alloc, &h, &a, &data));
//       
//       // переделываем вью
//       size_t tmp = 0;
//       ImageViewBase* view = imageView.operator->();
//       while (tmp < viewSize) {
//         view->recreate();
//         
//         view = view->next();
//         ++tmp;
//       }
//       
//       pointer = data.pMappedData;
//     }
//     
//     void ImageBase::flush() const {
//       VmaAllocationInfo data;
//     
//       vmaGetAllocationInfo(dev->imageAllocator(), a, &data);
//       
//       const VkMappedMemoryRange memRange{
//         VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
//         nullptr,
//         data.deviceMemory,
//         data.offset,
//         data.size
//       };
//       
//       vkFlushMappedMemoryRanges(dev->handle(), 1, &memRange);
//     }
//     
//     size_t ImageBase::viewCount() const {
//       return viewSize;
//     }
//     
//     ImageView ImageBase::view(const size_t &index) {
//       if (index == 0) return imageView;
//       
//       size_t tmp = 0;
//       ImageViewBase* view = imageView.operator->();
//       while (tmp < viewSize) {
//         if (tmp == index) return ImageView(view);
//         
//         view = view->next();
//         ++tmp;
//       }
//       
//       return ImageView(nullptr);
//     }
//     
//     ImageView ImageBase::createView(const VkImageViewType &type, const VkImageSubresourceRange &range,
//                                     const VkFormat &format, const VkComponentMapping &mapping) {
//       // будем создавать вью с помощью девайса
//       const VkImageViewCreateInfo info{
//         VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
//         nullptr,
//         0,
//         h,
//         type,
//         (format == VK_FORMAT_UNDEFINED ? this->info().format : format),
//         mapping,
//         range
//       };
//       ImageView v = dev->create(info, Image(this));
//       
//       if (!imageView.valid()) {
//         imageView = v;
//         ++viewSize;
//         return v;
//       }
//       
//       size_t tmp = 0;
//       ImageViewBase* view = imageView.operator->();
//       while (tmp < viewSize-1) {
//         view = view->next();
//         ++tmp;
//       }
//       
//       view->setNext(v.operator->());
//       ++viewSize;
//       return v;
//     }
// 
//     Internal::ImageInfo & ImageBase::info() {
//       return parameters;
//     }
//     
//     VkImage ImageBase::handle() const {
//       return h;
//     }
//     
//     VmaAllocation ImageBase::allocation() const {
//       return a;
//     }
//     
//     void* ImageBase::ptr() const {
//       return pointer;
//     }
//     
//     ImageBase::operator VkImage() const {
//       return h;
//     }
//     
//     bool ImageBase::operator==(const ImageBase &another) const {
//       return h == another.h;
//     }
//     
//     bool ImageBase::operator!=(const ImageBase &another) const {
//       return h != another.h;
//     }
//     
//     DescriptorSetBase::DescriptorSetBase(Device* device, VkDescriptorSet h, VkDescriptorType descType) : freeData(SIZE_MAX), device(device), h(h), descType(descType) {}
//     DescriptorSetBase::~DescriptorSetBase() {}
//     
//     size_t DescriptorSetBase::add(const DescriptorSetData &data) {
//       size_t index;
//       
//       if (freeData == SIZE_MAX) {
//         index = datas.size();
//         datas.push_back(data);
//       } else {
//         index = freeData;
//         freeData = datas[freeData].arrayElement;
//         datas[index] = data;
//       }
//       
// //       int a = getIntTypeFromDescType(descType);
// //       
// //       if (a == 0) {
// //         // два раза будет обновляться если есть сэмплер и вью
// //         if (datas[index].imageData.sampler.valid()) {
// //           datas[index].imageData.sampler.setDescriptor(DescriptorSet(this), index);
// //         }
// //         
// //         if (datas[index].imageData.imageView.valid()) {
// //           datas[index].imageData.imageView->setDescriptor(DescriptorSet(this), index);
// //         }
// //       } else if (a == 1) {
// //         if (datas[index].bufferData.buffer.valid()) {
// //           datas[index].bufferData.buffer->setDescriptor(DescriptorSet(this), index);
// //         }
// //       } else {
// //         if (datas[index].view.valid()) {
// //           datas[index].view->setDescriptor(DescriptorSet(this), index);
// //         }
// //       }
//       
//       return index;
//     }
//     
//     void DescriptorSetBase::erase(const size_t &index) {
//       datas[index].arrayElement = freeData;
//       datas[index].bindingNum = UINT32_MAX;
//       freeData = index;
//     }
//     
//     void DescriptorSetBase::update(const size_t &index) {
//       int a = getIntTypeFromDescType(descType);
//       
//       if (index == SIZE_MAX) {
//         DescriptorUpdater du(device);
//         du.currentSet(h);
//         
//         for (size_t i = 0; i < datas.size(); ++i) {
//           du.begin(datas[i].bindingNum, datas[i].arrayElement, descType);
//           
//           if (a == 0) {
//             du.image(datas[i].imageData.imageView, datas[i].imageData.imageLayout, datas[i].imageData.sampler);
//           } else if (a == 1) {
//             du.buffer(datas[i].bufferData.buffer, datas[i].bufferData.offset, datas[i].bufferData.range);
//           } else {
//             //du.texelBuffer()
//             throw std::runtime_error("TODO: buffer view descriptor update");
//           }
//           
//           du.update();
//         }
//         
//         return;
//       }
//       
//       if (index >= datas.size()) return;
//       
//       if (datas[index].bindingNum == UINT32_MAX) return;
//       
//       DescriptorUpdater du(device);
//       du.currentSet(h);
//       du.begin(datas[index].bindingNum, datas[index].arrayElement, descType);
//       
//       if (a == 0) {
//         du.image(datas[index].imageData.imageView, datas[index].imageData.imageLayout, datas[index].imageData.sampler);
//       } else if (a == 1) {
//         du.buffer(datas[index].bufferData.buffer, datas[index].bufferData.offset, datas[index].bufferData.range);
//       } else {
//         //du.texelBuffer()
//         throw std::runtime_error("TODO: buffer view descriptor update");
//       }
//       
//       du.update();
//     }
//     
//     size_t DescriptorSetBase::size() const {
//       return datas.size();
//     }
//     
//     VkDescriptorSet DescriptorSetBase::handle() const {
//       return h;
//     }
//     
//     VkDescriptorType DescriptorSetBase::type() const {
//       return descType;
//     }
//     
//     DescriptorSetData & DescriptorSetBase::operator[](const size_t &index) {
//       return datas[index];
//     }
//     
//     const DescriptorSetData & DescriptorSetBase::operator[](const size_t &index) const {
//       return datas[index];
//     }
//     
//     DescriptorSetData & DescriptorSetBase::at(const size_t &index) {
//       return datas[index];
//     }
//     
//     const DescriptorSetData & DescriptorSetBase::at(const size_t &index) const {
//       return datas[index];
//     }
//     
//     DescriptorSetBase::operator VkDescriptorSet() const {
//       return h;
//     }
//     
//     bool DescriptorSetBase::operator==(const DescriptorSetBase &another) const {
//       return h == another.h;
//     }
//     
//     bool DescriptorSetBase::operator!=(const DescriptorSetBase &another) const {
//       return h != another.h;
//     }
//   }
// }
