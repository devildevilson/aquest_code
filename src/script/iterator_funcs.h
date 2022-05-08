#ifndef DEVILS_ENGINE_SCRIPT_ITERATOR_FUNCS_H
#define DEVILS_ENGINE_SCRIPT_ITERATOR_FUNCS_H

#include <functional>
#include "utils/handle.h"
#include "core/declare_structures.h"
#include "object.h"
#include "change_context_commands_macro.h"

namespace devils_engine {
  namespace script {
    using func_t = std::function<bool(const object &obj)>;
    
    bool each_ancestor(const core::character* ch, const func_t &f);

#define COMMON_CHANGE_CONTEXT_COMMAND_FUNC(name, type) bool each_##name(const type, const func_t &);
    
#define CHANGE_CONTEXT_COMMAND_FUNC(name, in, in2, out_type) COMMON_CHANGE_CONTEXT_COMMAND_FUNC(name, core::character*)
    CHARACTER_CHANGE_CONTEXT_COMMANDS_LIST
#undef CHANGE_CONTEXT_COMMAND_FUNC

#define CHANGE_CONTEXT_COMMAND_FUNC(name, in, in2, out_type) COMMON_CHANGE_CONTEXT_COMMAND_FUNC(name, utils::handle<core::realm>)
    REALM_CHANGE_CONTEXT_COMMANDS_LIST
#undef CHANGE_CONTEXT_COMMAND_FUNC

#define CHANGE_CONTEXT_COMMAND_FUNC(name, in, in2, out_type) COMMON_CHANGE_CONTEXT_COMMAND_FUNC(name, utils::handle<core::war>)
    WAR_CHANGE_CONTEXT_COMMANDS_LIST
#undef CHANGE_CONTEXT_COMMAND_FUNC

#define CHANGE_CONTEXT_COMMAND_FUNC(name, in, in2, out_type) COMMON_CHANGE_CONTEXT_COMMAND_FUNC(name, core::religion*)
    RELIGION_CHANGE_CONTEXT_COMMANDS_LIST
#undef CHANGE_CONTEXT_COMMAND_FUNC

#define CHANGE_CONTEXT_COMMAND_FUNC(name, in, in2, out_type) COMMON_CHANGE_CONTEXT_COMMAND_FUNC(name, core::culture*)
    CULTURE_CHANGE_CONTEXT_COMMANDS_LIST
#undef CHANGE_CONTEXT_COMMAND_FUNC

#define CHANGE_CONTEXT_COMMAND_FUNC(name, in, in2, out_type) COMMON_CHANGE_CONTEXT_COMMAND_FUNC(name, core::province*)
    PROVINCE_CHANGE_CONTEXT_COMMANDS_LIST
#undef CHANGE_CONTEXT_COMMAND_FUNC

#define CHANGE_CONTEXT_COMMAND_FUNC(name, in, in2, out_type) COMMON_CHANGE_CONTEXT_COMMAND_FUNC(name, core::titulus*)
    TITLE_CHANGE_CONTEXT_COMMANDS_LIST
#undef CHANGE_CONTEXT_COMMAND_FUNC

// как глобал сделать? отдельно? мож указать void как входной тип?
#define CHANGE_CONTEXT_COMMAND_FUNC(name, in, in2, out_type) COMMON_CHANGE_CONTEXT_COMMAND_FUNC(name, void*)
    CHANGE_CONTEXT_COMMANDS_GLOBAL_LIST
#undef CHANGE_CONTEXT_COMMAND_FUNC
    
#undef COMMON_CHANGE_CONTEXT_COMMAND_FUNC
  }
}

#endif
