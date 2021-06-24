#include "army.h"

#include "utils/globals.h"
#include "utils/systems.h"
#include "bin/map.h"

namespace devils_engine {
  namespace core {
    const structure army::s_type;
    const size_t army::modificators_container_size;
    const size_t army::events_container_size;
    const size_t army::flags_container_size;
    const size_t army::max_troops_count;
    army::army() : 
      troops_count(0), 
      tile_index(UINT32_MAX), 
//       map_img{GPU_UINT_MAX}, 
      current_pos(0.0f),
      current_time(0),
      advancing(false)
    {
      auto map = global::get<systems::map_t>()->map;
      std::unique_lock<std::mutex> lock(map->mutex);
      army_gpu_slot = map->allocate_army_data();
      
      modificators.reserve(modificators_container_size);
      events.reserve(events_container_size);
      flags.reserve(flags_container_size);
    }
    
    army::~army() {
      auto map = global::get<systems::map_t>()->map;
      std::unique_lock<std::mutex> lock(map->mutex);
      map->release_army_data(army_gpu_slot);
    }
    
    void army::set_pos(const glm::vec3 &pos) {
      auto map = global::get<systems::map_t>()->map;
      std::unique_lock<std::mutex> lock(map->mutex); // по идее это нужно
      map->set_army_pos(army_gpu_slot, pos);
    }
    
    glm::vec3 army::get_pos() const {
      auto map = global::get<systems::map_t>()->map;
      return map->get_army_pos(army_gpu_slot);
    }
    
    void army::set_img(const render::image_t &img) {
      auto map = global::get<systems::map_t>()->map;
      std::unique_lock<std::mutex> lock(map->mutex); // по идее это нужно
      map->set_army_image(army_gpu_slot, img);
    }
    
    render::image_t army::get_img() const {
      auto map = global::get<systems::map_t>()->map;
      return map->get_army_image(army_gpu_slot);
    }
    
    // нужно тут наверное возвращать индекс части пути до которого мы можем дойти
    size_t army::can_advance() const {
      if (!has_path()) return 0;
      
      //const uint32_t speed = current_stats[core::army_stats::speed].uval;
      const uint32_t speed = 500;
      return ai::unit_advance(this, 0, speed);
    }
    
    bool army::can_full_advance() const {
      return path_size != 0 && can_advance() == path_size;
    }
    
    static void recompute_army_pos(core::army* army) {
      auto map = global::get<systems::map_t>()->map;
      
      const auto tile_data = map->get_tile(army->tile_index);
      const uint32_t point_index = tile_data.tile_indices.x;
      const glm::vec4 center = map->get_point(point_index);
      const glm::vec4 normal = glm::normalize(glm::vec4(glm::vec3(center), 0.0f));
      const float height = map->get_tile_height(army->tile_index);
      const uint32_t height_layer = render::compute_height_layer(height);
      const float final_height = render::layer_height * height_layer;
      
      const glm::vec4 final_point = center + normal * (final_height * render::render_tile_height + 1.0f);
      army->set_pos(glm::vec3(final_point));
    }
    
    void army::advance() { advancing = true; current_time = 0; }
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
    
    void army::update(const size_t &time) {
      if (!advancing) return;
      
      current_time += time;
      
      if (finding_path()) {
        if (current_time > ONE_SECOND*10) throw std::runtime_error("Bad path find. Start tile " + std::to_string(start_tile) + " end tile " + std::to_string(end_tile));
        return;
      }
      
      if (!has_path()) return;
                
      const size_t advance_index = can_advance();
      
      auto map = global::get<systems::map_t>()->map;
      const uint32_t current_container = current_path / ai::path_container::container_size;
      const uint32_t current_index     = current_path % ai::path_container::container_size;
      //const float current_cost = ai::advance_container(this->path, current_container)->tile_path[current_index].cost;
      const uint32_t tile_index = ai::advance_container(this->path, current_container)->tile_path[current_index].tile;
      if (current_time >= HALF_SECOND && current_path < advance_index) {
        // проверяем можем ли мы перейти на следующий тайл
        // почему переход может быть недоступен? кажется это все причины по которым такое может произойти
        // 1. закончились ходы
        // 2. изменилась политическая ситуация и мы больше не можем пройти по маршруту либо путь заблокирован другим юнитом
        // 3. на героя напали и он не может дальше пройти
        
        //const float next_tile_cost = ai::advance_container(this->path, container)->tile_path[index].cost - current_cost;
        
        //if (current_time < HALF_SECOND) break;
                
        current_time = 0;
        ++current_path;
        
        const uint32_t container = current_path / ai::path_container::container_size;
        const uint32_t index     = current_path % ai::path_container::container_size;
        const uint32_t current_tile_index = ai::advance_container(this->path, container)->tile_path[index].tile;
        uint32_t tmp = UINT32_MAX;
        const bool ret = map->tile_objects_index_comp_swap(current_tile_index, 4, tmp, army_gpu_slot);
        if (!ret) throw std::runtime_error("Could not place army on tile " + std::to_string(current_tile_index));
        map->set_tile_objects_index(tile_index, 4, UINT32_MAX);
        this->tile_index = current_tile_index;
        
        recompute_army_pos(this);
      }
    }
  }
}
