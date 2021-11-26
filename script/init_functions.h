#ifndef DEVILS_ENGINE_SCRIPT_INIT_FUNCTIONS_H
#define DEVILS_ENGINE_SCRIPT_INIT_FUNCTIONS_H

#include <cstddef>
#include <tuple>
#include "utils/sol.h"
#include "input_data.h"
#include "all_commands_macro.h"
#include "core/stats.h"

#define DEFAULT_ALIGNMENT 8

// еще дополнительно мы должны указать что лежит в текущем контексте и видимо нужно еще вернуть тип текущего контекста
#define DECLARE_INIT_FUNC2(name) std::tuple<interface*, size_t, size_t> name(const input_data &input, const sol::object &data, container* cont);
#define DECLARE_INIT_FUNC(name) std::tuple<interface*, size_t, size_t> name##_init(const input_data &input, const sol::object &data, container* cont);

namespace devils_engine {
  namespace script {
    class interface;
    class container;
    
    DECLARE_INIT_FUNC(numeric_table)
    DECLARE_INIT_FUNC(string_table)
    DECLARE_INIT_FUNC(condition_table)
    DECLARE_INIT_FUNC(effect_table)
    
#define COMMON_COMMAND_FUNC(name) DECLARE_INIT_FUNC(name)
    ONLY_INIT_LIST
#undef COMMON_COMMAND_FUNC
    
#define COMMAND_NAME_FUNC(name) DECLARE_INIT_FUNC(name)

#define LOGIC_BLOCK_COMMAND_FUNC(name, a, b) COMMAND_NAME_FUNC(name)
#define NUMERIC_COMMAND_BLOCK_FUNC(name, a, b) COMMAND_NAME_FUNC(name) 
#define NUMERIC_COMMAND_FUNC(name, a, b) COMMAND_NAME_FUNC(name)       
#define GET_SCOPE_COMMAND_FUNC(name, a, b) COMMAND_NAME_FUNC(name)     
#define CHANGE_CONTEXT_COMMAND_FUNC(name, a, b, c) COMMAND_NAME_FUNC(has##name) COMMAND_NAME_FUNC(random##name) COMMAND_NAME_FUNC(every##name)
#define CONDITION_COMMAND_FUNC(name) COMMAND_NAME_FUNC(name)           
#define ACTION_COMMAND_FUNC(name) COMMAND_NAME_FUNC(name)  
#define COUNT_OBJECTS_COMMAND_FUNC(name, a, b, c) COMMAND_NAME_FUNC(name)  
#define COMMON_COMMAND_FUNC(name) COMMAND_NAME_FUNC(name)              
        SCRIPT_COMMANDS_LIST 
#undef LOGIC_BLOCK_COMMAND_FUNC    
#undef NUMERIC_COMMAND_BLOCK_FUNC  
#undef NUMERIC_COMMAND_FUNC        
#undef GET_SCOPE_COMMAND_FUNC      
#undef CHANGE_CONTEXT_COMMAND_FUNC 
#undef CONDITION_COMMAND_FUNC      
#undef ACTION_COMMAND_FUNC       
#undef COUNT_OBJECTS_COMMAND_FUNC
#undef COMMON_COMMAND_FUNC     
    
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

#endif
