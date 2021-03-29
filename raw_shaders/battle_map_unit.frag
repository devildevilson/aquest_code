#version 450

#extension GL_GOOGLE_include_directive : enable
#include "../render/shared_structures.h"
#include "../render/image_container_constants.h"

layout(set = 0, binding = 0) uniform Camera {
  mat4 viewproj;
  mat4 view;
  vec4 pos;
  vec4 dir;
  uvec4 dim;
} camera;

layout(set = 1, binding = 0) uniform texture2DArray textures[IMAGE_CONTAINER_SLOT_SIZE];
layout(set = 1, binding = 1) uniform sampler samplers[IMAGE_SAMPLERS_COUNT];

//layout(location = 0) in flat color_t in_color;
layout(location = 0) in flat image_t in_image;
layout(location = 1) in vec2 in_uv;
layout(location = 0) out vec4 out_color;

void main() {
  //const vec4 unpacked_color = vec4(get_color_r(in_color), get_color_g(in_color), get_color_b(in_color), 1.0f);

  const bool valid_image = is_image_valid(in_image);
  const uint index = get_image_index(in_image);
  const uint layer = get_image_layer(in_image);
  const uint sampler_id = valid_image ? get_image_sampler(in_image) : 0;
  const float fu = flip_u(in_image) ? -1.0f : 1.0f;
  const float fv = flip_v(in_image) ? -1.0f : 1.0f;
  const vec3 final_uv = vec3(in_uv.x * fu, in_uv.y * fv, float(layer));

  const vec4 texture_color = texture(sampler2DArray(textures[index], samplers[sampler_id]), final_uv);
  //const vec4 final_color = mix(unpacked_color, texture_color, float(valid_image));
  const vec4 final_color = texture_color;

  if (final_color.w < 0.5f) discard;

  out_color = final_color;
}
