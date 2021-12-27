#ifndef DEVILS_ENGINE_SCRIPT_INTERFACE_H
#define DEVILS_ENGINE_SCRIPT_INTERFACE_H

#include "object.h"

namespace devils_engine {
  namespace script {
    struct context;
    
    class interface {
    public:
      inline interface() noexcept : next(nullptr) {}
      virtual ~interface() noexcept = default;
      virtual struct object process(context* ctx) const = 0;
      virtual void draw(context* ctx) const = 0;
      
      //const interface* next;
      interface* next; // мне нужно пушить указатель если я создаю сразу несколько функций
    };
  }
}

#endif
