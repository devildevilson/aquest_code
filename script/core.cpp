#include "core.h"

#include "init_functions.h"

#define MAKE_MAP_PAIR(name) std::make_pair(names[values::name], values::name)

namespace devils_engine {
  namespace script {
    namespace commands {
      const std::string_view names[] = {
#define COMMAND_NAME_FUNC(name) #name,

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
#undef ACTION_COMMAND_FUNC       
#undef COMMON_COMMAND_FUNC       
#undef CONDITION_ARG_COMMAND_FUNC
#undef CASUS_BELLI_FLAG_FUNC
#undef CASUS_BELLI_NUMBER_FUNC

#undef COMMAND_NAME_FUNC

#define STAT_FUNC(name) #name,
#define CHARACTER_PENALTY_STAT_FUNC(name) #name"_penalty",
        UNIQUE_STATS_RESOURCES_LIST
#undef STAT_FUNC
#undef CHARACTER_PENALTY_STAT_FUNC

#define STAT_FUNC(name) "base_"#name,
#define CHARACTER_PENALTY_STAT_FUNC(name) "base_"#name"_penalty",
        UNIQUE_STATS_LIST
#undef STAT_FUNC
#undef CHARACTER_PENALTY_STAT_FUNC

#define STAT_FUNC(name) "add_"#name,
#define CHARACTER_PENALTY_STAT_FUNC(name) "add_"#name"_penalty",
        UNIQUE_STATS_RESOURCES_LIST
#undef STAT_FUNC
#undef CHARACTER_PENALTY_STAT_FUNC
      };
      
      const phmap::flat_hash_map<std::string_view, values> map = {
#define COMMAND_NAME_FUNC(name) MAKE_MAP_PAIR(name),

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

#define STAT_FUNC(name) MAKE_MAP_PAIR(name),
#define CHARACTER_PENALTY_STAT_FUNC(name) MAKE_MAP_PAIR(name##_penalty),
        UNIQUE_STATS_RESOURCES_LIST
#undef STAT_FUNC
#undef CHARACTER_PENALTY_STAT_FUNC

#define STAT_FUNC(name) MAKE_MAP_PAIR(base_##name),
#define CHARACTER_PENALTY_STAT_FUNC(name) MAKE_MAP_PAIR(base_##name##_penalty),
        UNIQUE_STATS_LIST
#undef STAT_FUNC
#undef CHARACTER_PENALTY_STAT_FUNC

#define STAT_FUNC(name) MAKE_MAP_PAIR(add_##name),
#define CHARACTER_PENALTY_STAT_FUNC(name) MAKE_MAP_PAIR(add_##name##_penalty),
        UNIQUE_STATS_RESOURCES_LIST
#undef STAT_FUNC
#undef CHARACTER_PENALTY_STAT_FUNC
      };
      
      const init_func_p inits[] = {
#define COMMAND_NAME_FUNC(name) &name##_init,

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

#define STAT_FUNC(name) &name##_init,
#define CHARACTER_PENALTY_STAT_FUNC(name) &name##_penalty_init,
        UNIQUE_STATS_RESOURCES_LIST
#undef STAT_FUNC
#undef CHARACTER_PENALTY_STAT_FUNC

#define STAT_FUNC(name) &base_##name##_init,
#define CHARACTER_PENALTY_STAT_FUNC(name) &base_##name##_penalty_init,
        UNIQUE_STATS_LIST
#undef STAT_FUNC
#undef CHARACTER_PENALTY_STAT_FUNC

#define STAT_FUNC(name) &add_##name##_init,
#define CHARACTER_PENALTY_STAT_FUNC(name) &add_##name##_penalty_init,
        UNIQUE_STATS_RESOURCES_LIST
#undef STAT_FUNC
#undef CHARACTER_PENALTY_STAT_FUNC
      };
      
      static_assert(sizeof(names) / sizeof(names[0]) == count);
      static_assert(sizeof(inits) / sizeof(inits[0]) == count);
    }
    
    namespace ignore_keys {
      const std::string_view names[] = {
#define SCRIPT_IGNORE_KEY_FUNC(name) #name,
        SCRIPT_IGNORE_KEY_VALUES
#undef SCRIPT_IGNORE_KEY_FUNC
        "count"
      };
      
      const phmap::flat_hash_map<std::string_view, values> map = {
#define SCRIPT_IGNORE_KEY_FUNC(name) MAKE_MAP_PAIR(name),
        SCRIPT_IGNORE_KEY_VALUES
#undef SCRIPT_IGNORE_KEY_FUNC
        std::make_pair(names[_count], _count)
      };
      
      static_assert(sizeof(names) / sizeof(names[0]) == count);
    }
    
    namespace compare_operators {
      const std::string_view names[] = {
#define COMPARE_OPERATOR_FUNC(name) #name,
        COMPARE_OPERATORS_LIST
#undef COMPARE_OPERATOR_FUNC
      };
      
      const phmap::flat_hash_map<std::string_view, values> map = {
#define COMPARE_OPERATOR_FUNC(name) MAKE_MAP_PAIR(name),
        COMPARE_OPERATORS_LIST
#undef COMPARE_OPERATOR_FUNC
      };
      
      static_assert(sizeof(names) / sizeof(names[0]) == count);
    }
    
    namespace complex_object_tokens {
      const std::string_view names[] = {
#define COMPLEX_OBJECT_TOKEN_FUNC(name) #name,
        COMPLEX_OBJECT_TOKENS_LIST
#undef COMPLEX_OBJECT_TOKEN_FUNC
      };
      
      const phmap::flat_hash_map<std::string_view, values> map = {
#define COMPLEX_OBJECT_TOKEN_FUNC(name) MAKE_MAP_PAIR(name),
        COMPLEX_OBJECT_TOKENS_LIST
#undef COMPLEX_OBJECT_TOKEN_FUNC
      };
      
      static_assert(sizeof(names) / sizeof(names[0]) == count);
    }
    
    namespace complex_variable_valid_string {
      const std::string_view names[] = {
#define COMPLEX_VARIABLE_VALID_STRING_FUNC(name) #name,
        COMPLEX_VARIABLE_VALID_STRINGS_LIST
#undef COMPLEX_VARIABLE_VALID_STRING_FUNC
      };
      
      const phmap::flat_hash_map<std::string_view, values> map = {
#define COMPLEX_VARIABLE_VALID_STRING_FUNC(name) MAKE_MAP_PAIR(name),
        COMPLEX_VARIABLE_VALID_STRINGS_LIST
#undef COMPLEX_VARIABLE_VALID_STRING_FUNC 
      };
      
      static_assert(sizeof(names) / sizeof(names[0]) == count);
    }
    
    enum class uniquness_check {
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

#define SCRIPT_IGNORE_KEY_FUNC(name) name,
      SCRIPT_IGNORE_KEY_VALUES
#undef SCRIPT_IGNORE_KEY_FUNC
      _count,
        
#define COMPLEX_OBJECT_TOKEN_FUNC(name) name,
      COMPLEX_OBJECT_TOKENS_LIST
#undef COMPLEX_OBJECT_TOKEN_FUNC

#define COMMON_COMMAND_FUNC(name) name,
      ONLY_INIT_LIST
#undef COMMON_COMMAND_FUNC

      count
    };
  }
}
