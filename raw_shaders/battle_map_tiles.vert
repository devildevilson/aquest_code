#version 450

#extension GL_GOOGLE_include_directive : enable
#include "../render/shared_structures.h"
#include "../render/shared_battle_structures.h"
#include "../utils/shared_mathematical_constant.h"

const vec2 hexagon_uv[] = {
  vec2(0.75f, 0.932932f),
  vec2(1.0f, 0.5f),
  vec2(0.75f, 0.067068f),
  vec2(0.25f, 0.067068f),
  vec2(0.0f, 0.5f),
  vec2(0.25f, 0.932932f)
};

const vec2 walls_pos_offsets_arr[] = {
  vec2( 0.0f,-2.0f),
  vec2( 1.0f,-1.0f),
  vec2( 1.0f, 1.0f),
  vec2( 0.0f, 2.0f),
  vec2(-1.0f, 1.0f),
  vec2(-1.0f,-1.0f)
};

const vec2 hexagon_points_offsets_arr[] = {
  vec2( 0.0f,-2.0f), // 0
  vec2(-1.0f,-1.0f), // 1
  vec2( 1.0f,-1.0f), // 2
  vec2(-1.0f, 1.0f), // 3
  vec2( 1.0f, 1.0f), // 4
  vec2( 0.0f, 2.0f)  // 5
};

layout(set = 0, binding = 0) uniform Camera {
  mat4 viewproj;
  mat4 view;
  vec4 pos;
  vec4 dir;
  uvec4 dim;
} camera;

layout(std140, set = 2, binding = 0) readonly buffer tiles_buffer {
  battle_map_tile_data_t tiles[];
};

out gl_PerVertex {
  vec4 gl_Position;
};

layout(location = 0) in uint tile_index;  // инстансный буфер
layout(location = 0) out flat uint out_tmp_index;

void main() {
  const uint point_index = gl_VertexIndex;
  const bool rendering_walls = point_index < 12;

  // тут нужна высота и позиция
  const uint tile_map_width  = 128;
  const uint tile_map_height = 128;
  const uvec2 tile_coord = uvec2(tile_index / tile_map_width, tile_index % tile_map_height);

  const float tile_size = 1.0f;
  const float tile_width = sqrt(3.0f) * tile_size;
  const float tile_height = 2.0f * tile_size;
  const float tile_width_dist = 1.0f * tile_width;
  const float tile_height_dist = (3.0f/4.0f) * tile_height;
  const vec2 tile_pos = tile_coord * vec2(tile_width_dist, tile_height_dist); // + const ?

  const float height = 1.0f;
  const uint final_point_index   = rendering_walls ? point_index / 2 : point_index - 12;
  const float final_point_height = rendering_walls ? (point_index % 2 == 0 ? height : 0.0f) : height;

  const float offset_x = 0.5f * tile_width;
  const float offset_z_4 = 0.25f * tile_height;
  const float offset_z_2 = 0.5f * tile_height;

  const vec3 final_point_pos = rendering_walls ?
    vec3(tile_pos.x + walls_pos_offsets_arr[point_index].x * offset_x,      final_point_height, tile_pos.y + walls_pos_offsets_arr[point_index].y * offset_z_4) :
    vec3(tile_pos.x + hexagon_points_offsets_arr[point_index].x * offset_x, final_point_height, tile_pos.y + hexagon_points_offsets_arr[point_index].y * offset_z_4);
  gl_Position = vec4(final_point_pos, 1.0f);

  out_tmp_index = GPU_UINT_MAX;
}
