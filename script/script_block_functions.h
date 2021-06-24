#ifndef SCRIPT_BLOCK_FUNCTIONS_H
#define SCRIPT_BLOCK_FUNCTIONS_H

#include "script_header.h"
#include "script_block_commands_macro.h"

#define FALSE_BLOCK 0
#define TRUE_BLOCK 1
#define IGNORE_BLOCK 2

namespace devils_engine {
  namespace script {
    // по идее блок вообще мало чем отличается от обычной функции
#define SCRIPT_BLOCK_COMMAND_FUNC(name) int32_t name(const struct target &, const context &, const uint32_t &, const script_data*);
    CONDITION_SCRIPT_BLOCK_COMMANDS_LIST
    ACTION_SCRIPT_BLOCK_COMMANDS_LIST
#undef SCRIPT_BLOCK_COMMAND_FUNC

#define SCRIPT_BLOCK_COMMAND_FUNC(name) void name##_init(const uint32_t &, const sol::object &, script_data*);
    CONDITION_SCRIPT_BLOCK_COMMANDS_LIST
    ACTION_SCRIPT_BLOCK_COMMANDS_LIST
#undef SCRIPT_BLOCK_COMMAND_FUNC
  }
}

// теперь когда я понял что у нас функции скрипта должны вызываться рекурсивно
// мне не обязательно возвращать что то кроме бул
// эти блоки обрабатывают и кондишоны и экшоны
// все что нужно сделать это в парсере проверить чтобы в кондишоне были кондишоны
// а в блоке экшона - экшоны

#endif
