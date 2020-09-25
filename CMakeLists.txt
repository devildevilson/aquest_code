cmake_minimum_required(VERSION 3.1)
project(apates_quest) # 'эпати'

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME})

set(TARGET_NAME ${PROJECT_NAME})

set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 1)
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")
set(PROJECT_VERSION_FULL "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

## Git revision number ##
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
  execute_process(COMMAND git describe --tags HEAD
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT_VARIABLE GIT_DESCRIBE_TAGS ERROR_QUIET)
  if(GIT_DESCRIBE_TAGS)
    string(REGEX REPLACE "^v(.*)" "\\1" GIT_REVISION "${GIT_DESCRIBE_TAGS}")
    string(STRIP "${GIT_REVISION}" GIT_REVISION)
    if(GIT_REVISION)
      set(PROJECT_VERSION_FULL "${GIT_REVISION}")
    endif(GIT_REVISION)
  endif(GIT_DESCRIBE_TAGS)
endif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")

find_package(git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
# Update submodules as needed
  option(GIT_SUBMODULE "Check submodules during build" ON)
  if(GIT_SUBMODULE)
    message(STATUS "Submodule update")
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    RESULT_VARIABLE GIT_SUBMOD_RESULT)
    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
      message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
    endif()
  endif()
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/include/fmt/CMakeLists.txt")
  message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()

set(STB_INCLUDE_PATH include/stb)
set(LUA_LIB_PATH ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/scripts)

include_directories(
  .
  include/sol2/include/
  include/lanes/src/
  include/VulkanMemoryAllocator/src/
  include/FastNoise/
  include/
  include/json/single_include/
  include/stb/
  include/whereami/src/
  include/fmt/include/
  include/memory/include/
  include/magic_enum/include/
  include/lz4/lib/
  include/protobuf/src/
  include/cista/include/
)
link_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
  #${LUA_LIB_PATH} # требуется для moonnuklear
  libs/
)

if (WIN32)
  set(CPPFS_LIB_PATH C:/libs/cppfs)

  set(GLFW3_LIB_DIRS C:/libs/glfw-3.2.1.bin.WIN64/lib-mingw-w64)
  set(GLFW3_INCLUDE_DIRS C:/libs/glfw-3.2.1.bin.WIN64/include)
  set(GLFW3_LIBRARIES glfw3)

  set(VULKAN_SDK_PATH C:/VulkanSDK/1.1.97.0)
  set(VULKAN_LIB_PATH ${VULKAN_SDK_PATH}/Lib)
  set(VULKAN_INCLUDE_PATH ${VULKAN_SDK_PATH}/Include)
  set(GLSL_VALIDATOR "${VULKAN_SDK_PATH}/Bin/glslangValidator")
  set(VULKAN_LIB_NAME vulkan-1)

  include_directories(
    ${GLFW3_INCLUDE_DIRS}
	  ${VULKAN_INCLUDE_PATH}
	  ${GLM_LIB_PATH}
  )

  link_directories(
	  ${GLFW3_LIB_DIRS}
	  ${VULKAN_LIB_PATH}
    ${CPPFS_LIB_PATH}
  )
else()
  set(CPPFS_LIB_PATH libs)
  set(MEMORY_LIB_PATH include/memory/build/src/)

  # находим указанные зависимости (нужно чекнуть все ли это зависимости?)
  # вместо glm у меня установлен gli
  #find_package(glm)
  find_package(glfw3)
  find_package(PkgConfig REQUIRED)
  #pkg_search_module(GLFW REQUIRED glfw3)
  pkg_check_modules(GLFW3 REQUIRED glfw3)
  # не может найти почему то vulkan-headers
  #find_package(vulkan-headers)

  set(VULKAN_LIB_NAME vulkan)
  set(GLSL_VALIDATOR "glslangValidator")

  include_directories(
    ${GLFW3_INCLUDE_DIRS}
  )

  link_directories(
    ${CPPFS_LIB_PATH}
    ${MEMORY_LIB_PATH}
  )
endif()

set(COMPILED_SHADERS_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders)

file(MAKE_DIRECTORY ${COMPILED_SHADERS_PATH})

