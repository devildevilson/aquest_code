#include "character.h"

#include <stdexcept>

#include "utils/assert.h"
#include "utils/globals.h"
#include "utils/systems.h"
#include "utils/deferred_tasks.h"

#include "ai/path_container.h"

#include "script/script_block_functions.h"

#include "realm.h"
#include "titulus.h"
#include "context.h"

namespace devils_engine {
  namespace core {
//     hero::hero() : character(nullptr) {}
    
    // короче я подумал что фиг там, возможно нужно разные строки возвращать
    // мало того нужно еще и локализированные строки возвращать, поэтому нахой
    // другое дело что где это все должно быть?
//     std::string_view titulus::ruler_title() const {
//       switch (type) {
//         case type::city: return "";
//         case type::baron: return "Baron";
//         case type::duke: return "Duke";
//         case type::king: return "King";
//         case type::imperial: return "Emperior";
//         default: throw std::runtime_error("wtf");
//       }
//       
//       return "";
//     }
    
    namespace system {
      enum character_enum {
        player,
        hero,
        male,
        dead,
        has_troop,
        has_army,
        
        count
      };
    }
    
    const structure character::s_type;
    const size_t character::traits_container_size;
    const size_t character::modificators_container_size;
    const size_t character::events_container_size;
    const size_t character::flags_container_size;
    bool character::is_cousin(const character* a, const character* b) {
      bool ret = false;
      for (uint8_t i = 0; i < 2; ++i) {
        auto p1 = a->family.parents[i];
        for (uint8_t j = 0; j < 2; ++j) {
          auto gp1 = p1->family.parents[j];
          for (uint8_t k = 0; k < 2; ++k) {
            auto p2 = b->family.parents[k];
            for (uint8_t c = 0; c < 2; ++c) {
              auto gp2 = p2->family.parents[j];
              ret = ret || gp1 == gp2;
            }
          }
        }
      }
      
      return !is_sibling(a, b) && ret;
    }
    
    bool character::is_sibling(const character* a, const character* b) {
      return a->family.parents[0] == b->family.parents[0] || a->family.parents[1] == b->family.parents[1] || a->family.parents[0] == b->family.parents[1] || a->family.parents[1] == b->family.parents[0];
    }
    
    bool character::is_full_sibling(const character* a, const character* b) {
      return (a->family.parents[0] == b->family.parents[0] && a->family.parents[1] == b->family.parents[1]) || (a->family.parents[0] == b->family.parents[1] && a->family.parents[1] == b->family.parents[0]);
    }
    
    bool character::is_half_sibling(const character* a, const character* b) {
      return is_sibling(a, b) && !is_full_sibling(a, b);
    }
    
    bool character::is_relative(const character* a, const character* b) {
      // нужно проверить династию
      return false;
    }
    
    bool character::is_bastard(const character* a) {
      // незаконное рождение - нужно проверить законных жен
      auto parent1 = a->family.parents[0];
      auto parent2 = a->family.parents[1];
      bool ret = parent1->family.consort == parent2;
      auto prev_consort = parent1->family.previous_consorts;
      while (prev_consort != nullptr) {
        ret = ret || prev_consort == parent2;
        prev_consort = prev_consort->family.previous_consorts;
      }
      
      return !ret;
    }
    
    bool character::is_concubine_child(const character* a) {
      auto parent1 = a->family.parents[0];
      auto parent2 = a->family.parents[1];
      return (parent1->family.owner == parent2) || (parent1 == parent2->family.owner);
    }
    
    bool character::is_concubine(const character* a, const character* b) {
      return a->family.owner == b;
    }
    
    character::family::family() :
      real_parents{nullptr, nullptr},
      parents{nullptr, nullptr}, 
//       grandparents{nullptr, nullptr, nullptr, nullptr}, 
      children(nullptr), 
      next_sibling(nullptr), 
      prev_sibling(nullptr), 
      consort(nullptr), 
      previous_consorts(nullptr), 
      owner(nullptr), 
      concubines(nullptr),
      blood_dynasty(nullptr),
      dynasty(nullptr)
    {}
    
    character::relations::relations() :
      friends{nullptr},
      rivals{nullptr},
      lovers{nullptr}
    {
      ASSERT(friends[max_game_friends-1] == nullptr);
      ASSERT(rivals[max_game_rivals-1] == nullptr);
      ASSERT(lovers[max_game_lovers-1] == nullptr);
    }
    
