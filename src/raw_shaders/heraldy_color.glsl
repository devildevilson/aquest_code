#include "../render/heraldy.h"
#include "common.glsl"

// тут добавятся еще слои геральдики заданные в рантайме, может быть вообще все нудно в рантайме задавать?
// вообще это довольно большой объем копирования, мне нужно будет каждую страну (около 1000) нарисовать
// 1000 * [4-6] * 64 = 384кб (вообще не особ много, но вообще может быть больше)
// нет, думаю что реально надо отдельный буфер завести, просто надо бы аккуратно рассовать по функциям то что здесь происходит

layout(set = heraldy_layers_set_index, binding = 0) buffer readonly heraldy_layers_buffer {
  packed_heraldy_layer_t layers[];
};

layout(set = heraldy_layers_set_index, binding = 1) buffer readonly heraldy_layer_index_buffer {
  uvec4 layer_indices[];
};

uint get_layer_index(const uint buffer_index);
vec4 get_heraldy_color(const uint layers_count, const uint start_layer, const vec2 current_uv, const image_t img_stencil);

// нужно передавать щит отдельным слоем (то есть слой с трафаретом по которому отсечем все пискели вне щита)
// щит требует только трафарет? или что может быть что то еще? размер? если мы указываем щит
// то мы уже скорее всего хотим занять все пространство доступное
vec4 get_heraldy_color(const uint start_layer, const vec2 current_uv, const image_t img_stencil) {
  if (start_layer == GPU_UINT_MAX) return vec4(0.0f, 0.0f, 0.0f, 1.0f);
  // в start_layer хранится количество слоев, start_layer+1 - первый слой, который мы только что посчитали
  // размер который мы получили относится к индексам ПОСЛЕ собственно размера то есть индексы лежат например так
  // {3, 1, 2, 3}, было бы неплохо убедиться что титулы указывают именно на количество
  const uint layers_count = get_layer_index(start_layer);
  return get_heraldy_color(layers_count, start_layer+1, current_uv, img_stencil);
}

// у нас есть специальный слой отсечения, он нужен чтобы как нибудь верно замостить
// предыдущий трафарет, от этого слоя мы должны сохранить форму (то есть сохранить альфа канал)
// слой продолжения должен пименить полученный цвет и продолжить в нормальном виде
vec4 get_heraldy_color(const uint layers_count, const uint start_layer, const vec2 current_uv, const image_t img_stencil) {
  if (start_layer == GPU_UINT_MAX) return vec4(0.0f, 0.0f, 0.0f, 1.0f);

  vec4 final_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  vec4 discard_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  float discard_alpha_value = 0.0f;
  uint layer_counter = 0;
  bool discard_current = false;

  const vec4 stencil_color = get_stencil_color(img_stencil, current_uv);
  if (stencil_color.w == 0.0f) return vec4(0.0f, 0.0f, 0.0f, 0.0f);
  final_color.w = stencil_color.w;

  //const uint layers_count = get_layer_index(start_layer);
  //const uint first_layer  = get_layer_index(start_layer);

  // единственное место где я использую current_layer_index - это первый слой, нужно избавится от этой штуки
  //const heraldy_layer_t layer = unpack_heraldy_data(layers[next_layer]);
  //next_layer = layer.next_layer;
  //const heraldy_layer_t layer = unpack_heraldy_data(layers[first_layer]);
  // image_t current_stencil = layer.stencil;
  // if (shield_stencil != GPU_UINT_MAX) {
  //   const heraldy_layer_t shield_layer = unpack_heraldy_data(layers[shield_stencil]);
  //   current_stencil = shield_layer.stencil;
  // }

  //final_color.w = 
  //compute_layer(layer, current_uv, final_color, discard_color, discard_alpha_value, discard_current); //current_stencil
  //if (final_color.w == 0.0f) return vec4(0.0f, 0.0f, 0.0f, 0.0f);

  // const vec4 box = layer.coords;
  // // приводим текущий in_uv в пространство текстурок
  // const vec2 normalized_uv = normalize_current_fragment(current_uv, box);
  // //const vec2 final_uv = to_texture_space(normalized_uv, layer.tex_coords); // тут значение может выйти за пределы tex_coords
  // const vec2 final_uv = normalize_current_fragment(normalized_uv, layer.tex_coords); // по идее эта функция хорошо работает даже если значение выходит за пределы бокса
  // const vec4 local_color = get_stencil_color(current_stencil, final_uv);
  // const bool discard_level = (layer.stencil_type & DISCARD_LAYER_BIT) == DISCARD_LAYER_BIT;
  // const bool continue_level = (layer.stencil_type & CONTINUE_LAYER_BIT) == CONTINUE_LAYER_BIT;
  // // проверяем если это первый уровень или уровень отсечения, то выходим если не попадаем (discard)
  // if (local_color.w == 0.0f) return vec4(0.0f, 0.0f, 0.0f, 0.0f);
  // // после проверки смотрим что за цвет к нам пришел, если это р,г,б или черный, то берем цвет из массива, иначе из изображения
  // // в цк2 ргб - это оттенки заданных цветов, это означает лучшее смешение среди 4 цветов, но при этом отсутствие своих цветов на изображении
  // // с другой стороны мы можем записать свой цвет в особый альфа канал, то есть если альфа канал равен определенному числу (например x.w * 255 == 1)

  // if (continue_level) {
  //   final_color.xyz = blend_color(final_color.xyz, discard_color.xyz, discard_alpha_value);
  //   discard_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  //   discard_alpha_value = 0.0f;
  //   discard_current = false;
  // }

  // discard_alpha_value = discard_level ? local_color.w : discard_alpha_value;
  // discard_current = discard_level ? true : discard_current;

  // //if ((discard_current && discard_alpha_value == 0.0f) || local_color.w == 0.0f) continue;

  // const vec3 color_comp_r     = get_color3(layer.colors[0]);
  // const vec3 color_comp_g     = get_color3(layer.colors[1]);
  // const vec3 color_comp_b     = get_color3(layer.colors[2]);
  // const vec3 color_comp_black = get_color3(layer.colors[3]);

  // const float components_sum = local_color.r + local_color.g + local_color.b;
  // const float black_k = 1.0f - min(components_sum, 1.0f);
  // // нормализация цвета (сумма компонентов не должна быть больше 1, почему?)
  // const vec3 components = components_sum > 1.0f ? local_color.xyz / components_sum : local_color.xyz;
  // const vec4 final_local_color = vec4(color_comp_r * components.r + color_comp_g * components.g + color_comp_b * components.b + color_comp_black * black_k, local_color.w);

  // discard_color.xyz = discard_current ? blend_color(discard_color.xyz, final_local_color.xyz, final_local_color.w) : discard_color.xyz;
  // final_color.xyz = !discard_current ? blend_color(final_color.xyz, final_local_color.xyz, final_local_color.w) : final_color.xyz;
  // final_color.w = final_local_color.w; // current_layer_index == 0 ?

  for (uint i = start_layer; i < start_layer+layers_count; ++i) {
    const uint layer_index = get_layer_index(i);
    const heraldy_layer_t layer = unpack_heraldy_data(layers[layer_index]);
    compute_layer(layer, current_uv, final_color, discard_color, discard_alpha_value, discard_current); //layer.stencil, 
  }

  final_color.rgb = blend_color(final_color.rgb, discard_color.rgb, discard_alpha_value);
  return final_color;
}

uint get_layer_index(const uint buffer_index) {
  const uint cont_index = buffer_index / 4;
  const uint index_cont = buffer_index % 4;
  return layer_indices[cont_index][index_cont];
}