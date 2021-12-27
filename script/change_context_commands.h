#ifndef DEVILS_ENGINE_SCRIPT_CHANGE_CONTEXT_COMMANDS_H
#define DEVILS_ENGINE_SCRIPT_CHANGE_CONTEXT_COMMANDS_H

#include "object.h"
#include "interface.h"
#include "change_context_commands_macro.h"

#define EVERY_FUNC_EFFECT 0
#define EVERY_FUNC_LOGIC 1
#define EVERY_FUNC_NUMERIC 2

// для того чтобы тут сделать функцию compute, нужно будет создать массив для every с каждым объектом и итоговым значением, мех
// тут даже дело не в этом, а в том что невозможно заранее посчитать число для каждого
// выход тут один поменять функцию рандомизации: она не должна зависеть от предыдущего значения
// она должна зависеть от некоторых текущих значений контекста, от каких?
// в рандомные функции мы можем положить дополнительно статический стейт и тогда у нас будет:
// уникальный id интеракции/эвента/десижона/проч, уникальное (для объекта) название метода, текущий ход, уникальное число для каждого экземпляра функции рандомизации
// что то еще? еще есть название функции рандомизации, но оно не уникально, по идее этого должно хватить

namespace devils_engine {
  namespace script {
    struct context;

#define CHANGE_CONTEXT_COMMAND_FUNC(func_name, context_types_bits, expected_types_bits, output_type_bit) \
    class has_##func_name final : public interface {           \
    public:                                                    \
      static const size_t type_index;                          \
      static const size_t context_types = context_types_bits;  \
      static const size_t expected_types = INPUT_VALID_NUMBER; \
      static const size_t output_type = OUTPUT_NUMBER;         \
      has_##func_name(const interface* childs) noexcept;       \
      has_##func_name(const interface* childs, const interface* max_count, const interface* percentage) noexcept; \
      ~has_##func_name() noexcept;                             \
      struct object process(context* ctx) const override;      \
      void draw(context* ctx) const override;                  \
    private:                          \
      const interface* max_count;     \
      const interface* percentage;    \
      const interface* childs;        \
    };                                \
    \
    class every_##func_name final : public interface {          \
    public:                                                     \
      static const size_t type_index;                           \
      static const size_t context_types = context_types_bits;   \
      static const size_t expected_types = expected_types_bits; \
      static const size_t output_type = output_type_bit;        \
      every_##func_name(const size_t &type, const interface* condition, const interface* childs) noexcept; \
      ~every_##func_name() noexcept;                            \
      struct object process(context* ctx) const override;       \
      void draw(context* ctx) const override;                   \
    private:                          \
      size_t type;                    \
      const interface* condition;     \
      const interface* childs;        \
    };                                \
    \
    class random_##func_name final : public interface {         \
    public:                                                     \
      static const size_t type_index;                           \
      static const size_t context_types = context_types_bits;   \
      static const size_t expected_types = expected_types_bits; \
      static const size_t output_type = output_type_bit;        \
      random_##func_name(const size_t &state, const interface* condition, const interface* weight, const interface* childs) noexcept; \
      ~random_##func_name() noexcept;                           \
      struct object process(context* ctx) const override;       \
      void draw(context* ctx) const override;                   \
    private:                          \
      size_t state;                   \
      const interface* condition;     \
      const interface* weight;        \
      const interface* childs; /*эффекты*/ \
    };                                \
    
    CHANGE_CONTEXT_COMMANDS_FINAL_LIST
    
#undef CHANGE_CONTEXT_COMMAND_FUNC
  }
}

#endif
