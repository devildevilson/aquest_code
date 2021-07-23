#ifndef REALM_MECHANICS_H
#define REALM_MECHANICS_H

#include <cstdint>
#include <cmath>
#include "constexpr_funcs.h"

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

namespace devils_engine {
  namespace utils {
    namespace realm_mechanics {
      enum values {
        // права главы государства
        liege_can_force_vassal_to_return_a_title, // если в войне между вассалами был отобран титул, то глава может потребовать его вернуть
        liege_can_revoke_title,               // может ли господин отобрать титул безусловно
        liege_can_revoke_title_from_criminal, // может ли господин отобрать титул у "преступника" (возможно просто политический противник сидящий в тюрьме)
        liege_can_revoke_title_from_infidel,  // может ли господин отобрать титул у неверного
        liege_can_revoke_title_from_excommunicated,  // может ли господин отобрать титул у отлученного
        liege_can_interfere_in_vassal_realm,  // может ли глава юридически вмешиваться в законы вассала (например изменив им способ наследования)
        liege_can_give_title,                 // совет может запретить раздавать титулы господину, вместо него будет раздавать титулы либо совет (парламент) либо суд
        liege_can_give_title_to_infidel,      // может ли господин дать титул неверному
        liege_can_give_title_to_excommunicated, // может ли господин дать титул отлученному
        liege_can_give_title_to_pleb,         // глава государства может дать титул плебею
//         liege_can_give_title_to_council,      // глава государства может выдать титул в элективный орган (ну хотя запретить это не выйдет)
        liege_can_inherit_main_titles,        // если нет, глава государства избираемый или назначаемый
//         liege_can_inherit_imperial_titles,    // император в этом государстве избираемый или назначаемый (в зависимости от максимального титула)
//         liege_can_inherit_king_titles,        // король в этом государстве избираемый или назначаемый (в зависимости от максимального титула)
//         liege_can_inherit_duchy_titles,       // герцог в этом государстве избираемый или назначаемый (в зависимости от максимального титула)
        liege_can_declare_war,               // может ли глава начать войну
        liege_can_enact_laws,                // может ли глава изменить закон
//         liege_can_revoke_laws,               // наверное пункт выше более общий и будет включать в себя и это
        liege_has_the_veto_right,             // право вето, после голосования по закону мы можем его отменить
        liege_can_become_a_general,           
        liege_can_become_a_hero,              // глава государства может или не может становится героем 
        liege_can_become_a_councillor,        // может ли глава государства стать консулом (членом парламента)
        liege_can_become_a_magistrate,        // глава может стать судьей (то есть решать споры вассалов?)
        liege_can_appoint_an_elector,         // может ли глава назначить электора
        liege_can_appoint_a_councillor,       // может ли глава назначить члена парламента
        liege_can_appoint_a_magistrate,       // может ли глава назначить судью
        liege_can_appoint_a_heir,             // может ли глава назначить наследника (то есть выбрать из своих детей или возможно из каких то челиков)
        liege_can_appoint_a_noble,            // может ли глава государства давать дворянский титул плебею
        liege_can_appoint_a_hero,             // может ли глава государства сделать кого то героем
        liege_can_appoint_a_general,          // может ли глава государства сделать кого то генералом
        liege_can_appoint_a_clergyman,        // может ли глава государства сделать кого то священником
        liege_can_imprison,                   // может ли глава государства в принципе сажать в тюрьму (нужен для этого повод) (иначе нужно чтобы совет рассмотрел)
        liege_can_imprison_infidel,           // может ли глава государства сажать в тюрьму неверных
        liege_can_imprison_excommunicated,    // может ли глава государства сажать в тюрьму отлученных
        liege_can_imprison_freely,            // может ли глава государства сажать в тюрьму всех подряд
        liege_can_execute,                    // может ли глава государства в принципе казнить (должен быть в тюрьме) (иначе нужно чтобы совет рассмотрел)
        liege_can_execute_infidel,            // может ли глава государства казнить неверных
        liege_can_execute_excommunicated,     // может ли глава государства казнить отлученных
        liege_can_execute_freely,             // может ли глава государства казнить всех подряд
        liege_can_banish,                     // может ли глава государства в принципе высылать из страны (нужен повод) (иначе нужно чтобы совет рассмотрел)
        liege_can_banish_infidel,             // может ли глава государства высылать неверных
        liege_can_banish_excommunicated,      // может ли глава государства высылать отлученных
        liege_can_banish_freely,              // может ли глава государства высылать всех подряд
        liege_can_demand_religious_conversion, // глава государства может потребовать сменить религию
        liege_can_apply_to_the_tribunal,      // может ли глава государства обратиться в суд
        vassal_titles_goes_back_to_the_liege, // титулы вассалов (например если они умерли и не оставили наследников) возвращаются главе государства
        // тип выборов
        liege_elected_normally,               // все вассалы голосуют за главу
        liege_elected_random,                 // выборы настолько сложные что глава выбирается почти случайно (нужно придумать пару механик (например денежные донаты увеличивают шанс))
        liege_elected_thru_tournament,        // выборы главы происходят посредством турнира (ритуального мордобоя или чего то подобного)
        // тип наследования (не для выборов)
        oldest_succession,                    // из доступных людей для наследования выбирается старший
        youngest_succession,                  // из доступных людей для наследования выбирается младший
        powerful_succession,                  // из доступных людей для наследования выбирается самый сильный (по титулам)
        // из кого выбирают
        liege_elected_from_liege_family,      // глава государства избираем из королевской семьи
        liege_elected_from_liege_extended_family, // расширенная семья
        liege_elected_from_liege_dynasty,     // глава государства избираем из королевской династии 
        liege_elected_from_liege_courtiers,   // глава государства избираем из королевской двора (полезно для священников, они выбирают наследника из своего двора)
        liege_elected_from_vassals,           // элективная монархия, все вассалы могут избирать (скорее всего ситуация с малым количество вассалов)
        liege_elected_from_electors,          // элективная монархия, избирают только электоры (они либо назначаются главой, либо избираются, либо назначаются советом)
        liege_elected_from_council,           // глава может быть избран из членов парламента (кто учавствует в выборах? если есть электоры, то электоры выбирают из членов совета)
        liege_elected_from_tribunal,          // глава может быть избран из членов суда (по аналогии с советом)
        // вассал_может_стать_электором - не оч полезно когда есть vassal_can_become_an_elector
//         elector_elected_from_vassals,         // электоры избираются из вассалов (для каждого электора по раунду избрания?) (возможно как то стоит ограничить возможных электоров (например только старейшены))
        elector_can_become_a_liege,           // сам электор может стать правителем, и соответственно проголосовать сам за себя
        elector_elected_from_liege_family,    // электорами могут ставится только близкие родственники текущего правителя
        elector_elected_from_liege_extended_family, // более широкий круг родственников
        elector_elected_from_liege_dynasty,   // вся династия
//         elector_elected_from_liege_courtiers, // электорами могут быть и придворные (вообще то есть courtier_can_become_an_elector)
        // гендерные правила наследования
        agnatic_gender_law,                   // только мужчины могут наследовать
        agnatic_cognatic_gender_law,          // женщины только если нет подходящих наследников мужчин
        cognatic_gender_law,                  // абсолютно равное наследование
        enatic_cognatic_gender_law,           // мужчины только если нет подходящих наследников женщин
        enatic_gender_law,                    // только женщины могут наследовать
        // если есть два последних то нужно продублировать права и мужчинам
        // права вассалов, с наследованием не очевидно
        vassal_can_inherit_imperial_titles,   // назначаемый или избираемый вассал (чиновник) с имперским титулом (планирую добавить титул выше как благословление Апати (?))
        vassal_can_inherit_king_titles,       // назначаемый или избираемый вассал (чиновник) с королевским титулом
        vassal_can_inherit_duchy_titles,      // назначаемый или избираемый вассал (чиновник) с герцогским титулом
        vassal_can_become_a_hero, 
        vassal_can_become_a_diplomat,         // вассал может "договориться" перейти на другую строну побывав в другой стране (эвенты?)
        vassal_can_become_a_spymaster,
        vassal_can_become_a_merchant,
        // какая то третья должность должна быть
        vassal_can_become_a_general,
        vassal_can_become_an_elector,         // если запрещено, то это в принципе не эллективная монархия
        vassal_can_become_a_councillor,       // можно открыто запретить вассалам собираться в совет (элемент тирании)
        vassal_can_become_a_magistrate,       // кто то из вассалов может стать судьей (как? выборы?)
//         vassal_can_become_a_functionary,      // кто может запретить вассалу становится чиновником? ну вообще наверное запрет может существовать
        vassal_can_apply_to_the_tribunal,
        vassals_can_attack_external_enemies,  // вассалы могут обявлять войну внешним соверникам
        vassals_can_attack_internal_enemies,  // вассалы могут обявлять войну внутренним соперникам
        vassals_can_use_only_mercs_in_internal_wars, // во внутренних войнах вассалы могут использовать только наемников
        vassals_can_refuse_to_attack_external_enemies, // если этот закон отменен, то вассалы расматриваются как изменники ("преступники") если не вступают в войну 
        // права придворных главы государства
        courtier_can_become_a_hero,
        courtier_can_become_a_diplomat,      // можно потерять придворного таким образом
        courtier_can_become_a_spymaster,
        courtier_can_become_a_merchant,
        courtier_can_become_a_general,
        courtier_can_become_an_elector,       // придворный электор - это скорее всего формально элективная монархия (может быть эвенты с тем что мы хотим быть похожими на другую страну)
        courtier_can_become_a_councillor,     // на совет собираемый из придворных можно надавить силовым методом (по сути аналог светского парламента сейчас, где члены парламента - обычные чиновники)
        courtier_can_become_a_magistrate,     // кто то из придворных может стать судьей (назначение?)
        courtier_can_become_a_clergyman,      // может ли придворный стать священником
//         courtier_can_become_a_functionary,
        courtier_can_apply_to_the_tribunal,
        courtier_can_become_a_noble,
        // права дворян (простолюдин становится дворянином если у него появляется титул либо его специально назначают дворянином)
        noble_can_become_a_hero,              // если не дворянин то кто? тут наверное запрет во всем государстве на героев (например что то религиозное)
        noble_can_become_a_diplomat,
        noble_can_become_a_spymaster,
        noble_can_become_a_merchant,
        noble_can_become_a_general,           // тут совсем бред получается, 
        noble_can_become_an_elector,
        noble_can_become_a_councillor,
        noble_can_become_a_magistrate,
        noble_can_become_a_clergyman,
//         noble_can_become_a_functionary,
        noble_can_apply_to_the_tribunal,
        // права чиновников (это особый тип придворных, им выплачивается зарплата, их нужно набрать для некоторых государственных механик)
        // некоторые персонажи хотят устроить своих детей на эту работу, у чиновников могут быть какие то функции
        // вместо чиновников нужно использовать магистратов на примерно ту же механику
        // но штука с министерствами была бы неплохой механикой, но тогда потом
//         functionary_can_become_a_hero,
//         functionary_can_become_a_diplomat,
//         functionary_can_become_a_spymaster,
//         functionary_can_become_a_merchant,
//         functionary_can_become_a_general,
//         functionary_can_become_an_elector,
//         functionary_can_become_a_councillor,
//         functionary_can_become_a_magistrate,
//         functionary_can_become_a_clergyman,
//         functionary_can_apply_to_the_tribunal,
        // права женщин
        woman_can_become_a_hero,
        woman_can_become_a_diplomat,
        woman_can_become_a_spymaster,
        woman_can_become_a_merchant,
        woman_can_become_a_general,
        woman_can_become_an_elector,          // предубеждение к женщинам может не дать нам назначить или избрать электора
        woman_can_become_liege,               // вассалы или совет не могут избрать главу женщину (но может быть унаследован)
        woman_can_become_a_councillor,
        woman_can_become_a_magistrate,
        woman_can_become_a_noble,
        woman_can_become_a_clergyman,
//         woman_can_become_a_functionary,
        woman_can_apply_to_the_tribunal,
        woman_ruler_opinion_penalty,          // по идее может быть указан как отдельное число
        // права мужчин
        man_can_become_a_hero,
        man_can_become_a_diplomat,
        man_can_become_a_spymaster,
        man_can_become_a_merchant,
        man_can_become_a_general,
        man_can_become_an_elector,            // предубеждение к мужчинам может не дать нам назначить или избрать электора
        man_can_become_liege,                 // вассалы или совет не могут избрать главу мужчину (но может быть унаследован)
        man_can_become_a_councillor,
        man_can_become_a_magistrate,
        man_can_become_a_noble,
        man_can_become_a_clergyman,
//         man_can_become_a_functionary,
        man_can_apply_to_the_tribunal,
        man_ruler_opinion_penalty,
        // права представителей религии (регилия потенциально может быть параллельной силой в государстве + священники могут управлятся вообще из другого государства (папство))
        clergyman_can_become_a_hero,
        clergyman_can_become_a_diplomat,
        clergyman_can_become_a_spymaster,
        clergyman_can_become_a_merchant,
        clergyman_can_become_a_general,
        clergyman_can_become_an_elector,
        clergyman_can_become_liege,
        clergyman_can_become_a_councillor,
        clergyman_can_become_a_magistrate,
        // если духовникам запрещено становится дворянами, то запрещено ли дворянам становится священниками?
        // тут скорее речь идет не про запрет, а про возможность, 
        // то есть дворянин становится священником такое происходит относительно часто, а вот в обратную сторону нет
        clergyman_can_become_a_noble,           
//         clergyman_can_become_a_functionary,
        clergyman_can_appoint_a_heir,           // духовник может назначить наследника
        clergyman_status_is_inheritable,        // духовный статус наследуемый
        clergyman_can_apply_to_the_tribunal,
        
