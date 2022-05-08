#include "iterator_funcs.h"

#include "parallel_hashmap/phmap.h"

#include "utils/globals.h"
#include "utils/systems.h"
#include "core/structures_header.h"
#include "core/context.h"
#include "core/realm_rights_checker.h"

namespace devils_engine {
  namespace script {
#define DEFAULT_MAX_SIBLINGS_COUNT 20
    
    // до какого поколения? до 5го?
    static bool each_ancestor(const core::character* ch, const func_t & f, const size_t &level) {
      if (level == 6) return true;
      
      const auto anc1 = ch->family.parents[0];
      const auto anc2 = ch->family.parents[1];
      
      bool ret = true;
      ret = ret && anc1 != nullptr ? f(object(anc1)) : true;
      if (!ret) return ret;
      ret = ret && anc2 != nullptr ? f(object(anc2)) : true;
      if (!ret) return ret;
      
      if (anc1 != nullptr) ret = ret && each_ancestor(anc1, f, level+1);
      if (!ret) return ret;
      if (anc2 != nullptr) ret = ret && each_ancestor(anc2, f, level+1);
      return ret;
    }
    
    bool each_ancestor(const core::character* ch, const func_t & f) {
      return each_ancestor(ch, f, 0);
    }
    
    static void fill_brothers_set(const core::character* current, phmap::flat_hash_set<core::character*> &set) {
      {
        auto v = utils::ring::list_next<utils::list_type::father_line_siblings>(current, current);
        for (; v != nullptr; v = utils::ring::list_next<utils::list_type::father_line_siblings>(v, current)) { 
          if (v->is_male()) set.insert(v);
        }
      }
      {
        auto v = utils::ring::list_next<utils::list_type::mother_line_siblings>(current, current);
        for (; v != nullptr; v = utils::ring::list_next<utils::list_type::mother_line_siblings>(v, current)) {
          if (v->is_male()) set.insert(v);
        }
      }
    }
    
    static void fill_sisters_set(const core::character* current, phmap::flat_hash_set<core::character*> &set) {
      {
        auto v = utils::ring::list_next<utils::list_type::father_line_siblings>(current, current);
        for (; v != nullptr; v = utils::ring::list_next<utils::list_type::father_line_siblings>(v, current)) { 
          if (v->is_female()) set.insert(v);
        }
      }
      {
        auto v = utils::ring::list_next<utils::list_type::mother_line_siblings>(current, current);
        for (; v != nullptr; v = utils::ring::list_next<utils::list_type::mother_line_siblings>(v, current)) {
          if (v->is_female()) set.insert(v);
        }
      }
    }
    
    static void fill_sibling_set(const core::character* ch, phmap::flat_hash_set<core::character*> &set) {
      fill_brothers_set(ch, set);
      fill_sisters_set(ch, set);
    }
    
    bool each_sibling(const core::character* ch, const func_t & f) {
      // тут видимо придется заполнять set, было бы конечно прикольно тут использовать локальную память
      // но я заранее не знаю сколько будет всего родственников, хотя наверное вряд ли их будет больше 100
      
      phmap::flat_hash_set<core::character*> set;
      set.reserve(DEFAULT_MAX_SIBLINGS_COUNT);
      fill_sibling_set(ch, set);
      
      bool ret = true;
      for (auto s : set) {
        if (!ret) break;
        ret = f(object(s));
      }
      
      return ret;
    }
    
    bool each_child(const core::character* ch, const func_t & f) {
      bool ret = true;
      if (ch->is_male()) {
        for (auto v = ch->family.children; v != nullptr && ret; v = utils::ring::list_next<utils::list_type::father_line_siblings>(v, ch)) { ret = f(object(v)); }
      } else {
        for (auto v = ch->family.children; v != nullptr && ret; v = utils::ring::list_next<utils::list_type::mother_line_siblings>(v, ch)) { ret = f(object(v)); }
      }
      return ret;
    }
    
    bool each_brother(const core::character* ch, const func_t & f) {
      phmap::flat_hash_set<core::character*> set;
      set.reserve(DEFAULT_MAX_SIBLINGS_COUNT);
      fill_brothers_set(ch, set);
      
      bool ret = true;
      for (auto s : set) {
        if (!ret) break;
        ret = f(object(s));
      }
      return ret;
    }
    
