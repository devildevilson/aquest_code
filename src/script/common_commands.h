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

// это все функции прочитать контекст и получить оттуда данные
// мне еще нужно сделать функции запоминания в контексте,
// причем желательно 3 уровня времени жизни: 
// локальный (внутри скриптового блока? или внутри метода?), 
// эвентовый (передается дальше по эвенту, текущий тоже? или нужно его сделать?)
// глобальный (доступен всем и всегда, может перезаписаться в разных эвентах)
// во первых в цк3 отдельно запоминаются скоупы и переменные
// во вторых переменные можно запомнить прямо в каком то объекте (например в персонаже)
// + локальные переменные для метода (immediate) + глобальные переменные
// а скоупы бывают временные (только метод, но могут быть в кондитион блоке)
// или обычные, которые потом перейдут в эвент
// в моем случае можно не разделять на переменную и скоуп, 
// но при этом лучше не давать возможности запоминать переменные (к тому же у меня уже есть флаги)
// как быть с перезаписью? можно ли перезаписывать в контексте? кто может перезаписывать данные?
// может ли перезаписать данные временная переменная? скорее нет, может ли перезаписать данные 
// обычное сохранение? нет, оно записывает в отдельный контейнер, 
// можно ли перезаписать данные в глобальном контексте? это делать нужно, но только в эффектах
// 

// так же нужен глобальный обход по всем группам объектов
// тип: every_character

// нужен custom_description, но при этом мне нужно как то пометить его конец
// hidden_effect - тупа ничего не делаем и ничего не рисуем (хотя наверное лучше будет так же как с custom_description, пометить конец блока)
// для некоторых эвентов было бы неплохо сделать списки
// send_interface_toast - отправить сообщение интерфейсу
// trigger_event - запустить эвент, как указать аргументы?
// show_as_tooltip - показать часть эффектов в тултипе, где? кому? в описании сказано, что в этом блоке эффекты только показать (но не выполнять)

#define MAXIMUM_OVERLOADS 16

namespace devils_engine {
  namespace script {
    class change_scope_condition final : public interface {
    public:
      // эта информация нужна в основном только для смены скоупа, для конечных функций она бесполезна
      //static const size_t expected_types = object::type_bit::valid_boolean;
      //static const size_t output_type = object::type_bit::boolean;
      change_scope_condition(const interface* scope, const interface* condition, const interface* child) noexcept;
      ~change_scope_condition() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      const interface* scope;
      const interface* condition;
      const interface* child;
    };
    
    class change_scope_effect final : public interface {
    public:
      static const size_t expected_types = 0;
      //static const size_t output_type = object::type_bit::invalid;
      
      change_scope_effect(const interface* scope, const interface* condition, const interface* childs) noexcept;
      ~change_scope_effect() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      const interface* scope;
      const interface* condition;
      const interface* childs;
    };
    
    class compute_string final : public interface {
    public:
      static const size_t expected_types = 0;
      //static const size_t output_type = object::type_bit::string;
      compute_string(const interface* condition, const interface* childs) noexcept;
      ~compute_string() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      const interface* condition;
      const interface* childs;
    };
    
    class compute_object final : public interface {
    public:
      compute_object(const interface* condition, const interface* childs) noexcept;
      ~compute_object() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      const interface* condition;
      const interface* childs;
    };
    
    class compute_number final : public interface {
    public:
      //static const size_t expected_types = object::type_bit::valid_number;
      //static const size_t output_type = object::type_bit::number;
      compute_number(const interface* scope, const interface* condition, const interface* child) noexcept;
      ~compute_number() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      const interface* scope;
      const interface* condition;
      const interface* child;
    };
    
    class selector final : public interface {
    public:
      static const size_t type_index;
      //static const size_t expected_types = object::type_bit::all;
      //static const size_t output_type = object::type_bit::all;
      selector(const interface* childs) noexcept;
      ~selector() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      const interface* childs;
    };
    
    class sequence final : public interface {
    public:
      static const size_t type_index;
      //static const size_t expected_types = object::type_bit::all;
      //static const size_t output_type = object::type_bit::all;
      sequence(const interface* count, const interface* childs) noexcept;
      ~sequence() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      const interface* count;
      const interface* childs;
    };
    
    class overload final : public interface {
    public:
      overload(const std::array<size_t, MAXIMUM_OVERLOADS> &overload_types, const interface* childs) noexcept;
      ~overload() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      std::array<size_t, MAXIMUM_OVERLOADS> overload_types;
      const interface* childs;
    };
    
