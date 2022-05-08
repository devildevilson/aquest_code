#ifndef SALSA20_H
#define SALSA20_H

#include <climits>
#include <cstdint>
#include <cstddef>

namespace devils_engine {
  namespace cipher {
    using std::size_t;
    using std::int32_t;
    using std::uint8_t;
    using std::uint32_t;

    class salsa20 {
    public:
      static const size_t vector_size = 16;
      static const size_t block_size = 64;
      static const size_t key_size = 32;
      static const size_t iv_size = 8;

      salsa20();
      salsa20(const salsa20 &) = default;
      salsa20(salsa20&&) = default;
      ~salsa20() = default;
      salsa20 & operator=(const salsa20&) = default;
      salsa20 & operator=(salsa20&&) = default;

      void set_key(const uint8_t* key); // 256 bit
      void set_iv(const uint8_t* iv); // 64 bit
      void generate_key_stream(uint8_t output[block_size]);
      void process_blocks(const uint8_t* input, uint8_t* output, size_t num_blocks);
      void process_bytes(const uint8_t* input, uint8_t* output, size_t num_bytes); // используем только в конце (!) шифровки
    private:
      uint32_t vector[vector_size];

      uint32_t rotate(uint32_t value, uint32_t num_bits);
      void convert(uint32_t value, uint8_t* array); // 32 bit
      uint32_t convert(const uint8_t* array); // 32 bit
    };
  }
}

#endif
