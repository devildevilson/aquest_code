#include "salsa20.h"

#include <cstring>
#include <cassert>

namespace devils_engine {
  namespace cipher {
    salsa20::salsa20() {
      memset(vector, 0, sizeof(vector));
      //set_key(key);
    }

    //----------------------------------------------------------------------------------
    void salsa20::set_key(const uint8_t* key) {
      static const char constants[] = "expand 32-byte k";
      if(key == nullptr) return;

      vector[0] = convert(reinterpret_cast<const uint8_t*>(&constants[0]));
      vector[1] = convert(&key[0]);
      vector[2] = convert(&key[4]);
      vector[3] = convert(&key[8]);
      vector[4] = convert(&key[12]);
      vector[5] = convert(reinterpret_cast<const uint8_t*>(&constants[4]));

      std::memset(&vector[6], 0, 4 * sizeof(uint32_t));

      vector[10] = convert(reinterpret_cast<const uint8_t*>(&constants[8]));
      vector[11] = convert(&key[16]);
      vector[12] = convert(&key[20]);
      vector[13] = convert(&key[24]);
      vector[14] = convert(&key[28]);
      vector[15] = convert(reinterpret_cast<const uint8_t*>(&constants[12]));
    }

    //----------------------------------------------------------------------------------
    void salsa20::set_iv(const uint8_t* iv) {
      if(iv == nullptr) return;
      vector[6] = convert(&iv[0]);
      vector[7] = convert(&iv[4]);
      vector[8] = vector[9] = 0;
    }

    //----------------------------------------------------------------------------------
    void salsa20::generate_key_stream(uint8_t output[block_size])
    {
      uint32_t x[vector_size];
      memcpy(x, vector, sizeof(vector));

      for(int32_t i = 20; i > 0; i -= 2) {
        x[4 ] ^= rotate(static_cast<uint32_t>(x[0 ] + x[12]),  7);
        x[8 ] ^= rotate(static_cast<uint32_t>(x[4 ] + x[0 ]),  9);
        x[12] ^= rotate(static_cast<uint32_t>(x[8 ] + x[4 ]), 13);
        x[0 ] ^= rotate(static_cast<uint32_t>(x[12] + x[8 ]), 18);
        x[9 ] ^= rotate(static_cast<uint32_t>(x[5 ] + x[1 ]),  7);
        x[13] ^= rotate(static_cast<uint32_t>(x[9 ] + x[5 ]),  9);
        x[1 ] ^= rotate(static_cast<uint32_t>(x[13] + x[9 ]), 13);
        x[5 ] ^= rotate(static_cast<uint32_t>(x[1 ] + x[13]), 18);
        x[14] ^= rotate(static_cast<uint32_t>(x[10] + x[6 ]),  7);
        x[2 ] ^= rotate(static_cast<uint32_t>(x[14] + x[10]),  9);
        x[6 ] ^= rotate(static_cast<uint32_t>(x[2 ] + x[14]), 13);
        x[10] ^= rotate(static_cast<uint32_t>(x[6 ] + x[2 ]), 18);
        x[3 ] ^= rotate(static_cast<uint32_t>(x[15] + x[11]),  7);
        x[7 ] ^= rotate(static_cast<uint32_t>(x[3 ] + x[15]),  9);
        x[11] ^= rotate(static_cast<uint32_t>(x[7 ] + x[3 ]), 13);
        x[15] ^= rotate(static_cast<uint32_t>(x[11] + x[7 ]), 18);
        x[1 ] ^= rotate(static_cast<uint32_t>(x[0 ] + x[3 ]),  7);
        x[2 ] ^= rotate(static_cast<uint32_t>(x[1 ] + x[0 ]),  9);
        x[3 ] ^= rotate(static_cast<uint32_t>(x[2 ] + x[1 ]), 13);
        x[0 ] ^= rotate(static_cast<uint32_t>(x[3 ] + x[2 ]), 18);
        x[6 ] ^= rotate(static_cast<uint32_t>(x[5 ] + x[4 ]),  7);
        x[7 ] ^= rotate(static_cast<uint32_t>(x[6 ] + x[5 ]),  9);
        x[4 ] ^= rotate(static_cast<uint32_t>(x[7 ] + x[6 ]), 13);
        x[5 ] ^= rotate(static_cast<uint32_t>(x[4 ] + x[7 ]), 18);
        x[11] ^= rotate(static_cast<uint32_t>(x[10] + x[9 ]),  7);
        x[8 ] ^= rotate(static_cast<uint32_t>(x[11] + x[10]),  9);
        x[9 ] ^= rotate(static_cast<uint32_t>(x[8 ] + x[11]), 13);
        x[10] ^= rotate(static_cast<uint32_t>(x[9 ] + x[8 ]), 18);
        x[12] ^= rotate(static_cast<uint32_t>(x[15] + x[14]),  7);
        x[13] ^= rotate(static_cast<uint32_t>(x[12] + x[15]),  9);
        x[14] ^= rotate(static_cast<uint32_t>(x[13] + x[12]), 13);
        x[15] ^= rotate(static_cast<uint32_t>(x[14] + x[13]), 18);
      }

      for(size_t i = 0; i < vector_size; ++i) {
        x[i] += vector[i];
        convert(x[i], &output[4 * i]);
      }

      ++vector[8];
      vector[9] += vector[8] == 0 ? 1 : 0;
    }

    void salsa20::process_blocks(const uint8_t* input, uint8_t* output, size_t num_blocks) {
      assert(input != nullptr && output != nullptr);

      uint8_t key_stream[block_size];

      for(size_t i = 0; i < num_blocks; ++i) {
        generate_key_stream(key_stream);

        for(size_t j = 0; j < block_size; ++j) {
          *(output++) = key_stream[j] ^ *(input++);
        }
      }
    }

    void salsa20::process_bytes(const uint8_t* input, uint8_t* output, size_t num_bytes) {
      assert(input != nullptr && output != nullptr);

      uint8_t key_stream[block_size];
      size_t num_bytes_to_process;

      while(num_bytes != 0) {
        generate_key_stream(key_stream);
        num_bytes_to_process = num_bytes >= block_size ? block_size : num_bytes;

        for(size_t i = 0; i < num_bytes_to_process; ++i, --num_bytes) {
          *(output++) = key_stream[i] ^ *(input++);
        }
      }
    }

    uint32_t salsa20::rotate(uint32_t value, uint32_t num_bits) {
      return (value << num_bits) | (value >> (32 - num_bits));
    }

    void salsa20::convert(uint32_t value, uint8_t* array) {
      array[0] = static_cast<uint8_t>(value >> 0);
      array[1] = static_cast<uint8_t>(value >> 8);
      array[2] = static_cast<uint8_t>(value >> 16);
      array[3] = static_cast<uint8_t>(value >> 24);
    }

    uint32_t salsa20::convert(const uint8_t* array) {
      return ((static_cast<uint32_t>(array[0]) << 0)  |
              (static_cast<uint32_t>(array[1]) << 8)  |
              (static_cast<uint32_t>(array[2]) << 16) |
              (static_cast<uint32_t>(array[3]) << 24));
    }
  }
}
