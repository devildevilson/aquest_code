#ifndef CONSTEXPR_FUNCS_H
#define CONSTEXPR_FUNCS_H

#include <cstdint>
#include <cstddef>
#include <string_view>

namespace devils_engine {
  constexpr size_t align_to(size_t memory, size_t aligment) {
    return (memory + aligment - 1) / aligment * aligment;
  }
  
  constexpr int64_t ceil(double f) {
    const int64_t i = static_cast<int64_t>(f);
    return f > i ? i + 1 : i;
  }

  constexpr int64_t floor(double f) {
    const int64_t i = static_cast<int64_t>(f);
    return f < i ? i - 1 : i;
  }
  
  constexpr size_t divide_token(const std::string_view &str, const std::string_view &symbol, const size_t &max_count, std::string_view* array) {
    size_t current = 0;
    size_t prev = 0;
    size_t counter = 0;
    const size_t symbols_count = symbol.length();
    while (current != std::string_view::npos && counter < max_count) {
      current = str.find(symbol, prev);
      const auto part = str.substr(prev, current-prev);
      array[counter] = part;
      ++counter;
      prev = current + symbols_count;
    }
    
    counter = counter < max_count ? counter : SIZE_MAX;
    
    return counter;
  }
  
  template <typename T>
  constexpr const T & max(const T &first) { return first; }
  
  template <typename T, typename... Ts>
  constexpr const T & max(const T &first, const Ts &... other) {
    const auto &second = max(other...);
    return second < first ? first : second;
  }
  
  template <typename T>
  constexpr const T & min(const T &first) { return first; }
  
  template <typename T, typename... Ts>
  constexpr const T & min(const T &first, const Ts &... other) {
    const auto &second = min(other...);
    return first < second ? first : second;
  }
  
  constexpr size_t count_useful_bits(size_t number) {
    size_t i = 0;
    for (; number != 0; number = number >> 1, ++i);
    return i;
  }
  
  constexpr size_t count_bits(size_t number) {
    const size_t size_width = SIZE_WIDTH;
    size_t counter = 0;
    for (size_t i = 0; i < size_width; counter += size_t(bool(number & (size_t(1) << i))), ++i);
    return counter;
  }
  
  constexpr size_t make_mask(const size_t bits_count) {
    const size_t size_width = SIZE_WIDTH;
    size_t mask = 0;
    for (size_t i = 0; i < min(size_width, bits_count); ++i) { mask |= size_t(1) << i; }
    return mask;
  }
  
  constexpr uint64_t make_64bit(const uint32_t &first, const uint32_t &second) {
    return (uint64_t(first) << 32) | uint64_t(second);
  }
  
  constexpr size_t fnv_basis = 14695981039346656037ull;
  constexpr size_t fnv_prime = 1099511628211ull;
  constexpr size_t string_hash(const std::string_view &str) {
    size_t current_hash = fnv_basis;
    for (const char c : str) { current_hash = (current_hash ^ c) * fnv_prime; }
    return current_hash;
  }
}

#endif
