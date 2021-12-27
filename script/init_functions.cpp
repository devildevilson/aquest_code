#include "init_functions.h"

#include "utils/constexpr_funcs.h"
#include "interface.h"
#include "container.h"
#include "common_commands.h"
#include "numeric_functions.h"
#include "condition_functions.h"
#include "change_context_commands.h"
#include "action_functions.h"
#include "logic_commands.h"
#include "get_scope_commands.h"
#include "core.h"
#include "utils/globals.h"
#include "core/realm_mechanics_arrays.h"

#include <charconv>
#include <iostream>
#include "re2/re2.h"

// для дебага то поди нужно добавить какой то декоратор, хотя стек я могу посмотреть в gdb

#define DEBUG_FUNC_DECORATOR(bool_val) \
  if (bool_val) { std::cout << "Entered " << __func__ << "\n"; }
  
// как же можно такие функции генерировать? 

namespace devils_engine {
  namespace script {
    static const RE2 regex_number_matcher(number_matcher);
    
    // слева (то есть ключ в луа таблице) может быть только булеан, число или строка
    static bool valid_lvalue(const sol::object &data) {
      const auto type = data.get_type();
      return type == sol::type::boolean || type == sol::type::number || type == sol::type::string;
    }
    
    // справа (то есть значение в луа таблице) может быть только булеан, число, строка и таблица
    static bool valid_rvalue(const sol::object &data) {
      const auto type = data.get_type();
      return type == sol::type::boolean || type == sol::type::number || type == sol::type::string || type == sol::type::table;
    }
    
    static bool valid_complex_variable(const std::string_view &str) {
      const bool has_dot = str.find('.') != std::string_view::npos;
      const bool has_colon = str.find(':') != std::string_view::npos;
      const auto itr = complex_variable_valid_string::map.find(str);
      return has_dot || has_colon || itr != complex_variable_valid_string::map.end();
    }
    
    template <typename T>
    static std::tuple<interface*, size_t, size_t> complex_scope(
      const input_data &input, 
      const sol::object &data, 
      container* cont, 
      const sol::object &lvalue, 
      const table_init_func_t table_init, 
      const init_func_p parent_func
    ) {
      assert(lvalue.get_type() == sol::type::string);
      assert(data.get_type() == sol::type::table);
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) { offset = cont->add_delayed<T>(); }
      size_t final_count = 1;
      size_t final_size = sizeof(T);
      interface* final_ptr = nullptr;
      
      size_t new_current = SIZE_MAX;
      input_data new_input = input;
      
      interface* scope = nullptr;
      {
        new_input.script_type = script_types::object;
        new_input.expected_types = script::object::type_bit::all;
        const auto [ptr, count, size] = complex_object_init(new_input, lvalue, cont, new_current);
        scope = ptr;
        final_count += count;
        final_size += size;
      }
      
      new_input.prev = new_input.current;
      new_input.current = new_current;
      
      const auto t = data.as<sol::table>();
      
      interface* condition = nullptr;
      if (const auto proxy = t["condition"]; proxy.valid()) {
        new_input.script_type = script_types::condition;
        new_input.expected_types = script::object::type_bit::valid_boolean;
        const auto [ptr, count, size] = AND_init(new_input, proxy, cont); /* по умолчанию возьмем AND */
        condition = ptr;
        final_count += count;
        final_size += size;
      }
      
      interface* childs = nullptr;
      {
        
        const auto [ptr, count, size] = table_init(new_input, data, cont, parent_func);
        childs = ptr;
        final_count += count;
        final_size += size;
      }
      
      if (cont != nullptr) {
        assert(scope != nullptr);
        assert(childs != nullptr);
        auto init = cont->get_init<T>(offset);
        final_ptr = init.init(scope, condition, childs);
      }
      
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
    //static std::tuple<interface*, size_t, size_t> complex_object_init(const input_data &input, const sol::object &data, container* cont, interface* lvalue) 
    static std::tuple<interface*, size_t, size_t> complex_object_init(const input_data &input, const sol::object &data, container* cont, uint32_t &compare_operation_value, size_t &new_current) {
      // сюда к нам приходит строка, мы ее должны распарсить на части
      // строка вида "*особое действие или тип сравнения*:*число или данные для действия*.*следущий скоуп*. ... .*название переменной из скоупа*"
      // имеет смысл парсить иначе, сначала делим строку на '.', обходим это дело и смотрим если в строке есть ':', то это взятие контекста например
      // вообще так можно еще обрабатывать функции с одним аргументом, то есть строчки вида: контекст:майн_тайтл.икуалс_ту.контекст:сом_тайтл
      // тогда контекст:сом_тайтл рассматриваются как вход для функции, возможно в этом случае нужно рифтануть синтаксис, добавить '='?
      // тип: контекст:майн_тайтл.икуалс_ту=контекст:сом_тайтл, возможно тогда имеет смысл еще обрабатывать пробелы, чуть позже
      assert(data.get_type() == sol::type::string);
      
      sol::state_view s = data.lua_state();
      const auto str = data.as<std::string_view>();
      assert(!str.empty());
      
      if (str == commands::names[commands::root] || str == "context:root") {
        if ((input.expected_types & input.root) != input.root) throw std::runtime_error("Expected from root " + parse_type_bits(input.expected_types) + ", got " + parse_type_bits(input.root));
        new_current = input.root;
        return root_init(input, sol::nil, cont); // rvalue
      }
      
      if (str == commands::names[commands::prev] || str == "context:prev") {
        if ((input.expected_types & input.prev) != input.prev) throw std::runtime_error("Expected from prev " + parse_type_bits(input.expected_types) + ", got " + parse_type_bits(input.prev));
        new_current = input.prev;
        return prev_init(input, sol::nil, cont);
      }
      
      if (str == commands::names[commands::current] || str == "context:current") {
        if ((input.expected_types & input.current) != input.current) throw std::runtime_error("Expected from current " + parse_type_bits(input.expected_types) + ", got " + parse_type_bits(input.current));
        new_current = input.current;
        return current_init(input, sol::nil, cont);
      }
      
      const size_t max_size_colon = 16;
      std::array<std::string_view, max_size_colon> colon_array;
      const size_t colon_count = divide_token(str, ":", max_size_colon, colon_array.data());
      if (colon_count == SIZE_MAX) throw std::runtime_error("Invalid string token " + std::string(str));
      
      const auto after_colon = colon_array[colon_count-1];
      const size_t max_count = 256;
      std::array<std::string_view, max_count> arr;
      const size_t count = divide_token(after_colon, ".", max_count, arr.data());
      if (count == SIZE_MAX) throw std::runtime_error("Invalid string token " + std::string(str));
      
//       for (size_t i = 0; i < count; ++i) {
//         const auto cur = arr[i];
//         std::array<std::string_view, max_size_colon> colon_array;
//         const size_t colon_count = divide_token(cur, ":", max_size_colon, colon_array.data());
//         if (colon_count == SIZE_MAX) throw std::runtime_error("Invalid string token " + std::string(cur));
//         
//         if (colon_count > 1) { // проверяем контекст и особые переменные
//           for (size_t j = 0; j < colon_count; ++j) {
//             
//           }
//         } else { // ищем обычную функцию
//           assert(colon_count == 1);
//           
//         }
//       }
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<script::complex_object>();
      
      interface* begin = nullptr;
      interface* current = nullptr;
      size_t final_count = 0;
      size_t final_size = 0;
      
      input_data in = input;
      in.expected_types = script::object::type_bit::all;
      
      bool has_context = false;
      uint32_t compare_op = UINT32_MAX;
      size_t point_start = 0;
      for (size_t i = 0; i < colon_count-1; ++i) {
        if (i == 0 && RE2::FullMatch(colon_array[i], regex_number_matcher)) continue;
        
        const auto token = colon_array[i];
        // смотрим тип информации который указан в строке, это может быть тип сравнения + число
        if (const auto itr = compare_operators::map.find(token); itr != compare_operators::map.end()) {
          // может ли у меня быть сравнение со сложным числом? вообще по идее может
          // то есть мы должны сначала указать сравнение, а потом либо context, либо особое число
          // для сравнения нам нужно два числа, но если тут нет числа или тип не совпадает то вылетаем?
          if (input.script_type == script_types::effect || input.script_type == script_types::string) throw std::runtime_error("Compare operator must not be specified in effect script");
          if (input.script_type == script_types::object) throw std::runtime_error("Compare operator must not be specified in get object script");
          // кондишен?
          compare_op = itr->second;
          continue;
        }
        
        // это может быть взятие из контекста + id переменной + возможно проход дальше по функциям смены контекста
        if (token == "context") {
          // создаем функцию получения из контекста, контекст должен быть завершающим в череде данных через двоеточие
          assert(i == colon_count-2);
          point_start = 1;
          has_context = true;
          const auto context_id = arr[0];
          if (cont != nullptr) { begin = current = cont->add<script::get_context>(context_id); }
          final_count += 1;
          final_size += sizeof(script::get_context);
          in.current = object::type_bit::all;
          continue;
        }
        
        if (token == "title" || token == "titulus") {
          assert(i == colon_count-2);
          point_start = 1;
          has_context = true;
          const auto entity_id = arr[0];
          auto str_ptr = cont->add<script::string_container>(entity_id);
          begin = current = cont->add<script::get_titulus>(str_ptr);
          final_count += 2;
          final_size += sizeof(script::get_titulus) + sizeof(script::string_container);
          in.current = script::get_titulus::output_type;
          continue;
        }
        
#define GET_ENTITY_COMPLEX_CASE_FUNC(name)                               \
        if (token == #name) {                                            \
          assert(i == colon_count-2);                                    \
          point_start = 1;                                               \
          has_context = true;                                            \
          const auto entity_id = arr[0];                                 \
          auto str_ptr = cont->add<script::string_container>(entity_id); \
          begin = current = cont->add<script::get_##name>(str_ptr);      \
          final_count += 2;                                              \
          final_size += sizeof(script::get_##name) + sizeof(script::string_container); \
          in.current = script::get_##name::output_type;                  \
          continue;                                                      \
        }                                                                \
        
        GET_ENTITY_COMPLEX_CASE_FUNC(building_type)
        GET_ENTITY_COMPLEX_CASE_FUNC(holding_type)
        GET_ENTITY_COMPLEX_CASE_FUNC(city_type)
        GET_ENTITY_COMPLEX_CASE_FUNC(trait)
        GET_ENTITY_COMPLEX_CASE_FUNC(modificator)
        GET_ENTITY_COMPLEX_CASE_FUNC(troop_type)
        GET_ENTITY_COMPLEX_CASE_FUNC(religion_group)
        GET_ENTITY_COMPLEX_CASE_FUNC(religion)
        GET_ENTITY_COMPLEX_CASE_FUNC(culture_group)
        GET_ENTITY_COMPLEX_CASE_FUNC(culture)
        GET_ENTITY_COMPLEX_CASE_FUNC(law)
        GET_ENTITY_COMPLEX_CASE_FUNC(casus_belli)
        
#undef GET_ENTITY_COMPLEX_CASE_FUNC
        
        // это может быть особое число (например месячный доход) + множитель (в том числе отрицательный)
        if (const auto itr = complex_object_tokens::map.find(token); itr != complex_object_tokens::map.end()) {
          assert(false);
          continue;
        }
      }
      
      if (compare_op != UINT32_MAX) compare_operation_value = compare_op;
      
      if (colon_count != 1 && !has_context) {
        // ожидаем что последняя строка после ':' - это число
        const auto last_str = after_colon;
        if (!RE2::FullMatch(last_str, regex_number_matcher)) throw std::runtime_error("Context is undefined in string " + std::string(str) + " expected last section to be number, got " + std::string(last_str));
        // как же я число то находил?
        double num = 0.0;
        const auto res = std::from_chars(last_str.begin(), last_str.end(), num);
        if (res.ec == std::errc::invalid_argument || res.ec == std::errc::result_out_of_range) {
          throw std::runtime_error("Could not parse number " + std::string(last_str));
        }
        
        interface* ptr = nullptr;
        if (cont != nullptr) {
          ptr = cont->add<script::number_container>(num);
        }
        begin = ptr;
        final_count += 1;
        final_size += sizeof(script::number_container);
      } else {
        // если строка до двоеточия пустая, то это последовательное выполнение функций из текущего контекста (хотя в ином случае тоже может быть)
        for (size_t i = point_start; i < count; ++i) {
          const auto cur = arr[i];
          // находим функцию, нужен способ обновить инпут
          // инпут обновляется в функциях создания, но в них передается луа объект
          // если передавать туда строку, смогу ли я получить нужный мне результат?
          // то есть если в функцию например реалм приходит строка, то он должен ее воспринять не иначе как сложный объект
          // вообще это реально
          
          const auto itr = commands::map.find(cur);
          if (itr == commands::map.end()) throw std::runtime_error("Could not find script command '" + std::string(cur) + "'");
          
          const auto func = commands::inits[itr->second];
          const auto name_obj = sol::make_object(s, cur);
          const auto [ptr, type, size] = func(in, name_obj, cont);
          in.prev = in.current;
          in.current = type;
          
          final_count += 1;
          final_size += size;
          if (begin == nullptr) begin = ptr;
          if (current != nullptr) current->next = ptr;
          current = ptr;
        }
      }
      
      // последний in.current можно проверить на input.expected_types, чтобы понимать что к нам пришло
      if ((input.expected_types & in.current) != in.current) throw std::runtime_error("Wrong expectation from complex variable " + std::string(str) + ", expected " + parse_type_bits(input.expected_types));
      new_current = in.current;
      
      // имеет смысл запустить чистку мусора перед выходом? вряд ли
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        auto init = cont->get_init<script::complex_object>(offset);
        final_ptr = init.init(begin);
      }
      final_count += 1;
      final_size += sizeof(script::complex_object);
      
      // так же у нас может быть определен compare_op, а это значит что мы собирали rvalue
      // нужно создать number_comparator и положить в него текущий complex_object и lvalue
      // lvalue точно определен, а compare_op - нет, ко всему прочему даже если у нас определен lvalue 
      // комплекс валью может быть задействовано в add_money функции
      // хотя там вроде бы мы не задаем lvalue в качестве входа
      // наверное нелогично тут создавать number_comparator и лучше это сделать выше по стеку, да но как распарсить compare_op?
      // lvalue будет nullptr при первом обходе, как я говорил нужна более хорошая проверка
      // либо нужно все же создавать сравнение выше по стеку
//       if (input.script_type == script_types::numeric && lvalue != nullptr) { // compare_op != UINT32_MAX
//         assert(compare_op < compare_operators::count);
//         interface* ptr = nullptr;
//         if (cont != nullptr) ptr = cont->add<script::number_comparator>(compare_op, lvalue, final_ptr);
//         final_ptr = ptr;
//         final_count += 1;
//         final_size += sizeof(script::number_comparator);
//       }
      
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
    std::tuple<interface*, size_t, size_t> complex_object_init(const input_data &input, const sol::object &data, container* cont, size_t &new_current) {
      uint32_t unused = UINT32_MAX;
      return complex_object_init(input, data, cont, unused, new_current);
    }
    
    std::tuple<interface*, size_t, size_t> complex_object_init(const input_data &input, const sol::object &data, container* cont) {
      uint32_t unused = UINT32_MAX;
      size_t new_current = SIZE_MAX;
      return complex_object_init(input, data, cont, unused, new_current);
    }
    
    template <typename T>
    static std::tuple<interface*, size_t, size_t> create_block(const input_data &input, const sol::object &data, container* cont, const table_init_func_t &table_init, const init_func_p &parent_init) {
      const auto table = data.as<sol::table>();
      size_t counter = 0;
      // вот это ломает блок ребенка с одним членом, то есть сюда потенциально может придти таблица NAND, но в если в ней окажется лишь один ребенок, то вернет его
      // а это сломает отрицание в функциях NAND и NOR (а остальные?), желательно конечно во все функции добавить указателя на отца
      // пока что проблема только с NAND, NOR
      for (const auto &pair : table) { counter += size_t(valid_lvalue(pair.first) && valid_rvalue(pair.second)); }
      if (counter == 0) throw std::runtime_error("Could not find children for '" + std::string(commands::names[T::type_index]) + "' function");
      if (counter == 1 && T::type_index != commands::NAND && T::type_index != commands::NOR) return table_init(input, data, cont, parent_init);
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<T>();
      
      const auto [ptr, count, size] = table_init(input, data, cont, parent_init);
      
      interface* operation = nullptr;
      if (cont != nullptr) {
        auto init = cont->get_init<T>(offset);
        operation = init.init(ptr);
      }
      
      return std::make_tuple(operation, count+1, size+sizeof(T));
    }

    template <typename T, typename C>
    static std::tuple<interface*, size_t, size_t> create_compute(
      const input_data &input, 
      const sol::object &data, 
      container* cont, 
      interface* scope, 
      const table_init_func_t &table_init, 
      const init_func_p &parent_init
    ) {
      size_t offset = SIZE_MAX;
      if (cont != nullptr) { offset = cont->add_delayed<C>(); }
      
      size_t counter = 1;
      size_t final_size = sizeof(C);
      interface* condition = nullptr;
      const auto table = data.as<sol::table>();
      if (const auto proxy = table["condition"]; proxy.valid()) {
        input_data new_input = input;
        new_input.script_type = script_types::condition;
        new_input.expected_types = script::object::type_bit::valid_boolean;
        const auto [ptr, count, size] = AND_init(new_input, proxy, cont); /* по умолчанию возьмем AND */
        condition = ptr;
        counter += count;
        final_size += size;
      }
      
      interface* operation = nullptr;
      {
        // можно использовать функцию инициализации из T
        const auto [ptr, count, size] = create_block<T>(input, data, cont, table_init, parent_init);
        operation = ptr;
        counter += count;
        final_size += size;
      }
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        auto init = cont->get_init<C>(offset);
        final_ptr = init.init(scope, condition, operation);
      }
      
      return std::make_tuple(final_ptr, counter, final_size);
    }
    
    static std::tuple<interface*, size_t, size_t> change_scope_condition_init(const input_data &input, const sol::object &data, container* cont, interface* scope) {
      if (input.script_type != script_types::condition) throw std::runtime_error("script type must be condition");
      if ((input.expected_types & script::change_scope_condition::output_type) == 0) throw std::runtime_error("bad expectation");
      
      if (data.get_type() == sol::type::string && data.as<std::string_view>() == "change_scope_condition") { 
        throw std::runtime_error("Function 'change_scope_condition' could not be used in complex function");
      }
      
      return create_compute<script::AND, script::change_scope_condition>(input, data, cont, scope, &condition_table_init, &AND_init);
    }
    
