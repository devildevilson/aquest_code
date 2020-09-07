#include "condition_functions.h"
#include "core_structures.h"

namespace devils_engine {
  namespace utils {
    bool is_ai(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      ASSERT(count == 1);
      ASSERT(target.type == target_data::type::character);
      ASSERT(target.character != nullptr);
      const bool cur = bool(data[0].uval);
      return target.character->is_player() != cur;
    }
    
    bool is_player(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      ASSERT(count == 1);
      ASSERT(target.type == target_data::type::character);
      ASSERT(target.character != nullptr);
      const bool cur = bool(data[0].uval);
      return target.character->is_player() == cur;
    }
    
    bool is_independent(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      ASSERT(count == 1);
      ASSERT(target.type == target_data::type::character);
      ASSERT(target.character != nullptr);
      const bool cur = bool(data[0].uval);
      return target.character->is_independent() == cur;
    }
    
    bool is_vassal(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      ASSERT(count == 1);
      ASSERT(target.type == target_data::type::character);
      ASSERT(target.character != nullptr);
      const bool cur = bool(data[0].uval);
      return target.character->is_independent() != cur;
    }
    
    bool is_male(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      ASSERT(count == 1);
      ASSERT(target.type == target_data::type::character);
      ASSERT(target.character != nullptr);
      const bool cur = bool(data[0].uval);
      return target.character->is_male() == cur;
    }
    
    bool is_female(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      ASSERT(count == 1);
      ASSERT(target.type == target_data::type::character);
      ASSERT(target.character != nullptr);
      const bool cur = bool(data[0].uval);
      return target.character->is_male() != cur;
    }
    
    bool is_prisoner(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      ASSERT(count == 1);
      ASSERT(target.type == target_data::type::character);
      ASSERT(target.character != nullptr);
      const bool cur = bool(data[0].uval);
      return target.character->is_prisoner() == cur;
    }
    
    bool is_married(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      ASSERT(count == 1);
      ASSERT(target.type == target_data::type::character);
      ASSERT(target.character != nullptr);
      const bool cur = bool(data[0].uval);
      return target.character->is_married() == cur;
    }
    
    bool is_sick(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      ASSERT(count == 1);
      ASSERT(target.type == target_data::type::character);
      ASSERT(target.character != nullptr);
      const bool cur = bool(data[0].uval);
      // тут нужно проверить есть ли у нас трейт со свойством болезнь
      bool sick = false;
      for (uint32_t i = 0; i < target.character->traits.count; ++i) {
        auto trait = target.character->traits.container[i];
        if (trait->get_attrib(utils::trait::is_disease)) {
          sick = true;
          break;
        }
      }
      
      return sick == cur;
    }
    
    bool is_in_war(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      ASSERT(count == 1);
      ASSERT(target.type == target_data::type::character);
      ASSERT(target.character != nullptr);
      const bool cur = bool(data[0].uval);
      ASSERT(false); // как быть с войной? должен быть экран дипломатии похожий на тот что в европке
    }
    
    bool is_in_society(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      ASSERT(count == 1);
      ASSERT(target.type == target_data::type::character);
      ASSERT(target.character != nullptr);
      const bool cur = bool(data[0].uval);
      ASSERT(false);
    }
    
    bool is_hero(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      ASSERT(count == 1);
      ASSERT(target.type == target_data::type::character);
      ASSERT(target.character != nullptr);
      const bool cur = bool(data[0].uval);
      return target.character->is_hero();
    }
    
    bool is_clan_head(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      ASSERT(count == 1);
      ASSERT(target.type == target_data::type::character);
      ASSERT(target.character != nullptr);
      const bool cur = bool(data[0].uval);
      ASSERT(false);
    }
    
    bool is_religious_head(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      ASSERT(count == 1);
      ASSERT(target.type == target_data::type::character);
      ASSERT(target.character != nullptr);
      const bool cur = bool(data[0].uval);
      ASSERT(false); // по идее указатель должен лежать в самой религии
    }
    
    bool is_father_alive(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      ASSERT(count == 1);
      ASSERT(target.type == target_data::type::character);
      ASSERT(target.character != nullptr);
      const bool cur = bool(data[0].uval);
      if (target.character->family.parents[0] != nullptr && target.character->family.parents[0]->is_male()) {
        return target.character->family.parents[0]->is_dead() != cur;
      }
      
      if (target.character->family.parents[1] != nullptr && target.character->family.parents[1]->is_male()) {
        return target.character->family.parents[1]->is_dead() != cur;
      }
      
      return false;
    }
    
