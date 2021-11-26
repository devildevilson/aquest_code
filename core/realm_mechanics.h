#ifndef DEVILS_ENGINE_CORE_REALM_MECHANICS_H
#define DEVILS_ENGINE_CORE_REALM_MECHANICS_H

#include <cstdint>
#include "utils/constexpr_funcs.h"
#include "secret_types.h"

// еще должна быть возможность передачи какого то земельного титула
// например передача титула города венеции
// права страны основываются на основных титулах - это титулы верхнего уровня
// соответственно если мы получаем имперский титул 
// основным становится этот имперский титул

// нужно добавить такую вещь как чиновники
// у меня тут наклевывается иерархия по полномочиям некоторая
// то есть непосредственно землевладелец: власть потенциально может быть неограничена
// член совета (парламентарий или что то вроде): власти достаточно много, но она ограничена другими членами совета
// член суда: власти еще меньше чем у совета (потенциально отсутствует возможность инициации какого то действия)
// чиновник: власти совсем мало, их количество и качество и еще некоторые вещи вокруг них будут влиять например на такие вещи как корупция
// так же у чиновника может быть какая то элективная сила (например чиновники голосуют за царя)
// решил пересмотреть отношение к чиновникам, пусть чиновничая механика будет связана с магистратом, так она выглядит органичнее
// да и работает по очень похожему принципу
// придворный: самый базовый тип (просто присутствие)
// также придворные могут различаться по роду (дворяне или нет)

// еще нужно прикинуть цифры в стране, то есть какие то статы, которые явно относятся к законам
// например налогообложение, что то с войсками (контроль над частью войск государственный), 
// нужно придумать какую то механику, которая выносит на совет какие то вообще бессмысленные законы
// переименование земель, как один из таких законов

// решил немного переделать реалмы: теперь пусть они будут институтами или ветвями власти
// законы в той или иной ветви определяются в самом реалме (что в общем то нужно было с самого начала сделать)
// а значит лист механик довольно сильно сократится, что должно быть?
// тип органа (авторитарный, элективный, безинициативный и прочее прочее)
// основные права (если орган элективный то орган может что то сделать только после голосования)
// способ избрания/наследования
// кто может стать представителем
// с таким разделением у нас должны появиться еще общие государственные законы
// там будут гендерные законы, законы вступления в брак, и еще некоторые
// эти вещи лучше вообще никуда не включать, а так они будут болтаться во всех реалмах

// что делать с генералами, с генералами может быть та еще еботня, если начать запрещать всем становится генералами
// но для начала придется понять как будет выглядеть война в принципе

