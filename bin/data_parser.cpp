#include "data_parser.h"
#include <unordered_map>
#include "utils/globals.h"
#include "utils/assert.h"
#include "core_structures.h"

namespace devils_engine {
  namespace utils {
    functions_container::functions_container() {
      creation_funcs["potential"] = {
        logic_operation::op_and,
        stack_operation::nothing,
//         target_data::type::count,
//         target_data::type::count,
        [this] (const std::string &key, const sol::table &table, const enum target_data::type &target, std::vector<operation> &operations) {
          ASSERT(key == "potential" || key == "allow");
          for (auto itr = table.begin(); itr != table.end(); ++itr) {
            if (!(*itr).first.is<std::string>()) continue;
            if ((*itr).second == sol::lua_nil) continue;
            if ((*itr).second.is<sol::table>()) continue;
            
            const std::string &key = (*itr).first.as<std::string>();
            for (const auto &cond : conditions) {
              if (cond.first != key) continue; // долго?
              
              ASSERT(cond.second.type == target);
              
              operation o;
              o.type = 0;
              o.condition = &cond.second;
              o.logic_operation = cond.second.logic_operation;
              o.stack_operation = stack_operation::nothing;
              o.data = cond.second.check((*itr).second);
              
              operations.push_back(o);
            }
          }
          
          static const std::unordered_set<std::string> complex_condition = {
            "and", "nand", "or", "nor"
          };
          
          for (auto itr = table.begin(); itr != table.end(); ++itr) {
            if (!(*itr).first.is<std::string>()) continue;
            if ((*itr).second == sol::lua_nil) continue;
            if (!(*itr).second.is<sol::table>()) continue;
            
            // теперь мы должны проверить сложные условия 
            // (то есть вложенные условные операторы например)
            // для этого нужна дополнительная информация в creation_funcs
            const std::string &key = (*itr).first.as<std::string>();
            std::string str(key);
            std::transform(key.begin(), key.end(), str.begin(), [] (unsigned char c) { return std::tolower(c); });
            
            if (complex_condition.find(str) == complex_condition.end()) continue;
            
            const auto &table = (*itr).second.as<sol::table>();
            creation_funcs["complex_condition"].func(str, table, target, operations);
          }
          
          for (auto itr = table.begin(); itr != table.end(); ++itr) {
            if (!(*itr).first.is<std::string>()) continue;
            if ((*itr).second == sol::lua_nil) continue;
            if (!(*itr).second.is<sol::table>()) continue;
            
            const std::string &key = (*itr).first.as<std::string>();
            std::string str(key);
            std::transform(key.begin(), key.end(), str.begin(), [] (unsigned char c) { return std::tolower(c); });
            
            if (complex_condition.find(str) != complex_condition.end()) continue;
            
            const auto &table = (*itr).second.as<sol::table>();
            creation_funcs["change_context"].func(str, table, target, operations);
          }
        }
      };
      
      creation_funcs["effects"] = {
        logic_operation::count,
        stack_operation::nothing,
        [this] (const std::string &key, const sol::table &table, const enum target_data::type &target, std::vector<operation> &operations) {
          ASSERT(key == "effects");
          for (auto itr = table.begin(); itr != table.end(); ++itr) {
            if (!(*itr).first.is<std::string>()) continue;
            if ((*itr).second == sol::lua_nil) continue;
            if ((*itr).second.is<sol::table>()) continue;
            
            const std::string &key = (*itr).first.as<std::string>();
            for (const auto &effect : effects) {
              if (effect.first != key) continue;
              
              ASSERT(effect.second.type == target);
              ASSERT(effect.second.stack_operation == stack_operation::nothing); // сложные эффекты дальше
              
              operation o;
              o.type = 1;
              o.effect = &effect.second;
              o.logic_operation = logic_operation::count;
              o.stack_operation = stack_operation::nothing;
              o.data = effect.second.check((*itr).second);
              
              operations.push_back(o);
            }
          }
          
          // у нас тут во первых переключение контекста
          // а во вторых сложный эффект (например объявление войны, запуск эвента, передача провинции/титула)
          static const std::unordered_set<std::string> complex_effect = {
            "declare_war", "fire_event", "cede_holding", 
          };
          
          for (auto itr = table.begin(); itr != table.end(); ++itr) {
            if (!(*itr).first.is<std::string>()) continue;
            if ((*itr).second == sol::lua_nil) continue;
            if (!(*itr).second.is<sol::table>()) continue;
            
            const std::string &key = (*itr).first.as<std::string>();
            if (complex_effect.find(key) != complex_effect.end()) {
              const auto &table = (*itr).second.as<sol::table>();
              creation_funcs["complex_effect"].func(key, table, target, operations);
            } else {
              const auto &table = (*itr).second.as<sol::table>();
              creation_funcs["change_context"].func(key, table, target, operations);
            }
          }
        }
      };
      
      // теперь нужно сделать множество функций 
      // разные проверки условий и эффекты
      // хорошей метрикой для проверки функциональности этого инструмента будет тот факт
      // что я смогу сделать довольно длительную цепочку квестов об Апати
      // это одна из ультимативных целей
    }
    
