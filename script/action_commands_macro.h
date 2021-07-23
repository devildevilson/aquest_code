#ifndef ACTION_COMMANDS_MACRO_H
#define ACTION_COMMANDS_MACRO_H

// пока что добавил функции для персонажа, 
// попытался немного по категориям их раскидать
// сначала идут "легкие" функции персонажа
// затем секреты и "крючки", затем треиты, модификаторы, клеймы, договор о ненападении, 
// фракции (нужно изменить название государства игрока (realm)), статусы отношений и остальное
// статы прибавляются к базовым? если не к базовым то после пересчета они отвалятся
// с экшонами все довольно сложно, тут много вещей которые появятся с развитием игры,
// но уже сейчас явно можно сделать модификацию статов

#define ACTION_COMMANDS_LIST \
  ACTION_COMMAND_FUNC(add_flag) \
  ACTION_COMMAND_FUNC(marry) \
  \
  ACTION_COMMAND_FUNC(add_hook) \
  ACTION_COMMAND_FUNC(add_trait) \
  ACTION_COMMAND_FUNC(start_war) \
  \
  ACTION_COMMAND_FUNC(value) \
  ACTION_COMMAND_FUNC(add) \
  ACTION_COMMAND_FUNC(factor) \
  ACTION_COMMAND_FUNC(string) \
  ACTION_COMMAND_FUNC(object) \
  ACTION_COMMAND_FUNC(save_as)
  
  // как сделать сохранение в контекст? во первых может ли быть сохранение в контекст в кондишоне?
  // хороший вопрос, вообще было бы неплохо еще какие то глобальные вещи хранить, но это уже за гранью
  // у функций которые создают объект должен быть 100% способ сразу засунуть их в память
  // не помню как там было в цк, но определенно нужно сделать сохранение в экшонах
  
  // перки, у меня наверное это дело будет по другому
//   ACTION_COMMAND_FUNC(add_diplomacy_lifestyle_perk_points)
//   ACTION_COMMAND_FUNC(add_diplomacy_lifestyle_xp)
//   ACTION_COMMAND_FUNC(add_perk)
//   ACTION_COMMAND_FUNC(refund_all_perks) 
//   ACTION_COMMAND_FUNC(refund_perks) 
  
// war scope:
// add_attacker
// add_defender
// end_war
// remove_participant
// set_called_to
// set_casus_belli
// скриптовые блоки: эти относятся к экшонам, нужно добавить еще кондишоны
// every_war_attacker
// every_war_defender
// every_war_participant
// random_war_attacker
// random_war_defender
// random_war_participant
  
// story cycle scope: (не понимаю что это)
// end_story
// make_story_owner
  
// secret scope:
// add_secret_participant
// disable_exposure_by
// expose_secret
// remove_secret
// reveal_to
// set_secret_owner
// spend_by
// every_secret_knower
// every_secret_participant
// random_secret_knower
// random_secret_participant
  
// scheme scope:
// add_scheme_modifier
// add_scheme_progress
// end_scheme
// every_scheme_agent
// expose_scheme
// expose_scheme_agent
// remove_scheme_modifier
// scheme_freeze_days
// random_scheme_agent
  
// religion scope: (возможно потом когда нибудь я сделаю последовательный обход сущностей в скрипте)
// every_faith
// random_faith
  
// province scope: (провинцией в цк3 называется область с городом)
// add_building
// add_building_slot
// add_province_modifier
// add_special_building
// add_special_building_slot
// refill_garrison
// refill_levy
// remove_all_province_modifier_instances
// remove_province_modifier
// set_holding_type
// spawn_activity
  
