cmake_minimum_required(VERSION 3.1)
project(apates_quest) # 'эпати'

set(PROJECT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_DIR}/bin)

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
set(LUA_LIB_PATH ${PROJECT_DIR}/scripts)

include_directories(
  .
  #include/sol2/include/
  include/sol2_release/
  include/lanes/src/
  #include/VulkanMemoryAllocator/src/
  include/FastNoise/
  #include/
  include/json/single_include/
  include/stb/
  include/whereami/src/
  include/fmt/include/
  include/memory/include/
  include/magic_enum/include/
  include/lz4/lib/
  include/protobuf/src/
  include/cista/include/
  include/glm/
  include/parallel-hashmap/
  include/Nuklear/
  include/stb/
  include/re2/
  include/function2/include/function2/
)
link_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
  #${LUA_LIB_PATH} # требуется для moonnuklear
  libs/
)

set(CMAKE_SKIP_BUILD_RPATH TRUE)
#set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
#set(CMAKE_INSTALL_RPATH "")
#set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)

if (WIN32 OR MINGW)
  set(CPPFS_LIB_PATH C:/libs/cppfs)

  set(GLFW3_LIB_DIRS C:/libs/glfw-3.3.2.bin.WIN64/lib-vc2019)
  set(GLFW3_INCLUDE_DIRS C:/libs/glfw-3.3.2.bin.WIN64/include)
  set(GLFW3_LIBRARIES glfw3dll)

  set(VULKAN_SDK_PATH C:/VulkanSDK/1.2.162.0)
  set(VULKAN_LIB_PATH ${VULKAN_SDK_PATH}/Lib)
  set(VULKAN_INCLUDE_PATH ${VULKAN_SDK_PATH}/Include)
  set(GLSL_VALIDATOR "${VULKAN_SDK_PATH}/Bin/glslangValidator")
  set(VULKAN_LIB_NAME vulkan-1)
  set(LUA_LIB lua53)

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
  set(LUA_LIB lua5.3)
  set(LUA_LIBRARIES /usr/include/lua5.3)

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

  include_directories(SYSTEM
    ${LUA_LIBRARIES}
  )

  link_directories(
    ${CPPFS_LIB_PATH}
    ${MEMORY_LIB_PATH}
  )
endif()

set(COMPILED_SHADERS_PATH ${PROJECT_DIR}/shaders)

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
  if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17 /ZI /MTd /GR- /permissive- /bigobj") #/W3
  else()
    #-fsanitize=address -fsanitize=pointer-compare -fsanitize=pointer-subtract -fsanitize=undefined -fno-omit-frame-pointer
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2 -std=c++17 -Wall -Wextra -g -fno-rtti -pthread -Wl,-rpath=./")
  endif()
else()
  add_definitions(-D_NDEBUG)
  if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17 /Ox /MT /GR- /permissive- /GL") # если разделить кучу файлов lua_initialization, то можно обойтись без /bigobj
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2 -std=c++17 -Wall -Wextra -O3 -s -fno-rtti -pthread -Wl,-rpath=./")
  endif()
endif (CMAKE_BUILD_TYPE MATCHES "Debug")

# пока не уверен нужно ли мне это
# Second, for multi-config builds (e.g. msvc)
# foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
#     string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
#     set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${youroutputdirectory} )
#     set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${youroutputdirectory} )
#     set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${youroutputdirectory} )
# endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

file(GLOB_RECURSE GLSL_SOURCE_FILES
  raw_shaders/tiles.vert
  #raw_shaders/tiles.geom
  raw_shaders/tiles.frag
  raw_shaders/tiles.comp
  raw_shaders/tile_data.comp
  raw_shaders/tiles_borders.frag
  raw_shaders/gui.frag
  raw_shaders/gui.vert
  #raw_shaders/borders.comp
  raw_shaders/borders.vert
  raw_shaders/borders.frag
  raw_shaders/one_tile.vert
  raw_shaders/one_tile.frag
  #raw_shaders/walls.comp
  raw_shaders/walls.vert
  raw_shaders/first_object.vert
  raw_shaders/first_object.frag
  raw_shaders/structure.vert
  raw_shaders/heraldy.vert
  raw_shaders/heraldy.frag
  raw_shaders/map_objects.comp
  raw_shaders/armies.vert
  raw_shaders/battle_map_tiles.comp
  raw_shaders/battle_map_tiles.vert
  raw_shaders/battle_map_tiles.frag
  raw_shaders/battle_biome_objects.vert
  raw_shaders/battle_biome_objects.frag
  raw_shaders/battle_map_unit.vert
  raw_shaders/battle_map_unit.frag
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
  #utils/lib_crypto_fake_rtti.cpp
#   include/cryptopp/hc256.cpp
#   include/cryptopp/sha.cpp
  utils/salsa20.cpp
  utils/serializator_helper.cpp
#   protobuf/test1.pb.cc
)

