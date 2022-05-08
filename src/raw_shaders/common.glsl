#ifndef DEVILS_ENGINE_SHADER_COMMON_GLSL
#define DEVILS_ENGINE_SHADER_COMMON_GLSL

#include "../render/shared_structures.h"

bool point_in_box(const vec2 point, const vec4 box) {
  return point.x >= box.x &&
         point.y >= box.y &&
         point.x <  box.z &&
         point.y <  box.w;
}

// vec4 get_color(const color_t color, const float alpha) {
//   return vec4(
//     get_color_r(color),
//     get_color_g(color),
//     get_color_b(color),
//     alpha
//   );
// }

// для взятия цвета нужно пользоваться вот этим: unpackUnorm4x8
// vec3 get_color3(const color_t color) {
//   return vec3(
//     get_color_r(color),
//     get_color_g(color),
//     get_color_b(color)
//   );
// }

vec4 get_stencil_color(const image_t id, const vec2 uv) {
  if (!is_image_valid(id)) return vec4(0.0f, 0.0f, 0.0f, 1.0f);
  const uint index = get_image_index(id);
  const uint layer = get_image_layer(id);
  const uint sampler_id = get_image_sampler(id);
  const float fu = flip_u(id) ? -1.0f : 1.0f;
  const float fv = flip_v(id) ? -1.0f : 1.0f;
  const vec3 final_uv = vec3(uv.x * fu, uv.y * fv, float(layer));
  return texture(sampler2DArray(textures[index], samplers[sampler_id]), final_uv);
}

// предположительно box.xy - минимальная точка, box.zw - максимальная точка
vec2 normalize_current_fragment(const vec2 coord, const vec4 box) {
  return (coord - box.xy) / (box.zw - box.xy);
}

// vec2 to_texture_space(const vec2 coord, const vec4 box) {
//   //return vec2(mix());
//   //return mix(box.xy, box.zw, coord); // нет, не микс
//   // по идее нам просто нужно разделить числа
//   return (coord - box.xy) / (box.zw - box.xy); // мы пришли к формуле выше...
// }

vec3 blend_color(const vec3 dst, const vec3 src, const float alpha) {
  return dst * (1.0f - alpha) + src * alpha;
}

vec4 reverse_alpha(const vec4 color) {
  return vec4(color.rgb, 1.0f-color.a);
}

vec4 select(const bool cond, const vec4 first, const vec4 second) {
  return mix(first, second, 1.0f-float(cond));
}

float select(const bool cond, const float first, const float second) {
  return mix(first, second, 1.0f-float(cond));
}

float compute_layer(
  const heraldy_layer_t layer, 
  //const image_t current_stencil, 
  const vec2 current_uv,
  inout vec4 final_color, 
  inout vec4 discard_color, 
  inout float discard_alpha_value,
  inout bool discard_current
) {
  const vec4 box = layer.coords;
  // проверяем попадает ли текущий пиксель в слой, если нет, то пытаемся взять следующий слой
  if (!point_in_box(current_uv, box)) return 0.0f;
  // приводим текущий in_uv в пространство текстурок
  const vec2 normalized_uv = normalize_current_fragment(current_uv, box);
  //const vec2 final_uv = to_texture_space(normalized_uv, layer.tex_coords); // тут значение может выйти за пределы tex_coords
  const vec2 final_uv = normalize_current_fragment(normalized_uv, layer.tex_coords); // по идее эта функция хорошо работает даже если значение выходит за пределы бокса
  const bool discard_level = (layer.stencil_type & DISCARD_LAYER_BIT) == DISCARD_LAYER_BIT;
  const bool continue_level = (layer.stencil_type & CONTINUE_LAYER_BIT) == CONTINUE_LAYER_BIT;
  const bool reverse_stencil_aplha = (layer.stencil_type & REVERSE_STENCIL_ALPHA_BIT) == REVERSE_STENCIL_ALPHA_BIT;
  //const vec4 stencil_color = get_stencil_color(current_stencil, final_uv);
  const bool stencil_exists = is_image_valid(layer.stencil);
  const vec4 stencil_color = get_stencil_color(layer.stencil, final_uv);
  const vec4 local_color = select(reverse_stencil_aplha, reverse_alpha(stencil_color), stencil_color);

  // после проверки смотрим что за цвет к нам пришел, если это р,г,б или черный, то берем цвет из массива, иначе из изображения
  // в цк2 ргб - это оттенки заданных цветов, это означает лучшее смешение среди 4 цветов, но при этом отсутствие своих цветов на изображении
  // с другой стороны мы можем записать свой цвет в особый альфа канал, то есть если альфа канал равен определенному числу (например x.w * 255 == 1)

  if (continue_level) {
    final_color.rgb = blend_color(final_color.rgb, discard_color.rgb, discard_alpha_value);
    discard_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    discard_alpha_value = 0.0f;
    discard_current = false;
  }

  discard_alpha_value = discard_level ? local_color.w : discard_alpha_value;
  discard_current = discard_level ? true : discard_current;

  if ((discard_current && discard_alpha_value == 0.0f) || local_color.w == 0.0f) return 0.0f;

  vec4 final_local_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  if (is_image_valid(layer.image)) {
    const vec4 image_color = get_stencil_color(layer.image, final_uv);
    final_local_color = vec4(image_color.rgb, select(stencil_exists, local_color.a, image_color.a));
  } else {
    const vec3 color_comp_r     = vec3(get_color(layer.colors[0]));
    const vec3 color_comp_g     = vec3(get_color(layer.colors[1]));
    const vec3 color_comp_b     = vec3(get_color(layer.colors[2]));
    const vec3 color_comp_black = vec3(get_color(layer.colors[3]));

    const float components_sum = local_color.r + local_color.g + local_color.b;
    const float black_k = 1.0f - min(components_sum, 1.0f);
    // нормализация цвета (сумма компонентов не должна быть больше 1) (а зачем я это делаю? да еще и не особ правильно)
    const vec3 components = components_sum > 1.0f ? local_color.rgb / components_sum : local_color.rgb;
    // тут цвет может получиться больше 1 если не сделать нормализация раньше
    final_local_color = vec4(color_comp_r * components.r + color_comp_g * components.g + color_comp_b * components.b + color_comp_black * black_k, local_color.a);
  }

  discard_color.rgb = discard_current ? blend_color(discard_color.rgb, final_local_color.rgb, final_local_color.a) : discard_color.rgb;
  final_color.rgb = !discard_current ? blend_color(final_color.rgb, final_local_color.rgb, final_local_color.a) : final_color.rgb;
  return final_local_color.a;
}

#endif