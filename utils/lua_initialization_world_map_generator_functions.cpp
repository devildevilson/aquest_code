#include "lua_initialization.h"

#include "bin/image_parser.h"
#include "bin/data_parser.h"
#include "bin/heraldy_parser.h"
#include "magic_enum.hpp"

#define LOAD_FUNC(name, add_func)    \
  utils.set_function(name , [] (sol::this_state s, const std::string_view &path) {  \
    sol::state_view lua = s;         \
    auto t = lua.create_table(1, 0); \
    t.add(path);                     \
    add_func(t);                     \
  });

namespace devils_engine {
  namespace utils {
    void setup_lua_utility_map_generator_functions(sol::state_view lua) {
      auto utils = lua[magic_enum::enum_name<reserved_lua::values>(reserved_lua::utils)].get_or_create<sol::table>();
      utils.set_function("add_image", &add_image);
      utils.set_function("load_images", [] (sol::this_state s, const std::string_view &path) {
        sol::state_view lua = s;
        auto t = lua.create_table(1, 0);
        t.add(path);
        add_image(t);
      });
      utils.set_function("add_biome", &add_biome);
      utils.set_function("load_biomes", [] (sol::this_state s, const std::string_view &path) {
        sol::state_view lua = s;
        auto t = lua.create_table(1, 0);
        t.add(path);
        add_biome(t);
      });
      utils.set_function("add_province", &add_province);
      utils.set_function("add_building", &add_building);
      utils.set_function("add_character", &add_character);
      utils.set_function("add_city", &add_city);
      utils.set_function("add_city_type", &add_city_type);
//       utils.set_function("add_event", &add_event);
      utils.set_function("add_heraldy_layer", &add_heraldy_layer);
      utils.set_function("add_title", &add_title);
      
      LOAD_FUNC("load_provinces", add_province)
      LOAD_FUNC("load_buildings", add_building)
      LOAD_FUNC("load_characters", add_character)
      LOAD_FUNC("load_cities", add_city)
      LOAD_FUNC("load_city_types", add_city_type)
      LOAD_FUNC("load_heraldy_layers", add_heraldy_layer)
      LOAD_FUNC("load_titles", add_title)
    }
  }
}
