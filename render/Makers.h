#ifndef YAVF_MAKERS_H
#define YAVF_MAKERS_H

#include <vector>
#include <string>
#include <vulkan/vulkan.h>

#include "Core.h"
#include "Internal.h"
#include "Tasks.h"

#define YAVF_DEFAULT_COLOR_COMPONENT VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT

// 50 mb
#define YAVF_DEFAULT_BUFFER_BLOCK_SIZE 52428800
// 256 mb
#define YAVF_DEFAULT_IMAGE_BLOCK_SIZE 268435456

namespace yavf {
  class DescriptorPoolMaker {
  public:
    DescriptorPoolMaker(Device* device);

    DescriptorPoolMaker & flags(const VkDescriptorPoolCreateFlags &flags);
    DescriptorPoolMaker & poolSize(const VkDescriptorType &type, const uint32_t &count);

    DescriptorPool create(const std::string &name);
  protected:
    VkDescriptorPoolCreateFlags f = 0;
    
    Device* device = nullptr;
    std::vector<VkDescriptorPoolSize> sizes;
  };

  class DescriptorMaker {
  public:
    DescriptorMaker(Device* device);

    DescriptorMaker & layout(DescriptorSetLayout layout);
    //DescriptorMaker & data(const uint32_t &binding, const VkDescriptorType &type, const uint32_t &arrayElement = 0, const uint32_t &setNum = UINT32_MAX);
    // удалится сам при удалении VkDescriptorPool
    std::vector<DescriptorSet*> create(const DescriptorPool pool);
  protected:
    struct DescriptorData {
      uint32_t binding = 0;
      uint32_t arrayElement = 0;
      VkDescriptorType type;
    };

    Device* device = nullptr;
    std::vector<VkDescriptorSetLayout> layouts;
//     std::vector<VkDescriptorType> types;
//     std::vector<DescriptorData> datas;
  };

  class DescriptorUpdater {
  public:
    DescriptorUpdater(Device* device);

    DescriptorUpdater & currentSet(VkDescriptorSet set);
    DescriptorUpdater & begin(const uint32_t &binding, const uint32_t &arrayElement, VkDescriptorType type);

    DescriptorUpdater & image(Image* image, const VkImageLayout &layout = VK_IMAGE_LAYOUT_MAX_ENUM, Sampler sampler = VK_NULL_HANDLE);
    DescriptorUpdater & image(ImageView* image, const VkImageLayout &layout, Sampler sampler = VK_NULL_HANDLE);
    
    DescriptorUpdater & sampler(Sampler sampler);

    DescriptorUpdater & buffer(Buffer* buffer, const VkDeviceSize &offset = SIZE_MAX, const VkDeviceSize &size = 0);
//     DescriptorUpdater & buffer(Buffer buffer, const VkDeviceSize &offset, const VkDeviceSize &size);

    DescriptorUpdater & texelBuffer(BufferView* buffer);

    DescriptorUpdater & copy(DescriptorSet* srcSet, const uint32_t &srcBinding, const uint32_t &srcArrayElement, 
                             DescriptorSet* dstSet, const uint32_t &dstBinding, const uint32_t &dstArrayElement, 
                             const uint32_t &count);

    void update();
  protected:
    struct Write {
      uint8_t type = 0; // 0 == image, 1 == buffer, 2 == buffer view
      std::vector<VkDescriptorBufferInfo> buffers;
      std::vector<VkDescriptorImageInfo> images;
      std::vector<VkBufferView> bufferViews;
    };
    
    std::vector<Write> writes;
    std::vector<VkWriteDescriptorSet> infos;
    std::vector<VkCopyDescriptorSet> copies;

    VkDescriptorSet current = VK_NULL_HANDLE;
    Device* device = nullptr;
  };
  
  class FramebufferMaker {
  public:
    FramebufferMaker(Device* device);
    
    FramebufferMaker & renderpass(RenderPass pass);
    FramebufferMaker & addAttachment(ImageView* view);
    FramebufferMaker & dimensions(const uint32_t &width, const uint32_t &height);
    FramebufferMaker & layers(const uint32_t &layerCount);
    
    Framebuffer create(const std::string &name);
  protected:
    RenderPass pass;
    std::vector<VkImageView> views;
    uint32_t width;
    uint32_t height;
    uint32_t layerCount;
    
    Device* device = nullptr;
  };

  class SamplerMaker {
    //friend class Device;
  public:
    SamplerMaker(Device* device);

