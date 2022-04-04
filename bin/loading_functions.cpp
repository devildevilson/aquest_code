#include "loading_functions.h"

#include "utils/utility.h"
#include "utils/thread_pool.h"
#include "utils/globals.h"
#include "utils/works_utils.h"
#include "utils/serializator_helper.h"
#include "utils/systems.h"
#include "utils/string_container.h"
#include "utils/progress_container.h"
#include "utils/lua_initialization.h"
#include "utils/lua_environment.h"
#include "utils/localization_container.h"
#include "utils/interface_container2.h"
#include "utils/logging.h"

#include "render/vulkan_hpp_header.h"
#include "render/container.h"
#include "render/targets.h"
#include "render/stages.h"
#include "render/image_container.h"
#include "render/image_controller.h"
#include "render/shared_battle_structures.h"
#include "render/battle_render_stages.h"
#include "render/map_data.h"
#include "render/render_mode_container.h"

#include "figures.h"
#include "logic.h"
#include "map_creator.h"
#include "game_time.h"
#include "image_parser.h"
#include "data_parser.h"
#include "heraldy_parser.h"

#include "loading_defines.h"

#include "core/map.h"
#include "core/seasons.h"
#include "core/context.h"
#include "core/internal_lua_state.h"
#include "core/structures_header.h"
#include "core/generator_begin.h"

#include "battle/map.h"
#include "battle/structures_enum.h"
#include "battle/troop_parser.h"
#include "battle/troop_type_parser.h"
#include "battle/unit_state_parser.h"
#include "battle/context.h"
#include "battle/map_enum.h"

#define STATIC_ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

namespace devils_engine {
  namespace systems {
    struct loading_context {
      systems::core_t* core;
      systems::map_t* map;
      systems::battle_t* battle;
      systems::encounter_t* encounter;
      
      utils::data_string_container to_data;
      utils::numeric_string_container heraldy_data_container; // отвалится
      
      std::unique_ptr<utils::world_serializator> world_data_container;
      utils::world_serializator* world_data;
      std::unique_ptr<sol::state> lua;
      std::unique_ptr<sol::environment> env;
      std::unique_ptr<render::vk_buffer_data_unique> vk_buffer;
      size_t heraldy_layers_count;
      size_t heraldy_layers_size;
      bool create_map_tiles;
      
      inline loading_context() : 
        core(global::get<systems::core_t>()), 
        map(global::get<systems::map_t>()), 
        battle(global::get<systems::battle_t>()), 
        encounter(global::get<systems::encounter_t>()),
        heraldy_layers_count(0),
        heraldy_layers_size(0),
        create_map_tiles(true)
      {
        global::get(&to_data);
        global::get(&heraldy_data_container);
        global::get(core->image_controller);
        if (map != nullptr) {
          global::get(map->core_context);
          // создаем когда потребуется
          //world_data.reset(new utils::world_serializator);
        }
      }
      
      inline ~loading_context() {
        global::get(reinterpret_cast<utils::data_string_container*>(SIZE_MAX));
        global::get(reinterpret_cast<utils::numeric_string_container*>(SIZE_MAX));
        global::get(reinterpret_cast<render::image_controller*>(SIZE_MAX));
        global::get(reinterpret_cast<core::context*>(SIZE_MAX));
      }
    };
    
    loading_context* create_loading_context() { return new loading_context; }
    void destroy_loading_context(loading_context* ctx) { delete ctx; }
    void common_advance(loading_context* ctx, utils::progress_container* prog, const loading_piece_p* funcs, const std::string_view* names, const size_t &count) {
      for (size_t i = 0; i < count; ++i) {
        prog->set_hint2(names[i]);
        funcs[i](ctx);
        prog->advance();
      }
      
      assert(prog->finished());
    }
    
#define LOADING_FUNC(name) void name(loading_context*);
    LOADING_SAVED_WORLD_FUNC_LIST
#undef LOADING_FUNC

#define LOADING_FUNC(name) void name(loading_context*);
    LOADING_GENERATED_MAP_FUNC_LIST
#undef LOADING_FUNC

#define LOADING_FUNC(name) void name(loading_context*);
    LOADING_GENERATOR_FUNC_LIST
#undef LOADING_FUNC
    
    namespace loading_main_menu {
      const loading_piece_p funcs[] = {
        
      };
      
      const std::string_view names[] = {
        
      };
      
      const size_t func_count = STATIC_ARRAY_SIZE(funcs);
      static_assert(STATIC_ARRAY_SIZE(funcs) == STATIC_ARRAY_SIZE(names));
      
      void advance(loading_context* ctx, utils::progress_container* prog) { common_advance(ctx, prog, funcs, names, func_count); }
    }
    
    namespace loading_saved_world {
      const loading_piece_p funcs[] = {
#define LOADING_FUNC(name) &name,
        LOADING_SAVED_WORLD_FUNC_LIST
#undef LOADING_FUNC
      };
      
      const std::string_view names[] = {
#define LOADING_FUNC(name) #name,
        LOADING_SAVED_WORLD_FUNC_LIST
#undef LOADING_FUNC
      };
      
      const size_t func_count = STATIC_ARRAY_SIZE(funcs);
      static_assert(STATIC_ARRAY_SIZE(funcs) == STATIC_ARRAY_SIZE(names));
      
      void advance(loading_context* ctx, utils::progress_container* prog) { common_advance(ctx, prog, funcs, names, func_count); }
    }
    
    namespace loading_generated_map {
      const loading_piece_p funcs[] = {
#define LOADING_FUNC(name) &name,
        LOADING_GENERATED_MAP_FUNC_LIST
#undef LOADING_FUNC
      };
      
      const std::string_view names[] = {
#define LOADING_FUNC(name) #name,
        LOADING_GENERATED_MAP_FUNC_LIST
#undef LOADING_FUNC
      };
      
      const size_t func_count = STATIC_ARRAY_SIZE(funcs);
      static_assert(STATIC_ARRAY_SIZE(funcs) == STATIC_ARRAY_SIZE(names));
      
      void advance(loading_context* ctx, utils::progress_container* prog) { common_advance(ctx, prog, funcs, names, func_count); }
    }
    
    namespace loading_save_game {
      const loading_piece_p funcs[] = {
        
      };
      
      const std::string_view names[] = {
        
      };
      
      const size_t func_count = STATIC_ARRAY_SIZE(funcs);
      static_assert(STATIC_ARRAY_SIZE(funcs) == STATIC_ARRAY_SIZE(names));
      
      void advance(loading_context* ctx, utils::progress_container* prog) { common_advance(ctx, prog, funcs, names, func_count); }
    }
    
    // нужно заполнить эти вещи, иначе отваливается загрузка, давно хотел но забыл мех
    namespace loading_generator {
      const loading_piece_p funcs[] = {
#define LOADING_FUNC(name) &name,
        LOADING_GENERATOR_FUNC_LIST
#undef LOADING_FUNC
      };
      
      const std::string_view names[] = {
#define LOADING_FUNC(name) #name,
        LOADING_GENERATOR_FUNC_LIST
#undef LOADING_FUNC
      };
      
      const size_t func_count = STATIC_ARRAY_SIZE(funcs);
      static_assert(STATIC_ARRAY_SIZE(funcs) == STATIC_ARRAY_SIZE(names));
      
      void advance(loading_context* ctx, utils::progress_container* prog) { common_advance(ctx, prog, funcs, names, func_count); }
    }
    
    namespace loading_battle {
      const loading_piece_p funcs[] = {
        
      };
      
      const std::string_view names[] = {
        
      };
      
      const size_t func_count = STATIC_ARRAY_SIZE(funcs);
      static_assert(STATIC_ARRAY_SIZE(funcs) == STATIC_ARRAY_SIZE(names));
      
      void advance(loading_context* ctx, utils::progress_container* prog) { common_advance(ctx, prog, funcs, names, func_count); }
    }
    
    namespace loading_encounter {
      const loading_piece_p funcs[] = {
        
      };
      
      const std::string_view names[] = {
        
      };
      
      const size_t func_count = STATIC_ARRAY_SIZE(funcs);
      static_assert(STATIC_ARRAY_SIZE(funcs) == STATIC_ARRAY_SIZE(names));
      
      void advance(loading_context* ctx, utils::progress_container* prog) { common_advance(ctx, prog, funcs, names, func_count); }
    }
    
    void advance_progress(utils::progress_container* prog, const std::string &str) {
      prog->set_hint2(std::move(str));
      prog->advance();
    }
    
    std::array<std::pair<uint32_t, uint32_t>, MAX_BIOMES_COUNT> get_season_biomes_data(const systems::map_t* map_system) {
      std::array<std::pair<uint32_t, uint32_t>, MAX_BIOMES_COUNT> biomes_data;
      for (size_t i = 0; i < core::map::hex_count_d(core::map::detail_level); ++i) {
        const uint32_t index = map_system->seasons->get_tile_biome(map_system->seasons->current_season, i);
        assert(index < MAX_BIOMES_COUNT);
        ++biomes_data[index].second;
      }
      
      size_t current_offset = 0;
      for (uint32_t i = 0; i < MAX_BIOMES_COUNT; ++i) {
        biomes_data[i].first = current_offset;
        current_offset += biomes_data[i].second * (PACKED_INDEX_COEF + 1);
      }
      
      return biomes_data;
    }
    
    std::array<std::pair<uint32_t, uint32_t>, BATTLE_BIOMES_MAX_COUNT> get_battle_biomes_data(const systems::battle_t* battle_system) {
      std::array<std::pair<uint32_t, uint32_t>, BATTLE_BIOMES_MAX_COUNT> biomes_data;
      std::fill(biomes_data.begin(), biomes_data.end(), std::make_pair(0, 0));
      for (size_t i = 0; i < battle_system->map->tiles_count; ++i) {
        const uint32_t index = battle_system->map->get_tile_biome(i);
        ++biomes_data[index].second;
      }
      
      size_t current_offset = 0;
      for (uint32_t i = 0; i < BATTLE_BIOMES_MAX_COUNT; ++i) {
        biomes_data[i].first = current_offset;
        current_offset += biomes_data[i].second * (PACKED_INDEX_COEF + 1);
      }
      
      return biomes_data;
    }

    // сюда видимо бедт приходить какая таблица, по которой мы будем создавать генератор
    void setup_map_generator(const systems::map_t* map_data) {
      auto ptr = map_data->map_creator;
      ASSERT(ptr != nullptr);
      
      // теперь осталось понять откуда получить config_path, это должно лежать где то рядом с конфигом для интерфейса
      const std::string config_path = global::root_directory() + "scripts/generator_config.lua";
      
      auto &lua = ptr->state();
      auto &env = ptr->environment();
      const auto ret = lua.script_file(config_path, env);
      CHECK_ERROR_THROW(ret);
      
      if (ret.get_type() != sol::type::table) throw std::runtime_error("Bad return from config script");
      
      const auto config = ret.get<sol::table>();
      const auto scripts_proxy = config["scripts"];
      const auto steps_proxy = config["steps"];
      if (!scripts_proxy.valid() || scripts_proxy.get_type() != sol::type::table) throw std::runtime_error("Invalid config 'scripts' data");
      if (!steps_proxy.valid() || steps_proxy.get_type() != sol::type::table) throw std::runtime_error("Invalid config 'steps' data");
      const auto scripts = scripts_proxy.get<sol::table>();
      const auto steps = steps_proxy.get<sol::table>();
      
      struct step_data {
        std::string name;
        std::vector<std::pair<std::string, sol::function>> functions;
      };
      
      std::vector<step_data> steps_array;
      for (const auto &pair : steps) {
        if (pair.second.get_type() != sol::type::table) continue;
        
        const auto table = pair.second.as<sol::table>();
        const auto name_proxy = table["name"];
        const auto functions_proxy = table["functions"];
        if (!name_proxy.valid() || name_proxy.get_type() != sol::type::string) throw std::runtime_error("Invalid generator config step name data");
        if (!functions_proxy.valid() || functions_proxy.get_type() != sol::type::table) throw std::runtime_error("Invalid generator config step functions data");
        
        const auto functions = functions_proxy.get<sol::table>();
        steps_array.emplace_back();
        steps_array.back().name = name_proxy;
        for (const auto &pair : functions) {
          if (pair.second.get_type() != sol::type::string) continue;
          const auto f_name = pair.second.as<std::string>();
          steps_array.back().functions.push_back(std::make_pair(f_name, sol::function(sol::nil)));
        }
      }
      
      for (const auto &pair : scripts) {
        if (pair.second.get_type() != sol::type::string) continue;
        const auto path = pair.second.as<std::string_view>();
        const size_t pos = path.find('/');
        if (pos == 0 || pos == std::string_view::npos) throw std::runtime_error("Invalid generator config script path " + std::string(path));
        
        const auto mod_name = path.substr(0, pos);
        assert(mod_name == "apates_quest");
        const auto script_path = path.substr(pos+1);
        const auto final_path = global::root_directory() + std::string(script_path);
        const auto ret = lua.script_file(final_path, env);
        CHECK_ERROR_THROW(ret);
        
        if (ret.get_type() != sol::type::table) throw std::runtime_error("Bad return from generator functions script " + std::string(path));
        
        const auto func_table = ret.get<sol::table>();
        for (auto &step : steps_array) {
          for (auto &pair : step.functions) {
            const auto proxy = func_table[pair.first];
            if (!proxy.valid() || proxy.get_type() != sol::type::function) continue;
            // throw std::runtime_error("Invalid function " + pair.first + " from script " + std::string(path));
            pair.second = proxy;
          }
        }
      }
      
      size_t counter = 0;
      for (const auto &step : steps_array) {
        for (const auto &pair : step.functions) {
          if (pair.second.valid()) continue;
          PRINT("Could not find generator function " + pair.first);
          ++counter;
        }
      }
      
      if (counter != 0) throw std::runtime_error("Could not find " + std::to_string(counter) + " generator functions");
      
      static const auto gen_func = [] (map::generator::context* ctx, sol::table &table, sol::function func) {
        auto ret = func(ctx, table);
        CHECK_ERROR_THROW(ret);
      };
      
      for (const auto &step : steps_array) {
        std::vector<map::generator_pair> func_pairs;
        for (const auto &pair : step.functions) {
          func_pairs.emplace_back(pair.first, std::bind(gen_func, std::placeholders::_1, std::placeholders::_2, pair.second));
        }
        
        ptr->create(step.name, func_pairs);
      }
    

// //     ptr->run_interface_script(global::root_directory() + "scripts/gen_part1.lua");
// //     ptr->run_interface_script(global::root_directory() + "scripts/gen_part2.lua");
// //     ptr->run_interface_script(global::root_directory() + "scripts/gen_part3.lua");
//     ptr->run_script(global::root_directory() + "scripts/gen_part1_functions.lua");
//     ptr->run_script(global::root_directory() + "scripts/gen_part2_functions.lua");
//     ptr->run_script(global::root_directory() + "scripts/gen_part3_functions.lua");
// //     ptr->run_interface_script(global::root_directory() + "scripts/generator_progress.lua");
// //     ptr->progress_interface("gen_progress");
// //     interface->register_function("gen_part1_fun", "gen_part1_fun"); // тут регистрировать функции?
// //     interface->register_function("gen_part2_fun", "gen_part2_fun");
// //     interface->register_function("gen_part3_fun", "gen_part3_fun");
// 
//     {
//       auto func1 = ptr->get_func("setup_generator");
//       auto func2 = ptr->get_func("generate_plates");
//       auto func3 = ptr->get_func("generate_plate_datas");
//       
//       const std::vector<map::generator_pair> pairs = {
// //         map::default_generator_pairs[0], // это бегин
// //         map::default_generator_pairs[1],
// //         map::default_generator_pairs[2],
// //         map::default_generator_pairs[3]
//         std::make_pair("seting up generator", std::bind(gen_func, std::placeholders::_1, std::placeholders::_2, func1)),
//         std::make_pair("generating plates", std::bind(gen_func, std::placeholders::_1, std::placeholders::_2, func2)),
//         std::make_pair("generating plate datas", std::bind(gen_func, std::placeholders::_1, std::placeholders::_2, func3))
//       };
//       ptr->create("Tectonic plates generator", pairs);
//     }
// 
//     {
//       auto func1 = ptr->get_func("compute_boundary_edges");
//       auto func2 = ptr->get_func("compute_plate_boundary_stress");
//       auto func3 = ptr->get_func("compute_plate_boundary_distances");
//       auto func4 = ptr->get_func("calculate_vertex_elevation");
//       auto func5 = ptr->get_func("blur_tile_elevation");
//       auto func6 = ptr->get_func("normalize_tile_elevation");
//       auto func7 = ptr->get_func("compute_tile_heat");
//       auto func8 = ptr->get_func("compute_tile_water_distances");
//       auto func9 = ptr->get_func("compute_tile_moisture");
//       auto func10 = ptr->get_func("create_biomes");
//       
// //       const std::vector<map::generator_pair> pairs = {
// //         map::default_generator_pairs[4],
// //         map::default_generator_pairs[5],
// //         map::default_generator_pairs[6],
// //         map::default_generator_pairs[7],
// //         map::default_generator_pairs[8],
// //         map::default_generator_pairs[9],
// //         map::default_generator_pairs[10],
// //         map::default_generator_pairs[11],
// //         map::default_generator_pairs[12],
// //         map::default_generator_pairs[13]
// //       };
//       
//       const std::vector<map::generator_pair> pairs = {
//         std::make_pair(map::default_generator_pairs[4].first, std::bind(gen_func, std::placeholders::_1, std::placeholders::_2, func1)),
//         std::make_pair(map::default_generator_pairs[5].first, std::bind(gen_func, std::placeholders::_1, std::placeholders::_2, func2)),
//         std::make_pair(map::default_generator_pairs[6].first, std::bind(gen_func, std::placeholders::_1, std::placeholders::_2, func3)),
//         std::make_pair(map::default_generator_pairs[7].first, std::bind(gen_func, std::placeholders::_1, std::placeholders::_2, func4)),
//         std::make_pair(map::default_generator_pairs[8].first, std::bind(gen_func, std::placeholders::_1, std::placeholders::_2, func5)),
//         std::make_pair(map::default_generator_pairs[9].first, std::bind(gen_func, std::placeholders::_1, std::placeholders::_2, func6)),
//         std::make_pair(map::default_generator_pairs[10].first, std::bind(gen_func, std::placeholders::_1, std::placeholders::_2, func7)),
//         std::make_pair(map::default_generator_pairs[11].first, std::bind(gen_func, std::placeholders::_1, std::placeholders::_2, func8)),
//         std::make_pair(map::default_generator_pairs[12].first, std::bind(gen_func, std::placeholders::_1, std::placeholders::_2, func9)),
//         std::make_pair(map::default_generator_pairs[13].first, std::bind(gen_func, std::placeholders::_1, std::placeholders::_2, func10))
//       };
// 
//       ptr->create("Biomes generator", pairs);
//     }
// 
//     {
//       auto func1 = ptr->get_func("generate_provinces");
//       auto func2 = ptr->get_func("province_postprocessing");
//       auto func3 = ptr->get_func("calculating_province_neighbors");
//       auto func4 = ptr->get_func("generate_cultures");
//       auto func5 = ptr->get_func("generate_countries");
//       auto func6 = ptr->get_func("generate_heraldy");
//       auto func7 = ptr->get_func("generate_titles");
//       auto func8 = ptr->get_func("generate_characters");
//       //auto func9 = ptr->get_func("generate_tech_level");
//       auto func9 = ptr->get_func("generate_cities");
//       
// //       const std::vector<map::generator_pair> pairs = {
// //         map::default_generator_pairs[14],
// //         map::default_generator_pairs[15],
// //         map::default_generator_pairs[16],
// //         map::default_generator_pairs[17],
// //         map::default_generator_pairs[18],
// //         map::default_generator_pairs[22], // геральдики
// //         map::default_generator_pairs[19],
// //         map::default_generator_pairs[20],
// //         map::default_generator_pairs[21],
// //       };
//       
//       const std::vector<map::generator_pair> pairs = {
//         std::make_pair(map::default_generator_pairs[14].first, std::bind(gen_func, std::placeholders::_1, std::placeholders::_2, func1)),
//         std::make_pair(map::default_generator_pairs[15].first, std::bind(gen_func, std::placeholders::_1, std::placeholders::_2, func2)),
//         std::make_pair(map::default_generator_pairs[16].first, std::bind(gen_func, std::placeholders::_1, std::placeholders::_2, func3)),
//         std::make_pair(map::default_generator_pairs[17].first, std::bind(gen_func, std::placeholders::_1, std::placeholders::_2, func4)),
//         std::make_pair(map::default_generator_pairs[18].first, std::bind(gen_func, std::placeholders::_1, std::placeholders::_2, func5)),
//         std::make_pair(map::default_generator_pairs[22].first, std::bind(gen_func, std::placeholders::_1, std::placeholders::_2, func6)),
//         std::make_pair(map::default_generator_pairs[19].first, std::bind(gen_func, std::placeholders::_1, std::placeholders::_2, func7)),
//         std::make_pair(map::default_generator_pairs[20].first, std::bind(gen_func, std::placeholders::_1, std::placeholders::_2, func9)),
//         std::make_pair(map::default_generator_pairs[21].first, std::bind(gen_func, std::placeholders::_1, std::placeholders::_2, func8)),
//         //std::make_pair(map::default_generator_pairs[13].first, std::bind(gen_func, std::placeholders::_1, std::placeholders::_2, func10))
//       };
// 
//       ptr->create("Countries generator", pairs);
//     }
  }
  
//   void copy_structure_data(const core::city_type* data, const size_t &size, const size_t &offset, yavf::Buffer* buffer) {
//     auto buffer_data = reinterpret_cast<render::world_structure_t*>(buffer->ptr());
//     for (size_t i = 0; i < size; ++i) {
//       const size_t index = offset+i;
//       buffer_data[index].city_image_top = data[i].city_image_top;
//       buffer_data[index].city_image_face = data[i].city_image_face;
//       buffer_data[index].scale = data[i].scale;
//       buffer_data[index].dummy = 0;
//     }
//   }

