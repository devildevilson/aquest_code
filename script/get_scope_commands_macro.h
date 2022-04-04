#ifndef DEVILS_ENGINE_SCRIPT_GET_SCOPE_COMMANDS_MACRO_H
#define DEVILS_ENGINE_SCRIPT_GET_SCOPE_COMMANDS_MACRO_H

#define INPUT_CHARACTER script::object::type_bit::character
#define INPUT_REALM script::object::type_bit::realm
#define INPUT_PROVINCE script::object::type_bit::province
#define INPUT_CULTURE script::object::type_bit::culture
#define INPUT_RELIGION script::object::type_bit::religion
#define INPUT_TITLE script::object::type_bit::titulus
#define INPUT_CITY script::object::type_bit::city
#define INPUT_ARMY script::object::type_bit::army
#define INPUT_TRAIT script::object::type_bit::trait
#define INPUT_WAR script::object::type_bit::war
#define INPUT_CASUS_BELLI script::object::type_bit::casus_belli
#define INPUT_VALID_NUMBER script::object::type_bit::valid_number
#define INPUT_NOTHING script::object::type_bit::invalid

#define OUTPUT_CHARACTER script::object::type_bit::character
#define OUTPUT_REALM script::object::type_bit::realm
#define OUTPUT_CULTURE script::object::type_bit::culture
#define OUTPUT_CULTURE_GROUP script::object::type_bit::culture_group
#define OUTPUT_RELIGION script::object::type_bit::religion
#define OUTPUT_RELIGION_GROUP script::object::type_bit::religion_group
#define OUTPUT_DYNASTY script::object::type_bit::dynasty
#define OUTPUT_ARMY script::object::type_bit::army
#define OUTPUT_HERO_TROOP script::object::type_bit::hero_troop
#define OUTPUT_CITY script::object::type_bit::city
#define OUTPUT_CITY_TYPE script::object::type_bit::city_type
#define OUTPUT_TITLE script::object::type_bit::titulus
#define OUTPUT_TRAIT script::object::type_bit::trait
#define OUTPUT_PROVINCE script::object::type_bit::province
#define OUTPUT_CASUS_BELLI script::object::type_bit::casus_belli
#define OUTPUT_NUMBER script::object::type_bit::number
#define OUTPUT_BOOL script::object::type_bit::boolean
#define OUTPUT_NOTHING script::object::type_bit::invalid // или 0? ну функции действительно что то возвращают

#define OUTPUT_CHARACTER_TYPE core::character*
#define OUTPUT_REALM_TYPE utils::handle<core::realm>
#define OUTPUT_CULTURE_TYPE core::culture*
#define OUTPUT_CULTURE_GROUP_TYPE const core::culture_group*
#define OUTPUT_RELIGION_TYPE core::religion*
#define OUTPUT_RELIGION_GROUP_TYPE const core::religion_group*
#define OUTPUT_DYNASTY_TYPE core::dynasty*
#define OUTPUT_ARMY_TYPE utils::handle<core::army>
#define OUTPUT_HERO_TROOP_TYPE utils::handle<core::hero_troop>
#define OUTPUT_CITY_TYPE2 core::city*
#define OUTPUT_CITY_TYPE_TYPE const core::city_type*
#define OUTPUT_TITLE_TYPE core::titulus*
#define OUTPUT_TRAIT_TYPE core::trait*
#define OUTPUT_PROVINCE_TYPE core::province*
#define OUTPUT_CASUS_BELLI_TYPE const core::casus_belli*

// нужно ли эти функции засовывать в соответствующий тип? к сожалению в луа эти вещи выглядят немного иначе
// хотя может и не иначе

// от персонажа мы можем получить: собственный реалм, отца, мать, жену, (реалмы текущего государства), сюзерена, тюремщика, 
// то что пока не вошло:
// host 		character
// matchmaker 		character
// knight_army 		army
// ghw_beneficiary 		character
// primary_heir 		character
// primary_partner 		character
// real_father 		character
// realm_priest 		character
// council_task 		council task ???
// house 		dynasty house ????
// joined_faction 		faction
// location 	Usable in character, army and combat scopes. 	province

// мне хочется чтобы система "продолжения игры" была попроще, то есть player_heir и designated_heir по идее должны быть одним и тем же
//GET_SCOPE_COMMAND_FUNC(liege_or_suzerain, INPUT_CHARACTER, OUTPUT_CHARACTER) /* не могу в текущем виде к сожалению это совместить */

