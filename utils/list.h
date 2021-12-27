#ifndef DEVILS_ENGINE_UTILS_LIST_H
#define DEVILS_ENGINE_UTILS_LIST_H

#include <type_traits>
#include "assert.h"

// мы вот можем сделать лист: наследование
// другое дело что нам придется каким то образом обеспечить уникальность структуры
// но при этом нужно будет видимо сделать динамик каст
// тип листов изменится при изменении list_type, мы, как бы, будем наследоваться с разных типов
// некоторые листы у меня расположены во вложенной структуре, что делать? 
// можно по идее получить указатель по оффсету 

namespace devils_engine {
  namespace utils {
    enum class list_type { // возможно имеет смысл перенести типы в другое место
      prisoners,
      courtiers,
      father_line_siblings,
      mother_line_siblings,
      prev_consorts,
      concubines,
      vassals,
      titles,
      sibling_titles,
      faiths,
      sibling_cultures,
      victims,
      statemans,
      councilors,
      magistrates,
      assemblers,
      clergymans,
      state_electors,
      council_electors,
      tribunal_electors,
      assembly_electors,
      clergy_electors,
      city_troops,
      army_troops,
      province_cities,
      hero_companions,
      culture_member,
      dynasty_member,
      believer,
      secret_believer,
    };
    
    namespace forw {
      template <typename T, list_type t>
      struct list {
        T* m_next;
        
        list() noexcept : m_next(nullptr) {}
        
        void add(T* obj) noexcept {
          static_assert(std::is_base_of_v<list<T, t>, T>);
          auto l = static_cast<list<T, t>>(obj);
          l->m_next = m_next;
          m_next = obj;
        }
        
        void remove(T* prev) noexcept {
          static_assert(std::is_base_of_v<list<T, t>, T>);
          auto l = static_cast<list<T, t>>(prev);
          l->m_next = m_next;
          m_next = nullptr;
        }
        
        void invalidate() noexcept { m_next = nullptr; }
        bool empty() const noexcept { return m_next == nullptr; }
      };
      
      template <list_type t, typename T>
      void list_add(T* cur, T* obj) noexcept {
        list<T, t>* l = cur;
        l->add(obj);
      }
      
      template <list_type t, typename T>
      T* list_next(T* cur) noexcept;

      template <list_type t, typename T>
      void list_remove(T* root, T* cur) noexcept {
        auto ptr = root;
        while (ptr != nullptr) {
          list<T, t>* l = ptr;
          if (l->m_next == cur) {
            list<T, t>* cur_l = cur;
            cur_l->remove(ptr);
            break;
          }
          ptr = list_next<t>(ptr);
        }
      }

      template <list_type t, typename T>
      T* list_next(const T* cur) noexcept {
        const list<T, t>* l = cur;
        return l->m_next;
      }
      
      template <list_type t, typename T>
      bool list_empty(const T* cur) noexcept {
        const list<T, t>* l = cur;
        return l->empty();
      }
      
      template <list_type t, typename T>
      void list_invalidate(T* cur) noexcept {
        list<T, t>* l = cur;
        l->invalidate();
      }
      
      template <list_type t, typename T>
      size_t list_count(T* cur) noexcept {
        size_t counter = 0;
        for (auto c = cur; c != nullptr; c = list_next<t>(c, cur)) { ++counter; }
        return counter;
      }
    }
    
    namespace ring {
      template <typename T, list_type t>
      struct list {
        using current_list_p = list<T, t>*;
        
        T* m_next;
        T* m_prev;
        
        list() noexcept : m_next(static_cast<T*>(this)), m_prev(static_cast<T*>(this)) {}
        // пока непонятно насколько это испортит разные списки в игре
        // но с другой стороны, если мы в этот момент не будем ничего обходить
        // то так аккуратно уберем невалидные данные
        ~list() noexcept { remove(); }
    
        void add(T* obj) noexcept {
          static_assert(std::is_base_of_v<list<T, t>, T>);
          current_list_p l = obj;
          auto cur = static_cast<T*>(this);
          l->m_next = m_next;
          l->m_prev = cur;
          current_list_p n = m_next;
          m_next = obj;
          n->m_prev = obj;
        }
        
        void radd(T* obj) noexcept {
          static_assert(std::is_base_of_v<list<T, t>, T>);
          current_list_p l = obj;
          auto cur = static_cast<T*>(this);
          l->m_next = cur;
          l->m_prev = m_prev;
          current_list_p n = m_prev;
          m_prev = obj;
          n->m_next = obj;
        }
        
        void remove() noexcept {
          static_assert(std::is_base_of_v<list<T, t>, T>);
          auto l_next = static_cast<current_list_p>(m_next);
          auto l_prev = static_cast<current_list_p>(m_prev);
          l_next->m_prev = m_prev;
          l_prev->m_next = m_next;
          m_prev = static_cast<T*>(this);
          m_next = static_cast<T*>(this);
        }
        
        void invalidate() noexcept { m_next = static_cast<T*>(this); m_prev = static_cast<T*>(this); }
        bool empty() const noexcept { return m_next == static_cast<const T*>(this) && m_prev == static_cast<const T*>(this); }
      };
    
      template <list_type t, typename T>
      void list_add(T* cur, T* obj) noexcept {
        list<T, t>* l = cur;
        l->add(obj);
      }
      
      template <list_type t, typename T>
      void list_radd(T* cur, T* obj) noexcept {
        list<T, t>* l = cur;
        l->radd(obj);
      }

      template <list_type t, typename T>
      void list_remove(T* cur) noexcept {
        list<T, t>* l = cur;
        l->remove();
      }

      template <list_type t, typename T>
      T* list_next(const T* cur, const T* ref) noexcept {
        const list<T, t>* l = cur;
        return cur != nullptr && l->m_next != ref ? l->m_next : nullptr;
      }

      template <list_type t, typename T>
      T* list_prev(const T* cur, const T* ref) noexcept {
        const list<T, t>* l = cur;
        return cur != nullptr && l->m_prev != ref ? l->m_prev : nullptr;
      }
      
      template <list_type t, typename T>
      bool list_empty(const T* cur) noexcept {
        const list<T, t>* l = cur;
        return l->empty();
      }
      
      template <list_type t, typename T>
      void list_invalidate(T* cur) noexcept {
        list<T, t>* l = cur;
        l->invalidate();
      }
      
      template <list_type t, typename T>
      size_t list_count(T* cur) noexcept {
        size_t counter = 0;
        for (auto c = cur; c != nullptr; c = list_next<t>(c, cur)) { ++counter; }
        return counter;
      }
    }
  }
}

#endif
