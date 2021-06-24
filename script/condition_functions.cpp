#include "condition_functions.h"

//#include "bin/core_structures.h"
#include "core/structures_header.h"
#include "utils/traits_modifier_attribs.h"
#include "re2/re2.h"

namespace devils_engine {
  namespace script {
    static const RE2 regex_obj(complex_var_regex);
    
//     std::tuple<uint32_t, uint32_t> get_number_type_data(const uint8_t &data) {
//       return std::make_tuple(data >> 4, data & 0xf);
//     }
    
    static bool get_bool_from_data(const context &ctx, const script_data &data) {
      const uint8_t number_type = data.number_type;
      bool final_value = false;
      switch (number_type) {
        case number_type::boolean: {
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
            if (number_type != number_type::boolean) throw std::runtime_error("Bad scope value");
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
          if (number_type != number_type::boolean) throw std::runtime_error("Bad scope value " + std::string(str));
          final_value = val.value;
          break;
        }
        default: throw std::runtime_error("Bad value type");
      }
      
      return final_value;
    }
    
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
    
    static std::tuple<double, uint8_t, uint16_t> get_num_from_data(const struct target &target, const context &ctx, const script_data &data) {
      const uint8_t number_type = data.number_type;
      double final_value = 0.0;
      uint8_t compare_type = UINT8_MAX;
      uint16_t special_stat = UINT16_MAX;
      switch (number_type) {
        case number_type::number: {
          final_value = data.value;
          compare_type = data.compare_type;
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
            if (number_type != number_type::boolean) throw std::runtime_error("Bad scope value");
            final_value = val.value;
            compare_type = val.compare_type;
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
          compare_type = val.compare_type;
          break;
        }
        case number_type::stat: {
          compare_type = data.compare_type;
          special_stat = data.helper2;
          const double mult = data.value;
          final_value = get_value_from_special_data(target, special_stat, mult);
          break;
        }
        
        default: throw std::runtime_error("Bad value type");
      }
      
      assert(compare_type != UINT8_MAX);
      return std::make_tuple(final_value, compare_type, special_stat);
    }
    
    bool compare_two_num_int(const int64_t &first, const int64_t &second, const uint8_t &compare_type) {
      switch (compare_type) {
        case number_compare_type::equal:     return first == second;
        case number_compare_type::not_equal: return first != second;
        case number_compare_type::less:      return first <  second;
        case number_compare_type::more:      return first >  second;
        case number_compare_type::less_eq:   return first <= second;
        case number_compare_type::more_eq:   return first >= second;
        default: throw std::runtime_error("Add new comparison");
      }
      
      return false;
    }
    
