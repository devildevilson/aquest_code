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

out gl_PerVertex {
  vec4 gl_Position;
};

layout(location = 0) out flat image_t out_biom_texture;
layout(location = 1) out flat color_t out_biom_color;
layout(location = 2) out vec2 out_uv;
layout(location = 3) out flat float out_tile_height;

const float objects_radius = 1.2f;

mat4 translate(const mat4 mat, const vec4 vec);
mat4 rotate(const mat4 mat, const float angle, const vec4 normal);
mat4 scale(const mat4 mat, const vec4 vec);

void main() {
  const uint tile_index  = gl_VertexIndex / PACKED_INDEX_COEF;
  const uint point_index = gl_VertexIndex % PACKED_INDEX_COEF;
  const uint instance_index = gl_InstanceIndex;
  const vec4 center = tile_points[tiles[tile_index].tile_indices.x];
  uint prng_state = prng(tile_index);
  uint prng_state1 = prng(instance_index);
  prng_state ^= prng_state1; // вроде как работает неплохо
  // for (uint i = 1; i < instance_index+1; ++i) { // увеличиваем стейт
  //   prng_state = prng(prng_state);
  // }

  const uint additional = prng(prng_state);
  const float val_norm1 = prng_normalize(prng_state);
  const float val_norm2 = prng_normalize(additional);
  const vec4 normal = vec4(normalize(center.xyz), 0.0f); // центр к сожалению стоит чуть ближе чем обычные точки
  // нужно найти точку на тайле, мы можем легко найти точку в квадрате
  // но лучше будет искать точку в радиусе
  const float norm1 = val_norm1 * 2.0f * PI;
  const float norm2 = sqrt(val_norm2) * objects_radius; // радиус 1 на удивление довольно неплохо зашел

  const float cos_num = norm2 * cos(norm1); // x
  const float sin_num = norm2 * sin(norm1); // y

  // что на что теперь нужно умножить чтобы получить точку на плоскости?

  const float tile_height = uintBitsToFloat(tiles[tile_index].tile_indices.w);
  const uint height_layer = compute_height_layer(tile_height);
  const float final_height = layer_height * height_layer;

  vec4 x, y;
  if (abs(normal.x) < EPSILON && abs(normal.y) < EPSILON) {
    x = vec4(1.0f, 0.0f, 0.0f, 0.0f);
    y = vec4(0.0f, 1.0f, 0.0f, 0.0f);
  } else {
    x = normalize(vec4(-normal[1], normal[0], 0.0f, 0.0f));
    y = normalize(vec4(-normal[0]*normal[2], -normal[1]*normal[2], normal[0]*normal[0] + normal[1]*normal[1], 0.0f));
  }

  // первый объект - это объект у которого только одна степерь свободы (как у объектов в думе)
  // второй объект - это объект у которого две степени свободы (билборд) (потестировав, я не уверен нужно ли мне первый тип объектов теперь вообще)
  // сейчас пока что мы сделаем второй объект
  const uint biome_index = tiles[tile_index].packed_data4[2] >> 24;
  const biome_data_t biome = unpack_data(packed_biome_datas[biome_index]);
  const float min_scale = biome.min_scale2;
  const float max_scale = biome.max_scale2;
  const uint scale_num = prng(additional);
  const float obj_scale = mix(min_scale, max_scale, prng_normalize(scale_num));

  // тут нужно вычислить несколько матриц
  const vec4 point = center + x * cos_num + y * sin_num + normal * (final_height * render_tile_height + obj_scale/2); // как то вот так выглядит неплохо
  mat4 translaion = translate(mat4(1.0f), point);
  const mat3 rot = mat3(camera_matrices.invView);
  mat4 rotation = mat4(rot); // говорят что этого достаточно
  mat4 scaling = scale(rotation, vec4(obj_scale, -obj_scale, obj_scale, 0.0f)); // по идее так объекты должны стоять "на ногах"

  gl_Position = camera.viewproj * translaion * scaling * (object_points[point_index]);
  out_uv = object_uv[point_index];
  out_biom_texture = biome.object_texture2;

  // почти сделал биомы, нужно поискать текстурки, сделать камеру, кволити оф лайф в общем себе немного сделать
  // с этим способом у меня скорее всего возникнут проблемы с производительностью
  // нужно начинать делать какую то оптимизацию (в смысле ограничивать частоту кадров, дальность прорисовки и прочее и прочее)
  // а для этого нужно делать настройки, теперь когда я подгружаю текстуры, я могу приступать к отрисовке гербов
  // короче еще много нужно сделать
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
