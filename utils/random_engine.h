#ifndef RANDOM_ENGINE_H
#define RANDOM_ENGINE_H

#include <mutex>
#include <random>
#include "utility.h"
#include "linear_rng.h"

namespace devils_engine {
  namespace utils {
    struct random_engine {
      std::mutex mutex;
      std::mt19937_64 gen; // можно использовать random_device

      random_engine();
      random_engine(const uint32_t &seed);
      uint32_t num();
      float norm();
      uint32_t index(const uint32_t &size);
      glm::vec3 unit3();
      uint32_t closed(const uint32_t &min, const uint32_t &max);
      int32_t closed(const int32_t &min, const int32_t &max);
      float closed(const float &min, const float &max);
      bool probability(const float &val);
      float gaussian_distribution(const float &mean, const float &dev);
    };
    
    struct random_engine_st {
      //std::mt19937_64 gen; // можно использовать random_device
      using state = xoshiro256starstar::state;
      state s;

      random_engine_st();
      random_engine_st(const uint64_t &seed);
      void set_seed(const uint64_t &seed);
      uint64_t num();
      double norm();
      uint64_t index(const uint64_t &size);
      glm::vec3 unit3();
      uint64_t random_at_most(const uint64_t &max);
      uint64_t closed(const uint64_t &min, const uint64_t &max);
      int64_t closed(const int64_t &min, const int64_t &max);
      double closed(const double &min, const double &max);
      bool probability(const double &val);
      double gaussian_distribution(const double &mean, const double &dev);
    };
  }
}

#endif