  void make_random_player_character() {
    using namespace utils::xoroshiro128starstar;
    
    auto ctx = global::get<systems::map_t>()->core_context;
    const size_t chars_count = ctx->characters_count();
    state s = {67586, 987699695};
    s = rng(s);
    const double val = utils::rng_normalize(get_value(s));
    const size_t index = chars_count * val;
    auto c = ctx->get_character(index);
    c->make_player();
    game::update_player(c);
    
    auto title = c->self->titles;
    auto next_title = c->self->titles;
    core::city* city = nullptr;
    while (next_title != nullptr) {
      //next_title->heraldy = 0;
      next_title->heraldy_container = {1, 2, 3};
      next_title->heraldy_layers_count = 3;
      if (city == nullptr && next_title->type() == core::titulus::type::city) {
        ASSERT(next_title->city != nullptr);
        city = next_title->city;
      }
      next_title = utils::ring::list_next<utils::list_type::titles>(next_title, title);
    }
    
    ASSERT(city != nullptr);
    const uint32_t city_tile_index = city->tile_index;
    
    auto map = global::get<systems::map_t>()->map;
//     const auto data = map->get_tile_objects_indices(tile_index);
//     map->set_tile_objects_indices(tile_index, glm::uvec4(data.x, 1, data.z, data.w));
//     {
//       std::unique_lock<std::mutex> lock(map->mutex);
//       map->set_tile_objects_index(city_tile_index, 1, 1);
//     }
    // сработало, и камера еще довольно удачно встает
    // камеру стоит расчитывать при старте карты
    
    // создаем две армии, пока что нет никакой принадлежности у армий
    auto army1 = ctx->create_army();
    auto army2 = ctx->create_army();
//     auto army1 = ctx->get_army(army1_token);
//     auto army2 = ctx->get_army(army2_token);
    // их нужно расположить рядом с городом игрока
    auto province = city->province;
    uint32_t army_tile1 = UINT32_MAX;
    uint32_t army_tile2 = UINT32_MAX;
    uint32_t attempts = 0;
    while ((army_tile1 == UINT32_MAX || army_tile2 == UINT32_MAX) && attempts < 100) {
      s = rng(s);
      const double val = utils::rng_normalize(get_value(s));
      ++attempts;
      ASSERT(province->tiles.size() != 0);
      const uint32_t rand_index = (province->tiles.size()-1) * val;
      const uint32_t tile_index = province->tiles[rand_index];
      const float height = map->get_tile_height(tile_index);
      if (height < 0.0f) continue;
      if (height > 0.5f) continue;
      
      if (army_tile1 == UINT32_MAX) army_tile1 = tile_index;
      if (army_tile1 != tile_index) army_tile2 = tile_index;
    }
    
    ASSERT(army_tile1 != UINT32_MAX);
    ASSERT(army_tile2 != UINT32_MAX);
    
    army1->tile_index = army_tile1;
    army2->tile_index = army_tile2;
    
    {
      const auto tile = ctx->get_entity<core::tile>(army_tile1);
      const uint32_t point_index = tile->center;
      const glm::vec4 center = map->get_point(point_index);
      const glm::vec4 normal = glm::normalize(glm::vec4(glm::vec3(center), 0.0f));
      const float height = map->get_tile_height(army_tile1);
      
      const glm::vec4 final_point = center + normal * (height + 1.0f);
      army1->set_pos(glm::vec3(final_point));
      
//       PRINT_VEC3("army1", final_point)
    }
    
    {
      const auto tile = ctx->get_entity<core::tile>(army_tile2);
      const uint32_t point_index = tile->center;
      const glm::vec4 center = map->get_point(point_index);
      const glm::vec4 normal = glm::normalize(glm::vec4(glm::vec3(center), 0.0f));
      const float height = map->get_tile_height(army_tile2);
      
      const glm::vec4 final_point = center + normal * (height + 1.0f);
      army2->set_pos(glm::vec3(final_point));
      
//       PRINT_VEC3("army2", final_point)
    }
    
    auto img_cont = global::get<systems::core_t>()->image_controller;
    auto view = img_cont->get_view("hero_img");
    
    army1->set_img(view->get_image(0, false, false));
    army2->set_img(view->get_image(0, false, false));
    
    {
      auto ptr = ctx->get_entity<core::tile>(army1->tile_index);
      ptr->army_token = army1->object_token;
    }
    
    {
      auto ptr = ctx->get_entity<core::tile>(army2->tile_index);
      ptr->army_token = army2->object_token;
    }
    
    std::unique_lock<std::mutex> lock(map->mutex);
//     map->set_tile_objects_index(army_tile1, 4, army1->army_gpu_slot);
//     map->set_tile_objects_index(army_tile2, 4, army2->army_gpu_slot);
    
//     {
//       const uint32_t testing_tile = 317116;
//       const auto &tile_data = render::unpack_data(map->get_tile(testing_tile));
//       const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
//       PRINT_VAR("tile index ", testing_tile)
//       PRINT_VAR("n_count    ", n_count)
//       for (uint32_t i = 0; i < n_count; ++i) {
//         const uint32_t n_index = tile_data.neighbors[i];
//         PRINT_VAR("neighbour  " + std::to_string(i), n_index)
//         const auto &n_tile_data = render::unpack_data(map->get_tile(n_index));
//         const uint32_t n_n_count = render::is_pentagon(tile_data) ? 5 : 6;
//         bool found = false;
//         for (uint32_t j = 0; j < n_n_count; ++j) {
//           const uint32_t n_index = n_tile_data.neighbors[j];
//           if (n_index == testing_tile) found = true;
//         }
//         
//         ASSERT(found);
//         UNUSED_VARIABLE(found);
//       }
//     }
//     
//     {
//       const uint32_t testing_tile = 318586;
//       const auto &tile_data = render::unpack_data(map->get_tile(testing_tile));
//       const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
//       PRINT_VAR("tile index ", testing_tile)
//       PRINT_VAR("n_count    ", n_count)
//       for (uint32_t i = 0; i < n_count; ++i) {
//         const uint32_t n_index = tile_data.neighbors[i];
//         PRINT_VAR("neighbour  " + std::to_string(i), n_index)
//         const auto &n_tile_data = render::unpack_data(map->get_tile(n_index));
//         const uint32_t n_n_count = render::is_pentagon(tile_data) ? 5 : 6;
//         bool found = false;
//         for (uint32_t j = 0; j < n_n_count; ++j) {
//           const uint32_t n_index = n_tile_data.neighbors[j];
//           if (n_index == testing_tile) found = true;
//         }
//         
//         ASSERT(found);
//         UNUSED_VARIABLE(found);
//       }
//     }
  }

//   void copy_structure_data(const core::context* context, const size_t &offset, core::map* map, void* buffer) {
//     std::unordered_map<const core::city_type*, uint32_t> type_map;
//     auto buffer_data = reinterpret_cast<render::world_structure_t*>(buffer);
//     for (size_t i = 0; i < context->get_entity_count<core::city_type>(); ++i) {
//       const size_t index = offset+i;
//       auto city_type = context->get_entity<core::city_type>(i);
//       buffer_data[index].city_image_top = city_type->city_image_top;
//       buffer_data[index].city_image_face = city_type->city_image_face;
//       buffer_data[index].scale = city_type->scale;
//       buffer_data[index].heraldy_layer_index = 0;
//       
//       type_map[city_type] = index;
//     }
//     
//     for (size_t i = 0; i < context->get_entity_count<core::city>(); ++i) {
//       auto city = context->get_entity<core::city>(i);
//       const uint32_t struct_index = type_map[city->type];
//       const uint32_t tile_index = city->tile_index;
//       map->set_tile_structure_index(tile_index, struct_index);
//       auto title = city->title;
//       auto faction = title->owner;
//       core::titulus* first_city_title = nullptr;
//       auto first_title = faction->titles;
//       auto next_title = faction->titles;
//       while (next_title != nullptr) {
//         if (next_title->type() == core::titulus::type::city) first_city_title = next_title;
// //         auto next = first_title->next;
// //         first_title = next;
//         next_title = utils::ring::list_next<utils::list_type::titles>(next_title, first_title);
//       }
//       
//       //map->set_tile_objects_indices(tile_index, glm::uvec4(struct_index, title == first_city_title ? 0 : UINT32_MAX, 0, UINT32_MAX));
//       map->set_tile_objects_index(tile_index, 0, struct_index);
//       map->set_tile_objects_index(tile_index, 1, title == first_city_title ? 0 : UINT32_MAX);
//     }
//   }

