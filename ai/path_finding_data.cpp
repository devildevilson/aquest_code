#include "path_finding_data.h"

#include "utils/globals.h"
#include "utils/systems.h"
#include "bin/map.h"
#include "core/army.h"
#include "core/hero_troop.h"

namespace devils_engine {
  namespace ai {
    path_finding_data::path_finding_data() : 
      path(nullptr), 
      path_size(0), 
      current_path(0), 
      start_tile(UINT32_MAX), 
      end_tile(UINT32_MAX), 
      path_task(0) 
    {}
    
    bool path_finding_data::finding_path() const { return path_task != 0; }
    bool path_finding_data::has_path() const { return path != nullptr && path != reinterpret_cast<ai::path_container*>(SIZE_MAX); }
    bool path_finding_data::path_not_found() const { return path == reinterpret_cast<ai::path_container*>(SIZE_MAX); }
    
    size_t unit_advance(const path_finding_data* unit, const size_t &start, const size_t &speed) {
      if (start >= unit->path_size) return 0;
      if (!unit->has_path()) return 0;
      
      auto map = global::get<systems::map_t>()->map;
      const uint32_t current_container = start / ai::path_container::container_size;
      const uint32_t current_index     = start % ai::path_container::container_size;
      const float current_cost = ai::advance_container(unit->path, current_container)->tile_path[current_index].cost;
      size_t next_path_index = start+1;
      for (; next_path_index < unit->path_size; ++next_path_index) {
        const size_t final_path_index = next_path_index;
        const uint32_t container = final_path_index / ai::path_container::container_size;
        const uint32_t index     = final_path_index % ai::path_container::container_size;
        const float next_tile_cost = ai::advance_container(unit->path, container)->tile_path[index].cost - current_cost;
        //if (next_tile_cost > speed) return final_path_index; // мы можем перейти ДО тайла i
        if (next_tile_cost > speed) break; // мы можем перейти ДО тайла i
        
//         const uint32_t next_tile_index = ai::advance_container(this->path, container)->tile_path[index].tile;
        // тут должны быть проверки можем ли мы перейти на тайл next_tile_index
        // опять же если нет, то возвращаем i
//         if (final_path_index == path_size-1) {
//           auto map = global::get<systems::map_t>()->map;
//           const uint32_t another_army_index = map->get_tile_objects_index(next_tile_index, 4);
//           if (another_army_index != UINT32_MAX) {}
//         }
      }
      
      if (next_path_index == start+1) return 0;
      
//       PRINT("=================== curernt_path =================================")
//       for (size_t i = 0; i < unit->path_size; ++i) {
//         const uint32_t container = i / ai::path_container::container_size;
//         const uint32_t index     = i % ai::path_container::container_size;
//         const auto next_tile = ai::advance_container(unit->path, container)->tile_path[index];
//         PRINT_VAR("next_tile", next_tile.tile)
//         PRINT_VAR("next_cost", next_tile.cost)
//       }
//       PRINT("=================== curernt_path end =============================")
//       
//       PRINT_VAR("advancing", next_path_index)
      
      //ASSERT(next_path_index == unit->path_size);
      while (next_path_index != start) {
        const uint32_t container = (next_path_index-1) / ai::path_container::container_size;
        const uint32_t index     = (next_path_index-1) % ai::path_container::container_size;
        const uint32_t next_tile_index = ai::advance_container(unit->path, container)->tile_path[index].tile;
        const uint32_t another_army_index = map->get_tile_objects_index(next_tile_index, 4);
        const uint32_t structure_index = map->get_tile_objects_index(next_tile_index, 0);
        const bool has_not_another_army = another_army_index == UINT32_MAX;
        const bool has_not_structure = structure_index == UINT32_MAX;
        if (has_not_another_army && has_not_structure) break;
        --next_path_index;
      }
      
//       PRINT_VAR("speed    ", speed)
//       PRINT_VAR("start    ", start)
//       PRINT_VAR("path_size", unit->path_size)
//       PRINT_VAR("returning", next_path_index)
      
      return next_path_index;
    }
    
    template <>
    size_t maximum_unit_advance(const core::army* unit, const size_t &start) {
      ASSERT(unit != nullptr);
      const uint32_t speed = unit->stats.get(core::army_stats::speed);
      return unit_advance(unit, start, speed);
    }
  }
}