    static std::tuple<interface*, size_t, size_t> compute_number_init(const input_data &input, const sol::object &data, container* cont, interface* scope) {
      if (input.script_type != script_types::numeric) throw std::runtime_error("Script type must be numeric");
      if ((input.expected_types & script::compute_number::output_type) == 0) throw std::runtime_error("Bad expectation");
      
      if (data.get_type() == sol::type::string && data.as<std::string_view>() == "compute_number") { 
        throw std::runtime_error("Function 'compute_number' could not be used in complex function");
      }
      
      return create_compute<script::add, script::compute_number>(input, data, cont, scope, &numeric_table_init, &add_init);
    }
    
    // это подойдет только для нумериков, сюда мы приходим с уже сделанной функцией выше? скорее всего, тут собираем детей
    // инпут проверяем в функциях детей
    std::tuple<interface*, size_t, size_t> numeric_table_init(const input_data &input, const sol::object &data, container* cont, const init_func_p parent_func) {
      assert(input.script_type == script_types::condition || input.script_type == script_types::numeric);
      interface* begin = nullptr;
      interface* cur = nullptr;
      size_t accum_count = 0;
      size_t accum_size = 0;
      sol::state_view s = data.lua_state();
      const auto table = data.as<sol::table>();
      // возможно кондишен мы должны обработать выше
      //if (const auto proxy = table["condition"]; proxy.valid()) { return add_init(input, proxy, cont); }
      
      for (const auto &pair : table) {
        if (pair.first.get_type() == sol::type::boolean || pair.first.get_type() == sol::type::number) {
          std::tuple<interface*, size_t, size_t> tuple(nullptr, 0, 0);
          switch (pair.second.get_type()) {
            case sol::type::nil: break;
            case sol::type::boolean:
            case sol::type::number: {
              tuple = number_container_init(input, pair.second, cont);
              break;
            }
            
            case sol::type::string: {
              const auto str = pair.second.as<std::string_view>();
              const auto itr = commands::map.find(str);
              if (itr == commands::map.end()) {
                input_data new_input = input;
//                 new_input.script_type = script_types::numeric;
                tuple = complex_object_init(new_input, pair.second, cont);
                break;
              }
              
              const auto func = commands::inits[itr->second];
              tuple = func(input, sol::nil, cont);
              break;
            }
            
            case sol::type::table: {
              const auto t = pair.second.as<sol::table>();
              if (const auto proxy = t["condition"]; proxy.valid()) tuple = parent_func(input, pair.second, cont);
              else tuple = numeric_table_init(input, pair.second, cont, parent_func);
              break;
            }
            
            // нужно ли вылетать при неверном типе? я бы наверное просто ворнинг отправлял
            default: {
              const sol::function type = s["type"];
              const sol::function tostring = s["tostring"];
              const auto obj = type(pair.second);
              const std::string type_str = obj;
              const std::string key_str = tostring(pair.first);
              std::cout << "Ignored key " << key_str << " with value of type " << type_str << "\n";
              //throw std::runtime_error("Unsupported value type " + type_str);
            }
          }
          
          const auto [ptr, count, size] = tuple;
          interface* next = ptr;
          if (cur != nullptr) cur->next = next;
          if (begin == nullptr) begin = next;
          //cur = next;
          while (next != nullptr) { cur = next; next = next->next; }
          accum_count += count;
          accum_size += size;
          continue;
        }
        
        // здесь может быть два варианта: либо это название функции, либо это сложный скоуп (может быть указание на какое то числовое значение? вообще может быть)
        // тип: ["root.realm.money"] = 300, - вообще этот же эффект не сложно получить через просто функции
        // а может быть сложная команда? ["root.realm.add_money"] = 300, - сложная команда вряд ли, там нужно указать в команду число а как я могу это сделать при создании?
        // придется как то по особому изворачиваться, а эт костыль, значит некоторые функции должны выдавать ошибку в случае создания сложных объектов
        // тут нужно в два этапа создавать? сначала парсим лвалуе в надежде найти там скоуп (то есть например реалм) (мы можем зафорсить такое поведение с помощью expected_types),
        // затем парсим рвалуе - это должна быть таблица (?), а в ней обычное перечисление сложения
        // при любых раскладах, если ключ это строка то ожидаем таблицу для нумерика
        // как мы поймем что это сложный скоуп? мы не найдем строку среди названий функций
        // кстати теперь по идее весь список функций можно держать в одном месте
        if (pair.first.get_type() == sol::type::string) {
          std::tuple<interface*, size_t, size_t> tuple(nullptr, 0, 0);
          const auto str = pair.first.as<std::string_view>();
          // почему только таблица? 
//           if (pair.second.get_type() != sol::type::table) {
//             const sol::function type = s["type"];
//             const auto obj = type(pair.second);
//             const std::string type_str = obj;
//             throw std::runtime_error("Expected table in any string key value, got " + type_str + ", key " + std::string(str));
//           }
          
          // ищем штуки здесь отдельно ручками
          // указание на то что пропущено скорее будет сбивать с толку
          if (const auto ignore_key = ignore_keys::map.find(str); ignore_key != ignore_keys::map.end()) continue;
          
          const auto itr = commands::map.find(str);
          if (itr == commands::map.end()) {
            if (pair.second.get_type() != sol::type::table) throw std::runtime_error("Expected rvalue to be table for complex scope lvalue");
            if (!valid_complex_variable(str)) throw std::runtime_error(std::string(str) + " is not valid lvalue");
            // или мы можем взять parent_func
            tuple = complex_scope<script::compute_number>(input, pair.second, cont, pair.first, &numeric_table_init, &add_init);
          } else {
            // тут тогда непонятно как догадаться до parent_func, поэтому тут будем брать по умолчанию
            // нужно ли переделать именно для parent_func? нет пусть наверное будет по умолчанию для каждого определенного lvalue
            const auto func = commands::inits[itr->second];
            tuple = func(input, pair.second, cont);
          }
          
          const auto [ptr, count, size] = tuple;
          interface* next = ptr;
          if (cur != nullptr) cur->next = next;
          if (begin == nullptr) begin = next;
          //cur = next;
          while (next != nullptr) { cur = next; next = next->next; }
          accum_count += count;
          accum_size += size;
          continue;
        }
        
        const sol::function type = s["type"];
        const auto obj = type(pair.second);
        const std::string type_str = obj;
        //throw std::runtime_error("Unsupported key type " + type_str);
        std::cout << "Ignored key type " << type_str << "\n";
      }
      
      return std::make_tuple(begin, accum_count, accum_size);
    }
    
    std::tuple<interface*, size_t, size_t> string_table_init(const input_data &input, const sol::object &data, container* cont, const init_func_p parent_func) {
      assert(input.script_type == script_types::string);
      assert(parent_func == nullptr);
      interface* begin = nullptr;
      interface* cur = nullptr;
      size_t accum_count = 0;
      size_t accum_size = 0;
      sol::state_view s = data.lua_state();
//       interface* condition = nullptr;
//       size_t offset = SIZE_MAX;
      const auto table = data.as<sol::table>();
//       if (const auto proxy = table["condition"]; proxy.valid()) {
//         if (cont != nullptr) offset = cont->add_delayed<script::compute_string>();
//         input_data new_input = input;
//         new_input.script_type = script_types::condition;
//         const auto [ptr, count, size] = AND_init(new_input, proxy, cont); /* по умолчанию возьмем AND */
//         condition = ptr;
//         accum_count += count+1;
//         accum_size += size + sizeof(script::compute_string);
//       }
      
      for (const auto &pair : table) {
        if (pair.first.get_type() == sol::type::string) {
          if (pair.first.as<std::string_view>() == "condition") continue;
          throw std::runtime_error("Key " + pair.first.as<std::string>() + " is not allowed in string script");
        }
        
        // тут допустимыми значениями являются кондишены и строки (и с таблицей мы просто перезапустим функцию)
        // конечная функция должна вернуть последнюю строку, то есть у нас может быть ситуация:
        // { "str1", { "condition" = {}, "str2" } } - алгоритм должен пройтись по тому что имеет 
        // и запомнить последний валидный объект который НЕ игнор (ну и он конечно должен быть типа строка)
        // видимо нужно создать новый контейнер на каждую таблицу в которой есть кондишен, а если кондишена нет? то можно добавить все в один
        
        if (pair.first.get_type() != sol::type::number) throw std::runtime_error("Bad key type in string script");
        
        std::tuple<interface*, size_t, size_t> tuple(nullptr, 0, 0);
        if (pair.second.get_type() == sol::type::table) {
          const auto t = pair.second.as<sol::table>();
          if (const auto proxy = t["condition"]; proxy.valid()) tuple = compute_string_init(input, pair.second, cont);
          else tuple = string_table_init(input, pair.second, cont, parent_func);
        } 
        else if (pair.second.get_type() == sol::type::string) tuple = string_container_init(input, pair.second, cont);
        else throw std::runtime_error("Bad value type in string script");
        
        const auto [ptr, count, size] = tuple;
        interface* next = ptr;
        if (cur != nullptr) cur->next = next;
        if (begin == nullptr) begin = next;
        //cur = next;
        while (next != nullptr) { cur = next; next = next->next; }
        accum_count += count;
        accum_size += size;
      }
      
//       if (condition != nullptr && cont != nullptr) {
//         auto init = cont->get_init<script::compute_string>(offset);
//         auto ptr = init.init(condition, begin);
//         begin = ptr;
//       }
      
      return std::make_tuple(begin, accum_count, accum_size);
    }
    
    // что тут? тут самое большое количество разных штук
    // могут ли тут появиться просто числа? возможно стоит ожидать тут просто булеан, тип чтобы быстро выключить часть проверки
    // (а закомментить? пустой кондишен всегда возвращает труе, поэтому имеет смысл поставить false первым значением и комментить уже его)
    // здесь в качестве лвалуе может появиться строка указывающая на некое числовое значение, как такие отлавливать?
    // возможно нужно возвращать еще конечный тип для сложного объекта
    // слева таким образом у нас может быть: число (группирование условий), строка - название функции (особое условие), строка - переход по скоупу
    // справа могут появиться: число, строка (у меня еще может быть сравнение строк), таблица
    // когда мы встречаем realm например, мы должны создать change_scope_condition, где скоуп будет функцией реалма, возможно будет кондишен и далее обычные условия
    // так expected_types не будет нарушен
    // при группировке условий опять же если есть кондишен, то создаем новый контейнер, в ином случае просто последовательно кладем все подряд
    // наверное тут тоже не имеет смысла проверять числовые ключи строго после строковых, лучше пусть последовательно все это дело будет
    std::tuple<interface*, size_t, size_t> condition_table_init(const input_data &input, const sol::object &data, container* cont, const init_func_p parent_func) {
      assert(input.script_type == script_types::condition || input.script_type == script_types::numeric);
      interface* begin = nullptr;
      interface* cur = nullptr;
      size_t accum_count = 0;
      size_t accum_size = 0;
      sol::state_view s = data.lua_state();
      const auto table = data.as<sol::table>();
      // возможно кондишен мы должны обработать выше
      //if (const auto proxy = table["condition"]; proxy.valid()) { return AND_init(input, proxy, cont); }
      
      for (const auto &pair : table) {
        if (pair.first.get_type() == sol::type::boolean || pair.first.get_type() == sol::type::number) {
          std::tuple<interface*, size_t, size_t> tuple(nullptr, 0, 0);
          switch (pair.second.get_type()) {
            case sol::type::nil: break;
            case sol::type::boolean: 
            case sol::type::number: tuple = boolean_container_init(input, pair.second, cont); break;
            case sol::type::string: {
              const auto str = pair.second.as<std::string_view>();
              const auto itr = commands::map.find(str);
              if (itr == commands::map.end()) {
                input_data new_input = input;
                tuple = complex_object_init(new_input, pair.second, cont);
                break;
              }
              
              const auto func = commands::inits[itr->second];
              tuple = func(input, sol::nil, cont);
              break;
            }
            
            case sol::type::table: {
              const auto t = pair.second.as<sol::table>();
              if (const auto proxy = t["condition"]; proxy.valid()) tuple = parent_func(input, pair.second, cont); // change_scope_condition_init
              else tuple = condition_table_init(input, pair.second, cont, parent_func);
              break;
            }
            
            default: {
              const sol::function type = s["type"];
              const sol::function tostring = s["tostring"];
              const auto obj = type(pair.second);
              const std::string type_str = obj;
              const std::string key_str = tostring(pair.first);
              std::cout << "Ignored key " << key_str << " with value of type " << type_str << "\n";
//               throw std::runtime_error("Unsupported value type " + type_str);
            }
          }
          
          const auto [ptr, count, size] = tuple;
          interface* next = ptr;
          if (cur != nullptr) cur->next = next;
          if (begin == nullptr) begin = next;
//           cur = next;
//           if (next != nullptr) {
//             auto next_seq = next;
          while (next != nullptr) { cur = next; next = next->next; }
//           }
          accum_count += count;
          accum_size += size;
          continue;
        }
        
        if (pair.first.get_type() == sol::type::string) {
          std::tuple<interface*, size_t, size_t> tuple(nullptr, 0, 0);
          const auto str = pair.first.as<std::string_view>();
          
          // возможно только касается condition?
          // указание на то что пропущено скорее будет сбивать с толку
          if (const auto ignore_key = ignore_keys::map.find(str); ignore_key != ignore_keys::map.end()) continue;
          
          const auto itr = commands::map.find(str);
          if (itr == commands::map.end()) {
            if (pair.second.get_type() != sol::type::table) throw std::runtime_error("Expected rvalue to be table for complex scope lvalue");
            if (!valid_complex_variable(str)) throw std::runtime_error(std::string(str) + " is not valid lvalue");
            tuple = complex_scope<script::change_scope_condition>(input, pair.second, cont, pair.first, &condition_table_init, &AND_init); // по умолчанию AND_init
          } else {
            const auto func = commands::inits[itr->second];
            tuple = func(input, pair.second, cont);
          }
          
          const auto [ptr, count, size] = tuple;
          interface* next = ptr;
          if (cur != nullptr) cur->next = next;
          if (begin == nullptr) begin = next;
          //cur = next;
          while (next != nullptr) { cur = next; next = next->next; }
          accum_count += count;
          accum_size += size;
          
          continue;
        }
        
        const sol::function type = s["type"];
        const auto obj = type(pair.second);
        const std::string type_str = obj;
        std::cout << "Ignored key type " << type_str << "\n";
        //throw std::runtime_error("Unsupported key type " + type_str);
      }
      
      return std::make_tuple(begin, accum_count, accum_size);
    }
    
    static std::tuple<interface*, size_t, size_t> effect_table_init(const input_data &input, const sol::object &data, container* cont, const init_func_p parent_func) {
      assert(parent_func == nullptr);
      return effect_table_init(input, data, cont);
    }
    
    // эффекты отличаются от условий только тем что ничего не возвращают (ТОЛЬКО инвалидный объект)
    // игнор не нужен в эффектах, нет, игнор может пригодиться в будущем (с помощью него можно сделать if_else)
    // также в эффектах не должны случайно появиться кондитион функций, по идее это мы можем проверить если expected_types == type_bits::invalid
    // но при этом в эффектах может быть смена контекста разными способами (ну тут опять же создаем change_scope_action)
    // эффекты как обычно можно сгруппировать и для группы назначить условие 
    // (да тут интересный баг намечается, если до проверки условия изменить стат, то потом условие может по итогу изменить значение)
    // (как избежать, вообще наверное применение изменений должно быть отложенно или все условия должны быть вычисленны заранее (что проще))
    std::tuple<interface*, size_t, size_t> effect_table_init(const input_data &input, const sol::object &data, container* cont) {
      assert(input.script_type == script_types::effect);
      interface* begin = nullptr;
      interface* cur = nullptr;
      size_t accum_count = 0;
      size_t accum_size = 0;
      sol::state_view s = data.lua_state();
      const auto table = data.as<sol::table>();
      // возможно кондишен мы должны обработать выше
      //if (const auto proxy = table["condition"]; proxy.valid()) { return change_scope_effect_init(input, proxy, cont); }
      
      for (const auto &pair : table) {
        if (pair.first.get_type() == sol::type::boolean || pair.first.get_type() == sol::type::number) {
          std::tuple<interface*, size_t, size_t> tuple(nullptr, 0, 0);
          switch (pair.second.get_type()) {
            case sol::type::nil: break;
            
            case sol::type::table: {
              const auto t = pair.second.as<sol::table>();
              if (const auto proxy = t["condition"]; proxy.valid()) tuple = change_scope_effect_init(input, pair.second, cont);
              else tuple = effect_table_init(input, pair.second, cont);
              break;
            }
            
            default: {
              const sol::function type = s["type"];
              const sol::function tostring = s["tostring"];
              const auto obj = type(pair.second);
              const std::string type_str = obj;
              const std::string key_str = tostring(pair.first);
              std::cout << "Ignored key " << key_str << " with value of type " << type_str << "\n";
//               throw std::runtime_error("Unsupported value type " + type_str);
            }
          }
          
          const auto [ptr, count, size] = tuple;
          interface* next = ptr;
          if (cur != nullptr) cur->next = next;
          if (begin == nullptr) begin = next;
          //cur = next;
          while (next != nullptr) { cur = next; next = next->next; }
          accum_count += count;
          accum_size += size;
          
          continue;
        }
        
        if (pair.first.get_type() == sol::type::string) {
          std::tuple<interface*, size_t, size_t> tuple(nullptr, 0, 0);
          const auto str = pair.first.as<std::string_view>();

          // возможно только касается condition?
          // указание на то что пропущено скорее будет сбивать с толку
          if (const auto ignore_key = ignore_keys::map.find(str); ignore_key != ignore_keys::map.end()) continue;
          
          const auto itr = commands::map.find(str);
          if (itr == commands::map.end()) {
            if (pair.second.get_type() != sol::type::table) throw std::runtime_error("Expected rvalue to be table for complex scope lvalue");
            if (!valid_complex_variable(str)) throw std::runtime_error(std::string(str) + " is not valid lvalue");
            tuple = complex_scope<script::change_scope_effect>(input, pair.second, cont, pair.first, &effect_table_init, nullptr);
          } else {
            const auto func = commands::inits[itr->second];
            tuple = func(input, pair.second, cont);
          }
          
          const auto [ptr, count, size] = tuple;
          interface* next = ptr;
          if (cur != nullptr) cur->next = next;
          if (begin == nullptr) begin = next;
          //cur = next;
          while (next != nullptr) { cur = next; next = next->next; }
          accum_count += count;
          accum_size += size;
          
          continue;
        }
        
        const sol::function type = s["type"];
        const auto obj = type(pair.second);
        const std::string type_str = obj;
        std::cout << "Ignored key type " << type_str << "\n";
      }
      
      return std::make_tuple(begin, accum_count, accum_size);
    }
    
