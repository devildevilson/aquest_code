#include "get_scope_commands.h"

#include "core.h"
#include "context.h"
#include "core/structures_header.h"
#include "utils/type_info.h"

namespace devils_engine {
  namespace script {
#define GET_SCOPE_COMMAND_FUNC(func_name, context_types_bits, output_type_bit, unused) \
    const size_t func_name::type_index = commands::values::func_name; \
    const size_t func_name::context_types;            \
    const size_t func_name::output_type;              \
    void func_name::draw(context* ctx) const {        \
      const auto &obj = process(ctx);                 \
      draw_data dd(ctx);                              \
      dd.function_name = commands::names[type_index]; \
      dd.value = obj;                                 \
      ctx->draw_function(&dd);                        \
    }
    
    GET_SCOPE_COMMANDS_FINAL_LIST
    
#undef GET_SCOPE_COMMAND_FUNC

    template <typename T>
    static bool check_value_validity(const T &val) {
      if constexpr (std::is_pointer_v<T>) return val != nullptr;
      else return val.valid();
      return false;
    }
    
#define TYPED_GET_SCOPE_COMMAND_FUNC(current_type, func_name) \
    struct object func_name::process(context* ctx) const { \
      const auto c = ctx->current.get<current_type>(); \
      auto ret = c->get_##func_name(); \
      if (!check_value_validity(ret)) throw std::runtime_error(std::string(utils::remove_namespaces(utils::type_name<current_type>())) + " does not have a "#func_name); \
      return object(ret); \
    }    
    
#define GET_SCOPE_COMMAND_FUNC(func_name, context_types_bits, output_type_bit, unused) TYPED_GET_SCOPE_COMMAND_FUNC(core::character*, func_name)
    CHARACTER_GET_SCOPE_COMMANDS_LIST
#undef GET_SCOPE_COMMAND_FUNC

#define GET_SCOPE_COMMAND_FUNC(func_name, context_types_bits, output_type_bit, unused) TYPED_GET_SCOPE_COMMAND_FUNC(utils::handle<core::realm>, func_name)
    REALM_GET_SCOPE_COMMANDS_LIST
#undef GET_SCOPE_COMMAND_FUNC

#define GET_SCOPE_COMMAND_FUNC(func_name, context_types_bits, output_type_bit, unused) TYPED_GET_SCOPE_COMMAND_FUNC(core::titulus*, func_name)
    TITLE_GET_SCOPE_COMMANDS_LIST
#undef GET_SCOPE_COMMAND_FUNC

#define GET_SCOPE_COMMAND_FUNC(func_name, context_types_bits, output_type_bit, unused) TYPED_GET_SCOPE_COMMAND_FUNC(utils::handle<core::army>, func_name)
    ARMY_GET_SCOPE_COMMANDS_LIST
#undef GET_SCOPE_COMMAND_FUNC

#define GET_SCOPE_COMMAND_FUNC(func_name, context_types_bits, output_type_bit, unused) TYPED_GET_SCOPE_COMMAND_FUNC(core::province*, func_name)
  GET_SCOPE_COMMAND_FUNC(army, INPUT_PROVINCE, OUTPUT_TITLE, OUTPUT_ARMY_TYPE)
  GET_SCOPE_COMMAND_FUNC(capital, INPUT_PROVINCE, OUTPUT_CITY, OUTPUT_CITY_TYPE2)
#undef GET_SCOPE_COMMAND_FUNC
  
#define GET_SCOPE_COMMAND_FUNC(func_name, context_types_bits, output_type_bit, unused) TYPED_GET_SCOPE_COMMAND_FUNC(core::city*, func_name)
  GET_SCOPE_COMMAND_FUNC(city_type, INPUT_CITY, OUTPUT_CITY, OUTPUT_CITY_TYPE_TYPE)
#undef GET_SCOPE_COMMAND_FUNC

    struct object title::process(context* ctx) const {
      if (ctx->current.get_type() == script::object::type::province) {
        const auto c = ctx->current.get<core::province*>();
        auto ret = c->get_title();
        if (!check_value_validity(ret)) throw std::runtime_error("province does not have a title"); // этого быть не должно
        return object(ret);
      }
      
