#include "linear_rng.h"

namespace devils_engine {
  namespace utils {
    double rng_normalize(const uint64_t &value) {
      union { uint64_t i; double d; } u;
      u.i = (UINT64_C(0x3FF) << 52) | (value >> 12);
      return u.d - 1.0;
    }
    
    float rng_normalizef(const uint32_t &value) {
      union { uint32_t i; float f; } u;
      const uint32_t float_mask = 0x7f << 23;
      u.i = float_mask | (value >> 9);
      return u.f - 1.0f;
    }
    
    static inline uint64_t rotl(const uint64_t x, int k) {
      return (x << k) | (x >> (64 - k));
    }
    
    namespace mulberry32 {
      state init(const uint32_t &seed) {
        return rng({seed});
      }
      
      state rng(state s) {
        return {s.s[0] + 0x6D2B79F5};
      }
      
      uint32_t get_value(const state &s) {
        uint32_t z = s.s[0];
        z = (z ^ (z >> 15)) * (z | 1);
        z ^= z + (z ^ (z >> 7)) * (z | 61);
        return z ^ (z >> 14);
      }
    }
    
    namespace splitmix64 {
      state init(const uint64_t &seed) {
        return rng({seed});
      }
      
      state rng(state s) {
        return {s.s[0] + 0x9e3779b97f4a7c15};
      }
      
      uint64_t get_value(const state &s) {
        uint64_t z = s.s[0];
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
        z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
        return z ^ (z >> 31);
      }
    }
    
    namespace xorshift64 {
      state init(const uint64_t &seed) {
        state new_state;
        splitmix64::state splitmix_states[state_size];
        splitmix_states[0] = splitmix64::rng({seed});
        for (uint32_t i = 1; i < state_size; ++i) splitmix_states[i] = splitmix64::rng(splitmix_states[i-1]);
        for (uint32_t i = 0; i < state_size; ++i) new_state.s[i] = splitmix64::get_value(splitmix_states[i]);
        return new_state;
      }
      
      state rng(state s) {
        s.s[0] ^= s.s[0] << 13;
        s.s[0] ^= s.s[0] >> 7;
        s.s[0] ^= s.s[0] << 17;
        return s;
      }
      
      uint64_t get_value(const state &s) {
        return s.s[0];
      }
    }
    
    namespace xoroshiro128plus {
      state init(const uint64_t &seed) {
        state new_state;
        splitmix64::state splitmix_states[state_size];
        splitmix_states[0] = splitmix64::rng({seed});
        for (uint32_t i = 1; i < state_size; ++i) splitmix_states[i] = splitmix64::rng(splitmix_states[i-1]);
        for (uint32_t i = 0; i < state_size; ++i) new_state.s[i] = splitmix64::get_value(splitmix_states[i]);
        return new_state;
      }
      
      state rng(state s) {
        const uint64_t s0 = s.s[0];
        uint64_t s1 = s.s[1];
        s1 ^= s0;
        s.s[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16); // a, b
        s.s[1] = rotl(s1, 37); // c
        return s;
      }
      
      uint64_t get_value(const state &s) {
        return s.s[0] + s.s[1];
      }
    }
    
    namespace xoroshiro128plusplus {
      state init(const uint64_t &seed) {
        state new_state;
        splitmix64::state splitmix_states[state_size];
        splitmix_states[0] = splitmix64::rng({seed});
        for (uint32_t i = 1; i < state_size; ++i) splitmix_states[i] = splitmix64::rng(splitmix_states[i-1]);
        for (uint32_t i = 0; i < state_size; ++i) new_state.s[i] = splitmix64::get_value(splitmix_states[i]);
        return new_state;
      }
      
      state rng(state s) {
        const uint64_t s0 = s.s[0];
        uint64_t s1 = s.s[1];
        s1 ^= s0;
        s.s[0] = rotl(s0, 49) ^ s1 ^ (s1 << 21); // a, b
        s.s[1] = rotl(s1, 28); // c
        return s;
      }
      
      uint64_t get_value(const state &s) {
        return rotl(s.s[0] + s.s[1], 17) + s.s[0];
      }
    }
    
