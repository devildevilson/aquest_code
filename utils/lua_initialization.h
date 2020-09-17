#ifndef LUA_INITIALIZATION_H
#define LUA_INITIALIZATION_H

#include "utils/sol.h"

namespace devils_engine {
  namespace utils {
    void setup_lua_package_path(sol::state &lua);
    void setup_lua_constants(sol::state &lua);
    void setup_lua_types(sol::state &lua);
    void setup_lua_input(sol::state &lua);
    void setup_lua_game_logic(sol::state &lua);
  }
}

#endif