    bool each_sister(const core::character* ch, const func_t & f) {
      phmap::flat_hash_set<core::character*> set;
      set.reserve(DEFAULT_MAX_SIBLINGS_COUNT);
      fill_sisters_set(ch, set);
      
      bool ret = true;
      for (auto s : set) {
        if (!ret) break;
        ret = f(object(s));
      }
      return ret;
    }
    
    bool each_concubine(const core::character* ch, const func_t & f) {
      bool ret = true;
      for (auto v = ch->family.concubines; v != nullptr && ret; v = utils::ring::list_next<utils::list_type::concubines>(v, ch)) { 
        ret = f(object(v));
      }
      return ret;
    }
    
    bool each_acquaintance(const core::character* ch, const func_t & f) {
      bool ret = true;
      for (const auto &pair : ch->relations.acquaintances) { 
        if (!ret) break;
        if (pair.first == nullptr) continue;
        ret = f(object(pair.first));
      }
      return ret;
    }
    
    bool each_good_acquaintance(const core::character* ch, const func_t & f) {
      bool ret = true;
      for (const auto &pair : ch->relations.acquaintances) { 
        if (!ret) break;
        if (pair.first == nullptr || !core::relationship::has_good(pair.second.types)) continue;
        ret = f(object(pair.first));
      }
      return ret;
    }
    
    bool each_bad_acquaintance(const core::character* ch, const func_t & f) {
      bool ret = true;
      for (const auto &pair : ch->relations.acquaintances) { 
        if (!ret) break;
        if (pair.first == nullptr || !core::relationship::has_bad(pair.second.types)) continue;
        ret = f(object(pair.first));
      }
      return ret;
    }
    
    bool each_love_acquaintance(const core::character* ch, const func_t & f) {
      bool ret = true;
      for (const auto &pair : ch->relations.acquaintances) { 
        if (!ret) break;
        if (pair.first == nullptr || !core::relationship::has_love(pair.second.types)) continue;
        ret = f(object(pair.first));
      }
      return ret;
    }
    
    bool each_neutral_acquaintance(const core::character* ch, const func_t & f) {
      bool ret = true;
      for (const auto &pair : ch->relations.acquaintances) { 
        if (!ret) break;
        if (pair.first == nullptr || !core::relationship::has_neutral(pair.second.types)) continue;
        ret = f(object(pair.first));
      }
      return ret;
    }
    
    bool each_parent(const core::character* ch, const func_t & f) {
      return each_ancestor(ch, f, 5);
    }
    
    bool each_claim(const core::character* ch, const func_t & f) {
      
    }
    
    bool each_de_jure_claim(const core::character* ch, const func_t & f) {
      
    }
    
    // тут разве персонаж должен быть?
    bool each_heir_to_title(const core::character* ch, const func_t & f) {
      
    }
    
    // каждый реалм где мы можем участвовать в выборах, по идее нужно просто обойти все реалмы
    // и проверить права у этих реалмов
    bool each_election_realm(const core::character* ch, const func_t & f) {
      bool ret = true;
      const auto r = ch->self;
      // можем ли мы у себя голосовать (?), возможно любой реалм где элективные выборы не может быть личным вообще
      
      for (const auto &r : ch->realms) {
        if (!r.valid()) continue;
        // смотрим где что, можем ли мы голосовать у императора будучи герцогом под управлением короля?
        // вряд ли, вассал моего вассала - не мой вассал по идее
      }
      
      return ret;
    }
    
    bool each_war(const core::character* ch, const func_t & f) {
      bool ret = true;
      for (const auto &pair : ch->diplomacy) {
        if (!ret) break;
        if (!pair.second.types.get(core::diplomacy::war_attacker) && !pair.second.types.get(core::diplomacy::war_defender)) continue;
        assert(pair.second.war.valid());
        ret = f(object(pair.second.war));
      }
      return ret;
    }
    
    // союзники/противники в войне текущего персонажа (не во всех войнах, а только в войне именно персонажа)
    // нужно ли заводить для всех вообще войн? мы можем по идее получить их из каждой войны
    bool each_war_ally(const core::character* ch, const func_t & f) {
      bool ret = true;
      for (const auto &pair : ch->diplomacy) {
        if (!ret) break;
        const auto &w = pair.second.war;
        
        if (pair.second.types.get(core::diplomacy::war_attacker)) {
          assert(pair.second.war.valid());
          if (w->get_primary_attacker() != ch) continue;
          
          for (auto ally : w->attackers) {
            if (!ret) break;
            ret = f(object(ally));
          }
        }
        
        if (pair.second.types.get(core::diplomacy::war_defender)) {
          assert(pair.second.war.valid());
          if (w->get_primary_defender() != ch) continue;
          
          for (auto ally : w->defenders) {
            if (!ret) break;
            ret = f(object(ally));
          }
        }
        
      }
      
      return ret;
    }
    