        // права совета
        // выборы в совет это уже кажется из эпохи всеобщего избирательного права (нинужно)
        council_exist,                         // совет формально существует
        council_can_revoke_title,
        council_can_revoke_title_from_criminal,
        council_can_revoke_title_from_infidel,
        council_can_revoke_title_from_excommunicated,
        council_can_give_title,
        council_can_give_title_to_infidel,
        council_can_give_title_to_excommunicated,
        council_can_give_title_to_pleb,        // совет может дать титул плебею
        council_can_appoint_an_elector,
        council_can_appoint_the_liege,         // это уже по сути республика
        council_can_appoint_a_noble,            // может ли совет давать дворянский титул плебею
        council_can_appoint_a_hero,             // может ли совет сделать кого то героем
        council_can_appoint_a_general,          // может ли совет сделать кого то генералом
        council_can_appoint_a_clergyman,        // может ли совет сделать кого то священником
        council_can_force_vassal_to_return_a_title, // парламент может заставить вернуть титул
        council_can_declare_war,               // совет голосует за вступление в войну, а король может воспользоваться ветом
        council_can_enact_laws,                // может ли совет изменить закон
//         council_can_propose_laws,              // какой смысл с enact_law?
//         council_can_revoke_laws,
        council_can_interfere_in_vassal_realm, // совет вмешивается в законы вассалов (редко)
        council_can_appoint_a_magistrate,      // может ли совет назначить судью
        council_can_imprison,                  // может ли совет в принципе сажать в тюрьму (нужен для этого повод) (иначе через суд например)
        council_can_imprison_infidel,          // может ли совет сажать в тюрьму неверных (самостоятельно принимать решение)
        council_can_imprison_excommunicated,   // может ли совет сажать в тюрьму отлученных
        council_can_imprison_freely,           // может ли совет сажать в тюрьму всех подряд
        council_can_execute,                   // может ли совет в принципе казнить (должен быть в тюрьме) (иначе через суд например)
        council_can_execute_infidel,           // может ли совет казнить неверных
        council_can_execute_excommunicated,    // может ли совет казнить отлученных
        council_can_execute_freely,            // может ли совет казнить всех подряд
        council_can_banish,                    // может ли совет в принципе высылать из страны (нужен повод) (иначе через суд например)
        council_can_banish_infidel,            // может ли совет высылать неверных
        council_can_banish_excommunicated,     // может ли совет высылать отлученных
        council_can_banish_freely,             // может ли совет высылать всех подряд
        council_has_the_veto_right,            // обладает ли совет правом заблокировать закон
        council_position_is_inheritable,
        council_member_can_appoint_a_heir,     // парламентарий может назначить наследника на это место
        council_member_can_apply_to_the_tribunal, // может ли парламентарий обратиться в суд
        council_can_demand_religious_conversion, // совет может потребовать сменить религию
        vassal_titles_goes_back_to_the_council, // титулы вассалов (например если они умерли и не оставили наследников) возвращаются в совет
        council_elected_from_vassals,          // может быть ограничение на количество мест в совете
        council_elected_from_electors,         // совет избирается из электоров
        
