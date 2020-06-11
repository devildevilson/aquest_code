#ifndef MAP_GENERATOR_H
#define MAP_GENERATOR_H

#include <cstdint>
#include <cstddef>
#include <string>

namespace devils_engine {
  namespace core {
    struct map;
  }
  
  namespace utils {
    struct random_engine;
  }
  
  namespace map {
    template <typename T>
    class generator {
    public:
      virtual ~generator() {}
      virtual void process(T* context) = 0;
      virtual size_t progress() const = 0;
      virtual size_t complete_state(const T* context) const = 0;
      virtual std::string hint() const = 0;
    };
  }
}

#endif
