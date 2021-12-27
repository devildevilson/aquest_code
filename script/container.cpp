#include "container.h"

#include "interface.h"

namespace devils_engine {
  namespace script {
    container::container() noexcept : size(0), offset(0), aligment(0), memory(nullptr) {}
    container::container(const size_t &size, const size_t &aligment) noexcept :
      size(size), 
      offset(0), 
      aligment(aligment), 
      memory(new char[size])
    {}
    
    container::container(container &&move) noexcept : 
      size(move.size), 
      offset(move.offset), 
      aligment(move.aligment), 
      memory(move.memory)
    {
      move.size = 0;
      move.offset = 0;
      move.aligment = 0;
      move.memory = nullptr;
    }
    
    container::~container() noexcept {
      delete [] memory;
    }
    
    container & container::operator=(container &&move) noexcept {
      size = move.size;
      offset = move.offset;
      aligment = move.aligment;
      memory = move.memory;
      move.size = 0;
      move.offset = 0;
      move.aligment = 0;
      move.memory = nullptr;
      return *this;
    }
    
    void container::init(const size_t &size, const size_t &aligment) noexcept {
      ASSERT(memory == nullptr);
      this->size = size;
      offset = 0;
      this->aligment = aligment;
      memory = new char[size];
    }
    
    size_t container::mem_size() const {
      return size;
    }
  }
}
