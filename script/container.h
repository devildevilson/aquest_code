#ifndef DEVILS_ENGINE_SCRIPT_CONTAINER_H
#define DEVILS_ENGINE_SCRIPT_CONTAINER_H

#include <stdexcept>
#include "utils/constexpr_funcs.h"
#include "utils/assert.h"
// #include "utils/typeless_container.h"

namespace devils_engine {
  namespace script {
    class interface;
    
    class container {
    public:
      template <typename T>
      class delayed_initialization {
      public:
        delayed_initialization(void* ptr) noexcept : ptr(ptr), inited(false) {}
        
        template <typename... Args>
        T* init(Args&& ...args) {
          if (inited) throw std::runtime_error("Memory is already inited");
          auto obj = new (ptr) T(std::forward<Args>(args)...);
          inited = true;
          return obj;
        }
      private:
        void* ptr;
        bool inited;
      };
      
      container() noexcept;
      container(const size_t &size, const size_t &aligment) noexcept;
      container(container &&move) noexcept;
      container(const container &copy) = delete;
      ~container() noexcept;
      container & operator=(const container &copy) = delete;
      container & operator=(container &&move) noexcept;
      
      void init(const size_t &size, const size_t &aligment) noexcept;
      
      template <typename T, typename... Args>
      T* add(Args&& ...args) {
        ASSERT(memory != nullptr);
        ASSERT(offset + align_to(sizeof(T), aligment) <= size);
        auto ptr = &memory[offset];
        auto obj = new (ptr) T(std::forward<Args>(args)...);
        offset += align_to(sizeof(T), aligment);
        return obj;
      }
      
      template <typename T>
      size_t add_delayed() {
        ASSERT(memory != nullptr);
        ASSERT(offset + align_to(sizeof(T), aligment) <= size);
        const size_t cur = offset;
        offset += align_to(sizeof(T), aligment);
        return cur;
      }
      
      template <typename T>
      delayed_initialization<T> get_init(const size_t &offset) {
        ASSERT(memory != nullptr);
        ASSERT(offset + align_to(sizeof(T), aligment) <= size);
        auto cur = &memory[offset];
        return delayed_initialization<T>(cur);
      }
      
      size_t mem_size() const;
    private:
      size_t size;
      size_t offset;
      size_t aligment;
      char* memory;
    };
  }
}

#endif
