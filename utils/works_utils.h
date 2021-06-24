#ifndef WORKS_UTILS_H
#define WORKS_UTILS_H

#include <cmath>
#include "thread_pool.h"
#include "assert.h"

namespace devils_engine {
  namespace utils {
    template<typename T, typename... Args>
    void submit_works(dt::thread_pool* pool, const size_t &count, T&& func, Args&& ...args) {
      if (count == 0) return;

      const size_t work_count = std::ceil(float(count) / float(pool->size()+1));
      size_t start = 0;
      for (size_t i = 0; i < pool->size()+1; ++i) {
        const uint32_t jobCount = std::min(work_count, count-start);
        if (jobCount == 0) break;

        // тут все равно будет выделение памяти, видимо этого не избежать никак
        pool->submitbase([start, jobCount, func, args...] () {
          func(start, jobCount, args...);
        });
//         pool->submitnr(func, start, jobCount, std::forward<Args>(args)...);

        start += jobCount;
      }

      pool->compute();
      pool->wait();
    }
    
    template<typename T, typename... Args>
    void submit_works_async(dt::thread_pool* pool, const size_t &count, T&& func, Args&& ...args) {
      if (count == 0) return;

      const size_t work_count = std::ceil(float(count) / float(pool->size()));
      size_t start = 0;
      for (size_t i = 0; i < pool->size(); ++i) {
        const uint32_t jobCount = std::min(work_count, count-start);
        if (jobCount == 0) break;

        // тут все равно будет выделение памяти, видимо этого не избежать никак
        pool->submitbase([start, jobCount, func, args...] () {
          func(start, jobCount, args...);
        });
//         pool->submitnr(func, start, jobCount, std::forward<Args>(args)...);

        start += jobCount;
      }
    }
    
    // использовать только для тредов в тредпуле (то есть для вторичных тредов (не главный))
    inline void async_wait(dt::thread_pool* pool) noexcept {
      pool->compute();
      while (pool->working_count() != 1 || pool->tasks_count() != 0) { std::this_thread::sleep_for(std::chrono::microseconds(1)); }
      ASSERT(pool->working_count() == 1 && pool->tasks_count() == 0);
    }
    
    template <typename T>
    void atomic_max(std::atomic<T> &maximum_value, const T &value) noexcept {
      T prev_value = maximum_value;
      while (prev_value < value && !maximum_value.compare_exchange_weak(prev_value, value));
    }
    
    template <typename T>
    void atomic_min(std::atomic<T> &maximum_value, const T &value) noexcept {
      T prev_value = maximum_value;
      while (prev_value > value && !maximum_value.compare_exchange_weak(prev_value, value));
    }
  }
}

#endif
