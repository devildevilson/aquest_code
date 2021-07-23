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
    return first >= second ? first : second;
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
}

#endif
