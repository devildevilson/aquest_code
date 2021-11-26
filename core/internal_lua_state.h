#ifndef CORE_INTERNAL_LUA_STATE_H
#define CORE_INTERNAL_LUA_STATE_H

#include "utils/sol.h"

// где указать мапу с id слоев? прямо здесь? они от луа не зависят, так что надо в другое место
// добавить наверное поближе к геральдикам
// где указать функции генерации геральдики? можно и тут

namespace devils_engine {
  namespace core {
    struct internal_lua_state {
      sol::state lua;
      sol::environment env;
      
      sol::table gen_funcs_table;
      
      internal_lua_state();
      ~internal_lua_state();
    };
  }
}

#endif
