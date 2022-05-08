set(SRC_LIST
  thread_pool.cpp
  stack_allocator.cpp
  frame_allocator.cpp
  input.cpp
  id.cpp
  globals.cpp
  logging.cpp
  frame_time.cpp
  random_engine.cpp
  linear_rng.cpp
  deferred_tasks.cpp
  lua_environment.cpp
  string_container.cpp
  string_bank.cpp
  progress_container.cpp
  #interface_container.cpp
  interface_container2.cpp
  localization_container.cpp
  #main_menu.cpp
  demiurge.cpp
  settings.cpp
  ../bin/interface2.cpp
  frustum.cpp
  #objects_selector.cpp
  ../bin/objects_selection.cpp
  astar_search.cpp
  astar_search_mt.cpp
  structures_utils.cpp
  type_info.cpp
  murmur.cpp
  #../../include/FastNoise/FastNoise.cpp
  ../../include/whereami/src/whereami.c
  ../../include/fmt/src/format.cc
  ../../include/fmt/src/os.cc
  # странно что это сработало вообще
  #../core/realm_mechanics_arrays.cpp
  #../core/stats_table.cpp
  #../core/traits_modifier_attribs.cpp
)

set(HDR_LIST
  thread_pool.h
  stack_allocator.h
  frame_allocator.h
  typeless_container.h
  memory_pool.h
  shared_application_constant.h
  shared_mathematical_constant.h
  shared_time_constant.h
  globals.h
  input.h
  id.h
  logging.h
  frame_time.h
  random_engine.h
  concurrent_vector.h
  linear_rng.h
  table_container.h
  bit_field.h
  deferred_tasks.h
  lua_environment.h
  string_container.h
  string_bank.h
  progress_container.h
  #interface_container.h
  interface_container2.h
  localization_container.h
  #main_menu.h
  demiurge.h
  settings.h
  ../bin/interface2.h
  frustum.h
  #../bin/objects_selector.h
  ../bin/objects_selection.h
  astar_search.h
  astar_search_mt.h
  structures_utils.h
  type_info.h
  murmur.h
  pod_container.h
  ../../include/FastNoise/Cpp/FastNoiseLite.h
  ../../include/whereami/src/whereami.h

  #../core/realm_mechanics.h
  #../core/realm_mechanics_arrays.h
  #../core/stats_table.h
)

# if (UNIX)
#   add_library(lua53 SHARED IMPORTED GLOBAL)
#   set_property(TARGET lua53 PROPERTY IMPORTED_LOCATION "/usr/lib/liblua5.3.so")
# endif()
add_library(utils ${SRC_LIST} ${HDR_LIST})
target_link_libraries(utils) # PUBLIC foonathan_memory moonnuklear