    void find_border_points(core::map* map, core::context* ctx) {
      // возможно нужно найти первый граничный тайл, а потом обходить его соседей
      // тут можно поступить несколькими способами
      // 1) попытаться обойти тайлы правильно
      // 2) составить облако точек (ну хотя вряд ли)
      // у меня не всегда провинция более менее верной формы
      // не говоря даже про страны, значит нужно попытаться обойти тайлы верно
      // находится первый случайный граничный тайл, от него, по часовой стрелке
      // обходим граничные тайлы (у всех граничных тайлов, должно быть как минимум два граничных соседа)
      // вот еще вариант, отрисовывать границу с помощью тайловой графики
      // (то есть несколько спрайтов шестигранников с границами в разном положении)
      // осталось понять как сделать эту тайловую графику
      // самый простой способ - задавать рендер для трех точек (центр + ребро)
      // у этих трех точек всегда одинаковые uv координаты (потому что по сути рисуем только одну одинаковую картинку)
      // осталось только понять как решить некоторые проблемы такого рендеринга
      // есть разрывы в местах перехода от одного тайла к другому
      // разрывы по идее также можно подкорректировать тайловой графикой
      // короче вот что я подумал (точнее вспомнил как это по идее сделано в цк2)
      // границ должно быть две: граница провинций и граница государств (граница вассалов?)
      // граница провинций - статичная, граница государств и вассалов обновляются почти каждый ход
      // вообще я могу сгенерировать границы между провинциями и менять их свойства по ходу игры
      // то есть мне нужно верно определить ребра провинции
      // у ребра две стороны: мы должны рисовать границы разных свойств на каждой стороне
      // все точки ребер мы добавляем в буфер, ребро - треугольник, каждому ребру нужно передать
      // верную текстурку и какие то дополнительные данные

      // тайлы нам сильно помогут сгенерить границы
      // границы делаем по ребрам провинции, нам их нужно верно соединить
      // ребра провинции должны быть по часовой стрелке относительно провинции
      // для каждой провниции я должен сгенерировать ребра
      // между провинциями ребра нужно соединить,
      // для этого нужно подвести ребро провинции с помощью точки на ребре тайла
      // как получить uv координаты?
      // их можно расчитать для правильного тайла (для неправильного тайла тоже по идее, но нужно проверить)
      // тайл состоит из равносторонних треугольников + у нас имеется размер границы
      // uv для внутренней расчитываются с использованием размера
      // остается только последовательность гарантировать

      // теперь где то эту информацию нужно сохранить

      // как то так мы составляем ребра, что теперь с ними делать?
      // по идее все, можно рендерить
      // обходим все ребра и выдаем им характеристики на базе текущей ситуации (тип границы, цвет) (1 раз в ход)
      // затем параллельно обходим каждый треугольник и проверяем его с фрустумом
      // (можно на гпу сделать по идее) (каждый кадр)
      // рисуем поверх карты

      // так вообще я тут подумал
      // тайловая графика чем хороша - она предсказуема
      // все что мне нужно сделать это определить 3 типа отрисовки границы:
      // только внутреннее ребро, внутренее и внешняя часть с одной стороны, полное ребро
      // то есть по сути нужно только где-то убрать и где-то добавить внешнюю часть ребра
      // значит нужно просто найти все граничные ребра, с информацией о двух тайлов
      // как определить типы? внешняя часть добавляется только к ребру, смежное ребро которого
      // приходится на тайл с тем же самым государством (вассалом, провинцей)
      // нужно проверить смежные ребра
      // по идее этого достаточно для того чтобы собрать буфер после фрустум проверки
      // но это означает что мы каждый кадр обходим этот массив и закидываем данные в буфер
      // с другой стороны как иначе? добавить суда сразу данные о границе (цвет, размер)
      // и считать все на гпу
      struct border_data2 {
        uint32_t tile_index;
        uint32_t opposite_tile_index;
        // по этому индексу мы во первых должны найти opposite_tile_index
        // а во вторых две точки + два смежных тайла
        uint32_t edge_index;
      };

      struct border_buffer {
        glm::vec4 points[2];
        glm::vec4 dirs[2];
      };

      struct border_type2 {
        render::color_t color1; // цвет достаточно хранить в uint32
        uint32_t image;
        render::color_t color2;
        float thickness; // эту штуку не нужно делать здесь - это должны быть константы от титула
      };

      // кому еще соседи могут пригодиться?

  //     auto ctx = global::get<core::context>();
      auto updater = global::get<render::tile_updater>();
      
      std::vector<border_data2> borders;
      const uint32_t provinces_count = ctx->get_entity_count<core::province>();
      for (size_t i = 0; i < provinces_count; ++i) {
        const uint32_t province_index = i;
        auto province = ctx->get_entity<core::province>(province_index);
        // нужен более строгий способ получать тайлы у провинций
        //const auto &childs = container->get_childs(map::debug::entities::province, province_index);
        const auto &childs = province->tiles;
        if (childs.empty()) throw std::runtime_error("Could not find province tiles");

        const uint32_t tiles_count = childs.size();
        for (size_t j = 0; j < tiles_count; ++j) {
          const uint32_t tile_index = childs[j];

          const uint32_t offset = borders.size();
          uint32_t border_count = 0;
          //const auto &data = render::unpack_data(map->get_tile(tile_index));
          const auto tile_data = ctx->get_entity<core::tile>(tile_index);
          const uint32_t n_count = tile_data->neighbors_count();
          for (uint32_t k = 0; k < n_count; ++k) {
            const uint32_t n_index = tile_data->neighbors[k];

            //const uint32_t n_province_index = container->get_data<uint32_t>(map::debug::entities::tile, n_index, map::debug::properties::tile::province_index);
            const uint32_t n_province_index = ctx->get_entity<core::tile>(n_index)->province;
            if (province_index == n_province_index) continue;

  //           const auto &n_data = render::unpack_data(map->get_tile(n_index));
  //           const uint32_t k2 = k == 0 ? n_count-1 : k-1;
            const border_data2 d{
              tile_index,
              n_index,
              k
            };
            borders.push_back(d);
            ++border_count;
          }

          // теперь у нас есть offset и border_count
          if (border_count == 0) continue;
          
          ASSERT(border_count <= render::border_size_mask);
          ASSERT(offset < render::border_offset_mask);
          const uint32_t packed_border_data = (border_count << 28) | (offset & render::border_offset_mask);

//           map->set_tile_border_data(tile_index, offset, border_count);
//           updater->update_borders_data(tile_index, packed_border_data);
          auto tile = ctx->get_entity<core::tile>(tile_index);
          tile->borders_data = packed_border_data;
        }
      }

      const float borders_size[] = {0.5f, 0.3f, 0.15f}; // было бы неплохо это убрать в defines.lua (то есть считывать с диска)

      std::unordered_map<const core::titulus*, uint32_t> type_index;
      std::vector<border_type2> types;
      types.push_back({render::make_color(0.0f, 0.0f, 0.0f, 1.0f), UINT32_MAX, render::make_color(0.0f, 0.0f, 0.0f, 1.0f), 0.0f});
      for (size_t i = 0; i < ctx->get_entity_count<core::titulus>(); ++i) {
        // нужно задать типы границ
        // по владельцам?
        auto title = ctx->get_entity<core::titulus>(i);
        if (title->type() == core::titulus::type::city) continue;
  //       if (title->type == core::titulus::type::baron) continue;
        if (title->owner == nullptr) continue;
        if (title->owner->main_title != title) continue;
        ASSERT(type_index.find(title) == type_index.end());
        type_index.insert(std::make_pair(title, types.size()));
        types.push_back({title->border_color1, GPU_UINT_MAX, title->border_color2, 0.0f}); // толщину границы мы должны в поинт записать
      }

      render::vk_buffer_data buffer;
      {
        std::unique_lock<std::mutex> lock(map->mutex);
        auto world_buffers = global::get<systems::map_t>()->world_buffers;
        world_buffers->resize_border_buffer(borders.size() * sizeof(border_buffer));
        world_buffers->resize_border_types(types.size() * sizeof(border_type2));
        buffer = world_buffers->border_buffer;
        auto types_buffer = world_buffers->border_types;
        auto ptr = types_buffer.ptr;
        ASSERT(ptr != nullptr);
        memcpy(ptr, types.data(), types.size() * sizeof(border_type2));
        //global::get<render::tile_borders_optimizer>()->set_borders_count(0);
      }
      //global::get<render::tile_borders_optimizer>()->set_borders_count(borders.size());
      auto pool = global::get<dt::thread_pool>();
      auto* arr = reinterpret_cast<border_buffer*>(buffer.ptr);
      utils::submit_works_async(pool, borders.size(), [&] (const size_t &start, const size_t &count) {
      //for (size_t i = 0; i < borders.size(); ++i) {
        for (size_t i = start; i < start+count; ++i) {
          const auto &current_data = borders[i];
          const auto tile_data = ctx->get_entity<core::tile>(current_data.tile_index);
          //const auto &tile_data = render::unpack_data(map->get_tile(current_data.tile_index));
          //const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
          const uint32_t n_count = tile_data->neighbors_count();

          ASSERT(tile_data->neighbors[current_data.edge_index] == current_data.opposite_tile_index);

    #ifndef _NDEBUG
          {
            const uint32_t k = current_data.edge_index;
            //const uint32_t k2 = k == 0 ? n_count-1 : k-1;
            const uint32_t k2 = (k+1)%n_count;
            const uint32_t point1 = tile_data->points[k];
            const uint32_t point2 = tile_data->points[k2];
            ASSERT(point1 != UINT32_MAX);
            ASSERT(point2 != UINT32_MAX);
            ASSERT(point1 != point2);
            
            const auto n_tile_data = ctx->get_entity<core::tile>(current_data.tile_index);
            const uint32_t n_count2 = n_tile_data->neighbors_count();
//             const auto &n_tile_data = render::unpack_data(map->get_tile(current_data.opposite_tile_index));
//             const uint32_t n_count2 = render::is_pentagon(n_tile_data) ? 5 : 6;
            bool found1 = false;
            bool found2 = false;
            for (uint32_t j = 0; j < n_count2; ++j) {
              const uint32_t n_point_index = n_tile_data->points[j];
              if (n_point_index == point1) found1 = true;
              if (n_point_index == point2) found2 = true;
            }

            ASSERT(found1 && found2);
          }
    #endif

          const uint32_t tmp_index = (current_data.edge_index)%n_count;
          const uint32_t adjacent1 = tile_data->neighbors[(tmp_index+1)%n_count];
          const uint32_t adjacent2 = tile_data->neighbors[tmp_index == 0 ? n_count-1 : tmp_index-1];

    #ifndef _NDEBUG
          {
            const uint32_t k = (tmp_index+1)%n_count;
            const uint32_t k2 = k == 0 ? n_count-1 : k-1;
            const uint32_t k3 = (tmp_index+2)%n_count;
            const uint32_t k4 = k == 0 ? n_count-2 : (k == 1 ? n_count-1 : k-2);
            const uint32_t point1 = tile_data->points[k];
            const uint32_t point2 = tile_data->points[k2];
            const uint32_t point3 = tile_data->points[k3];
            const uint32_t point4 = tile_data->points[k4];
            ASSERT(point1 != UINT32_MAX);
            ASSERT(point2 != UINT32_MAX);
            ASSERT(point1 != point2);

            {
              bool found1 = false;
              bool found2 = false;
              bool found3 = false;
              bool found4 = false;

              const auto n_tile_data = ctx->get_entity<core::tile>(current_data.tile_index);
              const uint32_t n_count2 = n_tile_data->neighbors_count();
//               const auto &n_tile_data = render::unpack_data(map->get_tile(adjacent1));
//               const uint32_t n_count2 = render::is_pentagon(n_tile_data) ? 5 : 6;
              for (uint32_t j = 0; j < n_count2; ++j) {
                const uint32_t n_point_index = n_tile_data->points[j];
                if (n_point_index == point1) found1 = true;
                if (n_point_index == point2) found2 = true;
                if (n_point_index == point3) found3 = true;
                if (n_point_index == point4) found4 = true;
              }

              ASSERT(found1);
              ASSERT(found3);
              ASSERT(!found2);
              ASSERT(!found4);
            }

            {
              bool found1 = false;
              bool found2 = false;
              bool found3 = false;
              bool found4 = false;

              const auto n_tile_data = ctx->get_entity<core::tile>(current_data.tile_index);
              const uint32_t n_count2 = n_tile_data->neighbors_count();
//               const auto &n_tile_data = render::unpack_data(map->get_tile(adjacent2));
//               const uint32_t n_count2 = render::is_pentagon(n_tile_data) ? 5 : 6;
              for (uint32_t j = 0; j < n_count2; ++j) {
                const uint32_t n_point_index = n_tile_data->points[j];
                if (n_point_index == point1) found1 = true;
                if (n_point_index == point2) found2 = true;
                if (n_point_index == point3) found3 = true;
                if (n_point_index == point4) found4 = true;
              }

              ASSERT(!found1);
              ASSERT(!found3);
              ASSERT(found2);
              ASSERT(found4);
            }
          }
    #endif

          // каждый ход нам необходимо произвести небольшие вычисления
          // чтобы обновить границы по ходу игры
          // каждый кадр обновляю то что нужно нарисовать фрустум тест

          //advance_borders_count adv(map, i);
    //       if (i % 1000 == 0) {
    //         std::unique_lock<std::mutex> lock(map->mutex);
    //         global::get<render::tile_borders_optimizer>()->set_borders_count(i);
    //       }
    
          // я хочу сделать все данные карты в гпу локал памяти (вообще лучше ВСЕ данные в локальной памяти сделать)
          // поэтому здесь видимо придется копировать в стейджинг буфер это дело
          // каждый раз когда мы хотим поправить границы на карте

          const uint32_t k = (tmp_index+1)%n_count;
          const uint32_t k2 = k == 0 ? n_count-1 : k-1;
          //const uint32_t k2 = (k+1)%n_count;
          const uint32_t point1_index = tile_data->points[k];
          const uint32_t point2_index = tile_data->points[k2];
          const glm::vec4 point1 = map->get_point(point1_index);
          const glm::vec4 point2 = map->get_point(point2_index);
          arr[i].points[0] = point2;
          arr[i].points[1] = point1;

          // нужно заполнить направления
          // нужно проверить принадлежат ли смежные тайлы к тем же государствам
          // тут же мы определяем тип границы (государственная, вассальная, граница провинции)

          const uint32_t province_index = ctx->get_entity<core::tile>(current_data.tile_index)->province;
          const uint32_t opposite_province_index = ctx->get_entity<core::tile>(current_data.opposite_tile_index)->province;

          const uint32_t adjacent1_province_index = ctx->get_entity<core::tile>(adjacent1)->province;
          const uint32_t adjacent2_province_index = ctx->get_entity<core::tile>(adjacent2)->province;

          if (province_index != adjacent1_province_index) {
            const glm::vec4 center = map->get_point(tile_data->center);
            arr[i].dirs[1] = glm::normalize(center - arr[i].points[1]);
          } else {
            const uint32_t k3 = (tmp_index+2)%n_count;
            const uint32_t point3_index = tile_data->points[k3];
            const glm::vec4 point3 = map->get_point(point3_index);
            arr[i].dirs[1] = glm::normalize(point3 - arr[i].points[1]);
          }

          if (province_index != adjacent2_province_index) {
            const glm::vec4 center = map->get_point(tile_data->center);
            arr[i].dirs[0] = glm::normalize(center - arr[i].points[0]);
          } else {
            const uint32_t k4 = k == 0 ? n_count-2 : (k == 1 ? n_count-1 : k-2);
            const uint32_t point4_index = tile_data->points[k4];
            const glm::vec4 point4 = map->get_point(point4_index);
            arr[i].dirs[0] = glm::normalize(point4 - arr[i].points[0]);
          }

          const auto province = ctx->get_entity<core::province>(province_index);
          const auto opposite_province = opposite_province_index == UINT32_MAX ? nullptr : ctx->get_entity<core::province>(opposite_province_index);
          ASSERT(province->title->owner != nullptr);

          arr[i].dirs[0].w = glm::uintBitsToFloat(current_data.tile_index);
          if (opposite_province == nullptr) {
            auto title = province->title->owner->main_title;
            ASSERT(type_index.find(title) != type_index.end());
            const uint32_t type_idx = type_index[title];
            arr[i].points[0].w = borders_size[0];
            arr[i].dirs[1].w = glm::uintBitsToFloat(type_idx); // или здесь другую границу?
            continue;
          }

          ASSERT(opposite_province->title->owner != nullptr);
          if (province->title->owner == opposite_province->title->owner) {
            // один владелец, мы должны нарисовать базовую границу
            arr[i].points[0].w = borders_size[2];
            arr[i].dirs[1].w = glm::uintBitsToFloat(0);
            continue;
          }

          std::unordered_set<core::realm*> factions;
          auto liege1 = province->title->owner;
          while (liege1 != nullptr) {
            factions.insert(liege1.get());
            liege1 = liege1->liege;
          }

          bool found = false;
          auto liege2 = opposite_province->title->owner;
          while (liege2 != nullptr) {
            found = found || factions.find(liege2.get()) != factions.end();
            liege2 = liege2->liege;
          }

          if (found) {
            // эти провинции находятся в одном государстве
            auto title = province->title->owner->main_title;
            ASSERT(type_index.find(title) != type_index.end());
            const uint32_t type_idx = type_index[title];
            arr[i].points[0].w = borders_size[1];
            arr[i].dirs[1].w = glm::uintBitsToFloat(type_idx);

            continue;
          }

          // это граница разных государств
          auto title = province->title->owner->main_title;
          ASSERT(type_index.find(title) != type_index.end());
          const uint32_t type_idx = type_index[title];
          arr[i].points[0].w = borders_size[0];
          arr[i].dirs[1].w = glm::uintBitsToFloat(type_idx);
          
          // по идее все что мне нужно теперь делать каждый ход это:
          // обновить тип границы, обновить направления
          // и все
          // каждый кадр запихиваем три координаты в фрустум тест
        }
      });
      utils::async_wait(pool);

  //     yavf::Buffer* types_buffer = global::get<render::buffers>()->border_types;
  //     types_buffer->resize(types.size() * sizeof(border_type2));
  //     auto ptr = types_buffer->ptr();
  //     ASSERT(ptr != nullptr);
  //     memcpy(ptr, types.data(), types.size() * sizeof(border_type2));
      std::unique_lock<std::mutex> lock(map->mutex);
      //global::get<render::tile_borders_optimizer>()->set_borders_count(borders.size());
      global::get<render::tile_optimizer>()->set_borders_count(borders.size());
    }

//     const uint32_t layers_count = 10;
//     const float mountain_height = 0.5f;
//     const float render_tile_height = 10.0f;
//     const float layer_height = mountain_height / float(layers_count);
    void generate_tile_connections(core::map* map, dt::thread_pool* pool) {
      assert(false);
      (void)map;
      (void)pool;
//       struct wall {
//         uint32_t tile1;
//         uint32_t tile2;
//         uint32_t point1;
//         uint32_t point2;
//       };
// 
//       std::vector<wall> walls;
//       std::mutex mutex;
// 
//       utils::submit_works_async(pool, map->tiles_count(), [map, &walls, &mutex] (const size_t &start, const size_t &count) {
//         for (size_t i = start; i < start+count; ++i) {
//           const uint32_t tile_index = i;
//           const auto &tile_data = render::unpack_data(map->get_tile(tile_index));
//           const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
//           const float tile_height = tile_data.height;
//           const uint32_t height_layer = render::compute_height_layer(tile_height);
//           if (height_layer == 0) continue;
// 
//           uint32_t size = 0;
//           for (uint32_t j = 0; j < n_count; ++j) {
//             const uint32_t n_index = tile_data.neighbors[j];
//             const auto &n_tile_data = render::unpack_data(map->get_tile(n_index));
//             const float n_tile_height = n_tile_data.height;
//             const uint32_t n_height_layer = render::compute_height_layer(n_tile_height);
//             if (height_layer <= n_height_layer) continue;
//             ++size;
//           }
// 
//           if (size == 0) continue;
// 
// #ifdef _WIN32
//           std::vector<wall> tmp_arr(size); 
// #else
//           wall tmp_arr[size];
// #endif
//           uint32_t counter = 0;
//           for (uint32_t j = 0; j < n_count; ++j) {
//             const uint32_t n_index = tile_data.neighbors[j];
//             const auto &n_tile_data = render::unpack_data(map->get_tile(n_index));
//             const float n_tile_height = n_tile_data.height;
//             const uint32_t n_height_layer = render::compute_height_layer(n_tile_height);
//             if (height_layer <= n_height_layer) continue;
// 
//             // добавляем стенку
//             // нам нужны две точки и индексы тайлов
// 
//             const uint32_t point1 = tile_data.points[j];
//             const uint32_t point2 = tile_data.points[(j+1)%n_count];
// 
//   #ifndef _NDEBUG
//             {
//               std::unordered_set<uint32_t> tmp;
//               tmp.insert(point1);
//               tmp.insert(point2);
// 
//               uint32_t found = 0;
//               const uint32_t n_n_count = render::is_pentagon(n_tile_data) ? 5 : 6;
//               for (uint32_t k = 0; k < n_n_count; ++k) {
//                 const uint32_t point_index = n_tile_data.points[k];
//                 found += uint32_t(tmp.find(point_index) != tmp.end());
// 
//   //               if (point1 == point_index) {
//   //                 const bool a = (n_tile_data.points[(k+1)%n_n_count] == point2 || n_tile_data.points[k == 0 ? n_n_count-1 : k-1] == point2);
//   //                 if (a) found = true;
//   //               }
//               }
// 
//               ASSERT(found == 2);
//             }
//   #endif
// 
// //             std::unique_lock<std::mutex> lock(mutex);
// //             walls.push_back({tile_index, n_index, point1, point2});
//             tmp_arr[counter] = {tile_index, n_index, point1, point2};
//             ++counter;
//           }
// 
//           uint32_t offset = UINT32_MAX;
//           {
//             std::unique_lock<std::mutex> lock(mutex);
//             offset = walls.size();
//             for (uint32_t i = 0; i < size; ++i) {
//               walls.push_back(tmp_arr[i]);
//             }
//           }
// 
//           map->set_tile_connections_data(tile_index, offset, size);
//         }
//       });
//       utils::async_wait(pool); // текущий тред не должен быть главным!!!
// 
//       std::unique_lock<std::mutex> lock(map->mutex);
//       auto connections = global::get<systems::map_t>()->world_buffers->tiles_connections;
//       connections->resize(walls.size() * sizeof(walls[0]));
//       auto ptr = connections->ptr();
//       memcpy(ptr, walls.data(), walls.size() * sizeof(walls[0]));
//       //ASSERT(global::get<render::tile_walls_optimizer>() != nullptr);
//       //global::get<render::tile_walls_optimizer>()->set_connections_count(walls.size());
//       ASSERT(global::get<render::tile_optimizer>() != nullptr);
//       global::get<render::tile_optimizer>()->set_connections_count(walls.size());
    }

    void connect_game_data(core::map* map, core::context* ctx) {
      std::unordered_map<uint32_t, uint32_t> tile_city_map;
      
      {
        for (size_t i = 0; i < core::map::hex_count_d(core::map::detail_level); ++i) {
          const auto map_tile = map->get_tile_ptr(i);
          auto tile = ctx->get_entity<core::tile>(i);
          tile->center = map_tile->center;
          tile->height = map_tile->height;
          tile->texture = map_tile->texture;
          tile->color = map_tile->color;
          const uint32_t n_count = map_tile->neighbors[5] == UINT32_MAX ? 5 : 6;
          for (uint32_t i = 0; i < n_count; ++i) {
            tile->points[i] = map_tile->points[i];
            tile->neighbors[i] = map_tile->neighbors[i];
          }
          
          ASSERT(tile->neighbors_count() == n_count);
        }
      }
      
      {
        const size_t count = ctx->get_entity_count<core::city>();
        for (size_t i = 0; i < count; ++i) {
          auto city = ctx->get_entity<core::city>(i);
          ASSERT(city->title != nullptr);
//           ASSERT(city->title->count == 0);
//           city->title->create_children(1);
//           city->title->set_city(city);
          auto t = ctx->get_entity<core::titulus>(city->title->id);
          ASSERT(t->t == core::titulus::type::city);
          t->city = city;
          ASSERT(city->province != nullptr);
          ASSERT(city->province->cities_count < core::province::cities_max_game_count);
          if (city->province->cities == nullptr) city->province->cities = city;
          else utils::ring::list_radd<utils::list_type::province_cities>(city->province->cities, city);
//           city->province->cities[city->province->cities_count] = city;
          ++city->province->cities_count;
          if (city->province->capital == nullptr) city->province->capital = city;
          ASSERT(city->tile_index < core::map::hex_count_d(core::map::detail_level));
          tile_city_map[city->tile_index] = i;
        }
      }

      {
        const size_t count = ctx->get_entity_count<core::province>();
        for (size_t i = 0; i < count; ++i) {
          auto province = ctx->get_entity<core::province>(i);
          ASSERT(province->cities_count <= province->cities_max_count);
          ASSERT(province->title != nullptr);
//           ASSERT(province->title->count == 0);
//           province->title->create_children(1);
//           province->title->set_province(province);
          auto t = ctx->get_entity<core::titulus>(province->title->id);
          ASSERT(t->t == core::titulus::type::baron);
          t->province = province;
          ASSERT(!province->tiles.empty());
          for (const auto &tile_index : province->tiles) {
            auto itr = tile_city_map.find(tile_index);
            
            const auto tile = ctx->get_entity<core::tile>(tile_index);
            ASSERT(tile->province == UINT32_MAX);
            tile->city = itr == tile_city_map.end() ? UINT32_MAX : itr->second;;
            tile->province = i;
          }
        }
      }

      {
        const size_t count = ctx->get_entity_count<core::titulus>();
        std::unordered_map<core::titulus*, std::vector<core::titulus*>> childs;
        for (size_t i = 0; i < count; ++i) {
          auto title = ctx->get_entity<core::titulus>(i);
          if (title->parent != nullptr) childs[title->parent].push_back(title);
        }

        for (size_t i = 0; i < count; ++i) {
          auto title = ctx->get_entity<core::titulus>(i);
          auto itr = childs.find(title);
          if (itr == childs.end()) continue;
          const auto &childs_arr = itr->second;
          ASSERT(title->children == nullptr);
          ASSERT(!childs_arr.empty());
//           title->create_children(childs_arr.size());
//           if (childs_arr.size() == 1) {
//             title->set_child(0, childs_arr[0]);
//             continue;
//           }
          
          title->children = childs_arr[0];
          for (size_t j = 1; j < childs_arr.size(); ++j) {
            //title->set_child(j, childs_arr[j]);
            utils::ring::list_radd<utils::list_type::sibling_titles>(title->children, childs_arr[0]);
          }
        }
      }

      {
        const size_t count = ctx->characters_count();
        std::unordered_map<core::realm*, std::vector<core::realm*>> vassals;
        std::unordered_map<core::realm*, std::vector<core::character*>> prisoners;
        std::unordered_map<core::realm*, std::vector<core::character*>> court;
        std::unordered_map<core::character*, std::vector<core::character*>> concubines;
        std::unordered_map<core::character*, std::vector<core::character*>> children;
        for (size_t i = 0; i < count; ++i) {
          auto character = ctx->get_character(i);
          if (character->suzerain != nullptr) court[character->suzerain.get()].push_back(character);
          //if (character->imprisoner != nullptr) prisoners[character->imprisoner.get()].push_back(character);
          if (character->prison != nullptr) prisoners[character->prison.get()].push_back(character);
          if (character->self != nullptr && character->self->liege != nullptr) {
            vassals[character->self->liege.get()].push_back(character->self.get());
          }
          if (character->family.owner != nullptr) concubines[character->family.owner].push_back(character);
          // братья сестры, нужно выбрать кого то из родителей и положить туда? как быть с полуродственниками?
          // скорее всего несколько супругов может быть только у правителей
          // предыдущих супругов я кажется пока не задаю, тогда нужно найти правителя
//           ASSERT(character->family.previous_consorts == nullptr);
          const bool has_parent1 = character->family.parents[0] != nullptr;
          const bool has_parent2 = character->family.parents[1] != nullptr;
          core::character* parent = nullptr;
          if (has_parent1 || has_parent2) {
            if (parent == nullptr) parent = has_parent1 && has_parent2 && character->family.parents[0]->self != nullptr ? character->family.parents[0] : nullptr;
            if (parent == nullptr) parent = has_parent1 && has_parent2 && character->family.parents[1]->self != nullptr ? character->family.parents[1] : nullptr;
            if (parent == nullptr) parent = has_parent1 ? character->family.parents[0] : nullptr;
            if (parent == nullptr) parent = has_parent2 ? character->family.parents[1] : nullptr;
          }

          if (parent != nullptr) children[parent].push_back(character);
          
          // добавим всех знакомых
          for (const auto &pair : character->relations.acquaintances) {
            if (pair.first == nullptr) continue;
            const size_t t = pair.first->relations.get_acquaintance_raw(character);
            if (t != 0) continue;
            for (size_t i = 0; i < core::relationship::count; ++i) {
              const size_t mask = size_t(1) << i;
              const bool val = (t & mask) == mask;
              if (val) pair.first->relations.add_acquaintance(character, i);
            }
          }

          if (character->family.consort != nullptr) {
            auto consort = character->family.consort;
            ASSERT(consort->family.consort == nullptr || consort->family.consort == character);
            consort->family.consort = character;
          }
        }

        for (size_t i = 0; i < count; ++i) {
          auto character = ctx->get_character(i);
          // у персонажа может быть другая форма правления (но наверное в этом случае персонажи не будут считаться за вассалов)
          auto faction = character->self;
          if (faction == nullptr) continue;
          auto itr = vassals.find(faction.get());
          if (itr == vassals.end()) continue;
          const auto &vs = itr->second;
          for (size_t j = 0; j < vs.size(); ++j) {
            faction->add_vassal_raw(vs[j]);
          }
        }

        for (size_t i = 0; i < count; ++i) {
          auto character = ctx->get_character(i);
          auto faction = character->self;
          if (faction == nullptr) continue;
          auto itr = prisoners.find(faction.get());
          if (itr == prisoners.end()) continue;
          const auto &ps = itr->second;
          for (size_t j = 0; j < ps.size(); ++j) {
            faction->add_prisoner_raw(character);
          }
        }

        for (size_t i = 0; i < count; ++i) {
          auto character = ctx->get_character(i);
          auto faction = character->self;
          if (faction == nullptr) continue;
          auto itr = court.find(faction.get());
          if (itr == court.end()) continue;
          const auto &cs = itr->second;
          for (size_t j = 0; j < cs.size(); ++j) {
            faction->add_courtier_raw(cs[j]);
          }
        }

        for (size_t i = 0; i < count; ++i) {
          auto character = ctx->get_character(i);
          auto itr = concubines.find(character);
          if (itr == concubines.end()) continue;
          const auto &cs = itr->second;
          for (size_t j = 0; j < cs.size(); ++j) {
            ASSERT(character->is_male() != cs[j]->is_male());
            character->add_concubine_raw(cs[j]);
          }
        }

        for (size_t i = 0; i < count; ++i) {
          auto character = ctx->get_character(i);
          auto itr = children.find(character);
          if (itr == children.end()) continue;
          const auto &cs = itr->second;
          for (size_t j = 0; j < cs.size(); ++j) {
            character->add_child_raw(cs[j]);
          }

          if (character->family.consort != nullptr) {
            ASSERT(character->family.consort->family.children == nullptr);
            character->family.consort->family.children = character->family.children;
          }
        }
        
        for (size_t i = 0; i < count; ++i) {
          auto character = ctx->get_character(i);
          auto faction = character->self;
          if (faction == nullptr) continue;
          auto title = faction->titles;
          auto next_title = faction->titles;
          while (next_title != nullptr) {
            if (next_title->type() == core::titulus::type::city) {
              ASSERT(next_title->city != nullptr);
              // наверное лучше титул держать, и задавать его при генерации
              faction->set_capital(next_title->city);
              break;
            }
            next_title = utils::ring::list_next<utils::list_type::titles>(next_title, title);
          }
        }        
        
        for (size_t i = 0; i < count; ++i) {
          auto character = ctx->get_character(i);
          auto faction = character->self;
          if (faction == nullptr) continue;
          faction->sort_titles();
        }
      }
    }

