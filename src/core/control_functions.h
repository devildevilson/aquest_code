#ifndef DEVILS_ENGINE_CORE_CONTROL_FUNCTIONS_H
#define DEVILS_ENGINE_CORE_CONTROL_FUNCTIONS_H

#include <cstddef>
#include <cstdint>
#include "declare_structures.h"
#include "utils/sol.h"
#include "utils/handle.h"

// эти вещи только в луа используются, зададим их в lua_initialization_internal_decision.cpp
namespace devils_engine {
  namespace core {
    sol::table get_potential_interactions(sol::this_state s, const sol::object &actor, const sol::object &recipient);
    sol::table get_potential_decisions(sol::this_state s, const sol::object &actor);
  }
}

#endif
