#ifndef TILES_FUNCS_H
#define TILES_FUNCS_H

#include <cstdint>

namespace devils_engine {
  namespace core {
    struct province;
    struct city;
    
    float get_tile_height(const uint32_t &tile_index);
    province* get_tile_province(const uint32_t &tile_index);
    city* get_tile_city(const uint32_t &tile_index);
    
    // данные биома?
  }
}

#endif
