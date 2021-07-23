#include "utility.h"

#include <charconv>
#include "script_header.h"
#include "re2/re2.h"
#include "core/structures_header.h"
#include "utils/constexpr_funcs.h"
#include "utils/magic_enum_header.h"
#include "utils/globals.h"
#include "utils/systems.h"
#include "core/context.h"
#include "lua_data_struct.h"

static const std::string_view str_start_array[] = {
  "context"
};

static const size_t str_start_array_size = sizeof(str_start_array) / sizeof(str_start_array[0]);
static_assert(str_start_array_size == 1);

namespace devils_engine {
  namespace script {
    static const RE2 regex_obj(complex_var_regex);
    static const RE2 regex_dot_matcher(dot_matcher);
    static const RE2 regex_colon_matcher(colon_matcher);
    static const RE2 regex_number_matcher(number_matcher);
    
//     size_t divide_token(const std::string_view &str, const std::string_view &symbol, const size_t &max_count, std::string_view* array) {
//       size_t current = 0;
//       size_t prev = 0;
//       size_t counter = 0;
//       const size_t symbols_count = symbol.length();
//       while (current != std::string_view::npos && counter < max_count) {
//         current = str.find(symbol, prev);
//         const auto part = str.substr(prev, current-prev);
//         array[counter] = part;
//         ++counter;
//         prev = current + symbols_count;
//       }
//       
//       counter = counter < max_count ? counter : SIZE_MAX;
//       
//       return counter;
//     }
    
    bool check_target(const target_t &t) {
      auto core_ctx = global::get<systems::map_t>()->core_context;
      bool ret = true;
      switch (static_cast<core::structure>(t.type)) {
        case core::structure::realm: {
          auto ptr = core_ctx->get_realm(t.token);
          ret = t.data != nullptr && ptr == t.data;
          break;
        }
        
        case core::structure::army: {
          auto ptr = core_ctx->get_army(t.token);
          ret = t.data != nullptr && ptr == t.data;
          break;
        }
        
        case core::structure::hero_troop: {
          auto ptr = core_ctx->get_hero_troop(t.token);
          ret = t.data != nullptr && ptr == t.data;
          break;
        }
        
        case core::structure::war: {
          auto ptr = core_ctx->get_war(t.token);
          ret = t.data != nullptr && ptr == t.data;
          break;
        }
        
        default: break;
      }
      
      return ret;
    }
    
    char* copy_str_to_char_array(const std::string_view &str) {
      size_t size = str.size();
      size += size_t(str[str.size()-1] != '\0');
      char* mem = new char[size];
      memcpy(mem, str.data(), str.size());
      if (str[str.size()-1] != '\0') mem[str.size()] = '\0';
      return mem;
    }
    
    void setup_script_array(std::vector<script_data> &datas, script_data* data) {
      data->value = 0;
      if (datas.empty()) return;
      
      data->value = datas.size();
      auto ptr = new script_data[datas.size()];
      for (size_t i = 0; i < datas.size(); ++i) {
        ptr[i] = std::move(datas[i]);
      }
      data->data = ptr;
    }
    
    void make_lvalue(const std::string_view &str, const bool has_colon, script_data* data) {
      std::vector<script_data> datas;
      
      std::string_view final_str = str;
      if (has_colon) {
        // здесь нужен только объект, а значит нужно проверить только context
        // у нас есть возможность получить какие то объекты по id, но это не те объекты которые могут оказаться в скрипте
        // хотя может быть потребность в том чтобы взять переменную с таких объектов как: казус белли
        // неизвестно пока что зачем.....
        
        datas.emplace_back();
        auto data = &datas.back();
        data->command_type = command_type::rvalue;
        const size_t reserved_str_count = 256;
        std::array<std::string_view, reserved_str_count> array = {};
        const size_t count = divide_token(str, ":", reserved_str_count, array.data());
        assert(count != SIZE_MAX);
        
        const size_t point_start = final_str.find('.');
        final_str = point_start == std::string_view::npos ? "" : final_str.substr(point_start+1);
        
        data_source_type::values source;
        double num;
        uint8_t compare_type;
        std::string_view context_id;
        
        // что делать если count == 1? 
        for (size_t i = 0; i < count; ++i) {
          // проверяем разные токены
          const auto current = array[i];
          if (current.size() == 0) continue;
          
          if (const auto e = magic_enum::enum_cast<number_compare_type::values>(current); e) {
            compare_type = e.value();
            continue;
          }
          
          if (const auto e = magic_enum::enum_cast<data_source_type::values>(current); e) {
            source = e.value();
            continue;
          }
          
          // еще может быть число, нужно проверить реджекс
          if (RE2::FullMatch(current, regex_number_matcher)) {
            if (i == 0) continue; // первый числовой токен - это уникальное число для того чтобы луа не ругался
            
            const auto res = std::from_chars(current.begin(), current.end(), num);
            if (res.ec == std::errc::invalid_argument || res.ec == std::errc::result_out_of_range) {
              throw std::runtime_error("Could not parse number " + std::string(current));
            }
            
            continue;
          }
          
          if (!context_id.empty()) throw std::runtime_error("Can game find two id for context?");
          context_id = current;
        }
        
//         if (data->command_type == command_type::action && compare_type != number_compare_type::equal) 
//           throw std::runtime_error("Bad data type '" + std::string(magic_enum::enum_name(static_cast<number_compare_type::value>(compare_type))) + "' in add_money");
        
        switch (source) {
          case data_source_type::special: {
            // ???
            assert(false);
            break;
          }
          
          case data_source_type::context: {
            // в type2 должен хранится либо id либо индекс (или индекс распарсится в num?)
            data->number_type = number_type::get_scope;
            // нужно избавиться от индексов
//             if (context_id == "index") data->value = size_t(num);
//             else 
            data->data = copy_str_to_char_array(context_id);
            break;
          }
          
          case data_source_type::value: {
            data->number_type = number_type::number;
            data->value = num;
            data->compare_type = compare_type;
            break;
          }
          
#define DATA_SOURCE_CASE(name) case data_source_type::name: { \
  data->number_type = number_type::stat; \
  data->compare_type = compare_type; \
  data->value = num; \
  data->helper2 = data_source_type::name; \
  break; \
}
// в хелпере должен видимо храниться data_source_type

          // заменим потом лист
          DATA_SOURCE_CASE(money)
          DATA_SOURCE_CASE(money_income)
          DATA_SOURCE_CASE(money_month_income)
          DATA_SOURCE_CASE(money_yearly_income)
          
          default: throw std::runtime_error(std::string(magic_enum::enum_name(source)) + " not implemented yet");
        }
        
      }
      