    character::character(const bool male, const bool dead) : 
      name_number(0), 
      born_day(INT64_MAX), 
      death_day(INT64_MAX), 
//       name_str(SIZE_MAX), 
//       nickname_str(SIZE_MAX), 
      suzerain(nullptr),
      imprisoner(nullptr),
      next_prisoner(nullptr),
      prev_prisoner(nullptr),
      next_courtier(nullptr),
      prev_courtier(nullptr),
      culture(nullptr),
      religion(nullptr),
      hidden_religion(nullptr),
      realms{nullptr, nullptr, nullptr, nullptr}
//       modificators(nullptr),
//       events(nullptr),
//       flags(nullptr)
    {
      memset(stats.data(), 0, stats.size()*sizeof(stats[0]));
      memset(current_stats.data(), 0, current_stats.size()*sizeof(current_stats[0]));
      memset(hero_stats.data(), 0, hero_stats.size()*sizeof(hero_stats[0]));
      memset(current_hero_stats.data(), 0, current_hero_stats.size()*sizeof(current_hero_stats[0]));
      data.set(system::male, male);
      data.set(system::dead, dead);
      traits.reserve(traits_container_size);
      if (!dead) {
        modificators.reserve(modificators_container_size);
        events.reserve(events_container_size);
        flags.reserve(flags_container_size);
//         modificators = new modificators_container<modificators_container_size>;
//         events = new events_container<events_container_size>;
//         flags = new flags_container<flags_container_size>;
      }
    }
    
    character::~character() {
//       delete modificators;
//       delete events;
//       delete flags;
    }
    
    bool character::is_independent() const {
      if (realms[self] == nullptr) {
        ASSERT(suzerain != nullptr);
        return false;
      }
      
      return realms[self] != nullptr && realms[self]->is_independent(); //  || suzerain == nullptr // персонаж без титулов и не придворный это ошибка!
    }
    
    bool character::is_prisoner() const {
      return imprisoner != nullptr;
    }
    
    bool character::is_married() const {
      return family.consort != nullptr;
    }
    
    bool character::is_male() const {
      return data.get(system::male);
    }
    
    bool character::is_hero() const {
      return data.get(system::hero);
    }
    
    bool character::is_player() const {
      return data.get(system::player);
    }
    
    bool character::is_dead() const {
      return data.get(system::dead);
    }
    
    bool character::has_dynasty() const {
      return family.dynasty != nullptr || family.blood_dynasty != nullptr;
    }
    
    bool character::is_ai_playable() const {
      return realms[self] != nullptr && realms[self]->main_title->type != titulus::type::city;
    }
    
    bool character::is_troop_owner() const {
      return data.get(system::has_troop);
    }
    
    bool character::is_army_owner() const {
      return data.get(system::has_army);
    }
    
    character* character::get_father() const {
      character* c = nullptr;
      c = family.parents[0] != nullptr && family.parents[0]->is_male() ? family.parents[0] : c;
      c = family.parents[1] != nullptr && family.parents[1]->is_male() ? family.parents[1] : c;
      return c;
    }
    
    character* character::get_mother() const {
      character* c = nullptr;
      c = family.parents[0] != nullptr && !family.parents[0]->is_male() ? family.parents[0] : c;
      c = family.parents[1] != nullptr && !family.parents[1]->is_male() ? family.parents[1] : c;
      return c;
    }
    
    void character::set_dead() {
      data.set(system::dead, true);
      // разрушается когда? когда некому наследовать
      //global::get<core::context>()->destroy(factions[self]);
      
//       delete modificators;
//       delete events;
//       delete flags;
      {
        phmap::flat_hash_map<const modificator*, size_t> m;
        modificators.swap(m);
      }
      
      {
        phmap::flat_hash_map<const event*, size_t> e;
        events.swap(e);
      }
      
      {
        phmap::flat_hash_map<std::string, size_t> f;
        flags.swap(f);
      }
    }
    
    void character::make_hero() {
      data.set(system::hero, true);
      // тут нужно еще создать отряд героя
    }
    
    void character::make_player() {
      data.set(system::player, true);
      // передача титулов и денег
    }
    
    void character::add_title(titulus* title) {
      if (realms[self] == nullptr && title->type > titulus::type::baron) throw std::runtime_error("Cannot give title to unlanded");
      
      if (realms[self] == nullptr) {
        realms[self] = global::get<core::context>()->create_faction();
      }
      
      realms[self]->add_title(title);
    }
    
    void character::remove_title(titulus* title) {
      ASSERT(title->owner != nullptr);
      ASSERT(realms[self] != nullptr);
      ASSERT(title->owner == realms[self]);
      
      realms[self]->remove_title(title);
      if (realms[self]->titles == nullptr) {
        global::get<core::context>()->destroy(realms[self]);
      }
    }
    
    titulus* character::get_main_title() const {
      return realms[self] ? realms[self]->main_title : nullptr;
    }
    
    void character::add_vassal(character* vassal) {
      ASSERT(vassal != this);
      ASSERT(realms[self] != nullptr);
      ASSERT(vassal->realms[self] != nullptr);
      ASSERT(realms[self] != vassal->realms[self]->liege);
      
      realms[self]->add_vassal(vassal->realms[self]);
    }
    
