#ifndef DEVILS_ENGINE_RENDER_QUEUE_H
#define DEVILS_ENGINE_RENDER_QUEUE_H

#include "interfaces.h"
#include "utils/shared_time_constant.h"

namespace devils_engine {
  namespace render {
    class queue final : public stage {
    public:
      static const size_t max_semaphore_providers_count = 16;
      
      queue();
      queue(vk::Device device, vk::Queue queue);
      ~queue();
      void begin(resource_provider* ctx) override;
      bool process(resource_provider* ctx, vk::CommandBuffer task) override;
      void clear() override;
      vk::Result submit();
      vk::Result wait(const size_t &time = NANO_PRECISION);
      vk::Result present(vk::SwapchainKHR swapchain, const uint32_t &image_index);
      void add_provider(const semaphore_provider* prov);
      
      void set_childs(command_buffer* childs);
    private:
      vk::Device device;
      vk::Queue native;
      vk::Fence fence;
      command_buffer* childs;
      size_t providers_count;
      std::array<const semaphore_provider*, max_semaphore_providers_count> providers;
    };
  }
}

#endif
