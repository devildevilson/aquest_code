#ifndef ON_ACTION_TYPES_H
#define ON_ACTION_TYPES_H

#include <vector>
#include <cstdint>
#include <cstddef>

namespace devils_engine {
  namespace core {
    struct event;
  }
  
  namespace utils {
    namespace action_type {
      enum values {
        // система
        on_startup,                // при старте непосредственно игры (или после загрузки), вызываются для всех персонажей
        on_alternate_start,        // для каждого персонажа которые сгенерированы в случайном мире
        on_character_renamed,      // игрок сменил имя персонажа
        on_title_renamed,          // игрок сменил имя титула
        on_province_renamed,       // игрок сменил название провинции
        on_artifact_renamed,       // игрок сменил название артефакта
        on_bloodline_renamed,      // игрок сменил название крови
        on_create_chronicle_if_empty, // вызывается каждый год если летопись (хроника?) пуста (еще не создана)
        on_chronicle_owner_change, // смена владельца хроники
        on_chronicle_start,        // вызывается для игрока при старте игры (но не при загрузке сохранений)
        
        // переодические эвенты
        on_yearly_pulse,           // каждый год
        on_two_year_pulse,         // каждые 2 года
        on_five_year_pulse,        // каждые 5 лет
        on_decade_pulse,           // каждые 10 лет
        on_yearly_childhood_pulse, // каждый год для всех персонажей от 2 до 16 лет
        on_childhood_pulse,        // для детей вызываются в 6 лет (в 6 лет и 6 месяцев если быть точнее), в 8 и 10 (по тому же принципу)
        on_adolescence_pulse,      // для детей вызываются в 12 и 14 лет (+ полгода)
        on_focus_pulse,            // каждый год для персонажей с выбранным фокусом (нужно ли это мне?)
        
        // эвенты с провинцией
        on_province_major_modifier_add,    // добавляется модификатор с меткой major
        on_province_major_modifier_remove, // убирается модификатор с меткой major
        on_outbreak,               // начинается болезнь (как получить id болезни?)
        on_loot_settlement,        // при разграблении поселения
        on_loot_province,          // при грабеже провинции
        on_defect_to_rebels,       // когда провинция переходит в руки восставших
        on_defect_from_rebels,     // когда провинция освобождается от восстания
        on_settlement_looted,      // при разграблении поселения
        on_holding_building_start, // начало строительства здания в городе
        on_settlement_construction_start, // начало строительства в поселении (чем отличается от предыдущего?)
        on_settlement_construction_completed, // конец строительства
        on_trade_post_construction_start, // строительство торгового поста
        on_trade_post_construction_completed, // строительство торгового поста закончено
        on_fort_construction_start, // строительство форта
        on_fort_construction_completed, // строительство форта закончено
        on_wonder_construction_start, // старт строительства чуда
        on_wonder_destroyed,       // разрушение чуда
        on_wonder_loot_start,      // чудо пытаются разграбить
        on_wonder_owner_change,    // чудо сменило владельца
        on_wonder_renamed,         // смена имени чуда
        on_wonder_restore_finish,  // чудо восстановлено
        on_wonder_restore_start,   // начало восстановления чуда
        on_wonder_stage_finish,    // одно из этапов чуда закончено
        on_wonder_stage_loot_finish, // конец разграбления частично построенного чуда
        on_wonder_stage_loot_start, // старт разграбления частично построенного чуда
        on_wonder_upgrade_destroyed, // улучшение чуда разрушено
        on_wonder_upgrade_finish,  // улучшение чуда закончено
        on_wonder_upgrade_start,   // улучшение чуда начато
        
