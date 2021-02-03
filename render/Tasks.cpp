#include "Tasks.h"

#include "Core.h"

#ifdef _DEBUG
  #include <cassert>
  #define ASSERT(expr) assert(expr)
#else
  #define ASSERT(expr)
#endif
//#include "Types.h"

void getBarrierData(const VkImageLayout &old, const VkImageLayout &New, VkAccessFlags &srcFlags, VkAccessFlags &dstFlags, VkPipelineStageFlags &srcStage, VkPipelineStageFlags &dstStage) {
  switch (old) {
    case VK_IMAGE_LAYOUT_UNDEFINED:
      srcFlags = 0;
      srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

      break;
    case VK_IMAGE_LAYOUT_PREINITIALIZED:
      srcFlags = VK_ACCESS_HOST_WRITE_BIT;
      srcStage = VK_PIPELINE_STAGE_HOST_BIT;

      break;
    case VK_IMAGE_LAYOUT_GENERAL:
      srcFlags = VK_ACCESS_SHADER_WRITE_BIT;
      srcStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

      break;
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
      srcFlags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

      break;
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
      srcFlags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
      srcStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

      break;
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
      srcFlags = VK_ACCESS_TRANSFER_READ_BIT;
      srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

      break;
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
      srcFlags = VK_ACCESS_TRANSFER_WRITE_BIT;
      srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

      break;
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
      srcFlags = VK_ACCESS_SHADER_READ_BIT;
      srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

      break;
    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
      srcFlags = VK_ACCESS_MEMORY_READ_BIT;
      srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

      break;
    default:
      YAVF_WARNING_REPORT("This layout is not supported yet")
      break;
  }

  switch (New) {
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
      dstFlags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

      break;
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
      dstFlags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
      dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

      break;
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
      dstFlags = VK_ACCESS_TRANSFER_READ_BIT;
      dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

      break;
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
      dstFlags = VK_ACCESS_TRANSFER_WRITE_BIT;
      dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

      break;
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
      if (srcFlags == 0) {
        srcFlags = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_HOST_BIT;
      }

      dstFlags = VK_ACCESS_SHADER_READ_BIT;
      dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

      break;
    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
      dstFlags = VK_ACCESS_MEMORY_READ_BIT;
      dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

      break;
    case VK_IMAGE_LAYOUT_GENERAL:
      dstFlags = VK_ACCESS_SHADER_WRITE_BIT;
      dstStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

      break;
    default:
      YAVF_WARNING_REPORT("This layout is not supported yet")
      break;
  }
}

namespace yavf {
  TaskInterface::TaskInterface() : primary(true), flags(0), family(0), currentQueue{VK_NULL_HANDLE, VK_NULL_HANDLE}, device(nullptr), current(VK_NULL_HANDLE) {}
  TaskInterface::~TaskInterface() {
//     for (uint32_t i = 0; i < signalSemaphores.size(); ++i) {
//       device->destroySemaphoreProxy(signalSemaphores[i]);
//     }
  }

  void TaskInterface::copy(Buffer* src, Buffer* dst) {
    //if (src->param().size > dst->param().size) throw std::runtime_error(std::to_string(src->param().size) + " > " + std::to_string(dst->param().size));
    
    const VkBufferCopy region{
      0,
      0,
      std::min(src->info().size, dst->info().size)
      //src->param().size
    };

//     ASSERT((src->info().usage & VK_BUFFER_USAGE_TRANSFER_SRC_BIT) == VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
//     ASSERT((dst->info().usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT) == VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    
    vkCmdCopyBuffer(current, src->handle(), dst->handle(), 1, &region);
  }

  void TaskInterface::copy(Buffer* src, Buffer* dst, const VkDeviceSize &srcOffset, const VkDeviceSize &dstOffset, const VkDeviceSize &size) {
    const VkBufferCopy region{
      srcOffset,
      dstOffset,
      size
    };
    
//     ASSERT((src->info().usage & VK_BUFFER_USAGE_TRANSFER_SRC_BIT) == VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
//     ASSERT((dst->info().usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT) == VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    vkCmdCopyBuffer(current, src->handle(), dst->handle(), 1, &region);
    
    ASSERT(family < 4);
  }

//   void TaskInterface::copy(VkBuffer src, VkBuffer dst, const VkDeviceSize &srcOffset, const VkDeviceSize &dstOffset, const VkDeviceSize &size) {
//     const VkBufferCopy region{
//       srcOffset,
//       dstOffset,
//       size
//     };
// 
//     vkCmdCopyBuffer(current, src, dst, 1, &region);
//   }

  void TaskInterface::copy(Buffer* src, Buffer* dst, const std::vector<VkBufferCopy> &regions) {
//     ASSERT((src->info().usage & VK_BUFFER_USAGE_TRANSFER_SRC_BIT) == VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
//     ASSERT((dst->info().usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT) == VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    
    vkCmdCopyBuffer(current, src->handle(), dst->handle(), regions.size(), regions.data());
  }

  void TaskInterface::copy(Image* src, Image* dst,
                           const VkAccessFlags &aspect1,
                           const VkAccessFlags &aspect2,
                           const VkImageLayout &layoutSrc,
                           const VkImageLayout &layoutDst) {
    const VkImageCopy copyInfo{
      {
        aspect1,
        0, 0, 1
      },
      {0, 0, 0},
      {
        aspect2,
        0, 0, 1
      },
      {0, 0, 0},
      src->info().extent
    };

    vkCmdCopyImage(current, src->handle(), layoutSrc, dst->handle(), layoutDst, 1, &copyInfo);
    
    ASSERT(family < 4);
  }

  void TaskInterface::copy(Image* src, Image* dst, const VkImageCopy &copyInfo,
                          const VkImageLayout &layoutSrc,
                          const VkImageLayout &layoutDst) {
    vkCmdCopyImage(current, src->handle(), layoutSrc, dst->handle(), layoutDst, 1, &copyInfo);
    
    ASSERT(family < 4);
  }

  void TaskInterface::copy(Image* src, Image* dst, const std::vector<VkImageCopy> &copyInfos,
            const VkImageLayout &layoutSrc,
            const VkImageLayout &layoutDst) {
    vkCmdCopyImage(current, src->handle(), layoutSrc, dst->handle(), layoutDst, copyInfos.size(), copyInfos.data());
    
    ASSERT(family < 4);
  }

//   void TaskInterface::copy(VkImage src, VkImage dst, const VkImageCopy &copyInfo,
//             const VkImageLayout &layoutSrc,
//             const VkImageLayout &layoutDst) {
//     vkCmdCopyImage(current, src, layoutSrc, dst, layoutDst, 1, &copyInfo);
//   }

  void TaskInterface::copy(Buffer* src, Image* dst, const VkAccessFlags &aspect, const VkImageLayout &layoutDst) {
    const VkBufferImageCopy region{
      0,
      0, 0,
      {
        aspect,
        0, 0, 1
      },
      {0, 0, 0},
      dst->info().extent
    };

    vkCmdCopyBufferToImage(current, src->handle(), dst->handle(), layoutDst, 1, &region);
    
    ASSERT(family < 4);
  }

  void TaskInterface::copy(Buffer* src, Image* dst, const VkBufferImageCopy &region,
            const VkImageLayout &layoutDst) {
    vkCmdCopyBufferToImage(current, src->handle(), dst->handle(), layoutDst, 1, &region);
    
    ASSERT(family < 4);
  }

  void TaskInterface::copy(Buffer* src, Image* dst, const std::vector<VkBufferImageCopy> &regions,
            const VkImageLayout &layoutDst) {
    vkCmdCopyBufferToImage(current, src->handle(), dst->handle(), layoutDst, regions.size(), regions.data());
    
    ASSERT(family < 4);
  }

  void TaskInterface::copy(Image* src, Buffer* dst, const VkAccessFlags &aspect, const VkImageLayout &layoutSrc) {
    VkBufferImageCopy region{
      0,
      0, 0,
      {
        aspect,
        0, 0, 1
      },
      {0, 0, 0},
      src->info().extent
    };

    vkCmdCopyImageToBuffer(current, src->handle(), layoutSrc, dst->handle(), 1, &region);
    
    ASSERT(family < 4);
  }

  void TaskInterface::copy(Image* src, Buffer* dst, const VkBufferImageCopy &region,
                           const VkImageLayout &layoutSrc) {
    vkCmdCopyImageToBuffer(current, src->handle(), layoutSrc, dst->handle(), 1, &region);
    
    ASSERT(family < 4);
  }

  void TaskInterface::copy(Image* src, Buffer* dst, const std::vector<VkBufferImageCopy> &regions,
                           const VkImageLayout &layoutSrc) {
    vkCmdCopyImageToBuffer(current, src->handle(), layoutSrc, dst->handle(), regions.size(), regions.data());
    
    ASSERT(family < 4);
  }

  void TaskInterface::setBarrier(const VkPipelineStageFlags &srcFlags, const VkPipelineStageFlags &dstFlags, const VkImageMemoryBarrier &imageBarrier, const VkDependencyFlags &depFlags) {
    vkCmdPipelineBarrier(current,
                         srcFlags, dstFlags,
                         depFlags,
                         0, nullptr,
                         0, nullptr,
                         1, &imageBarrier);
  }

  void TaskInterface::setBarrier(Image* image, const VkImageLayout &newLayout, const VkAccessFlags &aspect,
                    const uint32_t &oldFamily, const uint32_t &newFamily){
    const VkImageSubresourceRange range{
      aspect,
      0, image->info().mipLevels,
      0, image->info().arrayLayers
    };

    uint32_t oldF = oldFamily == VK_QUEUE_FAMILY_IGNORED ? this->family : oldFamily;
    uint32_t newF = newFamily == VK_QUEUE_FAMILY_IGNORED ? this->family : newFamily;

    VkImageMemoryBarrier barrier{
      VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      nullptr,
      0,
      0,
      image->info().initialLayout,
      newLayout,
      oldF == newF ? VK_QUEUE_FAMILY_IGNORED : oldF,
      newF == oldF ? VK_QUEUE_FAMILY_IGNORED : newF,
      image->handle(),
      range
    };

    VkPipelineStageFlags srcStage;
    VkPipelineStageFlags dstStage;

    getBarrierData(image->info().initialLayout, newLayout, barrier.srcAccessMask, barrier.dstAccessMask, srcStage, dstStage);

    vkCmdPipelineBarrier(current,
                         srcStage, dstStage,
                         0,
                         0, nullptr,
                         0, nullptr,
                         1, &barrier);

    image->info().initialLayout = newLayout;
    
    ASSERT(family < 4);
  }

