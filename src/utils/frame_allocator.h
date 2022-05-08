#ifndef DEVILS_ENGINE_UTILS_FRAME_ALLOCATOR_H
#define DEVILS_ENGINE_UTILS_FRAME_ALLOCATOR_H

#include <cstdint>
#include <cstddef>
#include <atomic>
#include <type_traits>
//#include <stdexcept>
//#include "constexpr_funcs.h"

namespace devils_engine {
  namespace utils {
    class frame_allocator {
    public:
      static const size_t default_aligment = 16;
      
      frame_allocator(const size_t &size);
      ~frame_allocator();
      
      void* allocate(const size_t &size);
      
      template <typename T, typename... Args>
      T* create(Args&& ...args) {
        static_assert(std::is_trivially_destructible_v<T>);
        static_assert(alignof(T) <= default_aligment);
        auto ptr = allocate(sizeof(T));
        return new (ptr) T(std::forward<Args>(args)...);
      }
      
      size_t size() const;
      size_t allocated() const;
      
      void reset();
    private:
      char* m_memory;
      size_t m_size;
      std::atomic<size_t> m_current;
    };
    
//     template <size_t N>
//     class static_allocator {
//     public:
//       static_allocator() : m_current(0) { memset(m_memory, 0, N); }
//       
//       void* allocate(const size_t &size) {
//         const size_t final_size = align_to(size, 16);
//         const size_t prev = m_current.fetch_add(final_size);
//         if (prev > this->size() || this->size() - prev < final_size) throw std::runtime_error("Could not allocate " + std::to_string(size) + " from frame allocator");
//         return &m_memory[prev];
//       }
//       
//       template <typename T, typename... Args>
//       T* create(Args&& ...args) {
//         static_assert(std::is_trivially_destructible_v<T>);
//         auto ptr = allocate(sizeof(T)); // алигмент?
//         return new (ptr) T(std::forward<Args>(args)...);
//       }
//       
//       constexpr size_t size() const { return N; }
//       size_t allocated() const { return m_current; }
//       void reset() { m_current = 0; } // нужно ли чистить память?
//     private:
//       char m_memory[N];
//       std::atomic<size_t> m_current;
//     };
  }
}

#endif