    void pass_error(size_t &counter, const std::string &str) {
      ++counter;
      std::cout << str << "\n";
    }
    
    std::string get_type_name(const enum target_data::type &type) {
      switch (type) {
        case target_data::type::character: return "character";
        case target_data::type::province: return "province";
        case target_data::type::city: return "city";
        case target_data::type::army: return "army";
        case target_data::type::hero: return "hero";
        case target_data::type::religion: return "religion";
        case target_data::type::culture: return "culture";
        case target_data::type::dynasty: return "dynasty";
        case target_data::type::troop: return "troop";
        case target_data::type::party_member: return "party_member";
        case target_data::type::count: throw std::runtime_error("Bad target type");
      }
      
      return "";
    }
    
    void check_nesting_table_recursive(const bool is_condition, const std::string &key, const sol::table &table, const enum target_data::type &prev_target, size_t &counter) {
      auto cont = global::get<action_container>();
      auto current_target = prev_target;
      
      if (key != "potential" && key != "allow" && key != "effects") {
        std::string str(key);
        std::transform(key.begin(), key.end(), str.begin(), [] (unsigned char c) { return std::tolower(c); });
        
        if (str != "and" && str != "or" && str != "nand" && str != "nor") {
          auto context_itr = cont->change_context.find(key);
          if (context_itr == cont->change_context.end()) {
            pass_error(counter, "Bad structure key");
            return;
          } else {
            if (current_target != context_itr->second.type) {
              pass_error(counter, "Could not change context from " + get_type_name(current_target) + " to " + get_type_name(context_itr->second.type));
              return;
            }
            current_target = context_itr->second.new_type;
          }
        }
      }
        
      for (auto itr = table.begin(); itr != table.end(); ++itr) {
        if (!(*itr).first.is<std::string>()) continue;
        if ((*itr).second == sol::lua_nil) continue;
        const std::string &key = (*itr).first.as<std::string>();
        if ((*itr).second.is<sol::table>()) {
          const auto &local_table = (*itr).second.as<sol::table>();
          check_nesting_table_recursive(is_condition, key, local_table, current_target, counter);
          continue;
        }
        
        if (is_condition) {
          auto condition_itr = cont->conditions.find(key);
          if (condition_itr != cont->conditions.end()) {
            if (current_target != condition_itr->second.type) pass_error(counter, "Could not check condition " + key + " with data type " + get_type_name(current_target));
            if (condition_itr->second.check((*itr).second).uval == SIZE_MAX) pass_error(counter, "Could not pass data to condition func " + key);
            continue;
          }
        } else {
          auto effect_itr = cont->effects.find(key);
          if (effect_itr != cont->effects.end()) {
            if (current_target != effect_itr->second.type) pass_error(counter, "Could not call effect func " + key + " with data type " + get_type_name(current_target));
            if (effect_itr->second.check((*itr).second).uval == SIZE_MAX) pass_error(counter, "Could not pass data to effect func " + key);
            continue;
          }
        }
        
        if (is_condition) pass_error(counter, "Could not find condition func " + key);
        else pass_error(counter, "Could not find effect func " + key);
      }
    }
    