  void TaskInterface::setBarrier(Image* image, const VkImageLayout &newLayout, const VkImageSubresourceRange &range,
                                const uint32_t &oldFamily, const uint32_t &newFamily) {
    uint32_t oldF = oldFamily == VK_QUEUE_FAMILY_IGNORED ? this->family : oldFamily;
    uint32_t newF = newFamily == VK_QUEUE_FAMILY_IGNORED ? this->family : newFamily;

    VkImageMemoryBarrier barrier{
      VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      nullptr,
      0,
      0,
      image->info().initialLayout,
      newLayout,
      oldF == newF ? VK_QUEUE_FAMILY_IGNORED : oldF,
      newF == oldF ? VK_QUEUE_FAMILY_IGNORED : newF,
      image->handle(),
      range
    };

    VkPipelineStageFlags srcStage;
    VkPipelineStageFlags dstStage;

    getBarrierData(image->info().initialLayout, newLayout, barrier.srcAccessMask, barrier.dstAccessMask, srcStage, dstStage);

    vkCmdPipelineBarrier(current,
                         srcStage, dstStage,
                         0,
                         0, nullptr,
                         0, nullptr,
                         1, &barrier);

    image->info().initialLayout = newLayout;
  }

  void TaskInterface::setBarrier(VkImage image, const VkImageLayout &oldLayout, const VkImageLayout &newLayout, const VkImageSubresourceRange &range,
                    const uint32_t &oldFamily, const uint32_t &newFamily) {
    uint32_t oldF = oldFamily == VK_QUEUE_FAMILY_IGNORED ? this->family : oldFamily;
    uint32_t newF = newFamily == VK_QUEUE_FAMILY_IGNORED ? this->family : newFamily;

    VkImageMemoryBarrier barrier{
      VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      nullptr,
      0,
      0,
      oldLayout,
      newLayout,
      oldF == newF ? VK_QUEUE_FAMILY_IGNORED : oldF,
      newF == oldF ? VK_QUEUE_FAMILY_IGNORED : newF,
      image,
      range
    };

    VkPipelineStageFlags srcStage;
    VkPipelineStageFlags dstStage;

    getBarrierData(oldLayout, newLayout, barrier.srcAccessMask, barrier.dstAccessMask, srcStage, dstStage);

    vkCmdPipelineBarrier(current,
                         srcStage, dstStage, 0,
                         0, nullptr,
                         0, nullptr,
                         1, &barrier);
  }
  
  void TaskInterface::setBarrier(const VkPipelineStageFlags &srcFlags, const VkPipelineStageFlags &dstFlags, const VkDependencyFlags &depFlags) {
    vkCmdPipelineBarrier(current,
                         srcFlags, dstFlags, depFlags,
                         0, nullptr,
                         0, nullptr,
                         0, nullptr);
  }

  void TaskInterface::setBarrier(const VkPipelineStageFlags &srcFlags, const VkPipelineStageFlags &dstFlags,
                    const VkAccessFlags &srcAccess, const VkAccessFlags &dstAccess,
                    const VkDependencyFlags &depFlags) {
    const VkMemoryBarrier barrier{
      VK_STRUCTURE_TYPE_MEMORY_BARRIER,
      nullptr,
      srcAccess,
      dstAccess
    };

    vkCmdPipelineBarrier(current,
                         srcFlags, dstFlags, depFlags,
                         1, &barrier,
                         0, nullptr,
                         0, nullptr);
  }

  void TaskInterface::setBarrier(Buffer* buffer, const VkPipelineStageFlags &srcFlags, const VkPipelineStageFlags &dstFlags,
                  const VkAccessFlags &srcAccess, const VkAccessFlags &dstAccess,
                  const VkDeviceSize &offset, const VkDeviceSize &size,
                  const uint32_t &oldFamily, const uint32_t &newFamily,
                  const VkDependencyFlags &depFlags) {
    uint32_t oldF = oldFamily == VK_QUEUE_FAMILY_IGNORED ? this->family : oldFamily;
    uint32_t newF = newFamily == VK_QUEUE_FAMILY_IGNORED ? this->family : newFamily;

    const VkBufferMemoryBarrier barrier{
      VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
      nullptr,
      srcAccess,
      dstAccess,
      oldF == newF ? VK_QUEUE_FAMILY_IGNORED : oldF,
      newF == oldF ? VK_QUEUE_FAMILY_IGNORED : newF,
      buffer->handle(),
      offset == SIZE_MAX ? 0 : offset,
      size == SIZE_MAX ? buffer->info().size : size
    };

    vkCmdPipelineBarrier(current,
                         srcFlags, dstFlags,
                         depFlags,
                         0, nullptr,
                         1, &barrier,
                         0, nullptr);
  }

  void TaskInterface::setBarrier(VkBuffer buffer, const VkPipelineStageFlags &srcFlags, const VkPipelineStageFlags &dstFlags,
                  const VkAccessFlags &srcAccess, const VkAccessFlags &dstAccess,
                  const VkDeviceSize &offset, const VkDeviceSize &size,
                  const uint32_t &oldFamily, const uint32_t &newFamily,
                  const VkDependencyFlags &depFlags) {
    uint32_t oldF = oldFamily == VK_QUEUE_FAMILY_IGNORED ? this->family : oldFamily;
    uint32_t newF = newFamily == VK_QUEUE_FAMILY_IGNORED ? this->family : newFamily;

    const VkBufferMemoryBarrier barrier{
      VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
      nullptr,
      srcAccess,
      dstAccess,
      oldF == newF ? VK_QUEUE_FAMILY_IGNORED : oldF,
      newF == oldF ? VK_QUEUE_FAMILY_IGNORED : newF,
      buffer,
      offset,
      size
    };

    vkCmdPipelineBarrier(current,
                         srcFlags, dstFlags,
                         depFlags,
                         0, nullptr,
                         1, &barrier,
                         0, nullptr);
  }

  void TaskInterface::setBarrier(const VkPipelineStageFlags &srcFlags, const VkPipelineStageFlags &dstFlags,
                  const VkMemoryBarrier &memoryBarrier,
                  const VkDependencyFlags &depFlags) {
    vkCmdPipelineBarrier(current,
                         srcFlags, dstFlags,
                         depFlags,
                         1, &memoryBarrier,
                         0, nullptr,
                         0, nullptr);
  }

  void TaskInterface::setBarrier(const VkPipelineStageFlags &srcFlags, const VkPipelineStageFlags &dstFlags,
                  const VkBufferMemoryBarrier &buffersMemoryBarrier,
                  const VkDependencyFlags &depFlags) {
    vkCmdPipelineBarrier(current,
                         srcFlags, dstFlags,
                         depFlags,
                         0, nullptr,
                         1, &buffersMemoryBarrier,
                         0, nullptr);
  }

  void TaskInterface::setBarrier(const VkPipelineStageFlags &srcFlags, const VkPipelineStageFlags &dstFlags,
                  const VkMemoryBarrier &memoryBarrier,
                  const VkBufferMemoryBarrier &buffersMemoryBarrier,
                  const VkImageMemoryBarrier &imageMemoryBarrier,
                  const VkDependencyFlags &depFlags) {
    vkCmdPipelineBarrier(current,
                         srcFlags, dstFlags,
                         depFlags,
                         1, &memoryBarrier,
                         1, &buffersMemoryBarrier,
                         1, &imageMemoryBarrier);
  }

  void TaskInterface::setBarrier(const VkPipelineStageFlags &srcFlags, const VkPipelineStageFlags &dstFlags,
                  const std::vector<VkMemoryBarrier> &memoryBarriers,
                  const std::vector<VkBufferMemoryBarrier> &buffersMemoryBarriers,
                  const std::vector<VkImageMemoryBarrier> &imageMemoryBarriers,
                  const VkDependencyFlags &depFlags) {
    vkCmdPipelineBarrier(current,
                         srcFlags, dstFlags,
                         depFlags,
                         memoryBarriers.size(), memoryBarriers.data(),
                         buffersMemoryBarriers.size(), buffersMemoryBarriers.data(),
                         imageMemoryBarriers.size(), imageMemoryBarriers.data());
  }

  void TaskInterface::update(Buffer* buffer, const VkDeviceSize &offset, const VkDeviceSize &size, const void* data) {
    vkCmdUpdateBuffer(current, buffer->handle(), offset, size, data);
  }

  void TaskInterface::update(VkBuffer buffer, const VkDeviceSize &offset, const VkDeviceSize &size, const void* data) {
    vkCmdUpdateBuffer(current, buffer, offset, size, data);
  }

  void TaskInterface::fill(Buffer* buffer, const VkDeviceSize &offset, const VkDeviceSize &size, const uint32_t &data) {
    vkCmdFillBuffer(current, buffer->handle(), offset, size, data);
  }

  void TaskInterface::fill(VkBuffer buffer, const VkDeviceSize &offset, const VkDeviceSize &size, const uint32_t &data) {
    vkCmdFillBuffer(current, buffer, offset, size, data);
  }

  void TaskInterface::execute(TaskInterface* task) {
    vkCmdExecuteCommands(current, 1, &task->current);
  }

  void TaskInterface::execute(const std::vector<TaskInterface*> &tasks) {
#ifdef _WIN32
    std::vector<VkCommandBuffer> commands(tasks.size());
    for (uint32_t i = 0; i < tasks.size(); ++i) commands[i] = tasks[i]->current;
    vkCmdExecuteCommands(current, tasks.size(), commands.data());
#else
    VkCommandBuffer commands[tasks.size()];
    for (uint32_t i = 0; i < tasks.size(); ++i) commands[i] = tasks[i]->current;
    vkCmdExecuteCommands(current, tasks.size(), commands);
#endif
  }
  