    namespace xoroshiro128starstar {
      state init(const uint64_t &seed) {
        state new_state;
        splitmix64::state splitmix_states[state_size];
        splitmix_states[0] = splitmix64::rng({seed});
        for (uint32_t i = 1; i < state_size; ++i) splitmix_states[i] = splitmix64::rng(splitmix_states[i-1]);
        for (uint32_t i = 0; i < state_size; ++i) new_state.s[i] = splitmix64::get_value(splitmix_states[i]);
        return new_state;
      }
      
      state rng(state s) {
        const uint64_t s0 = s.s[0];
        uint64_t s1 = s.s[1];
        s1 ^= s0;
        s.s[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16); // a, b
        s.s[1] = rotl(s1, 37); // c
        return s;
      }
      
      uint64_t get_value(const state &s) {
        return rotl(s.s[0] * 5, 7) * 9;
      }
    }
    
    namespace xoshiro256plus {
      state init(const uint64_t &seed) {
        state new_state;
        splitmix64::state splitmix_states[state_size];
        splitmix_states[0] = splitmix64::rng({seed});
        for (uint32_t i = 1; i < state_size; ++i) splitmix_states[i] = splitmix64::rng(splitmix_states[i-1]);
        for (uint32_t i = 0; i < state_size; ++i) new_state.s[i] = splitmix64::get_value(splitmix_states[i]);
        return new_state;
      }
      
      state rng(state s) {
        const uint64_t t = s.s[1] << 17;
        s.s[2] ^= s.s[0];
        s.s[3] ^= s.s[1];
        s.s[1] ^= s.s[2];
        s.s[0] ^= s.s[3];
        s.s[2] ^= t;
        s.s[3] = rotl(s.s[3], 45);
        return s;
      }
      
      uint64_t get_value(const state &s) {
        return s.s[0] + s.s[3];
      }
    }
    
    namespace xoshiro256plusplus {
      state init(const uint64_t &seed) {
        state new_state;
        splitmix64::state splitmix_states[state_size];
        splitmix_states[0] = splitmix64::rng({seed});
        for (uint32_t i = 1; i < state_size; ++i) splitmix_states[i] = splitmix64::rng(splitmix_states[i-1]);
        for (uint32_t i = 0; i < state_size; ++i) new_state.s[i] = splitmix64::get_value(splitmix_states[i]);
        return new_state;
      }
      
      state rng(state s) {
        const uint64_t t = s.s[1] << 17;
        s.s[2] ^= s.s[0];
        s.s[3] ^= s.s[1];
        s.s[1] ^= s.s[2];
        s.s[0] ^= s.s[3];
        s.s[2] ^= t;
        s.s[3] = rotl(s.s[3], 45);
        return s;
      }
      
      uint64_t get_value(const state &s) {
        return rotl(s.s[0] + s.s[3], 23) + s.s[0];
      }
    }
    
    namespace xoshiro256starstar {
      state init(const uint64_t &seed) {
        state new_state;
        splitmix64::state splitmix_states[state_size];
        splitmix_states[0] = splitmix64::rng({seed});
        for (uint32_t i = 1; i < state_size; ++i) splitmix_states[i] = splitmix64::rng(splitmix_states[i-1]);
        for (uint32_t i = 0; i < state_size; ++i) new_state.s[i] = splitmix64::get_value(splitmix_states[i]);
        return new_state;
      }
      
      state rng(state s) {
        const uint64_t t = s.s[1] << 17;
        s.s[2] ^= s.s[0];
        s.s[3] ^= s.s[1];
        s.s[1] ^= s.s[2];
        s.s[0] ^= s.s[3];
        s.s[2] ^= t;
        s.s[3] = rotl(s.s[3], 45);
        return s;
      }
      
      uint64_t get_value(const state &s) {
        return rotl(s.s[1] * 5, 7) * 9;
      }
    }
    
    namespace xoshiro512plus {
      state init(const uint64_t &seed) {
        state new_state;
        splitmix64::state splitmix_states[state_size];
        splitmix_states[0] = splitmix64::rng({seed});
        for (uint32_t i = 1; i < state_size; ++i) splitmix_states[i] = splitmix64::rng(splitmix_states[i-1]);
        for (uint32_t i = 0; i < state_size; ++i) new_state.s[i] = splitmix64::get_value(splitmix_states[i]);
        return new_state;
      }
      