        // права суда
        tribunal_exist,                        // суд формально существует
        tribunal_can_revoke_title,
        tribunal_can_revoke_title_from_criminal,
        tribunal_can_revoke_title_from_infidel,
        tribunal_can_revoke_title_from_excommunicated,
        tribunal_can_give_title,
        tribunal_can_give_title_to_infidel,
        tribunal_can_give_title_to_excommunicated,
        tribunal_can_give_title_to_pleb,       // в суд можно обратиться с тем чтобы дать титул плебею (а может и сам плебей обратиться за этим)
        tribunal_can_force_vassal_to_return_a_title,
        tribunal_can_interfere_in_vassal_realm,
        tribunal_can_imprison,                 // может ли суд в принципе сажать в тюрьму (нужен для этого повод)
        tribunal_can_imprison_infidel,         // может ли суд сажать в тюрьму неверных (можно ли обратиться в суд с этой просьбой)
        tribunal_can_imprison_excommunicated,  // может ли суд сажать в тюрьму отлученных
        tribunal_can_execute,                  // может ли суд в принципе казнить (должен быть в тюрьме)
        tribunal_can_execute_infidel,          // может ли суд казнить неверных
        tribunal_can_execute_excommunicated,   // может ли суд казнить отлученных
        tribunal_can_banish,                   // может ли суд в принципе высылать из страны (нужен повод)
        tribunal_can_banish_infidel,           // может ли суд высылать неверных
        tribunal_can_banish_excommunicated,    // может ли суд высылать отлученных
        tribunal_can_declare_war,              // может ли суд начать войну
        tribunal_can_enact_laws,               // может ли суд принять закон
        tribunal_can_appoint_a_noble,          // может ли суд давать дворянский титул плебею (можно ли обратиться в суд за этим)
        tribunal_can_appoint_a_hero,           // может ли суд сделать кого то героем
        tribunal_can_appoint_a_general,        // может ли суд сделать кого то генералом
        tribunal_can_appoint_a_clergyman,      // может ли суд сделать кого то священником
        tribunal_have_the_veto_right,          // можно обратиться в суд за отменой закона
//         tribunal_can_revoke_laws,
        tribunal_position_is_inheritable,
        tribunal_can_force_religious_conversion, // суд может зафорсить смену религии
        magistrate_can_appoint_a_heir,         // судья может назначить наследника
        tribunal_elected_from_vassals,         // суд избирается из вассалов, которые не состоят в совете и не являются электорами
        tribunal_elected_from_council,         // суд избирается из вассалов, которые состоят в совете
        tribunal_elected_from_electors,        // суд избирается из электоров
        
