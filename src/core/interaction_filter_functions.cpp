#include "interaction_filter_functions.h"

#include "structures_header.h"
#include "realm_rights_checker.h"
#include "utils/globals.h"
#include "utils/systems.h"
#include "context.h"

#define MAKE_MAP_PAIR(name) std::make_pair(names[values::name], values::name)

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

namespace devils_engine {
  namespace core {
    namespace character_filters {
      const std::string_view names[] = {
#define CHARACTERS_FILTER_FUNC(name) #name,
        CHARACTERS_FILTERS_LIST
#undef CHARACTERS_FILTER_FUNC
      };
      
      const phmap::flat_hash_map<std::string_view, values> map = {
#define CHARACTERS_FILTER_FUNC(name) MAKE_MAP_PAIR(name),
        CHARACTERS_FILTERS_LIST
#undef CHARACTERS_FILTER_FUNC
      };
      
      const basic_func basics[] = {
#define CHARACTERS_FILTER_FUNC(name) &name##_basic,
        CHARACTERS_FILTERS_LIST
#undef CHARACTERS_FILTER_FUNC
      };
      
      const iterator_func iterators[] = {
#define CHARACTERS_FILTER_FUNC(name) &name##_iterator,
        CHARACTERS_FILTERS_LIST
#undef CHARACTERS_FILTER_FUNC
      };
      
      static_assert(ARRAY_SIZE(names) == count);
      static_assert(ARRAY_SIZE(names) == ARRAY_SIZE(basics));
      static_assert(ARRAY_SIZE(names) == ARRAY_SIZE(iterators));
    }
    
    namespace title_filters {
      const std::string_view names[] = {
#define TITLES_FILTER_FUNC(name) #name,
        TITLES_FILTERS_LIST
#undef TITLES_FILTER_FUNC
      };
      
      const phmap::flat_hash_map<std::string_view, values> map = {
#define TITLES_FILTER_FUNC(name) MAKE_MAP_PAIR(name),
        TITLES_FILTERS_LIST
#undef TITLES_FILTER_FUNC
      };
      
      const basic_func basics[] = {
#define TITLES_FILTER_FUNC(name) &name##_basic,
        TITLES_FILTERS_LIST
#undef TITLES_FILTER_FUNC
      };
      
      const iterator_func iterators[] = {
#define TITLES_FILTER_FUNC(name) &name##_iterator,
        TITLES_FILTERS_LIST
#undef TITLES_FILTER_FUNC
      };
      
      static_assert(ARRAY_SIZE(names) == count);
      static_assert(ARRAY_SIZE(names) == ARRAY_SIZE(basics));
      static_assert(ARRAY_SIZE(names) == ARRAY_SIZE(iterators));
    }
    
    namespace city_filters {
      const std::string_view names[] = {
#define CITIES_FILTER_FUNC(name) #name,
        CITIES_FILTERS_LIST
#undef CITIES_FILTER_FUNC
      };
      
      const phmap::flat_hash_map<std::string_view, values> map = {
#define CITIES_FILTER_FUNC(name) MAKE_MAP_PAIR(name),
        CITIES_FILTERS_LIST
#undef CITIES_FILTER_FUNC
      };
      
      const basic_func basics[] = {
#define CITIES_FILTER_FUNC(name) &name##_basic,
        CITIES_FILTERS_LIST
#undef CITIES_FILTER_FUNC
      };
      
      const iterator_func iterators[] = {
#define CITIES_FILTER_FUNC(name) &name##_iterator,
        CITIES_FILTERS_LIST
#undef CITIES_FILTER_FUNC
      };
      
      static_assert(ARRAY_SIZE(names) == count);
      static_assert(ARRAY_SIZE(names) == ARRAY_SIZE(basics));
      static_assert(ARRAY_SIZE(names) == ARRAY_SIZE(iterators));
    }
    
    namespace province_filters {
      const std::string_view names[] = {
#define PROVINCES_FILTER_FUNC(name) #name,
        PROVINCES_FILTERS_LIST
#undef PROVINCES_FILTER_FUNC
      };
      
      const phmap::flat_hash_map<std::string_view, values> map = {
#define PROVINCES_FILTER_FUNC(name) MAKE_MAP_PAIR(name),
        PROVINCES_FILTERS_LIST
#undef PROVINCES_FILTER_FUNC
      };
      
