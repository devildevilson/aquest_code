#include "queue.h"

#include "utils/assert.h"
#include <array>
#include <iostream>

namespace devils_engine {
  namespace render {
    queue::queue() : childs(nullptr), providers_count(0) {}
    queue::queue(vk::Device device, vk::Queue queue) : device(device), native(queue), childs(nullptr), providers_count(0), providers{nullptr}
    {
      fence = device.createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
    }
    
    queue::~queue() {
      device.destroy(fence);
    }
    
    void queue::begin(resource_provider* ctx) {
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        cur->begin(ctx);
      }
    }
    
    bool queue::process(resource_provider* ctx, vk::CommandBuffer task) {
      ASSERT(task == vk::CommandBuffer(nullptr));
      bool has_update = false;
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        const bool ret = cur->process(ctx, task);
        has_update = has_update || ret;
      }
      
      return has_update; // надо ли это запомнить чтобы потом не обновлять если не нужно?
    }
    
    void queue::clear() {
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        cur->clear();
      }
    }
    
    struct submit_info_data {
      static const size_t max_wait_semaphores_count = 16;
      static const size_t max_command_buffers_count = 8;
      static const size_t max_signal_semaphores_count = 8;
      
      size_t waits_count;
      std::array<vk::Semaphore, max_wait_semaphores_count> wait_arr;
      std::array<vk::PipelineStageFlags, max_wait_semaphores_count> flags_arr;
      size_t buffers_count;
      std::array<vk::CommandBuffer, max_command_buffers_count> buffer_arr;
      size_t signals_count;
      std::array<vk::Semaphore, max_signal_semaphores_count> signal_arr;
      
      inline submit_info_data() : waits_count(0), buffers_count(0), signals_count(0) {}
    };
    
    vk::Result queue::submit() {      
      device.resetFences(fence);
      
      // есть ли шанс что командные буферы будут пытаться формировать несколько vk::SubmitInfo? 
      // в голову приходит только раскидать тени по множеству сабмитов, но зачем? неочевидно
      const size_t max_count = 8;
      std::array<submit_info_data, max_count> submit_info_datas;
      std::array<vk::SubmitInfo, max_count> submit_infos;
      size_t counter = 0;
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        if (counter >= max_count) throw std::runtime_error("More submit info memory is needed");
        
        {
          const size_t current_offset = submit_info_datas[counter].waits_count;
          const size_t current_max = submit_info_datas[counter].wait_arr.size()-current_offset;
          const size_t count = cur->get_info_wait(current_max, &submit_info_datas[counter].wait_arr[current_offset], &submit_info_datas[counter].flags_arr[current_offset]);
          if (count == SIZE_MAX) throw std::runtime_error("More wait semaphores memory is needed");
          submit_info_datas[counter].waits_count += count;
        }
        
        {
          const size_t current_offset = submit_info_datas[counter].buffers_count;
          const size_t current_max = submit_info_datas[counter].buffer_arr.size()-current_offset;
          const size_t count = cur->get_info_buffer(current_max, &submit_info_datas[counter].buffer_arr[current_offset]);
          if (count == SIZE_MAX) throw std::runtime_error("More command buffers memory is needed");
          submit_info_datas[counter].buffers_count += count;
        }
        
        {
          const size_t current_offset = submit_info_datas[counter].signals_count;
          const size_t current_max = submit_info_datas[counter].signal_arr.size()-current_offset;
          const size_t count = cur->get_info_signal(current_max, &submit_info_datas[counter].signal_arr[current_offset]);
          if (count == SIZE_MAX) throw std::runtime_error("More signal semaphores memory is needed");
          submit_info_datas[counter].signals_count += count;
        }
        
        ++counter;
      }
      
      for (size_t i = 0; i < counter; ++i) {
        submit_infos[i] = vk::SubmitInfo(
          submit_info_datas[i].waits_count, 
          submit_info_datas[i].wait_arr.data(), 
          submit_info_datas[i].flags_arr.data(), 
          submit_info_datas[i].buffers_count, 
          submit_info_datas[i].buffer_arr.data(), 
          submit_info_datas[i].signals_count, 
          submit_info_datas[i].signal_arr.data()
        );
      }
      
//       for (size_t i = 0; i < offset; ++i) {
//         const auto &inf = submit_infos[i];
//         for (size_t i = 0; i < inf.waitSemaphoreCount; ++i) {
//           std::cout << "wait sem  " << i << " " << inf.pWaitSemaphores << "\n";
//           std::cout << "wait mask " << i << " " << inf.pWaitDstStageMask << "\n";
//         }
//         
//         for (size_t i = 0; i < inf.commandBufferCount; ++i) {
//           std::cout << "com  buf  " << i << " " << inf.pCommandBuffers << "\n";
//         }
//         
//         for (size_t i = 0; i < inf.signalSemaphoreCount; ++i) {
//           std::cout << "sig  sem  " << i << " " << inf.pSignalSemaphores << "\n";
//         }
//       }
      
//       std::cout << "queue " << native << "\n";
//       std::cout << "fence " << fence << "\n";
      
      return native.submit(counter, submit_infos.data(), fence);
    }
    
    vk::Result queue::wait(const size_t &time) {
      return device.waitForFences(fence, true, time);
    }
    
    vk::Result queue::present(vk::SwapchainKHR swapchain, const uint32_t &image_index) {
      const size_t max_count = 32;
      std::array<vk::Semaphore, max_count> wait_semaphores;
      size_t offset = 0;
      for (size_t i = 0; i < providers_count; ++i) {
        const size_t count = providers[i]->get_signaling(max_count-offset, &wait_semaphores[offset], nullptr);
        if (count == SIZE_MAX) throw std::runtime_error("More semaphores is needed");
        offset += count;
      }
      
      const vk::PresentInfoKHR info(offset, wait_semaphores.data(), 1, &swapchain, &image_index);
      return native.presentKHR(info);
    }
    
    void queue::add_provider(const semaphore_provider* prov) { 
      if (providers_count >= providers.size()) throw std::runtime_error("Small semaphore providers array");
      providers[providers_count] = prov;
      ++providers_count;
    }
    
    void queue::set_childs(command_buffer* childs) { this->childs = childs; }
  }
}
