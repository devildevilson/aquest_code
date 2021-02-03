#version 450 core

#extension GL_GOOGLE_include_directive : enable
#include "../render/shared_structures.h"

const vec2 default_uv[] = {
  vec2(0.0f, 0.0f),
  vec2(1.0f, 0.0f),
  vec2(1.0f, 1.0f),
  vec2(0.0f, 1.0f)
};

layout(set = 0, binding = 0) uniform UniformBufferObject {
  mat4 proj;
} ubo;

layout(push_constant) uniform uPushConstant {
  image_t texture;
} pc;

layout(location = 0) in vec2 in_pos;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in uvec4 in_color;

layout(location = 0) out flat uvec4 out_color;
layout(location = 1) out vec2 out_uv;
layout(location = 2) out vec2 out_square_uv;
layout(location = 3) out flat image_t out_texture;

out gl_PerVertex {
  vec4 gl_Position;
};

void main() {
  out_color = in_color;
  out_uv = in_uv;
  out_square_uv = default_uv[gl_VertexIndex % 4];
  //out_texture_indices = uvec2(pc.texture.imageIndex, pc.texture.layerIndex);
  out_texture = pc.texture;
  gl_Position = ubo.proj * vec4(in_pos, 0.0f, 1.0f);
}

// inPos - это точки квадратной поверхности гуи элемента, причем это может быть как окно
// так и буковка, в принципе и в том и вдругом случае мне может потребоваться нарисовать
// фон, при этом желательно чтобы фон не деформировался (то есть картинка не сжималась от координат)
// квадратные координаты = фон ровно по картинке, для этого мне по идее нужен доступ к
// предыдущим координатам, в общем я так подумал без доступа ко всей информации о шрифте
// крайне сложно сделать так как я хочу, при этом доступ ко всем шрифтам - это не сказать чтобы очень плохая идея
// но стандартными средствами наклира она не реализовывается