    bool check_valid_event(const sol::table &table) {
      size_t counter = 0;
      
      // у нас могут быть эвенты разных типов (например эвент провинции и эвент предложения войны, разное оформление)
//       {
//         auto proxy = table["type"];
//         if (!proxy.valid()) pass_error(counter, "Table must specify the type");
//         const uint32_t type = proxy.get_or<uint32_t, uint32_t>(UINT32_MAX);
//         if (type == UINT32_MAX) pass_error(counter, "Table must have event type");
//       }
      
      // должен быть уникальный id, по нему я вызываю эвент в триггерах
      std::string id;
      {
        auto proxy = table["id"];
        if (!proxy.valid()) pass_error(counter, "Event table must have an id");
        const std::string str = proxy.get_or<std::string, std::string>(std::string());
        if (str.empty()) pass_error(counter, "Event id must be a valid string");
        id = str;
      }
      
      auto current_target = target_data::type::count;
      {
        auto proxy = table["target"];
        if (!proxy.valid()) pass_error(counter, "Event table must specify the target");
        // тут все же скорее всего будет число
        const auto target = proxy.get_or<enum target_data::type, enum target_data::type>(target_data::type::count);
        if (target >= target_data::type::count) pass_error(counter, "Event " + id + " target must be a valid enumeration");
        current_target = target;
      }
      
      {
        auto proxy = table["name"];
        if (!proxy.valid()) pass_error(counter, "Event " + id + " table must have a name");
        const auto str = proxy.get_or<std::string, std::string>(std::string());
        if (str.empty()) pass_error(counter, "Event " + id + " name must be a valid string");
      }
      
      {
        auto proxy = table["desc"];
        if (!proxy.valid()) pass_error(counter, "Event " + id + " table must have a description");
        const auto str = proxy.get_or<std::string, std::string>(std::string());
        if (str.empty()) pass_error(counter, "Event " + id + " description must be a valid string");
      }
      
      {
        auto proxy = table["picture"];
        if (!proxy.valid()) pass_error(counter, "Event " + id + " must have a picture");
        const auto str = proxy.get_or<std::string, std::string>(std::string());
        if (str.empty()) pass_error(counter, "Event " + id + " picture must be a valid string");
      }
      
      {
        auto proxy = table["mean_time"];
        if (proxy.valid()) {
          const uint32_t opt = proxy.get_or<uint32_t, uint32_t>(UINT32_MAX);
          if (opt != UINT32_MAX) pass_error(counter, "Event " + id + " mean time must be a number");
        }
      }
      
      if (current_target == target_data::type::count) return false;
      
      {
        auto proxy = table["potential"]; // если мы вызываем откуда то эвент, то нужны ли нам эти условия?
        if (!proxy.valid()) pass_error(counter, "Event " + id + " table must specify potential contditions");
        const auto opt = proxy.get_or<sol::table, std::nullptr_t>(nullptr);
        if (opt.get_type() != sol::type::table) pass_error(counter, "Event " + id + " potential contditions must be a table");
        
        // нужно еще проверить валидность условий
        check_nesting_table_recursive(true, "potential", opt, current_target, counter);
      }
      
      {
        auto proxy = table["options"]; // может и не быть
        if (proxy.valid()) {
          const auto opt = proxy.get_or<sol::table, std::nullptr_t>(nullptr);
          if (opt.get_type() != sol::type::table) pass_error(counter, "Event " + id + " options must be a table");
          size_t opt_counter = 0;
          for (auto itr = opt.begin(); itr != opt.end(); ++itr) {
            if (!(*itr).second.is<sol::table>()) continue;
            
            ++opt_counter;
            const auto &nested_table = (*itr).second.as<sol::table>();
            std::string option_id;
            {
              auto proxy = nested_table["id"]; // только для дебага?
              if (proxy.valid()) {
                const auto str = proxy.get_or<std::string, std::string>(std::string());
                if (str.empty()) pass_error(counter, "Option id must be a valid string");
                option_id = str;
              }
            }
            
            {
              auto proxy = nested_table["name"];
              if (!proxy.valid()) pass_error(counter, "Event " + id + " option must have a name");
              const auto str = proxy.get_or<std::string, std::string>(std::string());
              if (str.empty()) pass_error(counter, "Option " + option_id + " name must be a valid string");
            }
            
            {
              // описание
            }
            
            {
              auto proxy = nested_table["potential"];
              if (proxy.valid()) {
                const auto &table = proxy.get_or<sol::table, std::nullptr_t>(nullptr);
                if (table.get_type() != sol::type::table) pass_error(counter, "Option " + option_id + " potential contditions must be a table");
                check_nesting_table_recursive(true, "potential", table, current_target, counter);
              }
            }
            
            {
              auto proxy = nested_table["effects"];
              if (proxy.valid()) {
                const auto &table = proxy.get_or<sol::table, std::nullptr_t>(nullptr);
                if (table.get_type() != sol::type::table) pass_error(counter, "Option " + option_id + " effects must be a table");
                check_nesting_table_recursive(false, "effects", table, current_target, counter);
              }
            }
          }
        }
      }
      
      if (counter != 0) return false;
      return true;
    }
    
