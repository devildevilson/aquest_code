#ifndef SHARED_STRUCTURES_H
#define SHARED_STRUCTURES_H

#ifdef __cplusplus

//#include "basic_tri.h"
#include "utils/utility.h"

#define DEFAULT_DESCRIPTOR_POOL_NAME "default_descriptor_pool"
#define UNIFORM_BUFFER_LAYOUT_NAME "uniform_layout"
#define MATRICES_BUFFER_LAYOUT_NAME "matrixes_layout"
#define STORAGE_BUFFER_LAYOUT_NAME "storage_layout"
#define SAMPLED_IMAGE_LAYOUT_NAME "sampled_image_layout"
#define SKYBOX_TEXTURE_LAYOUT_NAME "skybox_texture_layout"
#define TILES_DATA_LAYOUT_NAME "tiles_data_layout"

#define IMAGE_CONTAINER_DESCRIPTOR_POOL_NAME "image_container_descriptor_pool"
#define IMAGE_CONTAINER_DESCRIPTOR_LAYOUT_NAME "image_container_descriptor_layout"
#define IMAGE_SAMPLER_LINEAR_NAME "image_sampler_linear"
#define IMAGE_SAMPLER_NEAREST_NAME "image_sampler_nearest"

#define INLINE inline
#define INOUT(type) type&

namespace devils_engine {
  namespace render {
    using glm::floatBitsToUint;
    using glm::uintBitsToFloat;
    using glm::abs;
    using glm::dot;
    using glm::min;
    using glm::max;

    using uint = uint32_t;
    // using mat4 = basic_mat4;
    // using vec4 = basic_vec4;
    using mat4 = glm::mat4;
    using vec4 = glm::vec4;
    using vec2 = glm::vec2;
    using uvec2 = glm::uvec2;
    using vec3 = glm::vec3;
    using uvec4 = glm::uvec4;

