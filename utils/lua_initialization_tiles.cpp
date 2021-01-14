#include "lua_initialization.h"

#include "bin/tiles_funcs.h"
#include "bin/core_structures.h"

// нужно вытащить тайловые функции отдельно
namespace devils_engine {
  namespace utils {
    void setup_lua_tile(sol::state_view lua) {
      auto core = lua["core"].get_or_create<sol::table>();
      core["get_tile_height"] = core::get_tile_height;
      core["get_tile_province"] = core::get_tile_province;
      core["get_tile_city"] = core::get_tile_city;
      // нужно еще вывести какую то инфу для биома
      // тип биома, имя?
    }
  }
}
