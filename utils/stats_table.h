#ifndef STAT_TABLE_H
#define STAT_TABLE_H

#include <parallel_hashmap/phmap.h>
#include "core/stats.h"
#include <string>
#include "magic_enum_header.h"

#define MAKE_MAP_PAIR(type, index) std::make_pair(magic_enum::enum_name<type::values>(type::index), type::index)

namespace devils_engine {
  namespace utils {
    const phmap::flat_hash_map<std::string_view, core::character_stats::values> character_stats_map = {
      //std::make_pair(magic_enum::enum_name<core::character_stats::values>(core::character_stats::authority), core::character_stats::authority),
      //MAKE_MAP_PAIR(core::character_stats, authority)
      
#define CHARACTER_STAT_FUNC(val) MAKE_MAP_PAIR(core::character_stats, val)
      CHARACTER_STATS_LIST
#undef CHARACTER_STAT_FUNC
    };
    
    const phmap::flat_hash_map<std::string_view, core::troop_stats::values> troop_stats_map = {  
#define TROOP_STAT_FUNC(val) MAKE_MAP_PAIR(core::troop_stats, val)
      TROOP_STATS_LIST
#undef TROOP_STAT_FUNC
    };
    
    const phmap::flat_hash_map<std::string_view, core::hero_stats::values> hero_stats_map = {  
#define HERO_STAT_FUNC(val) MAKE_MAP_PAIR(core::hero_stats, val)
      HERO_STATS_LIST
#undef HERO_STAT_FUNC
    };
    
    const phmap::flat_hash_map<std::string_view, core::realm_stats::values> faction_stats_map = {  
#define REALM_STAT_FUNC(val) MAKE_MAP_PAIR(core::realm_stats, val)
      REALM_STATS_LIST
#undef REALM_STAT_FUNC
    };
    
    const phmap::flat_hash_map<std::string_view, core::province_stats::values> province_stats_map = {  
#define PROVINCE_STAT_FUNC(val) MAKE_MAP_PAIR(core::province_stats, val)
      PROVINCE_STATS_LIST
#undef PROVINCE_STAT_FUNC
    };
    
    const phmap::flat_hash_map<std::string_view, core::city_stats::values> city_stats_map = {  
#define CITY_STAT_FUNC(val) MAKE_MAP_PAIR(core::city_stats, val)
      CITY_STATS_LIST
#undef CITY_STAT_FUNC
    };
    
    const phmap::flat_hash_map<std::string_view, core::army_stats::values> army_stats_map = {  
#define ARMY_STAT_FUNC(val) MAKE_MAP_PAIR(core::army_stats, val)
      ARMY_STATS_LIST
#undef ARMY_STAT_FUNC
    };
    
    const phmap::flat_hash_map<std::string_view, core::hero_troop_stats::values> hero_troop_stats_map = {  
#define HERO_TROOP_STAT_FUNC(val) MAKE_MAP_PAIR(core::hero_troop_stats, val)
      HERO_TROOP_STATS_LIST
#undef HERO_TROOP_STAT_FUNC
    };
  }
}

#endif