  void TaskInterface::execute(const size_t &count, TaskInterface** tasks) {
#ifdef _WIN32
    std::vector<VkCommandBuffer> commands(count);
    for (uint32_t i = 0; i < count; ++i) commands[i] = tasks[i]->current;
    vkCmdExecuteCommands(current, count, commands.data());
#else
    VkCommandBuffer commands[count];
    for (uint32_t i = 0; i < count; ++i) commands[i] = tasks[i]->current;
    vkCmdExecuteCommands(current, count, commands);
#endif
  }
  
  Internal::Queue TaskInterface::start() {
    const VkSubmitInfo info{
      VK_STRUCTURE_TYPE_SUBMIT_INFO,
      nullptr,
      static_cast<uint32_t>(waitSemaphores.size()),
      waitSemaphores.data(),
      stageFlags.data(),
      1,
      &current,
      static_cast<uint32_t>(signalSemaphores.size()),
      signalSemaphores.data()
    };
    
    ASSERT(family < 4);
    
    currentQueue = device->submit(family, 1, &info);
    
    return currentQueue;
  }
  
  VkSubmitInfo TaskInterface::getSubmitInfo() const {
    ASSERT(family < 4);
//     ASSERT(waitSemaphores.size() == 1);
//     ASSERT(signalSemaphores.size() == 1);
    
//     std::cout << "command buffer " << current << "\n";
    
    return {
      VK_STRUCTURE_TYPE_SUBMIT_INFO,
      nullptr,
      static_cast<uint32_t>(waitSemaphores.size()),
      waitSemaphores.data(),
      stageFlags.data(),
      1,
      &current,
      static_cast<uint32_t>(signalSemaphores.size()),
      signalSemaphores.data()
    };
  }

  VkResult TaskInterface::wait(const uint64_t &time) {
    if (currentQueue.fence == VK_NULL_HANDLE) return VK_SUCCESS;
    
    return vkWaitForFences(device->handle(), 1, &currentQueue.fence, VK_FALSE, time);
  }

//   void TaskInterface::addWaitSemaphore(SemaphoreProxy* proxy) {
//     waitSemaphores.push_back(proxy);
//   }
// 
//   uint32_t TaskInterface::waitSemaphoresCount() const {
//     return waitSemaphores.size();
//   }
// 
//   void TaskInterface::getWaitSemaphores(Semaphore* semaphores, VkPipelineStageFlags* flags) const {
//     if (waitSemaphores.empty()) return;
//     
//     for (uint32_t i = 0; i < waitSemaphores.size(); ++i) {
//       semaphores[i] = waitSemaphores[i]->get();
//       flags[i] = waitSemaphores[i]->getFlags();
//     }
//   }
// 
//   uint32_t TaskInterface::signalSemaphoresCount() const {
//     return signalSemaphores.size();
//   }

  size_t TaskInterface::waitSemaphoresSize() const {
    return waitSemaphores.size();
  }
  
  size_t TaskInterface::signalSemaphoresSize() const {
    return signalSemaphores.size();
  }
  
  void TaskInterface::pushWaitSemaphore(const Semaphore semaphore, const VkPipelineStageFlags &flag) {
    waitSemaphores.push_back(semaphore);
    stageFlags.push_back(flag);
    
    ASSERT(family < 4);
  }
  
  void TaskInterface::pushSignalSemaphore(const Semaphore semaphore) {
    signalSemaphores.push_back(semaphore);
  }
  
  void TaskInterface::setWaitSemaphore(const size_t &index, const Semaphore semaphore, const VkPipelineStageFlags &flag) {
    ASSERT(waitSemaphores.size() > index);
    
    waitSemaphores[index] = semaphore;
    stageFlags[index] = flag;
  }
  
  void TaskInterface::setSignalSemaphore(const size_t &index, const Semaphore semaphore) {
    ASSERT(signalSemaphores.size() > index);
    signalSemaphores[index] = semaphore;
  }
  
  void TaskInterface::eraseWaitSemaphore(const size_t &index) {
//     std::swap(waitSemaphores[index], waitSemaphores.back());
//     waitSemaphores.pop_back();
//     
//     std::swap(stageFlags[index], stageFlags.back());
//     stageFlags.pop_back();
    
    waitSemaphores.erase(waitSemaphores.begin() + index);
    stageFlags.erase(stageFlags.begin() + index);
  }
  
  void TaskInterface::eraseSignalSemaphore(const size_t &index) {
//     std::swap(signalSemaphores[index], signalSemaphores.back());
//     signalSemaphores.pop_back();
    
    signalSemaphores.erase(signalSemaphores.begin() + index);
  }

  // void TaskInterface::addWaitSemaphore(VkSemaphore s, const VkPipelineStageFlags &flag) {
  //   waiting.push_back(s);
  //   waitFlags.push_back(flag);
  // }
  //
  // void TaskInterface::addSignalSemaphore(VkSemaphore s) {
  //   signal.push_back(s);
  // }
  //
  // void TaskInterface::clearWaitSemaphores() {
  //   waiting.clear();
  //   waitFlags.clear();
  // }
  //
  // void TaskInterface::clearSignalSemaphores() {
  //   signal.clear();
  // }
  //
  // void TaskInterface::setWaitSemaphores(const std::vector<VkSemaphore> &semaphores, const std::vector<VkPipelineStageFlags> &waitFlags) {
  //   waiting = semaphores;
  //   this->waitFlags = waitFlags;
  //   this->waitFlags.resize(waiting.size(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
  // }
  //
  // void TaskInterface::setSignalSemaphores(const std::vector<VkSemaphore> &semaphores) {
  //   signal = semaphores;
  // }

  uint32_t TaskInterface::getFamily() const {
//     ASSERT(family < 4);
    return family;
  }

  VkQueueFlags TaskInterface::getQueueFlags() const {
    return flags;
  }

  VkCommandBuffer TaskInterface::getCommandBuffer() const {
    return current;
  }

  TransferTask::TransferTask(Device* device, const bool &primary) {
    this->device = device;
    this->primary = primary;

    family = device->getQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);

    VkCommandPool pool = device->commandPool(1);

    const VkCommandBufferAllocateInfo info{
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      nullptr,
      pool,
      primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY,
      1
    };

    vkCheckError("vkAllocateCommandBuffers", nullptr,
    vkAllocateCommandBuffers(device->handle(), &info, &current));

    flags = VK_QUEUE_TRANSFER_BIT;
  }

  TransferTask::~TransferTask() {
    if (current != VK_NULL_HANDLE) {
      VkCommandPool pool = device->commandPool(1);
      vkFreeCommandBuffers(device->handle(), pool, 1, &current);
      current = VK_NULL_HANDLE;
    }
    
//     VkCommandPool pool = device->commandPool(1);
// 
//     vkFreeCommandBuffers(device->handle(), pool, 1, &current);

    // for (uint32_t i = 0; i < signalSemaphores.size(); ++i) {
    //   device->destroySemaphoreProxy(signalSemaphores[i]);
    // }

//     for (uint32_t i = 0; i < semaphores.size(); ++i) {
//       device->destroy(semaphores[i]);
//     }
  }

  void TransferTask::begin(const VkCommandBufferUsageFlags &flags) {
    vkCheckError("vkResetCommandBuffer", nullptr,
    vkResetCommandBuffer(current, 0));

    // здесь нужно любыми спсосбами получить информацию о всем происходящем
    // то есть текущий сабпасс, рендерпасс и фреймбуфер, как?
    // + oclusion query
    const VkCommandBufferInheritanceInfo ii{
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
      nullptr,
      VK_NULL_HANDLE,
      0,
      VK_NULL_HANDLE,
      VK_FALSE,
      0,
      0
    };

    const VkCommandBufferBeginInfo info{
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      nullptr,
      flags,
      primary ? nullptr : &ii
    };

    vkCheckError("vkBeginCommandBuffer", nullptr,
    vkBeginCommandBuffer(current, &info));
  }

  void TransferTask::end() {
    vkCheckError("vkEndCommandBuffer", nullptr,
    vkEndCommandBuffer(current));
  }

//   Internal::Queue TransferTask::start() {
//     VkSemaphore wait[waitSemaphoresCount() == 0 ? 1 : waitSemaphoresCount()];
//     VkPipelineStageFlags stageFlags[waitSemaphoresCount() == 0 ? 1 : waitSemaphoresCount()];
//     getWaitSemaphores(wait, stageFlags);
//     VkSemaphore signal[signalSemaphoresCount() == 0 ? 1 : signalSemaphoresCount()];
//     getSignalSemaphores(signal);
// 
//     VkSubmitInfo info{
//       VK_STRUCTURE_TYPE_SUBMIT_INFO,
//       nullptr,
//       static_cast<uint32_t>(waitSemaphores.size()),
//       waitSemaphores.empty() ? nullptr : wait,
//       waitSemaphores.empty() ? nullptr : stageFlags,
//       1,
//       &current,
//       static_cast<uint32_t>(signalSemaphores.size()),
//       signalSemaphores.empty() ? nullptr : signal
//     };
// 
//     currentQueue = device->submit(family, 1, &info);
// 
//     return currentQueue;
//   }

  // VkSubmitInfo TransferTask::getSubmitInfo() {
  //   return {
  //     VK_STRUCTURE_TYPE_SUBMIT_INFO,
  //     nullptr,
  //     static_cast<uint32_t>(waiting.size()),
  //     waiting.empty() ? nullptr : waiting.data(),
  //     waitFlags.empty() ? nullptr : waitFlags.data(),
  //     1,
  //     &current,
  //     static_cast<uint32_t>(signal.size()),
  //     signal.empty() ? nullptr : signal.data()
  //   };
  // }

