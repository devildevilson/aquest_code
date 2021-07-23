#include "context.h"

namespace devils_engine {
  namespace core {
    context::context() {
      //memset(containers.data(), 0, sizeof(container) * containers.size());
    }
    
    context::~context() {
      for (size_t i = 0; i < static_cast<size_t>(structure::static_types_count); ++i) {
        destroy_container(static_cast<structure>(i));
      }
      
      for (auto &d : dynasties) {
        dynasties_pool.destroy(d);
      }
      
      for (auto &c : characters) {
        characters_pool.destroy(c);
      }
      
      for (const auto &a : armies) {
        armies_pool.destroy(a.second);
      }
      
      for (const auto &a : hero_troops) {
        hero_troops_pool.destroy(a.second);
      }
      
      for (const auto &a : realms) {
        realms_pool.destroy(a.second);
      }
      
      for (const auto &a : wars) {
        wars_pool.destroy(a.second);
      }
    }
    
    template <typename T>
    void fill_type(const size_t &count, void* memory, phmap::flat_hash_map<std::string_view, void*> &map) {
      auto array = reinterpret_cast<T*>(memory);
      for (size_t i = 0; i < count; ++i) {
        map.emplace(array[i].id, &array[i]);
      }
    }
    
#define FILL_TYPE_FUNC(type) fill_type<type>(containers[static_cast<uint32_t>(structure::type)].count, \
                               containers[static_cast<uint32_t>(structure::type)].memory, \
                               id_maps[static_cast<uint32_t>(id_struct::type)]); \
      ASSERT(containers[static_cast<uint32_t>(structure::type)].count == id_maps[static_cast<uint32_t>(id_struct::type)].size());
    
    void context::fill_id_maps() {
      FILL_TYPE_FUNC(building_type)
      FILL_TYPE_FUNC(city_type)
      FILL_TYPE_FUNC(trait)
      FILL_TYPE_FUNC(modificator)
      FILL_TYPE_FUNC(troop_type)
      FILL_TYPE_FUNC(decision)
      FILL_TYPE_FUNC(interaction)
      FILL_TYPE_FUNC(religion_group)
      FILL_TYPE_FUNC(religion)
      FILL_TYPE_FUNC(culture)
      FILL_TYPE_FUNC(law)
      FILL_TYPE_FUNC(event)
      FILL_TYPE_FUNC(titulus)
      FILL_TYPE_FUNC(casus_belli)
    }
    
    void context::set_tile(const uint32_t &index, const tile &tile_data) {
      ASSERT(index < core::map::hex_count_d(core::map::detail_level));
      tile_array[index] = tile_data;
    }
    
    tile context::get_tile(const uint32_t &index) const {
      ASSERT(index < core::map::hex_count_d(core::map::detail_level));
      return tile_array[index];
    }
    
    dynasty* context::create_dynasty() {
      auto ptr = dynasties_pool.create();
      dynasties.push_back(ptr);
      return ptr;
    }
    
    character* context::create_character(const bool male, const bool dead) {
      auto ptr = characters_pool.create(male, dead);
      characters.push_back(ptr);
      return ptr;
    }
    
    size_t context::create_army() {
      auto ptr = armies_pool.create();
      //armies.push_back(ptr); // не думаю что это нужно, хотя армия может быть и без полководца
      ASSERT(ptr->army_gpu_slot != UINT32_MAX);
      if (armies.size() <= ptr->army_gpu_slot) armies.resize(ptr->army_gpu_slot+1, std::make_pair(SIZE_MAX, nullptr));
      ++armies[ptr->army_gpu_slot].first;
      armies[ptr->army_gpu_slot].second = ptr;
      ASSERT(armies[ptr->army_gpu_slot].first != SIZE_MAX);
      assert(ptr->army_gpu_slot < armies_max_count);
      assert(armies[ptr->army_gpu_slot].first < counter_max);
      const size_t token = type_mult * army_token + armies_max_count * armies[ptr->army_gpu_slot].first + ptr->army_gpu_slot;
      return token;
    }
    
