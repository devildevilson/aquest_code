#ifndef DEVILS_ENGINE_UTILS_MURMUR_H
#define DEVILS_ENGINE_UTILS_MURMUR_H

#include <cstddef>
#include <cstdint>

namespace devils_engine {
  namespace utils {
    // не получится constexpr - есть несколько приведений указателей
    uint64_t murmur_hash64A(const void* key, const size_t len, const uint64_t seed);
  }
}

#endif