      const basic_func basics[] = {
#define PROVINCES_FILTER_FUNC(name) &name##_basic,
        PROVINCES_FILTERS_LIST
#undef PROVINCES_FILTER_FUNC
      };
      
      const iterator_func iterators[] = {
#define PROVINCES_FILTER_FUNC(name) &name##_iterator,
        PROVINCES_FILTERS_LIST
#undef PROVINCES_FILTER_FUNC
      };
      
      static_assert(ARRAY_SIZE(names) == count);
      static_assert(ARRAY_SIZE(names) == ARRAY_SIZE(basics));
      static_assert(ARRAY_SIZE(names) == ARRAY_SIZE(iterators));
    }
    
    // мы должны проверить все титулы в непосредственном владении у персонажа
    bool directly_owned_titles_basic(const core::character* c, const core::titulus* t) {
      if (!t->owner.valid()) return false;
      if (!c->self.valid()) return false;
      return t->owner == c->self;
    }
    
    // а эту функцию по идее можно совместить с той что в скриптах (every_directly_owned_title)
    // там добавляется скриптовой контекст, у меня скорее всего проблема вот в чем, здесь было бы неплохо останавливать итератор
    bool directly_owned_titles_iterator(const core::character* c, const std::function<bool(const core::character*, const core::titulus*)> &func) {
      if (!c->self.valid()) return false;
      for (auto t = c->self->titles; t != nullptr; t = utils::ring::list_next<utils::list_type::titles>(t, c->self->titles)) {
        const bool ret = func(c, t);
        if (!ret) break;
      }
      
      return true;
    }
    
    // у нас есть титулы непосредственного владения и есть титулы которые даются от реалма полученного после выборов
    // нужно опять же разделить эти вещи, остальные титулы уже принадлежат вассалам 
    
    bool accessible_titles_basic(const core::character* c, const core::titulus* t) {
      //if (t->owner.valid() && t->owner->leader == c) return true;
      if (!t->owner.valid()) return false;
      return t->owner->leader == c;
    }
    
    bool accessible_titles_iterator(const core::character* c, const std::function<bool(const core::character*, const core::titulus*)> &func) {
      bool have_at_least_one_title = false;
      
      if (c->self.valid()) {
        for (auto t = c->self->titles; t != nullptr; t = utils::ring::list_next<utils::list_type::titles>(t, c->self->titles)) {
          have_at_least_one_title = have_at_least_one_title || true;
          const bool ret = func(c, t);
          if (!ret) return true;
        }
      }
      
      for (size_t i = 0; i < core::character::faction_type_count; ++i) {
        if (!c->realms[i].valid() || c->realms[i]->leader != c) continue;
        for (auto t = c->realms[i]->titles; t != nullptr; t = utils::ring::list_next<utils::list_type::titles>(t, c->realms[i]->titles)) {
          have_at_least_one_title = have_at_least_one_title || true;
          const bool ret = func(c, t);
          if (!ret) return true;
        }
      }
      
      return have_at_least_one_title;
    }
    
    bool directly_controlled_cities_basic(const core::character* c, const core::city* city) {
      return city->title->owner == c->self;
    }
    
    bool directly_controlled_cities_iterator(const core::character* c, const std::function<bool(const core::character*, const core::city*)> &func) {
      if (!c->self.valid()) return false;
      for (auto t = c->self->titles; t != nullptr; t = utils::ring::list_next<utils::list_type::titles>(t, c->self->titles)) {
        if (t->type() != core::titulus::type::city) continue;
        const bool ret = func(c, t->get_city());
        if (!ret) break;
      }
      
      return true;
    }
    
    bool accessible_cities_basic(const core::character* c, const core::city* city) {
      return city->title->owner->leader == c;
    }
    
