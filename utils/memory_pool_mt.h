#ifndef MEMORY_POOL_MT_H
#define MEMORY_POOL_MT_H

#include <cstddef>
#include <atomic>
#include <algorithm>
#include <chrono>
#include <thread>

#include "assert.h"

namespace devils_engine {
  namespace utils {
    template <typename T, size_t N>
    class memory_pool_mt {
    public:
      using value_ptr = T*;
      using memory_type = char;
      using memory_ptr = memory_type*;
      using size_type = size_t;
      using atomic_value_ptr = std::atomic<value_ptr>;
      using atomic_memory_ptr = std::atomic<memory_ptr>;
      
      static_assert(sizeof(T) >= sizeof(void*));
      
      memory_pool_mt() : allocated(0), free_slot(nullptr), memory(nullptr) { memory = allocate(nullptr); }
      memory_pool_mt(const memory_pool_mt<T, N> &pool) = delete;
      memory_pool_mt(memory_pool_mt<T, N> &&pool) {
        memory = pool.memory.exchange(nullptr);
        free_slot = pool.free_slot.exchange(nullptr);
        allocated = pool.allocated;
        pool.allocated = 0;
      }
      
      ~memory_pool_mt() { clear(); }
      
      memory_pool_mt<T, N> & operator=(const memory_pool_mt<T, N> &pool) = delete;
      void operator=(memory_pool_mt<T, N> &&pool) {
        memory = pool.memory.exchange(nullptr);
        free_slot = pool.free_slot.exchange(nullptr);
        allocated = pool.allocated;
        pool.allocated = 0;
      }
      
      template <typename... Args>
      value_ptr create(Args&& ...args) {
        auto ptr = find_memory();
        auto val = new (ptr) T(std::forward<Args>(args)...);
        return val;
      }
      
      void destroy(value_ptr ptr) {
        if (ptr == nullptr) return;
        
        ptr->~T();
        auto slot = reinterpret_cast<helper_slot*>(ptr);
        slot->next = free_slot.exchange(slot);
      }
      
      size_type size() const {
        const size_t type_aligment = alignof(T);
        const size_t final_size = align_to(N + sizeof(memory_ptr), type_aligment);
        size_t counter = 0;
        auto tmp = memory.load();
        while (tmp != nullptr) {
          auto nextBuffer = reinterpret_cast<memory_ptr*>(tmp)[0];
          counter += final_size;
          tmp = nextBuffer;
        }
        
        return counter;
      }
      
      void clear() {
        auto tmp = memory.load();
        while (tmp != nullptr) {
          auto nextBuffer = reinterpret_cast<memory_ptr*>(tmp)[0];
          delete [] tmp;
          tmp = nextBuffer;
        }
        
        memory = nullptr;
        free_slot = nullptr;
        allocated = 0;
      }
    private:
//       constexpr static size_t max(const size_t s1, const size_t s2) { return s1 > s2 ? s1 : s2; }
      
      struct alignas(alignof(T)) helper_slot {
        helper_slot* next;
      };
      
      using atomic_free_slot_ptr = std::atomic<helper_slot*>;
      
      size_t allocated;
      atomic_free_slot_ptr free_slot;
      atomic_memory_ptr memory;
      
      constexpr static size_t align_to(const size_t size, const size_t aligment) {
        return (size + aligment - 1) / aligment * aligment;
      }
      
      memory_ptr allocate(memory_ptr old_mem) {
        ASSERT(memory == nullptr);
        
        const size_t type_aligment = alignof(T);
        const size_t final_size = align_to(N + sizeof(memory_ptr), type_aligment);
        allocated = type_aligment; // нужно ли это сделать тут?
        auto mem = new memory_type[final_size];
        
        auto tmp = reinterpret_cast<memory_ptr*>(mem);
        tmp[0] = old_mem;
        
        return mem;
      }
      
      memory_ptr find_memory() {
        helper_slot* tmp = nullptr;
        if (!free_slot.compare_exchange_strong(tmp, nullptr)) {
          // в этом месте может быть удаление объекта
          // по идее все что мы потеряем - это создание одного объекта лишнего
          if (free_slot.compare_exchange_strong(tmp, tmp->next)) {
            return reinterpret_cast<memory_ptr>(tmp);
          }
        }

        // способ 1: кажется он самый адекватный, в этом случае мы имитируем мьютекс с помощью указателя памяти
        // слишком длинная блокировка? думаю что вряд ли это возможно
        // (другой способ в yacs pool)
        memory_ptr mem = nullptr;
        do {
          std::this_thread::sleep_for(std::chrono::microseconds(1)); // с этой строкой работает стабильнее
          mem = memory.exchange(nullptr);
        } while (mem == nullptr);

        if (allocated+sizeof(T) > N) {
          mem = allocate(mem);
        }

        const size_t place = allocated;
        allocated += sizeof(T);
        auto ptr = mem + place;

        memory = mem;
        
        return ptr;
      }
    };
  }
}

#endif
