#ifndef DEVILS_ENGINE_SCRIPT_CHANGE_CONTEXT_COMMANDS_MACRO_H
#define DEVILS_ENGINE_SCRIPT_CHANGE_CONTEXT_COMMANDS_MACRO_H

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
#define INPUT_ALL script::object::type_bit::all

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
#define OUTPUT_PROVINCE script::object::type_bit::province
#define OUTPUT_CASUS_BELLI script::object::type_bit::casus_belli
#define OUTPUT_NUMBER script::object::type_bit::number
#define OUTPUT_BOOL script::object::type_bit::boolean
#define OUTPUT_NOTHING script::object::type_bit::invalid // или 0? ну функции действительно что то возвращают

// функции every_*, has_* и random_* в луа не нужны или выглядят иначе

// можно придумать способ выдачи следующего вассала по запросу: например ставить в рвалуе предыдущего или nullptr
// если nullptr то берем первого, возвращаем инвалид на последнем, чем это нам может помочь? 
// такое поведение сделать сложно потому что не вегда список один, и соответственно не всегда очевидно что мы конкретно сейчас обходим

// мы можем еще пройти близких членов семьи и расширенную семью
// в цк3 еще выделяются предыдущие наложницы, в цк3 можно пройтись по всем наследникам это каким? по всем возможным наследникам?
// по всем возможным играбельным наследникам (не всегда совпадает с общим числом)
// было бы неплохо также пройтись по всем челикам которых мы зарейдили
#define CHARACTER_CHANGE_CONTEXT_COMMANDS_LIST \
  CHANGE_CONTEXT_COMMAND_FUNC(ancestor, INPUT_CHARACTER, INPUT_CHARACTER, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(sibling, INPUT_CHARACTER, INPUT_CHARACTER, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(child, INPUT_CHARACTER, INPUT_CHARACTER, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(brother, INPUT_CHARACTER, INPUT_CHARACTER, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(sister, INPUT_CHARACTER, INPUT_CHARACTER, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(concubine, INPUT_CHARACTER, INPUT_CHARACTER, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(acquaintance, INPUT_CHARACTER, INPUT_CHARACTER, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(good_acquaintance, INPUT_CHARACTER, INPUT_CHARACTER, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(bad_acquaintance, INPUT_CHARACTER, INPUT_CHARACTER, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(pal, INPUT_CHARACTER, INPUT_CHARACTER, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(foe, INPUT_CHARACTER, INPUT_CHARACTER, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(sympathy, INPUT_CHARACTER, INPUT_CHARACTER, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(dislike, INPUT_CHARACTER, INPUT_CHARACTER, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(parent, INPUT_CHARACTER, INPUT_CHARACTER, OUTPUT_NOTHING) /* пройти оба родителя */ \
  CHANGE_CONTEXT_COMMAND_FUNC(claim, INPUT_CHARACTER, INPUT_TITLE, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(de_jure_claim, INPUT_CHARACTER, INPUT_TITLE, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(heir_to_title, INPUT_CHARACTER, INPUT_TITLE, OUTPUT_NOTHING) /* каждый титул который может унаследовать персонаж */ \
  CHANGE_CONTEXT_COMMAND_FUNC(election_realm, INPUT_CHARACTER, INPUT_REALM, OUTPUT_NOTHING) /* каждый реалм в котором может головать персонаж */ \
  
// *_war_ally - эти функции нужны для того чтобы проверить персонажей вступающих в войну
// то есть у персонажа проверяем всех союзников (или противников) и смотрим чтобы люди случайно не напали друг на друга будучи союзниками
// по названию кажется что мы берем союзников у какой то определенной войны
// персонаж в другой войне может быть союзником текущего противника в той войне, в эту войну призвать его не получится
// скорее всего вся дипломатия переместится в персонажа, потому что она по сути имеет смысл только в персонаже
#define REALM_CHANGE_CONTEXT_COMMANDS_LIST \
  CHANGE_CONTEXT_COMMAND_FUNC(war, INPUT_REALM, INPUT_WAR, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(war_ally, INPUT_REALM, INPUT_CHARACTER, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(war_enemy, INPUT_REALM, INPUT_CHARACTER, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(ally, INPUT_REALM, INPUT_REALM, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(truce_holder, INPUT_REALM, INPUT_REALM, OUTPUT_NOTHING) /* все персонажи у которых перемеирие с этим персонажем (в моем случае реалмы) */ \
  CHANGE_CONTEXT_COMMAND_FUNC(truce_target, INPUT_REALM, INPUT_REALM, OUTPUT_NOTHING) /* все перемирия этого персонажа (в моем случае реалма) */ \
  CHANGE_CONTEXT_COMMAND_FUNC(army, INPUT_REALM, INPUT_ARMY, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(member, INPUT_REALM, INPUT_CHARACTER, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(elector, INPUT_REALM, INPUT_CHARACTER, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(election_candidate, INPUT_REALM, INPUT_CHARACTER, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(prisoner, INPUT_REALM, INPUT_CHARACTER, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(courtier, INPUT_REALM, INPUT_CHARACTER, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(owned_title, INPUT_REALM, INPUT_TITLE, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(realm_title, INPUT_REALM, INPUT_TITLE, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(directly_owned_province, INPUT_REALM, INPUT_PROVINCE, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(directly_owned_city, INPUT_REALM, INPUT_CITY, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(realm_city, INPUT_REALM, INPUT_CITY, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(realm_province, INPUT_REALM, INPUT_PROVINCE, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(de_jure_duchy, INPUT_REALM, INPUT_TITLE, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(de_jure_kingdom, INPUT_REALM, INPUT_TITLE, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(de_jure_empire, INPUT_REALM, INPUT_TITLE, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(neighboring_top_liege, INPUT_REALM, INPUT_REALM, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(neighboring_same_rank, INPUT_REALM, INPUT_REALM, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(vassal, INPUT_REALM, INPUT_REALM, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(vassal_or_below, INPUT_REALM, INPUT_REALM, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(liege_or_above, INPUT_REALM, INPUT_REALM, OUTPUT_NOTHING) /* начальник или начальник начальника и далее, тут? */ \
  
// у нас еще есть гости, возможно имеет смысл сделать для всех функций строгий инпут, просто добавить несколько дополнительных функций например self_realm_vassals
// так и сделаю
  
#define WAR_CHANGE_CONTEXT_COMMANDS_LIST \
  CHANGE_CONTEXT_COMMAND_FUNC(attacker, INPUT_WAR, INPUT_CHARACTER, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(defender, INPUT_WAR, INPUT_CHARACTER, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(participant, INPUT_WAR, INPUT_CHARACTER, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(target_title, INPUT_WAR, INPUT_TITLE, OUTPUT_NOTHING) \
  
// святые места?
#define RELIGION_CHANGE_CONTEXT_COMMANDS_LIST \
  CHANGE_CONTEXT_COMMAND_FUNC(child_religion, INPUT_RELIGION, INPUT_RELIGION, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(sibling_religion, INPUT_RELIGION, INPUT_RELIGION, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(believer, INPUT_RELIGION, INPUT_CHARACTER, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(secret_believer, INPUT_RELIGION, INPUT_CHARACTER, OUTPUT_NOTHING) \
  
#define CULTURE_CHANGE_CONTEXT_COMMANDS_LIST \
  CHANGE_CONTEXT_COMMAND_FUNC(child_culture, INPUT_CULTURE, INPUT_CULTURE, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(sibling_culture, INPUT_CULTURE, INPUT_CULTURE, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(culture_member, INPUT_CULTURE, INPUT_CHARACTER, OUTPUT_NOTHING) \
  
#define PROVINCE_CHANGE_CONTEXT_COMMANDS_LIST \
  CHANGE_CONTEXT_COMMAND_FUNC(local_city, INPUT_PROVINCE, INPUT_CITY, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(neighbour, INPUT_PROVINCE, INPUT_PROVINCE, OUTPUT_NOTHING) \
  
// каждый наследник титула? электор у нас в реалме
#define TITLE_CHANGE_CONTEXT_COMMANDS_LIST \
  CHANGE_CONTEXT_COMMAND_FUNC(sibling_title, INPUT_TITLE, INPUT_TITLE, OUTPUT_NOTHING) /* один титул выше по иерархии */ \
  CHANGE_CONTEXT_COMMAND_FUNC(child_title, INPUT_TITLE, INPUT_TITLE, OUTPUT_NOTHING) /* все титулы ниже по иерархии */ \
  CHANGE_CONTEXT_COMMAND_FUNC(in_hierarchy, INPUT_TITLE, INPUT_TITLE, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(in_de_facto_hierarchy, INPUT_TITLE, INPUT_TITLE, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(in_de_jure_hierarchy, INPUT_TITLE, INPUT_TITLE, OUTPUT_NOTHING) \
  
#define CHANGE_CONTEXT_COMMANDS_GLOBAL_LIST \
  CHANGE_CONTEXT_COMMAND_FUNC(city_global,              INPUT_ALL, INPUT_CITY, OUTPUT_NOTHING)  \
  CHANGE_CONTEXT_COMMAND_FUNC(province_global,          INPUT_ALL, INPUT_PROVINCE, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(barony_global,            INPUT_ALL, INPUT_TITLE, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(duchy_global,             INPUT_ALL, INPUT_TITLE, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(kingdom_global,           INPUT_ALL, INPUT_TITLE, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(empire_global,            INPUT_ALL, INPUT_TITLE, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(religion_group_global,    INPUT_ALL, INPUT_RELIGION, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(religion_global,          INPUT_ALL, INPUT_RELIGION, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(culture_group_global,     INPUT_ALL, INPUT_CULTURE, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(culture_global,           INPUT_ALL, INPUT_CULTURE, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(living_character_global,  INPUT_ALL, INPUT_CHARACTER, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(ruler_global,             INPUT_ALL, INPUT_CHARACTER, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(independent_ruler_global, INPUT_ALL, INPUT_CHARACTER, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(player_global,            INPUT_ALL, INPUT_CHARACTER, OUTPUT_NOTHING) \
  
#define CHANGE_CONTEXT_COMMANDS_FINAL_LIST \
  CHARACTER_CHANGE_CONTEXT_COMMANDS_LIST   \
  REALM_CHANGE_CONTEXT_COMMANDS_LIST       \
  WAR_CHANGE_CONTEXT_COMMANDS_LIST         \
  RELIGION_CHANGE_CONTEXT_COMMANDS_LIST    \
  CULTURE_CHANGE_CONTEXT_COMMANDS_LIST     \
  PROVINCE_CHANGE_CONTEXT_COMMANDS_LIST    \
  TITLE_CHANGE_CONTEXT_COMMANDS_LIST       \
  CHANGE_CONTEXT_COMMANDS_GLOBAL_LIST      \
  
//   COMMON_CHANGE_CONTEXT_COMMANDS_LIST
  
  //CHANGE_CONTEXT_COMMAND_FUNC(dejure_vassal_title_holder, INPUT_TITLE, INPUT_TITLE, OUTPUT_NOTHING) /* ??? */

// глобальный обход по всем энтити одного типа, в цк3 еще дополнительно выделяются такие функции:
// every_county (у меня это баронство)
// every_county_in_region (регион?)
// every_pool_character (персонажи в городе, как мне это сделать?)

// ancestor - родственники до 5-го поколения вверх, имеет смысл сделать рекурсивную функцию
// election_candidate - поидее это обход вассалов + придворных + семья правителя + применение на них функции проверки прав
// army - каждая армия реалма, это тупо нужно добавить в лист
// 

#endif