// landed title scope: (это в принципе титул, в зависимости от уровня титула это может быть провинцией или королевством)
// add_county_modifier
// change_county_control
// change_de_jure_drift_progress
// change_development_level
// change_development_progress
// change_development_progress_with_overflow
// clear_title_laws
// clear_title_laws_effects
// copy_title_history
// lease_out_to
// reset_title_name
// reset_title_prefix
// revoke_lease
// set_always_follows_primary_heir
// set_capital_county
// set_color_from_title
// set_county_culture
// set_county_faith
// set_de_jure_liege_title
// set_definitive_form
// set_delete_on_destroy
// set_destroy_if_invalid_heir
// set_destroy_on_succession
// set_landless_title
// set_no_automatic_claims
// set_title_name
// set_title_prefix
// title_create_faction
// title_join_faction
// title_leave_faction
// every_claimant
// every_connected_county
// every_county_province
// every_de_jure_county_holder
// every_de_jure_top_liege
// every_dejure_vassal_title_holder
// every_election_candidate
// every_elector
// every_in_de_facto_hierarchy
// every_in_de_jure_hierarchy
// every_neighboring_county
// every_this_title_or_de_jure_above
// every_title_heir
// every_title_joined_faction
// every_title_to_title_neighboring_and_across_water_barony
// every_title_to_title_neighboring_and_across_water_county
// every_title_to_title_neighboring_and_across_water_duchy
// every_title_to_title_neighboring_and_across_water_empire
// every_title_to_title_neighboring_and_across_water_kingdom
// every_title_to_title_neighboring_barony
// every_title_to_title_neighboring_county
// every_title_to_title_neighboring_duchy
// every_title_to_title_neighboring_empire
// every_title_to_title_neighboring_kingdom
// random_claimant
// random_connected_county
// random_county_province
// random_de_jure_county_holder
// random_de_jure_top_liege
// random_dejure_vassal_title_holder
// random_election_candidate
// random_elector
// random_in_de_facto_hierarchy
// random_in_de_jure_hierarchy
// random_neighboring_county
// random_this_title_or_de_jure_above
// random_title_heir
// random_title_joined_faction
// random_title_to_title_neighboring_and_across_water_barony
// random_title_to_title_neighboring_and_across_water_county
// random_title_to_title_neighboring_and_across_water_duchy
// random_title_to_title_neighboring_and_across_water_empire
// random_title_to_title_neighboring_and_across_water_kingdom
// random_title_to_title_neighboring_barony
// random_title_to_title_neighboring_county
// random_title_to_title_neighboring_duchy
// random_title_to_title_neighboring_empire
// random_title_to_title_neighboring_kingdom
  
// holy order scope:
// every_leased_title
// random_leased_title
  
// dynasty scope:
// add_dynasty_modifier
// add_dynasty_perk
// add_dynasty_prestige
// add_dynasty_prestige_level
// remove_all_dynasty_modifier_instances
// remove_dynasty_modifier
// every_dynasty_member
// random_dynasty_member
  
// dynasty house scope:
// add_house_modifier
// remove_all_house_modifier_instances
// remove_house_modifier
// every_house_member
// random_house_member
  
// culture scope:
// add_innovation
// add_random_innovation
// get_all_innovations_from
// get_random_innovation_from

// дальше идет скоуп персонажа, но он очень большой
  
// faction scope: (это скоуп восстания (может быть фракция не восстанием?))
// add_faction_discontent
// destroy_faction
// faction_remove_war
// faction_start_war
// remove_special_character
// remove_special_title
// set_special_character
// set_special_title
// every_faction_county_member
// every_faction_member
// random_faction_county_member
// random_faction_member
  
// faith scope: (в цк3 религия разделена на верования)
// activate_holy_site
// add_doctrine
// change_fervor
// remove_doctrine
// remove_religious_head_title
// set_religious_head_title
// start_great_holy_war
// every_defensive_great_holy_wars
// every_faith_holy_order
// every_holy_site
// random_defensive_great_holy_wars
// random_faith_holy_order
// random_holy_site
  
// activity scope: (что такое активити? это пирушка что ли?)
// accept_invitation_for_character
// decline_invitation_for_character
// invite_character_to_activity
// every_activity_declined
// every_activity_invited
// every_participant
// random_activity_declined
// random_activity_invited
// random_participant
  
// army scope: (скоуп армий у меня будет гораздо больше)
// assign_commander
// remove_commander
  
// great holy war scope:
// change_war_chest_gold
// change_war_chest_piety
// change_war_chest_prestige
// divide_war_chest
// do_ghw_title_handout
// pledge_attacker
// pledge_defender
// reset_designated_winner
// set_designated_winner
// set_great_holy_war_target
// set_war_declarer
// start_ghw_war
// unpledge_attacker
// unpledge_defender
// every_pledged_attacker
// every_pledged_defender
// random_pledged_attacker
// random_pledged_defender
  
// combat side scope:
// battle_event
// lose_combat
// win_combat
// every_side_commander
// every_side_knight
// random_side_commander
// random_side_knight
  
// casus belli scope:
// add_from_contribution_attackers
// add_from_contribution_defenders
// every_target_title
// random_target_title
 
// также вики выделяет общие функции
// там работа со скоупом, создание персонажей, утилити, обход вообще всех сущностей определенного типа

// в цк3 перечисленны такие скоупы: war, story cycle (?), secret, scheme, religion, province, landed title, holy order, 
// dynasty, dynasty house, culture, character, faction (восстание), faith, activity, army, grat holy war, combat side, casus belli
  
// многие из этих скоупов у меня будут совпадать, а некоторые добавятся
// возможно пока что просто передрать список из цк

#endif
