#version 450

#extension GL_GOOGLE_include_directive : enable
#include "../render/shared_structures.h"
#include "../utils/shared_mathematical_constant.h"

const vec4 object_points[] = {
  vec4(-1.0f, 1.0f, 0.0f, 1.0f),
  vec4( 1.0f, 1.0f, 0.0f, 1.0f),
  vec4(-1.0f,-1.0f, 0.0f, 1.0f),
  vec4( 1.0f,-1.0f, 0.0f, 1.0f)
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

out gl_PerVertex {
  vec4 gl_Position;
};

layout(location = 0) in uint current_index;
layout(location = 0) out flat uint out_layer_index;
layout(location = 1) out vec2 out_uv;

mat4 translate(const mat4 mat, const vec4 vec);
mat4 rotate(const mat4 mat, const float angle, const vec4 normal);
mat4 scale(const mat4 mat, const vec4 vec);
world_structure_t unpack(const uvec4 data);

void main() {
  const uint tile_index  = current_index;
  const uint point_index = gl_VertexIndex; // [0,3]
  //const uint tile_index  = gl_VertexIndex / PACKED_INDEX_COEF;
  //const uint point_index = gl_VertexIndex % PACKED_INDEX_COEF; // [0,3]

  const light_map_tile_t tile = tiles[tile_index];
  const float tile_height = uintBitsToFloat(tile.tile_indices.w);
  const vec4 center = tile_points[tile.tile_indices.x];
  const vec4 normal = vec4(normalize(center.xyz), 0.0f);

  //const mat4 tans_view = transpose(camera.view);
  //const vec4 cam_right =  vec4(camera_matrices.view[0][0], camera_matrices.view[1][0], camera_matrices.view[2][0], 0.0f);
  const vec4 cam_up    = -vec4(camera_matrices.view[0][1], camera_matrices.view[1][1], camera_matrices.view[2][1], 0.0f);
  //const vec4 cam_forw  = -vec4(camera_matrices.view[0][2], camera_matrices.view[1][2], camera_matrices.view[2][2], 0.0f);

  const uint heraldy_layer_index = additional_indices[tile_index].data[0].y;
  // const uint structure_index = tiles[tile_index].packed_data4.z & maximum_structure_types;
  // const world_structure_t structure_data = unpack(world_structures[structure_index]);
  // const uint heraldy_layer_index = structure_data.heraldy_layer_index;

  // нужно нарисовать геральдику на некотором удалении от тайлов
  // в цк3 геральдику можно выделить и получить немного информации
  // как это делается? как как нужно посчитать 4 точки и посмотреть куда направлен курсор
  // можно ли как то это сделать в шейдере?

  // размер зависит от дальности
  const uint height_layer = compute_height_layer(tile_height);
  const float final_height = layer_height * height_layer;
  const float zoom = uintBitsToFloat(camera.dim.z);
  const float obj_scale = mix(0.5f, 1.0f, zoom); // размер один и тот же? вряд ли
  const float dist = mix(1.0f, 10.0f, zoom); // независимые государства повыше?
  const vec4 point = center + normal * (final_height * render_tile_height) + cam_up * dist;
  const mat4 translaion = translate(mat4(1.0f), point);
  const mat3 rot = mat3(camera_matrices.invView);
  const mat4 rotation = mat4(rot);
  const mat4 scaling = scale(rotation, vec4(obj_scale, -obj_scale, obj_scale, 0.0f));

  gl_Position = camera.viewproj * translaion * scaling * (object_points[point_index]);
  out_uv = object_uv[point_index];
  out_layer_index = heraldy_layer_index;
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

world_structure_t unpack(const uvec4 data) {
  world_structure_t abc;
  abc.city_image_top.container = data[0];
  abc.city_image_face.container = data[1];
  abc.scale = uintBitsToFloat(data[2]);
  abc.heraldy_layer_index = data[3];
  return abc;
}
