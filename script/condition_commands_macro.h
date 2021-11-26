#ifndef DEVILS_ENGINE_SCRIPT_CONDITION_COMMANDS_MACRO_H
#define DEVILS_ENGINE_SCRIPT_CONDITION_COMMANDS_MACRO_H

#define CONDITION_COMMANDS_LIST \
  CONDITION_COMMAND_FUNC(is_ai) \
  CONDITION_COMMAND_FUNC(is_player) \
  CONDITION_COMMAND_FUNC(is_dead) \
  CONDITION_COMMAND_FUNC(is_alive) \
  CONDITION_COMMAND_FUNC(is_independent) \
  CONDITION_COMMAND_FUNC(is_vassal) \
  CONDITION_COMMAND_FUNC(is_courtier) \
  CONDITION_COMMAND_FUNC(is_pleb) \
  CONDITION_COMMAND_FUNC(is_noble) \
  CONDITION_COMMAND_FUNC(is_priest) \
  CONDITION_COMMAND_FUNC(is_male) \
  CONDITION_COMMAND_FUNC(is_female) \
  CONDITION_COMMAND_FUNC(is_hero) \
  CONDITION_COMMAND_FUNC(is_prisoner) \
  CONDITION_COMMAND_FUNC(is_married) \
  CONDITION_COMMAND_FUNC(is_sick) \
  CONDITION_COMMAND_FUNC(is_in_war) \
  CONDITION_COMMAND_FUNC(is_in_society) \
  CONDITION_COMMAND_FUNC(is_clan_head) \
  CONDITION_COMMAND_FUNC(is_religious_head) \
  CONDITION_COMMAND_FUNC(is_father_alive) \
  CONDITION_COMMAND_FUNC(is_mother_alive) \
  CONDITION_COMMAND_FUNC(is_elector) \
  CONDITION_COMMAND_FUNC(is_establishment_member) \
  CONDITION_COMMAND_FUNC(is_council_member) \
  CONDITION_COMMAND_FUNC(is_tribunal_member) \
  CONDITION_COMMAND_FUNC(is_assembly_member) \
  CONDITION_COMMAND_FUNC(is_clergy_member) \
  CONDITION_COMMAND_FUNC(is_establishment_elector) \
  CONDITION_COMMAND_FUNC(is_council_elector) \
  CONDITION_COMMAND_FUNC(is_tribunal_elector) \
  CONDITION_COMMAND_FUNC(is_assembly_elector) \
  CONDITION_COMMAND_FUNC(is_clergy_elector) \
  CONDITION_COMMAND_FUNC(is_among_most_powerful_vassals) \
  CONDITION_COMMAND_FUNC(can_change_religion) \
  CONDITION_COMMAND_FUNC(can_call_crusade) \
  CONDITION_COMMAND_FUNC(can_grant_title) \
  CONDITION_COMMAND_FUNC(can_marry) \
  CONDITION_COMMAND_FUNC(has_dynasty) \
  CONDITION_COMMAND_FUNC(has_self_realm) \
  CONDITION_COMMAND_FUNC(has_owner) \
  CONDITION_COMMAND_FUNC(age) \
  
#define CHARACTER_CONDITION_STRING_CHECK_COMMANDS_LIST \
  CONDITION_COMMAND_FUNC(has_culture) \
  CONDITION_COMMAND_FUNC(has_culture_group) \
  CONDITION_COMMAND_FUNC(has_religion) \
  CONDITION_COMMAND_FUNC(has_religion_group) \
  CONDITION_COMMAND_FUNC(has_trait) \
  CONDITION_COMMAND_FUNC(has_modificator) \
  CONDITION_COMMAND_FUNC(has_flag) \
  CONDITION_COMMAND_FUNC(has_title) \
  /*CONDITION_COMMAND_FUNC(has_nickname)*/ \
  
//   CONDITION_COMMAND_FUNC(bit_is_set)
//   CONDITION_COMMAND_FUNC(bit_is_unset)
  
// смена контекста на группу (то есть проверка есть ли хотя бы один кто отвечает на все условия)
// например has_brother = { age = 16 } - есть ли хотя бы один брат возраст которого больше или равно 16
// лучше чтобы эта функция возвращала количество таких братьев + можно добавить процент и максимальное количество
// процент среди всех братьев? возможно нужно добавить общую функцию компаре_валуе
  
#define CONDITION_COMMANDS_REALM_LIST \
  CONDITION_COMMAND_FUNC(is_state) \
  CONDITION_COMMAND_FUNC(is_council) \
  CONDITION_COMMAND_FUNC(is_tribunal) \
  CONDITION_COMMAND_FUNC(is_assembly) \
  CONDITION_COMMAND_FUNC(is_clergy) \
  CONDITION_COMMAND_FUNC(is_law_available) \
  CONDITION_COMMAND_FUNC(has_right) \
  CONDITION_COMMAND_FUNC(has_enacted_law) \
  
// потребуется еще война, соседи, союзники (да и вообще дипломатические отношения), те или иные возможности по управлению, 
  
#endif