#define CHARACTER_GET_SCOPE_COMMANDS_LIST \
  GET_SCOPE_COMMAND_FUNC(killer, INPUT_CHARACTER, OUTPUT_CHARACTER, OUTPUT_CHARACTER_TYPE) \
  GET_SCOPE_COMMAND_FUNC(employer, INPUT_CHARACTER, OUTPUT_CHARACTER, OUTPUT_CHARACTER_TYPE) /* тут поди наниматель государство? */ \
  GET_SCOPE_COMMAND_FUNC(concubinist, INPUT_CHARACTER, OUTPUT_CHARACTER, OUTPUT_CHARACTER_TYPE) /* предыдущее имя функции owner */ \
  GET_SCOPE_COMMAND_FUNC(primary_consort, INPUT_CHARACTER, OUTPUT_CHARACTER, OUTPUT_CHARACTER_TYPE) \
  GET_SCOPE_COMMAND_FUNC(betrothed, INPUT_CHARACTER, OUTPUT_CHARACTER, OUTPUT_CHARACTER_TYPE) \
  GET_SCOPE_COMMAND_FUNC(mother, INPUT_CHARACTER, OUTPUT_CHARACTER, OUTPUT_CHARACTER_TYPE) \
  GET_SCOPE_COMMAND_FUNC(real_mother, INPUT_CHARACTER, OUTPUT_CHARACTER, OUTPUT_CHARACTER_TYPE) \
  GET_SCOPE_COMMAND_FUNC(father, INPUT_CHARACTER, OUTPUT_CHARACTER, OUTPUT_CHARACTER_TYPE) \
  GET_SCOPE_COMMAND_FUNC(pregnancy_assumed_father, INPUT_CHARACTER, OUTPUT_CHARACTER, OUTPUT_CHARACTER_TYPE) \
  GET_SCOPE_COMMAND_FUNC(pregnancy_real_father, INPUT_CHARACTER, OUTPUT_CHARACTER, OUTPUT_CHARACTER_TYPE) \
  GET_SCOPE_COMMAND_FUNC(designated_heir, INPUT_CHARACTER, OUTPUT_CHARACTER, OUTPUT_CHARACTER_TYPE) \
  GET_SCOPE_COMMAND_FUNC(player_heir, INPUT_CHARACTER, OUTPUT_CHARACTER, OUTPUT_CHARACTER_TYPE) \
  GET_SCOPE_COMMAND_FUNC(suzerain, INPUT_CHARACTER, OUTPUT_REALM, OUTPUT_REALM_TYPE) \
  GET_SCOPE_COMMAND_FUNC(imprisoner, INPUT_CHARACTER, OUTPUT_CHARACTER, OUTPUT_CHARACTER_TYPE) \
  GET_SCOPE_COMMAND_FUNC(prison, INPUT_CHARACTER, OUTPUT_REALM, OUTPUT_REALM_TYPE) \
  GET_SCOPE_COMMAND_FUNC(self_realm, INPUT_CHARACTER, OUTPUT_REALM, OUTPUT_REALM_TYPE) \
  GET_SCOPE_COMMAND_FUNC(dynasty, INPUT_CHARACTER, OUTPUT_DYNASTY, OUTPUT_DYNASTY_TYPE) \
  GET_SCOPE_COMMAND_FUNC(commanding_army, INPUT_CHARACTER, OUTPUT_ARMY, OUTPUT_ARMY_TYPE) \
  GET_SCOPE_COMMAND_FUNC(hero_troop, INPUT_CHARACTER, OUTPUT_HERO_TROOP, OUTPUT_HERO_TROOP_TYPE) \
  GET_SCOPE_COMMAND_FUNC(secret_religion, INPUT_CHARACTER, OUTPUT_RELIGION, OUTPUT_RELIGION_TYPE) \
  GET_SCOPE_COMMAND_FUNC(secret_religion_group, INPUT_CHARACTER, OUTPUT_RELIGION_GROUP, OUTPUT_RELIGION_GROUP_TYPE) \
  
