#ifndef TRAITS_MODIFIER_ATTRIBS_H
#define TRAITS_MODIFIER_ATTRIBS_H

namespace devils_engine {
  namespace utils {
    namespace trait {
      enum values { // здесь только механики
        is_hero,
        is_agnatic,
        is_enatic,
        cannot_inherit,
        cannot_marry,
        educational,
        is_hidden,
        prevent_death_from_age,
        prevent_death_from_disease,
        prevent_death_from_magic_disease,
        inbred_trait,
        incapacitating,
        is_epidemic,
        is_health,
        is_disease,
        is_magic_disease,
        is_leadership,
        is_childhood,
        is_lifestyle,
        is_personality,
        is_priest,
        is_on_adventure,
        can_get_on_born,
        rebel_inherited,
        is_religious,
        same_trait_visibility,
        hidden_from_others,
        is_vice,
        is_virtue,
        succession_gfx,
        is_symptom,
        //prevent_decadence,
        // тут явно добавятся механики
        count
      };
      
      constexpr const uint32_t bit_container_size = count / UINT32_WIDTH + 1;
    }
    
    namespace modificators {
      enum values {
        is_visible,
        is_major,
        inherit,
        decay,
        revoke_reason,
        prison_reason,
        execute_reason,
        banish_reason,
        preparing_adventure_against_me,
        preparing_to_invade_me,
        preparing_invasion,
        enemy,
        crime,
        disable_non_aggression_pacts,
        non_aggression_pact,
        obedient,
        non_interference,
        count
      };
      
      constexpr const uint32_t bit_container_size = count / UINT32_WIDTH + 1;
    }
  }
}

#endif
