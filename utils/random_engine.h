#ifndef RANDOM_ENGINE_H
#define RANDOM_ENGINE_H

#include <mutex>
#include <random>
#include "utility.h"

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
      std::mt19937_64 gen; // можно использовать random_device

      random_engine_st();
      random_engine_st(const uint32_t &seed);
      void set_seed(const uint32_t &seed);
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
  }
}

#endif