    bool each_war_enemy(const core::character* ch, const func_t & f) {
      bool ret = true;
      for (const auto &pair : ch->diplomacy) {
        if (!ret) break;
        const auto &w = pair.second.war;
        
        if (pair.second.types.get(core::diplomacy::war_attacker)) {
          assert(pair.second.war.valid());
          if (w->get_primary_attacker() != ch) continue;
          ret = f(object(w->get_primary_defender()));
          
          for (auto ally : w->defenders) {
            if (!ret) break;
            if (ally == ch) continue;
            ret = f(object(ally));
          }
        }
        
        if (pair.second.types.get(core::diplomacy::war_defender)) {
          assert(pair.second.war.valid());
          if (w->get_primary_defender() != ch) continue;
          ret = f(object(w->get_primary_attacker()));
          
          for (auto ally : w->attackers) {
            if (!ret) break;
            if (ally == ch) continue;
            ret = f(object(ally));
          }
        }
        
      }
      
      return ret;
    }
    
    bool each_ally(const core::character* ch, const func_t & f) {
      bool ret = true;
      for (const auto &pair : ch->diplomacy) {
        if (!ret) break;
        if (!pair.second.types.get(core::diplomacy::ally)) continue;
        ret = f(object(pair.first));
      }
      return ret;
    }
    
    bool each_truce_holder(const core::character* ch, const func_t & f) {
      bool ret = true;
      for (const auto &pair : ch->diplomacy) {
        if (!ret) break;
        if (!pair.second.types.get(core::diplomacy::truce_holder)) continue;
        ret = f(object(pair.first));
      }
      return ret;
    }
    
    bool each_truce_target(const core::character* ch, const func_t & f) {
      bool ret = true;
      for (const auto &pair : ch->diplomacy) {
        if (!ret) break;
        if (!pair.second.types.get(core::diplomacy::truce_receiver)) continue;
        ret = f(object(pair.first));
      }
      return ret;
    }
    
    // тут что мы должны сделать? пройти каждую провинцию и взять там армию
    bool each_army(const utils::handle<core::realm> r, const func_t & f) {
      return each_directly_owned_province(r, [&] (const object &prov) -> bool {
        const auto p = prov.get<core::province*>();
        return f(object(p->offensive_army));
      });
    }
    
    bool each_member(const utils::handle<core::realm> r, const func_t & f) {
      bool ret = true;
      if (r->is_state_independent_power()) {
        for (auto m = r->members; m != nullptr && ret; m = utils::ring::list_next<utils::list_type::statemans>(m, r->members)) {
          ret = f(object(m));
        }
      } else if (r->is_council()) { 
        for (auto m = r->members; m != nullptr && ret; m = utils::ring::list_next<utils::list_type::councilors>(m, r->members)) {
          ret = f(object(m));
        }
      } else if (r->is_assembly()) {
        for (auto m = r->members; m != nullptr && ret; m = utils::ring::list_next<utils::list_type::assemblers>(m, r->members)) {
          ret = f(object(m));
        }
      } else if (r->is_tribunal()) {
        for (auto m = r->members; m != nullptr && ret; m = utils::ring::list_next<utils::list_type::magistrates>(m, r->members)) {
          ret = f(object(m));
        }
      } else if (r->is_clergy()) {
        for (auto m = r->members; m != nullptr && ret; m = utils::ring::list_next<utils::list_type::clergymans>(m, r->members)) {
          ret = f(object(m));
        }
      } else {
        // self, что тут? тут либо совсем нет мемберов, либо это лидер и наследник (наследники?)
        // пока не знаю
      }
      return ret;
    }
    