      if (final_str.empty()) return;
      
      const size_t reserved_str_count = 256;
      std::array<std::string_view, reserved_str_count> array = {};
      const size_t count = divide_token(final_str, ".", reserved_str_count, array.data());
      assert(count != SIZE_MAX);
      for (size_t i = 0; i < count; ++i) {
        const auto tmp_str = array[i];
        // по tmp_str находим lvalue функцию, какую? это объектная функция, ставит в контекст либо таргет либо значение
        // было бы неплохо использовать частично что у меня уже есть например функцию self_realm
        // для этого self_realm должен задавать текущую переменную как собственно реалм, и выходить если это конец lvalue
        // каждый последующий вызов принимает на вход предыдущий таргет
        if (const auto e = magic_enum::enum_cast<condition_function::values>(tmp_str); e) {
          const size_t index = e.value();
          datas.emplace_back();
          datas.back().command_type = command_type::condition; // тут нужно указать lvalue, чтобы избежать лишних проверок
          datas.back().helper1 = index;
          datas.back().number_type = number_type::lvalue;
          continue;
        }
        
        if (const auto e = magic_enum::enum_cast<action_function::values>(tmp_str); e) {
          const size_t index = e.value();
          datas.emplace_back();
          datas.back().command_type = command_type::action; // тут нужно указать lvalue, чтобы избежать лишних проверок
          datas.back().helper1 = index;
          datas.back().number_type = number_type::lvalue;
          continue;
        }
        
        if (const auto e = magic_enum::enum_cast<condition_block_function::values>(tmp_str); e) {
          const size_t index = e.value();
          datas.emplace_back();
          datas.back().command_type = command_type::condition_script_block; // тут нужно указать lvalue, чтобы избежать лишних проверок
          datas.back().helper1 = index;
          datas.back().number_type = number_type::lvalue;
          continue;
        }
        
        if (const auto e = magic_enum::enum_cast<action_block_function::values>(tmp_str); e) {
          const size_t index = e.value();
          datas.emplace_back();
          datas.back().command_type = command_type::action_script_block; // тут нужно указать lvalue, чтобы избежать лишних проверок
          datas.back().helper1 = index;
          datas.back().number_type = number_type::lvalue;
          continue;
        }
        
        if (const auto e = magic_enum::enum_cast<general_block_function::values>(tmp_str); e) {
          const size_t index = e.value();
          datas.emplace_back();
          datas.back().command_type = command_type::general_script_block; // тут нужно указать lvalue, чтобы избежать лишних проверок
          datas.back().helper1 = index;
          datas.back().number_type = number_type::lvalue; 
          continue;
        }
        
        throw std::runtime_error("Could not parse token " + std::string(tmp_str) + " for lvalue");
      }
      
      //data->command_type = command_type::object_function;
      data->number_type = number_type::lvalue_array;
      data->value = datas.size();
      auto array_ptr = new script_data[datas.size()];
      for (size_t i = 0; i < datas.size(); ++i) {
        array_ptr[i] = std::move(datas[i]);
      }
      data->data = array_ptr;
    }
    
