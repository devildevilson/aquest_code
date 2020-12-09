bool point_in_box(const vec2 point, const vec4 box);
vec4 get_color(const color_t color, const float alpha);
vec3 get_color(const color_t color);
vec4 get_color(const image_t id, const vec2 uv);
vec2 normalize_current_fragment(const vec2 coord, const vec4 box);
vec2 to_texture_space(const vec2 coord, const vec4 box);
vec3 blend_color(const vec3 dst, const vec3 src, const float alpha);

// у нас есть специальный слой отсечения, он нужен чтобы как нибудь верно замостить
// предыдущий трафарет, от этого слоя мы должны сохранить форму (то есть сохранить альфа канал)
// слой продолжения должен пименить полученный цвет и продолжить в нормальном виде
vec4 get_heraldy_color(const uint start_layer, const vec2 current_uv) {
  vec4 final_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  vec4 discard_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  float discard_alpha_value = 0.0f; // че за бред
  uint layer_counter = 0;
  uint next_layer = start_layer;
  bool discard_current = false;
  while (next_layer != GPU_UINT_MAX) {
    const uint current_layer_index = layer_counter;
    ++layer_counter;
    const heraldy_layer_t layer = unpack_heraldy_data(layers[next_layer]);
    next_layer = layer.next_layer;
    const vec4 box = current_layer_index == 0 ? vec4(0.0f, 0.0f, 1.0f, 1.0f) : layer.coords;
    // проверяем попадает ли текущий пиксель в слой, если нет, то пытаемся взять следующий слой
    if (!point_in_box(current_uv, box)) continue;
    // приводим текущий in_uv в пространство текстурок
    const vec2 normalized_uv = normalize_current_fragment(current_uv, box);
    //const vec2 final_uv = to_texture_space(normalized_uv, layer.tex_coords); // тут значение может выйти за пределы tex_coords
    const vec2 final_uv = normalize_current_fragment(normalized_uv, layer.tex_coords); // по идее эта функция хорошо работает даже если значение выходит за пределы бокса
    const vec4 local_color = get_color(layer.stencil, final_uv);
    const bool discard_level = (layer.stencil_type & DISCARD_LAYER_BIT) == DISCARD_LAYER_BIT;
    const bool continue_level = (layer.stencil_type & CONTINUE_LAYER_BIT) == CONTINUE_LAYER_BIT;
    // проверяем если это первый уровень или уровень отсечения, то выходим если не попадаем (discard)
    if (current_layer_index == 0 && local_color.w == 0.0f) break;
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

    const vec3 color_comp_r     = get_color(layer.colors[0]);
    const vec3 color_comp_g     = get_color(layer.colors[1]);
    const vec3 color_comp_b     = get_color(layer.colors[2]);
    const vec3 color_comp_black = get_color(layer.colors[3]);

    const float components_sum = local_color.r + local_color.g + local_color.b;
    const float black_k = 1.0f - min(components_sum, 1.0f);
    // нормализация цвета (сумма компонентов не должна быть больше 1)
    const vec3 components = components_sum > 1.0f ? local_color.xyz / components_sum : local_color.xyz;
    const vec4 final_local_color = vec4(color_comp_r * components.r + color_comp_g * components.g + color_comp_b * components.b + color_comp_black * black_k, local_color.w);

    discard_color.xyz = discard_current ? blend_color(discard_color.xyz, final_local_color.xyz, final_local_color.w) : discard_color.xyz;

    // const uint color_type = 1 * uint(local_color.x == 1.0f && local_color.y == 0.0f && local_color.z == 0.0f) +
    //                         2 * uint(local_color.x == 0.0f && local_color.y == 1.0f && local_color.z == 0.0f) +
    //                         3 * uint(local_color.x == 0.0f && local_color.y == 0.0f && local_color.z == 1.0f) +
    //                         4 * uint(local_color.x == 0.0f && local_color.y == 0.0f && local_color.z == 0.0f);
    // const vec4 final_local_color = color_type == 0 ? local_color : get_color(layer.colors[color_type-1], local_color.w);
    // смешиваем текущий цвет по альфа каналу
    //final_color.xyz = final_color.xyz * (1.0f - final_local_color.w) + final_local_color.xyz * final_local_color.w;
    final_color.xyz = !discard_current ? blend_color(final_color.xyz, final_local_color.xyz, final_local_color.w) : final_color.xyz;


    //final_color.w = final_color.w * final_local_color.w + final_local_color.w * (1.0f - final_local_color.w); // так расчитыватся альфа канал в интерфейсе
    //final_color.w = final_color.w * (1.0f - final_local_color.w) + final_local_color.w * final_local_color.w; // так расчитыватся альфа канал на карте
    //final_color.w = final_color.w * (1.0f - final_local_color.w) + final_local_color.w * final_local_color.w;
    final_color.w = current_layer_index == 0 ? final_local_color.w : final_color.w;
    // повторяем пока есть слои
  }

  //final_color.xyz = blend_color(final_color.xyz, discard_color.xyz, discard_alpha_value);
  final_color.xyz = blend_color(final_color.xyz, discard_color.xyz, discard_alpha_value);
  return final_color;
}

bool point_in_box(const vec2 point, const vec4 box) {
  return point.x >= box.x &&
         point.y >= box.y &&
         point.x <  box.z &&
         point.y <  box.w;
}

vec4 get_color(const color_t color, const float alpha) {
  vec4 ret;
  ret.x = get_color_r(color);
  ret.y = get_color_g(color);
  ret.z = get_color_b(color);
  ret.w = alpha;
  return ret;
}

vec3 get_color(const color_t color) {
  return vec3(
    get_color_r(color),
    get_color_g(color),
    get_color_b(color)
  );
}

vec4 get_color(const image_t id, const vec2 uv) {
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
