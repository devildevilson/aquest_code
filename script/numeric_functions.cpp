#include "numeric_functions.h"

#include <limits>
#include "core.h"
#include "utils/constexpr_funcs.h"
#include "context.h"

namespace devils_engine {
  namespace script {
#define NUMERIC_COMMAND_BLOCK_FUNC(name, expected_type_bits, output_type_bit) \
    name::name(const interface* childs) noexcept : childs(childs) {} \
    name::~name() noexcept {                          \
      for (auto cur = childs; cur != nullptr; cur = cur->next) { cur->~interface(); } \
    }                                                 \
    const size_t name::type_index = commands::values::name; \
    void name::draw(context* ctx) const {             \
      const auto obj = process(ctx);                  \
      draw_data dd(ctx);                              \
      dd.function_name = commands::names[type_index]; \
      dd.value = obj;                                 \
      ctx->draw_function(&dd);                        \
      for (auto cur = childs; cur != nullptr; cur = cur->next) { cur->draw(ctx); } \
    }
    
#define NUMERIC_COMMAND_FUNC(name, expected_type_bits, output_type_bit) \
    name::name(const interface* value) noexcept : value(value) {} \
    name::~name() noexcept { value->~interface(); }               \
    const size_t name::type_index = commands::values::name;       \
    void name::draw(context* ctx) const {                         \
      const auto obj = process(ctx);                              \
      draw_data dd(ctx);                                          \
      dd.function_name = commands::names[type_index];             \
      dd.value = obj;                                             \
      ctx->draw_function(&dd);                                    \
      value->draw(ctx);                                           \
    }
    
    NUMERIC_COMMANDS_LIST2
    
#undef NUMERIC_COMMAND_BLOCK_FUNC
#undef NUMERIC_COMMAND_FUNC

    std::tuple<struct object, const interface*> find_first_not_ignore(context* ctx, const interface* childs) {
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        const auto &obj = cur->process(ctx);
        if (!obj.ignore()) return std::make_tuple(obj, cur->next);
      }
      
      return std::make_tuple(ignore_value, nullptr);
    }  

    struct object add::process(context* ctx) const {
//       if (condition != nullptr) {
//         const auto obj = condition->process(ctx);
//         if (!obj.get<bool>()) return ignore_value;
//       }
      
      bool all_ignored = true;
      double value = 0.0;
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        const auto &obj = cur->process(ctx);
        all_ignored = all_ignored && obj.ignore();
        value += obj.ignore() ? 0.0 : obj.get<double>();
      }
      
      return all_ignored ? ignore_value : object(value);
    }
    
//     void add::draw(context* ctx) const {
//       const auto obj = process(ctx);
//       draw_data dd(ctx);
//       dd.function_name = commands::names[type_index];
//       dd.value = obj;
//       ctx->draw_function(&dd);
//       for (auto cur = childs; cur != nullptr; cur = cur->next) {
//         cur->draw(ctx);
//       }
//     }
    
    struct object sub::process(context* ctx) const {
      const auto [first, start] = find_first_not_ignore(ctx, childs);
      if (first.ignore()) return ignore_value;
      
      double value = first.get<double>();
      for (auto cur = start; cur != nullptr; cur = cur->next) {
        const auto &obj = cur->process(ctx);
        value -= obj.ignore() ? 0.0 : obj.get<double>();
      }
      
      return object(value);
    }
    
    struct object multiply::process(context* ctx) const {
      bool all_ignored = true;
      double value = 1.0;
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        const auto &obj = cur->process(ctx);
        all_ignored = all_ignored && obj.ignore();
        value *= obj.ignore() ? 1.0 : obj.get<double>();
      }
      
      return all_ignored ? ignore_value : object(value);
    }
    
    struct object divide::process(context* ctx) const {
      const auto [first, start] = find_first_not_ignore(ctx, childs);
      if (first.ignore()) return ignore_value;
      
      double value = first.get<double>();
      for (auto cur = start; cur != nullptr; cur = cur->next) {
        const auto &obj = cur->process(ctx);
        value /= obj.ignore() ? 1.0 : obj.get<double>();
      }
      
      return object(value);
    }
    
    struct object mod::process(context* ctx) const {
      const auto [first, start] = find_first_not_ignore(ctx, childs);
      if (first.ignore()) return ignore_value;
      
      double value = first.get<double>();
      for (auto cur = start; cur != nullptr; cur = cur->next) {
        const auto &obj = cur->process(ctx);
        value = obj.ignore() ? value : std::fmod(value, obj.get<double>());
      }
      
      return object(value);
    }
    
    struct object min::process(context* ctx) const {
      const auto [first, start] = find_first_not_ignore(ctx, childs);
      if (first.ignore()) return ignore_value;
      
      double value = first.get<double>();
      for (auto cur = start; cur != nullptr; cur = cur->next) {
        const auto &obj = cur->process(ctx);
        value = obj.ignore() ? value : std::min(value, obj.get<double>());
      }
      
      return object(value);
    }
    
    struct object max::process(context* ctx) const {
      const auto [first, start] = find_first_not_ignore(ctx, childs);
      if (first.ignore()) return ignore_value;
      
      double value = first.get<double>();
      for (auto cur = start; cur != nullptr; cur = cur->next) {
        const auto &obj = cur->process(ctx);
        value = obj.ignore() ? value : std::max(value, obj.get<double>());
      }
      
      return object(value);
    }
    
    struct object sin::process(context* ctx) const {
      const auto &obj = value->process(ctx);
      const double val = std::sin(obj.get<double>()); // на проверку поди можно забить
      return object(val);
    }
    
    struct object cos::process(context* ctx) const {
      const auto &obj = value->process(ctx);
      const double val = std::cos(obj.get<double>()); // на проверку поди можно забить
      return object(val);
    }
    
    struct object abs::process(context* ctx) const {
      const auto &obj = value->process(ctx);
      const double val = std::abs(obj.get<double>()); // на проверку поди можно забить
      return object(val);
    }
    
    struct object ceil::process(context* ctx) const {
      const auto &obj = value->process(ctx);
      const double val = std::ceil(obj.get<double>()); // на проверку поди можно забить
      return object(val);
    }
    
    struct object floor::process(context* ctx) const {
      const auto &obj = value->process(ctx);
      const double val = std::floor(obj.get<double>()); // на проверку поди можно забить
      return object(val);
    }
    
    struct object sqrt::process(context* ctx) const {
      const auto &obj = value->process(ctx);
      const double val = std::sqrt(obj.get<double>()); // на проверку поди можно забить
      return object(val);
    }
    
    struct object sqr::process(context* ctx) const {
      const auto &obj = value->process(ctx);
      const double orig = obj.get<double>();
      const double val = orig * orig; // на проверку поди можно забить
      return object(val);
    }
    
    struct object frac::process(context* ctx) const {
      const auto &obj = value->process(ctx);
      const double orig = obj.get<double>();
      double unused = 0.0f;
      const double val = std::modf(orig, &unused);
      return object(val);
    }
    
    struct object inv::process(context* ctx) const {
      const auto &obj = value->process(ctx);
      const double val = -obj.get<double>();
      return object(val);
    }
  }
}
