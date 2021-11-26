#include "common_commands.h"

#include "utils/assert.h"
#include "context.h"
#include "core.h"
#include "core/context.h"
#include "utils/globals.h"
#include "utils/systems.h"

namespace devils_engine {
  namespace script {
    change_scope_condition::change_scope_condition(const interface* scope, const interface* condition, const interface* child) noexcept : scope(scope), condition(condition), child(child) {}
    change_scope_condition::~change_scope_condition() noexcept {
      if (scope != nullptr) scope->~interface();
      if (condition != nullptr) condition->~interface();
      child->~interface();
    }
    
    object change_scope_condition::process(context* ctx) const {
      object prev_scope = ctx->prev;
      object new_scope = ctx->current;
      if (scope != nullptr) {
        prev_scope = new_scope;
        new_scope = scope->process(ctx);
      }
      
      change_scope sc(ctx, new_scope, prev_scope);
      
      if (condition != nullptr) {
        const auto &ret = condition->process(ctx);
        if (!ret.get<bool>()) return ignore_value;
      }
      
//       for (auto cur = childs; cur != nullptr; cur = cur->next) {
//         const auto &ret = cur->process(ctx);
//         if (!ret.ignore() && !ret.get<bool>()) return object(false);
//       }

      return child->process(ctx);
    }
    
    void change_scope_condition::draw(context* ctx) const {
      object prev_scope = ctx->prev;
      object new_scope = ctx->current;
      size_t nesting = ctx->nest_level;
      if (scope != nullptr) {
        scope->draw(ctx);
        prev_scope = new_scope;
        new_scope = scope->process(ctx);
        nesting += 1;
      }
      
      change_scope sc(ctx, new_scope, prev_scope);
      change_nesting сn(ctx, nesting);
      
      if (condition != nullptr) {
        const auto &ret = condition->process(ctx);
        draw_data dd(ctx);
        dd.function_name = "condition";
        dd.value = object(ret.get<bool>());
        ctx->draw_function(&dd);
        change_nesting cn(ctx, ++ctx->nest_level);
        condition->draw(ctx);
      }
      
      child->draw(ctx);
    }
    
    change_scope_effect::change_scope_effect(const interface* scope, const interface* condition, const interface* childs) noexcept : scope(scope), condition(condition), childs(childs) {}
    change_scope_effect::~change_scope_effect() noexcept {
      if (scope != nullptr) scope->~interface();
      if (condition != nullptr) condition->~interface();
      for (auto cur = childs; cur != nullptr; cur = cur->next) { cur->~interface(); }
    }
    
    object change_scope_effect::process(context* ctx) const {
      object prev_scope = ctx->prev;
      object new_scope = ctx->current;
      if (scope != nullptr) {
        prev_scope = new_scope;
        new_scope = scope->process(ctx);
      }
      
      change_scope sc(ctx, new_scope, prev_scope);
      
      if (condition != nullptr) {
        const auto &ret = condition->process(ctx);
        if (!ret.get<bool>()) return ignore_value;
      }
      
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        cur->process(ctx); // реагируем ли мы тут как то если получаем ignore_value? если бы это был ifelse то тогда бы реагировали
      }

      return object();
    }
    
