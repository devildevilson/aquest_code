#ifndef BATTLE_MAP_ENUM_H
#define BATTLE_MAP_ENUM_H

#include "string_container.h"

namespace devils_engine {
  namespace utils {
    enum class battle_strings {
      tile_texture_id,
      tile_walls_texture_id,
      
      count
    };
    
    using battle_map_string_container = container_strings<static_cast<size_t>(battle_strings::count)>;
    
    enum class world_map_strings {
      tile_biome_id,
      
      count
    };
    
    using world_map_string_container = container_strings<static_cast<size_t>(world_map_strings::count)>;
  }
}

#endif