    class chance final : public interface {
    public:
      static const size_t type_index;
      //static const size_t output_type = object::type_bit::boolean;
      chance(const size_t &state, const interface* value) noexcept;
      ~chance() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      size_t state;
      const interface* value;
    };
    
    // нужно сделать рандом по весам, как он должен выглядеть? 
    class weighted_random final : public interface {
    public:
      static const size_t type_index;
      //static const size_t expected_types = object::type_bit::all;
      //static const size_t output_type = object::type_bit::all;
      weighted_random(const size_t &state, const interface* childs, const interface* weights) noexcept;
      ~weighted_random() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      size_t state;
      const interface* childs;
      const interface* weights;
    };
    
    class random_value final : public interface {
    public:
      static const size_t type_index;
      //static const size_t output_type = object::type_bit::number;
      random_value(const size_t &state, const interface* maximum) noexcept;
      ~random_value() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      size_t state;
      const interface* maximum;
    };
    
    class boolean_container final : public interface {
    public:
      //static const size_t expected_types = object::type_bit::all;
      //static const size_t output_type = object::type_bit::boolean;
      boolean_container(const bool value) noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      bool value;
    };
    
    class number_container final : public interface {
    public:
      //static const size_t expected_types = object::type_bit::all;
      //static const size_t output_type = object::type_bit::number;
      number_container(const double &value) noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      double value;
    };
    
    class string_container final : public interface {
    public:
      //static const size_t expected_types = object::type_bit::all;
      //static const size_t output_type = object::type_bit::string;
      explicit string_container(const std::string &value) noexcept;
      explicit string_container(const std::string_view &value) noexcept;
      ~string_container() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      std::string value;
    };
    
    class object_container final : public interface {
    public:
      //static const size_t expected_types = object::type_bit::all;
      //static const size_t output_type = object::type_bit::all_objects;
      object_container(const object &value) noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      object value;
    };
    
    // нужно еще сравниватель объектов сделать
    
    class number_comparator final : public interface {
    public:
      //static const size_t expected_types = object::type_bit::boolean | object::type_bit::number;
      //static const size_t output_type = object::type_bit::boolean;
      number_comparator(const uint8_t op, const interface* lvalue, const interface* rvalue) noexcept;
      ~number_comparator() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      uint8_t op;
      const interface* lvalue;
      const interface* rvalue;
    };
    
    class boolean_comparator final : public interface {
    public:
      //static const size_t expected_types = object::type_bit::boolean | object::type_bit::number;
      //static const size_t output_type = object::type_bit::boolean;
      boolean_comparator(const interface* lvalue, const interface* rvalue) noexcept;
      ~boolean_comparator() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      const interface* lvalue;
      const interface* rvalue;
    };
    
    class equals_to final : public interface {
    public:
      static const size_t type_index;
      //static const size_t expected_types = object::type_bit::all;
      //static const size_t output_type = object::type_bit::boolean;
      equals_to(const interface* get_obj) noexcept;
      ~equals_to() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      const interface* get_obj;
    };
    
    class not_equals_to final : public interface {
    public:
      static const size_t type_index;
      //static const size_t expected_types = object::type_bit::all;
      //static const size_t output_type = object::type_bit::boolean;
      not_equals_to(const interface* get_obj) noexcept;
      ~not_equals_to() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      const interface* get_obj;
    };
    
    // если добавится функция compare, то тут наверное было бы неплохо сделать по умолчанию объект
    class equality final : public interface {
    public:
      static const size_t type_index;
      //static const size_t expected_types = object::type_bit::all;
      //static const size_t output_type = object::type_bit::boolean;
      equality(const interface* childs) noexcept;
      ~equality() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      const interface* childs;
    };
    
    class type_equality final : public interface {
    public:
      static const size_t type_index;
      //static const size_t expected_types = object::type_bit::all;
      //static const size_t output_type = object::type_bit::boolean;
      type_equality(const interface* childs) noexcept;
      ~type_equality() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      const interface* childs;
    };
    
    // эта функция может ответить на вопрос является ли первое число самым маленьким
    // или быстрая проверка нескольких чисел на то что они больше нуля
    class compare final : public interface {
    public:
      static const size_t type_index;
      //static const size_t expected_types = object::type_bit::valid_number;
      //static const size_t output_type = object::type_bit::boolean;
      compare(const uint8_t op, const interface* childs) noexcept;
      ~compare() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      uint8_t op;
      const interface* childs;
    };
    
