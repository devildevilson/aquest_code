#ifndef STACK_ALLOCATOR_H
#define STACK_ALLOCATOR_H

#include <cstddef>
#include <atomic>

// можно сделать двумя способами: стековая память и динамическая
// имеет смысл конечно делать динамику, но и статика будет полезной

namespace devils_engine {
  namespace utils {
    class stack_allocator {
    public:
      stack_allocator(const size_t &size, const size_t &aligment);
      stack_allocator(stack_allocator &&allocator);
      ~stack_allocator();
      
      void* alloc(const size_t &size);
      void clear();
      
      size_t size() const;
      size_t aligment() const;
      size_t allocated_size() const;
    private:
      char* m_memory;
      size_t m_size;
      size_t m_aligment;
      std::atomic<size_t> m_allocated;
    };
    
    // статику потом как нибудь
//     template <size_t N>
//     class stack_allocator {
//     public:
//       
//     };
  }
}

#endif
