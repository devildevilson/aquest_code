#include "stats_table.h"

#define MAKE_MAP_PAIR(name) std::make_pair(names[values::name], values::name)

namespace devils_engine {
  namespace core {
    namespace character_stats {
      const std::string_view names[] = {
#define STAT_FUNC(val) #val,
#define CHARACTER_PENALTY_STAT_FUNC(val) #val "_penalty",
        CHARACTER_STATS_LIST
#undef STAT_FUNC
#undef CHARACTER_PENALTY_STAT_FUNC
      };
      
      const phmap::flat_hash_map<std::string_view, values> map = {
#define STAT_FUNC(val) MAKE_MAP_PAIR(val),
#define CHARACTER_PENALTY_STAT_FUNC(val) MAKE_MAP_PAIR(val##_penalty),
        CHARACTER_STATS_LIST
#undef STAT_FUNC
#undef CHARACTER_PENALTY_STAT_FUNC
      };
      
      const size_t array_size = sizeof(names) / sizeof(names[0]);
      static_assert(array_size == values::count);
    }
    
    namespace character_resources {
      const std::string_view names[] = {
#define STAT_FUNC(val) #val,
        RESOURCE_STATS_LIST
#undef STAT_FUNC
      };
      
      const phmap::flat_hash_map<std::string_view, values> map = {
#define STAT_FUNC(val) MAKE_MAP_PAIR(val),
        RESOURCE_STATS_LIST
#undef STAT_FUNC
      };
      
      const size_t array_size = sizeof(names) / sizeof(names[0]);
      static_assert(array_size == values::count);
    }
    
    namespace troop_stats {
      const std::string_view names[] = {
#define STAT_FUNC(val) #val,
        TROOP_STATS_LIST
#undef STAT_FUNC
      };
      
      const phmap::flat_hash_map<std::string_view, values> map = {
#define STAT_FUNC(val) MAKE_MAP_PAIR(val),
        TROOP_STATS_LIST
#undef STAT_FUNC
      };
      
      const size_t array_size = sizeof(names) / sizeof(names[0]);
      static_assert(array_size == values::count);
    }
    
    namespace hero_stats {
      const std::string_view names[] = {
#define STAT_FUNC(val) #val,
        HERO_STATS_LIST
#undef STAT_FUNC
      };
      
      const phmap::flat_hash_map<std::string_view, values> map = {
#define STAT_FUNC(val) MAKE_MAP_PAIR(val),
        HERO_STATS_LIST
#undef STAT_FUNC
      };
      
      const size_t array_size = sizeof(names) / sizeof(names[0]);
      static_assert(array_size == values::count);
    }
    
    namespace realm_stats {
      const std::string_view names[] = {
#define STAT_FUNC(val) #val,
        REALM_STATS_LIST
#undef STAT_FUNC
      };
      
      const phmap::flat_hash_map<std::string_view, values> map = {
#define STAT_FUNC(val) MAKE_MAP_PAIR(val),
        REALM_STATS_LIST
#undef STAT_FUNC
      };
      
      const size_t array_size = sizeof(names) / sizeof(names[0]);
      static_assert(array_size == values::count);
    }
    
    namespace realm_resources {
      const std::string_view names[] = {
#define STAT_FUNC(val) #val,
        RESOURCE_STATS_LIST
#undef STAT_FUNC
      };
      
      const phmap::flat_hash_map<std::string_view, values> map = {
#define STAT_FUNC(val) MAKE_MAP_PAIR(val),
        RESOURCE_STATS_LIST
#undef STAT_FUNC
      };
      
      const size_t array_size = sizeof(names) / sizeof(names[0]);
      static_assert(array_size == values::count);
    }
    
    namespace province_stats {
      const std::string_view names[] = {
#define STAT_FUNC(val) #val,
        PROVINCE_STATS_LIST
#undef STAT_FUNC
      };
      
      const phmap::flat_hash_map<std::string_view, values> map = {
#define STAT_FUNC(val) MAKE_MAP_PAIR(val),
        PROVINCE_STATS_LIST
#undef STAT_FUNC
      };
      
      const size_t array_size = sizeof(names) / sizeof(names[0]);
      static_assert(array_size == values::count);
    }
    
    namespace city_stats {
      const std::string_view names[] = {
#define STAT_FUNC(val) #val,
        CITY_STATS_LIST
#undef STAT_FUNC
      };
      
      const phmap::flat_hash_map<std::string_view, values> map = {
#define STAT_FUNC(val) MAKE_MAP_PAIR(val),
        CITY_STATS_LIST
#undef STAT_FUNC
      };
      