    void character::remove_vassal(character* vassal) {
      ASSERT(vassal != this);
      ASSERT(realms[self] != nullptr);
      ASSERT(vassal->realms[self] != nullptr);
      ASSERT(realms[self] == vassal->realms[self]->liege);
      
      realms[self]->remove_vassal(vassal->realms[self]);
    }
    
    void character::add_courtier(character* courtier) {
      ASSERT(realms[self] != nullptr);
      ASSERT(courtier->realms[self] == nullptr);
      
      if (courtier->suzerain == this) return;
      if (courtier->suzerain != nullptr) courtier->suzerain->remove_courtier(courtier);
      
      ASSERT(courtier->suzerain == nullptr);
      ASSERT(courtier->next_courtier == nullptr);
      ASSERT(courtier->prev_courtier == nullptr);
      
      courtier->suzerain = this;
      courtier->next_courtier = realms[self]->courtiers;
      if (realms[self]->courtiers != nullptr) realms[self]->courtiers->prev_courtier = courtier;
      realms[self]->courtiers = courtier;
    }
    
    void character::add_courtier_raw(character* courtier) {
      ASSERT(courtier->suzerain == this);
      ASSERT(realms[self] != nullptr);
      ASSERT(courtier->realms[self] == nullptr);
      courtier->next_courtier = realms[self]->courtiers;
      if (realms[self]->courtiers != nullptr) realms[self]->courtiers->prev_courtier = courtier;
      realms[self]->courtiers = courtier;
    }
    
    void character::remove_courtier(character* courtier) {
      ASSERT(realms[self] != nullptr);
      ASSERT(courtier->suzerain == this);
      
      courtier->suzerain = nullptr;
      if (courtier->next_courtier != nullptr) courtier->next_courtier->prev_courtier = courtier->prev_courtier;
      if (courtier->prev_courtier != nullptr) courtier->prev_courtier->next_courtier = courtier->next_courtier;
      courtier->next_courtier = nullptr;
      courtier->prev_courtier = nullptr;
    }
    
    character* character::get_last_courtier() const {
      ASSERT(realms[self] != nullptr);
      if (realms[self]->courtiers == nullptr) return nullptr;
      
      auto tmp = realms[self]->courtiers, last = realms[self]->courtiers->prev_courtier;
      while (tmp != nullptr) {
        last = tmp;
        tmp = tmp->next_courtier;
      }
      
      ASSERT(!(last == nullptr || last->next_courtier != nullptr));
      return last;
    }
    
    void character::add_prisoner(character* prisoner) {
      ASSERT(realms[self] != nullptr);
      realms[self]->add_prisoner(prisoner);
    }
    
    void character::remove_prisoner(character* prisoner) {
      ASSERT(realms[self] != nullptr);
      realms[self]->remove_prisoner(prisoner);
    }
    
    void character::add_concubine(character* concubine) {
      if (concubine->family.owner == this) return;
      concubine->family.concubines = family.concubines;
      family.concubines = concubine;
      concubine->family.owner = this;
      // надо ли делать обход в обратную сторону?
    }
    
    void character::add_concubine_raw(character* concubine) {
      ASSERT(concubine->family.owner == this);
      concubine->family.concubines = family.concubines;
      family.concubines = concubine;
    }
    
    void character::remove_concubine(character* concubine) {
      ASSERT(concubine != nullptr);
      ASSERT(concubine->family.owner == this);
      character* prev_concubine = family.concubines;
      character* current_concubine = family.concubines;
      ASSERT(current_concubine != nullptr);
      while (current_concubine != nullptr) {
        if (current_concubine == concubine) break;
        prev_concubine = current_concubine;
        current_concubine = current_concubine->family.concubines;
      }
      
      ASSERT(current_concubine != nullptr);
      if (current_concubine != family.concubines) prev_concubine->family.concubines = current_concubine->family.concubines;
      if (current_concubine == family.concubines) family.concubines = current_concubine->family.concubines;
    }
    
//     void character::add_child(character* child) {
//       
//     }
    
    void character::add_child_raw(character* child) {
      ASSERT(child->family.parents[0] == this || child->family.parents[1] == this);
      child->family.next_sibling = family.children;
      if (family.children != nullptr) family.children->family.prev_sibling = child;
      family.children = child;
    }
    
//     void character::remove_child(character* child) {
//       
//     }
    
    float character::base_stat(const uint32_t &index) const {
      ASSERT(index < character_stats::count);
      auto s = stats[index];
      switch(character_stats::types[index]) {
        case stat_type::uint_t:  return float(s.uval);
        case stat_type::int_t:   return float(s.ival);
        case stat_type::float_t: return float(s.fval);
        default: assert(false);
      }
      
      return 0.0f;
    }
    
