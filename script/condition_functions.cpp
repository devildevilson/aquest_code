#include "condition_functions.h"

//#include "bin/core_structures.h"
#include "core/structures_header.h"
#include "utils/traits_modifier_attribs.h"
#include "re2/re2.h"
#include "utility.h"
#include "utils/magic_enum_header.h"

namespace devils_engine {
  namespace script {
    static const RE2 regex_obj(complex_var_regex);
    
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
    
    bool is_ai(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, special_stat, number_type, compare_type] = get_num_from_data(&target, ctx, &data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const bool original = !character->is_player();
      const bool ret = original == bool(final_value);
      call_lua_func(&target, ctx, &data[0], bool(final_value), original, ret);
      ctx->current_value.number_type = number_type::boolean;
      ctx->current_value.value = double(original);
      ctx->current_value.compare_type = compare_type;
      return ret;
    }
    
    bool is_player(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, special_stat, number_type, compare_type] = get_num_from_data(&target, ctx, &data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const bool original = character->is_player();
      const bool ret = original == bool(final_value);
      call_lua_func(&target, ctx, &data[0], bool(final_value), original, ret);
      ctx->current_value.number_type = number_type::boolean;
      ctx->current_value.value = double(original);
      ctx->current_value.compare_type = compare_type;
      return ret;
    }
    
    bool is_independent(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, special_stat, number_type, compare_type] = get_num_from_data(&target, ctx, &data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const bool original = character->is_independent();
      const bool ret = original == bool(final_value);
      call_lua_func(&target, ctx, &data[0], bool(final_value), original, ret);
      ctx->current_value.number_type = number_type::boolean;
      ctx->current_value.value = double(original);
      ctx->current_value.compare_type = compare_type;
      return ret;
    }
    
    bool is_vassal(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, special_stat, number_type, compare_type] = get_num_from_data(&target, ctx, &data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const bool original = !character->is_independent();
      const bool ret = original == bool(final_value);
      call_lua_func(&target, ctx, &data[0], bool(final_value), original, ret);
      ctx->current_value.number_type = number_type::boolean;
      ctx->current_value.value = double(original);
      ctx->current_value.compare_type = compare_type;
      return ret;
    }
    
    bool is_male(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, special_stat, number_type, compare_type] = get_num_from_data(&target, ctx, &data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const bool original = character->is_male();
      const bool ret = original == bool(final_value);
      call_lua_func(&target, ctx, &data[0], bool(final_value), original, ret);
      ctx->current_value.number_type = number_type::boolean;
      ctx->current_value.value = double(original);
      ctx->current_value.compare_type = compare_type;
      return ret;
    }
    
    bool is_female(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, special_stat, number_type, compare_type] = get_num_from_data(&target, ctx, &data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const bool original = !character->is_male();
      const bool ret = original == bool(final_value);
      call_lua_func(&target, ctx, &data[0], bool(final_value), original, ret);
      ctx->current_value.number_type = number_type::boolean;
      ctx->current_value.value = double(original);
      ctx->current_value.compare_type = compare_type;
      return ret;
    }
    
    bool is_prisoner(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, special_stat, number_type, compare_type] = get_num_from_data(&target, ctx, &data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const bool original = character->is_prisoner();
      const bool ret = original == bool(final_value);
      call_lua_func(&target, ctx, &data[0], bool(final_value), original, ret);
      ctx->current_value.number_type = number_type::boolean;
      ctx->current_value.value = double(original);
      ctx->current_value.compare_type = compare_type;
      return ret;
    }
    
    bool is_married(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, special_stat, number_type, compare_type] = get_num_from_data(&target, ctx, &data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const bool original = character->is_married();
      const bool ret = original == bool(final_value);
      call_lua_func(&target, ctx, &data[0], bool(final_value), original, ret);
      ctx->current_value.number_type = number_type::boolean;
      ctx->current_value.value = double(original);
      ctx->current_value.compare_type = compare_type;
      return ret;
    }
    
    bool is_sick(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, special_stat, number_type, compare_type] = get_num_from_data(&target, ctx, &data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      
      bool sick = false;
      for (const auto &pair : character->traits) {
        auto trait = pair;
        if (trait->get_attrib(utils::trait::is_disease)) {
          sick = true;
          break;
        }
      }
      
      const bool ret = sick == bool(final_value);
      call_lua_func(&target, ctx, &data[0], bool(final_value), sick, ret);
      ctx->current_value.number_type = number_type::boolean;
      ctx->current_value.value = double(sick);
      ctx->current_value.compare_type = compare_type;
      return ret;
    }
    
    bool is_in_war(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, special_stat, number_type, compare_type] = get_num_from_data(&target, ctx, &data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      ASSERT(false); // как быть с войной? должен быть экран дипломатии похожий на тот что в европке
      UNUSED_VARIABLE(character);
      UNUSED_VARIABLE(final_value);
      // тут нужно зайти в realm и почекать там есть ли война
      // хотя может быть эта функция вообще относится к государству а не к персонажу
      // придворные находятся ли в состоянии войны если их хозяин воюет?
    }
    
    // не уверен что будет присутствовать на старте игры
    bool is_in_society(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, special_stat, number_type, compare_type] = get_num_from_data(&target, ctx, &data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      ASSERT(false);
      UNUSED_VARIABLE(character);
      UNUSED_VARIABLE(final_value);
    }
    
    bool is_hero(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, special_stat, number_type, compare_type] = get_num_from_data(&target, ctx, &data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const bool original = character->is_hero();
      const bool ret = original == final_value;
      call_lua_func(&target, ctx, &data[0], bool(final_value), original, ret);
      ctx->current_value.number_type = number_type::boolean;
      ctx->current_value.value = double(original);
      ctx->current_value.compare_type = compare_type;
      return ret;
    }
    
    // государственный строй до икты (то есть муслимское племя) (?)
    bool is_clan_head(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, special_stat, number_type, compare_type] = get_num_from_data(&target, ctx, &data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      ASSERT(false);
      UNUSED_VARIABLE(character);
      UNUSED_VARIABLE(final_value);
    }
    
