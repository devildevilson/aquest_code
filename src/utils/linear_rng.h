#ifndef LINEAR_RNG_H
#define LINEAR_RNG_H

#include <cstdint>
#include <cstddef>

// реализация этих алгоритмов: http://prng.di.unimi.it/
// возможно мне потребуется джампинг оттуда

namespace devils_engine {
  namespace utils {
    double rng_normalize(const uint64_t &value); // или сделать через темлейт
    float rng_normalizef(const uint32_t &value);
    
    namespace mulberry32 { // используется для инициализации
      const size_t state_size = 1;
      struct state { uint32_t s[state_size]; };
      state init(const uint32_t &seed);
      state rng(state s);
      uint32_t get_value(const state &s);
    }
    
    namespace splitmix64 { // можно использовать для инициализации стейтов других генераторов
      const size_t state_size = 1;
      struct state { uint64_t s[state_size]; };
      state init(const uint64_t &seed);
      state rng(state s);
      uint64_t get_value(const state &s);
    }
    
    namespace xorshift64 {
      const size_t state_size = 1;
      struct state { uint64_t s[state_size]; };
      state init(const uint64_t &seed);
      state rng(state s);
      uint64_t get_value(const state &s);
    }
    
    namespace xoroshiro128plus {
      const size_t state_size = 2;
      struct state { uint64_t s[state_size]; };
      state init(const uint64_t &seed);
      state rng(state s);
      uint64_t get_value(const state &s);
    }
    
    namespace xoroshiro128plusplus {
      const size_t state_size = 2;
      struct state { uint64_t s[state_size]; };
      state init(const uint64_t &seed);
      state rng(state s);
      uint64_t get_value(const state &s);
    }
    
    namespace xoroshiro128starstar {
      const size_t state_size = 2;
      struct state { uint64_t s[state_size]; };
      state init(const uint64_t &seed);
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
      state init(const uint64_t &seed);
      state rng(state s);
      uint64_t get_value(const state &s);
    }
    
    namespace xoshiro256starstar {
      const size_t state_size = 4;
      struct state { uint64_t s[state_size]; };
      state init(const uint64_t &seed);
      state rng(state s);
      uint64_t get_value(const state &s);
    }
    
    namespace xoshiro512plus {
      const size_t state_size = 8;
      struct state { uint64_t s[state_size]; };
      state init(const uint64_t &seed);
      state rng(state s);
      uint64_t get_value(const state &s);
    }
    
    namespace xoshiro512plusplus {
      const size_t state_size = 8;
      struct state { uint64_t s[state_size]; };
      state init(const uint64_t &seed);
      state rng(state s);
      uint64_t get_value(const state &s);
    }
    
    namespace xoshiro512starstar {
      const size_t state_size = 8;
      struct state { uint64_t s[state_size]; };
      state init(const uint64_t &seed);
      state rng(state s);
      uint64_t get_value(const state &s);
    }
    
    namespace xoroshiro1024star {
      const size_t state_size = 16;
      struct state { int32_t p; uint64_t s[state_size]; };
      state init(const uint64_t &seed);
      state rng(state s);
      uint64_t get_value(const state &s);
    }
    
    namespace xoroshiro1024plusplus {
      const size_t state_size = 16;
      struct state { int32_t p; uint64_t s[state_size]; };
      state init(const uint64_t &seed);
      state rng(state s);
      uint64_t get_value(const state &s);
    }
    
    namespace xoroshiro1024starstar {
      const size_t state_size = 16;
      struct state { int32_t p; uint64_t s[state_size]; };
      state init(const uint64_t &seed);
      state rng(state s);
      uint64_t get_value(const state &s);
    }
    
    // вариант oneseq имеет плохие показатели для генератора (судя по форумам и прочей информации в сети)
    // unique варианты наверное вообще работать не будут в текущем интерфейсе, хотя если сделать const&
    // mcg вариант не определен для тех функций которые я выбрал (rxs_m_xs и xsl_rr_rr)
    // вообще pcg не выглядит каким то уж слишком хорошим
    
