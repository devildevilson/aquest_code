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
#include "core.h"

#include <charconv>
#include "re2/re2.h"

namespace devils_engine {
  namespace script {
    static const RE2 regex_number_matcher(number_matcher);
    
    template <typename T, typename F>
    static std::tuple<interface*, size_t, size_t> complex_scope(const input_data &input, const sol::object &data, container* cont, const sol::object &lvalue, const F &f) {
      assert(lvalue.get_type() == sol::type::string);
      assert(data.get_type() == sol::type::table);
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) { offset = cont->add_delayed<T>(); }
      size_t final_count = 1;
      size_t final_size = sizeof(T);
      interface* final_ptr = nullptr;
      
      interface* scope = nullptr;
      {
        input_data new_input = input;
        new_input.script_type = script_types::object;
        const auto [ptr, count, size] = complex_object_init(new_input, lvalue, cont);
        scope = ptr;
        final_count += count;
        final_size += size;
      }
      
      const auto t = data.as<sol::table>();
      
      interface* condition = nullptr;
      if (const auto proxy = t["condition"]; proxy.valid()) {
        input_data new_input = input;
        new_input.script_type = script_types::condition;
        const auto [ptr, count, size] = AND_init(new_input, proxy, cont); /* по умолчанию возьмем AND */
        condition = ptr;
        final_count += count;
        final_size += size;
      }
      
      interface* childs = nullptr;
      {
        const auto [ptr, count, size] = f(input, data, cont);
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
    
    static std::tuple<interface*, size_t, size_t> complex_object_init(const input_data &input, const sol::object &data, container* cont, interface* lvalue) {
      // сюда к нам приходит строка, мы ее должны распарсить на части
      // строка вида "*особое действие или тип сравнения*:*число или данные для действия*.*следущий скоуп*. ... .*название переменной из скоупа*"
      assert(data.get_type() == sol::type::string);
      
      sol::state_view s = data.lua_state();
      const auto str = data.as<std::string_view>();
      assert(!str.empty());
      const size_t max_size_colon = 16;
      std::array<std::string_view, max_size_colon> colon_array;
      const size_t colon_count = divide_token(str, ":", max_size_colon, colon_array.data());
      if (colon_count == SIZE_MAX) throw std::runtime_error("Invalid string token " + std::string(str));
      
      const auto after_colon = colon_array[colon_count-1];
      const size_t max_count = 256;
      std::array<std::string_view, max_count> arr;
      const size_t count = divide_token(after_colon, ".", max_count, arr.data());
      if (count == SIZE_MAX) throw std::runtime_error("Invalid string token " + std::string(str));
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<script::complex_object>();
      
      interface* begin = nullptr;
      interface* current = nullptr;
      size_t final_count = 0;
      size_t final_size = 0;
      
      input_data in = input;
      
      uint32_t compare_op = compare_operators::more_eq;
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
          point_start = 1;
          const auto context_id = arr[0];
          if (cont != nullptr) { begin = current = cont->add<script::get_context>(context_id); }
          final_count += 1;
          final_size += sizeof(script::get_context);
          assert(i == colon_count-2);
          in.current = object::type_bit::all;
          continue;
        }
        
        // это может быть особое число (например месячный доход) + множитель (в том числе отрицательный)
        if (const auto itr = complex_object_tokens::map.find(token); itr != complex_object_tokens::map.end()) {
          assert(false);
          continue;
        }
      }
      
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
        
        ++final_count;
        final_size += size;
        if (begin == nullptr) begin = ptr;
        if (current != nullptr) current->next = ptr;
        current = ptr;
      }
      