    float character::stat(const uint32_t &index) const {
      ASSERT(index < character_stats::count);
      auto s = current_stats[index];
      switch(character_stats::types[index]) {
        case stat_type::uint_t:  return float(s.uval);
        case stat_type::int_t:   return float(s.ival);
        case stat_type::float_t: return float(s.fval);
        default: assert(false);
      }
      
      return 0.0f;
    }
    
    void character::set_stat(const uint32_t &index, const float &value) {
      ASSERT(index < character_stats::count);
      switch(character_stats::types[index]) {
        case stat_type::uint_t:  current_stats[index].uval = uint32_t(value); break;
        case stat_type::int_t:   current_stats[index].ival =  int32_t(value); break;
        case stat_type::float_t: current_stats[index].fval =    float(value); break;
        default: assert(false);
      }
    }
    
    float character::add_to_stat(const uint32_t &index, const float &value) {
      ASSERT(index < character_stats::count);
      auto s = current_stats[index];
      switch(character_stats::types[index]) {
        case stat_type::uint_t:  current_stats[index].uval += uint32_t(value); return float(s.uval);
        case stat_type::int_t:   current_stats[index].ival +=  int32_t(value); return float(s.ival);
        case stat_type::float_t: current_stats[index].fval +=    float(value); return float(s.fval);
        default: assert(false);
      }
      
      return 0.0f;
    }
    
    float character::base_hero_stat(const uint32_t &index) const {
      ASSERT(index < hero_stats::count);
      auto s = hero_stats[index];
      return float(s.ival);
    }
    
    float character::hero_stat(const uint32_t &index) const {
      ASSERT(index < hero_stats::count);
      auto s = current_hero_stats[index];
      return float(s.ival);
    }
    
    void character::set_hero_stat(const uint32_t &index, const float &value) {
      ASSERT(index < hero_stats::count);
      current_hero_stats[index].ival = int32_t(value);
    }
    
    float character::add_to_hero_stat(const uint32_t &index, const float &value) {
      auto s = current_hero_stats[index];
      current_hero_stats[index].ival += int32_t(value);
      return float(s.ival);
    }
    
    bool character::get_bit(const size_t &index) const {
      ASSERT(index < SIZE_WIDTH - system::count);
      const auto final_index = index + system::count;
      return data.get(final_index);
    }
    
    bool character::set_bit(const size_t &index, const bool value) {
      ASSERT(index < SIZE_WIDTH - system::count);
      const auto final_index = index + system::count;
      return data.set(final_index, value);
    }
    
//     bool character::has_flag(const size_t &flag) const {
//       if (is_dead()) return false; // ASSERT(flags != nullptr);
//       return flags->has(flag);
//     }
//     
//     void character::add_flag(const size_t &flag) {
//       if (is_dead()) return;
//       flags->add(flag);
//     }
//     
//     void character::remove_flag(const size_t &flag) {
//       if (is_dead()) return;
//       flags->remove(flag);
//     }

    size_t character::has_flag(const std::string_view &flag) const { return flags.find(flag) != flags.end(); }
    // откуда мы берем flag? по идее если он приходит из эвента, то мы можем даже не париться по поводу хранилища
    // другое дело что скорее всего не все так просто, а значит нам похорошему нужно хранилище строк
    void character::add_flag(const std::string_view &flag, const size_t &turn) { flags.try_emplace(std::string(flag), turn); }
    void character::remove_flag(const std::string_view &flag) { flags.erase(flag); }
    
    bool character::has_trait(const trait* t) const { return traits.find(t) != traits.end(); }
    void character::add_trait(const trait* t) { traits.emplace(t); }
    void character::remove_trait(const trait* t) { traits.erase(t); }
    
    bool character::has_modificator(const modificator* m) const { return modificators.find(m) != modificators.end(); }
    void character::add_modificator(const modificator* m, const size_t &turn) { if (is_dead()) return; modificators.try_emplace(m, turn); }
    void character::remove_modificator(const modificator* m) { if (is_dead()) return; modificators.erase(m); }
    
    bool character::has_event(const event* e) const { if (is_dead()) return false; return utils::events_container::has_event(e); }
    //void character::add_event(const event* e, const event_container &cont) { if (is_dead()) return; events.try_emplace(e, cont); }
    void character::add_event(const event* e, const size_t &data) { if (is_dead()) return; utils::events_container::add_event(e, data); }
    void character::remove_event(const event* e) { if (is_dead()) return; utils::events_container::remove_event(e); }
    
    uint64_t character::get_random() {
      using namespace utils::xoshiro256plusplus;
      rng_state = rng(rng_state);
      return get_value(rng_state);
    }
  }
}
