file(GLOB_RECURSE GLSL_SOURCE_FILES
  tiles.vert
  #tiles.geom
  tiles.frag
  tiles.comp
  tile_data.comp
  count_biome_tiles.comp
  map_objects.comp
  tiles_borders.frag
  gui.frag
  gui.vert
  #borders.comp
  borders.vert
  borders.frag
  one_tile.vert
  one_tile.frag
  #walls.comp
  walls.vert
  first_object.vert
  first_object.frag
  structure.vert
  heraldy.vert
  heraldy.frag
  armies.vert
  battle_map_tiles.comp
  battle_map_tiles.vert
  battle_map_tiles.frag
  battle_biome_objects.vert
  battle_biome_objects.frag
  battle_map_unit.vert
  battle_map_unit.frag
  skybox.vert
  skybox.frag
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

add_custom_target(shaders DEPENDS ${SPIRV_BINARY_FILES})
