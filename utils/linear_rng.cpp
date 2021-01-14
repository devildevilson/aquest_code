#include "linear_rng.h"

namespace devils_engine {
  namespace utils {
    double rng_normalize(const uint64_t &value) {
      union { uint64_t i; double d; } u;
      u.i = (UINT64_C(0x3FF) << 52) | (value >> 12);
      return u.d - 1.0;
    }
    
    static inline uint64_t rotl(const uint64_t x, int k) {
      return (x << k) | (x >> (64 - k));
    }
    
    namespace splitmix64 {
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
  }
}
