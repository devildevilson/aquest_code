#include "command_buffer.h"

#include "utils/assert.h"
#include "defines.h"
#include "pass.h"
#include "utils/constexpr_funcs.h"

namespace devils_engine {
  namespace render {
    primary_command_buffer::primary_command_buffer(vk::Device device, vk::CommandPool pool, const size_t buffering) : 
      device(device), pool(pool), current(0), count(buffering), providers_count(0), providers{nullptr}, childs(nullptr)
    {
      ASSERT(count < max_command_buffers_count);
      vk::CommandBufferAllocateInfo info(pool, vk::CommandBufferLevel::ePrimary, count);
      const auto arr = device.allocateCommandBuffers(info);
      static_assert(sizeof(arr[0]) == sizeof(command_buffers[0]));
      memcpy(command_buffers.data(), arr.data(), count * sizeof(command_buffers[0]));
      for (size_t i = 0; i < count; ++i) {
        semaphores[i] = device.createSemaphore(vk::SemaphoreCreateInfo());
        set_name(device, semaphores[i], "frame command buffer " + std::to_string(i) + " semaphore");
        set_name(device, command_buffers[i], "frame command buffer " + std::to_string(i));
      }
    }
    
    primary_command_buffer::~primary_command_buffer() {
      // по идее при удалении коммандного пула удалятся и буферы, пул удаляем не здесь
      for (auto semaphore : semaphores) { device.destroy(semaphore); }
    }
    
    void primary_command_buffer::begin(resource_provider* ctx) {
      current = (current + 1) % count;
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        cur->begin(ctx);
      }
    }
    
    bool primary_command_buffer::process(resource_provider* ctx, vk::CommandBuffer task) {
      ASSERT(task == vk::CommandBuffer(nullptr));
      
      auto cur_task = command_buffers[current];
      cur_task.reset();
      vk::CommandBufferBeginInfo info(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
      cur_task.begin(info);
      
      bool has_update = false;
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        const bool ret = cur->process(ctx, cur_task);
        has_update = has_update || ret;
      }
      
      cur_task.end();
      
      return has_update;
    }
    
    void primary_command_buffer::clear() {
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        cur->clear();
      }
    }
    
    void primary_command_buffer::add(const semaphore_provider* provider) {
      if (providers_count >= providers.size()) throw std::runtime_error("Small semaphore providers array");
      providers[providers_count] = provider;
      ++providers_count;
    }
    
    size_t primary_command_buffer::get_signaling(const size_t &max, vk::Semaphore* semaphores_arr, vk::PipelineStageFlags* flags_arr) const {
      if (max == 0) return SIZE_MAX;
      semaphores_arr[0] = semaphores[current];
      if (flags_arr != nullptr) flags_arr[0] = vk::PipelineStageFlagBits::eColorAttachmentOutput;
      return 1;
    }
    
    size_t primary_command_buffer::get_info_wait  (const size_t &max, vk::Semaphore* arr, vk::PipelineStageFlags* flags_arr) const {
      if (max == 0) return SIZE_MAX;
      
      size_t offset = 0;
      for (size_t i = 0; i < providers_count; ++i) {
        const size_t count = providers[i]->get_signaling(max-offset, &arr[offset], &flags_arr[offset]);
        if (count == SIZE_MAX) return SIZE_MAX;
        offset += count;
      }
      
      return offset;
    }
    
    size_t primary_command_buffer::get_info_buffer(const size_t &max,  vk::CommandBuffer* arr) const {
      if (max == 0) return SIZE_MAX;
      arr[0] = command_buffers[current];
      return 1;
    }
    
    size_t primary_command_buffer::get_info_signal(const size_t &max,  vk::Semaphore* arr) const {
      if (max == 0) return SIZE_MAX;
      arr[0] = semaphores[current];
      return 1;
    }
    
    void primary_command_buffer::set_childs(stage* childs) { this->childs = childs; }
    
    secondary_command_buffer::secondary_command_buffer(vk::Device device, vk::CommandPool pool, const size_t buffering, const size_t &subpass_index) :
      device(device), current(0), count(buffering), subpass_index(subpass_index), childs(nullptr), renderpass(nullptr)
    {
      ASSERT(count < max_command_buffers_count);
      vk::CommandBufferAllocateInfo info(pool, vk::CommandBufferLevel::eSecondary, count);
      const auto arr = device.allocateCommandBuffers(info);
      static_assert(sizeof(arr[0]) == sizeof(command_buffers[0]));
      memcpy(command_buffers.data(), arr.data(), count * sizeof(command_buffers[0]));
      for (size_t i = 0; i < count; ++i) {
        set_name(device, command_buffers[i], ENVIRONMENT_COMMAND_BUFFER_NAME + std::to_string(i));
      }
    }
    
    secondary_command_buffer::~secondary_command_buffer() {}
    
    void secondary_command_buffer::begin(resource_provider* ctx) {
      if (childs == nullptr) return;
      
      current = (current + 1) % count;
      for (auto cur = childs; cur != nullptr; cur = cur->next) { cur->begin(ctx); }
    }
    
    bool secondary_command_buffer::process(resource_provider* ctx, vk::CommandBuffer task) {
      ASSERT(task == vk::CommandBuffer(nullptr));
      if (childs == nullptr) return false;
      
      auto cur_task = command_buffers[current];
      cur_task.reset();
      
//       const auto f = ctx->get_framebuffer(string_hash(MAIN_FRAMEBUFFER_NAME));
      const auto f = renderpass->get_framebuffer();
      vk::CommandBufferInheritanceInfo inheritance(renderpass->get_handle(), subpass_index, f, false);
      vk::CommandBufferBeginInfo info(vk::CommandBufferUsageFlagBits::eOneTimeSubmit | vk::CommandBufferUsageFlagBits::eRenderPassContinue, &inheritance);
      cur_task.begin(info);
      
      bool has_update = false;
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        has_update = has_update || cur->process(ctx, cur_task);
      }
      
      cur_task.end();
      
      return has_update;
    }
    
    void secondary_command_buffer::clear() {
      if (childs == nullptr) return;
      for (auto cur = childs; cur != nullptr; cur = cur->next) { cur->clear(); }
    }
    
    void secondary_command_buffer::add(const semaphore_provider*) { assert(false); }
    size_t secondary_command_buffer::get_signaling(const size_t &, vk::Semaphore*, vk::PipelineStageFlags*) const { return 0; }
    size_t secondary_command_buffer::get_info_wait  (const size_t &, vk::Semaphore*, vk::PipelineStageFlags*) const { return 0; }
    size_t secondary_command_buffer::get_info_buffer(const size_t &,  vk::CommandBuffer*) const { return 0; }
    size_t secondary_command_buffer::get_info_signal(const size_t &, vk::Semaphore*) const { return 0; }
    
    void secondary_command_buffer::set_childs(stage* childs) { this->childs = childs; }
    void secondary_command_buffer::set_renderpass(pass* renderpass) { this->renderpass = renderpass; }
    vk::CommandBuffer secondary_command_buffer::get_current() const { return command_buffers[current]; }
  }
}
