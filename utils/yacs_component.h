#ifndef YACS_COMPONENT_H
#define YACS_COMPONENT_H

#include <cstdint>
#include <utility>

#include "assert.h"

namespace yacs {
  template <typename T>
  struct component_storage { //alignas(alignof(T))
    static size_t type;
    T obj;
    
    template <typename... Args>
    component_storage(Args&& ...args) : obj(std::forward<Args>(args)...) {}
    T* ptr() { return &obj; }
    const T* ptr() const { return &obj; }
  };
  
//   class base_component_storage {
//   public:
//     base_component_storage() = default;
//     virtual ~base_component_storage() = default;
// 
//     //virtual void destroy() = 0;
//     size_t & index() { return componentIndex; }
//   private:
//     size_t componentIndex; // может не получиться индекс, но можно тогда сюда добавить указатель на энтити
//   };
// 
//   // дополнительные 16 байт к каждому объекту, можно ли сократить?
//   // убрать componentIndex, но тогда удаление будет O(n)
//   template <typename T>
//   class component_storage : public base_component_storage {
//   public:
//     static size_t type;
// 
//     template <typename... Args>
//     component_storage(Args&& ...args) : data(std::forward<Args>(args)...) {}
//     ~component_storage() = default;
// 
//     T* ptr() { return &data; }
//     const T* ptr() const { return &data; }
// 
//     T & ref() { return data; }
//     const T & ref() const { return data; }
//   private:
//     T data;
//   };

  template <typename T>
  size_t component_storage<T>::type = SIZE_MAX;
//   template <typename T>
//   size_t component_storage<T>::type = SIZE_MAX;

  template <typename T>
  class component_handle {
  public:
    component_handle() : ptr(nullptr) {}
    component_handle(T* ptr) : ptr(ptr) {}

    bool valid() const { return ptr != nullptr; }

    T* get() { return ptr; }
    const T* get() const { return ptr; }

    T* operator->() { return ptr; }
    const T* operator->() const { return ptr; }

    bool operator==(const component_handle &handle) const {
      return ptr == handle.get();
    }

    bool operator!=(const component_handle &handle) const {
      return ptr != handle.get();
    }
  private:
    T* ptr;
  };

  template <typename T>
  class const_component_handle {
  public:
    const_component_handle() : ptr(nullptr) {}
    const_component_handle(const T* ptr) : ptr(ptr) {}

    bool valid() const { return ptr != nullptr; }

    const T* get() const { return ptr; }

    const T* operator->() const { return ptr; }

    bool operator==(const const_component_handle &handle) const {
      return ptr == handle.get();
    }

    bool operator!=(const const_component_handle &handle) const {
      return ptr != handle.get();
    }
  private:
    const T* ptr;
  };
}

#endif //YACS_COMPONENT_H
