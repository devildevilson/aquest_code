#ifndef DEVILS_ENGINE_UTILS_ARRAY_VIEW_H
#define DEVILS_ENGINE_UTILS_ARRAY_VIEW_H

#include <cstddef>
#include <vector>
#include <array>
#include <cassert>
#include <stdexcept>

namespace devils_engine {
  namespace utils {
    template <typename T>
    struct array_view {
      using const_pointer = const T*;
      using const_reference = const T&;
      using const_iterator = const T*;
      
      const T* ptr;
      size_t count;
      
      constexpr inline array_view() noexcept : ptr(nullptr), count(0) {}
      array_view(const array_view &copy) noexcept = default;
      array_view(array_view &&move) noexcept = default;
      array_view & operator=(const array_view &copy) noexcept = default;
      array_view & operator=(array_view &&move) noexcept = default;
      
      constexpr inline array_view(std::nullptr_t) noexcept : ptr(nullptr), count(0) {}
      template <typename Alloc>
      constexpr inline array_view(const std::vector<T, Alloc> &arr) noexcept : ptr(arr.data()), count(arr.size()) {}
      template <size_t N>
      constexpr inline array_view(const std::array<T, N> &arr) noexcept : ptr(arr.data()), count(arr.size()) {}
      template <size_t N>
      constexpr inline array_view(const std::array<T, N> &arr, const size_t &count) noexcept : ptr(arr.data()), count(count) { assert(count < arr.size()); }
      constexpr inline array_view(const std::initializer_list<T> &arr) noexcept : ptr(arr.data()), count(arr.size()) {}
      constexpr inline array_view(const T* ptr, const size_t &size) noexcept : ptr(ptr), count(size) {}
      constexpr inline array_view(const T &ref) noexcept : ptr(std::addressof(ref)), count(1) {}
      
      template <typename Alloc>
      constexpr inline array_view & operator=(const std::vector<T, Alloc> &arr) noexcept { ptr = arr.data(); count = arr.size(); }
      template <size_t N>
      constexpr inline array_view & operator=(const std::array<T, N> &arr) noexcept { ptr = arr.data(); count = arr.size(); }
      constexpr inline array_view & operator=(const std::initializer_list<T> &arr) noexcept { ptr = arr.data(); count = arr.size(); }
      constexpr inline array_view & operator=(const T &ref) noexcept { ptr = std::addressof(ref); count = 1; }
      
      constexpr inline const_reference operator[](const size_t &index) const noexcept { return ptr[index]; }
      constexpr inline const_reference at(const size_t &index) const { if (index >= size()) throw std::runtime_error("Invalid index"); return ptr[index]; }
      constexpr inline const_reference front() const noexcept { return ptr[0]; }
      constexpr inline const_reference back() const noexcept { return ptr[size()-1]; }
      constexpr inline const_iterator begin() const noexcept { return &ptr[0]; }
      constexpr inline const_iterator end() const noexcept { return &ptr[size()]; }
      constexpr inline const_pointer data() const noexcept { return ptr; }
      constexpr inline size_t size() const noexcept { return count; }
      constexpr inline bool empty() const noexcept { return count == 0; }
    };
  }
}

#endif
