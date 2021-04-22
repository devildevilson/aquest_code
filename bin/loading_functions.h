#ifndef LOADING_FUNCTIONS_H
#define LOADING_FUNCTIONS_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <array>
#include "utils/sol.h"

#define MAX_BIOMES_COUNT 0xff

class FastNoise;

namespace dt {
  class thread_pool;
}

namespace devils_engine {
  namespace core {
    struct map;
    class context;
  }
  
  namespace battle {
    struct map;
  }
  
  namespace map {
    namespace generator {
      class container;
    }
  }
  
  namespace utils {
    class progress_container;
    class world_serializator;
    struct random_engine_st;
  }
  
  namespace systems {
    struct core_t;
    struct map_t;
    
    constexpr uint64_t make_64bit(const uint32_t &first, const uint32_t &second) {
      return (uint64_t(first) << 32) | uint64_t(second);
    }
    
    std::array<std::pair<uint32_t, uint32_t>, MAX_BIOMES_COUNT> get_season_biomes_data(const systems::map_t* map_system);
    
    void advance_progress(utils::progress_container* prog, const std::string &str);
    void setup_map_generator(const systems::map_t* map_data);
    
    void find_border_points(core::map* map, const core::context* ctx);
    void generate_tile_connections(core::map* map, dt::thread_pool* pool);
    void validate_and_create_data(systems::map_t* map_systems, utils::progress_container* prog);
    void post_generation_work(systems::map_t* map_systems, systems::core_t* systems, utils::progress_container* prog);
    
    void load_map_data(core::map* map, const utils::world_serializator* world, const bool make_tiles);
    //void loading_world(systems::map_t* map_systems, utils::progress_container* prog, const std::string &world_path);
    void loading_world(systems::map_t* map_systems, utils::progress_container* prog, const utils::world_serializator* world_data, const bool create_map_tiles);
    void from_menu_to_create_map(utils::progress_container* prog);
    void from_menu_to_map(utils::progress_container* prog);
    void from_create_map_to_map(utils::progress_container* prog);
    void from_map_to_battle_part1(sol::state_view lua, utils::progress_container* prog);
    void from_map_to_battle_part2(sol::state_view lua, utils::progress_container* prog);
  }
}

#endif
