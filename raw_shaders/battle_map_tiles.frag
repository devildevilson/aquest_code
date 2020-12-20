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

layout(location = 0) in flat uint in_tmp_index;
layout(location = 0) out vec4 out_color;

void main() {
  out_color = vec4(0.0f, 1.0f, 0.0f, 1.0f);
}