    bool each_elector(const utils::handle<core::realm> r, const func_t & f) {
      bool ret = true;
      if (r->is_state_independent_power()) {
        for (auto m = r->electors; m != nullptr && ret; m = utils::ring::list_next<utils::list_type::state_electors>(m, r->electors)) {
          ret = f(object(m));
        }
      } else if (r->is_council()) { 
        for (auto m = r->electors; m != nullptr && ret; m = utils::ring::list_next<utils::list_type::council_electors>(m, r->electors)) {
          ret = f(object(m));
        }
      } else if (r->is_assembly()) {
        for (auto m = r->electors; m != nullptr && ret; m = utils::ring::list_next<utils::list_type::assembly_electors>(m, r->electors)) {
          ret = f(object(m));
        }
      } else if (r->is_tribunal()) {
        for (auto m = r->electors; m != nullptr && ret; m = utils::ring::list_next<utils::list_type::tribunal_electors>(m, r->electors)) {
          ret = f(object(m));
        }
      } else if (r->is_clergy()) {
        for (auto m = r->electors; m != nullptr && ret; m = utils::ring::list_next<utils::list_type::clergy_electors>(m, r->electors)) {
          ret = f(object(m));
        }
      }
      return ret;
    }
    
    // тут мы тип проверяем из кого мы выбираем, по законам может быть выбор между всеми вассалами, всеми кортирами, героями, священниками и проч
    // группы наверное могут пересекаться, для того чтобы что то понять нужно сначала сделать полный обход всех персонажей реалма
    // по идее это обход вассалов + кортиров, могут быть в реалме кто то еще? по идее нет, только либо вассалы либо кортиры, причем это две отдельные группы
    bool each_election_candidate(const utils::handle<core::realm> r, const func_t &f) {
      // до этого нужно проверить что в реалме элективная форма правления
      // этого достаточно? скорее всего достаточно либо ДОЛЖНО быть достаточно
      const bool is_elective = r->has_right(core::power_rights::elective);
      if (!is_elective) return true;
      
      const bool ret = each_courtier(r, [&] (const object &obj) -> bool {
        // тут нужно проверить какие есть права в текущем реалме
        auto ch = obj.get<core::character*>();
        const bool cand = core::rights::is_election_candidate(r.get(), ch);
        if (!cand) return true;
        const bool ret = f(object(ch));
        return ret;
      });
      if (!ret) return false;
      return each_vassal(r, [&] (const object &obj) -> bool {
        const auto v = obj.get<utils::handle<core::realm>>();
        auto ch = v->leader;
        const bool cand = core::rights::is_election_candidate(r.get(), ch);
        if (!cand) return true;
        const bool ret = f(object(ch));
        return ret;
      });
    }
    
    bool each_prisoner(const utils::handle<core::realm> r, const func_t & f) {
      bool ret = true;
      for (auto c = r->prisoners; c != nullptr && ret; c = utils::ring::list_next<utils::list_type::prisoners>(c, r->prisoners)) {
        ret = f(object(c));
      }
      return ret;
    }
    
    bool each_courtier(const utils::handle<core::realm> r, const func_t & f) {
      bool ret = true;
      for (auto c = r->courtiers; c != nullptr && ret; c = utils::ring::list_next<utils::list_type::courtiers>(c, r->courtiers)) {
        ret = f(object(c));
      }
      return ret;
    }
    
    bool each_owned_title(const utils::handle<core::realm> r, const func_t & f) {
      bool ret = true;
      for (auto t = r->titles; t != nullptr && ret; t = utils::ring::list_next<utils::list_type::titles>(t, r->titles)) {
        ret = f(object(t));
      }
      return ret;
    }
    
    bool each_realm_title(const utils::handle<core::realm> r, const func_t & f) {
      bool ret = each_owned_title(r, f);
      for (auto v = r->vassals; v != nullptr && ret; v = utils::ring::list_next<utils::list_type::vassals>(v, r->vassals)) {
        const auto h = utils::handle<core::realm>(v, v->object_token);
        ret = each_realm_title(h, f);
      }
      return ret;
    }
    
    bool each_directly_owned_province(const utils::handle<core::realm> r, const func_t & f) {
      bool ret = true;
      for (auto t = r->titles; t != nullptr && ret; t = utils::ring::list_next<utils::list_type::titles>(t, r->titles)) {
        if (t->type() != core::titulus::type::baron) continue;
        assert(t->province != nullptr);
        ret = f(object(t->province));
      }
      return ret;
    }
    
    bool each_directly_owned_city(const utils::handle<core::realm> r, const func_t & f) {
      bool ret = true;
      for (auto t = r->titles; t != nullptr && ret; t = utils::ring::list_next<utils::list_type::titles>(t, r->titles)) {
        if (t->type() != core::titulus::type::city) continue;
        assert(t->city != nullptr);
        ret = f(object(t->city));
      }
      return ret;
    }
    