    size_t context::create_realm() {
      auto ptr = realms_pool.create();
      for (size_t i = 0; i < realms.size(); ++i) {
        if (realms[i].second == nullptr) {
          ++realms[i].first;
          realms[i].second = ptr;
          assert(realms[i].first < counter_max);
          const size_t token = type_mult * realm_token + realms_max_count * realms[i].first + i;
          return token;
        }
      }
      
      realms.emplace_back(0, ptr);
      assert(realms.size()-1 < realms_max_count);
      const size_t token = type_mult * realm_token + realms_max_count * realms.back().first + realms.size()-1;
      return token;
    }
    
    size_t context::create_hero_troop() {
      auto ptr = hero_troops_pool.create();
      ASSERT(ptr->army_gpu_slot != UINT32_MAX);
      // тут поди нужно поставить SIZE_MAX, что бы первый был 0 а не 1
      if (hero_troops.size() <= ptr->army_gpu_slot) armies.resize(ptr->army_gpu_slot+1, std::make_pair(SIZE_MAX, nullptr));
      ++hero_troops[ptr->army_gpu_slot].first;
      hero_troops[ptr->army_gpu_slot].second = ptr;
      ASSERT(hero_troops[ptr->army_gpu_slot].first != SIZE_MAX);
      assert(ptr->army_gpu_slot < hero_troops_max_count);
      assert(hero_troops[ptr->army_gpu_slot].first < counter_max);
      const size_t token = type_mult * hero_troop_token + hero_troops_max_count * hero_troops[ptr->army_gpu_slot].first + ptr->army_gpu_slot;
      return token;
    }
    
    size_t context::create_war() {
      auto ptr = wars_pool.create();
      for (size_t i = 0; i < wars.size(); ++i) {
        if (wars[i].second == nullptr) {
          ++wars[i].first;
          wars[i].second = ptr;
          assert(wars[i].first < counter_max);
          const size_t token = type_mult * war_token + wars_max_count * wars[i].first + i;
          return token;
        }
      }
      
      wars.emplace_back(0, ptr);
      assert(wars.size()-1 < wars_max_count);
      const size_t token = type_mult * war_token + wars_max_count * wars.back().first + wars.size()-1;
      return token;
    }
    
    void context::destroy(dynasty* d) {
      for (size_t i = 0; i < dynasties.size(); ++i) {
        if (dynasties[i] == d) {
          dynasties_pool.destroy(dynasties[i]);
          std::swap(dynasties.back(), dynasties[i]);
          dynasties.pop_back();
          break;
        }
      }
    }
    
    void context::destroy(character* c) {
      for (size_t i = 0; i < characters.size(); ++i) {
        if (characters[i] == c) {
          characters_pool.destroy(characters[i]);
          std::swap(characters.back(), characters[i]);
          characters.pop_back();
          break;
        }
      }
    }
    
    void context::destroy_army(const size_t &token) {
      auto [ptr, index] = get_army_index(token);
      armies_pool.destroy(ptr);
      if (ptr != nullptr) armies[index].second = nullptr;
    }
    
    void context::destroy_realm(const size_t &token) {
      auto [ptr, index] = get_realm_index(token);
      realms_pool.destroy(ptr);
      if (ptr != nullptr) realms[index].second = nullptr;
    }
    
    void context::destroy_hero_troop(const size_t &token) {
      auto [ptr, index] = get_hero_troop_index(token);
      hero_troops_pool.destroy(ptr);
      if (ptr != nullptr) hero_troops[index].second = nullptr;
    }
    
    void context::destroy_war(const size_t &token) {
      auto [ptr, index] = get_war_index(token);
      wars_pool.destroy(ptr);
      if (ptr != nullptr) wars[index].second = nullptr;
    }
    
    // с династиями не понятно пока что делать
    dynasty* context::get_dynasty(const size_t &index) const {
      ASSERT(index < dynasties.size());
      return dynasties[index];
    }
    
    character* context::get_character(const size_t &index) const {
      ASSERT(index < characters.size());
      return characters[index];
    }
    
    realm* context::get_realm(const size_t &token) const {
      auto [ptr, index] = get_realm_index(token);
      return ptr;
    }
    
    army* context::get_army(const size_t &token) const {
      auto [ptr, index] = get_army_index(token);
      return ptr;
    }
    
    hero_troop* context::get_hero_troop(const size_t &token) const {
      auto [ptr, index] = get_hero_troop_index(token);
      return ptr;
    }
    
    war* context::get_war(const size_t &token) const {
      auto [ptr, index] = get_war_index(token);
      return ptr;
    }
    
