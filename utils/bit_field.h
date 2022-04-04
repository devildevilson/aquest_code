#ifndef DEVILS_ENGINE_UTILS_BIT_FIELD_H
#define DEVILS_ENGINE_UTILS_BIT_FIELD_H

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <array>
#include "assert.h"
#include "constexpr_funcs.h"

#ifdef _WIN32
  #define SIZE_WIDTH 64
  #define UINT32_WIDTH 32
#endif

namespace devils_engine {
  namespace utils {
    template <size_t N>
    struct bit_field {
      using container_type = size_t;
      static const size_t container_size = ceil(double(N) / double(SIZE_WIDTH));
      std::array<container_type, container_size> container; // атомарность? вряд ли

      inline bit_field() { reset(); }
      inline bool set(const size_t &index, const bool value) {
        ASSERT(index < capacity());
        const size_t container_index = index / SIZE_WIDTH;
        const size_t index_container = index % SIZE_WIDTH;
        const container_type mask = container_type(1) << index_container;
        const bool old_value = bool(container[container_index] & mask);
        container[container_index] = value ? container[container_index] | mask : container[container_index] & ~(mask);
        return old_value;
      }

      inline bool get(const size_t &index) const {
        const size_t container_index = index / SIZE_WIDTH;
        const size_t index_container = index % SIZE_WIDTH;
        const container_type mask = container_type(1) << index_container;
        return index < capacity() ? bool((container[container_index] & mask) == mask) : false;
      }

      constexpr size_t capacity() const {
        return container_size * SIZE_WIDTH;
      }

      inline void reset() {
        memset(container.data(), 0, sizeof(container_type) * container_size);
      }
      
      inline bool empty() const {
        for (const container_type val : container) {
          if (val != 0) return false;
        }
        
        return true;
      }
      
      bit_field(const bit_field &other) = default;
      bit_field(bit_field &&other) = default;
      bit_field & operator=(const bit_field &other) = default;
      bit_field & operator=(bit_field &&other) = default;
    };

    template <uint32_t N>
    struct bit_field_32 {
      using container_type = uint32_t;
      static const size_t container_size = ceil(double(N) / double(UINT32_WIDTH));
      std::array<container_type, container_size> container;

      inline bit_field_32() { reset(); }
      inline bool set(const size_t &index, const bool value) {
        ASSERT(index < UINT32_WIDTH * N);
        const size_t container_index = index / UINT32_WIDTH;
        const size_t index_container = index % UINT32_WIDTH;
        const container_type mask = container_type(1) << index_container;
        const bool old_value = bool(container[container_index] & mask);
        container[container_index] = value ? container[container_index] | mask : container[container_index] & ~(mask);
        return old_value;
      }

      inline bool get(const size_t &index) const {
        const size_t container_index = index / UINT32_WIDTH;
        const size_t index_container = index % UINT32_WIDTH;
        const container_type mask = container_type(1) << index_container;
        return index < capacity() ? bool((container[container_index] & mask) == mask) : false;
      }

      constexpr size_t capacity() const {
        return container_size * SIZE_WIDTH;
      }

      inline void reset() {
        memset(container.data(), 0, sizeof(container_type) * container_size);
      }
      
      inline bool empty() const {
        for (const container_type val : container) {
          if (val != 0) return false;
        }
        
        return true;
      }
      
      bit_field_32(const bit_field_32 &other) = default;
      bit_field_32(bit_field_32 &&other) = default;
      bit_field_32 & operator=(const bit_field_32 &other) = default;
      bit_field_32 & operator=(bit_field_32 &&other) = default;
    };
  }
}

#endif
