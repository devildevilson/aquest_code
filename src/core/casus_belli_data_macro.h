#ifndef DEVILS_ENGINE_CORE_CASUS_BELLI_DATA_MACRO_H
#define DEVILS_ENGINE_CORE_CASUS_BELLI_DATA_MACRO_H

#define CASUS_BELLI_FLAGS_LIST \
  CASUS_BELLI_FLAG_FUNC(is_permanent) \
  CASUS_BELLI_FLAG_FUNC(is_independence) \
  CASUS_BELLI_FLAG_FUNC(is_claim_all) \
  CASUS_BELLI_FLAG_FUNC(is_revolt_cb) \
  CASUS_BELLI_FLAG_FUNC(is_holy_war) \
  CASUS_BELLI_FLAG_FUNC(is_tyranny_cb) \
  CASUS_BELLI_FLAG_FUNC(hostile_against_others) \
  CASUS_BELLI_FLAG_FUNC(full_hostility) \
  CASUS_BELLI_FLAG_FUNC(can_ask_to_join_war) \
  CASUS_BELLI_FLAG_FUNC(reflect_titleholder_change) \
  CASUS_BELLI_FLAG_FUNC(major_revolt) \
  CASUS_BELLI_FLAG_FUNC(press_claim) \
  CASUS_BELLI_FLAG_FUNC(allow_whitepeace) \
  CASUS_BELLI_FLAG_FUNC(allowed_to_target_suzerains) \
  CASUS_BELLI_FLAG_FUNC(allowed_to_target_tributaries) \
  CASUS_BELLI_FLAG_FUNC(can_call_allies) \
  CASUS_BELLI_FLAG_FUNC(attacker_can_call_allies) \
  CASUS_BELLI_FLAG_FUNC(defender_can_call_allies) \
  CASUS_BELLI_FLAG_FUNC(can_call_vassals) \
  CASUS_BELLI_FLAG_FUNC(can_attack_vassals) \
  CASUS_BELLI_FLAG_FUNC(attacker_alliance_occ_warscore) \
  CASUS_BELLI_FLAG_FUNC(check_dejure_duchies) \
  CASUS_BELLI_FLAG_FUNC(check_all_titles) \
  CASUS_BELLI_FLAG_FUNC(check_all_trade_posts) \
  CASUS_BELLI_FLAG_FUNC(check_actor_direct_vassals) \
  CASUS_BELLI_FLAG_FUNC(other_de_jure_claim) \
  CASUS_BELLI_FLAG_FUNC(apply_long_occ_mod) \
  CASUS_BELLI_FLAG_FUNC(apply_short_occ_mod) \
  CASUS_BELLI_FLAG_FUNC(allow_distant) \
  CASUS_BELLI_FLAG_FUNC(attacker_rel_head_is_ally) \
  CASUS_BELLI_FLAG_FUNC(display_on_map) \
  CASUS_BELLI_FLAG_FUNC(coalition_threat) \
  CASUS_BELLI_FLAG_FUNC(hostages_block_cb) \
  CASUS_BELLI_FLAG_FUNC(attacker_unoccupied_warscore) \
  CASUS_BELLI_FLAG_FUNC(defender_unoccupied_warscore) \
  CASUS_BELLI_FLAG_FUNC(capturing_attacker_is_complete_victory) \
  CASUS_BELLI_FLAG_FUNC(capturing_defender_is_complete_victory) \

#define CASUS_BELLI_NUMBERS_LIST \
  CASUS_BELLI_NUMBER_FUNC(battle_warscore_mult) \
  CASUS_BELLI_NUMBER_FUNC(infamy_modifier) \
  CASUS_BELLI_NUMBER_FUNC(ticking_war_score_multiplier) \
  CASUS_BELLI_NUMBER_FUNC(att_ticking_war_score_multiplier) \
  CASUS_BELLI_NUMBER_FUNC(def_ticking_war_score_multiplier) \
  CASUS_BELLI_NUMBER_FUNC(max_defender_occupation_score) \
  CASUS_BELLI_NUMBER_FUNC(max_attacker_occupation_score) \
  CASUS_BELLI_NUMBER_FUNC(max_defender_battle_score) \
  CASUS_BELLI_NUMBER_FUNC(max_attacker_battle_score) \

#endif