    bool is_religious_head(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, special_stat, number_type, compare_type] = get_num_from_data(&target, ctx, &data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      ASSERT(false); // по идее указатель должен лежать в самой религии
      UNUSED_VARIABLE(character);
      UNUSED_VARIABLE(final_value);
    }
    
    bool is_father_alive(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, special_stat, number_type, compare_type] = get_num_from_data(&target, ctx, &data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      auto father = character->get_father(); // что делать с реальным отцом?
      const bool original = !(father != nullptr ? father->is_dead() : true);
      const bool ret = original == final_value;
      call_lua_func(&target, ctx, &data[0], final_value, original, ret);
      ctx->current_value.number_type = number_type::boolean;
      ctx->current_value.value = double(original);
      ctx->current_value.compare_type = compare_type;
      return ret;
    }
    
    bool is_mother_alive(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, special_stat, number_type, compare_type] = get_num_from_data(&target, ctx, &data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      auto mother = character->get_mother(); // что делать с реальной матерью?
      const bool original = !(mother != nullptr ? mother->is_dead() : true);
      const bool ret = original == final_value;
      call_lua_func(&target, ctx, &data[0], final_value, original, ret);
      ctx->current_value.number_type = number_type::boolean;
      ctx->current_value.value = double(original);
      ctx->current_value.compare_type = compare_type;
      return ret;
    }
    
    bool has_dead_friends(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, special_stat, number_type, compare_type] = get_num_from_data(&target, ctx, &data[0]);
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
      call_lua_func(&target, ctx, &data[0], cur, compare_type, special_stat, counter, ret);
      ctx->current_value.number_type = number_type::number;
      ctx->current_value.value = double(counter);
      ctx->current_value.compare_type = compare_type;
      return ret; // тут поди сравнение должно задаваться в скрипте?
    }
    
    bool has_dead_rivals(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, special_stat, number_type, compare_type] = get_num_from_data(&target, ctx, &data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const size_t cur = std::min(size_t(final_value), size_t(core::character::relations::max_game_rivals));
      size_t counter = 0;
      for (uint32_t i = 0; i < core::character::relations::max_game_rivals; ++i) {
        auto char_rival = character->relations.rivals[i];
        counter += size_t(char_rival != nullptr && char_rival->is_dead());
      }
      
      const bool ret = compare_two_num_int(counter, cur, compare_type);
      call_lua_func(&target, ctx, &data[0], cur, compare_type, special_stat, counter, ret);
      ctx->current_value.number_type = number_type::number;
      ctx->current_value.value = double(counter);
      ctx->current_value.compare_type = compare_type;
      return ret;
    }
    
    bool has_owner(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, special_stat, number_type, compare_type] = get_num_from_data(&target, ctx, &data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const bool original = character->family.owner != nullptr;
      const bool ret = original == final_value;
      call_lua_func(&target, ctx, &data[0], final_value, original, ret);
      ctx->current_value.number_type = number_type::number;
      ctx->current_value.value = double(original);
      ctx->current_value.compare_type = compare_type;
      return ret;
    }
    
    bool has_dead_lovers(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, special_stat, number_type, compare_type] = get_num_from_data(&target, ctx, &data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const size_t cur = std::min(size_t(final_value), size_t(core::character::relations::max_game_lovers));
      size_t counter = 0;
      for (uint32_t i = 0; i < core::character::relations::max_game_lovers; ++i) {
        auto char_lover = character->relations.lovers[i];
        counter += size_t(char_lover != nullptr && char_lover->is_dead());
      }
      
      const bool ret = compare_two_num_int(counter, cur, compare_type);
      call_lua_func(&target, ctx, &data[0], cur, compare_type, special_stat, counter, ret);
      ctx->current_value.number_type = number_type::number;
      ctx->current_value.value = double(counter);
      ctx->current_value.compare_type = compare_type;
      return ret;
    }
    
    bool has_dead_brothers(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, special_stat, number_type, compare_type] = get_num_from_data(&target, ctx, &data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const size_t cur = final_value;
      auto current_char = character->family.next_sibling;
      size_t counter = 0;
      // это при условии что я сделал кольцевые указатели, а это не факт
      while (current_char != character) {
        counter += size_t(current_char->is_dead() && current_char->is_male());
        current_char = current_char->family.next_sibling;
      }
      
      const bool ret = compare_two_num_int(counter, cur, compare_type);
      call_lua_func(&target, ctx, &data[0], cur, compare_type, special_stat, counter, ret);
      ctx->current_value.number_type = number_type::number;
      ctx->current_value.value = double(counter);
      ctx->current_value.compare_type = compare_type;
      return ret;
    }
    
    bool has_dead_sisters(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, special_stat, number_type, compare_type] = get_num_from_data(&target, ctx, &data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const size_t cur = final_value;
      auto current_char = character->family.next_sibling;
      size_t counter = 0;
      while (current_char != character) {
        counter += size_t(current_char->is_dead() && !current_char->is_male());
        current_char = current_char->family.next_sibling;
      }
      
      const bool ret = compare_two_num_int(counter, cur, compare_type);
      call_lua_func(&target, ctx, &data[0], cur, compare_type, special_stat, counter, ret);
      ctx->current_value.number_type = number_type::number;
      ctx->current_value.value = double(counter);
      ctx->current_value.compare_type = compare_type;
      return ret;
    }
    
    bool has_dead_siblings(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, special_stat, number_type, compare_type] = get_num_from_data(&target, ctx, &data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const size_t cur = final_value;
      auto current_char = character->family.next_sibling;
      size_t counter = 0;
      while (current_char != character) {
        counter += size_t(current_char->is_dead());
        current_char = current_char->family.next_sibling;
      }
      
      const bool ret = compare_two_num_int(counter, cur, compare_type);
      call_lua_func(&target, ctx, &data[0], cur, compare_type, special_stat, counter, ret);
      ctx->current_value.number_type = number_type::number;
      ctx->current_value.value = double(counter);
      ctx->current_value.compare_type = compare_type;
      return ret;
    }
    
