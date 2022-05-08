#version 450

#extension GL_GOOGLE_include_directive : enable
#include "../render/shared_structures.h"

layout(location = 0) in vec2 in_uv;
layout(location = 1) in flat color_t in_color1;
layout(location = 2) in flat color_t in_color2;
layout(location = 0) out vec4 out_color;

void main() {
  const vec4 color1 = get_color(in_color1);
  const vec4 color2 = get_color(in_color2);

  //out_color = vec4(0.0f, 0.0f, 0.0f, 0.5f);
  out_color = in_uv.x <= 0.5f ? color1 : color2;
}
