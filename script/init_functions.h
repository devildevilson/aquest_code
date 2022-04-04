#ifndef DEVILS_ENGINE_SCRIPT_INIT_FUNCTIONS_H
#define DEVILS_ENGINE_SCRIPT_INIT_FUNCTIONS_H

#include <cstddef>
#include <tuple>
#include "utils/sol.h"
#include "input_data.h"
#include "all_commands_macro.h"
#include "core/stats.h"

// можно ли какнибудь сделать lvalue в условиях когда у нас функции распихиваются по контейнерам с типами?
// lvalue может быть случайного типа, а функции могут совпадать по названию, в этом случае нужно сделать перегрузку функции
// то есть это специальный класс, который должен пройти несколько типов и сделать некое действие в зависимости от типа
// как ее делать? по идее как и обычные функции, но через запятую указать другие типы, а потом в функции сгенерировать как нибудь последовательное 
// исполнение (цикл по типам? через рекурсию?)

#define DEFAULT_ALIGNMENT 8

// еще дополнительно мы должны указать что лежит в текущем контексте и видимо нужно еще вернуть тип текущего контекста
#define DECLARE_INIT_FUNC(name) std::tuple<interface*, size_t, size_t> name##_init(const input_data &input, const sol::object &data, container* cont);
#define DECLARE_TABLE_INIT_FUNC(name) std::tuple<interface*, size_t, size_t> name##_init(const input_data &input, const sol::object &data, container* cont, const init_func_p parent_func);
#define DECLARE_INIT_FUNC2(name) std::tuple<interface*, size_t, size_t> name##_init(const input_data &input, const sol::object &data, container* cont, const bool dot_is_valid_string_symbol);
#define DECLARE_INIT_FUNC3(name) std::tuple<interface*, size_t, size_t> name##_init(const input_data &input, const sol::object &data, container* cont, size_t &new_current);

namespace devils_engine {
  namespace script {
    class interface;
    class container;
    
    typedef std::tuple<interface*, size_t, size_t> (*table_init_func_t)(const input_data &input, const sol::object &data, container* cont, const init_func_p parent);
    
    DECLARE_TABLE_INIT_FUNC(numeric_table)
    DECLARE_TABLE_INIT_FUNC(string_table)
    DECLARE_TABLE_INIT_FUNC(condition_table)
    DECLARE_INIT_FUNC(effect_table)
//     DECLARE_INIT_FUNC(change_scope_condition)
    //DECLARE_TABLE_INIT_FUNC(change_scope_effect)
    
    DECLARE_INIT_FUNC(get_building_type)
    DECLARE_INIT_FUNC(get_holding_type)
    DECLARE_INIT_FUNC(get_city_type)
    DECLARE_INIT_FUNC(get_trait)
    DECLARE_INIT_FUNC(get_modificator)
    DECLARE_INIT_FUNC(get_troop_type)
    DECLARE_INIT_FUNC(get_religion_group)
    DECLARE_INIT_FUNC(get_religion)
    DECLARE_INIT_FUNC(get_culture_group)
    DECLARE_INIT_FUNC(get_culture)
    DECLARE_INIT_FUNC(get_law)
    DECLARE_INIT_FUNC(get_title)
    DECLARE_INIT_FUNC(get_casus_belli)
    
    // что с династией? будет ли у нее id? наверное, но как генерироватьновые династические id?
    DECLARE_INIT_FUNC(get_dynasty)
    
    // имеет смысл добавить id городам и провинциям, но пока что мы может эти вещи взять из контекста или из сложных объектов
    DECLARE_INIT_FUNC(get_city)
    DECLARE_INIT_FUNC(get_province)
    DECLARE_INIT_FUNC(get_character) // можно ли персонажей как нибудь по индексу брать? ну собственно игра будет хранить указатель на перса
    DECLARE_INIT_FUNC(get_realm)
    DECLARE_INIT_FUNC(get_hero_troop)
    DECLARE_INIT_FUNC(get_troop)
    DECLARE_INIT_FUNC(get_army)
    DECLARE_INIT_FUNC(get_war)
    
    //DECLARE_INIT_FUNC(get_tile) // вряд ли потребуется
    
