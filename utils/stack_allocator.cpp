#include "stack_allocator.h"

namespace devils_engine {
  namespace utils {
    stack_allocator::stack_allocator(const size_t &size) : m_memory(new char[size]), m_size(size), m_allocated(0) {}
    stack_allocator::stack_allocator(stack_allocator &&allocator) : m_memory(allocator.m_memory), m_size(allocator.m_size), m_allocated(allocator.m_allocated.load()) {
      allocator.m_memory = nullptr;
      allocator.m_size = 0;
      allocator.m_allocated = 0;
    }
    
    stack_allocator::~stack_allocator() { delete [] m_memory; }
    
    void* stack_allocator::alloc(const size_t &size) {
      if (m_memory == nullptr) return nullptr;
      if (size == 0) return nullptr;
      const size_t index = m_allocated.fetch_add(size);
      if (index + size > m_size) return nullptr;
      return &m_memory[index];
    }
    
    void stack_allocator::clear() {
      // нужно ли что то с памятью делать?
      m_allocated = 0;
    }
    
    size_t stack_allocator::size() const {
      return m_size;
    }
    
    size_t stack_allocator::allocated_size() const {
      return m_allocated;
    }
  }
}