    bool accessible_cities_iterator(const core::character* c, const std::function<bool(const core::character*, const core::city*)> &func) {
      const utils::handle<core::realm> realms[] = {
        c->self,
        c->realms[0],
        c->realms[1],
        c->realms[2],
        c->realms[3],
        c->realms[4]
      };
      
      static_assert(ARRAY_SIZE(realms) == core::character::faction_type_count+1);
      
      bool at_least_one = false;
      for (size_t i = 0; i < core::character::faction_type_count+1; ++i) {
        const auto r = realms[i];
        if (!r.valid() || r->leader != c) continue;
        
        for (auto t = r->titles; t != nullptr; t = utils::ring::list_next<utils::list_type::titles>(t, r->titles)) {
          if (t->type() != core::titulus::type::city) continue;
          at_least_one = true;
          const bool ret = func(c, t->get_city());
          if (!ret) break;
        }
      }
      
      return at_least_one;
    }
    
    // вот я думаю включать ли города под прямым управлением? мы можем по идее их выкинуть с помощью скрипта
    // это города хозяина + доступные города хозяина + города вассалов хозяина
    bool cities_in_realm_basic(const core::character* c, const core::city* city) {
      // как проверить? персонаж должен быть либо владельцем, либо владельцем владельца
      auto owner = city->title->owner->leader;
      while (owner != nullptr) {
        if (c == owner) return true;
        owner = owner->self->liege->leader;
      }
      
      return false;
    }
    
    bool cities_in_realm_iterator(const core::character* c, const std::function<bool(const core::character*, const core::city*)> &func) {
      bool have_at_least_one_city = false;
      
      for (size_t i = 0; i < core::character::faction_type_count; ++i) {
        if (!c->realms[i].valid() || c->realms[i]->leader != c) continue;
        for (auto t = c->realms[i]->titles; t != nullptr; t = utils::ring::list_next<utils::list_type::titles>(t, c->realms[i]->titles)) {
          if (t->type() != core::titulus::type::city) continue;
          have_at_least_one_city = have_at_least_one_city || true;
          const bool ret = func(c, t->get_city());
          if (!ret) return true;
        }
        
        for (auto v = c->realms[i]->vassals; v != nullptr; v = utils::ring::list_next<utils::list_type::vassals>(v, c->realms[i]->vassals)) {
          assert(v->object_token != SIZE_MAX);
          auto character = v->leader;
          cities_in_realm_iterator(character, func);
        }
      }
      
      if (c->self.valid()) {
        for (auto t = c->self->titles; t != nullptr; t = utils::ring::list_next<utils::list_type::titles>(t, c->self->titles)) {
          if (t->type() != core::titulus::type::city) continue;
          have_at_least_one_city = have_at_least_one_city || true;
          const bool ret = func(c, t->get_city());
          if (!ret) return true;
        }
        
        for (auto v = c->self->vassals; v != nullptr; v = utils::ring::list_next<utils::list_type::vassals>(v, c->self->vassals)) {
          assert(v->object_token != SIZE_MAX);
          auto character = v->leader;
          cities_in_realm_iterator(character, func);
        }
      }
      
      return have_at_least_one_city;
    }
    
    bool directly_controlled_provinces_basic(const core::character* c, const core::province* p) {
      return p->title->owner == c->self;
    }
    
    bool directly_controlled_provinces_iterator(const core::character* c, const std::function<bool(const core::character*, const core::province*)> &func) {
      if (!c->self.valid()) return false;
      for (auto t = c->self->titles; t != nullptr; t = utils::ring::list_next<utils::list_type::titles>(t, c->self->titles)) {
        if (t->type() != core::titulus::type::baron) continue;
        const bool ret = func(c, t->get_province());
        if (!ret) break;
      }
      
      return true;
    }
    
    bool accessible_provinces_basic(const core::character* c, const core::province* p) {
      return p->title->owner->leader == c;
    }
    
    bool accessible_provinces_iterator(const core::character* c, const std::function<bool(const core::character*, const core::province*)> &func) {
      const utils::handle<core::realm> realms[] = {
        c->self,
        c->realms[0],
        c->realms[1],
        c->realms[2],
        c->realms[3],
        c->realms[4]
      };
      
      static_assert(ARRAY_SIZE(realms) == core::character::faction_type_count+1);
      
      bool at_least_one = false;
      for (size_t i = 0; i < core::character::faction_type_count+1; ++i) {
        const auto r = realms[i];
        if (!r.valid() || r->leader != c) continue;
        
        for (auto t = r->titles; t != nullptr; t = utils::ring::list_next<utils::list_type::titles>(t, r->titles)) {
          if (t->type() != core::titulus::type::baron) continue;
          at_least_one = true;
          const bool ret = func(c, t->get_province());
          if (!ret) break;
        }
      }
      
      return at_least_one;
    }
    