    bool is_mother_alive(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      ASSERT(count == 1);
      ASSERT(target.type == target_data::type::character);
      ASSERT(target.character != nullptr);
      const bool cur = bool(data[0].uval);
      if (target.character->family.parents[0] != nullptr && !target.character->family.parents[0]->is_male()) {
        return target.character->family.parents[0]->is_dead() != cur;
      }
      
      if (target.character->family.parents[1] != nullptr && !target.character->family.parents[1]->is_male()) {
        return target.character->family.parents[1]->is_dead() != cur;
      }
      
      return false;
    }
    
    bool has_dead_friends(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      ASSERT(count == 1);
      ASSERT(target.type == target_data::type::character);
      ASSERT(target.character != nullptr);
      const size_t cur = std::min(data[0].uval, size_t(core::character::relations::max_game_friends));
      size_t counter = 0;
      for (uint32_t i = 0; i < core::character::relations::max_game_friends; ++i) {
        auto char_friend = target.character->relations.friends[i];
        counter += size_t(char_friend != nullptr && char_friend->is_dead());
      }
      
      return cur == 0 ? counter == 0 : counter >= cur;
    }
    
    bool has_dead_rivals(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      ASSERT(count == 1);
      ASSERT(target.type == target_data::type::character);
      ASSERT(target.character != nullptr);
      const size_t cur = std::min(data[0].uval, size_t(core::character::relations::max_game_rivals));
      size_t counter = 0;
      for (uint32_t i = 0; i < core::character::relations::max_game_rivals; ++i) {
        auto char_rival = target.character->relations.rivals[i];
        counter += size_t(char_rival != nullptr && char_rival->is_dead());
      }
      
      return cur == 0 ? counter == 0 : counter >= cur;
    }
    
    bool has_owner(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      ASSERT(count == 1);
      ASSERT(target.type == target_data::type::character);
      ASSERT(target.character != nullptr);
      const bool cur = bool(data[0].uval);
      return (target.character->family.owner != nullptr) == cur;
    }
    
    bool has_dead_lovers(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      ASSERT(count == 1);
      ASSERT(target.type == target_data::type::character);
      ASSERT(target.character != nullptr);
      const size_t cur = std::min(data[0].uval, size_t(core::character::relations::max_game_lovers));
      size_t counter = 0;
      for (uint32_t i = 0; i < core::character::relations::max_game_lovers; ++i) {
        auto char_lover = target.character->relations.lovers[i];
        counter += size_t(char_lover != nullptr && char_lover->is_dead());
      }
      
      return cur == 0 ? counter == 0 : counter >= cur;
    }
    
    bool has_dead_brothers(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      ASSERT(count == 1);
      ASSERT(target.type == target_data::type::character);
      ASSERT(target.character != nullptr);
      const size_t cur = data[0].uval;
      auto current_char = target.character->family.next_sibling;
      size_t counter = 0;
      while (current_char != target.character) {
        counter += size_t(current_char->is_dead() && current_char->is_male());
        current_char = current_char->family.next_sibling;
      }
      
      return cur == 0 ? counter == 0 : counter >= cur;
    }
    
    bool has_dead_sisters(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      ASSERT(count == 1);
      ASSERT(target.type == target_data::type::character);
      ASSERT(target.character != nullptr);
      const size_t cur = data[0].uval;
      auto current_char = target.character->family.next_sibling;
      size_t counter = 0;
      while (current_char != target.character) {
        counter += size_t(current_char->is_dead() && !current_char->is_male());
        current_char = current_char->family.next_sibling;
      }
      
      return cur == 0 ? counter == 0 : counter >= cur;
    }
    
    bool has_dead_siblings(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      ASSERT(count == 1);
      ASSERT(target.type == target_data::type::character);
      ASSERT(target.character != nullptr);
      const size_t cur = data[0].uval;
      auto current_char = target.character->family.next_sibling;
      size_t counter = 0;
      while (current_char != target.character) {
        counter += size_t(current_char->is_dead());
        current_char = current_char->family.next_sibling;
      }
      
      return cur == 0 ? counter == 0 : counter >= cur;
    }
    
