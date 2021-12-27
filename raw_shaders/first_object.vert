#version 450

#extension GL_GOOGLE_include_directive : enable
#include "../render/shared_structures.h"
#include "../render/shared_render_utility.h"
#include "../utils/shared_mathematical_constant.h"

const vec4 object_points[] = {
  vec4(-0.5f, 0.5f, 0.0f, 1.0f),
  vec4( 0.5f, 0.5f, 0.0f, 1.0f),
  vec4(-0.5f,-0.5f, 0.0f, 1.0f),
  vec4( 0.5f,-0.5f, 0.0f, 1.0f)
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

float angle_vec(const vec4 v1, const vec4 v2);
float minimum_distance_sqr(const vec4 s1, const vec4 s2, const vec4 point);
mat4 translate(const mat4 mat, const vec4 vec);
mat4 rotate(const mat4 mat, const float angle, const vec4 normal);
mat4 scale(const mat4 mat, const vec4 vec);

void main() {
  const uint tile_index  = gl_VertexIndex / PACKED_INDEX_COEF;
  const uint point_index = gl_VertexIndex % PACKED_INDEX_COEF;
  const uint instance_index = gl_InstanceIndex;
  const map_tile_t tile = unpack_data(tiles[tile_index]);
  const vec4 center = tile_points[tile.center];
  const vec4 first_point = tile_points[tile.points[0]];
  uint prng_state = prng(tile_index);
  const uint prng_state1 = prng(instance_index);
  //prng_state ^= prng_state1; // вроде как работает неплохо
  prng_state = prng2(prng_state, prng_state1);
  const float val_norm1 = prng_normalize(prng_state);
  prng_state = prng(prng_state);
  const float val_norm2 = prng_normalize(prng_state);
  const vec4 normal = vec4(normalize(center.xyz), 0.0f); // центр к сожалению стоит чуть ближе чем обычные точки
  // нужно найти точку на тайле, мы можем легко найти точку в квадрате
  // но лучше будет искать точку в радиусе
  const float norm1 = val_norm1 * 2.0f * PI;
  const float norm2 = sqrt(val_norm2) * objects_radius; // радиус 1 на удивление довольно неплохо зашел

  const float cos_num = norm2 * cos(norm1); // x
  const float sin_num = norm2 * sin(norm1); // y

  // что на что теперь нужно умножить чтобы получить точку на плоскости?

  const float tile_height = tile.height;
  //const uint height_layer = compute_height_layer(tile_height);
  //const float final_height = layer_height * height_layer;
  const float final_height = tile_height;

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
  // решил изменить подход: теперь 3 объекта с какой то вероятностью появляются на биоме
  // вообще я еще думал что эти данные можно еще использовать для анимаций
  // я передаю время в юниформ буфер, а время каждого состояния можно положить например в пробабилити
  const uint biome_index = tile.biome_index >> 24;
  const biome_data_t biome = unpack_data(packed_biome_datas[biome_index]);

  float sum = 0.0f;
  for (uint i = 0; i < map_biome_objects_count; ++i) { sum += biome.probabilities[i]; }
  prng_state = prng(prng_state);
  const float prob = prng_normalize(prng_state) * sum;

  uint index = 0;
  for (float cumulative = 0.0f; index < map_biome_objects_count && cumulative <= prob; cumulative += biome.probabilities[index], ++index);
  index -= 1; // index всегда будет стартовать от 1, из-за особенностей условий в цикле выше

  const image_t img = biome.object_textures[index];
  const float min_scale = biome.scales[index].x;
  const float max_scale = biome.scales[index].y;
  prng_state = prng(prng_state);
  const float obj_scale = mix(min_scale, max_scale, prng_normalize(prng_state));

  // еще я думал что можно добавить дороги, а это означает что нужно проверить дальность с 6 прямыми
  // прямые как задаются? двумя точками... можно упростить задачу если найти в каком треугольнике находится точка
  // это ищется видимо по углу

  const vec4 point = center + x * cos_num + y * sin_num;

  // const uint n_count = tile.points[5] == GPU_UINT_MAX ? 5 : 6;
  // const float external_tile_angle = PI_2 / n_count;
  // float angle = angle_vec(first_point - center, point - center);         // angle = [0,PI]
  // angle = cos_num < 0.0f ? angle + PI : angle; /* sin_num or cos_num? */ // angle = [0,2PI]
  // uint tri_index = 0;
  // for (float cumulative_angle = 0.0f; tri_index < n_count && cumulative_angle <= angle; cumulative_angle += external_tile_angle, ++tri_index);
  // tri_index -= 1;
  // // есть ли в треугольнике tri_index тайла дорога, проверим дистанцию до нее
  // const vec4 tri_point1 = tile_points[tile.points[tri_index]];
  // const vec4 tri_point2 = tile_points[tile.points[(tri_index+1)%n_count]];
  // const float dist_sqr = minimum_distance_sqr(center, (tri_point1+tri_point2) / 2.0f, point);
  // dist_sqr - квадрат расстояния до дороги, с чем мы его сравниваем?
  // во первых нас интересует наличие дороги, а во вторых сравниваем с квадратом половины толщины дороги
  // (const_road_thickness / 2.0f) * (const_road_thickness / 2.0f)

  const vec4 final_point = point + normal * (final_height * render_tile_height + obj_scale);

  mat4 translaion = translate(mat4(1.0f), final_point);
  const mat3 rot = mat3(camera_matrices.invView);
  mat4 rotation = mat4(rot); // говорят что этого достаточно
  mat4 scaling = scale(rotation, vec4(obj_scale, -obj_scale, obj_scale, 0.0f)); // по идее так объекты должны стоять "на ногах"

  gl_Position = camera.viewproj * translaion * scaling * (object_points[point_index]);
  out_uv = object_uv[point_index];
  out_biom_texture = img;

  // почти сделал биомы, нужно поискать текстурки, сделать камеру, кволити оф лайф в общем себе немного сделать
  // с этим способом у меня скорее всего возникнут проблемы с производительностью
  // нужно начинать делать какую то оптимизацию (в смысле ограничивать частоту кадров, дальность прорисовки и прочее и прочее)
  // а для этого нужно делать настройки, теперь когда я подгружаю текстуры, я могу приступать к отрисовке гербов
  // короче еще много нужно сделать
}

float minimum_distance_sqr(const vec4 s1, const vec4 s2, const vec4 point) {
  // Return minimum distance between line segment vw and point p
  const vec4 dir = s2-s1;
  const float l2 = dot(dir, dir);  // i.e. |w-v|^2 -  avoid a sqrt
  // impossible in this case
  //if (abs(l2) < EPSILON) return distance(point, s1);   // s1 == s2 case
  // Consider the line extending the segment, parameterized as v + t (w - v).
  // We find projection of point p onto the line.
  // It falls where t = [(p-v) . (w-v)] / |w-v|^2
  // We clamp t from [0,1] to handle points outside the segment vw.
  const float t = max(0, min(1, dot(point - s1, dir) / l2));
  const vec4 projection = s1 + t * dir;  // Projection falls on the segment
  //return distance(point, projection); // can avoid a sqrt
  const vec4 p_dir = projection - point;
  return dot(p_dir, p_dir);
}

float angle_vec(const vec4 v1, const vec4 v2) {
  const float l1 = dot(v1, v1); // ???
  const float l2 = dot(v2, v2); // ???
  const float d = dot(v1, v2);
  return acos(d / sqrt(l1 * l2));
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
