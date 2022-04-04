#include "realm.h"

#include "utils/globals.h"
#include "context.h"
#include "character.h"
#include "titulus.h"
#include "realm_rights_checker.h"

namespace devils_engine {
  namespace core {
    const structure realm::s_type;
    realm::realm() : 
      object_token(SIZE_MAX),
      leader(nullptr), 
      heir(nullptr), 
      liege(nullptr), 
      state(nullptr), 
      council(nullptr), 
      tribunal(nullptr), 
      vassals(nullptr), 
//       next_vassal(nullptr), 
//       prev_vassal(nullptr), 
      titles(nullptr), 
      main_title(nullptr), 
      capital(nullptr),
      courtiers(nullptr),
      prisoners(nullptr) 
    {
      // тут было бы неплохо создать сразу с персонажем это дело
//       memset(stats.data(), 0, sizeof(stats));
    }
    
    // нужно еще убрать этот объект из всех листов, пока не знаю как это делается, но скорее всего можно просто в деструкторе
    realm::~realm() { object_token = SIZE_MAX; }
    
    bool realm::is_state() const noexcept { return state == this; }
    bool realm::is_council() const noexcept { return council == this; }
    bool realm::is_tribunal() const noexcept { return tribunal == this; }
    bool realm::is_assembly() const noexcept { return assembly == this; }
    bool realm::is_clergy() const noexcept { return clergy == this; }
    bool realm::is_state_independent_power() const noexcept { return is_state() && !is_council() && !is_tribunal() && !is_assembly() && !is_clergy(); }
    bool realm::is_independent_realm() const noexcept { return liege == nullptr; }
    bool realm::is_self() const noexcept { return leader->self == this; }
    bool realm::has_council() const noexcept { return  council.valid() && !is_council(); }
    bool realm::has_tribunal() const noexcept { return tribunal.valid() && !is_tribunal(); }
    bool realm::has_assembly() const noexcept { return assembly.valid() && !is_assembly(); }
    bool realm::has_clergy() const noexcept { return clergy.valid() && !is_clergy(); }
    bool realm::has_capital() const noexcept { return capital != nullptr; }
    bool realm::has_titles() const noexcept { return titles != nullptr; }
    
    bool realm::has_right(const size_t &data) const noexcept { return get_power_mechanic(data); }
    bool realm::has_state_right(const size_t &data) const noexcept { return is_state() && get_state_mechanic(data); }
    bool realm::has_enacted_law_with_flag(const size_t &) const noexcept { return false; }
    
    void realm::succession() {
      ASSERT(is_state() || is_self());
      ASSERT(leader != nullptr);
      ASSERT(heir != nullptr);
      
      if (is_self()) {
        if (heir->self != nullptr) {
          auto f = heir->self;
          
//           {
//             auto last = f->get_last_title();
//             if (last != nullptr) { last->next = titles; titles = f->titles; }
//           }
//           
//           {
//             auto last = f->get_last_vassal();
//             if (last != nullptr) { last->next_vassal = vassals; vassals = f->vassals; }
//           }
//           
//           {
//             auto last = f->get_last_prisoner();
//             if (last != nullptr) { last->next_prisoner = prisoners; prisoners = f->prisoners; }
//           }
//           
//           {
//             auto last = heir->get_last_courtier();
//             if (last != nullptr) { last->next_courtier = courtiers; courtiers = f->courtiers; }
//           }
          
          const size_t token = f.get_token();
          global::get<core::context>()->destroy_realm(token);
        }
        
        if (heir->realms[character::establishment] != nullptr) {
          ASSERT(heir->realms[character::establishment]->is_state());
          heir->realms[character::establishment]->succession(); 
          // если у наследника выборная форма, то мы просто выбираем следующего
          // нет тут более сложная херня, наследник может быть вообще каким нибудь хером с горы
          // если это личные владения то они и прейдут в личные владения наследника
          // если это элективные владения, то они должны остаться и достаться элективному наследнику
          // то бишь получается что у челика может быть как бы множество наследников
        }
        
        heir->resources.add(core::character_resources::money, leader->resources.get(core::character_resources::money));
        
        heir->self = utils::handle<realm>(this, this->object_token);
      }
      
      leader = heir;
      heir = nullptr;
      
      // где то тут мы должны вызвать функцию on_action
      // можем ли мы вызвать эвенты до смерти персонажа и передачи прав? можем по идее
      // множество эвентов запускается в этот момент (в основном проверки и передача флагов наследникам)
      // просто проверяем че там в on_action контейнере
      
      // я теряю тут указатель
    }
    
