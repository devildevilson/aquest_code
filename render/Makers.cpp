#include "Makers.h"

#include "Internal.h"

// #include <fstream>
#include <iostream>

namespace yavf {
  DescriptorPoolMaker::DescriptorPoolMaker(Device* device) {
    this->device = device;
  }
  
  DescriptorPoolMaker & DescriptorPoolMaker::flags(const VkDescriptorPoolCreateFlags &flags) {
    f = flags;

    return *this;
  }
  
  DescriptorPoolMaker & DescriptorPoolMaker::poolSize(const VkDescriptorType &type, const uint32_t &count) {
    sizes.push_back({type, count});

    return *this;
  }
  
  DescriptorPool DescriptorPoolMaker::create(const std::string &name) {
    uint32_t maxSets = 0;
    for (size_t i = 0; i < sizes.size(); ++i) {
      maxSets += sizes[i].descriptorCount;
    }
    
    const VkDescriptorPoolCreateInfo info{
      VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      nullptr,
      f,
      maxSets,
      static_cast<uint32_t>(sizes.size()),
      sizes.data()
    };

    DescriptorPool p = device->create(info, name);
    
    // VkDescriptorPool p = VK_NULL_HANDLE;
    // vkCheckError("vkCreateDescriptorPool", nullptr, 
    // vkCreateDescriptorPool(device->handle(), &info, nullptr, &p));

    sizes.clear();
    
    //device->pools[name] = p;
    
    return p;
  }
  
  DescriptorMaker::DescriptorMaker(Device* device) {
    this->device = device;
  }
  
  DescriptorMaker & DescriptorMaker::layout(VkDescriptorSetLayout layout) {
    layouts.push_back(layout);
//     datas.push_back({});

    return *this;
  }

//   DescriptorMaker & DescriptorMaker::data(const uint32_t &binding, const VkDescriptorType &type, const uint32_t &arrayElement, const uint32_t &setNum) {
//     datas.back().binding = binding;
//     datas.back().arrayElement = arrayElement;
//     datas.back().type = type;
//     datas.back().setNum = setNum;
// 
//     return *this;
//   }
  
  std::vector<DescriptorSet*> DescriptorMaker::create(VkDescriptorPool pool) {
    std::vector<VkDescriptorSet> sets(layouts.size());

    const VkDescriptorSetAllocateInfo info{
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      nullptr,
      pool,
      static_cast<uint32_t>(layouts.size()),
      layouts.data()
    };

    vkCheckError("vkAllocateDescriptorSets", nullptr, 
    vkAllocateDescriptorSets(device->handle(), &info, sets.data()));
    
    std::vector<DescriptorSet*> descriptors(layouts.size());
    for (size_t i = 0; i < layouts.size(); ++i) {
      descriptors[i] = device->create(pool, sets[i]);
    }

    //std::vector<Descriptor> descs(layouts.size());
//     for (size_t i = 0; i < layouts.size(); ++i) {
//       descs[i].handle = sets[i];
//       descs[i].setNum = datas[i].setNum;
//       descs[i].bindingNum = datas[i].binding;
//       descs[i].arrayElement = datas[i].arrayElement;
//       descs[i].type = datas[i].type;
//     }
    
    layouts.clear();
//     datas.clear();

    return descriptors;
  }

  DescriptorUpdater::DescriptorUpdater(Device* device) {
    this->device = device;
  }

  DescriptorUpdater & DescriptorUpdater::currentSet(VkDescriptorSet set) {
    current = set;

    return *this;
  }

  DescriptorUpdater & DescriptorUpdater::begin(const uint32_t &binding, const uint32_t &arrayElement, VkDescriptorType type) {
    const VkWriteDescriptorSet info{
      VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      nullptr,
      current,
      binding,
      arrayElement,
      0,
      type,
      nullptr,
      nullptr,
      nullptr
    };

    infos.push_back(info);
    writes.push_back({});

    switch(type) {
      case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
      case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
        writes.back().type = 2;
        break;
      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
        writes.back().type = 1;
        break;
      default:
        writes.back().type = 0;
        break;
    }

    return *this;
  }

  DescriptorUpdater & DescriptorUpdater::image(Image* image, const VkImageLayout &layout, Sampler sampler) {
    const VkDescriptorImageInfo info{
      sampler,
      image->view()->handle(),
      layout == VK_IMAGE_LAYOUT_MAX_ENUM ? image->info().initialLayout : layout
    };

    writes.back().images.push_back(info);

    return *this;
  }

  DescriptorUpdater & DescriptorUpdater::image(ImageView* image, const VkImageLayout &layout, Sampler sampler) {
    const VkDescriptorImageInfo info{
      sampler,
      (image == nullptr ? VK_NULL_HANDLE : image->handle()),
      layout
    };

    writes.back().images.push_back(info);

    return *this;
  }

  DescriptorUpdater & DescriptorUpdater::sampler(Sampler sampler) {
    const VkDescriptorImageInfo info{
      sampler,
      VK_NULL_HANDLE,
      VK_IMAGE_LAYOUT_MAX_ENUM
    };

    writes.back().images.push_back(info);

    return *this;
  }

  DescriptorUpdater & DescriptorUpdater::buffer(Buffer* buffer, const VkDeviceSize &offset, const VkDeviceSize &size) {
    const VkDescriptorBufferInfo info{
      buffer->handle(),
      offset == SIZE_MAX ? 0 : offset,
      size == 0 ? buffer->info().size : size
    };

    writes.back().buffers.push_back(info);

    return *this;
  }

//   DescriptorUpdater & DescriptorUpdater::buffer(VkBuffer buffer, const VkDeviceSize &offset, const VkDeviceSize &size) {
//     const VkDescriptorBufferInfo info{
//       buffer,
//       offset,
//       size
//     };
// 
//     writes.back().buffers.push_back(info);
// 
//     return *this;
//   }

