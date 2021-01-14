#ifndef TASKS_H
#define TASKS_H

#include "Types.h"
#include "Internal.h"

#include <iostream>

namespace yavf {
  class Device;
  class RenderTarget;

  class TaskInterface;
  class SemaphoreProxy;
  class SemaphoreOwner;

  // каждый таск может создать прокси сигнального семафора
  // каждый таск может принять на вход прокси сигнального семафора
  // внутри прокси владелец будет менять текущий семафор
  // а остальные принимать семафор для того чтобы подождать его сигнала

  // проблемы возникают когда я захочу сделать параллельность внутри таска
  // но встанет ли у меня так задача когда нибудь?
  // семафор прокси выглядит неплохим выходом из положения в проблеме с рендертаргетом

  class TaskLayout {
  public:
    uint32_t next();
    void addTask();
    void addWaitSemaphore();
    void addSignalSemaphore();

    Internal::Queue start();
    std::vector<VkSubmitInfo> getSubmitInfo();
    VkResult wait(uint64_t time = 1000000000);
  private:
    struct BufferResolve {
      uint32_t stepIndex;
      uint32_t commandBufferIndex;
      TaskInterface* task;
    };

    struct SemaphoreResolve {
      uint32_t stepIndex;
      uint32_t waitSemaphoreIndex;
      RenderTarget* target;
    };

    struct StepData {
      std::vector<VkCommandBuffer> buffers;
      std::vector<VkSemaphore> waitSemaphores;
      std::vector<VkPipelineStageFlags> binds;
      std::vector<VkSemaphore> signalSemaphores;
    };

    Device* device;
    std::vector<VkSubmitInfo> infos;
    std::vector<StepData> datas;

    void resolve();
  };

  class TaskInterface {
  public:
    TaskInterface();
    virtual ~TaskInterface();

    virtual void begin(const VkCommandBufferUsageFlags &flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) = 0;
    virtual void end() = 0;

    void copy(Buffer* src, Buffer* dst); // fast copy whole Buffer*
    void copy(Buffer* src, Buffer* dst, const VkDeviceSize &srcOffset, const VkDeviceSize &dstOffset, const VkDeviceSize &size); // fast copy 1 Buffer* region
//     void copy(VkBuffer* src, VkBuffer* dst, const VkDeviceSize &srcOffset, const VkDeviceSize &dstOffset, const VkDeviceSize &size);
    void copy(Buffer* src, Buffer* dst, const std::vector<VkBufferCopy> &regions);