    bool provinces_in_realm_basic(const core::character* c, const core::province* p) {
      // как проверить? персонаж должен быть либо владельцем, либо владельцем владельца
      auto owner = p->title->owner->leader;
      while (owner != nullptr) {
        if (c == owner) return true;
        owner = owner->self->liege->leader;
      }
      
      return false;
    }
    
    bool provinces_in_realm_iterator(const core::character* c, const std::function<bool(const core::character*, const core::province*)> &func) {
      bool have_at_least_one_city = false;
      
      for (size_t i = 0; i < core::character::faction_type_count; ++i) {
        if (!c->realms[i].valid() || c->realms[i]->leader != c) continue;
        for (auto t = c->realms[i]->titles; t != nullptr; t = utils::ring::list_next<utils::list_type::titles>(t, c->realms[i]->titles)) {
          if (t->type() != core::titulus::type::baron) continue;
          have_at_least_one_city = have_at_least_one_city || true;
          const bool ret = func(c, t->get_province());
          if (!ret) return true;
        }
        
        for (auto v = c->realms[i]->vassals; v != nullptr; v = utils::ring::list_next<utils::list_type::vassals>(v, c->realms[i]->vassals)) {
          assert(v->object_token != SIZE_MAX);
          auto character = v->leader;
          provinces_in_realm_iterator(character, func);
        }
      }
      
      if (c->self.valid()) {
        for (auto t = c->self->titles; t != nullptr; t = utils::ring::list_next<utils::list_type::titles>(t, c->self->titles)) {
          if (t->type() != core::titulus::type::baron) continue;
          have_at_least_one_city = have_at_least_one_city || true;
          const bool ret = func(c, t->get_province());
          if (!ret) return true;
        }
        
        for (auto v = c->self->vassals; v != nullptr; v = utils::ring::list_next<utils::list_type::vassals>(v, c->self->vassals)) {
          assert(v->object_token != SIZE_MAX);
          auto character = v->leader;
          provinces_in_realm_iterator(character, func);
        }
      }
      
      return have_at_least_one_city;
    }
    
    bool known_secrets_basic(const core::character*, const core::character*) {
      return true;
    }
    
    bool known_secrets_iterator(const core::character*, const std::function<bool(const core::character*, const core::character*)> &) {
      return true;
    }
    
    bool hooked_characters_basic(const core::character*, const core::character*) {
      return true;
    }
    
    bool hooked_characters_iterator(const core::character*, const std::function<bool(const core::character*, const core::character*)> &) {
      return true;
    }
    
    // как искать соседей через море? хороший вопрос
    // надо сделать отдельно соседей из генератора и отдельно соседей по тайлам
    bool neighboring_rulers_basic(const core::character* c, const core::character* n) {
      auto top_liege = c->self.valid() ? c->self->leader : c->suzerain->leader;
      while (top_liege->self->liege.valid()) {
        top_liege = top_liege->self->liege->state->leader;
      }
      
      // что тут? по идее нужно пройтись по провинциям в реалме и проверить есть ли провинция с таким владельцем
      struct { bool ret; const core::character* n; } func_data { false, n };
      provinces_in_realm_iterator(top_liege, [&func_data] (const core::character*, const core::province* p) -> bool {
        if (func_data.ret) return false;
        
        for (const auto index : p->neighbours) {
          const auto n_prov = global::get<systems::map_t>()->core_context->get_entity<core::province>(index);
          if (provinces_in_realm_basic(func_data.n, n_prov)) {
            func_data.ret = true;
            break;
          }
        }
        
        return !func_data.ret;
      });
      
      return func_data.ret;
    }
    
