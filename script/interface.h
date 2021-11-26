#ifndef DEVILS_ENGINE_SCRIPT_INTERFACE_H
#define DEVILS_ENGINE_SCRIPT_INTERFACE_H

#include "object.h"

namespace devils_engine {
  namespace script {
    struct context;
    
    // как проверить входные/выходные данные? можно биты типов указать, где их указать? они по идее нужны только при создании скрипта
    class interface {
    public:
      inline interface() : next(nullptr) {}
      virtual ~interface() noexcept = default;
      virtual struct object process(context* ctx) const = 0;
      // самый простой способ - это собрать луа таблицу
      // возможно тут имеет смысл что то возвращать, например бул
      virtual void draw(context* ctx) const = 0;
      
      const interface* next;
    };
  }
}

#endif
