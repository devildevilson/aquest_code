#version 450

#extension GL_GOOGLE_include_directive : enable
#include "../render/shared_structures.h"
#include "../utils/shared_mathematical_constant.h"

layout(constant_id = 0) const float red   = 0.9f;
layout(constant_id = 1) const float green = 0.9f;
layout(constant_id = 2) const float blue  = 0.0f;
layout(constant_id = 3) const float alfa  = 1.0f;

layout(location = 0) out vec4 out_color;

void main() {
  out_color = vec4(red, green, blue, alfa);
}
