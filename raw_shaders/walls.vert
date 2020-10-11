#version 450

#extension GL_GOOGLE_include_directive : enable
#include "../render/shared_structures.h"

const vec2 uv_arr[] = {
  vec2(0.0f, 0.0f),
  vec2(1.0f, 0.0f),
  vec2(0.0f, 1.0f),
  vec2(1.0f, 1.0f)
};

layout(set = 0, binding = 0) uniform Camera {
  mat4 viewproj;
  mat4 view;
  vec4 pos;
  vec4 dir;
  uvec4 dim;
} camera;

layout(std140, set = 2, binding = 0) readonly buffer walls_datas {
  uvec4 datas[];
};

layout(std140, set = 3, binding = 0) readonly buffer tiles_buffer {
  light_map_tile_t tiles[];
};

layout(std140, set = 3, binding = 2) readonly buffer tile_points_buffer {
  vec4 tile_points[];
};

// наверное из вершинного буфера приходят индексы
//layout(location = 0) in uint current_index;
//layout(location = 0) out flat uint out_biom_index;
layout(location = 0) out flat image_t out_biom_texture;
layout(location = 1) out flat color_t out_biom_color;
layout(location = 2) out vec2 out_uv;
layout(location = 3) out flat float out_tile_height;

out gl_PerVertex {
  vec4 gl_Position;
};

void main() {
  const uint wall_index = gl_VertexIndex / PACKED_INDEX_COEF;
  const uint index_wall = gl_VertexIndex % PACKED_INDEX_COEF; // [0,5]
  const uvec4 wall_data = datas[wall_index];
  const uint tile1_index = wall_data[0];
  const uint tile2_index = wall_data[1];
  const uint point1_index = wall_data[2];
  const uint point2_index = wall_data[3];

  const float tile_height1 = uintBitsToFloat(tiles[tile1_index].tile_indices.w);
  const float tile_height2 = uintBitsToFloat(tiles[tile2_index].tile_indices.w);

  //const float layer_height1 = 1.0f / float(layers_count);
  const uint height_layer1 = compute_height_layer(tile_height1);
  const float final_height1 = layer_height * height_layer1;

  //const float layer_height2 = 1.0f / float(layers_count);
  const uint height_layer2 = compute_height_layer(tile_height2);
  const float final_height2 = layer_height * height_layer2;

  // const vec4 point1 = tile_points[point1_index];
  // const vec4 point2 = tile_points[point2_index];
  // const vec4 n_p1 = vec4(normalize(point1.xyz), 0.0f);
  // const vec4 n_p2 = vec4(normalize(point2.xyz), 0.0f);

  switch (index_wall) {
    case 0: {
      const vec4 point1 = tile_points[point1_index];
      const vec4 n_p1 = vec4(normalize(point1.xyz), 0.0f);
      gl_Position = camera.viewproj * (point1 + n_p1 * final_height1 * render_tile_height);
      break;
    }

    //case 2:
    case 2: {
      const vec4 point1 = tile_points[point1_index];
      const vec4 n_p1 = vec4(normalize(point1.xyz), 0.0f);
      gl_Position = camera.viewproj * (point1 + n_p1 * final_height2 * render_tile_height);
      break;
    }

    //case 1:
    case 1: {
      const vec4 point2 = tile_points[point2_index];
      const vec4 n_p2 = vec4(normalize(point2.xyz), 0.0f);
      gl_Position = camera.viewproj * (point2 + n_p2 * final_height1 * render_tile_height);
      break;
    }

    case 3: {
      const vec4 point2 = tile_points[point2_index];
      const vec4 n_p2 = vec4(normalize(point2.xyz), 0.0f);
      gl_Position = camera.viewproj * (point2 + n_p2 * final_height2 * render_tile_height);
      break;
    }
  }

  out_tile_height = tile_height1;
  //out_biom_index = tiles[tile1_index].tile_indices.y;
  color_t color;
  color.container = tiles[tile1_index].tile_indices.z;
  const float r = get_color_r(color) * 0.85f; // возможно чуть побольше значения сделать
  const float g = get_color_g(color) * 0.85f;
  const float b = get_color_b(color) * 0.85f;
  //out_biom_texture.container = tiles[tile1_index].tile_indices.y;
  out_biom_texture.container = GPU_UINT_MAX;
  out_biom_color = make_color1(r, g, b, 1.0f);
  out_uv = uv_arr[index_wall];
}
