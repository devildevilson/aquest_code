#ifndef DEVILS_ENGINE_SCRIPT_CONDITION_COMMANDS_MACRO_H
#define DEVILS_ENGINE_SCRIPT_CONDITION_COMMANDS_MACRO_H

// has_inactive_trait ??
// has_perk ???

// персонаж
// has_opinion_modifier
// has_owned_scheme
// has_pending_interaction_of_type - висит ли на персонаже неразрешенная интеракция?
// нужно проверить все отношения тип has_relation_best_friend
// в отношениях еще можно поставить какой то флаг, что это? какое то строковое значение
// я так понимаю отношения нужно сделать иначе
// has_revoke_title_reason
// has_strong_claim_on

// реалм
// has_primary_title
// has_raid_immunity_against - у реалма или у персонажа?
// has_raised_armies - у реалма или у персонажа?
// 

#define SCRIPT_VALUE_TYPE_CONSTNESS const
#define SCRIPT_VALUE_TYPE_CONSTLESS

#define CHARACTER_GET_BOOL_NO_ARGS_COMMANDS_LIST \
  CONDITION_COMMAND_FUNC(is_ai) \
  CONDITION_COMMAND_FUNC(is_ai_playable) \
  CONDITION_COMMAND_FUNC(is_player) \
  CONDITION_COMMAND_FUNC(is_dead) \
  CONDITION_COMMAND_FUNC(is_alive) \
  CONDITION_COMMAND_FUNC(is_adult) \
  CONDITION_COMMAND_FUNC(is_bastard) \
  CONDITION_COMMAND_FUNC(is_concubine_child) \
  CONDITION_COMMAND_FUNC(is_independent) \
  CONDITION_COMMAND_FUNC(is_vassal) \
  CONDITION_COMMAND_FUNC(is_courtier) \
  CONDITION_COMMAND_FUNC(is_pleb) \
  CONDITION_COMMAND_FUNC(is_noble) \
  CONDITION_COMMAND_FUNC(is_priest) \
  CONDITION_COMMAND_FUNC(is_male) \
  CONDITION_COMMAND_FUNC(is_female) \
  CONDITION_COMMAND_FUNC(is_hero) \
  CONDITION_COMMAND_FUNC(is_troop_leader) \
  CONDITION_COMMAND_FUNC(is_army_commander) \
  CONDITION_COMMAND_FUNC(is_excommunicated) \
  CONDITION_COMMAND_FUNC(is_general) \
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
  CONDITION_COMMAND_FUNC(has_father) \
  CONDITION_COMMAND_FUNC(has_mother) \
  
#define CHARACTER_GET_NUMBER_NO_ARGS_COMMANDS_LIST \
  CONDITION_COMMAND_FUNC(age) \
  
