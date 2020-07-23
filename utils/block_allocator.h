#ifndef BLOCK_ALLOCATOR_H
#define BLOCK_ALLOCATOR_H

#include <cstddef>
#include <cstdint>

namespace devils_engine {
  namespace utils {
    class block_allocator {
    public:
      block_allocator(const size_t &block_size, const size_t &block_align, const size_t &memory_size);
      ~block_allocator();
      
      void* allocate();
      void free(void* mem);
    private:
      size_t align;
      size_t block_size;
      size_t current_memory;
      size_t memory_size;
      char* memory;
      void* free_memory;
      
      void create_new_memory();
    };
  }
}

#endif
