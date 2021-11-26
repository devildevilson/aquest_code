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
  
  constexpr double cast_to_double(const int64_t &val) {
    union conv {double d; int64_t i; };
    conv c{.i = val};
    return c.d;
  }

  constexpr int64_t cast_to_int64(const double &val) {
    union conv {double d; int64_t i; };
    conv c{.d = val};
    return c.i;
  }

  constexpr float cast_to_float(const int32_t &val) {
    union conv {float d; int32_t i; };
    conv c{.i = val};
    return c.d;
  }

  constexpr int32_t cast_to_int32(const float &val) {
    union conv {float d; int32_t i; };
    conv c{.d = val};
    return c.i;
  }
  
  constexpr int32_t uns_to_signed32(const uint32_t &val) {
    union conv { uint32_t u; int32_t i; };
    conv c{.u = val};
    return c.i;
  }
  
  constexpr uint32_t s_to_unsigned32(const int32_t &val) {
    union conv { uint32_t u; int32_t i; };
    conv c{.i = val};
    return c.u;
  }
  
  constexpr int64_t uns_to_signed64(const uint64_t &val) {
    union conv { uint64_t u; int64_t i; };
    conv c{.u = val};
    return c.i;
  }
  
  constexpr uint64_t s_to_unsigned64(const int64_t &val) {
    union conv { uint64_t u; int64_t i; };
    conv c{.i = val};
    return c.u;
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
}

#endif