    static std::tuple<interface*, size_t, size_t> change_scope_effect_init(const input_data &input, const sol::object &data, container* cont, interface* scope) {
      if (data.get_type() == sol::type::string && data.as<std::string_view>() == "change_scope_effect") { 
        throw std::runtime_error("Function 'change_scope_effect' could not be used in complex function");
      }
      
      assert(input.script_type == script_types::effect);
      if (input.expected_types != 0 && (input.expected_types & script::object::type_bit::invalid) != script::object::type_bit::invalid) throw std::runtime_error("Bad expected types input for 'effect' function");
      if (data.get_type() == sol::type::number || data.get_type() == sol::type::boolean) return number_container_init(input, data, cont);
      if (data.get_type() == sol::type::string) return complex_object_init(input, data, cont);
      if (data.get_type() != sol::type::table) throw std::runtime_error("Bad lua object type for 'effect' function");
      const auto table = data.as<sol::table>();
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<script::change_scope_effect>();
      
      size_t counter = 1;
      size_t final_size = sizeof(script::change_scope_effect);
      interface* condition = nullptr;
      if (const auto proxy = table["condition"]; proxy.valid()) {
        input_data new_input = input;
        new_input.script_type = script_types::condition;
        new_input.expected_types = script::object::type_bit::valid_boolean;
        const auto [ptr, count, size] = AND_init(new_input, proxy, cont); /* по умолчанию возьмем AND */
        condition = ptr;
        counter += count;
        final_size += size;
      }
      
      interface* childs = nullptr;
      {
        const auto [ptr, count, size] = effect_table_init(input, data, cont);
        childs = ptr;
        counter += count;
        final_size += size;
      }
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        auto init = cont->get_init<script::change_scope_effect>(offset);
        final_ptr = init.init(scope, condition, childs);
      }
      
      return std::make_tuple(final_ptr, counter, final_size);
    }
    
    std::tuple<interface*, size_t, size_t> change_scope_effect_init(const input_data &input, const sol::object &data, container* cont) {
      return change_scope_effect_init(input, data, cont, nullptr);
    }
    
    std::tuple<interface*, size_t, size_t> boolean_container_init(const input_data &input, const sol::object &data, container* cont) {
      if (input.script_type != script_types::numeric && input.script_type != script_types::condition)
        throw std::runtime_error("Boolean value makes sense only in numeric or condition scripts");
      
      if ((input.expected_types & boolean_container::output_type) == 0) throw std::runtime_error("Bad input expected types from number container");
      
      sol::state_view s = data.lua_state();
      switch (data.get_type()) {
        case sol::type::boolean: break;
        case sol::type::number: return number_container_init(input, data, cont);
        case sol::type::string: { 
          input_data new_input = input;
          new_input.script_type = script_types::condition;
          return complex_object_init(new_input, data, cont);
        }
        // если в таблице лежит единственный объект по имени нумерик блока, то почему бы тут не указать его вместо адд? да но его еще найти надо
        case sol::type::table: return AND_init(input, data, cont);
        default: {
          const sol::function type = s["type"];
          const auto obj = type(data);
          const std::string type_str = obj;
          throw std::runtime_error("Unsupported value type " + type_str);
        }
      }
          
      interface* ptr = nullptr;
      if (cont != nullptr) {
        const bool val = data.as<bool>();
        ptr = cont->add<boolean_container>(val);
      }
      return std::make_tuple(ptr, 1, align_to(sizeof(boolean_container), DEFAULT_ALIGNMENT));
    }
    
    static std::tuple<interface*, size_t, size_t> number_container_init(const input_data &input, const sol::object &data, container* cont, uint32_t &compare_op) {
      // должна ли тут быть только проверка на число? или может быть еще таблица?
      // наверное проверка общая пусть будет, то есть потенциально тут мы можем вернуть и number_container и complex_object и вычисление особого значения
      static_assert(alignof(number_container) <= DEFAULT_ALIGNMENT);
      
      if (input.script_type != script_types::numeric && input.script_type != script_types::condition)
        throw std::runtime_error("Number value makes sense only in numeric or condition scripts");
      
      if ((input.expected_types & number_container::output_type) == 0) throw std::runtime_error("Bad input expected types from number container");
      
      sol::state_view s = data.lua_state();
      switch (data.get_type()) {
        case sol::type::number: break;
        case sol::type::boolean: return boolean_container_init(input, data, cont);
        case sol::type::string: {
          input_data new_input = input;
          new_input.script_type = script_types::numeric;
          new_input.expected_types = number_container::output_type;
          size_t unused = SIZE_MAX;
          return complex_object_init(new_input, data, cont, compare_op, unused);
        }
        // если в таблице лежит единственный объект по имени нумерик блока, то почему бы тут не указать его вместо адд? да но его еще найти надо
        case sol::type::table: {
          const auto table = data.as<sol::table>();
          // compare_operator ? будет тип строка compare = "less" , может быть использовать слово operator? operator - зарезервированное слово с++
          //if (const auto proxy = table["compare"]; proxy.valid()) 
          //if (const auto proxy = table["operator"]; proxy.valid())
          if (const auto proxy = table["op"]; proxy.valid()) {
            if (proxy.get_type() != sol::type::string) throw std::runtime_error("'op' field expects to be string");
            const auto str = proxy.get<std::string_view>();
            const auto itr = compare_operators::map.find(str);
            if (itr == compare_operators::map.end()) throw std::runtime_error("Invalid compare operator name " + std::string(str));
            compare_op = itr->second;
          }
          return add_init(input, data, cont);
        }
        default: {
          const sol::function type = s["type"];
          const auto obj = type(data);
          const std::string type_str = obj;
          throw std::runtime_error("Unsupported value type " + type_str);
        }
      }
          
      interface* ptr = nullptr;
      if (cont != nullptr) {
        const double val = data.as<double>();
        ptr = cont->add<number_container>(val);
      }
      return std::make_tuple(ptr, 1, align_to(sizeof(number_container), DEFAULT_ALIGNMENT));
    }
    
    std::tuple<interface*, size_t, size_t> number_container_init(const input_data &input, const sol::object &data, container* cont) {
      uint32_t unused = UINT32_MAX;
      return number_container_init(input, data, cont, unused);
    }
    
    std::tuple<interface*, size_t, size_t> string_container_init(const input_data &input, const sol::object &data, container* cont) {
      if (input.script_type != script_types::string) throw std::runtime_error("String value makes sense only in string scripts");
      if ((input.expected_types & script::string_container::output_type) == 0) throw std::runtime_error("Bad input expected types from number container");
      
      sol::state_view s = data.lua_state();
      switch (data.get_type()) {
        case sol::type::string: break;
        // если в таблице лежит единственный объект по имени нумерик блока, то почему бы тут не указать его вместо адд? да но его еще найти надо
        case sol::type::table: return compute_string_init(input, data, cont);
        default: {
          const sol::function type = s["type"];
          const auto obj = type(data);
          const std::string type_str = obj;
          throw std::runtime_error("Unsupported value type " + type_str);
        }
      }
      
      // во многих случаях должен быть запрет сложную строку вида "sfafa:asfasf.sfafsfa"
      // единственный случай который допускает использование точек - это локализация
      // эти строки появляются только в специальных скриптах
      const auto val = data.as<std::string_view>();
      const bool has_colon = val.find(':') != std::string_view::npos;
      const bool has_dot   = val.find('.') != std::string_view::npos;
      if (has_colon) throw std::runtime_error(std::string(val) + " is invalid string");
      if (!input.get_flag(input_data::flags::dot_is_valid_string_symbol)) if (has_dot) throw std::runtime_error(std::string(val) + " is invalid string in this context");
      
      interface* ptr = nullptr;
      if (cont != nullptr) ptr = cont->add<script::string_container>(val);
      return std::make_tuple(ptr, 1, align_to(sizeof(script::string_container), DEFAULT_ALIGNMENT));
    }
    
    std::tuple<interface*, size_t, size_t> compute_string_init(const input_data &input, const sol::object &data, container* cont) {
      if (input.script_type != script_types::string) throw std::runtime_error("String value makes sense only in string scripts");
      if ((input.expected_types & script::compute_string::output_type) == 0) throw std::runtime_error("Bad input expected types from compute string func");
      
      if (data.get_type() == sol::type::string) return string_container_init(input, data, cont);
      
      sol::state_view s = data.lua_state();
      if (data.get_type() != sol::type::table) {
        const sol::function type = s["type"];
        const auto obj = type(data);
        const std::string type_str = obj;
        throw std::runtime_error("Unsupported value type " + type_str);
      }
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) cont->add_delayed<script::compute_string>();
      
      const auto table = data.as<sol::table>();
      interface* condition_ptr = nullptr;
      size_t final_count = 1;
      size_t final_size = sizeof(script::compute_string);
      if (const auto proxy = table["condition"]; proxy.valid()) {
        input_data new_input = input;
        new_input.script_type = script_types::condition;
        new_input.expected_types = script::object::type_bit::valid_boolean;
        const auto [ptr, count, size] = AND_init(new_input, proxy, cont);
        condition_ptr = ptr;
        final_count += count;
        final_size += size;
      }
      
      const auto [ptr, count, size] = string_table_init(input, data, cont, nullptr);
      final_count += count;
      final_size += size;
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        auto init = cont->get_init<script::compute_string>(offset);
        final_ptr = init.init(condition_ptr, ptr);
      }
      
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
    std::tuple<interface*, size_t, size_t> chance_init(const input_data &input, const sol::object &data, container* cont) {
      if (input.script_type != script_types::numeric && input.script_type != script_types::condition) throw std::runtime_error("Random chance makes sense only in condition or numeric scripts");
      if ((input.expected_types & script::chance::output_type) == 0) throw std::runtime_error("Bad input expected types from number container");
      if (data.get_type() == sol::type::string && data.as<std::string_view>() == commands::names[script::chance::type_index]) {
        throw std::runtime_error("Function 'chance' could not be used in complex function");
      }
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<script::chance>();
      size_t final_count = 1;
      size_t final_size = sizeof(script::chance);
      
      input_data new_input = input;
      new_input.script_type = script_types::numeric;
      new_input.expected_types = script::object::type_bit::valid_number;
      const auto [ptr, count, size] = number_container_init(new_input, data, cont);
      final_count += count;
      final_size += size;
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        auto init = cont->get_init<script::chance>(offset);
        // причем тут может быть абсолютный рандом, но мне все равно нужно синхронизовать этот стейт в мультиплеере
        // возможно имеет смысл глобал стейт разделить на несколько стейтов?
        final_ptr = init.init(global::advance_state(), ptr);
      }
      
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
    std::tuple<interface*, size_t, size_t> random_value_init(const input_data &input, const sol::object &data, container* cont) {
      if (input.script_type != script_types::numeric && input.script_type != script_types::condition) throw std::runtime_error("Random chance makes sense only in condition or numeric scripts");
      if ((input.expected_types & script::random_value::output_type) == 0) throw std::runtime_error("Bad input expected types from number container");
      if (data.get_type() == sol::type::string && data.as<std::string_view>() == commands::names[script::random_value::type_index]) {
        throw std::runtime_error("Function 'random_value' could not be used in complex function");
      }
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<script::random_value>();
      size_t final_count = 1;
      size_t final_size = sizeof(script::random_value);
      
      input_data new_input = input;
      new_input.script_type = script_types::numeric;
      new_input.expected_types = script::object::type_bit::valid_number;
      const auto [ptr, count, size] = number_container_init(new_input, data, cont);
      final_count += count;
      final_size += size;
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        auto init = cont->get_init<script::random_value>(offset);
        final_ptr = init.init(global::advance_state(), ptr);
      }
      
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
    std::tuple<interface*, size_t, size_t> weighted_random_init(const input_data &input, const sol::object &data, container* cont) {
      if (data.get_type() == sol::type::string && data.as<std::string_view>() == commands::names[script::weighted_random::type_index]) {
        throw std::runtime_error("Function 'weighted_random' could not be used in complex function");
      }
      
      if (data.get_type() != sol::type::table) throw std::runtime_error("'weighted_random' expects lua table as input");
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<script::weighted_random>();
      size_t final_count = 1;
      size_t final_size = sizeof(script::weighted_random);
      
      interface* begin_childs = nullptr;
      interface* cur_child = nullptr;
      interface* begin_weights = nullptr;
      interface* cur_weight = nullptr;
      const auto table = data.as<sol::table>();
      for (const auto &pair : table) {
        if (pair.second.get_type() != sol::type::table) continue;
        if (pair.first.get_type() == sol::type::string) {
          const auto str = pair.first.as<std::string_view>();
          // указание на то что пропущено скорее будет сбивать с толку
          if (const auto itr = ignore_keys::map.find(str); itr != ignore_keys::map.end()) continue;
        }
        
        const auto cur_table = pair.second.as<sol::table>();
        const auto proxy = cur_table["weight"];
        if (!proxy.valid()) throw std::runtime_error("'weighted_random' expects array of lua tables with weight number script field");
        {
          input_data new_input = input;
          new_input.script_type = script_types::numeric;
          new_input.expected_types = script::object::type_bit::valid_number;
          const auto [ptr, count, size] = number_container_init(new_input, proxy, cont);
          if (begin_weights == nullptr) begin_weights = ptr;
          if (cur_weight != nullptr) cur_weight->next = ptr;
          cur_weight = ptr;
          final_count += count;
          final_size += size;
        }
        
        if (const auto proxy = cur_table["condition"]; proxy.valid()) throw std::runtime_error("'condition' field located in same block with 'weight' in function 'weighted_random' is considered as error");
        
        // тут наверное справедливы *_table_init, а в селекторе или в сиквенсе нет
        // нет, тут тоже требуется контейнеры по умолчанию
        {
          std::tuple<interface*, size_t, size_t> tuple;
          switch (static_cast<script_types::values>(input.script_type)) {
            case script_types::condition: tuple = change_scope_condition_init(input, data, cont); break;
            case script_types::numeric: tuple = add_init(input, data, cont); break;
            case script_types::effect: tuple = change_scope_effect_init(input, data, cont); break;
            case script_types::string: tuple = compute_string_init(input, data, cont); break;
            //case script_types::object: tuple = object_table_init(input, data, cont); break;
            default: throw std::runtime_error("Not implemented");
          }
          const auto [ptr, count, size] = tuple;
          if (begin_childs == nullptr) begin_childs = ptr;
          if (cur_child != nullptr) cur_child->next = ptr;
          cur_child = ptr;
          final_count += count;
          final_size += size;
        }
      }
      
      if (final_count == 1) throw std::runtime_error("Could not find children for 'weighted_random'");
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        assert(begin_childs != nullptr);
        assert(begin_weights != nullptr);
        auto init = cont->get_init<script::weighted_random>(offset);
        final_ptr = init.init(global::advance_state(), begin_childs, begin_weights);
      }
      
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
    template <typename T, typename Type_container>
    static std::tuple<interface*, size_t, size_t> block_func_init(const input_data &input, const sol::object &data, container* cont, table_init_func_t func) {
      if (data.get_type() == sol::type::string && data.as<std::string_view>() == commands::names[T::type_index]) {
        throw std::runtime_error("Function '"+ std::string(commands::names[T::type_index]) + "' could not be used in complex function");
      }
      assert(input.script_type == script_types::numeric || input.script_type == script_types::condition);
      if ((input.expected_types & T::output_type) != T::output_type)
        throw std::runtime_error("Bad expected types input for '"+ std::string(commands::names[T::type_index]) + "' function: expected " + parse_type_bits(input.expected_types) + ", got " + parse_type_bits(T::output_type));
      /* вопрос нужно ли тут дать возможность задать просто значение, например у блока condition, с учетом того что одно значение нормально пройдет в таблице то сомнительно */ 
      /*if (data.get_type() == sol::type::number || data.get_type() == sol::type::boolean) return number_container_init(input, data, cont);*/
      /*if (data.get_type() == sol::type::string) return complex_object_init(input, data, cont);*/
      if (data.get_type() != sol::type::table) throw std::runtime_error("Bad lua object type for '"+ std::string(commands::names[T::type_index]) + "' function");
      assert(data.valid());
      const auto table = data.as<sol::table>();
      const auto init_func = commands::inits[T::type_index];
      if (const auto proxy = table["condition"]; proxy != sol::nil) return create_compute<T, Type_container>(input, data, cont, nullptr, func, init_func);
      return create_block<T>(input, data, cont, func, init_func);
    }
    
#define BLOCK_INIT_FUNC(func_name, script_type_container, table_init_func) \
    std::tuple<interface*, size_t, size_t> func_name##_init(const input_data &input, const sol::object &data, container* cont) { \
      return block_func_init<script::func_name, script::script_type_container>(input, data, cont, &table_init_func); \
    }
    
