#version 450

#extension GL_GOOGLE_include_directive : enable
#include "../render/shared_structures.h"
#include "../utils/shared_mathematical_constant.h"

// std430 работает в vulkan иначе
// фактически работает как std140
// то есть требуется складывать данные
// в структуры размер которых делится на 16 без остатка

// гекс состоит из треугольников
const vec2 uv_borders[] = {
  vec2(0.0f, 0.0f),
  vec2(1.0f, 0.0f),
  vec2(0.5f, 0.5f)
};

layout(set = 0, binding = 0) uniform Camera {
  mat4 viewproj;
  mat4 view;
  vec4 pos;
  vec4 dir;
  uvec4 dim;
} camera;

layout(std140, set = 1, binding = 0) readonly buffer tiles_buffer {
  light_map_tile_t tiles[];
};

layout(std140, set = 1, binding = 1) readonly buffer biomes_buffer {
  packed_biom_data_t biomes[];
};

layout(std140, set = 1, binding = 2) readonly buffer tile_points_buffer {
  vec4 tile_points[];
};

out gl_PerVertex {
  vec4 gl_Position;
};

// теперь у нас нет никаких выходных вершинных буферов
//layout(location = 0) in uint tile_index;  // инстансный буфер
//layout(location = 1) in uint point_index; // буфер вида [0,...,4,0,...,5], указываем оффсет
//layout(location = 0) out flat uint out_biom_index;
layout(location = 0) out flat image_t out_biom_texture;
layout(location = 1) out flat color_t out_biom_color;
layout(location = 2) out vec2 out_uv;
layout(location = 3) out flat float out_tile_height;

// либо мы uv координаты можем посчитать в скрин спейсе
// нет лучше здесь
void main() {
  const uint tile_index  = gl_VertexIndex / PACKED_TILE_INDEX_COEF; 
  const uint point_index = gl_VertexIndex % PACKED_TILE_INDEX_COEF;
  const map_tile_t tile = unpack_data(tiles[tile_index]);
  //const uint tile_center = tile.center;
  const uint point_id = tile.points[point_index];
  //const uint point_id = tile.neighbours[point_index].points[0];
  const vec4 point = tile_points[point_id];
  //const vec4 tile_norm = vec4(normalize(tile_points[tile_center].xyz), 0.0f);
  // if (point_index == 0) point = tile_points[tile.index]; // мне не обязательно брать здесь центральную точку
  // else {
  //   const uint final_point_index = point_index - 1;
  //   const uint id = tile.points[final_point_index];
  //   point = tile_points[id];
  // }

  const vec3 n = normalize(point.xyz);
  const vec2 uv = vec2(atan(n.x, n.z) / (PI_2) + 0.5f, n.y * 0.5f + 0.5f);
  // const float layer_height = mountain_height / float(layers_count);
  // const uint height_layer = tile.height < 0.0f ? 0 : (tile.height >= mountain_height ? layers_count : uint(tile.height / layer_height));
  //const float layer_height = 1.0f / float(layers_count);
  const uint height_layer = compute_height_layer(tile.height);
  const float final_height = layer_height * height_layer;

  //const float final_height = tile.height < 0.0f ? 0.0f : tile.height;
  gl_Position = camera.viewproj * (point + vec4(n, 0.0f) * final_height * render_tile_height); // возможно не 10, а еще чуть чуть поменьше
  //gl_Position = camera.viewproj * point;
  out_uv = uv; // наверное на что нибудь нужно умножить
  //out_image = biomes[tile.biom_index].img;
  // image_t img;
  // img.container = 0;
  // out_image = img;
  //out_biom_index = tile.biom_index;
  //out_biom_index = tile.unique_object_index;
  out_biom_texture = tile.texture;
  out_biom_color = tile.color;
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
