#ifndef ACTION_FUNCTIONS_H
#define ACTION_FUNCTIONS_H

#include "script_header.h"
#include "action_commands_macro.h"
#include "core/stats.h"

namespace devils_engine {
  namespace script {
#define ACTION_COMMAND_FUNC(name) void name(const target_t&, context*, const uint32_t&, const script_data*);
    ACTION_COMMANDS_LIST
#undef ACTION_COMMAND_FUNC

#define ACTION_COMMAND_FUNC(name) void name##_init(const uint32_t &, const sol::object &, script_data*);
    ACTION_COMMANDS_LIST
#undef ACTION_COMMAND_FUNC

#define STAT_FUNC(name) void add_##name(const target_t&, context*, const uint32_t &, const script_data*);
#define CHARACTER_PENALTY_STAT_FUNC(name) void add_##name##_penalty(const target_t&, context*, const uint32_t &, const script_data*);
    UNIQUE_STATS_LIST
#undef STAT_FUNC
#undef CHARACTER_PENALTY_STAT_FUNC

#define STAT_FUNC(name) void add_##name##_init(const uint32_t &, const sol::object &, script_data*);
#define CHARACTER_PENALTY_STAT_FUNC(name) void add_##name##_penalty_init(const uint32_t &, const sol::object &, script_data*);
    UNIQUE_STATS_LIST
#undef STAT_FUNC
#undef CHARACTER_PENALTY_STAT_FUNC
  }
}

#endif
