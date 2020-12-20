#version 450

#extension GL_GOOGLE_include_directive : enable
#include "../render/shared_structures.h"
#include "../utils/shared_mathematical_constant.h"
#include "../render/image_container_constants.h"

layout(constant_id = 0) const float red   = 0.9f;
layout(constant_id = 1) const float green = 0.9f;
layout(constant_id = 2) const float blue  = 0.0f;
layout(constant_id = 3) const float alfa  = 0.5f;

//layout(set = 1, binding = 0) uniform texture2DArray textures[IMAGE_CONTAINER_SLOT_SIZE];
//layout(set = 1, binding = 1) uniform sampler samplers[IMAGE_SAMPLERS_COUNT];

layout(location = 0) in flat color_t in_color;
layout(location = 0) out vec4 out_color;

void main() {
  out_color = vec4(get_color_r(in_color), get_color_g(in_color), get_color_b(in_color), get_color_a(in_color));
}