      const size_t array_size = sizeof(names) / sizeof(names[0]);
      static_assert(array_size == values::count);
    }
    
    namespace city_resources {
      const std::string_view names[] = {
#define STAT_FUNC(val) #val,
        CITY_RESOURCE_STATS_LIST
#undef STAT_FUNC
      };
      
      const phmap::flat_hash_map<std::string_view, values> map = {
#define STAT_FUNC(val) MAKE_MAP_PAIR(val),
        CITY_RESOURCE_STATS_LIST
#undef STAT_FUNC
      };
      
      const size_t array_size = sizeof(names) / sizeof(names[0]);
      static_assert(array_size == values::count);
    }
    
    namespace army_stats {
      const std::string_view names[] = {
#define STAT_FUNC(val) #val,
        ARMY_STATS_LIST
#undef STAT_FUNC
      };
      
      const phmap::flat_hash_map<std::string_view, values> map = {
#define STAT_FUNC(val) MAKE_MAP_PAIR(val),
        ARMY_STATS_LIST
#undef STAT_FUNC
      };
      
      const size_t array_size = sizeof(names) / sizeof(names[0]);
      static_assert(array_size == values::count);
    }
    
    namespace army_resources {
      const std::string_view names[] = {
#define STAT_FUNC(val) #val,
        ARMY_RESOURCE_STATS_LIST
#undef STAT_FUNC
      };
      
      const phmap::flat_hash_map<std::string_view, values> map = {
#define STAT_FUNC(val) MAKE_MAP_PAIR(val),
        ARMY_RESOURCE_STATS_LIST
#undef STAT_FUNC
      };
      
      const size_t array_size = sizeof(names) / sizeof(names[0]);
      static_assert(array_size == values::count);
    }
    
    namespace hero_troop_stats {
      const std::string_view names[] = {
#define STAT_FUNC(val) #val,
        HERO_TROOP_STATS_LIST
#undef STAT_FUNC
      };
      
      const phmap::flat_hash_map<std::string_view, values> map = {
#define STAT_FUNC(val) MAKE_MAP_PAIR(val),
        HERO_TROOP_STATS_LIST
#undef STAT_FUNC
      };
      
      const size_t array_size = sizeof(names) / sizeof(names[0]);
      static_assert(array_size == values::count);
    }
    
    namespace offsets {
      const size_t array[] = {
#define STAT_OFFSET_FUNC(val) val,
        STATS_OFFSET_LIST
#undef STAT_OFFSET_FUNC
        count
      };
      
      const size_t array_size = sizeof(array) / sizeof(array[0]);
      static_assert(array_size == stat_type::count);
      
      const std::string_view names[] = {
#define STAT_OFFSET_FUNC(val) #val,
        STATS_OFFSET_LIST
#undef STAT_OFFSET_FUNC
      };
      
      static_assert(sizeof(names) / sizeof(names[0]) == stat_type::count-1);
    }
    
    namespace stat_type {
      const std::string_view names[] = {
#define STAT_TYPES_FUNC(val) #val,
        STAT_TYPES_LIST
#undef STAT_TYPES_FUNC
      };
      
      const phmap::flat_hash_map<std::string_view, values> map = {
#define STAT_TYPES_FUNC(val) MAKE_MAP_PAIR(val),
        STAT_TYPES_LIST
#undef STAT_TYPES_FUNC
      };
      
      const size_t array_size = sizeof(names) / sizeof(names[0]);
      static_assert(array_size == values::count);
    }
    
    namespace opinion_modifiers {
      const std::string_view names[] = {
#define OPINION_MODIFIER_TYPE_FUNC(val) #val,
        OPINION_MODIFIER_TYPES_LIST
#undef OPINION_MODIFIER_TYPE_FUNC
      };
      
      const phmap::flat_hash_map<std::string_view, values> map = {
#define OPINION_MODIFIER_TYPE_FUNC(val) MAKE_MAP_PAIR(val),
        OPINION_MODIFIER_TYPES_LIST
#undef OPINION_MODIFIER_TYPE_FUNC
      };
      
      const size_t array_size = sizeof(names) / sizeof(names[0]);
      static_assert(array_size == values::count);
    }
    
    stat_type::values get_stat_type(const uint32_t &stat) {
      for (size_t i = 1; i < core::stat_type::count; ++i) {
        const size_t offset = core::offsets::array[i-1];
        const size_t count = core::offsets::array[i];
        if (stat >= offset && stat < count) return static_cast<stat_type::values>(i);
      }
      
      return stat_type::invalid;
    }
  }
}
