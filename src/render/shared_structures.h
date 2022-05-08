#ifndef DEVILS_ENGINE_RENDER_SHARED_STRUCTURES_H
#define DEVILS_ENGINE_RENDER_SHARED_STRUCTURES_H

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
    using glm::unpackUnorm4x8;
    using glm::packUnorm4x8;

    using uint = uint32_t;
    // using mat4 = basic_mat4;
    // using vec4 = basic_vec4;
    using mat4 = glm::mat4;
    using vec4 = glm::vec4;
    using vec2 = glm::vec2;
    using uvec2 = glm::uvec2;
    using vec3 = glm::vec3;
    using uvec4 = glm::uvec4;
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
#define IMAGE_TYPE_LAYERED_IMAGE 3
#define IMAGE_TYPE_ALPHA_STENCIL 4
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
  
#ifdef __cplusplus
  inline image_t() : container(GPU_UINT_MAX) {}
  inline image_t(const uint container) : container(container) {}
#endif
};

struct color_t {
  uint container;
  
#ifdef __cplusplus
  inline color_t() : container(0) {}
  inline color_t(const uint container) : container(container) {}
#endif
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
  uvec4 dim;   // x - width, y - heigth, z - zoom, w - time
  uvec4 state; // x - persistent state, y - application state, z - turn state
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
  return image_t((uint(index) << 16) | (uint(layer) << 8) | (uint(flip_u) << 7) | (uint(flip_v) << 6) | uint(sampler_id));
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
//   return vec4(
//     get_color_r(col),
//     get_color_g(col),
//     get_color_b(col),
//     get_color_a(col)
//   );
  return unpackUnorm4x8(col.container);
}

INLINE color_t make_color(const float r, const float g, const float b, const float a) {
//   const uint ur = uint(255.0f * r);
//   const uint ug = uint(255.0f * g);
//   const uint ub = uint(255.0f * b);
//   const uint ua = uint(255.0f * a);
//   color_t c;
//   c.container = (ur << 24) | (ug << 16) | (ub << 8) | (ua << 0);
//   return c;
  return color_t(packUnorm4x8(vec4(r, g, b, a)));
}

INLINE color_t make_color(const vec4 col) {
  return color_t(packUnorm4x8(col));
}

INLINE uint compute_height_layer(const float height) {
  return height < 0.0f ? 0 : uint(height / layer_height);
}

#ifdef __cplusplus
    INLINE image_t create_image(const uint16_t &index, const uint8_t &layer, const uint8_t &sampler, const bool flip_u, const bool flip_v) {
      ASSERT(sampler < 0x3f);
      return {(uint(index) << 16) | (uint(layer) << 8) | (uint(flip_u) << 7) | (uint(flip_v) << 6) | uint(sampler)};
    }

    INLINE image_t create_image(const uint16_t &index, const uint8_t &layer, const uint8_t &sampler) {
      ASSERT(sampler < 0x3f);
      return {(uint(index) << 16) | (uint(layer) << 8) | uint(sampler)};
    }

    INLINE image_t create_image(const uint16_t &index, const uint8_t &layer) {
      return {(uint(index) << 16) | (uint(layer) << 8)};
    }

//     INLINE color_t make_color(const uint16_t &r, const uint16_t &g, const uint16_t &b, const uint16_t &a) {
//       ASSERT(r <= UINT8_MAX);
//       ASSERT(g <= UINT8_MAX);
//       ASSERT(b <= UINT8_MAX);
//       ASSERT(a <= UINT8_MAX);
//       color_t c;
//       c.container = (r << 24) | (g << 16) | (b << 8) | (a << 0);
//       return c;
//     }

//     INLINE color_t make_color(const float &r, const float &g, const float &b, const float &a) {
// //       ASSERT(r >= 0.0f && r <= 1.0f);
// //       ASSERT(g >= 0.0f && g <= 1.0f);
// //       ASSERT(b >= 0.0f && b <= 1.0f);
// //       ASSERT(a >= 0.0f && a <= 1.0f);
// //       const uint8_t final_r = uint8_t(UINT8_MAX * r);
// //       const uint8_t final_g = uint8_t(UINT8_MAX * g);
// //       const uint8_t final_b = uint8_t(UINT8_MAX * b);
// //       const uint8_t final_a = uint8_t(UINT8_MAX * a);
// // 
// //       return make_color(uint16_t(final_r), uint16_t(final_g), uint16_t(final_b), uint16_t(final_a));
//       return color_t(packUnorm4x8(vec4(r, g, b, a)));
//     }

    static_assert(sizeof(camera_data) % 16 == 0);
  }
}
#endif

#endif
