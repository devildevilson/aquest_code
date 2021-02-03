#version 450 core

#extension GL_GOOGLE_include_directive : enable
#include "../render/shared_structures.h"
#include "../render/image_container_constants.h"

#define UINT_MAX 0xffffffff

layout(constant_id = 0) const uint imagesCount = 2;
layout(constant_id = 1) const uint samplersCount = 1;

//layout(set = 1, binding = 0) uniform sampler2D font_atlas_texture;
layout(set = 1, binding = 0) uniform texture2DArray textures[IMAGE_CONTAINER_SLOT_SIZE];
layout(set = 1, binding = 1) uniform sampler samplers[IMAGE_SAMPLERS_COUNT];

layout(location = 0) in flat uvec4 in_color;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec2 in_square_uv;
layout(location = 3) in flat image_t in_texture;

layout(location = 0) out vec4 fragment_color;

void main() {
  vec4 color = vec4(in_color[0]/255.0, in_color[1]/255.0, in_color[2]/255.0, in_color[3]/255.0);

  // я могу какую то информацию записать в цвет
  const uint index = in_color[3] == 1 ? in_color[0] : UINT_MAX;
  const uint layer = in_color[3] == 1 ? in_color[1] : UINT_MAX;
  //color = in_color[3] == 1 ? texture(sampler2DArray(textures[index], samplers[0]), vec3(in_uv, float(layer))) : color;

  // изображение вместо атласа?
  const bool valid_texture = is_image_valid(in_texture);
  const uint image_index = get_image_index(in_texture);
  const uint layer_index = get_image_layer(in_texture);
  const uint sampler_index = get_image_sampler(in_texture);
  const vec4 texture_color = valid_texture ? texture(sampler2DArray(textures[image_index], samplers[sampler_index]), vec3(in_uv, float(layer_index))) : vec4(1.0f, 1.0f, 1.0f, 1.0f);

  fragment_color = color * texture_color;
}
