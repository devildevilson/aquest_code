#ifndef BATTLE_CONTEXT_H
#define BATTLE_CONTEXT_H

#include <cstdint>
#include <cstddef>
#include <array>
#include "battle_structures.h"

// что тут должно быть? он должен быть похож на кор контекст, то есть тут должно быть описание всех объектов битвы
// юниты, отряды, типы юнитов отрядов, постройки, биомы (?), словом почти все кроме непосредственно данных тайла
// 

namespace devils_engine {
  namespace battle {
    class context {
    public:
      context();
      ~context();
      
      template <typename T>
      void create_container(const size_t &count) {
        static_assert(T::type_id < structure_type::count);
        const size_t type_id = static_cast<size_t>(T::type_id);
        if (containers[type_id].memory != nullptr) throw std::runtime_error("Container is already exist");
        containers[type_id].count = count;
        containers[type_id].memory = new T[count];
      }
      
      template <typename T>
      T* get_entity(const size_t &index) {
        static_assert(T::type_id < structure_type::count);
        const size_t type_id = static_cast<size_t>(T::type_id);
        auto ptr = reinterpret_cast<T*>(containers[type_id].memory);
        if (index >= containers[type_id].count) throw std::runtime_error("Wrong index. Type " + std::to_string(type_id));
        return &ptr[index];
      }
      
      template <typename T>
      const T* get_entity(const size_t &index) const {
        static_assert(T::type_id < structure_type::count);
        const size_t type_id = static_cast<size_t>(T::type_id);
        auto ptr = reinterpret_cast<T*>(containers[type_id].memory);
        if (index >= containers[type_id].count) throw std::runtime_error("Wrong index. Type " + std::to_string(type_id));
        return &ptr[index];
      }
      
      template <typename T>
      size_t get_entity_count() const {
        static_assert(T::type_id < structure_type::count);
        const size_t type_id = static_cast<size_t>(T::type_id);
        return containers[type_id].count;
      }
      
      template <typename T>
      void set_entity_data(const size_t &index, const T &data) {
        static_assert(T::type_id < structure_type::count);
        const size_t type_id = static_cast<size_t>(T::type_id);
        if (index >= containers[type_id].count) throw std::runtime_error("Wrong index. Type " + std::to_string(type_id));
        auto ptr = reinterpret_cast<T*>(containers[type_id].memory);
        ptr[index] = data;
      }
      
      
    private:
      struct container {
        size_t count;
        void* memory;
      };
      
      // отряды и юниты - их не константное количество
      // но при этом очень редко они исчезают полностью
      std::array<container, static_cast<size_t>(structure_type::count)> containers;
    };
  }
}

#endif