//   SemaphoreProxy* TransferTask::createSignalSemaphore(const VkPipelineStageFlags &flag) {
//     Semaphore s = device->createSemaphore();
//     SemaphoreOwner* ow = device->createSemaphoreProxy();
//     ow->changeSemaphore(s, flag);
//     semaphores.push_back(s);
//     signalSemaphores.push_back(ow);
// 
//     return ow;
//   }
// 
//   void TransferTask::getSignalSemaphores(Semaphore* semaphores) const {
//     // for (uint32_t i = 0; i < signalSemaphores.size(); ++i) {
//     //   semaphores[i] = signalSemaphores[i]->get();
//     // }
//     
//     if (this->semaphores.empty()) return;
// 
//     memcpy(semaphores, this->semaphores.data(), this->semaphores.size() * sizeof(Semaphore));
//   }

  // TransferTask::TransferTask() {}

  TaskCommands::~TaskCommands() {
//     for (uint32_t i = 0; i < semaphores.size(); ++i) {
//       for (uint32_t j = 0; j < semaphores[i].size(); ++j) {
//         device->destroy(semaphores[i][j]);
//       }
//     }
  }

  void TaskCommands::clearImage(Image* image, const VkClearColorValue &value, const VkImageSubresourceRange &range) {
    vkCmdClearColorImage(current, image->handle(), image->info().initialLayout, &value, 1, &range);
  }

  void TaskCommands::clearImage(Image* image, const VkClearColorValue &value, const std::vector<VkImageSubresourceRange> &ranges) {
    vkCmdClearColorImage(current, image->handle(), image->info().initialLayout, &value, ranges.size(), ranges.data());
  }

  void TaskCommands::beginQuery(VkQueryPool pool, const uint32_t &query, const VkQueryControlFlags &flags) {
    vkCmdBeginQuery(current, pool, query, flags);
  }

  void TaskCommands::endQuery(VkQueryPool pool, const uint32_t &query) {
    vkCmdEndQuery(current, pool, query);
  }

  void TaskCommands::resetQueryPool(VkQueryPool pool, const uint32_t &firstQuery, const uint32_t &queryCount) {
    vkCmdResetQueryPool(current, pool, firstQuery, queryCount);
  }

  void TaskCommands::copyQuery(VkQueryPool pool, const uint32_t &firstQuery, const uint32_t &queryCount,
                  Buffer* buffer, const VkDeviceSize &offset, const VkDeviceSize &stride, const VkQueryResultFlags &flags) {
    vkCmdCopyQueryPoolResults(current, pool, firstQuery, queryCount, buffer->handle(), offset, stride, flags);
  }

  void TaskCommands::writeTimeStamp(const VkPipelineStageFlagBits &flags, VkQueryPool pool, const uint32_t &query) {
    vkCmdWriteTimestamp(current, flags, pool, query);
  }

//   SemaphoreProxy* TaskCommands::createSignalSemaphore(const VkPipelineStageFlags &flag) {
//     SemaphoreOwner* sw = device->createSemaphoreProxy();
//     signalSemaphores.push_back(sw);
// 
//     for (uint32_t i = 0 ; i < semaphores.size(); ++i) {
//       VkSemaphore s = device->createSemaphore();
//       semaphores[i].push_back(s);
//     }
// 
//     semaphoreFlags.push_back(flag);
//     sw->changeSemaphore(semaphores[currentFrameIndex].back(), semaphoreFlags.back());
// 
//     return sw;
//   }
// 
//   void TaskCommands::getSignalSemaphores(Semaphore* semaphores) const {
//     // for (uint32_t i = 0; i < signalSemaphores.size(); ++i) {
//     //   semaphores[i] = this->semaphores[currentFrameIndex][i];
//     // }
//     
//     if (this->semaphores[currentFrameIndex].empty()) return;
// 
//     memcpy(semaphores, this->semaphores[currentFrameIndex].data(), this->semaphores[currentFrameIndex].size() * sizeof(Semaphore));
//   }

  //const uint32_t &jobDataCount
  GraphicTask::GraphicTask(Device* device, const bool &primary) {
    this->device = device;
    this->primary = primary;

    family = device->getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);

    VkCommandPool pool = device->commandPool(0);

    const VkCommandBufferAllocateInfo info{
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      nullptr,
      pool,
      primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY,
      1
    };

    vkCheckError("vkAllocateCommandBuffers", nullptr,
    vkAllocateCommandBuffers(device->handle(), &info, &current));

    flags = VK_QUEUE_GRAPHICS_BIT;
  }

  GraphicTask::~GraphicTask() {
    if (current != VK_NULL_HANDLE) {
      VkCommandPool pool = device->commandPool(0);
      vkFreeCommandBuffers(device->handle(), pool, 1, &current);
      current = VK_NULL_HANDLE;
    }

//     commandBuffers.clear();

    // for (uint32_t i = 0; i < signalSemaphores.size(); ++i) {
    //   device->destroySemaphoreProxy(signalSemaphores[i]);
    // }
    //
    // for (uint32_t i = 0; i < semaphores.size(); ++i) {
    //   for (uint32_t j = 0; j < semaphores[i].size(); ++j) {
    //     device->destroy(semaphores[i][j]);
    //   }
    // }
  }

  //, const bool addSemaphore
  void GraphicTask::setRenderTarget(RenderTarget* target) {
    this->target = target;

//     VkSemaphore s1 = target->imageAvailable();
//     if (s1 != VK_NULL_HANDLE) {
//       waiting.push_back(s1);
//       waitFlags.push_back(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
//     }
//
//     VkSemaphore s2 = target->finishRendering();
//     if (s2 != VK_NULL_HANDLE) {
//       signal.push_back(s2);
//     }

//     if (!addSemaphore) return;

    //const VkPipelineStageFlags &flag = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT
//     SemaphoreProxy* p = createSignalSemaphore();
//     target->addSemaphoreProxy(p);
// 
//     addWaitSemaphore(target->getSemaphoreProxy());
  }

  void GraphicTask::begin(const VkCommandBufferUsageFlags &flags) {
//     currentFrameIndex = (currentFrameIndex + 1) % commandBuffers.size();
//     current = commandBuffers[currentFrameIndex];

    vkCheckError("vkResetCommandBuffer", nullptr,
    vkResetCommandBuffer(current, 0));

    // здесь нужно любыми спсосбами получить информацию о всем происходящем
    // то есть текущий сабпасс, рендерпасс и фреймбуфер, как?
    // + oclusion query

    // необходимо что то придумать с сабпассом
    // скорее всего отдельно его пихать сюда
    // и так же отдельно пихать сюда данные о query
    const VkCommandBufferInheritanceInfo ii{
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
      nullptr,
      target != nullptr ? target->renderPass() : VK_NULL_HANDLE,
      0,
      target != nullptr ? target->framebuffer() : VK_NULL_HANDLE,
      VK_FALSE,
      0,
      0
    };

    const VkCommandBufferBeginInfo info{
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      nullptr,
      flags,
      primary ? nullptr : &ii
    };

    vkCheckError("vkBeginCommandBuffer", nullptr,
    vkBeginCommandBuffer(current, &info));

//     for (uint32_t i = 0; i < semaphores[currentFrameIndex].size(); ++i) {
//       signalSemaphores[i]->changeSemaphore(semaphores[currentFrameIndex][i], semaphoreFlags[i]);
//     }
    currentSubpass = 0;
  }

  void GraphicTask::end() {
    vkEndCommandBuffer(current);

    // currentSets.clear();
    // currentSets.resize(YAVF_MIN_DESCRIPTOR_COUNT, VK_NULL_HANDLE);
    currentPipeline = Pipeline();
  }

  void GraphicTask::beginRenderPass(const VkSubpassContents &contents) {
    const std::vector<VkClearValue> &values = target->clearValues();

    const VkRenderPassBeginInfo info{
      VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      nullptr,
      target->renderPass(),
      target->framebuffer(),
      target->size(),
      static_cast<uint32_t>(values.size()),
      values.data()
    };

    vkCmdBeginRenderPass(current, &info, contents);

    const VkViewport &view = target->viewport();
    if (view.width != 0 && view.height != 0) vkCmdSetViewport(current, 0, 1, &view);
    const VkRect2D &scissor = target->scissor();
    if (scissor.extent.width != 0 && scissor.extent.height != 0) vkCmdSetScissor(current, 0, 1, &scissor);

    renderpassStart = true;
  }
  
  void GraphicTask::beginRenderPass(const VkRenderPass &pass, const VkSubpassContents &contents) {
    const std::vector<VkClearValue> &values = target->clearValues();

    const VkRenderPassBeginInfo info{
      VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      nullptr,
      pass,
      target->framebuffer(),
      target->size(),
      static_cast<uint32_t>(values.size()),
      values.data()
    };

    vkCmdBeginRenderPass(current, &info, contents);

    const VkViewport &view = target->viewport();
    if (view.width != 0 && view.height != 0) vkCmdSetViewport(current, 0, 1, &view);
    const VkRect2D &scissor = target->scissor();
    if (scissor.extent.width != 0 && scissor.extent.height != 0) vkCmdSetScissor(current, 0, 1, &scissor);

    renderpassStart = true;
  }

  void GraphicTask::endRenderPass() {
    vkCmdEndRenderPass(current);

    renderpassStart = false;
  }

  void GraphicTask::setViews(const std::vector<VkViewport> &views, const std::vector<VkRect2D> &scissors, const uint32_t &firstView, const uint32_t &firstScissor) {
    vkCmdSetViewport(current, firstView, views.size(), views.data());
    vkCmdSetScissor(current, firstScissor, scissors.size(), scissors.data());
  }

  void GraphicTask::setView(const VkViewport &view, const uint32_t &firstView) {
    vkCmdSetViewport(current, firstView, 1, &view);
  }

  void GraphicTask::setView(const std::vector<VkViewport> &views, const uint32_t &firstView) {
    vkCmdSetViewport(current, firstView, views.size(), views.data());
  }

  void GraphicTask::setScissor(const VkRect2D &scissor, const uint32_t &firstView) {
    vkCmdSetScissor(current, firstView, 1, &scissor);
  }

  void GraphicTask::setScissor(const std::vector<VkRect2D> &scissors, const uint32_t &firstScissor) {
    vkCmdSetScissor(current, firstScissor, scissors.size(), scissors.data());
  }

  void GraphicTask::setPipeline(const Pipeline &pipeline) {
    if (pipeline == currentPipeline) return;

    currentPipeline = pipeline;
    vkCmdBindPipeline(current, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle());
    
    ASSERT(family < 4);
  }