  DescriptorUpdater & DescriptorUpdater::texelBuffer(BufferView* buffer) {
    writes.back().bufferViews.push_back(buffer->handle());

    return *this;
  }

  DescriptorUpdater & DescriptorUpdater::copy(DescriptorSet* srcSet, const uint32_t &srcBinding, const uint32_t &srcArrayElement, 
                                              DescriptorSet* dstSet, const uint32_t &dstBinding, const uint32_t &dstArrayElement, 
                                              const uint32_t &count) {
    const VkCopyDescriptorSet copyInfo{
      VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET,
      nullptr,
      srcSet->handle(),
      srcBinding,
      srcArrayElement,
      dstSet->handle(),
      dstBinding,
      dstArrayElement,
      count
    };

    copies.push_back(copyInfo);

    return *this;
  }

  void DescriptorUpdater::update() {
    for (size_t i = 0; i < infos.size(); ++i) {
      infos[i].descriptorCount = writes[i].type == 2 ? writes[i].bufferViews.size() : writes[i].type == 1 ? writes[i].buffers.size() : writes[i].images.size();
      
      infos[i].pImageInfo = writes[i].type == 0 ? writes[i].images.data() : nullptr;
      infos[i].pBufferInfo = writes[i].type == 1 ? writes[i].buffers.data() : nullptr;
      infos[i].pTexelBufferView = writes[i].type == 2 ? writes[i].bufferViews.data() : nullptr;
    }
    
    if (infos.empty()) exit(-1);

    vkUpdateDescriptorSets(device->handle(), infos.size(), infos.data(), copies.size(), copies.data());
    
    writes.clear();
    infos.clear();
    copies.clear();
    
    current = VK_NULL_HANDLE;
  }
  
  FramebufferMaker::FramebufferMaker(Device* device) {
    this->device = device;
  }
    
  FramebufferMaker & FramebufferMaker::renderpass(RenderPass pass) {
    this->pass = pass;
    
    return *this;
  }
  
  FramebufferMaker & FramebufferMaker::addAttachment(ImageView* view) {
    views.push_back(view->handle());
    
    return *this;
  }
  
  FramebufferMaker & FramebufferMaker::dimensions(const uint32_t &width, const uint32_t &height) {
    this->width = width;
    this->height = height;
    
    return *this;
  }
  
  FramebufferMaker & FramebufferMaker::layers(const uint32_t &layerCount) {
    this->layerCount = layerCount;
    
    return *this;
  }
  
  Framebuffer FramebufferMaker::create(const std::string &name) {
//     const VkFramebufferCreateInfo i{
//       VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
//       nullptr,
//       0,
//       pass,
//       static_cast<uint32_t>(views.size()),
//       views.data(),
//       width,
//       height,
//       layerCount
//     };
    
    Framebuffer f = device->create(FramebufferCreateInfo::framebuffer(pass, static_cast<uint32_t>(views.size()), views.data(), width, height, layerCount), name);
    
//     VkFramebuffer f = VK_NULL_HANDLE;
//     vkCheckError("vkCreateFramebuffer", nullptr, 
//     vkCreateFramebuffer(device->handle(), &i, nullptr, &f));
//     
//     device->framebuffers.push_back(f);
    
    return f;
  }
  
  SamplerMaker::SamplerMaker(Device* device) {
    this->device = device;

    info = {
      VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
      nullptr,
      0,
      VK_FILTER_LINEAR,
      VK_FILTER_LINEAR,
      VK_SAMPLER_MIPMAP_MODE_LINEAR,
      VK_SAMPLER_ADDRESS_MODE_REPEAT,
      VK_SAMPLER_ADDRESS_MODE_REPEAT,
      VK_SAMPLER_ADDRESS_MODE_REPEAT,
      0.0f,
      VK_FALSE,
      0.0f,
      VK_FALSE,
      VK_COMPARE_OP_ALWAYS,
      0.0f,
      1.0f,
      VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
      VK_FALSE
    };
  }
  
  SamplerMaker & SamplerMaker::filter(const VkFilter &min, const VkFilter &mag) {
    info.minFilter = min;
    info.magFilter = mag;

    return *this;
  }
  
  SamplerMaker & SamplerMaker::mipmapMode(const VkSamplerMipmapMode &mode) {
    info.mipmapMode = mode;

    return *this;
  }
  
  SamplerMaker & SamplerMaker::addressMode(const VkSamplerAddressMode &u, const VkSamplerAddressMode &v, const VkSamplerAddressMode &w) {
    info.addressModeU = u;
    info.addressModeV = v;
    info.addressModeW = w;

    return *this;
  }
  
  SamplerMaker & SamplerMaker::anisotropy(const VkBool32 &enable, const float &max) {
    info.anisotropyEnable = enable;
    info.maxAnisotropy = max;

    return *this;
  }
  
  SamplerMaker & SamplerMaker::compareOp(const VkBool32 &enable, const VkCompareOp &op) {
    info.compareEnable = enable;
    info.compareOp = op;

    return *this;
  }
  
  SamplerMaker & SamplerMaker::lod(const float &min, const float &max, const float &bias) {
    info.minLod = min;
    info.maxLod = max;
    info.mipLodBias = bias;

    return *this;
  }
  
  SamplerMaker & SamplerMaker::borderColor(const VkBorderColor &color) {
    info.borderColor = color;

    return *this;
  }
  
  SamplerMaker & SamplerMaker::unnormalizedCoordinates(const VkBool32 &enable) {
    info.unnormalizedCoordinates = enable;

    return *this;
  }
  
  Sampler SamplerMaker::create(const std::string &name) {
    Sampler s = device->create(info, name);

    // VkSampler s = VK_NULL_HANDLE;
    // vkCheckError("vkCreateSampler", nullptr, 
    // vkCreateSampler(device->handle(), &info, nullptr, &s));

    // Sampler sampler;
    // sampler.sampler = s;
    // sampler.device = device;

    // device->samplers[name] = sampler;

    return s;
  }
  
