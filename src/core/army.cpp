#include "army.h"

#include "utils/globals.h"
#include "utils/systems.h"
// #include "bin/map.h"
#include "core/context.h"
#include "bin/tiles_funcs.h"

namespace devils_engine {
  namespace core {
    const structure army::s_type;
    const size_t army::modificators_container_size;
    const size_t army::events_container_size;
    const size_t army::flags_container_size;
    const size_t army::max_troops_count;
    army::army() : 
      object_token(SIZE_MAX),
      general(nullptr),
      troops_count(0), 
      tile_index(UINT32_MAX), 
//       map_img{GPU_UINT_MAX}, 
      state(state::stationed),
      origin(nullptr),
      target_city(nullptr),
      current_time(0),
      current_pos(0.0f),
      advancing(false)
    {
//       auto map = global::get<systems::map_t>()->map;
//       std::unique_lock<std::mutex> lock(map->mutex);
//       army_gpu_slot = map->allocate_army_data();
      
      modificators.reserve(modificators_container_size);
      events.reserve(events_container_size);
      flags.reserve(flags_container_size);
    }
    
    army::~army() {
//       auto map = global::get<systems::map_t>()->map;
//       std::unique_lock<std::mutex> lock(map->mutex);
//       map->release_army_data(army_gpu_slot);
      object_token = SIZE_MAX;
    }
    
    void army::set_pos(const glm::vec3 &pos) {
//       auto map = global::get<systems::map_t>()->map;
//       std::unique_lock<std::mutex> lock(map->mutex); // по идее это нужно
//       map->set_army_pos(army_gpu_slot, pos);
      this->pos = pos;
    }
    
    glm::vec3 army::get_pos() const {
//       auto map = global::get<systems::map_t>()->map;
//       return map->get_army_pos(army_gpu_slot);
      return pos;
    }
    
    void army::set_img(const render::image_t &img) {
//       auto map = global::get<systems::map_t>()->map;
//       std::unique_lock<std::mutex> lock(map->mutex); // по идее это нужно
//       map->set_army_image(army_gpu_slot, img);
      image = img;
    }
    
    render::image_t army::get_img() const {
//       auto map = global::get<systems::map_t>()->map;
//       return map->get_army_image(army_gpu_slot);
      return image;
    }
    
    // нужно тут наверное возвращать индекс части пути до которого мы можем дойти
    size_t army::can_advance() const {
      if (!has_path()) return 0;
      
      //const uint32_t speed = current_stats[core::army_stats::speed].uval;
      const uint32_t speed = 500;
      return ai::unit_advance(this, 0, speed, [this] (const ai::path_finding_data*, const uint32_t &cur_tile_index, const uint32_t &tile_index) -> bool {
        auto ctx = global::get<systems::map_t>()->core_context.get();
//         const auto cur_tile = ctx->get_entity<core::tile>(cur_tile_index);
        const auto tile = ctx->get_entity<core::tile>(tile_index);
        (void)cur_tile_index;
        
        if (tile->army_token != SIZE_MAX) {
          assert(tile->city == UINT32_MAX);
          auto army = ctx->get_army(tile->army_token);
          const utils::handle<core::army> army_h(army, tile->army_token);
          const auto rel = this->get_relation(army_h);
          if (rel == relationship::enemy_unit) return false;
          //if (rel == relationship::ally_unit) return false; // наверное сквозь армии союзника можно пройти
          if (rel == relationship::neutral_unit) return false;
          if (army_h->is_in_pending_state()) return false;
          
          return true;
        }
        
        if (tile->city != UINT32_MAX) {
          assert(tile->army_token == SIZE_MAX);
          auto city = ctx->get_entity<core::city>(tile->city);
          const auto rel = this->get_relation(city);
          if (rel == relationship::enemy_unit) return false;
          //if (rel == relationship::ally_unit) return false; // наверное сквозь города союзника можно пройти
          if (rel == relationship::neutral_unit) return false;
          
          
          return true;
        }
        
        return true;
      });
    }
    
    bool army::can_full_advance() const {
      return path_size != 0 && can_advance() == path_size;
    }
    
    static void recompute_army_pos(core::army* army) {
      auto ctx = global::get<systems::map_t>()->core_context.get();
      auto map = global::get<systems::map_t>()->map.get();
      
      const auto tile_data = ctx->get_entity<core::tile>(army->tile_index);
      const uint32_t point_index = tile_data->center;
      const glm::vec4 center = map->get_point(point_index);
      const glm::vec4 normal = glm::normalize(glm::vec4(glm::vec3(center), 0.0f));
      const float height = tile_data->height;
      
      const glm::vec4 final_point = center + normal * (height + 1.0f);
      army->set_pos(glm::vec3(final_point));
    }
    
    void army::advance() { advancing = true; current_time = 0; }
    // нужно поставить армию в центр тайла
    void army::stop() { advancing = false; current_time = 0; }
    
