#version 450

#extension GL_GOOGLE_include_directive : enable
#include "../render/shared_structures.h"

in gl_PerVertex {
    vec4  gl_Position;
} gl_in[];

layout(set = 0, binding = 0) uniform Camera{
  mat4 viewproj;
  mat4 view;
  vec4 pos;
  vec4 dir;
  uvec2 dim;
} camera;

struct neighbour_t {
  uint index;
  uint points[2];
};

struct tile_t {
  uint center;
  neighbour_t neighbours[6];
};

layout(std430, set = 1, binding = 0) readonly buffer tiles_buffer {
  tile_t tiles[];
};

layout(std430, set = 2, binding = 0) readonly buffer tile_points_buffer {
  vec3 tile_points[];
};

layout(points) in;
layout(triangle_strip, max_vertices = 18) out;

layout(location = 0) in flat uint tile_index[];

void main() {
  const tile_t tile = tiles[tile_index[0]];
  for (uint i = 0; i < 6; ++i) {
    const vec3 center = tile_points[tile.center];
    const vec3 point1 = tile_points[tile.neighbours[i].points[0]];
    const vec3 point2 = tile_points[tile.neighbours[i].points[1]];
    gl_Position = camera.viewproj * vec4(center, 1.0f);
    EmitVertex();
    gl_Position = camera.viewproj * vec4(point1, 1.0f);
    EmitVertex();
    gl_Position = camera.viewproj * vec4(point2, 1.0f);
    EmitVertex();
    EndPrimitive();
  }
}