    void realm::add_title(titulus* title) {
      if (title->owner == this) return;
      //if (title->owner != nullptr) title->owner->remove_title(title);
      
      ASSERT(title->owner == nullptr);
      ASSERT(utils::ring::list_empty<utils::list_type::titles>(title));
      
      title->owner = utils::handle<core::realm>(this, this->object_token);
      if (titles == nullptr) { titles = title; }
      else { utils::ring::list_add<utils::list_type::titles>(titles, title); }
      
      if (main_title == nullptr || main_title->type() < title->type()) main_title = title;
      // передача вассалов - отдельно
    }
    
    void realm::remove_title(titulus* title) {
      ASSERT(title->owner == this);
      if (titles == title) titles = utils::ring::list_next<utils::list_type::titles>(title, title);
      
      title->owner = nullptr;
      utils::ring::list_remove<utils::list_type::titles>(title);
      if (titles == nullptr) main_title = nullptr;
      
      if (main_title == title) {
        auto tmp = titles, current = tmp;
        auto max_type = tmp->type();
        while (tmp != nullptr) {
          if (max_type < tmp->type()) {
            current = tmp;
            max_type = tmp->type();
          }
          
          tmp = utils::ring::list_next<utils::list_type::titles>(tmp, titles);
        }
        
        main_title = current;
      }
    }
    
    titulus* realm::get_last_title() const {
      if (titles == nullptr) return nullptr;
      
      auto last = utils::ring::list_prev<utils::list_type::titles>(titles, titles);
      return last == nullptr ? titles : last;
    }
    
    void realm::sort_titles() {
      if (titles == nullptr) return;
      
      size_t title_count = 0;
      std::array<core::titulus*, 256> title_array;
      
      auto tmp = titles;
      while (tmp != nullptr) {
        title_array[title_count] = tmp;
        ++title_count;
        ASSERT(title_count < 256);
        tmp = utils::ring::list_next<utils::list_type::titles>(tmp, titles);
      }
      
      auto main_t = main_title;
      std::sort(title_array.begin(), title_array.begin()+title_count, [main_t] (const core::titulus* first, const core::titulus* second) -> bool {
        const size_t first_type = static_cast<size_t>(first->type());
        const size_t second_type = static_cast<size_t>(second->type());
        return first == main_t || first_type > second_type;
      });
      
      titles = title_array[0];
      utils::ring::list_invalidate<utils::list_type::titles>(title_array[0]);
      for (size_t i = 1; i < title_count; ++i) {
        utils::ring::list_invalidate<utils::list_type::titles>(title_array[i]);
        utils::ring::list_radd<utils::list_type::titles>(titles, title_array[i]);
      }
    }
    
    bool realm::has_landed_title() const {
      auto tmp = titles;
      while (tmp != nullptr) {
        if (tmp->type() == titulus::type::baron && !tmp->is_formal()) return true;
        tmp = utils::ring::list_next<utils::list_type::titles>(tmp, titles);
      }
      
      return false;
    }
    
    void realm::add_vassal(realm* vassal) {
      if (vassal->liege == this) return;
      if (vassal->liege != nullptr) vassal->liege->remove_vassal(vassal);
      
      ASSERT(vassal->liege == nullptr);
      ASSERT(utils::ring::list_empty<utils::list_type::vassals>(vassal));
      
      vassal->liege = utils::handle<core::realm>(this, this->object_token);
      if (vassals == nullptr) vassals = vassal;
      else utils::ring::list_radd<utils::list_type::vassals>(vassals, vassal);
    }
    
    void realm::add_vassal_raw(realm* vassal) {
      ASSERT(vassal->liege == this);
      if (vassals == nullptr) vassals = vassal;
      else utils::ring::list_radd<utils::list_type::vassals>(vassals, vassal);
    }
    
    void realm::remove_vassal(realm* vassal) {
      if (vassal->liege != this) return;
      
      //vassal->liege = nullptr;
      vassal->liege = utils::handle<core::realm>();
      if (vassals == vassal) vassals = utils::ring::list_next<utils::list_type::vassals>(vassal, vassal);
      utils::ring::list_remove<utils::list_type::vassals>(vassal);
    }
    
    realm* realm::get_last_vassal() const {
      if (vassals == nullptr) return nullptr;
      
      auto last = utils::ring::list_prev<utils::list_type::vassals>(vassals, vassals);
      return last == nullptr ? vassals : last;
    }
    