    // каких конкретно соседей берем? только хозяев верхнего уровня? в следующем фильтре мы возьмем вассалов у одного хозяина
    bool neighboring_rulers_iterator(const core::character* c, const std::function<bool(const core::character*, const core::character*)> &func) {
      auto top_liege = c->self.valid() ? c->self->leader : c->suzerain->leader;
      while (top_liege->self->liege.valid()) {
        top_liege = top_liege->self->liege->state->leader;
      }
      
      phmap::flat_hash_set<const core::character*> neighbours;
      neighbours.reserve(30);
      // может тут использовать top_liege? наверное лучше его
      provinces_in_realm_iterator(top_liege, [&neighbours] (const core::character*, const core::province* p) -> bool {
        for (const auto index : p->neighbours) {
          const auto n_prov = global::get<systems::map_t>()->core_context->get_entity<core::province>(index);
          auto n_top_liege = n_prov->title->owner->leader;
          while (n_top_liege->self->liege.valid()) {
            n_top_liege = n_top_liege->self->liege->state->leader;
          }
          
          //if (top_liege != n_top_liege) 
            neighbours.insert(n_top_liege);
        }
        
        return true;
      });
      
      for (auto n : neighbours) {
        const bool ret = func(c, n);
        if (!ret) return true;
      }
      
      return true;
    }
    
    // какие вассалы считаются равными? те что имеют одного хозяина? можно ли иметь разных хозяев внутри одного государства?
    // например быть ваасалов ассебмли? вообще наверное это оживит немного политику, можно оставить как фичу
    // что тогда проверять тут? лучше тут проверить вассалов одного стейта
    bool peer_vassals_basic(const core::character* c, const core::character* n) {
      if (!c->self.valid() || c->self->is_independent_realm()) return false;
      if (!n->self.valid() || n->self->is_independent_realm()) return false;
      return c->self->liege->state == n->self->liege->state;
    }
    
    bool peer_vassals_iterator(const core::character* c, const std::function<bool(const core::character*, const core::character*)> &func) {
      if (!c->self.valid() || c->self->is_independent_realm()) return false;
      
      vassals_iterator(c->self->liege->leader, [&func, c] (const core::character*, const core::character* vassal) -> bool {
        const bool ret = func(c, vassal);
        if (!ret) return false;
        return true;
      });
      
      return true;
    }
    
    bool guests_basic(const core::character*, const core::character*) {
      return true;
    }
    
    bool guests_iterator(const core::character*, const std::function<bool(const core::character*, const core::character*)> &) {
      return true;
    }
    
    bool dynasty_basic(const core::character* c, const core::character* n) {
      return c->family.dynasty == n->family.dynasty;
    }
    
    bool dynasty_iterator(const core::character* c, const std::function<bool(const core::character*, const core::character*)> &func) {
      // по идее в династии мы должны запомнить всех персонажей
    }
    
    bool courtiers_basic(const core::character* c, const core::character* n) {
      return n->suzerain.valid() && n->suzerain->leader == c;
    }
    
    bool courtiers_iterator(const core::character* c, const std::function<bool(const core::character*, const core::character*)> &func) {
      bool has_at_least_one = false;
      
      if (c->self.valid()) {
        for (auto courtier = c->self->courtiers; courtier != nullptr; courtier = utils::ring::list_next<utils::list_type::courtiers>(courtier, c->self->courtiers)) {
          has_at_least_one = true;
          const bool ret = func(c, courtier);
          if (!ret) return has_at_least_one;
        }
      }
      
      for (size_t i = 0; i < core::character::faction_type_count; ++i) {
        if (!c->realms[i].valid() || c->realms[i]->leader != c) continue;
        for (auto courtier = c->realms[i]->courtiers; courtier != nullptr; courtier = utils::ring::list_next<utils::list_type::courtiers>(courtier, c->realms[i]->courtiers)) {
          has_at_least_one = true;
          const bool ret = func(c, courtier);
          if (!ret) return has_at_least_one;
        }
      }
      
      return has_at_least_one;
    }
    
    bool prisoners_basic(const core::character*, const core::character*) {
      
    }
    
    bool prisoners_iterator(const core::character* c, const std::function<bool(const core::character*, const core::character*)> &func) {
      const utils::handle<core::realm> realms[] = {
        c->self,
        c->realms[0],
        c->realms[1],
        c->realms[2],
        c->realms[3],
        c->realms[4]
      };
      
      static_assert(ARRAY_SIZE(realms) == core::character::faction_type_count+1);
      
      bool has_at_least_one = false;
      for (size_t i = 0; i < core::character::faction_type_count+1; ++i) {
        const auto r = realms[i];
        if (!r.valid() || r->leader != c) continue;
        for (auto p = c->self->prisoners; p != nullptr; p = utils::ring::list_next<utils::list_type::prisoners>(p, c->self->prisoners)) {
          has_at_least_one = true;
          const bool ret = func(c, p);
          if (!ret) return has_at_least_one;
        }
      }
      
      return has_at_least_one;
    }
    