//   void GraphicTask::setDescriptor(Image* image, const uint32_t &firstSet) {
//     // uint32_t set = firstSet == UINT32_MAX ? image->descriptor().setNum : firstSet;
//     //
//     // if (set >= currentSets.size()) currentSets.resize(set+1);
//     // else if (currentSets[set] == image->descriptor().handle) return;
//     //
//     // currentSets[set] = image->descriptor().handle;
// 
//     const VkDescriptorSet s = image->descriptor();
// 
//     vkCmdBindDescriptorSets(current, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline.layout(),
//                             firstSet, 1, &s,
//                             0, nullptr);
//   }
// 
//   void GraphicTask::setDescriptor(Buffer* buffer, const uint32_t &firstSet) {
//     // uint32_t set = firstSet == UINT32_MAX ? buffer->descriptor().setNum : firstSet;
//     //
//     // if (set >= currentSets.size()) currentSets.resize(set+1);
//     // else if (currentSets[set] == buffer->descriptor().handle) return;
//     //
//     // currentSets[set] = buffer->descriptor().handle;
// 
//     const VkDescriptorSet s = buffer->descriptor();
// 
//     vkCmdBindDescriptorSets(current, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline.layout(),
//                             firstSet, 1, &s,
//                             0, nullptr);
//   }
// 
//   void GraphicTask::setDescriptor(const Sampler &sampler, const uint32_t &firstSet) {
//     // uint32_t set = firstSet == UINT32_MAX ? sampler.descriptor().setNum : firstSet;
//     //
//     // if (set >= currentSets.size()) currentSets.resize(set+1);
//     // else if (currentSets[set] == sampler.descriptor().handle) return;
//     //
//     // currentSets[set] = sampler.descriptor().handle;
// 
//     const VkDescriptorSet s = sampler.descriptor();
// 
//     vkCmdBindDescriptorSets(current, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline.layout(),
//                             firstSet, 1, &s,
//                             0, nullptr);
//   }
// 
//   void GraphicTask::setDescriptor(const Descriptor &descriptor, const uint32_t &firstSet) {
//     // uint32_t set = firstSet == UINT32_MAX ? descriptor.setNum : firstSet;
//     //
//     // if (set >= currentSets.size()) currentSets.resize(set+1);
//     // else if (currentSets[set] == descriptor.handle) return;
//     //
//     // currentSets[set] = descriptor.handle;
// 
//     vkCmdBindDescriptorSets(current, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline.layout(),
//                             firstSet, 1, &descriptor,
//                             0, nullptr);
//   }
// 
//   void GraphicTask::setDescriptor(const std::vector<Descriptor> &descriptors, const uint32_t &firstSet) {
//     vkCmdBindDescriptorSets(current, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline.layout(),
//                             firstSet, descriptors.size(), descriptors.data(),
//                             0, nullptr);
//   }

  void GraphicTask::setDescriptor(DescriptorSet* descriptor, const uint32_t &firstSet, const uint32_t &offset) {
    const VkDescriptorSet set = descriptor->handle();
    vkCmdBindDescriptorSets(current, VK_PIPELINE_BIND_POINT_COMPUTE, currentPipeline.layout(),
                            firstSet, 1, &set,
                            (offset == UINT32_MAX ? 0 : 1), (offset == UINT32_MAX ? nullptr : &offset));
  }

  void GraphicTask::setDescriptor(const VkDescriptorSet &descriptor, const uint32_t &firstSet, const uint32_t &offset) {
    vkCmdBindDescriptorSets(current, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline.layout(),
                            firstSet, 1, &descriptor,
                            (offset == UINT32_MAX ? 0 : 1), (offset == UINT32_MAX ? nullptr : &offset));
  }
  
  void GraphicTask::setDescriptor(const std::initializer_list<VkDescriptorSet> &descriptors, const uint32_t &firstSet, const std::initializer_list<uint32_t> &offsets) {
    vkCmdBindDescriptorSets(current, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline.layout(),
                            firstSet, descriptors.size(), descriptors.begin(),
                            offsets.size(), offsets.begin());
  }

  void GraphicTask::setConsts(const uint32_t &offset, const uint32_t &size, const void* value, const VkShaderStageFlags &flags) {
    vkCmdPushConstants(current, currentPipeline.layout(), flags, offset, size, value);
  }

  void GraphicTask::setIndexBuffer(Buffer* buffer, const VkIndexType &type, const VkDeviceSize &offset) {
//     indexCount = buffer->param().dataCount;
//     VkDeviceSize finalOffset = buffer->param().offset + offset;

    vkCmdBindIndexBuffer(current, buffer->handle(), offset, type);
    
    ASSERT(family < 4);
  }

  void GraphicTask::setVertexBuffer(Buffer* buffer, const uint32_t &firstBinding, const VkDeviceSize &offset) {
//     VkDeviceSize finalOffset = buffer->param().offset + offset;
//     vertexCount = buffer->param().dataCount;
    VkBuffer b = buffer->handle();

    vkCmdBindVertexBuffers(current, firstBinding, 1, &b, &offset);
    
    ASSERT(family < 4);
  }
  
  void GraphicTask::setVertexBuffer(const std::vector<VkBuffer> &buffers, const uint32_t &firstBinding, const std::vector<VkDeviceSize> &offsets) {
    vkCmdBindVertexBuffers(current, firstBinding, buffers.size(), buffers.data(), offsets.data());
    
    ASSERT(family < 4);
  }

//   void GraphicTask::setInstanceBuffer(Buffer* buffer, const uint32_t &firstBinding, const VkDeviceSize &offset) {
//     VkDeviceSize finalOffset = buffer->param().offset + offset;
//     instanceCount = buffer->param().dataCount;
//     VkBuffer b = buffer->get();
// 
//     vkCmdBindVertexBuffers(current, firstBinding, 1, &b, &finalOffset);
//   }
  
//   void GraphicTask::setIndexBuffer(VkBuffer buffer, const VkIndexType &type, const VkDeviceSize &offset) {
//     vkCmdBindIndexBuffer(current, buffer, offset, type);
//   }
//   
//   void GraphicTask::setVertexBuffer(VkBuffer buffer, const uint32_t &firstBinding, const VkDeviceSize &offset) {
//     vkCmdBindVertexBuffers(current, firstBinding, 1, &buffer, &offset);
//   }
//   
//   void GraphicTask::setInstanceBuffer(VkBuffer buffer, const uint32_t &firstBinding, const VkDeviceSize &offset) {
//     vkCmdBindVertexBuffers(current, firstBinding, 1, &buffer, &offset);
//   }

  void GraphicTask::nextSubpass() {
    vkCmdNextSubpass(current, VK_SUBPASS_CONTENTS_INLINE);

    ++currentSubpass;
  }

//   void GraphicTask::draw(const uint32_t &firstVertex, const uint32_t &firstInstance) {
//     vkCmdDraw(current, vertexCount, instanceCount, firstVertex, firstInstance);
// 
//     instanceCount = 1;
//     vertexCount = 1; 
//   }

  void GraphicTask::draw(const uint32_t &vertexCount, const uint32_t &instanceCount, const uint32_t &firstVertex, const uint32_t &firstInstance) {
    vkCmdDraw(current, vertexCount, instanceCount, firstVertex, firstInstance);
    
    ASSERT(family < 4);
  }

