#ifndef BATTLE_LUA_STATES_H
#define BATTLE_LUA_STATES_H

#include "utils/sol.h"
#include <vector>
#include <unordered_set>
#include <functional>

namespace devils_engine {
  namespace battle {
    struct lua_container {
      sol::state* states;
      std::vector<std::vector<sol::function>> registered_functions;
      
      lua_container();
      ~lua_container();
      
      uint32_t parse_function(const std::string_view &script);
      uint32_t parse_function(const std::string &path, const std::string_view &name);
      uint32_t parse_existing_function(const std::string_view &name);
    };
  }
}

#endif