    const char* const biome_names[] = {
      "biome_ocean",
      "biome_ocean_glacier",
      "biome_desert",
      "biome_rain_forest",
      "biome_rocky",
      "biome_plains",
      "biome_swamp",
      "biome_grassland",
      "biome_deciduous_forest",
      "biome_tundra",
      "biome_land_glacier",
      "biome_conifer_forest",
      "biome_mountain",
      "biome_snowy_mountain"
    };
#else

#define INLINE
#define INOUT(type) inout type

#endif

#define GPU_UINT_MAX 0xffffffff
#define GPU_INT_MAX  0x7fffffff
#define PACKED_INDEX_COEF 4
#define PACKED_TILE_INDEX_COEF 6
#define MAX_BIOMES_COUNT (0xff)
#define WORLD_RADIUS_CONSTANT 500.0f

#define IMAGE_TYPE_DEFAULT 0 // просто картинка из общего пула
#define IMAGE_TYPE_HERALDY 1 // геральдика уже как то рисуется
#define IMAGE_TYPE_FACE 2    // лицо будет рисоваться очень похожим на геральдику способом
// что тут может еще добавиться?

const uint layers_count = 20;
const float render_tile_height = 7.0f;
const float mountain_height = 0.5f;
const float layer_height = 1.0f / float(layers_count);

const uint border_offset_mask      = 0x0fffffff; // 2^28
const uint border_size_mask        = 0x0000000f; // 2^4
const uint connections_offset_mask = 0x0fffffff; // 2^28
const uint connections_size_mask   = 0x0000000f; // 2^4

const uint maximum_structure_types = 0x00ffffff; // 2^24

struct image_t {
  uint container;
};

struct color_t {
  uint container;
};

struct image_data {
  image_t img;
  float movementU;
  float movementV;
};

struct vertex {
  vec4 pos;
  vec4 color;
  vec2 tex_coord;
};

struct light_data {
  vec4 pos;
  vec4 color;
};

struct camera_data {
  mat4 viewproj;
  mat4 view;
  vec4 pos;
  vec4 dir;
};

struct matrices_data {
  mat4 proj;
  mat4 view;
  mat4 invProj;
  mat4 invView;
  mat4 invViewProj;
};

struct common_data {
  vec4 cursor_dir;
  uvec4 dim; // x - width, y - heigth, z - zoom, w - time
};

struct matrices {
  mat4 persp;
  mat4 ortho;
  mat4 view;
  matrices_data matrixes;
  camera_data camera;
  common_data additional;
};

struct instance_data_t {
  uvec4 index;
};

struct map_triangle_t {
  uint points[3];
  uint current_level;
  uint upper_level_index;
  uint next_level[4];
};

struct fast_triangle_t {
  uint points[3];
  uint offset;
  uint count;
};

struct packed_fast_triangle_t {
  uvec4 points;
  uvec4 data;
};

// добавится индекс биома
struct light_map_tile_t {
  uvec4 packed_data[5];
};

// мне нужно переместить map_tile_t в gpu память
// center, height, points, neighbors, connections_data (тут по идее будут дороги) - эти данные не меняются по ходу игры,
// borders_data - меняется потенциально каждый ход,
// texture, color, biome_index - потенциально могут измениться в любой момент (например при переключении вида карты)
// эти данные нужно поставить вместе и тогда я легко могу составить данные для копирования
struct map_tile_t {
  uint points[6];
  uint neighbors[6];
  uint center;
  float height;
  uint connections_data;
  uint dummy;
  // эти данные могут измениться
  image_t texture;
  color_t color;
  uint borders_data;
  uint biome_index;
};

struct packed_biome_data_t {
  uvec4 uint_data;
  vec4 scale_data;
  vec4 float_data;
  vec4 float_data2;
};

const uint map_biome_objects_count = 3;

// один из object_texture наверное не потребуется
struct biome_data_t {
  image_t texture;
  color_t color;
  // слишком малые размеры, лучше 3 текстурки замиксовать
  image_t object_textures[map_biome_objects_count];
  vec2 scales[map_biome_objects_count]; // mix по этим двум числам
  float probabilities[map_biome_objects_count];
  // "плотность" объектов на тайле, ниже единицы - каждые несколько тайлов объект, выше единицы - среднее количество объектов на тайле
  // нужно ли делать минимум максимум плотности? не уверен что это полезно
  float density;
  float dummy;
};

struct world_structure_t {
  image_t city_image_top;
  image_t city_image_face;
  float scale;
  uint heraldy_layer_index; // здесь мы наверное будем хранить индекс на первый слой геральдики
  // что то еще? где то это должно находится? по идее по центру тайла
  // размер строения, возможно нужно менять изображения по ходу дела
  // как нужно подстроить лес для объекта? мне тогда придется понять на каком тайле что стоит
  // а это и так переусложнение и без того тяжелого шейдера
};

struct biome_objects_data_t {
  uvec4 objects_indirect;
  uvec4 objects_data;
};

struct additional_data_t {
  uvec4 data[2];
};

struct army_data_t {
  vec4 data;
};

INLINE bool is_image_valid(const image_t img) {
  return img.container != GPU_UINT_MAX;
}

INLINE uint get_image_index(const image_t img) {
  const uint mask = 0xffff;
  return (img.container >> 16) & mask;
}

INLINE uint get_image_layer(const image_t img) {
  const uint mask = 0xff;
  return (img.container >> 8) & mask;
}

INLINE uint get_image_sampler(const image_t img) {
  const uint mask = 0x3f;
  return (img.container >> 0) & mask;
}

INLINE bool flip_u(const image_t img) {
  const uint mask = 0x1;
  return bool((img.container >> 7) & mask);
}

INLINE bool flip_v(const image_t img) {
  const uint mask = 0x1;
  return bool((img.container >> 6) & mask);
}

INLINE image_t make_image(const uint index, const uint layer, const uint sampler_id, const bool flip_u, const bool flip_v) {
  image_t img;
  img.container = (uint(index) << 16) | (uint(layer) << 8) | uint(sampler_id) | (uint(flip_u) << 7) | (uint(flip_v) << 6);
  return img;
}

INLINE float get_color_r(const color_t col) {
  const float div = 255.0f;
  const uint mask = 0xff;
  const uint comp = (col.container >> 24) & mask;
  return float(comp) / div;
}

INLINE float get_color_g(const color_t col) {
  const float div = 255.0f;
  const uint mask = 0xff;
  const uint comp = (col.container >> 16) & mask;
  return float(comp) / div;
}

INLINE float get_color_b(const color_t col) {
  const float div = 255.0f;
  const uint mask = 0xff;
  const uint comp = (col.container >> 8) & mask;
  return float(comp) / div;
}

INLINE float get_color_a(const color_t col) {
  const float div = 255.0f;
  const uint mask = 0xff;
  const uint comp = (col.container >> 0) & mask;
  return float(comp) / div;
}

INLINE vec4 get_color(const color_t col) {
  return vec4(
    get_color_r(col),
    get_color_g(col),
    get_color_b(col),
    get_color_a(col)
  );
}

INLINE color_t make_color1(const float r, const float g, const float b, const float a) {
  const uint ur = uint(255.0f * r);
  const uint ug = uint(255.0f * g);
  const uint ub = uint(255.0f * b);
  const uint ua = uint(255.0f * a);
  color_t c;
  c.container = (ur << 24) | (ug << 16) | (ub << 8) | (ua << 0);
  return c;
}

INLINE map_tile_t unpack_data(const light_map_tile_t data) {
  map_tile_t tile;
  tile.points[0] = data.packed_data[0][0];
  tile.points[1] = data.packed_data[0][1];
  tile.points[2] = data.packed_data[0][2];
  tile.points[3] = data.packed_data[0][3];
  
  tile.points[4] = data.packed_data[1][0];
  tile.points[5] = data.packed_data[1][1];
  tile.neighbors[0] = data.packed_data[1][2];
  tile.neighbors[1] = data.packed_data[1][3];
  
  tile.neighbors[2] = data.packed_data[2][0];
  tile.neighbors[3] = data.packed_data[2][1];
  tile.neighbors[4] = data.packed_data[2][2];
  tile.neighbors[5] = data.packed_data[2][3];
  
  tile.center = data.packed_data[3][0];
  tile.height = uintBitsToFloat(data.packed_data[3][1]);
  tile.connections_data = data.packed_data[3][2];
  tile.dummy = data.packed_data[3][3];
  
  tile.texture.container = data.packed_data[4][0];
  tile.color.container = data.packed_data[4][1];
  tile.borders_data = data.packed_data[4][2];
  tile.biome_index = data.packed_data[4][3];
  return tile;
}

INLINE biome_data_t unpack_data(const packed_biome_data_t data) {
  biome_data_t biome;
  biome.texture.container = data.uint_data[0];
  biome.color.container = data.uint_data[1];
  biome.object_textures[0].container = data.uint_data[2];
  biome.object_textures[1].container = data.uint_data[3];

  biome.object_textures[2].container = floatBitsToUint(data.scale_data[0]);
  biome.scales[0].x = data.scale_data[1];
  biome.scales[0].y = data.scale_data[2];
  biome.scales[1].x = data.scale_data[3];

  biome.scales[1].y = data.float_data[0];
  biome.scales[2].x = data.float_data[1];
  biome.scales[2].y = data.float_data[2];
  biome.probabilities[0] = data.float_data[3];

  biome.probabilities[1] = data.float_data2[0];
  biome.probabilities[2] = data.float_data2[1];
  biome.density = data.float_data2[2];
  biome.dummy = data.float_data2[3];
  return biome;
}

INLINE light_map_tile_t pack_data(const map_tile_t tile) {
  light_map_tile_t packed_data;
  packed_data.packed_data[0][0] = tile.points[0];
  packed_data.packed_data[0][1] = tile.points[1];
  packed_data.packed_data[0][2] = tile.points[2];
  packed_data.packed_data[0][3] = tile.points[3];
  
  packed_data.packed_data[1][0] = tile.points[4];
  packed_data.packed_data[1][1] = tile.points[5];
  packed_data.packed_data[1][2] = tile.neighbors[0];
  packed_data.packed_data[1][3] = tile.neighbors[1];
  
  packed_data.packed_data[2][0] = tile.neighbors[2];
  packed_data.packed_data[2][1] = tile.neighbors[3];
  packed_data.packed_data[2][2] = tile.neighbors[4];
  packed_data.packed_data[2][3] = tile.neighbors[5];
  
  packed_data.packed_data[3][0] = tile.center;
  packed_data.packed_data[3][1] = floatBitsToUint(tile.height);
  packed_data.packed_data[3][2] = tile.connections_data;
  packed_data.packed_data[3][3] = tile.dummy;
  
  packed_data.packed_data[4][0] = tile.texture.container;
  packed_data.packed_data[4][1] = tile.color.container;
  packed_data.packed_data[4][2] = tile.borders_data;
  packed_data.packed_data[4][3] = tile.biome_index;
  return packed_data;
}

INLINE packed_biome_data_t pack_data(const biome_data_t biome) {
  packed_biome_data_t packed_data;
  packed_data.uint_data[0] = biome.texture.container;
  packed_data.uint_data[1] = biome.color.container;
  packed_data.uint_data[2] = biome.object_textures[0].container;
  packed_data.uint_data[3] = biome.object_textures[1].container;
  packed_data.scale_data[0] = uintBitsToFloat(biome.object_textures[2].container);
  packed_data.scale_data[1] = biome.scales[0].x;
  packed_data.scale_data[2] = biome.scales[0].y;
  packed_data.scale_data[3] = biome.scales[1].x;
  packed_data.float_data[0] = biome.scales[1].y;
  packed_data.float_data[1] = biome.scales[2].x;
  packed_data.float_data[2] = biome.scales[2].y;
  packed_data.float_data[3] = biome.probabilities[0];
  packed_data.float_data2[0] = biome.probabilities[1];
  packed_data.float_data2[1] = biome.probabilities[2];
  packed_data.float_data2[2] = biome.density;
  packed_data.float_data2[3] = biome.dummy;
  return packed_data;
}

INLINE bool is_pentagon(const light_map_tile_t tile) {
  return tile.packed_data[2][3] == GPU_UINT_MAX;
}

INLINE bool is_pentagon(const map_tile_t tile) {
  return tile.neighbors[5] == GPU_UINT_MAX;
}

INLINE uint compute_height_layer(const float height) {
  return height < 0.0f ? 0 : uint(height / layer_height);
}

#ifdef __cplusplus
    INLINE image_t create_image(const uint16_t &index, const uint8_t &layer, const uint8_t &sampler, const bool flip_u, const bool flip_v) {
      ASSERT(sampler < 0x3f);
      return {(uint(index) << 16) | (uint(layer) << 8) | uint(sampler) | (uint(flip_u) << 7) | (uint(flip_v) << 6)};
    }