//   void GraphicTask::drawIndexed(const uint32_t &firstIndex, const int32_t &vertexOffset, const uint32_t &firstInstance) {
//     vkCmdDrawIndexed(current, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
// 
//     indexCount = 1;
//     instanceCount = 1;
//   }

  void GraphicTask::drawIndexed(const uint32_t &indexCount, const uint32_t &instanceCount, const uint32_t &firstIndex, const int32_t &vertexOffset, const uint32_t &firstInstance) {
    vkCmdDrawIndexed(current, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
  }

  void GraphicTask::drawIndirect(Buffer* buffer, const uint32_t &drawCount, const VkDeviceSize &offset, const uint32_t &stride) {
    vkCmdDrawIndirect(current, buffer->handle(), offset, drawCount, stride);
  }

  void GraphicTask::drawIndexedIndirect(Buffer* buffer, const uint32_t &drawCount, const VkDeviceSize &offset, const uint32_t &stride) {
    vkCmdDrawIndexedIndirect(current, buffer->handle(), offset, drawCount, stride);
  }

  void GraphicTask::clearAttachments(const std::vector<VkClearAttachment> &attachments, const std::vector<VkClearRect> &rects) {
    vkCmdClearAttachments(current, attachments.size(), attachments.data(), rects.size(), rects.data());
  }

  void GraphicTask::copyBlit(Image* src, Image* dst, const VkImageBlit &blitInfo, const VkFilter &filter,
                const VkImageLayout &layoutSrc,
                const VkImageLayout &layoutDst) {
    vkCmdBlitImage(current, src->handle(), layoutSrc, dst->handle(), layoutDst, 1, &blitInfo, filter);
  }

  void GraphicTask::copyBlit(Image* src, Image* dst, const std::vector<VkImageBlit> &blitInfos, const VkFilter &filter,
                const VkImageLayout &layoutSrc,
                const VkImageLayout &layoutDst) {
    vkCmdBlitImage(current, src->handle(), layoutSrc, dst->handle(), layoutDst, blitInfos.size(), blitInfos.data(), filter);
  }

//   void GraphicTask::copyBlit(VkImage src, VkImage dst, const VkImageBlit &blitInfo, const VkFilter &filter,
//                 const VkImageLayout &layoutSrc,
//                 const VkImageLayout &layoutDst) {
//     vkCmdBlitImage(current, src, layoutSrc, dst, layoutDst, 1, &blitInfo, filter);
//   }

  void GraphicTask::resolve(Image* src, Image* dst,
                            const VkAccessFlags &aspect1, const VkAccessFlags &aspect2,
                            const VkImageLayout &layoutSrc,
                            const VkImageLayout &layoutDst) {
    const VkImageResolve resolve{
      {
         aspect1,
         0, 0, 1
      },
      {0, 0, 0},
      {
         aspect2,
         0, 0, 1
      },
      {0, 0, 0},
      src->info().extent
    };

    vkCmdResolveImage(current, src->handle(), layoutSrc, dst->handle(), layoutDst, 1, &resolve);
  }

  void GraphicTask::resolve(Image* src, Image* dst, const VkImageResolve &resolve,
              const VkImageLayout &layoutSrc,
              const VkImageLayout &layoutDst) {
    vkCmdResolveImage(current, src->handle(), layoutSrc, dst->handle(), layoutDst, 1, &resolve);
  }

  void GraphicTask::resolve(Image* src, Image* dst, const std::vector<VkImageResolve> &resolves,
              const VkImageLayout &layoutSrc, const VkImageLayout &layoutDst) {
    vkCmdResolveImage(current, src->handle(), layoutSrc, dst->handle(), layoutDst, resolves.size(), resolves.data());
  }

//   void GraphicTask::resolve(VkImage src, VkImage dst, const VkImageResolve &resolve,
//               const VkImageLayout &layoutSrc, const VkImageLayout &layoutDst) {
//     vkCmdResolveImage(current, src, layoutSrc, dst, layoutDst, 1, &resolve);
//   }

  void GraphicTask::clearDepthStencil(Image* image, const VkClearDepthStencilValue &value, const VkImageSubresourceRange &range) {
    vkCmdClearDepthStencilImage(current, image->handle(), image->info().initialLayout, &value, 1, &range);
  }

  void GraphicTask::clearDepthStencil(Image* image, const VkClearDepthStencilValue &value, const std::vector<VkImageSubresourceRange> &ranges) {
    vkCmdClearDepthStencilImage(current, image->handle(), image->info().initialLayout, &value, ranges.size(), ranges.data());
  }

  void GraphicTask::setDepthBounds(const float &min, const float &max) {
    vkCmdSetDepthBounds(current, min, max);
  }

  void GraphicTask::setLineWidth(const float &width) {
    vkCmdSetLineWidth(current, width);
  }

  void GraphicTask::setDepthBias(const float &constantFactor, const float &clamp, const float &slopeFactor) {
    vkCmdSetDepthBias(current, constantFactor, clamp, slopeFactor);
  }

  void GraphicTask::setStencilCompareMask(const VkStencilFaceFlags &faceMask, const uint32_t &compareMask) {
    vkCmdSetStencilCompareMask(current, faceMask, compareMask);
  }

  void GraphicTask::setStencilWriteMask(const VkStencilFaceFlags &faceMask, const uint32_t &writeMask) {
    vkCmdSetStencilWriteMask(current, faceMask, writeMask);
  }

  void GraphicTask::setStencilReference(const VkStencilFaceFlags &faceMask, const uint32_t &reference) {
    vkCmdSetStencilReference(current, faceMask, reference);
  }

  void GraphicTask::setBlendConstants(const float consts[4]) {
    vkCmdSetBlendConstants(current, consts);
  }

//   Internal::Queue GraphicTask::start() {
// //     VkSemaphore w = target->imageAvailable();
// //     VkSemaphore f = target->finishRendering();
// //
// //     if (w != VK_NULL_HANDLE) {
// //       waiting.push_back(w);
// //       waitFlags.push_back(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
// //     }
// //
// //     if (f != VK_NULL_HANDLE) signal.push_back(f);
// 
//     VkSemaphore wait[waitSemaphoresCount() == 0 ? 1 : waitSemaphoresCount()];
//     VkPipelineStageFlags flags[waitSemaphoresCount() == 0 ? 1 : waitSemaphoresCount()];
//     getWaitSemaphores(wait, flags);
//     VkSemaphore signal[signalSemaphoresCount() == 0 ? 1 : signalSemaphoresCount()];
//     getSignalSemaphores(signal);
// 
//     VkSubmitInfo info{
//       VK_STRUCTURE_TYPE_SUBMIT_INFO,
//       nullptr,
//       static_cast<uint32_t>(waitSemaphores.size()),
//       wait,
//       flags,
//       1,
//       &current,
//       static_cast<uint32_t>(signalSemaphores.size()),
//       signal
//     };
// 
//     currentQueue = device->submit(family, 1, &info);
// 
// //     if (w != VK_NULL_HANDLE) {
// //       waiting.pop_back();
// //       waitFlags.pop_back();
// //     }
// //
// //     if (f != VK_NULL_HANDLE) signal.pop_back();
// 
//     return currentQueue;
//   }

  // VkSubmitInfo GraphicTask::getSubmitInfo() {
  //   return {
  //     VK_STRUCTURE_TYPE_SUBMIT_INFO,
  //     nullptr,
  //     static_cast<uint32_t>(waiting.size()),
  //     waiting.empty() ? nullptr : waiting.data(),
  //     waitFlags.empty() ? nullptr : waitFlags.data(),
  //     1,
  //     &current,
  //     static_cast<uint32_t>(signal.size()),
  //     signal.empty() ? nullptr : signal.data()
  //   };
  // }

//   void GraphicTask::clearWaitSemaphores() {
//     waiting.clear();
//     waitFlags.clear();
//
// //     VkSemaphore s = target->imageAvailable();
// //     if (s != VK_NULL_HANDLE) {
// //       waiting.push_back(s);
// //       waitFlags.push_back(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
// //     }
//   }
//
//   void GraphicTask::clearSignalSemaphores() {
//     signal.clear();
//
// //     VkSemaphore s = target->finishRendering();
// //     if (s != VK_NULL_HANDLE) {
// //       signal.push_back(s);
// //     }
//   }
//
//   void GraphicTask::setWaitSemaphores(const std::vector<VkSemaphore> &semaphores, const std::vector<VkPipelineStageFlags> &waitFlags) {
//     this->waiting = semaphores;
//     this->waitFlags = waitFlags;
//     this->waitFlags.resize(waiting.size(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
//
//     VkSemaphore s = target->imageAvailable();
//     if (s != VK_NULL_HANDLE) {
//       this->waiting.push_back(s);
//       this->waitFlags.push_back(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
//     }
//   }
//
//   void GraphicTask::setSignalSemaphores(const std::vector<VkSemaphore> &semaphores) {
//     this->signal = semaphores;
//
//     VkSemaphore s = target->finishRendering();
//     if (s != VK_NULL_HANDLE) {
//       signal.push_back(s);
//     }
//   }

  GraphicTask::GraphicTask() {}

  //, const uint32_t &framesCount
  ComputeTask::ComputeTask(Device* device, const bool &primary) {
    this->device = device;
    this->primary = primary;

    family = device->getQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);

    VkCommandPool pool = device->commandPool(2);

    const VkCommandBufferAllocateInfo info{
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      nullptr,
      pool,
      primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY,
      1
    };

    vkCheckError("vkAllocateCommandBuffers", nullptr,
    vkAllocateCommandBuffers(device->handle(), &info, &current));

    flags = VK_QUEUE_COMPUTE_BIT;
  }

  ComputeTask::~ComputeTask() {
    if (current != VK_NULL_HANDLE) {
      VkCommandPool pool = device->commandPool(2);
      vkFreeCommandBuffers(device->handle(), pool, 1, &current);
      current = VK_NULL_HANDLE;
    }
  }

  void ComputeTask::begin(const VkCommandBufferUsageFlags &flags) {
//     currentFrameIndex = (currentFrameIndex + 1) % commandBuffers.size();
//     current = commandBuffers[currentFrameIndex];

    vkCheckError("vkResetCommandBuffer", nullptr,
    vkResetCommandBuffer(current, 0));

    // здесь нужно любыми спсосбами получить информацию о всем происходящем
    // то есть текущий сабпасс, рендерпасс и фреймбуфер, как?
    // + oclusion query
    const VkCommandBufferInheritanceInfo ii{
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
      nullptr,
      VK_NULL_HANDLE,
      0,
      VK_NULL_HANDLE,
      VK_FALSE,
      0,
      0
    };

    const VkCommandBufferBeginInfo info{
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      nullptr,
      flags,
      primary ? nullptr : &ii
    };

    vkCheckError("vkBeginCommandBuffer", nullptr,
    vkBeginCommandBuffer(current, &info));

//     for (uint32_t i = 0; i < semaphores[currentFrameIndex].size(); ++i) {
//       signalSemaphores[i]->changeSemaphore(semaphores[currentFrameIndex][i], semaphoreFlags[i]);
//     }
  }

  void ComputeTask::end() {
    vkCheckError("vkEndCommandBuffer", nullptr,
    vkEndCommandBuffer(current));

    // currentSets.clear();
    // currentSets.resize(YAVF_MIN_DESCRIPTOR_COUNT, VK_NULL_HANDLE);
    currentPipeline = Pipeline();
  }

  void ComputeTask::setPipeline(const Pipeline &pipeline) {
    if (pipeline == currentPipeline) return;

    currentPipeline = pipeline;
    vkCmdBindPipeline(current, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.handle());
  }

