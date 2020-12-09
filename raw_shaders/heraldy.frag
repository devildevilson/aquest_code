#version 450

#extension GL_GOOGLE_include_directive : enable
#include "../render/shared_structures.h"
#include "../render/heraldy.h"
#include "../utils/shared_mathematical_constant.h"
#include "../render/image_container_constants.h"

layout(set = 1, binding = 0) uniform texture2DArray textures[IMAGE_CONTAINER_SLOT_SIZE];
layout(set = 1, binding = 1) uniform sampler samplers[IMAGE_SAMPLERS_COUNT];

layout(set = 3, binding = 0) buffer readonly layers_buffer {
  packed_heraldy_layer_t layers[];
};

layout(location = 0) in flat uint in_layer_index;
layout(location = 1) in vec2 in_uv; // сначало проверяем с coords, а потом переводим в tex_coords
layout(location = 0) out vec4 out_color;

// должно сработать
#include "heraldy_color.glsl"

// супер тяжелый шейдер, мне нужно нарисовать эти вещи и в интерфейсе и на карте, а это скорее всего два разных шейдера
// в цк2 используются оттенки ргб для разных цветов и по ним идет смешивание, возможно мне тоже стоит так сделать
//
void main() {
  out_color = get_heraldy_color(in_layer_index, in_uv);
}
