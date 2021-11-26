#include "tile.h"

namespace devils_engine {
  namespace core {
    const structure tile::s_type;
    tile::tile() : 
      texture{GPU_UINT_MAX},
      color{GPU_UINT_MAX},
      height(0.0f), 
      center(UINT32_MAX), 
      points{UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX},
      neighbors{UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX},
      borders_data(UINT32_MAX),
      province(UINT32_MAX), 
      city(UINT32_MAX), 
      struct_index(UINT32_MAX), 
      biome_index(UINT32_MAX), 
      army_token(SIZE_MAX), 
      hero_token(SIZE_MAX), 
      movement_token(SIZE_MAX) 
    {}
    
    uint32_t tile::neighbors_count() const { return neighbors[5] == UINT32_MAX ? 5 : 6; }
  }
}