    template <typename T>
    void create_entities_without_id(core::context* ctx, map::creator::table_container_t* tables) {
      const auto &data = tables->get_tables(static_cast<size_t>(T::s_type));
      ctx->create_container<T>(data.size());

      PRINT_VAR("cities count", data.size())
    }

    template <typename T>
    void create_entities(core::context* ctx, map::creator::table_container_t* tables) {
      // это заполнить в валидации не выйдет (потому что string_view)
      // в провинции нет id
//       auto to_data = global::get<utils::data_string_container>();

      const auto &data = tables->get_tables(static_cast<size_t>(T::s_type));
      ctx->create_container<T>(data.size());
      for (size_t i = 0; i < data.size(); ++i) {
        auto ptr = ctx->get_entity<T>(i);
        ptr->id = data[i]["id"];
//         const size_t index = to_data->get(ptr->id);
//         if (index != SIZE_MAX) throw std::runtime_error("Found duplicate id " + ptr->id + " while parsing " + std::string(magic_enum::enum_name<core::structure>(T::s_type)) + " type data");
//         to_data->insert(ptr->id, i);
      }
    }

    std::vector<sol::table> create_characters(core::context* ctx, map::creator::table_container_t* tables) {
      const auto &data = tables->get_tables(static_cast<size_t>(core::structure::character));
      ASSERT(!data.empty());
      for (const auto &table : data) {
        bool male = true;
        bool dead = false;

        if (const auto &proxy = table["male"]; proxy.valid()) {
          male = proxy.get<bool>();
        }

        if (const auto &proxy = table["dead"]; proxy.valid()) {
          dead = proxy.get<bool>();
        }

        auto c = ctx->create_character(male, dead);

        // нам нужно заполнить стейт рандомайзера
        // как это лучше всего сделать?
        // советуют использовать splitmix64
        // но с чем его использовать не говорят
        // мне нужно придумать как получить сид
        // по идее нужно задать некий игровой сид и от него отталкиваться
        // с другой стороны у меня локальные сиды на всех персах
        // да но они не будут меняться если я начинаю на тойже карте по нескольку раз
        // поэтому нужно придумать стейт глобальный
        
        // при загрузке сохранения стейты не очень полезны, но при этом не особ мешают, потому что должны быть заменены на данные из сохранения
        const size_t state1 = global::advance_state();
        const size_t state2 = global::advance_state();
        c->rng_state = {state1, state2};
        c->static_state = global::advance_state();
      }
      
      return data;
    }

    template <typename T>
    void parse_entities(core::context* ctx, map::creator::table_container_t* tables, const std::function<void(T*, const sol::table&)> &parsing_func) {
      const auto &data = tables->get_tables(static_cast<size_t>(T::s_type));
      for (size_t i = 0; i < data.size(); ++i) {
        auto ptr = ctx->get_entity<T>(i);
        parsing_func(ptr, data[i]);
      }
    }

    void parse_characters(core::context* ctx, map::creator::table_container_t* tables, const std::function<void(core::character*, const sol::table&)> &parsing_func) {
      const auto &data = tables->get_tables(static_cast<size_t>(core::character::s_type));
      for (size_t i = 0; i < data.size(); ++i) {
        auto ptr = ctx->get_character(i);
        parsing_func(ptr, data[i]);
      }
    }
    
//     void register_heraldy_data(utils::table_container* tables) {
//       auto to_data = global::get<utils::data_string_container>();
//       
//       const auto &datas = tables->get_tables(utils::table_container::additional_data::heraldy);
//       for (const auto &table : datas) {
//         
//       }
//     }

//     void validate_and_create_data(systems::map_t* map_systems, utils::progress_container* prog) { //systems::core_t* systems,
// //       auto ctx = map_systems->core_context;
//       auto creator = map_systems->map_creator;
// //       auto tables = &creator->table_container();
//       utils::data_string_container string_container;
//       global::get(&string_container);
//       utils::numeric_string_container heraldy_container;
//       global::get(&heraldy_container);
// 
//       //advance_progress(prog, "validating data"); // 1
// 
//       //if ()
// 
//       // создается сериализатор не здесь, а в креаторе
// //       utils::world_serializator cont;
// //       global::get(&cont);
//   //     const std::string_view test_name = "Test world 1\0";
//   //     const std::string_view test_tname = "test_world_1\0";
//   //     const std::string_view test_settings = "{}\0";
//       auto cont = creator->serializator_ptr();
//       cont->set_name(creator->get_world_name());
//       cont->set_technical_name(creator->get_folder_name());
//       cont->set_settings(creator->get_settings());
//       cont->set_rand_seed(creator->get_rand_seed());
//       cont->set_noise_seed(creator->get_noise_seed());
// 
// //       const std::function<bool(const size_t &, sol::this_state, const sol::table&, utils::world_serializator*)> validation_funcs[] = {
// //         nullptr,                   // tile    : нужна ли тайлу валидация? я не уверен что хорошей идеей будет использовать луа таблицы для заполнения тайла
// //         utils::validate_province_and_save,  // province
// //         utils::validate_building_and_save,  // building_type,
// //         utils::validate_city_type_and_save, // city_type,
// //         utils::validate_city_and_save,      // city,
// //         nullptr,                   // trait,
// //         nullptr,                   // modificator,
// //         nullptr,                   // troop_type,
// //         nullptr,                   // decision,
// //         nullptr,                   // religion_group,
// //         nullptr,                   // religion,
// //         nullptr,                   // culture,
// //         nullptr,                   // law,
// //         nullptr,                   // event,
// //         utils::validate_title_and_save,     // titulus,
// //         utils::validate_character_and_save, // character,
// //         nullptr,                   // dynasty,
// //         nullptr,                   // faction,    // это и далее делать не нужно по идее
// //         nullptr,                   // hero_troop,
// //         nullptr,                   // army,
// // 
// //       };
// // 
// //       global::get<utils::calendar>()->validate();
// // 
// //       auto &lua = creator->state();
// //       //ASSERT(lua != nullptr);
// //       
// //       // тут можно вызвать функцию лоад ворлд, ее естественно нужно немного переделать
// //       // данные мы уже распарсили и положили в ворлд дата, тут сократиться примерно 70% функции
// // 
// //   //     auto &tables = creator->table_container();
// //       const size_t count = static_cast<size_t>(core::structure::count);
// //       bool ret = true;
// //       for (size_t i = 0; i < count; ++i) {
// //         if (!validation_funcs[i]) continue;
// //         //const auto &data = tables->get_tables(static_cast<core::structure>(i));
// //         const auto &data = tables->get_tables(i);
// //         size_t counter = 0;
// //         for (const auto &table : data) {
// //           ret = ret && validation_funcs[i](counter, lua.lua_state(), table, &cont);
// //           ++counter;
// //         }
// //       }
// //       
// //       //const auto &image_data = tables->get_tables(utils::table_container::additional_data::image);
// //       const auto &image_data = tables->get_tables(static_cast<size_t>(utils::generator_table_container::additional_data::image));
// //       size_t counter = 0;
// //       for (const auto &table : image_data) {
// //         ret = ret && utils::validate_image_and_save(counter, lua.lua_state(), table, &cont);
// //         ++counter;
// //       }
// //       
// //       //const auto &heraldy_data = tables->get_tables(utils::table_container::additional_data::heraldy);
// //       const auto &heraldy_data = tables->get_tables(static_cast<size_t>(utils::generator_table_container::additional_data::heraldy));
// //       for (size_t i = 0; i < heraldy_data.size(); ++i) {
// //         const auto &table = heraldy_data[i];
// //         ret = ret && utils::validate_heraldy_layer_and_save(i, lua.lua_state(), table, &cont);
// //       }
// //       
// //       if (!ret) throw std::runtime_error("There is validation errors");
// //       
// //       auto device = global::get<systems::core_t>()->graphics_container->device;
// //       
// //       auto controller = global::get<systems::core_t>()->image_controller;
// //       global::get(controller);
// //       for (size_t i = 0; i < static_cast<size_t>(render::image_controller::image_type::count); ++i) {
// //         //utils::load_images(controller, image_data, static_cast<uint32_t>(render::image_controller::image_type::system));
// //         utils::load_images(controller, image_data, i);
// //       }
// //       //utils::load_biomes(controller, map_systems->seasons, tables->get_tables(utils::table_container::additional_data::biome));
// //       utils::load_biomes(controller, map_systems->seasons, tables->get_tables(static_cast<size_t>(utils::generator_table_container::additional_data::biome)));
// //       yavf::Buffer heraldy_tmp(device, yavf::BufferCreateInfo::buffer(16, VK_BUFFER_USAGE_TRANSFER_SRC_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
// //       utils::load_heraldy_layers(controller, heraldy_data, &heraldy_tmp);
// //       
// //       const auto &data = get_season_biomes_data(map_systems);
// //       
// //       map_systems->lock_map();
// //       controller->update_set();
// //       map_systems->map->copy_biomes(map_systems->seasons);
// //       global::get<render::tile_optimizer>()->set_biome_tile_count(data);
// //       map_systems->unlock_map();
// //       
// //       map_systems->map->set_tile_biome(map_systems->seasons);
// // 
// //       advance_progress(prog, "allocating memory"); // 2
// // 
// //       // нужно собрать инфу о дубликатах
// //       create_entities_without_id<core::province>(ctx, tables);
// //       create_entities<core::building_type>(ctx, tables);
// //       create_entities<core::city_type>(ctx, tables);
// //       create_entities_without_id<core::city>(ctx, tables);
// //       create_entities<core::titulus>(ctx, tables);
// //       create_characters(ctx, tables);
// // 
// //       advance_progress(prog, "creating entities"); // 3
// // 
// //       global::get(ctx);
// // 
// //       parse_entities<core::titulus>(ctx, tables, utils::parse_title);
// //       parse_entities<core::province>(ctx, tables, utils::parse_province);
// //       parse_entities<core::building_type>(ctx, tables, utils::parse_building);
// //       parse_entities<core::city_type>(ctx, tables, utils::parse_city_type);
// //       parse_entities<core::city>(ctx, tables, utils::parse_city);
// //       parse_characters(ctx, tables, utils::parse_character);
// //       parse_characters(ctx, tables, utils::parse_character_goverment);
// //       // тут нужно еще соединить все полученные данные друг с другом
// //       connect_game_data(map_systems->map, ctx);
// // 
// //       // по идее в этой точке все игровые объекты созданы
// //       // и можно непосредственно переходить к геймплею
// //       // если валидация и парсинг успешны это повод сохранить мир на диск
// //       // это означает: сериализация данных карты + записать на диск все таблицы + сериализация персонажей и династий (первых)
// //       // могу ли я сериализовать конкретные типы? скорее да чем нет,
// //       // но при этом мне придется делать отдельный сериализатор для каждого типа
// //       // понятное дело делать отдельный сериализатор не сруки
// //       
// //       {
// //         auto buffers = global::get<render::buffers>();
// //         const size_t structures_count = ctx->get_entity_count<core::city>();
// //         const size_t structure_data_size = ctx->get_entity_count<core::city_type>() * sizeof(render::world_structure_t);
// //         yavf::Buffer buf(device, yavf::BufferCreateInfo::buffer(structure_data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
// //         auto map = map_systems->map;
// //         map_systems->lock_map();
// //         copy_structure_data(ctx, 0, map, &buf);
// //         map->structures->resize(structure_data_size);
// //         buffers->heraldy->resize(heraldy_tmp.info().size);
// //         
// //         auto task = device->allocateTransferTask();
// //         task->begin();
// //         task->copy(&buf, map->structures);
// //         task->copy(&heraldy_tmp, buffers->heraldy);
// //         task->end();
// //         task->start();
// //         task->wait();
// //         device->deallocate(task);
// //         
// //         global::get<render::tile_optimizer>()->set_max_structures_count(structures_count);
// //         global::get<render::tile_optimizer>()->set_max_heraldy_count(structures_count);
// //         map->flush_structures();
// //         map_systems->unlock_map();
// //       }
//       
//       advance_progress(prog, "serializing world"); // 1
// 
//       cont->copy_seasons(map_systems->seasons);
//       cont->set_world_matrix(map_systems->map->world_matrix);
// 
//       for (size_t i = 0; i < core::map::hex_count_d(core::map::detail_level); ++i) {
//         //const auto &d = ctx->get_tile(i);
//         //cont->set_tile_data(i, {d.height, d.province}); // тут даже по идее ничего делать особо не нужно
//         
//         //counter += size_t(d.province != UINT32_MAX);
//         auto tile = map_systems->map->get_tile_ptr(i);
//         cont->set_tile_data(i, {tile->height, UINT32_MAX});
//       }
// 
//       cont->serialize(); //
//       
//       // вот это по идее не нужно, у меня грузится это все в loading_world
//       map_systems->world_name = cont->get_name();
//       map_systems->folder_name = cont->get_technical_name();
//       map_systems->generator_settings = cont->get_settings();
//       memcpy(map_systems->hash, cont->get_hash(), systems::map_t::hash_size);
//       
//       // тут нам не нужно создавать данные карты заново
//       loading_world(map_systems, prog, cont, false);
// 
//       advance_progress(prog, "checking world"); // 9
// 
//       utils::world_serializator test;
//       test.deserialize(global::root_directory() + "saves/" + creator->get_folder_name() + "/world_data");
// 
//       ASSERT(test.get_name() == creator->get_world_name());
//       ASSERT(test.get_technical_name() == creator->get_folder_name());
//       ASSERT(test.get_settings() == creator->get_settings());
//       ASSERT(test.get_rand_seed() == creator->get_rand_seed());
//       ASSERT(test.get_noise_seed() == creator->get_noise_seed());
// 
// //       ASSERT(tables->get_tables(utils::world_serializator::province).size() == cont.get_data_count(utils::world_serializator::province));
// //       ASSERT(tables->get_tables(utils::world_serializator::city_type).size() == cont.get_data_count(utils::world_serializator::city_type));
// //       ASSERT(tables->get_tables(utils::world_serializator::city).size() == cont.get_data_count(utils::world_serializator::city));
// //       ASSERT(tables->get_tables(utils::world_serializator::building_type).size() == cont.get_data_count(utils::world_serializator::building_type));
// //       ASSERT(tables->get_tables(utils::world_serializator::title).size() == cont.get_data_count(utils::world_serializator::title));
// //       ASSERT(tables->get_tables(utils::world_serializator::character).size() == cont.get_data_count(utils::world_serializator::character));
// 
//   //     PRINT_VAR("size", tables->get_tables(core::structure::province).size())
//   //     PRINT_VAR("size", tables->get_tables(core::structure::city_type).size())
//   //     PRINT_VAR("size", tables->get_tables(core::structure::city).size())
//   //     PRINT_VAR("size", tables->get_tables(core::structure::building_type).size())
//   //     PRINT_VAR("size", tables->get_tables(core::structure::titulus).size())
//   //     PRINT_VAR("size", tables->get_tables(core::structure::character).size())
//   //
//   //     PRINT_VAR("size", cont.get_data_count(core::structure::province))
//   //     PRINT_VAR("size", cont.get_data_count(core::structure::city_type))
//   //     PRINT_VAR("size", cont.get_data_count(core::structure::city))
//   //     PRINT_VAR("size", cont.get_data_count(core::structure::building_type))
//   //     PRINT_VAR("size", cont.get_data_count(core::structure::titulus))
//   //     PRINT_VAR("size", cont.get_data_count(core::structure::character))
// 
//       ASSERT(cont->get_data_count(utils::world_serializator::province) == test.get_data_count(utils::world_serializator::province));
//       ASSERT(cont->get_data_count(utils::world_serializator::city_type) == test.get_data_count(utils::world_serializator::city_type));
//       ASSERT(cont->get_data_count(utils::world_serializator::city) == test.get_data_count(utils::world_serializator::city));
//       ASSERT(cont->get_data_count(utils::world_serializator::building_type) == test.get_data_count(utils::world_serializator::building_type));
//       ASSERT(cont->get_data_count(utils::world_serializator::title) == test.get_data_count(utils::world_serializator::title));
//       ASSERT(cont->get_data_count(utils::world_serializator::character) == test.get_data_count(utils::world_serializator::character));
// 
//       //PRINT(cont.get_data(var, i))
//   #define CHECK_CONTENT(var) for (uint32_t i = 0; i < cont->get_data_count(var); ++i) { ASSERT(cont->get_data(var, i) == test.get_data(var, i)); }
//       CHECK_CONTENT(utils::world_serializator::province)
//       CHECK_CONTENT(utils::world_serializator::city_type)
//       CHECK_CONTENT(utils::world_serializator::city)
//       CHECK_CONTENT(utils::world_serializator::building_type)
//       CHECK_CONTENT(utils::world_serializator::title)
//       CHECK_CONTENT(utils::world_serializator::character)
// 
//       // кажется все правильно сериализуется и десериализуется
//       // сохранения должны храниться в папках с файлом world_data
//       // название папки - техническое название мира,
//       // техническое название мира - нужно зафорсить пользователя задать валидное техническое название
//       // (какнибудь бы упростить для пользователя это дело)
//       // что самое главное я могу оставить пока так как есть и это покроет почти всю сериализацию
//       // остается решить вопрос с климатом, хотя че тут решать: нужно выделить
//       // 64*500к или 128*500к памяти чтобы сохранить сезоны, сезонов по идее не имеет смысла делать больше чем размер массива на каждый тайл
//       // соответственно 64 сезона на каждый тайл, означает ли это что мы можем сохранить 4 тайла в одном чаре?
//       // в сохранениях видимо придется делать протомессадж для каждой сущности которую необходимо сохранить
//       // мне нужно еще придумать стендалоне сохранения: то есть сохранения в которых записаны данные мира дополнительно
//       //
// 
//       global::get(reinterpret_cast<utils::data_string_container*>(SIZE_MAX));
//       global::get(reinterpret_cast<utils::numeric_string_container*>(SIZE_MAX));
//       global::get(reinterpret_cast<core::context*>(SIZE_MAX));
//       global::get(reinterpret_cast<render::image_controller*>(SIZE_MAX));
//       map_systems->render_modes->at(render::modes::biome)();
//     }
// 
//   //   void create_interface(system_container_t &systems) {
//   //     systems.interface = systems.container.create<utils::interface>();
//   //     global::get(systems.interface);
//   //     systems.interface->init_constants();
//   //     systems.interface->init_input();
//   //   }