    // не понимаю чем принципиально отличаются sub_realm и realm, 
    // тип реалм - это только конкретный реалм, а sub_realm - это вассалы реалма?
    bool sub_realm_characters_basic(const core::character*, const core::character*) {
      
    }
    
    bool sub_realm_characters_iterator(const core::character*, const std::function<bool(const core::character*, const core::character*)> &) {
      
    }
    
    bool realm_characters_basic(const core::character*, const core::character*) {
      
    }
    
    bool realm_characters_iterator(const core::character*, const std::function<bool(const core::character*, const core::character*)> &) {
      
    }
    
    bool vassals_basic(const core::character* c, const core::character* v) {
      if (v->self.valid()) return v->self->liege->leader == c;
      for (size_t i = 0; i < core::character::faction_type_count; ++i) {
        if (!v->realms[i].valid() || v->realms[i]->leader != v) continue;
        if (v->realms[i]->get_state()->leader == c || v->realms[i]->get_state()->liege->leader == c) return true;
      }
      
      return false;
    }
    
    // являются ли лидеры реалмов - вассалами? ну они могут быть только вассалами или придворными, так что отдельно специально их выделять не нужно
    bool vassals_iterator(const core::character* c, const std::function<bool(const core::character*, const core::character*)> &func) {
      const utils::handle<core::realm> realms[] = {
        c->self,
        c->realms[0],
        c->realms[1],
        c->realms[2],
        c->realms[3],
        c->realms[4]
      };
      
      static_assert(ARRAY_SIZE(realms) == core::character::faction_type_count+1);
      
      bool at_least_one = false;
      for (size_t i = 0; i < core::character::faction_type_count+1; ++i) {
        const auto r = realms[i];
        if (!r.valid() || r->leader != c) continue;
        for (auto v = r->vassals; v != nullptr; v = utils::ring::list_next<utils::list_type::vassals>(v, r->vassals)) {
          assert(v->object_token != SIZE_MAX);
          at_least_one = true;
          const bool ret = func(c, v->leader);
          if (!ret) return true;
        }
      }
      
      return at_least_one;
    }
    
    bool liege_basic(const core::character* c, const core::character* l) {
      auto ch = c->self.valid() ? c : c->suzerain->leader;
      while (ch != nullptr && ch != l) {
        ch = ch->self->get_state()->get_liege().valid() ? ch->self->get_state()->get_liege()->leader : nullptr;
      }
      
      return ch != nullptr;
    }
    
    bool liege_iterator(const core::character* c, const std::function<bool(const core::character*, const core::character*)> &func) {
      if (c->is_independent()) return false;
      
      auto ch = c->self.valid() ? c->self->liege->leader : c->suzerain->leader;
      while (ch != nullptr) {
        const bool ret = func(c, ch);
        if (!ret) return true;
        ch = ch->self->get_state()->get_liege().valid() ? ch->self->get_state()->get_liege()->leader : nullptr;
      }
      
      return true;
    }
    
    bool self_basic(const core::character* c, const core::character* c2) {
      return c == c2;
    }
    
    bool self_iterator(const core::character* c, const std::function<bool(const core::character*, const core::character*)> &func) {
      return func(c, c);
    }
    
    // тут только один человек?
    bool head_of_faith_basic(const core::character* c, const core::character* h) {
      return c->religion->head == h;
    }
    
    bool head_of_faith_iterator(const core::character* c, const std::function<bool(const core::character*, const core::character*)> &func) {
      if (c->religion->head == nullptr) return false;
      func(c, c->religion->head);
      return true;
    }
    
    // жена + наложницы
    bool spouses_basic(const core::character* c, const core::character* s) {
      return c->family.consort == s || c->is_owner_of(s);
    }
    