#define POWER_RIGHTS_LIST \
  /* права государственной силы (глава государства, совет, трибунал, ассебли, церковь) */ \
  POWER_RIGHT_FUNC(communal) \
  POWER_RIGHT_FUNC(elective) /* элективный орган, избирается из электоров */ \
  POWER_RIGHT_FUNC(initiative) /* орган обладает инициативой - может сам принимать какие то законы или выполнять какие то действия */ \
  POWER_RIGHT_FUNC(abroad) /* орган управлется извне - то есть например церковный орган управлется из рима */ \
  POWER_RIGHT_FUNC(can_force_vassal_to_return_a_title) /* если в войне между вассалами был отобран титул, то орган может потребовать его вернуть (нужно еще сделать какую то историю титулу) */ \
  POWER_RIGHT_FUNC(can_revoke_title) /* может ли орган отобрать титул безусловно (или это возможность отобрать титул в принципе? у кого?) */ \
  POWER_RIGHT_FUNC(can_revoke_title_from_criminal) /* может ли орган отобрать титул у "преступника" (преступники определены по секретам, но скорее всего чел сначала должен оказаться в тюрьме) */ \
  POWER_RIGHT_FUNC(can_revoke_title_from_infidel) /* может ли орган отобрать титул у неверного (было бы неплохо еще аккуратно сделать смену религии в государстве) */ \
  POWER_RIGHT_FUNC(can_revoke_title_from_excommunicated) /* может ли орган отобрать титул у отлученного */ \
  POWER_RIGHT_FUNC(can_revoke_title_from_state) /* может ли орган отобрать титул у государства в пользу себя (наверное нужно сделать для каждой силы) */ \
  POWER_RIGHT_FUNC(can_interfere_in_vassal_realm) /* может ли орган юридически вмешиваться в законы вассала (например изменив им способ наследования) (как это сделать?) */ \
  POWER_RIGHT_FUNC(can_give_title) /* совет может запретить раздавать титулы господину, вместо него будет раздавать титулы либо совет (парламент) либо суд, должен ли быть какой то общий пул титулов? */ \
  POWER_RIGHT_FUNC(can_give_title_to_realm_leader) /* орган может дать титул без определенного хозяина главе органа (например совет - председателю совета) */ \
  POWER_RIGHT_FUNC(can_give_title_to_infidel) /* может ли орган дать титул неверному */ \
  POWER_RIGHT_FUNC(can_give_title_to_excommunicated) /* может ли орган дать титул отлученному */ \
  POWER_RIGHT_FUNC(can_give_title_to_pleb) /* орган может дать титул плебею */ \
  POWER_RIGHT_FUNC(can_give_title_to_state) /* будет зависеть от того что такое пул титулов (наверное пул свободных титулов должен быть один, но на него через законы могут претендовать разные силы) */ \
  POWER_RIGHT_FUNC(can_inherit_main_titles) /* скорее всего заменится на наследственность положения (то есть может ли председатель наследовать свой статус, или человек может наследовать только принадлежность к совету?) */ \
  POWER_RIGHT_FUNC(can_declare_war) /* может ли совет начать войну, кто тогда воюет? */ \
  POWER_RIGHT_FUNC(can_declare_holy_war) /* может ли совет начать священную войну, у священной войны немного другая механика */ \
  POWER_RIGHT_FUNC(can_enact_laws) /* может ли глава изменить (принять?) закон, законы можно отменить? */ \
  POWER_RIGHT_FUNC(has_the_veto_right) /* право вето, после голосования по закону мы можем его отменить */ \
  POWER_RIGHT_FUNC(can_imprison) /* может ли орган в принципе сажать в тюрьму (нужен для этого повод) (иначе нужно чтобы совет рассмотрел) */ \
  POWER_RIGHT_FUNC(can_imprison_infidel) /* может ли орган сажать в тюрьму неверных */ \
  POWER_RIGHT_FUNC(can_imprison_excommunicated) /* может ли орган сажать в тюрьму отлученных */ \
  POWER_RIGHT_FUNC(can_imprison_freely) /* может ли орган сажать в тюрьму всех подряд */ \
  POWER_RIGHT_FUNC(can_execute) /* может ли орган в принципе казнить (должен быть в тюрьме) (иначе нужно чтобы совет рассмотрел) */ \
  POWER_RIGHT_FUNC(can_execute_infidel) /* может ли орган казнить неверных */ \
  POWER_RIGHT_FUNC(can_execute_excommunicated) /* может ли орган казнить отлученных */ \
  POWER_RIGHT_FUNC(can_execute_freely) /* может ли орган казнить всех подряд */ \
  /* казнь преступников? */ \
  POWER_RIGHT_FUNC(can_banish) /* может ли орган в принципе высылать из страны (нужен повод) (иначе нужно чтобы совет рассмотрел) */ \
  POWER_RIGHT_FUNC(can_banish_shunned) /* может ли орган высылать осуждаемых */ \
  POWER_RIGHT_FUNC(can_banish_infidel) /* может ли орган высылать неверных */ \
  POWER_RIGHT_FUNC(can_banish_excommunicated) /* может ли орган высылать отлученных */ \
  POWER_RIGHT_FUNC(can_banish_freely) /* может ли орган высылать всех подряд */ \
  POWER_RIGHT_FUNC(can_free_from_prison) /* может ли орган освобождать из тюрьмы (тюрьма кстати общая? неуверен) */ \
  POWER_RIGHT_FUNC(can_free_criminal_from_prison) /* может ли орган освобождать из тюрьмы преступников */ \
  POWER_RIGHT_FUNC(can_free_infidel_from_prison) /* может ли орган освобождать из тюрьмы неверных */ \
  POWER_RIGHT_FUNC(can_free_excommunicated_from_prison) /* может ли орган освобождать из тюрьмы отлученных */ \
  POWER_RIGHT_FUNC(can_demand_religious_conversion) /* орган может потребовать сменить религию */ \
  POWER_RIGHT_FUNC(can_legitimate_divorce_with_shunned) /* орган может обеспечить развод с осуждаемым */ \
  POWER_RIGHT_FUNC(can_legitimate_divorce) /* орган может обеспечить развод любой (?) */ \
  POWER_RIGHT_FUNC(can_fire_shunned) /* орган может уволить осуждаемых с должности (с любой?) */ \
  POWER_RIGHT_FUNC(can_remove_hero_status_if_shunned) /* орган может убрать статус героя */ \
  POWER_RIGHT_FUNC(can_remove_priest_status_if_shunned) /* орган может убрать статус священника */ \
  POWER_RIGHT_FUNC(can_remove_general_status_if_shunned) /* орган может убрать статус генерала */ \
  POWER_RIGHT_FUNC(can_indulge_shunned_secret) /* орган может "искупить" осуждаемый секрет */ \
  POWER_RIGHT_FUNC(can_indulge_criminal_secret) /* орган может "искупить" преступный секрет */ \
  POWER_RIGHT_FUNC(can_appoint_a_heir) /* орган может назначит наследника (себе? или главе государства?) */ \
  POWER_RIGHT_FUNC(can_appoint_a_noble) /* орган может выдать дворянство */ \
  POWER_RIGHT_FUNC(can_appoint_a_priest) /* орган может выдать статус священника */ \
  POWER_RIGHT_FUNC(can_appoint_a_hero) /* орган может выдать статус героя */ \
  POWER_RIGHT_FUNC(can_appoint_a_general) /* орган может назначить генерала */ \
  POWER_RIGHT_FUNC(can_appoint_a_liege) /* орган может назначить главу (то есть выборный глава государства?) */ \
  POWER_RIGHT_FUNC(can_appoint_a_councillor) /* огран может назначить консула */ \
  POWER_RIGHT_FUNC(can_appoint_a_magister) /* огран может назначить магистра */ \
  POWER_RIGHT_FUNC(can_appoint_an_assembler) /* огран может назначить ассемблера */ \
  POWER_RIGHT_FUNC(can_appoint_a_clergyman) /* огран может назначить представителя церкви */ \
  POWER_RIGHT_FUNC(can_appoint_a_state_elector) /* огран может назначить электора государства */ \
  POWER_RIGHT_FUNC(can_appoint_a_councillor_elector) /* огран может назначить электора совета */ \
  POWER_RIGHT_FUNC(can_appoint_a_magister_elector) /* огран может назначить электора магистрата */ \
  POWER_RIGHT_FUNC(can_appoint_an_assembler_elector) /* огран может назначить электора ассебли */ \
  POWER_RIGHT_FUNC(can_appoint_a_clergyman_elector) /* огран может назначить электора церкви */ \
  POWER_RIGHT_FUNC(elector_can_appoint_an_elector) /* электор этой силы может назничать другого электора (наверное это не нужно) */ \
  POWER_RIGHT_FUNC(elected_from_electors) /* член органа избирается из электоров */ \
  POWER_RIGHT_FUNC(elected_from_nobles) /* член органа избирается из дворян (могут быть придвоными) */ \
  POWER_RIGHT_FUNC(elected_from_heroes) /* член органа избирается из героев */ \
  POWER_RIGHT_FUNC(elected_from_generals) /* член органа избирается из генералов */ \
  POWER_RIGHT_FUNC(elected_from_statemans) /* член органа избирается из членов стейта (может быть коллективным органом) */ \
  POWER_RIGHT_FUNC(elected_from_councillors) /* член органа избирается из консулов */ \
  POWER_RIGHT_FUNC(elected_from_magisters) /* член органа избирается из магистров */ \
  POWER_RIGHT_FUNC(elected_from_assemblers) /* член органа избирается из ассемблеров */ \
  POWER_RIGHT_FUNC(elected_from_clergyman) /* член органа избирается из церковников */ \
  POWER_RIGHT_FUNC(elected_from_vassals) /* член органа избирается из вассалов (ну то есть дворян с землей) */ \
  POWER_RIGHT_FUNC(elected_from_court) /* член органа избирается из придворных (исключая дворян с землей) */ \
  POWER_RIGHT_FUNC(elected_from_priests) /* член органа избирается из священников (особый статус) */ \
  POWER_RIGHT_FUNC(elected_from_liege_family) /* член органа избирается из близкой семьи главы государства (а если коллективный орган у стейта?) */ \
  POWER_RIGHT_FUNC(elected_from_liege_extended_family) /* член органа избирается из расширенной семьи главы государства */ \
  POWER_RIGHT_FUNC(elected_from_liege_dynasty) /* член органа избирается из династии главы государства */ \
  POWER_RIGHT_FUNC(elected_normally) /* орган избирается по нормальным правилам (по каким? больше голосов электората? скорее всего) */ \
  POWER_RIGHT_FUNC(elected_random) /* избирательная система настолько сложная что фактически случайная (можно ли как то повлиять? хороший вопрос) */ \
  POWER_RIGHT_FUNC(elected_thru_tournament) /* претендент на должность должен победить в специальном турнире (среди других претендентов? логично что так) */ \
  /* что в случае с стейтом? наследная монархия = -коммунал, -электив, инитиатив, -аброад, электед_фром_фамили, инхеритансе_тайп, тип, (достаточно?)  */ \
  POWER_RIGHT_FUNC(vassal_can_become_a_member)   /* в этот орган могут входить вассалы (тут разрешительный или запретительный закон? по умолчанию ограничений по типу нет, с отрицанием не удобно) */ \
  POWER_RIGHT_FUNC(courtier_can_become_a_member) /* в этот орган могут входить придворные */ \
  POWER_RIGHT_FUNC(pleb_can_become_a_member)     /* в этот орган могут входить плебеи */ \
  POWER_RIGHT_FUNC(noble_can_become_a_member)    /* в этот орган могут входить дворяне */ \
  POWER_RIGHT_FUNC(priest_can_become_a_member)   /* в этот орган могут входить священники */ \
  POWER_RIGHT_FUNC(man_can_become_a_member)      /* в этот орган могут входить мужчины */ \
  POWER_RIGHT_FUNC(woman_can_become_a_member)    /* в этот орган могут входить женщины */ \
  POWER_RIGHT_FUNC(hero_can_become_a_member)     /* в этот орган могут входить герои */ \
  POWER_RIGHT_FUNC(vassal_can_become_an_elector)   /* в этот орган могут входить вассалы (тут разрешительный или запретительный закон? по умолчанию ограничений по типу нет) */ \
  POWER_RIGHT_FUNC(courtier_can_become_an_elector) /* в этот орган могут входить придворные */ \
  POWER_RIGHT_FUNC(pleb_can_become_an_elector)     /* в этот орган могут входить плебеи */ \
  POWER_RIGHT_FUNC(noble_can_become_an_elector)    /* в этот орган могут входить дворяне */ \
  POWER_RIGHT_FUNC(priest_can_become_an_elector)   /* в этот орган могут входить священники */ \
  POWER_RIGHT_FUNC(man_can_become_an_elector)      /* в этот орган могут входить мужчины */ \
  POWER_RIGHT_FUNC(woman_can_become_an_elector)    /* в этот орган могут входить женщины */ \
  POWER_RIGHT_FUNC(hero_can_become_an_elector)     /* в этот орган могут входить герои */ \
  POWER_RIGHT_FUNC(vassal_can_apply)   /* в этот орган могут обращаться персонажи с землей (просьба или суд) */ \
  POWER_RIGHT_FUNC(courtier_can_apply) /* в этот орган могут обращаться придворные */ \
  POWER_RIGHT_FUNC(pleb_can_apply)     /* в этот орган могут обращаться плебеи */ \
  POWER_RIGHT_FUNC(noble_can_apply)    /* в этот орган могут обращаться дворяне */ \
  POWER_RIGHT_FUNC(priest_can_apply)   /* в этот орган могут обращаться священники */ \
  POWER_RIGHT_FUNC(man_can_apply)      /* в этот орган могут обращаться мужчины */ \
  POWER_RIGHT_FUNC(woman_can_apply)    /* в этот орган могут обращаться женщины */ \
  POWER_RIGHT_FUNC(hero_can_apply)     /* в этот орган могут обращаться герои */ \
  \
  POWER_RIGHT_FUNC(liege_can_get_this_status) /* глава государства автоматически получает членство в этом органе */ \
  POWER_RIGHT_FUNC(noble_can_get_this_status) /* дворянин автоматически получает членство в этом органе */ \
  POWER_RIGHT_FUNC(hero_can_get_this_status) /* герой автоматически получает членство в этом органе */ \
  POWER_RIGHT_FUNC(priest_can_get_this_status) /* священник автоматически получает членство в этом органе */ \
  POWER_RIGHT_FUNC(general_can_get_this_status) /* генерал автоматически получает членство в этом органе */ \
  POWER_RIGHT_FUNC(clergyman_can_get_this_status) /* церковник автоматически получает членство в этом органе */ \
  POWER_RIGHT_FUNC(councillor_can_get_this_status) /* консул автоматически получает членство в этом органе */ \
  POWER_RIGHT_FUNC(magister_can_get_this_status) /* магистр автоматически получает членство в этом органе */ \
  POWER_RIGHT_FUNC(assembler_can_get_this_status) /* ассемблер автоматически получает членство в этом органе */ \
  POWER_RIGHT_FUNC(member_can_inherit) /* член органа может унаследовать членство */ \
  POWER_RIGHT_FUNC(elector_can_inherit) /* электорат органа может унаследовать статус электора */ \
  POWER_RIGHT_FUNC(member_can_apply_to_the_state) /* член органа может направить запрос к стейту (просьба или судебное решение какое нибудь) */ \
  POWER_RIGHT_FUNC(member_can_apply_to_the_council) /* член органа может направить запрос в совет */ \
  POWER_RIGHT_FUNC(member_can_apply_to_the_tribunal) /* член органа может направить запрос в магистрат */ \
  POWER_RIGHT_FUNC(member_can_apply_to_the_assembly) /* член органа может направить запрос в ассембли */ \
  POWER_RIGHT_FUNC(member_can_apply_to_the_clergy) /* член органа может направить запрос в церковь */ \
  POWER_RIGHT_FUNC(elector_can_apply_to_the_state) /* электор органа может направить запрос в стейт */ \
  POWER_RIGHT_FUNC(elector_can_apply_to_the_council) /* электор органа может направить запрос в совет */ \
  POWER_RIGHT_FUNC(elector_can_apply_to_the_tribunal) /* электор органа может направить запрос в трибунал */ \
  POWER_RIGHT_FUNC(elector_can_apply_to_the_assembly) /* электор органа может направить запрос в ассембли */ \
  POWER_RIGHT_FUNC(elector_can_apply_to_the_clergy) /* электор органа может направить запрос в церковь */ \
  POWER_RIGHT_FUNC(inheritance_type_primogeniture) /* тип наследования (для кого?) - примогенитура (первый ребенок) */ \
  POWER_RIGHT_FUNC(inheritance_type_ultimogeniture) /* тип наследования (для кого?) - ультимогенитура (младший ребенок) */ \
  POWER_RIGHT_FUNC(inheritance_type_house_seniority) /* тип наследования - старшинство дома */ \
  POWER_RIGHT_FUNC(inheritance_type_gavelkind)
  
