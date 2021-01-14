#include "lua_initialization.h"

#include "bin/image_parser.h"

namespace devils_engine {
  namespace utils {
    void setup_lua_utility_battle_generator_functions(sol::state_view lua) {
      auto utils = lua["utils"].get_or_create<sol::table>();
      utils.set_function("add_image", &add_image);
      utils.set_function("add_biome", &add_battle_biome);
    }
  }
}