    SamplerMaker & filter(const VkFilter &min, const VkFilter &mag);
    SamplerMaker & mipmapMode(const VkSamplerMipmapMode &mode);
    SamplerMaker & addressMode(const VkSamplerAddressMode &u, const VkSamplerAddressMode &v, const VkSamplerAddressMode &w = VK_SAMPLER_ADDRESS_MODE_REPEAT);
    SamplerMaker & anisotropy(const VkBool32 &enable, const float &max = 1.0f);
    SamplerMaker & compareOp(const VkBool32 &enable, const VkCompareOp &op);
    SamplerMaker & lod(const float &min, const float &max, const float &bias = 0.0f);
    SamplerMaker & borderColor(const VkBorderColor &color);
    SamplerMaker & unnormalizedCoordinates(const VkBool32 &enable);

    Sampler create(const std::string &name);
    //VkSampler createNative();
  protected:
    Device* device = nullptr;
    VkSamplerCreateInfo info;
  };

  class DescriptorLayoutMaker {
  public:
    DescriptorLayoutMaker(Device* device);

    DescriptorLayoutMaker & binding(const uint32_t &bind, const VkDescriptorType &type, const VkShaderStageFlags &stage, const uint32_t &count = 1);
    DescriptorLayoutMaker & samplers(const uint32_t &bind, const VkDescriptorType &type, const VkShaderStageFlags &stage, const std::vector<VkSampler> &samplers);

    DescriptorSetLayout create(const std::string &name);
  protected:
    Device* device = nullptr;
    std::vector<VkDescriptorSetLayoutBinding> bindings;
  };

  class PipelineLayoutMaker {
  public:
    PipelineLayoutMaker(Device* device);

    PipelineLayoutMaker & addDescriptorLayout(DescriptorSetLayout setLayout);
    PipelineLayoutMaker & addPushConstRange(const uint32_t &offset, const uint32_t &size, const VkShaderStageFlags &flag = VK_SHADER_STAGE_VERTEX_BIT);

    PipelineLayout create(const std::string &name);
  protected:
    Device* device;
    std::vector<DescriptorSetLayout> setLayouts;
    std::vector<VkPushConstantRange> ranges;
  };

  class PipelineMaker {
  public:
    PipelineMaker(Device* device);

    //PipelineMaker & addShader(const VkShaderStageFlagBits &flag, const std::string &path, const char* name = nullptr);
    PipelineMaker & addShader(const VkShaderStageFlagBits &flag, const VkShaderModule &module, const char* name = nullptr);
    PipelineMaker & addSpecializationEntry(const uint32_t &constantID, const uint32_t &offset, const size_t &size);
    yavf::PipelineMaker& addData(const size_t& size, const void* data);

    PipelineMaker & vertexBinding(const uint32_t &binding, const uint32_t &stride, const VkVertexInputRate &rate = VK_VERTEX_INPUT_RATE_VERTEX);
    PipelineMaker & vertexAttribute(const uint32_t &location, const uint32_t &binding, const VkFormat &format, const uint32_t &offset);
    PipelineMaker & viewport(const VkViewport &view);
    PipelineMaker & viewport();
    PipelineMaker & scissor(const VkRect2D &scissor);
    PipelineMaker & scissor();

    PipelineMaker & dynamicState(const VkDynamicState &state);

    PipelineMaker & assembly(const VkPrimitiveTopology &topology, const VkBool32 &primitiveRestartEnable = VK_FALSE);
    PipelineMaker & tessellation(bool enabled, const uint32_t &patchControlPoints = 0);

    PipelineMaker & depthClamp(const VkBool32 &enable);
    PipelineMaker & rasterizerDiscard(const VkBool32 &enable);
    PipelineMaker & polygonMode(const VkPolygonMode &mode);
    PipelineMaker & cullMode(const VkCullModeFlags &flags);
    PipelineMaker & frontFace(const VkFrontFace &face);
    PipelineMaker & depthBias(const VkBool32 &enable, const float &constFactor = 0.0f, const float &clamp = 0.0f, const float &slopeFactor = 0.0f);
    PipelineMaker & lineWidth(const float &lineWidth);

    PipelineMaker & rasterizationSamples(const VkSampleCountFlagBits &count);
    PipelineMaker & sampleShading(const VkBool32 &enable, const float &minSampleShading = 0.0f, const VkSampleMask* masks = nullptr);
    PipelineMaker & multisampleCoverage(const VkBool32 &alphaToCoverage, const VkBool32 &alphaToOne);

