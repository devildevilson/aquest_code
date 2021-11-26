#include "logic_commands.h"

#include "core.h"
#include "context.h"

namespace devils_engine {
  namespace script {
#define LOGIC_BLOCK_COMMAND_FUNC(func_name, expected, output)     \
    func_name::func_name(const interface* childs) noexcept : childs(childs) {} \
    func_name::~func_name() noexcept {                            \
      for (auto cur = childs; cur != nullptr; cur = cur->next) { cur->~interface(); } \
    }                                                             \
    const size_t func_name::type_index = commands::values::func_name; \
    void func_name::draw(context* ctx) const {                    \
      const auto obj = process(ctx);                              \
      draw_data dd(ctx);                                          \
      dd.function_name = commands::names[type_index];             \
      dd.value = obj;                                             \
      ctx->draw_function(&dd);                                    \
      for (auto cur = childs; cur != nullptr; cur = cur->next) {  \
        cur->draw(ctx);                                           \
      }                                                           \
    }
    
    LOGIC_BLOCK_COMMANDS_LIST
#undef LOGIC_BLOCK_COMMAND_FUNC
    
    // возвращать ли здесь ignore_value? вообще можно
    
    struct object AND::process(context* ctx) const {
      bool all_ignore = true;
      bool value = true;
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        const auto &obj = cur->process(ctx);
        all_ignore = all_ignore && obj.ignore();
        value = obj.ignore() ? value : value && obj.get<bool>();
      }
      
      return all_ignore ? ignore_value : object(value);
    }
    
    struct object OR::process(context* ctx) const {
      bool all_ignore = true;
      bool value = false;
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        const auto &obj = cur->process(ctx);
        all_ignore = all_ignore && obj.ignore();
        value = obj.ignore() ? value : value || obj.get<bool>();
      }
      
      return all_ignore ? ignore_value : object(value);
    }
    
    struct object NAND::process(context* ctx) const {
      bool all_ignore = true;
      bool value = true;
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        const auto &obj = cur->process(ctx);
        all_ignore = all_ignore && obj.ignore();
        value = obj.ignore() ? value : value && obj.get<bool>();
      }
      
      return all_ignore ? ignore_value : object(!value);
    }
    
    struct object NOR::process(context* ctx) const {
      bool all_ignore = true;
      bool value = false;
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        const auto &obj = cur->process(ctx);
        all_ignore = all_ignore && obj.ignore();
        value = obj.ignore() ? value : value || obj.get<bool>();
      }
      
      return all_ignore ? ignore_value : object(!value);
    }
    
    struct object XOR::process(context* ctx) const {
      bool all_ignore = true;
      bool value = false;
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        const auto &obj = cur->process(ctx);
        all_ignore = all_ignore && obj.ignore();
        value = obj.ignore() ? value : value != obj.get<bool>();
      }
      
      return all_ignore ? ignore_value : object(value);
    }
    
    struct object IMPL::process(context* ctx) const {
      bool all_ignore = true;
      bool value = false;
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        const auto &obj = cur->process(ctx);
        all_ignore = all_ignore && obj.ignore();
        value = obj.ignore() ? value : value <= obj.get<bool>();
      }
      
      return all_ignore ? ignore_value : object(value);
    }
    
    struct object EQ::process(context* ctx) const {
      bool all_ignore = true;
      bool value = false;
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        const auto &obj = cur->process(ctx);
        all_ignore = all_ignore && obj.ignore();
        value = obj.ignore() ? value : value == obj.get<bool>();
      }
      
      return all_ignore ? ignore_value : object(value);
    }
  }
}