// титул? по идее это в реалме нужно проверять
#define CHARACTER_GET_BOOL_ONE_ARG_COMMANDS_LIST \
  CONDITION_ARG_COMMAND_FUNC(has_culture,        culture,        SCRIPT_VALUE_TYPE_CONSTNESS, core::culture*)        \
  CONDITION_ARG_COMMAND_FUNC(has_culture_group,  culture_group,  SCRIPT_VALUE_TYPE_CONSTNESS, core::culture_group*)  \
  CONDITION_ARG_COMMAND_FUNC(has_religion,       religion,       SCRIPT_VALUE_TYPE_CONSTNESS, core::religion*)       \
  CONDITION_ARG_COMMAND_FUNC(has_religion_group, religion_group, SCRIPT_VALUE_TYPE_CONSTNESS, core::religion_group*) \
  CONDITION_ARG_COMMAND_FUNC(has_trait,          trait,          SCRIPT_VALUE_TYPE_CONSTNESS, core::trait*)          \
  CONDITION_ARG_COMMAND_FUNC(has_modificator,    modificator,    SCRIPT_VALUE_TYPE_CONSTNESS, core::modificator*)    \
  CONDITION_ARG_COMMAND_FUNC(has_flag,           string,         SCRIPT_VALUE_TYPE_CONSTLESS, std::string_view)      \
  CONDITION_ARG_COMMAND_FUNC(has_same_culture_as,        character, SCRIPT_VALUE_TYPE_CONSTLESS, core::character*)   \
  CONDITION_ARG_COMMAND_FUNC(has_same_culture_group_as,  character, SCRIPT_VALUE_TYPE_CONSTLESS, core::character*)   \
  CONDITION_ARG_COMMAND_FUNC(has_same_religion_as,       character, SCRIPT_VALUE_TYPE_CONSTLESS, core::character*)   \
  CONDITION_ARG_COMMAND_FUNC(has_same_religion_group_as, character, SCRIPT_VALUE_TYPE_CONSTLESS, core::character*)   \
  /* close family */ \
  CONDITION_ARG_COMMAND_FUNC(is_child_of, character, SCRIPT_VALUE_TYPE_CONSTLESS, core::character*) \
  CONDITION_ARG_COMMAND_FUNC(is_parent_of, character, SCRIPT_VALUE_TYPE_CONSTLESS, core::character*) \
  CONDITION_ARG_COMMAND_FUNC(is_sibling_of, character, SCRIPT_VALUE_TYPE_CONSTLESS, core::character*) \
  CONDITION_ARG_COMMAND_FUNC(is_half_sibling_of, character, SCRIPT_VALUE_TYPE_CONSTLESS, core::character*) \
  CONDITION_ARG_COMMAND_FUNC(is_grandparent_of, character, SCRIPT_VALUE_TYPE_CONSTLESS, core::character*) \
  CONDITION_ARG_COMMAND_FUNC(is_grandchild_of, character, SCRIPT_VALUE_TYPE_CONSTLESS, core::character*) \
  CONDITION_ARG_COMMAND_FUNC(is_close_relative_of, character, SCRIPT_VALUE_TYPE_CONSTLESS, core::character*) \
  /* extended family */ \
  CONDITION_ARG_COMMAND_FUNC(is_cousin_of, character, SCRIPT_VALUE_TYPE_CONSTLESS, core::character*) \
  CONDITION_ARG_COMMAND_FUNC(is_nibling_of, character, SCRIPT_VALUE_TYPE_CONSTLESS, core::character*) \
  CONDITION_ARG_COMMAND_FUNC(is_uncle_or_aunt_of, character, SCRIPT_VALUE_TYPE_CONSTLESS, core::character*) \
  CONDITION_ARG_COMMAND_FUNC(is_extended_relative_of, character, SCRIPT_VALUE_TYPE_CONSTLESS, core::character*) \
  CONDITION_ARG_COMMAND_FUNC(is_close_or_extended_relative_of, character, SCRIPT_VALUE_TYPE_CONSTLESS, core::character*) \
  /* blood relative */ \
  CONDITION_ARG_COMMAND_FUNC(is_blood_relative_of, character, SCRIPT_VALUE_TYPE_CONSTLESS, core::character*) \
  /* other relative */ \
  CONDITION_ARG_COMMAND_FUNC(is_relative_of, character, SCRIPT_VALUE_TYPE_CONSTLESS, core::character*) \
  CONDITION_ARG_COMMAND_FUNC(is_owner_of, character, SCRIPT_VALUE_TYPE_CONSTLESS, core::character*) \
  CONDITION_ARG_COMMAND_FUNC(is_concubine_of, character, SCRIPT_VALUE_TYPE_CONSTLESS, core::character*) \
  
// смена контекста на группу (то есть проверка есть ли хотя бы один кто отвечает на все условия)
// например has_brother = { age = 16 } - есть ли хотя бы один брат возраст которого больше или равно 16
// лучше чтобы эта функция возвращала количество таких братьев + можно добавить процент и максимальное количество
// процент среди всех братьев? возможно нужно добавить общую функцию компаре_валуе
  
