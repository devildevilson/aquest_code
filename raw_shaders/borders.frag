#version 450

#extension GL_GOOGLE_include_directive : enable
#include "../render/shared_structures.h"

layout(location = 0) out vec4 out_color;

void main() {
  out_color = vec4(0.0f, 0.0f, 0.0f, 0.5f);
}
