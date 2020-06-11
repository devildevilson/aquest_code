#include "frame_time.h"

#include "shared_time_constant.h"
#include "utility.h"
#include <algorithm>

namespace devils_engine {
  namespace utils {
    frame_time::frame_time() : 
      start_point(std::chrono::steady_clock::now()), 
      end_point(std::chrono::steady_clock::now()), 
      accumulated_frame_time(0), 
      accumulated_sleep_time(0), 
      counter(0), 
      time(0), 
      prev_accumulated_frame_time(0), 
      prev_accumulated_sleep_time(0), 
      prev_counter(0) 
    {}
    
    void frame_time::start() {
      auto new_start_point = std::chrono::steady_clock::now();
      time = std::chrono::duration_cast<CHRONO_TIME_TYPE>(new_start_point - start_point).count();
      const size_t sleep = std::chrono::duration_cast<CHRONO_TIME_TYPE>(new_start_point - end_point).count();
      accumulated_sleep_time += sleep;
      ++counter;
      start_point = new_start_point;
    }
    
    void frame_time::end() {
      auto new_end_point = std::chrono::steady_clock::now();
      const size_t frame = std::chrono::duration_cast<CHRONO_TIME_TYPE>(new_end_point - start_point).count();
      accumulated_frame_time += frame;
      end_point = new_end_point;
      
      if (accumulated_frame_time + accumulated_sleep_time >= ONE_SECOND) {
        prev_accumulated_frame_time = accumulated_frame_time;
        prev_accumulated_sleep_time = accumulated_sleep_time;
        prev_counter = counter;
        accumulated_frame_time = 0;
        accumulated_sleep_time = 0;
        counter = 0;
      }
    }
    
    size_t frame_time::get() const {
      return time;
    }
    
    float frame_time::fps() const {
      const float av = average();
      return av == 0.0f ? 0.0f : float(ONE_SECOND) / av;
    }
    
    float frame_time::average() const {
      return float(prev_accumulated_frame_time + prev_accumulated_sleep_time) / float(std::max(prev_counter, size_t(1)));
    }
  }
}
