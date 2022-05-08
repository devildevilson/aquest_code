# наверное нужно перенести еще shared_battle_structures.h

set(SRC_LIST
  context.cpp
  structures.cpp
  map.cpp
  troop_parser.cpp
  troop_type_parser.cpp
  unit_state_parser.cpp
  lua_states.cpp
  render_stages.cpp
)

set(HDR_LIST
  context.h
  structures.h
  map.h
  troop_parser.h
  troop_type_parser.h
  unit_state_parser.h
  lua_states.h
  render_stages.h
)

add_library(battle ${SRC_LIST} ${HDR_LIST})
target_link_libraries(battle render systems utils)
