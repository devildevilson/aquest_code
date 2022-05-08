#ifndef DEVILS_ENGINE_RENDER_COMMAND_BUFFER_H
#define DEVILS_ENGINE_RENDER_COMMAND_BUFFER_H

#include "interfaces.h"
#include <array>

namespace devils_engine {
  namespace render {
    class pass;
    
    class primary_command_buffer final : public command_buffer {
    public:
      static const size_t max_command_buffers_count = 4;
      static const size_t max_semaphore_providers_count = 16;
      
      primary_command_buffer(vk::Device device, vk::CommandPool pool, const size_t buffering);
      ~primary_command_buffer();
      
      void begin(resource_provider* ctx) override;
      bool process(resource_provider* ctx, vk::CommandBuffer task) override;
      void clear() override;
      void add(const semaphore_provider* provider) override;
      size_t get_signaling(const size_t &max, vk::Semaphore* semaphores_arr, vk::PipelineStageFlags* flags_arr) const override;
      //size_t get_info(const size_t &max, vk::SubmitInfo* arr) const override;
      size_t get_info_wait  (const size_t &max_wait, vk::Semaphore* wait_arr, vk::PipelineStageFlags* wait_flags_arr) const override;
      size_t get_info_buffer(const size_t &max_buf,  vk::CommandBuffer* combuf_arr) const override;
      size_t get_info_signal(const size_t &max_sig,  vk::Semaphore* sig_arr) const override;
      
      void set_childs(stage* childs);
    private:
      vk::Device device;
      vk::CommandPool pool;
      size_t current;
      size_t count;
      std::array<vk::CommandBuffer, max_command_buffers_count> command_buffers;
      std::array<vk::Semaphore, max_command_buffers_count> semaphores;
      size_t providers_count;
      std::array<const semaphore_provider*, max_semaphore_providers_count> providers;
      stage* childs;
    };
    
    // сюда добавить вторичный командный буфер? как назвать?
    class secondary_command_buffer final : public command_buffer {
    public:
      static const size_t max_command_buffers_count = 4;
      
      secondary_command_buffer(vk::Device device, vk::CommandPool pool, const size_t buffering, const size_t &subpass_index);
      ~secondary_command_buffer();
      
      // во первых командный буфер нужно отсюда получить, а во вторых сюда нужно передать информацию о рендерпассе
      // по идее я могу получить из resource_provider, этот конкретный класс пересоздавать не нужно
      void begin(resource_provider* ctx) override;
      bool process(resource_provider* ctx, vk::CommandBuffer task) override;
      void clear() override;
      
      void add(const semaphore_provider* provider) override;
      size_t get_signaling(const size_t &max, vk::Semaphore* semaphores_arr, vk::PipelineStageFlags* flags_arr) const override;
      //size_t get_info(const size_t &max, vk::SubmitInfo* arr) const override;
      size_t get_info_wait  (const size_t &max_wait, vk::Semaphore* wait_arr, vk::PipelineStageFlags* wait_flags_arr) const override;
      size_t get_info_buffer(const size_t &max_buf,  vk::CommandBuffer* combuf_arr) const override;
      size_t get_info_signal(const size_t &max_sig,  vk::Semaphore* sig_arr) const override;
      
      void set_childs(stage* childs);
      void set_renderpass(pass* renderpass);
      vk::CommandBuffer get_current() const;
    private:
      vk::Device device;
      size_t current;
      size_t count;
      std::array<vk::CommandBuffer, max_command_buffers_count> command_buffers; // по одному на каждый кадр?
      // класс не выдает и не принимает семафоры
      size_t subpass_index;
      stage* childs;
      pass* renderpass;
    };
  }
}

#endif

