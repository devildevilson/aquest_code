#ifndef SHARED_PARTICLES_STRUCTURES_H
#define SHARED_PARTICLES_STRUCTURES_H

#include "shared_structures.h"

#ifdef __cplusplus
namespace devils_engine {
  namespace render {
    
#endif
    
struct gpu_particle_t {
  vec4 pos;
  vec4 vel;
  uvec4 int_data;
  vec4 float_data;
  vec4 speed_color;
};
    
INLINE image_t get_particle_image(const gpu_particle_t particle) {
  image_t img;
  img.container = particle.int_data.y;
  return img;
}

INLINE color_t get_particle_color(const gpu_particle_t particle) {
  color_t col;
  col.container = floatBitsToUint(particle.speed_color[2]);
  return col;
}

INLINE uint get_particle_life_time(const gpu_particle_t particle) {
  const uint mask = 0x7fffffff;
  return (particle.int_data.z & mask);
}

INLINE bool particle_is_on_ground(const gpu_particle_t particle) {
  const uint mask = 0x7fffffff;
  return bool(particle.int_data.z & ~mask);
}

INLINE uint get_particle_current_time(const gpu_particle_t particle) {
  return particle.int_data.w;
}

INLINE float get_particle_max_speed(const gpu_particle_t particle) {
  return particle.speed_color[0];
}

INLINE float get_particle_min_speed(const gpu_particle_t particle) {
  return particle.speed_color[1];
}

INLINE float get_particle_friction(const gpu_particle_t particle) {
  return particle.speed_color[3];
}

INLINE float get_particle_max_scale(const gpu_particle_t particle) {
  return particle.float_data[0];
}

INLINE float get_particle_min_scale(const gpu_particle_t particle) {
  return particle.float_data[1];
}

INLINE float get_particle_gravity(const gpu_particle_t particle) {
  return particle.float_data[2];
}

INLINE float get_particle_bounce(const gpu_particle_t particle) {
  return particle.float_data[3];
}

INLINE void set_particle_pos(INOUT gpu_particle_t particle, const vec4 pos) {
  particle.pos = pos;
}

INLINE void set_particle_vel(INOUT gpu_particle_t particle, const vec4 vel) {
  particle.vel = vel;
}

INLINE void set_particle_current_time(INOUT gpu_particle_t particle, const uint time) {
  particle.int_data.w = time;
#ifdef __cplusplus
  (void)particle;
#endif
}

INLINE void set_particle_is_on_ground(INOUT gpu_particle_t particle, const bool value) {
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

INLINE bool particle_min_speed_stop(const gpu_particle_t particle) {
  return (particle.int_data.x & min_speed_stop) == min_speed_stop;
}

INLINE bool particle_min_speed_remove(const gpu_particle_t particle) {
  return (particle.int_data.x & min_speed_remove) == min_speed_remove;
}

INLINE bool particle_max_speed_stop(const gpu_particle_t particle) {
  return (particle.int_data.x & max_speed_stop) == max_speed_stop;
}

INLINE bool particle_max_speed_remove(const gpu_particle_t particle) {
  return (particle.int_data.x & max_speed_remove) == max_speed_remove;
}

INLINE bool particle_speed_dec_over_time(const gpu_particle_t particle) {
  return (particle.int_data.x & speed_dec_over_time) == speed_dec_over_time;
}

INLINE bool particle_speed_inc_over_time(const gpu_particle_t particle) {
  return (particle.int_data.x & speed_inc_over_time) == speed_inc_over_time;
}

INLINE bool particle_scale_dec_over_time(const gpu_particle_t particle) {
  return (particle.int_data.x & scale_dec_over_time) == scale_dec_over_time;
}

INLINE bool particle_scale_inc_over_time(const gpu_particle_t particle) {
  return (particle.int_data.x & scale_inc_over_time) == scale_inc_over_time;
}

INLINE bool particle_scaling_along_vel(const gpu_particle_t particle) {
  return (particle.int_data.x & scaling_along_vel) == scaling_along_vel;
}

INLINE bool particle_limit_max_speed(const gpu_particle_t particle) {
  return (particle.int_data.x & limit_max_speed) == limit_max_speed;
}

INLINE bool particle_limit_min_speed(const gpu_particle_t particle) {
  return (particle.int_data.x & limit_min_speed) == limit_min_speed;
}
    
#ifdef __cplusplus    
  }
}
#endif

#endif