    void army::find_path(const uint32_t end_tile_index) {
      if (end_tile_index >= core::map::hex_count_d(core::map::detail_level)) throw std::runtime_error("Could not find path to tile index " + std::to_string(end_tile_index));
      
      stop();
      auto path_managment = global::get<systems::core_t>()->path_managment;
      path_managment->find_path(this, tile_index, end_tile_index);
    }
    
    void army::clear_path() {
      stop();
      auto path_managment = global::get<systems::core_t>()->path_managment;
      path_task = 0;
      if (has_path()) path_managment->free_path(path);
      path = nullptr;
      path_size = 0;
      current_path = 0;
      start_tile = UINT32_MAX;
      end_tile = UINT32_MAX;
    }
    
    // я так понимаю проще если я буду пересчитывать конечную точку каждый раз когда юнит попадает на новый тайл (ну или пытается покинуть тайл)
    // конечная точка для армии - это не конечная точка для игрока, мне нужно определить сколько юнит может двигаться 
    // не нужно пересчитывать выносливость и ставить все обратно
    void army::update(const size_t &time) {
      if (!is_in_movement_state()) return;
      if (!advancing) return;
      
      current_time += time;
      
      if (finding_path()) {
        if (current_time > ONE_SECOND*10) throw std::runtime_error("Bad path find. Start tile " + std::to_string(start_tile) + " end tile " + std::to_string(end_tile));
        return;
      }
      
      if (!has_path()) return;
      
      auto ctx = global::get<systems::map_t>()->core_context.get();
      const uint32_t current_container = current_path / ai::path_container::container_size;
      const uint32_t current_index     = current_path % ai::path_container::container_size;
      //const float current_cost = ai::advance_container(this->path, current_container)->tile_path[current_index].cost;
      const uint32_t tile_index = ai::advance_container(this->path, current_container)->tile_path[current_index].tile;
      auto prev_tile = ctx->get_entity<core::tile>(tile_index);
      
      if (prev_tile->movement_token != object_token) {
        if (prev_tile->movement_token != SIZE_MAX) return; // ждем
        prev_tile->movement_token = object_token;
        current_time = 0;
      }
      
      const size_t advance_index = can_advance();
      if (current_time >= HALF_SECOND && current_path < advance_index) {
        // проверяем можем ли мы перейти на следующий тайл
        // почему переход может быть недоступен? кажется это все причины по которым такое может произойти
        // 1. закончились ходы
        // 2. изменилась политическая ситуация и мы больше не можем пройти по маршруту либо путь заблокирован другим юнитом
        // 3. на героя напали и он не может дальше пройти
        
        //const float next_tile_cost = ai::advance_container(this->path, container)->tile_path[index].cost - current_cost;
        
        //if (current_time < HALF_SECOND) break;
        
        const size_t next_path_index = current_path + 1;
        const uint32_t container = next_path_index / ai::path_container::container_size;
        const uint32_t index     = next_path_index % ai::path_container::container_size;
        const uint32_t current_tile_index = ai::advance_container(this->path, container)->tile_path[index].tile;
        auto tile = ctx->get_entity<core::tile>(current_tile_index);
        
        if (tile->army_token != SIZE_MAX) {
          assert(tile->city == UINT32_MAX);
          auto army = ctx->get_army(tile->army_token);
          const utils::handle<core::army> army_h(army, tile->army_token);
          const auto rel = get_relation(army_h);
          if (rel == relationship::enemy_unit) {
            this->state = state::pending_battle;
            this->target_army = army_h;
            stop();
            place_army(tile_index); // расположить нужно на текущий тайл
            return;
          }
        }
        
        if (tile->city != UINT32_MAX) {
          assert(tile->army_token == SIZE_MAX);
          const auto city = ctx->get_entity<core::city>(tile->city);
          const auto rel = get_relation(city);
          if (rel == relationship::enemy_unit) {
            this->state = state::pending_siege;
            this->target_city = city;
            stop();
            place_army(tile_index);
            return;
          }
        }
        
        if (tile->movement_token != SIZE_MAX) { return; }
                
        current_time = 0;
        current_path = next_path_index;
        tile->movement_token = object_token;
        prev_tile->movement_token = SIZE_MAX;
        
        //const bool ret = map->tile_objects_index_comp_swap(current_tile_index, 4, tmp, army_gpu_slot);
        //if (!ret) throw std::runtime_error("Could not place army on tile " + std::to_string(current_tile_index));
        //map->set_tile_objects_index(tile_index, 4, UINT32_MAX);
        this->tile_index = current_tile_index;
        
        recompute_army_pos(this);
      }
      
      if (current_path == advance_index) {
        const uint32_t current_container = current_path / ai::path_container::container_size;
        const uint32_t current_index     = current_path % ai::path_container::container_size;
        const uint32_t tile_index = ai::advance_container(this->path, current_container)->tile_path[current_index].tile;
//         auto tile = ctx->get_entity<core::tile>(tile_index);
        
        place_army(tile_index);
        
//         assert(tile->movement_token == object_token);
//         tile->movement_token = SIZE_MAX;
//         tile->army_token = object_token;
        
        // нужно еще проверить следующий кусочек пути: если там есть что то то возможно нужно ставить в пендинг стейт
        // или только последний?
//         if (current_path < path_size-1) {
//           const uint32_t current_container = (current_path+1) / ai::path_container::container_size;
//           const uint32_t current_index     = (current_path+1) % ai::path_container::container_size;
//           const uint32_t next_tile_index   = ai::advance_container(this->path, current_container)->tile_path[current_index].tile;
//           const auto next_tile = ctx->get_entity<core::tile>(next_tile_index);
//           
//           
//         }
      }
    }
    