// нужно ли добавлять сюда гендерные законы, или они в государстве?
// нужно еще перечислить типы действующих лиц в государстве: придворные, вассалы, плеб, дворяне, священники, герои, мужчины и женщины
// придворными могут быть и плеб и дворяне и священники, все из них являются мужчинами или женщинами, все из них могут быть героями
// вассалами могут быть и дворяне и священники (плеб?), все из них являются мужчинами или женщинами, все из них могут быть героями
// еще у нас есть должности (торговец, дипломат, шпион, генерал) на которых могут быть все
  
  // вассал_кан_бекоме_а_мембер, вассал_кан_бекоме_ан_электор
  
//   STATE_RIGHT_FUNC(vassal_can_become_a_councillor) /* персонажи с землями не могут становиться консулами */  
//   STATE_RIGHT_FUNC(vassal_can_become_a_magistrate) /* персонажи с землями не могут становиться магистрами */  
//   STATE_RIGHT_FUNC(vassal_can_become_an_assembler) /* персонажи с землями не могут становиться ассемблерами */  
//   STATE_RIGHT_FUNC(vassal_can_become_a_clergyman) /* персонажи с землями не могут становиться церковниками */  
//   STATE_RIGHT_FUNC(vassal_can_become_a_state_elector) /* персонажи с землями не могут становиться электорами стейта (то есть электорат избирается не из владельцев земли) */  
//   STATE_RIGHT_FUNC(vassal_can_become_a_councillor_elector) /* персонажи с землями не могут становиться электорами совета */  
//   STATE_RIGHT_FUNC(vassal_can_become_a_tribunal_elector) /* персонажи с землями не могут становиться электорами трибунала */  
//   STATE_RIGHT_FUNC(vassal_can_become_an_assembler_elector) /*  */  
//   STATE_RIGHT_FUNC(vassal_can_become_a_clergyman_elector) /*  */  
  
