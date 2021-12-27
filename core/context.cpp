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
    
//     void context::set_tile(const uint32_t &index, const tile &tile_data) {
//       ASSERT(index < core::map::hex_count_d(core::map::detail_level));
//       tile_array[index] = tile_data;
//     }
//     
//     tile context::get_tile(const uint32_t &index) const {
//       ASSERT(index < core::map::hex_count_d(core::map::detail_level));
//       return tile_array[index];
//     }
//     
//     tile* context::get_tile_ptr(const uint32_t &index) {
//       ASSERT(index < core::map::hex_count_d(core::map::detail_level));
//       return &tile_array[index];
//     }
    
    dynasty* context::create_dynasty() {
      auto ptr = dynasties_pool.create();
      dynasties.push_back(ptr);
      return ptr;
    }
    
    // как добавить в плейабле? при добавлении титула?
    // всех персонажей все таки нужно пихнуть в один массив, 
    // при создании персонажей все связи настраиваются с помощью индексов
    character* context::create_character(const bool male, const bool dead) {
      auto ptr = characters_pool.create(male, dead);
      characters.push_back(ptr);
      if (dead) dead_characters.push_back(ptr);
      else living_characters.push_back(ptr);
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
      if (c->is_dead()) destroy_dead_character(c);
      else throw std::runtime_error("Trying to delete living character");
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
    
//     character* context::get_character(const size_t &index) const {
//       ASSERT(index < characters.size());
//       return characters[index];
//     }
    
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
//       std::sort(characters.begin(), characters.end(), [] (const character* first, const character* second) -> bool {
//         const uint32_t first_attribs = uint32_t(!first->is_dead()) + uint32_t(first->is_ai_playable());
//         const uint32_t second_attribs = uint32_t(!second->is_dead()) + uint32_t(second->is_ai_playable());
//         return first_attribs < second_attribs;
//       });
      std::sort(living_playable_characters.begin(), living_playable_characters.end(), [] (const character* first, const character* second) -> bool {
        return first->get_age() < second->get_age();
      });
    }
    
