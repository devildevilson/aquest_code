#ifndef LOADING_FUNCTIONS_H
#define LOADING_FUNCTIONS_H

#include <cstddef>
#include <cstdint>
#include <string>

namespace dt {
  class thread_pool;
}

namespace devils_engine {
  namespace core {
    struct map;
    class context;
  }
  
  namespace utils {
    class progress_container;
    class world_serializator;
  }
  
  namespace systems {
    struct core_t;
    struct map_t;
    
    void setup_map_generator(const systems::map_t* map_data);
    
    void find_border_points(core::map* map, const core::context* ctx);
    void generate_tile_connections(core::map* map, dt::thread_pool* pool);
    void validate_and_create_data(systems::map_t* map_systems, utils::progress_container* prog);
    void post_generation_work(systems::map_t* map_systems, systems::core_t* systems, utils::progress_container* prog);
    
    void load_map_data(core::map* map, utils::world_serializator* world);
    void loading_world(systems::map_t* map_systems, utils::progress_container* prog, const std::string &world_path);
    void from_menu_to_create_map(utils::progress_container* prog);
    void from_menu_to_map(utils::progress_container* prog);
    void from_create_map_to_map(utils::progress_container* prog);
  }
}

#endif
