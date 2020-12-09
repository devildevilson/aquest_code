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
#define INOUT

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
#define INOUT inout

#endif

#define GPU_UINT_MAX 0xffffffff
#define PACKED_INDEX_COEF 4
#define PACKED_TILE_INDEX_COEF 6
#define MAX_BIOMES_COUNT 0xff
#define WORLD_RADIUS_CONSTANT 500.0f
#define OUTSIDE_FRUSTUM   0
#define INTERSECT_FRUSTUM 1
#define INSIDE_FRUSTUM    2
    
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
  uvec4 dim;
  vec4 cursor_dir;
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
  // чем заполнить два последних инта?
  uvec4 packed_data4;
};

struct map_tile_t {
  uint center;
  image_t texture; 
  color_t color;
  float height; // мне бы еще подъем какой
  uint points[6];
  uint neighbours[6];
  uint borders_data;
  uint connections_data;
  uint biome_index; 
  // все таки вернулся сюда биом индекс
  // рядом с ним мы можем поставить индекс структуры
  // нам еще потребуется решить вопрос с дорогами
  // нам нужно только указать в какие стороны направлена дорога (6 бит)
  // тип дороги? довольно полезно, как указать? 
  // еще было бы неплохо сделать подъем дороги (наверное)
  
  // подъем означает что мне нужно генерить закраску какую для этих областей
  // что хорошо - закраску можно сгенерировать в screen space
  // там же мы должны сгенерить данные для отрисовки деревьев
  // закраску сгенерировал, но в буфере (вообще нужно посмотреть сколько гпу памяти занимают все это данные)
  // нужно добавить еще 2 переменные: оффсет для границ и оффсет для соединений
};

// формула безье (2 порядок): vec = (1-t)*(1-t)*vec0+2*(1-t)*t*vec1+t*t*vec2, где
// vec0 - начало линии, vec1 - контрольная точка, vec2 - конец линии,
// t - переменная от 0 до 1 обозначающая участок линии безье
// нужно выбрать подходящую степень разбиения и нарисовать кривую

struct packed_biome_data_t {
  uvec4 uint_data;
  vec4 scale_data;
  vec4 float_data;
};

