#ifndef DEVILS_ENGINE_UTILS_STATIC_VECTOR_H
#define DEVILS_ENGINE_UTILS_STATIC_VECTOR_H

#include <cstddef>
#include <array>
#include "assert.h"

namespace devils_engine {
  namespace utils {
    template <typename T, size_t N>
    class static_vector {
    public:
      struct alignas(T) obj_mem { char mem[sizeof(T)]; };
      static_assert(sizeof(obj_mem) == sizeof(T));
      static_assert(alignof(obj_mem) == alignof(T));
      
      static_vector() : current_size(0) {}
      static_vector(const std::initializer_list<T> &list) : current_size(list.size()) {
        ASSERT(current_size <= N);
        std::copy(list.begin(), list.end(), cont.begin());
      }
      
      ~static_vector() { while (!empty()) { pop_back(); } }
      
      void push_back(const T &obj) { ASSERT(current_size < N); new (end()) T(obj); ++current_size; }
      void pop_back() { ASSERT(current_size != 0); if constexpr (std::is_destructible_v<T>) { auto ptr = end(); ptr->~T(); } --current_size; }
      template <typename... Args>
      void emplace_back(Args&&... args) { ASSERT(current_size < N); new (end()) T(std::forward<Args>(args)...); ++current_size; }
      
      bool empty() const { return current_size == 0; }
      size_t size() const { return current_size; }
      
      T & front() { ASSERT(current_size != 0); return *begin(); }
      const T & front() const { ASSERT(current_size != 0); return *begin(); }
      T & back() { ASSERT(current_size <= N); return *reinterpret_cast<T*>(&cont[current_size-1]); }
      const T & back() const { ASSERT(current_size <= N); return *reinterpret_cast<const T*>(&cont[current_size-1]); }
      T & operator[] (const size_t &index) { ASSERT(index < current_size); return *reinterpret_cast<T*>(&cont[index]); }
      const T & operator[] (const size_t &index) const { ASSERT(index < current_size); return *reinterpret_cast<const T*>(&cont[index]); }
      T* begin() { return reinterpret_cast<T*>(&cont[0]); }
      T* end() { return reinterpret_cast<T*>(&cont[current_size]); }
      const T* begin() const { return reinterpret_cast<const T*>(&cont[0]); }
      const T* end() const { return reinterpret_cast<const T*>(&cont[current_size]); }
    private:
      size_t current_size;
      std::array<obj_mem, N> cont;
    };
  }
}

#endif
