#include "core_context.h"

namespace devils_engine {
  namespace core {
    context::context() {
      memset(containers.data(), 0, sizeof(container) * containers.size());
    }
    
    context::~context() {
      for (size_t i = 0; i < static_cast<size_t>(structure::static_types_count); ++i) {
        destroy_container(static_cast<structure>(i));
      }
      
      for (auto d : dynasties) {
        dynasties_pool.destroy(d);
      }
      
      for (auto c : characters) {
        characters_pool.destroy(c);
      }
      
      for (auto a : armies) {
        armies_pool.destroy(a);
      }
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
    
    army* context::create_army() {
      auto ptr = armies_pool.create();
      //armies.push_back(ptr); // не думаю что это нужно, хотя армия может быть и без полководца
      if (armies.size() <= ptr->army_gpu_slot) armies.resize(ptr->army_gpu_slot+1, nullptr);
      armies[ptr->army_gpu_slot] = ptr;
      return ptr;
    }
    
    faction* context::create_faction() {
      return factions_pool.create();
    }
    
    hero_troop* context::create_hero_troop() {
      return hero_troops_pool.create();
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
    
    void context::destroy(army* a) {
//       for (size_t i = 0; i < armies.size(); ++i) {
//         if (armies[i] == a) {
//           armies_pool.destroy(armies[i]);
//           std::swap(armies.back(), armies[i]);
//           armies.pop_back();
//           break;
//         }
//       }
      
      armies[a->army_gpu_slot] = nullptr;
      armies_pool.destroy(a);
    }
    
    void context::destroy(faction* f) {
      factions_pool.destroy(f);
    }
    
    void context::destroy(hero_troop* h) {
      hero_troops_pool.destroy(h);
    }
    
    dynasty* context::get_dynasty(const size_t &index) const {
      ASSERT(index < dynasties.size());
      return dynasties[index];
    }
    
    character* context::get_character(const size_t &index) const {
      ASSERT(index < characters.size());
      return characters[index];
    }
    
    army* context::get_army(const size_t &index) const {
      ASSERT(index < armies.size());
      return armies[index];
    }
    
    hero_troop* context::get_hero_troop(const size_t &index) const {
      ASSERT(index < armies.size());
      return nullptr;
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
        case structure::religion_group: destroy_container<religion_group>(); break;
        case structure::religion:       destroy_container<religion>();       break;
        case structure::culture:        destroy_container<culture>();        break;
        case structure::law:            destroy_container<law>();            break;
        case structure::event:          destroy_container<event>();          break;
        case structure::titulus:        destroy_container<titulus>();        break;
        default: throw std::runtime_error("Bad structure type");
      }
    }
  }
}
