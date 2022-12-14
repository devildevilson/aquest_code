#ifndef DEVILS_ENGINE_CORE_CONTEXT_H
#define DEVILS_ENGINE_CORE_CONTEXT_H

#include <vector>
#include <array>
#include <function2.hpp>

#include "declare_structures_table.h"
#include "structures_header.h"
#include "map.h"

#include "utils/handle.h"
#include "parallel_hashmap/phmap.h"
#include "utils/memory_pool.h"
#include "utils/constexpr_funcs.h"

namespace devils_engine {
  namespace core {
    struct map;
    
    class context {
    public:
      static const size_t invalid_token = SIZE_MAX;
      
      enum tokenized_type {
        realm_token,
        army_token,
        hero_troop_token,
        war_token,
        troop_token,
        count
      };
      
      static constexpr size_t type_bits_count = count_useful_bits(count);
      static_assert(type_bits_count == 3);
      static constexpr size_t counter_bits_count = 20;
      static constexpr size_t counter_max = make_mask(counter_bits_count); // 1048575
      static constexpr size_t index_bits_count = SIZE_WIDTH - type_bits_count - counter_bits_count;
      static constexpr size_t max_index = make_mask(index_bits_count);
      
      static std::tuple<tokenized_type, size_t, size_t> parse_token(const size_t &token);
      
      context() noexcept;
      ~context() noexcept;
      
      template <typename T>
      void create_container(const size_t &count) {
        static_assert(T::s_type < structure::static_types_count);
        const size_t index = static_cast<size_t>(T::s_type);
        if (containers[index].memory != nullptr) throw std::runtime_error("Memory for type " + std::string(structure_data::names[index]) + " is already created");
        containers[index].type_size = sizeof(T);
        containers[index].count = count;
        containers[index].memory = new T[count];
        
        if constexpr (T::s_type == structure::tile) tiles_data_created = true;
      }
      
      template <typename T>
      T* get_entity(const size_t &index) {
        static_assert(T::s_type < structure::static_types_count);
        auto &c = containers[static_cast<size_t>(T::s_type)];
        if (index >= c.count) return nullptr;
        auto mem = reinterpret_cast<T*>(c.memory);
        return &mem[index];
      }
      
      template <typename T>
      const T* get_entity(const size_t &index) const {
        static_assert(T::s_type < structure::static_types_count);
        const auto &c = containers[static_cast<size_t>(T::s_type)];
        if (index >= c.count) return nullptr;
        auto mem = reinterpret_cast<const T*>(c.memory);
        return &mem[index];
      }
      
      template <typename T>
      size_t get_entity_count() const {
        static_assert(T::s_type < structure::static_types_count);
        const auto &c = containers[static_cast<size_t>(T::s_type)];
        return c.count;
      }
      
      // ?????? ?????????? ?????? ?????????? ???????????? ???? id, ?????? ?????? ????????????? 
      // ?? ?????????????????? ???? ???????????????? ????????, ?????????? ???????????????? ?????????????????? ????????????
      template <typename T>
      T* get_entity(const std::string_view &id) {
        static_assert(T::s_type < structure::static_types_count);
        const size_t index = static_cast<size_t>(T::s_type);
        static_assert(index < static_cast<size_t>(core::structure::id_types_count), "Object of this type doesnt contain an id");
        //if constexpr (index >= static_cast<size_t>(core::structure::id_types_count)) throw std::runtime_error("Object of type " + std::string(magic_enum::enum_name(T::s_type)) + " does not have the id");
        const auto &map = id_maps[index];
        auto itr = map.find(id);
        return itr != map.end() ? reinterpret_cast<T*>(itr->second) : nullptr;
      }
      
      template <typename T>
      const T* get_entity(const std::string_view &id) const {
        static_assert(T::s_type < structure::static_types_count);
        const size_t index = static_cast<size_t>(T::s_type);
        static_assert(index < static_cast<size_t>(core::structure::id_types_count), "Object of this type doesnt contain an id");
        //if constexpr (index >= static_cast<size_t>(core::structure::id_types_count)) throw std::runtime_error("Object of type " + std::string(magic_enum::enum_name(T::s_type)) + " does not have the id");
        const auto &map = id_maps[index];
        auto itr = map.find(id);
        return itr != map.end() ? reinterpret_cast<T*>(itr->second) : nullptr;
      }
      
      inline bool is_tiles_created() const { return tiles_data_created; }
      
      void fill_id_maps();
      
//       void set_tile(const uint32_t &index, const tile &tile_data);
//       tile get_tile(const uint32_t &index) const;
//       tile* get_tile_ptr(const uint32_t &index);
      
