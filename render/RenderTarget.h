#ifndef RENDER_TARGET_H
#define RENDER_TARGET_H

#include <vector>
#include <vulkan/vulkan.h>

#include "Types.h"

namespace yavf {
//   // кстати можно ли слушать одинаковые семафоры в разных командных буферах?
//   // судя по всему нет, слушание семафор автоматом переводит их в отсигналенное состояние
//   class SemaphoreProxy {
//   public:
//     Semaphore get() const { return s; }
//     VkPipelineStageFlags getFlags() const { return flags; }
//   protected:
//     Semaphore s;
//     VkPipelineStageFlags flags; // контролируется поставщиком семафора
//   };
//   
//   class SemaphoreOwner : public SemaphoreProxy {
//   public:
//     void changeSemaphore(const VkSemaphore &s, const VkPipelineStageFlags &flags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT) {
//       this->s = s;
//       this->flags = flags;
//     }
//   };
  
  class RenderTarget {
  public:
    virtual ~RenderTarget() {}

    virtual std::vector<VkClearValue> clearValues() const = 0;
    //virtual VkImage image() const = 0;
    virtual VkRect2D size() const = 0;
    virtual RenderPass renderPass() const = 0;
    virtual VkViewport viewport() const = 0;
    virtual VkRect2D scissor() const = 0;

    virtual Framebuffer framebuffer() const = 0;
//     //virtual VkSemaphore imageAvailable() const { return VK_NULL_HANDLE; }
//     
//     // я создаю НЕСКОЛЬКО проксей для того чтобы НЕСКОЛЬКО командных буферов подождали
//     // для картинок не имеет смысла создавать несколько проксей
//     // aquireImage работает только с одним семафором
//     virtual SemaphoreProxy* getSemaphoreProxy() const = 0;
//     //virtual SemaphoreProxy* createSemaphoreProxy(const VkPipelineStageFlags &flag = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT) = 0;
//     // я могу добавить НЕСКОЛЬКО проксей, чтобы подождать выполнения всех связаных командных буферов
//     virtual void addSemaphoreProxy(SemaphoreProxy* proxy) = 0;
//     //virtual VkSemaphore finishRendering() const { return VK_NULL_HANDLE; }
  };
}

#endif // !RENDER_TARGET_H
