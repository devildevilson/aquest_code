#include "logic_commands.h"

#include "core.h"
#include "context.h"
#include "utils/shared_mathematical_constant.h"

namespace devils_engine {
  namespace script {
#define LOGIC_BLOCK_COMMAND_FUNC(func_name, expected, output)     \
    const size_t func_name::type_index = commands::values::func_name; \
    const size_t func_name::expected_types;                       \
    const size_t func_name::output_type;                          \
    func_name::func_name(const interface* childs) noexcept : childs(childs) {} \
    func_name::~func_name() noexcept {                            \
      for (auto cur = childs; cur != nullptr; cur = cur->next) { cur->~interface(); } \
    }                                                             \
    void func_name::draw(context* ctx) const {                    \
      const auto obj = process(ctx);                              \
      draw_data dd(ctx);                                          \
      dd.function_name = commands::names[type_index];             \
      dd.value = obj;                                             \
      ctx->draw(&dd);                                             \
      change_nesting cn(ctx, ++ctx->nest_level);                  \
      change_function_name cfn(ctx, dd.function_name);            \
      for (auto cur = childs; cur != nullptr; cur = cur->next) {  \
        cur->draw(ctx);                                           \
      }                                                           \
      if (type_index == commands::AND) throw std::runtime_error("AAAAAAAAAAAAAAAAAAAAAA"); \
    }
    
    LOGIC_BLOCK_COMMANDS_LIST
#undef LOGIC_BLOCK_COMMAND_FUNC
    
    // возвращать ли здесь ignore_value? вообще можно
    // в некоторых функциях присутствует ранний выход
    // значит что функция может пропустить декларацию локальной переменной
    
    struct object AND::process(context* ctx) const {
      bool all_ignore = true;
      bool value = true;
      for (auto cur = childs; cur != nullptr && value; cur = cur->next) {
        const auto &obj = cur->process(ctx);
        all_ignore = all_ignore && obj.ignore();
        value = obj.ignore() ? value : value && obj.get<bool>();
        //std::cout << "value " << value << "\n";
      }
      
      return all_ignore ? ignore_value : object(value);
    }
    
    struct object OR::process(context* ctx) const {
      bool all_ignore = true;
      bool value = false;
      for (auto cur = childs; cur != nullptr && !value; cur = cur->next) {
        const auto &obj = cur->process(ctx);
        all_ignore = all_ignore && obj.ignore();
        value = obj.ignore() ? value : value || obj.get<bool>();
      }
      
      return all_ignore ? ignore_value : object(value);
    }
    
    struct object NAND::process(context* ctx) const {
      bool all_ignore = true;
      bool value = true;
      for (auto cur = childs; cur != nullptr && value; cur = cur->next) {
        const auto &obj = cur->process(ctx);
        all_ignore = all_ignore && obj.ignore();
        value = obj.ignore() ? value : value && obj.get<bool>();
      }
      
      return all_ignore ? ignore_value : object(!value);
    }
    
    struct object NOR::process(context* ctx) const {
      bool all_ignore = true;
      bool value = false;
      for (auto cur = childs; cur != nullptr && !value; cur = cur->next) {
        const auto &obj = cur->process(ctx);
        all_ignore = all_ignore && obj.ignore();
        value = obj.ignore() ? value : value || obj.get<bool>();
      }
      
      return all_ignore ? ignore_value : object(!value);
    }
    
    static std::tuple<double, const interface*> get_first(context* ctx, const interface* childs) {
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        const auto &obj = cur->process(ctx);
        if (!obj.ignore()) return std::make_tuple(double(obj.get<bool>()), cur->next);
      }
      return std::make_tuple(dNAN, nullptr);
    }
    
    struct object XOR::process(context* ctx) const {
      const auto [val, start] = get_first(ctx, childs);
      if (std::isnan(val)) return ignore_value;
      
      bool value = val;
      for (auto cur = start; cur != nullptr; cur = cur->next) {
        const auto &obj = cur->process(ctx);
        value = obj.ignore() ? value : value != obj.get<bool>();
      }
      
      return object(value);
    }
    
    struct object IMPL::process(context* ctx) const {
      const auto [val, start] = get_first(ctx, childs);
      if (std::isnan(val)) return ignore_value;
      
      bool value = val;
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        const auto &obj = cur->process(ctx);
        value = obj.ignore() ? value : value <= obj.get<bool>();
      }
      
      return object(value);
    }
    
    struct object EQ::process(context* ctx) const {
      const auto [val, start] = get_first(ctx, childs);
      if (std::isnan(val)) return ignore_value;
      
      bool value = val;
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        const auto &obj = cur->process(ctx);
        value = obj.ignore() ? value : value == obj.get<bool>();
      }
      
      return object(value);
    }
    
    struct object AND_sequence::process(context* ctx) const {
      bool value = true;
      for (auto cur = childs; cur != nullptr && value; cur = cur->next) {
        const auto &obj = cur->process(ctx);
        if (obj.ignore()) break;
        value = value && obj.get<bool>();
      }
      
      return object(value);
    }
    
    struct object OR_sequence::process(context* ctx) const {
      bool value = false;
      for (auto cur = childs; cur != nullptr && !value; cur = cur->next) {
        const auto &obj = cur->process(ctx);
        if (obj.ignore()) break;
        value = value || obj.get<bool>();
      }
      
      return object(value);
    }
  }
}
