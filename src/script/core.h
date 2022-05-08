#ifndef DEVILS_ENGINE_SCRIPT_CORE2_H
#define DEVILS_ENGINE_SCRIPT_CORE2_H

#include <string_view>
#include "parallel_hashmap/phmap.h"
#include "utils/sol.h"
#include "interface.h"
#include "input_data.h"
#include "core/stats.h"
#include "all_commands_macro.h"

#define SCRIPT_TYPES_LIST \
  SCRIPT_TYPE_FUNC(condition) \
  SCRIPT_TYPE_FUNC(numeric) \
  SCRIPT_TYPE_FUNC(string) \
  SCRIPT_TYPE_FUNC(object) \
  SCRIPT_TYPE_FUNC(effect) \

namespace devils_engine {
  namespace script {
    class container;
    
    namespace script_types {
      enum values {
#define SCRIPT_TYPE_FUNC(name) name,
        SCRIPT_TYPES_LIST
#undef SCRIPT_TYPE_FUNC
      };
      
      extern const std::string_view names[];
      extern const phmap::flat_hash_map<std::string_view, values> map;
    }
    
    namespace commands {
      enum values {
#define COMMAND_NAME_FUNC(name) name,

#define LOGIC_BLOCK_COMMAND_FUNC(name, a, b) COMMAND_NAME_FUNC(name)
#define NUMERIC_COMMAND_BLOCK_FUNC(name, a, b) COMMAND_NAME_FUNC(name) 
#define NUMERIC_COMMAND_FUNC(name, a, b) COMMAND_NAME_FUNC(name)       
#define GET_SCOPE_COMMAND_FUNC(name, a, b, c) COMMAND_NAME_FUNC(name)     
#define CHANGE_CONTEXT_COMMAND_FUNC(name, a, b, c) COMMAND_NAME_FUNC(has_##name) COMMAND_NAME_FUNC(random_##name) COMMAND_NAME_FUNC(every_##name)
#define CONDITION_COMMAND_FUNC(name) COMMAND_NAME_FUNC(name)           
#define CONDITION_ARG_COMMAND_FUNC(name, a, b, c) COMMAND_NAME_FUNC(name)
#define ACTION_COMMAND_FUNC(name) COMMAND_NAME_FUNC(name)  
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
#undef COMMON_COMMAND_FUNC         
#undef CASUS_BELLI_FLAG_FUNC
#undef CASUS_BELLI_NUMBER_FUNC

#undef COMMAND_NAME_FUNC

#define STAT_FUNC(name) name,
#define CHARACTER_PENALTY_STAT_FUNC(name) name##_penalty,
        UNIQUE_STATS_RESOURCES_LIST
#undef STAT_FUNC
#undef CHARACTER_PENALTY_STAT_FUNC

#define STAT_FUNC(name) base_##name,
#define CHARACTER_PENALTY_STAT_FUNC(name) base_##name##_penalty,
        UNIQUE_STATS_LIST
#undef STAT_FUNC
#undef CHARACTER_PENALTY_STAT_FUNC

#define STAT_FUNC(name) add_##name,
#define CHARACTER_PENALTY_STAT_FUNC(name) add_##name##_penalty,
        UNIQUE_STATS_RESOURCES_LIST
#undef STAT_FUNC
#undef CHARACTER_PENALTY_STAT_FUNC

        count
      };
      
      extern const std::string_view names[];
      extern const phmap::flat_hash_map<std::string_view, values> map;
      extern const init_func_p inits[];
    }
    
    namespace ignore_keys {
      enum values {
#define SCRIPT_IGNORE_KEY_FUNC(name) name,
        SCRIPT_IGNORE_KEY_VALUES
#undef SCRIPT_IGNORE_KEY_FUNC
        _count,

        count
      };
      
      extern const std::string_view names[];
      extern const phmap::flat_hash_map<std::string_view, values> map;
    }
    
    namespace compare_operators {
      enum values : uint8_t {
#define COMPARE_OPERATOR_FUNC(name) name,
        COMPARE_OPERATORS_LIST
#undef COMPARE_OPERATOR_FUNC
        count
      };
      
      extern const std::string_view names[];
      extern const phmap::flat_hash_map<std::string_view, values> map;
    }
    
    namespace complex_object_tokens {
      enum values {
#define COMPLEX_OBJECT_TOKEN_FUNC(name) name,
        COMPLEX_OBJECT_TOKENS_LIST
#undef COMPLEX_OBJECT_TOKEN_FUNC
        count
      };
      
      extern const std::string_view names[];
      extern const phmap::flat_hash_map<std::string_view, values> map;
    }
    
    namespace complex_variable_valid_string {
      enum values {
#define COMPLEX_VARIABLE_VALID_STRING_FUNC(name) name,
        COMPLEX_VARIABLE_VALID_STRINGS_LIST
#undef COMPLEX_VARIABLE_VALID_STRING_FUNC
        count
      };
      
      extern const std::string_view names[];
      extern const phmap::flat_hash_map<std::string_view, values> map;
    }
  }
}

#endif