    bool spouses_iterator(const core::character* c, const std::function<bool(const core::character*, const core::character*)> &func) {
      bool has_at_least_one = false;
      
      if (c->family.consort != nullptr) {
        has_at_least_one = true;
        const bool ret = func(c, c->family.consort);
        if (!ret) return true;
      }
      
      for (auto con = c->family.concubines; con != nullptr; con = utils::ring::list_next<utils::list_type::concubines>(con, c->family.concubines)) {
        has_at_least_one = true;
        const bool ret = func(c, con);
        if (!ret) return true;
      }
      
      return has_at_least_one;
    }
    
    // семья это что? вся семья и близкая и дальняя? а женитьба? жены наверное нет, а так это расширенная семья
    bool family_basic(const core::character* c, const core::character* f) {
      return c->is_extended_relative_of(f);
    }
    
    bool family_iterator(const core::character* c, const std::function<bool(const core::character*, const core::character*)> &func) {
      // распространяемся сверху, проверяя всех детей 
      
      const std::function<bool(const core::character*, const core::character*)> rec_grand_func = [&] (const core::character*, const core::character* f) -> bool {
        const bool ret = func(c, f);
        if (!ret) return false;
                        
        if (c->is_cousin_of(f) || c->is_uncle_or_aunt_of(f) || c->is_grandparent_of(f)) return true;
        children_iterator(f, rec_grand_func);
      };
      
      const core::character* grandparents[] = {
        c->family.parents[0] != nullptr ? c->family.parents[0]->family.parents[0] : nullptr,
        c->family.parents[0] != nullptr ? c->family.parents[0]->family.parents[1] : nullptr,
        c->family.parents[1] != nullptr ? c->family.parents[1]->family.parents[0] : nullptr,
        c->family.parents[1] != nullptr ? c->family.parents[1]->family.parents[1] : nullptr
      };
      
      for (size_t i = 0; i < 4; ++i) {
        auto cur = grandparents[i];
        if (cur == nullptr) continue;
        const bool ret = func(c, cur);
        if (!ret) return true;
        children_iterator(cur, rec_grand_func);
      }
      
      const bool has_left_grandparents  = grandparents[0] != nullptr || grandparents[1] != nullptr;
      const bool has_right_grandparents = grandparents[2] != nullptr || grandparents[3] != nullptr;
      
      if (!has_left_grandparents && c->family.parents[0] != nullptr) {
        children_iterator(c->family.parents[0], rec_grand_func);
      }
      
      if (!has_right_grandparents && c->family.parents[1] != nullptr) {
        children_iterator(c->family.parents[1], rec_grand_func);
      }
      
      if (c->family.parents[0] == nullptr && c->family.parents[1] == nullptr) {
        children_iterator(c, rec_grand_func);
      }
      
      return true;
    }
    
    bool children_basic(const core::character* c, const core::character* child) {
      return c->is_parent_of(child);
    }
    
    bool children_iterator(const core::character* c, const std::function<bool(const core::character*, const core::character*)> &func) {
      bool at_least_one = false;
      if (c->is_male()) {
        for (auto child = c->family.children; child != nullptr; child = utils::ring::list_next<utils::list_type::father_line_siblings>(child, c->family.children)) {
          at_least_one = true;
          const bool ret = func(c, child);
          if (!ret) return true;
        }
      } else {
        for (auto child = c->family.children; child != nullptr; child = utils::ring::list_next<utils::list_type::mother_line_siblings>(child, c->family.children)) {
          at_least_one = true;
          const bool ret = func(c, child);
          if (!ret) return true;
        }
      }
      
      return at_least_one;
    }
    
    bool primary_war_enemies_basic(const core::character* c, const core::character* e) {
      const auto itr = c->diplomacy.find(e);
      if (itr == c->diplomacy.end()) return false;
      if (!itr->second.types.get(core::diplomacy::war_attacker) && !itr->second.types.get(core::diplomacy::war_defender)) return false;
      return (itr->second.war->get_primary_attacker() == c && itr->second.war->get_primary_defender() == e) || 
             (itr->second.war->get_primary_attacker() == e && itr->second.war->get_primary_defender() == c);
    }
    