      state rng(state s) {
        const uint64_t t = s.s[1] << 11;
        s.s[2] ^= s.s[0];
        s.s[5] ^= s.s[1];
        s.s[1] ^= s.s[2];
        s.s[7] ^= s.s[3];
        s.s[3] ^= s.s[4];
        s.s[4] ^= s.s[5];
        s.s[0] ^= s.s[6];
        s.s[6] ^= s.s[7];
        s.s[6] ^= t;
        s.s[7] = rotl(s.s[7], 21);
        return s;
      }
      
      uint64_t get_value(const state &s) {
        return s.s[0] + s.s[2];
      }
    }
    
    namespace xoshiro512plusplus {
      state init(const uint64_t &seed) {
        state new_state;
        splitmix64::state splitmix_states[state_size];
        splitmix_states[0] = splitmix64::rng({seed});
        for (uint32_t i = 1; i < state_size; ++i) splitmix_states[i] = splitmix64::rng(splitmix_states[i-1]);
        for (uint32_t i = 0; i < state_size; ++i) new_state.s[i] = splitmix64::get_value(splitmix_states[i]);
        return new_state;
      }
      
      state rng(state s) {
        const uint64_t t = s.s[1] << 11;
        s.s[2] ^= s.s[0];
        s.s[5] ^= s.s[1];
        s.s[1] ^= s.s[2];
        s.s[7] ^= s.s[3];
        s.s[3] ^= s.s[4];
        s.s[4] ^= s.s[5];
        s.s[0] ^= s.s[6];
        s.s[6] ^= s.s[7];
        s.s[6] ^= t;
        s.s[7] = rotl(s.s[7], 21);
        return s;
      }
      
      uint64_t get_value(const state &s) {
        return rotl(s.s[0] + s.s[2], 17) + s.s[2];
      }
    }
    
    namespace xoshiro512starstar {
      state init(const uint64_t &seed) {
        state new_state;
        splitmix64::state splitmix_states[state_size];
        splitmix_states[0] = splitmix64::rng({seed});
        for (uint32_t i = 1; i < state_size; ++i) splitmix_states[i] = splitmix64::rng(splitmix_states[i-1]);
        for (uint32_t i = 0; i < state_size; ++i) new_state.s[i] = splitmix64::get_value(splitmix_states[i]);
        return new_state;
      }
      
      state rng(state s) {
        const uint64_t t = s.s[1] << 11;
        s.s[2] ^= s.s[0];
        s.s[5] ^= s.s[1];
        s.s[1] ^= s.s[2];
        s.s[7] ^= s.s[3];
        s.s[3] ^= s.s[4];
        s.s[4] ^= s.s[5];
        s.s[0] ^= s.s[6];
        s.s[6] ^= s.s[7];
        s.s[6] ^= t;
        s.s[7] = rotl(s.s[7], 21);
        return s;
      }
      
      uint64_t get_value(const state &s) {
        return rotl(s.s[1] * 5, 7) * 9;
      }
    }
    
    namespace xoroshiro1024star {
      state init(const uint64_t &seed) {
        state new_state;
        splitmix64::state splitmix_states[state_size];
        splitmix_states[0] = splitmix64::rng({seed});
        for (uint32_t i = 1; i < state_size; ++i) splitmix_states[i] = splitmix64::rng(splitmix_states[i-1]);
        for (uint32_t i = 0; i < state_size; ++i) new_state.s[i] = splitmix64::get_value(splitmix_states[i]);
        new_state.p = splitmix64::get_value(splitmix64::rng(splitmix_states[state_size-1])) % state_size;
        return new_state;
      }
      
      state rng(state s) {
        const int32_t q = s.p;
        s.p = (s.p + 1) & 15;
        const uint64_t s0 = s.s[s.p];
        uint64_t s15 = s.s[q];

        s15 ^= s0;
        s.s[q] = rotl(s0, 25) ^ s15 ^ (s15 << 27);
        s.s[s.p] = rotl(s15, 36);

        return s;
      }
      
      uint64_t get_value(const state &s) {
        const uint64_t s0 = s.s[s.p];
        return s0 * 0x9e3779b97f4a7c13;
      }
    }
    
