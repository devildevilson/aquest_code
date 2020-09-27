#include "lua_initialization.h"

#include "globals.h"
#include "bin/core_structures.h"
// #include "bin/helper.h"
#include "utility.h"
#include "bin/logic.h"
#include "input.h"
#include "progress_container.h"
#include "main_menu.h"
#include "demiurge.h"

namespace devils_engine {
  namespace utils {
    void setup_lua_package_path(sol::state &lua) {
      {
        const std::string default_path = lua["package"]["path"];
        const std::string new_path =
#ifdef __linux__
          global::root_directory() + "scripts/?.lua;" +
          global::root_directory() + "scripts/?/init.lua;" +
#elif _WIN32
          global::root_directory() + "scripts\\?.lua;" +
          global::root_directory() + "scripts\\?\\init.lua;" +
#endif
          default_path;
        lua["package"]["path"] = new_path;
      }

      {
        const std::string default_path = lua["package"]["cpath"];
        const std::string new_path =
  #ifdef __linux__
          global::root_directory() + "scripts/?.so;" + // этого достаточно?
  #elif _WIN32
          global::root_directory() + "scripts\\?.dll;" +
  #endif
          default_path;
        lua["package"]["cpath"] = new_path;
      }
    }
  }
}