    void get_lvalue(context* ctx, const script_data* lvalue_array, script_data* value) {
      assert(value->command_type == command_type::invalid);
      assert(value->number_type == number_type::object);
      assert(lvalue_array->number_type == number_type::lvalue_array);
      const size_t size = lvalue_array->value;
      auto commands = reinterpret_cast<const script_data*>(lvalue_array->data);
      // значение сохраняется в контекст, а значит что он должен быть чистым
      script_data mem = std::move(ctx->current_value);
      for (size_t i = 0; i < size; ++i) {
        assert(commands[i].number_type == number_type::lvalue);
        assert(value->number_type == number_type::object);
        const uint32_t index = commands[i].helper1;
        target_t t(value->helper2, value->data, cast_to_int64(value->value));
        // тут возможна ситуция когда у нас отсутствует таргет
        // мы можем получить его в одной из функций (this, prev, root и проч)
        
        switch (commands[i].command_type) {
          case command_type::rvalue: {
            // тут нужно получить объект из скоупа и что вызвать? ничего не надо, только таргет получить
            // как получить тут таргет? так как это лвалуе, то мы работаем со строкой и тут должен быть 
            // id объекта в скоупе, тип "context:attacker"
            assert(commands[i].number_type == number_type::get_scope);
            const auto raw_str = reinterpret_cast<const char*>(commands[i].data);
            const std::string_view str = raw_str;
            auto itr = ctx->map_data.find(str);
            if (itr == ctx->map_data.end()) throw std::runtime_error("Could not find context object " + std::string(str));
            const auto &val = itr->second;
            if (val.number_type != number_type::object) throw std::runtime_error(std::string(str) + " is not an object");
            assert(val.helper2 != UINT16_MAX);
            assert(val.data != nullptr);
            // надо по дефолту ставить значение в велью SIZE_MAX
            t = target_t(val.helper2, val.data, cast_to_int64(val.value));
            break;
          }
          
          case command_type::condition: {
            const auto func = condition_functions[index];
            func(t, ctx, 1, &commands[i]);
            break;
          }
          
          case command_type::condition_script_block: {
            const auto func = condition_block_functions[index];
            func(t, ctx, 1, &commands[i]);
            break;
          }
          
          case command_type::action: {
            const auto func = action_functions[index];
            func(t, ctx, 1, &commands[i]);
            break;
          }
          
          case command_type::action_script_block: {
            const auto func = action_block_functions[index];
            func(t, ctx, 1, &commands[i]);
            break;
          }
          
          case command_type::general_script_block: {
            const auto func = general_block_functions[index];
            func(t, ctx, 1, &commands[i], 0);
            break;
          }
        }
        
        *value = std::move(ctx->current_value);
      }
      
      ctx->current_value = std::move(mem);
    }
    
    void make_complex_value(const uint32_t &target_type, const sol::table &table, script_data* data, const uint32_t &expected_type) {
      std::vector<script_data> datas;
      
      // по большому счету неважно какой ключ, но если нужна последовательность, то нужно число
      // для каждой таблицы мы сначала ищем ключи условий, а потом value, factor или string
      for (const auto &pair : table) {
        if (pair.first.get_type() == sol::type::string) {
          const auto str = pair.first.as<std::string_view>();
          if (const auto e = magic_enum::enum_cast<condition_block_function::values>(str); e) {
            const uint32_t index = e.value();
            datas.emplace_back();
            const auto func = condition_block_init_functions[index];
            func(target_type, pair.second, &datas.back());
          }
          
          if (const auto e = magic_enum::enum_cast<general_block_function::values>(str); e) {
            const uint32_t index = e.value();
            datas.emplace_back();
            const auto func = general_block_init_functions[index];
            func(target_type, pair.second, &datas.back(), CONDITION);
          }
          
          if (const auto e = magic_enum::enum_cast<condition_function::values>(str); e) {
            const uint32_t index = e.value();
            datas.emplace_back();
            const auto func = condition_init_functions[index];
            func(target_type, pair.second, &datas.back());
          }
          
          // несколько токенов сюда не подходят, но они валидные
        }
        
        // массивы проверим дальше
      }
      
      // factor должен быть перед add, а value должно быть по идее самым первым
      const auto value_str = magic_enum::enum_name(action_function::value);
      for (const auto &pair : table) {
        if (pair.first.get_type() == sol::type::string) {
          const auto str = pair.first.as<std::string_view>();
          if (str == value_str) {
            if (expected_type != number_type::number) throw std::runtime_error("'value' token is not allowed here");
            const auto func = action_init_functions[action_function::value];
            func(target_type, pair.second, &datas.back());
            break;
          }
        }
      }
      
      const auto factor_str = magic_enum::enum_name(action_function::factor);
      for (const auto &pair : table) {
        if (pair.first.get_type() == sol::type::string) {
          const auto str = pair.first.as<std::string_view>();
          if (str == factor_str) {
            if (expected_type != number_type::number) throw std::runtime_error("'factor' token is not allowed here");
            const auto func = action_init_functions[action_function::factor];
            func(target_type, pair.second, &datas.back());
            break;
          }
        }
      }
      
      for (const auto &pair : table) {
        if (pair.first.get_type() == sol::type::string) {
          const auto str = pair.first.as<std::string_view>();
          if (const auto e = magic_enum::enum_cast<action_function::values>(str); e) {
            const uint32_t index = e.value();
            datas.emplace_back();
            switch (index) {
              case action_function::value: break;
              case action_function::factor: break;
              case action_function::add: {
                if (expected_type != number_type::number) throw std::runtime_error("'add' token is not allowed here");
                
                const auto func = action_init_functions[index];
                func(target_type, pair.second, &datas.back());
                break;
              }
              case action_function::string: {
                if (expected_type != number_type::string) throw std::runtime_error("'string' token is not allowed here");
                
                const auto func = action_init_functions[index];
                func(target_type, pair.second, &datas.back());
                break;
              }
              case action_function::object: {
                if (expected_type != number_type::object) throw std::runtime_error("'object' token is not allowed here");
                const auto func = action_init_functions[index];
                func(target_type, pair.second, &datas.back());
                break;
              }
              
              default: throw std::runtime_error("Effect token '" + std::string(str) + "' is not alowed in complex variable");
            }
            
            continue;
          }
          
          throw std::runtime_error("Missed token '" + std::string(str) + "'");
        }
      }
      
      // где проверять оставшиеся таблицы? скорее всего тут
      for (const auto &pair : table) {
        if (pair.first.get_type() == sol::type::number) {
          if (pair.second.get_type() != sol::type::table) throw std::runtime_error("Expected table in array part");
          
          const sol::table t = pair.second.as<sol::table>();
          datas.emplace_back();
          make_complex_value(target_type, t, &datas.back(), expected_type);
          continue;
        }
        
        throw std::runtime_error("Bad table key type");
      }
      
      // мы должны указать что это комлекс переменная, но так чтобы у нас не заместилось предыдущее значение
      // а там должен быть указан тип функции, указать в хелпере?
      //data->command_type = command_type::complex_variable;
      data->number_type = number_type::array;
      setup_script_array(datas, data);
    }
    
