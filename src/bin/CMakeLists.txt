set(SRC_LIST
  interface_context.cpp
  figures.cpp
  camera.cpp
  map_creator.cpp
  data_parser.cpp
  image_parser.cpp
  heraldy_parser.cpp
  logic.cpp
  #../utils/quest_states.cpp
  tiles_funcs.cpp

  application.cpp
  game_resources.cpp
  main.cpp
  #bin/helper.cpp
)

set(HDR_LIST
  interface_context.h
  overlay.h
  figures.h
  camera.h
  core_structures.h
  map_creator.h
  data_parser.h
  image_parser.h
  heraldy_parser.h
  logic.h
  #../utils/quest_states.h
  #../utils/quest_state.h
  tiles_funcs.h

  application.h
  game_resources.h
  #bin/helper.h
)
add_executable(${TARGET_NAME} ${SRC_LIST} ${HDR_LIST})
target_link_libraries(${TARGET_NAME} systems lua_init core battle generator script ai render utils serialization tcmalloc unwind)
add_dependencies(${TARGET_NAME} shaders)
