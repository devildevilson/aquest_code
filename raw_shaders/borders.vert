#version 450

#extension GL_GOOGLE_include_directive : enable
#include "../render/shared_structures.h"

struct border_data {
  vec4 points[2];
  vec4 dirs[2];
};

struct border_type {
  vec4 color1;
  vec4 color2;
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

// наверное из вершинного буфера приходят индексы
layout(location = 0) in uint current_index;
// тип границы?
//layout(location = 0) out vec4 out_vert;

out gl_PerVertex {
  vec4 gl_Position;
};

//const float thickness = 0.2f;

void main() {
  const uint border_index = current_index / 6;
  const border_data current_data = datas[border_index];
  const uint index_border = current_index % 6; // [0,5]
  const uint type_index = floatBitsToUint(current_data.dirs[1].w); // так прячем тип границы
  const border_type type = types[type_index];
  const float thickness = type.color2.w;
  switch (index_border) {
    case 0: {
      gl_Position = camera.viewproj * current_data.points[0];
      break;
    }

    case 2:
    case 3: {
      const vec4 dir = vec4(current_data.dirs[0].x, current_data.dirs[0].y, current_data.dirs[0].z, 0.0f);
      gl_Position = camera.viewproj * (current_data.points[0] + dir * thickness);
      break;
    }

    case 1:
    case 4: {
      gl_Position = camera.viewproj * current_data.points[1];
      break;
    }

    case 5: {
      const vec4 dir = vec4(current_data.dirs[1].x, current_data.dirs[1].y, current_data.dirs[1].z, 0.0f);
      gl_Position = camera.viewproj * (current_data.points[1] + dir * thickness);
      break;
    }
  }
}