    bool each_realm_city(const utils::handle<core::realm> r, const func_t & f) {
      bool ret = each_directly_owned_city(r, f);
      for (auto v = r->vassals; v != nullptr && ret; v = utils::ring::list_next<utils::list_type::vassals>(v, r->vassals)) {
        const auto h = utils::handle<core::realm>(v, v->object_token);
        ret = each_realm_city(h, f);
      }
      return ret;
    }
    
    bool each_realm_province(const utils::handle<core::realm> r, const func_t & f) {
      bool ret = each_directly_owned_province(r, f);
      for (auto v = r->vassals; v != nullptr && ret; v = utils::ring::list_next<utils::list_type::vassals>(v, r->vassals)) {
        const auto h = utils::handle<core::realm>(v, v->object_token);
        ret = each_realm_province(h, f);
      }
      return ret;
    }
    
    template <typename F>
    static phmap::flat_hash_set<core::titulus*> get_de_jure_titles(const utils::handle<core::realm> r, const F &get_func) {
      phmap::flat_hash_set<core::titulus*> duchies;
      duchies.reserve(100);
      const auto func = [&] (const object &c) -> bool {
        auto city = c.get<core::city*>();
        auto t = get_func(city->title);
        if (t != nullptr) duchies.insert(t);
        return true;
      };
      each_realm_city(r, func);
      return duchies;
    }
    
    template <typename F>
    static bool each_de_jure_title(const utils::handle<core::realm> r, const func_t &f, const F &get_func) {
      const auto &titles = get_de_jure_titles(r, get_func);
      bool ret = true;
      for (const auto t : titles) {
        if (!ret) break;
        ret = f(object(t));
      }
      return ret;
    }
    
    typedef core::titulus* (*func)(core::titulus*);
    bool each_de_jure_duchy(const utils::handle<core::realm> r, const func_t & f) {
      return each_de_jure_title(r, f, func(core::rights::get_duchy));
    }
    
    bool each_de_jure_kingdom(const utils::handle<core::realm> r, const func_t & f) {
      return each_de_jure_title(r, f, func(core::rights::get_kingdom));
    }
    
    bool each_de_jure_empire(const utils::handle<core::realm> r, const func_t & f) {
      return each_de_jure_title(r, f, func(core::rights::get_empire));
    }
    
    bool each_neighboring_top_liege(const utils::handle<core::realm> r, const func_t & f) {
      
    }
    
    bool each_neighboring_same_rank(const utils::handle<core::realm> r, const func_t & f) {
      
    }
    
    bool each_vassal(const utils::handle<core::realm> r, const func_t & f) {
      bool ret = true;
      for (auto c = r->vassals; c != nullptr && ret; c = utils::ring::list_next<utils::list_type::vassals>(c, r->vassals)) {
        ASSERT(c->object_token != SIZE_MAX);
        const utils::handle<core::realm> h(c, c->object_token);
        ret = f(object(h));
      }
      return ret;
    }
    
    bool each_vassal_or_below(const utils::handle<core::realm> r, const func_t & f) {
      bool ret = true;
      for (auto c = r->vassals; c != nullptr && ret; c = utils::ring::list_next<utils::list_type::vassals>(c, r->vassals)) {
        ASSERT(c->object_token != SIZE_MAX);
        const utils::handle<core::realm> realm(c, c->object_token);
        ret = f(object(realm));
        if (!ret) break;
        ret = each_vassal_or_below(realm, f);
      }
      return ret;
    }
    
    bool each_liege_or_above(const utils::handle<core::realm> r, const func_t & f) {
      if (!r.valid()) return true;
      const bool ret = f(r->liege);
      return ret ? each_liege_or_above(r->liege, f) : ret;
    }
    
    // у нас еще есть гости, возможно имеет смысл сделать для всех функций строгий инпут, просто добавить несколько дополнительных функций например self_realm_vassals
    // так и сделаю
    
    bool each_attacker(const utils::handle<core::war> w, const func_t & f) {
      bool ret = true;
      for (const auto &realms : w->attackers) {
        if (!ret) break;
        ret = f(object(realms));
      }
      return ret;
    }
    
    bool each_defender(const utils::handle<core::war> w, const func_t & f) {
      bool ret = true;
      for (const auto &realms : w->defenders) {
        if (!ret) break;
        ret = f(object(realms));
      }
      return ret;
    }
    
