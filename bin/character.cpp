#include "character.h"
#include <stdexcept>
#include "utils/assert.h"
#include "utils/globals.h"
#include "generator_container.h"
#include "core_context.h"
#include "game_time.h"

namespace devils_engine {
  namespace core {
    const structure tile::s_type;
    const structure province::s_type;
    const size_t province::modificators_container_size;
    const size_t province::events_container_size;
    const size_t province::flags_container_size;
    const size_t province::cities_max_game_count;
    tile::tile() : height(0.0f), province(UINT32_MAX) {}
    province::province() : title(nullptr), cities_max_count(0), cities_count(0), cities{nullptr} {}
    const structure building_type::s_type;
    const size_t building_type::maximum_prev_buildings;
    const size_t building_type::maximum_limit_buildings;
    const size_t building_type::maximum_stat_modifiers;
    const size_t building_type::maximum_unit_stat_modifiers;
    building_type::building_type() : name_id(SIZE_MAX), desc_id(SIZE_MAX), prev_buildings{nullptr}, limit_buildings{nullptr}, replaced(nullptr), upgrades_from(nullptr), 
      time(SIZE_MAX), money_cost(0.0f), authority_cost(0.0f), esteem_cost(0.0f), influence_cost(0.0f) {}
    
    const structure city_type::s_type;
    const size_t city_type::maximum_buildings;
    city_type::city_type() : buildings{nullptr}, city_image{GPU_UINT_MAX}, city_icon{GPU_UINT_MAX} { memset(stats, 0, sizeof(stats[0]) * city_stats::count); }
    const structure city::s_type;
    const size_t city::modificators_container_size;
    const size_t city::events_container_size;
    const size_t city::flags_container_size;
    const size_t city::bit_field_size;
    city::city() : province(nullptr), title(nullptr), type(nullptr), start_building(SIZE_MAX), building_index(UINT32_MAX), tile_index(UINT32_MAX) { 
      memset(current_stats, 0, sizeof(current_stats[0]) * city_stats::count);
    }
    
    bool city::check_build(character* c, const uint32_t &building_index) const {
      ASSERT(building_index < core::city_type::maximum_buildings);
      const float build_cost_mod = current_stats[city_stats::build_cost_mod].fval; // по идее к этому моменту статы должны быть все расчитаны
      const float money_cost = type->buildings[building_index]->money_cost * build_cost_mod;
      const float influence_cost = type->buildings[building_index]->influence_cost * build_cost_mod;
      const float esteem_cost = type->buildings[building_index]->esteem_cost * build_cost_mod;
      const float authority_cost = type->buildings[building_index]->authority_cost * build_cost_mod;
      return start_building == SIZE_MAX && !complited_buildings.get(building_index) && available_buildings.get(building_index) && 
        c->current_stats[character_stats::money].fval     >= money_cost && 
        c->current_stats[character_stats::influence].fval >= influence_cost && 
        c->current_stats[character_stats::esteem].fval    >= esteem_cost && 
        c->current_stats[character_stats::authority].fval >= authority_cost;
    }
    
    bool city::start_build(character* c, const uint32_t &building_index) {
      if (!check_build(c, building_index)) return false;
      
      const float build_cost_mod = current_stats[city_stats::build_cost_mod].fval;
      const float money_cost = type->buildings[building_index]->money_cost * build_cost_mod;
      const float influence_cost = type->buildings[building_index]->influence_cost * build_cost_mod;
      const float esteem_cost = type->buildings[building_index]->esteem_cost * build_cost_mod;
      const float authority_cost = type->buildings[building_index]->authority_cost * build_cost_mod;
      
      c->current_stats[character_stats::money].fval -= money_cost;
      c->current_stats[character_stats::influence].fval -= influence_cost;
      c->current_stats[character_stats::esteem].fval -= esteem_cost;
      c->current_stats[character_stats::authority].fval -= authority_cost;
      start_building = global::get<const utils::calendar>()->current_turn();
      this->building_index = building_index;
      
      return true;
    }
    