#define NUMERIC_INIT_FUNC(func_name) \
  std::tuple<interface*, size_t, size_t> func_name##_init(const input_data &input, const sol::object &data, container* cont) { \
      if (data.get_type() == sol::type::string && data.as<std::string_view>() == commands::names[commands::func_name]) { \
        throw std::runtime_error("Function '"#func_name"' could not be used in complex function"); \
      } \
      \
      if ((input.expected_types & script::func_name::output_type) != script::func_name::output_type) throw std::runtime_error("Bad expected types input for '"#func_name"' function"); \
      assert(input.script_type == script_types::numeric); \
      size_t offset = SIZE_MAX; \
      if (cont != nullptr) cont->add_delayed<script::func_name>(); \
      size_t counter = 1; \
      size_t final_size = sizeof(script::func_name); \
      \
      const auto [ptr, count, size] = number_container_init(input, data, cont); \
      counter += count; \
      final_size += size; \
      interface* final_ptr = nullptr; \
      if (cont != nullptr) { \
        auto init = cont->get_init<script::func_name>(offset); \
        final_ptr = init.init(ptr); \
      } \
      return std::make_tuple(final_ptr, counter, final_size); \
    }

// наверное могут использоваться в кондишонах, а кондишоны в нумериках
#define NUMERIC_BLOCK_INIT_FUNC(func_name) BLOCK_INIT_FUNC(func_name, compute_number, numeric_table_init)
    
NUMERIC_BLOCK_INIT_FUNC(add)
NUMERIC_BLOCK_INIT_FUNC(sub)
NUMERIC_BLOCK_INIT_FUNC(multiply)
NUMERIC_BLOCK_INIT_FUNC(divide)
NUMERIC_BLOCK_INIT_FUNC(mod)
NUMERIC_BLOCK_INIT_FUNC(min)
NUMERIC_BLOCK_INIT_FUNC(max)
NUMERIC_BLOCK_INIT_FUNC(add_sequence)
NUMERIC_BLOCK_INIT_FUNC(multiply_sequence)
NUMERIC_BLOCK_INIT_FUNC(min_sequence)
NUMERIC_BLOCK_INIT_FUNC(max_sequence)
NUMERIC_INIT_FUNC(sin)
NUMERIC_INIT_FUNC(cos)
NUMERIC_INIT_FUNC(abs)
NUMERIC_INIT_FUNC(ceil)
NUMERIC_INIT_FUNC(floor)
NUMERIC_INIT_FUNC(sqrt)
NUMERIC_INIT_FUNC(sqr)
NUMERIC_INIT_FUNC(frac)
NUMERIC_INIT_FUNC(inv)

#define CONDITION_BLOCK_INIT_FUNC(func_name) BLOCK_INIT_FUNC(func_name, change_scope_condition, condition_table_init)

// здесь бы еще сиквенсы для кондишенов добавить, хотя бы для основных
CONDITION_BLOCK_INIT_FUNC(AND)
CONDITION_BLOCK_INIT_FUNC(OR)
CONDITION_BLOCK_INIT_FUNC(NAND)
CONDITION_BLOCK_INIT_FUNC(NOR)
CONDITION_BLOCK_INIT_FUNC(XOR)
CONDITION_BLOCK_INIT_FUNC(IMPL)
CONDITION_BLOCK_INIT_FUNC(EQ)
CONDITION_BLOCK_INIT_FUNC(AND_sequence)
CONDITION_BLOCK_INIT_FUNC(OR_sequence)

    template <typename T>
    static std::tuple<interface*, size_t, size_t> selector_sequence_init(const input_data &input, const sol::object &data, container* cont) {
      if (data.get_type() == sol::type::string && data.as<std::string_view>() == commands::names[T::type_index])
        throw std::runtime_error("Function '" + std::string(commands::names[T::type_index]) + "' can not be used in complex function");
      
      if (data.get_type() != sol::type::table) throw std::runtime_error("Bad lua object type for '" + std::string(commands::names[T::type_index]) + "' function");
      const auto table = data.as<sol::table>();
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<T>();
      size_t final_count = 1;
      size_t final_size = sizeof(T);
      
      interface* begin = nullptr;
      interface* cur = nullptr;
      for (const auto &pair : table) {
        if (pair.second.get_type() != sol::type::table) continue;
//         if (pair.first.get_type() == sol::type::string) {
//           const auto str = pair.first.as<std::string_view>();
//           // указание на то что пропущено скорее будет сбивать с толку
//           // может вообще запретить тут строковые ключи?
//           if (const auto itr = ignore_keys::map.find(str); itr != ignore_keys::map.end()) continue;
//         }
        
        if (pair.first.get_type() != sol::type::number && pair.first.get_type() != sol::type::boolean) throw std::runtime_error("String keys in 'selector' function is not allowed");
        
        const auto t = pair.second.as<sol::table>();
        
        // здесь нужно создать контейнеры для списка функций в таблице
        
        std::tuple<interface*, size_t, size_t> tuple;
        switch (static_cast<script_types::values>(input.script_type)) {
          case script_types::condition: tuple = change_scope_condition_init(input, data, cont); break;
          case script_types::numeric:   tuple = add_init(input, data, cont);                    break;
          case script_types::string:    tuple = compute_string_init(input, data, cont);         break;
          case script_types::effect:    tuple = change_scope_effect_init(input, data, cont);    break;
          default: throw std::runtime_error("Not implemented");
        }
        
        const auto [ptr, count, size] = tuple;
        if (begin == nullptr) begin = ptr;
        if (cur != nullptr) cur->next = ptr;
        cur = ptr;
        final_count += count;
        final_size += size;
      }
      
      if (final_count == 1) throw std::runtime_error("Could not find children for '" + std::string(commands::names[T::type_index]) + "'");
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        auto init = cont->get_init<T>(offset);
        final_ptr = init.init(begin);
      }
      
      return std::make_tuple(final_ptr, final_count, final_size);
    }

    std::tuple<interface*, size_t, size_t> selector_init(const input_data &input, const sol::object &data, container* cont) {
      return selector_sequence_init<script::selector>(input, data, cont);
    }
    
//     std::tuple<interface*, size_t, size_t> sequence_init(const input_data &input, const sol::object &data, container* cont) {
//       return selector_sequence_init<script::sequence>(input, data, cont);
//     }
    
    std::tuple<interface*, size_t, size_t> sequence_init(const input_data &input, const sol::object &data, container* cont) {
      if (data.get_type() == sol::type::string && data.as<std::string_view>() == commands::names[script::sequence::type_index])
        throw std::runtime_error("Function '" + std::string(commands::names[script::sequence::type_index]) + "' can not be used in complex function");
      
      if (data.get_type() != sol::type::table) throw std::runtime_error("Bad lua object type for '" + std::string(commands::names[script::sequence::type_index]) + "' function");
      const auto table = data.as<sol::table>();
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<script::sequence>();
      size_t final_count = 1;
      size_t final_size = sizeof(script::sequence);
      
      interface* count_ptr = nullptr;
      interface* begin = nullptr;
      interface* cur = nullptr;
      for (const auto &pair : table) {
        if (pair.first.get_type() == sol::type::string) {
          const auto str = pair.first.as<std::string_view>();
          if (str == "count") {
            input_data new_input = input;
            new_input.script_type = script_types::numeric;
            new_input.expected_types = script::object::type_bit::valid_number;
            const auto [ptr, count, size] = number_container_init(new_input, pair.second, cont);
            count_ptr = ptr;
            final_count += count;
            final_size += size;
            continue;
          }
          
          // указание на то что пропущено скорее будет сбивать с толку
          //if (const auto itr = ignore_keys::map.find(str); itr != ignore_keys::map.end()) continue;
          throw std::runtime_error("String keys, except 'count', in 'selector' function is not allowed");
        }
        
        if (pair.second.get_type() != sol::type::table) continue;
        const auto t = pair.second.as<sol::table>();
        
        std::tuple<interface*, size_t, size_t> tuple;
        switch (static_cast<script_types::values>(input.script_type)) {
          case script_types::condition: tuple = change_scope_condition_init(input, data, cont); break;
          case script_types::numeric:   tuple = add_init(input, data, cont);                    break;
          case script_types::string:    tuple = compute_string_init(input, data, cont);         break;
          case script_types::effect:    tuple = change_scope_effect_init(input, data, cont);    break;
          default: throw std::runtime_error("Not implemented");
        }
        
        const auto [ptr, count, size] = tuple;
        if (begin == nullptr) begin = ptr;
        if (cur != nullptr) cur->next = ptr;
        cur = ptr;
        final_count += count;
        final_size += size;
      }
      
      if (final_count == 1) throw std::runtime_error("Could not find children for '" + std::string(commands::names[script::sequence::type_index]) + "'");
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        auto init = cont->get_init<script::sequence>(offset);
        final_ptr = init.init(count_ptr, begin);
      }
      
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
//     std::tuple<interface*, size_t, size_t> at_least_sequence_init(const input_data &input, const sol::object &data, container* cont) {
//       if (data.get_type() == sol::type::string && data.as<std::string_view>() == commands::names[script::at_least_sequence::type_index])
//         throw std::runtime_error("Function '" + std::string(commands::names[script::at_least_sequence::type_index]) + "' can not be used in complex function");
//       
//       if (data.get_type() != sol::type::table) throw std::runtime_error("Bad lua object type for '" + std::string(commands::names[script::at_least_sequence::type_index]) + "' function");
//       const auto table = data.as<sol::table>();
//       
//       size_t offset = SIZE_MAX;
//       if (cont != nullptr) offset = cont->add_delayed<script::at_least_sequence>();
//       size_t final_count = 1;
//       size_t final_size = sizeof(script::at_least_sequence);
//       
//       interface* count_ptr = nullptr;
//       const auto proxy = table["count"]; 
//       if (!proxy.valid()) throw std::runtime_error("'at_least_sequence' expects 'count' to be valid number script");
//       input_data new_input = input;
//       new_input.script_type = script_types::numeric;
//       new_input.expected_types = script::object::type_bit::valid_number;
//       const auto [ptr, count, size] = number_container_init(new_input, proxy, cont);
//       count_ptr = ptr;
//       final_count += count;
//       final_size += size;
//       
//       interface* begin = nullptr;
//       interface* cur = nullptr;
//       for (const auto &pair : table) {
//         if (pair.second.get_type() != sol::type::table) continue;
//         const auto t = pair.second.as<sol::table>();
//         
//         std::tuple<interface*, size_t, size_t> tuple;
//         switch (static_cast<script_types::values>(input.script_type)) {
//           case script_types::condition: tuple = change_scope_condition_init(input, data, cont); break;
//           case script_types::numeric:   tuple = add_init(input, data, cont);                    break;
//           case script_types::string:    tuple = compute_string_init(input, data, cont);         break;
//           case script_types::effect:    tuple = change_scope_effect_init(input, data, cont);    break;
//           default: throw std::runtime_error("Not implemented");
//         }
//         
//         const auto [ptr, count, size] = tuple;
//         if (begin == nullptr) begin = ptr;
//         if (cur != nullptr) cur->next = ptr;
//         cur = ptr;
//         final_count += count;
//         final_size += size;
//       }
//       
//       if (final_count == 1) throw std::runtime_error("Could not find children for '" + std::string(commands::names[script::at_least_sequence::type_index]) + "'");
//       
//       interface* final_ptr = nullptr;
//       if (cont != nullptr) {
//         auto init = cont->get_init<script::at_least_sequence>(offset);
//         final_ptr = init.init(count_ptr, begin);
//       }
//       
//       return std::make_tuple(final_ptr, final_count, final_size);
//     }

    template <typename T>
    static std::tuple<interface*, size_t, size_t> create_has_func(const input_data &input, const sol::object &data, container* cont) {
      if (input.script_type != script_types::numeric && input.script_type != script_types::condition)
        throw std::runtime_error("Trying to create function '" + std::string(commands::names[T::type_index]) + "' in wrong script type");
      if (data.get_type() == sol::type::string && data.as<std::string_view>() == commands::names[T::type_index])
        throw std::runtime_error("Function '" + std::string(commands::names[T::type_index]) + "' can not be used in complex function");
      if ((input.current & T::context_types) == 0) throw std::runtime_error("Bad context for '" + std::string(commands::names[T::type_index]) + "' function");
      if ((input.expected_types & T::output_type) == 0) {
        throw std::runtime_error("Bad expectation from '" + 
          std::string(commands::names[T::type_index]) + 
          "' function: expected " + parse_type_bits(input.expected_types) + 
          ", got " + parse_type_bits(T::output_type));
      }
      
      if (data.get_type() == sol::type::string) throw std::runtime_error("Function '" + std::string(commands::names[T::type_index]) + "' can not handle string input");
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<T>();
      size_t final_count = 1;
      size_t final_size = sizeof(T);
      
      input_data new_input = input;
      new_input.prev = input.current;
      new_input.current = T::expected_types;
      
      interface* val_ptr = nullptr;
      interface* max_count_ptr = nullptr;
      interface* percentage_ptr = nullptr;
      if (data.get_type() == sol::type::boolean || data.get_type() == sol::type::number) {
        const auto [ptr, count, size] = boolean_container_init(input, data, cont);
        val_ptr = ptr;
        final_count += count;
        final_size += size;
      } else if (data.get_type() == sol::type::table) {
        const auto table = data.as<sol::table>();
        if (const auto proxy = table["percent"]; proxy.valid()) {
          new_input.script_type = script_types::numeric;
          new_input.expected_types = script::object::type_bit::valid_number;
          const auto [ptr, count, size] = number_container_init(new_input, proxy, cont);
          percentage_ptr = ptr;
          final_count += count;
          final_size += size;
        } else if (const auto proxy = table["count"]; proxy.valid()) {
          new_input.script_type = script_types::numeric;
          new_input.expected_types = script::object::type_bit::valid_number;
          const auto [ptr, count, size] = number_container_init(new_input, proxy, cont);
          max_count_ptr = ptr;
          final_count += count;
          final_size += size;
        }
        const auto [ptr, count, size] = condition_table_init(new_input, data, cont, &AND_init);
        val_ptr = ptr;
        final_count += count;
        final_size += size;
      } else throw std::runtime_error("Function '" + std::string(commands::names[T::type_index]) + "' can not handle this input type input");
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        auto init = cont->get_init<T>(offset);
        final_ptr = init.init(val_ptr, max_count_ptr, percentage_ptr);
      }
      
      return std::make_tuple(final_ptr, final_count, final_size);
    }

    template <typename T>
    static std::tuple<interface*, size_t, size_t> create_one_to_random_func(const input_data &input, const sol::object &data, container* cont) {
      if (input.script_type != script_types::effect) throw std::runtime_error("Trying to create function '" + std::string(commands::names[T::type_index]) + "' in wrong script type");
      if (data.get_type() == sol::type::string && data.as<std::string_view>() == commands::names[T::type_index])
        throw std::runtime_error("Function '" + std::string(commands::names[T::type_index]) + "' can not be used in complex function");
      if ((input.current & T::context_types) == 0) throw std::runtime_error("Bad context for '" + std::string(commands::names[T::type_index]) + "' function");
      if (input.expected_types != 0) throw std::runtime_error("Bad expectation from '" + std::string(commands::names[T::type_index]) + "' function");
      
      if (data.get_type() != sol::type::table) throw std::runtime_error("Function '" + std::string(commands::names[T::type_index]) + "' can not handle this input type input");
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<T>();
      
      input_data new_input = input; 
      new_input.prev = input.current;
      new_input.current = T::expected_types;
//       if constexpr (std::is_same_v<T, script::random_vassal>) new_input.current = script::object::type_bit::realm;
//       else new_input.current = script::object::type_bit::character;
      
      interface* val_ptr = nullptr;
      interface* condition_ptr = nullptr;
      interface* weight_ptr = nullptr;
      size_t final_count = 1;
      size_t final_size = sizeof(T);
      
      const auto table = data.as<sol::table>();
      if (const auto proxy = table["condition"]; proxy.valid()) {
        new_input.script_type = script_types::condition;
        new_input.expected_types = script::object::type_bit::valid_boolean;
        const auto [ptr, count, size] = AND_init(new_input, proxy, cont);
        condition_ptr = ptr;
        final_count += count;
        final_size += size;
      }
      
      if (const auto proxy = table["weight"]; proxy.valid()) {
        new_input.script_type = script_types::numeric;
        new_input.expected_types = script::object::type_bit::valid_number;
        const auto [ptr, count, size] = number_container_init(new_input, proxy, cont);
        weight_ptr = ptr;
        final_count += count;
        final_size += size;
      }
      
      const auto [ptr, count, size] = effect_table_init(new_input, data, cont);
      val_ptr = ptr;
      final_count += count;
      final_size += size;
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        auto init = cont->get_init<T>(offset);
        final_ptr = init.init(global::advance_state(), condition_ptr, weight_ptr, val_ptr);
      }
      
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
    template <typename T>
    static std::tuple<interface*, size_t, size_t> create_one_to_every_func(const input_data &input, const sol::object &data, container* cont) {
      if (data.get_type() == sol::type::string && data.as<std::string_view>() == commands::names[T::type_index])
        throw std::runtime_error("Function '" + std::string(commands::names[T::type_index]) + "' can not be used in complex function");
      if ((input.current & T::context_types) == 0) throw std::runtime_error("Bad context for '" + std::string(commands::names[T::type_index]) + "' function");
      
      size_t type = SIZE_MAX;
      switch (input.script_type) {
        case script_types::effect: {
          if (input.expected_types != 0) throw std::runtime_error("Bad expectation from '" + std::string(commands::names[T::type_index]) + "' function");
          type = EVERY_FUNC_EFFECT; 
          break;
        }
        case script_types::condition: {
          if ((input.expected_types & script::object::type_bit::valid_boolean) == 0) throw std::runtime_error("Bad expectation from '" + std::string(commands::names[T::type_index]) + "' function");
          type = EVERY_FUNC_LOGIC; 
          break;
        }
        case script_types::numeric: {
          if ((input.expected_types & script::object::type_bit::valid_number) == 0) throw std::runtime_error("Bad expectation from '" + std::string(commands::names[T::type_index]) + "' function");
          type = EVERY_FUNC_NUMERIC; 
          break;
        }
        default: throw std::runtime_error("Trying to create function '" + std::string(commands::names[T::type_index]) + "' in wrong script type");
      }
      
      ASSERT(type != SIZE_MAX);
      
      if (data.get_type() == sol::type::string) throw std::runtime_error("Function '" + std::string(commands::names[T::type_index]) + "' can not handle string input");
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<T>();
      
      input_data new_input = input; 
      new_input.prev = input.current;
      new_input.current = T::expected_types;
      
      interface* val_ptr = nullptr;
      interface* condition_ptr = nullptr;
      size_t final_count = 1;
      size_t final_size = sizeof(T);
      
      const auto table = data.as<sol::table>();
      if (const auto proxy = table["condition"]; proxy.valid()) {
        new_input.script_type = script_types::condition;
        new_input.expected_types = script::object::type_bit::valid_boolean;
        const auto [ptr, count, size] = AND_init(new_input, proxy, cont);
        condition_ptr = ptr;
        final_count += count;
        final_size += size;
      }
      
      std::tuple<interface*, size_t, size_t> tuple;
      switch (type) {
        case EVERY_FUNC_EFFECT:  tuple = effect_table_init(new_input, data, cont);    break;
        case EVERY_FUNC_LOGIC:   tuple = condition_table_init(new_input, data, cont, &AND_init); break;
        case EVERY_FUNC_NUMERIC: tuple = numeric_table_init(new_input, data, cont, &add_init);   break;
      }
      
      const auto [ptr, count, size] = tuple;
      val_ptr = ptr;
      final_count += count;
      final_size += size;
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        auto init = cont->get_init<T>(offset);
        final_ptr = init.init(type, condition_ptr, val_ptr);
      }
      
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
#define CHANGE_CONTEXT_COMMAND_FUNC(func_name, context_type_bits, expected_type_bits, output_type_bit) \
    std::tuple<interface*, size_t, size_t> has_##func_name##_init(const input_data &input, const sol::object &data, container* cont) { \
      return create_has_func<script::has_##func_name>(input, data, cont);                              \
    }                                                                                                  \
                                                                                                       \
    std::tuple<interface*, size_t, size_t> every_##func_name##_init(const input_data &input, const sol::object &data, container* cont) { \
      return create_one_to_every_func<script::every_##func_name>(input, data, cont);                   \
    }                                                                                                  \
                                                                                                       \
    std::tuple<interface*, size_t, size_t> random_##func_name##_init(const input_data &input, const sol::object &data, container* cont) { \
      return create_one_to_random_func<script::random_##func_name>(input, data, cont);                 \
    }                                                                                                  \
    
    CHANGE_CONTEXT_COMMANDS_FINAL_LIST

