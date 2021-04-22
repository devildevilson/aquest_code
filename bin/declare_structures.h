#ifndef DECLARE_STRUCTURES_H
#define DECLARE_STRUCTURES_H

#include <cstdint>

namespace devils_engine {
  namespace core {
    struct tile;
    struct province;
    struct building_type;
    struct city_type;
    struct city;
    struct trait;
    struct modificator;
    struct troop_type;
    struct troop;
    struct hero;
    struct decision;
    struct religion_group;
    struct religion;
    struct culture;
    struct law;
    struct right;
    struct event;
    struct titulus;
    struct character;
    struct dynasty;
    struct faction;
    struct hero_troop;
    struct army;
    
    enum class structure : uint32_t {
      tile,
      province,
      building_type,
      city_type,
      city,
      trait,
      modificator,
      troop_type,
//       troop,
//       hero,
      decision,
      religion_group,
      religion,
      culture,
      law,
//       right,
      event,
      titulus,
      
      character,
      dynasty,
      faction,  // создавать отдельно это не нужно
      hero_troop,
      army,
      count,
      
      static_types_count = character,
      parsing_types_count = faction
    };
  }
}

#endif