    void city::advance_building() {
      if (start_building == SIZE_MAX) return;
      const float build_time_mod = current_stats[city_stats::build_time_mod].fval;
      const size_t build_time = type->buildings[building_index]->time * build_time_mod;
      const size_t current_turn = global::get<const utils::calendar>()->current_turn();
      if (current_turn - start_building >= build_time) {
        complited_buildings.set(building_index, true);
        start_building = SIZE_MAX;
        building_index = UINT32_MAX;
      }
    }
    
    const structure trait::s_type;
    const size_t trait::max_stat_modifiers_count;
    trait::trait() : name_str(SIZE_MAX), description_str(SIZE_MAX), numeric_attribs{0,0,0,0}, icon{GPU_UINT_MAX} {}
    const structure modificator::s_type;
    const size_t modificator::max_stat_modifiers_count;
    modificator::modificator() : name_id(SIZE_MAX), description_id(SIZE_MAX), time(0), icon{GPU_UINT_MAX} {}
    const structure troop_type::s_type;
    troop_type::troop_type() : name_id(SIZE_MAX), description_id(SIZE_MAX), card{GPU_UINT_MAX} { memset(stats, 0, sizeof(stats[0]) * troop_stats::count); }
    troop::troop() : type(nullptr), character(nullptr) { 
      memset(moded_stats, 0, sizeof(moded_stats[0]) * troop_stats::count);
      memset(current_stats, 0, sizeof(current_stats[0]) * troop_stats::count);
    }
    
    const structure army::s_type;
    const size_t army::modificators_container_size;
    const size_t army::events_container_size;
    const size_t army::flags_container_size;
    army::army() : troops_count(0), tile_index(UINT32_MAX), map_img{GPU_UINT_MAX} {}
    hero::hero() : character(nullptr) {}
    const structure hero_troop::s_type;
    const size_t hero_troop::modificators_container_size;
    const size_t hero_troop::events_container_size;
    const size_t hero_troop::flags_container_size;
    hero_troop::hero_troop() : party_size(0), max_party_size(0), tile_index(UINT32_MAX) {}
    const structure decision::s_type;
    decision::decision() : name_id(SIZE_MAX), description_id(SIZE_MAX), target(UINT32_MAX) {}
    const structure religion_group::s_type;
    const structure religion::s_type;
    religion::religion() : 
      name_str(SIZE_MAX), 
      description_str(SIZE_MAX), 
      group(nullptr), 
      parent(nullptr), 
      reformed(nullptr), 
      aggression(0.0f), 
      crusade_str(SIZE_MAX),  
      scripture_str(SIZE_MAX),
      high_god_str(SIZE_MAX),
      piety_str(SIZE_MAX),
      priest_title_str(SIZE_MAX),
      opinion_stat_index(UINT32_MAX),
      image{GPU_UINT_MAX}
    {}
    
    const structure culture::s_type;
    const size_t culture::max_stat_modifiers_count;
    culture::culture() : name_id(SIZE_MAX), name_bank(nullptr), parent(nullptr) {}
    const structure law::s_type;
    const size_t law::max_stat_modifiers_count;
    const size_t law::max_mechanics_modifiers_count;
    law::law() : name_id(SIZE_MAX), description_id(SIZE_MAX) {}
    const structure event::s_type;
    event::event() : name_id(SIZE_MAX), description_id(SIZE_MAX), target(utils::target_data::type::count), image{GPU_UINT_MAX}, mtth(SIZE_MAX), options_count(0) {}
    event::option::option() : name_id(SIZE_MAX), desc_id(SIZE_MAX) {}
    
    const structure titulus::s_type;
    const size_t titulus::events_container_size;
    const size_t titulus::flags_container_size;
    titulus::titulus() : type(type::count), count(0), childs(nullptr), parent(nullptr), owner(nullptr), name_str(UINT32_MAX), description_str(UINT32_MAX), next(nullptr), prev(nullptr) {}
    titulus::titulus(const enum type &t) : type(t), count(0), childs(nullptr), parent(nullptr), owner(nullptr), name_str(UINT32_MAX), description_str(UINT32_MAX), next(nullptr), prev(nullptr) {}
    titulus::titulus(const enum type &t, const uint32_t &count) : 
      type(t), 
      count(count), 
      childs(count < 2 ? nullptr : new titulus*[count]), 
      parent(nullptr), 
      owner(nullptr), 
      name_str(UINT32_MAX), 
      description_str(UINT32_MAX), 
      next(nullptr) 
    {}
    
