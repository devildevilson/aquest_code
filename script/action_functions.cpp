#include "action_functions.h"

//#include "bin/core_structures.h"
#include "core/structures_header.h"
#include "re2/re2.h"

namespace devils_engine {
  namespace script {
    static const RE2 regex_obj(complex_var_regex);
    
    static double get_value_from_special_character_data(const core::character* c, const uint16_t &type, const double &mult) {
      double final_val = 0.0;
      switch (type) {
        case data_source_type::money: final_val = c->stat(core::character_stats::money) * mult; break;
        // какой инком по умолчанию? мне кажется что месячный, а этот инком за один ход
        // вообще не так все просто, мне придется обращаться тогда к календарю, чтобы узнать сколько ходов в году
        // вообще конечно в моем случае придется наверное сделать инком за один ход, а месячный вычислять умножая на какое нибудь число
        case data_source_type::money_income: final_val = c->stat(core::character_stats::income) * mult; break;
        case data_source_type::money_month_income: final_val = c->stat(core::character_stats::income) * 4 * mult; break;
        case data_source_type::money_yearly_income: final_val = c->stat(core::character_stats::income) * 4 * 12 * mult; break;
        default: assert(false);
      }
      
      return final_val;
    }
    
    static double get_value_from_special_data(const struct target &target, const uint16_t &type, const double &mult) {
      double final_val = 0.0;
      switch (static_cast<core::structure>(target.type)) {
        case core::structure::character: final_val = get_value_from_special_character_data(reinterpret_cast<core::character*>(target.data), type, mult); break;
        default: assert(false);
      }
      return final_val;
    }
    
    static std::tuple<double, uint16_t> get_num_from_data(const struct target &target, const context &ctx, const script_data &data) {
      const uint8_t number_type = data.number_type;
      double final_value = 0.0;
      uint16_t special_stat = UINT16_MAX;
      switch (number_type) {
        case number_type::number: {
          final_value = data.value;
          break;
        }
        case number_type::get_scope: {
          if (data.data == nullptr) {
            // должен быть индекс
            const size_t index = data.value;
            if (index >= ctx.array_data.size()) throw std::runtime_error("Could not find scope value at index " + std::to_string(index) + ". Maximum is " + std::to_string(ctx.array_data.size()));
            const auto &val = ctx.array_data[index];
            const uint8_t number_type = val.number_type;
            if (val.command_type != command_type::scope_value) throw std::runtime_error("Bad scope value");
            if (number_type != number_type::number) throw std::runtime_error("Bad scope value");
            final_value = val.value;
            break;
          }
          
          auto raw_str = reinterpret_cast<char*>(data.data);
          const std::string_view str = raw_str;
          const auto itr = ctx.map_data.find(str);
          if (itr == ctx.map_data.end()) throw std::runtime_error("Could not find scope value " + std::string(str));
          const auto &val = itr->second;
          const uint8_t number_type = val.number_type;
          if (val.command_type != command_type::scope_value) throw std::runtime_error("Bad scope value " + std::string(str));
          if (number_type != number_type::number) throw std::runtime_error("Bad scope value " + std::string(str));
          final_value = val.value;
          break;
        }
        case number_type::stat: {
          special_stat = data.helper2;
          const double mult = data.value;
          final_value = get_value_from_special_data(target, special_stat, mult);
          break;
        }
        default: throw std::runtime_error("Bad value type");
      }
      
      return std::make_tuple(final_value, special_stat);
    }
    
    // будет ли специальные переменные со строками?
    //std::tuple<std::string_view, uint16_t>
    static std::string_view get_string_from_data(const struct target &target, const context &ctx, const script_data &data) {
      std::string_view str;
      switch (data.number_type) {
        case number_type::string: {
          auto raw_str = reinterpret_cast<char*>(data.data);
          str = raw_str;
          break;
        }
        case number_type::get_scope: {
          if (data.data == nullptr) {
            const size_t index = data.value;
            if (index >= ctx.array_data.size()) throw std::runtime_error("Could not find scope value at index " + std::to_string(index) + ". Maximum is " + std::to_string(ctx.array_data.size()));
            const auto &val = ctx.array_data[index];
            if (val.command_type != command_type::scope_value) throw std::runtime_error("Bad scope value at index " + std::to_string(index));
            if (val.number_type != number_type::string) throw std::runtime_error("Bad scope value at index " + std::to_string(index));
            auto raw_str = reinterpret_cast<char*>(val.data);
            str = raw_str;
            break;
          }
          
          auto raw_str = reinterpret_cast<char*>(data.data);
          const std::string_view str_id = raw_str;
          const auto itr = ctx.map_data.find(str_id);
          if (itr == ctx.map_data.end()) throw std::runtime_error("Could not find scope value " + std::string(str_id));
          const auto &val = itr->second;
          if (val.command_type != command_type::scope_value) throw std::runtime_error("Bad scope value " + std::string(str_id));
          if (val.number_type != number_type::string) throw std::runtime_error("Bad scope value " + std::string(str_id));
          auto final_raw_str = reinterpret_cast<char*>(val.data);
          str = final_raw_str;
          break;
        }
        default: throw std::runtime_error("Bad value type");
      }
      
      return str;
    }
    
