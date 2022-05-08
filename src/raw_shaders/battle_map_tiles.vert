#version 450

#extension GL_GOOGLE_include_directive : enable
#include "../render/shared_structures.h"
#include "../render/shared_battle_structures.h"
#include "../utils/shared_mathematical_constant.h"
#include "../render/shared_render_utility.h"

const vec2 hexagon_uv[] = {
  vec2(0.5f, 1.0f),
  vec2(0.932932f, 0.749954f),
  vec2(0.067068f, 0.749953f),
  vec2(0.932932f, 0.250047f),
  vec2(0.067068f, 0.250047f),
  vec2(0.5f, 0.0f)
};

const vec2 box_uv[] = {
  vec2(0.0f, 0.0f),
  vec2(0.0f, 1.0f),
  vec2(1.0f, 0.0f),
  vec2(1.0f, 1.0f)
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

const vec2 hex_map_row_const_offset[] = {
  vec2(-0.5f, 0.0f),
  vec2( 0.5f, 0.0f),
  vec2( 0.0f,-0.5f),
  vec2( 0.0f, 0.5f)
};

layout(set = 0, binding = 0) uniform Camera {
  mat4 viewproj;
  mat4 view;
  vec4 pos;
  vec4 dir;
} camera;

layout(set = 2, binding = 0) uniform tiles_uniform_buffer {
  uvec4 map_properties;
} tiles_uniform;

layout(std140, set = 2, binding = 1) readonly buffer tiles_buffer {
  packed_battle_map_tile_data_t tiles[];
};

out gl_PerVertex {
  vec4 gl_Position;
};

layout(location = 0) in uint tile_index;  // инстансный буфер
layout(location = 0) out flat color_t out_color;
layout(location = 1) out flat image_t out_image;
layout(location = 2) out vec2 out_uv;
//layout(location = 3) out flat color_t out_color;

mat4 translate(const mat4 mat, const vec4 vec);
void swap(inout float a, inout float b);

void main() {
  const uint point_index = gl_VertexIndex;
  const bool rendering_walls = point_index < 12;
  const bool basement_point = point_index % 2 == 0;
  const uint side_index = point_index / 2;
  const uint hex_point_index = point_index - 12;

  const uvec4 uniform_property_data = tiles_uniform.map_properties;
  const uint map_width   = uniform_property_data.y;
  const uint map_height  = uniform_property_data.z;
  const uint map_type    = uniform_property_data.w;

  const bool is_square_map       = bool(map_type & (1 << 0));
  const bool is_flat_orientation = bool(map_type & (1 << 1));
  const bool is_odd_offset       = bool(map_type & (1 << 2));

  // тут нужна высота и позиция
  const uint tile_map_width  = map_width;
  const uint tile_map_height = map_height;
  const uint row_index    = tile_index / tile_map_height;
  const uint column_index = tile_index % tile_map_height;
  const uvec2 tile_coord = uvec2(column_index, row_index);
  //const uvec2 tile_coord = uvec2(0, 0);

  const float hex_size = 1.0f;
  const float hex_width = mix(sqrt(3.0f) * hex_size, 2.0f * hex_size, float(is_flat_orientation));
  const float hex_height = mix(2.0f * hex_size, sqrt(3.0f) * hex_size, float(is_flat_orientation));
  const float hex_width_dist = mix(1.0f * hex_width, (3.0f/4.0f) * hex_width, float(is_flat_orientation));
  const float hex_height_dist = mix((3.0f/4.0f) * hex_height, 1.0f * hex_height, float(is_flat_orientation));

  const uint offset_type_index = uint(is_odd_offset) + 2 * uint(is_flat_orientation);

  // 4 константы зависят от того как мы представляем координатные системы
  // квадратная карта может быть представлена: четный оффсет по строкам, нечетный оффсет по строкам, четный оффсет по столбцам, нечетный оффсет по столбцам
  const uint row_column = uint(is_flat_orientation) * column_index + uint(!is_flat_orientation) * row_index;
  const vec2 const_pos_k = float(row_column % 2 == 1) * hex_map_row_const_offset[offset_type_index] * vec2(hex_width_dist, hex_height_dist);

  const vec2 tile_pos = vec2(tile_coord) * vec2(hex_width_dist, hex_height_dist) + const_pos_k;
  //const vec2 tile_pos = tile_coord * vec2(0.0f, 0.0f);

  const battle_map_tile_data_t current_tile = unpack_data(tiles[tile_index]);
  const float tile_height = current_tile.height;

  const uint final_point_index   = uint(rendering_walls) * (point_index / 2) + uint(!rendering_walls) * (point_index - 12);
  //const float final_point_height = rendering_walls ? (height * float(point_index % 2 == 0)) : height;
  //const uint final_point_index   = uint(mix(float(point_index - 12), float(point_index / 2), float(rendering_walls))); // кажется mix быстрее
  const float final_point_height = mix(tile_height, mix(tile_height, 0.0f, float(basement_point)), float(rendering_walls));

  const float offset_x = mix(0.5f * hex_width, 0.25f * hex_width, float(is_flat_orientation));
  const float offset_z = mix(0.25f * hex_height, 0.5f * hex_height, float(is_flat_orientation));

  //const vec2 wall_point = tile_pos + walls_pos_offsets_arr[final_point_index] * vec2(offset_x, offset_z_4);
  //const vec2 hex_point  = tile_pos + hexagon_points_offsets_arr[final_point_index] * vec2(offset_x, offset_z_4);
  vec2 walls_pos_offset = walls_pos_offsets_arr[final_point_index];
  //vec2 walls_pos_offset = hexagon_points_offsets_arr[final_point_index];
  walls_pos_offset = mix(walls_pos_offset, walls_pos_offset.yx, float(is_flat_orientation)) * vec2(offset_x, offset_z);
  vec2 hexagon_point_offset = hexagon_points_offsets_arr[final_point_index];
  hexagon_point_offset = mix(hexagon_point_offset, hexagon_point_offset.yx, float(is_flat_orientation)) * vec2(offset_x, offset_z);
  const vec2 wall_point = tile_pos + walls_pos_offset;
  const vec2 hex_point  = tile_pos + hexagon_point_offset;

  const vec3 final_wall_point = vec3(wall_point.x, final_point_height, wall_point.y);
  const vec3 final_hex_point =  vec3(hex_point.x,  final_point_height, hex_point.y);

  //const vec3 final_point_pos = rendering_walls ? final_wall_point : final_hex_point;
  const vec3 final_point_pos = mix(final_hex_point, final_wall_point, float(rendering_walls));
  //const mat4 model = translate(mat4(1.0f), vec4(tile_pos.x, 0.0f, tile_pos.y, 1.0f));
  gl_Position = camera.viewproj * vec4(final_point_pos, 1.0f); // model
  //gl_Position = vec4(tile_pos, 0.0f, 1.0f);
  //const vec2 tmp = tile_pos * walls_pos_offsets_arr[final_point_index];
  //g_Position = camera.viewproj * vec4(tmp.x, 0.0f, tmp.y, 1.0f);

  out_color.container = prng(tile_index);
  out_image = rendering_walls ? current_tile.walls : current_tile.ground;
  const vec2 walls_uv = mix(vec2(0.0f, float(side_index)), vec2(1.0f, float(side_index)), float(basement_point));
  out_uv = mix(hexagon_uv[hex_point_index], walls_uv, float(rendering_walls));
}

mat4 translate(const mat4 mat, const vec4 vec) {
  mat4 ret = mat;
  ret[3] = mat[0] * vec[0] + mat[1] * vec[1] + mat[2] * vec[2] + mat[3];
  return ret;
}

void swap(inout float a, inout float b) {
  const float tmp = a;
  a = b;
  b = tmp;
}