    bool each_participant(const utils::handle<core::war> w, const func_t & f) {
      const bool ret = each_attacker(w, f);
      if (!ret) return ret;
      return each_defender(w, f);
    }
    
    bool each_target_title(const utils::handle<core::war> w, const func_t & f) {
      bool ret = true;
      for (const auto &title : w->target_titles) {
        if (!ret) break;
        ret = f(object(title));
      }
      return ret;
    }
    
    
    // святые места?
    
    bool each_child_religion(const core::religion* r, const func_t & f) {
      bool ret = true;
      for (auto faith = r->children; faith != nullptr && ret; faith = utils::ring::list_next<utils::list_type::faiths>(faith, r->children)) {
        ret = f(object(faith));
      }
      return ret;
    }
    
    bool each_sibling_religion(const core::religion* r, const func_t & f) {
      bool ret = true;
      auto faith = utils::ring::list_next<utils::list_type::faiths>(r, r);
      for (; faith != nullptr && ret; faith = utils::ring::list_next<utils::list_type::faiths>(faith, r)) {
        ret = f(object(faith));
      }
      return ret;
    }
    
    bool each_believer(const core::religion* r, const func_t & f) {
      bool ret = true;
      for (auto ch = r->believers; ch != nullptr && ret; ch = utils::ring::list_next<utils::list_type::believer>(ch, r->believers)) {
        ret = f(object(ch));
      }
      return ret;
    }
    
    bool each_secret_believer(const core::religion* r, const func_t & f) {
      bool ret = true;
      for (auto ch = r->secret_believers; ch != nullptr && ret; ch = utils::ring::list_next<utils::list_type::secret_believer>(ch, r->secret_believers)) {
        ret = f(object(ch));
      }
      return ret;
    }
    
    bool each_child_culture(const core::culture* c, const func_t & f) {
      bool ret = true;
      for (auto child = c->children; child != nullptr && ret; child = utils::ring::list_next<utils::list_type::sibling_cultures>(child, c->children)) {
        ret = f(object(child));
      }
      return ret;
    }
    
    bool each_sibling_culture(const core::culture* c, const func_t & f) {
      bool ret = true;
      auto child = utils::ring::list_next<utils::list_type::sibling_cultures>(c, c);
      for (; child != nullptr && ret; child = utils::ring::list_next<utils::list_type::sibling_cultures>(child, c)) {
        ret = f(object(child));
      }
      return ret;
    }
    
    bool each_culture_member(const core::culture* c, const func_t & f) {
      bool ret = true;
      for (auto m = c->members; m != nullptr && ret; m = utils::ring::list_next<utils::list_type::culture_member>(m, c->members)) {
        ret = f(object(m));
      }
      return ret;
    }
    
    bool each_local_city(const core::province* p, const func_t & f) {
      bool ret = true;
      for (auto c = p->cities; c != nullptr && ret; c = utils::ring::list_next<utils::list_type::province_cities>(c, p->cities)) {
        ret = f(object(c));
      }
      return ret;
    }
    
    bool each_neighbour(const core::province* p, const func_t & f) {
      bool ret = true;
      auto core_ctx = global::get<systems::map_t>()->core_context.get();
      for (const uint32_t index : p->neighbours) {
        if (!ret) break;
        auto prov = core_ctx->get_entity<core::province>(index);
        ret = f(object(prov));
      }
      return ret;
    }
    
    // каждый наследник титула? электор у нас в реалме
    
    bool each_sibling_title(const core::titulus* t, const func_t & f) {
      bool ret = true;
      auto s = utils::ring::list_next<utils::list_type::sibling_titles>(t, t);
      for (; s != nullptr && ret; s = utils::ring::list_next<utils::list_type::sibling_titles>(s, t)) {
        ret = f(object(s));
      }
      return ret;
    }
    
    bool each_child_title(const core::titulus* t, const func_t & f) {
      bool ret = true;
      for (auto s = t->children; s != nullptr && ret; s = utils::ring::list_next<utils::list_type::sibling_titles>(s, t->children)) {
        ret = f(object(s));
      }
      return ret;
    }
    
    bool each_in_hierarchy(const core::titulus* t, const func_t & f) {
      bool ret = true;
      for (auto s = t->children; s != nullptr && ret; s = utils::ring::list_next<utils::list_type::sibling_titles>(s, t->children)) {
        ret = f(object(s));
        if (!ret) break;
        ret = each_in_hierarchy(s, f);
      }
      return ret;
    }
    
