#version 450

#extension GL_GOOGLE_include_directive : enable
#include "../render/shared_render_utility.h"

layout (location = 0) in vec4 in_point;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in flat uint in_state;
layout (location = 0) out vec4 out_frag_color;

layout (constant_id = 0) const float chance = 0.0025f;

void main() {
  //const vec4 cur_pixel = gl_FragCoord;
  uint current = in_state;
  const vec2 final_k = floor(in_uv * 2000);
  //current = prng2(current, floatBitsToUint(final_k.x));
  //current = prng2(current, floatBitsToUint(final_k.y));
  //current = prng2(current, floatBitsToUint(final_k.z));
  // current = prng2(current, floatBitsToUint(cur_pixel.x));
  // current = prng2(current, floatBitsToUint(cur_pixel.y));
  current = prng2(current, floatBitsToUint(final_k.x));
  current = prng2(current, floatBitsToUint(final_k.y));
  const float final = prng_normalize(current);
  out_frag_color = float(final <= chance) * vec4(1.0f, 1.0f, 1.0f, 1.0f);
  out_frag_color.w = 1.0f;
  //out_frag_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}