// в реалме не так много кондишенов, тут в основном будут переходы и 
#define REALM_GET_BOOL_NO_ARGS_COMMANDS_LIST \
  CONDITION_COMMAND_FUNC(is_state) \
  CONDITION_COMMAND_FUNC(is_council) \
  CONDITION_COMMAND_FUNC(is_tribunal) \
  CONDITION_COMMAND_FUNC(is_assembly) \
  CONDITION_COMMAND_FUNC(is_clergy) \
  CONDITION_COMMAND_FUNC(is_state_independent_power) \
  CONDITION_COMMAND_FUNC(is_independent_realm) \
  CONDITION_COMMAND_FUNC(is_self) \
  CONDITION_COMMAND_FUNC(has_council) \
  CONDITION_COMMAND_FUNC(has_tribunal) \
  CONDITION_COMMAND_FUNC(has_assembly) \
  CONDITION_COMMAND_FUNC(has_clergy) \
  CONDITION_COMMAND_FUNC(has_capital) \
  CONDITION_COMMAND_FUNC(has_titles) \
  
#define REALM_GET_BOOL_ONE_ARG_COMMANDS_LIST \
  CONDITION_ARG_COMMAND_FUNC(has_enacted_law,  law,           SCRIPT_VALUE_TYPE_CONSTNESS, core::law*) \
  CONDITION_ARG_COMMAND_FUNC(is_law_available, law,           SCRIPT_VALUE_TYPE_CONSTNESS, core::law*) \


//CONDITION_ARG_COMMAND_FUNC(is_in_war_with,  character,           SCRIPT_VALUE_TYPE_CONSTLESS, core::character*)
  
#define REALM_GET_BOOL_EXISTED_ARG_COMMANDS_LIST \
  CONDITION_ARG_COMMAND_FUNC(has_right,       unused, unused, size_t) \
  CONDITION_ARG_COMMAND_FUNC(has_state_right, unused, unused, size_t) \
  CONDITION_ARG_COMMAND_FUNC(has_enacted_law_with_flag, unused, unused, size_t) \

#define PROVINCE_GET_BOOL_ONE_ARG_COMMANDS_LIST \
  /*CONDITION_ARG_COMMAND_FUNC(has_city,  city, SCRIPT_VALUE_TYPE_CONSTLESS, core::city*)*/ \
  CONDITION_ARG_COMMAND_FUNC(has_city_with_type,  city_type, SCRIPT_VALUE_TYPE_CONSTNESS, core::city_type*) \
  CONDITION_ARG_COMMAND_FUNC(has_city_with_holding_type,  holding_type, SCRIPT_VALUE_TYPE_CONSTNESS, core::holding_type*) \

#define RELIGION_GET_BOOL_ONE_ARG_COMMANDS_LIST \
  CONDITION_ARG_COMMAND_FUNC(allow_mariages_with, religion, SCRIPT_VALUE_TYPE_CONSTLESS, core::religion*) /* должны быть какие то строгие ограничения */ \
  
#define RELIGION_GET_BOOL_EXISTED_ARG_COMMANDS_LIST \
  CONDITION_ARG_COMMAND_FUNC(has_religion_feature, unused, unused, size_t) \
  
#define CULTURE_GET_BOOL_EXISTED_ARG_COMMANDS_LIST \
  CONDITION_ARG_COMMAND_FUNC(has_culture_feature, unused, unused, size_t) \
  
#define TITLE_GET_BOOL_NO_ARGS_COMMANDS_LIST \
  CONDITION_COMMAND_FUNC(is_contested) \
  CONDITION_COMMAND_FUNC(is_created) \
  CONDITION_COMMAND_FUNC(will_leave_realm_on_succession) \
  CONDITION_COMMAND_FUNC(is_formal) \
  
// религия: священное место, священное место с флагом, fervor, хостилити, фичи, доктрина?, тэг и треит
// культура: в цк3 только инновации, эра и графика
// титул: leased_out, создавать/входить во фракции, контроль, отношение, дрифт, девелопмент, священное место, тип наследования у титула (у меня не будет),
// закон титула (скорее всего не будет), неверный тип владений, с чем соединена, контестед (является целью войны), проверка на то является ли деюре хозяином,
// священный орден, создан ли титул, покинет ли титул реалм просле наследования
// армия: в движении, размеры, в битве, рейдит, осаждает, 
// еще функции для священной войны, для секретов, для активити (?)
  
// потребуется еще война, соседи, союзники (да и вообще дипломатические отношения), те или иные возможности по управлению, 
  
#endif