//   void ComputeTask::setDescriptor(Image* image, const uint32_t &firstSet) {
//     // uint32_t set = firstSet == UINT32_MAX ? image->descriptor().setNum : firstSet;
//     //
//     // if (set >= currentSets.size()) currentSets.resize(set+1);
//     // else if (currentSets[set] == image->descriptor().handle) return;
//     //
//     // currentSets[set] = image->descriptor().handle;
// 
//     const VkDescriptorSet s = image->descriptor();
// 
//     vkCmdBindDescriptorSets(current, VK_PIPELINE_BIND_POINT_COMPUTE, currentPipeline.layout(),
//                             firstSet, 1, &s,
//                             0, nullptr);
//   }
// 
//   void ComputeTask::setDescriptor(Buffer* buffer, const uint32_t &firstSet) {
//     // uint32_t set = firstSet == UINT32_MAX ? buffer->descriptor().setNum : firstSet;
//     //
//     // if (set >= currentSets.size()) currentSets.resize(set+1);
//     // else if (currentSets[set] == buffer->descriptor().handle) return;
//     //
//     // currentSets[set] = buffer->descriptor().handle;
// 
//     const VkDescriptorSet s = buffer->descriptor();
// 
//     vkCmdBindDescriptorSets(current, VK_PIPELINE_BIND_POINT_COMPUTE, currentPipeline.layout(),
//                             firstSet, 1, &s,
//                             0, nullptr);
//   }
// 
//   void ComputeTask::setDescriptor(const Sampler &sampler, const uint32_t &firstSet) {
//     // uint32_t set = firstSet == UINT32_MAX ? sampler.descriptor().setNum : firstSet;
//     //
//     // if (set >= currentSets.size()) currentSets.resize(set+1);
//     // else if (currentSets[set] == sampler.descriptor().handle) return;
//     //
//     // currentSets[set] = sampler.descriptor().handle;
// 
//     const VkDescriptorSet s = sampler.descriptor();
// 
//     vkCmdBindDescriptorSets(current, VK_PIPELINE_BIND_POINT_COMPUTE, currentPipeline.layout(),
//                             firstSet, 1, &s,
//                             0, nullptr);
//   }
// 
//   void ComputeTask::setDescriptor(const Descriptor &descriptor, const uint32_t &firstSet) {
//     // uint32_t set = firstSet == UINT32_MAX ? descriptor.setNum : firstSet;
//     //
//     // if (set >= currentSets.size()) currentSets.resize(set+1);
//     // else if (currentSets[set] == descriptor.handle) return;
//     //
//     // currentSets[set] = descriptor.handle;
// 
//     vkCmdBindDescriptorSets(current, VK_PIPELINE_BIND_POINT_COMPUTE, currentPipeline.layout(),
//                             firstSet, 1, &descriptor,
//                             0, nullptr);
//   }
// 
//   void ComputeTask::setDescriptor(const std::vector<Descriptor> &descriptors, const uint32_t &firstSet) {
//     vkCmdBindDescriptorSets(current, VK_PIPELINE_BIND_POINT_COMPUTE, currentPipeline.layout(),
//                             firstSet, descriptors.size(), descriptors.data(),
//                             0, nullptr);
//   }

  void ComputeTask::setDescriptor(DescriptorSet* descriptor, const uint32_t &firstSet, const uint32_t &offset) {
    const VkDescriptorSet set = descriptor->handle();
    vkCmdBindDescriptorSets(current, VK_PIPELINE_BIND_POINT_COMPUTE, currentPipeline.layout(),
                            firstSet, 1, &set,
                            (offset == UINT32_MAX ? 0 : 1), (offset == UINT32_MAX ? nullptr : &offset));
  }

  void ComputeTask::setDescriptor(const VkDescriptorSet &descriptor, const uint32_t &firstSet, const uint32_t &offset) {
    vkCmdBindDescriptorSets(current, VK_PIPELINE_BIND_POINT_COMPUTE, currentPipeline.layout(),
                            firstSet, 1, &descriptor,
                            (offset == UINT32_MAX ? 0 : 1), (offset == UINT32_MAX ? nullptr : &offset));
  }
  
  void ComputeTask::setDescriptor(const std::initializer_list<VkDescriptorSet> &descriptors, const uint32_t &firstSet, const std::initializer_list<uint32_t> &offsets) {
    vkCmdBindDescriptorSets(current, VK_PIPELINE_BIND_POINT_COMPUTE, currentPipeline.layout(),
                            firstSet, static_cast<uint32_t>(descriptors.size()), descriptors.begin(),
                            static_cast<uint32_t>(offsets.size()), offsets.begin());
//                             (offsets.empty() ? 0 : offsets.size()), (offsets.empty() ? nullptr : offsets.data()));
  }

  void ComputeTask::setConsts(const uint32_t &offset, const uint32_t &size, const void* value, const VkShaderStageFlags &flags) {
    vkCmdPushConstants(current, currentPipeline.layout(), flags, offset, size, value);
  }

  void ComputeTask::dispatch(const uint32_t &countX, const uint32_t &countY, const uint32_t &countZ) {
    vkCmdDispatch(current, countX, countY, countZ);
  }

  void ComputeTask::dispatchIndirect(Buffer* buffer, const VkDeviceSize &offset) {
//     if (buffer->param().offset != 0) {
//       std::cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAa" << "\n";
//       exit(1);
//     }
    vkCmdDispatchIndirect(current, buffer->handle(), offset);
  }

//   Internal::Queue ComputeTask::start() {
//     VkSemaphore wait[waitSemaphoresCount() == 0 ? 1 : waitSemaphoresCount()];
//     VkPipelineStageFlags stageFlags[waitSemaphoresCount() == 0 ? 1 : waitSemaphoresCount()];
//     getWaitSemaphores(wait, stageFlags);
//     VkSemaphore signal[signalSemaphoresCount() == 0 ? 1 : signalSemaphoresCount()];
//     getSignalSemaphores(signal);
// 
//     const VkSubmitInfo info{
//       VK_STRUCTURE_TYPE_SUBMIT_INFO,
//       nullptr,
//       static_cast<uint32_t>(waitSemaphores.size()),
//       waitSemaphores.empty() ? nullptr : wait,
//       waitSemaphores.empty() ? nullptr : stageFlags,
//       1,
//       &current,
//       static_cast<uint32_t>(signalSemaphores.size()),
//       signalSemaphores.empty() ? nullptr : signal
//     };
// 
//     currentQueue = device->submit(family, 1, &info);
// 
//     return currentQueue;
//   }

  // VkSubmitInfo ComputeTask::getSubmitInfo() {
  //   return {
  //     VK_STRUCTURE_TYPE_SUBMIT_INFO,
  //     nullptr,
  //     static_cast<uint32_t>(waiting.size()),
  //     waiting.empty() ? nullptr : waiting.data(),
  //     waitFlags.empty() ? nullptr : waitFlags.data(),
  //     1,
  //     &current,
  //     static_cast<uint32_t>(signal.size()),
  //     signal.empty() ? nullptr : signal.data()
  //   };
  // }

  ComputeTask::ComputeTask() {}

  CombinedTask::CombinedTask(Device* device, const bool &primary) {
    this->device = device;
    this->primary = primary;

    family = device->getQueueFamilyIndex(YAVF_ANY_QUEUE);
    ASSERT(family < 4);

    // наверное нужно чекнуть есть ли вообще у устройства такие очереди?

//     commandBuffers.resize(framesCount);
//     semaphores.resize(framesCount);
//     commandBuffers.shrink_to_fit();
//     semaphores.shrink_to_fit();

    VkCommandPool pool = device->commandPool(3);

    const VkCommandBufferAllocateInfo info{
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      nullptr,
      pool,
      primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY,
      1
    };

    vkCheckError("vkAllocateCommandBuffers", nullptr,
    vkAllocateCommandBuffers(device->handle(), &info, &current));

    flags = YAVF_ANY_QUEUE;
  }

  CombinedTask::~CombinedTask() {
//     std::cout << "CombinedTask" << "\n";
    
    if (current != VK_NULL_HANDLE) {
      VkCommandPool pool = device->commandPool(3);
      vkFreeCommandBuffers(device->handle(), pool, 1, &current);
      current = VK_NULL_HANDLE;
    }
  }

  void CombinedTask::begin(const VkCommandBufferUsageFlags &flags) {
//     currentFrameIndex = (currentFrameIndex + 1) % commandBuffers.size();
//     current = commandBuffers[currentFrameIndex];

    vkCheckError("vkResetCommandBuffer", nullptr,
    vkResetCommandBuffer(current, 0));

    // здесь нужно любыми спсосбами получить информацию о всем происходящем
    // то есть текущий сабпасс, рендерпасс и фреймбуфер, как?
    // + oclusion query
    const VkCommandBufferInheritanceInfo ii{
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
      nullptr,
      VK_NULL_HANDLE,
      0,
      VK_NULL_HANDLE,
      VK_FALSE,
      0,
      0
    };

    const VkCommandBufferBeginInfo info{
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      nullptr,
      flags,
      primary ? nullptr : &ii
    };

    vkCheckError("vkBeginCommandBuffer", nullptr,
    vkBeginCommandBuffer(current, &info));
    
    ASSERT(family < 4);

//     for (uint32_t i = 0; i < semaphores[currentFrameIndex].size(); ++i) {
//       signalSemaphores[i]->changeSemaphore(semaphores[currentFrameIndex][i], semaphoreFlags[i]);
//     }
  }

  void CombinedTask::end() {
    vkCheckError("vkEndCommandBuffer", nullptr,
    vkEndCommandBuffer(current));

    currentPipeline = Pipeline();
    
    ASSERT(family < 4);
  }

  void CombinedTask::setPipeline(const Pipeline &pipeline) {
    if (pipeline == currentPipeline) return;

    currentPipeline = pipeline;

    VkPipelineBindPoint point = lastBindPoint == VK_PIPELINE_BIND_POINT_MAX_ENUM &&  renderpassStart ? VK_PIPELINE_BIND_POINT_GRAPHICS :
                                lastBindPoint == VK_PIPELINE_BIND_POINT_MAX_ENUM && !renderpassStart ? VK_PIPELINE_BIND_POINT_COMPUTE : lastBindPoint;

    vkCmdBindPipeline(current, point, pipeline.handle());
    
    ASSERT(family < 4);
  }

