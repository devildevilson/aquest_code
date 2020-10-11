#version 450

#extension GL_GOOGLE_include_directive : enable
#include "../render/shared_structures.h"
#include "../utils/shared_mathematical_constant.h"
#include "../render/image_container_constants.h"

layout(set = 1, binding = 0) uniform texture2DArray textures[IMAGE_CONTAINER_SLOT_SIZE];
layout(set = 1, binding = 1) uniform sampler samplers[IMAGE_SAMPLERS_COUNT];

layout(location = 0) in flat image_t in_biom_texture;
layout(location = 1) in flat color_t in_biom_color;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in flat float in_tile_height;
layout(location = 0) out vec4 out_color;

void main() {
  const uint index = get_image_index(in_biom_texture);
  const uint layer = get_image_layer(in_biom_texture);
  const uint sampler_id = get_image_sampler(in_biom_texture);
  const float fu = flip_u(in_biom_texture) ? -1.0f : 1.0f;
  const float fv = flip_v(in_biom_texture) ? -1.0f : 1.0f;

  const vec3 final_uv = vec3(in_uv.x * fu, in_uv.y * fv, float(layer));
  const vec4 color = texture(sampler2DArray(textures[index], samplers[sampler_id]), final_uv);
  if (color.w < 0.5f) discard;

  out_color = color;
}
