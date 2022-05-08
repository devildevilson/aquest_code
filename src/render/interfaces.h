#ifndef DEVILS_ENGINE_RENDER_INTERFACES_H
#define DEVILS_ENGINE_RENDER_INTERFACES_H

#include "vulkan_declarations.h"
#include "vulkan_hpp_header.h"

#define RENDER_SINGLE_BUFFERING 1
#define RENDER_DOUBLE_BUFFERING 2
#define RENDER_TRIPLE_BUFFERING 3

namespace devils_engine {
  namespace render {
    struct context;
    
    class semaphore_provider {
    public:
      virtual ~semaphore_provider() = default;
      virtual size_t get_signaling(const size_t &max, vk::Semaphore* semaphores_arr, vk::PipelineStageFlags* flags_arr) const = 0;
    };
    
    // скорее всего я почти везде спокойно могу отправить resource_provider, но чтобы проверить это нужно просмотреть все написанные стейджи!
    // пока что нужно заменить в том что уже написано, более менее просмотрел
    class resource_provider {
    public:
      virtual ~resource_provider() = default;
      virtual vk::Rect2D get_render_area(const size_t &id) const = 0;
      
      virtual size_t get_number(const size_t &id) const = 0; // возможно потребуется получать еще какие то числа (например количество инстансов)
      virtual std::tuple<vk::Buffer, size_t> get_buffer(const size_t &id) const = 0; // нужно ли тут еще возвращать ptr? мне кажется что нет
      virtual vk::DescriptorSet get_descriptor_set(const size_t &id) const = 0; // оффсет для дескриптора?
      virtual vk::Framebuffer get_framebuffer(const size_t &id) const = 0;
      virtual vk::CommandBuffer get_command_buffer(const size_t &id) const = 0; // вторичные буферы
      virtual vk::ClearValue get_clear_value(const size_t &id) const = 0;
      
      // некоторые ресурсы можно брать сразу пачками
      virtual size_t get_numbers(const size_t &id, const size_t &max, size_t* arr) const = 0;
      virtual size_t get_buffers(const size_t &id, const size_t &max, std::tuple<vk::Buffer, size_t>* arr) const = 0;
      virtual size_t get_descriptor_sets(const size_t &id, const size_t &max, vk::DescriptorSet* sets) const = 0;
      virtual size_t get_framebuffers(const size_t &id, const size_t &max, vk::Framebuffer* framebuffers) const = 0;
      virtual size_t get_command_buffers(const size_t &id, const size_t &max, vk::CommandBuffer* command_buffers) const = 0;
      virtual size_t get_clear_values(const size_t &id, const size_t &max, vk::ClearValue* values) const = 0;
    };
    
    class interface {
    public:
      virtual ~interface() = default;
      virtual void begin(resource_provider* ctx) = 0;
      virtual bool process(resource_provider* ctx, vk::CommandBuffer task) = 0; // некоторые процессы не потребуют в кадре какой то работы, а значит и ждать нечего
      virtual void clear() = 0;
    };
    
    class stage : public interface {
    public:
      stage* next;
      inline stage() : next(nullptr) {}
    };
    
    class copy_stage {
    public:
      virtual ~copy_stage() = default;
      virtual void copy(resource_provider* ctx, vk::CommandBuffer task) const = 0;
    };
    
    // от одного командного буфера до множества, например для рендеринга теней
    class command_buffer : public interface, public semaphore_provider {
    public:
      command_buffer* next;
      
      command_buffer() : next(nullptr) {}
      virtual void add(const semaphore_provider* provider) = 0; // нужен ли ремув? недумаю
      
      // нужно получить 3 массива данных для одной структуры, как лучше одну или несколько функций? несколько функций проще для восприятия
      virtual size_t get_info_wait  (const size_t &max_wait, vk::Semaphore* wait_arr, vk::PipelineStageFlags* wait_flags_arr) const = 0;
      virtual size_t get_info_buffer(const size_t &max_buf,  vk::CommandBuffer* combuf_arr) const = 0;
      virtual size_t get_info_signal(const size_t &max_sig,  vk::Semaphore* sig_arr) const = 0;
    };
    
//     struct context : public resource_provider {
//     public:
//       // что еще в контексте? да вроде ничего
//     };
    
    // перенести
//     template <typename T>
//     class add_to_buffer {
//     public:
//       add_to_buffer(const vk_buffer_data &buffer, const size_t &max) : buffer(buffer), max(max), current(0) {}
//       
//       size_t add(const T &data) {
//         if (current >= max) throw std::runtime_error("Buffer is full");
//         auto arr = reinterpret_cast<T*>(buffer.ptr);
//         arr[current] = data;
//         ++current;
//       }
//       
//       void set(const size_t &index, const T &data) {
//         if (index >= max) throw std::runtime_error("Buffer max count (" + std::to_string(max) + ") is less then index (" + std::to_string(index) + ")");
//         auto arr = reinterpret_cast<T*>(buffer.ptr);
//         arr[index] = data;
//         current = std::max(index, current);
//       }
// 
//       void reset() { current = 0; }
//       size_t get_max_size() const { return max; }
//     protected:
//       vk_buffer_data buffer;
//       size_t max;
//       size_t current;
//     };
//       virtual size_t get_info(const size_t &max, vk::SubmitInfo* arr) const = 0;
//       virtual std::tuple<size_t, size_t, size_t> get_info_data(
//         const size_t &max_wait, vk::Semaphore* wait_arr, vk::PipelineStageFlags* wait_flags_arr,
//         const size_t &max_buf,  vk::CommandBuffer* combuf_arr,
//         const size_t &max_sig,  vk::Semaphore* sig_arr
//       ) const = 0;
    
  }
}

#endif
