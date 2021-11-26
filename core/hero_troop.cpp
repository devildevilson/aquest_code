#include "hero_troop.h"

#include "utils/globals.h"
#include "bin/map.h"
#include "utils/systems.h"

namespace devils_engine {
  namespace core {
    const structure hero_troop::s_type;
    const size_t hero_troop::modificators_container_size;
    const size_t hero_troop::events_container_size;
    const size_t hero_troop::flags_container_size;
    const size_t hero_troop::max_game_party_size;
    hero_troop::hero_troop() : 
      object_token(SIZE_MAX),
      party_size(0), 
      max_party_size(0),
      current_pos(0.0f), 
      tile_index(UINT32_MAX)
//       army_gpu_slot(UINT32_MAX) 
    {
//       auto map = global::get<systems::map_t>()->map;
//       std::unique_lock<std::mutex> lock(map->mutex);
//       army_gpu_slot = map->allocate_army_data();
      
      modificators.reserve(modificators_container_size);
      events.reserve(events_container_size);
      flags.reserve(flags_container_size);
    }
    
    hero_troop::~hero_troop() {
//       auto map = global::get<systems::map_t>()->map;
//       std::unique_lock<std::mutex> lock(map->mutex);
//       map->release_army_data(army_gpu_slot);
      object_token = SIZE_MAX;
    }
  }
}