    size_t context::get_realm_token(const realm* r) const {
      if (r == nullptr) return SIZE_MAX;
      for (size_t i = 0; i < realms.size(); ++i) {
        if (realms[i].second == r) return type_mult * realm_token + realms_max_count * realms[i].first + i;
      }
      
      throw std::runtime_error("Could not find realm in array");
      return SIZE_MAX;
    }
    
    size_t context::get_army_token(const army* a) const {
      if (a == nullptr) return SIZE_MAX;
      for (size_t i = 0; i < armies.size(); ++i) {
        if (armies[i].second == a) return type_mult * army_token + armies_max_count * armies[i].first + i;
      }
      
      throw std::runtime_error("Could not find army in array");
      return SIZE_MAX;
    }
    
    size_t context::get_hero_troop_token(const hero_troop* h) const {
      if (h == nullptr) return SIZE_MAX;
      for (size_t i = 0; i < hero_troops.size(); ++i) {
        if (hero_troops[i].second == h) return type_mult * hero_troop_token + hero_troops_max_count * hero_troops[i].first + i;
      }
      
      throw std::runtime_error("Could not find hero troop in array");
      return SIZE_MAX;
    }
    
    size_t context::get_war_token(const war* w) const {
      if (w == nullptr) return SIZE_MAX;
      
      for (size_t i = 0; i < wars.size(); ++i) {
        if (wars[i].second == w) return type_mult * war_token + wars_max_count * wars[i].first + i;
      }
      
      throw std::runtime_error("Could not find war in array");
      return SIZE_MAX;
    }
    
    size_t context::characters_count() const {
      return characters.size();
    }
    
    size_t context::dynasties_count() const {
      return dynasties.size();
    }
    
    void context::sort_characters() {
      std::sort(characters.begin(), characters.end(), [] (const character* first, const character* second) -> bool {
        const uint32_t first_attribs = uint32_t(!first->is_dead()) + uint32_t(first->is_ai_playable());
        const uint32_t second_attribs = uint32_t(!second->is_dead()) + uint32_t(second->is_ai_playable());
        return first_attribs < second_attribs;
      });
    }
    
    size_t context::first_not_dead_character() const {
      for (size_t i = 0; i < characters.size(); ++i) {
        if (!characters[i]->is_dead()) return i;
        ASSERT(!characters[i]->is_ai_playable());
      }
      
      throw std::runtime_error("All characters are dead");
    }
    
    size_t context::first_playable_character() const {
      for (size_t i = 0; i < characters.size(); ++i) {
        if (characters[i]->is_ai_playable()) return i;
      }
      
      throw std::runtime_error("All characters are unplayable");
    }
    
    void context::update_armies(const size_t &time) {
      for (const auto &pair : armies) {
        pair.second->update(time);
      }
    }
    
    size_t context::compute_data_memory() const {
      size_t counter = 0;
      for (size_t i = 0; i < static_cast<size_t>(structure::static_types_count); ++i) {
        counter += containers[i].type_size * containers[i].count;
      }
      
      counter += dynasties_pool.blocks_allocated() * dynasties_pool.block_size();
      counter += characters_pool.blocks_allocated() * characters_pool.block_size();
      counter += armies_pool.blocks_allocated() * armies_pool.block_size();
      counter += realms_pool.blocks_allocated() * realms_pool.block_size();
      counter += hero_troops_pool.blocks_allocated() * hero_troops_pool.block_size();
      
      return counter;
    }
    
    context::container::container() : type_size(0), count(0), memory(nullptr) {}
    
    void context::destroy_container(const structure &s) {
      ASSERT(s < structure::static_types_count);
      
      switch (s) {
        case structure::tile:           destroy_container<tile>();           break;
        case structure::province:       destroy_container<province>();       break;
        case structure::building_type:  destroy_container<building_type>();  break;
        case structure::city_type:      destroy_container<city_type>();      break;
        case structure::city:           destroy_container<city>();           break;
        case structure::trait:          destroy_container<trait>();          break;
        case structure::modificator:    destroy_container<modificator>();    break;
        case structure::troop_type:     destroy_container<troop_type>();     break;
        case structure::decision:       destroy_container<decision>();       break;
        case structure::interaction:    destroy_container<interaction>();    break;
        case structure::religion_group: destroy_container<religion_group>(); break;
        case structure::religion:       destroy_container<religion>();       break;
        case structure::culture:        destroy_container<culture>();        break;
        case structure::law:            destroy_container<law>();            break;
        case structure::event:          destroy_container<event>();          break;
        case structure::titulus:        destroy_container<titulus>();        break;
        case structure::casus_belli:    destroy_container<casus_belli>();    break;
        default: throw std::runtime_error("Bad structure type");
      }
    }
    
