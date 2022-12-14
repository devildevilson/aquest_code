#include "character.h"

#include <stdexcept>

#include "utils/assert.h"
#include "utils/globals.h"
#include "utils/systems.h"
#include "utils/deferred_tasks.h"
#include "bin/game_time.h"

#include "ai/path_container.h"

#include "realm.h"
#include "titulus.h"
#include "context.h"

#include "realm_rights_checker.h"

namespace devils_engine {
  namespace core {
    
    namespace system {
      enum character_enum {
        player,
        hero,
        male,
        dead,
        has_troop,
        has_army,
        excommunicated,
        general,
        
        count
      };
    }
    
    const structure character::s_type;
    const size_t character::traits_container_size;
    const size_t character::modificators_container_size;
    const size_t character::events_container_size;
    const size_t character::flags_container_size;
    
    character::family::family() noexcept :
      real_parents{nullptr, nullptr},
      parents{nullptr, nullptr}, 
//       grandparents{nullptr, nullptr, nullptr, nullptr}, 
      children(nullptr),
      consort(nullptr), 
      owner(nullptr), 
      concubines(nullptr),
      blood_dynasty(nullptr),
      dynasty(nullptr)
    {}
    
    const size_t character::relations::max_game_acquaintance;
    character::relations::relations() noexcept :
      acquaintances{std::make_pair(nullptr, character::relations::data{})}
    {
      ASSERT(acquaintances[max_game_acquaintance-1].first == nullptr);
    }
    
//     bool character::relations::is_acquaintance(character* c, int32_t* friendship, int32_t* love) const {
//       for (const auto &pair : acquaintances) {
//         if (pair.first == c) {
//           if (friendship != nullptr) *friendship = pair.second.friendship;
//           if (love != nullptr) *love = pair.second.friendship;
//           return true;
//         }
//       }
//       
//       return false;
//     }
//     
//     bool character::relations::add_acquaintance(character* c, int32_t friendship_level, int32_t love_level) {
//       for (auto &pair : acquaintances) {
//         if (pair.first == nullptr) {
//           pair.first = c;
//           pair.second = { friendship_level, love_level };
//           ASSERT(is_acquaintance(c));
//           return true;
//         }
//       }
//       
//       return false;
//     }

//     bool character::relations::is_acquaintance(character* c, int64_t* type) const {
//       for (const auto &pair : acquaintances) {
//         if (pair.first == c) {
//           if (type != nullptr) *type = pair.second.type;
//           return true;
//         }
//       }
//       
//       return false;
//     }
//     
//     bool character::relations::add_acquaintance(character* c, int64_t type) {
//       for (auto &pair : acquaintances) {
//         if (pair.first == nullptr) {
//           pair.first = c;
//           pair.second = relations::data{ static_cast<core::relationship::values>(type) };
//           ASSERT(is_acquaintance(c));
//           return true;
//         }
//       }
//       
//       return false;
//     }
//     
//     bool character::relations::remove_acquaintance(character* c) {
//       for (auto &pair : acquaintances) {
//         if (pair.first == c) {
//           pair.first = nullptr;
//           //pair.second = { INT64_MAX };
//           pair.second = relations::data{ static_cast<core::relationship::values>(INT64_MAX) };
//           return true;
//         }
//       }
//       
//       return false;
//     }

    bool character::relations::is_acquaintance(const character* c, const size_t &type) const {
      for (const auto &pair : acquaintances) {
        if (pair.first == c) {
          if (type == SIZE_MAX) return true;
          return pair.second.types.get(type);
        }
      }
      
      return false;
    }
    
    size_t character::relations::get_acquaintance_raw(const character* c) const {
      for (const auto &pair : acquaintances) {
        // ???????????? ???????????????????? 1, ?????????????????????? ???????????? ???????????????? ?? character::relations::data
        if (pair.first == c) return pair.second.types.container[0];
      }
      
      return 0;
    }
    
    bool character::relations::is_lover(const character* c) const {
      for (const auto &pair : acquaintances) {
        if (pair.first == c) return relationship::has_love(pair.second.types);
      }
      
      return false;
    }
    
    bool character::relations::is_friend(const character* c) const {
      for (const auto &pair : acquaintances) {
        if (pair.first == c) return relationship::has_good(pair.second.types);
      }
      
      return false;
    }
    
    bool character::relations::is_rival(const character* c) const {
      for (const auto &pair : acquaintances) {
        if (pair.first == c) return relationship::has_bad(pair.second.types);
      }
      
      return false;
    }
    
    bool character::relations::is_neutral(const character* c) const {
      for (const auto &pair : acquaintances) {
        if (pair.first == c) return relationship::has_neutral(pair.second.types);
      }
      
      return false;
    }
    