    void post_generation_work(systems::map_t* map_systems, systems::core_t* systems, utils::progress_container* prog) {
//       UNUSED_VARIABLE(systems);
//       //prog->set_hint1(std::string_view("Load map"));
//       //prog->set_max_value(8);
//       prog->set_hint2(std::string_view("starting"));
// 
//       auto ctx = map_systems->core_context;
// 
//   //     create_game_state();
//       validate_and_create_data(map_systems, prog); // создаем объекты
//       advance_progress(prog, "connecting tiles"); // 10
//       //generate_tile_connections(map_systems->map, global::get<dt::thread_pool>());
//       advance_progress(prog, "making borders"); // 11
//       find_border_points(map_systems->map, ctx); // после генерации нужно сделать много вещей
//       // по идее создать границы нужно сейчас, так как в титулах появились данные о цвете
//       
//       make_random_player_character();
// 
//       advance_progress(prog, "ending"); // 12

      //create_interface(systems);
  //     systems.interface->init_types();
  //     systems.interface->init_game_logic();
  //     auto &lua = systems.interface->get_state();
  //     load_interface_functions(systems.interface, lua);
  //     create_ai_systems(systems);
      // нужно выбрать себе какого нибудь персонажа
      // кажется у меня сейчас все персонажи живы, так что можно любого
    }

    void load_map_data(core::map* map, const utils::world_serializator* world, const bool make_tiles) {
      glm::mat4 mat1 = world->get_world_matrix();
  //     PRINT_VEC4("mat 0", mat1[0])
  //     PRINT_VEC4("mat 1", mat1[1])
  //     PRINT_VEC4("mat 2", mat1[2])
  //     PRINT_VEC4("mat 3", mat1[3])
      map->world_matrix = mat1;
      auto pool = global::get<dt::thread_pool>();
      
      if (make_tiles) core::make_tiles(mat1, map, pool);
      // данные в карте пока еще лежат в памяти хоста

  //       ctx->container->set_entity_count(debug::entities::tile, map->tiles_count());
      map->set_status(core::map::status::valid);

      for (size_t i = 0; i < core::map::hex_count_d(core::map::detail_level); ++i) {
        map->set_tile_height(i, world->get_tile_data(i).height);
      }
    }
    
    void parse_script_path(const std::string_view &path, std::string_view &mod_name, std::string_view &script_path) {
      const size_t index = path.find('/');
      if (index != std::string_view::npos) {
        mod_name = path.substr(0, index);
        script_path = path.substr(index+1, std::string_view::npos);
      }
    }
    
    std::vector<sol::table> get_data_tables(
      const utils::world_serializator* w, 
      sol::state_view state, 
      const sol::environment &env,
      const uint32_t &type, 
      const std::string_view &type_str, 
      const std::function<bool(const uint32_t&,const sol::table&)> validation_func
    ) {
      std::vector<sol::table> tables;
      tables.reserve(w->get_data_count(type)*2);
      for (size_t i = 0; i < w->get_data_count(type); ++i) {
        //auto ret = state.script("return " + std::string(w->get_data(type, i)), env);
//         if (type == utils::world_serializator::province) {
//           const auto str = w->get_data(type, i);
//           PRINT_VAR("province " + std::to_string(i), str)
//         }
        auto ret = state.script(w->get_data(type, i), env, sol::detail::default_chunk_name(), sol::load_mode::text); // в виде скрипта: do local _={ данные };return _;end
        if (!ret.valid()) {
          sol::error err = ret;
          std::cout << err.what();
          throw std::runtime_error("Could not load " + std::string(type_str) + " table");
        }
        
        sol::table t = ret;
        // тут нам было бы неплохо проверить что это за таблица
        const bool table_string = t[1].valid() && t[1].get_type() == sol::type::string && t.size() == 1;
        if (table_string) { // таблица-путь, подгрузим скрипт
          if (type == utils::world_serializator::biome) throw std::runtime_error("Removed biome table scripts");
          
          const std::string path = t[1];
          // путь вида "*название мода*/*путь до скрипта*"
          std::string_view mod_name;
          std::string_view script_path;
          parse_script_path(path, mod_name, script_path);
          if (mod_name.empty() || script_path.empty()) throw std::runtime_error("Path " + path + " is not a valid script path");
          // проверим есть ли такой мод, и проверим есть ли такой файл в моде
          // еще нужно проверить является ли этот путь относительным
          ASSERT(mod_name == "apates_quest");
          const std::string final_script_path = global::root_directory() + "/" + std::string(script_path);
          // нужно еще указать энвайронмент, хотя с другой стороны, возможно просто в самом стейте отменить некоторые функции
          // но в любом случае нужно еще погрузить минимально необходимый набор функций
          auto ret = state.script_file(final_script_path, env, sol::load_mode::text);
          if (!ret.valid()) {
            sol::error err = ret;
            std::cout << err.what();
            throw std::runtime_error("Could not load " + std::string(type_str) + " data file " + final_script_path);
          }
          
          size_t counter = 0;
          sol::table data_tables = ret;
          for (const auto &obj : data_tables) {
            if (obj.second.get_type() != sol::type::table) continue;
            
            sol::table final_t = obj.second.as<sol::table>();
            const bool valid = validation_func(counter, final_t);
            if (!valid) {
              std::string id;
              if (const auto proxy = final_t["id"]; proxy.valid()) id = proxy.get<std::string>();
              
              if (id.empty()) throw std::runtime_error("Invalid " + std::string(type_str) + " data table in script " + path);
              else throw std::runtime_error("Invalid " + std::string(type_str) + " data table with id " + id + " in script " + path);
            }
            ++counter;
            tables.push_back(final_t);
          }
        } else tables.push_back(t); // сгенерированная таблица данных
      }
      
      return tables;
    }
    
    std::vector<sol::table> get_data_tables_from_sequential_array(
      const utils::world_serializator* w, 
      sol::state_view state, 
      const sol::environment &env,
      const uint32_t &type, 
      const std::string_view &type_str, 
      const std::function<bool(const uint32_t&,const sol::table&)> validation_func
    ) {
      std::vector<sol::table> tables;
      tables.reserve(w->get_data_count(type)*2);
      for (size_t i = 0; i < w->get_data_count(type); ++i) {
        //auto ret = state.script("return " + std::string(w->get_data(type, i)), env);
//         if (type == utils::world_serializator::province) {
//           const auto str = w->get_data(type, i);
//           PRINT_VAR("province " + std::to_string(i), str)
//         }
        auto ret = state.script(w->get_data(type, i), env, sol::detail::default_chunk_name(), sol::load_mode::text); // в виде скрипта: do local _={ данные };return _;end
        if (!ret.valid()) {
          sol::error err = ret;
          std::cout << err.what();
          throw std::runtime_error("Could not load " + std::string(type_str) + " table");
        }
        
        sol::table t = ret;
        // тут нам было бы неплохо проверить что это за таблица
        const bool table_string = t[1].valid() && t[1].get_type() == sol::type::string && t.size() == 1;
        if (table_string) { // таблица-путь, подгрузим скрипт
          // кажется какое то старое ограничение, теперь не нужно
          //if (type == utils::world_serializator::biome) throw std::runtime_error("Removed biome table scripts");
          
          const std::string path = t[1];
          // путь вида "*название мода*/*путь до скрипта*"
          std::string_view mod_name;
          std::string_view script_path;
          parse_script_path(path, mod_name, script_path);
          if (mod_name.empty() || script_path.empty()) throw std::runtime_error("Path " + path + " is not a valid script path");
          // проверим есть ли такой мод, и проверим есть ли такой файл в моде
          // еще нужно проверить является ли этот путь относительным
          ASSERT(mod_name == "apates_quest");
          const std::string final_script_path = global::root_directory() + "/" + std::string(script_path);
          // нужно еще указать энвайронмент, хотя с другой стороны, возможно просто в самом стейте отменить некоторые функции
          // но в любом случае нужно еще погрузить минимально необходимый набор функций
          auto ret = state.script_file(final_script_path, env, sol::load_mode::text);
          if (!ret.valid()) {
            sol::error err = ret;
            std::cout << err.what();
            throw std::runtime_error("Could not load " + std::string(type_str) + " data file " + final_script_path);
          }
          
          size_t counter = 0;
          sol::table data_tables = ret;
          for (const auto &obj : data_tables) {
            //if (obj.first.get_type() != sol::type::number) continue;
            if (obj.second.get_type() != sol::type::table) continue;
            
            if (obj.first.get_type() != sol::type::number) {
              const size_t cur_index = obj.first.as<double>();
              if (cur_index != counter+1) throw std::runtime_error("Biomes table indices are not sequential. Use string keys to store additional data");
            }
            
            sol::table final_t = obj.second.as<sol::table>();
            const bool valid = validation_func(counter, final_t);
            if (!valid) {
              std::string id;
              if (const auto proxy = final_t["id"]; proxy.valid()) id = proxy.get<std::string>();
              
              if (id.empty()) throw std::runtime_error("Invalid " + std::string(type_str) + " data table in script " + path);
              else throw std::runtime_error("Invalid " + std::string(type_str) + " data table with id " + id + " in script " + path);
            }
            ++counter;
            tables.push_back(final_t);
          }
        } else tables.push_back(t); // сгенерированная таблица данных
      }
      
      return tables;
    }
    
    std::vector<sol::table> get_heraldy_data_tables(
      const utils::world_serializator* w, 
      core::internal_lua_state* internal, 
      const std::function<bool(const uint32_t&,const sol::table&)> validation_func
    ) {
      const uint32_t type = utils::world_serializator::heraldy_layer;
      std::vector<sol::table> tables;
      tables.reserve(w->get_data_count(type)*2);
      
      for (size_t i = 0; i < w->get_data_count(type); ++i) {  
        //auto ret = internal->lua.script("return " + std::string(w->get_data(type, i)), internal->env);
        auto ret = internal->lua.script(w->get_data(type, i), internal->env, sol::detail::default_chunk_name(), sol::load_mode::text); // в виде скрипта: do local _={ данные };return _;end
        if (!ret.valid()) {
          sol::error err = ret;
          std::cout << err.what();
          throw std::runtime_error("Could not load heraldy table");
        }
        
        sol::table t = ret;
        // тут нам было бы неплохо проверить что это за таблица
        const bool table_string = t[1].valid() && t[1].get_type() == sol::type::string && t.size() == 1;
        if (table_string) { // таблица-путь, подгрузим скрипт
          if (type == utils::world_serializator::biome) throw std::runtime_error("Removed biome table scripts");
          
          const std::string path = t[1];
          // путь вида "*название мода*/*путь до скрипта*"
          std::string_view mod_name;
          std::string_view script_path;
          parse_script_path(path, mod_name, script_path);
          if (mod_name.empty() || script_path.empty()) throw std::runtime_error("Path " + path + " is not a valid script path");
          // проверим есть ли такой мод, и проверим есть ли такой файл в моде
          // еще нужно проверить является ли этот путь относительным
          ASSERT(mod_name == "apates_quest");
          const std::string final_script_path = global::root_directory() + "/" + std::string(script_path);
          // нужно еще указать энвайронмент, хотя с другой стороны, возможно просто в самом стейте отменить некоторые функции
          // но в любом случае нужно еще погрузить минимально необходимый набор функций
          auto ret = internal->lua.script_file(final_script_path, internal->env, sol::load_mode::text);
          if (!ret.valid()) {
            sol::error err = ret;
            std::cout << err.what();
            throw std::runtime_error("Could not load heraldy data file " + final_script_path);
          }
          
          size_t counter = 0;
          sol::table data_tables = ret;
          for (const auto &pair : data_tables) {
            if (pair.first.get_type() == sol::type::number) {
              if (pair.second.get_type() != sol::type::table) 
                throw std::runtime_error("Heraldy layers data expected to be in array part of the table");
              
              // еще одна вложенность отсутствует
              const auto h_t = pair.second.as<sol::table>();
              const bool valid = validation_func(counter, h_t);
              if (!valid) {
                std::string id;
                if (const auto proxy = h_t["id"]; proxy.valid()) id = proxy.get<std::string>();
                
                if (id.empty()) throw std::runtime_error("Invalid heraldy data table in script " + path);
                else throw std::runtime_error("Invalid heraldy data table with id " + id + " in script " + path);
              }
              ++counter;
              tables.push_back(h_t);
              continue;
            }
            
            if (pair.first.get_type() == sol::type::string) {
              if (pair.second.get_type() != sol::type::function) 
                 throw std::runtime_error("Heraldy gen functions expected to be in dictionary part of the table");
              const auto name = pair.first.as<std::string>();
              const auto func = pair.second.as<sol::function>();
              const auto proxy = internal->gen_funcs_table[name];
              if (proxy.valid()) throw std::runtime_error("Heraldy gen function " + name + " is already exists");
              internal->gen_funcs_table[name] = func;
              continue;
            }
          }
        } else {
          for (const auto &pair : t) {
            if (pair.first.get_type() == sol::type::number) {
              if (pair.second.get_type() != sol::type::table) 
                throw std::runtime_error("Heraldy layers data expected to be in array part of the table");
              // тут еще одно вложение должно быть, или нет, зачем оно нужно?
              
              const auto h_t = pair.second.as<sol::table>();
              tables.push_back(h_t);
              continue;
            }
            
            if (pair.first.get_type() == sol::type::string) {
              if (pair.second.get_type() != sol::type::function) 
                 throw std::runtime_error("Heraldy gen functions expected to be in dictionary part of the table");
              const auto name = pair.first.as<std::string>();
              const auto func = pair.second.as<sol::function>();
              const auto proxy = internal->gen_funcs_table[name];
              if (proxy.valid()) throw std::runtime_error("Heraldy gen function " + name + " is already exists");
              internal->gen_funcs_table[name] = func;
              continue;
            }
          }
        }
      }
      
      return tables;
    }

//     template <typename T>
//     void create_entity_without_id(core::context* ctx, const utils::world_serializator* world, const uint32_t &world_type) {
//       ctx->create_container<T>(world->get_data_count(world_type));
//     }

//     template <typename T>
//     void create_entity(core::context* ctx, const utils::world_serializator* world, sol::state_view tmp_state, const uint32_t &world_type, utils::data_string_container* to_data) {
//       ctx->create_container<T>(world->get_data_count(world_type));
//       for (size_t i = 0; i < world->get_data_count(world_type); ++i) {
//         auto ret = tmp_state.script("return " + std::string(world->get_data(world_type, i)));
//         if (!ret.valid()) {
//           sol::error err = ret;
//           std::cout << err.what();
//           throw std::runtime_error("Could not load entity table");
//         }
//         sol::table t = ret;
//         auto proxy = t["id"];
//         ASSERT(proxy.valid());
// 
//         std::string_view str = proxy.get<std::string_view>();
//         auto ptr = ctx->get_entity<T>(i);
//         ptr->id = str;
//         to_data->insert(ptr->id, i);
//       }
//     }
    
//     template <typename T>
//     void create_entity_without_id(
//       core::context* ctx, 
//       const utils::world_serializator* world, 
//       sol::state_view state, 
//       const sol::environment &env,
//       const uint32_t &world_type, 
//       const std::string_view &world_type_str,
//       const std::function<bool(const uint32_t&,const sol::table&)> validation_func
//     ) {
//       // с новым типом таблиц придется парсить сначала все таблицы, потом создавать энтити
//       // потом опять парсить, и ужа зполнять данными, ну тип это все равно загрузка
//       
//       const auto &tables = get_data_tables(world, state, env, world_type, world_type_str, validation_func);
//       ctx->create_container<T>(tables.size());
//     }
//     
//     template <typename T>
//     void create_entity(
//       core::context* ctx, 
//       utils::data_string_container* to_data, 
//       const utils::world_serializator* world, 
//       sol::state_view state,
//       const sol::environment &env,
//       const uint32_t &world_type, 
//       const std::string_view &world_type_str,
//       const std::function<bool(const uint32_t&,const sol::table&)> validation_func
//     ) {
//       const auto &tables = get_data_tables(world, state, env, world_type, world_type_str, validation_func);
//       ctx->create_container<T>(tables.size());
//       for (size_t i = 0; i < tables.size(); ++i) {
//         const sol::table &t = tables[i];
//         auto proxy = t["id"];
//         ASSERT(proxy.valid());
// 
//         std::string_view str = proxy.get<std::string_view>();
//         auto ptr = ctx->get_entity<T>(i);
//         ptr->id = str;
//         to_data->insert(ptr->id, i);
//       }
//     }
    
    template <typename T>
    static std::vector<sol::table> create_entity_without_id(
      core::context* ctx, 
      const utils::world_serializator* world, 
      sol::state_view state, 
      const sol::environment &env,
      const uint32_t &world_type, 
      const std::string_view &world_type_str,
      const std::function<bool(const uint32_t&,const sol::table&)> validation_func
    ) {
      // с новым типом таблиц придется парсить сначала все таблицы, потом создавать энтити
      // потом опять парсить, и ужа зполнять данными, ну тип это все равно загрузка
      
      const auto &tables = get_data_tables(world, state, env, world_type, world_type_str, validation_func);
      ctx->create_container<T>(tables.size());
      return tables;
    }
    
    template <typename T>
    static std::vector<sol::table> create_entity(
      core::context* ctx, 
      utils::data_string_container* to_data, 
      const utils::world_serializator* world, 
      sol::state_view state,
      const sol::environment &env,
      const uint32_t &world_type, 
      const std::string_view &world_type_str,
      const std::function<bool(const uint32_t&,const sol::table&)> validation_func
    ) {
      const auto &tables = get_data_tables(world, state, env, world_type, world_type_str, validation_func);
      ctx->create_container<T>(tables.size());
      for (size_t i = 0; i < tables.size(); ++i) {
        const sol::table &t = tables[i];
        auto proxy = t["id"];
        ASSERT(proxy.valid());

        std::string_view str = proxy.get<std::string_view>();
        auto ptr = ctx->get_entity<T>(i);
        ptr->id = str;
        to_data->insert(ptr->id, i);
      }
      
      return tables;
    }

//     void create_characters(core::context* ctx, const utils::world_serializator* world, sol::state_view tmp_state) {
//       const size_t count = world->get_data_count(utils::world_serializator::character);
//       for (size_t i = 0; i < count; ++i) {
//         auto ret = tmp_state.script("return " + std::string(world->get_data(utils::world_serializator::character, i)));
//         if (!ret.valid()) {
//           sol::error err = ret;
//           std::cout << err.what();
//           throw std::runtime_error("Could not load entity table");
//         }
//         sol::table t = ret;
// 
//         bool male = true;
//         bool dead = false;
// 
//         if (const auto &proxy = t["male"]; proxy.valid()) {
//           male = proxy.get<bool>();
//         }
// 
//         if (const auto &proxy = t["dead"]; proxy.valid()) {
//           dead = proxy.get<bool>();
//         }
// 
//         auto c = ctx->create_character(male, dead);
// 
//         // нам нужно заполнить стейт рандомайзера
//         // нужно проконтролировать какой глобальный стейт мы используем
//         const size_t state1 = global::advance_state();
//         const size_t state2 = global::advance_state();
//         c->rng_state = {state1, state2};
//       }
//     }

