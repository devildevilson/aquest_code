#version 450

#extension GL_GOOGLE_include_directive : enable
#include "../render/shared_render_utility.h"

layout (location = 0) in vec4 in_point;
layout (location = 1) in flat uint in_state;
layout (location = 0) out vec4 out_frag_color;

layout (constant_id = 0) const float chance = 0.0015f;

void main() {
  uint current = in_state;
  current = prng2(current, floatBitsToUint(in_point.x));
  current = prng2(current, floatBitsToUint(in_point.y));
  current = prng2(current, floatBitsToUint(in_point.z));
  const float final = prng_normalize(current);
  out_frag_color = float(final <= chance) * vec4(1.0f, 1.0f, 1.0f, 1.0f);
  out_frag_color.w = 1.0f;
  //out_frag_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}
