#version 450 core

#extension GL_GOOGLE_include_directive : enable
#include "../render/shared_structures.h"
#include "../render/image_container_constants.h"

//layout(constant_id = 0) const uint imagesCount = 2;
//layout(constant_id = 1) const uint samplersCount = 1;
const uint heraldy_layers_set_index = 2;
const uint interface_layers_set_index = 3;

//layout(set = 1, binding = 0) uniform sampler2D font_atlas_texture;
layout(set = 1, binding = 0) uniform texture2DArray textures[IMAGE_CONTAINER_SLOT_SIZE];
layout(set = 1, binding = 1) uniform sampler samplers[IMAGE_SAMPLERS_COUNT];

#include "heraldy_color.glsl"
#include "layered_color.glsl"

layout(location = 0) in flat uvec4 in_color;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in flat uint texture_type;
layout(location = 3) in flat uint in_data;
layout(location = 4) in flat image_t in_stencil;
layout(location = 5) in flat uint in_user_color;

layout(location = 0) out vec4 fragment_color;

vec4 get_image_color(const image_t img, const vec2 uv);
vec4 get_color4(const color_t color);

void main() {
  vec4 color = vec4(in_color[0]/255.0, in_color[1]/255.0, in_color[2]/255.0, in_color[3]/255.0);

  switch(texture_type) {
    case IMAGE_TYPE_DEFAULT: { // обычное изображение
      const vec4 texture_color = get_image_color(image_t(in_data), in_uv);
      fragment_color = color * texture_color;
      break;
    }

    case IMAGE_TYPE_HERALDY: { // геральдика
      const uint heraldy_index = in_data;
      const vec4 heraldy_color = get_heraldy_color(heraldy_index, in_uv, in_stencil);
      fragment_color = color * heraldy_color;
      break;
    }

    case IMAGE_TYPE_FACE: { // лицо
      // по идее здесь по аналогии с предыдущим, здесь мы рисуем лицо из кусочков

      break;
    }

    case IMAGE_TYPE_LAYERED_IMAGE: { // послойное изображение, очень похоже на геральдику, но отличие в том откуда берем информацию о слоях
      const uint start_index = in_data;
      const vec4 layered_color = get_layered_color(start_index, 1, in_uv, in_stencil);
      fragment_color = color * layered_color;
      break;
    }

    case IMAGE_TYPE_ALPHA_STENCIL: {
      const vec4 stencil_data = get_image_color(in_stencil, in_uv);
      const vec4 user_color = get_color4(color_t(in_user_color));
      const float val = uintBitsToFloat(in_data);
      const vec4 final_frag_color = select(val <= stencil_data.a, user_color, vec4(0.0f, 0.0f, 0.0f, 0.0f)); // может умножить user_color.rgb на stencil_data.rgb дополнительно?
      fragment_color = color * final_frag_color;
      break;
    }

    // было бы неплохо еще сгененировать текстуру с помощью перлина, 
    // вообще наверное еще было бы неплохо воспользоваться перлином с слоях, но эт наверное совсем тяжело для вычислений
    // что нужно для перлина? базовый цвет + мультипликатор + неплохо было бы передать сид

    default: fragment_color = color;
  }
}

vec4 get_image_color(const image_t img, const vec2 uv) {
  const bool valid_texture = is_image_valid(img);
  if (!valid_texture) return vec4(1.0f, 1.0f, 1.0f, 1.0f);
  const uint image_index = get_image_index(img);
  const uint layer_index = get_image_layer(img);
  const uint sampler_index = get_image_sampler(img);
  const float fu = flip_u(img) ? -1.0f : 1.0f;
  const float fv = flip_v(img) ? -1.0f : 1.0f;
  const vec3 final_uv = vec3(uv.x * fu, uv.y * fv, float(layer_index));
  return texture(sampler2DArray(textures[image_index], samplers[sampler_index]), final_uv);
}

vec4 get_color4(const color_t color) {
  return vec4(
    get_color_r(color),
    get_color_g(color),
    get_color_b(color),
    get_color_a(color)
  );
}

// float color_perlin(const vec2 xy, const float seed) { return perlin3D_noise(vec3(1.5*xy, seed)); }
// vec4 fBm_perlin_gray_color(const vec2 coord, const float seed) {
//   // у меня наверное приходят изначально верные coord
//   vec2 p = (coord.xy/iResolution.y) * 2.0 - 1.0;

//   vec3 xyz = vec3(p, 0);
//   // осталось понять зачем так? мы используем фрактиальное Броуновское движение (fractal Brownian motion) для изменения свойств шума
//   // больше инфы тут https://www.redblobgames.com/x/2107-webgl-noise/webgl-noise/webdemo/
//   // 
//   // не будет ли эта функция слишком долгой? хотя наверное если только покрыть небольшую область то почему нет
//   vec2 step = vec2(1.3, 1.7);
//   float n =      color_perlin(xyz.xy, seed);
//   n += 0.5     * color_perlin(xyz.xy *  2.0 - 1.0 * step, seed);
//   n += 0.25    * color_perlin(xyz.xy *  4.0 - 2.0 * step, seed);
//   n += 0.125   * color_perlin(xyz.xy *  8.0 - 3.0 * step, seed);
//   n += 0.0625  * color_perlin(xyz.xy * 16.0 - 4.0 * step, seed);
//   n += 0.03125 * color_perlin(xyz.xy * 32.0 - 5.0 * step, seed);

//   return vec4(0.5 + 0.5 * vec3(n, n, n), 1.0f);
// }

// // в предыдущем получается как будто туман, в этом случайные продолжительные островки
// vec4 perlin_gray_color(const vec2 coord, const float seed) {
//   vec2 p = (coord.xy/iResolution.y) * 2.0 - 1.0;
//   vec3 xyz = vec3(p, 0);
//   float n = color_perlin(xyz.xy * 4.0, seed);
//   return vec4(0.5 + 0.5 * vec3(n, n, n), 1.0f);
// }