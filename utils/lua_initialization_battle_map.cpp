#include "lua_initialization.h"

#include "bin/battle_map.h"

namespace devils_engine {
  namespace utils {
    void setup_lua_battle_map(sol::state_view lua) {
      auto battle = lua["battle"].get_or_create<sol::table>();
      battle.new_usertype<battle::map>("map",
        "is_square", &battle::map::is_square,
        "is_flat", &battle::map::is_flat,
        "is_odd", &battle::map::is_odd,
        "set_tile_height", &battle::map::set_tile_height,
        "get_tile_height", &battle::map::get_tile_height,
        "set_tile_biome", &battle::map::set_tile_biome,
        "get_tile_biome", &battle::map::get_tile_biome,
        "tiles_count", sol::readonly_property([] (const battle::map* self) { return self->tiles_count; }),
        "width", sol::readonly_property([] (const battle::map* self) { return self->width; }),
        "height", sol::readonly_property([] (const battle::map* self) { return self->height; })
      );
    }
  }
}