  // VkSampler SamplerMaker::createNative() {
  //   VkSampler s = VK_NULL_HANDLE;
  //   vkCheckError("vkCreateSampler", nullptr, 
  //   vkCreateSampler(device->handle(), &info, nullptr, &s));

  //   device->samplerContainer.push_back(s);

  //   return s;
  // }
  
  DescriptorLayoutMaker::DescriptorLayoutMaker(Device* device) {
    this->device = device;
  }
  
  DescriptorLayoutMaker & DescriptorLayoutMaker::binding(const uint32_t &bind, const VkDescriptorType &type, const VkShaderStageFlags &stage, const uint32_t &count) {
    const VkDescriptorSetLayoutBinding b{
      bind,
      type,
      count,
      stage,
      nullptr
    };
    
    bindings.push_back(b);

    return *this;
  }
  
  DescriptorLayoutMaker & DescriptorLayoutMaker::samplers(const uint32_t &bind, const VkDescriptorType &type, const VkShaderStageFlags &stage, const std::vector<VkSampler> &samplers) {
    const VkDescriptorSetLayoutBinding b{
      bind,
      type,
      static_cast<uint32_t>(samplers.size()),
      stage,
      samplers.data()
    };
    
    bindings.push_back(b);

    return *this;
  }
  
  VkDescriptorSetLayout DescriptorLayoutMaker::create(const std::string &name) {
    const VkDescriptorSetLayoutCreateInfo info{
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      nullptr,
      0,
      static_cast<uint32_t>(bindings.size()),
      bindings.data()
    };

    DescriptorSetLayout sl = device->create(info, name);
    
    // VkDescriptorSetLayout sl = VK_NULL_HANDLE;
    // vkCheckError("vkCreateDescriptorSetLayout", name.c_str(), 
    // vkCreateDescriptorSetLayout(device->handle(), &info, nullptr, &sl));
    
    bindings.clear();
    
    //device->setLayouts[name] = sl;
    
    return sl;
  }
  
  PipelineLayoutMaker::PipelineLayoutMaker(Device* device) : device(device) {}
  
  PipelineLayoutMaker & PipelineLayoutMaker::addDescriptorLayout(VkDescriptorSetLayout setLayout) {
    setLayouts.push_back(setLayout);

    return *this;
  }
  
  PipelineLayoutMaker & PipelineLayoutMaker::addPushConstRange(const uint32_t &offset, const uint32_t &size, const VkShaderStageFlags &flag) {
    const VkPushConstantRange range{
      flag,
      offset,
      size
    };
    
    ranges.push_back(range);

    return *this;
  }
  
  PipelineLayout PipelineLayoutMaker::create(const std::string &name) {
    const VkPipelineLayoutCreateInfo info{
      VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      nullptr,
      0,
      static_cast<uint32_t>(setLayouts.size()),
      setLayouts.data(),
      static_cast<uint32_t>(ranges.size()),
      ranges.data()
    };
    
    PipelineLayout l = device->create(info, name);

//     std::cout << name << " " << l << "\n";
//     for (size_t i = 0; i < setLayouts.size(); ++i) {
//       std::cout << "layout " << i << " " << setLayouts[i] << "\n";
//     }
//     std::cout << "\n";
    
    setLayouts.clear();
    ranges.clear();
    
    //device->layouts[name] = l;
    return l;
  }
  
  PipelineMaker::PipelineMaker(Device* device) {
    this->device = device;
    
    vertexInfo = {
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      nullptr,
      0,
      0, nullptr,
      0, nullptr
    };
    
    inputAssembly = {
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      nullptr,
      0,
      VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
      VK_FALSE
    };
    
    tessellationInfo = {
      VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
      nullptr,
      0,
      0
    };
    
    viewportInfo = {
      VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      nullptr,
      0,
      0, nullptr,
      0, nullptr
    };
    
    rasterisationInfo = {
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      nullptr,
      0,
      VK_FALSE,
      VK_FALSE,
      VK_POLYGON_MODE_FILL,
      VK_CULL_MODE_NONE,
      VK_FRONT_FACE_COUNTER_CLOCKWISE,
      VK_FALSE,
      0.0f, 
      0.0f, 
      0.0f,
      1.0f
    };
    
    multisamplingInfo = {
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      nullptr,
      0,
      VK_SAMPLE_COUNT_1_BIT,
      VK_FALSE,
      1.0f,
      nullptr,
      VK_FALSE,
      VK_FALSE
    };
    
    depthStensilInfo = {
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
      nullptr,
      0,
      VK_FALSE,
      VK_FALSE,
      VK_COMPARE_OP_LESS,
      VK_FALSE,
      VK_FALSE,
      {
        VK_STENCIL_OP_ZERO,
        VK_STENCIL_OP_KEEP,
        VK_STENCIL_OP_ZERO,
        VK_COMPARE_OP_EQUAL,
        0,
        0,
        0
      },
      {
        VK_STENCIL_OP_ZERO,
        VK_STENCIL_OP_KEEP,
        VK_STENCIL_OP_ZERO,
        VK_COMPARE_OP_EQUAL,
        0,
        0,
        0
      },
      0.0f, 
      0.0f
    };
    
    colorBlendingInfo = {
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      nullptr,
      0,
      VK_FALSE,
      VK_LOGIC_OP_COPY,
      0,
      nullptr,
      {0.0f, 0.0f, 0.0f, 0.0f}
    };
    
    dymStateInfo = {
      VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      nullptr,
      0,
      0,
      nullptr
    };
    
    defaultBlending = {
      VK_TRUE,
//       VK_BLEND_FACTOR_SRC_COLOR,
      VK_BLEND_FACTOR_SRC_ALPHA,
//       VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
      VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
      VK_BLEND_OP_ADD,
      VK_BLEND_FACTOR_SRC_ALPHA,
//       VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
      VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
//       VK_BLEND_FACTOR_SRC_ALPHA,
      VK_BLEND_OP_ADD,
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };
  }

//   PipelineMaker & PipelineMaker::addShader(const VkShaderStageFlagBits &flag, const std::string &path, const char* name) {
//     auto file = std::ifstream(path, std::ios::binary);
//     if (!file) {
//       YAVF_ERROR_REPORT("addShader", "bad file path", 0)
//     }
//     
//     file.seekg(0, std::ios::end);
//     size_t length = file.tellg();
//     
//     std::vector<char> opcode(length);
//     file.seekg(0, std::ios::beg);
//     file.read(opcode.data(), opcode.size());
//     
//     VkShaderModuleCreateInfo info{
//       VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
//       nullptr,
//       0,
//       opcode.size(),
//       (uint32_t*)opcode.data()
//     };
//     
//     VkShaderModule sm = VK_NULL_HANDLE;
//     vkCheckError("vkCreateShaderModule", nullptr, 
//     vkCreateShaderModule(device->handle(), &info, nullptr, &sm));
//     
//     VkPipelineShaderStageCreateInfo stageInfo{
//       VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
//       nullptr,
//       0,
//       flag,
//       sm,
//       name == nullptr ? "main" : name,
//       nullptr
//     };
//     
//     shaders.push_back(stageInfo);
//     specs.push_back({});
// 
//     return *this;
//   }
  
