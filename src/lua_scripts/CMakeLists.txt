# file(GLOB LUA_SCRIPTS "*.lua")
# add_custom_target(copy_scripts ALL
#   COMMAND      ${CMAKE_COMMAND} -E copy
#                ${LUA_SCRIPTS}
#                "${PROJECT_DIR}/scripts/"
# )
#add_dependencies(${TARGET_NAME} copy_scripts)

macro(configure_scripts folders)
  message(STATUS "Configuring scripts ${folders}")

  foreach(folder ${folders})
    if (NOT ${folder} EQUAL "")
      make_directory(${PROJECT_DIR}/scripts/${folder})
      file(GLOB LUA_SCRIPTS "${folder}/*.lua")

      add_custom_target(copy_scripts.${folder} ALL
        COMMAND   ${CMAKE_COMMAND} -E copy
                  ${LUA_SCRIPTS}
                  "${PROJECT_DIR}/scripts/${folder}/"
      )
      add_dependencies(${TARGET_NAME} copy_scripts.${folder})
    else()
      file(GLOB LUA_SCRIPTS "*.lua")

      add_custom_target(copy_scripts ALL
        COMMAND   ${CMAKE_COMMAND} -E copy
                  ${LUA_SCRIPTS}
                  "${PROJECT_DIR}/scripts/"
      )
      add_dependencies(${TARGET_NAME} copy_scripts)
    endif(NOT ${folder} EQUAL "")

    message(STATUS "Configuring folder ${folder}")
  endforeach(folder ${folders})
endmacro(configure_scripts)

set(f "" "interface")
configure_scripts(${f})
