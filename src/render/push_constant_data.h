#ifndef DEVILS_ENGINE_RENDER_PUSH_CONSTANT_DATA_H
#define DEVILS_ENGINE_RENDER_PUSH_CONSTANT_DATA_H

#include <cstdint>

namespace devils_engine {
  namespace render {
    struct push_constant_data {
      uint32_t type;
      uint32_t data;
      uint32_t stencil;
      uint32_t color;
    };
  }
}

#endif