    PipelineMaker & depthTest(const VkBool32 &enable);
    PipelineMaker & depthWrite(const VkBool32 &enable);
    PipelineMaker & compare(const VkCompareOp &compare);
    PipelineMaker & stencilTest(const VkBool32 &enable, const VkStencilOpState &front = {}, const VkStencilOpState &back = {});
    PipelineMaker & depthBounds(const VkBool32 &enable, const float &minBounds = 0.0f, const float &maxBounds = 0.0f);

    PipelineMaker & logicOp(const VkBool32 &enable, const VkLogicOp &logic = VK_LOGIC_OP_COPY);
    PipelineMaker & blendConstant(const float* blendConst);
    PipelineMaker & colorBlending(const VkPipelineColorBlendAttachmentState &state);

    PipelineMaker & addDefaultBlending();
    PipelineMaker & clearBlending();

    PipelineMaker & colorBlendBegin(const VkBool32 &enable = VK_TRUE);
    PipelineMaker & srcColor(const VkBlendFactor &value);
    PipelineMaker & dstColor(const VkBlendFactor &value);
    PipelineMaker & colorOp(const VkBlendOp &value);
    PipelineMaker & srcAlpha(const VkBlendFactor &value);
    PipelineMaker & dstAlpha(const VkBlendFactor &value);
    PipelineMaker & alphaOp(const VkBlendOp &value);
    PipelineMaker & colorWriteMask(const VkColorComponentFlags &flags = YAVF_DEFAULT_COLOR_COMPONENT);

    Pipeline create(const std::string &name,
                    VkPipelineLayout layout,
                    VkRenderPass renderPass,
                    const uint32_t &subpass = 0,
                    VkPipeline base = VK_NULL_HANDLE,
                    const int32_t &baseIndex = -1);
  protected:
    struct ShaderSpecialization {
      std::vector<VkSpecializationMapEntry> entries;
      size_t dataSize;
      const void* data;
    };

    bool tessellationState = false;
    Device* device = nullptr;

    VkPipelineVertexInputStateCreateInfo vertexInfo;
    VkPipelineInputAssemblyStateCreateInfo inputAssembly;
    VkPipelineTessellationStateCreateInfo tessellationInfo;
    VkPipelineViewportStateCreateInfo viewportInfo;
    VkPipelineRasterizationStateCreateInfo rasterisationInfo;
    VkPipelineMultisampleStateCreateInfo multisamplingInfo;
    VkPipelineDepthStencilStateCreateInfo depthStensilInfo;
    VkPipelineColorBlendStateCreateInfo colorBlendingInfo;
    VkPipelineDynamicStateCreateInfo dymStateInfo;
    VkPipelineColorBlendAttachmentState defaultBlending;

    std::vector<VkPipelineShaderStageCreateInfo> shaders;
    std::vector<ShaderSpecialization> specs;

    std::vector<VkVertexInputBindingDescription> vertexBindings;
    std::vector<VkVertexInputAttributeDescription> vertexAttribs;
    std::vector<VkViewport> viewports;
    std::vector<VkRect2D> scissors;
    std::vector<VkPipelineColorBlendAttachmentState> colorBlends;
    std::vector<VkDynamicState> dynStates;
  };
  
  class ComputePipelineMaker {
  public:
    ComputePipelineMaker(Device* device);
    
    //ComputePipelineMaker & shader(const VkShaderStageFlagBits &flag, const std::string &path, const char* name = nullptr);
    ComputePipelineMaker & shader(const VkShaderModule &module, const char* name = nullptr);
    ComputePipelineMaker & addSpecializationEntry(const uint32_t &constantID, const uint32_t &offset, const size_t &size);
    ComputePipelineMaker & addData(const size_t &size, void* data);
    
    Pipeline create(const std::string &name,
                    VkPipelineLayout layout,
                    VkPipeline base = VK_NULL_HANDLE,
                    const int32_t &baseIndex = -1);
  protected:
    Device* device = nullptr;
    
    size_t dataSize = 0;
    void* data = nullptr;
    VkPipelineShaderStageCreateInfo shaderInfo;
    std::vector<VkSpecializationMapEntry> entries;
  };

  class RenderPassMaker {
  public:
    RenderPassMaker(Device* device);