      // ???????????? ???? ???????????????????? ?????????????? ????????????????, ???? ???????????? ???????? ?????? ???? ?????????? ???????????????????????? ???????????????????? ??????????????
      dynasty* create_dynasty();
      character* create_character(const bool male, const bool dead);
      utils::handle<realm> create_realm();
      utils::handle<army> create_army(); // ?????????? ???? ???????? ?????? ???????????????????? ?????????? ????????
      utils::handle<hero_troop> create_hero_troop();
      utils::handle<war> create_war();
      // ?????? ?????????????? ?????????? ?????? ??????????? ???????? ???????? ?????? ???? ?????????????????????? ?? ????????????????????
      // ?? ???????????? ?????????????? ?? ???????????? ?? ???? ???????? ???????????????? ???????????? ?????? ?????? ???? ?????? ??????????????
      // ?????????? ?????? ?? ???????????? ?? ?????????????? (?? ?????????????????)
      // (???????????? ????????????????, ???? ?????? ????????? ?????????? ???? ?????? ???????????? ???????????????????????????? ???? ?????????? ?????????? ?????????????)
      utils::handle<troop> create_troop();
      
      // ?????????????? ?? ???????? ???????????????????? ???? ???????? ??????????????????, ?????????????? ???????? ?????? ?????????? ???????????????? ?????????????
      // ?????? ???? ???????????????? ?????? ?????????? ?????????????? ????????????????
      void destroy(dynasty* d);
      void destroy(character* c); // ?????????? ???? ?????????????????? ???????????? ??????????????????? ???????? ?????????????????? ?????? ????????????????
      void destroy_realm(const size_t &token); // ?????????????? ?????????? ???????????? ?????? ????????????????
      void destroy_army(const size_t &token);
      void destroy_hero_troop(const size_t &token);
      void destroy_war(const size_t &token);
      void destroy_troop(const size_t &token);
      
      dynasty* get_dynasty(const size_t &index) const;
//       character* get_character(const size_t &index) const;
      realm* get_realm(const size_t &token) const;
      army* get_army(const size_t &token) const;
      hero_troop* get_hero_troop(const size_t &token) const;
      war* get_war(const size_t &token) const;
      troop* get_troop(const size_t &token) const;
      
      // ???????????????? ?????????????????? ???????????? ???????????????? ??????????????, ???????? ???? ???????????? ??????????????????
      // ?????????? ?????????? ?????????????????????? ???????????? ?? ???????????????? ?????? ???????????????? ????????????????????
      // ???? ?????????????? ???????? ?? ???????????? ?????????????????????? ???????????????? ???????????????????? 
      // (????????????, ??????????????????, ?????? ?????? ?????????????? ?? ?????????????? ?? ??????????????), 
      // ???????????? ???????????????? ?????????????????? ?????????? ?? ?????????? ?????????? ?? ?????? ???????????????????? ????????????????????
      // ???????? ??????, ?????????? ???????? ????????????????, ???????????? ???????? ?????? ?????????????????????? ???? ???????? ????????????????????
      // ?????????????????? ???? ?????????? ???????????????????????????? ??????????
      size_t get_realm_token(const realm* r) const;
      size_t get_army_token(const army* a) const;
      size_t get_hero_troop_token(const hero_troop* h) const;
      size_t get_war_token(const war* w) const;
      size_t get_troop_token(const troop* t) const;
      
      size_t characters_count() const;
      size_t dynasties_count() const;
      
      void sort_characters();
//       size_t first_not_dead_character() const;
//       size_t first_playable_character() const;
//       size_t characters_count() const;
      size_t dead_characters_count() const;
      size_t living_characters_count() const;
      size_t living_playable_characters_count() const;
      character* get_character(const size_t &index) const;
      character* get_dead_character(const size_t &index) const;
      character* get_living_character(const size_t &index) const;
      character* get_living_playable_character(const size_t &index) const;
      
      void destroy_dead_character(character* c);
      void make_dead(character* c);
      void make_not_playable(character* c);
      void make_playable(character* c);
      
      size_t get_army_container_size() const;
      army* get_army_raw(const size_t &index) const;
      
      void update_armies(const size_t &time, fu2::function_view<void(const size_t &, army*)> func);
      
      size_t get_character_debug_index(const character* c) const;
      
      // ?????? ?????????????? ?????????? ?????????? ??????? ???????? ?? ???????
      uint32_t cast_ray(const core::map* map, const utils::ray &ray, float &ray_dist);
      
      size_t compute_data_memory() const;
    private:
      struct container {
        size_t type_size;
        size_t count;
        void* memory;
        
        container();
      };
      
      // ??????, ???????????? ?????? ???????? ??????????????????????
      // ?????????? ?????????????????????? ?????????????????? ?????? ?????????????? ???????? ?????? set ?? map ?????????????????????? (???????????? ???????????????????? ???????????????? ?????? ?????????????? ????????)
      // ???? ???????????? ?????? ???????????????? (?????????? ???????????????????????? ???????????? ?????? ?????????? ??????????????, ?????? ???????????? ?????????? ???????????? ?????????????????????? ???????????? ?????????? ????????????)
//       memory::static_allocator_storage<POINTER_MEMORY_SIZE> pointer_storage; 
//       static_pool_t pointer_static_pool;
      
      std::atomic_bool tiles_data_created;
      