    bool character::relations::add_acquaintance(character* c, const size_t &type) {
      for (auto &pair : acquaintances) {
        if (pair.first == c) {
          pair.second.types.set(type, true);
          return true;
        }
      }
      
      for (auto &pair : acquaintances) {
        if (pair.first == nullptr) {
          pair.first = c;
          pair.second.types.set(type, true);
          return true;
        }
      }
      
      return false;
    }
    
    bool character::relations::remove_acquaintance(character* c, const size_t &type) {
      for (auto &pair : acquaintances) {
        if (pair.first == c) {
          pair.second.types.set(type, false);
          if (pair.second.types.empty()) pair.first = nullptr;
          return true;
        }
      }
      
      return false;
    }
    
    // ?????????? ???? ?????? ?????? ???? ?????????????????
//     void character::relations::remove_all_neutral() {
//       for (auto &pair : acquaintances) {
//         if (pair.second.friendship == 0 || pair.second.love == 0) {
//           pair.first = nullptr;
//           pair.second = { 0, 0 };
//         }
//       }
//     }
    
    character::character(const bool male, const bool dead) : 
      name_number(0), 
      born_day(INT64_MAX), 
      death_day(INT64_MAX), 
      victims(nullptr),
      killer(nullptr),
      culture(nullptr),
      religion(nullptr),
      secret_religion(nullptr)
    {
      data.set(system::male, male);
      data.set(system::dead, dead);
      traits.reserve(traits_container_size);
      if (!dead) {
        modificators.reserve(modificators_container_size);
        events.reserve(events_container_size);
        flags.reserve(flags_container_size);
      } 
//       else global::get<core::context>()->make_dead(this); // ???????????????????? ?? ??????????????????
    }
    
    character::~character() {
//       modificators.clear(); // ???
//       modificators.reserve(0);
//       flags.clear();
//       flags.reserve(0);
//       events.clear();
//       events.reserve(0);
    }
    
    // ???????? ???? ?? ?????????????????? ??????????? ?????? ?????????? ???????????
    bool character::is_landed() const {
      if (!self.valid()) return false;
      
      for (auto t = self->titles; t != nullptr; t = utils::ring::list_next<utils::list_type::titles>(t, self->titles)) {
        if (t->type() == core::titulus::type::city) return true;
      }
      
      return false;
    }
    
    bool character::is_independent() const {
      if (!self.valid()) {
        ASSERT(suzerain != nullptr);
        return false;
      }
      
      return self->is_independent_realm(); //  || suzerain == nullptr // ???????????????? ?????? ?????????????? ?? ???? ???????????????????? ?????? ????????????!
    }
    
    // ?????????? ?????????????????????? ?? ?????? ?????????? ???????????? ???? ???????????? ???????????????? ?????? ????????????????????
    bool character::is_prisoner() const { return prison.valid(); }
    bool character::is_married() const { return family.consort != nullptr; }
    bool character::is_male() const { return data.get(system::male); }
    bool character::is_female() const { return !is_male(); }
    bool character::is_adult() const { return get_age() >= CHARACTER_ADULT_AGE; }
    
    bool character::is_player() const { return data.get(system::player); }
    bool character::is_dead() const { return data.get(system::dead); }
    bool character::has_dynasty() const { return family.dynasty != nullptr || family.blood_dynasty != nullptr; }
    bool character::is_ai_playable() const { return self.valid() && self->main_title->type() != titulus::type::city; }
    bool character::is_troop_leader() const { return troop.valid(); }
    bool character::is_army_commander() const { return army.valid(); }
    bool character::is_excommunicated() const { return data.get(system::excommunicated); }
    bool character::is_general() const { return data.get(system::general); }
    bool character::is_bastard() const {
      for (const auto trait : traits) {
        if (trait->get_attrib(core::trait_attributes::is_bastard)) return true;
      }
      return false;
    }
    
    // ?????????????? ?? ???? ?????? ???????????????? ?? ?????????????? ????????????, ?????????????????? ???????? ???????? ???????????????? ???????????????????? ???????? ???????????????????? ???? ?????? ?????????????? ??????????????????
    bool character::is_concubine_child() const {
      for (const auto trait : traits) {
        if (trait->get_attrib(core::trait_attributes::is_concubine_child)) return true;
      }
      return false;
    }
    
    bool character::is_establishment_member() const { return realms[establishment].valid(); }
    bool character::is_council_member() const       { return realms[council].valid(); }
    bool character::is_tribunal_member() const      { return realms[tribunal].valid(); }
    bool character::is_assembly_member() const      { return realms[assembly].valid(); }
    bool character::is_clergy_member() const        { return realms[clergy].valid(); }
    
