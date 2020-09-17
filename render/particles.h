#ifndef PATRICLES_H
#define PATRICLES_H

#include <cstdint>
#include <mutex>
#include <atomic>
#include "shared_structures.h"
#include "GPUArray.h"
#include "target.h"

// namespace yavf {
//   class Device;
// }

namespace devils_engine {
  namespace render {
    struct particle {
      enum flag_bit {
        MIN_SPEED_STOP      = (1 << 0),
        MIN_SPEED_REMOVE    = (1 << 1),
        MAX_SPEED_STOP      = (1 << 2),
        MAX_SPEED_REMOVE    = (1 << 3),
        SPEED_DEC_OVER_TIME = (1 << 4),
        SPEED_INC_OVER_TIME = (1 << 5),
        SCALE_DEC_OVER_TIME = (1 << 6),
        SCALE_INC_OVER_TIME = (1 << 7)
      };
      
      struct color {
        union {
          uint32_t container;
          struct {
            uint8_t r, g, b, a;
          };
        };
        
        color();
        color(uint32_t color);
        color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a);
      };
      
      struct flags {
        uint32_t container;
        
        flags();
        flags(const uint32_t &container);
        bool min_speed_stop() const;
        bool min_speed_remove() const;
        bool max_speed_stop() const;
        bool max_speed_remove() const;
        bool speed_dec_over_time() const;
        bool speed_inc_over_time() const;
        bool scale_dec_over_time() const;
        bool scale_inc_over_time() const;
      };
      
      basic_vec4 pos;
      
      basic_vec4 vel;
      
      struct flags flags;
      struct image image;
      uint32_t life_time; // если life_time == 0 то это свободный слот
      uint32_t current_time;
      
      float max_scale;
      float min_scale;
      float gravity;
      float bounce;
      
      float max_speed;
      float min_speed;
      struct color color;
      float friction;
      
      particle();
      particle(const simd::vec4 &pos, const simd::vec4 &vel, const struct image &image, const uint32_t &life_time);
      particle(const simd::vec4 &pos, const simd::vec4 &vel, const struct color &color, const uint32_t &life_time);
    };
    
    struct particles_data {
      uint32_t old_count;
      uint32_t new_count;
      uint32_t particles_count;
      uint32_t frame_time;
      
      uint32_t new_max_count;
      uint32_t indices_count;
      uint32_t dummy[2];
      
      basic_vec4 gravity;
      basic_vec4 frustum[6];
    };
    
    struct particles : public target {
      static const size_t max_new_particles = 10000;
      
      std::atomic<uint32_t> next_index;
      GPUArray<particle> array; // старые частицы можно расположить в памяти гпу
      yavf::Buffer new_particles;
      yavf::Buffer data_buffer;
      yavf::Buffer indices;
      yavf::DescriptorSetLayout layout;
      // если разделить старые частицы и новые, 
      // то становится легко контролировать
      // добавление/удаление 
      
      particles(yavf::Device* device);
      bool add(const struct particle &particle);
      void reset();
      void update(const simd::mat4 &view_proj, const simd::vec4 &gravity, const size_t &time);
      void recreate(const uint32_t &width, const uint32_t &height) override;
    };
  }
}

#endif