    // возможно имеет смысл домашним регионом делать провинцию на один слой тайлов меньше
    bool army::is_home() const {
      if (tile_index == UINT32_MAX) return true;
      const auto prov = get_tile_province(tile_index);
      return origin == prov;
    }
    
    troop* army::next_troop(const troop* current) const {
      ASSERT(troops.valid());
      return utils::ring::list_next<utils::list_type::army_troops>(current, troops.get());
    }
    
    static army::relationship get_relation_characters(const core::character* cur_leader, const core::character* leader) {
      if (cur_leader == leader) return army::relationship::controlled_unit;
      
      bool enemy = false;
      bool ally = false;
      for (const auto &pair : cur_leader->diplomacy) {
        if (enemy) break;
        
        if (pair.second.types.get(core::diplomacy::war_attacker)) {
          ASSERT(pair.second.war.valid());
          if (pair.second.war->war_opener == leader) ally = true;
          if (pair.second.war->target_character == leader) { enemy = true; break; }
          
          for (const auto &character : pair.second.war->attackers) {
            ally = ally || character == leader;
          }
          
          for (const auto &character : pair.second.war->defenders) {
            if (character == leader) { enemy = true; break; }
          }
        }
        
        if (pair.second.types.get(core::diplomacy::war_defender)) {
          ASSERT(pair.second.war.valid());
          if (pair.second.war->war_opener == leader) { enemy = true; break; }
          if (pair.second.war->target_character == leader) ally = true;
          
          for (const auto &character : pair.second.war->attackers) {
            if (character == leader) { enemy = true; break; }
          }
          
          for (const auto &character : pair.second.war->defenders) {
            ally = ally || character == leader;
          }
        }
      }
      
      if (enemy) return army::relationship::enemy_unit;
      if (ally) return army::relationship::ally_unit;
      
      return army::relationship::neutral_unit;
    }
    
    // может ли союзник по одной войне быть противником в другой? вообще наверное потенциально может
    army::relationship army::get_relation(const utils::handle<core::army> &another) const {
      auto cur_leader = owner->leader;
      auto owner = another->owner;
      auto leader = owner->leader;
      return get_relation_characters(cur_leader, leader);
    }
    
    army::relationship army::get_relation(const utils::handle<core::hero_troop> &another) const {
      // тут пока что ничего не ясно
//       auto cur_leader = owner->leader;
//       auto owner = another->owner;
//       auto leader = owner->leader;
//       if (cur_leader == leader) return army::relationship::controlled_unit;
//       // этого не достаточно
//       //const auto itr = cur_leader->diplomacy.find(leader);
//       
//       bool enemy = false;
//       bool ally = false;
//       for (const auto &pair : cur_leader->diplomacy) {
//         if (enemy) break;
//         
//         if (pair.second.types.get(core::diplomacy::war_attacker)) {
//           ASSERT(pair.second.war.valid());
//           if (pair.second.war->war_opener == leader) ally = true;
//           if (pair.second.war->target_character == leader) { enemy = true; break; }
//           
//           for (const auto &character : pair.second.war->attackers) {
//             ally = ally || character == leader;
//           }
//           
//           for (const auto &character : pair.second.war->defenders) {
//             if (character == leader) { enemy = true; break; }
//           }
//         }
//         
//         if (pair.second.types.get(core::diplomacy::war_defender)) {
//           ASSERT(pair.second.war.valid());
//           if (pair.second.war->war_opener == leader) { enemy = true; break; }
//           if (pair.second.war->target_character == leader) ally = true;
//           
//           for (const auto &character : pair.second.war->attackers) {
//             if (character == leader) { enemy = true; break; }
//           }
//           
//           for (const auto &character : pair.second.war->defenders) {
//             ally = ally || character == leader;
//           }
//         }
//       }
//       
//       if (enemy) return army::relationship::enemy_unit;
//       if (ally) return army::relationship::ally_unit;
//       return army::relationship::neutral_unit;
      throw std::runtime_error("not implemented yet");
      return army::relationship::neutral_unit;
    }
    
