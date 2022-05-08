#ifndef DEVILS_ENGINE_CORE_SHARED_STRUCTURES_H
#define DEVILS_ENGINE_CORE_SHARED_STRUCTURES_H

#include "../render/shared_structures.h"

#ifdef __cplusplus

//#include "basic_tri.h"
#include "utils/utility.h"

#define TILES_DATA_LAYOUT_NAME "tiles_data_layout"

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
  uint neighbors[6]; // это зачем?
  uint center;
  float height; // как можно поменять высоту?
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

#ifdef __cplusplus

    static_assert(sizeof(light_map_tile_t) % 16 == 0);
    static_assert(sizeof(light_map_tile_t) == sizeof(map_tile_t));
    static_assert(alignof(light_map_tile_t) == alignof(map_tile_t));
    static_assert(sizeof(biome_data_t) % 16 == 0);
    static_assert(sizeof(packed_biome_data_t) % 16 == 0);
    static_assert(sizeof(biome_data_t) == sizeof(packed_biome_data_t));
  }
}

#endif
    
#endif