    void realm::add_prisoner(struct character* prisoner) {
      if (prisoner->prison == this) return;
      if (prisoner->imprisoner != nullptr) prisoner->imprisoner->remove_prisoner(prisoner);
      
      ASSERT(prisoner->imprisoner == nullptr);
      ASSERT(utils::ring::list_empty<utils::list_type::prisoners>(prisoner));
      
      prisoner->prison = utils::handle<realm>(this, this->object_token);
      if (prisoners == nullptr) prisoners = prisoner;
      else utils::ring::list_add<utils::list_type::prisoners>(prisoners, prisoner);
    }
    
    void realm::add_prisoner_raw(character* prisoner) {
      ASSERT(prisoner->prison == this);
      if (prisoners == nullptr) prisoners = prisoner;
      else utils::ring::list_add<utils::list_type::prisoners>(prisoners, prisoner);
    }
    
    void realm::remove_prisoner(struct character* prisoner) {
      ASSERT(prisoner->prison == this);
      
      prisoner->imprisoner = nullptr;
      if (prisoners == prisoner) prisoners = utils::ring::list_next<utils::list_type::prisoners>(prisoner, prisoner);
      utils::ring::list_remove<utils::list_type::prisoners>(prisoner);
    }
    
    character* realm::get_last_prisoner() const {
      if (prisoners == nullptr) return nullptr;
      auto last = utils::ring::list_prev<utils::list_type::prisoners>(prisoners, prisoners);
      return last == nullptr ? prisoners : last;
    }
    
    void realm::add_courtier(character* courtier) {
      if (courtier->suzerain == this) return;
      if (courtier->suzerain != nullptr) courtier->suzerain->remove_courtier(courtier);
      
      ASSERT(courtier->suzerain == nullptr);
      ASSERT(utils::ring::list_empty<utils::list_type::courtiers>(courtier));
      
      courtier->suzerain = utils::handle<realm>(this, this->object_token);
      if (courtiers == nullptr) courtiers = courtier;
      else utils::ring::list_add<utils::list_type::courtiers>(courtiers, courtier);
    }
    
    void realm::add_courtier_raw(character* courtier) {
      ASSERT(courtier->suzerain == this);
      if (courtiers == nullptr) courtiers = courtier;
      else utils::ring::list_add<utils::list_type::courtiers>(courtiers, courtier);
    }
    
    void realm::remove_courtier(character* courtier) {
      ASSERT(courtier->suzerain == this);
      
      courtier->suzerain = nullptr;
      if (courtiers == courtier) courtiers = utils::ring::list_next<utils::list_type::courtiers>(courtier, courtier);
      utils::ring::list_remove<utils::list_type::courtiers>(courtier);
    }
    
    character* realm::get_last_courtier() const {
      if (courtiers == nullptr) return nullptr;
      auto last = utils::ring::list_prev<utils::list_type::courtiers>(courtiers, courtiers);
      return last == nullptr ? courtiers : last;
    }
    
    void realm::set_capital(city* c) {
      auto t = c->title;
      assert(t->owner.get() == this);
      capital = c;
    }
    
    bool realm::include(character* c) const {
      if (is_state_independent_power()) return c->realms[character::establishment] == this;
      if (is_council()) return c->realms[character::council] == this;
      if (is_tribunal()) return c->realms[character::tribunal] == this;
      if (is_assembly()) return c->realms[character::assembly] == this;
      if (is_clergy()) return c->realms[character::clergy] == this;
      assert(is_self());
      return c->self == this;
    }
    
    OUTPUT_CITY_TYPE2 realm::get_capital_city() const { return capital; }
    OUTPUT_TITLE_TYPE realm::get_capital_barony() const { return capital->province->title; }
    OUTPUT_PROVINCE_TYPE realm::get_capital_province() const { return capital->province; }
    OUTPUT_TITLE_TYPE realm::get_main_title() const { return main_title; }
    OUTPUT_RELIGION_TYPE realm::get_dominant_religion() const {
      const auto state = get_state();
      return state->clergy.valid() ? state->clergy->dominant_religion : state->dominant_religion;
    }
    
    OUTPUT_REALM_TYPE realm::get_state() const { return state; }
    OUTPUT_REALM_TYPE realm::get_council() const { return council; }
    OUTPUT_REALM_TYPE realm::get_tribunal() const { return tribunal; }
    OUTPUT_REALM_TYPE realm::get_assembly() const { return assembly; }
    OUTPUT_REALM_TYPE realm::get_clergy() const { return clergy; }
    OUTPUT_REALM_TYPE realm::get_liege() const { return liege; }
    OUTPUT_REALM_TYPE realm::get_top_liege() const { return liege.valid() ? (liege->liege.valid() ? liege->get_top_liege() : liege) : utils::handle<core::realm>(); }
  }
}