    bool has_dead_childs(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      ASSERT(count == 1);
      ASSERT(target.type == target_data::type::character);
      ASSERT(target.character != nullptr);
      const size_t cur = data[0].uval;
      auto current_child = target.character->family.children;
      if (current_child == nullptr) return cur == 0;
      
      auto next_child = current_child->family.next_sibling;
      size_t counter = 1;
      while (current_char != next_child) {
        if (next_child->family.parents[0] == target.character || next_child->family.parents[1] == target.character) counter += size_t(next_child->is_dead());
        next_child = next_child->family.next_sibling;
      }
      
      return cur == 0 ? counter == 0 : counter >= cur;
    }
    
    bool has_concubines(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      ASSERT(count == 1);
      ASSERT(target.type == target_data::type::character);
      ASSERT(target.character != nullptr);
      const size_t cur = data[0].uval;
      
      auto current_concubine = target.character->family.concubines;
      if (current_concubine == nullptr) return cur == 0;
      ASSERT(current_concubine->family.owner == target.character);
      size_t counter = 1;
      auto next_concubine = current_concubine->family.concubines;
      while (next_concubine != nullptr) {
        ++counter;
        next_concubine = next_concubine->family.concubines;
      }
      
      return cur == 0 ? counter == 0 : counter >= cur;
    }
    
    bool has_alive_friends(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      ASSERT(count == 1);
      ASSERT(target.type == target_data::type::character);
      ASSERT(target.character != nullptr);
      const size_t cur = std::min(data[0].uval, size_t(core::character::relations::max_game_friends));
      size_t counter = 0;
      for (uint32_t i = 0; i < core::character::relations::max_game_friends; ++i) {
        auto char_friend = target.character->relations.friends[i];
        counter += size_t(char_friend != nullptr && !char_friend->is_dead());
      }
      
      return cur == 0 ? counter == 0 : counter >= cur;
    }
    
    bool has_alive_rivals(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      ASSERT(count == 1);
      ASSERT(target.type == target_data::type::character);
      ASSERT(target.character != nullptr);
      const size_t cur = std::min(data[0].uval, size_t(core::character::relations::max_game_rivals));
      size_t counter = 0;
      for (uint32_t i = 0; i < core::character::relations::max_game_rivals; ++i) {
        auto char_rival = target.character->relations.rivals[i];
        counter += size_t(char_rival != nullptr && !char_rival->is_dead());
      }
      
      return cur == 0 ? counter == 0 : counter >= cur;
    }
    
    bool has_alive_lovers(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      ASSERT(count == 1);
      ASSERT(target.type == target_data::type::character);
      ASSERT(target.character != nullptr);
      const size_t cur = std::min(data[0].uval, size_t(core::character::relations::max_game_lovers));
      size_t counter = 0;
      for (uint32_t i = 0; i < core::character::relations::max_game_lovers; ++i) {
        auto char_lover = target.character->relations.lovers[i];
        counter += size_t(char_lover != nullptr && !char_lover->is_dead());
      }
      
      return cur == 0 ? counter == 0 : counter >= cur;
    }
    
    bool has_alive_brothers(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      ASSERT(count == 1);
      ASSERT(target.type == target_data::type::character);
      ASSERT(target.character != nullptr);
      const size_t cur = data[0].uval;
      auto current_char = target.character->family.next_sibling;
      size_t counter = 0;
      while (current_char != target.character) {
        counter += size_t(!current_char->is_dead() && current_char->is_male());
        current_char = current_char->family.next_sibling;
      }
      
      return cur == 0 ? counter == 0 : counter >= cur;
    }
    
    bool has_alive_sisters(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      ASSERT(count == 1);
      ASSERT(target.type == target_data::type::character);
      ASSERT(target.character != nullptr);
      const size_t cur = data[0].uval;
      auto current_char = target.character->family.next_sibling;
      size_t counter = 0;
      while (current_char != target.character) {
        counter += size_t(!current_char->is_dead() && !current_char->is_male());
        current_char = current_char->family.next_sibling;
      }
      
      return cur == 0 ? counter == 0 : counter >= cur;
    }
    
    bool has_alive_siblings(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      ASSERT(count == 1);
      ASSERT(target.type == target_data::type::character);
      ASSERT(target.character != nullptr);
      const size_t cur = data[0].uval;
      auto current_char = target.character->family.next_sibling;
      size_t counter = 0;
      while (current_char != target.character) {
        counter += size_t(!current_char->is_dead());
        current_char = current_char->family.next_sibling;
      }
      
      return cur == 0 ? counter == 0 : counter >= cur;
    }
    