  PipelineMaker & PipelineMaker::addShader(const VkShaderStageFlagBits &flag, const VkShaderModule &module, const char* name) {
    const VkPipelineShaderStageCreateInfo stageInfo{
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      nullptr,
      0,
      flag,
      module,
      name == nullptr ? "main" : name,
      nullptr
    };
    
    shaders.push_back(stageInfo);
    specs.push_back({});

    return *this;
  }

  PipelineMaker & PipelineMaker::addSpecializationEntry(const uint32_t &constantID, const uint32_t &offset, const size_t &size) {
    specs.back().entries.push_back({constantID, offset, size});

    return *this;
  }

  PipelineMaker & PipelineMaker::addData(const size_t &size, void* data) {
    specs.back().dataSize = size;
    specs.back().data = data;

    return *this;
  }

  PipelineMaker & PipelineMaker::vertexBinding(const uint32_t &binding, const uint32_t &stride, const VkVertexInputRate &rate) {
    VkVertexInputBindingDescription desc{
      binding,
      stride,
      rate
    };
    
    vertexBindings.push_back(desc);

    return *this;
  }

  PipelineMaker & PipelineMaker::vertexAttribute(const uint32_t &location, const uint32_t &binding, const VkFormat &format, const uint32_t &offset) {
    VkVertexInputAttributeDescription desc{
      location,
      binding,
      format,
      offset
    };
    
    vertexAttribs.push_back(desc);

    return *this;
  }

  PipelineMaker & PipelineMaker::viewport(const VkViewport &view) {
    viewports.push_back(view);

    return *this;
  }
  
  PipelineMaker & PipelineMaker::viewport() {
    viewports.push_back({});

    return *this;
  }

  PipelineMaker & PipelineMaker::scissor(const VkRect2D &scissor) {
    scissors.push_back(scissor);

    return *this;
  }
  
  PipelineMaker & PipelineMaker::scissor() {
    scissors.push_back({});

    return *this;
  }

  PipelineMaker & PipelineMaker::dynamicState(const VkDynamicState &state) {
    dynStates.push_back(state);

    return *this;
  }


  PipelineMaker & PipelineMaker::assembly(const VkPrimitiveTopology &topology, const VkBool32 &primitiveRestartEnable) {
    inputAssembly.topology = topology;
    inputAssembly.primitiveRestartEnable = primitiveRestartEnable;

    return *this;
  }

  PipelineMaker & PipelineMaker::tessellation(bool enabled, const uint32_t &patchControlPoints) {
    tessellationState = enabled;
    tessellationInfo.patchControlPoints = patchControlPoints;

    return *this;
  }


  PipelineMaker & PipelineMaker::depthClamp(const VkBool32 &enable) {
    rasterisationInfo.depthBiasClamp = enable;

    return *this;
  }

  PipelineMaker & PipelineMaker::rasterizerDiscard(const VkBool32 &enable) {
    rasterisationInfo.rasterizerDiscardEnable = enable;

    return *this;
  }

  PipelineMaker & PipelineMaker::polygonMode(const VkPolygonMode &mode) {
    rasterisationInfo.polygonMode = mode;

    return *this;
  }

  PipelineMaker & PipelineMaker::cullMode(const VkCullModeFlags &flags) {
    rasterisationInfo.cullMode = flags;

    return *this;
  }

  PipelineMaker & PipelineMaker::frontFace(const VkFrontFace &face) {
    rasterisationInfo.frontFace = face;

    return *this;
  }

  PipelineMaker & PipelineMaker::depthBias(const VkBool32 &enable, const float &constFactor, const float &clamp, const float &slopeFactor) {
    rasterisationInfo.depthClampEnable = enable;
    rasterisationInfo.depthBiasConstantFactor = constFactor;
    rasterisationInfo.depthBiasClamp = clamp;
    rasterisationInfo.depthBiasSlopeFactor = slopeFactor;

    return *this;
  }
  
  PipelineMaker & PipelineMaker::lineWidth(const float &lineWidth) {
    rasterisationInfo.lineWidth = lineWidth;

    return *this;
  }


  PipelineMaker & PipelineMaker::rasterizationSamples(const VkSampleCountFlagBits &count) {
    multisamplingInfo.rasterizationSamples = count;

    return *this;
  }

