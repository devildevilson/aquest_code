#include "random_engine.h"

namespace devils_engine {
  namespace utils {
    random_engine::random_engine() : gen(1) {}
    random_engine::random_engine(const uint32_t &seed) : gen(seed) {}

    uint32_t random_engine::num() {
      std::unique_lock<std::mutex> lock(mutex);
      std::uniform_int_distribution<uint32_t> dis(0, UINT32_MAX-1);
      return dis(gen);
    }

    float random_engine::norm() {
      //std::unique_lock<std::mutex> lock(mutex);
      return float(num()) / float(UINT32_MAX-1);
    }

    uint32_t random_engine::index(const uint32_t &size) {
      ASSERT(size != 0);
      ASSERT(size != UINT32_MAX);
      if (size == 1) return 0;
      std::unique_lock<std::mutex> lock(mutex);
      std::uniform_int_distribution<uint32_t> dis(0, size-1);
      const uint32_t i = dis(gen);
      //std::cout << "i " << i << "\n";
      ASSERT(i < size);
      return i;
    }

    glm::vec3 random_engine::unit3() {
      return glm::normalize(glm::vec3(gaussian_distribution(0, 1), gaussian_distribution(0, 1), gaussian_distribution(0, 1)));
    }
    
    uint32_t random_engine::closed(const uint32_t &min, const uint32_t &max) {
      std::unique_lock<std::mutex> lock(mutex);
      std::uniform_int_distribution<uint32_t> dis(min, max-1);
      return dis(gen);
    }
    
    int32_t random_engine::closed(const int32_t &min, const int32_t &max) {
      std::unique_lock<std::mutex> lock(mutex);
      std::uniform_int_distribution<int32_t> dis(min, max-1);
      return dis(gen);
    }

    float random_engine::closed(const float &min, const float &max) {
      const float val = norm();
      return glm::mix(min, max, val);
    }

    bool random_engine::probability(const float &val) {
      const float v = norm();
      return v <= val;
    }

    float random_engine::gaussian_distribution(const float &mean, const float &dev) {
      float x1, x2, w, y1;
      static float y2;
      static bool again = false;
      float ret;

      if (again) ret = mean + y2 * dev;
      else {
        do {
          x1 = norm() * 2.0f - 1.0f;
          x2 = norm() * 2.0f - 1.0f;
          w = x1 * x1 + x2 * x2;
        } while (w >= 1.0f);

        w = sqrt((-2.0f * log(w)) / w);
        y1 = x1 * w;
        y2 = x2 * w;
        ret = mean + y1 * dev;
      }

      again = !again;
      return ret;
    }
    
    random_engine_st::random_engine_st() : gen(1) {}
    random_engine_st::random_engine_st(const uint32_t &seed) : gen(seed) {}
    uint32_t random_engine_st::num() {
      std::uniform_int_distribution<uint32_t> dis(0, UINT32_MAX-1);
      return dis(gen);
    }
    
    float random_engine_st::norm() {
//       return float(num()) / float(UINT32_MAX-1);
      std::uniform_real_distribution<float> dis(0.0f, 1.0f);
      return dis(gen);
    }
    
    uint32_t random_engine_st::index(const uint32_t &size) {
      ASSERT(size != 0);
      ASSERT(size != UINT32_MAX);
      if (size == 1) return 0;
      std::uniform_int_distribution<uint32_t> dis(0, size-1);
      const uint32_t i = dis(gen);
      //std::cout << "i " << i << "\n";
      ASSERT(i < size);
      return i;
    }
    
    glm::vec3 random_engine_st::unit3() {
      return glm::normalize(glm::vec3(gaussian_distribution(0, 1), gaussian_distribution(0, 1), gaussian_distribution(0, 1)));
    }
    
    uint32_t random_engine_st::closed(const uint32_t &min, const uint32_t &max) {
      std::uniform_int_distribution<uint32_t> dis(min, max-1);
      return dis(gen);
    }
    
    int32_t random_engine_st::closed(const int32_t &min, const int32_t &max) {
      std::uniform_int_distribution<int32_t> dis(min, max-1);
      return dis(gen);
    }
    
    float random_engine_st::closed(const float &min, const float &max) {
      const float val = norm();
      return glm::mix(min, max, val);
    }
    
    bool random_engine_st::probability(const float &val) {
      const float v = norm();
      return v < val;
    }
    
    float random_engine_st::gaussian_distribution(const float &mean, const float &dev) {
      float x1, x2, w, y1;
      static float y2;
      static bool again = false;
      float ret;

      if (again) ret = mean + y2 * dev;
      else {
        do {
          x1 = norm() * 2.0f - 1.0f;
          x2 = norm() * 2.0f - 1.0f;
          w = x1 * x1 + x2 * x2;
        } while (w >= 1.0f);

        w = sqrt((-2.0f * log(w)) / w);
        y1 = x1 * w;
        y2 = x2 * w;
        ret = mean + y1 * dev;
      }

      again = !again;
      return ret;
    }
  }
}