#define STATE_RIGHTS_LIST \
  /* означает всех персонажей с землей, имеет меньший приоритет */ \
  STATE_RIGHT_FUNC(vassal_can_inherit_imperial_titles)  /* наследуемы ли имперские титулы (или они передаются обратно в стейт и раздаются) */ \
  STATE_RIGHT_FUNC(vassal_can_inherit_king_titles)      /* наследуемы ли королевские титулы */ \
  STATE_RIGHT_FUNC(vassal_can_inherit_duchy_titles)     /* наследуемы ли герцогские титулы */ \
  STATE_RIGHT_FUNC(vassal_can_become_a_liege)           /* пока что не знаю что тут... запрет для персонажей с землей? */ \
  STATE_RIGHT_FUNC(vassal_can_become_a_hero)            /* персонажи с землями не могут становиться героями (спорная штука) */ \
  STATE_RIGHT_FUNC(vassal_can_become_a_diplomat)        /* персонажи с землями не могут становиться дипломатами */ \
  STATE_RIGHT_FUNC(vassal_can_become_a_spymaster)       /* персонажи с землями не могут становиться шпионами (разведчиками?) */ \
  STATE_RIGHT_FUNC(vassal_can_become_a_merchant)        /* персонажи с землями не могут становиться торговцами */ \
  STATE_RIGHT_FUNC(vassal_can_become_a_general)         /* персонажи с землями не могут становиться генералами */ \
  STATE_RIGHT_FUNC(vassal_can_become_a_priest)          /* персонажи с землями не могут становиться священниками */ \
  STATE_RIGHT_FUNC(vassals_can_attack_external_enemies) /* персонажи с землей обладают и армиями, могут ли они сами нападать на внешних врагов */ \
  STATE_RIGHT_FUNC(vassals_can_attack_internal_enemies) /* могут ли они сами нападать на внутренних врагов */  \
  STATE_RIGHT_FUNC(vassals_can_use_only_mercs_in_internal_wars) /* во внутренних войнах запрещено использовать обычные регулярные войска */  \
  STATE_RIGHT_FUNC(vassals_can_refuse_to_attack_external_enemies) /* вассалы могут отказаться участвовать во внешней войне, точнее они наверное будут по умолчанию так делать */  \
  /* означает всех придворных вообще (имеет наименьший приоритет) */ \
  STATE_RIGHT_FUNC(courtier_can_become_a_hero)      /* могут ли придворные быть героями (то есть сами становиться? ну да иначе придется через какой то институт получать разрешение) */  \
  STATE_RIGHT_FUNC(courtier_can_become_a_diplomat)  /* могут ли придворные быть дипломатами */  \
  STATE_RIGHT_FUNC(courtier_can_become_a_spymaster) /* могут ли придворные быть шпионами (тайной полицией?) */  \
  STATE_RIGHT_FUNC(courtier_can_become_a_merchant)  /* могут ли придворные быть торговцами (то есть владеть крупным бизнессом) */  \
  STATE_RIGHT_FUNC(courtier_can_become_a_general)   /* могут ли придворные быть генералами */  \
  STATE_RIGHT_FUNC(courtier_can_become_a_noble)     /* могут ли придворные стать дворянами (сами по себе что ли? это что то неудачное скорее всего) */  \
  STATE_RIGHT_FUNC(courtier_can_become_a_priest)    /* могут ли придворные становиться священниками (сами по себе, священник != церковник) */  \
  /* означает всех дворян, приоритет выше чем у вассалов или придворных */ \
  STATE_RIGHT_FUNC(noble_can_become_a_hero)      /* могут ли дворяне становиться героями (иначе через институт) */  \
  STATE_RIGHT_FUNC(noble_can_become_a_diplomat)  /* могут ли дворяне быть дипломатами */  \
  STATE_RIGHT_FUNC(noble_can_become_a_spymaster) /* могут ли дворяне быть шпионами */  \
  STATE_RIGHT_FUNC(noble_can_become_a_merchant)  /* могут ли дворяне быть торговцами */  \
  STATE_RIGHT_FUNC(noble_can_become_a_general)   /* могут ли дворяне быть генералами */  \
  STATE_RIGHT_FUNC(noble_can_become_a_priest)    /* могут ли дворяне становиться священниками */  \
  \
  STATE_RIGHT_FUNC(priest_can_become_a_hero)      /* могут ли священники становиться героями */  \
  STATE_RIGHT_FUNC(priest_can_become_a_diplomat)  /* могут ли священники становиться дипломатами */  \
  STATE_RIGHT_FUNC(priest_can_become_a_spymaster) /* могут ли священники становиться шпионами */  \
  STATE_RIGHT_FUNC(priest_can_become_a_merchant)  /* могут ли священники становиться торговцами */  \
  STATE_RIGHT_FUNC(priest_can_become_a_general)   /* могут ли священники становиться генералами */  \
  STATE_RIGHT_FUNC(priest_can_become_a_noble)     /* могут ли священники становиться дворянами */  \
  STATE_RIGHT_FUNC(priest_can_appoint_a_heir)     /* священник может назначить себе наследника */  \
  STATE_RIGHT_FUNC(priest_status_is_inheritable)  /* статус священника наследуем (то есть сын может унаследовать статус) */  \
  /* гендер, приоритет выше чем у дворян и священников, как у женщин с наследованием? может ли женщина унаследовать например место в совете? скорее всего это смотрим по возможности становления */ \
  STATE_RIGHT_FUNC(woman_can_become_a_hero)        /* могут ли женщины становиться героями */  \
  STATE_RIGHT_FUNC(woman_can_become_a_diplomat)    /* могут ли женщины становиться дипломатами */  \
  STATE_RIGHT_FUNC(woman_can_become_a_spymaster)   /* могут ли женщины становиться шпионами */  \
  STATE_RIGHT_FUNC(woman_can_become_a_merchant)    /* могут ли женщины становиться торговцами */  \
  STATE_RIGHT_FUNC(woman_can_become_a_general)     /* могут ли женщины становиться генералами */  \
  STATE_RIGHT_FUNC(woman_can_become_a_noble)       /* могут ли женщины становиться дворянами */  \
  STATE_RIGHT_FUNC(woman_can_become_a_priest)      /* могут ли женщины становиться священниками */  \
  STATE_RIGHT_FUNC(woman_ruler_opinion_penalty)    /* есть ли предубеждение к правителям женщинам */  \
  STATE_RIGHT_FUNC(woman_can_divorce_with_shunned) /* могут ли женщины разводиться с осуждаемыми */  \
  STATE_RIGHT_FUNC(woman_can_freely_divorce)       /* могут ли женщины свободно разводиться */  \
  /* гендер, приоритет выше чем у дворян и священников */ \
  STATE_RIGHT_FUNC(man_can_become_a_hero)        /* могут ли мужчины становиться героями */  \
  STATE_RIGHT_FUNC(man_can_become_a_diplomat)    /* могут ли мужчины становиться дипломатами */  \
  STATE_RIGHT_FUNC(man_can_become_a_spymaster)   /* могут ли мужчины становиться шпионами */  \
  STATE_RIGHT_FUNC(man_can_become_a_merchant)    /* могут ли мужчины становиться торговцами */  \
  STATE_RIGHT_FUNC(man_can_become_a_general)     /* могут ли мужчины становиться генералами */  \
  STATE_RIGHT_FUNC(man_can_become_a_noble)       /* могут ли мужчины становиться дворянами */  \
  STATE_RIGHT_FUNC(man_can_become_a_priest)      /* могут ли мужчины становиться священниками */  \
  STATE_RIGHT_FUNC(man_ruler_opinion_penalty)    /* есть ли предубеждение к правителям мужчинам */  \
  STATE_RIGHT_FUNC(man_can_divorce_with_shunned) /* могут ли мужчины разводиться с осуждаемыми */  \
  STATE_RIGHT_FUNC(man_can_freely_divorce)       /* могут ли мужчины свободно разводиться */  \
  /* статус героя - имеет наивысший приоритет */ \
  STATE_RIGHT_FUNC(hero_can_become_a_diplomat)  /* могут ли герои становиться дипломатами */  \
  STATE_RIGHT_FUNC(hero_can_become_a_spymaster) /* могут ли герои становиться шпионами */  \
  STATE_RIGHT_FUNC(hero_can_become_a_merchant)  /* могут ли герои становиться торговцами */  \
  STATE_RIGHT_FUNC(hero_can_become_a_general)   /* могут ли герои становиться генералами */  \
  STATE_RIGHT_FUNC(hero_can_become_a_noble)     /* могут ли герои становиться дворянами */  \
  STATE_RIGHT_FUNC(hero_can_become_a_priest)    /* могут ли герои становиться священниками */  \
  STATE_RIGHT_FUNC(hero_can_appoint_a_heir)     /* могут ли герои выбрать наследника */  \
  STATE_RIGHT_FUNC(hero_status_is_inheritable)  /* статус героя наследуем */  \
  /* остальные законы */ \
  STATE_RIGHT_FUNC(male_polygamy)              /* полигамия доступна мужчинам, остальные женщины - наложницы */  \
  STATE_RIGHT_FUNC(female_polygamy)            /* полигамия доступна женщинам, остальные мужчины - наложники */  \
  STATE_RIGHT_FUNC(male_polygamy_equal)        /* наложницы становятся вторыми женами */  \
  STATE_RIGHT_FUNC(female_polygamy_equal)      /* наложники становятся вторыми мужьями */  \
  STATE_RIGHT_FUNC(homosexual_marriage)        /* могут ли геи вступать в брак */ \
  STATE_RIGHT_FUNC(homosexual_can_inherit)     /* могут ли геи наследовать */ \
  STATE_RIGHT_FUNC(blinded_can_inherit)        /* могут ли наследовать ослепленные */ \
  STATE_RIGHT_FUNC(excommunicated_can_inherit) /* могут ли наследовать отлученные */ \
  STATE_RIGHT_FUNC(criminal_can_inherit)       /* могут ли наследовать преступники */ \
  STATE_RIGHT_FUNC(infidel_can_inherit)        /* могут ли наследовать неверные */  \
  STATE_RIGHT_FUNC(horse_can_inherit)          /* могут ли наследовать лошади */  \
  STATE_RIGHT_FUNC(cat_can_inherit)            /* могут ли наследовать коты */  \
  STATE_RIGHT_FUNC(dog_can_inherit)            /* могут ли наследовать собаки */  \
  STATE_RIGHT_FUNC(execution_allowed)          /* казнь разрешена в государстве */  \
  STATE_RIGHT_FUNC(banishment_allowed)         /* изгнание доступно в государстве */  \
  STATE_RIGHT_FUNC(divorce_allowed)            /* развод доступен в государстве */  \
  STATE_RIGHT_FUNC(priest_marriage_allowed)    /* могут ли священники вступать в брак */  \
  STATE_RIGHT_FUNC(brother_sister_marriage_allowed)  /* бракосочетания близких родственников доступны (инцест - тайные отношения? ну наверное это еще дополнительно регламентирует близкие отношения) */  \
  STATE_RIGHT_FUNC(parent_child_marriage_allowed)    /* бракосочетания ребенок/родитель доступны */  \
  STATE_RIGHT_FUNC(uncle_niece_marriage_allowed)     /* бракосочетания дядя/племянник доступны */  \
  STATE_RIGHT_FUNC(cousin_marriage_allowed)          /* бракосочетания с кузинами доступны */  \
  STATE_RIGHT_FUNC(matrilineal_marriages_allowed)    /* браки по материнской линии доступны */  \
  STATE_RIGHT_FUNC(infidels_pays_additional_taxes)   /* неверные платят дополнительный налог */  \
  STATE_RIGHT_FUNC(bastard_child_has_equals_rights)  /* бастарды имеют равные права с остальными детьми (их наверное тоже нужно регламентировать?) */  \
  STATE_RIGHT_FUNC(concubine_child_has_equal_rights) /* дети наложниц имеют равные права */ \
  STATE_RIGHT_FUNC(titles_can_be_inherited_by_someone_in_the_other_realm)
  