    class complex_object final : public interface {
    public:
      complex_object(const interface* childs) noexcept;
      ~complex_object() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      const interface* childs;
    };
    
    class root final : public interface {
    public:
      static const size_t type_index;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    };
    
    class prev final : public interface {
    public:
      static const size_t type_index;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    };
    
    class current final : public interface {
    public:
      static const size_t type_index;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    };
    
    class index final : public interface {
    public:
      static const size_t type_index;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    };
    
    class prev_index final : public interface {
    public:
      static const size_t type_index;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    };
    
    class value final : public interface {
    public:
      static const size_t type_index;
      value(const interface* compute) noexcept;
      ~value() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      const interface* compute;
    };
    
    class get_context final : public interface {
    public:
      static const size_t type_index;
      explicit get_context(const std::string &str) noexcept;
      explicit get_context(const std::string_view &str) noexcept;
      ~get_context() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      std::string name;
    };
    
#define GET_ENTITY_HELPER_FUNC(name) \
    class get_##name final : public interface {                      \
    public:                                                          \
      /*static const size_t expected_types = object::type_bit::string;*/ \
      /*static const size_t output_type = object::type_bit::name;*/      \
      static const size_t type_index;                                \
      get_##name(const interface* str) noexcept;                     \
      ~get_##name() noexcept;                                        \
      struct object process(context* ctx) const override;            \
      void draw(context* ctx) const override;                        \
      size_t get_type_id() const override;                           \
      std::string_view get_name() const override;                    \
    private:                                                         \
      const interface* str;                                          \
    };                                                               \
    
    GET_ENTITY_HELPER_FUNC(culture)
    GET_ENTITY_HELPER_FUNC(culture_group)
    GET_ENTITY_HELPER_FUNC(religion)
    GET_ENTITY_HELPER_FUNC(religion_group)
    GET_ENTITY_HELPER_FUNC(trait)
    GET_ENTITY_HELPER_FUNC(modificator)
    GET_ENTITY_HELPER_FUNC(titulus)
    GET_ENTITY_HELPER_FUNC(casus_belli)
    GET_ENTITY_HELPER_FUNC(building_type)
    GET_ENTITY_HELPER_FUNC(holding_type)
    GET_ENTITY_HELPER_FUNC(city_type)
    GET_ENTITY_HELPER_FUNC(troop_type)
    GET_ENTITY_HELPER_FUNC(law)
    
#undef GET_ENTITY_HELPER_FUNC
    
    // тут мы должны запомнить переменную только в методе, может быть лучше сменить название? save_method_var? просто save_temporary? последнее мне нравится больше
    // нужно ли к названию добавить _as?
//     class save_temporary final : public interface {
//     public:
//       static const size_t type_index;
//       save_temporary(const interface* str, const interface* var) noexcept;
//       ~save_temporary() noexcept;
//       object process(context* ctx) const override;
//       void draw(context* ctx) const override;
//     private:
//       const interface* str;
//       const interface* var; // если вар не задан, то сохраняем куррент
//     };
    
    // сохраняем для всего энтити (то есть для всего эвента например, или для всей интеракции)
    class save_local final : public interface {
    public:
      static const size_t type_index;
      save_local(const std::string &name, const size_t &index, const interface* var) noexcept;
      ~save_local() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      std::string name;
      size_t index;
      const interface* var; // если вар не задан, то сохраняем куррент
    };
    
    // сохраняем отдельно для будущего использования (например в эвенте), только в эффектах
    class save final : public interface {
    public:
      static const size_t type_index;
      save(const interface* str, const interface* var) noexcept;
      ~save() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      const interface* str;
      const interface* var; // если вар не задан, то сохраняем куррент
    };
    
    // сохраняем в специальном глобальном контексте
    class save_global final : public interface {
    public:
      static const size_t type_index;
      save_global(const interface* str, const interface* var) noexcept;
      ~save_global() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      const interface* str;
      const interface* var; // если вар не задан, то сохраняем куррент
    };
    
    // возможно имеет смысл еще запоминать листы (переменные массивы)
    
