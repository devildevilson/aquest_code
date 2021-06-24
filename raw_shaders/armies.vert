#version 450

#extension GL_GOOGLE_include_directive : enable
#include "../render/shared_structures.h"
#include "../utils/shared_mathematical_constant.h"

const vec4 object_points[] = {
  vec4(-1.0f, 2.0f, 0.0f, 1.0f),
  vec4( 1.0f, 2.0f, 0.0f, 1.0f),
  vec4(-1.0f, 0.0f, 0.0f, 1.0f),
  vec4( 1.0f, 0.0f, 0.0f, 1.0f)
};

const vec2 object_uv[] = {
  vec2(0.0f, 0.0f),
  vec2(1.0f, 0.0f),
  vec2(0.0f, 1.0f),
  vec2(1.0f, 1.0f)
};

layout(set = 0, binding = 0) uniform camera_uniform {
  mat4 viewproj;
  mat4 view;
  vec4 pos;
  vec4 dir;
  uvec4 dim;
} camera;

layout(set = 0, binding = 1) uniform matrices_uniform {
  mat4 proj;
  mat4 view;
  mat4 invProj;
  mat4 invView;
  mat4 invViewProj;
} camera_matrices;

layout(std140, set = 2, binding = 0) readonly buffer tiles_buffer {
  light_map_tile_t tiles[];
};

layout(std140, set = 2, binding = 1) readonly buffer biome_data_buffer {
  packed_biome_data_t packed_biome_datas[];
};

layout(std140, set = 2, binding = 2) readonly buffer tile_points_buffer {
  vec4 tile_points[];
};

layout(std140, set = 2, binding = 3) readonly buffer triangles_buffer {
  packed_fast_triangle_t triangles[];
};

layout(std140, set = 2, binding = 4) readonly buffer triangles_tile_indices_buffer {
  uvec4 triangles_tile_indices[];
};

layout(std140, set = 2, binding = 5) readonly buffer world_structure_buffer {
  uvec4 world_structures[];
};

layout(std140, set = 2, binding = 6) readonly buffer additional_index_buffer {
  additional_data_t additional_indices[];
};

layout(std140, set = 2, binding = 7) readonly buffer army_data_buffer { // эти вещи увеличаться в размерах
  army_data_t army_datas[];
};

out gl_PerVertex {
  vec4 gl_Position;
};

layout(location = 0) in uint current_index;
layout(location = 0) out flat image_t out_image;
layout(location = 1) out flat color_t in_biom_color;
layout(location = 2) out vec2 out_uv;
layout(location = 3) out flat float in_tile_height;

mat4 translate(const mat4 mat, const vec4 vec);
mat4 rotate(const mat4 mat, const float angle, const vec4 normal);
mat4 scale(const mat4 mat, const vec4 vec);

void main() {
  const uint army_index  = current_index;
  const uint point_index = gl_VertexIndex; // [0,3]
  //const uint army_index  = gl_VertexIndex / PACKED_INDEX_COEF;
  //const uint point_index = gl_VertexIndex % PACKED_INDEX_COEF; // [0,3]

  const army_data_t army = army_datas[army_index];
  const vec4 pos = vec4(army.data.xyz, 1.0f);
  const uint image_container = floatBitsToUint(army.data.w);
  const float obj_scale = 1.5f;

  const mat4 translaion = translate(mat4(1.0f), pos);
  const mat3 rot = mat3(camera_matrices.invView);
  const mat4 rotation = mat4(rot);
  const mat4 scaling = scale(rotation, vec4(obj_scale, -obj_scale, obj_scale, 0.0f));

  gl_Position = camera.viewproj * translaion * scaling * (object_points[point_index]);
  out_uv = object_uv[point_index];
  out_image.container = image_container;
  in_biom_color.container = 0;
  in_tile_height = 0.0f;

  // лучше конечно написать уникальный фрагментный шейдер для армий, но пока я могу обойтись без него
}

mat4 translate(const mat4 mat, const vec4 vec) {
  mat4 ret = mat;
  ret[3] = mat[0] * vec[0] + mat[1] * vec[1] + mat[2] * vec[2] + mat[3];
  return ret;
}

mat4 rotate(const mat4 mat, const float angle, const vec4 normal) {
  const float c = cos(angle);
  const float s = sin(angle);

  const vec4 temp = normalize(normal) * (1.0f - c);

  mat3 rot;

  rot[0][0] = c + temp[0] * normal[0];
  rot[0][1] = temp[0] * normal[1] + s * normal[2];
  rot[0][2] = temp[0] * normal[2] - s * normal[1];

  rot[1][0] = temp[1] * normal[0] - s * normal[2];
  rot[1][1] = c + temp[1] * normal[1];
  rot[1][2] = temp[1] * normal[2] + s * normal[0];

  rot[2][0] = temp[2] * normal[0] + s * normal[1];
  rot[2][1] = temp[2] * normal[1] - s * normal[0];
  rot[2][2] = c + temp[2] * normal[2];

  mat4 result;
  result[0] = mat[0] * rot[0][0] + mat[1] * rot[0][1] + mat[2] * rot[0][2];
  result[1] = mat[0] * rot[1][0] + mat[1] * rot[1][1] + mat[2] * rot[1][2];
  result[2] = mat[0] * rot[2][0] + mat[1] * rot[2][1] + mat[2] * rot[2][2];
  result[3] = mat[3];
  return result;
}

mat4 scale(const mat4 mat, const vec4 vec) {
  mat4 ret = mat;
  ret[0] *= vec[0];
  ret[1] *= vec[1];
  ret[2] *= vec[2];

  return ret;
}