#define REALM_GET_SCOPE_COMMANDS_LIST \
  GET_SCOPE_COMMAND_FUNC(capital_city, INPUT_REALM, OUTPUT_CITY, OUTPUT_CITY_TYPE2) \
  GET_SCOPE_COMMAND_FUNC(capital_barony, INPUT_REALM, OUTPUT_TITLE, OUTPUT_TITLE_TYPE) \
  GET_SCOPE_COMMAND_FUNC(capital_province, INPUT_REALM, OUTPUT_PROVINCE, OUTPUT_PROVINCE_TYPE) \
  GET_SCOPE_COMMAND_FUNC(main_title, INPUT_REALM, OUTPUT_TITLE, OUTPUT_TITLE_TYPE) \
  GET_SCOPE_COMMAND_FUNC(dominant_religion, INPUT_REALM, OUTPUT_RELIGION, OUTPUT_RELIGION_TYPE) \
  GET_SCOPE_COMMAND_FUNC(state, INPUT_REALM, OUTPUT_REALM, OUTPUT_REALM_TYPE) \
  GET_SCOPE_COMMAND_FUNC(council, INPUT_REALM, OUTPUT_REALM, OUTPUT_REALM_TYPE) \
  GET_SCOPE_COMMAND_FUNC(tribunal, INPUT_REALM, OUTPUT_REALM, OUTPUT_REALM_TYPE) \
  GET_SCOPE_COMMAND_FUNC(assembly, INPUT_REALM, OUTPUT_REALM, OUTPUT_REALM_TYPE) \
  GET_SCOPE_COMMAND_FUNC(clergy, INPUT_REALM, OUTPUT_REALM, OUTPUT_REALM_TYPE) \
  GET_SCOPE_COMMAND_FUNC(liege, INPUT_REALM, OUTPUT_REALM, OUTPUT_REALM_TYPE) \
  GET_SCOPE_COMMAND_FUNC(top_liege, INPUT_REALM, OUTPUT_REALM, OUTPUT_REALM_TYPE) \
  
#define TITLE_GET_SCOPE_COMMANDS_LIST \
  GET_SCOPE_COMMAND_FUNC(city, INPUT_TITLE, OUTPUT_CITY, OUTPUT_CITY_TYPE2) \
  GET_SCOPE_COMMAND_FUNC(province, INPUT_TITLE, OUTPUT_PROVINCE, OUTPUT_PROVINCE_TYPE) \
  GET_SCOPE_COMMAND_FUNC(barony, INPUT_TITLE, OUTPUT_TITLE, OUTPUT_TITLE_TYPE) \
  GET_SCOPE_COMMAND_FUNC(duchy, INPUT_TITLE, OUTPUT_TITLE, OUTPUT_TITLE_TYPE) \
  GET_SCOPE_COMMAND_FUNC(kingdom, INPUT_TITLE, OUTPUT_TITLE, OUTPUT_TITLE_TYPE) \
  GET_SCOPE_COMMAND_FUNC(empire, INPUT_TITLE, OUTPUT_TITLE, OUTPUT_TITLE_TYPE) \
  GET_SCOPE_COMMAND_FUNC(parent, INPUT_TITLE, OUTPUT_TITLE, OUTPUT_TITLE_TYPE) \
  GET_SCOPE_COMMAND_FUNC(top_title, INPUT_TITLE, OUTPUT_TITLE, OUTPUT_TITLE_TYPE) \
  GET_SCOPE_COMMAND_FUNC(de_facto_liege, INPUT_TITLE, OUTPUT_TITLE, OUTPUT_TITLE_TYPE) \
  GET_SCOPE_COMMAND_FUNC(de_jure_liege, INPUT_TITLE, OUTPUT_TITLE, OUTPUT_TITLE_TYPE) \
  GET_SCOPE_COMMAND_FUNC(main_province, INPUT_TITLE, OUTPUT_PROVINCE, OUTPUT_PROVINCE_TYPE) /* main_city? */ \
  GET_SCOPE_COMMAND_FUNC(owner, INPUT_TITLE, OUTPUT_REALM, OUTPUT_REALM_TYPE) /* какое другое название? holder? или переназвать владельца наложницы? переназвал выше */ \
  
#define ARMY_GET_SCOPE_COMMANDS_LIST \
  GET_SCOPE_COMMAND_FUNC(commander, INPUT_ARMY, OUTPUT_CHARACTER, OUTPUT_CHARACTER_TYPE) \
  GET_SCOPE_COMMAND_FUNC(affiliation, INPUT_ARMY, OUTPUT_REALM, OUTPUT_REALM_TYPE) \
  GET_SCOPE_COMMAND_FUNC(location, INPUT_ARMY, OUTPUT_PROVINCE, OUTPUT_PROVINCE_TYPE) \
  GET_SCOPE_COMMAND_FUNC(origin, INPUT_ARMY, OUTPUT_PROVINCE, OUTPUT_PROVINCE_TYPE) \
  
