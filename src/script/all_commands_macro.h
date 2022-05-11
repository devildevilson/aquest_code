#ifndef DEVILS_ENGINE_SCRIPT_ALL_COMMANDS_MACRO_H
#define DEVILS_ENGINE_SCRIPT_ALL_COMMANDS_MACRO_H

#include "logic_commands_macro.h"
#include "numeric_commands_macro.h"
#include "change_context_commands_macro.h"
#include "condition_commands_macro.h"
#include "action_commands_macro.h"
#include "get_scope_commands_macro.h"

//  COUNT_OBJECTS_COMMANDS_LIST

#define SCRIPT_COMMANDS_LIST \
  LOGIC_BLOCK_COMMANDS_LIST \
  NUMERIC_COMMANDS_LIST2 \
  GET_SCOPE_COMMANDS_FINAL_LIST \
  CHANGE_CONTEXT_COMMANDS_FINAL_LIST \
  CHARACTER_GET_BOOL_NO_ARGS_COMMANDS_LIST \
  CHARACTER_GET_NUMBER_NO_ARGS_COMMANDS_LIST \
  CHARACTER_GET_BOOL_ONE_ARG_COMMANDS_LIST \
  REALM_GET_BOOL_NO_ARGS_COMMANDS_LIST \
  REALM_GET_BOOL_EXISTED_ARG_COMMANDS_LIST \
  CASUS_BELLI_GET_BOOL_NO_ARGS_COMMANDS_LIST \
  CASUS_BELLI_GET_NUMBER_NO_ARGS_COMMANDS_LIST \
  ACTION_COMMANDS_LIST \
  COMMON_COMMAND_FUNC(root) \
  COMMON_COMMAND_FUNC(prev) \
  COMMON_COMMAND_FUNC(current) \
  COMMON_COMMAND_FUNC(index) \
  COMMON_COMMAND_FUNC(prev_index) \
  COMMON_COMMAND_FUNC(value) \
  COMMON_COMMAND_FUNC(selector) \
  COMMON_COMMAND_FUNC(sequence) \
  /*COMMON_COMMAND_FUNC(at_least_sequence)*/ \
  COMMON_COMMAND_FUNC(chance) \
  COMMON_COMMAND_FUNC(weighted_random) \
  COMMON_COMMAND_FUNC(random_value) \
  /*COMMON_COMMAND_FUNC(save_temporary)*/ \
  COMMON_COMMAND_FUNC(save_local) \
  COMMON_COMMAND_FUNC(save) \
  COMMON_COMMAND_FUNC(save_global) \
  COMMON_COMMAND_FUNC(has_local) \
  COMMON_COMMAND_FUNC(has_global) \
  COMMON_COMMAND_FUNC(remove_local) \
  COMMON_COMMAND_FUNC(remove_global) \
  COMMON_COMMAND_FUNC(assert_condition) \
  COMMON_COMMAND_FUNC(equals_to) \
  COMMON_COMMAND_FUNC(not_equals_to) \
  COMMON_COMMAND_FUNC(equality) \
  COMMON_COMMAND_FUNC(type_equality) \
  COMMON_COMMAND_FUNC(compare) \
  COMMON_COMMAND_FUNC(transform) \
  COMMON_COMMAND_FUNC(filter) \
  COMMON_COMMAND_FUNC(reduce) \
  COMMON_COMMAND_FUNC(take) \
  COMMON_COMMAND_FUNC(drop) \
  COMMON_COMMAND_FUNC(culture) \
  COMMON_COMMAND_FUNC(culture_group) \
  COMMON_COMMAND_FUNC(religion) \
  COMMON_COMMAND_FUNC(religion_group) \
  COMMON_COMMAND_FUNC(trait) \
  COMMON_COMMAND_FUNC(modificator) \
  COMMON_COMMAND_FUNC(titulus) \
  COMMON_COMMAND_FUNC(casus_belli) \
  COMMON_COMMAND_FUNC(building_type) \
  COMMON_COMMAND_FUNC(holding_type) \
  COMMON_COMMAND_FUNC(city_type) \
  COMMON_COMMAND_FUNC(troop_type) \
  COMMON_COMMAND_FUNC(law) \
  COMMON_COMMAND_FUNC(context) \
  COMMON_COMMAND_FUNC(local) \
  COMMON_COMMAND_FUNC(add_to_list) \
  COMMON_COMMAND_FUNC(is_in_list) \
  COMMON_COMMAND_FUNC(has_in_list) \
  COMMON_COMMAND_FUNC(random_in_list) \
  COMMON_COMMAND_FUNC(every_in_list) \
  COMMON_COMMAND_FUNC(list_view) \
  
// нужно сделать функцию указатель на текущий объект и еще их сравнивать
  
#define ONLY_INIT_LIST \
  COMMON_COMMAND_FUNC(number_container) \
  COMMON_COMMAND_FUNC(string_container) \
  COMMON_COMMAND_FUNC(object_container) \
  COMMON_COMMAND_FUNC(boolean_container) \
  COMMON_COMMAND_FUNC(compute_string) \
  COMMON_COMMAND_FUNC(compute_number) \
  /*COMMON_COMMAND_FUNC(complex_object)*/ \
  COMMON_COMMAND_FUNC(change_scope_condition) \
  COMMON_COMMAND_FUNC(change_scope_effect) \
  
#define SCRIPT_IGNORE_KEY_VALUES \
  SCRIPT_IGNORE_KEY_FUNC(condition) \
  /*SCRIPT_IGNORE_KEY_FUNC(count)*/ \
  SCRIPT_IGNORE_KEY_FUNC(percent) \
  SCRIPT_IGNORE_KEY_FUNC(weight) \
  SCRIPT_IGNORE_KEY_FUNC(hint) \
  SCRIPT_IGNORE_KEY_FUNC(text) \
  SCRIPT_IGNORE_KEY_FUNC(op) \
  SCRIPT_IGNORE_KEY_FUNC(list) \
  
#define COMPARE_OPERATORS_LIST \
  COMPARE_OPERATOR_FUNC(equal) \
  COMPARE_OPERATOR_FUNC(not_equal) \
  COMPARE_OPERATOR_FUNC(more) \
  COMPARE_OPERATOR_FUNC(less) \
  COMPARE_OPERATOR_FUNC(more_eq) \
  COMPARE_OPERATOR_FUNC(less_eq) \
  
#define COMPLEX_OBJECT_TOKENS_LIST \
  COMPLEX_OBJECT_TOKEN_FUNC(context) \
  
// скорее всего в будущем добавится
#define COMPLEX_VARIABLE_VALID_STRINGS_LIST \
  COMPLEX_VARIABLE_VALID_STRING_FUNC(root) \
  COMPLEX_VARIABLE_VALID_STRING_FUNC(prev) \
  COMPLEX_VARIABLE_VALID_STRING_FUNC(current) \
  
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
