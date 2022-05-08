#ifndef DEVILS_ENGINE_UTILS_LUA_INITIALIZATION_HANDLE_TYPES_H
#define DEVILS_ENGINE_UTILS_LUA_INITIALIZATION_HANDLE_TYPES_H

#include "utils/handle.h"
#include "core/realm.h"
#include "core/army.h"
#include "core/hero_troop.h"
#include "core/war.h"
#include "core/troop.h"

namespace devils_engine {
  namespace utils {
    LUA_HANDLE_DECLARATION(realm)
    LUA_HANDLE_DECLARATION(army)
    LUA_HANDLE_DECLARATION(hero_troop)
    LUA_HANDLE_DECLARATION(war)
    LUA_HANDLE_DECLARATION(troop)
  }
}

#endif
