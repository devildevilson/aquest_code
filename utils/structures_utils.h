#ifndef STRUCTURES_UTILS_H
#define STRUCTURES_UTILS_H

#include <string>
#include <cstddef>
#include <cstdint>
#include "parallel_hashmap/phmap.h"
#include "core/stat_data.h"
#include "core/stat_modifier.h"
#include "script/object.h"

// имеет ли смысл для этих структур добавлять std mutex?
// может сильно помочь на самом деле
// с другой стороны было бы неплохо ограничить доступ к этому всему во время загрузок или ходов
// как обходить? коллбек функция, можно ли как нибудь через итераторы?
// можно поставить секцию на всю функцию интерфейса и тогда без особых проблем 
// обойти структуры по итератору, желательно конечно чтобы ход был быстрым

namespace devils_engine {
  namespace core {
    struct trait;
    struct event;
    struct modificator;
    struct character;
  }
  
  namespace utils {
    struct flags_container {
      phmap::flat_hash_map<std::string, size_t> flags;
      
      void update_turn();
      bool has_flag(const std::string_view &flag) const;
      void add_flag(const std::string_view &flag, const size_t &turns);
      void remove_flag(const std::string_view &flag);
      void clear_timed_flags();
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
    
    // тут добавятся данные с мапой от предыдущих вызовов
    struct events_container {
      struct event_data {
        size_t mtth; // 0 мгновенно вызывается, 1 с шансом 50/50 на этом ходу
        // тут видимо нужно будет еще хранить контекст? 
        script::object root;
        phmap::flat_hash_map<std::string_view, script::object> context;
      };
      
      phmap::node_hash_map<const core::event*, event_data> events;
      
      //void update_turn(); // по идее тут нужен особый апдейт
      bool has_event(const core::event* event) const;
      void add_event(const core::event* event, event_data &&data);
      void remove_event(const core::event* event);
    };
    
    // тут надо как то учитывать модификатор отношений (дополнительные данные)
    struct modificators_container {
      struct modificator_data {
        static const size_t max_stat_modifiers_count = 4;
        static const size_t max_opinion_modifiers_count = 4;
        
        size_t turns_count;
        std::array<core::stat_modifier, max_stat_modifiers_count> bonuses;
        std::array<core::opinion_modifier, max_opinion_modifiers_count> opinion_mods;
      };
      
      phmap::node_hash_map<const core::modificator*, modificator_data> modificators;
      
      void update_turn();
      bool has_modificator(const core::modificator* modificator) const;
      void add_modificator(const core::modificator* modificator, const modificator_data &data);
      void remove_modificator(const core::modificator* modificator);
    };
    
    struct hooks_container {
      // хук? хук всегда связан с персонажем, у хука есть тип + некоторые характеристики
      struct data {
        uint32_t type; // сильный/слабый
//         union {
          uint32_t cooldown; // можно ли потратить сильный хук?
          uint32_t expries;
//         };
        uint32_t secret; // тип секрета, для локализации например
      };
      
      phmap::node_hash_map<const core::character*, data> hooks;
      
      void update_turn();
      bool has_hook(const core::character* c) const;
      void add_hook(const core::character* c, const struct data &data);
      void remove_hook(const core::character* c);
    };

    // тут нужно как то добавить проверку типов, в принципе особо проблем нет просто добавить указатель
    // нужно ли проверять указатель на баунды? а мы можем указать в теплейте? можем
    // почему наличие указателя выдает ворнинг subobject-linkage? пишет что есть анонимный неймспейс
    // стаковерфлоу говорит что это баг компилятора
    template <typename T, bool check_character_stat_type = false>
    struct stats_container {
      std::array<core::stat_container, T::count> array;
      
      stats_container() {
        memset(array.data(), 0, array.size() * sizeof(array[0]));
      }
      
      stats_container(const stats_container &other) {
        memcpy(array.data(), other.array.data(), array.size() * sizeof(array[0]));
      }
      
      stats_container(stats_container &&other) = delete;
      
      float get(const uint32_t &index) const {
        assert(index < array.size());
        if constexpr (!check_character_stat_type) return array[index].ival;
        switch (core::character_stats::types[index]) {
          case core::stat_value_type::int_t:   return array[index].ival;
          case core::stat_value_type::uint_t:  return array[index].uval;
          case core::stat_value_type::float_t: return array[index].fval;
          default: assert(false);
        }
        return 0.0f;
      }
      
      void set(const uint32_t &index, const float &value) {
        assert(index < array.size());
        if constexpr (!check_character_stat_type) { array[index].ival = value; return; }
        switch (core::character_stats::types[index]) {
          case core::stat_value_type::int_t:   array[index].ival = value; break;
          case core::stat_value_type::uint_t:  array[index].uval = value; break;
          case core::stat_value_type::float_t: array[index].fval = value; break;
          default: assert(false);
        }
      }
      
      float add(const uint32_t &index, const float &value) {
        assert(index < array.size());
        if constexpr (!check_character_stat_type) { const int32_t a = array[index].ival; array[index].ival += value; return a; }
        switch (core::character_stats::types[index]) {
          case core::stat_value_type::int_t:   { const  int32_t a = array[index].ival; array[index].ival += value; return a; }
          case core::stat_value_type::uint_t:  { const uint32_t a = array[index].uval; array[index].uval += value; return a; }
          case core::stat_value_type::float_t: { const    float a = array[index].fval; array[index].fval += value; return a; }
          default: assert(false);
        }
        return 0.0f;
      }
      
      stats_container & operator=(const stats_container &other) {
        memcpy(array.data(), other.array.data(), array.size() * sizeof(array[0]));
        return *this;
      }
      
      stats_container & operator=(stats_container &&other) = delete;
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
