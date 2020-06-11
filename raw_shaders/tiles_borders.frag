#version 450

#extension GL_GOOGLE_include_directive : enable
#include "../render/shared_structures.h"

layout(location = 0) in flat uint in_biom_index;
layout(location = 1) in vec2 in_uv;
layout(location = 0) out vec4 out_color;

void main() {
  //out_color = vec4(red_component, green_component, blue_component, 1.0f);
  //out_color = biomes_colors[in_biom_index];
  out_color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
}
