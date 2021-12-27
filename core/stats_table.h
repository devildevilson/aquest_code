#ifndef STAT_TABLE_H
#define STAT_TABLE_H

#include <string_view>
#include "core/stats.h"
#include <parallel_hashmap/phmap.h>

namespace devils_engine {
  namespace core {
    namespace stats_list {
      extern const std::string_view names[];
      extern const phmap::flat_hash_map<std::string_view, values> map;
      
#define STAT_TYPES_FUNC(val) uint32_t to_##val(const values &val);
      STAT_TYPES_LIST
#undef STAT_TYPES_FUNC
    }
    
    namespace character_stats {
      extern const std::string_view names[];
      extern const phmap::flat_hash_map<std::string_view, values> map;
    }
    
    namespace character_resources {
      extern const std::string_view names[];
      extern const phmap::flat_hash_map<std::string_view, values> map;
    }
    
    namespace troop_stats {
      extern const std::string_view names[];
      extern const phmap::flat_hash_map<std::string_view, values> map;
    }
    
    namespace hero_stats {
      extern const std::string_view names[];
      extern const phmap::flat_hash_map<std::string_view, values> map;
    }
    
    namespace realm_stats {
      extern const std::string_view names[];
      extern const phmap::flat_hash_map<std::string_view, values> map;
    }
    
    namespace realm_resources {
      extern const std::string_view names[];
      extern const phmap::flat_hash_map<std::string_view, values> map;
    }
    
    namespace province_stats {
      extern const std::string_view names[];
      extern const phmap::flat_hash_map<std::string_view, values> map;
    }
    
    namespace city_stats {
      extern const std::string_view names[];
      extern const phmap::flat_hash_map<std::string_view, values> map;
    }
    
    namespace city_resources {
      extern const std::string_view names[];
      extern const phmap::flat_hash_map<std::string_view, values> map;
    }
    
    namespace army_stats {
      extern const std::string_view names[];
      extern const phmap::flat_hash_map<std::string_view, values> map;
    }
    
    namespace army_resources {
      extern const std::string_view names[];
      extern const phmap::flat_hash_map<std::string_view, values> map;
    }
    
    namespace hero_troop_stats {
      extern const std::string_view names[];
      extern const phmap::flat_hash_map<std::string_view, values> map;
    }
    
    namespace offsets {
      extern const size_t array[];
      extern const size_t array_size;
      extern const std::string_view names[];
    }
    
    namespace stat_type {
      extern const std::string_view names[];
      extern const phmap::flat_hash_map<std::string_view, values> map;
    }
    
    namespace opinion_modifiers {
      extern const std::string_view names[];
      extern const phmap::flat_hash_map<std::string_view, values> map;
    }
    
    stat_type::values get_stat_type(const uint32_t &stat);
  }
}

#endif