if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE Debug ... FORCE)
endif()

add_definitions(
  -DTECHNICAL_NAME="${PROJECT_NAME}"
  -DAPP_NAME="Apate's quest"
  -DAPP_VERSION=${PROJECT_VERSION_FULL}
)

if (CMAKE_BUILD_TYPE MATCHES "Debug")
  # -fsanitize=address -fsanitize=pointer-compare -fsanitize=pointer-subtract -fsanitize=undefined
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2 -std=c++17 -Wall -Wextra -g -fno-rtti -pthread")
else()
  add_definitions(-D_NDEBUG)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2 -std=c++17 -Wall -Wextra -O3 -s -fno-rtti -pthread")
endif (CMAKE_BUILD_TYPE MATCHES "Debug")

file(GLOB_RECURSE GLSL_SOURCE_FILES
  raw_shaders/tiles.vert
  raw_shaders/tiles.geom
  raw_shaders/tiles.frag
  raw_shaders/tiles.comp
  raw_shaders/tiles_borders.frag
  raw_shaders/gui.frag
  raw_shaders/gui.vert
  raw_shaders/borders.comp
  raw_shaders/borders.vert
  raw_shaders/borders.frag
  raw_shaders/one_tile.vert
  raw_shaders/one_tile.frag
  raw_shaders/walls.comp
  raw_shaders/walls.vert
)

foreach(GLSL ${GLSL_SOURCE_FILES})
  get_filename_component(FILE_NAME ${GLSL} NAME_WE)
  get_filename_component(FILE_EXT ${GLSL} EXT)
  set(SPIRV "${COMPILED_SHADERS_PATH}/${FILE_NAME}${FILE_EXT}.spv")
  add_custom_command(
    OUTPUT ${SPIRV}
#     COMMAND ${CMAKE_COMMAND} -E make_directory "${COMPILED_SHADERS_PATH}"
    COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
    DEPENDS ${GLSL}
  )
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
  shaders
  DEPENDS ${SPIRV_BINARY_FILES}
)

set(SRC_LIST
  utils/lib_crypto_fake_rtti.cpp
#   include/cryptopp/hc256.cpp
#   include/cryptopp/sha.cpp
  utils/serializator_helper.cpp
#   protobuf/test1.pb.cc
)

set(HDR_LIST
  include/cryptopp/hc256.h
  include/cryptopp/sha.h
  utils/serializator_helper.h
#   protobuf/test1.pb.h
)

add_library(serialization ${SRC_LIST} ${HDR_LIST})
target_link_libraries(serialization lz4 cryptopp z) #protobuf-lite protobuf

set(SRC_LIST
  utils/thread_pool.cpp
  utils/stack_allocator.cpp
  utils/input.cpp
  utils/id.cpp
  utils/globals.cpp
  utils/logging.cpp
  utils/frame_time.cpp
  utils/ecs.cpp
  utils/random_engine.cpp
  utils/linear_rng.cpp
  utils/lua_initialization.cpp
  utils/string_container.cpp
  utils/string_bank.cpp
  utils/progress_container.cpp
  utils/interface_container.cpp
  utils/main_menu.cpp
  utils/demiurge.cpp
  bin/interface2.cpp
  include/FastNoise/FastNoise.cpp
  include/whereami/src/whereami.c
  include/fmt/src/format.cc
  include/fmt/src/os.cc
)

set(HDR_LIST
  utils/thread_pool.h
  utils/stack_allocator.h
  utils/typeless_container.h
  utils/memory_pool.h
  utils/shared_application_constant.h
  utils/shared_mathematical_constant.h
  utils/shared_time_constant.h
  utils/globals.h
  utils/input.h
  utils/id.h
  utils/logging.h
  utils/frame_time.h
  utils/ecs.h
  utils/yacs.h
  utils/yacs_pool.h
  utils/yacs_component.h
  utils/random_engine.h
  utils/concurrent_vector.h
  utils/linear_rng.h
  utils/table_container.h
  utils/bit_field.h
  utils/on_action_types.h
  utils/traits_modifier_attribs.h
  utils/lua_initialization.h
  utils/string_container.h
  utils/string_bank.h
  utils/progress_container.h
  utils/interface_container.h
  utils/main_menu.h
  utils/demiurge.h
  bin/interface2.h
  include/FastNoise/FastNoise.h
  include/whereami/src/whereami.h
)