    bool character::is_elector() const               { return is_establishment_elector() || is_council_elector() || is_tribunal_elector() || is_assembly_elector() || is_clergy_elector(); }
    bool character::is_establishment_elector() const { return electorate[establishment].valid(); }
    bool character::is_council_elector() const       { return electorate[council].valid(); }
    bool character::is_tribunal_elector() const      { return electorate[tribunal].valid(); }
    bool character::is_assembly_elector() const      { return electorate[assembly].valid(); }
    bool character::is_clergy_elector() const        { return electorate[clergy].valid(); }
    
    bool character::is_ai() const { return !is_player(); }
    bool character::is_alive() const { return !is_dead(); }
    bool character::is_vassal() const { return self.valid() && !is_independent(); }
    bool character::is_courtier() const { return suzerain.valid(); }
    bool character::is_pleb() const { return !has_dynasty(); }
    bool character::is_noble() const { return has_dynasty(); }
    
    bool character::has_self_realm() const { return self.valid(); }
    bool character::has_owner() const { return family.owner != nullptr; }
    bool character::has_father() const { return get_father() != nullptr; }
    bool character::has_mother() const { return get_mother() != nullptr; }
    double character::get_age() const { return double(compute_age()); }
    
    // ?????????? ???????????????? ???????????????? ???????????
    bool character::is_hero() const {
      for (const auto trait : traits) {
        if (trait->get_attrib(core::trait_attributes::is_hero)) return true;
      }
      return false;
    }
    
    bool character::is_priest() const {
      for (const auto trait : traits) { if (trait->get_attrib(core::trait_attributes::is_priest)) return true; }
      return false;
    }
    
    bool character::is_sick() const {
      for (const auto trait : traits) { 
        if (trait->get_attrib(core::trait_attributes::is_disease) || trait->get_attrib(core::trait_attributes::is_magic_disease)) return true;
      }
      return false;
    }
    
    bool character::is_in_war() const {
      // ???????????????? ?????????? ???????? ?? ?????????? ??????????? ???????? ?????????? ?????????? ?? ?????????????? ???? ?????????? ?????? ?????? ?????????????????????? ?????????? 
      return false;
    }
    
    bool character::is_in_society() const { // ?????? ?????? ?????????? ???????????????????????
      return false;
    }
    
    bool character::is_clan_head() const {
      // ???????????????? ?????????????????? ???? ?????????? ????????????????
      return false;
    }
    
    bool character::is_religious_head() const { return religion->head == this; }
    
    bool character::is_father_alive() const {
      auto c = get_father();
      return c != nullptr && c->is_alive();
    }
    
    bool character::is_mother_alive() const {
      auto c = get_mother();
      return c != nullptr && c->is_alive();
    }
    
    bool character::is_among_most_powerful_vassals() const {
      // ?????? ???????? ???????????????
      return false;
    }
    
    bool character::can_change_religion() const {
      // ???? ???????? ???????????? ???????? ?????????? ???? ??????????
      return true;
    }
    
    bool character::can_call_crusade() const {
      // ???????????? ???????? ???????????????? ???????????? ??????????????? ?? ?????? ??????? ???? ???????????? ???????? ?????????????? ???????????????????
    }
    
    bool character::can_grant_title() const {
      // ?????? ???????????? ???????????? ???????? ?????????? ???? ???????????????? ????????????????, ???????????????? ???? ???????????? ?????????????????? ?????????????????????
    }
    
    bool character::can_marry() const {
      if (is_married()) return false;
      
      for (const auto trait : traits) {
        if (trait->get_attrib(core::trait_attributes::cannot_marry)) return false;
      }
      
      return true;
    }
    
    bool character::has_raised_armies() const {
      if (!self.valid()) return false;
      
      for (auto t = self->titles; t != nullptr; t = utils::ring::list_next<utils::list_type::titles>(t, self->titles)) {
        if (t->type() != core::titulus::type::baron) continue;
        auto p = t->get_province();
        // ?????????? ???? ???? ???????? ?????????? ?? ??????????????????? ???????????? ?????????? ????????
        // ???? ???????? ???? ?????????????????? ?????? ?????????????? ???? ?????????? ??????????
        if (!p->offensive_army.valid()) continue;
        if (p->offensive_army->state != core::army::state::stationed) return true;
      }
      
      return false;
    }
    