      // ?????? ?????????? ?????????????????????????? ?? ?????????????? ????????????
//       std::array<tile, core::map::hex_count_d(core::map::detail_level)> tile_array;
      std::array<container, static_cast<size_t>(structure::static_types_count)> containers; // ?????????????? ?????????? ???????????????? ?????? ?????????????? ????????????, ?????????? ?????????????? ???????????? ????????????
      std::array<phmap::flat_hash_map<std::string_view, void*>, static_cast<size_t>(structure::id_types_count)> id_maps;
      
      // ?????? ?????????????? ?????????? ???????????????????????? ?????????????????????? ???? ???????? ???????????????????? ????????, ???? ???????????????????????? ?????????? ?????????????????????? ?????????????????? ??????????????
      utils::memory_pool<dynasty, sizeof(dynasty)*5000> dynasties_pool;
      utils::memory_pool<character, sizeof(character)*5000> characters_pool;
      utils::memory_pool<realm, sizeof(realm)*5000> realms_pool;
      // ?????? ?????????????????? ?????????????????? ?? ?????? ?????? ???????? ?????????? ??????????????????????????????
      // ?? ???????????? ?????????????????? ?????????? ????????????????????, ?????? ?????????? ?????????? ???? ?????????????? ???????? ???????????? ??????
      // ???????? ???????????? ??????????????????, ???? ???????? ?????? ?????????????? ???????????????????????? ????????????
      // ???????????????? ???????????????? ?????????????????????? ?? ?????? ?????? ???????????? ?????????? ?????????????? ?? ???????????????? ????????????
      utils::memory_pool<army, sizeof(army)*5000> armies_pool;
      utils::memory_pool<hero_troop, sizeof(hero_troop)*5000> hero_troops_pool;
      utils::memory_pool<war, sizeof(war)*5000> wars_pool;
      utils::memory_pool<troop, sizeof(troop)*5000> troops_pool;
      
      // ?????????? ?????????????????? ?????? ???????? ?????????????????????????? shared_ptr - ???????????? ????????
      // ???????????????? ???????????? - ?????? ???????????????????????? ?????????? ID, 
      // ???? ???????????????? ?????????????? ???????????? ???? ???????? ?????????? ?? ?????? ???????????????? ?????????? ???????? ???????????? ????????????
      // ID ?????? 100% ???????????? ??????????????, ?? ???? ???????????? ctx->get_obj(id) ?????? ?????????????? ???????? ?? ????????????????
      // ?????????????????? ?????? ??????????????????, ???????????????? ???? ???? ?????????? ?????? ?????????? ???????????? ?????????????? ?? ?????? ???????? ???????????????? ????????
      // ???????? ???????????????????????? ???????? ????????????, ???????? ???????????????????????? ??????????, ?????????? ?????????????? ?????????????? ???????????? ?? ?????????????? ???? ?????????? ????????????????
      // ?????? ???????????????????????? ?????????????
      
      // ?????? ?????????? ?????? ??????????????????? ?????????????????? ?????????? ???????? ?? ??????????????????, ?? ???????????? ?????????????????? ????????
      // ???????? ???? ?????????? ?? ?????????? ???????????? ????????, ?????????????????? ?????????? ???? ??????????????????, ???????????????? ????????
      // ?????????? ??????????????????, ?????????? ?????????????????? ?? ?????????? ????????, ?????????????? ???? ?????????? ?????????? ?????????
      // ???? ???? ???????? ?????? ?????????????? ??????????, ???????? ???????????????? ???? ???????????? ???????????? 1000, ?? ???? ?? ????????????
      std::vector<dynasty*> dynasties; // ?????????? ?????????? ?????????????????? ?????????? ???????????? ?????????????? ?????????????????? ????????????????
      std::vector<character*> characters;
      std::vector<character*> dead_characters;
      std::vector<character*> living_characters;
      std::vector<character*> living_playable_characters; // ???????????????????? ?????????????????? ???????????? ???????????????????? ???? ??????????/???? ??????????
      // ???????? ??????????????, ??????????????????? ?????????????? ????????
      std::vector<std::pair<size_t, realm*>> realms;
      std::vector<std::pair<size_t, army*>> armies;
      std::vector<std::pair<size_t, hero_troop*>> hero_troops;
      std::vector<std::pair<size_t, war*>> wars;
      std::vector<std::pair<size_t, troop*>> troops;

      void destroy_container(const structure &s) noexcept;
      
      template <typename T>
      void destroy_container() noexcept {
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
      std::tuple<troop*, size_t> get_troop_index(const size_t &token) const;
      
      size_t compute_realm_token(const size_t &index) const;
      size_t compute_army_token(const size_t &index) const;
      size_t compute_hero_troop_token(const size_t &index) const;
      size_t compute_war_token(const size_t &index) const;
      size_t compute_troop_token(const size_t &index) const;
    };
    
    //uint32_t cast_ray(const core::context* ctx, const core::map* map, const utils::ray &ray, float &ray_dist); // ?????? ?????????????? ???????? ??????
  }
}

#endif
