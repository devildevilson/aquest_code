#ifndef DEVILS_ENGINE_UTILS_STATIC_STACK_H
#define DEVILS_ENGINE_UTILS_STATIC_STACK_H

#include <cstddef>
#include <array>
#include <memory>
#include "assert.h"

// зачем я сделал заполнение с конца?
namespace devils_engine {
  namespace utils {
    template <typename T, size_t N>
    class static_stack {
    public:
      struct alignas(T) obj_mem { char mem[sizeof(T)]; };
      static_assert(sizeof(obj_mem) == sizeof(T));
      static_assert(alignof(obj_mem) == alignof(T));
      
      static_stack() : current_size(0) {}
      ~static_stack() { while (!empty()) { pop(); } }
      
      void push(const T &obj) { ASSERT(current_size <= N); ++current_size; new (&cont[N-current_size]) T(obj); }
      void pop() { ASSERT(current_size != 0); if constexpr (std::is_destructible_v<T>) { auto ptr = begin(); ptr->~T(); } --current_size; }
      template <typename... Args>
      void emplace(Args&&... args) { ASSERT(current_size <= N); ++current_size; new (&cont[N-current_size]) T(std::forward<Args>(args)...); }
      
      T & top() { ASSERT(current_size <= N); return *reinterpret_cast<T*>(&cont[N-current_size]); }
      const T & top() const { ASSERT(current_size <= N); return *reinterpret_cast<T*>(&cont[N-current_size]); }
      bool empty() const { return current_size == 0; }
      size_t size() const { return current_size; }
      T* begin() { return reinterpret_cast<T*>(&cont[N-current_size]); }
      T* end() { return reinterpret_cast<T*>(cont.end()); }
      const T* begin() const { return reinterpret_cast<T*>(&cont[N-current_size]); }
      const T* end() const { return reinterpret_cast<T*>(cont.end()); }
    private:
      size_t current_size;
      std::array<obj_mem, N> cont;
    };
  }
}

#endif
