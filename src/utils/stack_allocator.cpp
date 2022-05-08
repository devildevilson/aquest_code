#include "stack_allocator.h"

#include "constexpr_funcs.h"

namespace devils_engine {
  namespace utils {
    stack_allocator::stack_allocator(const size_t &size, const size_t &aligment) : m_memory(new char[size]), m_size(size), m_aligment(aligment), m_allocated(0) {}
    stack_allocator::stack_allocator(stack_allocator &&allocator) : m_memory(allocator.m_memory), m_size(allocator.m_size), m_aligment(allocator.m_aligment), m_allocated(allocator.m_allocated.load()) {
      allocator.m_memory = nullptr;
      allocator.m_size = 0;
      allocator.m_allocated = 0;
    }
    
    stack_allocator::~stack_allocator() { delete [] m_memory; }
    
    void* stack_allocator::alloc(const size_t &size) {
      if (m_memory == nullptr) return nullptr;
      if (size == 0) return nullptr;
      const size_t offset = m_allocated.fetch_add(align_to(size, m_aligment));
      if (offset + size > m_size) return nullptr;
      return &m_memory[offset];
    }
    
    // нужно ли что то с памятью делать?
    void stack_allocator::clear() { m_allocated = 0; }
    size_t stack_allocator::size() const { return m_size; }
    size_t stack_allocator::aligment() const { return m_aligment; }
    size_t stack_allocator::allocated_size() const { return m_allocated; }
  }
}