    bool has_dead_childs(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, special_stat, number_type, compare_type] = get_num_from_data(&target, ctx, &data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const size_t cur = final_value;
      auto current_child = character->family.children;
      
      size_t counter = size_t(current_child->is_dead());
      if (current_child != nullptr) {
        auto next_child = current_child->family.next_sibling;
        while (current_child != next_child) {
          if (core::character::is_parent(character, next_child)) counter += size_t(next_child->is_dead());
          next_child = next_child->family.next_sibling;
        }
      }
      
      const bool ret = compare_two_num_int(counter, cur, compare_type);
      call_lua_func(&target, ctx, &data[0], cur, compare_type, special_stat, counter, ret);
      ctx->current_value.number_type = number_type::number;
      ctx->current_value.value = double(counter);
      ctx->current_value.compare_type = compare_type;
      return ret;
    }
    
    bool has_concubines(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, special_stat, number_type, compare_type] = get_num_from_data(&target, ctx, &data[0]);
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
      call_lua_func(&target, ctx, &data[0], cur, compare_type, special_stat, counter, ret);
      ctx->current_value.number_type = number_type::number;
      ctx->current_value.value = double(counter);
      ctx->current_value.compare_type = compare_type;
      return ret;
    }
    
    bool has_alive_friends(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, special_stat, number_type, compare_type] = get_num_from_data(&target, ctx, &data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const size_t cur = std::min(size_t(final_value), size_t(core::character::relations::max_game_friends));
      size_t counter = 0;
      for (uint32_t i = 0; i < core::character::relations::max_game_friends; ++i) {
        auto char_friend = character->relations.friends[i];
        counter += size_t(char_friend != nullptr && !char_friend->is_dead());
      }
      
      const bool ret = compare_two_num_int(counter, cur, compare_type);
      call_lua_func(&target, ctx, &data[0], cur, compare_type, special_stat, counter, ret);
      ctx->current_value.number_type = number_type::number;
      ctx->current_value.value = double(counter);
      ctx->current_value.compare_type = compare_type;
      return ret;
    }
    
    bool has_alive_rivals(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, special_stat, number_type, compare_type] = get_num_from_data(&target, ctx, &data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const size_t cur = std::min(size_t(final_value), size_t(core::character::relations::max_game_rivals));
      size_t counter = 0;
      for (uint32_t i = 0; i < core::character::relations::max_game_rivals; ++i) {
        auto char_rival = character->relations.rivals[i];
        counter += size_t(char_rival != nullptr && !char_rival->is_dead());
      }
      
      const bool ret = compare_two_num_int(counter, cur, compare_type);
      call_lua_func(&target, ctx, &data[0], cur, compare_type, special_stat, counter, ret);
      ctx->current_value.number_type = number_type::number;
      ctx->current_value.value = double(counter);
      ctx->current_value.compare_type = compare_type;
      return ret;
    }
    
    bool has_alive_lovers(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, special_stat, number_type, compare_type] = get_num_from_data(&target, ctx, &data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const size_t cur = std::min(size_t(final_value), size_t(core::character::relations::max_game_lovers));
      size_t counter = 0;
      for (uint32_t i = 0; i < core::character::relations::max_game_lovers; ++i) {
        auto char_lover = character->relations.lovers[i];
        counter += size_t(char_lover != nullptr && !char_lover->is_dead());
      }
      
      const bool ret = compare_two_num_int(counter, cur, compare_type);
      call_lua_func(&target, ctx, &data[0], cur, compare_type, special_stat, counter, ret);
      ctx->current_value.number_type = number_type::number;
      ctx->current_value.value = double(counter);
      ctx->current_value.compare_type = compare_type;
      return ret;
    }
    
    bool has_alive_brothers(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, special_stat, number_type, compare_type] = get_num_from_data(&target, ctx, &data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const size_t cur = final_value;
      auto current_char = character->family.next_sibling;
      size_t counter = 0;
      while (current_char != character) {
        counter += size_t(!current_char->is_dead() && current_char->is_male());
        current_char = current_char->family.next_sibling;
      }
      
      const bool ret = compare_two_num_int(counter, cur, compare_type);
      call_lua_func(&target, ctx, &data[0], cur, compare_type, special_stat, counter, ret);
      ctx->current_value.number_type = number_type::number;
      ctx->current_value.value = double(counter);
      ctx->current_value.compare_type = compare_type;
      return ret;
    }
    
    bool has_alive_sisters(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, special_stat, number_type, compare_type] = get_num_from_data(&target, ctx, &data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const size_t cur = final_value;
      auto current_char = character->family.next_sibling;
      size_t counter = 0;
      while (current_char != character) {
        counter += size_t(!current_char->is_dead() && !current_char->is_male());
        current_char = current_char->family.next_sibling;
      }
      
      const bool ret = compare_two_num_int(counter, cur, compare_type);
      call_lua_func(&target, ctx, &data[0], cur, compare_type, special_stat, counter, ret);
      ctx->current_value.number_type = number_type::number;
      ctx->current_value.value = double(counter);
      ctx->current_value.compare_type = compare_type;
      return ret;
    }
    
    bool has_alive_siblings(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, special_stat, number_type, compare_type] = get_num_from_data(&target, ctx, &data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const size_t cur = final_value;
      auto current_char = character->family.next_sibling;
      size_t counter = 0;
      while (current_char != character) {
        counter += size_t(!current_char->is_dead());
        current_char = current_char->family.next_sibling;
      }
      
      const bool ret = compare_two_num_int(counter, cur, compare_type);
      call_lua_func(&target, ctx, &data[0], cur, compare_type, special_stat, counter, ret);
      ctx->current_value.number_type = number_type::number;
      ctx->current_value.value = double(counter);
      ctx->current_value.compare_type = compare_type;
      return ret;
    }
    
    bool has_alive_childs(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, special_stat, number_type, compare_type] = get_num_from_data(&target, ctx, &data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const size_t cur = final_value;
      auto current_child = character->family.children;
      
      size_t counter = size_t(current_child != nullptr);
      if (current_child != nullptr) {
        auto next_child = current_child->family.next_sibling;
        while (current_child != next_child) {
          if (core::character::is_parent(character, next_child)) counter += size_t(!next_child->is_dead());
          next_child = next_child->family.next_sibling;
        }
      }
      
      const bool ret = compare_two_num_int(counter, cur, compare_type);
      call_lua_func(&target, ctx, &data[0], cur, compare_type, special_stat, counter, ret);
      ctx->current_value.number_type = number_type::number;
      ctx->current_value.value = double(counter);
      ctx->current_value.compare_type = compare_type;
      return ret;
    }
    
    bool has_dynasty(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, special_stat, number_type, compare_type] = get_num_from_data(&target, ctx, &data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);
      const bool cur = final_value;
      const bool original = character->family.dynasty != nullptr;
      const bool ret = original == cur;
      call_lua_func(&target, ctx, &data[0], final_value, original, ret);
      ctx->current_value.number_type = number_type::boolean;
      ctx->current_value.value = double(original);
      ctx->current_value.compare_type = compare_type;
      return ret;
    }
    
    bool can_change_religion(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      assert(false);
      // какая тут проверка? есть ли скрытая религия?
    }
    
    bool can_call_crusade(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      assert(false);
      // если это глава религии, если религия позволяет это... и все? а не глава религии может ли?
    }
    
    bool can_grant_title(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      assert(false);
      // может дать титул, какой?
    }
    
    bool can_marry(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, special_stat, number_type, compare_type] = get_num_from_data(&target, ctx, &data[0]);
      auto character = reinterpret_cast<core::character*>(target.data);

      bool cur = !character->is_married();
      
      for (const auto &pair : character->traits) {
        auto trait = pair;
        //if (trait->get_attrib(utils::trait::cannot_marry)) return final_value == false;
        cur = cur && !trait->get_attrib(utils::trait::cannot_marry);
      }
      
      const bool ret = final_value == cur;
      call_lua_func(&target, ctx, &data[0], final_value, cur, ret);
      ctx->current_value.number_type = number_type::number;
      ctx->current_value.value = double(cur);
      ctx->current_value.compare_type = compare_type;
      return ret;
    }
    
    bool belongs_to_culture(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      assert(data[0].number_type != number_type::lvalue);
      assert(false);
      // тут нужно передать либо индекс либо указатель на культуру
    }
    
    bool belongs_to_culture_group(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      assert(data[0].number_type != number_type::lvalue);
      assert(false);
      // тут нужно передать либо индекс либо указатель на культуру
    }
    
    bool has_trait(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      // id или сам треит, если при инициализации указывается строка, то мы можем вполне держать здесь непосредственно треит
      // а если нет, то строку нужно сначала вычислить
    }
    
    bool has_modificator(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      
    }
    
    bool has_flag(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(data[0].command_type == command_type::condition);
      assert(data[0].helper1 == condition_function::has_flag);
      const auto str = get_string_from_data(&target, ctx, &data[0]);
      bool ret = false;
      
      const auto e = static_cast<core::structure>(target.type);
      switch (e) {
#define HAS_FLAG_CASE(name) case core::structure::name: {      \
        auto obj = reinterpret_cast<core::name*>(target.data); \
          ret = obj->has_flag(str);                            \
          break;                                               \
        }
        
        HAS_FLAG_CASE(army)
        HAS_FLAG_CASE(character)
        HAS_FLAG_CASE(city)
        HAS_FLAG_CASE(province)
        HAS_FLAG_CASE(realm)
        HAS_FLAG_CASE(titulus)
        HAS_FLAG_CASE(hero_troop)
        HAS_FLAG_CASE(war)
        // должно быть и в этих по идее
//         HAS_FLAG_CASE(dynasty)
//         HAS_FLAG_CASE(culture)
//         HAS_FLAG_CASE(religion)
        
#undef HAS_FLAG_CASE
        
        default: throw std::runtime_error(std::string(magic_enum::enum_name(e)) + " could not own a flag");
      }
      
      call_lua_func(&target, ctx, data, str, sol::nil, ret);
      ctx->current_value.number_type = number_type::boolean;
      ctx->current_value.value = double(ret);
      ctx->current_value.compare_type = number_compare_type::more_eq;
      return ret;
    }
    
    bool has_title(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      // таргет должен быть реалмом
    }
    
    bool has_nickname(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      
    }
    
    // скорее всего непотребуется
    bool bit_is_set(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      
    }
    
    bool bit_is_unset(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      
    }
    
    // переназвать и оставить для реалма
    bool realm_has_enacted_law(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      
    }
    
    bool realm_has_law_mechanic(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      
    }
    
    // реалм?
    bool is_among_most_powerful_vassals(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      
    }
    
    bool age(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      const auto [final_value, special_stat, number_type, compare_type] = get_num_from_data(&target, ctx, &data[0]);
      const size_t age = size_t(final_value);
      auto character = reinterpret_cast<core::character*>(target.data);
      //const uint8_t compare_type = data[0].compare_type;
      switch (compare_type) {
        case number_compare_type::equal: {
          // нужно добавить вычисление возраста персонажу
          break;
        }
      }
      
      // нужно добавить передачу значение в лвалуе
//       ctx->current_value.number_type = number_type::boolean;
//       ctx->current_value.value = double(character->family.dynasty != nullptr);
//       ctx->current_value.compare_type = compare_type;
    }
    
    bool character_stat_checker_float(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data, const uint32_t &stat) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      assert(stat < core::character_stats::count);
      assert(core::character_stats::types[stat] == core::stat_type::float_t);
      
      const auto [final_value, special_stat, number_type, compare_type] = get_num_from_data(&target, ctx, &data[0]);
      const double stat_val = final_value;
      auto character = reinterpret_cast<core::character*>(target.data);
      const double current_stat = character->current_stats.get(stat);
      const bool ret = compare_two_num_float(current_stat, stat_val, compare_type);
      call_lua_func(&target, ctx, &data[0], stat_val, compare_type, special_stat, current_stat, ret);
      ctx->current_value.number_type = number_type::number;
      ctx->current_value.value = double(current_stat);
      ctx->current_value.compare_type = compare_type;
      return ret;
    }
    
    bool character_stat_checker_int(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data, const uint32_t &stat) {
      assert(count == 1);
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      assert(data[0].command_type == command_type::condition);
      assert(stat < core::character_stats::count);
      assert(core::character_stats::types[stat] == core::stat_type::int_t || core::character_stats::types[stat] == core::stat_type::uint_t);
      
      const auto [final_value, special_stat, number_type, compare_type] = get_num_from_data(&target, ctx, &data[0]);
      const int64_t stat_val = final_value;
      auto character = reinterpret_cast<core::character*>(target.data);
      const int64_t current_stat = character->current_stats.get(stat);
      const bool ret = compare_two_num_int(current_stat, stat_val, compare_type);
      call_lua_func(&target, ctx, &data[0], stat_val, compare_type, special_stat, current_stat, ret);
      ctx->current_value.number_type = number_type::number;
      ctx->current_value.value = double(current_stat);
      ctx->current_value.compare_type = compare_type;
      return ret;
    }
    
    bool character_stat_checker(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data, const uint32_t &stat) {
      typedef bool(*func_t)(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data, const uint32_t &stat);
      
      static const func_t stat_funcs[] = {
        &character_stat_checker_int,
        &character_stat_checker_int,
        &character_stat_checker_float
      };
      
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(target.data != nullptr);
      static_assert(sizeof(stat_funcs) / sizeof(stat_funcs[0]) == core::stat_type::count);
      assert(stat < core::character_stats::count);
      
      const uint32_t index = core::character_stats::types[stat];
      return stat_funcs[index](target, ctx, count, data, stat);
    }

