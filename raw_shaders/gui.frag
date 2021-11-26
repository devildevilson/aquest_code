#version 450 core

#extension GL_GOOGLE_include_directive : enable
#include "../render/shared_structures.h"
#include "../render/image_container_constants.h"

//layout(constant_id = 0) const uint imagesCount = 2;
//layout(constant_id = 1) const uint samplersCount = 1;
const uint heraldy_layers_set_index = 2;

//layout(set = 1, binding = 0) uniform sampler2D font_atlas_texture;
layout(set = 1, binding = 0) uniform texture2DArray textures[IMAGE_CONTAINER_SLOT_SIZE];
layout(set = 1, binding = 1) uniform sampler samplers[IMAGE_SAMPLERS_COUNT];

#include "heraldy_color.glsl"

layout(location = 0) in flat uvec4 in_color;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in flat uint texture_type;
layout(location = 3) in flat uint in_data;

layout(location = 0) out vec4 fragment_color;

void main() {
  vec4 color = vec4(in_color[0]/255.0, in_color[1]/255.0, in_color[2]/255.0, in_color[3]/255.0);

  // я могу какую то информацию записать в цвет
  //const uint index = in_color[3] == 1 ? in_color[0] : UINT_MAX;
  //const uint layer = in_color[3] == 1 ? in_color[1] : UINT_MAX;
  //color = in_color[3] == 1 ? texture(sampler2DArray(textures[index], samplers[0]), vec3(in_uv, float(layer))) : color;

  switch(texture_type) {
    case IMAGE_TYPE_DEFAULT: { // обычное изображение
      image_t in_texture;
      in_texture.container = in_data;
      const bool valid_texture = is_image_valid(in_texture);
      const uint image_index = get_image_index(in_texture);
      const uint layer_index = get_image_layer(in_texture);
      const uint sampler_index = get_image_sampler(in_texture);
      const vec4 texture_color = valid_texture ? texture(sampler2DArray(textures[image_index], samplers[sampler_index]), vec3(in_uv, float(layer_index))) : vec4(1.0f, 1.0f, 1.0f, 1.0f);

      fragment_color = color * texture_color;
      break;
    }

    case IMAGE_TYPE_HERALDY: { // геральдика
      // тут мы передаем индекс геральдики в функцию
      const uint heraldy_index = in_data;
      const vec4 heraldy_color = get_heraldy_color(heraldy_index, in_uv, GPU_UINT_MAX);

      fragment_color = color * heraldy_color;
      break;
    }

    case IMAGE_TYPE_FACE: { // лицо
      // по идее здесь по аналогии с предыдущим, здесь мы рисуем лицо из кусочков

      break;
    }

    // что то еще? да должна добавиться текстура похожая на отдельный слой геральдики
    // то есть текстура и к ней 4 цвета, для чего? может быть легкий способ задизайнить интерфейс
    //

    default: fragment_color = color;
  }
}
