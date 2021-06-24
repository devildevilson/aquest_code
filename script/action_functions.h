#ifndef ACTION_FUNCTIONS_H
#define ACTION_FUNCTIONS_H

#include "script_header.h"
#include "action_commands_macro.h"

namespace devils_engine {
  namespace script {
#define ACTION_COMMAND_FUNC(name) void name(const target&, const context&, const uint32_t&, const script_data*);
    ACTION_COMMANDS_LIST
#undef ACTION_COMMAND_FUNC

#define ACTION_COMMAND_FUNC(name) void name##_init(const uint32_t &, const sol::object &, script_data*);
    ACTION_COMMANDS_LIST
#undef ACTION_COMMAND_FUNC
  }
}

#endif
