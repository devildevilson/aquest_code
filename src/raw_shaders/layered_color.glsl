#include "../render/heraldy.h"
#include "common.glsl"

layout(set = interface_layers_set_index, binding = 0) buffer readonly interface_layers_buffer {
  packed_heraldy_layer_t interface_layers[];
};

vec4 get_layered_color(const uint start_layer, const uint layers_count, const vec2 current_uv, const image_t img_stencil) {
  if (start_layer == GPU_UINT_MAX) return vec4(0.0f, 0.0f, 0.0f, 1.0f);

  vec4 final_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  vec4 discard_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  float discard_alpha_value = 0.0f;
  uint layer_counter = 0;
  bool discard_current = false;

  const vec4 stencil_color = get_stencil_color(img_stencil, current_uv);
  if (stencil_color.w == 0.0f) return vec4(0.0f, 0.0f, 0.0f, 0.0f);
  final_color.w = stencil_color.w;

  for (uint i = start_layer; i < start_layer+layers_count; ++i) {
    const uint layer_index = get_layer_index(i);
    const heraldy_layer_t layer = unpack_heraldy_data(interface_layers[layer_index]);
    compute_layer(layer, current_uv, final_color, discard_color, discard_alpha_value, discard_current); //layer.stencil, 
  }

  final_color.rgb = blend_color(final_color.rgb, discard_color.rgb, discard_alpha_value);
  return final_color;
}