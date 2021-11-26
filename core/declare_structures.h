#ifndef DEVILS_ENGINE_CORE_DECLARE_STRUCTURES_H
#define DEVILS_ENGINE_CORE_DECLARE_STRUCTURES_H

#include <cstdint>
#include <cstddef>
#include "utils/constexpr_funcs.h"

#define GAME_STRUCTURES_LIST \
  GAME_STRUCTURE_FUNC(biome) \
  GAME_STRUCTURE_FUNC(building_type) \
  GAME_STRUCTURE_FUNC(holding_type) \
  GAME_STRUCTURE_FUNC(city_type) \
  GAME_STRUCTURE_FUNC(trait) \
  GAME_STRUCTURE_FUNC(modificator) \
  GAME_STRUCTURE_FUNC(troop_type) \
  GAME_STRUCTURE_FUNC(decision) \
  GAME_STRUCTURE_FUNC(interaction) \
  GAME_STRUCTURE_FUNC(religion_group) \
  GAME_STRUCTURE_FUNC(religion) \
  GAME_STRUCTURE_FUNC(culture_group) \
  GAME_STRUCTURE_FUNC(culture) \
  GAME_STRUCTURE_FUNC(law) \
  GAME_STRUCTURE_FUNC(event) \
  GAME_STRUCTURE_FUNC(titulus) \
  GAME_STRUCTURE_FUNC(casus_belli) \
  GAME_STRUCTURE_FUNC(city) \
  GAME_STRUCTURE_FUNC(tile) \
  GAME_STRUCTURE_FUNC(province) \
  GAME_STRUCTURE_FUNC(character) \
  GAME_STRUCTURE_FUNC(dynasty) \
  GAME_STRUCTURE_FUNC(realm) \
  GAME_STRUCTURE_FUNC(hero_troop) \
  GAME_STRUCTURE_FUNC(troop) \
  GAME_STRUCTURE_FUNC(army) \
  GAME_STRUCTURE_FUNC(war)

namespace devils_engine {
  namespace core {
#define GAME_STRUCTURE_FUNC(val) struct val;
      GAME_STRUCTURES_LIST
#undef GAME_STRUCTURE_FUNC
    
    enum class structure : uint32_t {      
#define GAME_STRUCTURE_FUNC(val) val,
      GAME_STRUCTURES_LIST
#undef GAME_STRUCTURE_FUNC
      count,
      
      static_types_count = character,
      parsing_types_count = realm,
      id_types_count = casus_belli + 1
    };
    
    namespace structure_mask {
      enum values : size_t {
#define GAME_STRUCTURE_FUNC(val) val = size_t(1) << static_cast<size_t>(structure::val),
        GAME_STRUCTURES_LIST
#undef GAME_STRUCTURE_FUNC
      };
      
      constexpr size_t all = make_mask(static_cast<size_t>(structure::count));
      constexpr size_t none = 0;
      constexpr size_t count = static_cast<size_t>(structure::count);
      constexpr size_t bit_container_size = ceil(double(count) / double(SIZE_WIDTH));
    }
  }
}

#endif