    void complex_value_func(const target_t t, context* ctx, const script_data* complex_array) {
      assert(complex_array->number_type == number_type::array);
      const size_t size = complex_array->value;
      auto commands = reinterpret_cast<const script_data*>(complex_array->data);
      bool basic_check = false;
      for (size_t i = 0; i < size; ++i) {
        // должны быть дефолтные входные данные
        //assert(commands[i].number_type == number_type::lvalue);
        const uint32_t index = commands[i].helper1;
        
        switch (commands[i].command_type) {
          case command_type::condition: {
            const auto func = condition_functions[index];
            const bool ret = func(t, ctx, 1, &commands[i]);
            assert(!basic_check);
            if (!ret) return;
            break;
          }
          
          case command_type::condition_script_block: {
            const auto func = condition_block_functions[index];
            const int32_t ret = func(t, ctx, 1, &commands[i]);
            assert(!basic_check);
            if (ret == FALSE_BLOCK) return;
            break;
          }
          
          case command_type::general_script_block: {
            const auto func = general_block_functions[index];
            const int32_t ret = func(t, ctx, 1, &commands[i], CONDITION);
            assert(!basic_check);
            if (ret == FALSE_BLOCK) return;
            break;
          }
          
          // эффекты должны быть строго после всех кондишенов, как проверить?
          // только определенные функции? да, стейт не должен меняться
          case command_type::action: {
            const auto func = action_functions[index];
            func(t, ctx, 1, &commands[i]);
            basic_check = true;
            break;
          }
          
          // эти функции недоступны
          case command_type::action_script_block: {
            assert(false);
            break;
          }
          
          // этой функции не будет, потому что сюда мы попадаем из get_complex_value, а туда когда пытаемся получить переменную
//           case command_type::complex_variable: {
//             assert(commands[i].number_type == number_type::array);
//             complex_value_func(t, ctx, &commands[i]);
//             break;
//           }
        }
      }
    }
    
    void get_complex_value(const target_t* t, context* ctx, const script_data* complex_array, script_data* value) {
      assert(value->command_type == command_type::invalid);
      assert(value->helper2 == UINT16_MAX);
      assert(value->data == nullptr);
      assert(value->value == 0);
      // в value должен храниться ожидаемый тип
      assert(value->number_type == number_type::object || 
             value->number_type == number_type::number || 
             value->number_type == number_type::string_view || 
             value->number_type == number_type::boolean);
      assert(complex_array->number_type == number_type::array);
      
      // значение сохраняется в контекст, а значит что он должен быть чистым
      script_data mem = std::move(ctx->current_value);
      //const uint32_t expected_type = value->number_type;
      ctx->current_value.number_type = value->number_type;
      ctx->current_value.compare_type = number_compare_type::more_eq;
      ctx->current_value.value = 0.0;
      ctx->current_value.data = nullptr;
      ctx->current_value.helper2 = UINT16_MAX;
      
      complex_value_func(*t, ctx, complex_array);
      
      *value = std::move(ctx->current_value);
      ctx->current_value = std::move(mem);
    }
    
    bool regex_match(const std::string_view &str, std::string &type1, std::string &type2, std::string &num_str, data_source_type::values &source, data_type::values &value_data, double &num) {
      const bool ret = RE2::FullMatch(str, regex_obj, &type1, &type2, &num_str);
      if (!ret) return false;
      const auto e1 = magic_enum::enum_cast<data_source_type::values>(type1);
      if (!e1) throw std::runtime_error("Could not parse string data type " + type1);
      source = e1.value();
      const auto e2 = magic_enum::enum_cast<data_type::values>(type2);
      if (!e2) throw std::runtime_error("Could not parse string data type " + type2);
      value_data = e2.value();
      
      assert(!num_str.empty());
      num = std::atof(num_str.c_str());
      
      return true;
    }
    
