#ifndef LINEAR_RNG_H
#define LINEAR_RNG_H

#include <cstdint>
#include <cstddef>

namespace devils_engine {
  namespace utils {
    namespace splitmix64 { // можно использовать для инициализации стейтов других генераторов
      using state = uint64_t;
      state rng(const state &s);
      uint64_t get_value(const state &s);
      double normalize(const uint64_t &value);
    }
    
    namespace xorshift64 {
      using state = uint64_t;
      //const size_t period = (1 << 64)-1;
      state rng(const state &s);
      uint64_t get_value(const state &s);
      double normalize(const uint64_t &value);
    }
    
    namespace xoroshiro128starstar {
      struct state { uint64_t s[2]; };
      state rng(const state &s);
      uint64_t get_value(const state &s);
      double normalize(const uint64_t &value);
    }
    
    namespace xoshiro256plusplus {
      struct state { uint64_t s[4]; };
      state rng(const state &s);
      uint64_t get_value(const state &s);
      double normalize(const uint64_t &value);
    }
    
    namespace rng {
      using state = xoroshiro128starstar::state;
      constexpr const auto next = xoroshiro128starstar::rng;
      constexpr const auto value = xoroshiro128starstar::get_value;
      constexpr const auto normalize = xoroshiro128starstar::normalize;
    }
  }
}

#endif
