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
    struct interaction;
    struct religion_group;
    struct religion;
    struct culture;
    struct law;
    struct right;
    struct event;
    struct titulus;
    struct character;
    struct dynasty;
    struct realm;
    struct hero_troop;
    struct army;
    struct casus_belli;
    struct war;
    
    enum class structure : uint32_t {
      tile,
      province,
      city,
      building_type,
      city_type,
      trait,
      modificator,
      troop_type,
//       troop,
//       hero,
      decision,
      interaction,
      religion_group,
      religion,
      culture,
      law,
//       right,
      event,
      titulus,
      casus_belli,
      
      character,
      dynasty,
      realm,  // создавать отдельно это не нужно
      hero_troop,
      army,
      war,
      troop,
      count,
      
      static_types_count = character,
      parsing_types_count = realm
    };
    
    enum class id_struct {
      building_type,
      city_type,
      trait,
      modificator,
      troop_type,
      decision,
      interaction,
      religion_group,
      religion,
      culture,
      law,
      event,
      titulus,
      casus_belli,
      count
    };
  }
}

#endif
