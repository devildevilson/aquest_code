#ifndef DEVILS_ENGINE_CORE_TRAITS_MODIFIER_ATTRIBS_H
#define DEVILS_ENGINE_CORE_TRAITS_MODIFIER_ATTRIBS_H

#include <cstdint>
#include <string_view>
#include "parallel_hashmap/phmap.h"
#include "utils/constexpr_funcs.h"

#define TRAIT_ATTRIBUTES_LIST \
  TRAIT_ATTRIBUTE_FUNC(is_hero) \
  TRAIT_ATTRIBUTE_FUNC(is_bastard) \
  TRAIT_ATTRIBUTE_FUNC(is_concubine_child) \
  TRAIT_ATTRIBUTE_FUNC(is_agnatic) \
  TRAIT_ATTRIBUTE_FUNC(is_enatic) \
  TRAIT_ATTRIBUTE_FUNC(is_hidden) \
  TRAIT_ATTRIBUTE_FUNC(is_epidemic) \
  TRAIT_ATTRIBUTE_FUNC(is_health) \
  TRAIT_ATTRIBUTE_FUNC(is_disease) \
  TRAIT_ATTRIBUTE_FUNC(is_magic_disease) \
  TRAIT_ATTRIBUTE_FUNC(is_leadership) \
  TRAIT_ATTRIBUTE_FUNC(is_childhood) \
  TRAIT_ATTRIBUTE_FUNC(is_lifestyle) \
  TRAIT_ATTRIBUTE_FUNC(is_personality) \
  TRAIT_ATTRIBUTE_FUNC(is_priest) \
  TRAIT_ATTRIBUTE_FUNC(is_on_adventure) \
  TRAIT_ATTRIBUTE_FUNC(is_religious) \
  TRAIT_ATTRIBUTE_FUNC(is_vice) \
  TRAIT_ATTRIBUTE_FUNC(is_virtue) \
  TRAIT_ATTRIBUTE_FUNC(is_symptom) \
  TRAIT_ATTRIBUTE_FUNC(prevent_death_from_age) \
  TRAIT_ATTRIBUTE_FUNC(prevent_death_from_disease) \
  TRAIT_ATTRIBUTE_FUNC(prevent_death_from_magic_disease) \
  TRAIT_ATTRIBUTE_FUNC(inbred_trait) \
  TRAIT_ATTRIBUTE_FUNC(incapacitating) \
  TRAIT_ATTRIBUTE_FUNC(cannot_inherit) \
  TRAIT_ATTRIBUTE_FUNC(cannot_marry) \
  TRAIT_ATTRIBUTE_FUNC(educational) \
  TRAIT_ATTRIBUTE_FUNC(can_get_on_born) \
  TRAIT_ATTRIBUTE_FUNC(rebel_inherited) \
  TRAIT_ATTRIBUTE_FUNC(same_trait_visibility) \
  TRAIT_ATTRIBUTE_FUNC(hidden_from_others) \
  TRAIT_ATTRIBUTE_FUNC(strong) \
  TRAIT_ATTRIBUTE_FUNC(weak) \
  TRAIT_ATTRIBUTE_FUNC(succession_gfx)

//strong  // не заменяется треит при добавлении коллизии
//weak    // треит заменяется в любом случае при добавлении коллизии
//prevent_decadence
// тут явно добавятся механики
  
#define MODIFICATOR_ATTRIBUTES_LIST \
  MODIFICATOR_ATTRIBUTE_FUNC(is_visible) \
  MODIFICATOR_ATTRIBUTE_FUNC(is_major) \
  MODIFICATOR_ATTRIBUTE_FUNC(inherit) \
  MODIFICATOR_ATTRIBUTE_FUNC(decay) \
  MODIFICATOR_ATTRIBUTE_FUNC(revoke_reason) \
  MODIFICATOR_ATTRIBUTE_FUNC(prison_reason) \
  MODIFICATOR_ATTRIBUTE_FUNC(execute_reason) \
  MODIFICATOR_ATTRIBUTE_FUNC(banish_reason) \
  MODIFICATOR_ATTRIBUTE_FUNC(preparing_adventure_against_me) \
  MODIFICATOR_ATTRIBUTE_FUNC(preparing_to_invade_me) \
  MODIFICATOR_ATTRIBUTE_FUNC(preparing_invasion) \
  MODIFICATOR_ATTRIBUTE_FUNC(enemy) \
  MODIFICATOR_ATTRIBUTE_FUNC(crime) \
  MODIFICATOR_ATTRIBUTE_FUNC(disable_non_aggression_pacts) \
  MODIFICATOR_ATTRIBUTE_FUNC(non_aggression_pact) \
  MODIFICATOR_ATTRIBUTE_FUNC(obedient) \
  MODIFICATOR_ATTRIBUTE_FUNC(non_interference)
  
// имеет ли смысл скинуть все характеристики в одну кучу?
namespace devils_engine {
  namespace core {
    namespace trait_attributes {
      enum values { // здесь только механики
#define TRAIT_ATTRIBUTE_FUNC(val) val,
        TRAIT_ATTRIBUTES_LIST
#undef TRAIT_ATTRIBUTE_FUNC
        
        count
      };
      
      extern const std::string_view names[];
      extern const phmap::flat_hash_map<std::string_view, values> map;
    }
    
    namespace modificator_attributes {
      enum values {
#define MODIFICATOR_ATTRIBUTE_FUNC(val) val,
        MODIFICATOR_ATTRIBUTES_LIST
#undef MODIFICATOR_ATTRIBUTE_FUNC
        
        count
      };
      
      extern const std::string_view names[];
      extern const phmap::flat_hash_map<std::string_view, values> map;
    }
  }
}

#endif