    bool character::can_become_stateman() const { return rights::can_become_stateman(this); }
    bool character::can_become_councillor() const { return rights::can_become_councillor(this); }
    bool character::can_become_magistrate() const { return rights::can_become_magistrate(this); }
    bool character::can_become_assembler() const { return rights::can_become_assembler(this); }
    bool character::can_become_clergyman() const { return rights::can_become_clergyman(this); }
    bool character::has_right_to_become_stateman() const { return rights::has_right_to_become_stateman(this); }
    bool character::has_right_to_become_councillor() const { return rights::has_right_to_become_councillor(this); }
    bool character::has_right_to_become_magistrate() const { return rights::has_right_to_become_magistrate(this); }
    bool character::has_right_to_become_assembler() const { return rights::has_right_to_become_assembler(this); }
    bool character::has_right_to_become_clergyman() const { return rights::has_right_to_become_clergyman(this); }
    bool character::can_become_stateman_elector() const { return rights::can_become_stateman_elector(this); }
    bool character::can_become_councillor_elector() const { return rights::can_become_councillor_elector(this); }
    bool character::can_become_magistrate_elector() const { return rights::can_become_tribunal_elector(this); }
    bool character::can_become_assembler_elector() const { return rights::can_become_assembly_elector(this); }
    bool character::can_become_clergyman_elector() const { return rights::can_become_clergyman_elector(this); }
    bool character::has_right_to_become_stateman_elector() const { return rights::has_right_to_become_stateman_elector(this); }
    bool character::has_right_to_become_councillor_elector() const { return rights::has_right_to_become_councillor_elector(this); }
    bool character::has_right_to_become_magistrate_elector() const { return rights::has_right_to_become_tribunal_elector(this); }
    bool character::has_right_to_become_assembler_elector() const { return rights::has_right_to_become_assembly_elector(this); }
    bool character::has_right_to_become_clergyman_elector() const { return rights::has_right_to_become_clergyman_elector(this); }
    bool character::can_apply_to_the_state() const { return rights::can_apply_to_the_state(this); }
    bool character::can_apply_to_the_tribunal() const { return rights::can_apply_to_the_tribunal(this); }
    bool character::can_apply_to_the_council() const { return rights::can_apply_to_the_council(this); }
    bool character::can_apply_to_the_assembly() const { return rights::can_apply_to_the_assembly(this); }
    bool character::can_apply_to_the_clergy() const { return rights::can_apply_to_the_clergy(this); }
    bool character::has_right_to_apply_to_the_state() const { return rights::has_right_to_apply_to_the_state(this); }
    bool character::has_right_to_apply_to_the_tribunal() const { return rights::has_right_to_apply_to_the_tribunal(this); }
    bool character::has_right_to_apply_to_the_council() const { return rights::has_right_to_apply_to_the_council(this); }
    bool character::has_right_to_apply_to_the_assembly() const { return rights::has_right_to_apply_to_the_assembly(this); }
    bool character::has_right_to_apply_to_the_clergy() const { return rights::has_right_to_apply_to_the_clergy(this); }
    
    bool character::has_culture(const core::culture* culture) const { return this->culture == culture; }
    bool character::has_culture_group(const core::culture_group* group) const { return culture->group == group; }
    bool character::has_religion(const core::religion* religion) const { return this->religion == religion; }
    bool character::has_religion_group(const core::religion_group* group) const { return religion->group == group; }
    
    bool character::is_at_war_with(const core::character* with) const {
      const auto itr = diplomacy.find(with);
      if (itr == diplomacy.end()) return false;
      return itr->second.types.get(diplomacy::war_attacker) || itr->second.types.get(diplomacy::war_defender);
    }
    
    bool character::is_allied_in_war(const core::character* with) const {
      for (const auto &pair : diplomacy) {
        if (pair.first == with && (pair.second.types.get(diplomacy::war_attacker) || pair.second.types.get(diplomacy::war_defender))) return false;
        if (!pair.second.types.get(diplomacy::war_attacker) || !pair.second.types.get(diplomacy::war_defender)) continue;
        auto war = pair.second.war;
        assert(war.valid());
        if (pair.second.types.get(diplomacy::war_attacker)) {
          for (const auto &attacker : war->attackers) {
            if (attacker == with) return true;
          }
        }
        
        if (pair.second.types.get(diplomacy::war_defender)) {
          for (const auto &defender : war->defenders) {
            if (defender == with) return true;
          }
        }
      }
      
      return false;
    }
    
    bool character::is_liege_or_above(const core::character* l) const {
      utils::handle<core::realm> liege;
      if (!self.valid() && !realms[establishment].valid()) {
        liege = suzerain;
      } else if (realms[establishment].valid() && realms[establishment]->leader == this) {
        liege = realms[establishment]->liege;
      } else {
        liege = self->liege;
      }
      
      while (liege.valid() && liege->leader != l) {
        liege = liege->liege;
      }
      
      return !liege.valid();
    }
    
    bool character::is_vassal_or_below(const core::character* l) const {
      return l->is_liege_or_above(this);
    }
    
    bool character::has_love_relationship(const core::character* l) const {
      return relations.is_lover(l);
    }
    
    bool character::has_good_relationship(const core::character* l) const {
      return relations.is_friend(l);
    }
    
    bool character::has_bad_relationship(const core::character* l) const {
      return relations.is_rival(l);
    }
    
    bool character::has_neutral_relationship(const core::character* l) const {
      return relations.is_neutral(l);
    }
    