    army::relationship army::get_relation(const core::city* another) const {
      auto cur_leader = owner->leader;
      auto owner = another->title->owner;
      auto leader = owner->leader;
      return get_relation_characters(cur_leader, leader);
    }
    
    bool army::can_be_raized() const {
      return state == core::army::state::stationed;
    }
    
    bool army::can_be_returned() const {
      if (
//         state == core::army::state::stationed || 
//         state == core::army::state::pending_battle || 
//         state == core::army::state::pending_siege ||
//         state == core::army::state::besieging ||
//         state == core::army::state::fleeing
        !is_in_movement_state()
      ) return false;
      
      auto ctx = global::get<systems::map_t>()->core_context.get();
      auto tile = ctx->get_entity<core::tile>(tile_index);
      for (uint32_t i = 0; i < tile->neighbors_count(); ++i) {
        auto n_tile = ctx->get_entity<core::tile>(tile->neighbors[i]);
        if (n_tile->army_token == SIZE_MAX) continue;
        auto n_army = ctx->get_army(n_tile->army_token);
        assert(n_army != nullptr);
        if (n_army->owner->leader != owner->leader) return false;
      }
      
      return true;
    }
    
    bool army::is_in_pending_state() const { return state == state::pending_battle || state == state::pending_siege; }
    bool army::is_in_movement_state() const { return state == state::on_land || state == state::sailing; }
    
    void army::place_army(const uint32_t &tile_index) {
      auto ctx = global::get<systems::map_t>()->core_context.get();
      auto tile = ctx->get_entity<core::tile>(tile_index);
      if (tile->movement_token != object_token) throw std::runtime_error("Army must reach tile " + std::to_string(tile_index) + ", currently on tile " + std::to_string(this->tile_index));
      if (tile->army_token != SIZE_MAX) throw std::runtime_error("Another army already placed on tile " + std::to_string(tile_index));
      
      tile->army_token = tile->movement_token;
      tile->movement_token = SIZE_MAX;
      this->tile_index = tile_index;
    }
    
    void army::raize_army(const uint32_t &tile_index) {
      if (state != core::army::state::stationed) throw std::runtime_error("Army is already has left the city");
      
      auto ctx = global::get<systems::map_t>()->core_context.get();
      auto n_tile = ctx->get_entity<core::tile>(tile_index);
      if (n_tile->army_token != SIZE_MAX) throw std::runtime_error("Trying to place army on tile with another army");
      this->tile_index = tile_index;
      this->state = core::army::state::on_land;
      n_tile->army_token = this->object_token; // имеет смысл на хендл заменить
    }
    
    void army::return_army() {
      if (
        state == core::army::state::stationed || 
        state == core::army::state::pending_battle || 
        state == core::army::state::pending_siege ||
        state == core::army::state::besieging ||
        state == core::army::state::fleeing
      ) throw std::runtime_error("Could not return this army when army stationed or in a pending state");
      
      // так же мы не можем вернуть армию если рядом стоит вражеская армия
      // какая армия является армией противника?
      // это армия главного противника войны + армии всех его союзников + возможно какие то армии дополнительные
      // я бы на самом деле пока что оставил невозможность вернуть армию если вообще хоть какая то чужая рядом
      // + наверное вернуть армию не на своей земле какие то штрафы дает
      
      auto ctx = global::get<systems::map_t>()->core_context.get();
      auto tile = ctx->get_entity<core::tile>(tile_index);
      for (uint32_t i = 0; i < tile->neighbors_count(); ++i) {
        auto n_tile = ctx->get_entity<core::tile>(tile->neighbors[i]);
        if (n_tile->army_token == SIZE_MAX) continue;
        auto n_army = ctx->get_army(n_tile->army_token);
        assert(n_army != nullptr);
        
        // как проверить это другая армия? во первых эта армия может быть армией вассала
        // вассал может быть недружественным
        // ладно любые армии других фракций
        
        // армия может относится к другому реалму которым владеет тот же владелец
        if (n_army->owner->leader != owner->leader) throw std::runtime_error("Could not return an army that neighboring other army");
      }
      
      assert(tile->army_token == object_token);
      tile->army_token = SIZE_MAX; // имеет смысл на хендл заменить
      state = core::army::state::stationed;
      tile_index = UINT32_MAX;
    }
    
    OUTPUT_CHARACTER_TYPE army::get_commander() const { return general; }
    OUTPUT_REALM_TYPE army::get_affiliation() const { return owner; }
    OUTPUT_PROVINCE_TYPE army::get_location() const { return get_tile_province(tile_index); }
    OUTPUT_PROVINCE_TYPE army::get_origin() const { return origin; }
  }
}