add_library(utils ${SRC_LIST} ${HDR_LIST})
target_link_libraries(utils lua pthread) # PUBLIC foonathan_memory moonnuklear

set(SRC_LIST
  render/container.cpp
  render/Core.cpp
  render/Internal.cpp
  render/Makers.cpp
  render/RAII.cpp
  render/render.cpp
  render/Tasks.cpp
  render/Types.cpp
  render/window.cpp
  render/stages.cpp
  render/targets.cpp
  render/render_mode_container.cpp
  render/pipeline_mode_updater.cpp
  render/image_container.cpp
  render/persistent_resources.cpp
  render/slots.cpp
)

set(HDR_LIST
  render/container.h
  render/context.h
  render/shared_structures.h
  render/stage.h
  render/stages.h
  render/target.h
  render/targets.h
  render/yavf.h
  render/Core.h
  render/Internal.h
  render/Makers.h
  render/RAII.h
  render/render.h
  render/Tasks.h
  render/Types.h
  render/window.h
  render/nuklear_header.h
  render/render_mode_container.h
  render/pipeline_mode_updater.h
  render/image_container.h
  render/persistent_resources.h
  render/slots.h
)

add_library(render ${SRC_LIST} ${HDR_LIST})
target_link_libraries(render utils ${VULKAN_LIB_NAME} ${GLFW3_LIBRARIES})

set(SRC_LIST
  ai/build_subsystem.cpp
)

set(HDR_LIST
  ai/sub_system.h
  ai/build_subsystem.h
)

add_library(ai ${SRC_LIST} ${HDR_LIST})
target_link_libraries(ai utils)

set(SRC_LIST
  utils/systems.cpp
  bin/loading_functions.cpp
  bin/game_time.cpp
)

set(HDR_LIST
  utils/systems.h
  bin/loading_functions.h
  bin/game_time.h
)

add_library(systems ${SRC_LIST} ${HDR_LIST})
target_link_libraries(systems render utils)

set(SRC_LIST
  bin/interface_context.cpp
  #bin/interfaces.cpp
  bin/overlay.cpp
  bin/figures.cpp
  bin/main.cpp
  bin/helper.cpp
  bin/camera.cpp
  bin/generator_system2.cpp
  bin/map_generators2.cpp
  bin/map.cpp
  bin/generator_container.cpp
  #include/lanes/src/deep.c
  #include/lanes/src/universe.c
  #include/lanes/src/compat.c
  bin/character.cpp
  bin/map_creator.cpp
  bin/core_context.cpp
  bin/titulus_data_parser.cpp
  bin/province_data_parser.cpp
  bin/city_type_data_parser.cpp
  bin/city_data_parser.cpp
  bin/character_data_parser.cpp
  bin/building_data_parser.cpp
  bin/logic.cpp
  utils/quest_states.cpp
)

set(HDR_LIST
  bin/interface_context.h
  #bin/interfaces.h
  bin/overlay.h
  bin/helper.h
  bin/figures.h
  bin/camera.h
  bin/generator_system2.h
  bin/map_generators2.h
  bin/map.h
  bin/generator_context2.h
  bin/generator_container.h
  bin/map_generator.h
  bin/character.h
  bin/core_structures.h
  bin/declare_structures.h
  bin/map_creator.h
  bin/core_context.h
  bin/stats.h
  bin/data_parser.h
  bin/logic.h
  utils/quest_states.h
  utils/quest_state.h
)
add_executable(${TARGET_NAME} ${SRC_LIST} ${HDR_LIST})
target_link_libraries(${TARGET_NAME} systems ai render utils serialization)
add_dependencies(${TARGET_NAME} shaders)

# Strip binary for release builds
if (CMAKE_BUILD_TYPE STREQUAL Release)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_STRIP} -s "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET_NAME}")
endif()