      const auto c = ctx->current.get<core::city*>();
      auto ret = c->get_title();
      if (!check_value_validity(ret)) throw std::runtime_error("city does not have a title");
      return object(ret);
    }
    
// тип города было бы неплохо держать констом
#define GET_SCOPE_COMMAND_FUNC(func_name, context_types_bits, output_type_bit, unused) TYPED_GET_SCOPE_COMMAND_FUNC(core::city*, func_name)
    
#undef GET_SCOPE_COMMAND_FUNC

#define GET_SCOPE_COMMAND_FUNC(func_name, context_types_bits, output_type_bit, unused) TYPED_GET_SCOPE_COMMAND_FUNC(utils::handle<core::war>, func_name)
    WAR_GET_SCOPE_COMMANDS_LIST
#undef GET_SCOPE_COMMAND_FUNC

#define GET_SCOPE_COMMAND_FUNC(func_name, context_types_bits, output_type_bit, unused) TYPED_GET_SCOPE_COMMAND_FUNC(core::trait*, func_name)
    TRAIT_GET_SCOPE_COMMANDS_LIST
#undef GET_SCOPE_COMMAND_FUNC
  
    struct object culture::process(context* ctx) const {
      if (ctx->current.get_type() == script::object::type::province) {
        const auto c = ctx->current.get<core::province*>();
        auto ret = c->get_culture();
        if (!check_value_validity(ret)) throw std::runtime_error("province does not have a culture"); // этого быть не должно
        return object(ret);
      }
      
      const auto c = ctx->current.get<core::character*>();
      auto ret = c->get_culture();
      if (!check_value_validity(ret)) throw std::runtime_error("character does not have a culture"); // этого быть не должно
      return object(ret);
    }
    
    struct object culture_group::process(context* ctx) const {
      if (ctx->current.get_type() == script::object::type::province) {
        const auto c = ctx->current.get<core::province*>();
        auto ret = c->get_culture_group();
        if (!check_value_validity(ret)) throw std::runtime_error("province does not have a culture group"); // этого быть не должно
        return object(ret);
      }
      
      if (ctx->current.get_type() == script::object::type::culture) {
        const auto c = ctx->current.get<core::culture*>();
        auto ret = c->get_culture_group();
        if (!check_value_validity(ret)) throw std::runtime_error("culture does not have a culture group"); // этого быть не должно
        return object(ret);
      }
      
      const auto c = ctx->current.get<core::character*>();
      auto ret = c->get_culture_group();
      if (!check_value_validity(ret)) throw std::runtime_error("character does not have a culture group"); // этого быть не должно
      return object(ret);
    }
    
    struct object religion::process(context* ctx) const {
      if (ctx->current.get_type() == script::object::type::province) {
        const auto c = ctx->current.get<core::province*>();
        auto ret = c->get_religion();
        if (!check_value_validity(ret)) throw std::runtime_error("province does not have a religion"); // этого быть не должно
        return object(ret);
      }
      
      const auto c = ctx->current.get<core::character*>();
      auto ret = c->get_religion();
      if (!check_value_validity(ret)) throw std::runtime_error("character does not have a religion"); // этого быть не должно
      return object(ret);
    }
    
    struct object religion_group::process(context* ctx) const {
      if (ctx->current.get_type() == script::object::type::province) {
        const auto c = ctx->current.get<core::province*>();
        auto ret = c->get_religion_group();
        if (!check_value_validity(ret)) throw std::runtime_error("province does not have a religion group"); // этого быть не должно
        return object(ret);
      }
      
      if (ctx->current.get_type() == script::object::type::religion) {
        const auto c = ctx->current.get<core::religion*>();
        auto ret = c->get_religion_group();
        if (!check_value_validity(ret)) throw std::runtime_error("religion does not have a religion group"); // этого быть не должно
        return object(ret);
      }
      
      const auto c = ctx->current.get<core::character*>();
      auto ret = c->get_religion_group();
      if (!check_value_validity(ret)) throw std::runtime_error("character does not have a religion group"); // этого быть не должно
      return object(ret);
    }
    
    struct object head::process(context* ctx) const {
      const auto c = ctx->current.get<core::religion*>();
      auto ret = c->get_head();
      if (!check_value_validity(ret)) throw std::runtime_error("religion does not have a head"); // этого быть не должно
      return object(ret);
    }
    
    struct object head_heir::process(context* ctx) const {
      const auto c = ctx->current.get<core::religion*>();
      auto ret = c->get_head_heir();
      if (!check_value_validity(ret)) throw std::runtime_error("religion does not have a head heir"); // этого быть не должно
      return object(ret);
    }

#undef TYPED_GET_SCOPE_COMMAND_FUNC
  }
}