      // последний in.current можно проверить на input.expected_types, чтобы понимать что к нам пришло
      
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
      if (input.script_type == script_types::numeric && compare_op != UINT32_MAX) {
        assert(lvalue != nullptr);
        assert(compare_op < compare_operators::count);
        interface* ptr = nullptr;
        if (cont != nullptr) { ptr = cont->add<script::number_comparator>(compare_op, lvalue, final_ptr); }
        final_ptr = ptr;
        final_count += 1;
        final_size += sizeof(script::number_comparator);
      }
      
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
    // это подойдет только для нумериков, сюда мы приходим с уже сделанной функцией выше? скорее всего, тут собираем детей
    // инпут проверяем в функциях детей
    std::tuple<interface*, size_t, size_t> numeric_table_init(const input_data &input, const sol::object &data, container* cont) {
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
        if (pair.first.get_type() == sol::type::number) {
          switch (pair.second.get_type()) {
            case sol::type::nil: break;
            case sol::type::boolean:
            case sol::type::number: {
              // почему? должно быть несколько рекурсивных функций для разных типов данных
              const auto [next, count, size] = number_container_init(input, pair.second, cont);
              if (cur != nullptr) cur->next = next;
              if (begin == nullptr) begin = next;
              cur = next;
              accum_count += count;
              accum_size += size;
              break;
            }
            
            case sol::type::string: {
              const auto str = pair.first.as<std::string_view>();
              const auto itr = commands::map.find(str);
              if (itr == commands::map.end()) {
                const auto [next, count, size] = complex_object_init(input, pair.second, cont);
                if (cur != nullptr) cur->next = next;
                if (begin == nullptr) begin = next;
                cur = next;
                accum_count += count;
                accum_size += size;
                break;
              }
              
              const auto func = commands::inits[itr->second];
              const auto [next, count, size] = func(input, sol::nil, cont);
              if (cur != nullptr) cur->next = next;
              if (begin == nullptr) begin = next;
              cur = next;
              accum_count += count;
              accum_size += size;
              break;
            }
            
            case sol::type::table: {
              const auto t = pair.second.as<sol::table>();
              if (const auto proxy = t["condition"]; proxy.valid()) { 
                const auto [next, count, size] = add_init(input, pair.second, cont);
                if (cur != nullptr) cur->next = next;
                if (begin == nullptr) begin = next;
                cur = next;
                accum_count += count;
                accum_size += size;
              } else {
                const auto [next, count, size] = numeric_table_init(input, pair.second, cont);
                if (cur != nullptr) cur->next = next;
                if (begin == nullptr) begin = next;
                cur = next;
                accum_count += count;
                accum_size += size;
              }
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
          const auto str = pair.first.as<std::string_view>();
          // почему только таблица? 
//           if (pair.second.get_type() != sol::type::table) {
//             const sol::function type = s["type"];
//             const auto obj = type(pair.second);
//             const std::string type_str = obj;
//             throw std::runtime_error("Expected table in any string key value, got " + type_str + ", key " + std::string(str));
//           }
          
          // ищем штуки здесь отдельно ручками
          if (const auto ignore_key = ignore_keys::map.find(str); ignore_key != ignore_keys::map.end()) { std::cout << "Ignored " << ignore_key->first << "\n"; continue; }
          
          const auto itr = commands::map.find(str);
          if (itr == commands::map.end()) {
            const auto [next, count, size] = complex_scope<script::compute_number>(input, pair.second, cont, pair.first, &numeric_table_init);
            if (cur != nullptr) cur->next = next;
            if (begin == nullptr) begin = next;
            cur = next;
            accum_count += count;
            accum_size += size;
            continue;
          }
          
          const auto func = commands::inits[itr->second];
          const auto [next, count, size] = func(input, pair.second, cont);
          if (cur != nullptr) cur->next = next;
          if (begin == nullptr) begin = next;
          cur = next;
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
    
    std::tuple<interface*, size_t, size_t> string_table_init(const input_data &input, const sol::object &data, container* cont) {
      assert(input.script_type == script_types::string);
      interface* begin = nullptr;
      interface* cur = nullptr;
      size_t accum_count = 0;
      size_t accum_size = 0;
      sol::state_view s = data.lua_state();
      interface* condition = nullptr;
      size_t offset = SIZE_MAX;
      const auto table = data.as<sol::table>();
      if (const auto proxy = table["condition"]; proxy.valid()) {
        if (cont != nullptr) offset = cont->add_delayed<script::compute_string>();
        input_data new_input = input;
        new_input.script_type = script_types::condition;
        const auto [ptr, count, size] = AND_init(new_input, proxy, cont); /* по умолчанию возьмем AND */
        condition = ptr;
        accum_count += count+1;
        accum_size += size + sizeof(script::compute_string);
      }
      
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
        
        if (pair.second.get_type() == sol::type::table) {
          const auto [next, count, size] = string_table_init(input, pair.second, cont);
          if (cur != nullptr) cur->next = next;
          if (begin == nullptr) begin = next;
          cur = next;
          accum_count += count;
          accum_size += size;
          continue;
        }
        
        if (pair.second.get_type() != sol::type::string) throw std::runtime_error("Bad value type in string script");
        
        const auto [next, count, size] = string_container_init(input, pair.second, cont);
        if (cur != nullptr) cur->next = next;
        if (begin == nullptr) begin = next;
        cur = next;
        accum_count += count;
        accum_size += size;
      }
      
      if (condition != nullptr && cont != nullptr) {
        auto init = cont->get_init<script::compute_string>(offset);
        auto ptr = init.init(condition, begin);
        begin = ptr;
      }
      
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
    std::tuple<interface*, size_t, size_t> condition_table_init(const input_data &input, const sol::object &data, container* cont) {
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
        if (pair.first.get_type() == sol::type::number) {
          switch (pair.second.get_type()) {
            case sol::type::nil: break;
            case sol::type::boolean: 
            case sol::type::number: {
              const auto [next, count, size] = boolean_container_init(input, pair.second, cont);
              if (cur != nullptr) cur->next = next;
              if (begin == nullptr) begin = next;
              cur = next;
              accum_count += count;
              accum_size += size;
              break;
            }
            
            case sol::type::string: {
              const auto str = pair.first.as<std::string_view>();
              const auto itr = commands::map.find(str);
              if (itr == commands::map.end()) {
                const auto [next, count, size] = complex_object_init(input, pair.second, cont);
                if (cur != nullptr) cur->next = next;
                if (begin == nullptr) begin = next;
                cur = next;
                accum_count += count;
                accum_size += size;
                break;
              }
              
              const auto func = commands::inits[itr->second];
              const auto [next, count, size] = func(input, sol::nil, cont);
              if (cur != nullptr) cur->next = next;
              if (begin == nullptr) begin = next;
              cur = next;
              accum_count += count;
              accum_size += size;
              break;
            }
            
            case sol::type::table: {
              const auto t = pair.second.as<sol::table>();
              if (const auto proxy = t["condition"]; proxy.valid()) { 
                const auto [next, count, size] = change_scope_condition_init(input, pair.second, cont);
                if (cur != nullptr) cur->next = next;
                if (begin == nullptr) begin = next;
                cur = next;
                accum_count += count;
                accum_size += size;
              } else {
                const auto [next, count, size] = condition_table_init(input, pair.second, cont);
                if (cur != nullptr) cur->next = next;
                if (begin == nullptr) begin = next;
                cur = next;
                accum_count += count;
                accum_size += size;
              }
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
          
          continue;
        }
        
        if (pair.first.get_type() == sol::type::string) {
          const auto str = pair.first.as<std::string_view>();
//           if (pair.second.get_type() != sol::type::table) {
//             const sol::function type = s["type"];
//             const auto obj = type(pair.second);
//             const std::string type_str = obj;
//             throw std::runtime_error("Expected table in any string key value, got " + type_str + ", key " + std::string(str));
//           }
          
          // возможно только касается condition?
          if (const auto ignore_key = ignore_keys::map.find(str); ignore_key != ignore_keys::map.end()) { std::cout << "Ignored " << ignore_key->first << "\n"; continue; }
          
          const auto itr = commands::map.find(str);
          if (itr == commands::map.end()) {
            const auto [next, count, size] = complex_scope<script::change_scope_condition>(input, pair.second, cont, pair.first, &condition_table_init);
            if (cur != nullptr) cur->next = next;
            if (begin == nullptr) begin = next;
            cur = next;
            accum_count += count;
            accum_size += size;
            continue;
          }
          
          const auto func = commands::inits[itr->second];
          const auto [next, count, size] = func(input, pair.second, cont);
          if (cur != nullptr) cur->next = next;
          if (begin == nullptr) begin = next;
          cur = next;
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
        if (pair.first.get_type() == sol::type::number) {
          switch (pair.second.get_type()) {
            case sol::type::nil: break;
            
            case sol::type::table: {
              const auto t = pair.second.as<sol::table>();
              if (const auto proxy = t["condition"]; proxy.valid()) { 
                const auto [next, count, size] = change_scope_effect_init(input, pair.second, cont);
                if (cur != nullptr) cur->next = next;
                if (begin == nullptr) begin = next;
                cur = next;
                accum_count += count;
                accum_size += size;
              } else {
                const auto [next, count, size] = effect_table_init(input, pair.second, cont);
                if (cur != nullptr) cur->next = next;
                if (begin == nullptr) begin = next;
                cur = next;
                accum_count += count;
                accum_size += size;
              }
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
          
          continue;
        }
        
        if (pair.first.get_type() == sol::type::string) {
          const auto str = pair.first.as<std::string_view>();
//           if (pair.second.get_type() != sol::type::table) {
//             const sol::function type = s["type"];
//             const auto obj = type(pair.second);
//             const std::string type_str = obj;
//             throw std::runtime_error("Expected table in any string key value, got " + type_str + ", key " + std::string(str));
//           }
          
          // возможно только касается condition?
          if (const auto ignore_key = ignore_keys::map.find(str); ignore_key != ignore_keys::map.end()) { std::cout << "Ignored " << ignore_key->first << "\n"; continue; }
          
          const auto itr = commands::map.find(str);
          if (itr == commands::map.end()) {
            const auto [next, count, size] = complex_scope<script::change_scope_effect>(input, pair.second, cont, pair.first, &effect_table_init);
            if (cur != nullptr) cur->next = next;
            if (begin == nullptr) begin = next;
            cur = next;
            accum_count += count;
            accum_size += size;
            continue;
          }
          
          const auto func = commands::inits[itr->second];
          const auto [next, count, size] = func(input, pair.second, cont);
          if (cur != nullptr) cur->next = next;
          if (begin == nullptr) begin = next;
          cur = next;
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
      if ((input.expected_types & script::object::type_bit::invalid) == 0) throw std::runtime_error("Bad expected types input for 'effect' function");
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
        case sol::type::string: return complex_object_init(input, data, cont);
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
        const double val = data.as<double>();
        ptr = cont->add<boolean_container>(val);
      }
      return std::make_tuple(ptr, 1, align_to(sizeof(boolean_container), DEFAULT_ALIGNMENT));
    }
    
    std::tuple<interface*, size_t, size_t> number_container_init(const input_data &input, const sol::object &data, container* cont) {
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
        case sol::type::string: return complex_object_init(input, data, cont);
        // если в таблице лежит единственный объект по имени нумерик блока, то почему бы тут не указать его вместо адд? да но его еще найти надо
        case sol::type::table: return add_init(input, data, cont);
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
    
    std::tuple<interface*, size_t, size_t> string_container_init(const input_data &input, const sol::object &data, container* cont) {
      if (input.script_type != script_types::string)throw std::runtime_error("String value makes sense only in string scripts");
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
      
      interface* ptr = nullptr;
      if (cont != nullptr) {
        const auto val = data.as<std::string_view>();
        ptr = cont->add<script::string_container>(val);
      }
      return std::make_tuple(ptr, 1, align_to(sizeof(script::string_container), DEFAULT_ALIGNMENT));
    }
    
    std::tuple<interface*, size_t, size_t> compute_string_init(const input_data &input, const sol::object &data, container* cont) {
      if (input.script_type != script_types::string) throw std::runtime_error("String value makes sense only in string scripts");
      if ((input.expected_types & script::compute_string::output_type) == 0) throw std::runtime_error("Bad input expected types from number container");
      
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
        const auto [ptr, count, size] = AND_init(new_input, proxy, cont);
        condition_ptr = ptr;
        final_count += count;
        final_size += size;
      }
      
      const auto [ptr, count, size] = string_table_init(input, data, cont);
      final_count += count;
      final_size += size;
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        auto init = cont->get_init<script::compute_string>(offset);
        final_ptr = init.init(condition_ptr, ptr);
      }
      
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
    std::tuple<interface*, size_t, size_t> complex_object_init(const input_data &input, const sol::object &data, container* cont) {
      return complex_object_init(input, data, cont, nullptr);
    }

    template <typename T, typename F>
    static std::tuple<interface*, size_t, size_t> create_block(const input_data &input, const sol::object &data, container* cont, const F &table_init) {
      const auto table = data.as<sol::table>();
      const auto [child_ptr, tcount, tsize] = table_init(input, data, nullptr);
      if (tcount == 0) throw std::runtime_error("Could not find children for '" + std::string(commands::names[T::type_index]) + "' function");
      if (tcount == 1) return table_init(input, data, cont);
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<T>();
      
      const auto [ptr, count, size] = table_init(input, data, cont);
      assert(tcount == count);
      assert(tsize == size);
      
      interface* operation = nullptr;
      if (cont != nullptr) {
        auto init = cont->get_init<T>(offset);
        operation = init.init(ptr);
      }
      
      return std::make_tuple(operation, count+1, size+sizeof(T));
    }

    template <typename T, typename C, typename F>
    static std::tuple<interface*, size_t, size_t> create_compute(const input_data &input, const sol::object &data, container* cont, interface* scope, const F &table_init) {
      size_t offset = SIZE_MAX;
      if (cont != nullptr) { offset = cont->add_delayed<C>(); }
      
      size_t counter = 1;
      size_t final_size = sizeof(C);
      interface* condition = nullptr;
      const auto table = data.as<sol::table>();
      if (const auto proxy = table["condition"]; proxy.valid()) {
        input_data new_input = input;
        new_input.script_type = script_types::condition;
        const auto [ptr, count, size] = AND_init(new_input, proxy, cont); /* по умолчанию возьмем AND */
        condition = ptr;
        counter += count;
        final_size += size;
      }
      
      interface* operation = nullptr;
      {
        const auto [ptr, count, size] = create_block<T>(input, data, cont, table_init);
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
    
#define BLOCK_INIT_FUNC(func_name, script_type_container, table_init_func) \
    std::tuple<interface*, size_t, size_t> func_name##_init(const input_data &input, const sol::object &data, container* cont) { \
      if (data.get_type() == sol::type::string && data.as<std::string_view>() == commands::names[commands::func_name]) { \
        throw std::runtime_error("Function '"#func_name"' could not be used in complex function"); \
      } \
      assert(input.script_type == script_types::numeric || input.script_type == script_types::condition); \
      if ((input.expected_types & script::func_name::output_type) == 0) throw std::runtime_error("Bad expected types input for '"#func_name"' function"); \
      /* if (data.get_type() == sol::type::number || data.get_type() == sol::type::boolean) return number_container_init(input, data, cont); */ \
      /*if (data.get_type() == sol::type::string) return complex_object_init(input, data, cont);*/ \
      if (data.get_type() != sol::type::table) throw std::runtime_error("Bad lua object type for '"#func_name"' function"); \
      const auto table = data.as<sol::table>(); \
      if (table["condition"].valid()) return create_compute<script::func_name, script::script_type_container>(input, data, cont, nullptr, &table_init_func); \
      return create_block<script::func_name>(input, data, cont, &table_init_func); \
    }
    
#define NUMERIC_INIT_FUNC(func_name) \
  std::tuple<interface*, size_t, size_t> func_name##_init(const input_data &input, const sol::object &data, container* cont) { \
      if (data.get_type() == sol::type::string && data.as<std::string_view>() == commands::names[commands::func_name]) { \
        throw std::runtime_error("Function '"#func_name"' could not be used in complex function"); \
      } \
      \
      if ((input.expected_types & script::func_name::output_type) == 0) throw std::runtime_error("Bad expected types input for '"#func_name"' function"); \
      assert(input.script_type == script_types::numeric); \
      size_t offset = SIZE_MAX; \
      if (cont != nullptr) cont->add_delayed<script::func_name>(); \
      \
      size_t counter = 1; \
      size_t final_size = sizeof(script::func_name); \
      if (data.get_type() == sol::type::number || data.get_type() == sol::type::boolean) { \
        const auto [ptr, count, size] = number_container_init(input, data, cont); \
        counter += count; \
        final_size += size; \
        interface* final_ptr = nullptr; \
        if (cont != nullptr) { \
          auto init = cont->get_init<script::func_name>(offset); \
          final_ptr = init.init<script::func_name>(ptr); \
        } \
        return std::make_tuple(final_ptr, counter, final_size); \
      } \
      if (data.get_type() == sol::type::string) { \
        const auto [ptr, count, size] = complex_object_init(input, data, cont); \
        counter += count; \
        final_size += size; \
        interface* final_ptr = nullptr; \
        if (cont != nullptr) { \
          auto init = cont->get_init<script::func_name>(offset); \
          final_ptr = init.init<script::func_name>(ptr); \
        } \
        return std::make_tuple(final_ptr, counter, final_size); \
      } \
      \
      if (data.get_type() != sol::type::table) throw std::runtime_error("Bad lua object type for '"#func_name"' function"); \
      \
      const auto [ptr, count, size] = add_init(input, data, cont); \
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

CONDITION_BLOCK_INIT_FUNC(AND)
CONDITION_BLOCK_INIT_FUNC(OR)
CONDITION_BLOCK_INIT_FUNC(NAND)
CONDITION_BLOCK_INIT_FUNC(NOR)
CONDITION_BLOCK_INIT_FUNC(XOR)
CONDITION_BLOCK_INIT_FUNC(IMPL)
CONDITION_BLOCK_INIT_FUNC(EQ)

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
      if constexpr (std::is_same_v<T, script::random_vassal>) new_input.current = script::object::type_bit::realm;
      else new_input.current = script::object::type_bit::character;
      
      interface* val_ptr = nullptr;
      interface* condition_ptr = nullptr;
      interface* weight_ptr = nullptr;
      size_t final_count = 1;
      size_t final_size = sizeof(T);
      
      const auto table = data.as<sol::table>();
      if (const auto proxy = table["condition"]; proxy.valid()) {
        new_input.script_type = script_types::condition;
        const auto [ptr, count, size] = AND_init(new_input, proxy, cont);
        condition_ptr = ptr;
        final_count += count;
        final_size += size;
      }
      
      if (const auto proxy = table["weight"]; proxy.valid()) {
        new_input.script_type = script_types::numeric;
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
        final_ptr = init.init(condition_ptr, weight_ptr, val_ptr);
      }
      
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
    template <typename T>
    static std::tuple<interface*, size_t, size_t> create_one_to_every_func(const input_data &input, const sol::object &data, container* cont) {
      if (input.script_type != script_types::effect) throw std::runtime_error("Trying to create function '" + std::string(commands::names[T::type_index]) + "' in wrong script type");
      if (data.get_type() == sol::type::string && data.as<std::string_view>() == commands::names[T::type_index])
        throw std::runtime_error("Function '" + std::string(commands::names[T::type_index]) + "' can not be used in complex function");
      if ((input.current & T::context_types) == 0) throw std::runtime_error("Bad context for '" + std::string(commands::names[T::type_index]) + "' function");
      if ((input.expected_types & T::output_type) == 0) throw std::runtime_error("Bad expectation from '" + std::string(commands::names[T::type_index]) + "' function");
      
      if (data.get_type() == sol::type::string) throw std::runtime_error("Function '" + std::string(commands::names[T::type_index]) + "' can not handle string input");
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<T>();
      
      input_data new_input = input; 
      new_input.prev = input.current;
      if constexpr (std::is_same_v<T, script::every_vassal>) new_input.current = script::object::type_bit::realm;
      else new_input.current = script::object::type_bit::character;
      
      interface* val_ptr = nullptr;
      interface* condition_ptr = nullptr;
      size_t final_count = 1;
      size_t final_size = sizeof(T);
      
      const auto table = data.as<sol::table>();
      if (const auto proxy = table["condition"]; proxy.valid()) {
        new_input.script_type = script_types::condition;
        const auto [ptr, count, size] = AND_init(new_input, proxy, cont);
        condition_ptr = ptr;
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
        final_ptr = init.init(condition_ptr, val_ptr);
      }
      
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
#define CHANGE_CONTEXT_COMMAND_FUNC(func_name, context_type_bits, expected_type_bits, output_type_bit) \
    std::tuple<interface*, size_t, size_t> has##func_name##_init(const input_data &input, const sol::object &data, container* cont) { \
      if (input.script_type != script_types::numeric && input.script_type != script_types::condition)  \
        throw std::runtime_error("Trying to create function 'has"#func_name"' in wrong script type");  \
      if (data.get_type() == sol::type::string && data.as<std::string_view>() == "has"#func_name)      \
        throw std::runtime_error("Function 'has"#func_name"' can not be used in complex function");    \
      if ((input.current & script::has##func_name::context_types) == 0) throw std::runtime_error("Bad context for 'has"#func_name"' function"); \
      if ((input.expected_types & script::has##func_name::output_type) == 0) throw std::runtime_error("Bad expectation from 'has"#func_name"' function"); \
                                                                                                       \
      if (data.get_type() == sol::type::string) throw std::runtime_error("Function 'has"#func_name"' can not handle string input"); \
                                                                                                       \
      size_t offset = SIZE_MAX;                                                                        \
      if (cont != nullptr) offset = cont->add_delayed<script::has##func_name>();                       \
                                                                                                       \
      input_data new_input = input;                                                                    \
      new_input.prev = input.current;                                                                  \
      if constexpr (std::is_same_v<script::has##func_name, script::has_vassal>) new_input.current = script::object::type_bit::realm; \
      else new_input.current = script::object::type_bit::character;                                    \
                                                                                                       \
      interface* val_ptr = nullptr;                                                                    \
      interface* max_count_ptr = nullptr;                                                              \
      interface* percentage_ptr = nullptr;                                                             \
      size_t final_count = 1;                                                                          \
      size_t final_size = sizeof(script::has##func_name);                                              \
      if (data.get_type() == sol::type::boolean || data.get_type() == sol::type::number) {             \
        const auto [ptr, count, size] = boolean_container_init(input, data, cont);                     \
        val_ptr = ptr;                                                                                 \
        final_count += count;                                                                          \
        final_size += size;                                                                            \
      } else if (data.get_type() == sol::type::table) {                                                \
        const auto table = data.as<sol::table>();                                                      \
        if (const auto proxy = table["count"]; proxy.valid()) {                                        \
          new_input.script_type = script_types::numeric;                                               \
          const auto [ptr, count, size] = number_container_init(new_input, proxy, cont);               \
          max_count_ptr = ptr;                                                                         \
          final_count += count;                                                                        \
          final_size += size;                                                                          \
        }                                                                                              \
        if (const auto proxy = table["percent"]; proxy.valid()) {                                      \
          new_input.script_type = script_types::numeric;                                               \
          const auto [ptr, count, size] = number_container_init(new_input, proxy, cont);               \
          percentage_ptr = ptr;                                                                        \
          final_count += count;                                                                        \
          final_size += size;                                                                          \
        }                                                                                              \
        const auto [ptr, count, size] = condition_table_init(new_input, data, cont);                   \
        val_ptr = ptr;                                                                                 \
        final_count += count;                                                                          \
        final_size += size;                                                                            \
      } else throw std::runtime_error("Function 'has"#func_name"' can not handle this input type input"); \
                                                                                                       \
      interface* final_ptr = nullptr;                                                                  \
      if (cont != nullptr) {                                                                           \
        auto init = cont->get_init<script::has##func_name>(offset);                                    \
        final_ptr = init.init(val_ptr, max_count_ptr, percentage_ptr);                                 \
      }                                                                                                \
                                                                                                       \
      return std::make_tuple(final_ptr, final_count, final_size);                                      \
    }                                                                                                  \
                                                                                                       \
    std::tuple<interface*, size_t, size_t> every##func_name##_init(const input_data &input, const sol::object &data, container* cont) { \
      return create_one_to_every_func<script::every##func_name>(input, data, cont);                    \
    }                                                                                                  \
                                                                                                       \
    std::tuple<interface*, size_t, size_t> random##func_name##_init(const input_data &input, const sol::object &data, container* cont) { \
      return create_one_to_random_func<script::random##func_name>(input, data, cont);                  \
    }                                                                                                  \
    
    CHANGE_CONTEXT_COMMANDS_LIST

#undef CHANGE_CONTEXT_COMMAND_FUNC
    
    static std::tuple<interface*, size_t, size_t> create_scope_changer(const input_data &input, const sol::object &data, container* cont, interface* scope) {
      size_t final_count = 0;
      size_t final_size = 0;
      interface* final_ptr = nullptr;
      switch (static_cast<script_types::values>(input.script_type)) {
        case script_types::string: throw std::runtime_error("Could not use this function for string script");
        // должен быть частью сложной переменной?
        case script_types::object: throw std::runtime_error("Trying to get object not from complex object. Is it an error?");
        case script_types::numeric: {
          const auto [ptr, count, size] = create_compute<script::add, script::compute_number>(input, data, cont, scope, &numeric_table_init);
          final_ptr = ptr;
          final_count += count;
          final_size += size;
          break;
        }
        
        case script_types::condition: {
          const auto [ptr, count, size] = create_compute<script::AND, script::change_scope_condition>(input, data, cont, scope, &condition_table_init);
          final_ptr = ptr;
          final_count += count;
          final_size += size;
          break;
        }
        
        case script_types::effect: {
          const auto [ptr, count, size] = change_scope_effect_init(input, data, cont, scope);
          final_ptr = ptr;
          final_count += count;
          final_size += size;
          break;
        }
      }
      
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
    std::tuple<interface*, size_t, size_t> change_scope_condition_init(const input_data &input, const sol::object &data, container* cont) {
      if (input.script_type != script_types::condition) throw std::runtime_error("script type must be condition");
      if ((input.expected_types & script::change_scope_condition::output_type) == 0) throw std::runtime_error("bad expectation");
      
      if (data.get_type() == sol::type::string && data.as<std::string_view>() == "change_scope_condition") { 
        throw std::runtime_error("Function 'change_scope_condition' could not be used in complex function");
      }
      
      return create_compute<script::AND, script::change_scope_condition>(input, data, cont, nullptr, &condition_table_init);
    }
    
    // в эту функцию мы можем придти фактически при любых 3 раскладах, то есть если нам нужно посчитать условия
    // или посчитать число, или посчитать эффект, как это друг от друга отделить? тип в инпуте прописать? может ли смениться тип? может
    // например когда мы пытаемся сравнить два стата, может быть что мы можем ожидать два типа? не уверен, но вряд ли
    std::tuple<interface*, size_t, size_t> self_realm_init(const input_data &input, const sol::object &data, container* cont) {
      if ((input.current & script::self_realm::context_types) == 0) throw std::runtime_error("Bad context for 'self_realm' function");
      // вообще я могу использовать self_realm как rvalue
      if (!data.valid()) throw std::runtime_error("Could not use 'self_realm' function as rvalue");
      
      // может ли к нам прийти строка по другим причинам? функция для создания числовых значений может ожидать строку как сложное число
      // возможно имеет смысл здесь передать строку совпадающую с названием функции, чтобы точно определить что это инпут из сложного числа
      if (data.get_type() == sol::type::string && data.as<std::string_view>() == commands::names[commands::self_realm]) {
        interface* ptr = nullptr;
        if (cont != nullptr) ptr = cont->add<script::self_realm>();
        return std::make_tuple(ptr, script::object::type_bit::realm, sizeof(script::self_realm));
      }
      
      if (input.script_type == script_types::string) throw std::runtime_error("Could not use 'self_realm' function for string script");
      if (input.script_type == script_types::object) throw std::runtime_error("Trying to get object 'self_realm' not from complex object. Is it an error?");
      
      // не думаю что есть возможность проверить так expected_types
      //if ((input.expected_types & script::self_realm::output_type) == 0) throw std::runtime_error("Bad input expected types");
      
      auto new_input = input;
      new_input.prev = new_input.current;
      new_input.current = script::object::type_bit::realm;
      
      size_t final_count = 0;
      size_t final_size = 0;
      
      interface* scope = nullptr;
      if (cont != nullptr) scope = cont->add<script::self_realm>();
      final_count += 1;
      final_size += sizeof(script::self_realm);
      
      const auto [ptr, count, size] = create_scope_changer(new_input, data, cont, scope); /* к сожалению это означает что self_realm будет стоять за контейнером */
      final_count += count;
      final_size += size;
      
      return std::make_tuple(ptr, final_count, final_size);
    }
    
    std::tuple<interface*, size_t, size_t> root_init(const input_data &input, const sol::object &data, container* cont) {
      if (data.get_type() == sol::type::string && data.as<std::string_view>() == commands::names[commands::root]) {
        interface* ptr = nullptr;
        if (cont != nullptr) ptr = cont->add<script::root>();
        return std::make_tuple(ptr, input.root, sizeof(script::root));
      }
      
      // вообще я могу использовать prev как rvalue, невалидная дата может прийти только если мы указали "prev" в массиве луа, в этом случае возвращать прев?
      // прев может потребоваться как вход для некоторых функций (например для женитьбы), в этих функциях по идее должен требоваться объект
      if (!data.valid()) {
        if (input.script_type != script_types::object) throw std::runtime_error("Could not use 'prev' function as rvalue");
        
        interface* ptr = nullptr;
        if (cont != nullptr) ptr = cont->add<script::prev>();
        return std::make_tuple(ptr, 1, sizeof(script::prev));
      }
      
      if (input.script_type == script_types::string) throw std::runtime_error("Could not use 'self_realm' function for string script");
      if (input.script_type == script_types::object) throw std::runtime_error("Trying to get object 'self_realm' not from complex object. Is it an error?");
      
      auto new_input = input;
      new_input.prev = new_input.current;
      new_input.current = input.root;
      
      size_t final_count = 0;
      size_t final_size = 0;
      
      interface* scope = nullptr;
      if (cont != nullptr) scope = cont->add<script::root>();
      final_count += 1;
      final_size += sizeof(script::root);
      
      const auto [ptr, count, size] = create_scope_changer(new_input, data, cont, scope); /* к сожалению это означает что self_realm будет стоять за контейнером */
      final_count += count;
      final_size += size;
      
      return std::make_tuple(ptr, final_count, final_size);
    }
    
    std::tuple<interface*, size_t, size_t> prev_init(const input_data &input, const sol::object &data, container* cont) {
      if ((input.prev & object::type_bit::all) == 0) throw std::runtime_error("Invalid prev object");
      
      if (data.get_type() == sol::type::string && data.as<std::string_view>() == commands::names[commands::prev]) {
        interface* ptr = nullptr;
        if (cont != nullptr) ptr = cont->add<script::prev>();
        return std::make_tuple(ptr, input.prev, sizeof(script::prev));
      }
      
      // вообще я могу использовать prev как rvalue, невалидная дата может прийти только если мы указали "prev" в массиве луа, в этом случае возвращать прев?
      // прев может потребоваться как вход для некоторых функций (например для женитьбы), в этих функциях по идее должен требоваться объект
      if (!data.valid()) {
        if (input.script_type != script_types::object) throw std::runtime_error("Could not use 'prev' function as rvalue");
        
        interface* ptr = nullptr;
        if (cont != nullptr) ptr = cont->add<script::prev>();
        return std::make_tuple(ptr, 1, sizeof(script::prev));
      }
      
      if (input.script_type == script_types::string) throw std::runtime_error("Could not use 'self_realm' function for string script");
      if (input.script_type == script_types::object) throw std::runtime_error("Trying to get object 'self_realm' not from complex object. Is it an error?");
      
      auto new_input = input;
      new_input.prev = new_input.current;
      new_input.current = input.prev;
      
      size_t final_count = 0;
      size_t final_size = 0;
      
      interface* scope = nullptr;
      if (cont != nullptr) scope = cont->add<script::prev>();
      final_count += 1;
      final_size += sizeof(script::prev);
      
      const auto [ptr, count, size] = create_scope_changer(new_input, data, cont, scope); /* к сожалению это означает что self_realm будет стоять за контейнером */
      final_count += count;
      final_size += size;
      
      return std::make_tuple(ptr, final_count, final_size);
    }
    
    std::tuple<interface*, size_t, size_t> add_flag_init(const input_data &input, const sol::object &data, container* cont) {
      using type = script::add_flag;
      const size_t type_index = type::type_index;
      if ((input.current & type::context_types) == 0) throw std::runtime_error("Bad context for '" + std::string(commands::names[type_index]) + "' function");
      if (input.expected_types != 0) throw std::runtime_error("Bad expectation from '" + std::string(commands::names[type_index]) + "' function");
      
      // вилка: либо мы ождаем строку, либо таблицу ТОЛЬКО с максимум двумя значениями flag и time
      if (data.get_type() != sol::type::string && data.get_type() != sol::type::table) throw std::runtime_error("Bad data for '" + std::string(commands::names[type_index]) + "' function");
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<type>();
      size_t final_count = 1;
      size_t final_size = sizeof(type);
      
      if (data.get_type() == sol::type::string) {
        const auto str = data.as<std::string_view>();
        interface* final_ptr = nullptr;
        if (cont != nullptr) {
          auto container_ptr = cont->add<script::string_container>(str);
          auto init = cont->get_init<type>(offset);
          final_ptr = init.init(container_ptr, nullptr);
        }
        return std::make_tuple(final_ptr, 2, sizeof(script::string_container) + sizeof(type));
      }
      
      const auto table = data.as<sol::table>();
      const auto flag_proxy = table["flag"];
      if (!flag_proxy.valid()) throw std::runtime_error("'flag' variable must be specified for '" + std::string(commands::names[type_index]) + "' function");
      input_data new_input = input;
      new_input.script_type = script_types::string;
      const auto [flag_ptr, flag_count, flag_size] = string_container_init(new_input, flag_proxy, cont);
      final_count += flag_count;
      final_size += flag_size;
      
      interface* time_ptr = nullptr;
      if (const auto proxy = table["time"]; proxy.valid()) {
        new_input.script_type = script_types::numeric;
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
      if (input.expected_types != 0) throw std::runtime_error("Bad expectation from '" + std::string(commands::names[type_index]) + "' function");
      
      // тут пока что я не понимаю чего ожидать, но по идее хук_айди, хук_тайп, таргет
      if (data.get_type() != sol::type::string && data.get_type() != sol::type::table) throw std::runtime_error("Bad data for '" + std::string(commands::names[type_index]) + "' function");
      
      assert(false);
      return std::make_tuple(nullptr, 0, 0);
    }
    
    std::tuple<interface*, size_t, size_t> add_trait_init(const input_data &input, const sol::object &data, container* cont) {
      using type = script::add_trait;
      const size_t type_index = type::type_index;
      if ((input.current & type::context_types) == 0) throw std::runtime_error("Bad context for '" + std::string(commands::names[type_index]) + "' function");
      if (input.expected_types != 0) throw std::runtime_error("Bad expectation from '" + std::string(commands::names[type_index]) + "' function");
      
      if (data.get_type() != sol::type::string && data.get_type() != sol::type::table) throw std::runtime_error("Bad data for '" + std::string(commands::names[type_index]) + "' function");
      
      // тут я ожидаю энтити, чаще всего он к нам приходит с помощью строки
      // может ли он быть в контексте? не должен по идее
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<type>();
      size_t final_count = 1;
      size_t final_size = sizeof(type);
      
      input_data new_input = input;
      new_input.script_type = script_types::string;
      const auto [ptr, count, size] = string_container_init(new_input, data, cont);
      final_count += count;
      final_size += size;
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        auto get_ent = cont->add<script::get_trait>(ptr);
        auto init = cont->get_init<type>(offset);
        final_ptr = init.init(get_ent);
      }
      
      final_count += 2;
      final_size += sizeof(script::get_trait) + sizeof(type);
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
    static std::tuple<interface*, size_t, size_t> get_object_interface_functions(const input_data &input, const sol::object &data, container* cont, const size_t &expected_types) {
      if (data.get_type() != sol::type::string) throw std::runtime_error("Character can be found only in script context");
      
      input_data new_input = input;
      new_input.script_type = script_types::object;
      new_input.expected_types = expected_types;
      const auto str = data.as<std::string_view>();
      if (str == commands::names[commands::root]) return root_init(new_input, sol::nil, cont);
      else if (str == commands::names[commands::prev]) return prev_init(new_input, sol::nil, cont);
      
      return complex_object_init(new_input, data, cont);
    }
    
    template <typename T>
    static std::tuple<interface*, size_t, size_t> get_object_with_id_interface_functions(const input_data &input, const sol::object &data, container* cont, const size_t &expected_types) {
      if (data.get_type() != sol::type::string || data.get_type() != sol::type::table) throw std::runtime_error("Table or string is needed");
      
      size_t final_count = 1;
      size_t final_size = sizeof(T);
      interface* str_ptr = nullptr;
      if (data.get_type() == sol::type::string) {
        input_data new_input = input;
        new_input.script_type = script_types::object;
        new_input.expected_types = expected_types;
        const auto str = data.as<std::string_view>();
        const bool has_dot = str.find('.') != std::string_view::npos;
        const bool has_colon = str.find(':') != std::string_view::npos;
        if (has_dot || has_colon) {
          return complex_object_init(new_input, data, cont);
        }
        
        if (str == commands::names[commands::root]) return root_init(new_input, sol::nil, cont);
        else if (str == commands::names[commands::prev]) return prev_init(new_input, sol::nil, cont);
        
        str_ptr = cont->add<script::string_container>(str);
      } else {
        input_data new_input = input;
        new_input.script_type = script_types::string;
        
        const auto [ptr, count, size] = string_container_init(new_input, data, cont);
        str_ptr = ptr;
        final_count += count;
        final_size += size;
      }
      
      auto final_ptr = cont->add<T>(str_ptr);
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
    std::tuple<interface*, size_t, size_t> marry_init(const input_data &input, const sol::object &data, container* cont) {
      using type = script::marry;
      const size_t type_index = type::type_index;
      if ((input.current & type::context_types) == 0) throw std::runtime_error("Bad context for '" + std::string(commands::names[type_index]) + "' function");
      if (input.expected_types != 0) throw std::runtime_error("Bad expectation from '" + std::string(commands::names[type_index]) + "' function");

      if (data.get_type() != sol::type::string) throw std::runtime_error("Bad data for '" + std::string(commands::names[type_index]) + "' function");
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<type>();
      size_t final_count = 1;
      size_t final_size = sizeof(type);
      
      // тут я ожидаю персонажа: он может быть из контекста, может быть прев или рут, еще что нибудь?
      // все это строки, может ли тут быть что то кроме строки? сильно сомневаюсь, причем тут строго строка
      
      input_data new_input = input;
      new_input.script_type = script_types::object;
      const auto [char_ptr, count, size] = get_object_interface_functions(new_input, data, cont, script::object::type_bit::character);
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
      if (input.expected_types != 0) throw std::runtime_error("Bad expectation from '" + std::string(commands::names[type_index]) + "' function");
      
      // тут я ожидаю что угодно, но в контексте должен быть персонаж
      
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
      if (input.expected_types != 0) throw std::runtime_error("Bad expectation from '" + std::string(commands::names[type_index]) + "' function");
      
      if (data.get_type() != sol::type::table) throw std::runtime_error("Bad data for '" + std::string(commands::names[type_index]) + "' function");
      
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
        const auto [ptr, count, size] = get_object_interface_functions(input, table["target"], cont, script::object::type_bit::character);
        target = ptr;
        final_count += count;
        final_size += size;
      }
      
      interface* claimant = nullptr;
      {
        const auto [ptr, count, size] = get_object_interface_functions(input, table["claimant"], cont, script::object::type_bit::character);
        claimant = ptr;
        final_count += count;
        final_size += size;
      }
      
      interface* casus_belli = nullptr;
      {
        sol::object proxy = table["casus_belli"];
        if (!proxy.valid()) proxy = table["cb"];
        const auto [ptr, count, size] = get_object_with_id_interface_functions<script::get_casus_belli>(input, proxy, cont, script::object::type_bit::casus_belli);
        casus_belli = ptr;
        final_count += count;
        final_size += size;
      }
      
      const auto proxy = table["titles"];
      if (!proxy.valid() || proxy.get_type() != sol::type::table) throw std::runtime_error("Could not find titles table in start_war data");
      
      const auto titles_table = proxy.get<sol::table>();
      interface* titles = nullptr;
      for (const auto &pair : titles_table) {
        if (pair.first.get_type() != sol::type::number) continue;
        const auto [ptr, count, size] = get_object_with_id_interface_functions<script::get_titulus>(input, proxy, cont, script::object::type_bit::titulus);
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
      if (input.expected_types != 0) throw std::runtime_error("Bad expectation from '" + std::string(commands::names[type_index]) + "' function");
      
      if (data.get_type() != sol::type::table) throw std::runtime_error("Bad data for '" + std::string(commands::names[type_index]) + "' function");
      
      // тут я ожидаю что в данных будет указана строка не персонажа
      // или таблица в которой будет указан таргет и возможно реалм
      // а мне еще нужно указать причину !!! тут не может быть просто строки
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<type>();
      size_t final_count = 1;
      size_t final_size = sizeof(type);
      
      // тут должен быть указан таргет и может быть указан реалм
      const auto table = data.as<sol::table>();
      interface* target = nullptr;
      {
        const auto [ptr, count, size] = get_object_interface_functions(input, table["target"], cont, script::object::type_bit::character);
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
      if ((input.expected_types & T::output_type) == 0) throw std::runtime_error("Bad expectation from '" + std::string(commands::names[T::type_index]) + "' function");
      
      interface* ptr = nullptr;
      if (cont != nullptr) ptr = cont->add<T>();
      size_t final_count = 1;
      size_t final_size = sizeof(T);
      
      input_data new_input = input;
      new_input.script_type = script_types::numeric;
      new_input.expected_types = object::type_bit::valid_number;
      
      if (data.valid() && data.get_type() == sol::type::string && data.as<std::string_view>() == commands::names[T::type_index]) {
        return std::make_tuple(ptr, script::object::type_bit::valid_number, final_size);
      }
      
      if (!data.valid() || (data.valid() && data.get_type() == sol::type::nil)) {
        // примерно тоже самое что и для сложного числа
        return std::make_tuple(ptr, final_count, final_size);
      }
      
      // тут нам нужно собрать сравнение
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<script::boolean_comparator>();
      final_count += 1;
      final_size += sizeof(script::boolean_comparator);
      
      interface* rvalue = nullptr;
      if (data.get_type() == sol::type::string) {
        const auto [cptr, count, size] = complex_object_init(new_input, data, cont, ptr);
        rvalue = cptr;
        final_count += count;
        final_size += size;
      } else if (data.get_type() == sol::type::boolean || data.get_type() == sol::type::number || data.get_type() == sol::type::table)  {
        const auto [ptr, count, size] = number_container_init(new_input, data, cont);
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
    
    template <typename T, typename GET_OBJ>
    static std::tuple<interface*, size_t, size_t> character_condition_object_check_function_init(const input_data &input, const sol::object &data, container* cont, const size_t &expected_types) {
      if ((input.current & T::context_types) == 0) throw std::runtime_error("Bad context for '" + std::string(commands::names[T::type_index]) + "' function");
      if ((input.expected_types & T::output_type) == 0) throw std::runtime_error("Bad expectation from '" + std::string(commands::names[T::type_index]) + "' function");
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<T>();
      size_t final_count = 1;
      size_t final_size = sizeof(T);
      
      input_data new_input = input;
      new_input.script_type = script_types::object;
      new_input.expected_types = expected_types;
      
      if (data.valid() && data.get_type() == sol::type::string && data.as<std::string_view>() == commands::names[T::type_index]) {
        throw std::runtime_error("Function '" + std::string(commands::names[T::type_index]) + "' can not be used in complex function");
      }
      
      if (!data.valid() || (data.valid() && data.get_type() == sol::type::nil)) {
        throw std::runtime_error("Function '" + std::string(commands::names[T::type_index]) + "' demands input");
      }
      
      // тут мы ожидаем либо какой то объект, либо строку по которой мы можем получить объект
      // тут наверное стоит ожидать те объекты которые мы в принципе не можем получить через контекст
      // не, наверное лучше любые, но тут нужно разделить как то комплекс и просто строку с просто функциями
      
      interface* value_ptr = nullptr;
      if (data.get_type() == sol::type::string) {
        // тут может быть два типа строк: сложный объект и просто строка, 
        // причем некоторые функции без символов "." и ":" являются валидными сложными объектами, но для них не нужно этот контейнер создавать
        const auto str = data.as<std::string_view>();
        const bool has_dot = str.find('.') != std::string_view::npos;
        const bool has_colon = str.find(':') != std::string_view::npos;
        if (has_dot || has_colon) {
          const auto [ptr, count, size] = complex_object_init(new_input, data, cont);
          value_ptr = ptr;
          final_count += count;
          final_size += size;
        } else {
          const auto root_str = commands::names[commands::root];
          const auto prev_str = commands::names[commands::prev];
          if (str == root_str) {
            const auto [ptr, count, size] = root_init(new_input, data, cont);
            value_ptr = ptr;
            final_count += count;
            final_size += size;
          } else if (str == prev_str) {
            const auto [ptr, count, size] = prev_init(new_input, data, cont);
            value_ptr = ptr;
            final_count += count;
            final_size += size;
          } else {
            // просто строка
            if (cont != nullptr) {
              auto str_container = cont->add<script::string_container>(str);
              value_ptr = cont->add<GET_OBJ>(str_container);
            }
            final_count += 2;
            final_size += sizeof(script::string_container) + sizeof(GET_OBJ);
          }
        }
      } else if (data.get_type() == sol::type::table) {
        new_input.script_type = script_types::string;
        const auto [ptr, count, size] = string_table_init(new_input, data, cont);
        if (cont != nullptr) {
          value_ptr = cont->add<GET_OBJ>(ptr);
        }
        final_count += count + 1;
        final_size += size + sizeof(GET_OBJ);
      } else throw std::runtime_error("Ivalid data type for '" + std::string(commands::names[T::type_index]) + "' function");
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        auto init = cont->get_init<T>(offset);
        final_ptr = init.init(value_ptr);
      }
      
      final_count += 1;
      final_size += sizeof(T);
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
    template <typename T>
    static std::tuple<interface*, size_t, size_t> character_condition_string_check_function_init(const input_data &input, const sol::object &data, container* cont) {
      if ((input.current & T::context_types) == 0) throw std::runtime_error("Bad context for '" + std::string(commands::names[T::type_index]) + "' function");
      if ((input.expected_types & T::output_type) == 0) throw std::runtime_error("Bad expectation from '" + std::string(commands::names[T::type_index]) + "' function");
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<T>();
      size_t final_count = 1;
      size_t final_size = sizeof(T);
      
      input_data new_input = input;
      new_input.script_type = script_types::string;
      new_input.expected_types = object::type_bit::string;
      
      if (data.valid() && data.get_type() == sol::type::string && data.as<std::string_view>() == commands::names[T::type_index]) {
        throw std::runtime_error("Function '" + std::string(commands::names[T::type_index]) + "' can not be used in complex function");
      }
      
      if (!data.valid() || (data.valid() && data.get_type() == sol::type::nil)) {
        throw std::runtime_error("Function '" + std::string(commands::names[T::type_index]) + "' demands input");
      }
      
      interface* value_ptr = nullptr;
      if (data.get_type() == sol::type::string) {
        const auto [ptr, count, size] = string_container_init(new_input, data, cont);
        value_ptr = ptr;
        final_count += count;
        final_size += size;
      } else if (data.get_type() == sol::type::table) {
        const auto [ptr, count, size] = string_table_init(new_input, data, cont);
        value_ptr = ptr;
        final_count += count;
        final_size += size;
      } else throw std::runtime_error("Ivalid data type for '" + std::string(commands::names[T::type_index]) + "' function");
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        auto init = cont->get_init<T>(offset);
        final_ptr = init.init(value_ptr);
      }
      
      final_count += 1;
      final_size += sizeof(T);
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
    // так теперь бесконечное количество статов и условий
    // большая часть из этих функций следует одному паттерну
    template <typename T>
    static std::tuple<interface*, size_t, size_t> stat_function_init(const input_data &input, const sol::object &data, container* cont) {
      if ((input.current & T::context_types) == 0) throw std::runtime_error("Bad context for '" + std::string(commands::names[T::type_index]) + "' function");
      if ((input.expected_types & T::output_type) == 0) throw std::runtime_error("Bad expectation from '" + std::string(commands::names[T::type_index]) + "' function");
      
      interface* ptr = nullptr;
      if (cont != nullptr) ptr = cont->add<T>();
      size_t final_count = 1;
      size_t final_size = sizeof(T);
      
      input_data new_input = input;
      new_input.script_type = script_types::numeric;
      new_input.expected_types = object::type_bit::valid_number;
      
      if (data.valid() && data.get_type() == sol::type::string && data.as<std::string_view>() == commands::names[T::type_index]) {
        return std::make_tuple(ptr, script::object::type_bit::valid_number, final_size);
      }
      
      if (!data.valid() || (data.valid() && data.get_type() == sol::type::nil)) {
        // примерно тоже самое что и для сложного числа
        return std::make_tuple(ptr, final_count, final_size);
      }
      
      // тут нам нужно собрать сравнение
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<script::number_comparator>();
      final_count += 1;
      final_size += sizeof(script::number_comparator);
      
      interface* rvalue = nullptr;
      if (data.get_type() == sol::type::string) {
        const auto [cptr, count, size] = complex_object_init(new_input, data, cont, ptr);
        rvalue = cptr;
        final_count += count;
        final_size += size;
      } else if (data.get_type() == sol::type::boolean || data.get_type() == sol::type::number || data.get_type() == sol::type::table)  {
        const auto [ptr, count, size] = number_container_init(new_input, data, cont);
        rvalue = ptr;
        final_count += count;
        final_size += size;
      } else throw std::runtime_error("Ivalid data type for '" + std::string(commands::names[T::type_index]) + "' function");
      
      interface* final_ptr = nullptr;
      if (cont != nullptr) {
        auto init = cont->get_init<script::number_comparator>(offset);
        final_ptr = init.init(compare_operators::more_eq, ptr, rvalue);
      }
      
      return std::make_tuple(final_ptr, final_count, final_size);
    }
    
    template <typename T>
    static std::tuple<interface*, size_t, size_t> add_stat_function_init(const input_data &input, const sol::object &data, container* cont) {
      if ((input.current & T::context_types) == 0) throw std::runtime_error("Bad context for '" + std::string(commands::names[T::type_index]) + "' function");
      if ((input.expected_types & T::output_type) == 0) throw std::runtime_error("Bad expectation from '" + std::string(commands::names[T::type_index]) + "' function");
      
      if (data.valid() && data.get_type() == sol::type::string && data.as<std::string_view>() == commands::names[T::type_index]) {
        throw std::runtime_error("Trying to create function '" + std::string(commands::names[T::type_index]) + "' in complex script");
      }
      
      if (!data.valid() || (data.valid() && data.get_type() == sol::type::nil)) {
        throw std::runtime_error("Function '" + std::string(commands::names[T::type_index]) + "' demands input value");
      }
      
      size_t offset = SIZE_MAX;
      if (cont != nullptr) offset = cont->add_delayed<T>();
      size_t final_count = 1;
      size_t final_size = sizeof(T);
      
      input_data new_input = input;
      new_input.script_type = script_types::numeric;
      new_input.expected_types = object::type_bit::valid_number;
      
      interface* rvalue = nullptr;
      if (data.get_type() == sol::type::string) {
        const auto [ptr, count, size] = complex_object_init(new_input, data, cont);
        rvalue = ptr;
        final_count += count;
        final_size += size;
      } else if (data.get_type() == sol::type::table) {
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
    }
    
    CONDITION_COMMANDS_LIST

#undef CONDITION_COMMAND_FUNC

#define CONDITION_COMMAND_FUNC(name, get_obj, expected_type_bits) \
    std::tuple<interface*, size_t, size_t> name##_init(const input_data &input, const sol::object &data, container* cont) { \
      if (input.script_type != script_types::condition && input.script_type != script_types::numeric)                       \
        throw std::runtime_error("Trying to create function '"#name"' in wrong script type");                               \
      return character_condition_object_check_function_init<script::name, script::get_obj>(input, data, cont, expected_type_bits); \
    }

CONDITION_COMMAND_FUNC(has_culture, get_culture, object::type_bit::culture)
CONDITION_COMMAND_FUNC(has_culture_group, get_culture_group, object::type_bit::culture_group)
CONDITION_COMMAND_FUNC(has_religion, get_religion, object::type_bit::religion)
CONDITION_COMMAND_FUNC(has_religion_group, get_religion_group, object::type_bit::religion_group)
CONDITION_COMMAND_FUNC(has_trait, get_trait, object::type_bit::trait)
CONDITION_COMMAND_FUNC(has_modificator, get_modificator, object::type_bit::modificator)
CONDITION_COMMAND_FUNC(has_title, get_titulus, object::type_bit::titulus)

#undef CONDITION_COMMAND_FUNC

#define CONDITION_COMMAND_FUNC(name) \
    std::tuple<interface*, size_t, size_t> name##_init(const input_data &input, const sol::object &data, container* cont) { \
      if (input.script_type != script_types::condition && input.script_type != script_types::numeric)                       \
        throw std::runtime_error("Trying to create function '"#name"' in wrong script type");                               \
      return character_condition_string_check_function_init<script::name>(input, data, cont);                               \
    }
    
CONDITION_COMMAND_FUNC(has_flag)

#undef CONDITION_COMMAND_FUNC
    
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
    }
#define CHARACTER_PENALTY_STAT_FUNC(name) STAT_FUNC(name##_penalty)

    UNIQUE_STATS_LIST
    
#undef CHARACTER_PENALTY_STAT_FUNC
#undef STAT_FUNC

#define STAT_FUNC(name) \
    std::tuple<interface*, size_t, size_t> name##_init(const input_data &input, const sol::object &data, container* cont) { \
      if (input.script_type != script_types::condition && input.script_type != script_types::numeric)                       \
        throw std::runtime_error("Trying to create function '"#name"' in wrong script type");                               \
      return stat_function_init<script::name>(input, data, cont);                                                           \
    }
    
    UNIQUE_RESOURCES_LIST
    
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
    
    const std::string_view number_matcher = "[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)";
    const std::string_view dot_matcher = ".";
    const std::string_view colon_matcher = ":";
  }
}
