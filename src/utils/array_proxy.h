#ifndef ARRAY_PROXY_H
#define ARRAY_PROXY_H

#include <cstddef>
#include <vector>
#include <array>

namespace devils_engine {
  namespace utils {
    template <typename T>
    class array_proxy {
    public:
      const T* memory;
      size_t count;
      
      array_proxy(std::nullptr_t) : memory(nullptr), count(0) {}
      array_proxy(const T &obj) : memory(&obj), count(1) {}
      array_proxy(const std::initializer_list<T> &obj) : memory(obj.begin()), count(obj.size()) {}
      array_proxy(const std::vector<T> &obj) : memory(obj.data()), count(obj.size()) {}
      template <size_t N>
      array_proxy(const std::array<T, N> &obj) : memory(obj.data()), count(N) {}
      template <size_t N>
      array_proxy(const std::array<T, N> &obj, const size_t &count) : memory(obj.data()), count(count) {}
      array_proxy(const T* obj, const size_t &count) : memory(obj), count(count) {}
      
      T* begin() { if (count == 0) return nullptr; return &memory[0]; }
      T* end() { if (count == 0) return nullptr; return &memory[count]; }
      const T* begin() const { if (count == 0) return nullptr; return &memory[0]; }
      const T* end() const { if (count == 0) return nullptr; return &memory[count]; }
      size_t size() const { return count; }
      bool empty() const { return count == 0; }
      const T & operator[] (const size_t &index) const { return memory[index]; }
    };
  }
}

#endif