    titulus::~titulus() {
      if (count >= 2) {
        delete [] childs;
      }
    }
    
    bool titulus::is_formal() const {
      return count == 0;
    }
    
    void titulus::set_child(const uint32_t &index, titulus* child) {
      if (index >= count) throw std::runtime_error("titulus wrong child index");
      if (count == 1) this->child = child;
      else this->childs[index] = child;
    }
    
    titulus* titulus::get_child(const uint32_t &index) const {
      if (index >= count) throw std::runtime_error("titulus wrong child index");
      if (count == 1) return child;
      return childs[index];
    }
    
    void titulus::set_province(struct province* province) {
      if (is_formal()) throw std::runtime_error("Could not set province to formal titulus");
      if (count > 1) throw std::runtime_error("Could not set province to titulus with childs > 1");
      if (type != type::baron) throw std::runtime_error("Bad title type");
      this->province = province;
    }
    
    struct province* titulus::get_province() const {
      if (is_formal()) throw std::runtime_error("Could not get province from formal titulus");
      if (count > 1) throw std::runtime_error("Could not get province from titulus with childs > 1");
      if (type != type::baron) throw std::runtime_error("Bad title type");
      return province;
    }

    void titulus::set_city(struct city* city) {
      if (is_formal()) throw std::runtime_error("Could not set city to formal titulus");
      if (count > 1) throw std::runtime_error("Could not set city to titulus with childs > 1");
      if (type != type::city) throw std::runtime_error("Bad title type");
      this->city = city;
    }
    
    struct city* titulus::get_city() const {
      if (is_formal()) throw std::runtime_error("Could not get city from formal titulus");
      if (count > 1) throw std::runtime_error("Could not get province from titulus with childs > 1");
      if (type != type::city) throw std::runtime_error("Bad title type");
      return city;
    }
    
    void titulus::create_children(const uint32_t &count) {
      this->count = 1;
      if (type == type::baron) return;
      if (type == type::city) return;
      if (count < 2) return;
      
      ASSERT(childs == nullptr);
      
      this->count = count;
      childs = new titulus*[count];
      memset(childs, 0, sizeof(titulus*) * count);
    }
    
    namespace system {
      enum character_enum {
        player,
        hero,
        male,
        dead,
        
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
      name_str(SIZE_MAX), 
      nickname_str(SIZE_MAX), 
      suzerain(nullptr),
      imprisoner(nullptr),
      next_prisoner(nullptr),
      prev_prisoner(nullptr),
      next_courtier(nullptr),
      prev_courtier(nullptr),
      culture(nullptr),
      religion(nullptr),
      hidden_religion(nullptr),
      factions{nullptr, nullptr, nullptr, nullptr},
      modificators(nullptr),
      events(nullptr),
      flags(nullptr)
    {
      data.set(system::male, male);
      data.set(system::dead, dead);
      if (!dead) {
        modificators = new modificators_container<modificators_container_size>;
        events = new events_container<events_container_size>;
        flags = new flags_container<flags_container_size>;
      }
    }
    
    character::~character() {
      delete modificators;
      delete events;
      delete flags;
    }
    
    bool character::is_independent() const {
      if (factions[self] == nullptr) {
        ASSERT(suzerain != nullptr);
        return false;
      }
      
      return factions[self] != nullptr && factions[self]->is_independent(); //  || suzerain == nullptr // персонаж без титулов и не придворный это ошибка!
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
      return factions[self] != nullptr && factions[self]->main_title->type != titulus::type::city;
    }
    
    void character::set_dead() {
      data.set(system::dead, true);
      // разрушается когда? когда некому наследовать
      //global::get<core::context>()->destroy(factions[self]);
      
      delete modificators;
      delete events;
      delete flags;
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
      if (factions[self] == nullptr && title->type > titulus::type::baron) throw std::runtime_error("Cannot give title to unlanded");
      
      if (factions[self] == nullptr) {
        factions[self] = global::get<core::context>()->create_faction();
      }
      
      factions[self]->add_title(title);
    }
    
