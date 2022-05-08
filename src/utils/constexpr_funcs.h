#ifndef DEVILS_ENGINE_CONSTEXPR_FUNCS_H
#define DEVILS_ENGINE_CONSTEXPR_FUNCS_H

#include <cassert>
#include <cstdint>
#include <cstddef>
#include <string_view>

namespace devils_engine {
  constexpr size_t align_to(size_t memory, size_t aligment) {
    return (memory + aligment - 1) / aligment * aligment;
  }
  
  constexpr int64_t ceil(double f) {
    const int64_t i = static_cast<int64_t>(f);
    return f > i ? i + 1 : i;
  }

  constexpr int64_t floor(double f) {
    const int64_t i = static_cast<int64_t>(f);
    return f < i ? i - 1 : i;
  }
  
  constexpr size_t divide_token(const std::string_view &str, const std::string_view &symbol, const size_t &max_count, std::string_view* array) {
    size_t current = 0;
    size_t prev = 0;
    size_t counter = 0;
    const size_t symbols_count = symbol.length();
    while (current != std::string_view::npos && counter < max_count) {
      current = str.find(symbol, prev);
      const size_t part_count = counter+1 == max_count ? std::string_view::npos : current-prev;
      const auto part = str.substr(prev, part_count);
      array[counter] = part;
      ++counter;
      prev = current + symbols_count;
    }
    
    counter = counter < max_count ? counter : SIZE_MAX;
    
    return counter;
  }
  
  constexpr size_t rdivide_token(const std::string_view &str, const std::string_view &symbol, const size_t &max_count, std::string_view* array) {
    size_t current = 0;
    size_t prev = std::string_view::npos;
    size_t counter = 0;
    const size_t symbols_count = symbol.length();
    while (current != std::string_view::npos && counter < max_count) {
      current = str.rfind(symbol, prev-1);
      const size_t substr_start = counter+1 == max_count || current == std::string_view::npos ? 0 : current+symbols_count;
      const size_t part_count = prev-substr_start;
      const auto part = str.substr(substr_start, part_count);
      array[counter] = part;
      ++counter;
      prev = current;
    }
    
    counter = counter < max_count ? counter : SIZE_MAX;
    
    return counter;
  }
  
  template <typename T>
  constexpr const T & max(const T &first) { return first; }
  
  template <typename T, typename... Ts>
  constexpr const T & max(const T &first, const Ts &... other) {
    const auto &second = max(other...);
    return second < first ? first : second;
  }
  
  template <typename T>
  constexpr const T & min(const T &first) { return first; }
  
  template <typename T, typename... Ts>
  constexpr const T & min(const T &first, const Ts &... other) {
    const auto &second = min(other...);
    return first < second ? first : second;
  }
  
  constexpr size_t count_useful_bits(size_t number) {
    size_t i = 0;
    for (; number != 0; number = number >> 1, ++i);
    return i;
  }
  
  constexpr size_t count_bits(size_t number) {
    const size_t size_width = SIZE_WIDTH;
    size_t counter = 0;
    for (size_t i = 0; i < size_width; counter += size_t(bool(number & (size_t(1) << i))), ++i);
    return counter;
  }
  
  constexpr size_t make_mask(const size_t bits_count) {
    const size_t size_width = SIZE_WIDTH;
    size_t mask = 0;
    for (size_t i = 0; i < min(size_width, bits_count); ++i) { mask |= size_t(1) << i; }
    return mask;
  }
  
  constexpr uint64_t make_64bit(const uint32_t &first, const uint32_t &second) {
    return (uint64_t(first) << 32) | uint64_t(second);
  }

  namespace detail {

/* default: SipHash-2-4 */
#ifndef cROUNDS
#define cROUNDS 2
#endif
#ifndef dROUNDS
#define dROUNDS 4
#endif

    constexpr uint64_t ROTL(const uint64_t x, const uint64_t b) {
      return (((x) << (b)) | ((x) >> (64 - (b))));
    }

    constexpr void U64TO8_LE(const uint64_t v, uint8_t* p) {
      p[0] = uint8_t(v >> 0);
      p[1] = uint8_t(v >> 8);
      p[2] = uint8_t(v >> 16);
      p[3] = uint8_t(v >> 24);
      p[4] = uint8_t(v >> 32);
      p[5] = uint8_t(v >> 40);
      p[6] = uint8_t(v >> 48);
      p[7] = uint8_t(v >> 56);
    }

