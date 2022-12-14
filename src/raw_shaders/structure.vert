#version 450

#extension GL_GOOGLE_include_directive : enable
#include "../render/shared_structures.h"
#include "../core/shared_structures.h"
#include "../utils/shared_mathematical_constant.h"

// const vec4 object_points[] = {
//   vec4(-1.0f, 2.0f, 0.0f, 1.0f),
//   vec4( 1.0f, 2.0f, 0.0f, 1.0f),
//   vec4(-1.0f, 0.0f, 0.0f, 1.0f),
//   vec4( 1.0f, 0.0f, 0.0f, 1.0f)
// };

const vec4 object_points[] = {
  vec4(-0.5f, 1.0f, 0.0f, 1.0f),
  vec4( 0.5f, 1.0f, 0.0f, 1.0f),
  vec4(-0.5f, 0.0f, 0.0f, 1.0f),
  vec4( 0.5f, 0.0f, 0.0f, 1.0f)
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
} camera;

layout(set = 0, binding = 1) uniform matrices_uniform {
  mat4 proj;
  mat4 view;
  mat4 invProj;
  mat4 invView;
  mat4 invViewProj;
} camera_matrices;

layout(set = 0, binding = 2) uniform common_uniform {
  vec4 cursor_dir;
  uvec4 dim;
  uvec4 state;
} additional;

layout(std140, set = 2, binding = 0) readonly buffer tiles_buffer {
  light_map_tile_t tiles[];
};

layout(std140, set = 2, binding = 1) readonly buffer biome_data_buffer {
  packed_biome_data_t packed_biome_datas[];
};

layout(std140, set = 2, binding = 2) readonly buffer tile_points_buffer {
  vec4 tile_points[];
};

layout(std140, set = 2, binding = 5) readonly buffer world_structure_buffer {
  uvec4 world_structures[];
};

layout(std140, set = 2, binding = 6) readonly buffer additional_index_buffer {
  additional_data_t additional_indices[];
};

// где то тут еще должен быть буфер структур
// было бы неплохо его запихнуть к всем остальным буферам

out gl_PerVertex {
  vec4 gl_Position;
};

layout(location = 0) in uint tile_index;
layout(location = 1) in uint img;
layout(location = 2) in float obj_scale;
layout(location = 0) out flat image_t out_biom_texture;
layout(location = 1) out flat color_t out_biom_color;
layout(location = 2) out vec2 out_uv;
layout(location = 3) out flat float out_tile_height;

mat4 translate(const mat4 mat, const vec4 vec);
mat4 rotate(const mat4 mat, const float angle, const vec4 normal);
mat4 scale(const mat4 mat, const vec4 vec);
world_structure_t unpack(const uvec4 data);

void main() {
  const uint point_index = gl_VertexIndex; // [0, 3]

  const vec4 center = tile_points[tiles[tile_index].packed_data[3][0]];
  const vec4 normal = vec4(normalize(center.xyz), 0.0f);

  const float tile_height = uintBitsToFloat(tiles[tile_index].packed_data[3][1]);
  //const uint height_layer = compute_height_layer(tile_height);
  //const float final_height = layer_height * height_layer;
  const float final_height = tile_height;

  //const uint structure_index = additional_indices[tile_index].data[0].x;
  //const uint structure_index = tiles[tile_index].packed_data4[2] & maximum_structure_types;
  //const world_structure_t structure = unpack(world_structures[structure_index]);
  //const float obj_scale = structure.scale; // плохое изображение
  //const float obj_scale = scale; // плохое изображение
  //const float obj_scale = 2.0f; // приходит неправильный скейл

  // все что тут нужно сделать это:
  // вспомнить как сделать дум спрайты
  // сделать пару матриц
  const float zoom = uintBitsToFloat(additional.dim.z);

  const vec4 point = center + normal * (final_height * render_tile_height); //  + obj_scale/2.0f
  const mat4 translaion = translate(mat4(1.0f), point);
  // константа по которой делим приближение?
    const mat3 rot = mat3(camera_matrices.invView);
    const mat4 rotation = mat4(rot); // говорят что этого достаточно
    const mat4 scaling = scale(rotation, vec4(obj_scale, -obj_scale, obj_scale, 0.0f));
    gl_Position = camera.viewproj * translaion * scaling * (object_points[point_index]); // obj_scale
    out_uv = object_uv[point_index];

  out_biom_texture.container = img; // одно изображение?

  // if (zoom > 0.3f) {
  //   out_biom_texture = structure.city_image_top;
  // } else {
  //   out_biom_texture = structure.city_image_face;
  // }

  // } else {
  //   // кажется толку с этого не очень много, разве что все равно полезно сменить изображение
  //   // нам нужно тогда угол сделать гораздо больше, а это не особо имеет смысла
  //   const vec4 dir = camera.dir;
  //
  //   vec4 x, y;
  //   if (abs(normal.x) < EPSILON && abs(normal.y) < EPSILON) {
  //     x = vec4(1.0f, 0.0f, 0.0f, 0.0f);
  //     y = vec4(0.0f, 1.0f, 0.0f, 0.0f);
  //   } else {
  //     x = normalize(vec4(-normal[1], normal[0], 0.0f, 0.0f));
  //     y = normalize(vec4(-normal[0]*normal[2], -normal[1]*normal[2], normal[0]*normal[0] + normal[1]*normal[1], 0.0f));
  //   }
  //
  //   const mat4 mat = mat4(y, normal, x, vec4(0.0f, 0.0f, 0.0f, 1.0f));
  //   const mat4 scaling = scale(mat, vec4(obj_scale, obj_scale, obj_scale, 0.0f));
  //
  //   gl_Position = camera.viewproj * translaion * scaling * (object_points[point_index]);
  //   out_uv = object_uv[point_index];
  //   out_biom_texture = structure.city_image_face;
  // }
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
