#ifndef CONDITION_FUNCTIONS_H
#define CONDITION_FUNCTIONS_H

//#include "data_parser.h"
#include "script_header.h"
#include "condition_commands_macro.h"

namespace devils_engine {
  namespace script {
#define CONDITION_COMMAND_FUNC(name) bool name(const struct target &, const context &, const uint32_t &, const script_data*);
    CONDITION_COMMANDS_LIST
#undef CONDITION_COMMAND_FUNC

#define CONDITION_COMMAND_FUNC(name) void name##_init(const uint32_t &, const sol::object &, script_data*);
    CONDITION_COMMANDS_LIST
#undef CONDITION_COMMAND_FUNC
    
    // персонаж
//     bool is_ai(const struct target &target, const uint32_t &count, const script_data* data);
//     bool is_player(const struct target &target, const uint32_t &count, const script_data* data);
//     bool is_independent(const struct target &target, const uint32_t &count, const script_data* data);
//     bool is_vassal(const struct target &target, const uint32_t &count, const script_data* data);
//     bool is_male(const struct target &target, const uint32_t &count, const script_data* data);
//     bool is_female(const struct target &target, const uint32_t &count, const script_data* data);
//     bool is_prisoner(const struct target &target, const uint32_t &count, const script_data* data);
//     bool is_married(const struct target &target, const uint32_t &count, const script_data* data);
//     bool is_sick(const struct target &target, const uint32_t &count, const script_data* data);
//     bool is_in_war(const struct target &target, const uint32_t &count, const script_data* data);
//     bool is_in_society(const struct target &target, const uint32_t &count, const script_data* data);
//     bool is_hero(const struct target &target, const uint32_t &count, const script_data* data);
//     bool is_clan_head(const struct target &target, const uint32_t &count, const script_data* data); // глава клана кочевников
//     bool is_religious_head(const struct target &target, const uint32_t &count, const script_data* data);
//     bool is_father_alive(const struct target &target, const uint32_t &count, const script_data* data);
//     bool is_mother_alive(const struct target &target, const uint32_t &count, const script_data* data);
//     bool has_dead_friends(const struct target &target, const uint32_t &count, const script_data* data);
//     bool has_dead_rivals(const struct target &target, const uint32_t &count, const script_data* data);
//     bool has_owner(const struct target &target, const uint32_t &count, const script_data* data);
//     bool has_dead_lovers(const struct target &target, const uint32_t &count, const script_data* data);
//     bool has_dead_brothers(const struct target &target, const uint32_t &count, const script_data* data); // проверять количество?
//     bool has_dead_sisters(const struct target &target, const uint32_t &count, const script_data* data);
//     bool has_dead_siblings(const struct target &target, const uint32_t &count, const script_data* data);
//     bool has_dead_childs(const struct target &target, const uint32_t &count, const script_data* data);
//     bool has_concubines(const struct target &target, const uint32_t &count, const script_data* data);
//     bool has_alive_friends(const struct target &target, const uint32_t &count, const script_data* data);
//     bool has_alive_rivals(const struct target &target, const uint32_t &count, const script_data* data);
//     bool has_alive_lovers(const struct target &target, const uint32_t &count, const script_data* data);
//     bool has_alive_brothers(const struct target &target, const uint32_t &count, const script_data* data); // проверять количество?
//     bool has_alive_sisters(const struct target &target, const uint32_t &count, const script_data* data);
//     bool has_alive_siblings(const struct target &target, const uint32_t &count, const script_data* data);
//     bool has_alive_childs(const struct target &target, const uint32_t &count, const script_data* data);
//     bool has_dynasty(const struct target &target, const uint32_t &count, const script_data* data);
//     bool can_change_religion(const struct target &target, const uint32_t &count, const script_data* data);
//     bool can_call_crusade(const struct target &target, const uint32_t &count, const script_data* data);
//     bool can_grant_title(const struct target &target, const uint32_t &count, const script_data* data); // минорные титулы или все?
//     bool can_marry(const struct target &target, const uint32_t &count, const script_data* data);
//     bool belongs_to_culture(const struct target &target, const uint32_t &count, const script_data* data);
//     bool belongs_to_culture_group(const struct target &target, const uint32_t &count, const script_data* data);
//     bool has_trait(const struct target &target, const uint32_t &count, const script_data* data);
//     bool has_modificator(const struct target &target, const uint32_t &count, const script_data* data);
//     bool has_flag(const struct target &target, const uint32_t &count, const script_data* data);
//     bool has_title(const struct target &target, const uint32_t &count, const script_data* data);
//     bool has_nickname(const struct target &target, const uint32_t &count, const script_data* data);
//     bool bit_is_set(const struct target &target, const uint32_t &count, const script_data* data);
//     bool bit_is_unset(const struct target &target, const uint32_t &count, const script_data* data);
//     bool realm_has_enacted_law(const struct target &target, const uint32_t &count, const script_data* data);
//     bool realm_has_law_mechanic(const struct target &target, const uint32_t &count, const script_data* data);
//     // проверки личностных характеристик ии ()
// //     bool ai_ambition(const struct target &target, const uint32_t &count, const script_data* data);
// //     bool ai_greed(const struct target &target, const uint32_t &count, const script_data* data);
// //     bool ai_honor(const struct target &target, const uint32_t &count, const script_data* data);
// //     bool ai_rationality(const struct target &target, const uint32_t &count, const script_data* data);
// //     bool ai_zeal(const struct target &target, const uint32_t &count, const script_data* data);
//     // персонаж один из самых сильных вассалов (если он не является ваасалом то че?)
//     bool is_among_most_powerful_vassals(const struct target &target, const uint32_t &count, const script_data* data); 
//     // проверки статов
//     bool age_more(const struct target &target, const uint32_t &count, const script_data* data);
//     bool age_equal(const struct target &target, const uint32_t &count, const script_data* data);
//     bool age_less(const struct target &target, const uint32_t &count, const script_data* data);
//     bool money_more(const struct target &target, const uint32_t &count, const script_data* data);
//     bool money_equal(const struct target &target, const uint32_t &count, const script_data* data);
//     bool money_less(const struct target &target, const uint32_t &count, const script_data* data);
//     bool character_stat_more(const uint32_t &stat, const struct target &target, const uint32_t &count, const script_data* data);
//     bool character_stat_equal(const uint32_t &stat, const struct target &target, const uint32_t &count, const script_data* data);
//     bool character_stat_less(const uint32_t &stat, const struct target &target, const uint32_t &count, const script_data* data);
//     bool hero_stat_more(const uint32_t &stat, const struct target &target, const uint32_t &count, const script_data* data);
//     bool hero_stat_equal(const uint32_t &stat, const struct target &target, const uint32_t &count, const script_data* data);
//     bool hero_stat_less(const uint32_t &stat, const struct target &target, const uint32_t &count, const script_data* data);
//     
//     bool can_have_more_leadership_traits();
//     bool can_hold_title(); // может ли персонаж иметь титул (в цк2 указывался рабочий или минорный титул) (думаю что нужно разделить)
//     bool can_join_society(); // сообщества наврно потом добавлю
//     bool can_see_secret_religion(); // может ли один персонаж видеть секретную религию другого
//     bool can_swap_job_title(); // может ли персонаж получить работу другого персонажа
//     bool can_use_cb(); // появится ли конкретный цб в интерфейсе (полезно для проверки например дальности провинции которую пытаемся захватить)
//     bool character(); // проверка на одинаковых персонажей
//     bool check_variable(); // пока что нет возможности задать свои переменные
//     bool clan_opinion(); // проверяем отношение соклановца
//     bool clan_opinion_diff();
//     bool combat_rating(); // у меня нет такой переменной, но есть переменные героя
//     bool combat_rating_diff();
//     bool completely_controls(); // персонаж контролирует все баронства под указанным титулом
//     bool completely_controls_region(); // персонаж контролирует все баронства в указанном регионе
//     bool controls(); // контролирует провинцию 
//     bool could_be_child_of(); // проверяет возраст персонажа, достаточно ли тот мал чтобы приходиться ребенком к персонажу
//     bool could_be_parent_of(); // наоборот
//     bool day_of_birth(); // совпадает ли день рождения с этим числом
//     bool days_in_society();
//     bool days_at_current_society_rank();
//     bool days_since_last_hostile_action();
//     
//     
//     // где находимся либо по отножению к какому то персонажу либо титул (нужно почекать канкретные применения)
//     bool at_location();
//     
//     // артефакты
//     bool same_artifact(const struct target &target, const uint32_t &count, const script_data* data); // проверка артифакта (нужно указать где лежит в стеке, указать scope (скоуп))
//     bool artifact_age(const struct target &target, const uint32_t &count, const script_data* data); // проверка "старости" артефакта (больше или равно)
//     bool artifact_age_less(const struct target &target, const uint32_t &count, const script_data* data); // меньше
//     bool artifact_can_be_gifted_to(const struct target &target, const uint32_t &count, const script_data* data);
//     bool artifact_type(const struct target &target, const uint32_t &count, const script_data* data);
//     bool artifact_type_owned_by(const struct target &target, const uint32_t &count, const script_data* data);
//     
//     // кровь
//     bool bloodline(); // проверка крови
//     bool bloodline_is_active_for(); // если кровь активна для персонажа
//     
//     // провинция
//     bool borders_lake();
//     bool borders_major_river();
//     bool can_land_path_to(); // возможность прохода в другую провинцию (для чего это нам?)
//     bool can_naval_path_to(); // возможность прохода по морю в другую провинцию (???)
//     bool climate(); // климат в провинции
//     
//     // титулы
//     bool claimed_by(); 
//     bool conquest_culture(); // проверка на завоевательную культуру (почему скоуп титула?)
//     bool controlled_by(); 
//     
//     // offmap
//     bool days_since_policy_change();
//     bool days_since_status_change();
//     
//     // система 
//     bool calc_true_if(); // проверка хотя бы нескольких условий
//     bool age_diff();
//     bool attribute_diff();
//     bool can_be_given_away(); // титул может отдан кому нибудь
//     bool can_copy_personality_trait_from();
//     bool count(); // количество элементов подходящих по скоупу и лимиту
//     bool date(); // проверяем дату
  }
}

#endif
