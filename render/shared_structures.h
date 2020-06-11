#ifndef SHARED_STRUCTURES_H
#define SHARED_STRUCTURES_H

#ifdef __cplusplus
#include <cstdint>
//#include "basic_tri.h"
#include "utils/utility.h"

#define DEFAULT_DESCRIPTOR_POOL_NAME "default_descriptor_pool"
#define UNIFORM_BUFFER_LAYOUT_NAME "uniform_layout"
#define MATRICES_BUFFER_LAYOUT_NAME "matrixes_layout"
#define STORAGE_BUFFER_LAYOUT_NAME "storage_layout"
#define SAMPLED_IMAGE_LAYOUT_NAME "sampled_image_layout"
#define SKYBOX_TEXTURE_LAYOUT_NAME "skybox_texture_layout"
#define TILES_DATA_LAYOUT_NAME "tiles_data_layout"

#define INLINE inline
#define INOUT

namespace devils_engine {
  namespace render {
    using glm::floatBitsToUint;
    using glm::uintBitsToFloat;

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
#define INOUT inout

#endif

#define GPU_UINT_MAX 0xffffffff
    
const uint biome_ocean            = 0;
const uint biome_ocean_glacier    = 1;
const uint biome_desert           = 2;
const uint biome_rain_forest      = 3;
const uint biome_rocky            = 4;
const uint biome_plains           = 5;
const uint biome_swamp            = 6;
const uint biome_grassland        = 7;
const uint biome_deciduous_forest = 8;
const uint biome_tundra           = 9;
const uint biome_land_glacier     = 10;
const uint biome_conifer_forest   = 11;
const uint biome_mountain         = 12;
const uint biome_snowy_mountain   = 13;

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
  // uint width;
  // uint height;
  uvec4 dim;
};

struct matrices_data {
  mat4 proj;
  mat4 view;
  mat4 invProj;
  mat4 invView;
  mat4 invViewProj;
};

struct matrices {
  mat4 persp;
  mat4 ortho;
  mat4 view;
  matrices_data matrixes;
  camera_data camera;
};

struct gpu_particle {
  vec4 pos;
  vec4 vel;
  uvec4 int_data;
  vec4 float_data;
  vec4 speed_color;
};

struct frustum_t {
  vec4 planes[6];
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
  uvec4 tile_indices;
  uvec4 packed_data1;
  uvec4 packed_data2;
  uvec4 packed_data3;
};

struct map_tile_t {
  uint center;
  uint biom_index;
  uint unique_object_index; // шахты или еще какие постройки, но скорее всего это можно будет вынести в данные провинции
  float height; // мне бы еще подъем какой
  uint points[6];
  uint neighbours[6];
  
  // подъем означает что мне нужно генерить закраску какую для этих областей
  // что хорошо - закраску можно сгенерировать в screen space
  // там же мы должны сгенерить данные для отрисовки деревьев
};

struct packed_biom_data_t {
  uvec4 uint_data;
  vec4 float_data;
};

struct biom_data_t {
  image_t img;
  image_t obj;
  
