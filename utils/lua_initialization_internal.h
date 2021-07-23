#ifndef LUA_INITIALIZATION_INTERNAL_H
#define LUA_INITIALIZATION_INTERNAL_H

#include "utils/sol.h"
#include "reserved_lua_table_names.h"

namespace devils_engine {
  namespace utils {
    namespace internal {
      void setup_lua_enums(sol::state_view lua);
      void setup_lua_province(sol::state_view lua);
      void setup_lua_building_type(sol::state_view lua);
      void setup_lua_city_type(sol::state_view lua);
      void setup_lua_city(sol::state_view lua);
      void setup_lua_titulus(sol::state_view lua);
      void setup_lua_character(sol::state_view lua);
      void setup_lua_faction(sol::state_view lua);
      void setup_lua_army(sol::state_view lua);
    }
  }
}

#endif