    void variable_input_init_raw(const std::string_view &func_name, const uint32_t &target_type, const sol::object &obj, script_data* data, const uint32_t &expected_type) {
      ASSERT(data != nullptr);
      switch (obj.get_type()) {
        case sol::type::number: {
          const double m = obj.as<double>();
          data->number_type = number_type::number;
          data->value = m;
          data->compare_type = number_compare_type::more_eq;
          break;
        }
        
        case sol::type::string: {
          // тут может быть как особый инком, так и взятие из контекста
          const auto str = obj.as<std::string_view>();
          
          const bool has_dot = RE2::PartialMatch(str, regex_dot_matcher);
          const bool has_colon = RE2::PartialMatch(str, regex_colon_matcher);
          
          if (has_dot || has_colon) {
            // нужно сюда добавить работу с data_source_type
            make_lvalue(str, has_colon, data); // тут бы тоже добавить ожидание типа
            break;
          }
          
#ifndef _NDEBUG
          {
            const std::string_view s = ":";
            assert(s.length() == 1);
          }
#endif
          break;
        }
        
        case sol::type::table: {
          // сложная переменная, нужно добавить ожидания типа, чтобы проверить валидность сложной переменной
          const sol::table t = obj.as<sol::table>();
          make_complex_value(target_type, t, data, expected_type);
          break;
        }
        
        default: throw std::runtime_error("Bad value type for " + std::string(func_name));
      }
      
      // добавление статов можно сделать унифицированно, но у нас куча функции в которых уникальный инпут
    }
    
    void boolean_input_init(const std::string_view &func_name, const uint32_t &target_type, const sol::object &obj, script_data* data) {
      ASSERT(data != nullptr);
      if (obj.get_type() == sol::type::boolean) {
        const bool m = obj.as<bool>();
        data->number_type = number_type::boolean;
        data->value = m;
        data->compare_type = number_compare_type::more_eq;
        return;
      }
      
      variable_input_init_raw(func_name, target_type, obj, data, number_type::boolean);
    }
    
    void variable_input_init(const std::string_view &func_name, const uint32_t &target_type, const sol::object &obj, script_data* data) {
      variable_input_init_raw(func_name, target_type, obj, data, number_type::number);
    }
    
    void string_input_init(const std::string_view &func_name, const uint32_t &target_type, const sol::object &obj, script_data* data) {
      ASSERT(data != nullptr);
      switch (obj.get_type()) {
        case sol::type::string: {
          const auto str = obj.as<std::string_view>();
          
          const bool has_colon = RE2::PartialMatch(str, regex_colon_matcher);
          const bool has_dot = RE2::PartialMatch(str, regex_dot_matcher);
          
          if (has_dot && has_colon) throw std::runtime_error("Invalid string input for " + std::string(func_name));
          
          if (has_colon) {
            const size_t reserved_str_count = 256;
            std::array<std::string_view, reserved_str_count> array = {};
            const size_t count = divide_token(str, ":", reserved_str_count, array.data());
            assert(count != SIZE_MAX);
            
            data_source_type::values source;
            double num;
            //uint8_t compare_type;
            std::string_view context_id;
            
            // что делать если count == 1? 
            for (size_t i = 0; i < count; ++i) {
              // проверяем разные токены
              const auto current = array[i];
              if (current.size() == 0) continue;
              
//               if (const auto e = magic_enum::enum_cast<number_compare_type::value>(current); e) {
//                 compare_type = e.value();
//                 continue;
//               }
              
              if (const auto e = magic_enum::enum_cast<data_source_type::values>(current); e) {
                source = e.value();
                continue;
              }
              
              // еще может быть число, нужно проверить реджекс
              if (RE2::FullMatch(current, regex_number_matcher)) {
                if (i == 0) continue; // первый числовой токен - это уникальное число для того чтобы луа не ругался
                
                const auto res = std::from_chars(current.begin(), current.end(), num);
                if (res.ec == std::errc::invalid_argument || res.ec == std::errc::result_out_of_range) {
                  throw std::runtime_error("Could not parse number " + std::string(current));
                }
                
                continue;
              }
              
              if (!context_id.empty()) throw std::runtime_error("Can game find two id for context?");
              context_id = current;
            }
            
              switch (source) {
                case data_source_type::special: {
                  // ???
                  assert(false);
                  break;
                }
                
                case data_source_type::context: {
                  data->number_type = number_type::get_scope;
                  data->data = copy_str_to_char_array(context_id);
                  break;
                }
                
                default: throw std::runtime_error(std::string(magic_enum::enum_name(source)) + " not implemented yet for string input");
              }
            
            break;
          }
          
          // если в строке нет символа ":", то мы считаем что это валидная строка
          data->number_type = number_type::string;
          data->data = copy_str_to_char_array(str);
          break;
        }
        
        case sol::type::table: {
          const sol::table t = obj.as<sol::table>();
          make_complex_value(target_type, t, data, number_type::string);
          break;
        }
        
        default: throw std::runtime_error("Bad string init value type for " + std::string(func_name));
      }
    }
    
    void target_input_init(const std::string_view &func_name, const uint32_t &target_type, const sol::object &obj, script_data* data) {
      ASSERT(data != nullptr);
      switch (obj.get_type()) {
        case sol::type::string: {
          // может быть как lvalue, так и просто id
          const auto str = obj.as<std::string_view>();
          const bool has_dot = RE2::PartialMatch(str, regex_dot_matcher);
          const bool has_colon = RE2::PartialMatch(str, regex_colon_matcher);
          const auto const_current = magic_enum::enum_name(general_block_function::current);
          const auto const_prev = magic_enum::enum_name(general_block_function::prev);
          const auto const_root = magic_enum::enum_name(general_block_function::root);
          const bool lone_func = str == const_current || str == const_prev || str == const_root;
          if (has_dot || lone_func) {
            make_lvalue(str, has_colon, data); // тут бы тоже добавить ожидание типа
            break;
          }
          
          break;
        }
        
        case sol::type::table: {
          const sol::table t = obj.as<sol::table>();
          make_complex_value(target_type, t, data, number_type::object);
          break;
        }
        
        default: throw std::runtime_error("Bad target init value type for " + std::string(func_name));
      }
    }
    
