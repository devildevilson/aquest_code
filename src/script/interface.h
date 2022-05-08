#ifndef DEVILS_ENGINE_SCRIPT_INTERFACE_H
#define DEVILS_ENGINE_SCRIPT_INTERFACE_H

#include "object.h"

namespace devils_engine {
  namespace script {
    struct context;
    class interface;
    
    // возможно было бы неплохо иметь что то такое для каждого скрипта
    struct script_data {
      interface* begin;
      size_t max_locals;
      size_t size;
      
    };
    
    // я думал о том что можно предрассчитать какие то вещи в скрипте,
    // это могло бы быть полезным для мультитрединга, но потом я подумал что
    // большую часть вещей нужно перевычислять в зависимости от обновления персонажа
    // вычислять то я могу параллельно, но вот ПРИМЕНИТЬ вычисленное можно последовательно
    // применяю вычисленное я только в ЭФФЕКТАХ, функция make_context видимо
    // будет иметь смысл только в эффектах
    class interface {
    public:
      inline interface() noexcept : next(nullptr) {}
      virtual ~interface() noexcept = default;
      virtual struct object process(context* ctx) const = 0;
      virtual void draw(context* ctx) const = 0;
      virtual size_t get_type_id() const = 0; // expeсted context type id
      virtual std::string_view get_name() const = 0;
      //virtual void make_context(context* ctx) const = 0;
      
      interface* next;
    };
  }
}

#endif
