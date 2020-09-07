#ifndef CORE_CONTEXT_H
#define CORE_CONTEXT_H

#include "utils/memory_pool.h"
#include "core_structures.h"
#include <vector>
#include <array>

// #include <foonathan/memory/container.hpp> // vector, list, list_node_size
// #include <foonathan/memory/memory_pool.hpp> // memory_pool
// #include <foonathan/memory/smart_ptr.hpp> // allocate_unique
// #include <foonathan/memory/static_allocator.hpp> // static_allocator_storage, static_block_allocator
// #include <foonathan/memory/temporary_allocator.hpp> // temporary_allocator

// alias namespace foonathan::memory as memory for easier access
//#include <foonathan/memory/namespace_alias.hpp>

#define POINTER_MEMORY_SIZE (1 * 1024 * 1024)

// using namespace foonathan;

// количество всех типов структур, кроме персонажей, фракций и династий, заранее определено

namespace devils_engine {
  namespace core {
    class context {
//       using static_pool_t = memory::memory_pool<memory::array_pool, memory::static_block_allocator>;
    public:
      context();
      ~context();
      
      template <typename T>
      void create_container(const size_t &count) {
        static_assert(T::s_type < structure::static_types_count);
        const size_t index = static_cast<size_t>(T::s_type);
        if (containers[index].memory != nullptr) throw std::runtime_error("Memory for type " + std::string(magic_enum::enum_name<structure>(T::s_type)) + " is already created");
        containers[index].count = count;
        containers[index].memory = new T[count];
      }
      
      template <typename T>
      T* get_entity(const size_t &index) {
        static_assert(T::s_type < structure::static_types_count);
        auto &c = containers[static_cast<size_t>(T::s_type)];
        ASSERT(index < c.count);
        auto mem = reinterpret_cast<T*>(c.memory);
        return &mem[index];
      }
      
      template <typename T>
      const T* get_entity(const size_t &index) const {
        static_assert(T::s_type < structure::static_types_count);
        auto &c = containers[static_cast<size_t>(T::s_type)];
        ASSERT(index < c.count);
        auto mem = reinterpret_cast<T*>(c.memory);
        return &mem[index];
      }
      
      dynasty* create_dynasty();
      character* create_character(const bool male, const bool dead);
      army* create_army();
      faction* create_faction();
      hero_troop* create_hero_troop();
      
      // удаляет и всех персонажей по всей видимости, удаляем если все члены династии мертвы?
      // мне бы хотелось еще какую историю записать
      void destroy(dynasty* d);
      void destroy(character* c); // это будет происходить довольно часто (персонажи без династии удаляются бесследно скорее всего)
      void destroy(army* a);
      void destroy(faction* f); // удаляем когда теряем все владения
      void destroy(hero_troop* h);
      
      dynasty* get_dynasty(const size_t &index);
      character* get_character(const size_t &index);
      
      size_t characters_count() const;
      size_t dynasties_count() const;
      
      void sort_characters();
      size_t first_not_dead_character() const;
      size_t first_playable_character() const;
      
      size_t compute_data_memory() const;
    private:
      struct container {
        size_t count;
        void* memory;
      };
      
      // так, похоже что идея накрывается
      // такой статический аллокатор это хорошая идея для set и map контейнеров (память выделяется отдельно для каждого нода)
      // но плохая для массивов (нужно перевыделять память для всего массива, что скорее всего забьет статическую память очень быстро)
//       memory::static_allocator_storage<POINTER_MEMORY_SIZE> pointer_storage; 
//       static_pool_t pointer_static_pool;
      
      // это будет сериализовано с помощью таблиц
      std::array<container, static_cast<size_t>(structure::static_types_count)> containers; // титулам нужно добавить еще немного памяти, чтобы создать особые титулы
      
      utils::memory_pool<dynasty, sizeof(dynasty)*5000> dynasties_pool;
      utils::memory_pool<character, sizeof(character)*5000> characters_pool;
      utils::memory_pool<army, sizeof(army)*5000> armies_pool;
      // эти два по идее создаются только у персонажа
      utils::memory_pool<faction, sizeof(faction)*5000> factions_pool;
      utils::memory_pool<hero_troop, sizeof(hero_troop)*5000> hero_troops_pool;
      
      // нужно это сериализовать
//       memory::vector<dynasty*, static_pool_t> dynasties; // это по сути хранилища для персонажей
//       memory::vector<character*, static_pool_t> characters;
//       memory::vector<army*, static_pool_t> armies;            // армии могут быть без полководцев
//       memory::vector<faction*, static_pool_t> factions;       // скорее всего будем создавать в персонажах
//       memory::vector<hero_troop*, static_pool_t> hero_troops; 
      
      std::vector<dynasty*> dynasties; // здесь можно придумать какой нибудь большой начальный капасити
      std::vector<character*> characters;
      std::vector<army*> armies;         

      void destroy_container(const structure &s);
      
      template <typename T>
      void destroy_container() {
        static_assert(T::s_type < structure::static_types_count);
        const size_t index = static_cast<size_t>(T::s_type);
        auto mem = reinterpret_cast<T*>(containers[index].memory);
        delete [] mem;
        containers[index].memory = nullptr;
        containers[index].count = 0;
      }
    };
  }
}

#endif
