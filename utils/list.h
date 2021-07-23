#ifndef UTILS_LIST_H
#define UTILS_LIST_H

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
      siblings,
      vassals
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
          assert(l->m_next == static_cast<T*>(this));
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
      T* list_next(T* cur) noexcept {
        list<T, t>* l = cur;
        return l->m_next;
      }
    }
    
    namespace ring {
      template <typename T, list_type t>
      struct list {
        T* m_next;
        T* m_prev;
        
        list() noexcept : m_next(static_cast<T*>(this)), m_prev(static_cast<T*>(this)) {}
    
        void add(T* obj) noexcept {
          static_assert(std::is_base_of_v<list<T, t>, T>);
          list<T, t>* l = obj;
          auto cur = static_cast<T*>(this);
          l->m_next = m_next;
          l->m_prev = cur;
          list<T, t>* n = m_next;
          m_next = obj;
          n->m_prev = obj;
        }
        
        void remove() noexcept {
          static_assert(std::is_base_of_v<list<T, t>, T>);
          auto l_next = static_cast<list<T, t>*>(m_next);
          auto l_prev = static_cast<list<T, t>*>(m_prev);
          l_next->m_prev = m_prev;
          l_prev->m_next = m_next;
          m_prev = static_cast<T*>(this);
          m_next = static_cast<T*>(this);
        }
        
        void invalidate() noexcept { m_next = static_cast<T*>(this); m_prev = static_cast<T*>(this); }
        bool empty() const noexcept { return m_next == static_cast<T*>(this) && m_prev == static_cast<T*>(this); }
      };
    
      template <list_type t, typename T>
      void list_add(T* cur, T* obj) noexcept {
        list<T, t>* l = cur;
        l->add(obj);
      }

      template <list_type t, typename T>
      void list_remove(T* cur) noexcept {
        list<T, t>* l = cur;
        l->remove();
      }

      template <list_type t, typename T>
      T* list_next(T* cur, T* ref) noexcept {
        list<T, t>* l = cur;
        return l->m_next == ref ? nullptr : l->m_next;
      }

      template <list_type t, typename T>
      T* list_prev(T* cur, T* ref) noexcept {
        list<T, t>* l = cur;
        return l->m_prev == ref ? nullptr : l->m_prev;
      }
    }
    
//     struct test_struct : public ring_list<test_struct, list_type::siblings> {
//       int fsf121f2;
//       int dfdf323;
//       
//     };
  }
}

#endif
