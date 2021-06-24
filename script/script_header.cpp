#include "script_header.h"

#include "utils/magic_enum_header.h"
#include "condition_functions.h"
#include "action_functions.h"
#include "script_block_functions.h"

namespace devils_engine {
  namespace script {
    script_data::script_data() : 
      command_type(command_type::invalid), 
      number_type(number_type::boolean), 
      compare_type(number_compare_type::equal), 
      helper1(UINT16_MAX),
      helper2(UINT16_MAX),
      value(0.0),
      data(nullptr)
    {}
    
    script_data::script_data(script_data &&move) :
      command_type(move.command_type), 
      number_type(move.number_type), 
      compare_type(move.compare_type), 
      helper1(move.helper1),
      helper2(move.helper2),
      value(move.value),
      data(move.data)
    {
      move.data = nullptr;
    }
    
    script_data::script_data(const struct target &t) :
      command_type(command_type::invalid), 
      number_type(number_type::object), 
      compare_type(number_compare_type::equal), 
      helper1(UINT16_MAX),
      helper2(t.type),
      value(0.0),
      data(t.data)
    {}
    
    script_data::~script_data() {
      switch (number_type) {
        case number_type::boolean: break;
        case number_type::number: break;
        case number_type::array: {
          auto array = reinterpret_cast<script_data*>(data);
          delete [] array;
          break;
        }
        
        case number_type::string:
        case number_type::get_scope: {
          auto str = reinterpret_cast<char*>(data);
          delete [] str;
          break;
        }
      };
      
      data = nullptr;
      
//       switch (command_type) {
//         case command_type::action: {
//           std::cout << "Deleted " << magic_enum::enum_name(static_cast<action_function::value>(helper1)) << "\n";
//           break;
//         }
//         
//         case command_type::action_script_block: {
//           std::cout << "Deleted " << magic_enum::enum_name(static_cast<action_block_function::value>(helper1)) << "\n";
//           break;
//         }
//         
//         case command_type::condition: {
//           std::cout << "Deleted " << magic_enum::enum_name(static_cast<condition_function::value>(helper1)) << "\n";
//           break;
//         }
//         
//         case command_type::condition_script_block: {
//           std::cout << "Deleted " << magic_enum::enum_name(static_cast<condition_block_function::value>(helper1)) << "\n";
//           break;
//         }
//       }
    }
    
    script_data & script_data::operator=(script_data &&move) {
      switch (number_type) {
        case number_type::boolean: break;
        case number_type::number: break;
        case number_type::array: {
          auto array = reinterpret_cast<script_data*>(data);
          delete [] array;
          break;
        }
        
        case number_type::string:
        case number_type::get_scope: {
          auto str = reinterpret_cast<char*>(data);
          delete [] str;
          break;
        }
      };
      
      command_type = move.command_type;
      number_type = move.number_type;
      compare_type = move.compare_type;
      helper1 = move.helper1;
      helper2 = move.helper2;
      value = move.value;
      data = move.data;
      move.data = nullptr;
      
      return *this;
    }
    
    uint64_t random_state::next() {
      s = utils::xoroshiro128starstar::rng(s);
      return utils::xoroshiro128starstar::get_value(s);
    }
    
    const condition_function_p condition_functions[] = {
#define CONDITION_COMMAND_FUNC(name) &name,
      CONDITION_COMMANDS_LIST
#undef CONDITION_COMMAND_FUNC
    };
    
    
    const action_function_p action_functions[] = {
#define ACTION_COMMAND_FUNC(name) &name,
      ACTION_COMMANDS_LIST
#undef ACTION_COMMAND_FUNC
    };
    
    
    const block_function_p condition_block_functions[] = {
#define SCRIPT_BLOCK_COMMAND_FUNC(name) &name,
      CONDITION_SCRIPT_BLOCK_COMMANDS_LIST
#undef SCRIPT_BLOCK_COMMAND_FUNC
    };
    
    
    const block_function_p action_block_functions[] = {
#define SCRIPT_BLOCK_COMMAND_FUNC(name) &name,
      ACTION_SCRIPT_BLOCK_COMMANDS_LIST
#undef SCRIPT_BLOCK_COMMAND_FUNC
    };
    
    static_assert(condition_function::count == sizeof(condition_functions) / sizeof(condition_functions[0]));
    static_assert(action_function::count == sizeof(action_functions) / sizeof(action_functions[0]));
    static_assert(condition_block_function::count == sizeof(condition_block_functions) / sizeof(condition_block_functions[0]));
    static_assert(action_block_function::count == sizeof(action_block_functions) / sizeof(action_block_functions[0]));
    
    const function_init_p condition_init_functions[] = {
#define CONDITION_COMMAND_FUNC(name) &name##_init,
      CONDITION_COMMANDS_LIST
#undef CONDITION_COMMAND_FUNC
    };
    
    const function_init_p action_init_functions[] = {
#define ACTION_COMMAND_FUNC(name) &name##_init,
      ACTION_COMMANDS_LIST
#undef ACTION_COMMAND_FUNC
    };
    
    const function_init_p condition_block_init_functions[] = {
#define SCRIPT_BLOCK_COMMAND_FUNC(name) &name##_init,
      CONDITION_SCRIPT_BLOCK_COMMANDS_LIST
#undef SCRIPT_BLOCK_COMMAND_FUNC
    };
    
    const function_init_p action_block_init_functions[] = {
#define SCRIPT_BLOCK_COMMAND_FUNC(name) &name##_init,
      ACTION_SCRIPT_BLOCK_COMMANDS_LIST
#undef SCRIPT_BLOCK_COMMAND_FUNC      
    };
    
    const std::string_view complex_var_regex = "(\\w+):(\\w+):([+-]?([0-9]+([.][0-9]*)?|[.][0-9]+))";
  }
}