    constexpr uint8_t to_u8(const char c) { return uint8_t(c); }
    constexpr uint8_t to_u8(const uint8_t c) { return uint8_t(c); }
    constexpr uint64_t to_u64(const char c) { return uint64_t(to_u8(c)); }

    constexpr uint64_t U8TO64_LE(const char* data) {
      return  uint64_t(to_u8(data[0]))        | (uint64_t(to_u8(data[1])) << 8)  | (uint64_t(to_u8(data[2])) << 16) |
            (uint64_t(to_u8(data[3])) << 24) | (uint64_t(to_u8(data[4])) << 32) | (uint64_t(to_u8(data[5])) << 40) |
            (uint64_t(to_u8(data[6])) << 48) | (uint64_t(to_u8(data[7])) << 56);
    }

    constexpr uint64_t U8TO64_LE(const uint8_t* data) {
      return  uint64_t(data[0])        | (uint64_t(data[1]) << 8)  | (uint64_t(data[2]) << 16) |
            (uint64_t(data[3]) << 24) | (uint64_t(data[4]) << 32) | (uint64_t(data[5]) << 40) |
            (uint64_t(data[6]) << 48) | (uint64_t(data[7]) << 56);
    }

#define SIPROUND                                                               \
    do {                                                                       \
        v0 += v1;                                                              \
        v1 = ROTL(v1, 13);                                                     \
        v1 ^= v0;                                                              \
        v0 = ROTL(v0, 32);                                                     \
        v2 += v3;                                                              \
        v3 = ROTL(v3, 16);                                                     \
        v3 ^= v2;                                                              \
        v0 += v3;                                                              \
        v3 = ROTL(v3, 21);                                                     \
        v3 ^= v0;                                                              \
        v2 += v1;                                                              \
        v1 = ROTL(v1, 17);                                                     \
        v1 ^= v2;                                                              \
        v2 = ROTL(v2, 32);                                                     \
    } while (0)

#ifdef DEBUG
#include <stdio.h>
#define TRACE                                                                  \
    do {                                                                       \
        printf("(%3zu) v0 %016" PRIx64 "\n", inlen, v0);                       \
        printf("(%3zu) v1 %016" PRIx64 "\n", inlen, v1);                       \
        printf("(%3zu) v2 %016" PRIx64 "\n", inlen, v2);                       \
        printf("(%3zu) v3 %016" PRIx64 "\n", inlen, v3);                       \
    } while (0)
#else
#define TRACE
#endif

    constexpr int siphash(const std::string_view& in_str, const uint8_t* k, const size_t outlen, uint8_t* out) {
      assert((outlen == 8) || (outlen == 16));
      uint64_t v0 = UINT64_C(0x736f6d6570736575);
      uint64_t v1 = UINT64_C(0x646f72616e646f6d);
      uint64_t v2 = UINT64_C(0x6c7967656e657261);
      uint64_t v3 = UINT64_C(0x7465646279746573);
      uint64_t k0 = U8TO64_LE(k);
      uint64_t k1 = U8TO64_LE(k + 8);
      uint64_t m = 0;
      const size_t inlen = in_str.size();
      //const unsigned char* end = ni + inlen - (inlen % sizeof(uint64_t));
      const size_t end = inlen - (inlen % sizeof(uint64_t));
      const auto ni_end = in_str.substr(end);
      const int left = inlen & 7;
      uint64_t b = inlen << 56;
      v3 ^= k1;
      v2 ^= k0;
      v1 ^= k1;
      v0 ^= k0;

      if (outlen == 16) v1 ^= 0xee;

      for (size_t i = 0; i < end; i += 8) {
        m = U8TO64_LE(&in_str[i]);
        v3 ^= m;

        TRACE;
        for (size_t i = 0; i < cROUNDS; ++i) { SIPROUND; }
        v0 ^= m;
      }

      switch (left) {
        case 7: b |= (to_u64(ni_end[6])) << 48; [[fallthrough]];
        case 6: b |= (to_u64(ni_end[5])) << 40; [[fallthrough]];
        case 5: b |= (to_u64(ni_end[4])) << 32; [[fallthrough]];
        case 4: b |= (to_u64(ni_end[3])) << 24; [[fallthrough]];
        case 3: b |= (to_u64(ni_end[2])) << 16; [[fallthrough]];
        case 2: b |= (to_u64(ni_end[1])) << 8;  [[fallthrough]];
        case 1: b |= (to_u64(ni_end[0])); break;
        case 0: break;
      }

      v3 ^= b;

      TRACE;
      for (size_t i = 0; i < cROUNDS; ++i) {
        SIPROUND;
      }

      v0 ^= b;

      if (outlen == 16) v2 ^= 0xee;
      else v2 ^= 0xff;

      TRACE;
      for (size_t i = 0; i < dROUNDS; ++i) {
        SIPROUND;
      }

      b = v0 ^ v1 ^ v2 ^ v3;
      U64TO8_LE(b, out);

      if (outlen == 8) return 0;

      v1 ^= 0xdd;

      TRACE;
      for (size_t i = 0; i < dROUNDS; ++i) {
        SIPROUND;
      }

      b = v0 ^ v1 ^ v2 ^ v3;
      U64TO8_LE(b, out + 8);

      return 0;
    }