    bool character::is_child_of(const core::character* of) const { return family.parents[0] == of || family.parents[1] == of; }
    bool character::is_parent_of(const core::character* of) const { return of->is_child_of(this); }
    bool character::is_sibling_of(const core::character* of) const { 
      return family.parents[0] != nullptr && family.parents[1] != nullptr && 
            ((family.parents[0] == of->family.parents[0] && family.parents[1] == of->family.parents[1]) || 
             (family.parents[0] == of->family.parents[1] && family.parents[1] == of->family.parents[0]));
    }
    
    bool character::is_half_sibling_of(const core::character* of) const {
      return ((family.parents[0] != nullptr && (family.parents[0] == of->family.parents[0] || family.parents[0] == of->family.parents[1])) || 
              (family.parents[1] != nullptr && (family.parents[1] == of->family.parents[0] || family.parents[1] == of->family.parents[1]))) &&
              !is_sibling_of(of);
    }
    
    bool character::is_grandchild_of(const core::character* of) const {
      for (uint8_t i = 0; i < 2; ++i) {
        const auto p = family.parents[i];
        for (uint8_t j = 0; j < 2; ++j) {
          const auto g = p->family.parents[i];
          if (g == of) return true;
        }
      }
      
      return false;
    }
    
    bool character::is_grandparent_of(const core::character* of) const {
      return of->is_grandchild_of(this);
    }
    
    bool character::is_close_relative_of(const core::character* of) const {
      return this == of || is_child_of(of) || is_parent_of(of) || is_sibling_of(of) || is_half_sibling_of(of) || is_grandparent_of(of) || is_grandchild_of(of);
    }
    
    bool character::is_uncle_or_aunt_of(const core::character* of) const {
      return (of->family.parents[0] != nullptr && is_sibling_of(of->family.parents[0])) || (of->family.parents[1] != nullptr && is_sibling_of(of->family.parents[1]));
    }
    
    bool character::is_cousin_of(const core::character* of) const {
      return (family.parents[0] != nullptr && family.parents[0]->is_uncle_or_aunt_of(of)) || (family.parents[1] != nullptr && family.parents[1]->is_uncle_or_aunt_of(of));
    }
    
    bool character::is_nibling_of(const core::character* of) const {
      return (family.parents[0] != nullptr && family.parents[0]->is_sibling_of(of)) || (family.parents[1] != nullptr && family.parents[1]->is_sibling_of(of));
    }
    
    bool character::is_extended_relative_of(const core::character* of) const {
      return is_uncle_or_aunt_of(of) || is_cousin_of(of) || is_nibling_of(of);
    }
    
    bool character::is_close_or_extended_relative_of(const core::character* of) const {
      return is_close_relative_of(of) || is_extended_relative_of(of);
    }
    
    // ?? ???????? ?????? ????????????????? ?????????????? ????????????, ???? ?????????? ???????????????????? ?????????? ???????? ???? ??????
    bool character::is_blood_relative_of(const core::character* of) const {
      return (family.blood_dynasty != nullptr && family.blood_dynasty == of->family.blood_dynasty) || is_close_or_extended_relative_of(of);
    }
    
    // ?????? ?????????????? ?????????????? ??????????????????????????
    bool character::is_relative_of(const core::character* of) const {
      return (of->family.consort != nullptr && is_close_relative_of(of->family.consort));
    }
    
    bool character::is_owner_of(const core::character* of) const {
      return of->family.owner == this;
    }
    
    bool character::is_concubine_of(const core::character* of) const {
      return of->is_owner_of(this);
    }
    
    void character::set_dead() {
      data.set(system::dead, true);

      {
        decltype(utils::modificators_container::modificators) m;
        modificators.swap(m);
      }
      
      {
        decltype(utils::events_container::events) e;
        events.swap(e);
      }
      
      // ?????????? ???? ?????????????? ??????????? ?????????? ???????????????? ???????????? ???? ?????? ?????????? ??????????
      utils::flags_container::clear_timed_flags();
//       {
//         decltype(utils::flags_container::flags) f;
//         flags.swap(f);
//       }
      
      {
        decltype(utils::hooks_container::hooks) h;
        hooks.swap(h);
      }
      
      death_day = global::get<utils::calendar>()->current_day();
      global::get<core::context>()->make_dead(this);
    }
    
    void character::make_hero() {
      data.set(system::hero, true);
      // ?????? ?????????? ?????? ?????????????? ?????????? ??????????
    }
    
    void character::make_player() {
      data.set(system::player, true);
      // ???????????????? ?????????????? ?? ??????????
    }
    
    void character::make_excommunicated() {
      data.set(system::excommunicated, true);
    }
    
    void character::make_not_excommunicated() {
      data.set(system::excommunicated, false);
    }
    