set(HDR_LIST
  #include/cryptopp/hc256.h
  #include/cryptopp/sha.h
  utils/picosha2.h
  utils/salsa20.h
  utils/serializator_helper.h
#   protobuf/test1.pb.h
)

add_library(serialization ${SRC_LIST} ${HDR_LIST})
target_link_libraries(serialization lz4) #protobuf-lite protobuf

set(SRC_LIST
  utils/thread_pool.cpp
  utils/stack_allocator.cpp
  utils/input.cpp
  utils/id.cpp
  utils/globals.cpp
  utils/logging.cpp
  utils/frame_time.cpp
  #utils/ecs.cpp
  utils/random_engine.cpp
  utils/linear_rng.cpp
  utils/lua_initialization_game_logic.cpp
  utils/lua_initialization_input.cpp
  utils/lua_initialization_types.cpp
  utils/lua_initialization_main_menu.cpp
  utils/lua_initialization_constants.cpp
  utils/lua_initialization_package.cpp
  utils/lua_initialization_settings.cpp
  utils/lua_initialization_tiles.cpp
  utils/lua_initialization_internal_enums.cpp
  utils/lua_initialization_internal_building_type.cpp
  utils/lua_initialization_internal_character.cpp
  utils/lua_initialization_internal_city.cpp
  utils/lua_initialization_internal_city_type.cpp
  utils/lua_initialization_internal_faction.cpp
  utils/lua_initialization_internal_province.cpp
  utils/lua_initialization_internal_titulus.cpp
  utils/lua_initialization_internal_army.cpp
  utils/lua_initialization_internal_war.cpp
  utils/lua_initialization_internal_religion.cpp
  utils/lua_initialization_internal_culture.cpp
  utils/lua_initialization_internal_troop.cpp
  utils/lua_initialization_internal_troop_type.cpp
  #utils/lua_initialization_internal_trait.cpp
  #utils/lua_initialization_internal_modificator.cpp
  #utils/lua_initialization_internal_casus_belli.cpp
  utils/lua_initialization_battle_map.cpp
  utils/lua_initialization_generator_container.cpp
  utils/lua_initialization_noiser.cpp
  utils/lua_initialization_random_engine.cpp
  utils/lua_initialization_progress_container.cpp
  utils/lua_initialization_battle_generator_functions.cpp
  utils/lua_initialization_battle_unit.cpp
  utils/lua_initialization_world_map.cpp
  utils/lua_initialization_world_map_generator_functions.cpp
  utils/lua_initialization_calendar.cpp
  utils/lua_initialization_safe_utils.cpp
  utils/lua_initialization_interface_utils.cpp
  utils/lua_initialization_game_context.cpp
  utils/lua_initialization_localization.cpp
  utils/lua_initialization_core_loading.cpp
  utils/lua_initialization_camera.cpp
  utils/lua_initialization_selection.cpp
  #utils/lua_initialization_script_utils.cpp
  utils/lua_initialization.cpp
  utils/deferred_tasks.cpp
  utils/lua_environment.cpp
  utils/string_container.cpp
  utils/string_bank.cpp
  utils/progress_container.cpp
  #utils/interface_container.cpp
  utils/interface_container2.cpp
  utils/localization_container.cpp
  #utils/main_menu.cpp
  utils/demiurge.cpp
  utils/settings.cpp
  bin/interface2.cpp
  utils/frustum.cpp
  #bin/objects_selector.cpp
  bin/objects_selection.cpp
  utils/astar_search.cpp
  utils/astar_search_mt.cpp
  utils/battle_lua_states.cpp
  bin/battle_context.cpp
  bin/battle_structures.cpp
  utils/structures_utils.cpp
  #include/FastNoise/FastNoise.cpp
  include/whereami/src/whereami.c
  include/fmt/src/format.cc
  include/fmt/src/os.cc
  # странно что это сработало вообще
  #core/realm_mechanics_arrays.cpp
  #core/stats_table.cpp
  #core/traits_modifier_attribs.cpp
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
  #utils/ecs.h
  #utils/yacs.h
  #utils/yacs_pool.h
  #utils/yacs_component.h
  utils/random_engine.h
  utils/concurrent_vector.h
  utils/linear_rng.h
  utils/table_container.h
  utils/bit_field.h
  utils/lua_initialization.h
  utils/lua_initialization_hidden.h
  utils/lua_initialization_internal.h
  utils/deferred_tasks.h
  utils/lua_environment.h
  utils/string_container.h
  utils/string_bank.h
  utils/progress_container.h
  #utils/interface_container.h
  utils/interface_container2.h
  utils/localization_container.h
  #utils/main_menu.h
  utils/demiurge.h
  utils/settings.h
  bin/interface2.h
  utils/frustum.h
  #bin/objects_selector.h
  bin/objects_selection.h
  utils/astar_search.h
  utils/astar_search_mt.h
  utils/battle_lua_states.h
  bin/battle_context.h
  bin/battle_structures.h
  utils/structures_utils.h
  include/FastNoise/Cpp/FastNoiseLite.h
  include/whereami/src/whereami.h
  
  #core/realm_mechanics.h
  #core/realm_mechanics_arrays.h
  #core/stats_table.h
)

