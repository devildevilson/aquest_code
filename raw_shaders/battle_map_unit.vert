#version 450

#extension GL_GOOGLE_include_directive : enable
#include "../render/shared_structures.h"
#include "../render/shared_battle_structures.h"
#include "../utils/shared_mathematical_constant.h"
#include "../render/shared_render_utility.h"

const vec2 box_uv[] = {
  vec2(0.0f, 0.0f),
  vec2(1.0f, 0.0f),
  vec2(0.0f, 1.0f),
  vec2(1.0f, 1.0f)
};

const vec4 box_pos[] = {
  vec4(-0.5f, 0.5f, 0.0f, 1.0f),
  vec4( 0.5f, 0.5f, 0.0f, 1.0f),
  vec4(-0.5f,-0.5f, 0.0f, 1.0f),
  vec4( 0.5f,-0.5f, 0.0f, 1.0f)
};

layout(set = 0, binding = 0) uniform Camera {
  mat4 viewproj;
  mat4 view;
  vec4 pos;
  vec4 dir;
  uvec4 dim;
  vec4 cursor_dir;
} camera;

layout(set = 0, binding = 1) uniform matrices_uniform {
  mat4 proj;
  mat4 view;
  mat4 invProj;
  mat4 invView;
  mat4 invViewProj;
} camera_matrices;

layout(set = 2, binding = 0) uniform tiles_uniform_buffer {
  uvec4 map_properties;
} tiles_uniform;

layout(std140, set = 2, binding = 5) readonly buffer units_buffer {
  unit_t units[];
};

out gl_PerVertex {
  vec4 gl_Position;
};

layout(location = 0) in uint unit_index;  // инстансный буфер
layout(location = 0) out flat image_t out_image;
layout(location = 1) out vec2 out_uv;

mat4 translate(const mat4 mat, const vec4 vec);
mat4 rotate(const mat4 mat, const float angle, const vec4 normal);
mat4 scale(const mat4 mat, const vec4 vec);
vec4 project_vector_on_plane(const vec4 normal, const vec4 origin, const vec4 vector);

void main() {
  const uint point_index = gl_VertexIndex;
  const unit_t current_unit = units[unit_index];

  image_t unit_texture;
  unit_texture.container = floatBitsToUint(current_unit.pos.w);
  //const float unit_scale = unit_index == 0 ? 1.0f : current_unit.dir.w;
  const float unit_scale = current_unit.dir.w;

  //const uint textures_offset = floatBitsToUint(current_unit.pos.w);
  //const uint textures_count = floatBitsToUint(current_unit.dir.w);
  //const vec4 unit_pos = unit_index == 0 ? vec4(0.0f, 3.5f, 0.0f, 1.0f) : vec4(current_unit.pos.xyz, 1.0f) + vec4(0.0f, 1.0f, 0.0f, 0.0f) * (unit_scale / 2.0f);
  const vec4 unit_pos = vec4(current_unit.pos.xyz, 1.0f) + vec4(0.0f, 1.0f, 0.0f, 0.0f) * (unit_scale / 2.0f);
  const vec4 unit_dir = vec4(current_unit.dir.xyz, 0.0f);

  // тут нужно во первых составить матрицу, не уверен что хорошая идея ставить юнитов как в думе
  // то есть нужно придумать как вывести изображение юнитов как бы с видом сверху
  // во вторых что с текстурками? хорошо было бы если бы камера оставалась фиксированой, так можно
  // было бы использовать только одну текстурку ..... нет, это неважно
  // нужно сделать так чтобы юнит выглядел так будто у него есть направление
  // с другой стороны хорошая идея заключается в том что мы можем флипать изображение в
  // зависимости от того где находится враг и камера, так реально можно будет использовать одну картинку
  // но все равно было бы неплохо сделать выбор текстурок в зачаточном виде

  // const float angle_part = PI_2 / textures_count;
  // const float angle_half = angle_part / 2.0f;
  // // нужно привести к одной плоскости
  // const vec4 dir = normalize(unit_pos - camera.pos);
  // // вернет угол до 180 градусов, нужно сохранить знак
  // const float angle = find_angle(unit_dir, dir);
  // // где то нужно прибавить angle_half
  // const uint index = uint((angle + angle_half) / (angle_part)) % textures_count;
  // // как то так получим индекс

  const mat4 translaion = translate(mat4(1.0f), unit_pos);
  const mat4 rot_matrix = camera_matrices.invView;
  const mat4 final_rot = mat4(mat3(rot_matrix));
  const float final_scale = unit_scale; // кстати а какой скейл? и как указать? скейл явно зависит от юнита
  const mat4 scaling = scale(final_rot, vec4(final_scale, -final_scale, final_scale, 0.0f));

  // я так понимаю, лучше использовать две картинки по аналогии с биомами
  const vec4 up_dir = vec4(0.0f, 1.0f, 0.0f, 0.0f);
  const vec4 forward_dir = normalize(project_vector_on_plane(up_dir, vec4(0.0f, 0.0f, 0.0f, 1.0f), camera.dir));
  const vec4 right_dir = normalize(vec4(cross(vec3(forward_dir), vec3(up_dir)), 0.0f));

  const bool need_flip = dot(right_dir, unit_dir) < 0;
  // составим изображение

  const uint img_index = get_image_index(unit_texture);
  const uint img_layer = get_image_layer(unit_texture);
  const uint img_sampler_id = get_image_sampler(unit_texture);
  const bool img_fu = flip_u(unit_texture);
  const bool img_fv = flip_v(unit_texture);

  gl_Position = camera.viewproj * translaion * scaling * (box_pos[point_index]);
  out_image = make_image(img_index, img_layer, img_sampler_id, need_flip ? !img_fu : img_fu, img_fv);
  out_uv = box_uv[point_index];
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

vec4 project_vector_on_plane(const vec4 normal, const vec4 origin, const vec4 vector) {
  const float dist = dot(vector, normal);
  const vec4 point2 = origin + vector - normal*dist;
  return point2 - origin;
}