    static bool each_liege_title(const core::realm* owner, const core::titulus* t, const std::function<bool(core::titulus*)> &f) {
      bool ret = true;
      for (auto owned = t->children; owned != nullptr && ret; owned = utils::ring::list_next<utils::list_type::sibling_titles>(owned, t->children)) {
        if (!owned->owner.valid()) continue;
        const bool is_liege = core::rights::is_liege_or_above(owned->owner.get(), owner);
        if (!is_liege) continue;
        ret = f(owned);
        if (!ret) break;
        ret = each_liege_title(owner, owned, f);
      }
      return ret;
    }
    
    // нужно сидеть тестировать че
    bool each_in_de_facto_hierarchy(const core::titulus* t, const func_t & f) {
      if (!t->owner.valid()) return 0.0;
      bool ret = true;
      const bool same_rank_title = t->type() == t->owner->main_title->type();
      assert(same_rank_title != (t->type() < t->owner->main_title->type()));
      const auto cur_type = t->type();
      const auto owner = t->owner;
      
      for (auto owned = owner->titles; owned != nullptr && ret; owned = utils::ring::list_next<utils::list_type::titles>(owned, owner->titles)) {
        if (cur_type <= owned->type()) continue;
        ret = f(object(owned));
      }
      
      if (same_rank_title) {
        // вассалы государства и у них титулы
        for (auto owned = owner->vassals; owned != nullptr && ret; owned = utils::ring::list_next<utils::list_type::vassals>(owned, owner->vassals)) {
          auto m = owned->main_title;
          ret = f(object(m));
          if (!ret) break;
          ret = each_in_de_facto_hierarchy(m, f);
        }
      } else {
        // взят титул более низкого ранга
        // нужно пройтись по его иерархии и посмотреть кто есть
        if (ret) each_liege_title(owner.get(), t, [&] (core::titulus* t) -> bool {
          ret = f(object(t));
          return ret;
        });
      }
      
      return ret;
    }
    
    bool each_in_de_jure_hierarchy(const core::titulus* t, const func_t & f) {
      if (!t->owner.valid()) return true;
      bool ret = true;
      for (auto v = t->owner->vassals; v != nullptr && ret; v = utils::ring::list_next<utils::list_type::vassals>(v, t->owner->vassals)) {
        const bool parent = core::rights::is_parent_or_above(t, v->main_title);
        if (!parent) continue;
        ret = f(object(v->main_title));
        ret = each_in_de_jure_hierarchy(v->main_title, f);
      }
      return ret;
    }
    
    bool each_city_global(const void*, const func_t & f) {
      bool ret = true;
      auto core_ctx = global::get<systems::map_t>()->core_context.get();
      const size_t count = core_ctx->get_entity_count<core::city>();
      for (size_t i = 0; i < count && ret; ++i) {
        auto ent = core_ctx->get_entity<core::city>(i);
        ret = f(object(ent));
      }
      return ret;
    }
    
    bool each_province_global(const void*, const func_t & f) {
      bool ret = true;
      auto core_ctx = global::get<systems::map_t>()->core_context.get();
      const size_t count = core_ctx->get_entity_count<core::province>();
      for (size_t i = 0; i < count && ret; ++i) {
        auto ent = core_ctx->get_entity<core::province>(i);
        ret = f(object(ent));
      }
      return ret;
    }
    
    bool each_barony_global(const void*, const func_t & f) {
      bool ret = true;
      auto core_ctx = global::get<systems::map_t>()->core_context.get();
      const size_t count = core_ctx->get_entity_count<core::titulus>();
      for (size_t i = 0; i < count && ret; ++i) {
        auto ent = core_ctx->get_entity<core::titulus>(i);
        if (ent->type() != core::titulus::type::baron) continue;
        ret = f(object(ent));
      }
      return ret;
    }
    
    bool each_duchy_global(const void*, const func_t & f) {
      bool ret = true;
      auto core_ctx = global::get<systems::map_t>()->core_context.get();
      const size_t count = core_ctx->get_entity_count<core::titulus>();
      for (size_t i = 0; i < count && ret; ++i) {
        auto ent = core_ctx->get_entity<core::titulus>(i);
        if (ent->type() != core::titulus::type::duke) continue;
        ret = f(object(ent));
      }
      return ret;
    }
    