    RenderPassMaker & attachmentBegin(const VkFormat &format);
    RenderPassMaker & attachmentFlags(const VkAttachmentDescriptionFlags &value);
    RenderPassMaker & attachmentFormat(const VkFormat &value);
    RenderPassMaker & attachmentSamples(const VkSampleCountFlagBits &value);
    RenderPassMaker & attachmentLoadOp(const VkAttachmentLoadOp &value);
    RenderPassMaker & attachmentStoreOp(const VkAttachmentStoreOp &value);
    RenderPassMaker & attachmentStencilLoadOp(const VkAttachmentLoadOp &value);
    RenderPassMaker & attachmentStencilStoreOp(const VkAttachmentStoreOp &value);
    RenderPassMaker & attachmentInitialLayout(const VkImageLayout &value);
    RenderPassMaker & attachmentFinalLayout(const VkImageLayout &value);

    RenderPassMaker & subpassBegin(const VkPipelineBindPoint &bind);
    RenderPassMaker & subpassInputAttachment(const VkImageLayout &layout, const uint32_t &attachment);
    RenderPassMaker & subpassColorAttachment(const VkImageLayout &layout, const uint32_t &attachment);
    RenderPassMaker & subpassResolveAttachment(const VkImageLayout &layout, const uint32_t &attachment);
    RenderPassMaker & subpassDepthStencilAttachment(const VkImageLayout &layout, const uint32_t &attachment);
    RenderPassMaker & addPreservedAttachmentIndex(const uint32_t &index);

    RenderPassMaker & dependencyBegin(const uint32_t &srcSubpass, const uint32_t &dstSubpass);
    RenderPassMaker & dependencySrcSubpass(const uint32_t &value);
    RenderPassMaker & dependencyDstSubpass(const uint32_t &value);
    RenderPassMaker & dependencySrcStageMask(const VkPipelineStageFlags &value);
    RenderPassMaker & dependencyDstStageMask(const VkPipelineStageFlags &value);
    RenderPassMaker & dependencySrcAccessMask(const VkAccessFlags &value);
    RenderPassMaker & dependencyDstAccessMask(const VkAccessFlags &value);
    RenderPassMaker & dependencyDependencyFlags(const VkDependencyFlags &value);

    RenderPass create(const std::string &name);
  protected:
    struct SubpassDescription {
      VkSubpassDescriptionFlags flags;
      VkPipelineBindPoint pipelineBindPoint;
      
      std::vector<VkAttachmentReference> input; // уникальный
      std::vector<VkAttachmentReference> color;
      std::vector<VkAttachmentReference> resolve;
      VkAttachmentReference stensil = {0, VK_IMAGE_LAYOUT_MAX_ENUM};
      std::vector<uint32_t> preservedAttachments;
    };
    
    Device* device = nullptr;
    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkSubpassDependency> dependencies;
    std::vector<SubpassDescription> descriptions;
  };

  // layers такие же как и у инстанса
  class DeviceMaker {
  public:
//     static void addExtension(const char* ext);
//     static void setExtensions(const std::vector<const char*> &extensions);
//     static std::vector<const char*> & getExtensions();
    
    DeviceMaker(Instance* inst);

    DeviceMaker & beginDevice(const VkPhysicalDevice phys);
    DeviceMaker & createQueues(const uint32_t &maxCount = 4, const float* priority = nullptr);
    // нужно добавить еще создание комманд пулов
    DeviceMaker & bufferBlockSize(const size_t &size);
    DeviceMaker & imageBlockSize(const size_t &size);
    DeviceMaker & features(const VkPhysicalDeviceFeatures &f);
    DeviceMaker & setExtensions(const std::vector<const char*> &extensions, bool printExtensionInfo = false);

    Device* create(const std::vector<const char*> &layers, const std::string &name);
  protected:
    bool printExtensionInfo;
    size_t bufferBlock = YAVF_DEFAULT_BUFFER_BLOCK_SIZE;
    size_t imageBlock = YAVF_DEFAULT_IMAGE_BLOCK_SIZE;
    
    Instance* inst = nullptr;
    VkPhysicalDevice phys = VK_NULL_HANDLE;

    float** priorities = nullptr;

    std::vector<const char*> extensions;
    std::vector<VkDeviceQueueCreateInfo> queueInfos;
    std::vector<Internal::QueueInfo> familyProperties;

    VkPhysicalDeviceFeatures f;
    VkDeviceCreateInfo info;
    
//     static std::vector<const char*> deviceExtensions;
  };
}

#endif
