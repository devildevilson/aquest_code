#include "internal_lua_state.h"

#include "utils/lua_initialization.h"
#include "utils/lua_environment.h"

namespace devils_engine {
  namespace core {
    internal_lua_state::internal_lua_state() : env(lua, sol::create), gen_funcs_table(lua, sol::create) {
      lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::table, sol::lib::bit32, sol::lib::math, sol::lib::utf8, sol::lib::coroutine);
      utils::world_map_loading::setup_lua(lua);
      utils::make_environment(lua, env);
    }
    
    internal_lua_state::~internal_lua_state() {}
  }
}
