#ifndef FRAME_TIME_H
#define FRAME_TIME_H

#include <chrono>
#include <cstddef>

namespace devils_engine {
  namespace utils {
    struct frame_time {
      std::chrono::steady_clock::time_point start_point;
      std::chrono::steady_clock::time_point end_point;
      size_t accumulated_frame_time;
      size_t accumulated_sleep_time;
      size_t counter;
      size_t time;
      size_t prev_accumulated_frame_time;
      size_t prev_accumulated_sleep_time;
      size_t prev_counter;
      
      frame_time();
      void start();
      void end();
      
      size_t get() const;
      float fps() const;
      float average() const;
    };
  }
}

#endif
