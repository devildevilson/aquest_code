#version 450

#extension GL_GOOGLE_include_directive : enable
#include "../render/shared_structures.h"
#include "../utils/shared_mathematical_constant.h"

// std430 работает в vulkan иначе
// фактически работает как std140
// то есть требуется складывать данные
// в структуры размер которых делится на 16 без остатка

const vec2 hexagon_uv[] = {
  vec2(0.75f, 0.932932f),
  vec2(1.0f, 0.5f),
  vec2(0.75f, 0.067068f),
  vec2(0.25f, 0.067068f),
  vec2(0.0f, 0.5f),
  vec2(0.25f, 0.932932f)
};

const vec2 pentagon_uv[] = {
  vec2(0.363298f, 1.0f),
  vec2(0.950969f, 0.808957f),
  vec2(0.950968f, 0.191043f),
  vec2(0.363298f, 0.0f),
  vec2(0.0f, 0.5f)
};

const uint hex_tile_points_indices[] = { 0, 1, 5, 2, 4, 3 };
const uint pen_tile_points_indices[] = { 0, 1, 4, 2, 3 };

layout(set = 0, binding = 0) uniform Camera {
  mat4 viewproj;
  mat4 view;
  vec4 pos;
  vec4 dir;
  uvec4 dim;
} camera;

layout(std140, set = 2, binding = 0) readonly buffer tiles_buffer {
  light_map_tile_t tiles[];
};

layout(std140, set = 2, binding = 1) readonly buffer biomes_buffer {
  packed_biome_data_t biomes[];
};

layout(std140, set = 2, binding = 2) readonly buffer tile_points_buffer {
  vec4 tile_points[];
};

out gl_PerVertex {
  vec4 gl_Position;
};

// теперь у нас нет никаких выходных вершинных буферов
layout(location = 0) in uint tile_index;  // инстансный буфер
layout(location = 0) out flat image_t out_biom_texture;
layout(location = 1) out flat color_t out_biom_color;
layout(location = 2) out vec2 out_uv;
layout(location = 3) out flat float out_tile_height;

//void convert_xyz_to_cube_uv(float x, float y, float z, int *index, float *u, float *v);
vec2 convert_xyz_to_cube_uv(const vec3 point);

// либо мы uv координаты можем посчитать в скрин спейсе
// нет лучше здесь
void main() {
  const uint point_index = gl_VertexIndex;
  //const uint tile_index  = gl_VertexIndex / PACKED_TILE_INDEX_COEF;
  //const uint point_index = gl_VertexIndex % PACKED_TILE_INDEX_COEF;
  const map_tile_t tile = unpack_data(tiles[tile_index]);

  const bool is_pentagon = tile_index < 12;
  const uint index_offset = is_pentagon ? 10 : 12;
  const bool rendering_walls = point_index < index_offset;
  const bool basement_point = point_index % 2 == 1;
  const uint side_index = point_index / 2;
  const uint tile_point_index = point_index - index_offset;

  //const uint final_point_index = uint(rendering_walls) * side_index + uint(!rendering_walls) * tile_point_index;

  const uint final_point_index = is_pentagon ? pen_tile_points_indices[tile_point_index] : hex_tile_points_indices[tile_point_index];
  const uint point_id = rendering_walls ? tile.points[side_index] : tile.points[final_point_index];
  const vec4 point = tile_points[point_id];

  // если мы используем сферические текстурные координаты, то к краям сферы текстурка сильно деформируется
  // чтобы избежать этого мы можем сделать несколько вещей: не использовать текстурки вообще,
  // использовать текстурные координаты гексагона или расчитать текстурные координаты кубемап

  //const vec3 n = normalize(point.xyz);
  const vec3 n = point.xyz / WORLD_RADIUS_CONSTANT;
  //const vec2 uv = vec2(atan(n.x, n.z) / (PI_2) + 0.5f, n.y * 0.5f + 0.5f);
  //const vec2 uv = vec2(atan(n.x, n.z) / (PI_2) + 0.5f, asin(n.y) / PI_2 - 0.5f); // в оригинале должно быть asin(n.y) / PI
  // на границах куба получаются полоски плохой текстуры с неправильными координатами
  // наверное можно исправить случайно раскидав эти тайлы по соседним кубам
  //const vec2 uv = convert_xyz_to_cube_uv(n);
  const vec2 walls_uv = mix(vec2(0.0f, float(side_index)), vec2(1.0f, float(side_index)), float(basement_point));
  const vec2 tile_uv = is_pentagon ? pentagon_uv[tile_point_index] : hexagon_uv[tile_point_index];
  // const float layer_height = mountain_height / float(layers_count);
  // const uint height_layer = tile.height < 0.0f ? 0 : (tile.height >= mountain_height ? layers_count : uint(tile.height / layer_height));
  //const float layer_height = 1.0f / float(layers_count);
  const uint height_layer = compute_height_layer(tile.height);
  const float final_height = rendering_walls && basement_point ? 0.0f : layer_height * height_layer;

  //const float final_height = tile.height < 0.0f ? 0.0f : tile.height;
  gl_Position = camera.viewproj * (point + vec4(n, 0.0f) * final_height * render_tile_height); // возможно не 10, а еще чуть чуть поменьше
  //gl_Position = camera.viewproj * point;
  //out_uv = uv * 100.0f; // наверное на что нибудь нужно умножить
  out_uv = rendering_walls ? walls_uv : tile_uv;
  //out_image = biomes[tile.biom_index].img;
  // image_t img;
  // img.container = 0;
  // out_image = img;
  //out_biom_index = tile.biom_index;
  //out_biom_index = tile.unique_object_index;
  const float wall_color_coef = 0.85f;
  const color_t wall_color = make_color1(
    get_color_r(tile.color)*wall_color_coef,
    get_color_g(tile.color)*wall_color_coef,
    get_color_b(tile.color)*wall_color_coef,
    1.0f
  );
  out_biom_texture = tile.texture;
  out_biom_color = rendering_walls ? wall_color : tile.color;
  out_tile_height = tile.height;
}