    void character::remove_title(titulus* title) {
      ASSERT(title->owner != nullptr);
      ASSERT(factions[self] != nullptr);
      ASSERT(title->owner == factions[self]);
      
      factions[self]->remove_title(title);
      if (factions[self]->titles == nullptr) {
        global::get<core::context>()->destroy(factions[self]);
      }
    }
    
    titulus* character::get_main_title() const {
      return factions[self] ? factions[self]->main_title : nullptr;
    }
    
    void character::add_vassal(character* vassal) {
      ASSERT(vassal != this);
      ASSERT(factions[self] != nullptr);
      ASSERT(vassal->factions[self] != nullptr);
      ASSERT(factions[self] != vassal->factions[self]->liege);
      
      factions[self]->add_vassal(vassal->factions[self]);
    }
    
    void character::remove_vassal(character* vassal) {
      ASSERT(vassal != this);
      ASSERT(factions[self] != nullptr);
      ASSERT(vassal->factions[self] != nullptr);
      ASSERT(factions[self] == vassal->factions[self]->liege);
      
      factions[self]->remove_vassal(vassal->factions[self]);
    }
    
    void character::add_courtier(character* courtier) {
      ASSERT(factions[self] != nullptr);
      ASSERT(courtier->factions[self] == nullptr);
      
      if (courtier->suzerain == this) return;
      if (courtier->suzerain != nullptr) courtier->suzerain->remove_courtier(courtier);
      
      ASSERT(courtier->suzerain == nullptr);
      ASSERT(courtier->next_courtier == nullptr);
      ASSERT(courtier->prev_courtier == nullptr);
      
      courtier->suzerain = this;
      courtier->next_courtier = factions[self]->courtiers;
      if (factions[self]->courtiers != nullptr) factions[self]->courtiers->prev_courtier = courtier;
      factions[self]->courtiers = courtier;
    }
    
    void character::add_courtier_raw(character* courtier) {
      ASSERT(courtier->suzerain == this);
      ASSERT(factions[self] != nullptr);
      ASSERT(courtier->factions[self] == nullptr);
      courtier->next_courtier = factions[self]->courtiers;
      if (factions[self]->courtiers != nullptr) factions[self]->courtiers->prev_courtier = courtier;
      factions[self]->courtiers = courtier;
    }
    
    void character::remove_courtier(character* courtier) {
      ASSERT(factions[self] != nullptr);
      ASSERT(courtier->suzerain == this);
      
      courtier->suzerain = nullptr;
      if (courtier->next_courtier != nullptr) courtier->next_courtier->prev_courtier = courtier->prev_courtier;
      if (courtier->prev_courtier != nullptr) courtier->prev_courtier->next_courtier = courtier->next_courtier;
      courtier->next_courtier = nullptr;
      courtier->prev_courtier = nullptr;
    }
    
    character* character::get_last_courtier() const {
      ASSERT(factions[self] != nullptr);
      if (factions[self]->courtiers == nullptr) return nullptr;
      
      auto tmp = factions[self]->courtiers, last = factions[self]->courtiers->prev_courtier;
      while (tmp != nullptr) {
        last = tmp;
        tmp = tmp->next_courtier;
      }
      
      ASSERT(!(last == nullptr || last->next_courtier != nullptr));
      return last;
    }
    
    void character::add_prisoner(character* prisoner) {
      ASSERT(factions[self] != nullptr);
      factions[self]->add_prisoner(prisoner);
    }
    