#define RELIGION_MECHANICS_LIST \
  /* тип главенства (его может не быть) */ \
  RELIGION_MECHANICS_FUNC(head_is_independent) /* независимый от светской власти */ \
  RELIGION_MECHANICS_FUNC(head_is_authoritarian) /* неприемлит какие то отдельные течения (по идее обратный автокефалиям) */ \
  RELIGION_MECHANICS_FUNC(head_is_council) /* вместо единого главы - совет */ \
  RELIGION_MECHANICS_FUNC(independence_structure_autocephaly) /* автокефалия - независимый от основной религии священик высокого сана (может примерно то же что и глава религии) */ \
  RELIGION_MECHANICS_FUNC(pentarchy) /* 5 исторических пентархов (священики-бароны) всегда считатются патриархами правителя в соответсвующем королевстве (не очень понимаю в чем прикол) */ \
  /* выборы и наследование */ \
  RELIGION_MECHANICS_FUNC(head_can_inherit) /* наследуем статус главы религии */ \
  RELIGION_MECHANICS_FUNC(head_is_elected) /* либо избираем нового главу */ \
  RELIGION_MECHANICS_FUNC(priests_can_become_an_elector) /* священник может стать электором (в данном случае Кардинал) */ \
  RELIGION_MECHANICS_FUNC(secular_rulers_can_become_an_elector) /* светский правитель может стать электором (и избирать папу) */ \
  RELIGION_MECHANICS_FUNC(elector_elected_from_priests) /* священники избирают электоров */ \
  RELIGION_MECHANICS_FUNC(elector_elected_from_secular_rulers) /* светские правители избирают электоров */ \
  RELIGION_MECHANICS_FUNC(head_can_appoint_an_elector) /* электоры назначаются главой */ \
  RELIGION_MECHANICS_FUNC(head_can_appoint_a_secular_rulers) /* глава религии может назначить светских правителей (пока не уверен как это будет работать) */ \
  /* что может глава религии */ \
  RELIGION_MECHANICS_FUNC(papal_investiture) /* глава религии ставит своих священников */ \
  RELIGION_MECHANICS_FUNC(head_can_demand_religious_conversion) /* */ \
  RELIGION_MECHANICS_FUNC(head_can_excommunicate) /* */ \
  RELIGION_MECHANICS_FUNC(head_can_grant_divorce) /* */ \
  RELIGION_MECHANICS_FUNC(head_can_grant_claim) /* */ \
  RELIGION_MECHANICS_FUNC(head_can_call_crusade) /* */ \
  RELIGION_MECHANICS_FUNC(head_can_call_a_holy_tribunal) /* священный суд, может быть вызван чтобы публично покарать отступника или убийцу единоверцев или еще чего */ \
  RELIGION_MECHANICS_FUNC(head_can_appoint_a_caste_for_secular_ruler) /* касты назначаются главой (иначе выдаются при рождении от родителей) (можно сделать случайное распределение) */ \
  RELIGION_MECHANICS_FUNC(secular_ruler_caste_is_determined_random) /* случайное распределение каст (можно сделать эвенты на сокрытие каст) */ \
  /* особенности (права) последователей */ \
  RELIGION_MECHANICS_FUNC(can_have_antipopes) /* */ \
  RELIGION_MECHANICS_FUNC(can_retire_to_monastery) /* */ \
  RELIGION_MECHANICS_FUNC(can_choose_patron_deity) /* */ \
  RELIGION_MECHANICS_FUNC(can_revoke_title_from_excommunicated) /* */ \
  RELIGION_MECHANICS_FUNC(can_give_title_to_excommunicated) /* */ \
  RELIGION_MECHANICS_FUNC(can_imprison_excommunicated) /* */ \
  RELIGION_MECHANICS_FUNC(can_execute_excommunicated) /* */ \
  RELIGION_MECHANICS_FUNC(can_banish_excommunicated) /* */ \
  RELIGION_MECHANICS_FUNC(have_sects) /* */ \
  RELIGION_MECHANICS_FUNC(priests_can_marry) /* */ \
  RELIGION_MECHANICS_FUNC(priests_can_inherit) /* */ \
  RELIGION_MECHANICS_FUNC(feminist) /* возможно более тонкая настройка? */ \
  RELIGION_MECHANICS_FUNC(pacifist) /* ниже агрессивность у последователей */ \
  RELIGION_MECHANICS_FUNC(brother_sister_marriage_allowed) /* */ \
  RELIGION_MECHANICS_FUNC(parent_child_marriage_allowed) /* */ \
  RELIGION_MECHANICS_FUNC(uncle_niece_marriage_allowed) /* */ \
  RELIGION_MECHANICS_FUNC(cousin_marriage_allowed) /* */ \
  RELIGION_MECHANICS_FUNC(matrilineal_marriages_allowed) /* */ \
  RELIGION_MECHANICS_FUNC(female_temple_holders) /* могут ли женщины владеть храмами */ \
  RELIGION_MECHANICS_FUNC(male_temple_holders) /* */ \
  RELIGION_MECHANICS_FUNC(castes) /* использует ли религия кастовую систему */ \
  RELIGION_MECHANICS_FUNC(caste_opinions) /* модификаторы к отношениям между кастами */ \
  /* викинги, в принципе может пригоджиться */ \
  RELIGION_MECHANICS_FUNC(allow_viking_invasion) /* ? */ \
  RELIGION_MECHANICS_FUNC(allow_looting) /* ? */ \
  RELIGION_MECHANICS_FUNC(seafarer) /* написано что ИИ препочитает нападать на береговые провинции */ \
  RELIGION_MECHANICS_FUNC(allow_rivermovement) /* врядли нужно */ \
  /* механики */ \
  RELIGION_MECHANICS_FUNC(divine_blood) /* священные близкородственные связи (можно ли как то мне сделать иначе) */ \
  RELIGION_MECHANICS_FUNC(has_heir_designation) /* выбор наследника */ \
  RELIGION_MECHANICS_FUNC(peace_authority_loss) /* */ \
  RELIGION_MECHANICS_FUNC(peace_esteem_loss) /* */ \
  RELIGION_MECHANICS_FUNC(peace_inluence_loss) /* */ \
  RELIGION_MECHANICS_FUNC(attacking_same_religion_authority_loss) /* */ \
  RELIGION_MECHANICS_FUNC(attacking_same_religion_esteem_loss) /* */ \
  RELIGION_MECHANICS_FUNC(attacking_same_religion_inluence_loss) /* */ \
  RELIGION_MECHANICS_FUNC(vassal_king_authority_bonus) /* */ \
  RELIGION_MECHANICS_FUNC(vassal_king_esteem_bonus) /* */ \
  RELIGION_MECHANICS_FUNC(vassal_king_inluence_bonus) /* */ \
  RELIGION_MECHANICS_FUNC(hostile_within_group) /* аргесивность внутри религиозной группы */ \
  RELIGION_MECHANICS_FUNC(ai_peaceful) /* орда теряет супер агрессивность */ \
  RELIGION_MECHANICS_FUNC(raised_vassal_opinion_loss) /* использование войск вассала портит отношения (нужно ли? нужно сделать механику передачи войск от вассалов) */ \
  RELIGION_MECHANICS_FUNC(reformer_head_of_religion) /* игрок кто реформировал религию становится главой */ \
  RELIGION_MECHANICS_FUNC(pre_reformed) /* не очень понятно что делает */ \
  RELIGION_MECHANICS_FUNC(dislike_tribal_organization) /* увеличение организованности племен злит вассалов (организованность в обычном понимании наверное будет отсутствовать) */ \
  /* механика муслимов  */ \
  /* (обозначает "обычный" мусульманский цикл правителей: правитель приходит на бедную землю, отстраивает ее, начинает жить лучше, его влияние ослабевает, его замещиет другая мусульманская семья)  */ \
  /* чем лучше живут мусульмане, тем менее стабильно становится их государство... интересно */ \
  RELIGION_MECHANICS_FUNC(uses_decadence) /* это можно обыграть */ \
  RELIGION_MECHANICS_FUNC(infidels_pays_additional_taxes) /* налоги на неверных */ \
  RELIGION_MECHANICS_FUNC(hard_to_convert) /* тяжело конвертировать */ \
  RELIGION_MECHANICS_FUNC(men_can_take_consorts) /* позволяет брать наложниц мужчинам */ \
  RELIGION_MECHANICS_FUNC(women_can_take_consorts) /* то же самое женщинам */ \
  \
  RELIGION_MECHANICS_FUNC(join_crusade_if_bordering_hostile) /* ??? (модификатор для ии скорее всего) */ \
  RELIGION_MECHANICS_FUNC(rel_head_defense) /* ии пытается защитить главу религии */ \
  /* */ \
  RELIGION_MECHANICS_FUNC(ai_try_to_convert_same_group) /* */ \
  RELIGION_MECHANICS_FUNC(ai_try_to_convert_same_group_if_zealot) /* */ \
  RELIGION_MECHANICS_FUNC(ai_try_to_convert_other_group) /* */ \
  RELIGION_MECHANICS_FUNC(ai_try_to_convert_other_group_if_zealot) /* */ \
  
