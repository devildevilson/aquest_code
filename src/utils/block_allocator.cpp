#include "block_allocator.h"

#include <algorithm>
#include "utils/assert.h"

size_t aligned_size(const size_t &old_size, const size_t &align) {
  return (((old_size-1) / align) + 1) * align;
}

namespace devils_engine {
  namespace utils {
    block_allocator::block_allocator(const size_t &block_size, const size_t &block_align, const size_t &memory_size) : 
      align(std::max(block_align, sizeof(char*))), 
      block_size(aligned_size(block_size, align)), 
      current_memory(0),
      memory_size(aligned_size(memory_size, align)),
      memory(nullptr),
      free_memory(nullptr)
    {
      create_new_memory();
    }
    
    block_allocator::~block_allocator() {
      char* current = memory;
      while (current != nullptr) {
        char* tmp = reinterpret_cast<char**>(current)[0];
        delete [] current;
        current = tmp;
      }
    }
    
    void* block_allocator::allocate() {
      if (free_memory != nullptr) {
        void* tmp = free_memory;
        free_memory = reinterpret_cast<char**>(free_memory)[0];
        return tmp;
      }
      
      if (current_memory + align >= memory_size) {
        create_new_memory();
      }
      
      char* mem = &memory[current_memory + align];
      current_memory += block_size;
      return mem;
    }
    
    void block_allocator::free(void* mem) {
      reinterpret_cast<void**>(mem)[0] = free_memory;
      free_memory = mem;
    }
    
    void block_allocator::create_new_memory() {
      char* new_mem = new char[memory_size + align];
      reinterpret_cast<char**>(new_mem)[0] = memory;
      memory = new_mem;
    }
  }
}
