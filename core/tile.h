#ifndef TILE_H
#define TILE_H

#include "declare_structures.h"

namespace devils_engine {
  namespace core {
    struct tile_template {
      
    };
    
    struct tile {
      static const structure s_type = structure::tile;
      
      // возможно стоит выделить тайл и некоторые характеристики в нем
      // например для того чтобы описать правильно режим отображения карты
      float height; // вообще то это уже задано
      uint32_t province;
      uint32_t city;
      uint32_t struct_index;
      
      // темплейт
      tile();
    };
  }
}

#endif