# if (UNIX)
#   add_library(lua53 SHARED IMPORTED GLOBAL)
#   set_property(TARGET lua53 PROPERTY IMPORTED_LOCATION "/usr/lib/liblua5.3.so")
# endif()
add_library(utils ${SRC_LIST} ${HDR_LIST})
target_link_libraries(utils ${LUA_LIB}) # PUBLIC foonathan_memory moonnuklear
# нужно сделать одинаковый файл что в линуксе что в винде
# в любом случае нужно компилировать с одной версией луа

set(SRC_LIST
  #render/Core.cpp
  #render/Internal.cpp
  #render/Makers.cpp
  #render/RAII.cpp
  #render/Tasks.cpp
  #render/Types.cpp
  render/container.cpp
  render/render.cpp
  render/window.cpp
  render/stages.cpp
  render/targets.cpp
  render/render_mode_container.cpp
  #render/pipeline_mode_updater.cpp
  render/image_container.cpp
  render/image_controller.cpp
  #render/persistent_resources.cpp
  render/slots.cpp
  render/battle_render_stages.cpp
  render/utils.cpp
  render/makers.cpp
  render/vulkan_hpp_header.cpp
  render/vk_mem_alloc_impl.cpp
)

set(HDR_LIST
  #render/yavf.h
  #render/Core.h
  #render/Internal.h
  #render/Makers.h
  #render/RAII.h
  #render/Tasks.h
  #render/Types.h
  render/container.h
  render/context.h
  render/shared_structures.h
  render/stage.h
  render/stages.h
  render/target.h
  render/targets.h
  render/render.h
  render/window.h
  render/nuklear_header.h
  render/render_mode_container.h
  #render/pipeline_mode_updater.h
  render/image_container.h
  render/image_controller.h
  #render/persistent_resources.h
  render/slots.h
  render/shared_battle_structures.h
  render/battle_render_stages.h
  render/utils.h
  render/vk_mem_alloc.hpp
  render/vulkan_declarations.h
  render/vulkan_hpp_header.h
  render/makers.h
)

add_library(render ${SRC_LIST} ${HDR_LIST})
target_link_libraries(render utils ${VULKAN_LIB_NAME} ${GLFW3_LIBRARIES})

set(SRC_LIST
  ai/build_subsystem.cpp
  ai/path_container.cpp
  ai/path_finding_data.cpp
)

set(HDR_LIST
  ai/sub_system.h
  ai/build_subsystem.h
  ai/path_container.h
  ai/path_finding_data.h
)

add_library(ai ${SRC_LIST} ${HDR_LIST})
target_link_libraries(ai utils)

