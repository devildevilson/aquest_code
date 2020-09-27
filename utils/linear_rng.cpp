#include "linear_rng.h"

namespace devils_engine {
  namespace utils {
    namespace splitmix64 {
      state rng(const state &s) {
        return s + 0x9e3779b97f4a7c15;
      }
      
      uint64_t get_value(const state &s) {
        uint64_t z = s;
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
        z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
        return z ^ (z >> 31);
      }
      
      double normalize(const uint64_t &value) {
        union { uint64_t i; double d; } u; 
        u.i = UINT64_C(0x3FF) << 52 | value >> 12;
        return u.d - 1.0;
      }
    }
    
    namespace xorshift64 {
      state rng(const state &s) {
        state x = s;
        x ^= x << 13;
        x ^= x >> 7;
        x ^= x << 17;
        return x;
      }
      
      uint64_t get_value(const state &s) {
        return s;
      }
      
      double normalize(const uint64_t &value) {
        union { uint64_t i; double d; } u;
        u.i = UINT64_C(0x3FF) << 52 | value >> 12;
        return u.d - 1.0;
      }
    }
    
    namespace xoroshiro128starstar {
      static inline uint64_t rotl(const uint64_t x, int k) {
        return (x << k) | (x >> (64 - k));
      }
      
      state rng(const state &s) {
        state next = s;
        const uint64_t s0 = s.s[0];
        uint64_t s1 = s.s[1];

        s1 ^= s0;
        next.s[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16); // a, b
        next.s[1] = rotl(s1, 37); // c

        return next;
      }
      
      uint64_t get_value(const state &s) {
        return rotl(s.s[0] * 5, 7) * 9;
      }
      
      double normalize(const uint64_t &value) {
        union { uint64_t i; double d; } u;
        u.i = UINT64_C(0x3FF) << 52 | value >> 12;
        return u.d - 1.0;
      }
    }
    
    namespace xoshiro256plusplus {
      static inline uint64_t rotl(const uint64_t x, int k) {
        return (x << k) | (x >> (64 - k));
      }
      
      state rng(const state &s) {
        state next = s;
        const uint64_t t = next.s[1] << 17;

        next.s[2] ^= next.s[0];
        next.s[3] ^= next.s[1];
        next.s[1] ^= next.s[2];
        next.s[0] ^= next.s[3];

        next.s[2] ^= t;

        next.s[3] = rotl(next.s[3], 45);

        return next;
      }
      
      uint64_t get_value(const state &s) {
        return rotl(s.s[0] + s.s[3], 23) + s.s[0];
      }
      
      double normalize(const uint64_t &value) {
        union { uint64_t i; double d; } u;
        u.i = UINT64_C(0x3FF) << 52 | value >> 12;
        return u.d - 1.0;
      }
    }
  }
}