    static double get_value_from_special_character_data(const core::character* c, const uint16_t &type, const double &mult) {
      double final_val = 0.0;
      switch (type) {
        case data_source_type::money: final_val = c->resources.get(core::character_resources::money) * mult; break;
        // какой инком по умолчанию? мне кажется что месячный, а этот инком за один ход
        // вообще не так все просто, мне придется обращаться тогда к календарю, чтобы узнать сколько ходов в году
        // вообще конечно в моем случае придется наверное сделать инком за один ход, а месячный вычислять умножая на какое нибудь число
        case data_source_type::money_income: final_val = (c->stats.get(core::character_stats::tax_income) + c->stats.get(core::character_stats::trade_income)) * mult; break;
        case data_source_type::money_month_income: final_val = (c->stats.get(core::character_stats::tax_income) + c->stats.get(core::character_stats::trade_income)) * 4 * mult; break;
        case data_source_type::money_yearly_income: final_val = (c->stats.get(core::character_stats::tax_income) + c->stats.get(core::character_stats::trade_income)) * 4 * 12 * mult; break;
        default: assert(false);
      }
      
      return final_val;
    }
    
    static double get_value_from_special_data(const struct target_t &target, const uint16_t &type, const double &mult) {
      double final_val = 0.0;
      switch (static_cast<core::structure>(target.type)) {
        case core::structure::character: final_val = get_value_from_special_character_data(reinterpret_cast<core::character*>(target.data), type, mult); break;
        default: assert(false);
      }
      return final_val;
    }
    
    std::tuple<double, uint16_t, uint8_t, uint8_t> get_num_from_data(const struct target_t* target, context* ctx, const script_data* data) {
      double final_value = 0.0;
      uint16_t special_stat = UINT16_MAX;
      uint8_t number_type = UINT8_MAX;
      uint8_t compare_type = UINT8_MAX;
      switch (data->number_type) {
        case number_type::number: {
          final_value = data->value;
          number_type = data->number_type;
          compare_type = data->compare_type;
          break;
        }
        case number_type::get_scope: {
          if (data->data == nullptr) { // лучше отказаться от массива
//             // должен быть индекс
//             const size_t index = data->value;
//             if (index >= ctx->array_data.size()) throw std::runtime_error("Could not find scope value at index " + std::to_string(index) + ". Maximum is " + std::to_string(ctx->array_data.size()));
//             const auto &val = ctx->array_data[index];
//             if (val.command_type != command_type::scope_value) throw std::runtime_error("Bad scope value");
//             if (val.number_type != number_type::number || val.number_type != number_type::boolean) throw std::runtime_error("Bad scope value");
//             final_value = val.value;
//             number_type = val.number_type;
//             compare_type = val.compare_type;
//             break;
            throw std::runtime_error("Bad map data token");
          }
          
          auto raw_str = reinterpret_cast<char*>(data->data);
          const std::string_view str = raw_str;
          const auto itr = ctx->map_data.find(str);
          if (itr == ctx->map_data.end()) throw std::runtime_error("Could not find scope value " + std::string(str));
          const auto &val = itr->second;
          if (val.command_type != command_type::scope_value) throw std::runtime_error("Bad scope value " + std::string(str));
          if (val.number_type != number_type::number || val.number_type != number_type::boolean) throw std::runtime_error("Bad scope value " + std::string(str));
          final_value = val.value;
          number_type = val.number_type;
          compare_type = val.compare_type;
          break;
        }
        case number_type::stat: {
          special_stat = data->helper2;
          const double mult = data->value;
          final_value = get_value_from_special_data(*target, special_stat, mult);
          break;
        }
        case number_type::lvalue_array: {
          script_data d;
          d.number_type = number_type::object;
          d.helper2 = target->type;
          d.data = target->data;
          get_lvalue(ctx, data, &d);
          assert(d.number_type == number_type::boolean || d.number_type == number_type::number);
          final_value = d.value;
          number_type = d.number_type;
          compare_type = d.compare_type;
          break;
        }
        case number_type::array: {
          script_data d;
          d.number_type = number_type::number;
          get_complex_value(target, ctx, data, &d);
          assert(d.number_type == number_type::boolean || d.number_type == number_type::number);
          final_value = d.value;
          number_type = d.number_type;
          compare_type = d.compare_type;
          break;
        }
        default: throw std::runtime_error("Bad value type");
      }
      
      return std::make_tuple(final_value, special_stat, number_type, compare_type);
    }
    
