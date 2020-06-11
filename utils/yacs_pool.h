#ifndef YACS_POOL_H
#define YACS_POOL_H

#include <cstdint>
#include <algorithm>

#ifdef YACS_MULTITHREADING
#include <atomic>
#include <thread>

#ifndef YACS_THREADSAFE_ARRAY_DEFAULT_CAPACITY
#define YACS_THREADSAFE_ARRAY_DEFAULT_CAPACITY 20
#endif // YACS_THREADSAFE_ARRAY_DEFAULT_CAPACITY

#ifndef YACS_THREADSAFE_ARRAY_GROWTH_POLICY
#define YACS_THREADSAFE_ARRAY_GROWTH_POLICY 2
#endif // YACS_THREADSAFE_ARRAY_GROWTH_POLICY
#endif // YACS_MULTITHREADING

#include "yacs_component.h"

namespace yacs {
  class typeless_pool {
  public:
    typeless_pool(const size_t &typeId, const size_t &blockSize) : typeId(typeId), blockSize(blockSize), currentSize(0), memory(nullptr), freeSlots(nullptr) {
      allocateBlock();
    }

    typeless_pool(typeless_pool &&pool) : typeId(pool.typeId), blockSize(pool.blockSize), currentSize(pool.currentSize), memory(pool.memory), freeSlots(pool.freeSlots) {
      pool.memory = nullptr;
      pool.currentSize = 0;
      pool.freeSlots = nullptr;
    }

    ~typeless_pool() {
      char* tmp = memory;
      while (tmp != nullptr) {
        char** ptr = reinterpret_cast<char**>(tmp);
        char* nextBuffer = ptr[0];

        delete [] tmp;
        tmp = nextBuffer;
      }
    }

    template<typename T, typename... Args>
    T* create(Args&&... args) {
      ASSERT(T::type == typeId && "Wrong object type for typeless pool");

      T* ptr = nullptr;

      if (freeSlots != nullptr) {
        ptr = reinterpret_cast<T*>(freeSlots);
        freeSlots = freeSlots->next;
      } else {
        if (currentSize + std::max(sizeof(T), sizeof(_Slot)) > blockSize) allocateBlock();

        ptr = reinterpret_cast<T*>(memory+sizeof(char*)+currentSize);
        currentSize += std::max(sizeof(T), sizeof(_Slot));
      }

      new (ptr) T(std::forward<Args>(args)...);

      return ptr;
    }

    template<typename T>
    void destroy(T* ptr) {
      if (ptr == nullptr) return;
      ASSERT(T::type == typeId && "Wrong object type for typeless pool");

      ptr->~T();

      reinterpret_cast<_Slot*>(ptr)->next = freeSlots;
      freeSlots = reinterpret_cast<_Slot*>(ptr);
    }

//     void destroy(const size_t &type, base_component_storage* ptr) {
//       if (ptr == nullptr) return;
//       ASSERT(type == typeId && "Wrong object type for typeless pool");
// 
//       ptr->~base_component_storage();
// 
//       reinterpret_cast<_Slot*>(ptr)->next = freeSlots;
//       freeSlots = reinterpret_cast<_Slot*>(ptr);
//     }

    typeless_pool & operator=(const typeless_pool &pool) = delete;
    typeless_pool & operator=(typeless_pool &&pool) = delete;
  private:
    union _Slot {
      _Slot* next;
    };

    const size_t typeId;
    const size_t blockSize;
    size_t currentSize;
    char* memory;
    _Slot* freeSlots;

    void allocateBlock() {
      const size_t newBufferSize = blockSize + sizeof(char*);
      char* newBuffer = new char[newBufferSize];

      char** tmp = reinterpret_cast<char**>(newBuffer);
      tmp[0] = memory;

      memory = newBuffer;
      currentSize = 0;
    }
  };

#ifdef YACS_MULTITHREADING
  class ThreadsafeTypelessPool {
  public:
    ThreadsafeTypelessPool(const size_t &type, const size_t &blockSize) : type(type), blockSize(blockSize), currentSize(0), memory(nullptr), freeSlots(nullptr) {
      memory = allocate(nullptr);
      //allocateBlock();
      //tryAllocate(mem);
    }

    ~ThreadsafeTypelessPool() {
      char* tmp = memory;
      while (tmp != nullptr) {
        char** ptr = reinterpret_cast<char**>(tmp);
        char* nextBuffer = ptr[0];

        delete [] tmp;
        tmp = nextBuffer;
      }
    }

    template<typename T, typename... Args>
    T* create(Args&&... args) {
      ASSERT(T::type == type && "Wrong object type for typeless pool");

      const size_t objectSize = std::max(sizeof(T), sizeof(_Slot));

      _Slot* tmp = nullptr;
      T* ptr = nullptr;
      if (!freeSlots.compare_exchange_strong(tmp, nullptr)) {
        if (freeSlots.compare_exchange_strong(tmp, tmp->next)) {
          ptr = reinterpret_cast<T*>(tmp);
          new (ptr) T(std::forward<Args>(args)...);
          return ptr;
        }
      }

      // способ 1: кажется он самый адекватный, в этом случае мы имитируем мьютекс с помощью указателя памяти
      // слишком длинная блокировка? думаю что вряд ли это возможно
      char* mem = nullptr;
      do {
        std::this_thread::sleep_for(std::chrono::nanoseconds(1)); // с этой строкой работает стабильнее
        mem = memory.exchange(nullptr);
      } while (mem == nullptr);

      if (currentSize+objectSize > blockSize) {
        mem = allocate(mem);
        currentSize = 0;
      }

      const size_t place = currentSize;
      currentSize += objectSize;
      ptr = reinterpret_cast<T*>(mem + sizeof(char*) + place);

      memory = mem;

      // способ 2: может быть повторная ситуация, когда place+objectSize > blockSize, но при этом mem != nullptr,
      // это приведет к выделению лишних блоков памяти, ВОЗМОЖНО может помочь сон треда
      // size_t place = currentSize.fetch_add(objectSize);
      // char* mem = memory;
      // while (place+objectSize > blockSize || mem == nullptr) {
      //   if (mem != nullptr) tryAllocate(mem);
      //
      //   std::this_thread::sleep_for(std::chrono::nanoseconds(1)); // с этой строкой работает стабильнее
      //
      //   place = currentSize.fetch_add(objectSize);
      //   mem = memory;
      // }
      //
      // ptr = reinterpret_cast<T*>(mem == nullptr ? nullptr : mem + sizeof(char*) + place);


      ASSERT(ptr != nullptr);
      ASSERT(place+std::max(sizeof(T), sizeof(_Slot)) <= blockSize);

      new (ptr) T(std::forward<Args>(args)...);
      return ptr;
    }

