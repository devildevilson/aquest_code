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
    
    //const size_t state_size = xoshiro256starstar::state_size;
    using state = xoshiro256starstar::state;
    using namespace xoshiro256starstar;
//     static inline state seed_state(const uint64_t seed) {
//       state new_state;
//       splitmix64::state splitmix_states[state_size];
//       splitmix_states[0] = splitmix64::rng({seed});
//       for (uint32_t i = 1; i < state_size; ++i) splitmix_states[i] = splitmix64::rng(splitmix_states[i-1]);
//       for (uint32_t i = 0; i < state_size; ++i) new_state.s[i] = splitmix64::get_value(splitmix_states[i]);
//       return new_state;
//     }
    
    random_engine_st::random_engine_st() : s(init(1)) {}
    random_engine_st::random_engine_st(const uint64_t &seed) : s(init(seed)) {}
    void random_engine_st::set_seed(const uint64_t &seed) {
      s = init(seed);
    }
    
    uint64_t random_engine_st::num() {
      s = rng(s);
      return get_value(s);
    }
    
    double random_engine_st::norm() {
      const double v = utils::rng_normalize(num());
      ASSERT(v >= 0.0 && v <= 1.0);
      return v;
    }
    
    uint64_t random_engine_st::index(const uint64_t &size) {
      ASSERT(size != 0);
      ASSERT(size != UINT64_MAX);
      if (size == 1) return 0;
      
      //const uint64_t i = num() % size;
      //const uint64_t i = random_at_most(size);
      //const uint64_t i = norm() * double(size);
      uint64_t i = UINT64_MAX;
      while (i >= size) {
        i = norm() * double(size);
      }
      
      ASSERT(i < size);
      return i;
    }
    
    glm::vec3 random_engine_st::unit3() {
      return glm::normalize(glm::vec3(gaussian_distribution(0, 1), gaussian_distribution(0, 1), gaussian_distribution(0, 1)));
    }
    
    // проверил в другом месте, кажется не работает
    uint64_t random_engine_st::random_at_most(const uint64_t &max) {
      ASSERT(max < UINT64_MAX-1);
      // max <= RAND_MAX < ULONG_MAX, so this is okay.
      const uint64_t num_bins = max+1;
      const uint64_t num_rand = UINT64_MAX;
      const uint64_t bin_size = num_rand / num_bins;
      const uint64_t defect   = num_rand % num_bins;
      
      uint64_t x;
      do {
        x = num();
      } while (num_rand - defect <= x); // This is carefully written not to overflow
      return x/bin_size;
    }
    
    // смотри https://stackoverflow.com/questions/2509679/how-to-generate-a-random-integer-number-from-within-a-range/6852396#6852396
    // надеюсь не будет сильной просадки по скорости
    // вообще еще говорят хороший способ, это умножение на число с плавающей точкой (понятное дело могут быть проблемы с точностью)
    // в double максимальное число 1.8*10^308 (против 1.8*10^19), может быть ошибка не столь значительна, посмотрим
    uint64_t random_engine_st::closed(const uint64_t &min, const uint64_t &max) {
//       if (min == max) return 0;
//       const uint64_t final_max = std::max(min, max);
//       const uint64_t final_min = std::min(min, max);
//       const uint64_t interval = final_max - final_min + 1;
//       return random_at_most(interval) + final_min; // говорят что это гораздо лучшее распределение, чем формула ниже
      //return (num() % (final_max - final_min + 1)) + final_min;
      if (min == max) return 0;
      const uint64_t final_max = std::max(min, max);
      const uint64_t final_min = std::min(min, max);
      const uint64_t interval = final_max - final_min + 1;
      uint64_t val = UINT64_MAX;
      while (val >= interval) {
        val = norm() * double(interval);
      }
      
      return val + final_min;
    }
    
    int64_t random_engine_st::closed(const int64_t &min, const int64_t &max) {
//       if (min == max) return 0;
//       const int64_t final_max = std::max(min, max);
//       const int64_t final_min = std::min(min, max);
//       const int64_t interval = final_max - final_min + 1;
//       ASSERT(interval > 0);
//       return random_at_most(interval) + final_min; // говорят что это гораздо лучшее распределение, чем формула ниже
      //return (num() % (final_max - final_min + 1)) + final_min;
      if (min == max) return 0;
      const int64_t final_max = std::max(min, max);
      const int64_t final_min = std::min(min, max);
      const int64_t interval = final_max - final_min + 1;
      int64_t val = INT64_MAX;
      while (val >= interval) {
        val = norm() * double(interval);
      }
      
      return val + final_min;
    }
    
    double random_engine_st::closed(const double &min, const double &max) {
      const double val = norm();
      return glm::mix(min, max, val);
    }
    
    bool random_engine_st::probability(const double &val) {
      const double v = norm();
      return v < val;
    }
    
    double random_engine_st::gaussian_distribution(const double &mean, const double &dev) {
      double x1, x2, w, y1;
      static double y2;
      static bool again = false;
      double ret;

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