    char* get_raw_string_from_data(const struct target_t* target, context* ctx, const script_data* data) {
      char* str = nullptr;
      switch (data->number_type) {
        case number_type::string: {
          auto raw_str = reinterpret_cast<char*>(data->data);
          str = raw_str;
          break;
        }
        case number_type::get_scope: {
          if (data->data == nullptr) {
//             const size_t index = data->value;
//             if (index >= ctx->array_data.size()) throw std::runtime_error("Could not find scope value at index " + std::to_string(index) + ". Maximum is " + std::to_string(ctx->array_data.size()));
//             const auto &val = ctx->array_data[index];
//             if (val.command_type != command_type::scope_value) throw std::runtime_error("Bad scope value at index " + std::to_string(index));
//             if (val.number_type != number_type::string) throw std::runtime_error("Bad scope value at index " + std::to_string(index));
//             auto raw_str = reinterpret_cast<char*>(val.data);
//             str = raw_str;
//             break;
            throw std::runtime_error("Bad map data token");
          }
          
          auto raw_str = reinterpret_cast<char*>(data->data);
          const std::string_view str_id = raw_str;
          const auto itr = ctx->map_data.find(str_id);
          if (itr == ctx->map_data.end()) throw std::runtime_error("Could not find scope value " + std::string(str_id));
          const auto &val = itr->second;
          if (val.command_type != command_type::scope_value) throw std::runtime_error("Bad scope value " + std::string(str_id));
          if (val.number_type != number_type::string) throw std::runtime_error("Bad scope value " + std::string(str_id));
          auto final_raw_str = reinterpret_cast<char*>(val.data);
          str = final_raw_str;
          break;
        }
        // может ли быть lvalue у строки? это супер неудобно на самом деле
        // локализация использует в своем значении точки, как отличить ключ от lvalue?
        // + ко всему скорее всего ни одна функция lvalue не возвращает строку
        
        case number_type::array: {
          script_data d;
          d.number_type = number_type::string_view;
          get_complex_value(target, ctx, data, &d);
          assert(d.number_type == number_type::string_view); // важно чтобы приходил view
          str = reinterpret_cast<char*>(d.data);
          break;
        }
        
        default: throw std::runtime_error("Bad value type");
      }
      
      return str;
    }
    
    std::string_view get_string_from_data(const struct target_t* target, context* ctx, const script_data* data) {
      return get_raw_string_from_data(target, ctx, data);
    }
    
    // было бы неплохо передать сюда ожидания типа объекта
    target_t get_target_from_data(const struct target_t* target, context* ctx, const script_data* data) {
      target_t final_t;
      switch (data->number_type) {
        // строки тут не будут использоваться... будут, хотя может быть и нет
//         case number_type::string: 
//         case number_type::string_view: {
//           // тут либо lvalue либо id объекта (но хотя это странно, так как id мы распарсим на этапе инициализации)
//           // тут скорее ТОЛЬКО lvalue, нет, могут быть строки типа "prev", "root", "current" и проч
//           // хотя по идее это lvalue
//           const auto raw_str = reinterpret_cast<const char*>(data->data);
//           const std::string_view str = raw_str;
//           const auto e = magic_enum::enum_cast<general_block_function::values>(str);
//           if (!e) throw std::runtime_error("Bad object token " + std::string(str));
//           const uint32_t index = e.value();
//           assert(index == general_block_function::prev || index == general_block_function::root || index == general_block_function::current);
//           const auto func = general_block_functions[index];
//           
//           break;
//         }
        
        // заранее найденный объект (объект с указанным id)
        case number_type::object: {
          // тут мы еще дополнительно токен передадим
          // но токен может быть только у определенных типов
          // в принципе здесь нам все равно
          final_t = target_t(data->helper2, data->data, cast_to_int64(data->value));
          break;
        }
        
        case number_type::lvalue_array: {
          script_data d;
          d.number_type = number_type::object;
          d.helper2 = target->type;
          d.data = target->data;
          get_lvalue(ctx, data, &d);
          assert(d.number_type == number_type::object);
          assert(d.helper2 != UINT16_MAX);
          final_t = target_t(d.helper2, d.data, cast_to_int64(d.value));
          break;
        }
        
        case number_type::get_scope: {
          assert(data->data != nullptr);
          auto raw_str = reinterpret_cast<char*>(data->data);
          const std::string_view str_id = raw_str;
          const auto itr = ctx->map_data.find(str_id);
          if (itr == ctx->map_data.end()) throw std::runtime_error("Could not find scope value " + std::string(str_id));
          const auto &val = itr->second;
          if (val.command_type != command_type::scope_value) throw std::runtime_error("Bad scope value " + std::string(str_id));
          if (val.number_type != number_type::object) throw std::runtime_error("Bad scope value " + std::string(str_id));
          final_t = target_t(val.helper2, val.data, cast_to_int64(val.value));
          
          break;
        }
        
        case number_type::array: {
          script_data d;
          d.number_type = number_type::object;
          get_complex_value(target, ctx, data, &d);
          assert(d.number_type == number_type::object);
          assert(d.helper2 != UINT16_MAX);
          final_t = target_t(d.helper2, d.data, cast_to_int64(d.value));
          break;
        }
        
        default: throw std::runtime_error("Bad value type");
      }
      
      // где то тут должна быть проверка на валидность указателя по токену
      // для этого что нужно? контекст
      // я так понимаю часть проверок можно будет убрать за ненадобностью
      // а часть проверок неплохо было бы завернуть в дебаг режим
      assert(check_target(final_t));
      
      return final_t;
    }
    