    namespace xoroshiro1024plusplus {
      state init(const uint64_t &seed) {
        state new_state;
        splitmix64::state splitmix_states[state_size];
        splitmix_states[0] = splitmix64::rng({seed});
        for (uint32_t i = 1; i < state_size; ++i) splitmix_states[i] = splitmix64::rng(splitmix_states[i-1]);
        for (uint32_t i = 0; i < state_size; ++i) new_state.s[i] = splitmix64::get_value(splitmix_states[i]);
        new_state.p = splitmix64::get_value(splitmix64::rng(splitmix_states[state_size-1])) % state_size;
        return new_state;
      }
      
      state rng(state s) {
        const int32_t q = s.p;
        s.p = (s.p + 1) & 15;
        const uint64_t s0 = s.s[s.p];
        uint64_t s15 = s.s[q];

        s15 ^= s0;
        s.s[q] = rotl(s0, 25) ^ s15 ^ (s15 << 27);
        s.s[s.p] = rotl(s15, 36);

        return s;
      }
      
      uint64_t get_value(const state &s) {
        const int32_t q = s.p;
        const int32_t p = (s.p + 1) & 15;
        const uint64_t s0 = s.s[p];
        const uint64_t s15 = s.s[q];
        return rotl(s0 + s15, 23) + s15;
      }
    }
    
    namespace xoroshiro1024starstar {
      state init(const uint64_t &seed) {
        state new_state;
        splitmix64::state splitmix_states[state_size];
        splitmix_states[0] = splitmix64::rng({seed});
        for (uint32_t i = 1; i < state_size; ++i) splitmix_states[i] = splitmix64::rng(splitmix_states[i-1]);
        for (uint32_t i = 0; i < state_size; ++i) new_state.s[i] = splitmix64::get_value(splitmix_states[i]);
        new_state.p = splitmix64::get_value(splitmix64::rng(splitmix_states[state_size-1])) % state_size;
        return new_state;
      }
      
      state rng(state s) {
        const int32_t q = s.p;
        s.p = (s.p + 1) & 15;
        const uint64_t s0 = s.s[s.p];
        uint64_t s15 = s.s[q];

        s15 ^= s0;
        s.s[q] = rotl(s0, 25) ^ s15 ^ (s15 << 27);
        s.s[s.p] = rotl(s15, 36);

        return s;
      }
      
      uint64_t get_value(const state &s) {
        const uint64_t s0 = s.s[s.p];
        return rotl(s0 * 5, 7) * 9;
      }
    }
    
    static inline uint32_t pcg_rotr_32(uint32_t value, unsigned int rot) {
      return (value >> rot) | (value << ((- rot) & 31));
    }
    
    static inline uint64_t pcg_rotr_64(uint64_t value, unsigned int rot) {
      return (value >> rot) | (value << ((- rot) & 63));
    }

//     static inline __uint128_t pcg_rotr_128(__uint128_t value, unsigned int rot) {
//       return (value >> rot) | (value << ((- rot) & 127));
//     }
    
    static inline uint64_t pcg_output_rxs_m_xs_64_64(const uint64_t state) {
      const uint64_t word = ((state >> ((state >> 59u) + 5u)) ^ state) * 12605985483714917081ull;
      return (word >> 43u) ^ word;
    }
    
    #define PCG_128BIT_CONSTANT(high,low) ((((__uint128_t)high) << 64) + low)
    static inline __uint128_t pcg_output_rxs_m_xs_128_128(__uint128_t state) {
      const __uint128_t word = ((state >> ((state >> 122u) + 6u)) ^ state) * (PCG_128BIT_CONSTANT(17766728186571221404ULL, 12605985483714917081ULL));
      /* 327738287884841127335028083622016905945 */
      return (word >> 86u) ^ word; 
    }
    
    static inline uint64_t pcg_output_xsl_rr_rr_64_64(uint64_t state) {
      uint32_t rot1 = (uint32_t)(state >> 59u);
      uint32_t high = (uint32_t)(state >> 32u);
      uint32_t low  = (uint32_t)state;
      uint32_t xored = high ^ low;
      uint32_t newlow  = pcg_rotr_32(xored, rot1);
      uint32_t newhigh = pcg_rotr_32(high, newlow & 31u);
      return (((uint64_t)newhigh) << 32u) | newlow;
    }