  PipelineMaker & PipelineMaker::sampleShading(const VkBool32 &enable, const float &minSampleShading, const VkSampleMask* masks) {
    multisamplingInfo.sampleShadingEnable = enable;
    multisamplingInfo.minSampleShading = minSampleShading;
    multisamplingInfo.pSampleMask = masks;

    return *this;
  }

  PipelineMaker & PipelineMaker::multisampleCoverage(const VkBool32 &alphaToCoverage, const VkBool32 &alphaToOne) {
    multisamplingInfo.alphaToCoverageEnable = alphaToCoverage;
    multisamplingInfo.alphaToOneEnable = alphaToOne;

    return *this;
  }


  PipelineMaker & PipelineMaker::depthTest(const VkBool32 &enable) {
    depthStensilInfo.depthTestEnable = enable;

    return *this;
  }

  PipelineMaker & PipelineMaker::depthWrite(const VkBool32 &enable) {
    depthStensilInfo.depthWriteEnable = enable;

    return *this;
  }
  
  PipelineMaker & PipelineMaker::compare(const VkCompareOp &compare) {
    depthStensilInfo.depthCompareOp = compare;

    return *this;
  }

  PipelineMaker & PipelineMaker::stencilTest(const VkBool32 &enable, const VkStencilOpState &front, const VkStencilOpState &back) {
    depthStensilInfo.stencilTestEnable = enable;
    depthStensilInfo.front = front;
    depthStensilInfo.back = back;

    return *this;
  }

  PipelineMaker & PipelineMaker::depthBounds(const VkBool32 &enable, const float &minBounds, const float &maxBounds) {
    depthStensilInfo.depthBoundsTestEnable = enable;
    depthStensilInfo.minDepthBounds = minBounds;
    depthStensilInfo.maxDepthBounds = maxBounds;

    return *this;
  }


  PipelineMaker & PipelineMaker::logicOp(const VkBool32 &enable, const VkLogicOp &logic) {
    colorBlendingInfo.logicOpEnable = enable;
    colorBlendingInfo.logicOp = logic;

    return *this;
  }

  PipelineMaker & PipelineMaker::blendConstant(const float* blendConst) {
    colorBlendingInfo.blendConstants[0] = blendConst[0];
    colorBlendingInfo.blendConstants[1] = blendConst[1];
    colorBlendingInfo.blendConstants[2] = blendConst[2];
    colorBlendingInfo.blendConstants[3] = blendConst[3];

    return *this;
  }

  PipelineMaker & PipelineMaker::colorBlending(const VkPipelineColorBlendAttachmentState &state) {
    colorBlends.push_back(state);

    return *this;
  }


  PipelineMaker & PipelineMaker::addDefaultBlending() {
    colorBlends.push_back(defaultBlending);

    return *this;
  }

  PipelineMaker & PipelineMaker::clearBlending() {
    colorBlends.clear();

    return *this;
  }

  PipelineMaker & PipelineMaker::colorBlendBegin(const VkBool32 &enable) {
    VkPipelineColorBlendAttachmentState state{
      enable,
      VK_BLEND_FACTOR_SRC_ALPHA,
      VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
      VK_BLEND_OP_ADD,
      VK_BLEND_FACTOR_SRC_ALPHA,
      VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
      VK_BLEND_OP_ADD,
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };

    colorBlends.push_back(state);

    return *this;
  }

  PipelineMaker & PipelineMaker::srcColor(const VkBlendFactor &value) {
    colorBlends.back().srcColorBlendFactor = value;

    return *this;
  }
  
  PipelineMaker & PipelineMaker::dstColor(const VkBlendFactor &value) {
    colorBlends.back().dstColorBlendFactor = value;

    return *this;
  }
  
  PipelineMaker & PipelineMaker::colorOp(const VkBlendOp &value) {
    colorBlends.back().colorBlendOp = value;

    return *this;
  }
  
  PipelineMaker & PipelineMaker::srcAlpha(const VkBlendFactor &value) {
    colorBlends.back().srcAlphaBlendFactor = value;

    return *this;
  }
  
  PipelineMaker & PipelineMaker::dstAlpha(const VkBlendFactor &value) {
    colorBlends.back().dstAlphaBlendFactor = value;

    return *this;
  }
  
  PipelineMaker & PipelineMaker::alphaOp(const VkBlendOp &value) {
    colorBlends.back().alphaBlendOp = value;

    return *this;
  }
  
  PipelineMaker & PipelineMaker::colorWriteMask(const VkColorComponentFlags &flags) {
    colorBlends.back().colorWriteMask = flags;

    return *this;
  }
  