    void character::make_general() {
      data.set(system::general, true);
    }
    
    void character::fire_general() {
      data.set(system::general, false);
    }
    
    void character::add_title(titulus* title) {
      if (is_dead()) throw std::runtime_error("Could not give title to deadman");
      const bool first_title = !self.valid();
      if (first_title && title->type() > titulus::type::baron) throw std::runtime_error("Cannot give title to unlanded");
      if (first_title || self->main_title->type() < titulus::type::baron) global::get<core::context>()->make_playable(this);
      if (first_title) self = global::get<core::context>()->create_realm();
      self->add_title(title);
    }
    
    void character::remove_title(titulus* title) {
      ASSERT(self.valid());
      ASSERT(title->owner == self);
      
      self->remove_title(title);
      if (self->main_title == nullptr || self->main_title->type() == titulus::type::city) {
        global::get<core::context>()->make_not_playable(this);
      }
      
      if (self->titles == nullptr) {
        // ?????? ???????????????????????? ?????????????? ???? ???????????????????? ?????????????
        const size_t token = self.get_token();
        global::get<core::context>()->destroy_realm(token);
      }
    }
    
    titulus* character::get_main_title() const {
      return self.valid() ? self->main_title : nullptr;
    }
    
    void character::add_vassal(character* vassal) {
      ASSERT(vassal != this);
      ASSERT(self.valid());
      ASSERT(vassal->self.valid());
      ASSERT(self != vassal->self->liege);
      
      self->add_vassal(vassal->self.get());
    }
    
    void character::remove_vassal(character* vassal) {
      ASSERT(vassal != this);
      ASSERT(self != nullptr);
      ASSERT(self != nullptr);
      ASSERT(self == vassal->self->liege);
      
      self->remove_vassal(vassal->self.get());
    }
    
    void character::add_prisoner(character* prisoner) {
      ASSERT(self != nullptr);
      self->add_prisoner(prisoner);
    }
    
    void character::remove_prisoner(character* prisoner) {
      ASSERT(self != nullptr);
      self->remove_prisoner(prisoner);
    }
    
    void character::add_concubine(character* concubine) {
      if (concubine->family.owner == this) return;
      
      if (family.concubines == nullptr) family.concubines = concubine;
      else utils::ring::list_radd<utils::list_type::concubines>(family.concubines, concubine);
    }
    
    void character::add_concubine_raw(character* concubine) {
      ASSERT(concubine->family.owner == this);
      
      if (family.concubines == nullptr) family.concubines = concubine;
      else utils::ring::list_radd<utils::list_type::concubines>(family.concubines, concubine);
    }
    
    void character::remove_concubine(character* concubine) {
      ASSERT(concubine->family.owner == this);
      
      concubine->family.owner = nullptr;
      if (family.concubines == concubine) family.concubines = utils::ring::list_next<utils::list_type::concubines>(concubine, concubine);
      utils::ring::list_remove<utils::list_type::concubines>(concubine);
    }
    