    static inline __uint128_t pcg_output_xsl_rr_rr_128_128(__uint128_t state) {
      uint32_t rot1 = (uint32_t)(state >> 122u);
      uint64_t high = (uint64_t)(state >> 64u);
      uint64_t low  = (uint64_t)state;
      uint64_t xored = high ^ low;
      uint64_t newlow  = pcg_rotr_64(xored, rot1);
      uint64_t newhigh = pcg_rotr_64(high, newlow & 63u);
      return (((__uint128_t)newhigh) << 64u) | newlow;
    }
    
    #define PCG_DEFAULT_MULTIPLIER_64 6364136223846793005ULL
    #define PCG_DEFAULT_INCREMENT_64  1442695040888963407ULL
    #define PCG_DEFAULT_MULTIPLIER_128 PCG_128BIT_CONSTANT(2549297995355413924ULL,4865540595714422341ULL)
    #define PCG_DEFAULT_INCREMENT_128  PCG_128BIT_CONSTANT(6364136223846793005ULL,1442695040888963407ULL)
    
    namespace pcg_rxs_m_xs64unique {
      state init(const uint64_t &seed) {
        state s = {0U};
        s = rng(s);
        s.s[0] += seed;
        return rng(s);
      }
      
      state rng(const state &s) {
        state new_s = s;
        new_s.s[0] = new_s.s[0] * PCG_DEFAULT_MULTIPLIER_64 + (uint64_t)(((intptr_t)&s) | 1u);
        return new_s;
      }
      
      uint64_t get_value(const state &s) {
        return pcg_output_rxs_m_xs_64_64(s.s[0]);
      }
    }
    
    namespace pcg_rxs_m_xs64setseq {
      state init(const uint64_t &seed, const uint64_t &initseq) {
        state s = {{0U}, (initseq << 1u) | 1u};
        s = rng(s);
        s.s[0] += seed;
        return rng(s);
      }
      
      state rng(state s) {
        s.s[0] = s.s[0] * PCG_DEFAULT_MULTIPLIER_64 + s.inc;
        return s;
      }
      
      uint64_t get_value(const state &s) {
        return pcg_output_rxs_m_xs_64_64(s.s[0]);
      }
    }
    
    namespace pcg_xsl_rr_rr64unique {
      state init(const uint64_t &seed) {
        state s = {0U};
        s = rng(s);
        s.s[0] += seed;
        return rng(s);
      }
      
      state rng(const state &s) {
        state new_s = s;
        new_s.s[0] = new_s.s[0] * PCG_DEFAULT_MULTIPLIER_64 + (uint64_t)(((intptr_t)&s) | 1u);
        return new_s;
      }
      
      uint64_t get_value(const state &s) {
        return pcg_output_xsl_rr_rr_64_64(s.s[0]);
      }
    }
    
    namespace pcg_xsl_rr_rr64setseq {
      state init(const uint64_t &seed, const uint64_t &initseq) {
        state s = {{0U}, (initseq << 1u) | 1u};
        s = rng(s);
        s.s[0] += seed;
        return rng(s);
      }
      
      state rng(state s) {
        s.s[0] = s.s[0] * PCG_DEFAULT_MULTIPLIER_64 + s.inc;
        return s;
      }
      
      uint64_t get_value(const state &s) {
        return pcg_output_xsl_rr_rr_64_64(s.s[0]);
      }
    }
    
    namespace pcg_rxs_m_xs128unique {
      state init(const uint128_t &seed) {
        state s = {0U};
        s = rng(s);
        s.s[0] += seed;
        return rng(s);
      }
      
      state rng(const state &s) {
        state new_s = s;
        new_s.s[0] = new_s.s[0] * PCG_DEFAULT_MULTIPLIER_128 + (uint128_t)(((intptr_t)&s) | 1u);
        return new_s;
      }
      
      uint128_t get_value(const state &s) {
        return pcg_output_rxs_m_xs_128_128(s.s[0]);
      }
    }
    
    namespace pcg_rxs_m_xs128setseq {
      state init(const uint128_t &seed, const uint128_t &initseq) {
        state s = {{0U}, (initseq << 1u) | 1u};
        s = rng(s);
        s.s[0] += seed;
        return rng(s);
      }
      
      state rng(state s) {
        s.s[0] = s.s[0] * PCG_DEFAULT_MULTIPLIER_128 + s.inc;
        return s;
      }
      
      uint128_t get_value(const state &s) {
        return pcg_output_rxs_m_xs_128_128(s.s[0]);
      }
    }
    
