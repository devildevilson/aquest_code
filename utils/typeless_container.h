#ifndef TYPELESS_CONTAINER_H
#define TYPELESS_CONTAINER_H

#include "assert.h"

#include "stack_allocator.h"

//#include <iostream>

namespace devils_engine {
  namespace utils {
    class typeless_container {
    public:
      inline typeless_container(const size_t &size, const size_t &aligment) : allocator(size, aligment) {}
      inline typeless_container(typeless_container &&cont) : allocator(std::move(cont.allocator)) {}

      template <typename T, typename... Args>
      T* create(Args&&... args) {
        void* mem = allocator.alloc(sizeof(T));
        //std::cout << "alloc " << sizeof(T) << " current size " << allocator.allocated_size() << '\n';
        assert(mem != nullptr && "Not enough allocator size");
        T* ptr = new (mem) T(std::forward<Args>(args)...);
        return ptr;
      }

      // аллокатор возвращающий память это new и delete, сомневаюсь что мне нужно заморачиваться с этим
      template <typename T>
      void destroy(T* ptr) {
        ptr->~T(); // проверить что эта память из этого диапазона?
      }

      inline void clear() { allocator.clear(); }
      inline size_t size() const { return allocator.size(); }
      inline size_t allocated_size() const { return allocator.allocated_size(); }
    private:
      stack_allocator allocator;
    };
  }
}

#endif