#define STAT_FUNC(name) bool name(const target_t &target, context* ctx, const uint32_t &count, const script_data* data) \
    { return character_stat_checker(target, ctx, count, data, core::character_stats::name); }
#define CHARACTER_PENALTY_STAT_FUNC(name) bool name##_penalty(const target_t &target, context* ctx, const uint32_t &count, const script_data* data) \
    { return character_stat_checker(target, ctx, count, data, core::character_stats::name##_penalty); }
    
    BASE_CHARACTER_STATS_LIST
    CHARACTER_PENALTY_STATS_LIST
    
#undef STAT_FUNC
#undef CHARACTER_PENALTY_STAT_FUNC

    template <typename T, typename STAT_T>
    bool stat_checker(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data, const uint32_t &stat) {
      assert(count == 1);
      assert(STAT_T::count < stat);
      
      auto obj = reinterpret_cast<T*>(target.data);
      const auto [final_value, special_stat, number_type, compare_type] = get_num_from_data(&target, ctx, &data[0]);
      const double stat_val = obj->current_stats.get(stat);
      const bool ret = compare_two_num_float(stat_val, final_value, compare_type);
      call_lua_func(&target, ctx, &data[0], final_value, compare_type, special_stat, stat_val, ret);
      ctx->current_value.number_type = number_type::number;
      ctx->current_value.value = double(stat_val);
      ctx->current_value.compare_type = compare_type;
      return ret;
    }
    
    bool hero_stat_checker(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data, const uint32_t &stat) {
      assert(count == 1);
      assert(core::hero_stats::count < stat);
      assert(target.type == static_cast<uint32_t>(core::structure::character));
      
      auto obj = reinterpret_cast<core::character*>(target.data);
      const auto [final_value, special_stat, number_type, compare_type] = get_num_from_data(&target, ctx, &data[0]);
      const double stat_val = obj->current_hero_stats.get(stat);
      const bool ret = compare_two_num_float(stat_val, final_value, compare_type);
      call_lua_func(&target, ctx, &data[0], final_value, compare_type, special_stat, stat_val, ret);
      ctx->current_value.number_type = number_type::number;
      ctx->current_value.value = double(stat_val);
      ctx->current_value.compare_type = compare_type;
      return ret;
    }
    
    template <typename T, typename STAT_T>
    bool resource_checker(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data, const uint32_t &stat) {
      assert(count == 1);
      assert(STAT_T::count < stat);
      
      auto obj = reinterpret_cast<T*>(target.data);
      const auto [final_value, special_stat, number_type, compare_type] = get_num_from_data(&target, ctx, &data[0]);
      const double stat_val = obj->resources.get(stat);
      const bool ret = compare_two_num_float(stat_val, final_value, compare_type);
      call_lua_func(&target, ctx, &data[0], final_value, compare_type, special_stat, stat_val, ret);
      ctx->current_value.number_type = number_type::number;
      ctx->current_value.value = double(stat_val);
      ctx->current_value.compare_type = compare_type;
      return ret;
    }

#define CHECKER_CASE_FUNC(type, name) case core::structure::type: return stat_checker<core::type, core::type##_stats::values>(target, ctx, count, data, core::type##_stats::name);
#define CHECKER_CASE_FUNC2(type, name) case core::structure::type: return hero_stat_checker(target, ctx, count, data, core::hero_stats::name);
    
#define STAT_FUNC(name) bool name(const target_t &target, context* ctx, const uint32_t &count, const script_data* data) \
    { assert(count == 1); assert(data[0].helper1 == condition_function::name); \
      switch (static_cast<core::structure>(target.type)) { \
        CHECKER_CASE_FUNC(province, name) \
        CHECKER_CASE_FUNC(city, name) \
        CHECKER_CASE_FUNC(character, name) \
        default: throw std::runtime_error("Bad object type " + std::string(magic_enum::enum_name(static_cast<core::structure>(target.type))) + " input for function " + std::string(#name)); \
      } \
      return false; \
    }
    
    SHARED_PROVINCE_CITY_CHARACTER_STATS_LIST
    
#undef STAT_FUNC

#define STAT_FUNC(name) bool name(const target_t &target, context* ctx, const uint32_t &count, const script_data* data) \
    { assert(count == 1); assert(data[0].helper1 == condition_function::name); \
      switch (static_cast<core::structure>(target.type)) { \
        CHECKER_CASE_FUNC(province, name) \
        CHECKER_CASE_FUNC(city, name) \
        CHECKER_CASE_FUNC(character, name) \
        CHECKER_CASE_FUNC(realm, name) \
        default: throw std::runtime_error("Bad object type " + std::string(magic_enum::enum_name(static_cast<core::structure>(target.type))) + " input for function " + std::string(#name)); \
      } \
      return false; \
    }
    
    RESOURCE_INCOME_STATS_LIST
    RESOURCE_INCOME_FACTOR_STATS_LIST
    BUILD_FACTOR_STATS_LIST
    
#undef STAT_FUNC

#define STAT_FUNC(name) bool name(const target_t &target, context* ctx, const uint32_t &count, const script_data* data) \
    { assert(count == 1); assert(data[0].helper1 == condition_function::name); \
      switch (static_cast<core::structure>(target.type)) { \
        CHECKER_CASE_FUNC(army, name) \
        CHECKER_CASE_FUNC(character, name) \
        CHECKER_CASE_FUNC(realm, name) \
        default: throw std::runtime_error("Bad object type " + std::string(magic_enum::enum_name(static_cast<core::structure>(target.type))) + " input for function " + std::string(#name)); \
      } \
      return false; \
    }
    
    ARMY_FACTOR_STATS_LIST
    
#undef STAT_FUNC

#define STAT_FUNC(name) bool name(const target_t &target, context* ctx, const uint32_t &count, const script_data* data) \
    { assert(count == 1); assert(data[0].helper1 == condition_function::name); \
      switch (static_cast<core::structure>(target.type)) { \
        CHECKER_CASE_FUNC(realm, name) \
        default: throw std::runtime_error("Bad object type " + std::string(magic_enum::enum_name(static_cast<core::structure>(target.type))) + " input for function " + std::string(#name)); \
      } \
      return false; \
    }
    
    BASE_REALM_STATS_LIST
    VASSAL_RESOURCE_INCOME_FACTOR_STATS_LIST
    
#undef STAT_FUNC

#define STAT_FUNC(name) bool name(const target_t &target, context* ctx, const uint32_t &count, const script_data* data) \
    { assert(count == 1); assert(data[0].helper1 == condition_function::name); \
      switch (static_cast<core::structure>(target.type)) { \
        CHECKER_CASE_FUNC(province, name) \
        CHECKER_CASE_FUNC(city, name) \
        CHECKER_CASE_FUNC(realm, name) \
        default: throw std::runtime_error("Bad object type " + std::string(magic_enum::enum_name(static_cast<core::structure>(target.type))) + " input for function " + std::string(#name)); \
      } \
      return false; \
    }
    
    SHARED_PROVINCE_CITY_REALM_STATS_LIST
    
#undef STAT_FUNC

#define STAT_FUNC(name) bool name(const target_t &target, context* ctx, const uint32_t &count, const script_data* data) \
    { assert(count == 1); assert(data[0].helper1 == condition_function::name); \
      switch (static_cast<core::structure>(target.type)) { \
        CHECKER_CASE_FUNC(province, name) \
        CHECKER_CASE_FUNC(city, name) \
        CHECKER_CASE_FUNC(troop, name) \
        CHECKER_CASE_FUNC(army, name) \
        default: throw std::runtime_error("Bad object type " + std::string(magic_enum::enum_name(static_cast<core::structure>(target.type))) + " input for function " + std::string(#name)); \
      } \
      return false; \
    }
    
    TROOP_FACTOR_STATS_LIST
    
#undef STAT_FUNC

#define STAT_FUNC(name) bool name(const target_t &target, context* ctx, const uint32_t &count, const script_data* data) \
    { assert(count == 1); assert(data[0].helper1 == condition_function::name); \
      switch (static_cast<core::structure>(target.type)) { \
        CHECKER_CASE_FUNC(province, name) \
        default: throw std::runtime_error("Bad object type " + std::string(magic_enum::enum_name(static_cast<core::structure>(target.type))) + " input for function " + std::string(#name)); \
      } \
      return false; \
    }
    
    BASE_PROVINCE_STATS_LIST
    
#undef STAT_FUNC

#define STAT_FUNC(name) bool name(const target_t &target, context* ctx, const uint32_t &count, const script_data* data) \
    { assert(count == 1); assert(data[0].helper1 == condition_function::name); \
      switch (static_cast<core::structure>(target.type)) { \
        CHECKER_CASE_FUNC(city, name) \
        default: throw std::runtime_error("Bad object type " + std::string(magic_enum::enum_name(static_cast<core::structure>(target.type))) + " input for function " + std::string(#name)); \
      } \
      return false; \
    }
    
    BASE_CITY_STATS_LIST
    LIEGE_RESOURCE_INCOME_STATS_LIST
    
#undef STAT_FUNC

#define STAT_FUNC(name) bool name(const target_t &target, context* ctx, const uint32_t &count, const script_data* data) \
    { assert(count == 1); assert(data[0].helper1 == condition_function::name); \
      switch (static_cast<core::structure>(target.type)) { \
        CHECKER_CASE_FUNC(army, name) \
        default: throw std::runtime_error("Bad object type " + std::string(magic_enum::enum_name(static_cast<core::structure>(target.type))) + " input for function " + std::string(#name)); \
      } \
      return false; \
    }
    
    BASE_ARMY_STATS_LIST
    
#undef STAT_FUNC

#define STAT_FUNC(name) bool name(const target_t &target, context* ctx, const uint32_t &count, const script_data* data) \
    { assert(count == 1); assert(data[0].helper1 == condition_function::name); \
      switch (static_cast<core::structure>(target.type)) { \
        CHECKER_CASE_FUNC(army, name) \
        CHECKER_CASE_FUNC(hero_troop, name) \
        CHECKER_CASE_FUNC(troop, name) \
        CHECKER_CASE_FUNC2(character, name) \
        default: throw std::runtime_error("Bad object type " + std::string(magic_enum::enum_name(static_cast<core::structure>(target.type))) + " input for function " + std::string(#name)); \
      } \
      return false; \
    }
    
    SHARED_HERO_TROOP_ARMY_STATS_LIST
    
#undef STAT_FUNC

#define STAT_FUNC(name) bool name(const target_t &target, context* ctx, const uint32_t &count, const script_data* data) \
    { assert(count == 1); assert(data[0].helper1 == condition_function::name); \
      switch (static_cast<core::structure>(target.type)) { \
        CHECKER_CASE_FUNC(hero_troop, name) \
        default: throw std::runtime_error("Bad object type " + std::string(magic_enum::enum_name(static_cast<core::structure>(target.type))) + " input for function " + std::string(#name)); \
      } \
      return false; \
    }
    
    BASE_HERO_TROOP_STATS_LIST
    
#undef STAT_FUNC

#define STAT_FUNC(name) bool name(const target_t &target, context* ctx, const uint32_t &count, const script_data* data) \
    { assert(count == 1); assert(data[0].helper1 == condition_function::name); \
      switch (static_cast<core::structure>(target.type)) { \
        CHECKER_CASE_FUNC(troop, name) \
        default: throw std::runtime_error("Bad object type " + std::string(magic_enum::enum_name(static_cast<core::structure>(target.type))) + " input for function " + std::string(#name)); \
      } \
      return false; \
    }
    
    BASE_TROOP_STATS_LIST
    
#undef STAT_FUNC

#define STAT_FUNC(name) bool name(const target_t &target, context* ctx, const uint32_t &count, const script_data* data) \
    { assert(count == 1); assert(data[0].helper1 == condition_function::name); \
      switch (static_cast<core::structure>(target.type)) { \
        CHECKER_CASE_FUNC(city, name) \
        CHECKER_CASE_FUNC(province, name) \
        default: throw std::runtime_error("Bad object type " + std::string(magic_enum::enum_name(static_cast<core::structure>(target.type))) + " input for function " + std::string(#name)); \
      } \
      return false; \
    }
    
    SHARED_PROVINCE_CITY_STATS_LIST
    
#undef STAT_FUNC

#define STAT_FUNC(name) bool name(const target_t &target, context* ctx, const uint32_t &count, const script_data* data) \
    { assert(count == 1); assert(data[0].helper1 == condition_function::name); \
      switch (static_cast<core::structure>(target.type)) { \
        CHECKER_CASE_FUNC2(character, name) \
        default: throw std::runtime_error("Bad object type " + std::string(magic_enum::enum_name(static_cast<core::structure>(target.type))) + " input for function " + std::string(#name)); \
      } \
      return false; \
    }
    
    BASE_HERO_STATS_LIST
    
#undef STAT_FUNC

#define STAT_FUNC(name) bool name(const target_t &target, context* ctx, const uint32_t &count, const script_data* data) \
    { assert(count == 1); assert(data[0].helper1 == condition_function::name); \
      switch (static_cast<core::structure>(target.type)) { \
        CHECKER_CASE_FUNC(troop, name) \
        CHECKER_CASE_FUNC2(character, name) \
        default: throw std::runtime_error("Bad object type " + std::string(magic_enum::enum_name(static_cast<core::structure>(target.type))) + " input for function " + std::string(#name)); \
      } \
      return false; \
    }
    
    SHARED_TROOP_HERO_STATS_LIST
    
#undef STAT_FUNC

#define STAT_FUNC(name) bool name(const target_t &target, context* ctx, const uint32_t &count, const script_data* data) \
    { assert(count == 1); assert(data[0].helper1 == condition_function::name); \
      switch (static_cast<core::structure>(target.type)) { \
        CHECKER_CASE_FUNC(realm, name) \
        CHECKER_CASE_FUNC2(character, name) \
        default: throw std::runtime_error("Bad object type " + std::string(magic_enum::enum_name(static_cast<core::structure>(target.type))) + " input for function " + std::string(#name)); \
      } \
      return false; \
    }
    
    HERO_FACTOR_STATS_LIST
    
#undef STAT_FUNC

#undef CHECKER_CASE_FUNC
#undef CHECKER_CASE_FUNC2

#define CHECKER_CASE_FUNC(type, name) case core::structure::type: return resource_checker<core::type, core::type##_stats::values>(target, ctx, count, data, core::type##_resources::name);

#define STAT_FUNC(name) bool name(const target_t &target, context* ctx, const uint32_t &count, const script_data* data) \
    { assert(count == 1); assert(data[0].helper1 == condition_function::name); \
      switch (static_cast<core::structure>(target.type)) { \
        CHECKER_CASE_FUNC(character, name) \
        CHECKER_CASE_FUNC(realm, name) \
        default: throw std::runtime_error("Bad object type " + std::string(magic_enum::enum_name(static_cast<core::structure>(target.type))) + " input for function " + std::string(#name)); \
      } \
      return false; \
    }
    
    RESOURCE_STATS_LIST
    
#undef STAT_FUNC

// для геройских статов поди придется делать отдельный чекер

/* =============================================    
                  START INIT
   ============================================= */

    
    // пока что вход сделаем только бул, это функции персонажа
#define CONDITION_COMMAND_FUNC_INIT_BOOL(name) void name##_init(const uint32_t &target_type, const sol::object &obj, script_data* data) { \
      if (obj.get_type() != sol::type::boolean && obj.get_type() != sol::type::string) throw std::runtime_error("Bad input for " + std::string(#name)); \
      if (target_type != UINT32_MAX && target_type != static_cast<uint32_t>(core::character::s_type)) throw std::runtime_error(std::string(#name) + " can be executed only for character"); \
      data->command_type = command_type::condition; \
      data->helper1 = condition_function::name; \
      boolean_input_init(#name, target_type, obj, data); \
    }
    
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
    
    bool check_has_flag_target_type(const uint32_t &target_type) {
      const auto casted = static_cast<core::structure>(target_type);
      return target_type == UINT32_MAX || 
             casted == core::army::s_type ||
             casted == core::character::s_type ||
             casted == core::hero_troop::s_type ||
             casted == core::province::s_type ||
             casted == core::realm::s_type ||
             casted == core::city::s_type ||
             casted == core::war::s_type ||
             casted == core::titulus::s_type;
    }
    
    void has_flag_init(const uint32_t &target_type, const sol::object &obj, script_data* data) {
      // тип должен быть либо UINT32_MAX либо одним из следующих
      if (!check_has_flag_target_type(target_type)) throw std::runtime_error("Bad current entity type for has_flag");
      data->command_type = command_type::condition;
      data->helper1 = condition_function::has_flag;
      
      string_input_init("has_flag", target_type, obj, data);
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
      if (target_type != UINT32_MAX && target_type != static_cast<uint32_t>(core::character::s_type)) throw std::runtime_error(std::string(#name) + " can be executed only for character"); \
      data->command_type = command_type::condition; \
      data->helper1 = condition_function::name; \
      variable_input_init(#name, target_type, obj, data); \
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
  
#undef CHARACTER_CONDITION_COMMAND_FUNC_INIT_NUM
    
#define CHARACTER_PENALTY_STAT_FUNC(name) void name##_penalty_init(const uint32_t &target_type, const sol::object &obj, script_data* data) { \
      if (target_type != UINT32_MAX && target_type != static_cast<uint32_t>(core::character::s_type)) throw std::runtime_error(std::string(#name) + "_penalty can be executed only for character"); \
      data->command_type = command_type::condition; \
      data->helper1 = condition_function::name##_penalty; \
      variable_input_init(#name, target_type, obj, data); \
    }
    
    CHARACTER_PENALTY_STATS_LIST
    
#undef CHARACTER_PENALTY_STAT_FUNC
    
#define DEFAULT_STAT_FUNC(name, cond) void name##_init(const uint32_t &target_type, const sol::object &obj, script_data* data) { \
      if (target_type != UINT32_MAX && (cond))            \
        throw std::runtime_error(                         \
          "Bad input type '" +                            \
          std::string(magic_enum::enum_name<core::structure>(static_cast<core::structure>(target_type))) + \
          "' for " + std::string(magic_enum::enum_name(condition_function::name))                          \
        );                                                \
      data->command_type = command_type::condition;       \
      data->helper1 = condition_function::name;           \
      variable_input_init(#name, target_type, obj, data); \
    }
    
#define STAT_FUNC(name) DEFAULT_STAT_FUNC(name, target_type != static_cast<uint32_t>(core::character::s_type))
    BASE_CHARACTER_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) DEFAULT_STAT_FUNC(name, \
 target_type != static_cast<uint32_t>(core::province::s_type) && \
 target_type != static_cast<uint32_t>(core::city::s_type) && \
 target_type != static_cast<uint32_t>(core::character::s_type))
    SHARED_PROVINCE_CITY_CHARACTER_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) DEFAULT_STAT_FUNC(name, \
 target_type != static_cast<uint32_t>(core::province::s_type) && \
 target_type != static_cast<uint32_t>(core::city::s_type) && \
 target_type != static_cast<uint32_t>(core::character::s_type) && \
 target_type != static_cast<uint32_t>(core::realm::s_type))
    RESOURCE_INCOME_STATS_LIST
    RESOURCE_INCOME_FACTOR_STATS_LIST
    BUILD_FACTOR_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) DEFAULT_STAT_FUNC(name, \
 target_type != static_cast<uint32_t>(core::army::s_type) && \
 target_type != static_cast<uint32_t>(core::character::s_type) && \
 target_type != static_cast<uint32_t>(core::realm::s_type))
    ARMY_FACTOR_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) DEFAULT_STAT_FUNC(name, target_type != static_cast<uint32_t>(core::realm::s_type))
    BASE_REALM_STATS_LIST
    VASSAL_RESOURCE_INCOME_FACTOR_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) DEFAULT_STAT_FUNC(name, \
 target_type != static_cast<uint32_t>(core::province::s_type) && \
 target_type != static_cast<uint32_t>(core::city::s_type) && \
 target_type != static_cast<uint32_t>(core::realm::s_type))
    SHARED_PROVINCE_CITY_REALM_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) DEFAULT_STAT_FUNC(name, \
 target_type != static_cast<uint32_t>(core::province::s_type) && \
 target_type != static_cast<uint32_t>(core::city::s_type) && \
 target_type != static_cast<uint32_t>(core::army::s_type) && \
 target_type != static_cast<uint32_t>(core::troop::s_type))
    TROOP_FACTOR_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) DEFAULT_STAT_FUNC(name, target_type != static_cast<uint32_t>(core::province::s_type))
    BASE_PROVINCE_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) DEFAULT_STAT_FUNC(name, target_type != static_cast<uint32_t>(core::city::s_type))
    BASE_CITY_STATS_LIST
    LIEGE_RESOURCE_INCOME_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) DEFAULT_STAT_FUNC(name, target_type != static_cast<uint32_t>(core::army::s_type))
    BASE_ARMY_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) DEFAULT_STAT_FUNC(name, \
  target_type != static_cast<uint32_t>(core::army::s_type) && \
  target_type != static_cast<uint32_t>(core::hero_troop::s_type) && \
  target_type != static_cast<uint32_t>(core::troop::s_type) && \
  target_type != static_cast<uint32_t>(core::character::s_type))
    SHARED_HERO_TROOP_ARMY_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) DEFAULT_STAT_FUNC(name, target_type != static_cast<uint32_t>(core::hero_troop::s_type))
    BASE_HERO_TROOP_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) DEFAULT_STAT_FUNC(name, target_type != static_cast<uint32_t>(core::troop::s_type))
    BASE_TROOP_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) DEFAULT_STAT_FUNC(name, \
 target_type != static_cast<uint32_t>(core::province::s_type) && \
 target_type != static_cast<uint32_t>(core::city::s_type))
    SHARED_PROVINCE_CITY_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) DEFAULT_STAT_FUNC(name, target_type != static_cast<uint32_t>(core::character::s_type))
    BASE_HERO_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) DEFAULT_STAT_FUNC(name, \
  target_type != static_cast<uint32_t>(core::troop::s_type) && \
  target_type != static_cast<uint32_t>(core::character::s_type))
    SHARED_TROOP_HERO_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) DEFAULT_STAT_FUNC(name, \
  target_type != static_cast<uint32_t>(core::character::s_type) && \
  target_type != static_cast<uint32_t>(core::realm::s_type))
    HERO_FACTOR_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) DEFAULT_STAT_FUNC(name, \
 target_type != static_cast<uint32_t>(core::character::s_type) && \
 target_type != static_cast<uint32_t>(core::realm::s_type))
    RESOURCE_STATS_LIST
#undef STAT_FUNC
    
#undef DEFAULT_STAT_FUNC
  }
}
