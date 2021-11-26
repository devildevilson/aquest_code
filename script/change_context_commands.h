#ifndef DEVILS_ENGINE_SCRIPT_CHANGE_CONTEXT_COMMANDS_H
#define DEVILS_ENGINE_SCRIPT_CHANGE_CONTEXT_COMMANDS_H

#include "object.h"
#include "interface.h"
#include "change_context_commands_macro.h"

namespace devils_engine {
  namespace script {
    struct context;
    
#define GET_SCOPE_COMMAND_FUNC(func_name, context_types_bits, output_type_bit) \
    class func_name final : public interface { \
    public: \
      static const size_t type_index; \
      static const size_t context_types = context_types_bits; \
      static const size_t output_type = output_type_bit; \
      struct object process(context* ctx) const override; \
      void draw(context* ctx) const override; \
    };
    
    GET_SCOPE_COMMANDS_LIST
    
#undef GET_SCOPE_COMMAND_FUNC

#define CHANGE_CONTEXT_COMMAND_FUNC(func_name, context_types_bits, expected_types_bits, output_type_bit) \
    class has##func_name final : public interface { \
    public:                           \
      static const size_t type_index; \
      static const size_t context_types = context_types_bits; \
      static const size_t expected_types = INPUT_VALID_NUMBER; \
      static const size_t output_type = OUTPUT_NUMBER; \
      has##func_name(const interface* childs) noexcept; \
      has##func_name(const interface* childs, const interface* max_count, const interface* percentage) noexcept; \
      ~has##func_name() noexcept;     \
      struct object process(context* ctx) const override; \
      void draw(context* ctx) const override; \
    private:                          \
      const interface* max_count;     \
      const interface* percentage;    \
      const interface* childs;        \
    };                                \
    \
    class every##func_name final : public interface { \
    public:                           \
      static const size_t type_index; \
      static const size_t context_types = context_types_bits;   \
      static const size_t expected_types = expected_types_bits; \
      static const size_t output_type = output_type_bit;        \
      every##func_name(const interface* condition, const interface* childs) noexcept; \
      ~every##func_name() noexcept;   \
      struct object process(context* ctx) const override;       \
      void draw(context* ctx) const override;                   \
    private:                          \
      const interface* condition;     \
      const interface* childs;        \
    };                                \
    \
    class random##func_name final : public interface { \
    public:                           \
      static const size_t type_index; \
      static const size_t context_types = context_types_bits;   \
      static const size_t expected_types = expected_types_bits; \
      static const size_t output_type = output_type_bit;        \
      random##func_name(const interface* condition, const interface* weight, const interface* childs) noexcept; \
      ~random##func_name() noexcept;  \
      struct object process(context* ctx) const override;       \
      void draw(context* ctx) const override;                   \
    private:                          \
      const interface* condition;     \
      const interface* weight;        \
      const interface* childs; /*эффекты*/ \
    };                                \
    
    CHANGE_CONTEXT_COMMANDS_LIST
    
#undef CHANGE_CONTEXT_COMMAND_FUNC
  }
}

#endif