    void character::remove_prisoner(character* prisoner) {
      ASSERT(factions[self] != nullptr);
      factions[self]->remove_prisoner(prisoner);
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
    
    bool character::has_flag(const size_t &flag) const {
      if (is_dead()) return false; // ASSERT(flags != nullptr);
      return flags->has(flag);
    }
    
    void character::add_flag(const size_t &flag) {
      if (is_dead()) return;
      flags->add(flag);
    }
    
    void character::remove_flag(const size_t &flag) {
      if (is_dead()) return;
      flags->remove(flag);
    }
    
    bool character::has_trait(const trait* t) const { return traits.has(t); }
    void character::add_trait(const trait* t) { traits.add(t); }
    void character::remove_trait(const trait* t) { traits.remove(t); }
    
    bool character::has_modificator(const modificator* m) const {
      if (is_dead()) return false;
      return modificators->has(m);
    }
    
    void character::add_modificator(const modificator* m, const size_t &turn) {
      if (is_dead()) return;
      modificators->add(m, turn);
    }
    
    void character::remove_modificator(const modificator* m) {
      if (is_dead()) return;
      modificators->remove(m);
    }
    
    bool character::has_event(const event* e) const {
      if (is_dead()) return false;
      return events->has(e);
    }
    
    void character::add_event(const event* e, const event_container &cont) {
      if (is_dead()) return;
      events->add(e, cont);
    }
    
    void character::remove_event(const event* e) {
      if (is_dead()) return;
      events->remove(e);
    }
    
    const structure faction::s_type;
    faction::faction() : 
      leader(nullptr), 
      heir(nullptr), 
      liege(nullptr), 
      state(nullptr), 
      council(nullptr), 
      tribunal(nullptr), 
      vassals(nullptr), 
      next_vassal(nullptr), 
      prev_vassal(nullptr), 
      titles(nullptr), 
      main_title(nullptr), 
      courtiers(nullptr),
      prisoners(nullptr) 
    {}
    
    void faction::succession() {
      ASSERT(is_state() || is_self());
      ASSERT(leader != nullptr);
      ASSERT(heir != nullptr);
      
      if (is_self()) {
        if (heir->factions[character::self] != nullptr) {
          auto f = heir->factions[character::self];
          
          {
            auto last = f->get_last_title();
            if (last != nullptr) { last->next = titles; titles = f->titles; }
          }
          
          {
            auto last = f->get_last_vassal();
            if (last != nullptr) { last->next_vassal = vassals; vassals = f->vassals; }
          }
          
          {
            auto last = f->get_last_prisoner();
            if (last != nullptr) { last->next_prisoner = prisoners; prisoners = f->prisoners; }
          }
          
          {
            auto last = heir->get_last_courtier();
            if (last != nullptr) { last->next_courtier = courtiers; courtiers = f->courtiers; }
          }
          
          global::get<core::context>()->destroy(f);
        }
        
        if (heir->factions[character::elective] != nullptr) {
          ASSERT(heir->factions[character::elective]->is_state());
          heir->factions[character::elective]->succession(); // если у наследника выборная форма, то мы просто выбираем следующего
        }
        
        heir->current_stats[core::character_stats::money].fval += leader->current_stats[core::character_stats::money].fval;
      }
      
      leader = heir;
      heir = nullptr;
      
      // где то тут мы должны вызвать функцию on_action
      // можем ли мы вызвать эвенты до смерти персонажа и передачи прав? можем по идее
      // множество эвентов запускается в этот момент (в основном проверки и передача флагов наследникам)
      // просто проверяем че там в on_action контейнере
    }
    
    void faction::add_title(titulus* title) {
      if (title->owner == this) return;
      if (title->owner != nullptr) title->owner->remove_title(title);
      
      ASSERT(title->next == nullptr);
      ASSERT(title->prev == nullptr);
      ASSERT(title->owner == nullptr);
      
      title->owner = this;
      title->next = titles;
      if (titles != nullptr) titles->prev = title;
      titles = title;
      
      if (main_title == nullptr || main_title->type < title->type) main_title = title;
      
      // не уверен что тут нужно передавать вассалов
//       for (uint32_t i = 0; i < title->count; ++i) {
//         auto child = title->get_child(i);
//         this->add_vassal(child->owner);
//       }
    }
    
    void faction::remove_title(titulus* title) {
      ASSERT(title->owner == this);
      if (this->titles == title) this->titles = title->next;
      
      title->owner = nullptr;
      if (title->prev != nullptr) title->prev->next = title->next;
      if (title->next != nullptr) title->next->prev = title->prev;
      title->next = nullptr;
      title->prev = nullptr;
      
//       for (uint32_t i = 0; i < title->count; ++i) {
//         auto child = title->get_child(i);
//         this->remove_vassal(child->owner);
//       }
      
      if (this->main_title == title) {
        auto tmp = this->titles, current = tmp;
        auto max_type = tmp->type;
        while (tmp != nullptr) {
          if (max_type < tmp->type) {
            current = tmp;
            max_type = tmp->type;
          }
          
          tmp = tmp->next;
        }
        
        this->main_title = current;
      }
    }
    
    titulus* faction::get_last_title() const {
      if (titles == nullptr) return nullptr;
      
      auto tmp = titles, last = titles->prev;
      while (tmp != nullptr) {
        last = tmp;
        tmp = tmp->next;
      }
      
      ASSERT(!(last == nullptr || last->next != nullptr));
      return last;
    }
    
    void faction::add_vassal(faction* vassal) {
      if (vassal->liege == this) return;
      if (vassal->liege != nullptr) vassal->liege->remove_vassal(vassal);
      
      ASSERT(vassal->liege == nullptr);
      ASSERT(vassal->next_vassal == nullptr);
      ASSERT(vassal->prev_vassal == nullptr);
      
      vassal->liege = this;
      vassal->next_vassal = vassals;
      if (vassals != nullptr) vassals->prev_vassal = vassal;
      vassals = vassal;
    }
    
    void faction::add_vassal_raw(faction* vassal) {
      ASSERT(vassal->liege == this);
      vassal->next_vassal = vassals;
      if (vassals != nullptr) vassals->prev_vassal = vassal;
      vassals = vassal;
    }
    
    void faction::remove_vassal(faction* vassal) {
      if (vassal->liege != this) return;
      
      vassal->liege = nullptr;
      if (vassal->prev_vassal != nullptr) vassal->prev_vassal->next_vassal = vassal->next_vassal;
      if (vassal->next_vassal != nullptr) vassal->next_vassal->prev_vassal = vassal->prev_vassal;
      vassal->prev_vassal = nullptr;
      vassal->next_vassal = nullptr;
    }
    
    faction* faction::get_last_vassal() const {
      if (vassals == nullptr) return nullptr;
      
      auto tmp = vassals, last = vassals->prev_vassal;
      while (tmp != nullptr) {
        last = tmp;
        tmp = tmp->next_vassal;
      }
      
      ASSERT(!(last == nullptr || last->next_vassal != nullptr));
      return last;
    }
    
    void faction::add_prisoner(struct character* prisoner) {
      if (prisoner->imprisoner == this) return;
      if (prisoner->imprisoner != nullptr) prisoner->imprisoner->remove_prisoner(prisoner);
      
      ASSERT(prisoner->imprisoner == nullptr);
      ASSERT(prisoner->next_prisoner == nullptr);
      ASSERT(prisoner->prev_prisoner == nullptr);
      
      prisoner->imprisoner = this;
      prisoner->next_prisoner = prisoners;
      if (prisoners != nullptr) prisoners->prev_prisoner = prisoner;
      prisoners = prisoner;
    }
    
    void faction::add_prisoner_raw(character* prisoner) {
      ASSERT(prisoner->imprisoner == this);
      prisoner->next_prisoner = prisoners;
      if (prisoners != nullptr) prisoners->prev_prisoner = prisoner;
      prisoners = prisoner;
    }
    
    void faction::remove_prisoner(struct character* prisoner) {
      ASSERT(prisoner->imprisoner == this);
      
      prisoner->imprisoner = nullptr;
      if (prisoner->next_prisoner != nullptr) prisoner->next_prisoner->prev_prisoner = prisoner->prev_prisoner;
      if (prisoner->prev_prisoner != nullptr) prisoner->prev_prisoner->next_prisoner = prisoner->next_prisoner;
      prisoner->next_prisoner = nullptr;
      prisoner->prev_prisoner = nullptr;
    }
    
    character* faction::get_last_prisoner() const {
      if (prisoners == nullptr) return nullptr;
      
      auto tmp = prisoners, last = prisoners->prev_prisoner;
      while (tmp != nullptr) {
        last = tmp;
        tmp = tmp->next_prisoner;
      }
      
      ASSERT(!(last == nullptr || last->next_prisoner != nullptr));
      return last;
    }
  }
}
