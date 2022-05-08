struct color_pack_t {
  vec4 r_component;
  vec4 g_component;
  vec4 b_component;
  vec4 inv_component;
};

layout(constant_id = 666) const uint composite_colors_set_index = 2;

layout(set = composite_colors_set_index, binding = 0) buffer readonly composite_colors {
  color_pack_t colors_container[];
};

vec4 get_composite_color(const vec4 stencil_color, const uint index) {
  const color_pack_t pack = colors_container[index];
  const float inv_k = max(1.0f - (stencil_color.r + stencil_color.g + stencil_color.b), 0.0f);
  const vec4 color1 = stencil_color.r * pack.r_component;
  const vec4 color2 = stencil_color.g * pack.g_component;
  const vec4 color3 = stencil_color.b * pack.b_component;
  const vec4 color4 = inv_k * pack.inv_component;

  vec4 final_color = color1 + color2 + color3 + color4;
  final_color.a = stencil_color.a;
  return final_color;
}