    bool compare_two_num_float(const double &first, const double &second, const uint8_t &compare_type) {
      const bool eq = std::abs(first - second) < EPSILON;
      switch (compare_type) {
        case number_compare_type::equal:     return eq;
        case number_compare_type::not_equal: return !eq;
        case number_compare_type::less:      return first < second;
        case number_compare_type::more:      return first > second;
        case number_compare_type::less_eq:   return first < second || eq;
        case number_compare_type::more_eq:   return first > second || eq;
        default: throw std::runtime_error("Add new comparison");
      }
      
      return false;
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
    
    void call_lua_func_bool(const struct target &t, const context &ctx, const script_data &data, const bool &value, const bool &res) {
      if (ctx.itr_func == nullptr) return;
      
      const auto &func = *ctx.itr_func;
      lua_State* state = func.lua_state();
      // нужно передать название
      const auto name = magic_enum::enum_name(static_cast<condition_function::value>(data.helper1));
      // нужно отправить таргета
      // возможно нужно передать данные, какие? размер массива или число или указатель на объект
      // причем нужно отправить вычисленное значение? к вычесленному значению нужно отправить 
      // тип сравнения и дополнительный тип переменной (например указать что это месячный инком)
      const sol::object target_obj = make_object_from_target(state, t);
      
      func(name, target_obj, value, sol::nil, sol::nil, res);
    }
    
    // хотя я подумал что ля локализации мне лучше будет все в таблицу оформлять для сложных функций
    // так я сразу могу взять нужные данные
    void call_lua_func_value(const struct target &t, const context &ctx, const script_data &data, const double &value, const uint8_t &compare_type, const uint16_t &special_stat, const bool &res) {
      if (ctx.itr_func == nullptr) return;
      
      const auto &func = *ctx.itr_func;
      lua_State* state = func.lua_state();
      // нужно передать название
      const auto name = magic_enum::enum_name(static_cast<condition_function::value>(data.helper1));
      // нужно отправить таргета
      // возможно нужно передать данные, какие? размер массива или число или указатель на объект
      // причем нужно отправить вычисленное значение? к вычесленному значению нужно отправить 
      // тип сравнения и дополнительный тип переменной (например указать что это месячный инком)
      const sol::object target_obj = make_object_from_target(state, t);
      const sol::object special_stat_obj = special_stat == UINT16_MAX ? sol::nil : sol::make_object(state, special_stat);
      func(name, target_obj, value, compare_type, special_stat_obj, res);
    }
    
    bool is_ai(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const bool final_value = get_bool_from_data(ctx, data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const bool ret = character->is_player() != final_value;
      call_lua_func_bool(target, ctx, data[0], final_value, ret);
      return ret;
    }
    
    bool is_player(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const bool final_value = get_bool_from_data(ctx, data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const bool ret = character->is_player() == final_value;
      call_lua_func_bool(target, ctx, data[0], final_value, ret);
      return ret;
    }
    
    bool is_independent(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const bool final_value = get_bool_from_data(ctx, data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const bool ret = character->is_independent() == final_value;
      call_lua_func_bool(target, ctx, data[0], final_value, ret);
      return ret;
    }
    
    bool is_vassal(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const bool final_value = get_bool_from_data(ctx, data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const bool ret = character->is_player() == final_value;
      call_lua_func_bool(target, ctx, data[0], final_value, ret);
      return character->is_independent() != final_value;
    }
    
    bool is_male(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const bool final_value = get_bool_from_data(ctx, data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const bool ret = character->is_player() == final_value;
      call_lua_func_bool(target, ctx, data[0], final_value, ret);
      return character->is_male() == final_value;
    }
    
    bool is_female(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const bool final_value = get_bool_from_data(ctx, data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const bool ret = character->is_player() == final_value;
      call_lua_func_bool(target, ctx, data[0], final_value, ret);
      return character->is_male() != final_value;
    }
    
    bool is_prisoner(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const bool final_value = get_bool_from_data(ctx, data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const bool ret = character->is_player() == final_value;
      call_lua_func_bool(target, ctx, data[0], final_value, ret);
      return character->is_prisoner() == final_value;
    }
    
    bool is_married(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const bool final_value = get_bool_from_data(ctx, data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const bool ret = character->is_player() == final_value;
      call_lua_func_bool(target, ctx, data[0], final_value, ret);
      return character->is_married() == final_value;
    }
    
    bool is_sick(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const bool final_value = get_bool_from_data(ctx, data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      
      bool sick = false;
      for (const auto &pair : character->traits) {
        auto trait = pair;
        if (trait->get_attrib(utils::trait::is_disease)) {
          sick = true;
          break;
        }
      }
      
      const bool ret = sick == final_value;
      call_lua_func_bool(target, ctx, data[0], final_value, ret);
      return ret;
    }
    
    bool is_in_war(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const bool final_value = get_bool_from_data(ctx, data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      ASSERT(false); // как быть с войной? должен быть экран дипломатии похожий на тот что в европке
      UNUSED_VARIABLE(character);
      UNUSED_VARIABLE(final_value);
    }
    
    bool is_in_society(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const bool final_value = get_bool_from_data(ctx, data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      ASSERT(false);
      UNUSED_VARIABLE(character);
      UNUSED_VARIABLE(final_value);
    }
    
    bool is_hero(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const bool final_value = get_bool_from_data(ctx, data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const bool ret = character->is_player() == final_value;
      call_lua_func_bool(target, ctx, data[0], final_value, ret);
      return character->is_hero() == final_value;
    }
    
    bool is_clan_head(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const bool final_value = get_bool_from_data(ctx, data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      ASSERT(false);
      UNUSED_VARIABLE(character);
      UNUSED_VARIABLE(final_value);
    }
    
    bool is_religious_head(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const bool final_value = get_bool_from_data(ctx, data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      ASSERT(false); // по идее указатель должен лежать в самой религии
      UNUSED_VARIABLE(character);
      UNUSED_VARIABLE(final_value);
    }
    
    bool is_father_alive(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const bool final_value = get_bool_from_data(ctx, data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      auto father = character->get_father(); // что делать с реальным отцом?
      const bool ret = father != nullptr ? father->is_dead() != final_value : false != final_value;
      call_lua_func_bool(target, ctx, data[0], final_value, ret);
      return ret;
    }
    
    bool is_mother_alive(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const bool final_value = get_bool_from_data(ctx, data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      auto mother = character->get_mother(); // что делать с реальной матерью?
      const bool ret = mother != nullptr ? mother->is_dead() != final_value : false != final_value;
      call_lua_func_bool(target, ctx, data[0], final_value, ret);
      return ret;
    }
    
    bool has_dead_friends(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, compare_type, special_stat] = get_num_from_data(target, ctx, data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const size_t cur = std::min(size_t(final_value), size_t(core::character::relations::max_game_friends));
      size_t counter = 0;
      for (uint32_t i = 0; i < core::character::relations::max_game_friends; ++i) {
        auto char_friend = character->relations.friends[i];
        counter += size_t(char_friend != nullptr && char_friend->is_dead());
      }
      
      //cur == 0 ? counter == 0 : counter >= cur
      const bool ret = compare_two_num_int(counter, cur, compare_type);
      // сюда можно еще передать специальное значение, что это за специалльное значение? 
      // это например стат, или специально вычисленный стат (годовой доход)
      call_lua_func_value(target, ctx, data[0], cur, compare_type, special_stat, ret);
      return ret; // тут поди сравнение должно задаваться в скрипте?
    }
    
    bool has_dead_rivals(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, compare_type, special_stat] = get_num_from_data(target, ctx, data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const size_t cur = std::min(size_t(final_value), size_t(core::character::relations::max_game_rivals));
      size_t counter = 0;
      for (uint32_t i = 0; i < core::character::relations::max_game_rivals; ++i) {
        auto char_rival = character->relations.rivals[i];
        counter += size_t(char_rival != nullptr && char_rival->is_dead());
      }
      
      const bool ret = compare_two_num_int(counter, cur, compare_type);
      call_lua_func_value(target, ctx, data[0], cur, compare_type, special_stat, ret);
      return ret;
    }
    
    bool has_owner(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const bool final_value = get_bool_from_data(ctx, data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const bool ret = (character->family.owner != nullptr) == final_value;
      call_lua_func_bool(target, ctx, data[0], final_value, ret);
      return ret;
    }
    
    bool has_dead_lovers(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, compare_type, special_stat] = get_num_from_data(target, ctx, data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const size_t cur = std::min(size_t(final_value), size_t(core::character::relations::max_game_lovers));
      size_t counter = 0;
      for (uint32_t i = 0; i < core::character::relations::max_game_lovers; ++i) {
        auto char_lover = character->relations.lovers[i];
        counter += size_t(char_lover != nullptr && char_lover->is_dead());
      }
      
      const bool ret = compare_two_num_int(counter, cur, compare_type);
      call_lua_func_value(target, ctx, data[0], cur, compare_type, special_stat, ret);
      return ret;
    }
    
    bool has_dead_brothers(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, compare_type, special_stat] = get_num_from_data(target, ctx, data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const size_t cur = final_value;
      auto current_char = character->family.next_sibling;
      size_t counter = 0;
      while (current_char != character) {
        counter += size_t(current_char->is_dead() && current_char->is_male());
        current_char = current_char->family.next_sibling;
      }
      
      const bool ret = compare_two_num_int(counter, cur, compare_type);
      call_lua_func_value(target, ctx, data[0], cur, compare_type, special_stat, ret);
      return ret;
    }
    
    bool has_dead_sisters(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, compare_type, special_stat] = get_num_from_data(target, ctx, data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const size_t cur = final_value;
      auto current_char = character->family.next_sibling;
      size_t counter = 0;
      while (current_char != character) {
        counter += size_t(current_char->is_dead() && !current_char->is_male());
        current_char = current_char->family.next_sibling;
      }
      
      const bool ret = compare_two_num_int(counter, cur, compare_type);
      call_lua_func_value(target, ctx, data[0], cur, compare_type, special_stat, ret);
      return ret;
    }
    
    bool has_dead_siblings(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, compare_type, special_stat] = get_num_from_data(target, ctx, data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const size_t cur = final_value;
      auto current_char = character->family.next_sibling;
      size_t counter = 0;
      while (current_char != character) {
        counter += size_t(current_char->is_dead());
        current_char = current_char->family.next_sibling;
      }
      
      const bool ret = compare_two_num_int(counter, cur, compare_type);
      call_lua_func_value(target, ctx, data[0], cur, compare_type, special_stat, ret);
      return ret;
    }
    
    bool has_dead_childs(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, compare_type, special_stat] = get_num_from_data(target, ctx, data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const size_t cur = final_value;
      auto current_child = character->family.children;
      if (current_child == nullptr) return cur == 0;
      
      auto next_child = current_child->family.next_sibling;
      size_t counter = 1;
      while (current_child != next_child) {
        if (next_child->family.parents[0] == character || next_child->family.parents[1] == character) counter += size_t(next_child->is_dead());
        next_child = next_child->family.next_sibling;
      }
      
      const bool ret = compare_two_num_int(counter, cur, compare_type);
      call_lua_func_value(target, ctx, data[0], cur, compare_type, special_stat, ret);
      return ret;
    }
    
    bool has_concubines(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, compare_type, special_stat] = get_num_from_data(target, ctx, data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const size_t cur = final_value;
      
      auto current_concubine = character->family.concubines;
      if (current_concubine == nullptr) return cur == 0;
      ASSERT(current_concubine->family.owner == character);
      size_t counter = 1;
      auto next_concubine = current_concubine->family.concubines;
      while (next_concubine != nullptr) {
        ++counter;
        next_concubine = next_concubine->family.concubines;
      }
      
      const bool ret = compare_two_num_int(counter, cur, compare_type);
      call_lua_func_value(target, ctx, data[0], cur, compare_type, special_stat, ret);
      return ret;
    }
    
    bool has_alive_friends(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, compare_type, special_stat] = get_num_from_data(target, ctx, data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const size_t cur = std::min(size_t(final_value), size_t(core::character::relations::max_game_friends));
      size_t counter = 0;
      for (uint32_t i = 0; i < core::character::relations::max_game_friends; ++i) {
        auto char_friend = character->relations.friends[i];
        counter += size_t(char_friend != nullptr && !char_friend->is_dead());
      }
      
      const bool ret = compare_two_num_int(counter, cur, compare_type);
      call_lua_func_value(target, ctx, data[0], cur, compare_type, special_stat, ret);
      return ret;
    }
    
    bool has_alive_rivals(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, compare_type, special_stat] = get_num_from_data(target, ctx, data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const size_t cur = std::min(size_t(final_value), size_t(core::character::relations::max_game_rivals));
      size_t counter = 0;
      for (uint32_t i = 0; i < core::character::relations::max_game_rivals; ++i) {
        auto char_rival = character->relations.rivals[i];
        counter += size_t(char_rival != nullptr && !char_rival->is_dead());
      }
      
      const bool ret = compare_two_num_int(counter, cur, compare_type);
      call_lua_func_value(target, ctx, data[0], cur, compare_type, special_stat, ret);
      return ret;
    }
    
    bool has_alive_lovers(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, compare_type, special_stat] = get_num_from_data(target, ctx, data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const size_t cur = std::min(size_t(final_value), size_t(core::character::relations::max_game_lovers));
      size_t counter = 0;
      for (uint32_t i = 0; i < core::character::relations::max_game_lovers; ++i) {
        auto char_lover = character->relations.lovers[i];
        counter += size_t(char_lover != nullptr && !char_lover->is_dead());
      }
      
      const bool ret = compare_two_num_int(counter, cur, compare_type);
      call_lua_func_value(target, ctx, data[0], cur, compare_type, special_stat, ret);
      return ret;
    }
    
    bool has_alive_brothers(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, compare_type, special_stat] = get_num_from_data(target, ctx, data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const size_t cur = final_value;
      auto current_char = character->family.next_sibling;
      size_t counter = 0;
      while (current_char != character) {
        counter += size_t(!current_char->is_dead() && current_char->is_male());
        current_char = current_char->family.next_sibling;
      }
      
      const bool ret = compare_two_num_int(counter, cur, compare_type);
      call_lua_func_value(target, ctx, data[0], cur, compare_type, special_stat, ret);
      return ret;
    }
    
    bool has_alive_sisters(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, compare_type, special_stat] = get_num_from_data(target, ctx, data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const size_t cur = final_value;
      auto current_char = character->family.next_sibling;
      size_t counter = 0;
      while (current_char != character) {
        counter += size_t(!current_char->is_dead() && !current_char->is_male());
        current_char = current_char->family.next_sibling;
      }
      
      const bool ret = compare_two_num_int(counter, cur, compare_type);
      call_lua_func_value(target, ctx, data[0], cur, compare_type, special_stat, ret);
      return ret;
    }
    
    bool has_alive_siblings(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, compare_type, special_stat] = get_num_from_data(target, ctx, data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const size_t cur = final_value;
      auto current_char = character->family.next_sibling;
      size_t counter = 0;
      while (current_char != character) {
        counter += size_t(!current_char->is_dead());
        current_char = current_char->family.next_sibling;
      }
      
      const bool ret = compare_two_num_int(counter, cur, compare_type);
      call_lua_func_value(target, ctx, data[0], cur, compare_type, special_stat, ret);
      return ret;
    }
    
    bool has_alive_childs(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, compare_type, special_stat] = get_num_from_data(target, ctx, data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const size_t cur = final_value;
      auto current_child = character->family.children;
      if (current_child == nullptr) return cur == 0;
      
      auto next_child = current_child->family.next_sibling;
      size_t counter = 1;
      while (current_child != next_child) {
        if (next_child->family.parents[0] == character || next_child->family.parents[1] == character) counter += size_t(!next_child->is_dead());
        next_child = next_child->family.next_sibling;
      }
      
      const bool ret = compare_two_num_int(counter, cur, compare_type);
      call_lua_func_value(target, ctx, data[0], cur, compare_type, special_stat, ret);
      return ret;
    }
    
    bool has_dynasty(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const bool final_value = get_bool_from_data(ctx, data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const bool cur = final_value;
      const bool ret = (character->family.dynasty != nullptr) == cur;
      call_lua_func_bool(target, ctx, data[0], final_value, ret);
      return ret;
    }
    
    bool can_change_religion(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      assert(false);
      // какая тут проверка? есть ли скрытая религия?
    }
    
    bool can_call_crusade(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      assert(false);
      // если это глава религии, если религия позволяет это... и все? а не глава религии может ли?
    }
    
    bool can_grant_title(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      assert(false);
      // может дать титул, какой?
    }
    
    bool can_marry(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const bool final_value = get_bool_from_data(ctx, data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      if (character->is_married()) return final_value == false;
      bool cur = !character->is_married();
      
      for (const auto &pair : character->traits) {
        auto trait = pair;
        //if (trait->get_attrib(utils::trait::cannot_marry)) return final_value == false;
        cur = cur && !trait->get_attrib(utils::trait::cannot_marry);
      }
      
      const bool ret = final_value == cur;
      call_lua_func_bool(target, ctx, data[0], final_value, ret);
      return ret;
    }
    
    bool belongs_to_culture(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      assert(false);
      // тут нужно передать либо индекс либо указатель на культуру
    }
    
    bool belongs_to_culture_group(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      assert(false);
      // тут нужно передать либо индекс либо указатель на культуру
    }
    
    bool has_trait(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      
    }
    
    bool has_modificator(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      
    }
    
    bool has_flag(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      
    }
    
    bool has_title(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      
    }
    
    bool has_nickname(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      
    }
    
    bool bit_is_set(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      
    }
    
    bool bit_is_unset(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      
    }
    
    bool realm_has_enacted_law(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      
    }
    
    bool realm_has_law_mechanic(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      
    }
    
    bool is_among_most_powerful_vassals(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      
    }
    
    bool age(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, compare_type, special_stat] = get_num_from_data(target, ctx, data[0]);
      const size_t age = size_t(final_value);
      auto character = reinterpret_cast<core::character*>(target.data);
      //const uint8_t compare_type = data[0].compare_type;
      switch (compare_type) {
        case number_compare_type::equal: {
          // нужно добавить вычисление возраста персонажу
          break;
        }
      }
    }
    
    bool money(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, compare_type, special_stat] = get_num_from_data(target, ctx, data[0]);
      const double money = final_value;
      auto character = reinterpret_cast<core::character*>(target.data);
      //const uint8_t compare_type = data[0].compare_type;
      // тут может быть несколько констант и команд на вычисление суммы
      // команды и константы можно задать с помощью строки, и потом скомпилировать в enum
      // но для этого нужно больше данных хранить, придется задавать данные в описание 
      // десижена или эвента объектом (это небольшая проблема)
      const float current_money = character->stat(core::character_stats::money);
      const bool ret = compare_two_num_float(current_money, money, compare_type);
      call_lua_func_value(target, ctx, data[0], current_money, compare_type, special_stat, ret);
      return ret;
    }
    
    bool character_stat_checker_float(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data, const uint32_t &stat) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      assert(stat < core::character_stats::count);
      assert(core::character_stats::types[stat] == core::stat_type::float_t);
      
      const auto [final_value, compare_type, special_stat] = get_num_from_data(target, ctx, data[0]);
      const double stat_val = final_value;
      auto character = reinterpret_cast<core::character*>(target.data);
      const double current_stat = character->stat(stat);
      const bool ret = compare_two_num_float(current_stat, stat_val, compare_type);
      call_lua_func_value(target, ctx, data[0], current_stat, compare_type, special_stat, ret);
      return ret;
    }
    
    bool character_stat_checker_int(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data, const uint32_t &stat) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      assert(stat < core::character_stats::count);
      assert(core::character_stats::types[stat] == core::stat_type::int_t || core::character_stats::types[stat] == core::stat_type::uint_t);
      
      const auto [final_value, compare_type, special_stat] = get_num_from_data(target, ctx, data[0]);
      const int64_t stat_val = final_value;
      auto character = reinterpret_cast<core::character*>(target.data);
      const int64_t current_stat = character->stat(stat);
      const bool ret = compare_two_num_int(current_stat, stat_val, compare_type);
      call_lua_func_value(target, ctx, data[0], current_stat, compare_type, special_stat, ret);
      return ret;
    }
    
    bool character_stat_checker(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data, const uint32_t &stat) {
      typedef bool(*func_t)(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data, const uint32_t &stat);
      
      static const func_t stat_funcs[] = {
        &character_stat_checker_int,
        &character_stat_checker_int,
        &character_stat_checker_float
      };
      
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(stat < core::character_stats::count);
      
      const uint32_t index = core::character_stats::types[stat];
      return stat_funcs[index](target, ctx, count, data, stat);
    }
    
    // функции в общем списке можно разделить на функции с бул инпутом, числом, строкой и сложные функции где придется самому описывать
    // но это по большому счету подходит только для инит
#define CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(name) bool name(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) \
  { return character_stat_checker(target, ctx, count, data, core::character_stats::name); }
    
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(military)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(managment)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(diplomacy)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(health)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(fertility)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(strength)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(agility)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(intellect)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(PENALTY_STAT(military))
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(PENALTY_STAT(managment))
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(PENALTY_STAT(diplomacy))
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(PENALTY_STAT(health))
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(PENALTY_STAT(fertility))
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(PENALTY_STAT(strength))
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(PENALTY_STAT(agility))
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(PENALTY_STAT(intellect))
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(demesne_size)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(ai_rationality)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(ai_zeal)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(ai_greed)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(ai_honor)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(ai_ambition)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(authority_income_mod)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(esteem_income_mod)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(influence_income_mod)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(income_mod)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(authority_income)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(esteem_income)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(influence_income)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(income)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(authority)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(esteem)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(influence)
    
#undef CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM
    
    void bool_variable_init(const std::string_view &name, const sol::object &obj, script_data* data) {
      switch (obj.get_type()) {
        case sol::type::boolean: {
          const auto val = obj.as<bool>();
          data->number_type = number_type::boolean;
          data->value = val;
          break;
        }
        
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
          
          if (RE2::FullMatch(str, regex_obj, &type1, &type2, &num_str)) {
            const auto e1 = magic_enum::enum_cast<data_source_type::values>(type1);
            if (!e1) throw std::runtime_error("Could not parse string data type " + type1);
            source = e1.value();
            const auto e2 = magic_enum::enum_cast<data_type::values>(type2);
            if (!e2) throw std::runtime_error("Could not parse string data type " + type2);
            value_data = e2.value();
            
            assert(!num_str.empty());
            num = std::atof(num_str.c_str());
          } else throw std::runtime_error("Could not parse string '" + std::string(str) + "'");
          
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
          
          switch (source) {
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
            
            default: throw std::runtime_error("Bad input variable type for " + std::string(name));
          }
          
          break;
        }
        
        default: throw std::runtime_error("Bad input variable type for " + std::string(name));
      }
    }
    
    void variable_init(const std::string_view &name, const sol::object &obj, script_data* data) {
      switch (obj.get_type()) {
        case sol::type::number: {
          const auto val = obj.as<double>();
          data->number_type = number_type::number;
          data->compare_type = number_compare_type::more_eq;
          data->value = val;
          break;
        }
        
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
          
          if (RE2::FullMatch(str, regex_obj, &type1, &type2, &num_str)) {
            const auto e1 = magic_enum::enum_cast<data_source_type::values>(type1);
            if (!e1) throw std::runtime_error("Could not parse string data type " + type1);
            source = e1.value();
            const auto e2 = magic_enum::enum_cast<data_type::values>(type2);
            if (!e2) throw std::runtime_error("Could not parse string data type " + type2);
            value_data = e2.value();
            
            assert(!num_str.empty());
            num = std::atof(num_str.c_str());
          } else throw std::runtime_error("Could not parse string '" + std::string(str) + "'");
          
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
          
          switch (source) {
            case data_source_type::special: {
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
            
#define DATA_SOURCE_CASE(enum_name) case data_source_type::enum_name: { \
    data->number_type = number_type::stat; \
    data->compare_type = compare_type; \
    data->value = num; \
    data->helper2 = data_source_type::enum_name; \
    break; \
  }
  
            DATA_SOURCE_CASE(money)
            DATA_SOURCE_CASE(money_income)
            DATA_SOURCE_CASE(money_month_income)
            DATA_SOURCE_CASE(money_yearly_income)
            
            default: throw std::runtime_error(std::string(magic_enum::enum_name<data_type::values>(value_data)) + " not implemented yet");
          }
          
          break;
        }
        
        default: throw std::runtime_error("Bad input variable type for " + std::string(name));
      }
    }
    
    // пока что вход сделаем только бул
#define CONDITION_COMMAND_FUNC_INIT_BOOL(name) void name##_init(const uint32_t &, const sol::object &obj, script_data* data) { \
      if (obj.get_type() != sol::type::boolean && obj.get_type() != sol::type::string) throw std::runtime_error("Bad input for " + std::string(#name)); \
      data->command_type = command_type::condition; \
      data->helper1 = condition_function::name; \
      bool_variable_init(#name, obj, data); \
    }
    
//     void is_ai_init(const uint32_t &, const sol::object &obj, script_data* data) {
//       if (obj.get_type() != sol::type::boolean) throw std::runtime_error("Bad input for " + std::string("is_ai"));
//       data->command_type = command_type::condition;
//       data->helper1 = condition_function::is_ai;
// //       const auto val = obj.as<bool>();
// //       data->number_type = number_type::boolean;
// //       data->value = val;
//       variable_init("is_ai", obj, data);
//     }
    
    CONDITION_COMMAND_FUNC_INIT_BOOL(is_ai)
    CONDITION_COMMAND_FUNC_INIT_BOOL(is_player)
    CONDITION_COMMAND_FUNC_INIT_BOOL(is_independent)
    CONDITION_COMMAND_FUNC_INIT_BOOL(is_vassal)
    CONDITION_COMMAND_FUNC_INIT_BOOL(is_male)
    CONDITION_COMMAND_FUNC_INIT_BOOL(is_female)
    CONDITION_COMMAND_FUNC_INIT_BOOL(is_prisoner)
    CONDITION_COMMAND_FUNC_INIT_BOOL(is_married)
    CONDITION_COMMAND_FUNC_INIT_BOOL(is_sick)
    CONDITION_COMMAND_FUNC_INIT_BOOL(is_in_war)
    CONDITION_COMMAND_FUNC_INIT_BOOL(is_in_society)
    CONDITION_COMMAND_FUNC_INIT_BOOL(is_hero)
    CONDITION_COMMAND_FUNC_INIT_BOOL(is_clan_head)
    CONDITION_COMMAND_FUNC_INIT_BOOL(is_religious_head)
    CONDITION_COMMAND_FUNC_INIT_BOOL(is_father_alive)
    CONDITION_COMMAND_FUNC_INIT_BOOL(is_mother_alive)
    CONDITION_COMMAND_FUNC_INIT_BOOL(has_dynasty)
    CONDITION_COMMAND_FUNC_INIT_BOOL(has_owner)
    CONDITION_COMMAND_FUNC_INIT_BOOL(can_change_religion)
    CONDITION_COMMAND_FUNC_INIT_BOOL(can_call_crusade)
    CONDITION_COMMAND_FUNC_INIT_BOOL(can_grant_title)
    CONDITION_COMMAND_FUNC_INIT_BOOL(can_marry)
    CONDITION_COMMAND_FUNC_INIT_BOOL(is_among_most_powerful_vassals)
    
#undef CONDITION_COMMAND_FUNC_INIT_BOOL
    
    void belongs_to_culture_init(const uint32_t &, const sol::object &obj, script_data* data) {
      // вход - строка, по которой мы можем сразу получить указатель на культуру
    }
    
    void belongs_to_culture_group_init(const uint32_t &, const sol::object &obj, script_data* data) {
      
    }
    
    void has_trait_init(const uint32_t &, const sol::object &obj, script_data* data) {
      
    }
    
    void has_modificator_init(const uint32_t &, const sol::object &obj, script_data* data) {
      
    }
    
    void has_flag_init(const uint32_t &, const sol::object &obj, script_data* data) {
      
    }
    
    void has_title_init(const uint32_t &, const sol::object &obj, script_data* data) {
      
    }
    
    // как никнейм указать? если мы выделим id никнейма без индекса, то мы можем здесь пихнуть сравнение ключа локализации
    void has_nickname_init(const uint32_t &, const sol::object &obj, script_data* data) {
      
    }
    
    // это не нужно
    void bit_is_set_init(const uint32_t &, const sol::object &obj, script_data* data) {
      
    }
    
    void bit_is_unset_init(const uint32_t &, const sol::object &obj, script_data* data) {
      
    }
    
    // есть закон как отдельная сущность, там может быть неколько механик прав и статы для государства
    void realm_has_enacted_law_init(const uint32_t &, const sol::object &obj, script_data* data) {
      
    }
    
    // есть механика закона (реалм работает по этим правилам)
    void realm_has_law_mechanic_init(const uint32_t &, const sol::object &obj, script_data* data) {
      
    }
    
    
#define CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(name) void name##_init(const uint32_t &target_type, const sol::object &obj, script_data* data) { \
      if (obj.get_type() != sol::type::number && obj.get_type() != sol::type::string) throw std::runtime_error("Bad input for " + std::string(#name)); \
      if (target_type != static_cast<uint32_t>(core::character::s_type)) throw std::runtime_error("Current target must be character"); \
      data->command_type = command_type::condition; \
      data->helper1 = condition_function::name; \
      variable_init(#name, obj, data); \
    }
    
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(has_dead_friends)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(has_dead_rivals)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(has_dead_lovers)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(has_dead_brothers)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(has_dead_sisters)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(has_dead_siblings)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(has_dead_childs)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(has_concubines)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(has_alive_friends)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(has_alive_rivals)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(has_alive_lovers)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(has_alive_brothers)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(has_alive_sisters)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(has_alive_siblings)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(has_alive_childs)
    
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(age)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(money)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(military)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(managment)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(diplomacy)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(health)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(fertility)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(strength)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(agility)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(intellect)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(military_penalty)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(managment_penalty)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(diplomacy_penalty)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(health_penalty)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(fertility_penalty)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(strength_penalty)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(agility_penalty)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(intellect_penalty)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(demesne_size)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(ai_rationality)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(ai_zeal)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(ai_greed)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(ai_honor)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(ai_ambition)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(authority_income_mod)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(esteem_income_mod)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(influence_income_mod)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(income_mod)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(authority_income)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(esteem_income)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(influence_income)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(income)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(authority)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(esteem)
    CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM(influence)
  
#undef CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM
    
    
  }
}