#undef CHANGE_CONTEXT_COMMAND_FUNC

    std::tuple<interface*, size_t, size_t> change_scope_condition_init(const input_data &input, const sol::object &data, container* cont) {
      return change_scope_condition_init(input, data, cont, nullptr);
    }
    
    std::tuple<interface*, size_t, size_t> compute_number_init(const input_data &input, const sol::object &data, container* cont) {
      return compute_number_init(input, data, cont, nullptr);
    }
    
    static std::tuple<interface*, size_t, size_t> create_scope_changer(const input_data &input, const sol::object &data, container* cont, interface* scope) {
      switch (static_cast<script_types::values>(input.script_type)) {
        case script_types::string: throw std::runtime_error("Could not use this function for string script");
        // должен быть частью сложной переменной?
        case script_types::object: throw std::runtime_error("Trying to get object not from complex object. Is it an error?");
        case script_types::numeric: return compute_number_init(input, data, cont, scope); // create_compute<script::add, script::compute_number>
        case script_types::condition: return change_scope_condition_init(input, data, cont, scope); // create_compute<script::AND, script::change_scope_condition>
        case script_types::effect: return change_scope_effect_init(input, data, cont, scope);
      }
      
      return std::make_tuple(nullptr, 0, 0);
    }
    
    // в эту функцию мы можем придти фактически при любых 3 раскладах, то есть если нам нужно посчитать условия
    // или посчитать число, или посчитать эффект, как это друг от друга отделить? тип в инпуте прописать? может ли смениться тип? может
    // например когда мы пытаемся сравнить два стата, может быть что мы можем ожидать два типа? не уверен, но вряд ли
//     std::tuple<interface*, size_t, size_t> self_realm_init(const input_data &input, const sol::object &data, container* cont) {
//       if ((input.current & script::self_realm::context_types) == 0) throw std::runtime_error("Bad context for 'self_realm' function");
//       // вообще я могу использовать self_realm как rvalue
//       if (!data.valid()) throw std::runtime_error("Could not use 'self_realm' function as rvalue");
//       
//       // может ли к нам прийти строка по другим причинам? функция для создания числовых значений может ожидать строку как сложное число
//       // возможно имеет смысл здесь передать строку совпадающую с названием функции, чтобы точно определить что это инпут из сложного числа
//       if (data.get_type() == sol::type::string && data.as<std::string_view>() == commands::names[commands::self_realm]) {
//         interface* ptr = nullptr;
//         if (cont != nullptr) ptr = cont->add<script::self_realm>();
//         return std::make_tuple(ptr, script::object::type_bit::realm, sizeof(script::self_realm));
//       }
//       
//       if (input.script_type == script_types::string) throw std::runtime_error("Could not use 'self_realm' function for string script");
//       if (input.script_type == script_types::object) throw std::runtime_error("Trying to get object 'self_realm' not from complex object. Is it an error?");
//       
//       // не думаю что есть возможность проверить так expected_types
//       //if ((input.expected_types & script::self_realm::output_type) == 0) throw std::runtime_error("Bad input expected types");
//       
//       auto new_input = input;
//       new_input.prev = new_input.current;
//       new_input.current = script::object::type_bit::realm;
//       
//       size_t final_count = 0;
//       size_t final_size = 0;
//       
//       interface* scope = nullptr;
//       if (cont != nullptr) scope = cont->add<script::self_realm>();
//       final_count += 1;
//       final_size += sizeof(script::self_realm);
//       
//       const auto [ptr, count, size] = create_scope_changer(new_input, data, cont, scope); /* к сожалению это означает что self_realm будет стоять за контейнером */
//       final_count += count;
//       final_size += size;
//       
//       return std::make_tuple(ptr, final_count, final_size);
//     }
    
    template <typename T>
    std::tuple<interface*, size_t, size_t> get_scope_func_init(const input_data &input, const sol::object &data, container* cont) {
      if ((input.current & T::context_types) == 0) throw std::runtime_error("Bad context for '" + std::string(commands::names[T::type_index]) + "' function");
      // вообще я могу использовать self_realm как rvalue
      if (!data.valid()) throw std::runtime_error("Could not use '" + std::string(commands::names[T::type_index]) + "' function as rvalue");
      
      // может ли к нам прийти строка по другим причинам? функция для создания числовых значений может ожидать строку как сложное число
      // возможно имеет смысл здесь передать строку совпадающую с названием функции, чтобы точно определить что это инпут из сложного числа
      if (data.get_type() == sol::type::string && data.as<std::string_view>() == commands::names[T::type_index]) {
        interface* ptr = nullptr;
        if (cont != nullptr) ptr = cont->add<T>();
        return std::make_tuple(ptr, T::output_type, sizeof(T));
      }
      
      if (input.script_type == script_types::string) throw std::runtime_error("Could not use '" + std::string(commands::names[T::type_index]) + "' function for string script");
      if (input.script_type == script_types::object) throw std::runtime_error("Trying to get object '" + std::string(commands::names[T::type_index]) + "' not from complex object. Is it an error?");
      
      // не думаю что есть возможность проверить так expected_types
      //if ((input.expected_types & script::self_realm::output_type) == 0) throw std::runtime_error("Bad input expected types");
      
      auto new_input = input;
      new_input.prev = new_input.current;
      new_input.current = T::output_type;
      
      size_t final_count = 0;
      size_t final_size = 0;
      
      interface* scope = nullptr;
      if (cont != nullptr) scope = cont->add<T>();
      final_count += 1;
      final_size += sizeof(T);
      
      const auto [ptr, count, size] = create_scope_changer(new_input, data, cont, scope); /* к сожалению это означает что self_realm будет стоять за контейнером */
      final_count += count;
      final_size += size;
      
      return std::make_tuple(ptr, final_count, final_size);
    }
    
#define GET_SCOPE_COMMAND_FUNC(name, context_types_bits, output_type_bit, unused) \
    std::tuple<interface*, size_t, size_t> name##_init(const input_data &input, const sol::object &data, container* cont) { \
      return get_scope_func_init<script::name>(input, data, cont); \
    }
    
    GET_SCOPE_COMMANDS_FINAL_LIST
    
#undef GET_SCOPE_COMMAND_FUNC
    
    std::tuple<interface*, size_t, size_t> root_init(const input_data &input, const sol::object &data, container* cont) {
      using type = script::root;
      
      if ((input.expected_types & input.root) != input.root) 
        throw std::runtime_error("Wrong expectation from 'root' function: expected " + parse_type_bits(input.expected_types) + ", got " + parse_type_bits(input.root));
      
      // вообще я могу использовать prev как rvalue, невалидная дата может прийти только если мы указали "prev" в массиве луа, в этом случае возвращать прев?
      // прев может потребоваться как вход для некоторых функций (например для женитьбы), в этих функциях по идее должен требоваться объект
      if (!data.valid()) {
        if (input.script_type != script_types::object) throw std::runtime_error("Could not use 'root' function as rvalue");
        
        interface* ptr = nullptr;
        if (cont != nullptr) ptr = cont->add<type>();
        return std::make_tuple(ptr, 1, sizeof(type));
      }
      
      if (data.get_type() == sol::type::string && data.as<std::string_view>() == commands::names[type::type_index]) {
        interface* ptr = nullptr;
        if (cont != nullptr) ptr = cont->add<type>();
        return std::make_tuple(ptr, input.root, sizeof(type));
      }
      
      if (input.script_type == script_types::string) throw std::runtime_error("Could not use 'root' function for string script");
      if (input.script_type == script_types::object) throw std::runtime_error("Trying to get object 'root' not from complex object. Is it an error?");
      
      auto new_input = input;
      new_input.prev = new_input.current;
      new_input.current = input.root;
      
      size_t final_count = 0;
      size_t final_size = 0;
      
      interface* scope = nullptr;
      if (cont != nullptr) scope = cont->add<type>();
      final_count += 1;
      final_size += sizeof(type);
      
      const auto [ptr, count, size] = create_scope_changer(new_input, data, cont, scope); /* к сожалению это означает что self_realm будет стоять за контейнером */
      final_count += count;
      final_size += size;
      
      return std::make_tuple(ptr, final_count, final_size);
    }
    
    std::tuple<interface*, size_t, size_t> prev_init(const input_data &input, const sol::object &data, container* cont) {
      using type = script::prev;
      
      if ((input.expected_types & input.prev) != input.prev) 
        throw std::runtime_error("Wrong expectation from 'prev' function: expected " + parse_type_bits(input.expected_types) + ", got " + parse_type_bits(input.prev));
      
      // вообще я могу использовать prev как rvalue, невалидная дата может прийти только если мы указали "prev" в массиве луа, в этом случае возвращать прев?
      // прев может потребоваться как вход для некоторых функций (например для женитьбы), в этих функциях по идее должен требоваться объект
      if (!data.valid()) {
        if (input.script_type != script_types::object) throw std::runtime_error("Could not use 'prev' function as rvalue");
        
        interface* ptr = nullptr;
        if (cont != nullptr) ptr = cont->add<type>();
        return std::make_tuple(ptr, 1, sizeof(type));
      }
      
      if (data.get_type() == sol::type::string && data.as<std::string_view>() == commands::names[type::type_index]) {
        interface* ptr = nullptr;
        if (cont != nullptr) ptr = cont->add<type>();
        return std::make_tuple(ptr, input.prev, sizeof(type));
      }
      
      if (input.script_type == script_types::string) throw std::runtime_error("Could not use 'prev' function for string script");
      if (input.script_type == script_types::object) throw std::runtime_error("Trying to get object 'prev' not from complex object. Is it an error?");
      
      auto new_input = input;
      new_input.prev = new_input.current;
      new_input.current = input.prev;
      
      size_t final_count = 0;
      size_t final_size = 0;
      
      interface* scope = nullptr;
      if (cont != nullptr) scope = cont->add<type>();
      final_count += 1;
      final_size += sizeof(type);
      
      const auto [ptr, count, size] = create_scope_changer(new_input, data, cont, scope); /* к сожалению это означает что self_realm будет стоять за контейнером */
      final_count += count;
      final_size += size;
      
      return std::make_tuple(ptr, final_count, final_size);
    }
    
    std::tuple<interface*, size_t, size_t> current_init(const input_data &input, const sol::object &data, container* cont) {
      using type = script::current;
      
      if ((input.expected_types & input.current) != input.current) 
        throw std::runtime_error("Wrong expectation from 'current' function: expected " + parse_type_bits(input.expected_types) + ", got " + parse_type_bits(input.current));
      
      // вообще я могу использовать current как rvalue, невалидная дата может прийти только если мы указали "current" в массиве луа, в этом случае возвращать прев?
      // прев может потребоваться как вход для некоторых функций (например для женитьбы), в этих функциях по идее должен требоваться объект
      if (!data.valid()) {
        if (input.script_type != script_types::object) throw std::runtime_error("Could not use 'current' function as rvalue");
        
        interface* ptr = nullptr;
        if (cont != nullptr) ptr = cont->add<type>();
        return std::make_tuple(ptr, 1, sizeof(type));
      }
      
      if (data.get_type() == sol::type::string && data.as<std::string_view>() == commands::names[type::type_index]) {
        interface* ptr = nullptr;
        if (cont != nullptr) ptr = cont->add<type>();
        return std::make_tuple(ptr, input.current, sizeof(type));
      }
      
      if (input.script_type == script_types::string) throw std::runtime_error("Could not use 'current' function for string script");
      if (input.script_type == script_types::object) throw std::runtime_error("Trying to get object 'current' not from complex object. Is it an error?");
      
      auto new_input = input;
      new_input.prev = new_input.current;
      new_input.current = input.current;
      
      size_t final_count = 0;
      size_t final_size = 0;
      
      interface* scope = nullptr;
      if (cont != nullptr) scope = cont->add<type>();
      final_count += 1;
      final_size += sizeof(type);
      
      const auto [ptr, count, size] = create_scope_changer(new_input, data, cont, scope); /* к сожалению это означает что self_realm будет стоять за контейнером */
      final_count += count;
      final_size += size;
      
      return std::make_tuple(ptr, final_count, final_size);
    }
    
    std::tuple<interface*, size_t, size_t> equals_to_init(const input_data &input, const sol::object &data, container* cont) {
      using type = script::equals_to;
      // ожидаю что угодно, но из луа должна придти строка
      if ((input.expected_types & type::output_type) != type::output_type) 
        throw std::runtime_error("Wrong expectation from 'current' function: expected " + parse_type_bits(input.expected_types) + ", got " + parse_type_bits(type::output_type));
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<type>();
      size_t final_count = 1;
      size_t final_size = sizeof(type);
      
      interface* obj = nullptr;
      if (data.get_type() == sol::type::string) {
        auto new_input = input;
        new_input.expected_types = script::object::type_bit::all;
        const auto [ptr, count, size] = complex_object_init(new_input, data, cont);
        obj = ptr;
        final_count += count;
        final_size += size;
      } else throw std::runtime_error("Not implemented yet");
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        auto init = cont->get_init<type>(offset);
        final_ptr = init.init(obj);
      }
      
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
    std::tuple<interface*, size_t, size_t> equality_init(const input_data &input, const sol::object &data, container* cont) {
      using type = script::equality;
      
      if ((input.expected_types & type::output_type) != type::output_type) 
        throw std::runtime_error("Wrong expectation from 'equality' function: expected " + parse_type_bits(input.expected_types) + ", got " + parse_type_bits(type::output_type));
      
      if (input.script_type != script_types::numeric && input.script_type != script_types::condition)
        throw std::runtime_error("Trying to create function 'equality' in wrong script type");
      
      if (data.get_type() != sol::type::table) throw std::runtime_error("'equality' expects a lua table");
      const auto table = data.as<sol::table>();
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<type>();
      size_t final_count = 1;
      size_t final_size = sizeof(type);
      
      interface* begin = nullptr;
      interface* cur = nullptr;
      for (const auto &pair : table) {
        if (pair.first.get_type() != sol::type::number) throw std::runtime_error("Invalid key type in lua table for 'equality' function");
        
        std::tuple<interface*, size_t, size_t> tuple;
        // здесь как то нужно получить что угодно, может ли тут придти строка? вряд ли, а значит 
        switch (pair.second.get_type()) {
          case sol::type::nil: continue;
          case sol::type::boolean: tuple = boolean_container_init(input, pair.second, cont); break;
          case sol::type::number:  tuple = number_container_init(input, pair.second, cont);  break;
          case sol::type::string:  tuple = complex_object_init(input, pair.second, cont);    break;
          case sol::type::table:   tuple = number_container_init(input, pair.second, cont);  break; // по умолчанию ждем число
          
          default: throw std::runtime_error("Bad value type");
        }
        
        const auto [ptr, count, size] = tuple;
        if (begin == nullptr) begin = ptr;
        if (cur != nullptr) cur->next = ptr;
        cur = ptr;
        final_count += count;
        final_size += size;
      }
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        auto init = cont->get_init<type>(offset);
        final_ptr = init.init(begin);
      }
      
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
    std::tuple<interface*, size_t, size_t> type_equality_init(const input_data &input, const sol::object &data, container* cont) {
      using type = script::type_equality;
      
      if ((input.expected_types & type::output_type) != type::output_type) 
        throw std::runtime_error("Wrong expectation from 'type_equality' function: expected " + parse_type_bits(input.expected_types) + ", got " + parse_type_bits(type::output_type));
      
      if (input.script_type != script_types::numeric && input.script_type != script_types::condition)
        throw std::runtime_error("Trying to create function 'type_equality' in wrong script type");
      
      if (data.get_type() != sol::type::table) throw std::runtime_error("'type_equality' expects a lua table");
      const auto table = data.as<sol::table>();
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<type>();
      size_t final_count = 1;
      size_t final_size = sizeof(type);
      
      interface* begin = nullptr;
      interface* cur = nullptr;
      for (const auto &pair : table) {
        if (pair.first.get_type() != sol::type::number) throw std::runtime_error("Invalid key type in lua table for 'type_equality' function");
        
        std::tuple<interface*, size_t, size_t> tuple;
        // здесь как то нужно получить что угодно, может ли тут придти строка? вряд ли, а значит тут либо объект либо число, объект я пока не научился получать по таблице, остается число
        switch (pair.second.get_type()) {
          case sol::type::nil: continue;
          case sol::type::boolean: tuple = boolean_container_init(input, pair.second, cont); break;
          case sol::type::number:  tuple = number_container_init(input, pair.second, cont);  break;
          case sol::type::string:  tuple = complex_object_init(input, pair.second, cont);    break;
          case sol::type::table:   tuple = number_container_init(input, pair.second, cont);  break; // по умолчанию ждем число
          
          default: throw std::runtime_error("Bad value type");
        }
        
        const auto [ptr, count, size] = tuple;
        if (begin == nullptr) begin = ptr;
        if (cur != nullptr) cur->next = ptr;
        cur = ptr;
        final_count += count;
        final_size += size;
      }
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        auto init = cont->get_init<type>(offset);
        final_ptr = init.init(begin);
      }
      
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
    std::tuple<interface*, size_t, size_t> compare_init(const input_data &input, const sol::object &data, container* cont) {
      using type = script::compare;
      
      if ((input.expected_types & type::output_type) != type::output_type) 
        throw std::runtime_error("Wrong expectation from 'compare' function: expected " + parse_type_bits(input.expected_types) + ", got " + parse_type_bits(type::output_type));
      
      if (input.script_type != script_types::numeric && input.script_type != script_types::condition)
        throw std::runtime_error("Trying to create function 'compare' in wrong script type");
      
      if (data.get_type() != sol::type::table) throw std::runtime_error("'compare' expects a lua table");
      const auto table = data.as<sol::table>();
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<type>();
      size_t final_count = 1;
      size_t final_size = sizeof(type);
      
      uint32_t op = UINT32_MAX;
      interface* begin = nullptr;
      interface* cur = nullptr;
      for (const auto &pair : table) {
        if (pair.first.get_type() == sol::type::string) {
          const auto str = pair.first.as<std::string_view>();
          if (str == "op") {
            if (pair.second.get_type() != sol::type::string) throw std::runtime_error("'op' field expects to be string");
            const auto str = pair.second.as<std::string_view>();
            const auto itr = compare_operators::map.find(str);
            if (itr == compare_operators::map.end()) throw std::runtime_error("Invalid compare operator name " + std::string(str));
            op = itr->second;
          } else throw std::runtime_error("Function 'compare' expects 'op' field to be valid compare operation string");
        }
        
        if (pair.first.get_type() != sol::type::number) throw std::runtime_error("Invalid key type in lua table for 'compare' function");
        
        const auto [ptr, count, size] = number_container_init(input, pair.second, cont);
        if (begin == nullptr) begin = ptr;
        if (cur != nullptr) cur->next = ptr;
        cur = ptr;
        final_count += count;
        final_size += size;
      }
      
      if (op == UINT32_MAX) throw std::runtime_error("Function 'compare' expects string 'op' field in input lua table");
      if (final_count == 1) throw std::runtime_error("Function 'compare' expects at least one number script");
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        auto init = cont->get_init<type>(offset);
        final_ptr = init.init(op, begin);
      }
      
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
    std::tuple<interface*, size_t, size_t> add_flag_init(const input_data &input, const sol::object &data, container* cont) {
      using type = script::add_flag;
      const size_t type_index = type::type_index;
      if ((input.current & type::context_types) == 0) throw std::runtime_error("Bad context for '" + std::string(commands::names[type_index]) + "' function");
      const bool invalid_expect = input.expected_types != 0 && (input.expected_types & script::object::type_bit::invalid) != script::object::type_bit::invalid;
      if (invalid_expect) throw std::runtime_error("Bad expectation from '" + std::string(commands::names[type_index]) + "' function, expected " + parse_type_bits(input.expected_types));
      
      // вилка: либо мы ождаем строку, либо таблицу ТОЛЬКО с максимум двумя значениями flag и time
      if (data.get_type() != sol::type::string && data.get_type() != sol::type::table) throw std::runtime_error("Bad data for '" + std::string(commands::names[type_index]) + "' function");
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<type>();
      size_t final_count = 1;
      size_t final_size = sizeof(type);
      
      input_data new_input = input;
      new_input.script_type = script_types::string;
      new_input.expected_types = script::object::type_bit::string;
      if (data.get_type() == sol::type::string) {
        const auto [flag_ptr, flag_count, flag_size] = string_container_init(new_input, data, cont);
        interface* final_ptr = nullptr;
        if (cont != nullptr) {
          auto init = cont->get_init<type>(offset);
          final_ptr = init.init(flag_ptr, nullptr);
        }
        return std::make_tuple(final_ptr, 1+flag_count, sizeof(type)+flag_size);
      }
      
      const auto table = data.as<sol::table>();
      const auto flag_proxy = table["flag"];
      if (!flag_proxy.valid()) throw std::runtime_error("'flag' variable must be specified for '" + std::string(commands::names[type_index]) + "' function");
      const auto [flag_ptr, flag_count, flag_size] = string_container_init(new_input, flag_proxy, cont);
      final_count += flag_count;
      final_size += flag_size;
      
      interface* time_ptr = nullptr;
      if (const auto proxy = table["time"]; proxy.valid()) {
        new_input.script_type = script_types::numeric;
        new_input.expected_types = script::object::type_bit::valid_number;
        const auto [ptr, count, size] = number_container_init(input, flag_proxy, cont);
        time_ptr = ptr;
        final_count += count;
        final_size += size;
      }
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        auto init = cont->get_init<type>(offset);
        final_ptr = init.init(flag_ptr, time_ptr);
      }
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
    std::tuple<interface*, size_t, size_t> add_hook_init(const input_data &input, const sol::object &data, container* cont) {
      using type = script::add_hook;
      const size_t type_index = type::type_index;
      if ((input.current & type::context_types) == 0) throw std::runtime_error("Bad context for '" + std::string(commands::names[type_index]) + "' function");
      const bool invalid_expect = input.expected_types != 0 && (input.expected_types & script::object::type_bit::invalid) != script::object::type_bit::invalid;
      if (invalid_expect) throw std::runtime_error("Bad expectation from '" + std::string(commands::names[type_index]) + "' function");
      
      // тут пока что я не понимаю чего ожидать, но по идее хук_айди, хук_тайп, таргет
      if (!data.valid() || (data.get_type() != sol::type::string && data.get_type() != sol::type::table)) throw std::runtime_error("Bad data for '" + std::string(commands::names[type_index]) + "' function");
      
      assert(false);
      return std::make_tuple(nullptr, 0, 0);
    }
    
    std::tuple<interface*, size_t, size_t> add_trait_init(const input_data &input, const sol::object &data, container* cont) {
      using type = script::add_trait;
      const size_t type_index = type::type_index;
      if ((input.current & type::context_types) == 0) throw std::runtime_error("Bad context for '" + std::string(commands::names[type_index]) + "' function");
      const bool invalid_expect = input.expected_types != 0 && (input.expected_types & script::object::type_bit::invalid) != script::object::type_bit::invalid;
      if (invalid_expect) throw std::runtime_error("Bad expectation from '" + std::string(commands::names[type_index]) + "' function");
      
      if (!data.valid() || (data.get_type() != sol::type::string && data.get_type() != sol::type::table)) throw std::runtime_error("Bad data for '" + std::string(commands::names[type_index]) + "' function");
      
      // тут я ожидаю энтити, чаще всего он к нам приходит с помощью строки
      // может ли он быть в контексте? не должен по идее
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<type>();
      size_t final_count = 1;
      size_t final_size = sizeof(type);
      
      input_data new_input = input;
      const auto [ptr, count, size] = get_trait_init(new_input, data, cont);
      final_count += count;
      final_size += size;
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        auto init = cont->get_init<type>(offset);
        final_ptr = init.init(ptr);
      }
      
      final_count += 2;
      final_size += sizeof(script::get_trait) + sizeof(type);
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
//     static std::tuple<interface*, size_t, size_t> get_object_interface_functions(const input_data &input, const sol::object &data, container* cont, const size_t &expected_types) {
//       if (data.get_type() != sol::type::string) throw std::runtime_error("Character can be found only in script context");
//       
//       input_data new_input = input;
//       new_input.script_type = script_types::object;
//       new_input.expected_types = expected_types;
//       const auto str = data.as<std::string_view>();
//       if (str == commands::names[commands::root]) return root_init(new_input, sol::nil, cont);
//       else if (str == commands::names[commands::prev]) return prev_init(new_input, sol::nil, cont);
//       
//       return complex_object_init(new_input, data, cont);
//     }
    