    sol::object make_object(sol::state_view s, const struct target_t &target) {
      sol::object obj = sol::make_object(s, sol::nil);
      if (target.data == nullptr) return obj;
      
      switch (static_cast<core::structure>(target.type)) {
        case core::structure::army:      obj = sol::make_object(s, reinterpret_cast<core::army*>(target.data));      break;
        case core::structure::character: obj = sol::make_object(s, reinterpret_cast<core::character*>(target.data)); break;
        case core::structure::city:      obj = sol::make_object(s, reinterpret_cast<core::city*>(target.data));      break;
        case core::structure::province:  obj = sol::make_object(s, reinterpret_cast<core::province*>(target.data));  break;
        case core::structure::realm:     obj = sol::make_object(s, reinterpret_cast<core::realm*>(target.data));     break;
        case core::structure::titulus:   obj = sol::make_object(s, reinterpret_cast<core::titulus*>(target.data));   break;
        default: break;
      }
      
      return obj;
    }
    
    std::string_view get_func_name(const script_data* data) {
      std::string_view name;
      switch (data->command_type) {
        case command_type::action: {
          name = magic_enum::enum_name(static_cast<action_function::values>(data->helper1));
          break;
        }
        
        case command_type::action_script_block: {
          name = magic_enum::enum_name(static_cast<action_block_function::values>(data->helper1));
          break;
        }
        
        case command_type::condition: {
          name = magic_enum::enum_name(static_cast<condition_function::values>(data->helper1));
          break;
        }
        
        case command_type::condition_script_block: {
          name = magic_enum::enum_name(static_cast<condition_block_function::values>(data->helper1));
          break;
        }
        
        case command_type::general_script_block: {
          name = magic_enum::enum_name(static_cast<general_block_function::values>(data->helper1));
          break;
        }
        
        case command_type::rvalue: {
          name = "rvalue";
          break;
        }
        
        default: throw std::runtime_error(std::string(magic_enum::enum_name(static_cast<command_type::values>(data->command_type))) + " is unused");
      }
      
      return name;
    }
    
    void call_lua_func(const target_t* target, context* ctx, const script_data* data, const bool value, const bool original, const int32_t &ret) {
      call_lua_func(target, ctx, data, double(value), UINT32_MAX, UINT32_MAX, double(original), ret);
    }
    
    void call_lua_func(
      const target_t* target, 
      context* ctx, 
      const script_data* data, 
      const double &value, 
      const uint32_t &compare_type, 
      const uint32_t &special_stat, 
      const double &original, 
      const int32_t &ret
    ) {
      if (ctx->itr_func == nullptr) return;
      
      sol::state_view s = ctx->itr_func->lua_state();
      const auto name = get_func_name(data);
      const auto t_obj = script::make_object(s, *target);
      
      const lua_data_struct st{
        name,
        ctx->nest_level,
        compare_type,
        special_stat,
        sol::make_object(s, value),
        sol::make_object(s, original), // сюда легко можно пихнуть оригинальное значение
        ret != IGNORE_BLOCK ? sol::make_object(s, bool(ret)) : sol::nil
      };
      
      const auto &func = *ctx->itr_func;
      func(&st);
    }
    
    void call_lua_func(const target_t* target, context* ctx, const script_data* data, const std::string_view &str, const sol::object &ad_data, const int32_t &ret) {
      if (ctx->itr_func == nullptr) return;
      
      sol::state_view s = ctx->itr_func->lua_state();
      const auto name = get_func_name(data);
      const auto t_obj = script::make_object(s, *target);
      
      const lua_data_struct st{
        name,
        ctx->nest_level,
        UINT32_MAX,
        UINT32_MAX,
        sol::make_object(s, str),
        ad_data,
        ret != IGNORE_BLOCK ? sol::make_object(s, bool(ret)) : sol::nil
      };
      
      const auto &func = *ctx->itr_func;
      func(&st);
    }
    
    void call_lua_func(const target_t* target, context* ctx, const script_data* data, const target_t* obj, const sol::object &ad_data, const int32_t &ret) {
      if (ctx->itr_func == nullptr) return;
      
      sol::state_view s = ctx->itr_func->lua_state();
      const auto name = get_func_name(data);
      const auto t_obj = script::make_object(s, *target);
      
      const lua_data_struct st{
        name,
        ctx->nest_level,
        UINT32_MAX,
        UINT32_MAX,
        script::make_object(s, *obj),
        ad_data,
        ret != IGNORE_BLOCK ? sol::make_object(s, bool(ret)) : sol::nil
      };
      
      const auto &func = *ctx->itr_func;
      func(&st);
    }
    
    void call_lua_func(
      const target_t* target, 
      context* ctx, 
      const script_data* data, 
      const sol::object &obj, 
      const sol::object &ad_data, 
      const sol::object &ret, 
      const uint32_t &compare_type, 
      const uint32_t &special_stat
    ) {
      if (ctx->itr_func == nullptr) return;
      
      sol::state_view s = ctx->itr_func->lua_state();
      const auto name = get_func_name(data);
      const auto t_obj = script::make_object(s, *target);
      
      const lua_data_struct st{
        name,
        ctx->nest_level,
        compare_type,
        special_stat,
        obj,
        ad_data,
        ret
      };
      
      const auto &func = *ctx->itr_func;
      func(&st);
    }
  }
}
