#ifndef LUA_INITIALIZATION_INTERNAL_H
#define LUA_INITIALIZATION_INTERNAL_H

#include "utils/sol.h"

namespace devils_engine {
  namespace utils {
    namespace internal {
      void setup_lua_enums(sol::state &lua);
      void setup_lua_province(sol::state &lua);
      void setup_lua_building_type(sol::state &lua);
      void setup_lua_city_type(sol::state &lua);
      void setup_lua_city(sol::state &lua);
      void setup_lua_titulus(sol::state &lua);
      void setup_lua_character(sol::state &lua);
      void setup_lua_faction(sol::state &lua);
    }
  }
}

#endif