//     template <typename T>
//     static std::tuple<interface*, size_t, size_t> get_object_with_id_interface_functions(const input_data &input, const sol::object &data, container* cont, const size_t &expected_types) {
//       if (data.get_type() != sol::type::string || data.get_type() != sol::type::table) throw std::runtime_error("Table or string is needed");
//       
//       size_t final_count = 1;
//       size_t final_size = sizeof(T);
//       interface* str_ptr = nullptr;
//       if (data.get_type() == sol::type::string) {
//         input_data new_input = input;
//         new_input.script_type = script_types::object;
//         new_input.expected_types = expected_types;
//         const auto str = data.as<std::string_view>();
//         const bool has_dot = str.find('.') != std::string_view::npos;
//         const bool has_colon = str.find(':') != std::string_view::npos;
//         if (has_dot || has_colon) {
//           return complex_object_init(new_input, data, cont);
//         }
//         
//         if (str == commands::names[commands::root]) return root_init(new_input, sol::nil, cont);
//         else if (str == commands::names[commands::prev]) return prev_init(new_input, sol::nil, cont);
//         
//         str_ptr = cont->add<script::string_container>(str);
//       } else {
//         input_data new_input = input;
//         new_input.script_type = script_types::string;
//         
//         const auto [ptr, count, size] = string_container_init(new_input, data, cont);
//         str_ptr = ptr;
//         final_count += count;
//         final_size += size;
//       }
//       
//       auto final_ptr = cont->add<T>(str_ptr);
//       return std::make_tuple(final_ptr, final_count, final_size);
//     }
    
    std::tuple<interface*, size_t, size_t> marry_init(const input_data &input, const sol::object &data, container* cont) {
      using type = script::marry;
      const size_t type_index = type::type_index;
      if ((input.current & type::context_types) == 0) throw std::runtime_error("Bad context for '" + std::string(commands::names[type_index]) + "' function");
      const bool invalid_expect = input.expected_types != 0 && (input.expected_types & script::object::type_bit::invalid) != script::object::type_bit::invalid;
      if (invalid_expect) throw std::runtime_error("Bad expectation from '" + std::string(commands::names[type_index]) + "' function");

      if (!data.valid() || data.get_type() != sol::type::string) throw std::runtime_error("Bad data for '" + std::string(commands::names[type_index]) + "' function");
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<type>();
      size_t final_count = 1;
      size_t final_size = sizeof(type);
      
      // тут я ожидаю персонажа: он может быть из контекста, может быть прев или рут, еще что нибудь?
      // все это строки, может ли тут быть что то кроме строки? сильно сомневаюсь, причем тут строго строка
      
      input_data new_input = input;
      new_input.script_type = script_types::object;
      const auto [char_ptr, count, size] = get_character_init(new_input, data, cont);
      final_count += count;
      final_size += size;
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        auto init = cont->get_init<type>(offset);
        final_ptr = init.init(char_ptr);
      }
      
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
    std::tuple<interface*, size_t, size_t> divorce_init(const input_data &input, const sol::object &data, container* cont) {
      using type = script::divorce;
      const size_t type_index = type::type_index;
      if ((input.current & type::context_types) == 0) throw std::runtime_error("Bad context for '" + std::string(commands::names[type_index]) + "' function");
      const bool invalid_expect = input.expected_types != 0 && (input.expected_types & script::object::type_bit::invalid) != script::object::type_bit::invalid;
      if (invalid_expect) throw std::runtime_error("Bad expectation from '" + std::string(commands::names[type_index]) + "' function");
      
      // тут я ожидаю что угодно, но в контексте должен быть персонаж
      (void)data;
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) final_ptr = cont->add<type>();
      size_t final_count = 1;
      size_t final_size = sizeof(type);
      
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
    std::tuple<interface*, size_t, size_t> start_war_init(const input_data &input, const sol::object &data, container* cont) {
      using type = script::start_war;
      const size_t type_index = type::type_index;
      if ((input.current & type::context_types) == 0) throw std::runtime_error("Bad context for '" + std::string(commands::names[type_index]) + "' function");
      const bool invalid_expect = input.expected_types != 0 && (input.expected_types & script::object::type_bit::invalid) != script::object::type_bit::invalid;
      if (invalid_expect) throw std::runtime_error("Bad expectation from '" + std::string(commands::names[type_index]) + "' function");
      
      if (!data.valid() || data.get_type() != sol::type::table) throw std::runtime_error("Bad data for '" + std::string(commands::names[type_index]) + "' function");
      
      // я ожидаю целый ряд данных: таргет, клаимант, казус белли, титулы (может быть несколько), возможно атакующий реалм, возможно обороняющийся реалм
      // как это все дело найти?
      
      const auto table = data.as<sol::table>();
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<type>();
      size_t final_count = 1;
      size_t final_size = sizeof(type);
      
      // откуда взять казус белли? по идее он может прийти либо из контекста, либо мы можем попробовать найти его по id
      // с титулами тоже самое? скорее всего
      interface* target = nullptr;
      {
        const auto [ptr, count, size] = get_character_init(input, table["target"], cont);
        target = ptr;
        final_count += count;
        final_size += size;
      }
      
      interface* claimant = nullptr;
      {
        const auto [ptr, count, size] = get_character_init(input, table["claimant"], cont);
        claimant = ptr;
        final_count += count;
        final_size += size;
      }
      
      interface* casus_belli = nullptr;
      {
        sol::object proxy = table["casus_belli"];
        if (!proxy.valid()) proxy = table["cb"];
        const auto [ptr, count, size] = get_casus_belli_init(input, proxy, cont);
        casus_belli = ptr;
        final_count += count;
        final_size += size;
      }
      assert(casus_belli != nullptr);
      
      const auto proxy = table["titles"];
      if (!proxy.valid() || proxy.get_type() != sol::type::table) throw std::runtime_error("Could not find titles table in start_war data");
      
      const auto titles_table = proxy.get<sol::table>();
      interface* titles = nullptr;
      for (const auto &pair : titles_table) {
        if (pair.first.get_type() != sol::type::number) continue;
        const auto [ptr, count, size] = get_title_init(input, proxy, cont);
        if (titles != nullptr) titles->next = ptr;
        titles = ptr;
        final_count += count;
        final_size += size;
      }
      assert(titles != nullptr);
      
      // этих данных более менее хватает
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        auto init = cont->get_init<type>(offset);
        final_ptr = init.init(target, casus_belli, claimant, titles, nullptr, nullptr);
      }
      
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
    std::tuple<interface*, size_t, size_t> imprison_init(const input_data &input, const sol::object &data, container* cont) {
      using type = script::imprison;
      const size_t type_index = type::type_index;
      if ((input.current & type::context_types) == 0) throw std::runtime_error("Bad context for '" + std::string(commands::names[type_index]) + "' function");
      const bool invalid_expect = input.expected_types != 0 && (input.expected_types & script::object::type_bit::invalid) != script::object::type_bit::invalid;
      if (invalid_expect) throw std::runtime_error("Bad expectation from '" + std::string(commands::names[type_index]) + "' function");
      
      if (!data.valid() || data.get_type() != sol::type::table) throw std::runtime_error("Bad data for '" + std::string(commands::names[type_index]) + "' function");
      
      // тут я ожидаю что в данных будет указана строка не персонажа
      // или таблица в которой будет указан таргет и возможно реалм
      // а мне еще нужно указать причину !!! тут не может быть просто строки
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<type>();
      size_t final_count = 1;
      size_t final_size = sizeof(type);
      
      interface* target = nullptr;
      if (data.get_type() == sol::type::string) {
        const auto [ptr, count, size] = get_character_init(input, data, cont);
        target = ptr;
        final_count += count;
        final_size += size;
      } else {
        // тут должен быть указан таргет и может быть указан реалм
        const auto table = data.as<sol::table>();
        const auto [ptr, count, size] = get_character_init(input, table["target"], cont);
        target = ptr;
        final_count += count;
        final_size += size;
      }
        
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        auto init = cont->get_init<type>(offset);
        final_ptr = init.init(target, nullptr);
      }
      
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
    template <typename T>
    static std::tuple<interface*, size_t, size_t> character_condition_function_init(const input_data &input, const sol::object &data, container* cont) {
      if ((input.current & T::context_types) == 0) throw std::runtime_error("Bad context for '" + std::string(commands::names[T::type_index]) + "' function");
      if ((input.expected_types & T::output_type) != T::output_type) throw std::runtime_error("Bad expectation from '" + std::string(commands::names[T::type_index]) + "' function");
      
      interface* ptr = nullptr;
      if (cont != nullptr) ptr = cont->add<T>();
      size_t final_count = 1;
      size_t final_size = sizeof(T);
      
      input_data new_input = input;
      new_input.script_type = script_types::numeric;
      new_input.expected_types = object::type_bit::valid_boolean;
      
      if (!data.valid() || (data.valid() && data.get_type() == sol::type::nil)) {
        // примерно тоже самое что и для сложного числа
        return std::make_tuple(ptr, final_count, final_size);
      }
      
      if (data.get_type() == sol::type::string && data.as<std::string_view>() == commands::names[T::type_index]) {
        return std::make_tuple(ptr, script::object::type_bit::valid_boolean, final_size);
      }
      
      // тут нам нужно собрать сравнение
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<script::boolean_comparator>();
      final_count += 1;
      final_size += sizeof(script::boolean_comparator);
      
      interface* rvalue = nullptr;
      if (data.get_type() == sol::type::boolean || data.get_type() == sol::type::number || data.get_type() == sol::type::string || data.get_type() == sol::type::table)  {
        const auto [ptr, count, size] = boolean_container_init(new_input, data, cont);
        rvalue = ptr;
        final_count += count;
        final_size += size;
      } else throw std::runtime_error("Ivalid data type for '" + std::string(commands::names[T::type_index]) + "' function");
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        auto init = cont->get_init<script::boolean_comparator>(offset);
        final_ptr = init.init(ptr, rvalue);
      }
      
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
    template <typename T, typename GET_OBJ_FUNC>
    static std::tuple<interface*, size_t, size_t> character_condition_object_check_function_init(const input_data &input, const sol::object &data, container* cont, const GET_OBJ_FUNC &func) {
      if ((input.current & T::context_types) == 0) throw std::runtime_error("Bad context for '" + std::string(commands::names[T::type_index]) + "' function");
      if ((input.expected_types & T::output_type) != T::output_type) throw std::runtime_error("Bad expectation from '" + std::string(commands::names[T::type_index]) + "' function");
      
      if (!data.valid() || (data.valid() && data.get_type() == sol::type::nil)) {
        throw std::runtime_error("Function '" + std::string(commands::names[T::type_index]) + "' demands input");
      }
      
      if (data.get_type() == sol::type::string && data.as<std::string_view>() == commands::names[T::type_index]) {
        throw std::runtime_error("Function '" + std::string(commands::names[T::type_index]) + "' can not be used in complex function");
      }
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<T>();
      size_t final_count = 1;
      size_t final_size = sizeof(T);
      
      input_data new_input = input;
      new_input.script_type = script_types::object;
      new_input.expected_types = T::expected_types;
      
      // тут мы ожидаем либо какой то объект, либо строку по которой мы можем получить объект
      // тут наверное стоит ожидать те объекты которые мы в принципе не можем получить через контекст
      // не, наверное лучше любые, но тут нужно разделить как то комплекс и просто строку с просто функциями
      
      const auto [ptr, count, size] = func(new_input, data, cont);
      final_count += count;
      final_size += size;
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        auto init = cont->get_init<T>(offset);
        final_ptr = init.init(ptr);
      }
      
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
    template <typename T>
    static std::tuple<interface*, size_t, size_t> character_condition_string_check_function_init(const input_data &input, const sol::object &data, container* cont) {
      if ((input.current & T::context_types) == 0) throw std::runtime_error("Bad context for '" + std::string(commands::names[T::type_index]) + "' function");
      if ((input.expected_types & T::output_type) != T::output_type) throw std::runtime_error("Bad expectation from '" + std::string(commands::names[T::type_index]) + "' function");
      
      if (data.valid() && data.get_type() == sol::type::string && data.as<std::string_view>() == commands::names[T::type_index]) {
        throw std::runtime_error("Function '" + std::string(commands::names[T::type_index]) + "' can not be used in complex function");
      }
      
      if (!data.valid() || (data.valid() && data.get_type() == sol::type::nil)) {
        throw std::runtime_error("Function '" + std::string(commands::names[T::type_index]) + "' demands input");
      }
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<T>();
      size_t final_count = 1;
      size_t final_size = sizeof(T);
      
      input_data new_input = input;
      new_input.script_type = script_types::string;
      new_input.expected_types = object::type_bit::string;
      
      interface* value_ptr = nullptr;
      if (data.get_type() == sol::type::string || data.get_type() == sol::type::table) {
        const auto [ptr, count, size] = string_container_init(new_input, data, cont);
        value_ptr = ptr;
        final_count += count;
        final_size += size;
      } else throw std::runtime_error("Ivalid data type for '" + std::string(commands::names[T::type_index]) + "' function");
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        auto init = cont->get_init<T>(offset);
        final_ptr = init.init(value_ptr);
      }
      
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
    // так теперь бесконечное количество статов и условий
    // большая часть из этих функций следует одному паттерну
    template <typename T>
    static std::tuple<interface*, size_t, size_t> stat_function_init(const input_data &input, const sol::object &data, container* cont) {
      if ((input.current & T::context_types) == 0) throw std::runtime_error("Bad context for '" + std::string(commands::names[T::type_index]) + "' function");
      if ((input.expected_types & T::output_type) != T::output_type) throw std::runtime_error("Bad expectation from '" + std::string(commands::names[T::type_index]) + "' function");
      
      interface* ptr = nullptr;
      if (cont != nullptr) ptr = cont->add<T>();
      size_t final_count = 1;
      size_t final_size = sizeof(T);
      
      if (!data.valid() || (data.valid() && data.get_type() == sol::type::nil)) {
        // примерно тоже самое что и для сложного числа
        return std::make_tuple(ptr, final_count, final_size);
      }
      
      if (data.get_type() == sol::type::string && data.as<std::string_view>() == commands::names[T::type_index]) {
        return std::make_tuple(ptr, script::object::type_bit::valid_number, final_size);
      }
      
      input_data new_input = input;
      new_input.script_type = script_types::numeric;
      new_input.expected_types = object::type_bit::valid_number;
      
      // тут нам нужно собрать сравнение
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<script::number_comparator>();
      final_count += 1;
      final_size += sizeof(script::number_comparator);
      
      uint32_t compare_op = compare_operators::more_eq;
      interface* rvalue = nullptr;
//       if (data.get_type() == sol::type::string) {
//         size_t unused = SIZE_MAX;
//         const auto [cptr, count, size] = complex_object_init(new_input, data, cont, compare_op, unused);
//         rvalue = cptr;
//         final_count += count;
//         final_size += size;
//       } else 
      if (const auto data_type = data.get_type(); data_type == sol::type::boolean || data_type == sol::type::number || data_type == sol::type::string || data_type == sol::type::table)  {
        const auto [ptr, count, size] = number_container_init(new_input, data, cont, compare_op);
        rvalue = ptr;
        final_count += count;
        final_size += size;
      } else throw std::runtime_error("Ivalid data type for '" + std::string(commands::names[T::type_index]) + "' function");
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        auto init = cont->get_init<script::number_comparator>(offset);
        final_ptr = init.init(compare_op, ptr, rvalue);
      }
      
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
    template <typename T>
    static std::tuple<interface*, size_t, size_t> add_stat_function_init(const input_data &input, const sol::object &data, container* cont) {
      if ((input.current & T::context_types) == 0) throw std::runtime_error("Bad context for '" + std::string(commands::names[T::type_index]) + "' function");
      const bool invalid_expect = input.expected_types != 0 && (input.expected_types & script::object::type_bit::invalid) != script::object::type_bit::invalid;
      if (invalid_expect) throw std::runtime_error("Bad expectation from '" + std::string(commands::names[T::type_index]) + "' function");
      
      if (!data.valid() || (data.valid() && data.get_type() == sol::type::nil)) {
        throw std::runtime_error("Function '" + std::string(commands::names[T::type_index]) + "' demands input value");
      }
      
      if (data.get_type() == sol::type::string && data.as<std::string_view>() == commands::names[T::type_index]) {
        throw std::runtime_error("Trying to create function '" + std::string(commands::names[T::type_index]) + "' in complex script");
      }
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<T>();
      size_t final_count = 1;
      size_t final_size = sizeof(T);
      
      input_data new_input = input;
      new_input.script_type = script_types::numeric;
      new_input.expected_types = object::type_bit::valid_number;
      
      interface* rvalue = nullptr;
      if (data.get_type() == sol::type::string || data.get_type() == sol::type::table || data.get_type() == sol::type::number) {
        const auto [ptr, count, size] = number_container_init(new_input, data, cont);
        rvalue = ptr;
        final_count += count;
        final_size += size;
      } else throw std::runtime_error("Ivalid data type for '" + std::string(commands::names[T::type_index]) + "' function");
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        auto init = cont->get_init<T>(offset);
        final_ptr = init.init(rvalue);
      }
      
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
#define CONDITION_COMMAND_FUNC(name) \
    std::tuple<interface*, size_t, size_t> name##_init(const input_data &input, const sol::object &data, container* cont) { \
      if (input.script_type != script_types::condition && input.script_type != script_types::numeric)                       \
        throw std::runtime_error("Trying to create function '"#name"' in wrong script type");                               \
      return character_condition_function_init<script::name>(input, data, cont);                                            \
    }                                                                                                                       \
    
    CHARACTER_GET_BOOL_NO_ARGS_COMMANDS_LIST
    REALM_GET_BOOL_NO_ARGS_COMMANDS_LIST

