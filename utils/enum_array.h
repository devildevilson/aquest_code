#ifndef DEVILS_ENGINE_UTILS_ENUM_ARRAY_H
#define DEVILS_ENGINE_UTILS_ENUM_ARRAY_H

#include <cstring>
#include "assert.h"
#include "constexpr_funcs.h"

namespace devils_engine {
  namespace utils {
    template <size_t M, size_t N>
    struct enum_array {
      static const size_t max_bits_count = count_useful_bits(M-1);
      static const size_t size_width_count = SIZE_WIDTH / max_bits_count;
      static const size_t array_size = ceil(double(N) / double(size_width_count));
      static const size_t enum_mask = make_mask(max_bits_count);
      
      size_t array[array_size];
      
      enum_array() noexcept { memset(array, 0, array_size * sizeof(size_t)); }
      
      size_t get(const size_t &index) const noexcept {
        ASSERT(index < N);
        const size_t array_index = index / size_width_count;
        const size_t offset      = (index % size_width_count) * max_bits_count;
        const size_t data = array[array_index];
        return (data >> offset) & enum_mask;
      }
      
      size_t set(const size_t &index, const size_t &value) noexcept {
        ASSERT(value < M);
        ASSERT(index < N);
        const size_t array_index = index / size_width_count;
        const size_t offset      = (index % size_width_count) * max_bits_count;
        const size_t clear_value = ~(enum_mask << offset);
        const size_t final_value = value << offset;
        const size_t data = array[array_index];
        array[array_index] = (data & clear_value) | final_value;
        return (data >> offset) & enum_mask;
      }
    };
  }
}

#endif