    static sol::object make_object_from_target(lua_State* state, const struct target &t) {
      sol::object target_obj;
      // желательно отдельный таргет тайп, с другой стороны таргет всегда объект мира
      switch (static_cast<core::structure>(t.type)) {
        case core::structure::army:      target_obj = sol::make_object(state, reinterpret_cast<core::army*>(t.data));      break;
        case core::structure::character: target_obj = sol::make_object(state, reinterpret_cast<core::character*>(t.data)); break;
        case core::structure::city:      target_obj = sol::make_object(state, reinterpret_cast<core::city*>(t.data));      break;
        case core::structure::province:  target_obj = sol::make_object(state, reinterpret_cast<core::province*>(t.data));  break;
        case core::structure::realm:     target_obj = sol::make_object(state, reinterpret_cast<core::realm*>(t.data));     break;
        case core::structure::titulus:   target_obj = sol::make_object(state, reinterpret_cast<core::titulus*>(t.data));   break;
        default: throw std::runtime_error("Not implemented yet");
      }
      
      return target_obj;
    }
    
    // в случае экшонов, value может быть и строкой и таргетом
    static void call_lua_func_value(const struct target &t, const context &ctx, const script_data &data, const double &value, const uint16_t &special_stat) {
      const auto &func = *ctx.itr_func;
      lua_State* state = func.lua_state();
      // нужно передать название
      const auto name = magic_enum::enum_name(static_cast<action_function::value>(data.helper1));
      // нужно отправить таргета
      // возможно нужно передать данные, какие? размер массива или число или указатель на объект
      // причем нужно отправить вычисленное значение? к вычесленному значению нужно отправить 
      // тип сравнения и дополнительный тип переменной (например указать что это месячный инком)
      const sol::object target_obj = make_object_from_target(state, t);
      
      func(name, target_obj, value, sol::nil, special_stat == UINT16_MAX ? sol::nil : sol::make_object(state, special_stat), sol::nil);
    }
    
    static void call_lua_func_object(const struct target &t, const context &ctx, const script_data &data, const sol::object &obj, const uint16_t &special_stat) {
      const auto &func = *ctx.itr_func;
      lua_State* state = func.lua_state();
      // нужно передать название
      const auto name = magic_enum::enum_name(static_cast<action_function::value>(data.helper1));
      // нужно отправить таргета
      // возможно нужно передать данные, какие? размер массива или число или указатель на объект
      // причем нужно отправить вычисленное значение? к вычесленному значению нужно отправить 
      // тип сравнения и дополнительный тип переменной (например указать что это месячный инком)
      const sol::object target_obj = make_object_from_target(state, t);
      func(name, target_obj, obj, sol::nil, special_stat == UINT16_MAX ? sol::nil : sol::make_object(state, special_stat), sol::nil);
    }
    
    void add_money(const target &t, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(t.type == static_cast<uint32_t>(core::character::s_type));
      assert(count == 1);
      assert(data[0].command_type == command_type::action);
      auto character = reinterpret_cast<core::character*>(t.data);
      const auto [val, special_stat] = get_num_from_data(t, ctx, data[0]);
      if (ctx.itr_func != nullptr) { call_lua_func_value(t, ctx, data[0], val, special_stat); return; }
      character->add_to_stat(core::character_stats::money, val);
    }
    
    void add_trait(const target &t, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(t.type == static_cast<uint32_t>(core::character::s_type));
      assert(count == 1);
      assert(data[0].command_type == command_type::action);
      assert(data[0].number_type == number_type::string); // тут может быть еще и объект игрового мира
      assert(data[0].data != nullptr);
      auto str_raw = reinterpret_cast<char*>(data[0].data);
      std::string_view str = str_raw;
      // по этой строке мы должны найти трейт и добавить его персонажу
      auto character = reinterpret_cast<core::character*>(t.data);
      
    }
    
