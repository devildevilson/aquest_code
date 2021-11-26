#ifndef DEVILS_ENGINE_UTILS_USER_DATA_H
#define DEVILS_ENGINE_UTILS_USER_DATA_H

#include <cstddef>

namespace devils_engine {
  namespace utils {
    struct user_data {
      size_t value;
      const void* data;
      
      inline user_data() : value(0), data(nullptr) {}
    };
  }
}

#endif