#define PROVINCE_GET_SCOPE_COMMANDS_LIST \
  GET_SCOPE_COMMAND_FUNC(title, INPUT_PROVINCE | INPUT_CITY, OUTPUT_TITLE, OUTPUT_TITLE_TYPE) \
  GET_SCOPE_COMMAND_FUNC(army, INPUT_PROVINCE, OUTPUT_TITLE, OUTPUT_ARMY_TYPE) \
  GET_SCOPE_COMMAND_FUNC(capital, INPUT_PROVINCE, OUTPUT_CITY, OUTPUT_CITY_TYPE2) \
  GET_SCOPE_COMMAND_FUNC(city_type, INPUT_CITY, OUTPUT_CITY, OUTPUT_CITY_TYPE_TYPE) \
  
#define WAR_GET_SCOPE_COMMANDS_LIST \
  GET_SCOPE_COMMAND_FUNC(casus_belli, INPUT_WAR, OUTPUT_CASUS_BELLI, OUTPUT_CASUS_BELLI_TYPE) \
  GET_SCOPE_COMMAND_FUNC(claimant, INPUT_WAR, OUTPUT_CHARACTER, OUTPUT_CHARACTER_TYPE) \
  GET_SCOPE_COMMAND_FUNC(primary_attacker, INPUT_WAR, OUTPUT_CHARACTER, OUTPUT_CHARACTER_TYPE) \
  /*GET_SCOPE_COMMAND_FUNC(primary_attacker_realm, INPUT_WAR, OUTPUT_REALM, OUTPUT_REALM_TYPE)*/ \
  GET_SCOPE_COMMAND_FUNC(primary_defender, INPUT_WAR, OUTPUT_CHARACTER, OUTPUT_CHARACTER_TYPE) \
  /*GET_SCOPE_COMMAND_FUNC(primary_defender_realm, INPUT_WAR, OUTPUT_REALM, OUTPUT_REALM_TYPE)*/ \
  
// группа здесь не выглядит особо полезной
#define TRAIT_GET_SCOPE_COMMANDS_LIST \
  GET_SCOPE_COMMAND_FUNC(opposite, INPUT_TRAIT, OUTPUT_TRAIT, OUTPUT_TRAIT_TYPE) \
  
//   GET_SCOPE_COMMAND_FUNC(group, INPUT_TRAIT, INPUT_TRAIT)
  
#define COMMON_GET_SCOPE_COMMANDS_LIST \
  GET_SCOPE_COMMAND_FUNC(culture, INPUT_CHARACTER | INPUT_PROVINCE, OUTPUT_CULTURE, OUTPUT_CULTURE_TYPE) \
  GET_SCOPE_COMMAND_FUNC(culture_group, INPUT_CHARACTER | INPUT_CULTURE | INPUT_PROVINCE, OUTPUT_CULTURE_GROUP, OUTPUT_CULTURE_GROUP_TYPE) \
  GET_SCOPE_COMMAND_FUNC(religion, INPUT_CHARACTER | INPUT_PROVINCE, OUTPUT_RELIGION, OUTPUT_RELIGION_TYPE) \
  GET_SCOPE_COMMAND_FUNC(religion_group, INPUT_CHARACTER | INPUT_RELIGION | INPUT_PROVINCE, OUTPUT_RELIGION_GROUP, OUTPUT_RELIGION_GROUP_TYPE) \
  GET_SCOPE_COMMAND_FUNC(head, INPUT_RELIGION, OUTPUT_CHARACTER, OUTPUT_CHARACTER_TYPE) \
  GET_SCOPE_COMMAND_FUNC(head_heir, INPUT_RELIGION, OUTPUT_CHARACTER, OUTPUT_CHARACTER_TYPE) \
  
#define GET_SCOPE_COMMANDS_FINAL_LIST \
  CHARACTER_GET_SCOPE_COMMANDS_LIST   \
  REALM_GET_SCOPE_COMMANDS_LIST       \
  TITLE_GET_SCOPE_COMMANDS_LIST       \
  ARMY_GET_SCOPE_COMMANDS_LIST        \
  PROVINCE_GET_SCOPE_COMMANDS_LIST    \
  WAR_GET_SCOPE_COMMANDS_LIST         \
  TRAIT_GET_SCOPE_COMMANDS_LIST       \
  COMMON_GET_SCOPE_COMMANDS_LIST      \

#endif
