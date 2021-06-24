#include "tile.h"

namespace devils_engine {
  namespace core {
    const structure tile::s_type;
    tile::tile() : height(0.0f), province(UINT32_MAX), city(UINT32_MAX), struct_index(UINT32_MAX) {}
  }
}
