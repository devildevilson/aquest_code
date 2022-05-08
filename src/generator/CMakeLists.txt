set(SRC_LIST
  # map.cpp
  system.cpp
  #generator_system2.cpp
  container.cpp
)

set(HDR_LIST
  # map.h
  system.h
  #generator_system2.h
  context2.h
  container.h
)

add_library(generator ${SRC_LIST} ${HDR_LIST})
target_link_libraries(generator systems utils)
