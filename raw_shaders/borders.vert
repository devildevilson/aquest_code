#version 450

#extension GL_GOOGLE_include_directive : enable
#include "../render/shared_structures.h"

// короче мне нужно решить будут ли эти вещи константными
// нужно ли ограничивать высоту гор? вряд ли
// остальные вещи можно сделать константными
// нужно прикинуть сколько слоев мне может пригодиться
// 10 слоев чет мало, а 20 кажется много

// layout(constant_id = 0) const uint layers_count = 20;
// layout(constant_id = 1) const float mountain_height = 0.5f;
// layout(constant_id = 2) const float render_tile_height = 7.0f;

struct border_data {
  vec4 points[2];
  vec4 dirs[2];
};

struct border_type {
  uvec4 data;
};

layout(set = 0, binding = 0) uniform Camera {
  mat4 viewproj;
  mat4 view;
  vec4 pos;
  vec4 dir;
  uvec4 dim;
} camera;

layout(std140, set = 1, binding = 0) readonly buffer border_datas {
  border_data datas[];
};

layout(std140, set = 2, binding = 0) readonly buffer border_types {
  border_type types[];
};

layout(std140, set = 3, binding = 0) readonly buffer tiles_buffer {
  light_map_tile_t tiles[];
};

// наверное из вершинного буфера приходят индексы
layout(location = 0) in uint current_index;
// тип границы?
//layout(location = 0) out vec4 out_vert;

out gl_PerVertex {
  vec4 gl_Position;
};

//const float thickness = 0.2f;

void main() {
  const uint border_index = current_index;
  const uint index_border = gl_VertexIndex; // [0,3]
  //const uint border_index = gl_VertexIndex / PACKED_INDEX_COEF;
  //const uint index_border = gl_VertexIndex % PACKED_INDEX_COEF; // [0,3]
  border_data current_data = datas[border_index];
  const uint type_index = floatBitsToUint(current_data.dirs[1].w); // так прячем тип границы
  const border_type type = types[type_index];
  const float thickness = current_data.points[0].w;
  current_data.points[0].w = 1.0f;

  const uint tile_index = floatBitsToUint(current_data.dirs[0].w);
  const float tile_height = uintBitsToFloat(tiles[tile_index].tile_indices.w);
  //const float layer_height = mountain_height / float(layers_count);
  //const uint height_layer = tile_height < 0.0f ? 0 : (tile_height >= mountain_height ? layers_count : uint(tile_height / layer_height));
  //const float layer_height = 1.0f / float(layers_count);
  const uint height_layer = compute_height_layer(tile_height);
  const float final_height = layer_height * height_layer;

  // пытаюсь смоделировать депт биас
  // похоже что эта функция недоступна на большинстве мобильных устройств
  // 0.1f - нормальный отступ на большом удалении от земли
  // близко появляются ошибки
  const float zoom = uintBitsToFloat(camera.dim.z);
  const float depth_mod = mix(0.05f, 0.15f, zoom);

  switch (index_border) {
    case 0: {
      const vec4 up = normalize(vec4(current_data.points[0].xyz, 0.0f));
      gl_Position = camera.viewproj * (current_data.points[0] + up * final_height * render_tile_height + up * depth_mod);
      break;
    }

    //case 2:
    case 2: {
      const vec4 up = normalize(vec4(current_data.points[0].xyz, 0.0f));
      const vec4 dir = vec4(current_data.dirs[0].x, current_data.dirs[0].y, current_data.dirs[0].z, 0.0f);
      gl_Position = camera.viewproj * (current_data.points[0] + up * final_height * render_tile_height + dir * thickness + up * depth_mod);
      break;
    }

    //case 1:
    case 1: {
      const vec4 up = normalize(vec4(current_data.points[1].xyz, 0.0f));
      gl_Position = camera.viewproj * (current_data.points[1] + up * final_height * render_tile_height + up * depth_mod);
      break;
    }

    case 3: {
      const vec4 up = normalize(vec4(current_data.points[1].xyz, 0.0f));
      const vec4 dir = vec4(current_data.dirs[1].x, current_data.dirs[1].y, current_data.dirs[1].z, 0.0f);
      gl_Position = camera.viewproj * (current_data.points[1] + up * final_height * render_tile_height + dir * thickness + up * depth_mod);
      break;
    }
  }
}
