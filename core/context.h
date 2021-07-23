#ifndef CORE_CONTEXT_H
#define CORE_CONTEXT_H

#include <vector>
#include <array>
#include "parallel_hashmap/phmap.h"
#include "bin/map.h"
#include "utils/memory_pool.h"
#include "utils/magic_enum_header.h"
#include "structures_header.h"
#include "utils/constexpr_funcs.h"

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
      // тут тип нужно прикинуть количество провинций * 4, я думаю что провок будет не больше чем 5к (и то скорее всего это много)
      static const size_t realms_max_count = 5000 * 4;
      // потенциально каждая провка может сделать уникальную армию, поставим тут пока что от балды 10к
      static const size_t armies_max_count = 10000;
      // отряды героя не зависят от количества провинций, но вряд ли из будет слишком много, оставим пока 10к
      static const size_t hero_troops_max_count = 10000; 
      // одновременно в мире сколько масимально будет происходить войн? меньше чем всего реалмов
      static const size_t wars_max_count = 10000;
      // для того чтобы ввести тип токена в формулу, надо понять насколько большим может быть счетчик, тоже 10к?
      static const size_t counter_max = 10000;
      
      static const size_t maximum_type_count = max(realms_max_count, armies_max_count, hero_troops_max_count, wars_max_count);
      static_assert(maximum_type_count == realms_max_count);
      static const size_t type_mult = maximum_type_count * counter_max;
      
      static const size_t invalid_token = SIZE_MAX;
      
      enum tokenized_type {
        realm_token,
        army_token,
        hero_troop_token,
        war_token,
        count
      };
      
      static const size_t maximum_value = type_mult * count + maximum_type_count * counter_max + maximum_type_count-1;
      static_assert(maximum_value < INT64_MAX);
      
      static const uint32_t id_struct_map[];
      
      context();
      ~context();
      
      template <typename T>
      void create_container(const size_t &count) {
        static_assert(T::s_type < structure::static_types_count);
        const size_t index = static_cast<size_t>(T::s_type);
        if (containers[index].memory != nullptr) throw std::runtime_error("Memory for type " + std::string(magic_enum::enum_name<structure>(T::s_type)) + " is already created");
        containers[index].type_size = sizeof(T);
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
      
      template <typename T>
      size_t get_entity_count() const {
        static_assert(T::s_type < structure::static_types_count);
        const auto &c = containers[static_cast<size_t>(T::s_type)];
        return c.count;
      }
      
      // тут нужно еще брать энтити по id, как это делать? 
      // в контейнер не добавить мапу, нужно отдельно создавать массив
      template <typename T>
      T* get_entity(const std::string_view &id) {
        static_assert(T::s_type < structure::static_types_count);
        const size_t index = static_cast<size_t>(T::s_type);
        const uint32_t map_index = id_struct_map[index];
        if constexpr (map_index == UINT32_MAX) throw std::runtime_error("Object of type " + std::string(magic_enum::enum_name(T::s_type)) + " does not have the id");
        const auto &map = id_maps[map_index];
        auto itr = map.find(id);
        return itr != map.end() ? reinterpret_cast<T*>(itr->second) : nullptr;
      }
      
      template <typename T>
      const T* get_entity(const std::string_view &id) const {
        static_assert(T::s_type < structure::static_types_count);
        const size_t index = static_cast<size_t>(T::s_type);
        const uint32_t map_index = id_struct_map[index];
        if constexpr (map_index == UINT32_MAX) throw std::runtime_error("Object of type " + std::string(magic_enum::enum_name(T::s_type)) + " does not have the id");
        const auto &map = id_maps[map_index];
        auto itr = map.find(id);
        return itr != map.end() ? reinterpret_cast<T*>(itr->second) : nullptr;
      }
      
      void fill_id_maps();
      
      void set_tile(const uint32_t &index, const tile &tile_data);
      tile get_tile(const uint32_t &index) const;
      
      // вообще то количество городов меняется, но другое дело что мы знаем максимальное количество городов
      dynasty* create_dynasty();
      character* create_character(const bool male, const bool dead);
      size_t create_realm();
      size_t create_army(); // армии то поди без полководца могут быть
      size_t create_hero_troop();
      size_t create_war();
      
      // удаляет и всех персонажей по всей видимости, удаляем если все члены династии мертвы?
      // мне бы хотелось еще какую историю записать
      void destroy(dynasty* d);
      void destroy(character* c); // будут ли персонажи вообще удаляться? поди персонажи без династии
      void destroy_realm(const size_t &token); // удаляем когда теряем все владения
      void destroy_army(const size_t &token);
      void destroy_hero_troop(const size_t &token);
      void destroy_war(const size_t &token);
      
      dynasty* get_dynasty(const size_t &index) const;
      character* get_character(const size_t &index) const;
      realm* get_realm(const size_t &token) const;
      army* get_army(const size_t &token) const;
      hero_troop* get_hero_troop(const size_t &token) const;
      war* get_war(const size_t &token) const;
      
      // операция получения токена довольно опасная, если мы храним указатель
      // имеет смысл задизайнить доступ к объектам без хранения указателей
      // по крайней мере в местах длительного хранения информации 
      // (скрипт, интерфейс, все что свзяано с войнами и армиями), 
      // реалмы работают несколько иначе и время жизни у них достаточно длительное
      // хотя нет, могут быть проблемы, другое дело что практически во всех структурах
      // указатели на реалм инвалидируются игрой
      size_t get_realm_token(const realm* r) const;
      size_t get_army_token(const army* a) const;
      size_t get_hero_troop_token(const hero_troop* h) const;
      size_t get_war_token(const war* w) const;
      
      size_t characters_count() const;
      size_t dynasties_count() const;
      
      void sort_characters();
      size_t first_not_dead_character() const;
      size_t first_playable_character() const;
      
      void update_armies(const size_t &time);
      
      size_t compute_data_memory() const;
    private:
      struct container {
        size_t type_size;
        size_t count;
        void* memory;
        
        container();
      };
      
      // так, похоже что идея накрывается
      // такой статический аллокатор это хорошая идея для set и map контейнеров (память выделяется отдельно для каждого нода)
      // но плохая для массивов (нужно перевыделять память для всего массива, что скорее всего забьет статическую память очень быстро)
//       memory::static_allocator_storage<POINTER_MEMORY_SIZE> pointer_storage; 
//       static_pool_t pointer_static_pool;
      
      // это будет сериализовано с помощью таблиц
      std::array<tile, core::map::hex_count_d(core::map::detail_level)> tile_array;
      std::array<container, static_cast<size_t>(structure::static_types_count)> containers; // титулам нужно добавить еще немного памяти, чтобы создать особые титулы
      std::array<phmap::flat_hash_map<std::string_view, void*>, static_cast<size_t>(id_struct::count)> id_maps;
      
      // эти объекты будут существовать практически на всем протяжении игры, но потенциально могут приключится несколько проблем
      utils::memory_pool<dynasty, sizeof(dynasty)*5000> dynasties_pool;
      utils::memory_pool<character, sizeof(character)*5000> characters_pool;
      utils::memory_pool<realm, sizeof(realm)*5000> realms_pool;
      // эти структуры нуждаются в той или иной форме индентификатора
      // в лучших традициях сырых указателей, мне нужно каким то образом дать понять что
      // этот объект уничтожен, до того как сделать непоправимые ошибки
      // основная проблема заключается в том что объект может попасть в контекст эвента
      utils::memory_pool<army, sizeof(army)*5000> armies_pool;
      utils::memory_pool<hero_troop, sizeof(hero_troop)*5000> hero_troops_pool;
      utils::memory_pool<war, sizeof(war)*5000> wars_pool;
      
      // гугол подсказал что даже использование shared_ptr - плохая идея
      // типичный подход - это использовать некий ID, 
      // по которому берется объект из пула когда с ним работает какой либо другой объект
      // ID это 100% индекс массива, и мы делаем ctx->get_obj(id) для каждого чиха с объектом
      // возвращая его указатель, примерно то же самое мне нужно видимо сделать и для всех объектов выше
      // либо использовать тупа индекс, либо использовать хендл, хендл добавит немного памяти к объекту не особо полезной
      // как использовать индекс?
      
      // что можно еще придумать? поставить здесь лист с массивами, и искать свободный слот
      // хотя мы можем и здесь искать слот, персонажи точно не удаляются, династии тоже
      // армии удаляются, войны удаляются и герои тоже, сколько их всего может быть?
      // ну за один раз реально много, хотя наверное не сильно больше 1000, а то и меньше
      std::vector<dynasty*> dynasties; // здесь можно придумать какой нибудь большой начальный капасити
      std::vector<character*> characters;
      // пара счетчик, указатель? хорошая идея
      std::vector<std::pair<size_t, realm*>> realms;
      std::vector<std::pair<size_t, army*>> armies;
      std::vector<std::pair<size_t, hero_troop*>> hero_troops;
      std::vector<std::pair<size_t, war*>> wars;

      void destroy_container(const structure &s);
      
      template <typename T>
      void destroy_container() {
        static_assert(T::s_type < structure::static_types_count);
        const size_t index = static_cast<size_t>(T::s_type);
        auto mem = reinterpret_cast<T*>(containers[index].memory);
        delete [] mem;
        containers[index].memory = nullptr;
        containers[index].count = 0;
        containers[index].type_size = 0;
      }
      
      std::tuple<realm*, size_t> get_realm_index(const size_t &token) const;
      std::tuple<army*, size_t> get_army_index(const size_t &token) const;
      std::tuple<hero_troop*, size_t> get_hero_troop_index(const size_t &token) const;
      std::tuple<war*, size_t> get_war_index(const size_t &token) const;
    };
  }
}

#endif