    // пока что наверное уберу, а потом имеет смысл проверять может ли персонаж сделать те или иные вещи
//     DECLARE_INIT_FUNC(get_decision)
//     DECLARE_INIT_FUNC(get_interaction)
//     DECLARE_INIT_FUNC(get_event)
//     DECLARE_INIT_FUNC(get_biome)
    
//     DECLARE_INIT_FUNC2(string_table)
//     DECLARE_INIT_FUNC2(string_container)
//     DECLARE_INIT_FUNC2(compute_string)
    
    DECLARE_INIT_FUNC3(complex_object)
    
#define COMMON_COMMAND_FUNC(name) DECLARE_INIT_FUNC(name)
    ONLY_INIT_LIST
#undef COMMON_COMMAND_FUNC
    
#define COMMAND_NAME_FUNC(name) DECLARE_INIT_FUNC(name)

#define LOGIC_BLOCK_COMMAND_FUNC(name, a, b) COMMAND_NAME_FUNC(name)
#define NUMERIC_COMMAND_BLOCK_FUNC(name, a, b) COMMAND_NAME_FUNC(name) 
#define NUMERIC_COMMAND_FUNC(name, a, b) COMMAND_NAME_FUNC(name)       
#define GET_SCOPE_COMMAND_FUNC(name, a, b, c) COMMAND_NAME_FUNC(name)     
#define CHANGE_CONTEXT_COMMAND_FUNC(name, a, b, c) COMMAND_NAME_FUNC(has_##name) COMMAND_NAME_FUNC(random_##name) COMMAND_NAME_FUNC(every_##name)
#define CONDITION_COMMAND_FUNC(name) COMMAND_NAME_FUNC(name)           
#define CONDITION_ARG_COMMAND_FUNC(name, a, b, c) COMMAND_NAME_FUNC(name)
#define ACTION_COMMAND_FUNC(name) COMMAND_NAME_FUNC(name)  
#define COUNT_OBJECTS_COMMAND_FUNC(name, a, b, c) COMMAND_NAME_FUNC(name)  
#define COMMON_COMMAND_FUNC(name) COMMAND_NAME_FUNC(name)              
#define CASUS_BELLI_FLAG_FUNC(name) CONDITION_COMMAND_FUNC(name)
#define CASUS_BELLI_NUMBER_FUNC(name) CONDITION_COMMAND_FUNC(name)
        SCRIPT_COMMANDS_LIST 
#undef LOGIC_BLOCK_COMMAND_FUNC    
#undef NUMERIC_COMMAND_BLOCK_FUNC  
#undef NUMERIC_COMMAND_FUNC        
#undef GET_SCOPE_COMMAND_FUNC      
#undef CHANGE_CONTEXT_COMMAND_FUNC 
#undef CONDITION_COMMAND_FUNC
#undef CONDITION_ARG_COMMAND_FUNC
#undef ACTION_COMMAND_FUNC       
#undef COUNT_OBJECTS_COMMAND_FUNC
#undef COMMON_COMMAND_FUNC     
#undef CASUS_BELLI_FLAG_FUNC
#undef CASUS_BELLI_NUMBER_FUNC
    
#undef COMMAND_NAME_FUNC

#define STAT_FUNC(name) DECLARE_INIT_FUNC(name)
#define CHARACTER_PENALTY_STAT_FUNC(name) DECLARE_INIT_FUNC(name##_penalty)
        UNIQUE_STATS_RESOURCES_LIST
#undef STAT_FUNC
#undef CHARACTER_PENALTY_STAT_FUNC

#define STAT_FUNC(name) DECLARE_INIT_FUNC(base_##name)
#define CHARACTER_PENALTY_STAT_FUNC(name) DECLARE_INIT_FUNC(base_##name##_penalty)
        UNIQUE_STATS_LIST
#undef STAT_FUNC
#undef CHARACTER_PENALTY_STAT_FUNC

#define STAT_FUNC(name) DECLARE_INIT_FUNC(add_##name)
#define CHARACTER_PENALTY_STAT_FUNC(name) DECLARE_INIT_FUNC(add_##name##_penalty)
        UNIQUE_STATS_RESOURCES_LIST
#undef STAT_FUNC
#undef CHARACTER_PENALTY_STAT_FUNC

    extern const std::string_view number_matcher;
    extern const std::string_view dot_matcher;
    extern const std::string_view colon_matcher;
  }
}

#undef DECLARE_INIT_FUNC
#undef DECLARE_INIT_FUNC2

#endif
