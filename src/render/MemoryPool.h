/**
 * Реализация MemoryPool. Выделяется дополнительно 8 байт на блок
 * Для корректного удаления элементов, необходимо вызвать deleteElement
 * для каждого указателя из этого пула!
 */

#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <cstdint>
#include <cstddef>
#include <utility>

#ifndef _NDEBUG
  #include <cassert>
  #define ASSERT(expr) assert(expr);
#else
  #define ASSERT(expr)
#endif

//#include <unordered_set>

template<typename T, size_t blockSize = 4096>
class MemoryPool {
public:
    template <typename U> struct rebind {
      typedef MemoryPool<U> other;
    };

    MemoryPool() noexcept {}

    MemoryPool(const MemoryPool& memoryPool) noexcept : MemoryPool() {}

    MemoryPool(MemoryPool&& memoryPool) noexcept {
        currentBlock_ = memoryPool.currentBlock_;
        currentSlot_  = memoryPool.currentSlot_;
        lastSlot_     = memoryPool.lastSlot_;
        freeSlots_    = memoryPool.freeSlots;
        memoryPool.currentBlock_ = nullptr;
        memoryPool.currentSlot_  = nullptr;
        memoryPool.lastSlot_     = nullptr;
        memoryPool.freeSlots_    = nullptr;
    }

    template <class U> MemoryPool(const MemoryPool<U>& memoryPool) noexcept : MemoryPool() {}

    ~MemoryPool() noexcept {
      clear();
    }

    MemoryPool& operator=(const MemoryPool& memoryPool) = delete;

    MemoryPool& operator=(MemoryPool&& memoryPool) noexcept {
      if (this != &memoryPool) {
        std::swap(currentBlock_, memoryPool.currentBlock_);
        currentSlot_ = memoryPool.currentSlot_;
        lastSlot_ = memoryPool.lastSlot_;
        freeSlots_ = memoryPool.freeSlots;
      }

      return *this;
    }

    T* address(T& x) const noexcept {
        return &x;
    }

    const T* address(const T& x) const noexcept {
        return &x;
    }

    T* allocate(const size_t &n = 1, const T* hint = nullptr) {
      (void)n;
      (void)hint;
      if (freeSlots_ != nullptr) {
        T* result = reinterpret_cast<T*>(freeSlots_);
        freeSlots_ = freeSlots_->next;

        return result;
      } else {
        if (currentSlot_ >= lastSlot_) allocateBlock();

        return reinterpret_cast<T*>(currentSlot_++);
      }
    }

    void deallocate(T* p, const size_t &n = 1) {
      (void)n;
      if (p != nullptr) {
        reinterpret_cast<Slot_*>(p)->next = freeSlots_;
        freeSlots_ = reinterpret_cast<Slot_*>(p);
      }
    }

    size_t max_size() const noexcept {
      size_t maxBlocks = -1 / blockSize;
      return (blockSize - sizeof(char*)) / sizeof(Slot_) * maxBlocks;
    }

    template <class U, class... Args> void construct(U* p, Args&&... args) {
      new (p) U (std::forward<Args>(args)...);
    }

    template <class U> void destroy(U* p) {
        p->~U();
    }

    template <class... Args> T* newElement(Args&&... args)  {
      ASSERT(sizeof(T)*3 < blockSize);

      T* result = allocate();
      construct<T>(result, std::forward<Args>(args)...);
//       T* result = new T(std::forward<Args>(args)...);
      return result;
    }

    void deleteElement(T* p) {
      if (p != nullptr) {
        p->~T();
        deallocate(p);
      }
//       delete p;
    }

    size_t getSize() {
      return blockSize;
    }

    void clear() {
      Slot_* curr = currentBlock_;
      while (curr != nullptr) {
        Slot_* prev = curr->next;
        delete [] reinterpret_cast<char*>(curr);
        curr = prev;
      }
    }
private:
    union Slot_ {
      T element;
      Slot_* next;
    };

    Slot_* currentBlock_ = nullptr;
    Slot_* currentSlot_  = nullptr;
    Slot_* lastSlot_     = nullptr;
    Slot_* freeSlots_    = nullptr;

    size_t padPointer(char* p, const size_t &align) const noexcept {
      uintptr_t result = reinterpret_cast<uintptr_t>(p);
      return ((align - result) % align);
    }

    void allocateBlock() {
      // выделим память для нового блока + 8 байт под указатель на предыдущий
      size_t size = blockSize + sizeof(Slot_*);
      char* newBlock = new char[size];
      reinterpret_cast<Slot_*>(newBlock)->next = currentBlock_;
      currentBlock_ = reinterpret_cast<Slot_*>(newBlock);

      // немножко пододвинем указатель для того чтобы добиться выравнивания элементов
      char* body = newBlock + sizeof(Slot_*); // в первых 8 байтах у нас лежит указатель
      size_t bodyPadding = padPointer(body, alignof(Slot_)); // получаем выравнивание
      currentSlot_ = reinterpret_cast<Slot_*>(body + bodyPadding); // двигаем указатель
      lastSlot_ = reinterpret_cast<Slot_*>(newBlock + size - sizeof(Slot_) + 1); // находим последний слот
    }
};

#endif
