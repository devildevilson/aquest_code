# old files located to exclude/
set(SRC_LIST
  on_action_container.cpp
  on_action_types_arrays.cpp
  #init_functions.cpp
  core.cpp
  container.cpp
  context.cpp
  numeric_functions.cpp
  #condition_functions.cpp
  #change_context_commands.cpp
  #get_scope_commands.cpp
  logic_commands.cpp
  action_functions.cpp
  common_commands.cpp
  #header.cpp
  object.cpp
  system.cpp
  #type_info.cpp
  functions_init.cpp
  iterator_funcs.cpp
)

set(HDR_LIST
  on_action_types.h
  on_action_container.h
  on_action_types_arrays.h
  numeric_commands_macro.h
  numeric_functions.h
  condition_commands_macro.h
  #condition_functions.h
  action_commands_macro.h
  action_functions.h
  change_context_commands_macro.h
  #change_context_commands.h
  get_scope_commands_macro.h
  #get_scope_commands.h
  logic_commands_macro.h
  logic_commands.h
  common_commands.h
  #init_functions.h
  core.h
  header.h
  container.h
  context.h
  input_data.h
  interface.h
  object.h
  system.h
  type_info.h
  type_traits.h
  templates.h
  functions_init.h
  iterator_funcs.h
)

add_library(script ${SRC_LIST} ${HDR_LIST})
target_link_libraries(script utils re2)
