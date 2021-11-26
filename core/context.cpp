#include "context.h"

namespace devils_engine {
  namespace core {
    std::tuple<context::tokenized_type, size_t, size_t> context::parse_token(const size_t &token) {
      const size_t type_mask = make_mask(type_bits_count);
      const size_t counter_mask = make_mask(counter_bits_count);
      const size_t index_mask = make_mask(index_bits_count);
      const size_t type = (token >> (SIZE_WIDTH - type_bits_count)) & type_mask;
      const size_t counter = (token >> (SIZE_WIDTH - type_bits_count - counter_bits_count)) & counter_mask;
      const size_t index = token & index_mask;
      return std::make_tuple(static_cast<tokenized_type>(type), counter, index);
    }
    
    context::context() noexcept {
      //memset(containers.data(), 0, sizeof(container) * containers.size());
    }
    
    context::~context() noexcept {
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
        if (map.find(array[i].id) != map.end()) throw std::runtime_error("Duplicate id " + array[i].id);
        map.emplace(array[i].id, &array[i]);
      }
    }
    
#define FILL_TYPE_FUNC(type) fill_type<type>(containers[static_cast<uint32_t>(structure::type)].count, \
                               containers[static_cast<uint32_t>(structure::type)].memory, \
                               id_maps[static_cast<uint32_t>(structure::type)]); \
      ASSERT(containers[static_cast<uint32_t>(structure::type)].count == id_maps[static_cast<uint32_t>(structure::type)].size());
    
    void context::fill_id_maps() {
      FILL_TYPE_FUNC(biome)
      FILL_TYPE_FUNC(building_type)
      FILL_TYPE_FUNC(holding_type)
      FILL_TYPE_FUNC(city_type)
      FILL_TYPE_FUNC(trait)
      FILL_TYPE_FUNC(modificator)
      FILL_TYPE_FUNC(troop_type)
      FILL_TYPE_FUNC(decision)
      FILL_TYPE_FUNC(interaction)
      FILL_TYPE_FUNC(religion_group)
      FILL_TYPE_FUNC(religion)
      FILL_TYPE_FUNC(culture_group)
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
    
    tile* context::get_tile_ptr(const uint32_t &index) {
      ASSERT(index < core::map::hex_count_d(core::map::detail_level));
      return &tile_array[index];
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
    
    utils::handle<army> context::create_army() {
      auto ptr = armies_pool.create();
      size_t index = SIZE_MAX;
      for (size_t i = 0; i < armies.size(); ++i) {
        if (armies[i].second != nullptr) continue;
        
        ++armies[i].first;
        armies[i].second = ptr;
        assert(armies[i].first < counter_max);
        index = i;
        break;
      }
      
      if (index == SIZE_MAX) {
        index = armies.size();
        armies.emplace_back(0, ptr);
        ASSERT(index < max_index);
      }
      
      ASSERT(index != SIZE_MAX);
      const size_t token = compute_army_token(index);
      ptr->object_token = token;
      return utils::handle<army>(ptr, token);
    }
    
    // если каунтер все же достигнет counter_max то что делать?
    // это может произойти только в том случае если текущая структура очень часто переделывается
    // потенциально может произойти с любой структурой в длинной катке
    // быстро ли закончится каунтер больше миллиона? мне кажется что очень вряд ли
    utils::handle<realm> context::create_realm() {
      auto ptr = realms_pool.create();
      size_t token = SIZE_MAX;
      for (size_t i = 0; i < realms.size(); ++i) {
        if (realms[i].second != nullptr) continue;
        
        ++realms[i].first;
        realms[i].second = ptr;
        assert(realms[i].first < counter_max);
        token = compute_realm_token(i);
        break;
      }
      
      if (token == SIZE_MAX) {
        realms.emplace_back(0, ptr);
        ASSERT(realms.size()-1 < max_index); // естественно вряд ли это когда либо произойдет
        token = compute_realm_token(realms.size()-1);
      }
      
      ASSERT(token != SIZE_MAX);
      ptr->object_token = token;
      return utils::handle<realm>(ptr, token);
    }
    
    utils::handle<hero_troop> context::create_hero_troop() {
      auto ptr = hero_troops_pool.create();
      size_t token = SIZE_MAX;
      for (size_t i = 0; i < hero_troops.size(); ++i) {
        if (hero_troops[i].second != nullptr) continue;
        
        ++hero_troops[i].first;
        hero_troops[i].second = ptr;
        assert(hero_troops[i].first < counter_max);
        token = compute_hero_troop_token(i);
        break;
      }
      
      if (token == SIZE_MAX) {
        hero_troops.emplace_back(0, ptr);
        ASSERT(hero_troops.size()-1 < max_index); // естественно вряд ли это когда либо произойдет
        token = compute_hero_troop_token(hero_troops.size()-1);
      }
      
      ASSERT(token != SIZE_MAX);
      ptr->object_token = token;
      return utils::handle<hero_troop>(ptr, token);
    }
    
    utils::handle<war> context::create_war() {
      auto ptr = wars_pool.create();
      size_t token = SIZE_MAX;
      for (size_t i = 0; i < wars.size(); ++i) {
        if (wars[i].second != nullptr) continue;
        
        ++wars[i].first;
        wars[i].second = ptr;
        assert(wars[i].first < counter_max);
        token = compute_war_token(i);
        break;
      }
      
      if (token == SIZE_MAX) {
        wars.emplace_back(0, ptr);
        ASSERT(wars.size()-1 < max_index); // естественно вряд ли это когда либо произойдет
        token = compute_war_token(wars.size()-1);
      }
      
      ASSERT(token != SIZE_MAX);
      ptr->object_token = token;
      return utils::handle<war>(ptr, token);
    }
    
    utils::handle<troop> context::create_troop() {
      auto ptr = troops_pool.create();
      size_t token = SIZE_MAX;
      for (size_t i = 0; i < troops.size(); ++i) {
        if (troops[i].second != nullptr) continue;
        
        ++troops[i].first;
        troops[i].second = ptr;
        assert(troops[i].first < counter_max);
        token = compute_troop_token(i);
        break;
      }
      
      if (token == SIZE_MAX) {
        troops.emplace_back(0, ptr);
        ASSERT(troops.size()-1 < max_index); // естественно вряд ли это когда либо произойдет
        token = compute_troop_token(troops.size()-1);
      }
      
      ASSERT(token != SIZE_MAX);
      ptr->object_token = token;
      return utils::handle<troop>(ptr, token);
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
    
    void context::destroy_troop(const size_t &token) {
      auto [ptr, index] = get_troop_index(token);
      if (ptr == nullptr) return;
      troops_pool.destroy(ptr);
      troops[index].second = nullptr;
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
    
    troop* context::get_troop(const size_t &token) const {
      auto [ptr, index] = get_troop_index(token);
      return ptr;
    }
    
    size_t context::get_realm_token(const realm* r) const {
      if (r == nullptr) return SIZE_MAX;
      for (size_t i = 0; i < realms.size(); ++i) {
        if (realms[i].second == r) return compute_realm_token(i);
      }
      
      throw std::runtime_error("Could not find realm in array");
      return SIZE_MAX;
    }
    
    size_t context::get_army_token(const army* a) const {
      if (a == nullptr) return SIZE_MAX;
      for (size_t i = 0; i < armies.size(); ++i) {
        if (armies[i].second == a) return compute_army_token(i);
      }
      
      throw std::runtime_error("Could not find army in array");
      return SIZE_MAX;
    }
    
    size_t context::get_hero_troop_token(const hero_troop* h) const {
      if (h == nullptr) return SIZE_MAX;
      for (size_t i = 0; i < hero_troops.size(); ++i) {
        if (hero_troops[i].second == h) return compute_hero_troop_token(i);
      }
      
      throw std::runtime_error("Could not find hero troop in array");
      return SIZE_MAX;
    }
    
    size_t context::get_war_token(const war* w) const {
      if (w == nullptr) return SIZE_MAX;
      
      for (size_t i = 0; i < wars.size(); ++i) {
        if (wars[i].second == w) return compute_war_token(i);
      }
      
      throw std::runtime_error("Could not find war in array");
      return SIZE_MAX;
    }
    
    size_t context::get_troop_token(const troop* t) const {
      if (t == nullptr) return SIZE_MAX;
      
      for (size_t i = 0; i < troops.size(); ++i) {
        if (troops[i].second == t) return compute_troop_token(i);
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
    
    size_t context::get_army_container_size() const {
      return armies.size();
    }
    
    army* context::get_army_raw(const size_t &index) const {
      if (index >= armies.size()) return nullptr;
      return armies[index].second;
    }
    
    void context::update_armies(const size_t &time, fu2::function_view<void(const size_t &, army*)> func) {
      for (const auto &pair : armies) {
        if (pair.second == nullptr) continue;
        func(time, pair.second);
      }
    }
    
    size_t context::get_character_debug_index(const character* c) const {
      for (size_t i = 0; i < characters.size(); ++i) {
        if (c == characters[i]) return i;
      }
      
      return SIZE_MAX;
    }
    
    // эту функцию надо бы вынести вне констекста, как это сделать аккуратно?
    // просто по индексу в пределах сайз? или вызывать функцию? сделаю ка я так и так
    // возможно придется делать мультитрединг для этого
//     void context::update_armies(const size_t &time) {
//       
//     }
    
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
    
    void context::destroy_container(const structure &s) noexcept {
      ASSERT(s < structure::static_types_count);
      
      switch (s) {
        case structure::tile:           destroy_container<tile>();           break;
        case structure::biome:          destroy_container<biome>();          break;
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
        case structure::holding_type:   destroy_container<holding_type>();   break;
        case structure::culture_group:  destroy_container<culture_group>();  break;
        default: assert(false && "Bad structure type");
      }
    }
    
    std::tuple<realm*, size_t> context::get_realm_index(const size_t &token) const {
      const auto [type, counter, index] = parse_token(token);
      const auto &pair = realms[index];
      return token != invalid_token && 
             type == realm_token && 
             index < realms.size() && 
             pair.first == counter ? 
               std::make_tuple(pair.second, index) : 
               std::make_tuple(nullptr, SIZE_MAX);
    }
    
    std::tuple<army*, size_t> context::get_army_index(const size_t &token) const {
      const auto [type, counter, index] = parse_token(token);
      const auto &pair = armies[index];
      return token != invalid_token && 
             type == army_token && 
             index < armies.size() && 
             pair.first == counter ? 
               std::make_tuple(pair.second, index) : 
               std::make_tuple(nullptr, SIZE_MAX);
    }
    
    std::tuple<hero_troop*, size_t> context::get_hero_troop_index(const size_t &token) const {
      const auto [type, counter, index] = parse_token(token);
      const auto &pair = hero_troops[index];
      return token != invalid_token && 
             type == hero_troop_token && 
             index < hero_troops.size() && 
             pair.first == counter ? 
               std::make_tuple(pair.second, index) : 
               std::make_tuple(nullptr, SIZE_MAX);
    }
    
    std::tuple<war*, size_t> context::get_war_index(const size_t &token) const {
      const auto [type, counter, index] = parse_token(token);
      const auto &pair = wars[index];
      return token != invalid_token && 
             type == war_token && 
             index < wars.size() && 
             pair.first == counter ? 
               std::make_tuple(pair.second, index) : 
               std::make_tuple(nullptr, SIZE_MAX);
    }
    
    std::tuple<troop*, size_t> context::get_troop_index(const size_t &token) const {      
      const auto [type, counter, index] = parse_token(token);
      const auto &pair = troops[index];
      return token != invalid_token && 
             type == troop_token && 
             index < troops.size() && 
             pair.first == counter ? 
               std::make_tuple(pair.second, index) : 
               std::make_tuple(nullptr, SIZE_MAX);
    }
    
    size_t context::compute_realm_token(const size_t &index) const {
      //return type_mult * realm_token + counter_max * realms[index].first + index;
      ASSERT(index < realms.size());
      ASSERT(realms[index].first < counter_max);
      return (size_t(realm_token) << (SIZE_WIDTH - type_bits_count)) | (realms[index].first << (SIZE_WIDTH - type_bits_count - counter_bits_count)) | index;
    }
    
    size_t context::compute_army_token(const size_t &index) const {
      ASSERT(index < armies.size());
      ASSERT(armies[index].first < counter_max);
      return (size_t(army_token) << (SIZE_WIDTH - type_bits_count)) | (armies[index].first << (SIZE_WIDTH - type_bits_count - counter_bits_count)) | index;
    }
    
    size_t context::compute_hero_troop_token(const size_t &index) const {
      ASSERT(index < hero_troops.size());
      ASSERT(hero_troops[index].first < counter_max);
      return (size_t(hero_troop_token) << (SIZE_WIDTH - type_bits_count)) | (hero_troops[index].first << (SIZE_WIDTH - type_bits_count - counter_bits_count)) | index;
    }
    
    size_t context::compute_war_token(const size_t &index) const {
      ASSERT(index < wars.size());
      ASSERT(wars[index].first < counter_max);
      return (size_t(war_token) << (SIZE_WIDTH - type_bits_count)) | (wars[index].first << (SIZE_WIDTH - type_bits_count - counter_bits_count)) | index;
    }
    
    size_t context::compute_troop_token(const size_t &index) const {
      ASSERT(index < troops.size());
      ASSERT(troops[index].first < counter_max);
      return (size_t(troop_token) << (SIZE_WIDTH - type_bits_count)) | (troops[index].first << (SIZE_WIDTH - type_bits_count - counter_bits_count)) | index;
    }
    
//     const size_t context::realms_max_count;
//     const size_t context::armies_max_count;
//     const size_t context::hero_troops_max_count; 
//     const size_t context::wars_max_count;
//     const size_t context::troops_max_count;
    const size_t context::counter_max;
//     const size_t context::maximum_type_count;
//     const size_t context::type_mult;
    const size_t context::invalid_token;
//     const size_t context::maximum_value;
//     const uint32_t context::id_struct_map[] = {
//       UINT32_MAX, // tile
//       UINT32_MAX, // province
// //       UINT32_MAX,
//       static_cast<uint32_t>(id_struct::building_type),
//       static_cast<uint32_t>(id_struct::holding_type),
//       static_cast<uint32_t>(id_struct::city_type),
//       static_cast<uint32_t>(id_struct::trait),
//       static_cast<uint32_t>(id_struct::modificator),
//       static_cast<uint32_t>(id_struct::troop_type),
//       static_cast<uint32_t>(id_struct::decision),
//       static_cast<uint32_t>(id_struct::interaction),
//       static_cast<uint32_t>(id_struct::religion_group),
//       static_cast<uint32_t>(id_struct::religion),
//       static_cast<uint32_t>(id_struct::culture_group),
//       static_cast<uint32_t>(id_struct::culture),
//       static_cast<uint32_t>(id_struct::law),
//       static_cast<uint32_t>(id_struct::event),
//       static_cast<uint32_t>(id_struct::titulus),
//       static_cast<uint32_t>(id_struct::casus_belli),
//       UINT32_MAX,
//       UINT32_MAX,
//       UINT32_MAX,
//       UINT32_MAX,
//       UINT32_MAX,
//       UINT32_MAX,
//       UINT32_MAX,
//       UINT32_MAX
//     };
    
//     static_assert(sizeof(context::id_struct_map) / sizeof(context::id_struct_map[0]) == static_cast<uint32_t>(structure::count));
  }
}