    void change_scope_effect::draw(context* ctx) const {
      object prev_scope = ctx->prev;
      object new_scope = ctx->current;
      size_t nesting = ctx->nest_level;
      if (scope != nullptr) {
        scope->draw(ctx); // вызываем ли тут вообще драв? может быть нужно просто получить скоуп, наверрное все же вызываем
        prev_scope = new_scope;
        new_scope = scope->process(ctx);
        nesting += 1;
      }
      
      change_scope sc(ctx, new_scope, prev_scope);
      change_nesting cn(ctx, nesting);
      
      // нужно как то дать понять что выполняется условие
      if (condition != nullptr) {
        const auto &ret = condition->process(ctx);
        draw_data dd(ctx);
        dd.function_name = "condition";
        dd.value = object(ret.get<bool>());
        ctx->draw_function(&dd);
        change_nesting cn(ctx, ++ctx->nest_level);
        condition->draw(ctx); 
      }
      
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        cur->draw(ctx);
      }
    }
    
    compute_string::compute_string(const interface* condition, const interface* childs) noexcept : condition(condition), childs(childs) {}
    compute_string::~compute_string() noexcept {
      if (condition != nullptr) condition->~interface();
      for (auto cur = childs; cur != nullptr; cur = cur->next) { cur->~interface(); }
    }
    
    object compute_string::process(context* ctx) const {
      if (condition != nullptr) {
        const auto &ret = condition->process(ctx);
        if (!ret.get<bool>()) return ignore_value;
      }
      
      object cur_obj = ignore_value;
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        cur_obj = cur->process(ctx);
      }
      
      return cur_obj;
    }
    
    void compute_string::draw(context*) const {
      // будет ли тут драв?
      assert(false);
    }
    
    compute_number::compute_number(const interface* scope, const interface* condition, const interface* child) noexcept : scope(scope), condition(condition), child(child) {}
    compute_number::~compute_number() noexcept {
      if (scope != nullptr) scope->~interface();
      if (condition != nullptr) condition->~interface();
      child->~interface();
    }
    object compute_number::process(context* ctx) const {
      object prev_scope = ctx->prev;
      object new_scope = ctx->current;
      if (scope != nullptr) {
        prev_scope = new_scope;
        new_scope = scope->process(ctx);
      }
      
      change_scope sc(ctx, new_scope, prev_scope);
      
      if (condition != nullptr) {
        const auto &ret = condition->process(ctx);
        if (!ret.get<bool>()) return ignore_value;
      }

      return child->process(ctx);
    }
    
    void compute_number::draw(context* ctx) const {
      object prev_scope = ctx->prev;
      object new_scope = ctx->current;
      size_t nesting = ctx->nest_level;
      if (scope != nullptr) {
        scope->draw(ctx);
        prev_scope = new_scope;
        new_scope = scope->process(ctx);
        nesting += 1;
      }
      
      change_scope sc(ctx, new_scope, prev_scope);
      change_nesting сn(ctx, nesting);
      
      if (condition != nullptr) {
        const auto &ret = condition->process(ctx);
        draw_data dd(ctx);
        dd.function_name = "condition";
        dd.value = object(ret.get<bool>());
        ctx->draw_function(&dd);
        change_nesting cn(ctx, ++ctx->nest_level);
        condition->draw(ctx);
      }
      
      child->draw(ctx);
    }
    
    boolean_container::boolean_container(const bool value) noexcept : value(value) {}
    object boolean_container::process(context*) const { return object(value); }
    void boolean_container::draw(context*) const { assert(false); }
    
    number_container::number_container(const double &value) noexcept : value(value) {}
    object number_container::process(context*) const { return object(value); }
    void number_container::draw(context*) const { assert(false); }
  
    string_container::string_container(const std::string &value) noexcept : value(value) {}
    string_container::string_container(const std::string_view &value) noexcept : value(value) {}
    object string_container::process(context*) const { return object(value); }
    void string_container::draw(context*) const { assert(false); }
  
    object_container::object_container(const object &value) noexcept : value(value) {}
    object object_container::process(context*) const { return value; }
    void object_container::draw(context*) const { assert(false); }
    
    static bool compare(const uint8_t op, const double num1, const double num2) {
      const bool eq = std::abs(num1 - num2) < EPSILON;
      switch (static_cast<compare_operators::values>(op)) {
        case compare_operators::equal:     return eq;
        case compare_operators::not_equal: return !eq;
        case compare_operators::more:      return num1 > num2;
        case compare_operators::less:      return num1 < num2;
        case compare_operators::more_eq:   return num1 > num2 || eq;
        case compare_operators::less_eq:   return num1 < num2 || eq;
        default: throw std::runtime_error("Add aditional comparisons");
      }
      
      return false;
    }
    
    number_comparator::number_comparator(const uint8_t op, const interface* lvalue, const interface* rvalue) noexcept : op(op), lvalue(lvalue), rvalue(rvalue) {}
    number_comparator::~number_comparator() noexcept {
      lvalue->~interface();
      rvalue->~interface();
    }
    
    object number_comparator::process(context* ctx) const {
      const auto &num1 = lvalue->process(ctx);
      const auto &num2 = rvalue->process(ctx);
      const double num1_final = num1.get<double>();
      const double num2_final = num2.get<double>();
      return object(compare(op, num1_final, num2_final));
    }
    
    void number_comparator::draw(context* ctx) const {
      const auto &num2 = rvalue->process(ctx);
      change_rvalue crv(ctx, num2, op);
      lvalue->draw(ctx);
    }
    
    boolean_comparator::boolean_comparator(const interface* lvalue, const interface* rvalue) noexcept : lvalue(lvalue), rvalue(rvalue) {}
    boolean_comparator::~boolean_comparator() noexcept {
      lvalue->~interface();
      rvalue->~interface();
    }
    object boolean_comparator::process(context* ctx) const {
      const auto &num1 = lvalue->process(ctx);
      const auto &num2 = rvalue->process(ctx);
      const bool num1_final = num1.get<bool>();
      const bool num2_final = num2.get<bool>();
      return object(num1_final == num2_final);
    }
    
    void boolean_comparator::draw(context* ctx) const {
      const auto &num2 = rvalue->process(ctx);
      // вот где нужно как то выкручиваться, положить рвалуе в контекст? или возможно наследовать компаратор?
      // не, наследование компаратора выглядит бессмысленным, можно и правда положить рвалуе только для рендеринга
      change_rvalue cr(ctx, num2, 0);
      lvalue->draw(ctx);
    }
      
    // в текущем объекте к нам должен был придти новый контекст
    // меняем его пока не достигнем последнего ребенка, 
    // в последнем ребенке должен находиться интересующий нас объект
    // как можно проверить при создании? такое поведение задается с помощью строки
    // при парсинге строки будет видно что возвращается 
    complex_object::complex_object(const interface* childs) noexcept : childs(childs) {}
    complex_object::~complex_object() noexcept {
      for (auto cur = childs; cur != nullptr; cur = cur->next) { cur->~interface(); }
    }
    object complex_object::process(context* ctx) const {
      change_scope sc(ctx, ctx->current, ctx->prev);
      
      // надо запомнить старый контекст
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        const auto &obj = cur->process(ctx);
        ctx->prev = ctx->current;
        ctx->current = obj;
      }
      
      assert(ctx->current.valid() && !ctx->current.ignore());
      return ctx->current;
    }
    
    void complex_object::draw(context*) const { assert(false); }
    
    object root::process(context* ctx) const { return ctx->root; }
    void root::draw(context*) const { assert(false); } // мы должны что то нарисовать
    object prev::process(context* ctx) const { return ctx->prev; }
    void prev::draw(context*) const { assert(false); }
    
    get_context::get_context(const std::string &str) noexcept : name(str) {}
    get_context::get_context(const std::string_view &str) noexcept : name(str) {}
    object get_context::process(context* ctx) const {
      const auto itr = ctx->map.find(name);
      if (itr == ctx->map.end()) throw std::runtime_error("Could not find context object using key " + name);
      return itr->second;
    }
    
    void get_context::draw(context* ctx) const {
      const auto obj = process(ctx);
      // тут нам нужно как то словестно описать что мы получили на основе типа
    }
    
#define GET_ENTITY_HELPER_FUNC(name) \
    get_##name::get_##name(const interface* str) noexcept : str(str) {} \
    get_##name::~get_##name() noexcept { str->~interface(); }           \
    void get_##name::draw(context*) const {                             \
      assert(false);                                                    \
    }                                                                   \
    object get_##name::process(context* ctx) const {               \
      const auto str_obj = str->process(ctx);                      \
      const auto view = str_obj.get<std::string_view>();           \
      auto core_ctx = global::get<systems::map_t>()->core_context; \
      auto entity = core_ctx->get_entity<core::name>(view);        \
      if (entity == nullptr) throw std::runtime_error("Could not find "#name" " + std::string(view)); \
      return object(entity);                                       \
    }                                                              \
    
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
