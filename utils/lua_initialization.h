#ifndef LUA_INITIALIZATION_H
#define LUA_INITIALIZATION_H

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

namespace devils_engine {
  namespace utils {
    void setup_lua_types(sol::state &lua);
  }
}

#endif