    INLINE image_t create_image(const uint16_t &index, const uint8_t &layer, const uint8_t &sampler) {
      ASSERT(sampler < 0x3f);
      return {(uint(index) << 16) | (uint(layer) << 8) | uint(sampler)};
    }

    INLINE image_t create_image(const uint16_t &index, const uint8_t &layer) {
      return {(uint(index) << 16) | (uint(layer) << 8)};
    }

    INLINE color_t make_color(const uint16_t &r, const uint16_t &g, const uint16_t &b, const uint16_t &a) {
      ASSERT(r <= UINT8_MAX);
      ASSERT(g <= UINT8_MAX);
      ASSERT(b <= UINT8_MAX);
      ASSERT(a <= UINT8_MAX);
      color_t c;
      c.container = (r << 24) | (g << 16) | (b << 8) | (a << 0);
      return c;
    }

    INLINE color_t make_color(const float &r, const float &g, const float &b, const float &a) {
      ASSERT(r >= 0.0f && r <= 1.0f);
      ASSERT(g >= 0.0f && g <= 1.0f);
      ASSERT(b >= 0.0f && b <= 1.0f);
      ASSERT(a >= 0.0f && a <= 1.0f);
      const uint8_t final_r = uint8_t(UINT8_MAX * r);
      const uint8_t final_g = uint8_t(UINT8_MAX * g);
      const uint8_t final_b = uint8_t(UINT8_MAX * b);
      const uint8_t final_a = uint8_t(UINT8_MAX * a);

      return make_color(uint16_t(final_r), uint16_t(final_g), uint16_t(final_b), uint16_t(final_a));
    }

    static_assert(sizeof(light_map_tile_t) % 16 == 0);
    static_assert(sizeof(light_map_tile_t) == sizeof(map_tile_t));
    static_assert(alignof(light_map_tile_t) == alignof(map_tile_t));
    static_assert(sizeof(biome_data_t) % 16 == 0);
    static_assert(sizeof(packed_biome_data_t) % 16 == 0);
    static_assert(sizeof(biome_data_t) == sizeof(packed_biome_data_t));
    static_assert(sizeof(camera_data) % 16 == 0);
  }
}
#endif

#endif