  Pipeline PipelineMaker::create(const std::string &name,
                                 VkPipelineLayout layout,
                                 VkRenderPass renderPass,
                                 const uint32_t &subpass,
                                 VkPipeline base,
                                 const int32_t &baseIndex) {
    const VkPipelineVertexInputStateCreateInfo vertex{
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      nullptr,
      0,
      static_cast<uint32_t>(vertexBindings.size()),
      vertexBindings.data(),
      static_cast<uint32_t>(vertexAttribs.size()),
      vertexAttribs.data()
    };
    
    const VkPipelineViewportStateCreateInfo viewport{
      VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      nullptr,
      0,
      static_cast<uint32_t>(viewports.size()),
      viewports.data(),
      static_cast<uint32_t>(scissors.size()),
      scissors.data()
    };
    
    const VkPipelineDynamicStateCreateInfo dynInfo{
      VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      nullptr,
      0,
      static_cast<uint32_t>(dynStates.size()),
      dynStates.data()
    };

    colorBlendingInfo.attachmentCount = colorBlends.size();
    colorBlendingInfo.pAttachments = colorBlends.data();

    std::vector<VkSpecializationInfo> infos(shaders.size());
    for (uint32_t i = 0; i < shaders.size(); ++i) {
      infos[i].mapEntryCount = specs[i].entries.size();
      infos[i].pMapEntries = specs[i].entries.data();
      infos[i].dataSize = specs[i].dataSize;
      infos[i].pData = specs[i].data;

      shaders[i].pSpecializationInfo = specs[i].entries.empty() ? nullptr : &infos[i];
    }
    
    const VkGraphicsPipelineCreateInfo info{
      VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      nullptr,
      0,
      static_cast<uint32_t>(shaders.size()),
      shaders.data(),
      &vertex,
      &inputAssembly,
      tessellationState ? &tessellationInfo : nullptr,
      &viewport,
      &rasterisationInfo,
      &multisamplingInfo,
      &depthStensilInfo,
      &colorBlendingInfo,
      dynStates.empty() ? nullptr : &dynInfo,
      layout,
      renderPass,
      subpass,
      base,
      baseIndex
    };
    
    Pipeline p = device->create(VK_NULL_HANDLE, info, name);

    // VkPipeline p = VK_NULL_HANDLE;
    // vkCheckError("vkCreateGraphicsPipelines", name.c_str(), 
    // vkCreateGraphicsPipelines(device->handle(), VK_NULL_HANDLE, 1, &info, nullptr, &p));
    
//     for (size_t i = 0; i < shaders.size(); ++i) {
//       vkDestroyShaderModule(device->handle(), shaders[i].module, nullptr);
//     }
    
    shaders.clear();
    vertexBindings.clear();
    vertexAttribs.clear();
    viewports.clear();
    scissors.clear();
    dynStates.clear();
    
    // Pipeline pipeline(p, layout);
    // device->pipelines[name] = pipeline;
    
    return p;
  }
  
  ComputePipelineMaker::ComputePipelineMaker(Device* device) {
    this->device = device;
    
    shaderInfo = {
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      nullptr,
      0,
      VK_SHADER_STAGE_ALL,
      VK_NULL_HANDLE,
      nullptr,
      nullptr
    };
  }
    
//   ComputePipelineMaker & ComputePipelineMaker::shader(const VkShaderStageFlagBits &flag, const std::string &path, const char* name) {
//     if (shaderInfo.module != VK_NULL_HANDLE) {
//       vkDestroyShaderModule(device->handle(), shaderInfo.module, nullptr);
//       shaderInfo.module = VK_NULL_HANDLE;
//     }
//     
//     auto file = std::ifstream(path, std::ios::binary);
//     if (!file) {
//       YAVF_ERROR_REPORT("addShader", ("file " + path + " not found").c_str(), 0)
//     }
//       
//     file.seekg(0, std::ios::end);
//     size_t length = file.tellg();
//     
//     std::vector<char> opcode(length);
//     file.seekg(0, std::ios::beg);
//     file.read(opcode.data(), opcode.size());
//     
//     VkShaderModuleCreateInfo info{
//       VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
//       nullptr,
//       0,
//       opcode.size(),
//       (uint32_t*)opcode.data()
//     };
//     
//     VkShaderModule sm = VK_NULL_HANDLE;
//     vkCheckError("vkCreateShaderModule", nullptr, 
//     vkCreateShaderModule(device->handle(), &info, nullptr, &sm));
//     
//     shaderInfo = {
//       VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
//       nullptr,
//       0,
//       flag,
//       sm,
//       name == nullptr ? "main" : name,
//       nullptr
//     };
//     
//     return *this;
//   }

  //const VkShaderStageFlagBits &flag, 
  ComputePipelineMaker & ComputePipelineMaker::shader(const VkShaderModule &module, const char* name) {
    shaderInfo = {
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      nullptr,
      0,
      //flag,
      VK_SHADER_STAGE_COMPUTE_BIT,
      module,
      name == nullptr ? "main" : name,
      nullptr
    };
    
    return *this;
  }
  
  ComputePipelineMaker & ComputePipelineMaker::addSpecializationEntry(const uint32_t &constantID, const uint32_t &offset, const size_t &size) {
    const VkSpecializationMapEntry entry{
      constantID,
      offset,
      size
    };
    
    entries.push_back(entry);
    
    return *this;
  }
  
  ComputePipelineMaker & ComputePipelineMaker::addData(const size_t &size, void* data) {
    this->dataSize = size;
    this->data = data;
    
    return *this;
  }
  
  Pipeline ComputePipelineMaker::create(const std::string &name,
                                        VkPipelineLayout layout,
                                        VkPipeline base,
                                        const int32_t &baseIndex) {
    const VkSpecializationInfo specInfo{
      static_cast<uint32_t>(entries.size()),
      entries.data(),
      dataSize,
      data
    };
    
    shaderInfo.pSpecializationInfo = &specInfo;
    
    const VkComputePipelineCreateInfo info{
      VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
      nullptr,
      0,
      shaderInfo,
      layout,
      base,
      baseIndex
    };

    Pipeline p = device->create(VK_NULL_HANDLE, info, name);
    
    // VkPipeline p = VK_NULL_HANDLE;
    // vkCheckError("vkCreateComputePipelines", nullptr, 
    // vkCreateComputePipelines(device->handle(), VK_NULL_HANDLE, 1, &info, nullptr, &p));
    
//     vkDestroyShaderModule(device->handle(), shaderInfo.module, nullptr);
//     shaderInfo.module = VK_NULL_HANDLE;
    entries.clear();
    
    // Pipeline pipeline(p, layout);
    // //auto itr = device->pipelines.find(name);
    // // нужно будет все равно сделать проверку на имена переменных
    // device->pipelines[name] = pipeline;
    
    return p;
  }
  
  RenderPassMaker::RenderPassMaker(Device* device) {
    this->device = device;
  }

  RenderPassMaker & RenderPassMaker::attachmentBegin(const VkFormat &format) {
    const VkAttachmentDescription desc{
      0,
      VK_FORMAT_R8G8B8A8_UNORM,
      VK_SAMPLE_COUNT_1_BIT,
      VK_ATTACHMENT_LOAD_OP_CLEAR,
      VK_ATTACHMENT_STORE_OP_STORE,
      VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      VK_ATTACHMENT_STORE_OP_DONT_CARE,
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };
    
    attachments.push_back(desc);
    attachments.back().format = format;
    
    return *this;
  }
  