        // возможности духовенства (если духовенство не совет, не суд, не глава, то это отдельная сила)
        // в духовенство не попасть если персонаж не священник, статус священника может быть как обычным статусом без особый бонусов, 
        // так и наделять человека разными послаблениями, ограничениями и обязаностями
//         clergy_independent,                    // независимые в том плане что не глава, не совет и не трибунал (то есть обладает своими полномочиями)
        clergy_is_liege,                       // обладает полномочиями главы (действуют по законам главы государства)
        clergy_is_council,                     // обладает полномочиями совета
        clergy_is_tribunal,                    // обладает полномочиями трибунала
        clergy_administration_is_abroad,       // духовенство управляется из другой страны (папство)
        // некоторые права у духовенства если оно независимо
        clergy_is_authoritarian,               // один главный представитель среди духовенства, работает как "абсолютный" правитель
        clergy_is_collective,                  // духовенство коллективный орган, работает как совет (сколько членов?)
        clergy_is_elective,                    // избираемое духовенство (избираемое из кого? из священников, кем? хороший вопрос), если не избираемое то что?
        clergy_elects_from_unlanded_clergyman, // духовенство избираемо только из безземельных священников (избираемо или наследуемо и проч)
        clergy_elects_from_landed_clergyman,   // духовенство избираемо только из священников с землей
        clergy_position_is_inheritable,        // дети могут получить место в духовенстве
        // как получают титулы духовенство? что с ними делают? может ли духовенство их передать?
        // я так подозреваю операции с титулами должны быть ограничены у духовенства
        clergy_can_revoke_title,
        clergy_can_revoke_title_from_criminal,
        clergy_can_revoke_title_from_infidel,
        clergy_can_revoke_title_from_excommunicated,
        clergy_can_give_title,
        clergy_can_give_title_to_infidel,
        clergy_can_give_title_to_excommunicated,
        clergy_can_give_title_to_pleb,        // совет может дать титул плебею
        clergy_can_appoint_an_elector,
        clergy_can_appoint_the_liege,         // духовенство может назначить правителя, либо это может быть благословлением
        clergy_can_appoint_a_noble,           // может ли духовенство давать дворянский титул плебею
        clergy_can_appoint_a_hero,            // может ли духовенство сделать кого то героем
        clergy_can_appoint_a_general,         // может ли духовенство сделать кого то генералом
        clergy_can_appoint_a_clergyman,       // может ли духовенство сделать кого то священником
        clergy_can_appoint_a_magistrate,      // может ли духовенство назначить судью
        clergy_can_force_vassal_to_return_a_title, // духовенство может заставить вернуть титул
        clergy_can_declare_war,               // вообще духовенство должно как то объявить священные войны
        clergy_can_enact_laws,                // может ли духовенство изменить закон
//         clergy_can_propose_laws,
//         council_can_revoke_laws,
        // эти вещи до права вето под большуим вопросом
        clergy_can_interfere_in_vassal_realm, // духовенство вмешивается в законы вассалов (редко)
        clergy_can_imprison,                  // может ли духовенство в принципе сажать в тюрьму (нужен для этого повод) (иначе через суд например)
        clergy_can_imprison_infidel,          // может ли духовенство сажать в тюрьму неверных (самостоятельно принимать решение)
        clergy_can_imprison_excommunicated,   // может ли духовенство сажать в тюрьму отлученных
        clergy_can_imprison_freely,           // может ли духовенство сажать в тюрьму всех подряд
        clergy_can_execute,                   // может ли духовенство в принципе казнить (должен быть в тюрьме) (иначе через суд например)
        clergy_can_execute_infidel,           // может ли духовенство казнить неверных
        clergy_can_execute_excommunicated,    // может ли духовенство казнить отлученных
        clergy_can_execute_freely,            // может ли духовенство казнить всех подряд
        clergy_can_banish,                    // может ли духовенство в принципе высылать из страны (нужен повод) (иначе через суд например)
        clergy_can_banish_infidel,            // может ли духовенство высылать неверных
        clergy_can_banish_excommunicated,     // может ли духовенство высылать отлученных
        clergy_can_banish_freely,             // может ли духовенство высылать всех подряд
        clergy_has_the_veto_right,            // обладает ли духовенство правом заблокировать закон
        clergy_can_demand_religious_conversion, // духовенство может потребовать сменить религию
        