        // эвенты с войной
        // битва по идее у меня не будет разделена на фазы
        // но с другой стороны в самой битве какие то случайные модификаторы было бы неплохо добавить
        on_combat_pulse,           
        on_combat_starting,        // сомневаюсь что мне пригодиться
        on_siege_pulse,            // каждое некоторое время вызываются при осаде, может пригодиться
        on_battle_won,             // при победе в битве
        on_major_battle_won,       // при победе в крупной битве (какой?)
        on_battle_won_leader,      // в цк2 эвент вызывался для других таргетов, мне возможно не нужно
        on_major_battle_won_leader, // см. выше
        on_battle_won_owner,       // см. выше
        on_battle_lost,            // проигрыш в битве
        on_major_battle_lost,      // проигрыш в крупной битве
        on_battle_lost_leader,     // в цк2 эвент вызывался для других таргетов, мне возможно не нужно
        on_major_battle_lost_leader, // см. выше
        on_battle_lost_owner,      // см. выше
        on_siege_won_leader,       // победа при осаде (у меня скорее всего будет разделены осада и штурм)
        on_siege_won_leader_fort,  // победа при осаде форта
        on_siege_won_leader_trade_post, // победа при осаде торгового поста (будет ли что то такое?)
        on_siege_lost_leader,      // проигрыш осады (передача города в другие руки)
        on_siege_lost_leader_fort, // проигрыш осады форта
        on_siege_lost_leader_trade_post, // проигрыш осады торгового поста
        on_siege_over_winner,      // вызывается для победителя при осаде
        on_siege_over_winner_fort, // вызывается для победителя при осаде форта
        on_siege_over_winner_trade_post, // вызывается для победителя при осаде торгового поста
        on_siege_over_loc_chars,   // вызывается для всях персонажей, которые предположительно находились в поселении
        on_siege_over_loc_chars_fort, // вызывается для всях персонажей, которые предположительно находились в провинции в которой находился форт
        on_siege_over_loc_chars_trade_post, // вызывается для всях персонажей, которые предположительно находились в провинции в которой находился торговый пост
        on_prepared_invasion_monthly, // каждый месяц для персонажа который готовит вторжение
        on_prepared_invasion_aborts, // если вторжение отменяется (при смерти готовившего что ли?)
        on_prepared_invasion_expires, // персонаж в итоге не воспользовался вторжением
        on_merc_rampage,           // эвент связаный с наемниками (судя по всему теряют голову в бою)
        on_merc_leave,             // эвент связаный с наемниками (судя по всему сбегают из боя)
        on_merc_turn_coat_from,    // эвент связаный с наемниками (судя по всему меняют сторону (?))
        on_merc_turn_coat_to,      // эвент связаный с наемниками (судя по всему меняют сторону (?))
        on_player_mercenary_income, // ??? (может быть проверка что у игрока достаточно денег?)
        on_holy_order_leave,       // сбегает орден (орден госпитальеров например)
        on_warleader_death,        // не используется в цк2 (предположительно вызыватся при смерти правителя)
        on_war_started,            // старт войны
        on_war_ended_victory,      // победа в войне, начатой персонажем
        on_war_ended_invalid,      // условия войны не выполняются (наверное когда была начата из-за слабых претензий и персонаж умер)
        on_war_ended_whitepeace,   // белый мир в войне
        on_war_ended_defeat,       // проигрыш в войне начатой персонажем
        on_ai_end_raid,            // ИИ закончил рейдить провинцию
        on_mercenary_hired,        // наемники наняты
        on_mercenary_dismissed,    // наемники распущены
        on_mercenary_captain_replacement, // смена командира наемников
        on_unit_entering_province, // вызывается для всех персонажей которые входят в провинцию (в качестве войска)
        on_command_unit,           // вызывается для персонажей когда их ставят управлять флангом
        on_command_subunit,        // вызывается для персонажей когда их ставят управлять подъюнитом (?)
        
        // заговоры
        on_failed_assassination,   // провал заговора
        on_failed_assassination_disc, // провал заговора (чем отличается?)
        on_assassination,          // успешный заговор
        on_assassination_disc,     // успешный заговор (чем отличается?)
        
