#version 450

#extension GL_GOOGLE_include_directive : enable
#include "../render/shared_structures.h"
#include "../utils/shared_mathematical_constant.h"

// std430 работает в vulkan иначе
// фактически работает как std140
// то есть требуется складывать данные
// в структуры размер которых делится на 16 без остатка

// struct neighbour_t {
//   uint index;
//   uint points[2];
// };
//
// struct tile_t {
//   uint center;
//   neighbour_t neighbours[6];
// };
//
// struct tile_memory_t {
//   uint center;
//   uvec3 neighbours_1;
//   uvec3 neighbours_2;
//   uvec3 neighbours_3;
//   uvec3 neighbours_4;
//   uvec3 neighbours_5;
//   uvec3 neighbours_6;
// };
//
// struct tile_memory2_t {
//
// };

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
  packed_biom_data_t biomes[];
};

layout(std140, set = 1, binding = 2) readonly buffer tile_points_buffer {
  vec4 tile_points[];
};

out gl_PerVertex {
  vec4 gl_Position;
};

layout(location = 0) in uint tile_index;  // инстансный буфер
layout(location = 1) in uint point_index; // буфер вида [0,...,4,0,...,5], указываем оффсет
layout(location = 0) out flat uint out_biom_index;
layout(location = 1) out vec2 out_uv;
layout(location = 2) out flat float out_tile_height;

// либо мы uv координаты можем посчитать в скрин спейсе
// нет лучше здесь
void main() {
  const map_tile_t tile = unpack_data(tiles[tile_index]);
  const uint point_id = tile.points[point_index];
  //const uint point_id = tile.neighbours[point_index].points[0];
  const vec4 point = tile_points[point_id];
  // if (point_index == 0) point = tile_points[tile.index]; // мне не обязательно брать здесь центральную точку
  // else {
  //   const uint final_point_index = point_index - 1;
  //   const uint id = tile.points[final_point_index];
  //   point = tile_points[id];
  // }

  const vec3 n = normalize(point.xyz);
  const vec2 uv = vec2(atan(n.x, n.z) / (PI_2) + 0.5f, n.y * 0.5f + 0.5f);

  gl_Position = camera.viewproj * point;
  out_uv = uv; // наверное на что нибудь нужно умножить
  //out_image = biomes[tile.biom_index].img;
  // image_t img;
  // img.container = 0;
  // out_image = img;
  out_biom_index = tile.biom_index;
  //out_biom_index = tile.unique_object_index;
  out_tile_height = tile.height;
}
