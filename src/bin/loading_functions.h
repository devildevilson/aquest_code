#ifndef DEVILS_ENGINE_BIN_LOADING_FUNCTIONS_H
#define DEVILS_ENGINE_BIN_LOADING_FUNCTIONS_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <array>
#include <future>
#include "utils/sol.h"

#define MAX_BIOMES_COUNT (0xff)

class FastNoiseLite;

namespace dt {
  class thread_pool;
}

namespace devils_engine {
  namespace core {
    struct map;
    class context;
    struct game_resources_t;
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
    struct generator_t;
    
    struct loading_context;
    
    typedef void (*loading_piece_p) (loading_context*);
    typedef std::future<void> (*loading_piece_async_p) (loading_context*);
    
    loading_context* create_loading_context(core::game_resources_t* res);
    void destroy_loading_context(loading_context* ctx);
    
    // самым простым вариантом будет указать здесь все функции которые могут быть сделаны параллельно
    // вообще было бы неплохо сделать АСИНХРОННОСТЬ (не мультитрединг), но я чет не понимаю как
    // + не понимаю некоторые вещи такие как асинхронный мьютекс и прочее
    // короче узнал вот что, меня все же интересует многопоточность
    // асинхронность сильно завязана на ожидание процессором какого то действия, во время ожидания можно что нибудь повычислять
    // а меня конечно нужна больше параллельность, даже при том что у меня есть целых 16 миллисекунд ожидания
    // асио не может гарантировать насколько сложной будет задача которая займет процессор во время ожидания
    // поэтому мне это не подходит
    namespace loading_main_menu {
      extern const loading_piece_p funcs[];
      extern const std::string_view names[];
      extern const size_t func_count;
      void advance(loading_context*, utils::progress_container*);
    }
    
    namespace loading_saved_world {
      extern const loading_piece_p funcs[];
      extern const std::string_view names[];
      extern const size_t func_count;
      void advance(loading_context*, utils::progress_container*);
    }
    
    namespace loading_generated_map {
      extern const loading_piece_p funcs[];
      extern const std::string_view names[];
      extern const size_t func_count;
      void advance(loading_context*, utils::progress_container*);
    }
    
    namespace loading_save_game {
      extern const loading_piece_p funcs[];
      extern const std::string_view names[];
      extern const size_t func_count;
      void advance(loading_context*, utils::progress_container*);
    }
    
    namespace loading_generator {
      extern const loading_piece_p funcs[];
      extern const std::string_view names[];
      extern const size_t func_count;
      void advance(loading_context*, utils::progress_container*);
    }
    
    namespace loading_battle {
      extern const loading_piece_p funcs[];
      extern const std::string_view names[];
      extern const size_t func_count;
      void advance(loading_context*, utils::progress_container*);
    }
    
    namespace loading_encounter {
      extern const loading_piece_p funcs[];
      extern const std::string_view names[];
      extern const size_t func_count;
      void advance(loading_context*, utils::progress_container*);
    }
    
    namespace post_gen {
      extern const loading_piece_p funcs[];
      extern const std::string_view names[];
      extern const size_t func_count;
    }
    
    // асинхронная версия поверх синхронных, не все функции послностью асинхронны, в некоторых некое действие + фьючер из пустой функции
    namespace loading_main_menu_async {
      extern const loading_piece_async_p funcs[];
      extern const std::string_view names[];
      extern const size_t func_count;
    }
    
    namespace loading_saved_world_async {
      extern const loading_piece_async_p funcs[];
      extern const std::string_view names[];
      extern const size_t func_count;
    }
    
    namespace loading_generated_map_async {
      extern const loading_piece_async_p funcs[];
      extern const std::string_view names[];
      extern const size_t func_count;
    }
    
    namespace loading_save_game_async {
      extern const loading_piece_async_p funcs[];
      extern const std::string_view names[];
      extern const size_t func_count;
    }
    
    namespace loading_generator_async {
      extern const loading_piece_async_p funcs[];
      extern const std::string_view names[];
      extern const size_t func_count;
    }
    
    namespace loading_battle_async {
      extern const loading_piece_async_p funcs[];
      extern const std::string_view names[];
      extern const size_t func_count;
    }
    
    namespace loading_encounter_async {
      extern const loading_piece_async_p funcs[];
      extern const std::string_view names[];
      extern const size_t func_count;
    }
    
    namespace post_gen_async {
      extern const loading_piece_async_p funcs[];
      extern const std::string_view names[];
      extern const size_t func_count;
    }
    
    std::array<std::pair<uint32_t, uint32_t>, MAX_BIOMES_COUNT> get_season_biomes_data(const systems::map_t* map_system);
    
    void advance_progress(utils::progress_container* prog, const std::string &str);
    //void setup_map_generator(const systems::map_t* map_data);
    void setup_map_generator(const systems::generator_t* gen_data);
    
    void find_border_points(core::map* map, const core::context* ctx);
    void generate_tile_connections(core::map* map, dt::thread_pool* pool);
    //void validate_and_create_data(systems::map_t* map_systems, utils::progress_container* prog);
    //void post_generation_work(systems::map_t* map_systems, systems::core_t* systems, utils::progress_container* prog);
    
    // нужно короч сделать так же как в генераторе пары (название, функция)
    // чтобы легко следить было за состоянием загрузки
    
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