    bool has_alive_childs(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      ASSERT(count == 1);
      ASSERT(target.type == target_data::type::character);
      ASSERT(target.character != nullptr);
      const size_t cur = data[0].uval;
      auto current_child = target.character->family.children;
      if (current_child == nullptr) return cur == 0;
      
      auto next_child = current_child->family.next_sibling;
      size_t counter = 1;
      while (current_char != next_child) {
        if (next_child->family.parents[0] == target.character || next_child->family.parents[1] == target.character) counter += size_t(!next_child->is_dead());
        next_child = next_child->family.next_sibling;
      }
      
      return cur == 0 ? counter == 0 : counter >= cur;
    }
    
    bool has_dynasty(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      ASSERT(count == 1);
      ASSERT(target.type == target_data::type::character);
      ASSERT(target.character != nullptr);
      const bool cur = bool(data[0].uval);
      return (target.character->family.dynasty != nullptr) == cur;
    }
    
    bool can_change_religion(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      ASSERT(count == 1);
      ASSERT(target.type == target_data::type::character);
      ASSERT(target.character != nullptr);
      ASSERT(false);
      // какая тут проверка? есть ли скрытая религия?
    }
    
    bool can_call_crusade(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      ASSERT(count == 1);
      ASSERT(target.type == target_data::type::character);
      ASSERT(target.character != nullptr);
      ASSERT(false);
      // если это глава религии, если религия позволяет это... и все? а не глава религии может ли?
    }
    
    bool can_grant_title(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      ASSERT(count == 1);
      ASSERT(target.type == target_data::type::character);
      ASSERT(target.character != nullptr);
      ASSERT(false);
      // может дать титул, какой?
    }
    
    bool can_marry(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      ASSERT(count == 1);
      ASSERT(target.type == target_data::type::character);
      ASSERT(target.character != nullptr);
      const bool cur = bool(data[0].uval);
      if (target.character->is_married()) return cur == false;
      
      for (uint32_t i = 0; i < target.character->traits.count; ++i) {
        auto trait = target.character->traits.container[i];
        if (trait->get_attrib(utils::trait::cannot_marry)) return cur == false;
      }
      
      return cur == true;
    }
    
    bool belongs_to_culture(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      ASSERT(count == 1);
      ASSERT(target.type == target_data::type::character);
      ASSERT(target.character != nullptr);
      ASSERT(false);
      // тут нужно передать либо индекс либо указатель на культуру
    }
    
    bool belongs_to_culture_group(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      ASSERT(count == 1);
      ASSERT(target.type == target_data::type::character);
      ASSERT(target.character != nullptr);
      ASSERT(false);
      // тут нужно передать либо индекс либо указатель на культуру
    }
    
    bool has_trait(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      
    }
    
    bool has_modificator(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      
    }
    
    bool has_flag(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      
    }
    
    bool has_title(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      
    }
    
    bool has_nickname(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      
    }
    
    bool bit_is_set(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      
    }
    
    bool bit_is_unset(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      
    }
    
    bool realm_has_enacted_law(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      
    }
    
    bool realm_has_law_mechanic(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      
    }
    
    bool is_among_most_powerful_vassals(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      
    }
    
    bool age_more(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      ASSERT(count == 1);
      ASSERT(target.type == target_data::type::character);
      ASSERT(target.character != nullptr);
      const size_t age = data[0].uval;
      
    }
    
    bool age_equal(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      
    }
    
    bool age_less(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      
    }
    
    bool money_more(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      
    }
    
    bool money_equal(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      
    }
    
    bool money_less(const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      
    }
    
    bool character_stat_more(const uint32_t &stat, const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      
    }
    
    bool character_stat_equal(const uint32_t &stat, const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      
    }
    
    bool character_stat_less(const uint32_t &stat, const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      
    }
    
    bool hero_stat_more(const uint32_t &stat, const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      
    }
    
    bool hero_stat_equal(const uint32_t &stat, const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      
    }
    
    bool hero_stat_less(const uint32_t &stat, const target_data &target, const uint32_t &count, const functions_container::data_container* data) {
      
    }
  }
}