    template<typename T>
    void destroy(T* ptr) {
      if (ptr == nullptr) return;
      ASSERT(T::type == type && "Wrong object type for typeless pool");

      ptr->~T();

      auto slot = reinterpret_cast<_Slot*>(ptr);
      slot->next = freeSlots.exchange(slot);
    }

    void destroy(const size_t &type, BaseComponentStorage* ptr) {
      if (ptr == nullptr) return;
      ASSERT(type == this->type && "Wrong object type for typeless pool");

      ptr->~BaseComponentStorage();

      auto slot = reinterpret_cast<_Slot*>(ptr);
      slot->next = freeSlots.exchange(slot);
    }
  private:
    union _Slot {
      //char obj[typeSize];
      //char* obj;
      _Slot* next;
    };

    const size_t type;
    const size_t blockSize;
    //std::atomic<size_t> currentSize;
    size_t currentSize; // для второго способа atomic не нужен
    std::atomic<char*> memory;
    std::atomic<_Slot*> freeSlots;

    char* allocate(char* mem) const {
      const size_t newBufferSize = blockSize + sizeof(char*);
      char* newBuffer = new char[newBufferSize];

      // в первые sizeof(char*) байт кладем указатель
      char** tmp = reinterpret_cast<char**>(newBuffer);
      tmp[0] = mem;

      //memory = newBuffer;
      return newBuffer;
    }

    void tryAllocate(char* mem) {
      if (!memory.compare_exchange_strong(mem, nullptr)) return;

      const size_t newBufferSize = blockSize + sizeof(char*);
      char* newBuffer = new char[newBufferSize];

      // в первые sizeof(char*) байт кладем указатель
      char** tmp = reinterpret_cast<char**>(newBuffer);
      tmp[0] = mem;

      memory = newBuffer;
      currentSize = 0;
    }
  };

  // можно переделать Lock так, чтобы использовать его вне ThreadsafeArray
  // для того чтобы поработать с массивом безопасно
  template <typename T>
  class ThreadsafeArray {
  public:
    class Lock {
    public:
      Lock(std::atomic<T*>* memory) : memory(memory), mem(nullptr) {
        do {
          std::this_thread::sleep_for(std::chrono::nanoseconds(1));
          mem = memory->exchange(nullptr);
        } while (mem == nullptr);
      }
      
      Lock(Lock&& lock) : memory(lock.memory), mem(lock.mem) {
        lock.memory = nullptr;
        lock.mem = nullptr;
      }

      ~Lock() {
        if (lock.memory != nullptr) *memory = mem;
      }

      T* get() const { return mem; }
      void set(T* mem) { this->mem = mem; }
    private:
      std::atomic<T*>* memory;
      T* mem;
    };

    ThreadsafeArray() : m_capacity(YACS_THREADSAFE_ARRAY_DEFAULT_CAPACITY), m_size(0), m_memory(nullptr) {
      m_memory = new T[m_capacity];
    }

    ~ThreadsafeArray() {
      delete [] m_memory;
    }

    size_t capacity() const { Lock l(&m_memory); return m_capacity; }
    size_t size() const { Lock l(&m_memory); return m_size; }

    T front() const { Lock l(&m_memory); return l.get()[0]; }
    T back() const { Lock l(&m_memory); return l.get()[m_size-1]; }

    size_t push(const T &obj) {
      Lock l(&m_memory);

      T* mem = l.get();
      if (m_size == m_capacity) mem = allocate(mem);
      l.set(mem);

      const size_t index = m_size;
      mem[index] = obj;
      ++m_size;

      return index;
    }

    T erase(const size_t &index) {
      Lock l(&m_memory);
      if (index >= m_size) return T();

      --m_size;

      T* mem = l.get();
      std::swap(mem[index], mem[m_size]);

      return mem[m_size];
    }

    void erase(const T &obj) {
      Lock l(&m_memory);

      size_t index = 0;
      T* mem = l.get();
      for (; index < m_size; ++index) {
        if (mem[index] == obj) break;
      }

      --m_size;
      std::swap(mem[index], mem[m_size]);

//       return mem[m_size];
    }

    T pop() {
      Lock l(&m_memory);
      if (m_size == 0) return T();

      T* mem = l.get();
      --m_size;
      return mem[m_size];
    }
  private:
    T* allocate(T* mem) {
      m_capacity = m_capacity * YACS_THREADSAFE_ARRAY_GROWTH_POLICY;
      T* new_mem = new T[m_capacity];
      memcpy(new_mem, mem, m_size*sizeof(T));
      delete [] mem;
      return new_mem;
    }

    size_t m_capacity;
    size_t m_size;
    mutable std::atomic<T*> m_memory;
  };
#endif // YACS_MULTITHREADING
}

#endif //YACS_POOL_H