  float density;
  float width; // лучше бы чтобы для таких объектов размеры были одинаковыми
  float height; // хотя может и нет
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

INLINE image_t get_particle_image(const gpu_particle particle) {
  image_t img;
  img.container = particle.int_data.y;
  return img;
}

INLINE color_t get_particle_color(const gpu_particle particle) {
  color_t col;
  col.container = floatBitsToUint(particle.speed_color[2]);
  return col;
}

INLINE uint get_particle_life_time(const gpu_particle particle) {
  const uint mask = 0x7fffffff;
  return (particle.int_data.z & mask);
}

INLINE bool particle_is_on_ground(const gpu_particle particle) {
  const uint mask = 0x7fffffff;
  return bool(particle.int_data.z & ~mask);
}

INLINE uint get_particle_current_time(const gpu_particle particle) {
  return particle.int_data.w;
}

INLINE float get_particle_max_speed(const gpu_particle particle) {
  return particle.speed_color[0];
}

INLINE float get_particle_min_speed(const gpu_particle particle) {
  return particle.speed_color[1];
}

INLINE float get_particle_friction(const gpu_particle particle) {
  return particle.speed_color[3];
}

INLINE float get_particle_max_scale(const gpu_particle particle) {
  return particle.float_data[0];
}

INLINE float get_particle_min_scale(const gpu_particle particle) {
  return particle.float_data[1];
}

INLINE float get_particle_gravity(const gpu_particle particle) {
  return particle.float_data[2];
}

INLINE float get_particle_bounce(const gpu_particle particle) {
  return particle.float_data[3];
}

INLINE void set_particle_pos(INOUT gpu_particle particle, const vec4 pos) {
  particle.pos = pos;
}

INLINE void set_particle_vel(INOUT gpu_particle particle, const vec4 vel) {
  particle.vel = vel;
}

INLINE void set_particle_current_time(INOUT gpu_particle particle, const uint time) {
  particle.int_data.w = time;
#ifdef __cplusplus
  (void)particle;
#endif
}

INLINE void set_particle_is_on_ground(INOUT gpu_particle particle, const bool value) {
  const uint mask = 0x7fffffff;
  particle.int_data.z = value ? particle.int_data.z | ~mask : particle.int_data.z & mask;
}

const uint min_speed_stop      = (1 << 0);
const uint min_speed_remove    = (1 << 1);
const uint max_speed_stop      = (1 << 2);
const uint max_speed_remove    = (1 << 3);
const uint speed_dec_over_time = (1 << 4);
const uint speed_inc_over_time = (1 << 5);
const uint scale_dec_over_time = (1 << 6);
const uint scale_inc_over_time = (1 << 7);
const uint scaling_along_vel   = (1 << 8);
const uint limit_max_speed     = (1 << 9);
const uint limit_min_speed     = (1 << 10);

INLINE bool particle_min_speed_stop(const gpu_particle particle) {
  return (particle.int_data.x & min_speed_stop) == min_speed_stop;
}

INLINE bool particle_min_speed_remove(const gpu_particle particle) {
  return (particle.int_data.x & min_speed_remove) == min_speed_remove;
}

INLINE bool particle_max_speed_stop(const gpu_particle particle) {
  return (particle.int_data.x & max_speed_stop) == max_speed_stop;
}

INLINE bool particle_max_speed_remove(const gpu_particle particle) {
  return (particle.int_data.x & max_speed_remove) == max_speed_remove;
}

INLINE bool particle_speed_dec_over_time(const gpu_particle particle) {
  return (particle.int_data.x & speed_dec_over_time) == speed_dec_over_time;
}

INLINE bool particle_speed_inc_over_time(const gpu_particle particle) {
  return (particle.int_data.x & speed_inc_over_time) == speed_inc_over_time;
}

INLINE bool particle_scale_dec_over_time(const gpu_particle particle) {
  return (particle.int_data.x & scale_dec_over_time) == scale_dec_over_time;
}

INLINE bool particle_scale_inc_over_time(const gpu_particle particle) {
  return (particle.int_data.x & scale_inc_over_time) == scale_inc_over_time;
}

INLINE bool particle_scaling_along_vel(const gpu_particle particle) {
  return (particle.int_data.x & scaling_along_vel) == scaling_along_vel;
}

INLINE bool particle_limit_max_speed(const gpu_particle particle) {
  return (particle.int_data.x & limit_max_speed) == limit_max_speed;
}

INLINE bool particle_limit_min_speed(const gpu_particle particle) {
  return (particle.int_data.x & limit_min_speed) == limit_min_speed;
}

INLINE map_tile_t unpack_data(const light_map_tile_t data) {
  map_tile_t tile;
  tile.center = data.tile_indices.x;
  tile.biom_index = data.tile_indices.y;
  tile.unique_object_index = data.tile_indices.z;
  tile.height = uintBitsToFloat(data.tile_indices.w);
  tile.points[0] = data.packed_data1[0];
  tile.points[1] = data.packed_data1[1];
  tile.points[2] = data.packed_data1[2];
  tile.points[3] = data.packed_data1[3];
  tile.points[4] = data.packed_data2[0];
  tile.points[5] = data.packed_data2[1];
  tile.neighbours[0] = data.packed_data2[2];
  tile.neighbours[1] = data.packed_data2[3];
  tile.neighbours[2] = data.packed_data3[0];
  tile.neighbours[3] = data.packed_data3[1];
  tile.neighbours[4] = data.packed_data3[2];
  tile.neighbours[5] = data.packed_data3[3];
  return tile;
}

INLINE biom_data_t unpack_data(const packed_biom_data_t data) {
  biom_data_t biom;
  biom.img.container = data.uint_data.x;
  biom.obj.container = data.uint_data.y;
  biom.density = data.float_data.x;
  biom.width = data.float_data.y;
  biom.height = data.float_data.z;
  return biom;
}

INLINE light_map_tile_t pack_data(const map_tile_t tile) {
  light_map_tile_t packed_data;
  packed_data.tile_indices.x = tile.center;
  packed_data.tile_indices.y = tile.biom_index;
  packed_data.tile_indices.z = tile.unique_object_index;
  packed_data.tile_indices.w = floatBitsToUint(tile.height);
  packed_data.packed_data1[0] = tile.points[0];
  packed_data.packed_data1[1] = tile.points[1];
  packed_data.packed_data1[2] = tile.points[2];
  packed_data.packed_data1[3] = tile.points[3];
  packed_data.packed_data2[0] = tile.points[4];
  packed_data.packed_data2[1] = tile.points[5];
  packed_data.packed_data2[2] = tile.neighbours[0];
  packed_data.packed_data2[3] = tile.neighbours[1];
  packed_data.packed_data3[0] = tile.neighbours[2];
  packed_data.packed_data3[1] = tile.neighbours[3];
  packed_data.packed_data3[2] = tile.neighbours[4];
  packed_data.packed_data3[3] = tile.neighbours[5];
  return packed_data;
}

INLINE packed_biom_data_t pack_data(const biom_data_t biom) {
  packed_biom_data_t packed_data;
  packed_data.uint_data.x = biom.img.container;
  packed_data.uint_data.y = biom.obj.container;
  packed_data.float_data.x = biom.density;
  packed_data.float_data.y = biom.width;
  packed_data.float_data.z = biom.height;
  return packed_data;
}

INLINE bool is_pentagon(const light_map_tile_t tile) {
  return tile.packed_data3[3] == GPU_UINT_MAX;
}

INLINE bool is_pentagon(const map_tile_t tile) {
  return tile.neighbours[5] == GPU_UINT_MAX;
}

const uint modulus = 1 << 31;
const uint multiplier = 1103515245;
const uint increment = 12345;

INLINE uint lcg(const uint prev) {
  return (prev * multiplier + increment) % modulus;
}

INLINE float lcg_normalize(const uint val) {
  return float(val) / float(modulus);
}

// bool lcg_probability(const float val) {
//   const uint generated = lcg(floatBitsToUint(val));
//   const float prob = lcg_normalize(generated);
//   return val <= prob;
// }

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
    
    static_assert(sizeof(light_map_tile_t) % 16 == 0);
    static_assert(sizeof(packed_biom_data_t) % 16 == 0);
    static_assert(sizeof(camera_data) % 16 == 0);
  }
}
#endif

#endif
