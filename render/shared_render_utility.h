#ifndef DEVILS_ENGINE_SHARED_RENDER_UTILITY_H
#define DEVILS_ENGINE_SHARED_RENDER_UTILITY_H

#include "shared_structures.h"

#ifdef __cplusplus

#include "utils/utility.h"

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
    using glm::mix;
    using glm::cross;
    using glm::clamp;

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

#include "../utils/shared_mathematical_constant.h"

#endif

#define OUTSIDE_FRUSTUM   0
#define INTERSECT_FRUSTUM 1
#define INSIDE_FRUSTUM    2

struct frustum_t {
  vec4 planes[6];
};

struct aabb_t {
  vec4 min;
  vec4 max;
};

INLINE uint prng(const uint prev) {
  uint z = prev + 0x6D2B79F5;
  z = (z ^ z >> 15) * (1 | z);
  z ^= z + (z ^ z >> 7) * (61 | z);
  return z ^ z >> 14;
}

INLINE uint rotl(const uint x, int k) {
  return (x << k) | (x >> (32 - k));
}

INLINE uint prng2(const uint s0, const uint s1) { // по идее должно давать хорошие результаты для чисел не 0
  const uint s1_tmp = s1 ^ s0;
  const uint new_s0 = rotl(s0, 26) ^ s1_tmp ^ (s1_tmp << 9);
  //const uint new_s1 = rotl(s1_tmp, 13);
  return rotl(new_s0 * 0x9E3779BB, 5) * 5;
}

INLINE float prng_normalize(const uint state) {
  // float32 - 1 бит знак, 8 бит экспонента и 23 мантисса
  const uint float_mask = 0x7f << 23;
  const uint float_val = float_mask | (state >> 9); // зачем двигать? первые несколько бит обладают плохой энтропией
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
    const bool res = !(d <= -(radius * radius));
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

// по идее это должно чудесно работать и в цпу
INLINE bool ray_triangle_test(const vec4 ray_pos, const vec4 ray_dir, const vec4 point1, const vec4 point2, const vec4 point3, INOUT(float) dist) {
  const vec4 e1 = point2 - point1;
  const vec4 e2 = point3 - point1;
  // Вычисление вектора нормали к плоскости
  const vec4 pvec = vec4(cross(vec3(ray_dir), vec3(e2)), 0.0f);
  const float det = dot(e1, pvec);

  // Луч параллелен плоскости
  const bool not_parallel_ray = !(abs(det) < EPSILON);

  const float inv_det = 1.0f / det;
  const vec4 tvec = ray_pos - point1;
  const float u = mix(10000.0f, dot(tvec, pvec) * inv_det, float(not_parallel_ray));
  const bool valid_u = u >= 0.0f && u <= 1.0f;

  const vec4 qvec = vec4(cross(vec3(tvec), vec3(e1)), 0.0f);
  const float v = mix(10000.0f, dot(ray_dir, qvec) * inv_det, float(not_parallel_ray));
  const bool valid_v = v >= 0.0f && u + v <= 1.0f;

  const bool ret = not_parallel_ray && valid_u && valid_v;

  dist = mix(10000.0f, dot(e2, qvec) * inv_det, float(ret));
  return ret;
}

INLINE bool test_ray_aabb(const vec4 ray_pos, const vec4 ray_dir, const vec4 box_center, const vec4 box_extents, INOUT(float) dist) {
  const vec4 box_max = box_center + box_extents;
  const vec4 box_min = box_center - box_extents;

  const vec4 ray_inv_dir = 1.0f / ray_dir;
  float t1 = (box_min[0] - ray_pos[0])*ray_inv_dir[0];
  float t2 = (box_max[0] - ray_pos[0])*ray_inv_dir[0];

  float tmin = min(t1, t2);
  float tmax = max(t1, t2);

  for (int i = 1; i < 3; ++i) {
    t1 = (box_min[i] - ray_pos[i])*ray_inv_dir[i];
    t2 = (box_max[i] - ray_pos[i])*ray_inv_dir[i];

    tmin = max(tmin, min(min(t1, t2), tmax));
    tmax = min(tmax, max(max(t1, t2), tmin));
  }

  const bool ret = tmax > max(tmin, 0.0f);
  const float final_dist = tmin > 0.0f ? tmin : tmax;
  dist = ret ? final_dist : dist;
  //dist = ret ? tmax : dist;
  return ret;
  // if (tmax > glm::max(tmin, 0.0f)) {
  //   dist = tmin > 0.0f ? tmin : tmax;
  //   return true;
  // }
  //
  // return false;
}

INLINE vec4 get_heraldy_pos(const vec4 tile_center, const float tile_height, const float zoom, const mat4 matrix) {
  const vec4 cam_right =  vec4(matrix[0][0], matrix[1][0], matrix[2][0], 0.0f);

  const float offset_k = mix(0.0f, 1.0f, clamp((zoom - 0.2f) * 3.0f, 0.0f, 1.0f));
  const vec4 normal = vec4(normalize(vec3(tile_center)), 0.0f);
  return tile_center + normal * tile_height * render_tile_height + cam_right * offset_k * 5.0f;
}

INLINE float point_to_line_dist(const vec4 point, const vec4 line_dir, const vec4 line_point) {
  const vec4 M = line_point - point;
  const vec3 S = cross(vec3(M), vec3(line_dir));
  const float s_l = length(S);
  //const float dir_l = length(line_dir); // если line_dir нормализованный, то это не нужно? скорее всего
  //return s_l / dir_l;
  return s_l;
}

// формула безье (2 порядок): vec = (1-t)*(1-t)*p1+2*(1-t)*t*p2+t*t*p3, где
// p1 - начало линии, p2 - контрольная точка, p3 - конец линии,
// t - переменная от 0 до 1 обозначающая участок линии безье
// нужно выбрать подходящую степень разбиения и нарисовать кривую
INLINE vec4 quadratic_bezier(const vec4 p1, const vec4 p2, const vec4 p3, const float t) {
  const float inv_t = 1.0f - t;
  return inv_t*inv_t*p1 + 2*inv_t*t*p2 + t*t*p3;
}

// две контрольных точки, 0 <= t <= 1
INLINE vec4 cubic_bezier(const vec4 p1, const vec4 p2, const vec4 p3, const vec4 p4, const float t) {
  const float inv_t = 1.0f - t;
  return inv_t*inv_t*inv_t*p1 + 3*inv_t*inv_t*t*p2 + 3*inv_t*t*t*p3 + t*t*t*p4;
}

#ifdef __cplusplus

  }
}

#endif

#endif