#define CULTURE_MECHANICS_LIST \
  CULTURE_MECHANICS_FUNC(horde) \
  CULTURE_MECHANICS_FUNC(used_for_random) \
  CULTURE_MECHANICS_FUNC(allow_in_ruler_designer) \
  CULTURE_MECHANICS_FUNC(dukes_called_kings) \
  CULTURE_MECHANICS_FUNC(baron_titles_hidden) \
  CULTURE_MECHANICS_FUNC(count_titles_hidden) \
  CULTURE_MECHANICS_FUNC(founder_named_dynasties) \
  CULTURE_MECHANICS_FUNC(dynasty_title_names) \
  CULTURE_MECHANICS_FUNC(disinherit_from_blinding) \
  CULTURE_MECHANICS_FUNC(allow_looting) \
  CULTURE_MECHANICS_FUNC(seafarer) \
  CULTURE_MECHANICS_FUNC(dynasty_name_first) \
  CULTURE_MECHANICS_FUNC(feminist) \
  \
  CULTURE_MECHANICS_FUNC(has_master_gender) \
  CULTURE_MECHANICS_FUNC(has_lord_gender) \
  CULTURE_MECHANICS_FUNC(has_king_gender) \
  CULTURE_MECHANICS_FUNC(has_emperor_gender) \
  CULTURE_MECHANICS_FUNC(has_hero_gender) \
  CULTURE_MECHANICS_FUNC(has_wizard_gender) \
  CULTURE_MECHANICS_FUNC(has_duke_gender) \
  CULTURE_MECHANICS_FUNC(has_count_gender) \
  CULTURE_MECHANICS_FUNC(has_heir_gender) \
  CULTURE_MECHANICS_FUNC(has_prince_gender) \
  CULTURE_MECHANICS_FUNC(has_baron_gender) \
  