    bool each_kingdom_global(const void*, const func_t & f) {
      bool ret = true;
      auto core_ctx = global::get<systems::map_t>()->core_context.get();
      const size_t count = core_ctx->get_entity_count<core::titulus>();
      for (size_t i = 0; i < count && ret; ++i) {
        auto ent = core_ctx->get_entity<core::titulus>(i);
        if (ent->type() != core::titulus::type::king) continue;
        ret = f(object(ent));
      }
      return ret;
    }
    
    bool each_empire_global(const void*, const func_t & f) {
      bool ret = true;
      auto core_ctx = global::get<systems::map_t>()->core_context.get();
      const size_t count = core_ctx->get_entity_count<core::titulus>();
      for (size_t i = 0; i < count && ret; ++i) {
        auto ent = core_ctx->get_entity<core::titulus>(i);
        if (ent->type() != core::titulus::type::imperial) continue;
        ret = f(object(ent));
      }
      return ret;
    }
    
    bool each_religion_group_global(const void*, const func_t & f) {
      bool ret = true;
      auto core_ctx = global::get<systems::map_t>()->core_context.get();
      const size_t count = core_ctx->get_entity_count<core::religion_group>();
      for (size_t i = 0; i < count && ret; ++i) {
        auto ent = core_ctx->get_entity<core::religion_group>(i);
        ret = f(object(ent));
      }
      return ret;
    }
    
    bool each_religion_global(const void*, const func_t & f) {
      bool ret = true;
      auto core_ctx = global::get<systems::map_t>()->core_context.get();
      const size_t count = core_ctx->get_entity_count<core::religion>();
      for (size_t i = 0; i < count && ret; ++i) {
        auto ent = core_ctx->get_entity<core::religion>(i);
        ret = f(object(ent));
      }
      return ret;
    }
    
    bool each_culture_group_global(const void*, const func_t & f) {
      bool ret = true;
      auto core_ctx = global::get<systems::map_t>()->core_context.get();
      const size_t count = core_ctx->get_entity_count<core::culture_group>();
      for (size_t i = 0; i < count && ret; ++i) {
        auto ent = core_ctx->get_entity<core::culture_group>(i);
        ret = f(object(ent));
      }
      return ret;
    }
    
    bool each_culture_global(const void*, const func_t & f) {
      bool ret = true;
      auto core_ctx = global::get<systems::map_t>()->core_context.get();
      const size_t count = core_ctx->get_entity_count<core::culture>();
      for (size_t i = 0; i < count && ret; ++i) {
        auto ent = core_ctx->get_entity<core::culture>(i);
        ret = f(object(ent));
      }
      return ret;
    }
    
    bool each_living_character_global(const void*, const func_t & f) {
      bool ret = true;
      auto core_ctx = global::get<systems::map_t>()->core_context.get();
      const size_t count = core_ctx->living_characters_count();
      for (size_t i = 0; i < count && ret; ++i) {
        auto ent = core_ctx->get_living_character(i);
        ret = f(object(ent));
      }
      return ret;
    }
    
    bool each_ruler_global(const void*, const func_t & f) {
      bool ret = true;
      auto core_ctx = global::get<systems::map_t>()->core_context.get();
      const size_t count = core_ctx->living_playable_characters_count();
      for (size_t i = 0; i < count && ret; ++i) {
        auto ent = core_ctx->get_living_playable_character(i);
        ret = f(object(ent));
      }
      return ret;
    }
    
    bool each_independent_ruler_global(const void*, const func_t & f) {
      bool ret = true;
      auto core_ctx = global::get<systems::map_t>()->core_context.get();
      const size_t count = core_ctx->living_playable_characters_count();
      for (size_t i = 0; i < count && ret; ++i) {
        auto ent = core_ctx->get_living_playable_character(i);
        if (!ent->is_independent()) continue;
        ret = f(object(ent));
      }
      return ret;
    }
    
    bool each_player_global(const void*, const func_t & f) {
      bool ret = true;
      auto core_ctx = global::get<systems::map_t>()->core_context.get();
      const size_t count = core_ctx->living_playable_characters_count();
      for (size_t i = 0; i < count && ret; ++i) {
        auto ent = core_ctx->get_living_playable_character(i);
        if (!ent->is_player()) continue;
        ret = f(object(ent));
      }
      return ret;
    }
  }
}
