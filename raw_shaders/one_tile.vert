#version 450

#extension GL_GOOGLE_include_directive : enable
#include "../render/shared_structures.h"
#include "../utils/shared_mathematical_constant.h"

layout(set = 0, binding = 0) uniform Camera {
  mat4 viewproj;
  mat4 view;
  vec4 pos;
  vec4 dir;
  uvec4 dim;
} camera;

layout(std140, set = 1, binding = 0) readonly buffer tiles_buffer {
  light_map_tile_t tiles[];
};

layout(std140, set = 1, binding = 1) readonly buffer biomes_buffer {
  packed_biome_data_t biomes[];
};

layout(std140, set = 1, binding = 2) readonly buffer tile_points_buffer {
  vec4 tile_points[];
};

layout(location = 0) in uint in_tile_index;
layout(location = 1) in uint in_tile_color;
layout(location = 0) out flat color_t out_tile_color;

void main() {
  const uint tile_index  = in_tile_index;
  const uint point_index = gl_VertexIndex;
  const map_tile_t tile = unpack_data(tiles[tile_index]);
  const uint point_id = tile.points[point_index];
  const vec4 point = tile_points[point_id];

  //const bool is_pentagon = tile_index < 12;
  const vec3 n = point.xyz / WORLD_RADIUS_CONSTANT;

  const uint height_layer = compute_height_layer(tile.height);
  const float final_height = layer_height * height_layer;
  const float computed_height = final_height * render_tile_height + 0.3f;

  gl_Position = camera.viewproj * (point + vec4(n, 0.0f) * computed_height);
  out_tile_color.container = in_tile_color;
}

// нужно ли ривсовать какие либо текстурки на этом выделении?
