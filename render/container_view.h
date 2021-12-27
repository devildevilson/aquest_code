#ifndef DEVILS_ENGINE_RENDER_CONTAINER_VIEW_H
#define DEVILS_ENGINE_RENDER_CONTAINER_VIEW_H

#include "interfaces.h"
#include "target.h"

namespace devils_engine {
  namespace render {
    struct container;
    
    class container_view final : public target, public semaphore_provider, public resource_provider {
    public:
      inline container_view(container* c) : c(c) {}
      
      void recreate(const uint32_t &width, const uint32_t &height) override;
      vk::Rect2D get_render_area(const size_t &id) const override;
      size_t get_number(const size_t &id) const override;
      std::tuple<vk::Buffer, size_t> get_buffer(const size_t &id) const override;
      vk::DescriptorSet get_descriptor_set(const size_t &id) const override;
      vk::Framebuffer get_framebuffer(const size_t &id) const override;
      vk::CommandBuffer get_command_buffer(const size_t &id) const override;
      vk::ClearValue get_clear_value(const size_t &id) const override;
      size_t get_numbers(const size_t &id, const size_t &max, size_t* arr) const override;
      size_t get_buffers(const size_t &id, const size_t &max, std::tuple<vk::Buffer, size_t>* arr) const override;
      size_t get_descriptor_sets(const size_t &id, const size_t &max, vk::DescriptorSet* sets) const override;
      size_t get_framebuffers(const size_t &id, const size_t &max, vk::Framebuffer* framebuffers) const override;
      size_t get_command_buffers(const size_t &id, const size_t &max, vk::CommandBuffer* command_buffers) const override;
      size_t get_clear_values(const size_t &id, const size_t &max, vk::ClearValue* values) const override;
      size_t get_signaling(const size_t &max, vk::Semaphore* semaphores_arr, vk::PipelineStageFlags* flags_arr) const override;
    private:
      container* c;
    };
  }
}

#endif