        // персонаж
        on_birth,                  // при рождении ребенка
        on_adulthood,              // перед непосредственным становлением взрослым
        on_post_birth,             // после рождения (on_birth возможно используется для матери)
        on_pregnancy,              // вызывается на втором месяце беременности
        on_marriage,               // при женитьбе (вызывается для двух владык супругов)
        on_betrothal,              // при обручении (вызывается для двух владык обрученных)
        on_become_imprisoned_any_reason, // вызывается при заключении персонажа в тюрьму (должны храниться указатели на заключенного и того кто это сделал)
        on_avoided_imprison_started_war, // вызывается если была попытка посадить в тюрьму кого то с землей и тот избежал этого
        on_became_imprisoned,      // вызывается при заключении персонажа в тюрьму (указано что это при diplo-action, я так понимаю имеются ввиду desicions)
        on_avoided_imprison_fled_country, // попытка заключить в тюрьму кого то без земли, персонаж сбегает в другую страну
        on_released_from_prison,   // при освобождении из тюрьмы
        on_executed,               // вызывается при казнях (до казни или после?)
        on_exiled,                 // вызывается при ссылке персонажа 
        on_death,                  // вызывается непосредственно перед наследованием
        on_forced_consort,         // правитель заставляет стать кого то своим наложником/наложницей (из тюрьмы)
        on_become_doge,            // вызывается при переизбрании дожа (главы республики я так понимаю)
        on_elective_gavelkind_succession, // равный раздел (между детьми правителя) с выбором кому пойдет основной титул, вызывается для всех вассалов после наследования
        on_create_title,           // при создании титула 
        on_new_holder,             // при смене владельца титула города 
        on_new_holder_inheritance, // при наследовании титула города 
        on_new_holder_usurpation,  // при узурпации титула города (тюрьма?)
        on_character_convert_religion, // персонаж меняет религию
        on_character_convert_secret_religion, // персонаж обращается в религию в которую он верил в тайне
        on_character_convert_culture, // персонаж меняет культуру
        on_acquire_nickname,       // персонаж получает прозвище
        on_over_vassal_limit_succession, // вызывается для вассалов которые могу стать независимыми из-за того что у правителя переизбыток вассалов (больше лимита вассального)
        on_divorce,                // при разводе
        on_feud_started,           // старт междоусобицы (то есть восстания?)
        on_feud_ended,             // конец междоусобицы
        on_blood_brother_death,    // смерть кровного брата (сестры?)
        on_artifact_inheritance,   // при наследовании артефакта (вызывается для каждого артефакта)
        on_tyranny_gained,         // вызывается для каждого персонажа у которых портится отношения с тираном (в цк2 вызывается скорее всего где то в коде)
        on_tyranny_gained_tyrant_only, // вызывается для тирана при попытках тирании (?)
        on_revoke_attempted_started_war, // вызывается когда персонаж отказывается передавать титул и начинает войну
        on_retract_vassal_attempted_started_war, // вызывается когда персонаж отказывается вступать в войну и сам объявляет войну господину
        on_absorb_clan_attempted_started_war, // персонаж отказывается при попытке погложения клана (кочевники) и объявляет войну
        on_split_clan_attempted_started_war, // персонаж отказывается при попытке разделения клана (кочевники) и объявляет войну
        on_employer_change,        // смена "работодателя" (вики пишет что так можно ловить вновь созданных персонажей, но я по не понимаю что конкретно это означает)
        on_host_change,            // смена хоста (?)
        
        // страна
        on_approve_law,            // ответ на предложенное изменение в де факто законе (вызывается для правителя и титула, не понимаю что конкретно)
        on_approve_de_jure_law,    // ответ на предложенное изменение в де юре законе (вызывается для правителя и титула, не понимаю что конкретно)
        on_rebel_revolt,           // вызывается для провинции в которых начинается востание
        on_enforce_peace,          // механика примирения (я так понимаю кнопка в совете)
        on_enforce_peace_start,    // начало примирения (?)
        on_enforce_peace_six_vassals, // вызывается для всех вассалов в совете (?)
        on_law_vote_passed,        // успешное голосование по закону
        on_law_vote_failed,        // неуспешное голосование по закону
        
