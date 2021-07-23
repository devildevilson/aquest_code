#include "realm.h"

#include "utils/globals.h"
#include "context.h"
#include "character.h"
#include "titulus.h"

namespace devils_engine {
  namespace core {
    const structure realm::s_type;
    realm::realm() : 
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
    {
      // тут было бы неплохо создать сразу с персонажем это дело
//       memset(stats.data(), 0, sizeof(stats));
    }
    
    void realm::succession() {
      ASSERT(is_state() || is_self());
      ASSERT(leader != nullptr);
      ASSERT(heir != nullptr);
      
      if (is_self()) {
        if (heir->realms[character::self] != nullptr) {
          auto f = heir->realms[character::self];
          
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
          
          const size_t token = global::get<core::context>()->get_realm_token(f);
          global::get<core::context>()->destroy_realm(token);
        }
        
        if (heir->realms[character::elective] != nullptr) {
          ASSERT(heir->realms[character::elective]->is_state());
          heir->realms[character::elective]->succession(); 
          // если у наследника выборная форма, то мы просто выбираем следующего
          // нет тут более сложная херня, наследник может быть вообще каким нибудь хером с горы
          // если это личные владения то они и прейдут в личные владения наследника
          // если это элективные владения, то они должны остаться и достаться элективному наследнику
          // то бишь получается что у челика может быть как бы множество наследников
        }
        
        heir->resources.add(core::character_resources::money, leader->resources.get(core::character_resources::money));
        
        heir->realms[character::self] = this;
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
    
    void realm::remove_title(titulus* title) {
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
    
    titulus* realm::get_last_title() const {
      if (titles == nullptr) return nullptr;
      
      auto tmp = titles, last = titles->prev;
      while (tmp != nullptr) {
        last = tmp;
        tmp = tmp->next;
      }
      
      ASSERT(!(last == nullptr || last->next != nullptr));
      return last;
    }
    
    void realm::sort_titles() {
      size_t title_count = 0;
      std::array<core::titulus*, 256> title_array;
      
      auto tmp = titles;
      while (tmp != nullptr) {
        title_array[title_count] = tmp;
        ++title_count;
        ASSERT(title_count < 256);
        tmp = tmp->next;
      }
      
      auto main_t = main_title;
      std::sort(title_array.begin(), title_array.begin()+title_count, [main_t] (const core::titulus* first, const core::titulus* second) -> bool {
        const size_t first_type = static_cast<size_t>(first->type);
        const size_t second_type = static_cast<size_t>(second->type);
        return first == main_t || first_type > second_type;
      });
      
      auto prev = title_array[0];
      titles = title_array[0];
      prev->next = nullptr;
      prev->prev = nullptr;
      for (size_t i = 1; i < title_count; ++i) {
        title_array[i]->next = nullptr;
        title_array[i]->prev = nullptr;
        
        prev->next = title_array[i];
        title_array[i]->prev = prev;
        prev = title_array[i];
      }
    }
    
    void realm::add_vassal(realm* vassal) {
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
    
    void realm::add_vassal_raw(realm* vassal) {
      ASSERT(vassal->liege == this);
      vassal->next_vassal = vassals;
      if (vassals != nullptr) vassals->prev_vassal = vassal;
      vassals = vassal;
    }
    
    void realm::remove_vassal(realm* vassal) {
      if (vassal->liege != this) return;
      
      vassal->liege = nullptr;
      if (vassal->prev_vassal != nullptr) vassal->prev_vassal->next_vassal = vassal->next_vassal;
      if (vassal->next_vassal != nullptr) vassal->next_vassal->prev_vassal = vassal->prev_vassal;
      vassal->prev_vassal = nullptr;
      vassal->next_vassal = nullptr;
    }
    
    realm* realm::get_last_vassal() const {
      if (vassals == nullptr) return nullptr;
      
      auto tmp = vassals, last = vassals->prev_vassal;
      while (tmp != nullptr) {
        last = tmp;
        tmp = tmp->next_vassal;
      }
      
      ASSERT(!(last == nullptr || last->next_vassal != nullptr));
      return last;
    }
    
    void realm::add_prisoner(struct character* prisoner) {
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
    
    void realm::add_prisoner_raw(character* prisoner) {
      ASSERT(prisoner->imprisoner == this);
      prisoner->next_prisoner = prisoners;
      if (prisoners != nullptr) prisoners->prev_prisoner = prisoner;
      prisoners = prisoner;
    }
    
    void realm::remove_prisoner(struct character* prisoner) {
      ASSERT(prisoner->imprisoner == this);
      
      prisoner->imprisoner = nullptr;
      if (prisoner->next_prisoner != nullptr) prisoner->next_prisoner->prev_prisoner = prisoner->prev_prisoner;
      if (prisoner->prev_prisoner != nullptr) prisoner->prev_prisoner->next_prisoner = prisoner->next_prisoner;
      prisoner->next_prisoner = nullptr;
      prisoner->prev_prisoner = nullptr;
    }
    
    character* realm::get_last_prisoner() const {
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