    std::vector<sol::table> create_characters(core::context* ctx, const utils::world_serializator* world, sol::state_view state, const sol::environment &env) {
      const auto &tables = get_data_tables(world, state, env, utils::world_serializator::character, "character", core::validate_character);
      for (size_t i = 0; i < tables.size(); ++i) {
        const sol::table &t = tables[i];

        bool male = true;
        bool dead = false;

        if (const auto &proxy = t["male"]; proxy.valid()) {
          male = proxy.get<bool>();
        }

        if (const auto &proxy = t["dead"]; proxy.valid()) {
          dead = proxy.get<bool>();
        }

        auto c = ctx->create_character(male, dead);

        // нам нужно заполнить стейт рандомайзера
        // нужно проконтролировать какой глобальный стейт мы используем
        // скорее здесь нужно использовать какой то локальный генератор
        const size_t state1 = global::advance_state();
        const size_t state2 = global::advance_state();
        const size_t state3 = global::advance_state();
        const size_t state4 = global::advance_state();
        c->rng_state = {state1, state2, state3, state4};
        c->static_state = global::advance_state();
      }
      
      return tables;
    }

//     template <typename T>
//     void parse_entities(core::context* ctx, const utils::world_serializator* world, sol::state_view tmp_state, const uint32_t &world_type, const std::function<void(T*, const sol::table&)> &parsing_func) {
//       const size_t count = world->get_data_count(world_type);
//       for (size_t i = 0; i < count; ++i) {
//         auto ret = tmp_state.script("return " + std::string(world->get_data(world_type, i)));
//         if (!ret.valid()) {
//           sol::error err = ret;
//           std::cout << err.what();
//           throw std::runtime_error("Could not load entity table");
//         }
//         sol::table t = ret;
//         auto ptr = ctx->get_entity<T>(i);
// 
//         parsing_func(ptr, t);
//       }
//     }
// 
//     void parse_character(core::context* ctx, const utils::world_serializator* world, sol::state_view tmp_state, const std::function<void(core::character*, const sol::table&)> &parsing_func) {
//       const size_t count = world->get_data_count(utils::world_serializator::character);
//       for (size_t i = 0; i < count; ++i) {
//         auto ret = tmp_state.script("return " + std::string(world->get_data(utils::world_serializator::character, i)));
//         if (!ret.valid()) {
//           sol::error err = ret;
//           std::cout << err.what();
//           throw std::runtime_error("Could not load entity table");
//         }
//         sol::table t = ret;
//         auto ptr = ctx->get_character(i);
// 
//         parsing_func(ptr, t);
//       }
//     }

    template <typename T>
    void parse_entities(
      core::context* ctx, 
      const utils::world_serializator* world, 
      sol::state_view state,
      const sol::environment &env,
      const uint32_t &world_type, 
      const std::string_view &world_type_str,
      const std::function<bool(const uint32_t&,const sol::table&)> validation_func, 
      const std::function<void(T*, const sol::table&)> &parsing_func
    ) {
      const auto &tables = get_data_tables(world, state, env, world_type, world_type_str, validation_func);
      for (size_t i = 0; i < tables.size(); ++i) {
        const sol::table &t = tables[i];
        auto ptr = ctx->get_entity<T>(i);
        parsing_func(ptr, t);
      }
    }

    void parse_character(core::context* ctx, const utils::world_serializator* world, sol::state_view state, const sol::environment &env) {
      const auto &tables = get_data_tables(world, state, env, utils::world_serializator::character, "character", core::validate_character);
      for (size_t i = 0; i < tables.size(); ++i) {
        const sol::table &t = tables[i];
        auto ptr = ctx->get_character(i);
        core::parse_character(ptr, t);
      }
      
      for (size_t i = 0; i < tables.size(); ++i) {
        const sol::table &t = tables[i];
        auto ptr = ctx->get_character(i);
        core::parse_character_government(ptr, t);
      }
    }
    
    template <typename T>
    void parse_entities(
      core::context* ctx, 
      const std::vector<sol::table> &tables,
      const std::function<void(T*, const sol::table&)> &parsing_func
    ) {
      for (size_t i = 0; i < tables.size(); ++i) {
        const sol::table &t = tables[i];
        auto ptr = ctx->get_entity<T>(i);
        parsing_func(ptr, t);
      }
    }

    void parse_character(core::context* ctx, const std::vector<sol::table> &tables) {
      for (size_t i = 0; i < tables.size(); ++i) {
        const sol::table &t = tables[i];
        auto ptr = ctx->get_character(i);
        core::parse_character(ptr, t);
      }
      
      for (size_t i = 0; i < tables.size(); ++i) {
        const sol::table &t = tables[i];
        auto ptr = ctx->get_character(i);
        core::parse_character_government(ptr, t);
      }
    }
    
//     std::vector<sol::table> get_images_tables(const utils::world_serializator* w, sol::state_view state) {
//       std::vector<sol::table> tables;
//       for (size_t i = 0; i < w->get_images_count(); ++i) {
//         auto ret = state.script("return " + std::string(w->get_image_data(i)));
//         if (!ret.valid()) {
//           sol::error err = ret;
//           std::cout << err.what();
//           throw std::runtime_error("Could not load image table");
//         }
//         
//         sol::table t = ret;
//         // тут нам было бы неплохо проверить что это за таблица
//         const bool table_string = t[1].valid() && t[1].get_type() == sol::type::string && t.size() == 1;
//         if (table_string) { // таблица-путь, подгрузим скрипт
//           const std::string path = t[1];
//           // путь вида "*название мода*/*путь до скрипта*"
//           std::string_view mod_name;
//           std::string_view script_path;
//           parse_script_path(path, mod_name, script_path);
//           if (mod_name.empty() || script_path.empty()) throw std::runtime_error("Path " + path + " is not a valid script path");
//           // проверим есть ли такой мод, и проверим есть ли такой файл в моде
//           // еще нужно проверить является ли этот путь относительным
//           const std::string final_script_path = global::root_directory() + "/" + std::string(script_path);
//           // нужно еще указать энвайронмент, хотя с другой стороны, возможно просто в самом стейте отменить некоторые функции
//           // но в любом случае нужно еще погрузить минимально необходимый набор функций
//           auto ret = state.script_file(final_script_path);
//           if (!ret.valid()) {
//             sol::error err = ret;
//             std::cout << err.what();
//             throw std::runtime_error("Could not load image table");
//           }
//           
//           size_t counter = 0;
//           sol::table data_tables = ret;
//           for (const auto &obj : data_tables) {
//             if (obj.second.get_type() != sol::type::table) continue;
//             
//             sol::table final_t = obj.second.as<sol::table>();
//             const bool valid = utils::validate_image(counter, final_t);
//             ++counter;
//             if (!valid) throw std::runtime_error("Invalid image table in script " + path);
//             tables.push_back(final_t);
//           }
//         } else tables.push_back(t); // сгенерированная таблица данных
//       }
//       
//       return tables;
//     }
//     
//     std::vector<sol::table> get_heraldy_layers_tables(const utils::world_serializator* w, sol::state_view state) {
//       std::vector<sol::table> tables;
//       for (size_t i = 0; i < w->get_heraldies_count(); ++i) {
//         auto ret = state.script("return " + std::string(w->get_heraldy_data(i)));
//         if (!ret.valid()) {
//           sol::error err = ret;
//           std::cout << err.what();
//           throw std::runtime_error("Could not load heraldy table");
//         }
//         
//         sol::table t = ret;
//         tables.push_back(t);
//       }
//       
//       return tables;
//     }

    void load_localization(const utils::world_serializator* w, sol::state_view lua, const sol::environment &env, localization::container* cont) {
      // этот луа стейт ДОЛЖЕН БЫТЬ стейтом интерфейса
      
      for (size_t i = 0; i < w->get_localization_size(); ++i) {
        const std::string_view path = w->get_localization(i);
        std::string_view mod_name;
        std::string_view script_path;
        parse_script_path(path, mod_name, script_path);
        if (mod_name.empty() || script_path.empty()) throw std::runtime_error("Path " + std::string(path) + " is not a valid script path");
        ASSERT(mod_name == "apates_quest");
        
        const std::string final_script_path = global::root_directory() + "/" + std::string(script_path);
        // нужно еще указать энвайронмент, хотя с другой стороны, возможно просто в самом стейте отменить некоторые функции
        // но в любом случае нужно еще погрузить минимально необходимый набор функций
        auto ret = lua.script_file(final_script_path, env);
        if (!ret.valid()) {
          sol::error err = ret;
          std::cout << err.what();
          throw std::runtime_error("Could not load localization table");
        }
        
        sol::table data_tables = ret;
        for (const auto &pair : data_tables) {
          if (pair.first.get_type() != sol::type::string) throw std::runtime_error("Locale key must be a string");
          const auto key = pair.first.as<std::string_view>();
          if (!localization::is_valid_locale(key)) throw std::runtime_error("Locale key " + std::string(key) + " is invalid");
          const auto l = localization::container::locale(key);
          if (localization::container::get_current_locale() != l && localization::container::get_fall_back_locale() != l) continue;
          
          if (pair.second.get_type() != sol::type::table) throw std::runtime_error("Locale " + std::string(key) + " value must be a valid table");
          const auto t = pair.second.as<sol::table>();
          if (!localization::is_valid_table(t)) throw std::runtime_error("Locale " + std::string(key) + " value must be a valid table");
          
          cont->set_table(l, t);
        }
      }
    }

    //void loading_world(systems::map_t* map_systems, utils::progress_container* prog, const std::string &world_path) {
//     void loading_world(systems::map_t* map_systems, utils::progress_container* prog, const utils::world_serializator* world_data, const bool create_map_tiles) {
//       //ASSERT(!world_path.empty());
//       advance_progress(prog, "preparation"); // 1
//       //utils::world_serializator world_data;
//       //world_data.deserialize(world_path);
//       utils::data_string_container to_data;
//       global::get(&to_data);
//       utils::numeric_string_container heraldy_data_container;
//       global::get(&heraldy_data_container);
//       global::get(map_systems->core_context);
// 
//       map_systems->world_name = world_data->get_name();
//       map_systems->folder_name = world_data->get_technical_name();
//       map_systems->generator_settings = world_data->get_settings();
//       static_assert(systems::map_t::hash_size == utils::world_serializator::hash_size);
//       memcpy(map_systems->hash, world_data->get_hash(), systems::map_t::hash_size);
//       // и что собственно теперь? мы должны парсить таблицы и заполнять данные в типах
//       // create, parse, connect, end
// 
//       map_systems->map->world_matrix = world_data->get_world_matrix();
//       
//       // это нужно только если мы создаем карту заново
//       advance_progress(prog, "creating earth"); // 2
//       // нужно создать непосредственно core map
//       load_map_data(map_systems->map, world_data, create_map_tiles);
//       
//       auto graphics_container = global::get<systems::core_t>()->graphics_container;
//       sol::state lua;
//       lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::table, sol::lib::bit32, sol::lib::math, sol::lib::utf8); // sol::lib::coroutine // корутины не нужны
// //       utils::setup_lua_safe_utils(lua);
// //       utils::setup_lua_loading_functions(lua);
// //       utils::setup_lua_constants(lua);
//       utils::world_map_loading::setup_lua(lua);
//       
//       // мне тут нужен энвайронмент, там нужно удалить рандомность у математики + в этом конкретном случае не делать 
//       // где бы функцию для энвайронмента сделать?
//       // нужно добавить несколько функций из утилс
//       sol::environment env(lua, sol::create);
//       utils::make_environment(lua, env);
//       // тут наверное пригодятся и реквайр и ио линес
//       
//       advance_progress(prog, "loading images"); // 3
// //       const auto image_data = get_images_tables(world_data, lua);
// //       const auto heraldies_data = get_heraldy_layers_tables(world_data, lua);
//       auto controller = global::get<systems::core_t>()->image_controller;
//       global::get(controller);
//       
//       // тут у нас появляются два типа таблиц: таблица описание, таблица путь
//       {
//         const auto image_data = get_data_tables(world_data, lua, env, utils::world_serializator::image, "image", utils::validate_image);
//         // было бы неплохо избавиться от этого, но нужно сортануть таблицы
//         for (size_t i = 0; i < static_cast<size_t>(render::image_controller::image_type::count); ++i) {
//           //utils::load_images(controller, image_data, static_cast<uint32_t>(render::image_controller::image_type::system));
//           utils::load_images(controller, image_data, i);
//         }
//       }
//       
//       // геральдику будем грузить с помощью специального стейта
//       auto internal_state = global::get<systems::core_t>()->internal.get();
//       auto device = graphics_container->vulkan->device;
//       auto allocator = graphics_container->vulkan->buffer_allocator;
//       size_t heraldy_layers_count = 0;
//       size_t heraldy_layers_size = 0;
//       render::vk_buffer_data_unique heraldy_tmp(nullptr, nullptr, nullptr, nullptr);
//       {
//         //const auto heraldies_data = get_data_tables(world_data, lua, utils::world_serializator::heraldy_layer, "heraldy", utils::validate_heraldy_layer);
//         const auto heraldies_data = get_heraldy_data_tables(world_data, internal_state, utils::validate_heraldy_layer);
//         const auto layers_buffer = utils::load_heraldy_layers(controller, heraldies_data);
//         heraldy_layers_size = layers_buffer.size() * sizeof(layers_buffer[0]);
//         heraldy_tmp = render::create_buffer_unique(allocator, render::buffer(
//           heraldy_layers_size, vk::BufferUsageFlagBits::eTransferSrc
//         ), vma::MemoryUsage::eCpuOnly);
//         
//         memcpy(heraldy_tmp.ptr, layers_buffer.data(), heraldy_layers_size);
//         heraldy_layers_count = layers_buffer.size();
//       }
//       
//       (void)heraldy_layers_count;
//       
//       {
//         const auto biomes_data = get_data_tables(world_data, lua, env, utils::world_serializator::biome, "biome", utils::validate_biome);
//         utils::load_biomes(controller, map_systems->seasons, biomes_data);
//         
//         map_systems->seasons->validate();
//       }
//       
//       {
//         //load_localization(world_data, lua, global::get<systems::core_t>()->loc);
//         auto inter = global::get<systems::core_t>()->interface_container.get();
//         load_localization(world_data, inter->lua, inter->env, global::get<systems::core_t>()->loc.get());
//       }
// 
//       advance_progress(prog, "creating entities"); // 4
//       create_entity<core::culture_group>(map_systems->core_context, &to_data, world_data, lua, env, utils::world_serializator::culture_group, "culture_group", core::validate_culture_group);
//       create_entity<core::religion_group>(map_systems->core_context, &to_data, world_data, lua, env, utils::world_serializator::religion_group, "religion_group", core::validate_religion_group);
//       create_entity<core::culture>(map_systems->core_context, &to_data, world_data, lua, env, utils::world_serializator::culture, "culture", core::validate_culture);
//       create_entity<core::religion>(map_systems->core_context, &to_data, world_data, lua, env, utils::world_serializator::religion, "religion", core::validate_religion);
//       create_entity<core::building_type>(map_systems->core_context, &to_data, world_data, lua, env, utils::world_serializator::building_type, "building_type", core::validate_building_type);
//       create_entity<core::city_type>(map_systems->core_context, &to_data, world_data, lua, env, utils::world_serializator::city_type, "city_type", core::validate_city_type);
//       create_entity<core::titulus>(map_systems->core_context, &to_data, world_data, lua, env, utils::world_serializator::title, "title", core::validate_title);
//       create_entity_without_id<core::province>(map_systems->core_context, world_data, lua, env, utils::world_serializator::province, "province", core::validate_province);
//       create_entity_without_id<core::city>(map_systems->core_context, world_data, lua, env, utils::world_serializator::city, "city", core::validate_city);
//       create_characters(map_systems->core_context, world_data, lua, env);
//       
//       advance_progress(prog, "parsing entities"); // 5
//       parse_entities<core::culture_group>(map_systems->core_context, world_data, lua, env, utils::world_serializator::culture_group, "culture_group", core::validate_culture_group, core::parse_culture_group);
//       parse_entities<core::religion_group>(map_systems->core_context, world_data, lua, env, utils::world_serializator::religion_group, "religion_group", core::validate_religion_group, core::parse_religion_group);
//       parse_entities<core::culture>(map_systems->core_context, world_data, lua, env, utils::world_serializator::culture, "culture", core::validate_culture, core::parse_culture);
//       parse_entities<core::religion>(map_systems->core_context, world_data, lua, env, utils::world_serializator::religion, "religion", core::validate_religion, core::parse_religion);
//       parse_entities<core::titulus>(map_systems->core_context, world_data, lua, env, utils::world_serializator::title, "title", core::validate_title, core::parse_title);
//       parse_entities<core::province>(map_systems->core_context, world_data, lua, env, utils::world_serializator::province, "province", core::validate_province, core::parse_province);
//       parse_entities<core::building_type>(map_systems->core_context, world_data, lua, env, utils::world_serializator::building_type, "building_type", core::validate_building_type, core::parse_building);
//       parse_entities<core::city_type>(map_systems->core_context, world_data, lua, env, utils::world_serializator::city_type, "city_type", core::validate_city_type, core::parse_city_type);
//       parse_entities<core::city>(map_systems->core_context, world_data, lua, env, utils::world_serializator::city, "city", core::validate_city, core::parse_city);
//       parse_character(map_systems->core_context, world_data, lua, env);
//       // тут нужно еще соединить все полученные данные друг с другом
//       advance_progress(prog, "connecting game data"); // 6
//       connect_game_data(map_systems->map, map_systems->core_context);
//       // тут тупое копирование для того чтобы это работало нужно расположить данные так же как они были до сохранения
//       // это зависит от расположения картинок в массиве и по идее это расположение не изменяется
//       // мы можем просто добавить таблицы биомов в сохранку
//       
//       advance_progress(prog, "preparing biomes"); // 7
//       //world_data->fill_seasons(map_systems->seasons);
//       
//       const auto &data = get_season_biomes_data(map_systems);
//       
//       auto buffers = global::get<render::buffers>();
// //       std::vector<uint32_t> buffer_mem = {3, 1, 2, 3};
// //       size_t heraldy_indices_count = buffer_mem.size();
// //       for (const auto &data : buffers->new_indices) {
// //         assert(data.title != nullptr || data.dynasty != nullptr);
// //         
// //         // возможно хорошей идеей будет соединить каким либо образом титул и династию
// //         // в том плане что они используют довольно много сходных данных
// //         if (data.title != nullptr) data.title->heraldy = heraldy_indices_count;
// //         if (data.dynasty != nullptr) assert(false);
// //         
// //         heraldy_indices_count += data.indices.size()+1;
// //         assert(data.title->heraldy == buffer_mem.size());
// //         buffer_mem.push_back(data.indices.size());
// //         for (const auto &idx : data.indices) { buffer_mem.push_back(idx); }
// //       }
//       
//       {
// //         const size_t heraldy_indices_buffer_size = align_to(heraldy_indices_count * sizeof(uint32_t), 16);
//         auto ctx = map_systems->core_context;
//         const size_t structures_count = ctx->get_entity_count<core::city>();
//         const size_t structure_data_size = ctx->get_entity_count<core::city_type>() * sizeof(render::world_structure_t);
//         auto buf = render::create_buffer_unique(allocator, render::buffer(
//           structure_data_size, vk::BufferUsageFlagBits::eTransferSrc
//         ), vma::MemoryUsage::eCpuOnly);
//         
// //         auto buf_idx = render::create_buffer_unique(allocator, render::buffer(
// //           heraldy_indices_buffer_size, vk::BufferUsageFlagBits::eTransferSrc
// //         ), vma::MemoryUsage::eCpuOnly);
// //         
// //         memcpy(buf_idx.ptr, buffer_mem.data(), buffer_mem.size() * sizeof(buffer_mem[0]));
//         
//         auto map = map_systems->map;
//         map_systems->lock_map();
//         controller->update_set();
//         map_systems->map->copy_biomes(map_systems->seasons);
//         global::get<render::tile_optimizer>()->set_biome_tile_count(data);
//         
//         copy_structure_data(ctx, 0, map, buf.ptr);
//         map->resize_structures_buffer(structure_data_size);
//         ASSERT(heraldy_layers_size != 0);
//         buffers->resize_heraldy_buffer(heraldy_layers_size);
// //         buffers->resize_heraldy_indices_buffer(heraldy_indices_buffer_size, buffer_mem.size());
//         
//         auto pool = graphics_container->vulkan->transfer_command_pool;
//         auto queue = graphics_container->vulkan->graphics;
//         auto fence = graphics_container->vulkan->transfer_fence;
//         render::do_command(device, pool, queue, fence, [&] (vk::CommandBuffer task) {
//           const vk::BufferCopy c1{0, 0, structure_data_size};
//           const vk::BufferCopy c2{0, 0, heraldy_layers_size};
// //           const vk::BufferCopy c3{0, 0, heraldy_indices_buffer_size};
//           const vk::CommandBufferBeginInfo info(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
//           task.begin(info);
//           task.copyBuffer(buf.handle, map->data->structures.handle, c1);
//           task.copyBuffer(heraldy_tmp.handle, buffers->heraldy.handle, c2);
// //           task.copyBuffer(buf_idx.handle, buffers->heraldy_indices.handle, c3);
//           task.end();
//         });
//         
//         global::get<render::tile_optimizer>()->set_max_structures_count(structures_count);
//         // тут нужно учесть еще армии и героев, по идее теперь должно хватить на все вообще
//         global::get<render::tile_optimizer>()->set_max_heraldy_count(structures_count + core::context::armies_max_count + core::context::hero_troops_max_count);
//         map->flush_structures();
//         map_systems->unlock_map();
//       }
//       
//       map_systems->map->set_tile_biome(map_systems->seasons);
// 
//       map_systems->render_modes->at(render::modes::biome)();
//       global::get(reinterpret_cast<utils::data_string_container*>(SIZE_MAX));
//       global::get(reinterpret_cast<core::context*>(SIZE_MAX));
//       global::get(reinterpret_cast<render::image_controller*>(SIZE_MAX));
//       global::get(reinterpret_cast<utils::numeric_string_container*>(SIZE_MAX));
// 
//       //advance_progress(prog, "filling tile connections");
//       //generate_tile_connections(map_systems->map, global::get<dt::thread_pool>()); // это теперь не нужно
//       advance_progress(prog, "creating borders"); // 8
//       find_border_points(map_systems->map, map_systems->core_context); // после генерации нужно сделать много вещей
//       
//       const size_t mem = map_systems->core_context->compute_data_memory();
//       std::cout << "game data takes " << mem << " bytes (" << ((double(mem) / 1024.0) / 1024.0) << " mb)" << "\n";
//     }

