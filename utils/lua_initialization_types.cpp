#include "lua_initialization_hidden.h"
#include "lua_initialization_internal.h"

namespace devils_engine {
  namespace utils {
    void setup_lua_types(sol::state_view lua) {
      internal::setup_lua_enums(lua);
      internal::setup_lua_province(lua);
      internal::setup_lua_building_type(lua);
      internal::setup_lua_city_type(lua);
      internal::setup_lua_city(lua);
      internal::setup_lua_titulus(lua);
      internal::setup_lua_character(lua);
      internal::setup_lua_realm(lua);
      internal::setup_lua_army(lua);
      internal::setup_lua_culture(lua);
      internal::setup_lua_religion(lua);
      internal::setup_lua_war(lua);
      internal::setup_lua_troop(lua);
      internal::setup_lua_troop_type(lua);
//       internal::setup_lua_trait(lua);
//       internal::setup_lua_modificator(lua);
//       internal::setup_lua_casus_belli(lua);
    }
  }
}
