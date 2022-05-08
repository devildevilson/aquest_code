#ifndef DEVILS_ENGINE_SCRIPT_CONTEXT_H
#define DEVILS_ENGINE_SCRIPT_CONTEXT_H

#include <array>
#include <string_view>
#include <functional>
#include "object.h"
#include "parallel_hashmap/phmap.h"
#include "utils/linear_rng.h"
#include "utils/bit_field.h"

// как выглядит контекст? должны быть указаны объекты root, prev, current, для рисования пригодится еще запомнить rvalue и запомнить тип сравнения
// должны быть указаны названия того где мы находимся, название интеракции, название конкретного метода, название текущей функции
// должен быть ассоциативный массив с данными, мы можем положить туда все кроме строк
// строку можем оформить как флаг (то есть ассоциация с инвалидным (или игнор) значением)
// у нас еще должен присутствовать глобальный контекст, скорее всего он будет сильно похож на обычный

// мне нужен еще стейт случайных чисел тут по идее достаточно просто одного стейта, но он должен вычисляться для каждого запуска единообразно
// как это сделать? что значит запускаться единообразно? состояние генератора зависит от рута и от самой структуры со скриптом
// да но так для каждого рута в одном скрипте будет одинаковый результат, нужно еще домешать сюда индекс хода видимо
// таким образом состояния генератора будет уникальным для каждого рута в каждой структуре скрипта на каждом ходу
// самый адекватный вариант

#define DEFAULT_GENERATOR_NAMESPACE utils::xoshiro256starstar

namespace devils_engine {
  namespace script {
    struct context;
    
    struct draw_data {
      static const size_t arguments_count = 16;
      
      std::string_view id;
      std::string_view method_name;
      std::string_view function_name;
      std::string_view prev_function_name;
      size_t type;
      size_t operator_type;
      size_t nest_level;
      object current, value, original;
      std::array<std::pair<std::string_view, object>, arguments_count> arguments;
      
      draw_data();
      draw_data(context* ctx);
      void set_arg(const uint32_t &index, const std::string_view &name, const object &obj);
    };
    
    using draw_function_t = std::function<bool(const draw_data* data)>;
    
//     struct random_state {
//       using state = DEFAULT_GENERATOR_NAMESPACE::state;
//       
//       static double normalize(const uint64_t value);
//       
//       state cur;
//       
//       random_state();
//       random_state(const size_t &val1, const size_t &val2, const size_t &val3, const size_t &val4);
//       random_state(const size_t &state_root, const size_t &state_container, const size_t &current_turn);
//       uint64_t next();
//     };
    
    struct context {
      static const size_t locals_container_size = 64;
      static const size_t lists_container_size = 64;
      
      enum {
        current_context_is_undefined,
      };
      
      std::string_view id;
      std::string_view method_name;
      std::string_view function_name; // нужно ли это здесь?
      std::string_view prev_function_name; // вот это пригодится
      size_t type;
      size_t operator_type;
      size_t nest_level;
      object root, prev, current, rvalue;
      
      // данные для рандомизации, хэш поди нужно использовать std, использую свой
      size_t id_hash;
      size_t method_hash;
      size_t current_turn; // наверное тут укажем просто какой нибдуь сид
      
      // а можем ли мы тут хранить строку? можем ли мы использовать string_view в качестве ключа?
      // контекст в каком то виде может перейти в контекст эвента игрока
      // все строки в скрипте храняться в контейнерах, могут ли какие нибудь строки прийти откуда нибудь извне так чтобы у них не было своего хранилища? 
      // откуда строки берутся? из скрипта (хранятся в контейнерах) + из on_action (константные строки?) + из вызова метода (константные строки?)
      // возможно мы тут легко можем обойтись string_view
      phmap::flat_hash_map<std::string_view, object> map; // здесь же могут храниться флаги
      // + контейнер который мы соберем для эвента, наверное данные для ивента соберуться прямо на месте
      //phmap::flat_hash_map<std::string_view, object> event_container;
      
      // помимо локальных переменных, нужно еще сделать массивы переменных, может быть тоже std::array?
      // как тогда угадать куда передать этот лист при вызове другого скрипта?
      // наверное нужно все таки использовать flat_hash_map
      phmap::flat_hash_map<std::string_view, std::vector<object>> object_lists;
      //std::array<std::vector<object>, lists_container_size> object_lists;
      
      std::array<object, locals_container_size> locals;
      
      // функция рисования
      draw_function_t draw_function;
      // дебаг данные скорее всего
      utils::bit_field<1> attributes;
      
      static double normalize_value(const uint64_t value);
      
      context() noexcept;
      context(const std::string_view &id, const std::string_view &method_name, const size_t &current_turn) noexcept;
      void set_data(const std::string_view &id, const std::string_view &method_name, const size_t &current_turn) noexcept;
      void set_data(const std::string_view &id, const std::string_view &method_name) noexcept;
      
      inline bool get_attribute(const size_t &index) const noexcept { return attributes.get(index); }
      inline void set_attribute(const size_t &index, const bool value) noexcept { attributes.set(index, value); }
      
      bool draw(const draw_data* data) const;
      uint64_t get_random_value(const size_t &static_state) const noexcept;
      
      void clear();
    };
    
    // классы смены скоупа
    struct change_scope {
      context* ctx;
      object prev, current;
      
      inline change_scope(context* ctx, const object &new_scope, const object &prev_scope) noexcept : ctx(ctx), prev(ctx->prev), current(ctx->current) {
        ctx->prev = prev_scope;
        ctx->current = new_scope;
      }
      
      inline ~change_scope() noexcept { ctx->prev = prev; ctx->current = current; }
    };
    
    struct change_rvalue {
      context* ctx;
      object rvalue;
      size_t operator_type;
      
      inline change_rvalue(context* ctx, const object &new_rvalue, const size_t &new_operator_type) noexcept : ctx(ctx), rvalue(ctx->rvalue), operator_type(ctx->operator_type) {
        ctx->rvalue = new_rvalue;
        ctx->operator_type = new_operator_type;
      }
      
      inline ~change_rvalue() noexcept { ctx->rvalue = rvalue; ctx->operator_type = operator_type; }
    };
    
    struct change_nesting {
      context* ctx;
      size_t nesting;
      inline change_nesting(context* ctx, const size_t &new_nesting) noexcept : ctx(ctx), nesting(ctx->nest_level) { ctx->nest_level = new_nesting; }
      inline ~change_nesting() noexcept { ctx->nest_level = nesting; }
    };
    
    struct change_attribute {
      context* ctx;
      size_t attrib;
      bool value;
      inline change_attribute(context* ctx, const size_t &attrib, const bool new_value) noexcept : 
        ctx(ctx), attrib(attrib), value(ctx->get_attribute(attrib)) 
      { ctx->set_attribute(attrib, new_value); }
      inline ~change_attribute() noexcept { ctx->set_attribute(attrib, value); }
    };
    
    struct draw_condition {
      context* ctx;
      inline draw_condition(context* ctx) : ctx(ctx) {
        draw_data dd(ctx);
        dd.function_name = "condition";
        ctx->draw(&dd);
      }
      
      inline ~draw_condition() {
        draw_data dd(ctx);
        dd.function_name = "condition_end";
        ctx->draw(&dd);
      }
    };
    
    struct change_function_name {
      context* ctx;
      std::string_view function_name;
      inline change_function_name(context* ctx, const std::string_view &new_function_name) : ctx(ctx), function_name(ctx->prev_function_name) { ctx->prev_function_name = new_function_name; }
      inline ~change_function_name() { ctx->prev_function_name = function_name; }
    };
  }
}

#endif