// для того чтобы сработал подъем на карте нужно три вещи
// стенки между тайлами (закроют фон)
// более адекватный рейкастинг (с учетом стенок по всей видимости)
// более адекватный фрустум куллинг (я полагаю что нужно просто все треугольники засунуть в октодерево (?))

// не уверен правда что это будет хорошо выглядеть
// нужно поиграться с высотой, возможно ранжировать их по уровням
// (несколько высот, последняя высота - непроходимые горы)
// (так еще поменьше стенок можно делать, но и выглядеть это будет скоре всего не очень)
// у меня горы начинаются от 0.5, мне наверное нужно еще нормализовать
// хорошо будет выглядеть это все дело в случе если расстояния между тайлами будут небольшими,
// маскимальное количество тайлов будет неоднородным и дополнительно графика будет отмечать особенности ландшафта
// тут видимо нужно использовать комбинацию из разных методов

// вопрос с структурой данных остается открытым
// проблема в том что структура данных занимает место, у меня около 500к тайлов
// какое нужно разбиение? ко всему прочему октодерево не особенно эффективно
// у нас уже есть вообще то структура данных, нам нужно проверить значит максимумы и минимумы
// то есть проверить условный 3-мерный треугольник с заданной толщиной
// бокс? ну эт просто, фрустум также? до тайлов, как тайлы проверить?
// в фрустум поди можно закинуть бокс, а проверку с лучем хотелось бы поточнее
//

vec2 convert_xyz_to_cube_uv(const vec3 point) {
  const float abs_x = abs(point.x);
  const float abs_y = abs(point.y);
  const float abs_z = abs(point.z);

  const bool x_positive = point.x > 0;
  const bool y_positive = point.y > 0;
  const bool z_positive = point.z > 0;

  float max_axis;
  vec2 uv;

  // POSITIVE X
  if (x_positive && abs_x >= abs_y && abs_x >= abs_z) {
    // u (0 to 1) goes from +z to -z
    // v (0 to 1) goes from -y to +y
    max_axis = abs_x;
    uv.x = -point.z;
    uv.y = point.y;
  }
  // NEGATIVE X
  if (!x_positive && abs_x >= abs_y && abs_x >= abs_z) {
    // u (0 to 1) goes from -z to +z
    // v (0 to 1) goes from -y to +y
    max_axis = abs_x;
    uv.x = point.z;
    uv.y = point.y;
  }
  // POSITIVE Y
  if (y_positive && abs_y >= abs_x && abs_y >= abs_z) {
    // u (0 to 1) goes from -x to +x
    // v (0 to 1) goes from +z to -z
    max_axis = abs_y;
    uv.x = point.x;
    uv.y = -point.z;
  }
  // NEGATIVE Y
  if (!y_positive && abs_y >= abs_x && abs_y >= abs_z) {
    // u (0 to 1) goes from -x to +x
    // v (0 to 1) goes from -z to +z
    max_axis = abs_y;
    uv.x = point.x;
    uv.y = point.z;
  }
  // POSITIVE Z
  if (z_positive && abs_z >= abs_x && abs_z >= abs_y) {
    // u (0 to 1) goes from -x to +x
    // v (0 to 1) goes from -y to +y
    max_axis = abs_z;
    uv.x = point.x;
    uv.y = point.y;
  }
  // NEGATIVE Z
  if (!z_positive && abs_z >= abs_x && abs_z >= abs_y) {
    // u (0 to 1) goes from +x to -x
    // v (0 to 1) goes from -y to +y
    max_axis = abs_z;
    uv.x = -point.x;
    uv.y = point.y;
  }

  // Convert range from -1 to 1 to 0 to 1
  uv.x = 0.5f * (uv.x / max_axis + 1.0f);
  uv.y = 0.5f * (uv.y / max_axis + 1.0f);
  return uv;
}
