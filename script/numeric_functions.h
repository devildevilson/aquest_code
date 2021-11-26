#ifndef DEVILS_ENGINE_SCRIPT_NUMERIC_FUNCTIONS_H
#define DEVILS_ENGINE_SCRIPT_NUMERIC_FUNCTIONS_H

#include "numeric_commands_macro.h"
#include "utils/sol.h"
#include "interface.h"

namespace devils_engine {
  namespace script {
    struct context;
    struct script_data;
    
// #define NUMERIC_COMMAND_FUNC(name) double name(const target_t&, context*, const uint32_t&, const script_data*);
//     NUMERIC_COMMANDS_LIST
// #undef NUMERIC_COMMAND_FUNC
// 
// #define NUMERIC_COMMAND_FUNC(name) void name##_init(const uint32_t &, const sol::object &, script_data*);
//     NUMERIC_COMMANDS_LIST
// #undef NUMERIC_COMMAND_FUNC
//     
//     // как добавить тип сравнения? вообще лучше не делать эти штуки для сравниваемых значений
//     double compute_complex_value(const target_t&, context*, const uint32_t&, const script_data*, uint32_t&);
    
    // как тут указать вход/выход? в функцию через запятую перечислить? полезно
    // к входу/выходу нам нужно указать что мы ожидаем в текущем контексте
#define NUMERIC_COMMAND_BLOCK_FUNC(func_name, expected_type_bits, output_type_bit) \
  class func_name final : public interface { \
  public: \
    static const size_t type_index;              /* пригодится для draw */ \
    /*static const size_t expected_context = script::object::type_bit::all;*/ /* это мне вряд ли пригодится здесь */ \
    static const size_t expected_types = expected_type_bits; \
    static const size_t output_type = output_type_bit; \
    func_name(const interface* childs) noexcept; \
    ~func_name() noexcept; \
    struct object process(context* ctx) const override; \
    void draw(context* ctx) const override; \
  private: \
    const interface* childs; \
  };
  
#define NUMERIC_COMMAND_FUNC(func_name, expected_type_bits, output_type_bit) \
  class func_name final : public interface { \
  public: \
    static const size_t type_index;              /* пригодится для draw */ \
    /*static const size_t expected_context = script::object::type_bit::all;*/ \
    static const size_t expected_types = expected_type_bits; \
    static const size_t output_type = output_type_bit; \
    func_name(const interface* value) noexcept; \
    ~func_name() noexcept; \
    struct object process(context* ctx) const override; \
    void draw(context* ctx) const override; \
  private: \
    const interface* value; \
  };
  
    NUMERIC_COMMANDS_LIST2
    
#undef NUMERIC_COMMAND_BLOCK_FUNC
#undef NUMERIC_COMMAND_FUNC
    
  }
}

#endif