    namespace pcg_rxs_m_xs64unique {
      const size_t state_size = 1;
      struct state { uint64_t s[state_size]; };
      state init(const uint64_t &seed);
      state rng(const state &s);
      uint64_t get_value(const state &s);
    }
    
    namespace pcg_rxs_m_xs64setseq {
      const size_t state_size = 1;
      struct state { uint64_t s[state_size]; uint64_t inc; };
      state init(const uint64_t &seed, const uint64_t &initseq);
      state rng(state s);
      uint64_t get_value(const state &s);
    }
    
    namespace pcg_xsl_rr_rr64unique {
      const size_t state_size = 1;
      struct state { uint64_t s[state_size]; };
      state init(const uint64_t &seed);
      state rng(const state &s);
      uint64_t get_value(const state &s);
    }
    
    namespace pcg_xsl_rr_rr64setseq {
      const size_t state_size = 1;
      struct state { uint64_t s[state_size]; uint64_t inc; };
      state init(const uint64_t &seed, const uint64_t &initseq);
      state rng(state s);
      uint64_t get_value(const state &s);
    }
    
    // по идее 128 версия медленее чем 64, но имеет больший период
    namespace pcg_rxs_m_xs128unique {
      using uint128_t = __uint128_t;
      const size_t state_size = 1;
      struct state { uint128_t s[state_size]; };
      state init(const uint128_t &seed);
      state rng(const state &s);
      uint128_t get_value(const state &s);
    }
    
    namespace pcg_rxs_m_xs128setseq {
      using uint128_t = __uint128_t;
      const size_t state_size = 1;
      struct state { uint128_t s[state_size]; uint128_t inc; };
      state init(const uint128_t &seed, const uint128_t &initseq);
      state rng(state s);
      uint128_t get_value(const state &s);
    }
    
    namespace pcg_xsl_rr_rr128unique {
      using uint128_t = __uint128_t;
      const size_t state_size = 1;
      struct state { uint128_t s[state_size]; };
      state init(const uint128_t &seed);
      state rng(const state &ss);
      uint128_t get_value(const state &s);
    }
    
    namespace pcg_xsl_rr_rr128setseq {
      using uint128_t = __uint128_t;
      const size_t state_size = 1;
      struct state { uint128_t s[state_size]; uint128_t inc; };
      state init(const uint128_t &seed, const uint128_t &initseq);
      state rng(state s);
      uint128_t get_value(const state &s);
    }
    
    // 32 бита, иной интерфейс, зато очень большой период
    namespace cmwc {
      const size_t cycle = 4096;      // as Marsaglia recommends
      const size_t c_max = 809430660; // as Marsaglia recommends
      struct state {
        uint32_t Q[cycle];
        uint32_t c;
        uint32_t i;
      };
      
      void init(state &s, const uint32_t &seed);
      uint32_t get_value(state &s);
    }
    
    // используют __uint128_t, по идее скорость ниже, не уверен на счет переносимости
    // на windows не будет работать по всей видимости
    namespace cmwc128 { 
      using uint128_t = __uint128_t;
      const size_t state_size = 2;
      struct state { uint64_t s[state_size]; };
      state init(const uint64_t &seed);
      state rng(state s);
      uint64_t get_value(const state &s);
    }
    
    namespace cmwc256 {
      using uint128_t = __uint128_t;
      const size_t state_size = 4;
      struct state { uint64_t s[state_size]; };
      state init(const uint64_t &seed);
      state rng(state s);
      uint64_t get_value(const state &s);
    }
    
    // дефолтный
//     namespace rng {
//       const size_t state_size = xoroshiro128starstar::state_size;
//       using state = xoroshiro128starstar::state;
//       constexpr const auto init = xoroshiro128starstar::init;
//       constexpr const auto next = xoroshiro128starstar::rng;
//       constexpr const auto value = xoroshiro128starstar::get_value;
//       constexpr const auto normalize = rng_normalize;
//     }
  }
}

#endif
