#ifndef SHARED_RENDER_UTILITY_H
#define SHARED_RENDER_UTILITY_H

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
  const uint float_val = float_mask | (state >> 9); // зачем двигать?
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

#ifdef __cplusplus

  }
}

#endif

#endif