    const std::vector<std::string> effects_priority = {
      "id",
      "mean_time",
      "cb"
    };
    
    void add_nesting_table_recursive(const bool is_condition, const std::string &key, const sol::table &table, const enum target_data::type &prev_target, std::vector<operation> &operations) {
      auto cont = global::get<action_container>();
      auto current_target = prev_target;
      auto context_itr = cont->change_context.find(key);
      if (key != "potential" && key != "allow" && key != "effect") {
        if (context_itr != cont->change_context.end()) {
          ASSERT(current_target == context_itr->second.type);
          current_target = context_itr->second.type;
          operation o;
          o.type = 2;
          o.logic_operation = functions_container::logic_operation::count;
          o.stack_operation = context_itr->second.stack_operation;
          o.context = &context_itr->second;
          o.data.uval = SIZE_MAX;
          operations.push_back(o);
          // что пихнуть в качестве конца? по идее key + "_end" должно сработать
        } else {
          if (is_condition) {
            auto cond_itr = cont->conditions.find(key);
            ASSERT(cond_itr != cont->conditions.end());
            operation o;
            o.type = 0;
            o.logic_operation = cond_itr->second.logic_operation;
            o.stack_operation = functions_container::stack_operation::count;
            o.condition = &cond_itr->second;
            o.data.uval = SIZE_MAX;
            operations.push_back(o);
          } else {
            auto effect_itr = cont->effects.find(key);
            ASSERT(effect_itr != cont->effects.end());
            operation o;
            o.type = 1;
            o.logic_operation = functions_container::logic_operation::count;
            o.stack_operation = effect_itr->second.stack_operation;
            o.effect = &effect_itr->second;
            o.data.uval = SIZE_MAX;
            operations.push_back(o);
          }
        }
      }
      
      if (context_itr == cont->change_context.end() && !is_condition) {
        for (const auto &str : effects_priority) {
          auto proxy = table[str];
          // тут нужно как то ограничить функции 
        }
      } else {
        for (auto itr = table.begin(); itr != table.end(); ++itr) {
          if (!(*itr).first.is<std::string>()) continue;
          if ((*itr).second == sol::lua_nil) continue;
          
          const std::string &key = (*itr).first.as<std::string>();
          if ((*itr).second.is<sol::table>()) {
            const auto &table = (*itr).second.as<sol::table>();
            add_nesting_table_recursive(is_condition, key, table, current_target, operations);
            continue;
          }
          
          if (is_condition) {
            auto cond_itr = cont->conditions.find(key);
            if (cond_itr != cont->conditions.end()) {
              const auto &data = cond_itr->second.check((*itr).second);
              operations.push_back({0, cond_itr->second.logic_operation, functions_container::stack_operation::count, &cond_itr->second, data});
              continue;
            }
          } else {
            auto effect_itr = cont->effects.find(key);
            if (effect_itr != cont->effects.end()) {
              operation o;
              o.type = 1;
              o.logic_operation = functions_container::logic_operation::count;
              o.stack_operation = effect_itr->second.stack_operation;
              o.effect = &effect_itr->second;
              o.data = effect_itr->second.check((*itr).second);
              operations.push_back(o);
              continue;
            }
          }
          
          ASSERT(false);
        }
      }
      
      if (key != "potential" && key != "allow" && key != "effect") {
        if (context_itr != cont->change_context.end()) {
          operation o;
          o.type = 2;
          o.logic_operation = functions_container::logic_operation::count;
          o.stack_operation = static_cast<functions_container::stack_operation>(static_cast<uint32_t>(context_itr->second.stack_operation)+1);
          o.context = &context_itr->second;
          o.data.uval = SIZE_MAX;
          operations.push_back(o);
          // что пихнуть в качестве конца? по идее key + "_end" должно сработать
        } else {
          if (is_condition) {
            auto cond_itr = cont->conditions.find(key);
            ASSERT(cond_itr != cont->conditions.end());
            operation o;
            o.type = 0;
            o.logic_operation = static_cast<functions_container::logic_operation>(static_cast<uint32_t>(cond_itr->second.logic_operation)+1);
            o.stack_operation = functions_container::stack_operation::count;
            o.condition = &cond_itr->second;
            o.data.uval = SIZE_MAX;
            operations.push_back(o);
          } else {
            auto effect_itr = cont->effects.find(key);
            ASSERT(effect_itr != cont->effects.end());
            operation o;
            o.type = 1;
            o.logic_operation = functions_container::logic_operation::count;
            o.stack_operation = static_cast<functions_container::stack_operation>(static_cast<uint32_t>(effect_itr->second.stack_operation)+1);
            o.effect = &effect_itr->second;
            o.data.uval = SIZE_MAX;
            operations.push_back(o);
          }
        }
      }
    }
    