namespace devils_engine {
  namespace core {
    namespace power_rights {
      enum values : uint32_t {
#define POWER_RIGHT_FUNC(name) name,
        POWER_RIGHTS_LIST
#undef POWER_RIGHT_FUNC
        
        count
      };
      
      constexpr size_t offset = 0;
      constexpr uint32_t bit_container_size = ceil(double(count) / double(UINT32_WIDTH));
    }
    
    namespace state_rights {
      enum values : uint32_t {
#define STATE_RIGHT_FUNC(name) name,
        STATE_RIGHTS_LIST
#undef STATE_RIGHT_FUNC

#define SECRET_TYPE_FUNC(name) name##_is_considered_shunned,
        SECRET_TYPES_LIST
#undef SECRET_TYPE_FUNC

#define SECRET_TYPE_FUNC(name) name##_is_considered_criminal,
        SECRET_TYPES_LIST
#undef SECRET_TYPE_FUNC
        
        count,
        shunned_start = attempted_murder_is_considered_shunned,
        criminal_start = attempted_murder_is_considered_criminal
      };
      
      constexpr size_t offset = power_rights::count;
      constexpr uint32_t bit_container_size = ceil(double(count) / double(UINT32_WIDTH));
    }
    
    // тип главенства, наследование, как избирают, влияние, механики
    // религия определяет какие то свои законы и может пытаться их зафорсить в стране при не совпадении
    // + должны быть законы непосредственно самой религии (то есть как избирается глава религии, и что из себя представляет администрация)
    // и наверное хорошим решением будет отделить эти вещи, ко всему прочему ожидания от государств могут меняться с течением времени
    // религия сильно влияла на культурные вещи (на восприятие что хорошо что плохо) + ко всему священники в религии зачастую имели довольно высокий статус
    // тут нужно немного переделать на чисто политические стремления (менеджмент + что должна мочь делать в государстве) + чисто культурные стремления
    // должен ли меняться меджмент у религии? скорее нет, вообще могут ли меняться приорететы у религии по ходу игры? мне кажется что врядли
    // политические стремления должны поидее быть сразу максимальными, другое дело как понять есть ли механика отлучения от церкви?
    // тут почти все должно браться из политической силы + несколько дополнений
    // верующие стремятся к тому чтобы государственные законы были такими как в этом списке
    // верующие также могут начать гундерь если что то в государстве происходит не так как в вере
    namespace religion_mechanics {
      enum values : uint32_t {
#define RELIGION_MECHANICS_FUNC(name) name,
        RELIGION_MECHANICS_LIST
#undef RELIGION_MECHANICS_FUNC
        
        count
      };
      
      constexpr uint32_t bit_container_size = ceil(double(count) / double(UINT32_WIDTH));
    }
    
    namespace culture_mechanics {
      enum values : uint32_t {        
#define CULTURE_MECHANICS_FUNC(name) name,
        CULTURE_MECHANICS_LIST
#undef CULTURE_MECHANICS_FUNC
        
        count
      };
      
      constexpr uint32_t bit_container_size = ceil(double(count) / double(UINT32_WIDTH));
    }
  }
}

#endif