# old files located to exclude/
set(SRC_LIST
  script/on_action_container.cpp
  script/on_action_types_arrays.cpp
  script/init_functions.cpp
  script/core.cpp
  script/container.cpp
  script/context.cpp
  script/numeric_functions.cpp
  script/condition_functions.cpp
  script/change_context_commands.cpp
  script/logic_commands.cpp
  script/action_functions.cpp
  script/common_commands.cpp
  script/header.cpp
  script/object.cpp
)

set(HDR_LIST
  script/on_action_types.h
  script/on_action_container.h
  script/on_action_types_arrays.h
  script/numeric_commands_macro.h
  script/numeric_functions.h
  script/condition_commands_macro.h
  script/condition_functions.h
  script/action_commands_macro.h
  script/action_functions.h
  script/change_context_commands_macro.h
  script/change_context_commands.h
  script/logic_commands_macro.h
  script/logic_commands.h
  script/common_commands.h
  script/init_functions.h
  script/core.h
  script/header.h
  script/container.h
  script/context.h
  script/input_data.h
  script/interface.h
  script/object.h
)

add_library(script ${SRC_LIST} ${HDR_LIST})
target_link_libraries(script utils re2)

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
  core/realm_mechanics_arrays.cpp
  core/stats_table.cpp
  core/traits_modifier_attribs.cpp
  core/decision.cpp
  core/army.cpp
  core/building_type.cpp
  core/character.cpp
  core/character_data_parser.cpp
  core/city_type.cpp
  core/city.cpp
  core/culture.cpp
  core/event.cpp
  core/hero_troop.cpp
  core/law.cpp
  core/modificator.cpp
  core/province.cpp
  core/realm.cpp
  core/religion.cpp
  core/tile.cpp
  core/titulus.cpp
  core/trait.cpp
  core/troop_type.cpp
  core/troop.cpp
  core/context.cpp
  core/interaction.cpp
  core/war.cpp
  core/casus_belli.cpp
  core/internal_lua_state.cpp
  core/generator_begin.cpp
  core/biome.cpp
  core/declare_structures_table.cpp
)

set(HDR_LIST
  core/realm_mechanics.h
  core/realm_mechanics_arrays.h
  core/stats_table.h
  core/stats.h
  core/traits_modifier_attribs.h
  core/casus_belli.h
  core/war.h
  core/decision.h
  core/army.h
  core/building_type.h
  core/character.h
  core/city_type.h
  core/city.h
  core/culture.h
  core/event.h
  core/hero_troop.h
  core/law.h
  core/modificator.h
  core/province.h
  core/realm.h
  core/religion.h
  core/tile.h
  core/titulus.h
  core/trait.h
  core/troop_type.h
  core/troop.h
  core/context.h
  core/interaction.h
  core/internal_lua_state.h
  core/generator_begin.h
  core/biome.h
  core/declare_structures_table.h
)

add_library(core ${SRC_LIST} ${HDR_LIST})
target_link_libraries(core systems utils)

set(SRC_LIST
  bin/interface_context.cpp
  #bin/interfaces.cpp
  #bin/overlay.cpp
  bin/figures.cpp
  bin/main.cpp
  bin/helper.cpp
  bin/camera.cpp
  bin/generator_system2.cpp
  #bin/map_generators2.cpp
  bin/map.cpp
  bin/generator_container.cpp
  #include/lanes/src/deep.c
  #include/lanes/src/universe.c
  #include/lanes/src/compat.c
  #bin/character.cpp
  bin/map_creator.cpp
  #bin/titulus_data_parser.cpp
  #bin/province_data_parser.cpp
  #bin/city_type_data_parser.cpp
  #bin/city_data_parser.cpp
  #bin/character_data_parser.cpp
  #bin/building_data_parser.cpp
  bin/data_parser.cpp
  bin/image_parser.cpp
  bin/heraldy_parser.cpp
  bin/logic.cpp
  utils/quest_states.cpp
  bin/tiles_funcs.cpp
  bin/battle_map.cpp
  bin/battle_troop_parser.cpp
  bin/battle_troop_type_parser.cpp
  bin/battle_unit_state_parser.cpp
)