    void from_menu_to_create_map(utils::progress_container* prog) {
      // тут по идее нам нужно создать мап системы
      // ну и все
      auto map_systems = global::get<systems::map_t>();
      ASSERT(map_systems != nullptr);
      //prog->set_max_value(2);
//       prog->set_hint1(std::string_view("Creating demiurge"));
//       prog->set_hint2(std::string_view("create container"));
//       prog->set_type(utils::progress_container::creating_map);
  //     map_systems->create_map_container(); // эти вещи нужно сделать во время загрузок
//       advance_progress(prog, "setup rendering");
      map_systems->setup_rendering_modes();
      //advance_progress(prog, "feeding up demiurge");
      //map_systems->setup_map_generator();
      //advance_progress(prog, "creating tools for demiurge");
      //setup_map_generator(map_systems);
//       advance_progress(prog, "end");
      //advance_progress(prog, "end");
      //advance_progress(prog, "end");
      //advance_progress(prog, "end");
      
      UNUSED_VARIABLE(prog);
    }

    void from_menu_to_map(utils::progress_container* prog) {
//       auto base_systems = global::get<systems::core_t>();
      auto map_systems = global::get<systems::map_t>();
//       prog->set_max_value(11);
//       prog->set_hint1(std::string_view("Load world"));
//       prog->set_hint2(std::string_view("create container"));
//       prog->set_type(utils::progress_container::loading_map);
      // так я могу не успеть создать ничего более прежде чем подойду к блокировке мьютекса в мейне
      // что делать в этом случае? использовать atomic_bool?
  //     map_systems->create_map_container();
      advance_progress(prog, "setup rendering");
      map_systems->setup_rendering_modes();
  //     advance_progress(prog, "feeding up demiurge");
  //     map_systems->setup_map_generator();
  //     advance_progress(prog, "creating tools for demiurge");
  //     setup_map_generator(*map_systems);
      //auto t = global::get<render::tile_optimizer>();
      //t->set_border_rendering(false);
      
      //ASSERT(!base_systems->menu->loading_path.empty())
      ASSERT(!map_systems->load_world.empty());
      
      utils::world_serializator world_data;
      world_data.deserialize(map_systems->load_world);
      
      const size_t mem = world_data.count_memory();
      std::cout << "world data takes " << mem << " bytes (" << ((double(mem) / 1024.0) / 1024.0) << " mb)" << "\n";
      
      world_data.fill_seasons(map_systems->seasons);
      // мы должны выбрать: либо загрузка мира, либо загрузка сохранения (там загрузка мира тоже будет)
//       loading_world(map_systems, prog, &world_data, true);
//       ASSERT(false);
      make_random_player_character();
      map_systems->load_world.clear();
      map_systems->load_world.shrink_to_fit();
      advance_progress(prog, "end");
      
      ASSERT(prog->finished());
    }

    void from_create_map_to_map(utils::progress_container* prog) {
      auto map_systems = global::get<systems::map_t>();
      auto base_systems = global::get<systems::core_t>();
      //prog->set_type(utils::progress_container::loading_created_map);
      auto t = global::get<render::tile_optimizer>();
      t->set_border_rendering(false);
      post_generation_work(map_systems, base_systems, prog);
      map_systems->destroy_map_generator();
      //advance_progress(prog, "end");
      //advance_progress(prog, "end");
      //advance_progress(prog, "end");
      
      ASSERT(prog->finished());
    }
    
    void parse_func_table(sol::state_view &lua, const sol::table &data_table, sol::table &container, std::unordered_set<std::string> &loaded_scripts, std::unordered_set<std::string> &loaded_functions) {
      const auto &root_string = global::root_directory();
      
      if (data_table["hint"].get_type() == sol::type::string) container["hint"] = data_table["hint"];
      if (data_table["path"].get_type() == sol::type::string) {
        const bool has_name = data_table["name"].get_type() == sol::type::string;
        const std::string path = data_table["path"];
        if (loaded_scripts.find(path) == loaded_scripts.end()) { // наверное это будет работать не идеально
          auto func_script = lua.safe_script_file(root_string + "/scripts/" + path);
          if (!func_script.valid()) {
            const sol::error err = func_script;
            PRINT(err.what())
            throw std::runtime_error("Could not load " + path + " script");
          }
          
          if (!has_name) {
            if (func_script.get_type() != sol::type::function) throw std::runtime_error("Bad function from script " + path);
            sol::function f = func_script;
            container["func"] = f;
          }
          
          loaded_scripts.insert(path);
        }
        
        if (has_name) {
          const std::string name = data_table["name"];
          if (loaded_functions.find(name) != loaded_functions.end()) throw std::runtime_error("Function " + name + "is already loaded");
          if (lua[name].get_type() != sol::type::function) throw std::runtime_error("Could not find function " + name + " in the script " + path);
          container["func"] = lua[name];
          lua[name] = sol::nil;
          loaded_functions.insert(name);
        }
        
        return;
      }
      
      if (data_table["string"].get_type() != sol::type::string) throw std::runtime_error("Invalid config data");
      
      const std::string func_str = data_table["string"];
      auto ret = lua.safe_script(func_str);
      if (!ret.valid()) {
        const sol::error err = ret;
        PRINT(err.what())
        throw std::runtime_error("Could not parse function string: " + func_str);
      }
      
      if (ret.get_type() != sol::type::function) throw std::runtime_error("Bad function string return type: " + func_str);
      
      sol::function f = ret;
      container["func"] = f;
    }
    
    void from_map_to_battle_part1(sol::state_view lua, utils::progress_container* prog) {
      // тут грузим конфиг, собираем данные о карте
      lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::table, sol::lib::utf8, sol::lib::math, sol::lib::bit32);
      utils::battle_map_generation::setup_lua(lua);
      
      std::unordered_set<std::string> loaded_scripts;
      std::unordered_set<std::string> loaded_functions;
      
      const auto &root_string = global::root_directory();
      // по идее путь до скрипта подготовки нужно задать при генерации
      auto config_table = lua.safe_script_file(root_string + "/scripts/battle_generator_config.lua");
      if (!config_table.valid()) {
        const sol::error err = config_table;
        PRINT(err.what())
        throw std::runtime_error("Could not load battle_generator_config.lua script");
      }
      
      if (config_table.get_type() != sol::type::table) throw std::runtime_error("Config script return error");
      
      sol::table t = config_table;
      //lua["config_table"] = t;
      sol::table parsed_config = lua.create_table();
      // тут наверное распарсим может полученную информацию? 
      // вообще я тут подумал что мне нужно так или иначе сделать какой то контроль над этими функциями
      // хотя опять же никакого особо контроля не получится, функция генерации проходит и выдает новые данные какие то
      // переместить их вперед назад скорее всего приведет к ошибкам, тут достаточно сделать порядок загрузки модов
      // куда сохранить все функции из конфига? хороший вопрос, можно в принципе в таблицу отдельную все это спихнуть
      // зачем нужен отдельный класс для этого? да в общем то нинужен
      
      for (const auto &obj : t) {
        if (!obj.first.is<std::string>()) continue;
        if (!obj.second.is<sol::table>()) continue;
        
        if (obj.first.as<std::string>() == "preparation_function") {
//           PRINT("preparation_function")
          const sol::table t = obj.second.as<sol::table>();
          auto p = parsed_config["preparation"].get_or_create<sol::table>();
          parse_func_table(lua, t, p, loaded_scripts, loaded_functions);
        }
        
        if (obj.first.as<std::string>() == "battle_over_function") {
//           PRINT("battle_over_function")
          const sol::table t = obj.second.as<sol::table>();
          auto p = parsed_config["end"].get_or_create<sol::table>();
          parse_func_table(lua, t, p, loaded_scripts, loaded_functions);
        }
        
        if (obj.first.as<std::string>() == "generator_functions") {
//           PRINT("generator_functions")
          const sol::table t = obj.second.as<sol::table>();
          auto p = parsed_config["generator"].get_or_create<sol::table>();
          for (const auto &obj : t) {
            if (!obj.second.is<sol::table>()) continue;
            const sol::table data = obj.second.as<sol::table>();
            auto container = lua.create_table();
            parse_func_table(lua, data, container, loaded_scripts, loaded_functions);
            p.add(container);
          }
        }
      }
      
      lua["config_table"] = parsed_config;
      
      // по идее здесь же мы запустим функцию подготовки
      // там у нас по идее должно быть прикидка какую карту создать 
      // то есть оттуда мы должны получить размер и тип 
      // + запомнить армии и что происходит вокруг (биомы)
      // 
      
      // при загрузке нужно будет перенести еще какие то данные из карты
      // например данные о персонажах и титулах, кажется данные для рендера знамен остаются в памяти 
      // вообще рендер лиц то поди тоже примерно так выглядит: есть некоторое количество слоев,
      // нужно по данным из буфера наложить их друг на друга
      
      advance_progress(prog, "cleaning");
    }
    
    void set_map_textures(battle::map* map, const std::vector<std::pair<render::image_t, render::image_t>> &textures) {
      ASSERT(textures.size() == map->tiles_count);
      
      auto buffer = reinterpret_cast<render::battle_map_tile_data_t*>(map->get_tiles_buffer_memory());
      ASSERT(buffer != nullptr);
      for (size_t i = 0; i < map->tiles_count; ++i) {
        if (!render::is_image_valid(textures[i].first)) throw std::runtime_error("Tile " + std::to_string(i) + " has invalid texture");
        if (!render::is_image_valid(textures[i].second)) throw std::runtime_error("Tile " + std::to_string(i) + " has invalid texture");
        buffer[i].ground = textures[i].first;
        buffer[i].walls = textures[i].second;
      }
    }
    
    void validate_tables(const std::function<bool(const uint32_t&, const sol::table&)> &func, map::creator::table_container_t* tables_container, const size_t &index) {
      const auto &tables = tables_container->get_tables(index);
      bool ret = true;
      for (size_t i = 0; i < tables.size(); ++i) {
        ret = ret && func(i, tables[i]);
      }
      
      if (!ret) throw std::runtime_error("There are tables parsing errors");
    }
    
    void from_map_to_battle_part2(sol::state_view lua, utils::progress_container* prog) {
      // тут собственно генерация, нужно передать контекст + луа табличку
      
      map::creator::table_container_t tables_container;
      global::get(&tables_container);
      // по аналогии с tables_container нужно сделать какой то держатель строк
      utils::battle_map_string_container strings;
      global::get(&strings);
      utils::data_string_container cont;
      global::get(&cont);
      
      std::unordered_set<std::string> unique_scripts;
      
      sol::table t = lua["config_table"];
      sol::table ctx_t = lua["ctx_table"];
      sol::table user_t = lua["usertable"];
      sol::table gen_table = t["generator"];
      for (const auto &func : gen_table) {
        if (!func.second.is<sol::table>()) continue;
        
        const auto table = func.second.as<sol::table>();
        const std::string hint = table["hint"];
        advance_progress(prog, hint);
        const sol::function gen_func = table["func"];
        //const std::function<void(sol::table &, sol::table &)> std_func = gen_func;
        //std_func(ctx_t, user_t);
        const auto res = gen_func(ctx_t, user_t);
        if (!res.valid()) {
          sol::error err = res;
          PRINT(err.what())
          throw std::runtime_error("There is lua errors");
        }
      }
      
      advance_progress(prog, "validate generated data");
      
      validate_tables(utils::validate_image, &tables_container, static_cast<size_t>(utils::generator_table_container::additional_data::image));
      validate_tables(utils::validate_battle_biome, &tables_container, static_cast<size_t>(utils::generator_table_container::additional_data::biome));
      
      validate_tables(utils::validate_battle_unit_state, &tables_container, static_cast<size_t>(battle::structure_type::unit_state));
      validate_tables(utils::validate_battle_troop_type, &tables_container, static_cast<size_t>(battle::structure_type::troop_type));
      validate_tables(utils::validate_battle_troop, &tables_container, static_cast<size_t>(battle::structure_type::troop));
      
      advance_progress(prog, "loading battle");
      
      {
        auto controller = global::get<systems::core_t>()->image_controller;
        const auto &tables = tables_container.get_tables(static_cast<size_t>(utils::generator_table_container::additional_data::image));
        for (size_t i = 0; i < static_cast<size_t>(render::image_controller::image_type::count); ++i) { // че за бред? мне нужно было гарантировать порядок
          utils::load_images(controller, tables, i);
        }
      }
      
      {
        auto controller = global::get<systems::core_t>()->image_controller;
        const auto &tables = tables_container.get_tables(static_cast<size_t>(utils::generator_table_container::additional_data::biome));
        utils::load_battle_biomes(controller, tables);
      }
      
      // + unit_state + troop + troop_type
      
      auto map = global::get<systems::battle_t>()->map;
      std::vector<std::pair<render::image_t, render::image_t>> tiles_textures(map->tiles_count, std::make_pair(render::image_t{GPU_UINT_MAX}, render::image_t{GPU_UINT_MAX}));
      {
        auto controller = global::get<systems::core_t>()->image_controller;
        const auto &strings_array = strings.get_strings(static_cast<size_t>(utils::battle_strings::tile_texture_id));
        ASSERT(strings_array.size() != 0);
        for (size_t i = 0; i < strings_array.size(); ++i) {
          if (strings_array[i].empty()) throw std::runtime_error("Texture id is missed for " + std::to_string(i) + " tile");
          auto img = utils::parse_image(strings_array[i], controller);
          tiles_textures[i].first = img;
        }
      }
      
      {
        auto controller = global::get<systems::core_t>()->image_controller;
        const auto &strings_array = strings.get_strings(static_cast<size_t>(utils::battle_strings::tile_walls_texture_id));
        ASSERT(strings_array.size() != 0);
        for (size_t i = 0; i < strings_array.size(); ++i) {
          if (strings_array[i].empty()) throw std::runtime_error("Texture id is missed for " + std::to_string(i) + " tile walls");
          auto img = utils::parse_image(strings_array[i], controller);
          tiles_textures[i].second = img;
        }
      }
      
      {
        auto ctx = global::get<systems::battle_t>()->context;
        auto cont = global::get<systems::battle_t>()->unit_states_map;
        const auto &tables = tables_container.get_tables(static_cast<size_t>(static_cast<size_t>(battle::structure_type::unit_state)));
        ctx->create_container<core::state>(tables.size());
        for (size_t i = 0; i < tables.size(); ++i) {
          const std::string id = tables[i]["id"];
          auto ptr = ctx->get_entity<core::state>(i);
          ptr->id = id;
          cont->insert(ptr->id, i);
        }
        
        utils::load_battle_unit_states(ctx, tables, unique_scripts);
      }
      
      {
        auto ctx = global::get<systems::battle_t>()->context;
        const auto &tables = tables_container.get_tables(static_cast<size_t>(static_cast<size_t>(battle::structure_type::troop_type)));
        ctx->create_container<battle::troop_type>(tables.size());
        utils::load_battle_troop_types(ctx, tables);
      }
      
      {
        auto ctx = global::get<systems::battle_t>()->context;
        const auto &tables = tables_container.get_tables(static_cast<size_t>(static_cast<size_t>(battle::structure_type::troop)));
        ctx->create_container<battle::troop>(tables.size());
        utils::load_battle_troops(ctx, tables);
      }
      
      // тип создаем юнитов
      {
        auto ctx = global::get<systems::battle_t>()->context;
        auto map = global::get<systems::battle_t>()->map;
        size_t counter = 0;
        for (size_t i = 0; i < ctx->get_entity_count<battle::troop>(); ++i) {
          auto troop = ctx->get_entity<battle::troop>(i);
          counter += troop->type->units_count;
        }
        
        map->set_units_count(counter);
        ctx->create_container<battle::unit>(counter);
        global::get<render::battle::tile_optimizer>()->update_unit_container();
        
        std::random_device dev;
        
        size_t offset = 0;
        for (size_t i = 0; i < ctx->get_entity_count<battle::troop>(); ++i) {
          auto troop = ctx->get_entity<battle::troop>(i);
          
          for (size_t j = 0; j < troop->type->units_count; ++j) {
            const size_t unit_index = offset + j;
            auto unit = ctx->get_entity<battle::unit>(unit_index);
            unit->unit_gpu_index = unit_index;
            unit->scale = troop->type->unit_scale;
            const uint64_t seed = make_64bit(dev(), dev());
            unit->seed_random(seed);
            ASSERT(troop->type->default_unit_state != nullptr);
            unit->set_state(troop->type->default_unit_state);
          }
          
          troop->unit_count = troop->type->units_count;
          troop->unit_offset = offset;
          const uint32_t troop_data = render::make_troop_data(troop->type->units_count, offset);
          map->set_tile_troop_data(troop->tile_index, troop_data);
          
          offset += troop->type->units_count;
        }
      }
      
      {
        auto system = global::get<systems::battle_t>();
        auto controller = global::get<systems::core_t>()->image_controller;
        const auto &data = get_battle_biomes_data(system);
        system->lock_map();
        global::get<render::battle::tile_optimizer>()->update_biome_data(data);
        set_map_textures(map, tiles_textures);
        controller->update_set();
        system->unlock_map();
      }
      
      global::get(reinterpret_cast<map::creator::table_container_t*>(SIZE_MAX));
      global::get(reinterpret_cast<utils::data_string_container*>(SIZE_MAX));
      
      ASSERT(prog->finished());
    }
    