        // права чиновничего аппарата (в широком смысле) (короче не, слишком сложно, я например не понимаю как чиновники будут принимать решения)
        // вообще все это дело может быть похоже на функции трибунала, так что чиновники нинужны, нужно от трибунала отталкиваться в этом смысле
//         cabinet_can_make_a_ministry,          // чиновничий аппарат монолитный или состоит из министерств?
//         cabinet_must_be_divided_to_ministries, // чиновники должны быть поделены на министерства
        // вообще министерства 
        
        // система
        gavelkind_inheritance,                 // равный раздел между всеми наследниками
        titles_can_be_inherited_by_someone_in_the_other_realm, // титулы могут уйти наследникам находящимся в другой юрисдикции
        
        // глава религии может как то влиять на светскую жизнь внутри государства
        // например назначать священников, причем религия влияет на всех вне зависимости от титула (точнее пытается - этому можно сопротивлятся в игре)
        // но при этом сопротивляется этому целое государство (то есть вассалы наследуют решение главы государства)
        // тут у нас появляются типы владений
//         papal_investiture,                      // глава религии ставит своих священников
        // возможно другие права нужно делать через непосредственно религию или через эвенты
        
        // светские законы
        male_polygamy,                          // мужчины могут иметь наложниц
        female_polygamy,                        // женщины могут иметь наложников
        male_polygamy_equal,                    // наложницы мужчин - это вторые жены (больше прав)
        female_polygamy_equal,                  // наложники женщин - это вторые мужья
        homosexual_marriage,                    // позволяется ли гомосексуалистам делать брак
        homosexual_can_inherit,
        blinded_can_inherit,
        excommunicated_can_inherit,
        criminal_can_inherit,
        infidel_can_inherit,
        horse_can_inherit,
        cat_can_inherit,
        dog_can_inherit,                        // "Я назначаю своего пса наследником!" безумный персонаж может иногда делать что то такое
        execution_allowed,                      // в общем по государству доступна смертная казнь
        banishment_allowed,
        divorce_allowed,
        clergyman_marriage_allowed,
        brother_sister_marriage_allowed,
        parent_child_marriage_allowed,
        uncle_niece_marriage_allowed,
        cousin_marriage_allowed,
        matrilineal_marriages_allowed,
        infidels_pays_additional_taxes,         // можно отменить религиозное предубеждение
        bastard_child_has_equals_rights,
        concubine_child_has_equal_rights,
        
