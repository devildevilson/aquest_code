set(PROJECT_DIR ${CMAKE_CURRENT_BINARY_DIR}/tests)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_DIR})

macro(build_tests tests_name)
  foreach(name ${tests_name})
    set(SRC_LIST ../bin/tiles_funcs.cpp ../bin/data_parser.cpp ${name}.cpp )
    set(HDR_LIST )
    add_executable(${name} ${SRC_LIST} ${HDR_LIST})
    target_link_libraries(${name} script core render battle ai utils serialization ${LUA_LIB})
  endforeach(name ${tests_name})
endmacro(build_tests)

set(f "script_tests")
build_tests(${f})
