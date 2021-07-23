#ifndef SCRIPT_LUA_DATA_STRUCT_H
#define SCRIPT_LUA_DATA_STRUCT_H

#include <cstdint>
#include <string_view>
#include "utils/sol.h"

namespace devils_engine {
  namespace script {
    struct lua_data_struct {
      std::string_view name;
      size_t nesting;
      uint32_t compare_type;
      uint32_t special_stat;
      sol::object value;
      sol::object data;
      sol::object return_value;
    };
  }
}

#endif