        // нужно ли менять кто может голосовать? вассалы, вассалы вассалов, придворные, думаю что пока нет
        
        count
      };
      
      // + можество законов особенно ни на что не влияющих или изменяющие статы
      constexpr const uint32_t bit_container_size = ceil(double(count) / double(UINT32_WIDTH));
    }
    
    // тип главенства, наследование, как избирают, влияние, механики
    // религия определяет какие то свои законы и может пытаться их зафорсить в стране при не совпадении
    // + должны быть законы непосредственно самой религии (то есть как избирается глава религии, и что из себя представляет администрация)
    // и наверное хорошим решением будет отделить эти вещи, ко всему прочему ожидания от государств могут меняться с течением времени
    namespace religion_mechanics {
      enum values {
        // тип главенства (его может не быть)
        head_is_independent,                    // независимый от светской власти
        head_is_authoritarian,                  // неприемлит какие то отдельные течения (по идее обратный автокефалиям)
        head_is_council,                        // вместо единого главы - совет
        independence_structure_autocephaly,     // автокефалия - независимый от основной религии священик высокого сана (может примерно то же что и глава религии)
        pentarchy,                              // 5 исторических пентархов (священики-бароны) всегда считатются патриархами правителя в соответсвующем королевстве (не очень понимаю в чем прикол)
        // выборы и наследование
        head_can_inherit,                       // наследуем статус главы религии
        head_is_elected,                        // либо избираем нового главу
        priests_can_become_an_elector,          // священник может стать электором (в данном случае Кардинал)
        secular_rulers_can_become_an_elector,   // светский правитель может стать электором (и избирать папу)
        elector_elected_from_priests,           // священники избирают электоров
        elector_elected_from_secular_rulers,    // светские правители избирают электоров
        head_can_appoint_an_elector,            // электоры назначаются главой
        head_can_appoint_a_secular_rulers,      // глава религии может назначить светских правителей (пока не уверен как это будет работать)
        // что может глава религии
        papal_investiture,                      // глава религии ставит своих священников
        head_can_demand_religious_conversion,
        head_can_excommunicate,
        head_can_grant_divorce,
        head_can_grant_claim,
        head_can_call_crusade,
        head_can_call_a_holy_tribunal,          // священный суд, может быть вызван чтобы публично покарать отступника или убийцу единоверцев или еще чего
        head_can_appoint_a_caste_for_secular_ruler, // касты назначаются главой (иначе выдаются при рождении от родителей) (можно сделать случайное распределение)
        secular_ruler_caste_is_determined_random, // случайное распределение каст (можно сделать эвенты на сокрытие каст)
        // особенности (права последователей)
        can_have_antipopes,
        can_retire_to_monastery,
        can_choose_patron_deity,
        can_revoke_title_from_excommunicated,
        can_give_title_to_excommunicated,
        can_imprison_excommunicated,
        can_execute_excommunicated,
        can_banish_excommunicated,
        have_sects,
        priests_can_marry,
        priests_can_inherit,
        feminist,                               // возможно более тонкая настройка?
        pacifist,                               // ниже агрессивность у последователей
        brother_sister_marriage_allowed,
        parent_child_marriage_allowed,
        uncle_niece_marriage_allowed,
        cousin_marriage_allowed,
        matrilineal_marriages_allowed,
        female_temple_holders,                  // могут ли женщины владеть храмами
        male_temple_holders,
        castes,                                 // использует ли религия кастовую систему
        caste_opinions,                         // модификаторы к отношениям между кастами
        // викинги, в принципе может пригоджиться
        allow_viking_invasion,                  // ?
        allow_looting,                          // ?
        seafarer,                               // написано что ИИ препочитает нападать на береговые провинции
        allow_rivermovement,                    // врядли нужно
        // механики
        divine_blood,                           // священные близкородственные связи (можно ли как то мне сделать иначе)
        has_heir_designation,                   // выбор наследника
//         defensive_attrition,                    // защитные небоевые потери (для язычников например) (скорее всего не мой случай)
//         ignores_defensive_attrition,
        peace_authority_loss,
        peace_esteem_loss,
        peace_inluence_loss,
        attacking_same_religion_authority_loss,
        attacking_same_religion_esteem_loss,
        attacking_same_religion_inluence_loss,
        vassal_king_authority_bonus,             
        vassal_king_esteem_bonus,             
        vassal_king_inluence_bonus,           
        hostile_within_group,                   // аргесивность внутри религиозной группы
        ai_peaceful,                            // орда теряет супер агрессивность
        raised_vassal_opinion_loss,             // использование войск вассала портит отношения (нужно ли? нужно сделать механику передачи войск от вассалов)
        reformer_head_of_religion,              // игрок кто реформировал религию становится главой
        pre_reformed,                           // не очень понятно что делает
//         vassal_king_prestige_bonus,             // престиж от вассалов королей  
        dislike_tribal_organization,            // увеличение организованности племен злит вассалов (организованность в обычном понимании наверное будет отсутствовать)
        // механика муслимов 
        // (обозначает "обычный" мусульманский цикл правителей: правитель приходит на бедную землю, отстраивает ее, начинает жить лучше, его влияние ослабевает, его замещиет другая мусульманская семья)
        // чем лучше живут мусульмане, тем менее стабильно становится их государство... интересно
        uses_decadence,                         // это можно обыграть
        infidels_pays_additional_taxes,         // налоги на неверных
        hard_to_convert,                        // тяжело конвертировать
        men_can_take_consorts,                  // позволяет брать наложниц мужчинам
        women_can_take_consorts,                // то же самое женщинам
        