    void copy(Image* src, Image* dst, 
              const VkAccessFlags &aspect1 = VK_IMAGE_ASPECT_COLOR_BIT,
              const VkAccessFlags &aspect2 = VK_IMAGE_ASPECT_COLOR_BIT,
              const VkImageLayout &layoutSrc = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
              const VkImageLayout &layoutDst = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL); // fast copy whole Image*
    void copy(Image* src, Image* dst, const VkImageCopy &copyInfo,
              const VkImageLayout &layoutSrc = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
              const VkImageLayout &layoutDst = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL); // fast copy 1 Image* region
    void copy(Image* src, Image* dst, const std::vector<VkImageCopy> &copyInfos,
              const VkImageLayout &layoutSrc = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
              const VkImageLayout &layoutDst = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
//     void copy(VkImage src, VkImage dst, const VkImageCopy &copyInfo,
//               const VkImageLayout &layoutSrc = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
//               const VkImageLayout &layoutDst = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    void copy(Buffer* src, Image* dst, 
              const VkAccessFlags &aspect = VK_IMAGE_ASPECT_COLOR_BIT, const VkImageLayout &layoutDst = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL); // fast copy whole Buffer* to Image*
    void copy(Buffer* src, Image* dst, const VkBufferImageCopy &region,
              const VkImageLayout &layoutDst = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL); // fast copy 1 Buffer* region to Image*
    void copy(Buffer* src, Image* dst, const std::vector<VkBufferImageCopy> &regions,
              const VkImageLayout &layoutDst = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    void copy(Image* src, Buffer* dst, const VkAccessFlags &aspect = VK_IMAGE_ASPECT_COLOR_BIT, const VkImageLayout &layoutSrc = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL); // fast copy Image* to Buffer*
    void copy(Image* src, Buffer* dst, const VkBufferImageCopy &region,
              const VkImageLayout &layoutSrc = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL); // fast copy 1 Buffer* region to Image*
    void copy(Image* src, Buffer* dst, const std::vector<VkBufferImageCopy> &regions,
              const VkImageLayout &layoutSrc = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    void setBarrier(const VkPipelineStageFlags &srcFlags, const VkPipelineStageFlags &dstFlags, const VkImageMemoryBarrier &imageBarrier, const VkDependencyFlags &depFlags = 0);
    void setBarrier(Image* image, const VkImageLayout &newLayout, const VkAccessFlags &aspect = VK_IMAGE_ASPECT_COLOR_BIT,
                    const uint32_t &oldFamily = VK_QUEUE_FAMILY_IGNORED, const uint32_t &newFamily = VK_QUEUE_FAMILY_IGNORED);
    void setBarrier(Image* image, const VkImageLayout &newLayout, const VkImageSubresourceRange &range,
                    const uint32_t &oldFamily = VK_QUEUE_FAMILY_IGNORED, const uint32_t &newFamily = VK_QUEUE_FAMILY_IGNORED);
    void setBarrier(VkImage image, const VkImageLayout &oldLayout, const VkImageLayout &newLayout, const VkImageSubresourceRange &range,
                    const uint32_t &oldFamily = VK_QUEUE_FAMILY_IGNORED, const uint32_t &newFamily = VK_QUEUE_FAMILY_IGNORED);
    
    void setBarrier(const VkPipelineStageFlags &srcFlags, const VkPipelineStageFlags &dstFlags, const VkDependencyFlags &depFlags = 0);
    void setBarrier(const VkPipelineStageFlags &srcFlags, const VkPipelineStageFlags &dstFlags,
                    const VkAccessFlags &srcAccess, const VkAccessFlags &dstAccess,
                    const VkDependencyFlags &depFlags = 0);

    void setBarrier(Buffer* buffer, const VkPipelineStageFlags &srcFlags, const VkPipelineStageFlags &dstFlags,
                    const VkAccessFlags &srcAccess, const VkAccessFlags &dstAccess,
                    const VkDeviceSize &offset = SIZE_MAX, const VkDeviceSize &size = SIZE_MAX,
                    const uint32_t &oldFamily = VK_QUEUE_FAMILY_IGNORED, const uint32_t &newFamily = VK_QUEUE_FAMILY_IGNORED,
                    const VkDependencyFlags &depFlags = 0);
    void setBarrier(VkBuffer buffer, const VkPipelineStageFlags &srcFlags, const VkPipelineStageFlags &dstFlags,
                    const VkAccessFlags &srcAccess, const VkAccessFlags &dstAccess,
                    const VkDeviceSize &offset, const VkDeviceSize &size,
                    const uint32_t &oldFamily = VK_QUEUE_FAMILY_IGNORED, const uint32_t &newFamily = VK_QUEUE_FAMILY_IGNORED,
                    const VkDependencyFlags &depFlags = 0);

    void setBarrier(const VkPipelineStageFlags &srcFlags, const VkPipelineStageFlags &dstFlags,
                    const VkMemoryBarrier &memoryBarrier,
                    const VkDependencyFlags &depFlags = 0);

    void setBarrier(const VkPipelineStageFlags &srcFlags, const VkPipelineStageFlags &dstFlags,
                    const VkBufferMemoryBarrier &buffersMemoryBarrier,
                    const VkDependencyFlags &depFlags = 0);

    void setBarrier(const VkPipelineStageFlags &srcFlags, const VkPipelineStageFlags &dstFlags,
                    const VkMemoryBarrier &memoryBarrier,
                    const VkBufferMemoryBarrier &buffersMemoryBarrier,
                    const VkImageMemoryBarrier &imageMemoryBarrier,
                    const VkDependencyFlags &depFlags = 0);

    void setBarrier(const VkPipelineStageFlags &srcFlags, const VkPipelineStageFlags &dstFlags,
                    const std::vector<VkMemoryBarrier> &memoryBarriers,
                    const std::vector<VkBufferMemoryBarrier> &buffersMemoryBarriers,
                    const std::vector<VkImageMemoryBarrier> &imageMemoryBarriers,
                    const VkDependencyFlags &depFlags = 0);

    void update(Buffer* buffer, const VkDeviceSize &offset, const VkDeviceSize &size, const void* data);
    void update(VkBuffer buffer, const VkDeviceSize &offset, const VkDeviceSize &size, const void* data);
    void fill(Buffer* buffer, const VkDeviceSize &offset, const VkDeviceSize &size, const uint32_t &data);
    void fill(VkBuffer buffer, const VkDeviceSize &offset, const VkDeviceSize &size, const uint32_t &data);

    void execute(TaskInterface* task);
    void execute(const std::vector<TaskInterface*> &tasks);
    void execute(const size_t &count, TaskInterface** tasks);

    //virtual Internal::Queue start() = 0;
    Internal::Queue start();
    VkSubmitInfo getSubmitInfo() const;
    //virtual VkSubmitInfo getSubmitInfo() = 0;
    VkResult wait(const uint64_t &time = 1000000000);

//     void addWaitSemaphore(SemaphoreProxy* proxy);
//     virtual SemaphoreProxy* createSignalSemaphore(const VkPipelineStageFlags &flag = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT) = 0;

//     void addWaitSemaphore(VkSemaphore s, const VkPipelineStageFlags &flag = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
//     void addSignalSemaphore(VkSemaphore s);
//     virtual void clearWaitSemaphores();
//     virtual void clearSignalSemaphores();
//     virtual void setWaitSemaphores(const std::vector<VkSemaphore> &semaphores, const std::vector<VkPipelineStageFlags> &waitFlags = {});
//     virtual void setSignalSemaphores(const std::vector<VkSemaphore> &semaphores);

    //std::vector<SemaphoreProxy*> waitSemaphoresVector() const;
//     uint32_t waitSemaphoresCount() const;
//     void getWaitSemaphores(Semaphore* semaphores, VkPipelineStageFlags* flags) const;
//     uint32_t signalSemaphoresCount() const;
//     virtual void getSignalSemaphores(Semaphore* semaphores) const = 0;
    
    size_t waitSemaphoresSize() const;
    size_t signalSemaphoresSize() const;
    
    void pushWaitSemaphore(const Semaphore semaphore, const VkPipelineStageFlags &flag);
    void pushSignalSemaphore(const Semaphore semaphore);
    
    void setWaitSemaphore(const size_t &index, const Semaphore semaphore, const VkPipelineStageFlags &flag);
    void setSignalSemaphore(const size_t &index, const Semaphore semaphore);
    
    void eraseWaitSemaphore(const size_t &index);
    void eraseSignalSemaphore(const size_t &index);

    uint32_t getFamily() const;
    VkQueueFlags getQueueFlags() const;
    VkCommandBuffer getCommandBuffer() const;
  protected:
    bool primary;
    VkQueueFlags flags;
    uint32_t family;
    Internal::Queue currentQueue;
    Device* device;
    VkCommandBuffer current;

//     std::vector<VkSemaphore> waiting;
//     std::vector<VkPipelineStageFlags> waitFlags;
//     std::vector<VkSemaphore> signal;

//     std::vector<SemaphoreProxy*> waitSemaphores;
//     std::vector<SemaphoreOwner*> signalSemaphores;
    std::vector<Semaphore> waitSemaphores;
    std::vector<VkPipelineStageFlags> stageFlags;
    std::vector<Semaphore> signalSemaphores;
  };

  class TransferTask : public TaskInterface {
  public:
    TransferTask(Device* device, const bool &primary = true);
    virtual ~TransferTask();

    void begin(const VkCommandBufferUsageFlags &flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) override;
    void end() override;

//     Internal::Queue start() override;
    //VkSubmitInfo getSubmitInfo() override;

//     SemaphoreProxy* createSignalSemaphore(const VkPipelineStageFlags &flag = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT) override;
//     void getSignalSemaphores(Semaphore* semaphores) const override;
  protected:
    // TransferTask();

//     std::vector<Semaphore> semaphores;
  };

  class TaskCommands : public TaskInterface {
  public:
    virtual ~TaskCommands();

    virtual void setPipeline(const Pipeline &pipe) = 0;
//     virtual void setDescriptor(Image* Image*, const uint32_t &firstSet) = 0;
//     virtual void setDescriptor(Buffer* Buffer*, const uint32_t &firstSet) = 0;
//     virtual void setDescriptor(const Sampler &sampler, const uint32_t &firstSet) = 0;
//     virtual void setDescriptor(const Descriptor &descriptor, const uint32_t &firstSet) = 0;
//     virtual void setDescriptor(const std::vector<Descriptor> &descriptors, const uint32_t &firstSet) = 0;
    virtual void setDescriptor(DescriptorSet* descriptor, const uint32_t &firstSet, const uint32_t &offset = UINT32_MAX) = 0;
    virtual void setDescriptor(const VkDescriptorSet &descriptor, const uint32_t &firstSet, const uint32_t &offset = UINT32_MAX) = 0;
    virtual void setDescriptor(const std::initializer_list<VkDescriptorSet> &descriptors, const uint32_t &firstSet, const std::initializer_list<uint32_t> &offses = {}) = 0;
    virtual void setConsts(const uint32_t &offset, const uint32_t &size, void* value, const VkShaderStageFlags &flags = VK_SHADER_STAGE_VERTEX_BIT) = 0;

    // нужно переделать descriptor
    //virtual void setDescriptor(VkDescriptorSet set, const uint32_t &firstSet = UINT32_MAX) = 0;

    void clearImage(Image* image, const VkClearColorValue &value, const VkImageSubresourceRange &range);
    void clearImage(Image* image, const VkClearColorValue &value, const std::vector<VkImageSubresourceRange> &ranges);

    // с эвентами немного повременю
    //void setEvent(VkEvent event, const VkPipelineStageFlags &stage);
    //void resetEvent(VkEvent event, const VkPipelineStageFlags &stage);
    //void waitEvents();

    void beginQuery(VkQueryPool pool, const uint32_t &query, const VkQueryControlFlags &flags = 0);
    void endQuery(VkQueryPool pool, const uint32_t &query);
    void resetQueryPool(VkQueryPool pool, const uint32_t &firstQuery, const uint32_t &queryCount);
    void copyQuery(VkQueryPool pool, const uint32_t &firstQuery, const uint32_t &queryCount,
                   Buffer* buffer, const VkDeviceSize &offset = 0, const VkDeviceSize &stride = 0, const VkQueryResultFlags &flags = 0);
    void writeTimeStamp(const VkPipelineStageFlagBits &flags, VkQueryPool pool, const uint32_t &query);

//     SemaphoreProxy* createSignalSemaphore(const VkPipelineStageFlags &flag = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT) override;
//     void getSignalSemaphores(Semaphore* semaphores) const override;
  protected:
    Pipeline currentPipeline;

//     uint32_t currentFrameIndex = 0;
    //std::vector<VkDescriptorSet> currentSets;
//     std::vector<VkCommandBuffer*> commandBuffer*s;
//     std::vector<std::vector<Semaphore>> semaphores;
//     std::vector<VkPipelineStageFlags> semaphoreFlags;
  };

  class GraphicTask : public virtual TaskCommands {
  public:
    GraphicTask(Device* device, const bool &primary = true);
    virtual ~GraphicTask();
//, const bool addSemaphore = true
    void setRenderTarget(RenderTarget* target);

    void begin(const VkCommandBufferUsageFlags &flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) override;
    void end() override;

    void beginRenderPass(const VkSubpassContents &contents = VK_SUBPASS_CONTENTS_INLINE);
    void beginRenderPass(const VkRenderPass &pass, const VkSubpassContents &contents = VK_SUBPASS_CONTENTS_INLINE);
    void endRenderPass();

    void setViews(const std::vector<VkViewport> &views, const std::vector<VkRect2D> &scissors = {}, const uint32_t &firstView = 0, const uint32_t &firstScissor = 0);
    void setView(const VkViewport &view, const uint32_t &firstView = 0);
    void setView(const std::vector<VkViewport> &views, const uint32_t &firstView = 0);
    void setScissor(const VkRect2D &scissor, const uint32_t &firstView = 0);
    void setScissor(const std::vector<VkRect2D> &scissors, const uint32_t &firstScissor = 0);

    void setPipeline(const Pipeline &pipe) override;
//     void setDescriptor(Image* Image*, const uint32_t &firstSet) override;
//     void setDescriptor(Buffer* Buffer*, const uint32_t &firstSet) override;
//     void setDescriptor(const Sampler &sampler, const uint32_t &firstSet) override;
//     void setDescriptor(const Descriptor &descriptor, const uint32_t &firstSet) override;
//     void setDescriptor(const std::vector<Descriptor> &descriptors, const uint32_t &firstSet) override;
    void setDescriptor(DescriptorSet* descriptor, const uint32_t &firstSet, const uint32_t &offset = UINT32_MAX) override;
    void setDescriptor(const VkDescriptorSet &descriptor, const uint32_t &firstSet, const uint32_t &offset = UINT32_MAX) override;
    void setDescriptor(const std::initializer_list<VkDescriptorSet> &descriptors, const uint32_t &firstSet, const std::initializer_list<uint32_t> &offsets = {}) override;
    void setConsts(const uint32_t &offset, const uint32_t &size, void* value, const VkShaderStageFlags &flags = VK_SHADER_STAGE_VERTEX_BIT) override;

    void setIndexBuffer(Buffer* buffer, const VkIndexType &type = VK_INDEX_TYPE_UINT32, const VkDeviceSize &offset = 0);
    void setVertexBuffer(Buffer* buffer, const uint32_t &firstBinding, const VkDeviceSize &offset = 0);
//     void setInstanceBuffer*(Buffer* Buffer*, const uint32_t &firstBinding, const VkDeviceSize &offset = 0);
    
//     void setIndexBuffer(VkBuffer buffer, const VkIndexType &type = VK_INDEX_TYPE_UINT32, const VkDeviceSize &offset = 0);
//     void setVertexBuffer(VkBuffer buffer, const uint32_t &firstBinding, const VkDeviceSize &offset = 0);
    void setVertexBuffer(const std::vector<VkBuffer> &buffers, const uint32_t &firstBinding, const std::vector<VkDeviceSize> &offsets);
//     void setInstanceBuffer*(VkBuffer* Buffer*, const uint32_t &firstBinding, const VkDeviceSize &offset = 0);

    void nextSubpass();

//     void draw(const uint32_t &firstVertex = 0, const uint32_t &firstInstance = 0);
    void draw(const uint32_t &vertexCount, const uint32_t &instanceCount, const uint32_t &firstVertex, const uint32_t &firstInstance);
//     void drawIndexed(const uint32_t &firstIndex = 0, const int32_t &vertexOffset = 0, const uint32_t &firstInstance = 0);
    void drawIndexed(const uint32_t &indexCount, const uint32_t &instanceCount, const uint32_t &firstIndex, const int32_t &vertexOffset, const uint32_t &firstInstance);
    void drawIndirect(Buffer* buffer, const uint32_t &drawCount, const VkDeviceSize &offset = 0, const uint32_t &stride = sizeof(VkDrawIndirectCommand));
    void drawIndexedIndirect(Buffer* buffer, const uint32_t &drawCount, const VkDeviceSize &offset = 0, const uint32_t &stride = sizeof(VkDrawIndexedIndirectCommand));

    void clearAttachments(const std::vector<VkClearAttachment> &attachments, const std::vector<VkClearRect> &rects);

    void copyBlit(Image* src, Image* dst, const VkImageBlit &blitInfo, const VkFilter &filter = VK_FILTER_LINEAR,
                  const VkImageLayout &layoutSrc = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                  const VkImageLayout &layoutDst = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    void copyBlit(Image* src, Image* dst, const std::vector<VkImageBlit> &blitInfos, const VkFilter &filter = VK_FILTER_LINEAR,
                  const VkImageLayout &layoutSrc = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                  const VkImageLayout &layoutDst = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
//     void copyBlit(VkImage src, VkImage dst, const VkImageBlit &blitInfo, const VkFilter &filter = VK_FILTER_LINEAR,
//                   const VkImageLayout &layoutSrc = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
//                   const VkImageLayout &layoutDst = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    void resolve(Image* src, Image* dst,
                const VkAccessFlags &aspect1 = VK_IMAGE_ASPECT_COLOR_BIT, const VkAccessFlags &aspect2 = VK_IMAGE_ASPECT_COLOR_BIT,
                const VkImageLayout &layoutSrc = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                const VkImageLayout &layoutDst = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    void resolve(Image* src, Image* dst, const VkImageResolve &resolve,
                const VkImageLayout &layoutSrc = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                const VkImageLayout &layoutDst = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    void resolve(Image* src, Image* dst, const std::vector<VkImageResolve> &resolves,
                const VkImageLayout &layoutSrc = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                const VkImageLayout &layoutDst = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
//     void resolve(VkImage src, VkImage dst, const VkImageResolve &resolve,
//                 const VkImageLayout &layoutSrc = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
//                 const VkImageLayout &layoutDst = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    void clearDepthStencil(Image* image, const VkClearDepthStencilValue &value, const VkImageSubresourceRange &range);
    void clearDepthStencil(Image* image, const VkClearDepthStencilValue &value, const std::vector<VkImageSubresourceRange> &ranges);

    void setDepthBounds(const float &min, const float &max);
    void setLineWidth(const float &width);
    void setDepthBias(const float &constantFactor, const float &clamp, const float &slopeFactor);
    void setStencilCompareMask(const VkStencilFaceFlags &faceMask, const uint32_t &compareMask);
    void setStencilWriteMask(const VkStencilFaceFlags &faceMask, const uint32_t &writeMask);
    void setStencilReference(const VkStencilFaceFlags &faceMask, const uint32_t &reference);
    void setBlendConstants(const float consts[4]);

//     Internal::Queue start() override;
//     VkSubmitInfo getSubmitInfo() override;

//     void clearWaitSemaphores() override;
//     void clearSignalSemaphores() override;
//     void setWaitSemaphores(const std::vector<VkSemaphore> &semaphores, const std::vector<VkPipelineStageFlags> &waitFlags = {}) override;
//     void setSignalSemaphores(const std::vector<VkSemaphore> &semaphores) override;
  protected:
    GraphicTask();

    bool renderpassStart = false;

    uint32_t currentSubpass = 0;

//     uint32_t indexCount = 1;
//     uint32_t instanceCount = 1;
//     uint32_t vertexCount = 1;

    RenderTarget* target = nullptr;
  };

  class ComputeTask : public virtual TaskCommands {
  public:
    ComputeTask(Device* device, const bool &primary = true);
    virtual ~ComputeTask();

    void begin(const VkCommandBufferUsageFlags &flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) override;
    void end() override;

    void setPipeline(const Pipeline &pipe) override;
//     void setDescriptor(Image* Image*, const uint32_t &firstSet) override;
//     void setDescriptor(Buffer* Buffer*, const uint32_t &firstSet) override;
//     void setDescriptor(const Sampler &sampler, const uint32_t &firstSet) override;
//     void setDescriptor(const Descriptor &descriptor, const uint32_t &firstSet) override;
//     void setDescriptor(const std::vector<Descriptor> &descriptors, const uint32_t &firstSet) override;
    void setDescriptor(DescriptorSet* descriptor, const uint32_t &firstSet, const uint32_t &offset = UINT32_MAX) override;
    void setDescriptor(const VkDescriptorSet &descriptor, const uint32_t &firstSet, const uint32_t &offset = UINT32_MAX) override;
    void setDescriptor(const std::initializer_list<VkDescriptorSet> &descriptors, const uint32_t &firstSet, const std::initializer_list<uint32_t> &offsets = {}) override;
    void setConsts(const uint32_t &offset, const uint32_t &size, void* value, const VkShaderStageFlags &flags = VK_SHADER_STAGE_VERTEX_BIT) override;

    void dispatch(const uint32_t &countX, const uint32_t &countY, const uint32_t &countZ);
    void dispatchIndirect(Buffer* buffer, const VkDeviceSize &offset = 0);

//     Internal::Queue start() override;
//     VkSubmitInfo getSubmitInfo() override;
  protected:
    ComputeTask();
  };

  //, const uint32_t &framesCount
  class CombinedTask : public GraphicTask, public ComputeTask {
  public:
    CombinedTask(Device* device, const bool &primary = true);
    virtual ~CombinedTask();

    void begin(const VkCommandBufferUsageFlags &flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) override;
    void end() override;

    void setPipeline(const Pipeline &pipe) override;
//     void setDescriptor(Image* Image*, const uint32_t &firstSet) override;
//     void setDescriptor(Buffer* Buffer*, const uint32_t &firstSet) override;
//     void setDescriptor(const Sampler &sampler, const uint32_t &firstSet) override;
//     void setDescriptor(const Descriptor &descriptor, const uint32_t &firstSet) override;
//     void setDescriptor(const std::vector<Descriptor> &descriptors, const uint32_t &firstSet) override;
    void setDescriptor(DescriptorSet* descriptor, const uint32_t &firstSet, const uint32_t &offset = UINT32_MAX) override;
    void setDescriptor(const VkDescriptorSet &descriptor, const uint32_t &firstSet, const uint32_t &offset = UINT32_MAX) override;
    void setDescriptor(const std::initializer_list<VkDescriptorSet> &descriptors, const uint32_t &firstSet, const std::initializer_list<uint32_t> &offsets = {}) override;
    void setConsts(const uint32_t &offset, const uint32_t &size, void* value, const VkShaderStageFlags &flags = VK_SHADER_STAGE_VERTEX_BIT) override;

    void setComputePipeline(const Pipeline &pipe);
    void setGraphicPipeline(const Pipeline &pipe);
    void setBindPoint(const VkPipelineBindPoint &point);

//     Internal::Queue start() override;
//     VkSubmitInfo getSubmitInfo() override;

//     void clearWaitSemaphores() override;
//     void clearSignalSemaphores() override;
//     void setWaitSemaphores(const std::vector<VkSemaphore> &semaphores, const std::vector<VkPipelineStageFlags> &waitFlags = {}) override;
//     void setSignalSemaphores(const std::vector<VkSemaphore> &semaphores) override;
  protected:
    VkPipelineBindPoint lastBindPoint = VK_PIPELINE_BIND_POINT_MAX_ENUM;
    //std::vector<VkDescriptorSet> computeSets;
  };
}

#endif
