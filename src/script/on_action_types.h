#ifndef DEVILS_ENGINE_SCRIPT_ON_ACTION_TYPES_H
#define DEVILS_ENGINE_SCRIPT_ON_ACTION_TYPES_H

#define ACTION_TYPES_LIST \
  /* система */ \
  ACTION_TYPE_FUNC(on_startup)                /* при старте непосредственно игры (или после загрузки), вызываются для всех персонажей */ \
  ACTION_TYPE_FUNC(on_alternate_start)        /* для каждого персонажа которые сгенерированы в случайном мире */ \
  ACTION_TYPE_FUNC(on_character_renamed)      /* игрок сменил имя персонажа */ \
  ACTION_TYPE_FUNC(on_title_renamed)          /* игрок сменил имя титула */ \
  ACTION_TYPE_FUNC(on_province_renamed)       /* игрок сменил название провинции */ \
  ACTION_TYPE_FUNC(on_artifact_renamed)       /* игрок сменил название артефакта */ \
  ACTION_TYPE_FUNC(on_bloodline_renamed)      /* игрок сменил название крови */ \
  ACTION_TYPE_FUNC(on_create_chronicle_if_empty) /* вызывается каждый год если летопись (хроника?) пуста (еще не создана) */ \
  ACTION_TYPE_FUNC(on_chronicle_owner_change) /* смена владельца хроники */ \
  ACTION_TYPE_FUNC(on_chronicle_start)        /* вызывается для игрока при старте игры (но не при загрузке сохранений) */ \
  /* переодические эвенты */ \
  ACTION_TYPE_FUNC(on_yearly_pulse)           /* каждый год */ \
  ACTION_TYPE_FUNC(on_two_year_pulse)         /* каждые 2 года */ \
  ACTION_TYPE_FUNC(on_five_year_pulse)        /* каждые 5 лет */ \
  ACTION_TYPE_FUNC(on_decade_pulse)           /* каждые 10 лет */ \
  ACTION_TYPE_FUNC(on_yearly_childhood_pulse) /* каждый год для всех персонажей от 2 до 16 лет */ \
  ACTION_TYPE_FUNC(on_childhood_pulse)        /* для детей вызываются в 6 лет (в 6 лет и 6 месяцев если быть точнее), в 8 и 10 (по тому же принципу) */ \
  ACTION_TYPE_FUNC(on_adolescence_pulse)      /* для детей вызываются в 12 и 14 лет (+ полгода) */ \
  ACTION_TYPE_FUNC(on_focus_pulse)            /* каждый год для персонажей с выбранным фокусом (нужно ли это мне?) */ \
  /* эвенты с провинцией */ \
  ACTION_TYPE_FUNC(on_province_major_modifier_add)    /* добавляется модификатор с меткой major */ \
  ACTION_TYPE_FUNC(on_province_major_modifier_remove) /* убирается модификатор с меткой major */ \
  ACTION_TYPE_FUNC(on_outbreak)                /* начинается болезнь (как получить id болезни?) */ \
  ACTION_TYPE_FUNC(on_loot_settlement)         /* при начале разграбления поселения */ \
  ACTION_TYPE_FUNC(on_loot_province)           /* при грабеже провинции */ \
  ACTION_TYPE_FUNC(on_defect_to_rebels)        /* когда провинция переходит в руки восставших */ \
  ACTION_TYPE_FUNC(on_defect_from_rebels)      /* когда провинция освобождается от восстания */ \
  ACTION_TYPE_FUNC(on_settlement_looted)       /* при разграблении поселения */ \
  ACTION_TYPE_FUNC(on_holding_building_start)  /* начало строительства здания в городе */ \
  ACTION_TYPE_FUNC(on_settlement_construction_start)     /* начало строительства в поселении (чем отличается от предыдущего?) */ \
  ACTION_TYPE_FUNC(on_settlement_construction_completed) /* конец строительства */ \
  ACTION_TYPE_FUNC(on_trade_post_construction_start)     /* строительство торгового поста */ \
  ACTION_TYPE_FUNC(on_trade_post_construction_completed) /* строительство торгового поста закончено */ \
  ACTION_TYPE_FUNC(on_fort_construction_start)     /* строительство форта */ \
  ACTION_TYPE_FUNC(on_fort_construction_completed) /* строительство форта закончено */ \
  ACTION_TYPE_FUNC(on_wonder_construction_start)   /* старт строительства чуда */ \
  ACTION_TYPE_FUNC(on_wonder_destroyed)         /* разрушение чуда */ \
  ACTION_TYPE_FUNC(on_wonder_loot_start)        /* чудо пытаются разграбить */ \
  ACTION_TYPE_FUNC(on_wonder_owner_change)      /* чудо сменило владельца */ \
  ACTION_TYPE_FUNC(on_wonder_renamed)           /* смена имени чуда */ \
  ACTION_TYPE_FUNC(on_wonder_restore_finish)    /* чудо восстановлено */ \
  ACTION_TYPE_FUNC(on_wonder_restore_start)     /* начало восстановления чуда */ \
  ACTION_TYPE_FUNC(on_wonder_stage_finish)      /* одно из этапов чуда закончено */ \
  ACTION_TYPE_FUNC(on_wonder_stage_loot_finish) /* конец разграбления частично построенного чуда */ \
  ACTION_TYPE_FUNC(on_wonder_stage_loot_start)  /* старт разграбления частично построенного чуда */ \
  ACTION_TYPE_FUNC(on_wonder_upgrade_destroyed) /* улучшение чуда разрушено */ \
  ACTION_TYPE_FUNC(on_wonder_upgrade_finish)    /* улучшение чуда закончено */ \
  ACTION_TYPE_FUNC(on_wonder_upgrade_start)     /* улучшение чуда начато */ \
  /* эвенты с войной */ \
  /* битва по идее у меня не будет разделена на фазы */ \
  /* но с другой стороны в самой битве какие то случайные модификаторы было бы неплохо добавить */ \
  ACTION_TYPE_FUNC(on_combat_pulse)           \
  ACTION_TYPE_FUNC(on_combat_starting)        /* сомневаюсь что мне пригодиться */ \
  ACTION_TYPE_FUNC(on_siege_pulse)            /* каждое некоторое время вызываются при осаде, может пригодиться */ \
  ACTION_TYPE_FUNC(on_battle_won)             /* при победе в битве */ \
  ACTION_TYPE_FUNC(on_major_battle_won)       /* при победе в крупной битве (какой?) */ \
  ACTION_TYPE_FUNC(on_battle_won_leader)      /* в цк2 эвент вызывался для других таргетов, мне возможно не нужно */ \
  ACTION_TYPE_FUNC(on_major_battle_won_leader) /* см. выше */ \
  ACTION_TYPE_FUNC(on_battle_won_owner)       /* см. выше */ \
  ACTION_TYPE_FUNC(on_battle_lost)            /* проигрыш в битве */ \
  ACTION_TYPE_FUNC(on_major_battle_lost)      /* проигрыш в крупной битве */ \
  ACTION_TYPE_FUNC(on_battle_lost_leader)     /* в цк2 эвент вызывался для других таргетов, мне возможно не нужно */ \
  ACTION_TYPE_FUNC(on_major_battle_lost_leader) /* см. выше */ \
  ACTION_TYPE_FUNC(on_battle_lost_owner)      /* см. выше */ \
  ACTION_TYPE_FUNC(on_siege_won_leader)       /* победа при осаде (у меня скорее всего будет разделены осада и штурм) */ \
  ACTION_TYPE_FUNC(on_siege_won_leader_fort)  /* победа при осаде форта */ \
  ACTION_TYPE_FUNC(on_siege_won_leader_trade_post) /* победа при осаде торгового поста (будет ли что то такое?) */ \
  ACTION_TYPE_FUNC(on_siege_lost_leader)      /* проигрыш осады (передача города в другие руки) */ \
  ACTION_TYPE_FUNC(on_siege_lost_leader_fort) /* проигрыш осады форта */ \
  ACTION_TYPE_FUNC(on_siege_lost_leader_trade_post) /* проигрыш осады торгового поста */ \
  ACTION_TYPE_FUNC(on_siege_over_winner)      /* вызывается для победителя при осаде */ \
  ACTION_TYPE_FUNC(on_siege_over_winner_fort) /* вызывается для победителя при осаде форта */ \
  ACTION_TYPE_FUNC(on_siege_over_winner_trade_post) /* вызывается для победителя при осаде торгового поста */ \
  ACTION_TYPE_FUNC(on_siege_over_loc_chars)   /* вызывается для всях персонажей, которые предположительно находились в поселении */ \
  ACTION_TYPE_FUNC(on_siege_over_loc_chars_fort) /* вызывается для всях персонажей, которые предположительно находились в провинции в которой находился форт */ \
  ACTION_TYPE_FUNC(on_siege_over_loc_chars_trade_post) /* вызывается для всях персонажей, которые предположительно находились в провинции в которой находился торговый пост */ \
  ACTION_TYPE_FUNC(on_prepared_invasion_monthly) /* каждый месяц для персонажа который готовит вторжение */ \
  ACTION_TYPE_FUNC(on_prepared_invasion_aborts) /* если вторжение отменяется (при смерти готовившего что ли?) */ \
  ACTION_TYPE_FUNC(on_prepared_invasion_expires) /* персонаж в итоге не воспользовался вторжением */ \
  ACTION_TYPE_FUNC(on_merc_rampage)           /* эвент связаный с наемниками (судя по всему теряют голову в бою) */ \
  ACTION_TYPE_FUNC(on_merc_leave)             /* эвент связаный с наемниками (судя по всему сбегают из боя) */ \
  ACTION_TYPE_FUNC(on_merc_turn_coat_from)    /* эвент связаный с наемниками (судя по всему меняют сторону (?)) */ \
  ACTION_TYPE_FUNC(on_merc_turn_coat_to)      /* эвент связаный с наемниками (судя по всему меняют сторону (?)) */ \
  ACTION_TYPE_FUNC(on_player_mercenary_income) /* ??? (может быть проверка что у игрока достаточно денег?) */ \
  ACTION_TYPE_FUNC(on_holy_order_leave)       /* сбегает орден (орден госпитальеров например) */ \
  ACTION_TYPE_FUNC(on_warleader_death)        /* не используется в цк2 (предположительно вызыватся при смерти правителя) */ \
  ACTION_TYPE_FUNC(on_war_started)            /* старт войны */ \
  ACTION_TYPE_FUNC(on_war_ended_victory)      /* победа в войне, начатой персонажем */ \
  ACTION_TYPE_FUNC(on_war_ended_invalid)      /* условия войны не выполняются (наверное когда была начата из-за слабых претензий и персонаж умер) */ \
  ACTION_TYPE_FUNC(on_war_ended_whitepeace)   /* белый мир в войне */ \
  ACTION_TYPE_FUNC(on_war_ended_defeat)       /* проигрыш в войне начатой персонажем */ \
  ACTION_TYPE_FUNC(on_ai_end_raid)            /* ИИ закончил рейдить провинцию */ \
  ACTION_TYPE_FUNC(on_mercenary_hired)        /* наемники наняты */ \
  ACTION_TYPE_FUNC(on_mercenary_dismissed)    /* наемники распущены */ \
  ACTION_TYPE_FUNC(on_mercenary_captain_replacement) /* смена командира наемников */ \
  ACTION_TYPE_FUNC(on_unit_entering_province) /* вызывается для всех персонажей которые входят в провинцию (в качестве войска) */ \
  ACTION_TYPE_FUNC(on_command_unit)           /* вызывается для персонажей когда их ставят управлять флангом */ \
  ACTION_TYPE_FUNC(on_command_subunit)        /* вызывается для персонажей когда их ставят управлять подъюнитом (?) */ \
  /* заговоры */ \
  ACTION_TYPE_FUNC(on_failed_assassination)   /* провал заговора */ \
  ACTION_TYPE_FUNC(on_failed_assassination_disc) /* провал заговора (чем отличается?) */ \
  ACTION_TYPE_FUNC(on_assassination)          /* успешный заговор */ \
  ACTION_TYPE_FUNC(on_assassination_disc)     /* успешный заговор (чем отличается?) */ \
  /* персонаж */ \
  ACTION_TYPE_FUNC(on_birth)                  /* при рождении ребенка */ \
  ACTION_TYPE_FUNC(on_adulthood)              /* перед непосредственным становлением взрослым */ \
  ACTION_TYPE_FUNC(on_post_birth)             /* после рождения (on_birth возможно используется для матери) */ \
  ACTION_TYPE_FUNC(on_pregnancy)              /* вызывается на втором месяце беременности */ \
  ACTION_TYPE_FUNC(on_marriage)               /* при женитьбе (вызывается для двух владык супругов) */ \
  ACTION_TYPE_FUNC(on_betrothal)              /* при обручении (вызывается для двух владык обрученных) */ \
  ACTION_TYPE_FUNC(on_become_imprisoned_any_reason) /* вызывается при заключении персонажа в тюрьму (должны храниться указатели на заключенного и того кто это сделал) */ \
  ACTION_TYPE_FUNC(on_avoided_imprison_started_war) /* вызывается если была попытка посадить в тюрьму кого то с землей и тот избежал этого */ \
  ACTION_TYPE_FUNC(on_became_imprisoned)      /* вызывается при заключении персонажа в тюрьму (указано что это при diplo-action, я так понимаю имеются ввиду desicions) */ \
  ACTION_TYPE_FUNC(on_avoided_imprison_fled_country) /* попытка заключить в тюрьму кого то без земли, персонаж сбегает в другую страну */ \
  ACTION_TYPE_FUNC(on_released_from_prison)   /* при освобождении из тюрьмы */ \
  ACTION_TYPE_FUNC(on_executed)               /* вызывается при казнях (до казни или после?) */ \
  ACTION_TYPE_FUNC(on_exiled)                 /* вызывается при ссылке персонажа  */ \
  ACTION_TYPE_FUNC(on_death)                  /* вызывается непосредственно перед наследованием */ \
  ACTION_TYPE_FUNC(on_forced_consort)         /* правитель заставляет стать кого то своим наложником/наложницей (из тюрьмы) */ \
  ACTION_TYPE_FUNC(on_become_doge)            /* вызывается при переизбрании дожа (главы республики я так понимаю) */ \
  ACTION_TYPE_FUNC(on_elective_gavelkind_succession) /* равный раздел (между детьми правителя) с выбором кому пойдет основной титул, вызывается для всех вассалов после наследования */ \
  ACTION_TYPE_FUNC(on_create_title)           /* при создании титула  */ \
  ACTION_TYPE_FUNC(on_new_holder)             /* при смене владельца титула города  */ \
  ACTION_TYPE_FUNC(on_new_holder_inheritance) /* при наследовании титула города  */ \
  ACTION_TYPE_FUNC(on_new_holder_usurpation)  /* при узурпации титула города (тюрьма?) */ \
  ACTION_TYPE_FUNC(on_character_convert_religion) /* персонаж меняет религию */ \
  ACTION_TYPE_FUNC(on_character_convert_secret_religion) /* персонаж обращается в религию в которую он верил в тайне */ \
  ACTION_TYPE_FUNC(on_character_convert_culture) /* персонаж меняет культуру */ \
  ACTION_TYPE_FUNC(on_acquire_nickname)       /* персонаж получает прозвище */ \
  ACTION_TYPE_FUNC(on_over_vassal_limit_succession) /* вызывается для вассалов которые могу стать независимыми из-за того что у правителя переизбыток вассалов (больше лимита вассального) */ \
  ACTION_TYPE_FUNC(on_divorce)                /* при разводе */ \
  ACTION_TYPE_FUNC(on_feud_started)           /* старт междоусобицы (то есть восстания?) */ \
  ACTION_TYPE_FUNC(on_feud_ended)             /* конец междоусобицы */ \
  ACTION_TYPE_FUNC(on_blood_brother_death)    /* смерть кровного брата (сестры?) */ \
  ACTION_TYPE_FUNC(on_artifact_inheritance)   /* при наследовании артефакта (вызывается для каждого артефакта) */ \
  ACTION_TYPE_FUNC(on_tyranny_gained)         /* вызывается для каждого персонажа у которых портится отношения с тираном (в цк2 вызывается скорее всего где то в коде) */ \
  ACTION_TYPE_FUNC(on_tyranny_gained_tyrant_only) /* вызывается для тирана при попытках тирании (?) */ \
  ACTION_TYPE_FUNC(on_revoke_attempted_started_war) /* вызывается когда персонаж отказывается передавать титул и начинает войну */ \
  ACTION_TYPE_FUNC(on_retract_vassal_attempted_started_war) /* вызывается когда персонаж отказывается вступать в войну и сам объявляет войну господину */ \
  ACTION_TYPE_FUNC(on_absorb_clan_attempted_started_war) /* персонаж отказывается при попытке погложения клана (кочевники) и объявляет войну */ \
  ACTION_TYPE_FUNC(on_split_clan_attempted_started_war) /* персонаж отказывается при попытке разделения клана (кочевники) и объявляет войну */ \
  ACTION_TYPE_FUNC(on_employer_change)        /* смена "работодателя" (вики пишет что так можно ловить вновь созданных персонажей, но я по не понимаю что конкретно это означает) */ \
  ACTION_TYPE_FUNC(on_host_change)            /* смена хоста (?) */ \
  /* страна */ \
  ACTION_TYPE_FUNC(on_approve_law)            /* ответ на предложенное изменение в де факто законе (вызывается для правителя и титула) не понимаю что конкретно) */ \
  ACTION_TYPE_FUNC(on_approve_de_jure_law)    /* ответ на предложенное изменение в де юре законе (вызывается для правителя и титула, не понимаю что конкретно) */ \
  ACTION_TYPE_FUNC(on_rebel_revolt)           /* вызывается для провинции в которых начинается востание */ \
  ACTION_TYPE_FUNC(on_enforce_peace)          /* механика примирения (я так понимаю кнопка в совете) */ \
  ACTION_TYPE_FUNC(on_enforce_peace_start)    /* начало примирения (?) */ \
  ACTION_TYPE_FUNC(on_enforce_peace_six_vassals) /* вызывается для всех вассалов в совете (?) */ \
  ACTION_TYPE_FUNC(on_law_vote_passed)        /* успешное голосование по закону */ \
  ACTION_TYPE_FUNC(on_law_vote_failed)        /* неуспешное голосование по закону */ \
  /* крестовые походы */ \
  ACTION_TYPE_FUNC(on_crusade_creation)       /* созывается крестовый поход, вызывается для главы религии, атакуемого титула и атакуемой страны */ \
  ACTION_TYPE_FUNC(on_crusade_invalid)        /* невозможно продолжить крестовый поход */ \
  ACTION_TYPE_FUNC(on_crusade_success)        /* успешный крестовый поход */ \
  ACTION_TYPE_FUNC(on_crusade_failure)        /* крестовый поход провалился */ \
  ACTION_TYPE_FUNC(on_crusade_preparation_starts) /* вызывается для каждого крестового похода при подготовке */ \
  ACTION_TYPE_FUNC(on_crusade_preparation_ends) /* вызывается для каждого крестового похода при конце подготовки */ \
  ACTION_TYPE_FUNC(on_crusade_canceled)       /* вызывается при отмене похода */ \
  ACTION_TYPE_FUNC(on_crusade_monthly)        /* каждый месяц когда крестовый поход активен */ \
  ACTION_TYPE_FUNC(on_crusade_target_changes) /* вызывается когда цеть похода изменяется (например титул наследуется) */ \
  ACTION_TYPE_FUNC(on_pledge_crusade_participation) /* персонаж присоединяется к крестовому походу */ \
  ACTION_TYPE_FUNC(on_pledge_crusade_defense) /* персонаж присоединяется к защите от крестового похода */ \
  ACTION_TYPE_FUNC(on_unpledge_crusade_participation) /* персонаж больше не участвует в походе (например умер или сменил религию) */ \
  ACTION_TYPE_FUNC(on_unpledge_crusade_defense) /* персонаж больше не участвует в защите от похода (например умер или сменил религию) */ \
  /* религия */ \
  ACTION_TYPE_FUNC(on_reform_religion)        /* при реформировании религии */ \
  ACTION_TYPE_FUNC(on_county_religion_change) /* при изменении религии в провинции (?) */ \
  ACTION_TYPE_FUNC(on_vassal_accepts_religious_conversion) /* персонаж принимает предложение сменить религию, вызывается для вассала и для всех персонажей при дворе этого вассала и для всех вассалов этого вассала */ \
  ACTION_TYPE_FUNC(on_heresy_takeover)        /* ересь становится основной религией, заменяя предыдущий уклад (вызывается для всех персонажей?) */ \
  ACTION_TYPE_FUNC(on_rel_elector_chosen)     /* после выборов кардинала */ \
  ACTION_TYPE_FUNC(on_rel_head_chosen)        /* после выборов Папы римского (или подобного главы религии) */ \
  ACTION_TYPE_FUNC(on_excommunicate_interaction) /* персонаж отлучен */ \
  /* юниты (армии?) */ \
  ACTION_TYPE_FUNC(on_entering_port)          /* корабли заходят в порт */ \
  ACTION_TYPE_FUNC(on_navy_returns_with_loot) /* корабли возвращаются с добычей */ \
  /* сообщество */ \
  ACTION_TYPE_FUNC(on_society_two_year_pulse) /* вызывается для персонажей в тайном обществе каждые два года */ \
  ACTION_TYPE_FUNC(on_society_created)        /* кто то вступает в общество в котором нет членов */ \
  ACTION_TYPE_FUNC(on_society_destroyed)      /* последний член общества покидает его */ \
  ACTION_TYPE_FUNC(on_society_failed_to_find_new_leader) /* общество не может найти подходящего лидера */ \
  ACTION_TYPE_FUNC(on_society_progress_full)  /* прогресс по заданию общества (в описании написано странно "increased/set to the value 100" или то или то что ли?) */ \
  ACTION_TYPE_FUNC(on_society_progress_zero)  /* прогресс по заданию общества (в описании написано странно "decreased/set to the value 0" или то или то что ли?) */ \
  /* сила на карте */ \
  ACTION_TYPE_FUNC(on_offmap_policy_changed)  /* offmap - это специальные сущности которые моделируют безземельные титулы, например авантюристы или крестьянские восстания ... */ \
  ACTION_TYPE_FUNC(on_offmap_status_changed)  /* одна из таких сил - китай, политика (открытость, изоляционизм и проч) и статус (гражданская война, спокойствие и проч) ... */ \
  ACTION_TYPE_FUNC(on_offmap_governor_changed) /* показываются в специальном окошке, эти эвенты отвечают за разные вещи которые могут произойти с этой силой (смена губернатора? доверенного лица?) */ \
  ACTION_TYPE_FUNC(on_offmap_ruler_changed)   /* смена правителя в offmap power */ \
  ACTION_TYPE_FUNC(on_offmap_monthly_pulse)   /* ежемесячный вызов эвентов для offmap power */ \
  ACTION_TYPE_FUNC(on_offmap_yearly_pulse)    /* ежегодичный вызов эвентов для offmap power */ \

namespace devils_engine {
  namespace script {
    namespace action_type {
      enum values {
#define ACTION_TYPE_FUNC(name) name,
        ACTION_TYPES_LIST
#undef ACTION_TYPE_FUNC
        
        count
      };
    }
  }
}

#endif
