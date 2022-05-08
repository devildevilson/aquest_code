#ifndef DEVILS_ENGINE_LUA_INITIALIZATION_INTERNAL_H
#define DEVILS_ENGINE_LUA_INITIALIZATION_INTERNAL_H

#include "utils/sol.h"
#include "utils/reserved_lua_table_names.h"

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
      void setup_lua_realm(sol::state_view lua);
      void setup_lua_army(sol::state_view lua);
      void setup_lua_culture(sol::state_view lua);
      void setup_lua_religion(sol::state_view lua);
      void setup_lua_war(sol::state_view lua);
      void setup_lua_trait(sol::state_view lua);
      void setup_lua_modificator(sol::state_view lua);
      void setup_lua_casus_belli(sol::state_view lua);
      void setup_lua_troop(sol::state_view lua);
      void setup_lua_troop_type(sol::state_view lua);
      void setup_lua_interaction(sol::state_view lua);
      void setup_lua_decision(sol::state_view lua);
    }
  }
}

#endif
