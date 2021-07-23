#include "script_header.h"

#include "utils/magic_enum_header.h"
#include "condition_functions.h"
#include "action_functions.h"
#include "script_block_functions.h"
#include "utility.h"
#include "utils/constexpr_funcs.h"

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
      move.clear();
    }
    
    script_data::script_data(const struct target_t &t) :
      command_type(command_type::invalid), 
      number_type(number_type::object), 
      compare_type(number_compare_type::equal), 
      helper1(UINT16_MAX),
      helper2(t.type),
      value(cast_to_double(t.token)),
      data(t.data)
    {}
    
    script_data::script_data(const std::string_view &str) :
      command_type(command_type::invalid), 
      number_type(number_type::string), 
      compare_type(number_compare_type::equal), 
      helper1(UINT16_MAX),
      helper2(UINT16_MAX),
      value(0.0),
      data(nullptr)
    {
      data = copy_str_to_char_array(str);
    }
    
    script_data::script_data(const double &value, const uint8_t &compare_type) :
      command_type(command_type::invalid), 
      number_type(number_type::number), 
      compare_type(compare_type), 
      helper1(UINT16_MAX),
      helper2(UINT16_MAX),
      value(value),
      data(nullptr)
    {}
    
    script_data::~script_data() {
      clear();
      
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
      clear();
      
      command_type = move.command_type;
      number_type = move.number_type;
      compare_type = move.compare_type;
      helper1 = move.helper1;
      helper2 = move.helper2;
      value = move.value;
      data = move.data;
      move.data = nullptr;
      move.clear();
      
      return *this;
    }
    
    script_data & script_data::operator=(const struct target_t &t) {
      clear();
      number_type = number_type::object;
      helper2 = t.type;
      data = t.data;
      value = cast_to_double(t.token);
      return *this;
    }
    
    script_data & script_data::operator=(const std::string_view &str) {
      clear();
      number_type = number_type::string;
      data = copy_str_to_char_array(str);
      return *this;
    }
    
    script_data & script_data::operator=(const double &value) {
      clear();
      number_type = number_type::number;
      compare_type = number_compare_type::more_eq;
      this->value = value;
      return *this;
    }
    
    void script_data::clear() {
      switch (number_type) {
        case number_type::boolean: break;
        case number_type::number: break;
        case number_type::object: break;
        case number_type::stat: break;
        case number_type::array: 
        case number_type::lvalue_array: {
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
      
      command_type = command_type::invalid;
      number_type = number_type::boolean; 
      compare_type = number_compare_type::equal;
      helper1 = UINT16_MAX;
      helper2 = UINT16_MAX;
      value = 0.0;
      data = nullptr;
    }
    
    uint64_t random_state::next() {
      s = utils::xoroshiro128starstar::rng(s);
      return utils::xoroshiro128starstar::get_value(s);
    }
    
    context::context() :
      //root{UINT32_MAX, nullptr, SIZE_MAX},
      rnd(nullptr),
      itr_func(nullptr),
      //prev{UINT32_MAX, nullptr, SIZE_MAX}
      nest_level(0)
    {}
    
    const condition_function_p condition_functions[] = {
#define CONDITION_COMMAND_FUNC(name) &name,
      CONDITION_COMMANDS_LIST
#undef CONDITION_COMMAND_FUNC

#define STAT_FUNC(name) &name,
#define CHARACTER_PENALTY_STAT_FUNC(name) &name##_penalty,
        UNIQUE_STATS_LIST
#undef STAT_FUNC
#undef CHARACTER_PENALTY_STAT_FUNC
    };
    
    
    const action_function_p action_functions[] = {
#define ACTION_COMMAND_FUNC(name) &name,
      ACTION_COMMANDS_LIST
#undef ACTION_COMMAND_FUNC

#define STAT_FUNC(name) &add_##name,
#define CHARACTER_PENALTY_STAT_FUNC(name) &add_##name##_penalty,
        UNIQUE_STATS_LIST
#undef STAT_FUNC
#undef CHARACTER_PENALTY_STAT_FUNC
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
    
    const general_block_function_p general_block_functions[] = {
#define SCRIPT_BLOCK_COMMAND_FUNC(name) &name,
      SCRIPT_BLOCK_COMMANDS_LIST
#undef SCRIPT_BLOCK_COMMAND_FUNC
    };
    
    static_assert(condition_function::count == sizeof(condition_functions) / sizeof(condition_functions[0]));
    static_assert(action_function::count == sizeof(action_functions) / sizeof(action_functions[0]));
    static_assert(condition_block_function::count == sizeof(condition_block_functions) / sizeof(condition_block_functions[0]));
    static_assert(action_block_function::count == sizeof(action_block_functions) / sizeof(action_block_functions[0]));
    static_assert(general_block_function::count == sizeof(general_block_functions) / sizeof(general_block_functions[0]));
    
    static_assert(condition_function::count       < MAGIC_ENUM_RANGE_MAX);
    static_assert(action_function::count          < MAGIC_ENUM_RANGE_MAX);
    static_assert(condition_block_function::count < MAGIC_ENUM_RANGE_MAX);
    static_assert(action_block_function::count    < MAGIC_ENUM_RANGE_MAX);
    static_assert(general_block_function::count   < MAGIC_ENUM_RANGE_MAX);
    
    const function_init_p condition_init_functions[] = {
#define CONDITION_COMMAND_FUNC(name) &name##_init,
      CONDITION_COMMANDS_LIST
#undef CONDITION_COMMAND_FUNC

#define STAT_FUNC(name) &name##_init,
#define CHARACTER_PENALTY_STAT_FUNC(name) &name##_penalty_init,
        UNIQUE_STATS_LIST
#undef STAT_FUNC
#undef CHARACTER_PENALTY_STAT_FUNC
    };
    
    const function_init_p action_init_functions[] = {
#define ACTION_COMMAND_FUNC(name) &name##_init,
      ACTION_COMMANDS_LIST
#undef ACTION_COMMAND_FUNC

#define STAT_FUNC(name) &add_##name##_init,
#define CHARACTER_PENALTY_STAT_FUNC(name) &add_##name##_penalty_init,
        UNIQUE_STATS_LIST
#undef STAT_FUNC
#undef CHARACTER_PENALTY_STAT_FUNC
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
    
    const general_function_init_p general_block_init_functions[] = {
#define SCRIPT_BLOCK_COMMAND_FUNC(name) &name##_init,
      SCRIPT_BLOCK_COMMANDS_LIST
#undef SCRIPT_BLOCK_COMMAND_FUNC
    };
    
    static_assert(condition_function::count == sizeof(condition_init_functions) / sizeof(condition_init_functions[0]));
    static_assert(action_function::count == sizeof(action_init_functions) / sizeof(action_init_functions[0]));
    static_assert(condition_block_function::count == sizeof(condition_block_init_functions) / sizeof(condition_block_init_functions[0]));
    static_assert(action_block_function::count == sizeof(action_block_init_functions) / sizeof(action_block_init_functions[0]));
    static_assert(general_block_function::count == sizeof(general_block_init_functions) / sizeof(general_block_init_functions[0]));
    
    const std::string_view complex_var_regex = "(\\w+):(\\w+):([+-]?([0-9]+([.][0-9]*)?|[.][0-9]+))";
    const std::string_view dot_matcher = "[.]";
    const std::string_view colon_matcher = "[:]";
    const std::string_view number_matcher = "[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)";
    
    enum class unique_name_checker {
#define CONDITION_COMMAND_FUNC(name) name,
        CONDITION_COMMANDS_LIST
#undef CONDITION_COMMAND_FUNC
  
#define STAT_FUNC(name) name,
#define CHARACTER_PENALTY_STAT_FUNC(name) name##_penalty,
        UNIQUE_STATS_LIST
#undef STAT_FUNC
#undef CHARACTER_PENALTY_STAT_FUNC

#define ACTION_COMMAND_FUNC(name) name,
        ACTION_COMMANDS_LIST
#undef ACTION_COMMAND_FUNC

#define STAT_FUNC(name) add_##name,
#define CHARACTER_PENALTY_STAT_FUNC(name) add_##name##_penalty,
        UNIQUE_STATS_LIST
#undef STAT_FUNC
#undef CHARACTER_PENALTY_STAT_FUNC

#define SCRIPT_BLOCK_COMMAND_FUNC(name) name,
        SCRIPT_BLOCK_COMMANDS_LIST
#undef SCRIPT_BLOCK_COMMAND_FUNC

#define SCRIPT_BLOCK_COMMAND_FUNC(name) name,
        CONDITION_SCRIPT_BLOCK_COMMANDS_LIST
#undef SCRIPT_BLOCK_COMMAND_FUNC

#define SCRIPT_BLOCK_COMMAND_FUNC(name) name,
        ACTION_SCRIPT_BLOCK_COMMANDS_LIST
#undef SCRIPT_BLOCK_COMMAND_FUNC

      count
    };
    
    void init_condition(const uint32_t &target_type, const sol::object &obj, script_data* data) {
      condition_init(target_type, obj, data);
    }
    
    void init_action(const uint32_t &target_type, const sol::object &obj, script_data* data) {
      action_init(target_type, obj, data);
    }
    
    void init_string_from_script(const uint32_t &target_type, const sol::object &obj, script_data* data) {
      string_input_init("init_string_from_script", target_type, obj, data);
    }
    
    void init_number_from_script(const uint32_t &target_type, const sol::object &obj, script_data* data) {
      variable_input_init("init_number_from_script", target_type, obj, data);
    }
    
    std::string_view get_string_from_script(const target_t &t, context* ctx, const script_data* data) {
      return get_string_from_data(&t, ctx, data);
    }
    
    double get_number_from_script(const target_t &t, context* ctx, const script_data* data) {
      const auto &tuple = get_num_from_data(&t, ctx, data);
      return std::get<0>(tuple);
    }
  }
}
