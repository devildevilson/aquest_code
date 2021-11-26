#include "header.h"

#include "interface.h"
#include "core.h"
#include "object.h"
#include "init_functions.h"

namespace devils_engine {
  namespace script {
    
    // вход указывать не имеет смысла по идее, точнее нужен ожидаемый тип?
    // укажем в структуре: рут, прев, куррент, что то еще?
    void create_number(const input_data &input, number* num, const sol::object &obj) {
      if (!obj.valid()) return;
      
      input_data new_input = input;
      new_input.script_type = script_types::numeric;
      new_input.expected_types = object::type_bit::valid_number;
      
      const auto [invalid_ptr, unnessesary_count, init_size] = number_container_init(new_input, obj, nullptr);
      container cont(init_size, 8);
      const auto [ptr, count, size] = number_container_init(new_input, obj, &cont);
      num->init(std::move(cont), ptr);
    }
    
    void create_string(const input_data &input, string* num, const sol::object &obj) {
      if (!obj.valid()) return;
      
      input_data new_input = input;
      new_input.script_type = script_types::string;
      new_input.expected_types = object::type_bit::string;
      
      const auto [invalid_ptr, unnessesary_count, init_size] = string_container_init(new_input, obj, nullptr);
      container cont(init_size, 8);
      const auto [ptr, count, size] = string_container_init(new_input, obj, &cont);
      num->init(std::move(cont), ptr);
    }
    
    void create_condition(const input_data &input, condition* num, const sol::object &obj) {
      if (!obj.valid()) return;
      
      input_data new_input = input;
      new_input.script_type = script_types::condition;
      new_input.expected_types = object::type_bit::valid_boolean;
      
      const auto [invalid_ptr, unnessesary_count, init_size] = boolean_container_init(new_input, obj, nullptr);
      container cont(init_size, 8);
      const auto [ptr, count, size] = boolean_container_init(new_input, obj, &cont);
      num->init(std::move(cont), ptr);
    }
    
    void create_effect(const input_data &input, effect* num, const sol::object &obj) {
      if (!obj.valid()) return;
      
      input_data new_input = input;
      new_input.script_type = script_types::effect;
      new_input.expected_types = 0;
      
      const auto [invalid_ptr, unnessesary_count, init_size] = change_scope_effect_init(new_input, obj, nullptr);
      container cont(init_size, 8);
      const auto [ptr, count, size] = change_scope_effect_init(new_input, obj, &cont);
      num->init(std::move(cont), ptr);
    }
  }
}