  RenderPassMaker & RenderPassMaker::attachmentFlags(const VkAttachmentDescriptionFlags &value) {
    attachments.back().flags = value;
    
    return *this;
  }
  
  RenderPassMaker & RenderPassMaker::attachmentFormat(const VkFormat &value) {
    attachments.back().format = value;
    
    return *this;
  }
  
  RenderPassMaker & RenderPassMaker::attachmentSamples(const VkSampleCountFlagBits &value) {
    attachments.back().samples = value;
    
    return *this;
  }
  
  RenderPassMaker & RenderPassMaker::attachmentLoadOp(const VkAttachmentLoadOp &value) {
    attachments.back().loadOp = value;
    
    return *this;
  }
  
  RenderPassMaker & RenderPassMaker::attachmentStoreOp(const VkAttachmentStoreOp &value) {
    attachments.back().storeOp = value;
    
    return *this;
  }
  
  RenderPassMaker & RenderPassMaker::attachmentStencilLoadOp(const VkAttachmentLoadOp &value) {
    attachments.back().stencilLoadOp = value;
    
    return *this;
  }
  
  RenderPassMaker & RenderPassMaker::attachmentStencilStoreOp(const VkAttachmentStoreOp &value) {
    attachments.back().stencilStoreOp = value;
    
    return *this;
  }
  
  RenderPassMaker & RenderPassMaker::attachmentInitialLayout(const VkImageLayout &value) {
    attachments.back().initialLayout = value;
    
    return *this;
  }
  
  RenderPassMaker & RenderPassMaker::attachmentFinalLayout(const VkImageLayout &value) {
    attachments.back().finalLayout = value;
    
    return *this;
  }
  

  RenderPassMaker & RenderPassMaker::subpassBegin(const VkPipelineBindPoint &bind) {
    SubpassDescription desc{};
    descriptions.push_back(desc);
    descriptions.back().pipelineBindPoint = bind;
    
    return *this;
  }
  
  RenderPassMaker & RenderPassMaker::subpassInputAttachment(const VkImageLayout &layout, const uint32_t &attachment) {
    descriptions.back().input.push_back({attachment, layout});
    
    return *this;
  }
  
  RenderPassMaker & RenderPassMaker::subpassColorAttachment(const VkImageLayout &layout, const uint32_t &attachment) {
    descriptions.back().color.push_back({attachment, layout});
    
    return *this;
  }
  
  RenderPassMaker & RenderPassMaker::subpassResolveAttachment(const VkImageLayout &layout, const uint32_t &attachment) {
    descriptions.back().resolve.push_back({attachment, layout});
    
    return *this;
  }
  
  RenderPassMaker & RenderPassMaker::subpassDepthStencilAttachment(const VkImageLayout &layout, const uint32_t &attachment) {
    descriptions.back().stensil = {attachment, layout};
    
    return *this;
  }
  
  RenderPassMaker & RenderPassMaker::addPreservedAttachmentIndex(const uint32_t &index) {
    descriptions.back().preservedAttachments.push_back(index);
    
    return *this;
  }
  

  RenderPassMaker & RenderPassMaker::dependencyBegin(const uint32_t &srcSubpass, const uint32_t &dstSubpass) {
    const VkSubpassDependency dep{
      srcSubpass,
      dstSubpass,
      VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      VK_ACCESS_MEMORY_READ_BIT,
      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
      0
    };
    
    dependencies.push_back(dep);
    dependencies.back().srcSubpass = srcSubpass;
    dependencies.back().dstSubpass = dstSubpass;
    
    return *this;
  }
  
  RenderPassMaker & RenderPassMaker::dependencySrcSubpass(const uint32_t &value) {
    dependencies.back().srcSubpass = value;
    
    return *this;
  }
  
  RenderPassMaker & RenderPassMaker::dependencyDstSubpass(const uint32_t &value) {
    dependencies.back().dstSubpass = value;
    
    return *this;
  }
  
  RenderPassMaker & RenderPassMaker::dependencySrcStageMask(const VkPipelineStageFlags &value) {
    dependencies.back().srcStageMask = value;
    
    return *this;
  }
  
  RenderPassMaker & RenderPassMaker::dependencyDstStageMask(const VkPipelineStageFlags &value) {
    dependencies.back().dstStageMask = value;
    
    return *this;
  }
  
  RenderPassMaker & RenderPassMaker::dependencySrcAccessMask(const VkAccessFlags &value) {
    dependencies.back().srcAccessMask = value;
    
    return *this;
  }
  
  RenderPassMaker & RenderPassMaker::dependencyDstAccessMask(const VkAccessFlags &value) {
    dependencies.back().dstAccessMask = value;
    
    return *this;
  }
  
  RenderPassMaker & RenderPassMaker::dependencyDependencyFlags(const VkDependencyFlags &value) {
    dependencies.back().dependencyFlags = value;
    
    return *this;
  }
  