    void add_flag(const target &t, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(data[0].command_type == command_type::action);
      assert(data[0].data != nullptr);
      
      if (ctx.itr_func != nullptr) {
        sol::state_view state = ctx.itr_func->lua_state();
        sol::object obj;
        
        switch (data[0].number_type) {
          case number_type::array: {
            auto array_data = reinterpret_cast<script_data*>(data[0].data);
            const size_t array_count = data[0].value;
            assert(array_data[0].command_type == command_type::invalid);
            ASSERT(array_count > 0);
            auto table = state.create_table(0, 2);
            const auto str = get_string_from_data(t, ctx, array_data[0]);
            // с другой стороны нужно сохранить то как задается объект в конфиге
            //obj = sol::make_object(state, str);
            table["flag"] = str;
            if (array_count != 1) {
              const auto [value, special_stat] = get_num_from_data(t, ctx, array_data[1]);
              table["time"] = value;
            }
            
            obj = table;
            
            break;
          }
          
          case number_type::string:
          case number_type::get_scope: {
            const auto str = get_string_from_data(t, ctx, data[0]);
            obj = sol::make_object(state, str);
            break;
          }
          
          default: assert(false);
        }
        
        // пока что оставим UINT16_MAX
        call_lua_func_object(t, ctx, data[0], obj, UINT16_MAX);
        return;
      }
      
      // тут может быть несколько входных данных
      std::string_view str;
      size_t turns = SIZE_MAX;
      switch (data[0].number_type) {
        case number_type::array: {
          auto array_data = reinterpret_cast<script_data*>(data[0].data);
          const size_t array_count = data[0].value;
          assert(array_data[0].command_type == command_type::invalid);
          // в этом случае у меня должен соблюдаться строгий порядок входных данных
          // например тут я бы ожидал в первом слоте строку, а во втором слоте сколько ходов
          //assert(array_data[0].number_type == number_type::string || array_data[0].number_type == number_type::get_scope);
          //auto str_raw = reinterpret_cast<char*>(array_data[0].data);
          //str = str_raw;
          str = get_string_from_data(t, ctx, array_data[0]);
          if (array_count > 1) {
            //assert(array_data[1].number_type == number_type::number || array_data[1].number_type == number_type::get_scope);
            const auto [value, special_stat] = get_num_from_data(t, ctx, array_data[1]);
            turns = value;
          }
          
          break;
        }
        
        case number_type::string: 
        case number_type::get_scope: {
//           auto str_raw = reinterpret_cast<char*>(data[0].data);
//           str = str_raw;
          str = get_string_from_data(t, ctx, data[0]);
          turns = SIZE_MAX;
          break;
        }
        
        default: assert(false);
      }
      
      // эдд флаг как раз пример того что нужно делать сложную систему огранизации инпута в луа функцию
      // во первых у нас явно может здесь быть как и массив данных, так и просто строка
      // вообще нам бы вернуть таблицу в том же виде в котором она была в конфиге скрипта
      // другое дело что у нас очень четкий порядок переменных, 
      // и скорее всего можно не запариваться над наименованиями переменных
      //if (ctx.itr_func != nullptr) { call_lua_func_value(t, ctx, data[0], val, special_stat); return; }
      
      // по идее строка должна остаться валидной
      const core::structure target_type = static_cast<core::structure>(t.type);
      switch (target_type) {
        case core::structure::character: {
          auto character = reinterpret_cast<core::character*>(t.data);
          character->add_flag(str, turns);
          break;
        }
        
        case core::structure::army: {
          auto army = reinterpret_cast<core::army*>(t.data);
          army->add_flag(str, turns);
          break;
        }
        
        case core::structure::city: {
          auto city = reinterpret_cast<core::city*>(t.data);
          city->add_flag(str, turns);
          break;
        }
        
        case core::structure::province: {
          auto province = reinterpret_cast<core::province*>(t.data);
          province->add_flag(str, turns);
          break;
        }
        
        case core::structure::titulus: {
          auto title = reinterpret_cast<core::titulus*>(t.data);
          title->add_flag(str, turns);
          break;
        }
        
        case core::structure::realm: {
          auto realm = reinterpret_cast<core::realm*>(t.data);
          realm->add_flag(str, turns);
          break;
        }
        
        default: throw std::runtime_error("Bad target type " + std::string(magic_enum::enum_name<core::structure>(target_type)));
      }
    }
    