    void validate_seasons(loading_context* loading_ctx) {
      auto map_systems = loading_ctx->map;
      auto ctx = loading_ctx->map->core_context;
      map_systems->seasons->validate();
      
      size_t counter = 0;
      const size_t biomes_count = ctx->get_entity_count<core::biome>();
      for (size_t i = 0; i < map_systems->seasons->seasons_count; ++i) {
        for (size_t tile_index = 0; tile_index < core::map::hex_count_d(core::map::detail_level); ++tile_index) {
          const uint32_t index = map_systems->seasons->get_tile_biome(i, tile_index);
          if (index >= biomes_count) {
            PRINT("Tile " + std::to_string(tile_index) + " biome index " + std::to_string(index) + " is larger than created biomes count " + std::to_string(biomes_count));
            ++counter;
          }
        }
      }
      
      if (counter != 0) throw std::runtime_error("Found " + std::to_string(counter) + " tiles with invalid biome index");
    }
    
    // эта функция запускается при загрузке карты
    void deserialize_world(loading_context* loading_ctx) {
      loading_ctx->map->setup_rendering_modes();
      ASSERT(!loading_ctx->map->load_world.empty());
      
      loading_ctx->world_data_container.reset(new utils::world_serializator);
      loading_ctx->world_data = loading_ctx->world_data_container.get();
      loading_ctx->world_data->deserialize(loading_ctx->map->load_world);
      
      const size_t mem = loading_ctx->world_data->count_memory();
      std::cout << "world data takes " << mem << " bytes (" << ((double(mem) / 1024.0) / 1024.0) << " mb)" << "\n";
      
      loading_ctx->world_data->fill_seasons(loading_ctx->map->seasons);
      loading_ctx->create_map_tiles = true;
    }
    
    // эта функция запускается при загрузке карты и после генерации карты
    void preparation(loading_context* loading_ctx) {
      auto map_systems = loading_ctx->map;
      // ворлд дату нужно тут брать из мап креатора
      // starting() и deserialize_world() используются в соответствующих загрузках (после генерации, сгенерированная)
      //loading_ctx->world_data = map_systems->map_creator->serializator_ptr();
//       auto world_data = loading_ctx->world_data.get();
      auto world_data = loading_ctx->world_data;
      ASSERT(world_data != nullptr);
      map_systems->world_name = world_data->get_name();
      map_systems->folder_name = world_data->get_technical_name();
      map_systems->generator_settings = world_data->get_settings();
      static_assert(systems::map_t::hash_size == utils::world_serializator::hash_size);
      memcpy(map_systems->hash, world_data->get_hash(), systems::map_t::hash_size);
      // и что собственно теперь? мы должны парсить таблицы и заполнять данные в типах
      // create, parse, connect, end

      map_systems->map->world_matrix = world_data->get_world_matrix();
    }
    
    void creating_earth(loading_context* loading_ctx) {
      auto map_systems = loading_ctx->map;
      auto world_data = loading_ctx->world_data;
      // нужно создать непосредственно core map
      load_map_data(map_systems->map, world_data, loading_ctx->create_map_tiles);
      
      loading_ctx->lua.reset(new sol::state);
      loading_ctx->lua->open_libraries(sol::lib::base, sol::lib::string, sol::lib::table, sol::lib::bit32, sol::lib::math, sol::lib::utf8); // sol::lib::coroutine // корутины не нужны
      utils::world_map_loading::setup_lua(*loading_ctx->lua);
      
      // мне тут нужен энвайронмент, там нужно удалить рандомность у математики + в этом конкретном случае не делать 
      // где бы функцию для энвайронмента сделать?
      // нужно добавить несколько функций из утилс
      loading_ctx->env.reset(new sol::environment(*loading_ctx->lua, sol::create));
      utils::make_environment(*loading_ctx->lua, *loading_ctx->env);
      // тут наверное пригодятся и реквайр и ио линес
    }
    
    // нужно разделить эту функцию, помимо картинок тут грузится локализация, геральдика, биомы
    void loading_images(loading_context* loading_ctx) {
      auto world_data = loading_ctx->world_data;
      auto lua = loading_ctx->lua.get();
      auto controller = loading_ctx->core->image_controller;
      auto graphics_container = loading_ctx->core->graphics_container;
      auto map_systems = loading_ctx->map;
      auto ctx = loading_ctx->map->core_context;
      auto env = loading_ctx->env.get();
      
      // тут у нас появляются два типа таблиц: таблица описание, таблица путь
      {
        const auto image_data = get_data_tables(world_data, *lua, *env, utils::world_serializator::image, "image", utils::validate_image);
        // было бы неплохо избавиться от этого, но нужно сортануть таблицы
        for (size_t i = 0; i < static_cast<size_t>(render::image_controller::image_type::count); ++i) {
          //utils::load_images(controller, image_data, static_cast<uint32_t>(render::image_controller::image_type::system));
          utils::load_images(controller, image_data, i);
        }
      }
      
      {
        auto inter = global::get<systems::core_t>()->interface_container.get();
        load_localization(world_data, inter->lua, inter->env, global::get<systems::core_t>()->loc.get());
      }
      
      loading_ctx->vk_buffer.reset(new render::vk_buffer_data_unique(nullptr, nullptr, nullptr, nullptr));
      auto heraldy_tmp = loading_ctx->vk_buffer.get();
      
      // геральдику будем грузить с помощью специального стейта
      auto internal_state = global::get<systems::core_t>()->internal.get();
      auto allocator = graphics_container->vulkan->buffer_allocator;
      {
        const auto heraldies_data = get_heraldy_data_tables(world_data, internal_state, utils::validate_heraldy_layer);
        const auto layers_buffer = utils::load_heraldy_layers(controller, heraldies_data);
        loading_ctx->heraldy_layers_size = layers_buffer.size() * sizeof(layers_buffer[0]);
        *heraldy_tmp = render::create_buffer_unique(allocator, render::buffer(
          loading_ctx->heraldy_layers_size, vk::BufferUsageFlagBits::eTransferSrc
        ), vma::MemoryUsage::eCpuOnly);
        
        memcpy(heraldy_tmp->ptr, layers_buffer.data(), loading_ctx->heraldy_layers_size);
        loading_ctx->heraldy_layers_count = layers_buffer.size();
      }
      
      {
//         const auto biomes_data = get_data_tables(world_data, *lua, *env, utils::world_serializator::biome, "biome", utils::validate_biome);
//         utils::load_biomes(controller, map_systems->seasons, biomes_data);
        
        const auto &biomes_data = get_data_tables_from_sequential_array(world_data, *lua, *env, utils::world_serializator::biome, "biome", core::validate_biome);
        ctx->create_container<core::biome>(biomes_data.size());
        parse_entities<core::biome>(map_systems->core_context, biomes_data, core::parse_biome); // id у биома задается здесь
        
        validate_seasons(loading_ctx);
      }
    }
    
    void creating_entities(loading_context* loading_ctx) {
      auto map_systems = loading_ctx->map;
      auto world_data = loading_ctx->world_data;
      auto to_data = &loading_ctx->to_data;
      auto lua = loading_ctx->lua.get();
      auto env = loading_ctx->env.get();
      
      utils::time_log log("creating_entities");
      
      map_systems->core_context->create_container<core::tile>(core::map::hex_count_d(core::map::detail_level));
      
      // нам бы все эти вещи спихнуть в одну функцию + env
      const auto &religion_group_tables = create_entity<core::religion_group>(
        map_systems->core_context, to_data, world_data, *lua, *env, 
        utils::world_serializator::religion_group, "religion_group", core::validate_religion_group
      );
      const auto &culture_group_tables = create_entity<core::culture_group>(
        map_systems->core_context, to_data, world_data, *lua, *env, 
        utils::world_serializator::culture_group, "culture_group", core::validate_culture_group
      );
      const auto &religion_tables = create_entity<core::religion>(
        map_systems->core_context, to_data, world_data, *lua, *env, 
        utils::world_serializator::religion, "religion", core::validate_religion
      );
      const auto &culture_tables = create_entity<core::culture>(
        map_systems->core_context, to_data, world_data, *lua, *env, 
        utils::world_serializator::culture, "culture", core::validate_culture
      );
      const auto &building_type_tables = create_entity<core::building_type>(
        map_systems->core_context, to_data, world_data, *lua, *env, 
        utils::world_serializator::building_type, "building_type", core::validate_building_type
      );
      const auto &city_type_tables = create_entity<core::city_type>(map_systems->core_context, to_data, world_data, *lua, *env, utils::world_serializator::city_type, "city_type", core::validate_city_type);
      const auto &titulus_tables = create_entity<core::titulus>(map_systems->core_context, to_data, world_data, *lua, *env, utils::world_serializator::title, "title", core::validate_title);
      const auto &province_tables = create_entity_without_id<core::province>(map_systems->core_context, world_data, *lua, *env, utils::world_serializator::province, "province", core::validate_province);
      const auto &city_tables = create_entity_without_id<core::city>(map_systems->core_context, world_data, *lua, *env, utils::world_serializator::city, "city", core::validate_city);
      const auto &character_tables = create_characters(map_systems->core_context, world_data, *lua, *env);
      
      const auto &interactions_tables = create_entity<core::interaction>(
        map_systems->core_context, to_data, world_data, *lua, *env, utils::world_serializator::interaction, "interaction", core::validate_interaction
      );
      
      const auto &decisions_tables = create_entity<core::decision>(
        map_systems->core_context, to_data, world_data, *lua, *env, utils::world_serializator::decision, "decision", core::validate_decision
      );
      
      const auto &events_tables = create_entity<core::event>(
        map_systems->core_context, to_data, world_data, *lua, *env, utils::world_serializator::event, "event", core::validate_event
      );
      
      map_systems->core_context->fill_id_maps();
      
      parse_entities<core::religion_group>(map_systems->core_context, religion_group_tables, core::parse_religion_group);
      parse_entities<core::culture_group>(map_systems->core_context, culture_group_tables, core::parse_culture_group);
      parse_entities<core::religion>(map_systems->core_context, religion_tables, core::parse_religion);
      parse_entities<core::culture>(map_systems->core_context, culture_tables, core::parse_culture);
      
      parse_entities<core::building_type>(map_systems->core_context, building_type_tables, core::parse_building);
      parse_entities<core::titulus>(map_systems->core_context, titulus_tables, core::parse_title);
      parse_entities<core::city_type>(map_systems->core_context, city_type_tables, core::parse_city_type);
      parse_entities<core::province>(map_systems->core_context, province_tables, core::parse_province);
      parse_entities<core::city>(map_systems->core_context, city_tables, core::parse_city);
      parse_character(map_systems->core_context, character_tables);
      
      parse_entities<core::interaction>(map_systems->core_context, interactions_tables, core::parse_interaction);
      parse_entities<core::decision>(map_systems->core_context, decisions_tables, core::parse_decision);
      parse_entities<core::event>(map_systems->core_context, events_tables, core::parse_event);
    }
    
    void connecting_game_data(loading_context* loading_ctx) {
      auto map_systems = loading_ctx->map;
      connect_game_data(map_systems->map, map_systems->core_context);
    }
    
    std::vector<render::biome_data_t> get_biomes_data(loading_context* loading_ctx) {
      auto ctx = loading_ctx->map->core_context;
      const size_t count = ctx->get_entity_count<core::biome>();
      assert(count < core::seasons::maximum_biomes);
      std::vector<render::biome_data_t> biomes(count);
      for (size_t i = 0; i < count; ++i) {
        const auto biome = ctx->get_entity<core::biome>(i);
        biomes[i] = biome->data;
      }
      return biomes;
    }
    
    void preparing_biomes(loading_context* loading_ctx) {
      auto map_systems = loading_ctx->map;
      auto graphics_container = loading_ctx->core->graphics_container;
      auto device = graphics_container->vulkan->device;
//       auto allocator = graphics_container->vulkan->buffer_allocator;
      auto controller = loading_ctx->core->image_controller;
      size_t heraldy_layers_size = loading_ctx->heraldy_layers_size;
      
      const auto &data = get_season_biomes_data(map_systems);
      
      auto buffers = global::get<render::buffers>();
      
      const auto &biomes_data = get_biomes_data(loading_ctx);
      
      {
        auto ctx = map_systems->core_context;
        const size_t structures_count = ctx->get_entity_count<core::city>();
//         const size_t structure_data_size = structures_count * sizeof(render::world_structure_t);
//         auto buf = render::create_buffer_unique(allocator, render::buffer(
//           structure_data_size, vk::BufferUsageFlagBits::eTransferSrc
//         ), vma::MemoryUsage::eCpuOnly);
        
//         auto map = map_systems->map;
        map_systems->lock_map();
        controller->update_set();
        map_systems->map->copy_biomes(biomes_data);
        global::get<render::tile_optimizer>()->set_biome_tile_count(data);
        
//         copy_structure_data(ctx, 0, map, buf.ptr);
//         map->resize_structures_buffer(structure_data_size);
        ASSERT(heraldy_layers_size != 0);
        buffers->resize_heraldy_buffer(heraldy_layers_size);
        
        auto pool = graphics_container->vulkan->transfer_command_pool;
        auto queue = graphics_container->vulkan->graphics;
        auto fence = graphics_container->vulkan->transfer_fence;
        render::do_command(device, pool, queue, fence, [&] (vk::CommandBuffer task) {
//           const vk::BufferCopy c1{0, 0, structure_data_size};
          const vk::BufferCopy c2{0, 0, heraldy_layers_size};
          const vk::CommandBufferBeginInfo info(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
          task.begin(info);
//           task.copyBuffer(buf.handle, map->data->structures.handle, c1);
          task.copyBuffer(loading_ctx->vk_buffer->handle, buffers->heraldy.handle, c2);
          task.end();
        });
        
        global::get<render::tile_optimizer>()->set_max_structures_count(structures_count);
        global::get<render::tile_updater>()->setup_default(graphics_container);
        // тут нужно учесть еще армии и героев, по идее теперь должно хватить на все вообще
        // теперь это тут не нужно
        //global::get<render::tile_optimizer>()->set_max_heraldy_count(structures_count + core::context::armies_max_count + core::context::hero_troops_max_count);
//         map->flush_structures();
        map_systems->unlock_map();
      }
      
      map_systems->map->set_tile_biome(map_systems->seasons);
      for (size_t i = 0; i < map_systems->map->tiles_count(); ++i) {
        const uint32_t index = map_systems->seasons->get_tile_biome(map_systems->seasons->current_season, i);
        auto tile = map_systems->core_context->get_entity<core::tile>(i);
        tile->biome_index = index;
      }
      
      {
        utils::time_log log("copying data to gpu");
        map_systems->map->flush_data();
      }
      
      // все данные уже в гпу памяти
    }
    
    void creating_borders(loading_context* loading_ctx) {
      auto map_systems = loading_ctx->map;
      find_border_points(map_systems->map, map_systems->core_context); // после генерации нужно сделать много вещей
      
      map_systems->render_modes->at(render::modes::biome)();
      
      const size_t mem = map_systems->core_context->compute_data_memory();
      std::cout << "game data takes " << mem << " bytes (" << ((double(mem) / 1024.0) / 1024.0) << " mb)" << "\n";
    }
    
    void make_player(loading_context* loading_ctx) {
      auto map_systems = loading_ctx->map;
      
      make_random_player_character();
      map_systems->load_world.clear();
      map_systems->load_world.shrink_to_fit();
    }
    
    void starting(loading_context* loading_ctx) {
      auto t = global::get<render::tile_optimizer>();
      t->set_border_rendering(false);
      
      auto map_systems = loading_ctx->map;
      auto creator = map_systems->map_creator;
      auto cont = creator->serializator_ptr();
      cont->set_name(creator->get_world_name());
      cont->set_technical_name(creator->get_folder_name());
      cont->set_settings(creator->get_settings());
      cont->set_rand_seed(creator->get_rand_seed());
      cont->set_noise_seed(creator->get_noise_seed());
      // стартинг должен быть только после генерации
      loading_ctx->world_data = cont;
    }
    
    void serializing_world(loading_context* loading_ctx) {
      auto map_systems = loading_ctx->map;
      auto creator = map_systems->map_creator;
      auto cont = creator->serializator_ptr();
      
      cont->copy_seasons(map_systems->seasons);
      cont->set_world_matrix(map_systems->map->world_matrix);

      for (size_t i = 0; i < core::map::hex_count_d(core::map::detail_level); ++i) {
        //const auto &d = ctx->get_tile(i);
        //cont->set_tile_data(i, {d.height, d.province}); // тут даже по идее ничего делать особо не нужно
        
        //counter += size_t(d.province != UINT32_MAX);
        auto tile = map_systems->map->get_tile_ptr(i);
        cont->set_tile_data(i, {tile->height, UINT32_MAX});
      }

      cont->serialize(); //
      
      // вот это по идее не нужно, у меня грузится это все в loading_world
      map_systems->world_name = cont->get_name();
      map_systems->folder_name = cont->get_technical_name();
      map_systems->generator_settings = cont->get_settings();
      memcpy(map_systems->hash, cont->get_hash(), systems::map_t::hash_size);
      
      loading_ctx->create_map_tiles = false;
    }
    
    void checking_world(loading_context* loading_ctx) {
      auto map_systems = loading_ctx->map;
      auto creator = map_systems->map_creator;
      auto cont = creator->serializator_ptr();
      
      utils::world_serializator test;
      test.deserialize(global::root_directory() + "saves/" + creator->get_folder_name() + "/world_data");

      ASSERT(test.get_name() == creator->get_world_name());
      ASSERT(test.get_technical_name() == creator->get_folder_name());
      ASSERT(test.get_settings() == creator->get_settings());
      ASSERT(test.get_rand_seed() == creator->get_rand_seed());
      ASSERT(test.get_noise_seed() == creator->get_noise_seed());

      ASSERT(cont->get_data_count(utils::world_serializator::province) == test.get_data_count(utils::world_serializator::province));
      ASSERT(cont->get_data_count(utils::world_serializator::city_type) == test.get_data_count(utils::world_serializator::city_type));
      ASSERT(cont->get_data_count(utils::world_serializator::city) == test.get_data_count(utils::world_serializator::city));
      ASSERT(cont->get_data_count(utils::world_serializator::building_type) == test.get_data_count(utils::world_serializator::building_type));
      ASSERT(cont->get_data_count(utils::world_serializator::title) == test.get_data_count(utils::world_serializator::title));
      ASSERT(cont->get_data_count(utils::world_serializator::character) == test.get_data_count(utils::world_serializator::character));

      //PRINT(cont.get_data(var, i))
  #define CHECK_CONTENT(var) for (uint32_t i = 0; i < cont->get_data_count(var); ++i) { ASSERT(cont->get_data(var, i) == test.get_data(var, i)); }
      CHECK_CONTENT(utils::world_serializator::province)
      CHECK_CONTENT(utils::world_serializator::city_type)
      CHECK_CONTENT(utils::world_serializator::city)
      CHECK_CONTENT(utils::world_serializator::building_type)
      CHECK_CONTENT(utils::world_serializator::title)
      CHECK_CONTENT(utils::world_serializator::character)

      // кажется все правильно сериализуется и десериализуется
      // сохранения должны храниться в папках с файлом world_data
      // название папки - техническое название мира,
      // техническое название мира - нужно зафорсить пользователя задать валидное техническое название
      // (какнибудь бы упростить для пользователя это дело)
      // что самое главное я могу оставить пока так как есть и это покроет почти всю сериализацию
      // остается решить вопрос с климатом, хотя че тут решать: нужно выделить
      // 64*500к или 128*500к памяти чтобы сохранить сезоны, сезонов по идее не имеет смысла делать больше чем размер массива на каждый тайл
      // соответственно 64 сезона на каждый тайл, означает ли это что мы можем сохранить 4 тайла в одном чаре?
      // в сохранениях видимо придется делать протомессадж для каждой сущности которую необходимо сохранить
      // мне нужно еще придумать стендалоне сохранения: то есть сохранения в которых записаны данные мира дополнительно
      //
    }
    
    void ending(loading_context* loading_ctx) {
      auto map_systems = loading_ctx->map;
      map_systems->destroy_map_generator();
    }
    
    void gen_prepare(loading_context* loading_ctx) {
      auto map_systems = loading_ctx->map;
      ASSERT(map_systems != nullptr);
      map_systems->setup_rendering_modes();
      map_systems->start_rendering();
    }
  }
}
