#include "../render/heraldy.h"

layout(set = heraldy_layers_set_index, binding = 0) buffer readonly layers_buffer {
  packed_heraldy_layer_t layers[];
};

layout(set = heraldy_layers_set_index, binding = 1) buffer readonly layer_index_buffer {
  uvec4 layer_indices[];
};

bool point_in_box(const vec2 point, const vec4 box);
//vec4 get_color(const color_t color, const float alpha);
vec3 get_color3(const color_t color);
vec4 get_stencil_color(const image_t id, const vec2 uv);
vec2 normalize_current_fragment(const vec2 coord, const vec4 box);
vec2 to_texture_space(const vec2 coord, const vec4 box);
vec3 blend_color(const vec3 dst, const vec3 src, const float alpha);
uint get_layer_index(const uint buffer_index);

// у нас есть специальный слой отсечения, он нужен чтобы как нибудь верно замостить
// предыдущий трафарет, от этого слоя мы должны сохранить форму (то есть сохранить альфа канал)
// слой продолжения должен пименить полученный цвет и продолжить в нормальном виде
vec4 get_heraldy_color(const uint start_layer, const vec2 current_uv, const uint shield_stencil) {
  if (start_layer == GPU_UINT_MAX) return vec4(0.0f, 0.0f, 0.0f, 1.0f);

  vec4 final_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  vec4 discard_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  float discard_alpha_value = 0.0f;
  uint layer_counter = 0;
  bool discard_current = false;

  const uint layers_count = get_layer_index(start_layer);
  const uint first_layer  = get_layer_index(start_layer+1);

  // единственное место где я использую current_layer_index - это первый слой, нужно избавится от этой штуки
  //const heraldy_layer_t layer = unpack_heraldy_data(layers[next_layer]);
  //next_layer = layer.next_layer;
  const heraldy_layer_t layer = unpack_heraldy_data(layers[first_layer]);
  image_t current_stencil = layer.stencil;
  if (shield_stencil != GPU_UINT_MAX) {
    const heraldy_layer_t shield_layer = unpack_heraldy_data(layers[shield_stencil]);
    current_stencil = shield_layer.stencil;
  }
  const vec4 box = layer.coords;
  // приводим текущий in_uv в пространство текстурок
  const vec2 normalized_uv = normalize_current_fragment(current_uv, box);
  //const vec2 final_uv = to_texture_space(normalized_uv, layer.tex_coords); // тут значение может выйти за пределы tex_coords
  const vec2 final_uv = normalize_current_fragment(normalized_uv, layer.tex_coords); // по идее эта функция хорошо работает даже если значение выходит за пределы бокса
  const vec4 local_color = get_stencil_color(current_stencil, final_uv);
  const bool discard_level = (layer.stencil_type & DISCARD_LAYER_BIT) == DISCARD_LAYER_BIT;
  const bool continue_level = (layer.stencil_type & CONTINUE_LAYER_BIT) == CONTINUE_LAYER_BIT;
  // проверяем если это первый уровень или уровень отсечения, то выходим если не попадаем (discard)
  if (local_color.w == 0.0f) return vec4(0.0f, 0.0f, 0.0f, 0.0f);
  // после проверки смотрим что за цвет к нам пришел, если это р,г,б или черный, то берем цвет из массива, иначе из изображения
  // в цк2 ргб - это оттенки заданных цветов, это означает лучшее смешение среди 4 цветов, но при этом отсутствие своих цветов на изображении
  // с другой стороны мы можем записать свой цвет в особый альфа канал, то есть если альфа канал равен определенному числу (например x.w * 255 == 1)

  if (continue_level) {
    final_color.xyz = blend_color(final_color.xyz, discard_color.xyz, discard_alpha_value);
    discard_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    discard_alpha_value = 0.0f;
    discard_current = false;
  }

  discard_alpha_value = discard_level ? local_color.w : discard_alpha_value;
  discard_current = discard_level ? true : discard_current;

  //if ((discard_current && discard_alpha_value == 0.0f) || local_color.w == 0.0f) continue;

  const vec3 color_comp_r     = get_color3(layer.colors[0]);
  const vec3 color_comp_g     = get_color3(layer.colors[1]);
  const vec3 color_comp_b     = get_color3(layer.colors[2]);
  const vec3 color_comp_black = get_color3(layer.colors[3]);

  const float components_sum = local_color.r + local_color.g + local_color.b;
  const float black_k = 1.0f - min(components_sum, 1.0f);
  // нормализация цвета (сумма компонентов не должна быть больше 1)
  const vec3 components = components_sum > 1.0f ? local_color.xyz / components_sum : local_color.xyz;
  const vec4 final_local_color = vec4(color_comp_r * components.r + color_comp_g * components.g + color_comp_b * components.b + color_comp_black * black_k, local_color.w);

  discard_color.xyz = discard_current ? blend_color(discard_color.xyz, final_local_color.xyz, final_local_color.w) : discard_color.xyz;
  final_color.xyz = !discard_current ? blend_color(final_color.xyz, final_local_color.xyz, final_local_color.w) : final_color.xyz;
  final_color.w = final_local_color.w; // current_layer_index == 0 ?

  // в start_layer хранится количество слоев, start_layer+1 - первый слой, который мы только что посчитали
  // размер который мы получили относится к индексам ПОСЛЕ собственно размера то есть индексы лежат например так
  // {3, 1, 2, 3}, было бы неплохо убедиться что титулы указывают именно на количество
  for (uint i = start_layer+1+1; i < start_layer+1+layers_count; ++i) {
    const uint layer_index = get_layer_index(i);
  //while (next_layer != GPU_UINT_MAX) {
    //const heraldy_layer_t layer = unpack_heraldy_data(layers[next_layer]);
    //next_layer = layer.next_layer;
    const heraldy_layer_t layer = unpack_heraldy_data(layers[layer_index]);
    const vec4 box = layer.coords;
    // проверяем попадает ли текущий пиксель в слой, если нет, то пытаемся взять следующий слой
    if (!point_in_box(current_uv, box)) continue;
    // приводим текущий in_uv в пространство текстурок
    const vec2 normalized_uv = normalize_current_fragment(current_uv, box);
    //const vec2 final_uv = to_texture_space(normalized_uv, layer.tex_coords); // тут значение может выйти за пределы tex_coords
    const vec2 final_uv = normalize_current_fragment(normalized_uv, layer.tex_coords); // по идее эта функция хорошо работает даже если значение выходит за пределы бокса
    const vec4 local_color = get_stencil_color(layer.stencil, final_uv);
    const bool discard_level = (layer.stencil_type & DISCARD_LAYER_BIT) == DISCARD_LAYER_BIT;
    const bool continue_level = (layer.stencil_type & CONTINUE_LAYER_BIT) == CONTINUE_LAYER_BIT;

    // после проверки смотрим что за цвет к нам пришел, если это р,г,б или черный, то берем цвет из массива, иначе из изображения
    // в цк2 ргб - это оттенки заданных цветов, это означает лучшее смешение среди 4 цветов, но при этом отсутствие своих цветов на изображении
    // с другой стороны мы можем записать свой цвет в особый альфа канал, то есть если альфа канал равен определенному числу (например x.w * 255 == 1)

    if (continue_level) {
      final_color.xyz = blend_color(final_color.xyz, discard_color.xyz, discard_alpha_value);
      discard_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
      discard_alpha_value = 0.0f;
      discard_current = false;
    }

    discard_alpha_value = discard_level ? local_color.w : discard_alpha_value;
    discard_current = discard_level ? true : discard_current;

    if ((discard_current && discard_alpha_value == 0.0f) || local_color.w == 0.0f) continue;

    const vec3 color_comp_r     = get_color3(layer.colors[0]);
    const vec3 color_comp_g     = get_color3(layer.colors[1]);
    const vec3 color_comp_b     = get_color3(layer.colors[2]);
    const vec3 color_comp_black = get_color3(layer.colors[3]);

    const float components_sum = local_color.r + local_color.g + local_color.b;
    const float black_k = 1.0f - min(components_sum, 1.0f);
    // нормализация цвета (сумма компонентов не должна быть больше 1) (а зачем я это делаю? да еще и не особ правильно)
    const vec3 components = components_sum > 1.0f ? local_color.xyz / components_sum : local_color.xyz;
    const vec4 final_local_color = vec4(color_comp_r * components.r + color_comp_g * components.g + color_comp_b * components.b + color_comp_black * black_k, local_color.w);

    discard_color.xyz = discard_current ? blend_color(discard_color.xyz, final_local_color.xyz, final_local_color.w) : discard_color.xyz;
    final_color.xyz = !discard_current ? blend_color(final_color.xyz, final_local_color.xyz, final_local_color.w) : final_color.xyz;
  }

  final_color.xyz = blend_color(final_color.xyz, discard_color.xyz, discard_alpha_value);
  return final_color;
}

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

vec3 get_color3(const color_t color) {
  return vec3(
    get_color_r(color),
    get_color_g(color),
    get_color_b(color)
  );
}

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

vec2 to_texture_space(const vec2 coord, const vec4 box) {
  //return vec2(mix());
  //return mix(box.xy, box.zw, coord); // нет, не микс
  // по идее нам просто нужно разделить числа
  return (coord - box.xy) / (box.zw - box.xy); // мы пришли к формуле выше...
}

vec3 blend_color(const vec3 dst, const vec3 src, const float alpha) {
  return dst * (1.0f - alpha) + src * alpha;
}

uint get_layer_index(const uint buffer_index) {
  const uint cont_index = buffer_index / 4;
  const uint index_cont = buffer_index % 4;
  return layer_indices[cont_index][index_cont];
}
