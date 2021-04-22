#ifndef LUA_ENVIRONMENT_H
#define LUA_ENVIRONMENT_H

#include "sol.h"

namespace devils_engine {
  namespace utils {
    void make_environment(sol::state_view lua, sol::environment &env);
    void add_io_lines(sol::state_view lua, sol::environment &env);
    void add_require(sol::state_view lua, sol::environment &env);
    void add_interface_require(sol::state_view lua, sol::environment &env);
  }
}

#endif