        // крестовые походы
        on_crusade_creation,       // созывается крестовый поход, вызывается для главы религии, атакуемого титула и атакуемой страны
        on_crusade_invalid,        // невозможно продолжить крестовый поход
        on_crusade_success,        // успешный крестовый поход
        on_crusade_failure,        // крестовый поход провалился
        on_crusade_preparation_starts, // вызывается для каждого крестового похода при подготовке
        on_crusade_preparation_ends, // вызывается для каждого крестового похода при конце подготовки
        on_crusade_canceled,       // вызывается при отмене похода
        on_crusade_monthly,        // каждый месяц когда крестовый поход активен
        on_crusade_target_changes, // вызывается когда цеть похода изменяется (например титул наследуется)
        on_pledge_crusade_participation, // персонаж присоединяется к крестовому походу
        on_pledge_crusade_defense, // персонаж присоединяется к защите от крестового похода
        on_unpledge_crusade_participation, // персонаж больше не участвует в походе (например умер или сменил религию)
        on_unpledge_crusade_defense, // персонаж больше не участвует в защите от похода (например умер или сменил религию)
        
        // религия
        on_reform_religion,        // при реформировании религии
        on_county_religion_change, // при изменении религии в провинции (?)
        on_vassal_accepts_religious_conversion, // персонаж принимает предложение сменить религию, вызывается для вассала и для всех персонажей при дворе этого вассала и для всех вассалов этого вассала
        on_heresy_takeover,        // ересь становится основной религией, заменяя предыдущий уклад (вызывается для всех персонажей?)
        on_rel_elector_chosen,     // после выборов кардинала 
        on_rel_head_chosen,        // после выборов Папы римского (или подобного главы религии)
        on_excommunicate_interaction, // персонаж отлучен
        
        // юниты (армии?)
        on_entering_port,          // корабли заходят в порт
        on_navy_returns_with_loot, // корабли возвращаются с добычей
        
        // сообщество
        on_society_two_year_pulse, // вызывается для персонажей в тайном обществе каждые два года
        on_society_created,        // кто то вступает в общество в котором нет членов
        on_society_destroyed,      // последний член общества покидает его
        on_society_failed_to_find_new_leader, // общество не может найти подходящего лидера
        on_society_progress_full,  // прогресс по заданию общества (в описании написано странно "increased/set to the value 100" или то или то что ли?)
        on_society_progress_zero,  // прогресс по заданию общества (в описании написано странно "decreased/set to the value 0" или то или то что ли?)
        
        // сила на карте
        on_offmap_policy_changed,  // offmap - это специальные сущности которые моделируют безземельные титулы, например авантюристы или крестьянские восстания ...
        on_offmap_status_changed,  // одна из таких сил - китай, политика (открытость, изоляционизм и проч) и статус (гражданская война, спокойствие и проч) ...
        on_offmap_governor_changed, // показываются в специальном окошке, эти эвенты отвечают за разные вещи которые могут произойти с этой силой (смена губернатора? доверенного лица?)
        on_offmap_ruler_changed,   // смена правителя в offmap power
        on_offmap_monthly_pulse,   // ежемесячный вызов эвентов для offmap power
        on_offmap_yearly_pulse,    // ежегодичный вызов эвентов для offmap power
        
        count
      };
    }
    
    struct on_action_container {
      struct data {
        struct complex_event {
          const core::event* ev;
          uint32_t delay;
          float probability;
        };
        
        std::vector<const core::event*> events;
        std::vector<complex_event> complex_events;
        // эффекты?
      };
      
      struct data data[action_type::count];
    };
  }
}

#endif
