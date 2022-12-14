#include "lua_initialization_hidden.h"

#include "bin/image_parser.h"
#include "battle/troop_parser.h"
#include "battle/troop_type_parser.h"
#include "battle/unit_state_parser.h"
#include "utils/magic_enum_header.h"

namespace devils_engine {
  namespace utils {
    void setup_lua_utility_battle_generator_functions(sol::state_view lua) {
      auto utils = lua[magic_enum::enum_name<reserved_lua::values>(reserved_lua::utils)].get_or_create<sol::table>();
      utils.set_function("add_image", &add_image);
      utils.set_function("add_biome", &add_battle_biome);
      utils.set_function("add_troop_type", &add_battle_troop_type);
      utils.set_function("add_troop", &add_battle_troop);
      utils.set_function("add_unit_state", &add_battle_unit_state);
    }
    
    void setup_lua_utility_battle_generator_functions(sol::environment &lua) {
      auto utils = lua[magic_enum::enum_name<reserved_lua::values>(reserved_lua::utils)].get_or_create<sol::table>();
      utils.set_function("add_image", &add_image);
      utils.set_function("add_biome", &add_battle_biome);
      utils.set_function("add_troop_type", &add_battle_troop_type);
      utils.set_function("add_troop", &add_battle_troop);
      utils.set_function("add_unit_state", &add_battle_unit_state);
    }
  }
}