    void character::add_child_raw(character* child) {
      ASSERT(child->family.parents[0] == this || child->family.parents[1] == this);
      
      if (family.children == nullptr) family.children = child;
      else {
        if (is_male()) utils::ring::list_radd<utils::list_type::father_line_siblings>(family.children, child);
        else utils::ring::list_radd<utils::list_type::mother_line_siblings>(family.children, child);
      }
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

    bool character::has_flag(const std::string_view flag) const { return utils::flags_container::has_flag(flag); }
    // ???????????? ???? ?????????? flag? ???? ???????? ???????? ???? ???????????????? ???? ????????????, ???? ???? ?????????? ???????? ???? ???????????????? ???? ???????????? ??????????????????
    // ???????????? ???????? ?????? ???????????? ?????????? ???? ?????? ?????? ????????????, ?? ???????????? ?????? ???????????????????? ?????????? ?????????????????? ??????????
    void character::add_flag(const std::string_view &flag, const size_t &turn) { flags.try_emplace(std::string(flag), turn); }
    void character::remove_flag(const std::string_view &flag) { flags.erase(flag); }
    
    bool character::has_trait(const trait* t) const { return traits.find(t) != traits.end(); }
    void character::add_trait(const trait* t) { traits.emplace(t); }
    void character::remove_trait(const trait* t) { traits.erase(t); }
    
    bool character::has_modificator(const modificator* m) const { return modificators.find(m) != modificators.end(); }
    void character::add_modificator(const modificator* m, const utils::modificators_container::modificator_data &data) { if (is_dead()) return; modificators.try_emplace(m, data); }
    void character::remove_modificator(const modificator* m) { if (is_dead()) return; modificators.erase(m); }
    
    bool character::has_event(const event* e) const { if (is_dead()) return false; return utils::events_container::has_event(e); }
    //void character::add_event(const event* e, const event_container &cont) { if (is_dead()) return; events.try_emplace(e, cont); }
    void character::add_event(const event* e, utils::events_container::event_data &&data) { if (is_dead()) return; utils::events_container::add_event(e, std::move(data)); }
    void character::remove_event(const event* e) { if (is_dead()) return; utils::events_container::remove_event(e); }
    
    bool character::has_same_culture_as(const character* other) const { return get_culture() == other->get_culture(); }
    bool character::has_same_culture_group_as(const character* other) const { return get_culture_group() == other->get_culture_group(); }
    bool character::has_same_religion_as(const character* other) const { return get_religion() == other->get_religion(); }
    bool character::has_same_religion_group_as(const character* other) const { return get_religion_group() == other->get_religion_group(); }
    
    uint64_t character::get_random() {
      using namespace utils::xoshiro256plusplus;
      rng_state = rng(rng_state);
      return get_value(rng_state);
    }
    
    uint32_t character::compute_age() const {
      auto cal = global::get<utils::calendar>();
      if (death_day == INT64_MAX) {
        const int64_t current_day = cal->current_day();
        assert(born_day <= current_day);
        const int64_t days = current_day - born_day;
        assert(days >= 0);
        return cal->days_to_years(days);
      }
      
      assert(born_day <= death_day);
      const int64_t days = death_day - born_day;
      assert(days >= 0);
      return cal->days_to_years(days);
    }
    
    core::character* character::get_killer() const {
      return killer;
    }
    
    core::character* character::get_employer() const {
      return nullptr;
    }
    
    core::character* character::get_concubinist() const {
      return family.owner;
    }
    
    core::character* character::get_primary_consort() const {
      return family.consort;
    }
    
    core::character* character::get_betrothed() const {
      return nullptr;
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
    
    core::character* character::get_real_mother() const {
      character* c = nullptr;
      c = family.real_parents[0] != nullptr && !family.real_parents[0]->is_male() ? family.real_parents[0] : c;
      c = family.real_parents[1] != nullptr && !family.real_parents[1]->is_male() ? family.real_parents[1] : c;
      return c;
    }
    
    core::character* character::get_pregnancy_assumed_father() const {
      return nullptr;
    }
    
    core::character* character::get_pregnancy_real_father() const {
      return nullptr;
    }
    
    // ?????????????????? ?? ????????????, ???? ???????? ???????????? ???????? ?? ????????????
    core::character* character::get_designated_heir() const {
      return nullptr;
    }
    
    // ?????????????????? ?? ??????????????????
    core::character* character::get_player_heir() const {
      return nullptr;
    }

    utils::handle<core::realm> character::get_suzerain() const {
      return suzerain;
    }
    
    character* character::get_imprisoner() const {
      return imprisoner;
    }
    
    utils::handle<core::realm> character::get_prison() const {
      return prison;
    }
    
    utils::handle<core::realm> character::get_self_realm() const {
      return self;
    }
    
    core::dynasty* character::get_dynasty() const {
      return family.dynasty;
    }
    
    utils::handle<core::army> character::get_commanding_army() const {
      return army;
    }
    
    utils::handle<core::hero_troop> character::get_hero_troop() const {
      return troop;
    }

    script::culture_t character::get_culture() const { return culture; }
    script::culture_group_t character::get_culture_group() const { return culture->group; }
    script::religion_t character::get_religion() const { return religion; }
    script::religion_group_t character::get_religion_group() const { return religion->group; }
    script::religion_t character::get_secret_religion() const { return secret_religion; }
    script::religion_group_t character::get_secret_religion_group() const { return secret_religion->group; }
    
    std::string_view character::object_pronoun() const {
      
    }
    
    std::string_view character::poss_pronoun() const {
      
    }
    
    std::string_view character::reflexive_pronoun() const {
      
    }
    
    std::string_view character::subject_pronoun() const {
      
    }
    
    std::string_view character::gender() const {
      return is_male() ? "male" : "female";
    }
    
    std::string_view character::man_woman() const {
      return is_male() ? "man" : "woman";
    }
    
    std::string_view character::boy_girl() const {
      return is_male() ? "boy" : "girl";
    }
    
    std::string_view character::boy_man_girl_woman() const {
      const uint32_t age = get_age();
      // ?????????? ?????????????? ????????????????????????? 16 ???????
      return is_male() ? (age >= CHARACTER_ADULT_AGE ? "man" : "boy") : (age >= CHARACTER_ADULT_AGE ? "woman" : "girl");
    }
    
    std::string_view character::spouse_type() const {
      return is_male() ? "husband" : "wife";
    }
    
    std::string_view character::parent_type() const {
      return is_male() ? "father" : "mother";
    }
    
    std::string_view character::child_type() const {
      // ?????? ?????? ????????, ??????????????, ??????????????, ?????????? ?? ??????????
      // ?? ?????????? ?? ??????
      return is_male() ? "son" : "daughter";
    }
    
    std::string_view character::master_gender() const {
      return !is_male() && culture->get_mechanic(core::culture_mechanics::has_master_gender) ? "mistress" : "master";
    }
    
    std::string_view character::lad_gender() const {
      return is_male() ? "lad" : "lass";
    }
    
    std::string_view character::lord_gender() const {
      return !is_male() && culture->get_mechanic(core::culture_mechanics::has_lord_gender) ? "lady" : "lord";
    }
    
    std::string_view character::king_gender() const {
      return !is_male() && culture->get_mechanic(core::culture_mechanics::has_king_gender) ? "queen" : "king";
    }
    
    std::string_view character::emperor_gender() const {
      return !is_male() && culture->get_mechanic(core::culture_mechanics::has_emperor_gender) ? "empress" : "emperor";
    }
    
    std::string_view character::patriarch_gender() const {
      return is_male() ? "patriarch" : "matriarch";
    }
    
    std::string_view character::sibling_gender() const {
      return is_male() ? "brother" : "sister";
    }
    
    std::string_view character::hero_gender() const {
      return !is_male() && culture->get_mechanic(core::culture_mechanics::has_hero_gender) ? "heroine" : "hero";
    }
    
    std::string_view character::wizard_gender() const {
      return !is_male() && culture->get_mechanic(core::culture_mechanics::has_wizard_gender) ? "witch" : "wizard";
    }
    
    std::string_view character::duke_gender() const {
      return !is_male() && culture->get_mechanic(core::culture_mechanics::has_duke_gender) ? "duchess" : "duke";
    }
    
    std::string_view character::count_gender() const {
      return !is_male() && culture->get_mechanic(core::culture_mechanics::has_count_gender) ? "countess" : "count";
    }
    
    std::string_view character::heir_gender() const {
      return !is_male() && culture->get_mechanic(core::culture_mechanics::has_heir_gender) ? "heiress" : "heir";
    }
    
    std::string_view character::prince_gender() const {
      return !is_male() && culture->get_mechanic(core::culture_mechanics::has_prince_gender) ? "princess" : "prince";
    }
    
    std::string_view character::baron_gender() const {
      return !is_male() && culture->get_mechanic(core::culture_mechanics::has_baron_gender) ? "baroness" : "baron";
    }
    
    static bool add_bonus(core::character* character, const stat_modifier &bonus) {
      if (bonus.invalid()) return false;
          
      if (bonus.type == core::stat_type::character_stat) {
        character->current_stats.add(bonus.stat, bonus.mod);
      }
      
      if (bonus.type == core::stat_type::hero_stat) {
        character->current_hero_stats.add(bonus.stat, bonus.mod);
      }
      
      return true;
    }
    
    void update_character_stats(core::character* character) {
      // ?????????? ?????????????????? ???????????????????? ?????????????
      character->current_stats = character->stats;
      character->current_hero_stats = character->hero_stats;
      
      // ?????????????? ????????????, ?????????????????? ?? ?????????????? +/- ?? ?????? ?????????????????????????? ?????????????????? ?????? ????????????????
      for (const auto &trait : character->traits) {
        for (size_t i = 0; i < trait->bonuses.size() && trait->bonuses[i].valid(); ++i) {
          add_bonus(character, trait->bonuses[i]);
        }
      }
      
      for (const auto &mod : character->modificators) {
        for (const auto &bonus : mod.first->bonuses) {
          if (!add_bonus(character, bonus)) break;
        }
        
        for (const auto &bonus : mod.second.bonuses) {
          if (!add_bonus(character, bonus)) break;
        }
      }
      
      for (const auto &bonus : character->religion->bonuses) {
        if (!add_bonus(character, bonus)) break;
      }
      
      for (const auto &bonus : character->culture->bonuses) {
        if (!add_bonus(character, bonus)) break;
      }
      
      // ???????????? ?? ????????????????
      
      // ???????????? ???? ?????????????????????????? ?? ????????????????
      // ?????????? ???? ???????????? ???????????? ?????????????????? ?????????????? ???????????? ???? ????????????????? ??????????, ?????????????????
      
      // ???????????????????????? ???????????? ?????????????????????? ??????????, ?????????? ???????????????????? ?? ?????????? ?? ???? ???????????? ???????????????????????????? ???? ????????????????????
      // ?? ???? ?????????????????? ???????????????????? ?? ????????????
      // ?????????? ?????????????????? ?????????? ?????????? ???????????????? ???????????????? ????????????/???????????????? ???? ?????????????? ?? ???????????????? ?? ???????????????? ?????? ?????? ?? ??????????????????/??????????
    }
  }
}
