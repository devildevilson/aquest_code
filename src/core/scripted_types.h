#ifndef DEVILS_ENGINE_CORE_SCRIPTED_TYPES_H
#define DEVILS_ENGINE_CORE_SCRIPTED_TYPES_H

#include "declare_structures.h"
#include "utils/handle.h"

#define SCRIPTED_TYPES_LIST \
  SCRIPTED_TYPE_FUNC(character_t,      core::character*)            \
  SCRIPTED_TYPE_FUNC(dynasty_t,        core::dynasty*)              \
  SCRIPTED_TYPE_FUNC(culture_t,        core::culture*)              \
  SCRIPTED_TYPE_FUNC(religion_t,       core::religion*)             \
  SCRIPTED_TYPE_FUNC(titulus_t,        core::titulus*)              \
  SCRIPTED_TYPE_FUNC(province_t,       core::province*)             \
  SCRIPTED_TYPE_FUNC(city_t,           core::city*)                 \
  SCRIPTED_TYPE_FUNC(culture_group_t,  const core::culture_group*)  \
  SCRIPTED_TYPE_FUNC(religion_group_t, const core::religion_group*) \
  SCRIPTED_TYPE_FUNC(city_type_t,      const core::city_type*)      \
  SCRIPTED_TYPE_FUNC(holding_type_t,   const core::holding_type*)   \
  SCRIPTED_TYPE_FUNC(building_type_t,  const core::building_type*)  \
  SCRIPTED_TYPE_FUNC(casus_belli_t,    const core::casus_belli*)    \
  SCRIPTED_TYPE_FUNC(troop_type_t,     const core::troop_type*)     \
  SCRIPTED_TYPE_FUNC(trait_t,          const core::trait*)          \
  SCRIPTED_TYPE_FUNC(modificator_t,    const core::modificator*)    \
  SCRIPTED_TYPE_FUNC(law_t,            const core::law*)            \
  SCRIPTED_TYPE_FUNC(realm_t,          utils::handle<core::realm>)  \
  SCRIPTED_TYPE_FUNC(war_t,            utils::handle<core::war>)    \
  SCRIPTED_TYPE_FUNC(army_t,           utils::handle<core::army>)   \
  SCRIPTED_TYPE_FUNC(hero_troop_t,     utils::handle<core::hero_troop>) \
  SCRIPTED_TYPE_FUNC(troop_t,          utils::handle<core::troop>)  \
  

namespace devils_engine {
  namespace script {
#define SCRIPTED_TYPE_FUNC(name, type) using name = type;
    SCRIPTED_TYPES_LIST
#undef SCRIPTED_TYPE_FUNC
  }
}

#endif