    // как проверить тип? да и нужно ли его проверять таким способом или лучше как то иначе
    // когда мы попадаем в скоуп переменной там можно вызвать функции проверки типа, а что если это число или строка?
    // ну у них функции не определены, кроме видимо проверок типа
    class has_local final : public interface {
    public:
      static const size_t type_index;
      has_local(const size_t &index) noexcept;
      ~has_local() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      size_t index;
    };
    
    class has_global final : public interface {
    public:
      static const size_t type_index;
      has_global(const interface* str) noexcept;
      ~has_global() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      const interface* str;
    };
    
    // для того чтобы это работало как флаги, переменные нужно еще выгружать из контекста
    class remove_local final : public interface {
    public:
      static const size_t type_index;
      remove_local(const std::string &name, const size_t &index) noexcept;
      ~remove_local() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      std::string name;
      size_t index;
    };
    
    class remove_global final : public interface {
    public:
      static const size_t type_index;
      remove_global(const interface* str) noexcept;
      ~remove_global() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      const interface* str;
    };
    
    // имеет смысл хранить название для дебага
    class get_local final : public interface {
    public:
      static const size_t type_index;
      get_local(const std::string &name, const size_t &index) noexcept;
      ~get_local() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      std::string name;
      size_t index;
    };
    
    class add_to_list final : public interface {
    public:
      static const size_t type_index;
      add_to_list(const std::string &name) noexcept;
      ~add_to_list() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      std::string name;
    };
    
    class is_in_list final : public interface {
    public:
      static const size_t type_index;
      is_in_list(const std::string &name) noexcept;
      ~is_in_list() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      std::string name;
    };
    
    class has_in_list final : public interface {
    public:
      static const size_t type_index;
      has_in_list(const std::string &name, const interface* max_count, const interface* percentage, const interface* childs) noexcept;
      ~has_in_list() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      std::string name;
      const interface* max_count;
      const interface* percentage;
      const interface* childs;
    };
    
    class random_in_list final : public interface {
    public:
      static const size_t type_index;
      random_in_list(const std::string &name, const size_t &state, const interface* condition, const interface* weight, const interface* childs) noexcept;
      ~random_in_list() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      std::string name;
      size_t state;
      const interface* condition;
      const interface* weight;
      const interface* childs;
    };
    
    class every_in_list_numeric final : public interface {
    public:
      static const size_t type_index;
      every_in_list_numeric(const std::string &name, const interface* condition, const interface* childs) noexcept;
      ~every_in_list_numeric() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      std::string name;
      const interface* condition;
      const interface* childs;
    };
    
    class every_in_list_logic final : public interface {
    public:
      static const size_t type_index;
      every_in_list_logic(const std::string &name, const interface* condition, const interface* childs) noexcept;
      ~every_in_list_logic() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      std::string name;
      const interface* condition;
      const interface* childs;
    };
    
    class every_in_list_effect final : public interface {
    public:
      static const size_t type_index;
      every_in_list_effect(const std::string &name, const interface* condition, const interface* childs) noexcept;
      ~every_in_list_effect() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      std::string name;
      const interface* condition;
      const interface* childs;
    };
    
    class list_view final : public interface {
    public:
      static const size_t type_index;
      list_view(const std::string &name, const interface* default_value, const interface* childs) noexcept;
      ~list_view() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      std::string name;
      const interface* default_value;
      const interface* childs;
    };
    
    // по идее тут просто все: делаем действия возращаем объект, это скорее со стороны инициализации вопрсо
    class transform final : public interface {
    public:
      static const size_t type_index;
      transform(const interface* changes) noexcept;
      ~transform() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      const interface* changes;
    };
    
    class filter final : public interface {
    public:
      static const size_t type_index;
      filter(const interface* condition) noexcept;
      ~filter() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      const interface* condition;
    };
    
    class reduce final : public interface {
    public:
      static const size_t type_index;
      reduce(const interface* value) noexcept;
      ~reduce() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      const interface* value;
    };
    
    class take final : public interface {
    public:
      static const size_t type_index;
      take(const size_t count) noexcept;
      ~take() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      size_t count;
    };
    
    class drop final : public interface {
    public:
      static const size_t type_index;
      drop(const size_t count) noexcept;
      ~drop() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      size_t count;
    };
    
    class assert_condition final : public interface {
    public:
      static const size_t type_index;
      assert_condition(const interface* condition, const interface* str) noexcept;
      ~assert_condition() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      const interface* condition;
      const interface* str;
    };
    
    // нужен еще debug_log
  }
}

#endif