    std::pair<const core::event*, uint32_t> parse_event(const sol::table &table) {
      const bool ret = check_valid_event(table);
      if (!ret) return std::make_pair(nullptr, UINT32_MAX);
      
      auto cont = global::get<action_container>();
      
      const enum target_data::type target = table["target"];
      
      core::event* ev;
      //ev->name_id = 
      if (table["mean_time"].valid()) {
        ev->mtth = table["mean_time"];
      } else {
        ev->mtth = SIZE_MAX;
      }
      
      std::vector<functions_container::operation> conditions;
      const auto &pot_table = table["potential"].get<sol::table>();
      cont->creation_funcs.at("potential").func("potential", pot_table, target, conditions);
      //add_nesting_table_recursive(true, "potential", pot_table, target, conditions);
      
      ASSERT(conditions.size() < core::max_conditions_count);
      
      for (uint32_t i = 0; i < conditions.size(); ++i) {
        ev->conditions[i] = std::move(conditions[i]);
      }
      ev->conditions_count = conditions.size();
      
      size_t counter = 0;
      const auto &opt_table = table["options"].get<sol::table>();
      for (auto itr = opt_table.begin(); itr != opt_table.end(); ++itr) {
        if (!(*itr).second.is<sol::table>()) continue;
        
        const auto &opt = (*itr).second.as<sol::table>();
        for (auto it = opt.begin(); it != opt.end(); ++it) {
          //ev->options[counter].name = ;
          if ((*itr).first.is<std::string>() && (*itr).second.is<std::string>() && (*itr).first.as<std::string>() == "name") {
            const std::string name = (*itr).second.as<std::string>();
            
          }
          
          if ((*itr).first.is<std::string>() && (*itr).second.is<sol::table>()  && (*itr).first.as<std::string>() == "potential") {
            std::vector<functions_container::operation> conditions;
            const auto &potential_table = (*itr).second.as<sol::table>();
            cont->creation_funcs.at("potential").func("potential", potential_table, target, conditions);
            //add_nesting_table_recursive(true, "potential", potential_table, target, conditions);
            ASSERT(conditions.size() < core::max_conditions_count);
            for (uint32_t i = 0; i < conditions.size(); ++i) {
              ev->options[counter].conditions[i] = std::move(conditions[i]);
            }
            ev->options[counter].conditions_count = conditions.size();
          }
          
          if ((*itr).first.is<std::string>() && (*itr).second.is<sol::table>()  && (*itr).first.as<std::string>() == "effects") {
            std::vector<functions_container::operation> conditions;
            const auto &effects_table = (*itr).second.as<sol::table>();
            cont->creation_funcs.at("effects").func("effects", effects_table, target, conditions);
//             add_nesting_table_recursive(false, "effects", effects_table, target, conditions);
            ASSERT(conditions.size() < core::max_effects_count);
            for (uint32_t i = 0; i < conditions.size(); ++i) {
              ev->options[counter].effects[i] = std::move(conditions[i]);
            }
            ev->options[counter].effects_count = conditions.size();
          }
        }
        
        ++counter;
      }
      
      // должно быть два стака: стак выполнения, и стак данных который перейдет дальше по вызову эвентов
      // стак можно использовать чтобы хранить несколько таргетов decision'а
      // нужно придумать адекватные способы манипуляции со стаком и обрамления блоков команд
      // то есть у нас есть операция пихнуть в стак, после ее окончания мы должны взять из стака значение обратно
      // по идее стак можно использовать чтобы передавать значения для эвентов или для объявления войны по цб
      // то есть должен быть еще стек данных? стек мы можем собрать при выполнении, нужно принимать стек в функции?
      // видимо, по идее достаточно соблюдать порядок стека, а значит нужно где то указать последовательность команд
      // этот стек только для того чтобы собрать данные для сложных функций
//       uint32_t stack_size = 0;
//       uint32_t local_stack_size = 0;
//       uint32_t operation_stack_size = 0;
//       uint32_t data_stack_size = 0;
//       core::target_data stack[8]; // этот стак пойдет в следующий эвент в цепочке
//       core::target_data local_stack[8];
//       std::pair<operation, bool> operation_stack[8];
//       func_data_container data_stack[16];
//       
//       core::target_data current;
//       operation current_operation = op_and;
//       bool current_ret = true;
//       
//       for () {
//         // начинаем обходить 
//         // сначала стак пустой 
//         
//       }
    }
    