    std::tuple<realm*, size_t> context::get_realm_index(const size_t &token) const {
      const size_t token_type      = token / type_mult;
      const size_t current_token   = token % type_mult;
      const size_t current_counter = current_token / realms_max_count;
      const size_t current_index   = current_token % realms_max_count;
      const auto &pair = realms[current_index];
      return token != invalid_token && 
             token_type == realm_token && 
             current_index < realms.size() && 
             pair.first == current_counter ? 
               std::make_tuple(pair.second, current_index) : 
               std::make_tuple(nullptr, SIZE_MAX);
    }
    
    std::tuple<army*, size_t> context::get_army_index(const size_t &token) const {
      const size_t token_type      = token / type_mult;
      const size_t current_token   = token % type_mult;
      const size_t current_counter = current_token / armies_max_count;
      const size_t current_index   = current_token % armies_max_count;
      const auto &pair = armies[current_index];
      return token != invalid_token && 
             token_type == army_token && 
             current_index < armies.size() && 
             pair.first == current_counter ? 
               std::make_tuple(pair.second, current_index) : 
               std::make_tuple(nullptr, SIZE_MAX);
    }
    
    std::tuple<hero_troop*, size_t> context::get_hero_troop_index(const size_t &token) const {
      const size_t token_type      = token / type_mult;
      const size_t current_token   = token % type_mult;
      const size_t current_counter = current_token / hero_troops_max_count;
      const size_t current_index   = current_token % hero_troops_max_count;
      const auto &pair = hero_troops[current_index];
      return token != invalid_token && 
             token_type == hero_troop_token && 
             current_index < hero_troops.size() && 
             pair.first == current_counter ? 
               std::make_tuple(pair.second, current_index) : 
               std::make_tuple(nullptr, SIZE_MAX);
    }
    
    std::tuple<war*, size_t> context::get_war_index(const size_t &token) const {
      const size_t token_type      = token / type_mult;
      const size_t current_token   = token % type_mult;
      const size_t current_counter = current_token / wars_max_count;
      const size_t current_index   = current_token % wars_max_count;
      const auto &pair = wars[current_index];
      return token != invalid_token && 
             token_type == war_token && 
             current_index < wars.size() && 
             pair.first == current_counter ? 
               std::make_tuple(pair.second, current_index) : 
               std::make_tuple(nullptr, SIZE_MAX);
    }
    
    const size_t context::realms_max_count;
    const size_t context::armies_max_count;
    const size_t context::hero_troops_max_count; 
    const size_t context::wars_max_count;
    const size_t context::counter_max;
    const size_t context::maximum_type_count;
    const size_t context::type_mult;
    const size_t context::invalid_token;
    const size_t context::maximum_value;
    const uint32_t context::id_struct_map[] = {
      UINT32_MAX,
      UINT32_MAX,
      UINT32_MAX,
      static_cast<uint32_t>(id_struct::building_type),
      static_cast<uint32_t>(id_struct::city_type),
      static_cast<uint32_t>(id_struct::trait),
      static_cast<uint32_t>(id_struct::modificator),
      static_cast<uint32_t>(id_struct::troop_type),
      static_cast<uint32_t>(id_struct::decision),
      static_cast<uint32_t>(id_struct::interaction),
      static_cast<uint32_t>(id_struct::religion_group),
      static_cast<uint32_t>(id_struct::religion),
      static_cast<uint32_t>(id_struct::culture),
      static_cast<uint32_t>(id_struct::law),
      static_cast<uint32_t>(id_struct::event),
      static_cast<uint32_t>(id_struct::titulus),
      static_cast<uint32_t>(id_struct::casus_belli),
      UINT32_MAX,
      UINT32_MAX,
      UINT32_MAX,
      UINT32_MAX,
      UINT32_MAX,
      UINT32_MAX,
      UINT32_MAX
    };
    
    static_assert(sizeof(context::id_struct_map) / sizeof(context::id_struct_map[0]) == static_cast<uint32_t>(structure::count));
  }
}
