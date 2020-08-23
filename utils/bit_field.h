#ifndef BIT_FIELD_H
#define BIT_FIELD_H

#include <cstddef>
#include <cstdint>
#include <cstring>
#include "assert.h"

namespace devils_engine {
  namespace utils {
    template <size_t N>
    struct bit_field {
      size_t container[N]; // атомарность? вряд ли
      
      inline bit_field() {
        memset(container, 0, sizeof(size_t) * N);
      }
      
      inline void set(const size_t &index, const bool value) {
        ASSERT(index < SIZE_WIDTH * N);
        const size_t container_index = index / SIZE_WIDTH;
        const size_t index_container = index % SIZE_WIDTH;
        const size_t mask = size_t(1) << index_container;
        container[container_index] = value ? container[container_index] | mask : container[container_index] & ~(mask);
      }
      
      inline bool get(const size_t &index) const {
        ASSERT(index < SIZE_WIDTH * N);
        const size_t container_index = index / SIZE_WIDTH;
        const size_t index_container = index % SIZE_WIDTH;
        const size_t mask = size_t(1) << index_container;
        return bool(container[container_index] & mask);
      }
      
      constexpr size_t capacity() const {
        return SIZE_WIDTH * N;
      }
    };
    
    template <size_t N>
    struct bit_field_32 {
      uint32_t container[N];
      
      inline bit_field_32() {
        memset(container, 0, sizeof(uint32_t) * N);
      }
      
      inline void set(const size_t &index, const bool value) {
        ASSERT(index < UINT32_WIDTH * N);
        const size_t container_index = index / UINT32_WIDTH;
        const uint32_t index_container = index % UINT32_WIDTH;
        const uint32_t mask = uint32_t(1) << index_container;
        container[container_index] = value ? container[container_index] | mask : container[container_index] & ~(mask);
      }
      
      inline bool get(const size_t &index) const {
        ASSERT(index < UINT32_WIDTH * N);
        const size_t container_index = index / UINT32_WIDTH;
        const uint32_t index_container = index % UINT32_WIDTH;
        const uint32_t mask = uint32_t(1) << index_container;
        return bool(container[container_index] & mask);
      }
      
      constexpr size_t capacity() const {
        return UINT32_WIDTH * N;
      }
    };
  }
}

#endif
