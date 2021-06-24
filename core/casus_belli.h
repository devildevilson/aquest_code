#ifndef CASUS_BELLI_H
#define CASUS_BELLI_H

#include <string>
#include <cstdint>
#include <cstddef>
#include "script/script_header.h"

namespace devils_engine {
  namespace core {
    struct casus_belli {
      enum flags {
        is_permanent,
        is_independence,
        hostile_against_others,
        full_hostility,
        can_ask_to_join_war,
        reflect_titleholder_change,
        is_claim_all,
        is_revolt_cb,
        major_revolt,
        press_claim,
        allow_whitepeace,
        allowed_to_target_suzerains,
        allowed_to_target_tributaries,
        can_call_allies,
        attacker_can_call_allies,
        defender_can_call_allies,
        can_call_vassals,
        can_attack_vassals,
        attacker_alliance_occ_warscore,
        check_dejure_duchies,
        check_all_titles,
        check_all_trade_posts,
        check_actor_direct_vassals,
        other_de_jure_claim,
        is_holy_war,
        apply_long_occ_mod,
        apply_short_occ_mod,
        allow_distant,
        attacker_rel_head_is_ally,
        display_on_map,
        coalition_threat,
        is_tyranny_cb,
        hostages_block_cb,
        attacker_unoccupied_warscore,
        defender_unoccupied_warscore,
        capturing_attacker_is_complete_victory,
        capturing_defender_is_complete_victory,
        count
      };
      
      std::string id;
      std::string name_id;
      std::string war_name_id;
      float authority_cost;
      float esteem_cost;
      float influence_cost;
      float money_cost;
      float battle_warscore_mult;
      float infamy_modifier;
      float ticking_war_score_multiplier;
      float att_ticking_war_score_multiplier;
      float def_ticking_war_score_multiplier;
      float max_defender_occupation_score;
      float max_attacker_occupation_score;
      float max_defender_battle_score;
      float max_attacker_battle_score;
      size_t truce_turns;
      // картиночка
      // diplo_view_region - меняет провинции котолрые подсвечиваются при выборе этого цб
      // check_de_jure_tier - сканирует (?) все де юре королевства на предмет владений у вассалов?
      
      // в казус белли у нас добавится куча скриптов
      // can_use (разные скоупы), is_valid (разные скоупы), ai_will_do, on_add, on_add_post, 
      // on_success, on_success_post, on_fail, on_fail_post, on_reverse_demand, on_reverse_demand_post, 
      // on_attacker_leader_death, on_defender_leader_death, on_thirdparty_death, on_invalidation
      script::script_data can_use; // условия для того чтобы персонаж мог воспользоваться этим казус белли
    };
  }
}

#endif