//   void CombinedTask::setDescriptor(Image* image, const uint32_t &firstSet) {
//     // uint32_t set = firstSet == UINT32_MAX ? image->descriptor().setNum : firstSet;
// 
//     VkPipelineBindPoint point = lastBindPoint == VK_PIPELINE_BIND_POINT_MAX_ENUM &&  renderpassStart ? VK_PIPELINE_BIND_POINT_GRAPHICS :
//                                 lastBindPoint == VK_PIPELINE_BIND_POINT_MAX_ENUM && !renderpassStart ? VK_PIPELINE_BIND_POINT_COMPUTE : lastBindPoint;
// 
//     // VkDescriptorSet setHandle = image->descriptor().handle;
// 
//     // if (point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
//     //   if (set >= currentSets.size()) currentSets.resize(set+1);
//     //   else if (currentSets[set] == setHandle) return;
//     //
//     //   currentSets[set] = setHandle;
//     // } else {
//     //   if (set >= computeSets.size()) computeSets.resize(set+1);
//     //   else if (computeSets[set] == setHandle) return;
//     //
//     //   computeSets[set] = setHandle;
//     // }
// 
//     const VkDescriptorSet s = image->descriptor();
// 
//     vkCmdBindDescriptorSets(current, point, currentPipeline.layout(),
//                             firstSet, 1, &s,
//                             0, nullptr);
//   }
// 
//   void CombinedTask::setDescriptor(Buffer* buffer, const uint32_t &firstSet) {
//     // uint32_t set = firstSet == UINT32_MAX ? buffer->descriptor().setNum : firstSet;
// 
//     VkPipelineBindPoint point = lastBindPoint == VK_PIPELINE_BIND_POINT_MAX_ENUM &&  renderpassStart ? VK_PIPELINE_BIND_POINT_GRAPHICS :
//                                 lastBindPoint == VK_PIPELINE_BIND_POINT_MAX_ENUM && !renderpassStart ? VK_PIPELINE_BIND_POINT_COMPUTE : lastBindPoint;
// 
//     // VkDescriptorSet setHandle = buffer->descriptor().handle;
//     //
//     // if (point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
//     //   if (set >= currentSets.size()) currentSets.resize(set+1);
//     //   else if (currentSets[set] == setHandle) return;
//     //
//     //   currentSets[set] = setHandle;
//     // } else {
//     //   if (set >= computeSets.size()) computeSets.resize(set+1);
//     //   else if (computeSets[set] == setHandle) return;
//     //
//     //   computeSets[set] = setHandle;
//     // }
// 
//     const VkDescriptorSet s = buffer->descriptor();
// 
//     vkCmdBindDescriptorSets(current, point, currentPipeline.layout(),
//                             firstSet, 1, &s,
//                             0, nullptr);
//   }
// 
//   void CombinedTask::setDescriptor(const Sampler &sampler, const uint32_t &firstSet) {
//     // uint32_t set = firstSet == UINT32_MAX ? sampler.descriptor().setNum : firstSet;
// 
//     VkPipelineBindPoint point = lastBindPoint == VK_PIPELINE_BIND_POINT_MAX_ENUM &&  renderpassStart ? VK_PIPELINE_BIND_POINT_GRAPHICS :
//                                 lastBindPoint == VK_PIPELINE_BIND_POINT_MAX_ENUM && !renderpassStart ? VK_PIPELINE_BIND_POINT_COMPUTE : lastBindPoint;
// 
//     // VkDescriptorSet setHandle = sampler.descriptor().handle;
//     //
//     // if (point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
//     //   if (set >= currentSets.size()) currentSets.resize(set+1);
//     //   else if (currentSets[set] == setHandle) return;
//     //
//     //   currentSets[set] = setHandle;
//     // } else {
//     //   if (set >= computeSets.size()) computeSets.resize(set+1);
//     //   else if (computeSets[set] == setHandle) return;
//     //
//     //   computeSets[set] = setHandle;
//     // }
// 
//     const VkDescriptorSet s = sampler.descriptor();
// 
//     vkCmdBindDescriptorSets(current, point, currentPipeline.layout(),
//                             firstSet, 1, &s,
//                             0, nullptr);
//   }
// 
//   void CombinedTask::setDescriptor(const Descriptor &descriptor, const uint32_t &firstSet) {
//     // uint32_t set = firstSet == UINT32_MAX ? descriptor.setNum : firstSet;
// 
//     VkPipelineBindPoint point = lastBindPoint == VK_PIPELINE_BIND_POINT_MAX_ENUM &&  renderpassStart ? VK_PIPELINE_BIND_POINT_GRAPHICS :
//                                 lastBindPoint == VK_PIPELINE_BIND_POINT_MAX_ENUM && !renderpassStart ? VK_PIPELINE_BIND_POINT_COMPUTE : lastBindPoint;
// 
//     // VkDescriptorSet setHandle = descriptor.handle;
//     //
//     // if (point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
//     //   if (set >= currentSets.size()) currentSets.resize(set+1);
//     //   else if (currentSets[set] == setHandle) return;
//     //
//     //   currentSets[set] = setHandle;
//     // } else {
//     //   if (set >= computeSets.size()) computeSets.resize(set+1);
//     //   else if (computeSets[set] == setHandle) return;
//     //
//     //   computeSets[set] = setHandle;
//     // }
// 
//     vkCmdBindDescriptorSets(current, point, currentPipeline.layout(),
//                             firstSet, 1, &descriptor,
//                             0, nullptr);
//   }
// 
//   void CombinedTask::setDescriptor(const std::vector<Descriptor> &descriptors, const uint32_t &firstSet) {
//     VkPipelineBindPoint point = lastBindPoint == VK_PIPELINE_BIND_POINT_MAX_ENUM &&  renderpassStart ? VK_PIPELINE_BIND_POINT_GRAPHICS :
//                                 lastBindPoint == VK_PIPELINE_BIND_POINT_MAX_ENUM && !renderpassStart ? VK_PIPELINE_BIND_POINT_COMPUTE : lastBindPoint;
// 
//     vkCmdBindDescriptorSets(current, point, currentPipeline.layout(),
//                             firstSet, descriptors.size(), descriptors.data(),
//                             0, nullptr);
//   }

  void CombinedTask::setDescriptor(DescriptorSet* descriptor, const uint32_t &firstSet, const uint32_t &offset) {
    VkPipelineBindPoint point = lastBindPoint == VK_PIPELINE_BIND_POINT_MAX_ENUM &&  renderpassStart ? VK_PIPELINE_BIND_POINT_GRAPHICS :
                                lastBindPoint == VK_PIPELINE_BIND_POINT_MAX_ENUM && !renderpassStart ? VK_PIPELINE_BIND_POINT_COMPUTE : lastBindPoint;
    
    const VkDescriptorSet set = descriptor->handle();
    vkCmdBindDescriptorSets(current, point, currentPipeline.layout(),
                            firstSet, 1, &set,
                            (offset == UINT32_MAX ? 0 : 1), (offset == UINT32_MAX ? nullptr : &offset));
    
    ASSERT(family < 4);
  }

  void CombinedTask::setDescriptor(const VkDescriptorSet &descriptor, const uint32_t &firstSet, const uint32_t &offset) {
    VkPipelineBindPoint point = lastBindPoint == VK_PIPELINE_BIND_POINT_MAX_ENUM &&  renderpassStart ? VK_PIPELINE_BIND_POINT_GRAPHICS :
                                lastBindPoint == VK_PIPELINE_BIND_POINT_MAX_ENUM && !renderpassStart ? VK_PIPELINE_BIND_POINT_COMPUTE : lastBindPoint;
                                
    vkCmdBindDescriptorSets(current, point, currentPipeline.layout(),
                            firstSet, 1, &descriptor,
                            (offset == UINT32_MAX ? 0 : 1), (offset == UINT32_MAX ? nullptr : &offset));
    
    ASSERT(family < 4);
  }
  
  void CombinedTask::setDescriptor(const std::initializer_list<VkDescriptorSet> &descriptors, const uint32_t &firstSet, const std::initializer_list<uint32_t> &offsets) {
    VkPipelineBindPoint point = lastBindPoint == VK_PIPELINE_BIND_POINT_MAX_ENUM &&  renderpassStart ? VK_PIPELINE_BIND_POINT_GRAPHICS :
                                lastBindPoint == VK_PIPELINE_BIND_POINT_MAX_ENUM && !renderpassStart ? VK_PIPELINE_BIND_POINT_COMPUTE : lastBindPoint;
                                
    vkCmdBindDescriptorSets(current, point, currentPipeline.layout(),
                            firstSet, descriptors.size(), descriptors.begin(),
                            offsets.size(), offsets.begin());
    
    ASSERT(family < 4);
  }

  void CombinedTask::setConsts(const uint32_t &offset, const uint32_t &size, const void* value, const VkShaderStageFlags &flags) {
    vkCmdPushConstants(current, currentPipeline.layout(), flags, offset, size, value);
  }

  void CombinedTask::setComputePipeline(const Pipeline &pipe) {
    if (pipe == currentPipeline) return;

    currentPipeline = pipe;

    lastBindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;

    vkCmdBindPipeline(current, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.handle());
    
    ASSERT(family < 4);
  }

  void CombinedTask::setGraphicPipeline(const Pipeline &pipe) {
    if (pipe == currentPipeline) return;

    currentPipeline = pipe;

    lastBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    vkCmdBindPipeline(current, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    
    ASSERT(family < 4);
  }

  void CombinedTask::setBindPoint(const VkPipelineBindPoint &point) {
    lastBindPoint = point;
    
    ASSERT(family < 4);
  }

//   Internal::Queue CombinedTask::start() {
//     VkSemaphore wait[waitSemaphoresCount() == 0 ? 1 : waitSemaphoresCount()];
//     VkPipelineStageFlags stageFlags[waitSemaphoresCount() == 0 ? 1 : waitSemaphoresCount()];
//     getWaitSemaphores(wait, stageFlags);
//     VkSemaphore signal[signalSemaphoresCount() == 0 ? 1 : signalSemaphoresCount()];
//     getSignalSemaphores(signal);
// 
//     const VkSubmitInfo info{
//       VK_STRUCTURE_TYPE_SUBMIT_INFO,
//       nullptr,
//       static_cast<uint32_t>(waitSemaphores.size()),
//       waitSemaphores.empty() ? nullptr : wait,
//       waitSemaphores.empty() ? nullptr : stageFlags,
//       1,
//       &current,
//       static_cast<uint32_t>(signalSemaphores.size()),
//       signalSemaphores.empty() ? nullptr : signal
//     };
// 
//     currentQueue = device->submit(family, 1, &info);
// 
//     return currentQueue;
//   }
}