    namespace pcg_xsl_rr_rr128unique {
      state init(const uint128_t &seed) {
        state s = {0U};
        s = rng(s);
        s.s[0] += seed;
        return rng(s);
      }
      
      state rng(const state &s) {
        state new_s = s;
        new_s.s[0] = new_s.s[0] * PCG_DEFAULT_MULTIPLIER_128 + (uint128_t)(((intptr_t)&s) | 1u);
        return new_s;
      }
      
      uint128_t get_value(const state &s) {
        return pcg_output_xsl_rr_rr_128_128(s.s[0]);
      }
    }
    
    namespace pcg_xsl_rr_rr128setseq {
      state init(const uint128_t &seed, const uint128_t &initseq) {
        state s = {{0U}, (initseq << 1u) | 1u};
        s = rng(s);
        s.s[0] += seed;
        return rng(s);
      }
      
      state rng(state s) {
        s.s[0] = s.s[0] * PCG_DEFAULT_MULTIPLIER_128 + s.inc;
        return s;
      }
      
      uint128_t get_value(const state &s) {
        return pcg_output_xsl_rr_rr_128_128(s.s[0]);
      }
    }
    
    namespace cmwc {
      void init(state &s, const uint32_t &seed) {
        auto rand_state = mulberry32::init({seed});
        for (size_t i = 0; i < cycle; ++i) {
          s.Q[i] = mulberry32::get_value(rand_state);
          rand_state = mulberry32::rng(rand_state);
        }
        
        do {
          s.c = mulberry32::get_value(rand_state);
          rand_state = mulberry32::rng(rand_state);
        } while (s.c >= c_max);
        
        s.i = cycle - 1;
      }
      
      uint32_t get_value(state &s) {
        const uint64_t a = 18782;      // as Marsaglia recommends
        const uint32_t m = 0xfffffffe; // as Marsaglia recommends
        uint64_t t;
        uint32_t x;

        s.i = (s.i + 1) & (cycle - 1);
        t = a * s.Q[s.i] + s.c;
        /* Let c = t / 0xffffffff, x = t mod 0xffffffff */
        s.c = t >> 32;
        x = t + s.c;
        if (x < s.c) {
          x++;
          s.c++;
        }
        
        return s.Q[s.i] = m - x;
      }
    }
    
    namespace cmwc128 {
      state init(const uint64_t &seed) {
        state new_state;
        splitmix64::state splitmix_states[state_size];
        splitmix_states[0] = splitmix64::rng({seed});
        for (uint32_t i = 1; i < state_size; ++i) splitmix_states[i] = splitmix64::rng(splitmix_states[i-1]);
        for (uint32_t i = 0; i < state_size; ++i) new_state.s[i] = splitmix64::get_value(splitmix_states[i]);
        return new_state;
      }
      
      state rng(state s) {
        const uint128_t t = 0xff8fa3db04bb588e * (uint128_t)s.s[0] + s.s[1];
        s.s[0] = 0xd81fdde4eba3aae9 * (uint64_t)t;
        s.s[1] = (t + 0xadca32a7 * (uint128_t)s.s[0]) >> 64;
        return s;
      }
      
      uint64_t get_value(const state &s) {
        return s.s[0];
      }
    }
    
    namespace cmwc256 {
      state init(const uint64_t &seed) {
        state new_state;
        splitmix64::state splitmix_states[state_size];
        splitmix_states[0] = splitmix64::rng({seed});
        for (uint32_t i = 1; i < state_size; ++i) splitmix_states[i] = splitmix64::rng(splitmix_states[i-1]);
        for (uint32_t i = 0; i < state_size; ++i) new_state.s[i] = splitmix64::get_value(splitmix_states[i]);
        return new_state;
      }
      
      state rng(state s) {
        const uint128_t t = 0xff2a4b18846bbee2 * static_cast<uint128_t>(s.s[0]) + s.s[3];
        s.s[0] = s.s[1];
        s.s[1] = s.s[2];
        s.s[2] = 0x94d34db4cd59d099 * static_cast<uint64_t>(t);
        s.s[3] = (t + 0x96e36616f07c57 * static_cast<uint128_t>(s.s[2])) >> 64;
        return s;
      }
      
      uint64_t get_value(const state &s) {
        return s.s[2];
      }
    }
  }
}
