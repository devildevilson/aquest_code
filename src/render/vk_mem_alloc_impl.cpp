#define VMA_IMPLEMENTATION
#include "vk_mem_alloc/vk_mem_alloc.hpp"

namespace devils_engine {
  namespace render {
    vk::Device allocator_device(vma::Allocator allocator) { return (*allocator).m_hDevice; }
    vk::Instance allocator_instance(vma::Allocator allocator) { return (*allocator).m_hInstance; }
  }
}
