# include_directories шарятся для всех add_subdirectory

set(SRC_LIST
  build_subsystem.cpp
  path_container.cpp
  path_finding_data.cpp
)

set(HDR_LIST
  sub_system.h
  build_subsystem.h
  path_container.h
  path_finding_data.h
)

add_library(ai ${SRC_LIST} ${HDR_LIST})
target_link_libraries(ai utils)