        join_crusade_if_bordering_hostile,      // ???
        rel_head_defense,                       // ии пытается защитить главу религии
        
        ai_try_to_convert_same_group,
        ai_try_to_convert_same_group_if_zealot,
        ai_try_to_convert_other_group,
        ai_try_to_convert_other_group_if_zealot,
        
        count
      };
      
      constexpr const uint32_t bit_container_size = ceil(double(count) / double(UINT32_WIDTH));
    }
    
    namespace culture_mechanics {
      enum values {
        horde,
        used_for_random,
        allow_in_ruler_designer,
        dukes_called_kings,
        baron_titles_hidden,
        count_titles_hidden,
        founder_named_dynasties,
        dynasty_title_names,
        disinherit_from_blinding,
        allow_looting,
        seafarer,
        dynasty_name_first,
        feminist,
        
        count
      };
      
      constexpr const uint32_t bit_container_size = ceil(double(count) / double(UINT32_WIDTH));
    }
    
//     struct mechanics_modifier {
//       uint32_t container;
//       
//       inline mechanics_modifier() : container(UINT32_MAX) {}
//       inline mechanics_modifier(const uint32_t &index, const bool mechanic) : container(uint32_t(mechanic) | uint32_t(index << 1)) { ASSERT(index < INT32_MAX-1); }
//       inline bool value() const { return bool(container & 0x1); }
//       inline uint32_t index() const { return uint32_t(container >> 1); }
//       inline bool valid() const { return container != UINT32_MAX; }
//     };
  }
}

#endif