    constexpr uint64_t murmur_hash64A(const std::string_view& in_str, const uint64_t seed) {
      constexpr uint64_t m = 0xc6a4a7935bd1e995LLU;
      constexpr int r = 47;
      const size_t len = in_str.size();
      const size_t end = len - (len % sizeof(uint64_t));

      uint64_t h = seed ^ (len * m);

      for (size_t i = 0; i < end; i += 8) {
        uint64_t k = U8TO64_LE(&in_str[i]);
        k *= m;
        k ^= k >> r;
        k *= m;

        h ^= k;
        h *= m;
      }

      const auto key_end = in_str.substr(end);
      const int left = len & 7;
      switch (left) {
        case 7: h ^= to_u64(key_end[6]) << 48; [[fallthrough]];
        case 6: h ^= to_u64(key_end[5]) << 40; [[fallthrough]];
        case 5: h ^= to_u64(key_end[4]) << 32; [[fallthrough]];
        case 4: h ^= to_u64(key_end[3]) << 24; [[fallthrough]];
        case 3: h ^= to_u64(key_end[2]) << 16; [[fallthrough]];
        case 2: h ^= to_u64(key_end[1]) << 8;  [[fallthrough]];
        case 1: h ^= to_u64(key_end[0]);
          h *= m;
      };

      h ^= h >> r;
      h *= m;
      h ^= h >> r;

      return h;
    }
    
    constexpr uint64_t fnv_basis = 14695981039346656037ull;
    constexpr uint64_t fnv_prime = 1099511628211ull;
    constexpr uint64_t old_fnv1a(const std::string_view &str) {
      uint64_t current_hash = fnv_basis;
      for (const char c : str) { current_hash = (current_hash ^ c) * fnv_prime; }
      return current_hash;
    }
    
    constexpr size_t fnv1a(const std::string_view &str) {
      static_assert(sizeof(size_t) == 8);
      size_t result = 0xcbf29ce484222325; // FNV offset basis

      for (const char c : str) {
        result ^= c;
        result *= 1099511628211; // FNV prime
      }

      return result;
    }
  }

  constexpr int siphash(const std::string_view &in_str, const uint8_t* k, const size_t outlen, uint8_t* out) {
    return detail::siphash(in_str, k, outlen, out);
  }
  
  constexpr uint64_t default_murmur_seed = 14695981039346656037ull;
  constexpr uint64_t murmur_hash64A(const std::string_view &in_str, const uint64_t seed) {
    return detail::murmur_hash64A(in_str, seed);
  }

  constexpr size_t fnv1a(const std::string_view &str) {
    return detail::fnv1a(str);
  }
  
  constexpr uint64_t copy_u8(const uint8_t* data) {
    return  uint64_t(data[0])        | (uint64_t(data[1]) << 8)  | (uint64_t(data[2]) << 16) |
           (uint64_t(data[3]) << 24) | (uint64_t(data[4]) << 32) | (uint64_t(data[5]) << 40) |
           (uint64_t(data[6]) << 48) | (uint64_t(data[7]) << 56);
  }
  
  constexpr size_t string_hash(const std::string_view &str) {
//     constexpr uint8_t k[] = { 123, 211, 11, 213, 125, 111, 100, 189, 181, 168, 174, 34, 45, 13, 1, 34 };
//     uint8_t data[8] = {0};
//     siphash(str, k, 8, data);
//     const size_t out = copy_u8(data);
    const size_t out = murmur_hash64A(str, default_murmur_seed);
    //const size_t out = fnv1a(str);
    return out;
  }
}

#endif
