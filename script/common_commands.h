#ifndef DEVILS_ENGINE_SCRIPT_COMMON_COMMANDS_H
#define DEVILS_ENGINE_SCRIPT_COMMON_COMMANDS_H

#include "utils/shared_mathematical_constant.h"
#include "interface.h"
#include "utils/assert.h"

// в таком виде значительно чище, осталось только входные/выходные данные просмотреть и сгенерировать функции создания
// нужно положить весь скрипт рядом, для этого нужно 2 раза обойти таблицу создания: собрать размеры, алигмент и количество,
// а потом создать контейнер и уже с контейнером пройтись по таблице

// взять объект из контекста, проверить наличие флага в контексте, обход строковых данных, ???

// нужно добавить ожидаемые и выходные типы функций

namespace devils_engine {
  namespace script {
    // нужно ли отдельно делать кондишен_блок? я могу в change_scope_condition скоуп указать как nullptr
    
    // кондишенов у меня будет несколько (И, ИЛИ, НЕИ, НЕИЛИ), здесь надо бы указать не детей, а одного ребенка - тип операции
    class change_scope_condition final : public interface {
    public:
      // эта информация нужна в основном только для смены скоупа, для конечных функций она бесполезна
      static const size_t expected_types = object::type_bit::valid_boolean;
      static const size_t output_type = object::type_bit::boolean;
      change_scope_condition(const interface* scope, const interface* condition, const interface* child) noexcept;
      ~change_scope_condition() noexcept;
      object process(context* ctx) const override;
      void draw(context* ctx) const override;
    private:
      const interface* scope;
      const interface* condition;
      const interface* child;
    };
    
    class change_scope_effect final : public interface {
    public:
      static const size_t expected_types = object::type_bit::invalid;
      static const size_t output_type = object::type_bit::invalid;
      
      change_scope_effect(const interface* scope, const interface* condition, const interface* childs) noexcept;
      ~change_scope_effect() noexcept;
      object process(context* ctx) const override;
      void draw(context* ctx) const override;
      // возможно имеет смысл добавить какой нибудь способ посчитать условия заранее, хотя возможно лучше просто избегать некоторых ситуаций by_design
      // (может возникнуть ситуация когда до условия мы поменяем стат и у условия дальше может измениться итоговое значение)
    private:
      const interface* scope;
      const interface* condition;
      const interface* childs;
    };
    
    class compute_string final : public interface {
    public:
      static const size_t expected_types = 0;
      static const size_t output_type = object::type_bit::string;
      compute_string(const interface* condition, const interface* childs) noexcept;
      ~compute_string() noexcept;
      object process(context* ctx) const override;
      void draw(context* ctx) const override;
    private:
      const interface* condition;
      const interface* childs;
    };
    
    class compute_number final : public interface {
    public:
      static const size_t expected_types = object::type_bit::valid_number;
      static const size_t output_type = object::type_bit::number;
      compute_number(const interface* scope, const interface* condition, const interface* child) noexcept;
      ~compute_number() noexcept;
      object process(context* ctx) const override;
      void draw(context* ctx) const override;
    private:
      const interface* scope;
      const interface* condition;
      const interface* child;
    };
    
    class boolean_container final : public interface {
    public:
      static const size_t expected_types = object::type_bit::all;
      static const size_t output_type = object::type_bit::boolean;
      boolean_container(const bool value) noexcept;
      object process(context* ctx) const override;
      void draw(context* ctx) const override;
    private:
      bool value;
    };
    
    class number_container final : public interface {
    public:
      static const size_t expected_types = object::type_bit::all;
      static const size_t output_type = object::type_bit::number;
      number_container(const double &value) noexcept;
      object process(context* ctx) const override;
      void draw(context* ctx) const override;
    private:
      double value;
    };
    
    class string_container final : public interface {
    public:
      static const size_t expected_types = object::type_bit::all;
      static const size_t output_type = object::type_bit::string;
      explicit string_container(const std::string &value) noexcept;
      explicit string_container(const std::string_view &value) noexcept;
      object process(context* ctx) const override;
      void draw(context* ctx) const override;
    private:
      std::string value;
    };
    
    class object_container final : public interface {
    public:
      static const size_t expected_types = object::type_bit::all;
      static const size_t output_type = object::type_bit::all_objects;
      object_container(const object &value) noexcept;
      object process(context* ctx) const override;
      void draw(context* ctx) const override;
    private:
      object value;
    };
    
    class number_comparator final : public interface {
    public:
      static const size_t expected_types = object::type_bit::boolean | object::type_bit::number;
      static const size_t output_type = object::type_bit::boolean;
      number_comparator(const uint8_t op, const interface* lvalue, const interface* rvalue) noexcept;
      ~number_comparator() noexcept;
      object process(context* ctx) const override;
      void draw(context* ctx) const override;
    private:
      uint8_t op;
      const interface* lvalue;
      const interface* rvalue;
    };
    
    class boolean_comparator final : public interface {
    public:
      static const size_t expected_types = object::type_bit::boolean | object::type_bit::number;
      static const size_t output_type = object::type_bit::boolean;
      boolean_comparator(const interface* lvalue, const interface* rvalue) noexcept;
      ~boolean_comparator() noexcept;
      object process(context* ctx) const override;
      void draw(context* ctx) const override;
    private:
      const interface* lvalue;
      const interface* rvalue;
    };
    
    class complex_object final : public interface {
    public:
      complex_object(const interface* childs) noexcept;
      ~complex_object() noexcept;
      object process(context* ctx) const override;
      void draw(context* ctx) const override;
    private:
      const interface* childs;
    };
    
    class root final : public interface {
    public:
      object process(context* ctx) const override;
      void draw(context* ctx) const override;
    };
    
    class prev final : public interface {
    public:
      object process(context* ctx) const override;
      void draw(context* ctx) const override;
    };
    
    class get_context final : public interface {
    public:
      explicit get_context(const std::string &str) noexcept;
      explicit get_context(const std::string_view &str) noexcept;
      object process(context* ctx) const override;
      void draw(context* ctx) const override;
    private:
      std::string name;
    };
    
#define GET_ENTITY_HELPER_FUNC(name) \
    class get_##name final : public interface {                      \
    public:                                                          \
      static const size_t expected_types = object::type_bit::string; \
      static const size_t output_type = object::type_bit::name;      \
      get_##name(const interface* str) noexcept;                     \
      ~get_##name() noexcept;                                        \
      object process(context* ctx) const override;                   \
      void draw(context* ctx) const override;                        \
    private:                                                         \
      const interface* str;                                          \
    };
    
    GET_ENTITY_HELPER_FUNC(culture)
    GET_ENTITY_HELPER_FUNC(culture_group)
    GET_ENTITY_HELPER_FUNC(religion)
    GET_ENTITY_HELPER_FUNC(religion_group)
    GET_ENTITY_HELPER_FUNC(trait)
    GET_ENTITY_HELPER_FUNC(modificator)
    GET_ENTITY_HELPER_FUNC(titulus)
    GET_ENTITY_HELPER_FUNC(casus_belli)
    
#undef GET_ENTITY_HELPER_FUNC
  }
}

#endif
