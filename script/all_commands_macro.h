#ifndef DEVILS_ENGINE_SCRIPT_ALL_COMMANDS_MACRO_H
#define DEVILS_ENGINE_SCRIPT_ALL_COMMANDS_MACRO_H

#include "logic_commands_macro.h"
#include "numeric_commands_macro.h"
#include "change_context_commands_macro.h"
#include "condition_commands_macro.h"
#include "action_commands_macro.h"

//  COUNT_OBJECTS_COMMANDS_LIST

#define SCRIPT_COMMANDS_LIST \
  LOGIC_BLOCK_COMMANDS_LIST \
  NUMERIC_COMMANDS_LIST2 \
  GET_SCOPE_COMMANDS_LIST \
  CHANGE_CONTEXT_COMMANDS_LIST \
  CONDITION_COMMANDS_LIST \
  CHARACTER_CONDITION_STRING_CHECK_COMMANDS_LIST \
  ACTION_COMMANDS_LIST \
  COMMON_COMMAND_FUNC(root) \
  COMMON_COMMAND_FUNC(prev) \
  
#define ONLY_INIT_LIST \
  COMMON_COMMAND_FUNC(number_container) \
  COMMON_COMMAND_FUNC(string_container) \
  COMMON_COMMAND_FUNC(object_container) \
  COMMON_COMMAND_FUNC(boolean_container) \
  COMMON_COMMAND_FUNC(compute_string) \
  COMMON_COMMAND_FUNC(complex_object) \
  COMMON_COMMAND_FUNC(change_scope_condition) \
  COMMON_COMMAND_FUNC(change_scope_effect) \
  
#define SCRIPT_IGNORE_KEY_VALUES \
  SCRIPT_IGNORE_KEY_FUNC(condition) \
  /*SCRIPT_IGNORE_KEY_FUNC(count)*/ \
  SCRIPT_IGNORE_KEY_FUNC(percent) \
  SCRIPT_IGNORE_KEY_FUNC(weight) \
  
#define COMPARE_OPERATORS_LIST \
  COMPARE_OPERATOR_FUNC(equal) \
  COMPARE_OPERATOR_FUNC(not_equal) \
  COMPARE_OPERATOR_FUNC(more) \
  COMPARE_OPERATOR_FUNC(less) \
  COMPARE_OPERATOR_FUNC(more_eq) \
  COMPARE_OPERATOR_FUNC(less_eq) \
  
#define COMPLEX_OBJECT_TOKENS_LIST \
  COMPLEX_OBJECT_TOKEN_FUNC(context) \
  
// добавятся такие вещи как: годовой/месячный доход, 

//#define COMMAND_NAME_FUNC(name)

/*
#define GET_ALL_COMMANDS_NAMES \
  #define LOGIC_BLOCK_COMMAND_FUNC(name, a, b) COMMAND_NAME_FUNC(name)   \
  #define NUMERIC_COMMAND_BLOCK_FUNC(name, a, b) COMMAND_NAME_FUNC(name) \
  #define NUMERIC_COMMAND_FUNC(name, a, b) COMMAND_NAME_FUNC(name)       \
  #define GET_SCOPE_COMMAND_FUNC(name, a, b) COMMAND_NAME_FUNC(name)     \
  #define CHANGE_CONTEXT_COMMAND_FUNC(name, a, b, c) COMMAND_NAME_FUNC(name) \
  #define CONDITION_COMMAND_FUNC(name) COMMAND_NAME_FUNC(name)           \
  #define ACTION_COMMAND_FUNC(name) COMMAND_NAME_FUNC(name)              \
  #define COMMON_COMMAND_FUNC(name) COMMAND_NAME_FUNC(name)              \
          SCRIPT_COMMANDS_LIST \
  #undef LOGIC_BLOCK_COMMAND_FUNC    \
  #undef NUMERIC_COMMAND_BLOCK_FUNC  \
  #undef NUMERIC_COMMAND_FUNC        \
  #undef GET_SCOPE_COMMAND_FUNC      \
  #undef CHANGE_CONTEXT_COMMAND_FUNC \
  #undef CONDITION_COMMAND_FUNC      \
  #undef ACTION_COMMAND_FUNC         \
  #undef COMMON_COMMAND_FUNC         \
  
*/

#endif