    void marry(const target &t, const context &ctx, const uint32_t &count, const script_data* data) {
      // по идее здесь уже должен по умолчанию приходить массив
      // какой размер? нужно указать на ком женитьба, кто патрон, ???
      // как быть с условиями женитьбы? то есть как посчитать очки хочет/не хочет?
      // хотя в цк3 приходит только персонаж на ком жениться
      // да это очень простое действие
      // мне нужно сделать значит проверку возможности женитьбы отдельную
      // и для нее сделать вывод полезной информации
      
      // самое важное - эта фукция будет вызывать еще и эвенты по цепочке 
      // utils::action_type::on_marriage, другое дело что нужно супер аккуратно вызвать только один раз
      // 
    }
    
    void add_hook(const target &t, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(t.type == static_cast<uint32_t>(core::character::s_type));
      // тут нужно 4 входных данных: тип, на кого, какой секрет, сколько дней держится
      // таргет только персонаж
      // по крайней мере один входной будет в контексте
      // с хуками немног посложнее, их может быть очень много с учетом всех персонажей в игре
      // нужно как то оптимизировать все это дело
    }
    
    void start_war(const target &t, const context &ctx, const uint32_t &count, const script_data* data) {
      // одна из самых важных функций, для старта войны нам нужно 
      // casus_belli, таргет, кто клаймат (то есть из-за кого (из-за чьих амбиций) мы начали войну)
      // и титулы за которые мы боремся, война стартуется между персонажами, что делать если персонаж умирает на войне?
      // если клаймат умирает во время войны, то война заканчивается (точнее если клейм не передается по наследству)
      // война длится чаще всего много лет и игра заключается в том чтобы сохранить жизнь челику до конца войны
      // хватит ли мне ходов до конца войны? точнее сколько ходов у меня будет средняя война? 
      // несколько лет это примерно 150-200 ходов, на такое количество ходов нужно расчитывать 
      // для обычной войны за титул с примерно равным противником, хотя это сильно зависит от присутствия союзников
      // в моем случае война поди будет уделом реалма, даже притом что зависит от клеймов персонажа
      // можно даже сделать государственный клейм, и например совет может решить стартануть войну по этому клейму
      // клаймат и таргет может быть как и персонажем так и реалмом?
      assert(count == 1);
      assert(data[0].command_type == command_type::action);
      assert(data[0].number_type == number_type::array);
      auto array_data = reinterpret_cast<script_data*>(data[0].data);
      const size_t array_count = data[0].value;
      assert(array_count == 4); // кажется здесь может приходить только 4 штуки данных
      // что такое казус белли? по идее это тоже игровая сущность, которую можно задать
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
    
    // например у нас появятся строки "value:more:200"
    // "money_income:more:1.75"
    
    // короче я только что проверил "\\d" не работает для числе с плавающей точкой
    // для них нужно заводить более сложное условие + неполучится парсить сразу в RE2
    // короч можно доконца распарсить число, а потом спокойно кормить atof строкой
    
    void variable_input_init(const sol::object &obj, script_data* data) {
      ASSERT(data != nullptr);
      switch (obj.get_type()) {
        case sol::type::number: {
          const double m = obj.as<double>();
          data->number_type = number_type::number;
          data->value = m;
          break;
        }
        
        case sol::type::string: {
          // тут может быть как особый инком, так и взятие из контекста
          const auto str = obj.as<std::string_view>();
          
          data_source_type::values source;
          data_type::values value_data;
          std::string type1;
          std::string type2;
          std::string num_str;
          double num;
          
          type1.reserve(20);
          type2.reserve(20);
          num_str.reserve(30);
          
          const bool ret = regex_match(str, type1, type2, num_str, source, value_data, num);
          if (!ret) throw std::runtime_error("Could not parse string '" + std::string(str) + "'");
          
          
          // возможно будет лучше все свести к одной регулярке, нужно оставить первую
//           if (RE2::FullMatch(str, regex_obj, &type1, &type2, &num_str)) {
//             const auto e1 = magic_enum::enum_cast<data_source_type::values>(type1);
//             if (!e1) throw std::runtime_error("Could not parse string data type " + type1);
//             source = e1.value();
//             const auto e2 = magic_enum::enum_cast<data_type::values>(type2);
//             if (!e2) throw std::runtime_error("Could not parse string data type " + type2);
//             value_data = e2.value();
//             
//             assert(!num_str.empty());
//             num = std::atof(num_str.c_str());
//           } else throw std::runtime_error("Could not parse string '" + std::string(str) + "'");
          
          uint32_t compare_type = UINT32_MAX;
          switch (value_data) {
            case data_type::equal: compare_type = number_compare_type::equal; break;
            case data_type::not_equal: compare_type = number_compare_type::not_equal; break;
            case data_type::more: compare_type = number_compare_type::more; break;
            case data_type::more_eq: compare_type = number_compare_type::more_eq; break;
            case data_type::less: compare_type = number_compare_type::less; break;
            case data_type::less_eq: compare_type = number_compare_type::less_eq; break;
            default: break;
          }
          
          if (data->command_type == command_type::action && compare_type != number_compare_type::equal) 
            throw std::runtime_error("Bad data type '" + std::string(magic_enum::enum_name<data_type::values>(value_data)) + "' in add_money");
          
          switch (source) {
            case data_source_type::special: {
              // ???
              assert(false);
              break;
            }
            
            case data_source_type::context: {
              // в type2 должен хранится либо id либо индекс (или индекс распарсится в num?)
              data->number_type = number_type::get_scope;
              if (type2 == "index") {
                data->value = size_t(num);
              } else {
                char* mem = new char[type2.size()];
                memcpy(mem, type2.data(), type2.size());
                data->data = mem;
              }
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
          
//           // как дальше ориентироваться? можно переводить строки в энум
//           // и по энуму уже составлять данные
//           if (str.substr(0, sp.size()) == sp) {
//             // специальная строка, нужно проверить что следует дальше, например инком
//             if (str.substr(sp.size(), income.size()) == income) {
//               // за подстрокой инком должен следовать множитель (?)
//               const auto mult = str.substr(sp.size() + income.size(), std::string_view::npos);
//               const double mult_v = std::atof(mult.data()); // по идее 
//               if (mult_v == 0.0) throw std::runtime_error("Bad income multiplier");
//               data->number_type = number_type::number;
//               data->value = mult_v;
//               // нужно указать что это именно инком, можно использовать helper2 для очередного энума
//             }
//           }
//           
//           if (str.substr(0, sc.size()) == sc) {
//             // строка скоуп, следующая подстрока должна содержать ключ, либо числовой либо текстовый
//           }
          break;
        }
        
        // заменится на строку
//         case sol::type::lightuserdata:
//         case sol::type::userdata: {
//           assert(obj.is<script_data>());
//           const auto &d = obj.as<script_data>();
//           // возможно имеет смысл хранить в объектах юзердату
//           data->number_type = d.number_type;
//           data->value = d.value;
//           data->helper2 = d.helper2;
//           if (d.number_type == number_type::string) {
//             // может ли такое быть? может если скоуп
//             // если оформить все это дело как строки, тогда можно сериализовать десижоны в строку
//             // 
//           }
//           break;
//         }
        
        default: throw std::runtime_error("Bad value data for action command");
      }
      
      // добавление статов можно сделать унифицированно, но у нас куча функции в которых уникальный инпут
    }
    
    void string_input_init(const sol::object &obj, script_data* data) {
      ASSERT(data != nullptr);
      switch (obj.get_type()) {
        case sol::type::string: {
          const auto str = obj.as<std::string_view>();
          
          data_source_type::values source;
          data_type::values value_data;
          std::string type1;
          std::string type2;
          std::string num_str;
          double num;
          
          type1.reserve(20);
          type2.reserve(20);
          num_str.reserve(30);
          
          const bool ret = regex_match(str, type1, type2, num_str, source, value_data, num);
          if (ret) {
            switch (source) {
              case data_source_type::special: {
                // ???
                assert(false);
                break;
              }
              
              case data_source_type::context: {
                // в type2 должен хранится либо id либо индекс (или индекс распарсится в num?)
                data->number_type = number_type::get_scope;
                if (type2 == "index") {
                  data->value = size_t(num);
                } else {
                  char* mem = new char[type2.size()];
                  memcpy(mem, type2.data(), type2.size());
                  data->data = mem;
                }
                break;
              }
              
              default: throw std::runtime_error(std::string(magic_enum::enum_name(source)) + " not implemented yet");
            }
            
            return;
          }
          
          // если не попадает в regex, то мы считаем что это валидная строка
          data->number_type = number_type::string;
          char* mem = new char[str.size()];
          memcpy(mem, str.data(), str.size());
          data->data = mem;
          break;
        }
        
        default: throw std::runtime_error("Bad string data for action command");
      }
    }
    
    // это функция инит, тут нужно бы придумать способ сделать унифицированный интерфейс
    // сюда наверное нужно передавать тип таргета, да нужно
    void add_money_init(const uint32_t &target_type, const sol::object &obj, script_data* data) {
      if (target_type != static_cast<uint32_t>(core::character::s_type) && target_type != static_cast<uint32_t>(core::realm::s_type)) 
        throw std::runtime_error(
          "Bad input type '" + 
          std::string(magic_enum::enum_name<core::structure>(static_cast<core::structure>(target_type))) + 
          "' for add_money");
      
      data->command_type = command_type::action;
      data->helper1 = action_function::add_money;
      
      variable_input_init(obj, data);
    }
  
  // пока что у нас мало статов в экшоне
//#define ADD_CHARACTER_STAT_INIT_COMMAND(name) void add_money_init(const uint32_t &target_type, const sol::object &obj, script_data* data)

    static bool check_context(const std::string_view &str, script_data* data) {
      data_source_type::values source;
      data_type::values value_data;
      std::string type1;
      std::string type2;
      std::string num_str;
      double num;
      
      type1.reserve(20);
      type2.reserve(20);
      num_str.reserve(30);
      
      const bool ret = regex_match(str, type1, type2, num_str, source, value_data, num);
      if (ret) {
        if (source != data_source_type::context) return false;
        data->number_type = number_type::get_scope;
        if (value_data == data_type::index) {
          data->value = size_t(num);
        } else {
          char* mem = new char[type2.size()];
          memcpy(mem, type2.data(), type2.size());
          data->data = mem;
        }
      }
      
      return ret;
    }

    void add_flag_init(const uint32_t &target_type, const sol::object &obj, script_data* data) {
      // флаг мы можем добавить почти ко всему
      // 
      
      data->command_type = command_type::action;
      data->helper1 = action_function::add_flag;
      
      // на вход может податься только строка? число нет, контекст? контекст или строка
      // если строка не парсится через regex, то это флаг?
      
      if (obj.get_type() != sol::type::string || obj.get_type() != sol::type::table) throw std::runtime_error("Bad input obj for add_flag");
      
      if (obj.get_type() == sol::type::string) {
        string_input_init(obj, data);
        return;
      }
      
      if (obj.get_type() == sol::type::table) {
        const sol::table t = obj.as<sol::table>();
        const auto flag_proxy = t["flag"]; 
        const bool valid_flag = flag_proxy.valid() && flag_proxy.get_type() == sol::type::string;
        const auto time_proxy = t["time"]; 
        const bool valid_time = flag_proxy.valid() && (flag_proxy.get_type() == sol::type::string || flag_proxy.get_type() == sol::type::number);
        if (!valid_flag) throw std::runtime_error("Bad add_flag input value type");
        
        script_data* flag_data = nullptr;
        script_data* turns_data = nullptr;
        if (valid_flag && valid_time) {
          data->number_type = number_type::array;
          data->value = 2;
          script_data* d = new script_data[2];
          data->data = d;
          flag_data = &d[0];
          turns_data = &d[1];
        } else {
          assert(valid_flag);
          assert(!valid_time);
          flag_data = data;
        }
        
        string_input_init(flag_proxy, flag_data);
        if (valid_time) variable_input_init(time_proxy, turns_data);
      }
      
      UNUSED_VARIABLE(target_type);
    }
    
    void add_hook_init(const uint32_t &target_type, const sol::object &obj, script_data* data) {
      if (obj.get_type() != sol::type::table) throw std::runtime_error("Bad data for add_money command");
      
      const sol::table t = obj.as<sol::table>();
      // обязательно должны быть указаны: тип, таргет, секрет, и по желанию количество ходов
    }
    
    void marry_init(const uint32_t &target_type, const sol::object &obj, script_data* data) {
      // тут должен быть контекст скорее всего который вернет таргет, таргет может быть только персонажем
      // скорее всего здесь не должно быть ни одного способа получить персонажа без контекста
    }
    
    void add_trait_init(const uint32_t &target_type, const sol::object &obj, script_data* data) {
      // тут может быть id треита или мы можем взять его из контекста
      // откуда треит мы можем получить? нужна мапа со всеми такими объектами
    }
    
    void start_war_init(const uint32_t &, const sol::object &, script_data*) {
      // тут особо разночтений не будет, таргет, цб, клаймат, титулы
    }
  }
}