// один из object_texture наверное не потребуется
struct biome_data_t {
  image_t texture;
  color_t color;
  // кажется что объекты биома (например лес) будут состоять из двух текстурок
  // первая - это та часть которая будет ровно стоять на тайле, но при этом повернута на игрока
  // вторая - это та часть которая будет ображена к игроку полностью (билборд)
  image_t object_texture1;
  image_t object_texture2;
  float min_scale1; // mix по этим двум числам 
  float max_scale1;
  float min_scale2;
  float max_scale2;
  // "плотность" объектов на тайле, ниже нуля - каждые несколько тайлов объект, выше нуля - среднее количество объектов на тайле
  // нужно ли делать минимум максимум плотности? не уверен что это полезно
  float density;
  float dummy1; // что тут?
  float dummy2;
  float dummy3;
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

struct aabb_t {
  vec4 min;
  vec4 max;
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

INLINE color_t make_color1(const float r, const float g, const float b, const float a) {
  const uint ur = uint(255.0f * r);
  const uint ug = uint(255.0f * g);
  const uint ub = uint(255.0f * b);
  const uint ua = uint(255.0f * a);
  color_t c;
  c.container = (ur << 24) | (ug << 16) | (ub << 8) | (ua << 0);
  return c;
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
//   tile.biom_index = data.tile_indices.y;
//   tile.unique_object_index = data.tile_indices.z;
  tile.texture.container = data.tile_indices.y;
  tile.color.container = data.tile_indices.z;
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
  tile.borders_data = data.packed_data4[0];
  tile.connections_data = data.packed_data4[1];
  tile.biome_index = data.packed_data4[2];
  return tile;
}

INLINE biome_data_t unpack_data(const packed_biome_data_t data) {
  biome_data_t biome;
  biome.texture.container = data.uint_data.x;
  biome.color.container = data.uint_data.y;
  biome.object_texture1.container = data.uint_data.z;
  biome.object_texture2.container = data.uint_data.w;
  
  biome.min_scale1 = data.scale_data.x;
  biome.max_scale1 = data.scale_data.y;
  biome.min_scale2 = data.scale_data.z;
  biome.max_scale2 = data.scale_data.w;
  
  biome.density = data.float_data.x;
  biome.dummy1 = data.float_data.y;
  biome.dummy2 = data.float_data.z;
  biome.dummy3 = data.float_data.w;
  return biome;
}

INLINE light_map_tile_t pack_data(const map_tile_t tile) {
  light_map_tile_t packed_data;
  packed_data.tile_indices.x = tile.center;
//   packed_data.tile_indices.y = tile.biom_index;
//   packed_data.tile_indices.z = tile.unique_object_index;
  packed_data.tile_indices.y = tile.texture.container;
  packed_data.tile_indices.z = tile.color.container;
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
  packed_data.packed_data4[0] = tile.borders_data;
  packed_data.packed_data4[1] = tile.connections_data;
  packed_data.packed_data4[2] = tile.biome_index;
  return packed_data;
}

INLINE packed_biome_data_t pack_data(const biome_data_t biome) {
  packed_biome_data_t packed_data;
  packed_data.uint_data.x = biome.texture.container;
  packed_data.uint_data.y = biome.color.container;
  packed_data.uint_data.z = biome.object_texture1.container;
  packed_data.uint_data.w = biome.object_texture2.container;
  packed_data.scale_data.x = biome.min_scale1;
  packed_data.scale_data.y = biome.max_scale1;
  packed_data.scale_data.z = biome.min_scale2;
  packed_data.scale_data.w = biome.max_scale2;
  packed_data.float_data.x = biome.density;
  packed_data.float_data.y = biome.dummy1;
  packed_data.float_data.z = biome.dummy2;
  packed_data.float_data.w = biome.dummy3;
  return packed_data;
}

INLINE bool is_pentagon(const light_map_tile_t tile) {
  return tile.packed_data3[3] == GPU_UINT_MAX;
}

INLINE bool is_pentagon(const map_tile_t tile) {
  return tile.neighbours[5] == GPU_UINT_MAX;
}

INLINE uint compute_height_layer(const float height) {
  return height < 0.0f ? 0 : uint(height / layer_height);
}

// const uint modulus = uint(1) << 31;
// const uint multiplier = 1103515245;
// const uint increment = 12345;
// 
// INLINE uint lcg(const uint prev) {
//   return (prev * multiplier + increment) % modulus;
// }
// 
// INLINE float lcg_normalize(const uint val) {
//   return float(val) / float(modulus);
// }

INLINE uint prng(const uint prev) {
//   prev ^= prev << 13;
//   prev ^= prev >> 17;
//   prev ^= prev << 5;
//   return prev;
  
  uint z = prev + 0x6D2B79F5;
  z = (z ^ z >> 15) * (1 | z);
  z ^= z + (z ^ z >> 7) * (61 | z);
  return z ^ z >> 14;
}

INLINE float prng_normalize(const uint state) {
  // float32 - 8 бит экспонента и 23 мантисса
  const uint float_mask = 0x7f << 23;
  const uint float_val = float_mask | state >> 9;
  return uintBitsToFloat(float_val) - 1.0f;
}

INLINE int frustum_test(const frustum_t frustum, const vec4 center, const vec4 extent) {
  int result = INSIDE_FRUSTUM;
  for (uint i = 0; i < 6; ++i) {
    const vec4 plane = vec4(frustum.planes[i][0], frustum.planes[i][1], frustum.planes[i][2], 0.0f);
    const float p3 = frustum.planes[i][3];
    
    const float d = dot(center,     plane);
    const float r = dot(extent, abs(plane));
    const float d_p_r = d + r;
    const float d_m_r = d - r;
    
    result = min(result, d_p_r < -p3 ? OUTSIDE_FRUSTUM   : result);
    result = min(result, d_m_r < -p3 ? INTERSECT_FRUSTUM : result);
  }
  
  return result;
}

INLINE bool frustum_test(const frustum_t frustum, const vec4 center, const float radius) {
  bool result = true;
  for (uint i = 0; i < 6; ++i) {
    const float d = dot(center, frustum.planes[i]);
    const bool res = !(d <= -radius);
    result = bool(min(uint(result), uint(res)));
  }
  
  return result;
}

INLINE bool intersect(const aabb_t box1, const aabb_t box2) {
  return 
    (box1.min.x <= box2.max.x && box1.max.x >= box2.min.x) && 
    (box1.min.y <= box2.max.y && box1.max.y >= box2.min.y) && 
    (box1.min.z <= box2.max.z && box1.max.z >= box2.min.z);
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
    static_assert(sizeof(packed_biome_data_t) % 16 == 0);
    static_assert(sizeof(camera_data) % 16 == 0);
  }
}
#endif

#endif