//     size_t context::first_not_dead_character() const {
//       for (size_t i = 0; i < characters.size(); ++i) {
//         if (!characters[i]->is_dead()) return i;
//         ASSERT(!characters[i]->is_ai_playable());
//       }
//       
//       throw std::runtime_error("All characters are dead");
//     }
//     
//     size_t context::first_playable_character() const {
//       for (size_t i = 0; i < characters.size(); ++i) {
//         if (characters[i]->is_ai_playable()) return i;
//       }
//       
//       throw std::runtime_error("All characters are unplayable");
//     }

    size_t context::dead_characters_count() const { return dead_characters.size(); }
    size_t context::living_characters_count() const { return living_characters.size(); }
    size_t context::living_playable_characters_count() const { return living_playable_characters.size(); }
    character* context::get_character(const size_t &index) const {
      if (index >= characters_count()) return nullptr;
      return characters[index];
    }
    
    character* context::get_dead_character(const size_t &index) const {
      if (index >= dead_characters_count()) return nullptr;
      return dead_characters[index];
    }
    
    character* context::get_living_character(const size_t &index) const {
      if (index >= living_characters_count()) return nullptr;
      return living_characters[index];
    }
    
    character* context::get_living_playable_character(const size_t &index) const {
      if (index >= living_playable_characters_count()) return nullptr;
      return living_playable_characters[index];
    }
    
    // нужно ли делать удаление живых персонажей? не думаю что удаление живых персонажей хорошая идея
    void context::destroy_dead_character(character* c) {
      assert(c->is_dead());
      for (size_t i = 0; i < dead_characters.size(); ++i) {
        if (dead_characters[i] != c) continue;
        characters_pool.destroy(dead_characters[i]);
        std::swap(dead_characters[i], dead_characters.back());
        dead_characters.pop_back();
        break;
      }
    }
    
    template <typename T>
    static void remove_from_vector(std::vector<T> &array, T obj) {
      for (size_t i = 0; i < array.size(); ++i) {
        if (array[i] != obj) continue;
        std::swap(array[i], array.back());
        array.pop_back();
        break;
      }
    }
    
    void context::make_dead(character* c) {
      remove_from_vector(living_characters, c);
      remove_from_vector(living_playable_characters, c);
      dead_characters.push_back(c);
      assert(c->is_dead());
    }
    
    void context::make_not_playable(character* c) {
      assert(!c->is_dead());
      remove_from_vector(living_playable_characters, c);
    }
    
    void context::make_playable(character* c) {
      assert(!c->is_ai_playable());
      assert(!c->is_dead());
      living_playable_characters.push_back(c);
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

#define MAX_VALUE 1000000.0f
    uint32_t cast_ray_reck(const context* ctx, const map* m, const utils::ray &ray, const uint32_t &tri_index, float &distance) {
      const bool ret = m->intersect_container(tri_index, ray);
      if (!ret) return UINT32_MAX;
      
      const map::triangle &tri = m->triangles[tri_index];
      
      const uint32_t level = tri.current_level;
      if (level == map::detail_level) {
        uint32_t final_tile_index = UINT32_MAX;
        float final_tile_dist = MAX_VALUE;
        for (size_t i = 0; i < 4; ++i) {
          const uint32_t tile_index = tri.next_level[i];
          // тут нужно проверить дальность до тайла + проверить пересечение со стенками
          // нужно ли чекать ближайший треугольник? не уверен что это необходимо
          
          const auto tile = ctx->get_entity<core::tile>(tile_index);
          
          const uint32_t p_count = tile->neighbors_count();
//           const uint32_t point_a_index = tile->center;
          const float height = tile->height;
//           const uint32_t height_layer = render::compute_height_layer(height);
//           const float final_height = render::layer_height * height_layer;
//           const float computed_height = final_height * render::render_tile_height;
          const float computed_height = height;
          
//           glm::vec4 center = m->get_point(point_a_index);
//           glm::vec4 center_height = center + glm::normalize(glm::vec4(glm::vec3(center), 0.0f)) * (computed_height);
          glm::vec4 local_points[6];
          glm::vec4 local_points_height[6];
          for (uint32_t j = 0; j < p_count; ++j) {
            const uint32_t point1_index = tile->points[j];
            const glm::vec4 point = m->get_point(point1_index);
            const glm::vec4 point_normal = glm::vec4(glm::vec3(point) / map::world_radius, 0.0f);
            const glm::vec4 point_height = point + point_normal * (computed_height);
            local_points[j] = point;
            local_points_height[j] = point_height;
          }
          
          // проверяем теугольники тайла, возможно имеет смысл брать треугольники как при рендеринге - меньше треугольников
          static const uint32_t indices_hex[] = {0, 1, 5, 2, 4, 3};
          static const uint32_t indices_pen[] = {0, 1, 4, 2, 3};
          uint32_t index1 = 0;
          uint32_t index2 = 1;
          uint32_t index3 = 2;
          for (uint32_t j = 0; j < p_count-2; ++j) {
            const uint32_t final_index1 = p_count == 6 ? indices_hex[index1] : indices_pen[index1];
            const uint32_t final_index2 = p_count == 6 ? indices_hex[index2] : indices_pen[index2];
            const uint32_t final_index3 = p_count == 6 ? indices_hex[index3] : indices_pen[index3];
            
            const auto point1 = local_points_height[final_index1];
            const auto point2 = local_points_height[final_index2];
            const auto point3 = local_points_height[final_index3];
            
            float dist = MAX_VALUE;
            const bool ret = m->intersect_tri(point1, point2, point3, ray, dist);
            if (ret && dist < final_tile_dist) {
              final_tile_index = tile_index;
              final_tile_dist = dist;
              break;
            }
            
            index1 = index2;
            index2 = index3;
            index3 = (index3+1)%p_count;
          }
          
//           for (uint32_t j = 0; j < p_count; ++j) {
//             const uint32_t b_index = j;
//             const uint32_t c_index = (j+1)%p_count;
//             
//             ASSERT(c_index < p_count);
//             
//             float dist = MAX_VALUE;
// //               const bool ret = intersect_tri(get_point(point_a_index), get_point(point_b_index), get_point(point_c_index), ray, dist);
//             const bool ret = m->intersect_tri(center_height, local_points_height[b_index], local_points_height[c_index], ray, dist);
//             if (ret && dist < final_tile_dist) {
//               final_tile_index = tile_index;
//               final_tile_dist = dist;
//               //break;
//             }
//           }
          
          if (final_tile_index != UINT32_MAX) {
            distance = final_tile_dist;
            return final_tile_index;
          }

          // почему то по приоритету берутся стенки
          for (uint32_t j = 0; j < p_count; ++j) {
            const uint32_t b_index = j;
            const uint32_t c_index = (j+1)%p_count;
            const glm::vec4 point1 = local_points[b_index];
            const glm::vec4 point2 = local_points[c_index];
            const glm::vec4 point3 = local_points_height[b_index];
            const glm::vec4 point4 = local_points_height[c_index];
            // две стенки
            const std::tuple<glm::vec4, glm::vec4, glm::vec4> wall_triangle[] = {
              std::tie(point1, point2, point3),
              std::tie(point4, point3, point2)
            };
            
            float dist1 = MAX_VALUE;
            float dist2 = MAX_VALUE;
            const bool ret1 = m->intersect_tri(std::get<0>(wall_triangle[0]), std::get<1>(wall_triangle[0]), std::get<2>(wall_triangle[0]), ray, dist1);
            const bool ret2 = m->intersect_tri(std::get<0>(wall_triangle[1]), std::get<1>(wall_triangle[1]), std::get<2>(wall_triangle[1]), ray, dist2);
            
            if (ret1 && dist1 < final_tile_dist) {
              final_tile_dist = dist1;
              final_tile_index = tile_index;
            }
            
            if (ret2 && dist2 < final_tile_dist) {
              final_tile_dist = dist2;
              final_tile_index = tile_index;
            }
          }
        }
        
        distance = final_tile_dist;
        return final_tile_index;
      }
      
      float global_dist = MAX_VALUE;
      uint32_t global_index = UINT32_MAX;
      for (size_t i = 0; i < 4; ++i) {
        const uint32_t tri_index = tri.next_level[i];
        
        float dist = MAX_VALUE;
        const uint32_t index = cast_ray_reck(ctx, m, ray, tri_index, dist); // может я тут что то не так делаю
        
        if (index == UINT32_MAX) continue;
        
        if (dist < global_dist) {
          global_dist = dist;
          global_index = index;
        }
      }
      
      distance = global_dist;
      return global_index;
    }


    uint32_t context::cast_ray(const core::map* map, const utils::ray &ray, float &ray_dist) {
      size_t current_detail_level = 0;
      
      float dist = MAX_VALUE;
      uint32_t final_tile = UINT32_MAX;
      for (size_t i = 0; i < 20*power4(current_detail_level); ++i) {
        float local_dist = MAX_VALUE;
        const uint32_t tile_index = cast_ray_reck(this, map, ray, i, local_dist);
        
        if (local_dist < dist) {
          dist = local_dist;
          final_tile = tile_index;
        }
      }
      
      ray_dist = dist;
      return final_tile;
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
