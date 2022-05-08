#include "lua_initialization_hidden.h"

#include "utils/globals.h"

namespace devils_engine {
  namespace utils {
    void setup_lua_package_path(sol::state_view lua) {
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