    std::pair<const core::event*, uint32_t> parse_event_and_save(sol::this_state lua, const sol::table &table) {
      const auto &pair = parse_event(table);
      if (pair.first == nullptr) throw std::runtime_error("Could not parse event");
      
      // можно сериалиазовать луа таблицу вместо непосредственного core::event
      // нормального способа сериализовать std function нет
      // как перестроить core::event для целей сериализации я не понимаю (нужно делать вложенность и хранилище данных)
      // самым легким способом будет использовать https://github.com/pkulchenko/serpent для сериализации
      // можно ли чекать эвент в луа? мы не можем чекать эвент в луа потому что я хочу часть вычислений сделать в мультитрединге
      // мы можем использовать мультитрединг в луа только если напишем всю игру в луа, что очень не хотелось бы
      // возможно когда нибудь я и преведу игру в луа, но не сегодня!
      
      //const std::string id = table["id"];
      
      sol::state_view l(lua);
      auto serializator = l["serpent"];
      if (!serializator.valid()) throw std::runtime_error("Serpent not loaded!");
      sol::function block = serializator["block"]; // нужно покрутить настройки
      std::string content = block(table); // теперь легко запихиваем в протобаф
      
      // нужно решить пихать ли все в один сейв (информация об использованных модах, сгенерированные данные, состояние игры)
      // или же создавать папочки с конфигурацией и с сейвами
      
      return pair;
    }
  }
}