  RenderPass RenderPassMaker::create(const std::string &name) {
    std::vector<VkSubpassDescription> descs;
    
    for (size_t i = 0; i < descriptions.size(); ++i) {
      const VkSubpassDescription info{
        0,
        descriptions[i].pipelineBindPoint,
        static_cast<uint32_t>(descriptions[i].input.size()),
        descriptions[i].input.data(),
        static_cast<uint32_t>(descriptions[i].color.size()),
        descriptions[i].color.data(),
        descriptions[i].resolve.data(),
        descriptions[i].stensil.layout == VK_IMAGE_LAYOUT_MAX_ENUM ? nullptr : &descriptions[i].stensil,
        static_cast<uint32_t>(descriptions[i].preservedAttachments.size()),
        descriptions[i].preservedAttachments.data()
      };
      
      descs.push_back(info);
    }
    
    const VkRenderPassCreateInfo info{
      VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      nullptr,
      0,
      static_cast<uint32_t>(attachments.size()),
      attachments.data(),
      static_cast<uint32_t>(descs.size()),
      descs.data(),
      static_cast<uint32_t>(dependencies.size()),
      dependencies.data()
    };
    
    RenderPass newPass = device->create(info, name);
    
    attachments.clear();
    descs.clear();
    descriptions.clear();

    // VkRenderPass newPass = VK_NULL_HANDLE;
    // vkCheckError("vkCreateRenderPass", name.c_str(), 
    // vkCreateRenderPass(device->handle(), &info, nullptr, &newPass));
    
    // device->passes[name] = newPass;
    
    return newPass;
  }
  
  
//   void DeviceMaker::addExtension(const char* ext) {
//     deviceExtensions.push_back(ext);
//   }
//   
//   void DeviceMaker::setExtensions(const std::vector<const char*> &extensions) {
//     deviceExtensions = extensions;
//   }
//   
//   std::vector<const char*> & DeviceMaker::getExtensions() {
//     return deviceExtensions;
//   }
  
  DeviceMaker::DeviceMaker(Instance* inst) {
    this->inst = inst;
    
    info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;
    info.queueCreateInfoCount = 0;
    info.pQueueCreateInfos = nullptr;
    info.enabledLayerCount = 0;
    info.ppEnabledLayerNames = nullptr;
    info.enabledExtensionCount = 0;
    info.ppEnabledExtensionNames = nullptr;
    info.pEnabledFeatures = nullptr;

    f = {};
  }
  
  DeviceMaker & DeviceMaker::beginDevice(const VkPhysicalDevice phys) {
    this->phys = phys;

    return *this;
  }

  DeviceMaker & DeviceMaker::createQueues(const uint32_t &maxCount, const float* priority) {
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(phys, &count, nullptr);
    
    std::vector<VkQueueFamilyProperties> props(count);
    vkGetPhysicalDeviceQueueFamilyProperties(phys, &count, props.data());
    
    priorities = new float*[props.size()];
    
    for (size_t i = 0; i < props.size(); ++i) {
      uint32_t queuesCount = std::min(maxCount, props[i].queueCount);
      
      priorities[i] = new float[queuesCount];
      
      for (uint32_t j = 0; j < queuesCount; ++j) {
        priorities[i][j] = priority == nullptr ? 1.0f : priority[j];
      }
      
      VkDeviceQueueCreateInfo info{
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        nullptr,
        0,
        static_cast<uint32_t>(i),
        queuesCount,
        priorities[i]
      };
      queueInfos.push_back(info);
      
      familyProperties.emplace_back();
      familyProperties.back().count = queuesCount;
      familyProperties.back().flags = props[i].queueFlags;
    }

    return *this;
  }
  
  DeviceMaker & DeviceMaker::bufferBlockSize(const size_t &size) {
    bufferBlock = size;
    
    return *this;
  }
  
  DeviceMaker & DeviceMaker::imageBlockSize(const size_t &size) {
    imageBlock = size;
    
    return *this;
  }
  
  DeviceMaker & DeviceMaker::features(const VkPhysicalDeviceFeatures &f) {
    this->f = f;

    return *this;
  }
  
  DeviceMaker & DeviceMaker::setExtensions(const std::vector<const char*> &extensions, bool printExtensionInfo) {
    this->extensions = extensions;
    this->printExtensionInfo = printExtensionInfo;
    
    return *this;
  }

  //Device* DeviceMaker::create(const std::string &name) {
  Device* DeviceMaker::create(const std::vector<const char*> &layers, const std::string &name) {
    std::vector<const char*> lay(layers);
    getRequiredValidationLayers(lay);
    
    //const auto &ext = getRequiredDeviceExtensions(phys, Instance::getLayers(), extensions);
    const auto &ext = getRequiredDeviceExtensions(phys, lay, extensions);
    
    if (printExtensionInfo) {
      YAVF_INFO_REPORT("")
      YAVF_INFO_REPORT("Using device extensions:")
      for (const auto &e : ext) {
        YAVF_INFO_REPORT(("Name: "+std::string(e.extensionName)).c_str())
        YAVF_INFO_REPORT(("Version: "+VK_VERSION_TO_STRING(e.specVersion)).c_str())
      }
    }

    info.queueCreateInfoCount = queueInfos.size();
    info.pQueueCreateInfos = queueInfos.data();
//     info.enabledLayerCount = Instance::getLayers().size();
//     info.ppEnabledLayerNames = Instance::getLayers().data();
    info.enabledLayerCount = lay.size();
    info.ppEnabledLayerNames = lay.data();
    info.enabledExtensionCount = extensions.size();
    info.ppEnabledExtensionNames = extensions.data();
    info.pEnabledFeatures = &f;
    
    VkDevice dev = VK_NULL_HANDLE;
    vkCheckError("vkCreateDevice", name.c_str(), 
    vkCreateDevice(phys, &info, nullptr, &dev));
    
    for (uint32_t i = 0; i < queueInfos.size(); ++i) {
      delete [] priorities[i];
    }
    delete [] priorities;
    
    queueInfos.clear();
    extensions.clear();
    
    const Device::CreateInfo i{
      inst,
      dev,
      phys,
      static_cast<uint32_t>(familyProperties.size()),
      familyProperties.data(),
      bufferBlock,
      imageBlock
    };
    
    //Device* newDevice = new Device(dev, phys, familyProperties);
    Device* newDevice = inst->create(i, name);
//     Device* newDevice = new Device(i);
//     inst->devices[name] = newDevice;
    
    return newDevice;
  }
  
//   std::vector<const char*> DeviceMaker::deviceExtensions;
}
