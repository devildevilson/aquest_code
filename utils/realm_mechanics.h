#ifndef REALM_MECHANICS_H
#define REALM_MECHANICS_H

#include <cstdint>
#include <cmath>

// еще должна быть возможность передачи какого то земельного титула
// например передача титула города венеции
// права страны основываются на основных титулах - это титулы верхнего уровня
// соответственно если мы получаем имперский титул 
// основным становится этот имперский титул

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
        liege_can_inherit_main_titles,        // если нет, глава государства избираемый или назначаемый
//         liege_can_inherit_imperial_titles,    // император в этом государстве избираемый или назначаемый (в зависимости от максимального титула)
//         liege_can_inherit_king_titles,        // король в этом государстве избираемый или назначаемый (в зависимости от максимального титула)
//         liege_can_inherit_duchy_titles,       // герцог в этом государстве избираемый или назначаемый (в зависимости от максимального титула)
        liege_can_revoke_laws,
        liege_can_become_a_general,           
        liege_can_become_a_hero,              // глава государства может или не может становится героем 
        liege_can_become_a_councillor,        // может ли глава государства стать консулом (членом парламента)
        liege_can_become_a_magistrate,        // глава может стать судьей (то есть решать споры вассалов?)
        liege_can_appoint_an_elector,         // может ли глава назначить электора
        liege_can_appoint_a_councillor,       // может ли глава назначить члена парламента
        liege_can_appoint_a_magistrate,       // может ли глава назначить судью
        liege_can_appoint_a_heir,             // может ли глава назначить наследника (то есть выбрать из своих детей или возможно из каких то челиков)
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
        liege_elected_from_liege_family,      // глава государства избираем из королевской семьи (дети?)
        liege_elected_from_liege_dynasty,     // глава государства избираем из королевской династии 
        liege_elected_from_liege_courtiers,   // глава государства избираем из королевской двора (полезно для священников, они выбирают наследника из своего двора)
        liege_elected_from_vassals,           // элективная монархия, все вассалы могут избирать (скорее всего ситуация с малым количество вассалов)
        liege_elected_from_electors,          // элективная монархия, избирают только электоры (они либо назначаются главой, либо избираются, либо назначаются советом)
        liege_elected_from_council,           // глава может быть избран из членов парламента (кто учавствует в выборах? если есть электоры, то электоры выбирают из членов совета)
        elector_elected_from_vassals,         // электоры избираются из вассалов (для каждого электора по раунду избрания?) (возможно как то стоит ограничить возможных электоров (например только старейшены))
        // гендерные правила наследования
        agnatic_gender_law,                   // только мужчины могут наследовать
        agnatic_cognatic_gender_law,          // женщины только если нет подходящих наследников мужчин
        cognatic_gender_law,                  // абсолютно равное наследование
        enatic_cognatic_gender_law,           // мужчины только если нет подходящих наследников женщин
        enatic_gender_law,                    // только женщины могут наследовать
        // если есть два последних то нужно продублировать права и мужчинам
        // права вассалов
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
        woman_ruler_opinion_penalty,
        // права мужчин, специально для enatic_gender_law
        man_can_become_a_hero,
        man_can_become_a_diplomat,
        man_can_become_a_spymaster,
        man_can_become_a_merchant,
        man_can_become_a_general,
        man_can_become_an_elector,            // предубеждение к мужчинам может не дать нам назначить или избрать электора
        man_can_become_liege,                 // вассалы или совет не могут избрать главу мужчину (но может быть унаследован)
        man_can_become_a_councillor,
        man_can_become_a_magistrate,
        man_ruler_opinion_penalty,
        
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
        council_can_appoint_an_elector,
        council_can_propose_laws,
        council_can_appoint_the_liege,         // это уже по сути республика
        council_can_force_vassal_to_return_a_title, // парламент может заставить вернуть титул
        council_can_declare_war,               // совет голосует за вступление в войну, а король может воспользоваться ветом
        council_can_interfere_in_vassal_realm, // совет вмешивается в законы вассалов (редко)
        council_can_revoke_laws,
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
        council_can_demand_religious_conversion, // совет может потребовать сменить религию
        vassal_titles_goes_back_to_the_council, // титулы вассалов (например если они умерли и не оставили наследников) возвращаются в совет
        liege_can_propose_war,                 // главе может быть запрещено объявлять войну
        liege_must_propose_war,                // глава может объявить войну только с одобрения совета
        liege_can_propose_laws,                // главе может быть запрещено выдвигать законы
        liege_must_propose_laws,               // глава не может принять закон без совета
        liege_has_the_veto_right,              // право вето, после голосования по закону мы можем его отменить
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
        tribunal_have_the_veto_right,          // можно обратиться в суд за отменой закона
        tribunal_can_revoke_laws,
        tribunal_position_is_inheritable,
        tribunal_can_force_religious_conversion, // суд может зафорсить смену религии
        magistrate_can_appoint_a_heir,         // судья может назначить наследника
        tribunal_elected_from_vassals,         // суд избирается из вассалов, которые не состоят в совете и не являются электорами
        tribunal_elected_from_council,         // суд избирается из вассалов, которые состоят в совете
        tribunal_elected_from_electors,        // суд избирается из электоров
        
        // система
        gavelkind_inheritance,                 // равный раздел между всеми наследниками
        titles_can_be_inherited_by_someone_in_the_other_realm, // титулы могут уйти наследникам находящимся в другой юрисдикции
        
        // глава религии может как то влиять на светскую жизнь внутри государства
        // например назначать священников, причем религия влияет на всех вне зависимости от титула (точнее пытается - этому можно сопротивлятся в игре)
        // но при этом сопротивляется этому целое государство (то есть вассалы наследуют решение главы государства)
        // тут у нас появляются типы владений
        papal_investiture,                      // глава религии ставит своих священников
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
        execution_allowed,
        banishment_allowed,
        divorce_allowed,
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
      //constexpr const uint32_t bit_container_size = std::ceil(float(count) / float(UINT32_WIDTH));
      constexpr const uint32_t bit_container_size = count / UINT32_WIDTH + 1;
    }
    
    // тип главенства, наследование, как избирают, влияние, механики
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
      
      //const uint32_t bit_container_size = std::ceil(float(count) / float(UINT32_WIDTH));
      constexpr const uint32_t bit_container_size = count / UINT32_WIDTH + 1;
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
      
      constexpr const uint32_t bit_container_size = count / UINT32_WIDTH + 1;
    }
    
    struct mechanics_modifier {
      uint32_t container;
      
      inline mechanics_modifier() : container(UINT32_MAX) {}
      inline mechanics_modifier(const uint32_t &index, const bool mechanic) : container(uint32_t(mechanic) | uint32_t(index << 1)) { ASSERT(index < INT32_MAX-1); }
      inline bool value() const { return bool(container & 0x1); }
      inline uint32_t index() const { return uint32_t(container >> 1); }
      inline bool valid() const { return container != UINT32_MAX; }
    };
  }
}

#endif
