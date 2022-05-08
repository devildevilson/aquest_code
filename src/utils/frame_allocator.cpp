#include "frame_allocator.h"

#include <stdexcept>
#include "constexpr_funcs.h"
#include <cstring>

namespace devils_engine {
  namespace utils {
    frame_allocator::frame_allocator(const size_t &size) : m_memory(new char[size]), m_size(size), m_current(0) { memset(m_memory, 0, m_size); }
    frame_allocator::~frame_allocator() { delete [] m_memory; m_memory = nullptr; }
    void* frame_allocator::allocate(const size_t &size) {
      const size_t final_size = align_to(size, default_aligment);
      const size_t prev = m_current.fetch_add(final_size);
      if (prev > this->size() || this->size() - prev < final_size) throw std::runtime_error("Could not allocate " + std::to_string(size) + " from frame allocator");
      return &m_memory[prev];
    }
    
    size_t frame_allocator::size() const { return m_size; }
    size_t frame_allocator::allocated() const { return m_current; }
    void frame_allocator::reset() { m_current = 0; }
  }
}
