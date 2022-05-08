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

option(BULD_TESTS "compile tests" ON)

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
  src/
  include/sol2/include/
  #include/sol2_release/
  include/lanes/src/
  #include/VulkanMemoryAllocator/src/
  include/FastNoise/
  #include/
  include/dsml/
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
  include/doctest/doctest/
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
    # ${CPPFS_LIB_PATH}
  )
else()
  set(CPPFS_LIB_PATH libs)
  set(MEMORY_LIB_PATH include/memory/build/src/)
  set(LUA_LIB lua)
  set(LUA_LIBRARIES /usr/include)

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

  # link_directories(
  #   ${CPPFS_LIB_PATH}
  #   ${MEMORY_LIB_PATH}
  # )
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

add_subdirectory(src/raw_shaders/)

set(SRC_LIST
  #utils/lib_crypto_fake_rtti.cpp
#   include/cryptopp/hc256.cpp
#   include/cryptopp/sha.cpp
  src/utils/salsa20.cpp
  src/utils/serializator_helper.cpp
#   protobuf/test1.pb.cc
)

set(HDR_LIST
  #include/cryptopp/hc256.h
  #include/cryptopp/sha.h
  src/utils/picosha2.h
  src/utils/salsa20.h
  src/utils/serializator_helper.h
#   protobuf/test1.pb.h
)

add_library(serialization ${SRC_LIST} ${HDR_LIST})
target_link_libraries(serialization lz4) #protobuf-lite protobuf

add_subdirectory(src/utils/)
# нужно сделать одинаковый файл что в линуксе что в винде
# в любом случае нужно компилировать с одной версией луа

#add_subdirectory(src/sound/)
add_subdirectory(src/render/)
add_subdirectory(src/script/)
add_subdirectory(src/generator/)

set(SRC_LIST
  src/utils/systems.cpp
  src/bin/loading_functions.cpp
  src/bin/game_time.cpp
)

set(HDR_LIST
  src/utils/systems.h
  src/bin/loading_functions.h
  src/bin/game_time.h
)

add_library(systems ${SRC_LIST} ${HDR_LIST})
target_link_libraries(systems generator render utils)

add_subdirectory(src/core/)
add_subdirectory(src/battle/)
add_subdirectory(src/lua_init/)
add_subdirectory(src/bin/)

if (BULD_TESTS)
  add_subdirectory(src/tests/)
endif()

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

add_subdirectory(src/lua_scripts/)

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
