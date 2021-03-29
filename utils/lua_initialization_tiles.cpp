#include "lua_initialization.h"

#include "bin/tiles_funcs.h"
#include "bin/core_structures.h"
#include "magic_enum.hpp"

// нужно вытащить тайловые функции отдельно
namespace devils_engine {
  namespace utils {
    void setup_lua_tile(sol::state_view lua) {
      auto core = lua[magic_enum::enum_name<reserved_lua::values>(reserved_lua::core)].get_or_create<sol::table>();
      core["get_tile_height"] = core::get_tile_height;
      core["get_tile_province"] = core::get_tile_province;
      core["get_tile_city"] = core::get_tile_city;
      // нужно еще вывести какую то инфу для биома
      // тип биома, имя?
    }
  }
}