#undef CONDITION_COMMAND_FUNC

#define CONDITION_COMMAND_FUNC(name) \
    std::tuple<interface*, size_t, size_t> name##_init(const input_data &input, const sol::object &data, container* cont) { \
      if (input.script_type != script_types::condition && input.script_type != script_types::numeric)                       \
        throw std::runtime_error("Trying to create function '"#name"' in wrong script type");                               \
      return stat_function_init<script::name>(input, data, cont);                                                           \
    }                                                                                                                       \
    
    CHARACTER_GET_NUMBER_NO_ARGS_COMMANDS_LIST

#undef CONDITION_COMMAND_FUNC

#define CONDITION_COMMAND_FUNC(name, get_entity_init_func) \
    std::tuple<interface*, size_t, size_t> name##_init(const input_data &input, const sol::object &data, container* cont) { \
      if (input.script_type != script_types::condition && input.script_type != script_types::numeric)                       \
        throw std::runtime_error("Trying to create function '"#name"' in wrong script type");                               \
      return character_condition_object_check_function_init<script::name>(input, data, cont, get_entity_init_func);         \
    }                                                                                                                       \
    
CONDITION_COMMAND_FUNC(has_culture,        get_culture_init)
CONDITION_COMMAND_FUNC(has_culture_group,  get_culture_group_init)
CONDITION_COMMAND_FUNC(has_religion,       get_religion_init)
CONDITION_COMMAND_FUNC(has_religion_group, get_religion_group_init)
CONDITION_COMMAND_FUNC(has_trait,          get_trait_init)
CONDITION_COMMAND_FUNC(has_modificator,    get_modificator_init)
CONDITION_COMMAND_FUNC(has_same_culture_as,        get_character_init)
CONDITION_COMMAND_FUNC(has_same_culture_group_as,  get_character_init)
CONDITION_COMMAND_FUNC(has_same_religion_as,       get_character_init)
CONDITION_COMMAND_FUNC(has_same_religion_group_as, get_character_init)

CONDITION_COMMAND_FUNC(is_child_of, get_character_init)
CONDITION_COMMAND_FUNC(is_parent_of, get_character_init)
CONDITION_COMMAND_FUNC(is_sibling_of, get_character_init)
CONDITION_COMMAND_FUNC(is_half_sibling_of, get_character_init)
CONDITION_COMMAND_FUNC(is_grandparent_of, get_character_init)
CONDITION_COMMAND_FUNC(is_grandchild_of, get_character_init)
CONDITION_COMMAND_FUNC(is_close_relative_of, get_character_init)
CONDITION_COMMAND_FUNC(is_cousin_of, get_character_init)
CONDITION_COMMAND_FUNC(is_nibling_of, get_character_init)
CONDITION_COMMAND_FUNC(is_uncle_or_aunt_of, get_character_init)
CONDITION_COMMAND_FUNC(is_extended_relative_of, get_character_init)
CONDITION_COMMAND_FUNC(is_close_or_extended_relative_of, get_character_init)
CONDITION_COMMAND_FUNC(is_blood_relative_of, get_character_init)
CONDITION_COMMAND_FUNC(is_relative_of, get_character_init)
CONDITION_COMMAND_FUNC(is_owner_of, get_character_init)
CONDITION_COMMAND_FUNC(is_concubine_of, get_character_init)

#undef CONDITION_COMMAND_FUNC

#define CONDITION_COMMAND_FUNC(name)                                                                                        \
    std::tuple<interface*, size_t, size_t> name##_init(const input_data &input, const sol::object &data, container* cont) { \
      if (input.script_type != script_types::condition && input.script_type != script_types::numeric)                       \
        throw std::runtime_error("Trying to create function '"#name"' in wrong script type");                               \
      return character_condition_string_check_function_init<script::name>(input, data, cont);                               \
    }                                                                                                                       \
    
CONDITION_COMMAND_FUNC(has_flag)

#undef CONDITION_COMMAND_FUNC

    std::tuple<interface*, size_t, size_t> has_right_init(const input_data &input, const sol::object &data, container* cont) {
      using type = script::has_right;
      
      if (input.script_type != script_types::condition && input.script_type != script_types::numeric)
        throw std::runtime_error("Trying to create function '" + std::string(commands::names[type::type_index]) + "' in wrong script type");
      
      if ((input.current & type::context_types) == 0) throw std::runtime_error("Bad context for '" + std::string(commands::names[type::type_index]) + "' function");
      if ((input.expected_types & type::output_type) != type::output_type) throw std::runtime_error("Bad expectation from '" + std::string(commands::names[type::type_index]) + "' function");
      
      if (!data.valid() || data.get_type() != sol::type::string) throw std::runtime_error("Function '" + std::string(commands::names[type::type_index]) + "' expects string as input");
      
      const auto str = data.as<std::string_view>();
      
      const auto itr = core::power_rights::map.find(str);
      if (itr == core::power_rights::map.end()) throw std::runtime_error("Could not find political power right " + std::string(str));
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        final_ptr = cont->add<type>(itr->second);
      }
      
      return std::make_tuple(final_ptr, 1, sizeof(type));
    }
    
    std::tuple<interface*, size_t, size_t> has_state_right_init(const input_data &input, const sol::object &data, container* cont) {
      using type = script::has_state_right;
      
      if (input.script_type != script_types::condition && input.script_type != script_types::numeric)
        throw std::runtime_error("Trying to create function '" + std::string(commands::names[type::type_index]) + "' in wrong script type");
      
      if ((input.current & type::context_types) == 0) throw std::runtime_error("Bad context for '" + std::string(commands::names[type::type_index]) + "' function");
      if ((input.expected_types & type::output_type) != type::output_type) throw std::runtime_error("Bad expectation from '" + std::string(commands::names[type::type_index]) + "' function");
      
      if (!data.valid() || data.get_type() != sol::type::string) throw std::runtime_error("Function '" + std::string(commands::names[type::type_index]) + "' expects string as input");
      
      const auto str = data.as<std::string_view>();
      
      const auto itr = core::state_rights::map.find(str);
      if (itr == core::state_rights::map.end()) throw std::runtime_error("Could not find state right " + std::string(str));
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        final_ptr = cont->add<type>(itr->second);
      }
      
      return std::make_tuple(final_ptr, 1, sizeof(type));
    }
    
    std::tuple<interface*, size_t, size_t> has_enacted_law_with_flag_init(const input_data &input, const sol::object &data, container* cont) {
      using type = script::has_enacted_law_with_flag;
      
      if (input.script_type != script_types::condition && input.script_type != script_types::numeric)
        throw std::runtime_error("Trying to create function '" + std::string(commands::names[type::type_index]) + "' in wrong script type");
      
      if ((input.current & type::context_types) == 0) throw std::runtime_error("Bad context for '" + std::string(commands::names[type::type_index]) + "' function");
      if ((input.expected_types & type::output_type) != type::output_type) throw std::runtime_error("Bad expectation from '" + std::string(commands::names[type::type_index]) + "' function");
      
      if (!data.valid() || data.get_type() != sol::type::string) throw std::runtime_error("Function '" + std::string(commands::names[type::type_index]) + "' expects string as input");
      
      const auto str = data.as<std::string_view>();
      
//       const auto itr = core::state_rights::map.find(str);
//       if (itr == core::state_rights::map.end()) throw std::runtime_error("Could not find state right " + std::string(str));
      size_t type_data = 0;
      assert(false && "Not implemented yet");
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        final_ptr = cont->add<type>(type_data);
      }
      
      return std::make_tuple(final_ptr, 1, sizeof(type));
    }
    
// для статов нужно еще добавить быстрое сравнение разницы
#define STAT_FUNC(name)                                                                                                     \
    std::tuple<interface*, size_t, size_t> name##_init(const input_data &input, const sol::object &data, container* cont) { \
      if (input.script_type != script_types::condition && input.script_type != script_types::numeric)                       \
        throw std::runtime_error("Trying to create function '"#name"' in wrong script type");                               \
      return stat_function_init<script::name>(input, data, cont);                                                           \
    }                                                                                                                       \
                                                                                                                            \
    std::tuple<interface*, size_t, size_t> base_##name##_init(const input_data &input, const sol::object &data, container* cont) { \
      if (input.script_type != script_types::condition && input.script_type != script_types::numeric)                       \
        throw std::runtime_error("Trying to create function 'base_"#name"' in wrong script type");                          \
      return stat_function_init<script::base_##name>(input, data, cont);                                                    \
    }                                                                                                                       \
    
#define CHARACTER_PENALTY_STAT_FUNC(name) STAT_FUNC(name##_penalty)

    UNIQUE_STATS_LIST
    
#undef CHARACTER_PENALTY_STAT_FUNC
#undef STAT_FUNC

#define STAT_FUNC(name)                                                                                                     \
    std::tuple<interface*, size_t, size_t> name##_init(const input_data &input, const sol::object &data, container* cont) { \
      if (input.script_type != script_types::condition && input.script_type != script_types::numeric)                       \
        throw std::runtime_error("Trying to create function '"#name"' in wrong script type");                               \
      return stat_function_init<script::name>(input, data, cont);                                                           \
    }                                                                                                                       \
    
    UNIQUE_RESOURCES_LIST
    
//     STAT_FUNC(age)
    
#undef STAT_FUNC

#define STAT_FUNC(name) \
    std::tuple<interface*, size_t, size_t> add_##name##_init(const input_data &input, const sol::object &data, container* cont) { \
      if (input.script_type != script_types::effect) throw std::runtime_error("Trying to create function 'add_"#name"' in wrong script type"); \
      return add_stat_function_init<script::add_##name>(input, data, cont);                                                           \
    }

#define CHARACTER_PENALTY_STAT_FUNC(name) STAT_FUNC(name##_penalty)
        UNIQUE_STATS_RESOURCES_LIST
#undef STAT_FUNC
#undef CHARACTER_PENALTY_STAT_FUNC

    // что сюда приходит? строка, а может ли быть не строка? это технические функции, они не будут расшарены в паблик, поэтому ожидаем здесь строку всегда
    // можно ли просто строку ожидать? а не сол объект? вряд ли, можем ли мы тут ожидать таблицу для выбора из строк? вообще возможно
    template <typename T>
    static std::tuple<interface*, size_t, size_t> get_type_with_id_init(const input_data &input, const sol::object &data, container* cont) {
      if (!data.valid() || (data.get_type() != sol::type::string && data.get_type() != sol::type::table)) throw std::runtime_error("Invalid data for get type, bug?");
      if ((input.expected_types & T::output_type) != T::output_type) throw std::runtime_error("Expected " + parse_type_bits(input.expected_types) + ", got " + parse_type_bits(T::output_type));
      
      if (data.get_type() == sol::type::string) {
        const auto str = data.as<std::string_view>();
        if (valid_complex_variable(str)) {
          input_data new_input = input;
          new_input.script_type = script_types::object;
          new_input.expected_types = T::output_type;
          
//           if (str == commands::names[commands::root]) return root_init(new_input, sol::nil, cont);
//           else if (str == commands::names[commands::prev]) return prev_init(new_input, sol::nil, cont);
          
          return complex_object_init(new_input, data, cont);
          //throw std::runtime_error("Invalid string " + std::string(str)); // почему?
        }
        interface* final_ptr = nullptr;
        if (cont != nullptr) {
          const size_t offset = cont->add_delayed<T>();
          auto ptr = cont->add<script::string_container>(str);
          auto init = cont->get_init<T>(offset);
          final_ptr = init.init(ptr);
        }
        
        return std::make_tuple(final_ptr, 2, sizeof(script::string_container) + sizeof(T));
      }
      
      input_data new_input = input;
      new_input.script_type = script_types::string;
      new_input.expected_types = script::object::type_bit::string;
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<T>();
      
      const auto [ptr, count, size] = compute_string_init(new_input, data, cont);
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        auto init = cont->get_init<T>(offset);
        final_ptr = init.init(ptr);
      }
      
      return std::make_tuple(final_ptr, 1+count, sizeof(T)+size);
    }
    
    static std::tuple<interface*, size_t, size_t> get_type_init(const input_data &input, const sol::object &data, container* cont, const size_t &output_type) {
      if (!data.valid() || (data.get_type() != sol::type::string && data.get_type() != sol::type::table)) throw std::runtime_error("Invalid data for get type, bug?");
      if ((input.expected_types & output_type) != output_type) throw std::runtime_error("Expected " + parse_type_bits(input.expected_types) + ", got " + parse_type_bits(output_type));
      
      if (data.get_type() == sol::type::string) {
        const auto str = data.as<std::string_view>();
        if (!valid_complex_variable(str)) throw std::runtime_error("Could not get " + parse_type_bits(output_type) + " from complex varable " + std::string(str));
        
        input_data new_input = input;
        new_input.script_type = script_types::object;
        new_input.expected_types = output_type;
        
//         if (str == commands::names[commands::root]) return root_init(new_input, sol::nil, cont);
//         else if (str == commands::names[commands::prev]) return prev_init(new_input, sol::nil, cont);
        
        return complex_object_init(new_input, data, cont);
      }
      
      throw std::runtime_error("Could not get " + parse_type_bits(output_type) + " from table");
      return std::make_tuple(nullptr, 0, 0);
    }
    