set(HDR_LIST
  bin/interface_context.h
  #bin/interfaces.h
  bin/overlay.h
  bin/helper.h
  bin/figures.h
  bin/camera.h
  bin/generator_system2.h
  #bin/map_generators2.h
  bin/map.h
  bin/generator_context2.h
  bin/generator_container.h
  bin/map_generator.h
  #bin/character.h
  bin/core_structures.h
  #bin/declare_structures.h
  bin/map_creator.h
  #bin/stats.h
  bin/data_parser.h
  bin/image_parser.h
  bin/heraldy_parser.h
  bin/logic.h
  utils/quest_states.h
  utils/quest_state.h
  bin/tiles_funcs.h
  bin/battle_map.h
  bin/battle_troop_parser.h
  bin/battle_troop_type_parser.h
  bin/battle_unit_state_parser.h
)
add_executable(${TARGET_NAME} ${SRC_LIST} ${HDR_LIST})
target_link_libraries(${TARGET_NAME} systems core script ai render utils serialization tcmalloc unwind)
add_dependencies(${TARGET_NAME} shaders)

if (MSVC)
  if (CMAKE_BUILD_TYPE MATCHES "Debug")
    set_property(TARGET ${TARGET_NAME} APPEND PROPERTY LINK_FLAGS /DEBUG)
    set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    set_target_properties(${TARGET_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
    set_target_properties(${TARGET_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
  else()
    set_property(TARGET ${TARGET_NAME} APPEND PROPERTY LINK_FLAGS "/LTCG /OPT:REF /OPT:ICF")
    set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    set_target_properties(${TARGET_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
    set_target_properties(${TARGET_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
  endif()
endif()

# эта штука как то плохо работала в mingw, но можно ее использовать для линуксов
# Strip binary for release builds
if (CMAKE_BUILD_TYPE STREQUAL Release AND UNIX)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_STRIP} -s "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET_NAME}")
endif()

# кажется работает
make_directory(${PROJECT_DIR}/scripts)
#make_directory(${PROJECT_DIR}/scripts/interface)
file(GLOB LUA_SCRIPTS
  "lua_scripts/*.lua"
)
add_custom_target(copy_scripts ALL
  COMMAND      ${CMAKE_COMMAND} -E copy
               ${LUA_SCRIPTS}
               "${PROJECT_DIR}/scripts/"
)
add_dependencies(${TARGET_NAME} copy_scripts)

macro(configure_scripts folders)
  message(STATUS "Configuring scripts ${folders}")

  foreach(folder ${folders})
    if (NOT ${folder} EQUAL "")
      make_directory(${PROJECT_DIR}/scripts/${folder})
      file(GLOB LUA_SCRIPTS "lua_scripts/${folder}/*.lua")
      
      add_custom_target(copy_scripts.${folder} ALL
        COMMAND   ${CMAKE_COMMAND} -E copy
                  ${LUA_SCRIPTS}
                  "${PROJECT_DIR}/scripts/${folder}/"
      )
      add_dependencies(${TARGET_NAME} copy_scripts.${folder})
    endif(NOT ${folder} EQUAL "")
    
    message(STATUS "Configuring folder ${folder}")
  endforeach(folder ${folders})
endmacro(configure_scripts)

set(f "interface")
configure_scripts(${f})

#make_directory(${PROJECT_DIR}/textures)
#file(GLOB_RECURSE PROJECT_TEXTURES LIST_DIRECTORIES true
  #"textures/*"
#)
file(COPY textures DESTINATION ${PROJECT_DIR})
#install(DIRECTORY textures DESTINATION ${PROJECT_DIR})

# а это не работает
#macro(configure_files srcDir destDir)
    #message(STATUS "Configuring directory ${destDir}")
    #make_directory(${destDir})

    #file(GLOB_RECURSE templateFiles RELATIVE ${srcDir} ${srcDir}/*)
    #foreach(templateFile ${templateFiles})
        #set(srcTemplatePath ${srcDir}/${templateFile})
        #if(NOT IS_DIRECTORY ${srcTemplatePath})
            #message(STATUS "Configuring file ${templateFile}")
            #configure_file(
                    #${srcTemplatePath}
                    #${destDir}/${templateFile}
                    #@ONLY)
        #endif(NOT IS_DIRECTORY ${srcTemplatePath})
    #endforeach(templateFile)
#endmacro(configure_files)
#configure_files("textures/" "${PROJECT_DIR}/textures/")
