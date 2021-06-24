#ifndef STRUCTURES_UTILS_H
#define STRUCTURES_UTILS_H

#include <string>
#include <cstddef>
#include <cstdint>
#include "parallel_hashmap/phmap.h"

namespace devils_engine {
  namespace core {
    struct trait;
    struct event;
    struct modificator;
  }
  
  namespace utils {
    struct flags_container {
      phmap::flat_hash_map<std::string, size_t> flags;
      
      void update_turn();
      bool has_flag(const std::string_view &flag) const;
      void add_flag(const std::string_view &flag, const size_t &turns);
      void remove_flag(const std::string_view &flag);
    };
    
    struct traits_container {
      phmap::flat_hash_set<const core::trait*> traits;
      
      // часто ли я буду проверять треиты по указателю? 
      // проверять имеет смысл только в эвентах или в решениях (?)
      // а там можно будет найти треит
      bool has_trait(const core::trait* trait) const;
      void add_trait(const core::trait* trait);
      void remove_trait(const core::trait* trait);
    };
    
    struct events_container {
      phmap::flat_hash_map<const core::event*, size_t> events;
      
      //void update_turn(); // по идее тут нужен особый апдейт
      bool has_event(const core::event* event) const;
      void add_event(const core::event* event, const size_t &turns);
      void remove_event(const core::event* event);
    };
    
    struct modificators_container {
      phmap::flat_hash_map<const core::modificator*, size_t> modificators;
      
      void update_turn();
      bool has_modificator(const core::modificator* modificator) const;
      void add_modificator(const core::modificator* modificator, const size_t &turns);
      void remove_modificator(const core::modificator* modificator);
    };
   
    // массивы вообще имеют право на жизнь
//     struct event_container {
//       size_t time; // время (в ходах) когда флаг был добавлен
//       std::array<utils::target_data, 8> event_stack; // размер стека получаем по валидным указателям
//       event_container() : time(SIZE_MAX) {}
//     };
//     
//     // трейты остаются после смерти персонажа
//     template <size_t N>
//     struct traits_container {
//       size_t count;
//       std::array<const trait*, N> container;
//       
//       traits_container() : count(0), container{nullptr} {}
//       bool has(const trait* t) const {
//         for (uint32_t i = 0; i < count; ++i) {
//           if (container[i] == t) return true;
//         }
//         
//         return false;
//       }
//       
//       void add(const trait* t) {
//         if (has(t)) return;
//         ASSERT(count < N);
//         container[count] = t;
//         ++count;
//       }
//       
//       void remove(const trait* t) {
//         for (uint32_t i = 0; i < count; ++i) {
//           if (container[i] == t) {
//             --count;
//             std::swap(container[i], container[count]);
//             break;
//           }
//         }
//       }
//     };
//     
//     // модификаторы, эвенты и флаги можно будет удалить после смерти персонажа
//     template <size_t N>
//     struct modificators_container {
//       size_t count;
//       std::array<std::pair<const modificator*, size_t>, N> container;
//       
//       modificators_container() : count(0), container{std::make_pair(nullptr, 0)} {}
//       bool has(const modificator* m) const {
//         for (uint32_t i = 0; i < count; ++i) {
//           if (container[i].first == m) return true;
//         }
//         
//         return false;
//       }
//       
//       void add(const modificator* m, const size_t &turn) {
//         if (has(m)) return;
//         ASSERT(count < N);
//         container[count] = std::make_pair(m, turn);
//         ++count;
//       }
//       
//       void remove(const modificator* m) {
//         for (uint32_t i = 0; i < count; ++i) {
//           if (container[i].first == m) {
//             --count;
//             std::swap(container[i], container[count]);
//             break;
//           }
//         }
//       }
//     };
//     
//     template <size_t N>
//     struct events_container {
//       size_t count;
//       std::array<std::pair<const event*, event_container>, N> container;
//       
//       events_container() : count(0), container{std::make_pair(nullptr, event_container())} {}
//       bool has(const event* e) const {
//         for (uint32_t i = 0; i < count; ++i) {
//           if (container[i].first == e) return true;
//         }
//         
//         return false;
//       }
//       
//       void add(const event* e, const event_container &cont) {
//         if (has(e)) return;
//         ASSERT(count < N);
//         container[count] = std::make_pair(e, cont);
//         ++count;
//       }
//       
//       void remove(const event* e) {
//         for (uint32_t i = 0; i < count; ++i) {
//           if (container[i].first == e) {
//             --count;
//             std::swap(container[i], container[count]);
//             break;
//           }
//         }
//       }
//     };
//     
//     template <size_t N>
//     struct flags_container {
//       size_t count;
//       std::array<size_t, N> container;
//       
//       flags_container() : count(0), container{0} {}
//       bool has(const size_t &f) const {
//         for (uint32_t i = 0; i < count; ++i) {
//           if (container[i] == f) return true;
//         }
//         
//         return false;
//       }
//       
//       void add(const size_t &f) {
//         if (has(f)) return;
//         ASSERT(count < N);
//         container[count] = f;
//         ++count;
//       }
//       
//       void remove(const size_t &f) {
//         for (uint32_t i = 0; i < count; ++i) {
//           if (container[i] == f) {
//             --count;
//             std::swap(container[i], container[count]);
//             break;
//           }
//         }
//       }
//     };
    
    // это частично может быть удалено
//     struct data_storage {
//       static const size_t flags_set_storage_size = 4352u;
//       static const size_t flags_set_max_size = 100;
//       static const size_t modificators_map_storage_size = 4864u;
//       static const size_t modificators_map_max_size = 131;
//       static const size_t event_map_storage_size = 4352u;
//       
//       memory::static_allocator_storage<flags_set_storage_size> flags_set_storage;
//       memory::static_allocator_storage<modificators_map_storage_size> modificators_map_storage;
//       memory::static_allocator_storage<event_map_storage_size> event_map_storage;
//       static_pool_t flags_set_static_pool;
//       static_pool_t modificators_map_static_pool;
//       static_pool_t event_map_static_pool;
//       
//       // не думаю что unordered_map и unordered_set - это хорошая идея
//       // можно вполне использовать vector, так как элементов будет сравнительно мало
//       memory::unordered_map<const modificator*, size_t, static_pool_t> modificators;
//       memory::unordered_map<const event*, event_container, static_pool_t> events;
//       memory::unordered_set<std::string_view, static_pool_t> flags;
//       
//       data_storage();
//     };
  }
}

#endif
