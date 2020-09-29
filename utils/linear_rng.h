#ifndef LINEAR_RNG_H
#define LINEAR_RNG_H

#include <cstdint>
#include <cstddef>

// реализация этих алгоритмов: http://prng.di.unimi.it/
// возможно мне потребуется джампинг оттуда

namespace devils_engine {
  namespace utils {
    double rng_normalize(const uint64_t &value);
    
    namespace splitmix64 { // можно использовать для инициализации стейтов других генераторов
      const size_t state_size = 1;
      struct state { uint64_t s[state_size]; };
      state rng(state s);
      uint64_t get_value(const state &s);
    }
    
    namespace xorshift64 {
      const size_t state_size = 1;
      struct state { uint64_t s[state_size]; };
      state rng(state s);
      uint64_t get_value(const state &s);
    }
    
    namespace xoroshiro128plus {
      const size_t state_size = 2;
      struct state { uint64_t s[state_size]; };
      state rng(state s);
      uint64_t get_value(const state &s);
    }
    
    namespace xoroshiro128plusplus {
      const size_t state_size = 2;
      struct state { uint64_t s[state_size]; };
      state rng(state s);
      uint64_t get_value(const state &s);
    }
    
    namespace xoroshiro128starstar {
      const size_t state_size = 2;
      struct state { uint64_t s[state_size]; };
      state rng(state s);
      uint64_t get_value(const state &s);
    }
    
    namespace xoshiro256plus {
      const size_t state_size = 4;
      struct state { uint64_t s[state_size]; };
      state rng(state s);
      uint64_t get_value(const state &s);
    }
    
    namespace xoshiro256plusplus {
      const size_t state_size = 4;
      struct state { uint64_t s[state_size]; };
      state rng(state s);
      uint64_t get_value(const state &s);
    }
    
    namespace xoshiro256starstar {
      const size_t state_size = 4;
      struct state { uint64_t s[state_size]; };
      state rng(state s);
      uint64_t get_value(const state &s);
    }
    
    namespace xoshiro512plus {
      const size_t state_size = 8;
      struct state { uint64_t s[state_size]; };
      state rng(state s);
      uint64_t get_value(const state &s);
    }
    
    namespace xoshiro512plusplus {
      const size_t state_size = 8;
      struct state { uint64_t s[state_size]; };
      state rng(state s);
      uint64_t get_value(const state &s);
    }
    
    namespace xoshiro512starstar {
      const size_t state_size = 8;
      struct state { uint64_t s[state_size]; };
      state rng(state s);
      uint64_t get_value(const state &s);
    }
    
    namespace rng {
      const size_t state_size = xoroshiro128starstar::state_size;
      using state = xoroshiro128starstar::state;
      constexpr const auto next = xoroshiro128starstar::rng;
      constexpr const auto value = xoroshiro128starstar::get_value;
      constexpr const auto normalize = rng_normalize;
    }
  }
}

#endif