#define IMPLEMENT_INIT_FUNC(name) \
    std::tuple<interface*, size_t, size_t> name##_init(const input_data &input, const sol::object &data, container* cont) { \
      return get_type_with_id_init<script::name>(input, data, cont);                                                        \
    }
    
    IMPLEMENT_INIT_FUNC(get_building_type)
    IMPLEMENT_INIT_FUNC(get_holding_type)
    IMPLEMENT_INIT_FUNC(get_city_type)
    IMPLEMENT_INIT_FUNC(get_trait)
    IMPLEMENT_INIT_FUNC(get_modificator)
    IMPLEMENT_INIT_FUNC(get_troop_type)
    IMPLEMENT_INIT_FUNC(get_religion_group)
    IMPLEMENT_INIT_FUNC(get_religion)
    IMPLEMENT_INIT_FUNC(get_culture_group)
    IMPLEMENT_INIT_FUNC(get_culture)
    IMPLEMENT_INIT_FUNC(get_law)
    IMPLEMENT_INIT_FUNC(get_casus_belli)
    
#undef IMPLEMENT_INIT_FUNC
    
    std::tuple<interface*, size_t, size_t> get_title_init(const input_data &input, const sol::object &data, container* cont) {
      return get_type_with_id_init<script::get_titulus>(input, data, cont);
    }
    
#define IMPLEMENT_INIT_FUNC(name, output_type)                                                                              \
    std::tuple<interface*, size_t, size_t> name##_init(const input_data &input, const sol::object &data, container* cont) { \
      return get_type_init(input, data, cont, output_type);                                                                 \
    }
    
    IMPLEMENT_INIT_FUNC(get_city,       script::object::type_bit::city)
    IMPLEMENT_INIT_FUNC(get_province,   script::object::type_bit::province)
    IMPLEMENT_INIT_FUNC(get_character,  script::object::type_bit::character)
    IMPLEMENT_INIT_FUNC(get_realm,      script::object::type_bit::realm)
    IMPLEMENT_INIT_FUNC(get_hero_troop, script::object::type_bit::hero_troop)
    IMPLEMENT_INIT_FUNC(get_troop,      script::object::type_bit::troop)
    IMPLEMENT_INIT_FUNC(get_army,       script::object::type_bit::army)
    IMPLEMENT_INIT_FUNC(get_war,        script::object::type_bit::war)
    
#undef IMPLEMENT_INIT_FUNC

    std::tuple<interface*, size_t, size_t> save_local_init(const input_data &input, const sol::object &data, container* cont) {
      if (!data.valid() || (data.get_type() == sol::type::string && data.as<std::string_view>() == commands::names[script::save_local::type_index])) {
        throw std::runtime_error("Trying to create function '" + std::string(commands::names[script::save_local::type_index]) + "' in complex script");
      }
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<script::save_local>();
      size_t final_count = 1;
      size_t final_size = sizeof(script::save_local);
      
      interface* str_ptr = nullptr;
      interface* var_ptr = nullptr;
      if (data.get_type() == sol::type::string) {
        const auto str = data.as<std::string_view>();
        if (valid_complex_variable(str)) throw std::runtime_error(std::string(str) + " is invalid name. Name for variable must not contain dots '.' and colons ':' and must not equals \"root\", \"prev\" or \"current\"");
        if (str == commands::names[script::save_local::type_index]) throw std::runtime_error("Name must not match function name");
        
        if (cont != nullptr) str_ptr = cont->add<script::string_container>(str);
        final_count += 1;
        final_size += sizeof(script::string_container);
      } else if (data.get_type() == sol::type::table) {
        const auto table = data.as<sol::table>();
        const auto proxy = table["name"];
        if (!proxy.valid()) throw std::runtime_error(std::string(commands::names[script::save_local::type_index]) + " requires 'name' table field and optional 'value'");
        
        input_data new_input = input;
        new_input.script_type = script_types::string;
        new_input.expected_types = script::object::type_bit::string;
        const auto [ptr, count, size] = string_container_init(new_input, proxy, cont);
        str_ptr = ptr;
        final_count += count;
        final_size += size;
         
        if (const auto val_proxy = table["value"]; val_proxy.valid()) {
          if (val_proxy.get_type() == sol::type::string) {
            input_data new_input = input;
            new_input.expected_types = script::object::type_bit::all;
            const auto [ptr, count, size] = complex_object_init(new_input, val_proxy, cont);
            str_ptr = ptr;
            final_count += count;
            final_size += size;
          } else throw std::runtime_error("Not implemented");
        } 
      } else throw std::runtime_error("Invalid value type for function '" + std::string(commands::names[script::save_local::type_index]) + "'");
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        auto init = cont->get_init<script::save_local>(offset);
        final_ptr = init.init(str_ptr, var_ptr);
      }
      
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
    std::tuple<interface*, size_t, size_t> has_local_init(const input_data &input, const sol::object &data, container* cont) {
      if (input.script_type != script_types::condition && input.script_type != script_types::numeric) {
        throw std::runtime_error("Trying to create function '" + std::string(commands::names[script::has_local::type_index]) + "' in wrong script type");
      }
      
      if (!data.valid() || (data.get_type() != sol::type::string && data.get_type() != sol::type::table)) 
        throw std::runtime_error("Invalid value type for function '" + std::string(commands::names[script::has_local::type_index]) + "'");
      
      if (data.get_type() == sol::type::string && data.as<std::string_view>() == commands::names[script::has_local::type_index]) {
        throw std::runtime_error("Trying to create function '" + std::string(commands::names[script::has_local::type_index]) + "' in complex script");
      }
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<script::has_local>();
      size_t final_count = 1;
      size_t final_size = sizeof(script::has_local);
      
      interface* str_ptr = nullptr;
      input_data new_input = input;
      new_input.script_type = script_types::string;
      new_input.expected_types = script::object::type_bit::string;
      const auto [ptr, count, size] = string_container_init(new_input, data, cont);
      str_ptr = ptr;
      final_count += count;
      final_size += size;
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        auto init = cont->get_init<script::has_local>(offset);
        final_ptr = init.init(str_ptr);
      }
      
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
    std::tuple<interface*, size_t, size_t> remove_local_init(const input_data &input, const sol::object &data, container* cont) {
      if (!data.valid() || (data.get_type() != sol::type::string && data.get_type() != sol::type::table)) 
        throw std::runtime_error("Invalid value type for function '" + std::string(commands::names[script::remove_local::type_index]) + "'");
      
      if (data.get_type() == sol::type::string && data.as<std::string_view>() == commands::names[script::remove_local::type_index]) {
        throw std::runtime_error("Trying to create function '" + std::string(commands::names[script::remove_local::type_index]) + "' in complex script");
      }
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<script::remove_local>();
      size_t final_count = 1;
      size_t final_size = sizeof(script::remove_local);
      
      interface* str_ptr = nullptr;
      input_data new_input = input;
      new_input.script_type = script_types::string;
      new_input.expected_types = script::object::type_bit::string;
      const auto [ptr, count, size] = string_container_init(new_input, data, cont);
      str_ptr = ptr;
      final_count += count;
      final_size += size;
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        auto init = cont->get_init<script::remove_local>(offset);
        final_ptr = init.init(str_ptr);
      }
      
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
    std::tuple<interface*, size_t, size_t> save_global_init(const input_data &input, const sol::object &data, container* cont) {
      if (!data.valid()) throw std::runtime_error("Wrong function '" + std::string(commands::names[script::save_global::type_index]) + "' input");
      
      if (input.script_type != script_types::effect) {
        throw std::runtime_error("Trying to create function '" + std::string(commands::names[script::save_global::type_index]) + "' in wrong script type");
      }
      
      if (data.get_type() == sol::type::string && data.as<std::string_view>() == commands::names[script::save_global::type_index]) {
        throw std::runtime_error("Trying to create function '" + std::string(commands::names[script::save_global::type_index]) + "' in complex script");
      }
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<script::save_global>();
      size_t final_count = 1;
      size_t final_size = sizeof(script::save_global);
      
      interface* str_ptr = nullptr;
      interface* var_ptr = nullptr;
      if (data.get_type() == sol::type::string) {
        const auto str = data.as<std::string_view>();
        const bool has_colon = str.find(':') != std::string_view::npos;
        const bool has_dot   = str.find('.') != std::string_view::npos;
        if (has_colon || has_dot) throw std::runtime_error(std::string(str) + " is invalid name. Name for variable must not contain dots '.' and colons ':'");
        if (str == commands::names[script::save_global::type_index]) throw std::runtime_error("Name must not match function name");
        
        if (cont != nullptr) str_ptr = cont->add<script::string_container>(str);
        final_count += 1;
        final_size += sizeof(script::string_container);
      } else if (data.get_type() == sol::type::table) {
        const auto table = data.as<sol::table>();
        const auto proxy = table["name"];
        if (!proxy.valid()) throw std::runtime_error(std::string(commands::names[script::save_global::type_index]) + " requires 'name' table field and optional 'value'");
        
        input_data new_input = input;
        new_input.script_type = script_types::string;
        new_input.expected_types = script::object::type_bit::string;
        const auto [ptr, count, size] = string_container_init(new_input, proxy, cont);
        str_ptr = ptr;
        final_count += count;
        final_size += size;
         
        if (const auto val_proxy = table["value"]; val_proxy.valid()) {
          if (val_proxy.get_type() == sol::type::string) {
            input_data new_input = input;
            new_input.expected_types = script::object::type_bit::all;
            const auto [ptr, count, size] = complex_object_init(new_input, val_proxy, cont);
            str_ptr = ptr;
            final_count += count;
            final_size += size;
          } else throw std::runtime_error("Not implemented");
        } 
      } else throw std::runtime_error("Invalid value type for function '" + std::string(commands::names[script::save_global::type_index]) + "'");
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        auto init = cont->get_init<script::save_global>(offset);
        final_ptr = init.init(str_ptr, var_ptr);
      }
      
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
    std::tuple<interface*, size_t, size_t> save_init(const input_data &input, const sol::object &data, container* cont) {
      if (!data.valid()) throw std::runtime_error("Wrong function '" + std::string(commands::names[script::save::type_index]) + "' input");
      
      if (input.script_type != script_types::effect) {
        throw std::runtime_error("Trying to create function '" + std::string(commands::names[script::save::type_index]) + "' in wrong script type");
      }
      
      if (data.get_type() == sol::type::string && data.as<std::string_view>() == commands::names[script::save::type_index]) {
        throw std::runtime_error("Trying to create function '" + std::string(commands::names[script::save::type_index]) + "' in complex script");
      }
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<script::save>();
      size_t final_count = 1;
      size_t final_size = sizeof(script::save);
      
      interface* str_ptr = nullptr;
      interface* var_ptr = nullptr;
      if (data.get_type() == sol::type::string) {
        const auto str = data.as<std::string_view>();
        const bool has_colon = str.find(':') != std::string_view::npos;
        const bool has_dot   = str.find('.') != std::string_view::npos;
        if (has_colon || has_dot) throw std::runtime_error(std::string(str) + " is invalid name. Name for variable must not contain dots '.' and colons ':'");
        if (str == commands::names[script::save::type_index]) throw std::runtime_error("Name must not match function name");
        
        if (cont != nullptr) str_ptr = cont->add<script::string_container>(str);
        final_count += 1;
        final_size += sizeof(script::string_container);
      } else if (data.get_type() == sol::type::table) {
        const auto table = data.as<sol::table>();
        const auto proxy = table["name"];
        if (!proxy.valid()) throw std::runtime_error(std::string(commands::names[script::save::type_index]) + " requires 'name' table field and optional 'value'");
        
        input_data new_input = input;
        new_input.script_type = script_types::string;
        new_input.expected_types = script::object::type_bit::string;
        const auto [ptr, count, size] = string_container_init(new_input, proxy, cont);
        str_ptr = ptr;
        final_count += count;
        final_size += size;
         
        if (const auto val_proxy = table["value"]; val_proxy.valid()) {
          if (val_proxy.get_type() == sol::type::string) {
            input_data new_input = input;
            new_input.expected_types = script::object::type_bit::all;
            const auto [ptr, count, size] = complex_object_init(new_input, val_proxy, cont);
            str_ptr = ptr;
            final_count += count;
            final_size += size;
          } else throw std::runtime_error("Not implemented");
        } 
      } else throw std::runtime_error("Invalid value type for function '" + std::string(commands::names[script::save::type_index]) + "'");
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        auto init = cont->get_init<script::save>(offset);
        final_ptr = init.init(str_ptr, var_ptr);
      }
      
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
    std::tuple<interface*, size_t, size_t> has_global_init(const input_data &input, const sol::object &data, container* cont) {
      if (!data.valid() || (data.get_type() != sol::type::string && data.get_type() != sol::type::table)) 
        throw std::runtime_error("Invalid value type for function '" + std::string(commands::names[script::has_global::type_index]) + "'");
      
      if (input.script_type != script_types::condition && input.script_type != script_types::numeric) {
        throw std::runtime_error("Trying to create function '" + std::string(commands::names[script::has_global::type_index]) + "' in wrong script type");
      }
      
      if (data.get_type() == sol::type::string && data.as<std::string_view>() == commands::names[script::has_global::type_index]) {
        throw std::runtime_error("Trying to create function '" + std::string(commands::names[script::has_global::type_index]) + "' in complex script");
      }
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<script::has_global>();
      size_t final_count = 1;
      size_t final_size = sizeof(script::has_global);
      
      interface* str_ptr = nullptr;
      input_data new_input = input;
      new_input.script_type = script_types::string;
      new_input.expected_types = script::object::type_bit::string;
      const auto [ptr, count, size] = string_container_init(new_input, data, cont);
      str_ptr = ptr;
      final_count += count;
      final_size += size;
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        auto init = cont->get_init<script::has_global>(offset);
        final_ptr = init.init(str_ptr);
      }
      
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
    std::tuple<interface*, size_t, size_t> remove_global_init(const input_data &input, const sol::object &data, container* cont) {
      if (!data.valid() || (data.get_type() != sol::type::string && data.get_type() != sol::type::table)) 
        throw std::runtime_error("Invalid value type for function '" + std::string(commands::names[script::remove_global::type_index]) + "'");
      
      if (data.get_type() == sol::type::string && data.as<std::string_view>() == commands::names[script::remove_global::type_index]) {
        throw std::runtime_error("Trying to create function '" + std::string(commands::names[script::remove_global::type_index]) + "' in complex script");
      }
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<script::remove_global>();
      size_t final_count = 1;
      size_t final_size = sizeof(script::remove_global);
      
      interface* str_ptr = nullptr;
      input_data new_input = input;
      new_input.script_type = script_types::string;
      new_input.expected_types = script::object::type_bit::string;
      const auto [ptr, count, size] = string_container_init(new_input, data, cont);
      str_ptr = ptr;
      final_count += count;
      final_size += size;
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        auto init = cont->get_init<script::remove_global>(offset);
        final_ptr = init.init(str_ptr);
      }
      
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
    std::tuple<interface*, size_t, size_t> assert_condition_init(const input_data &input, const sol::object &data, container* cont) {
      if (!data.valid() || data.get_type() != sol::type::table) 
        throw std::runtime_error("Invalid value type for function '" + std::string(commands::names[script::assert_condition::type_index]) + "'");
      
      if (data.get_type() == sol::type::string && data.as<std::string_view>() == commands::names[script::assert_condition::type_index]) {
        throw std::runtime_error("Trying to create function '" + std::string(commands::names[script::assert_condition::type_index]) + "' in complex script");
      }
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<script::assert_condition>();
      size_t final_count = 1;
      size_t final_size = sizeof(script::assert_condition);
      
      const auto table = data.as<sol::table>();
      input_data new_input = input;
      
      interface* str_ptr = nullptr;
      if (const auto proxy = table["hint"]; proxy.valid()) {
        new_input.script_type = script_types::string;
        new_input.expected_types = script::object::type_bit::string;
        const auto [ptr, count, size] = string_container_init(new_input, proxy, cont);
        str_ptr = ptr;
        final_count += count;
        final_size += size;
      }
      
      new_input.script_type = script_types::condition;
      new_input.expected_types = script::object::type_bit::valid_boolean;
//       const auto proxy = table["condition"];
//       if (!proxy.valid()) throw std::runtime_error("'" + std::string(commands::names[script::assert_condition::type_index]) + "' function requires a condition block");
      const auto [cond_ptr, count, size] = change_scope_condition_init(new_input, data, cont);
      final_count += count;
      final_size += size;
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        auto init = cont->get_init<script::assert_condition>(offset);
        final_ptr = init.init(cond_ptr, str_ptr);
      }
      
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
    const std::string_view number_matcher = "[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)";
    const std::string_view dot_matcher = ".";
    const std::string_view colon_matcher = ":";
  }
}
