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

layout(push_constant) uniform push {
  uint tile_index;
} pc;

layout(std140, set = 1, binding = 0) readonly buffer tiles_buffer {
  light_map_tile_t tiles[];
};

layout(std140, set = 1, binding = 1) readonly buffer biomes_buffer {
  packed_biom_data_t biomes[];
};

layout(std140, set = 1, binding = 2) readonly buffer tile_points_buffer {
  vec4 tile_points[];
};

layout(location = 0) in uint point_index; // буфер вида [0,...,4,0,...,5], указываем оффсет

void main() {
  const map_tile_t tile = unpack_data(tiles[pc.tile_index]);
  const uint point_id = tile.points[point_index];
  const vec4 point = tile_points[point_id];

  gl_Position = camera.viewproj * point;
}