    bool primary_war_enemies_iterator(const core::character* c, const std::function<bool(const core::character*, const core::character*)> &func) {
      bool at_least_one = false;
      for (const auto &pair : c->diplomacy) {
        if (!pair.second.types.get(core::diplomacy::war_attacker) && !pair.second.types.get(core::diplomacy::war_defender)) continue;
        at_least_one = true;
        auto ch = c;
        if (pair.second.war->get_primary_attacker() == c) ch = pair.second.war->get_primary_defender();
        if (pair.second.war->get_primary_defender() == c) ch = pair.second.war->get_primary_attacker();
        const bool ret = func(c, ch);
        if (!ret) return true;
      }
      
      return at_least_one;
    }
    
    bool war_enemies_basic(const core::character* c, const core::character* e) {
      for (const auto &pair : c->diplomacy) {
        if (pair.second.types.get(core::diplomacy::war_attacker)) {
          if (pair.second.war->get_primary_defender() == e) return true;
          for (const auto ch : pair.second.war->defenders) {
            if (ch == e) return true;
          }
        }
        
        if (pair.second.types.get(core::diplomacy::war_defender)) {
          if (pair.second.war->get_primary_attacker() == e) return true;
          for (const auto ch : pair.second.war->attackers) {
            if (ch == e) return true;
          }
        }
      }
      
      return false;
    }
    
    bool war_enemies_iterator(const core::character* c, const std::function<bool(const core::character*, const core::character*)> &func) {
      bool at_least_one = false;
      
      for (const auto &pair : c->diplomacy) {
        if (pair.second.types.get(core::diplomacy::war_attacker)) {
          at_least_one = true;
          const auto ch = pair.second.war->get_primary_defender();
          const bool ret = func(c, ch);
          if (!ret) return true;
          for (const auto ch : pair.second.war->defenders) {
            const bool ret = func(c, ch);
            if (!ret) return true;
          }
        }
        
        if (pair.second.types.get(core::diplomacy::war_defender)) {
          at_least_one = true;
          const auto ch = pair.second.war->get_primary_attacker();
          const bool ret = func(c, ch);
          if (!ret) return true;
          for (const auto ch : pair.second.war->attackers) {
            const bool ret = func(c, ch);
            if (!ret) return true;
          }
        }
      }
      
      return at_least_one;
    }
    
    bool war_allies_basic(const core::character* c, const core::character* e) {
      for (const auto &pair : c->diplomacy) {
        if (pair.second.types.get(core::diplomacy::war_attacker)) {
          if (pair.second.war->get_primary_attacker() == e) return true;
          for (const auto ch : pair.second.war->attackers) {
            if (ch == e) return true;
          }
        }
        
        if (pair.second.types.get(core::diplomacy::war_defender)) {
          if (pair.second.war->get_primary_defender() == e) return true;
          for (const auto ch : pair.second.war->defenders) {
            if (ch == e) return true;
          }
        }
      }
      
      return false;
    }
    
    bool war_allies_iterator(const core::character* c, const std::function<bool(const core::character*, const core::character*)> &func) {
      bool at_least_one = false;
      
      for (const auto &pair : c->diplomacy) {
        if (pair.second.types.get(core::diplomacy::war_attacker)) {
          at_least_one = true;
          const auto ch = pair.second.war->get_primary_attacker();
          const bool ret = func(c, ch);
          if (!ret) return true;
          for (const auto ch : pair.second.war->attackers) {
            const bool ret = func(c, ch);
            if (!ret) return true;
          }
        }
        
        if (pair.second.types.get(core::diplomacy::war_defender)) {
          at_least_one = true;
          const auto ch = pair.second.war->get_primary_defender();
          const bool ret = func(c, ch);
          if (!ret) return true;
          for (const auto ch : pair.second.war->defenders) {
            const bool ret = func(c, ch);
            if (!ret) return true;
          }
        }
      }
      
      return at_least_one;
    }
    
    // что такое scripted_relations? это обычные отношения? по идее да
    bool scripted_relations_basic(const core::character* c, const core::character* a) {
      return c->relations.is_acquaintance(a);
    }
    
    bool scripted_relations_iterator(const core::character* c, const std::function<bool(const core::character*, const core::character*)> &func) {
      bool at_least_one = false;
      
      for (size_t i = 0; i < c->relations.acquaintances.size(); ++i) {
        const auto &pair = c->relations.acquaintances[i];
        if (pair.first == nullptr) continue;
        at_least_one = true;
        const bool ret = func(c, pair.first);
        if (!ret) return true;
      }
      
      return at_least_one;
    }
  }
}
