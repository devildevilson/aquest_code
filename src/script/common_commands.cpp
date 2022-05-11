#include "common_commands.h"

#include "utils/assert.h"
#include "context.h"
#include "core.h"
#include "core/context.h"
#include "utils/globals.h"
#include "utils/systems.h"

#include <iostream>

namespace devils_engine {
  namespace script {
    //const size_t change_scope_condition::expected_types;
    //const size_t change_scope_condition::output_type;
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
        if (ret.ignore() || !ret.get<bool>()) return ignore_value;
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
      nesting += size_t(scope != nullptr || condition != nullptr);
      if (scope != nullptr) {
        scope->draw(ctx);
        prev_scope = new_scope;
        new_scope = scope->process(ctx);
      }
      
      {
        const auto &ret = process(ctx);
        draw_data dd(ctx);
        dd.function_name = get_name();
        dd.value = ret;
        if (!ctx->draw(&dd)) return;
      }
      
      change_scope sc(ctx, new_scope, prev_scope);
      change_nesting сn(ctx, nesting);
      
      if (condition != nullptr) {
        {
          const auto &ret = condition->process(ctx);
          draw_data dd(ctx);
          dd.function_name = "condition";
          dd.value = object(ret.get<bool>());
          ctx->draw(&dd);
        }
        change_nesting cn(ctx, ctx->nest_level+1);
        change_function_name cfn(ctx, "condition");
        condition->draw(ctx);
      }
      
      child->draw(ctx);
    }
    
    size_t change_scope_condition::get_type_id() const { return type_id<object>(); }
    std::string_view change_scope_condition::get_name() const { return "change_scope_condition"; }
    
    const size_t change_scope_effect::expected_types;
    //const size_t change_scope_effect::output_type;
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
        if (ret.ignore() || !ret.get<bool>()) return ignore_value;
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
      nesting += size_t(scope != nullptr || condition != nullptr);
      if (scope != nullptr) {
        scope->draw(ctx); // вызываем ли тут вообще драв? может быть нужно просто получить скоуп, наверрное все же вызываем
        prev_scope = new_scope;
        new_scope = scope->process(ctx);
      }
      
      {
        const auto &ret = process(ctx);
        draw_data dd(ctx);
        dd.function_name = get_name();
        dd.value = ret;
        if (!ctx->draw(&dd)) return;
      }
      
      change_scope sc(ctx, new_scope, prev_scope);
      change_nesting cn(ctx, nesting);
      
      // нужно как то дать понять что выполняется условие
      if (condition != nullptr) {
        {
          const auto &ret = condition->process(ctx);
          draw_data dd(ctx);
          dd.function_name = "condition";
          dd.value = object(ret.get<bool>());
          ctx->draw(&dd);
        }
        change_nesting cn(ctx, ctx->nest_level+1);
        change_function_name cfn(ctx, "condition");
        condition->draw(ctx); 
      }
      
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        cur->draw(ctx);
      }
    }
    
    size_t change_scope_effect::get_type_id() const { return type_id<object>(); }
    std::string_view change_scope_effect::get_name() const { return "change_scope_effect"; }
    
    const size_t compute_string::expected_types;
    //const size_t compute_string::output_type;
    compute_string::compute_string(const interface* condition, const interface* childs) noexcept : condition(condition), childs(childs) {}
    compute_string::~compute_string() noexcept {
      if (condition != nullptr) condition->~interface();
      for (auto cur = childs; cur != nullptr; cur = cur->next) { cur->~interface(); }
    }
    
    object compute_string::process(context* ctx) const {
      if (condition != nullptr) {
        const auto &ret = condition->process(ctx);
        if (ret.ignore() || !ret.get<bool>()) return ignore_value;
      }
      
      object cur_obj = ignore_value;
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        cur_obj = cur->process(ctx);
      }
      
      return cur_obj;
    }
    
    void compute_string::draw(context* ctx) const {
      {
        const auto &ret = process(ctx);
        draw_data dd(ctx);
        dd.function_name = get_name();
        dd.value = ret;
        if (!ctx->draw(&dd)) return;
      }
      
      change_nesting cn(ctx, ctx->nest_level+1);
      
      if (condition != nullptr) {
        {
          const auto &ret = condition->process(ctx);
          draw_data dd(ctx);
          dd.function_name = "condition";
          dd.value = object(ret.get<bool>());
          ctx->draw(&dd);
        }
        change_nesting cn(ctx, ctx->nest_level+1);
        change_function_name cfn(ctx, "condition");
        condition->draw(ctx);
      }
      
      change_function_name cfn(ctx, get_name());
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        cur->draw(ctx);
      }
    }
    
    size_t compute_string::get_type_id() const { return type_id<object>(); }
    std::string_view compute_string::get_name() const { return "compute_string"; }
    
    compute_object::compute_object(const interface* condition, const interface* childs) noexcept : condition(condition), childs(childs) {}
    compute_object::~compute_object() noexcept {
      if (condition != nullptr) condition->~interface();
      for (auto cur = childs; cur != nullptr; cur = cur->next) { cur->~interface(); }
    }
    
    object compute_object::process(context* ctx) const {
      if (condition != nullptr) {
        const auto &ret = condition->process(ctx);
        if (ret.ignore() || !ret.get<bool>()) return ignore_value;
      }
      
      object cur_obj = ignore_value;
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        cur_obj = cur->process(ctx);
      }
      
      return cur_obj;
    }
    
    void compute_object::draw(context* ctx) const {
      // тут надо че нить рисовать?
      {
        const auto &ret = process(ctx);
        draw_data dd(ctx);
        dd.function_name = get_name();
        dd.value = ret;
        if (!ctx->draw(&dd)) return;
      }
      
      change_nesting cn(ctx, ctx->nest_level+1);
      
      if (condition != nullptr) {
        {
          const auto &ret = condition->process(ctx);
          draw_data dd(ctx);
          dd.function_name = "condition";
          dd.value = object(ret.get<bool>());
          ctx->draw(&dd);
        }
        change_nesting cn(ctx, ctx->nest_level+1);
        change_function_name cfn(ctx, "condition");
        condition->draw(ctx);
      }
      
      change_function_name cfn(ctx, get_name());
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        cur->draw(ctx);
      }
    }
    
    size_t compute_object::get_type_id() const { return type_id<object>(); }
    std::string_view compute_object::get_name() const { return "compute_object"; }
    
    //const size_t compute_number::expected_types;
    //const size_t compute_number::output_type;
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
        if (ret.ignore() || !ret.get<bool>()) return ignore_value;
      }

      return child->process(ctx);
    }
    
    void compute_number::draw(context* ctx) const {
      object prev_scope = ctx->prev;
      object new_scope = ctx->current;
      size_t nesting = ctx->nest_level;
      nesting += size_t(scope != nullptr || condition != nullptr);
      if (scope != nullptr) {
        scope->draw(ctx);
        prev_scope = new_scope;
        new_scope = scope->process(ctx);
      }
      
      {
        const auto &ret = process(ctx);
        draw_data dd(ctx);
        dd.function_name = get_name();
        dd.value = ret;
        if (!ctx->draw(&dd)) return;
      }
      
      change_scope sc(ctx, new_scope, prev_scope);
      change_nesting cn(ctx, nesting);
      
      if (condition != nullptr) {
        {
          const auto &ret = condition->process(ctx);
          draw_data dd(ctx);
          dd.function_name = "condition";
          dd.value = object(ret.get<bool>());
          ctx->draw_function(&dd);
        }
        change_nesting cn(ctx, ctx->nest_level+1);
        change_function_name cfn(ctx, "condition");
        condition->draw(ctx);
      }
      
      child->draw(ctx);
    }
    
    size_t compute_number::get_type_id() const { return type_id<object>(); }
    std::string_view compute_number::get_name() const { return "compute_number"; }
    
    const size_t selector::type_index = commands::values::selector;
    //const size_t selector::expected_types;
    //const size_t selector::output_type;
    selector::selector(const interface* childs) noexcept : childs(childs) {} 
    selector::~selector() noexcept { for (auto cur = childs; cur != nullptr; cur = cur->next) { cur->~interface(); } }
    object selector::process(context* ctx) const {
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        const auto obj = cur->process(ctx);
        if (!obj.ignore()) return obj;
      }
      return ignore_value;
    }
    
    void selector::draw(context* ctx) const {
      {
        const auto obj = process(ctx);
        draw_data dd(ctx);
        dd.function_name = get_name();
        dd.value = obj.ignore() ? object() : obj;
        if (!ctx->draw(&dd)) return;
      }
      
      change_nesting cn(ctx, ctx->nest_level+1);
      change_function_name cfn(ctx, get_name());
      
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        cur->draw(ctx);
      }
    }
    
    size_t selector::get_type_id() const { return type_id<object>(); }
    std::string_view selector::get_name() const { return commands::names[type_index]; }
    
    const size_t sequence::type_index = commands::values::sequence;
    //const size_t sequence::expected_types;
    //const size_t sequence::output_type;
    sequence::sequence(const interface* count, const interface* childs) noexcept : count(count), childs(childs) {} 
    sequence::~sequence() noexcept { for (auto cur = childs; cur != nullptr; cur = cur->next) { cur->~interface(); } }
    object sequence::process(context* ctx) const {
      size_t max_count = SIZE_MAX;
      if (count != nullptr) {
        const auto obj = count->process(ctx);
        max_count = obj.get<double>();
      }
      
      size_t counter = 0;
      object obj = ignore_value;
      for (auto cur = childs; cur != nullptr && counter < max_count; cur = cur->next, ++counter) {
        const auto cur_obj = cur->process(ctx);
        if (obj.ignore()) break;
        obj = cur_obj;
      }
      return obj;
    }
    
    void sequence::draw(context* ctx) const {
      {
        const auto obj = process(ctx);
        draw_data dd(ctx);
        dd.function_name = get_name();
        dd.value = obj.ignore() ? object() : obj;
        if (!ctx->draw(&dd)) return;
      }
      
      change_nesting cn(ctx, ctx->nest_level+1);
      change_function_name cfn(ctx, get_name());
      for (auto cur = childs; cur != nullptr; cur = cur->next) { cur->draw(ctx); }
    }
    
    size_t sequence::get_type_id() const { return type_id<object>(); }
    std::string_view sequence::get_name() const { return commands::names[type_index]; }
    
    overload::overload(const std::array<size_t, MAXIMUM_OVERLOADS> &overload_types, const interface* childs) noexcept : overload_types(overload_types), childs(childs) {}
    overload::~overload() noexcept { for (auto cur = childs; cur != nullptr; cur = cur->next) { cur->~interface(); } }
    struct object overload::process(context* ctx) const {
      // здесь к нам приходит какой то объект, по его типу нужно найти подходящего ребенка и вызвать его
      // как искать? нужно ввести еще парочку функций в интерфейс, например get_type, к сожалению придется делать их virtual
      // потенциально к ним может прийти type_id<object>()
      const interface* any_child = nullptr;
      size_t counter = 0;
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        assert(counter < MAXIMUM_OVERLOADS);
        const size_t expected_type = overload_types[counter];
        ++counter;
        
        if (expected_type == type_id<void>()) any_child = cur; // может быть только один? 
        if (expected_type == ctx->current.get_type()) return cur->process(ctx);
      }
      return any_child != nullptr ? any_child->process(ctx) : object();
    }
    
    void overload::draw(context* ctx) const {
      const interface* any_child = nullptr;
      size_t counter = 0;
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        assert(counter < MAXIMUM_OVERLOADS);
        const size_t expected_type = overload_types[counter];
        ++counter;
        
        //const size_t id = cur->get_type_id(); // get_type_id не потребовался, че я сразу не догадался до того чтобы хранить массив?
        //if (id == type_id<object>()) {}
        if (expected_type == type_id<void>()) any_child = cur; // может быть только один? 
        if (expected_type == ctx->current.get_type()) { cur->draw(ctx); return; }
      }
      
      if (any_child != nullptr) any_child->draw(ctx);
    }
    
    size_t overload::get_type_id() const { return type_id<object>(); }
    std::string_view overload::get_name() const { return "overload"; }
    
//     const size_t at_least_sequence::type_index = commands::values::at_least_sequence;
//     at_least_sequence::at_least_sequence(const interface* count, const interface* childs) noexcept : count(count), childs(childs) {}
//     at_least_sequence::~at_least_sequence() noexcept { count->~interface(); for (auto c = childs; c != nullptr; c = c->next) { c->~interface(); } }
//     object at_least_sequence::process(context* ctx) const {
//       const auto obj = count->process(ctx);
//       const size_t max_count = obj.get<double>();
//       size_t counter = 0;
//       object cur = ignore_value;
//       for (auto c = childs; c != nullptr; c = c->next) {
//         if (counter >= max_count) break;
//         const auto cur_obj = c->process(ctx);
//         if (cur_obj.ignore()) break;
//         cur = cur_obj;
//         ++counter;
//       }
//       return obj;
//     }
//     
//     void at_least_sequence::draw(context* ctx) const {
//       const auto obj = process(ctx);
//       draw_data dd(ctx);
//       dd.function_name = commands::names[type_index];
//       dd.value = obj.ignore() ? object() : obj;
//       ctx->draw(&dd);
//       
//       change_nesting cn(ctx, ctx->nest_level+1);
//       change_function_name cfn(ctx, dd.function_name);
//       
//       for (auto cur = childs; cur != nullptr; cur = cur->next) {
//         cur->draw(ctx);
//       }
//     }
    
    const size_t chance::type_index = commands::values::chance;
    //const size_t chance::output_type;
    chance::chance(const size_t &state, const interface* value) noexcept : state(state), value(value) {}
    chance::~chance() noexcept { value->~interface(); }
    object chance::process(context* ctx) const {
      const auto obj = value->process(ctx); // может ли к нам придти игнор? вероятность не нулевая, что делать?
      if (obj.ignore()) return ignore_value;
      const double val = obj.get<double>(); // имеет ли смысл выводить стак, имеет смысл хотя бы как то помочь при дебаге
      const uint64_t rnd_val = ctx->get_random_value(state);
      const double norm = context::normalize_value(rnd_val);
      return object(norm < val);
    }
    
    void chance::draw(context* ctx) const {
      {
        const auto origin = value->process(ctx);
        const auto obj = process(ctx);
        draw_data dd(ctx);
        dd.function_name = get_name();
        dd.value = obj;
        dd.original = origin;
        ctx->draw(&dd);
      }
      
      change_function_name cfn(ctx, get_name());
      change_nesting cn(ctx, ctx->nest_level+1);
      value->draw(ctx);
    }
    
    size_t chance::get_type_id() const { return type_id<object>(); }
    std::string_view chance::get_name() const { return commands::names[type_index]; }
    
    static const interface* get_random_child(context* ctx, const size_t &random_state, const interface* childs, const interface* weights) {
      size_t counter = 0;
      double accum_w = 0.0;
      const size_t array_size = 128;
      std::array<double, array_size> w_numbers;
      for (auto cur_w = weights, cur = childs; cur_w != nullptr; cur_w = cur_w->next, cur = cur->next) {
        if (counter >= array_size) throw std::runtime_error("'weighted_random' has childs more than array size (" + std::to_string(array_size) + ")");
        if (cur == nullptr) throw std::runtime_error("Childs count less than weights");
        const auto obj = cur_w->process(ctx);
        const double local_w = obj.get<double>();
        accum_w += local_w;
        w_numbers[counter] = local_w;
        counter += 1;
      }
      
      const uint64_t rand = ctx->get_random_value(random_state);
      const double rand_num = context::normalize_value(rand);
      const double final_num = accum_w * rand_num;
      
      const interface* choosed = childs;
      const interface* prev_choosed = childs;
      double cumulative = 0.0;
      size_t index = 0;
      for (; index < counter && cumulative <= final_num; cumulative += w_numbers[counter], ++index) { prev_choosed = choosed; choosed = choosed->next; }
      index -= 1; // нужно брать предыдущее значение, потому что в choosed может лежать nullptr
      
//       double cumulative = 0.0;
//       const interface* choosed = nullptr;
//       size_t child_counter = 0;
//       for (auto cur = childs; cur != nullptr && cumulative <= final_num; cur = cur->next) {
//         cumulative += w_numbers[child_counter];
//         ++child_counter;
//         assert(child_counter < counter);
//         choosed = cur; // по этим условиям цикл пройдет хотя бы один раз
//       }
      
      return prev_choosed;
    }
    
    const size_t weighted_random::type_index = commands::values::weighted_random;
    //const size_t weighted_random::expected_types;
    //const size_t weighted_random::output_type;
    weighted_random::weighted_random(const size_t &state, const interface* childs, const interface* weights) noexcept : state(state), childs(childs), weights(weights) {}
    weighted_random::~weighted_random() noexcept { for (auto cur = childs, cur_w = weights; cur != nullptr; cur = cur->next, cur_w = cur_w->next) { cur->~interface(); cur_w->~interface(); } }
    object weighted_random::process(context* ctx) const {
      auto choosed = get_random_child(ctx, state, childs, weights);
      return choosed->process(ctx);
    }
    
    void weighted_random::draw(context* ctx) const {
      auto choosed = get_random_child(ctx, state, childs, weights);
      {
        // запуск таких вот обходов для получения значения череват дополнительным запуском функций рандома, что может поломать описание, исправил
        const auto obj = choosed->process(ctx);
        draw_data dd(ctx);
        dd.function_name = get_name();
        dd.value = obj;
        if (!ctx->draw(&dd)) return;
      }
      
      change_function_name cfn(ctx, get_name());
      change_nesting cn(ctx, ctx->nest_level+1);
      choosed->draw(ctx);
    }
    
    size_t weighted_random::get_type_id() const { return type_id<object>(); }
    std::string_view weighted_random::get_name() const { return commands::names[type_index]; }
    
    const size_t random_value::type_index = commands::values::random_value;
    //const size_t random_value::output_type;
    random_value::random_value(const size_t &state, const interface* maximum) noexcept : state(state), maximum(maximum) {}
    random_value::~random_value() noexcept { if (maximum != nullptr) maximum->~interface(); }
    struct object random_value::process(context* ctx) const {
      double max = 1.0;
      if (maximum != nullptr) {
        const auto obj = maximum->process(ctx);
        max = obj.get<double>();
      }
      
      const uint64_t rand = ctx->get_random_value(state);
      const double rand_num = context::normalize_value(rand);
      const double final_num = max * rand_num;
      
      return object(final_num);
    }
    
    void random_value::draw(context* ctx) const {
      {
        draw_data dd(ctx);
        dd.function_name = get_name();
        dd.value = process(ctx);
        if (maximum != nullptr) dd.original = maximum->process(ctx);
        if (!ctx->draw(&dd)) return;
      }
      
      // нужно ли идти дальше? не думаю, вообще наверное надо, хотя бы для дебага
      if (maximum == nullptr) return;
      
      change_function_name cfn(ctx, get_name());
      change_nesting cn(ctx, ctx->nest_level+1);
      maximum->draw(ctx);
    }
    
    size_t random_value::get_type_id() const { return type_id<object>(); }
    std::string_view random_value::get_name() const { return commands::names[type_index]; }
    
    //const size_t boolean_container::expected_types;
    //const size_t boolean_container::output_type;
    boolean_container::boolean_container(const bool value) noexcept : value(value) {}
    object boolean_container::process(context*) const { return object(value); }
    void boolean_container::draw(context* ctx) const {
      draw_data dd(ctx);
      dd.function_name = get_name();
      dd.value = value;
      ctx->draw(&dd);
    }
    size_t boolean_container::get_type_id() const { return type_id<object>(); }
    std::string_view boolean_container::get_name() const { return "boolean_container"; }
    
    //const size_t number_container::expected_types;
    //const size_t number_container::output_type;
    number_container::number_container(const double &value) noexcept : value(value) {}
    object number_container::process(context*) const { return object(value); }
    void number_container::draw(context* ctx) const {
      draw_data dd(ctx);
      dd.function_name = get_name();
      dd.value = value;
      ctx->draw(&dd);
    }
    size_t number_container::get_type_id() const { return type_id<object>(); }
    std::string_view number_container::get_name() const { return "number_container"; }
  
    //const size_t string_container::expected_types;
    //const size_t string_container::output_type;
    string_container::string_container(const std::string &value) noexcept : value(value) {}
    string_container::string_container(const std::string_view &value) noexcept : value(value) {}
    string_container::~string_container() noexcept { std::cout << "~string_container()\n"; }
    object string_container::process(context*) const { return object(value); }
    void string_container::draw(context* ctx) const {
      draw_data dd(ctx);
      dd.function_name = get_name();
      dd.value = value;
      ctx->draw(&dd);
    }
    size_t string_container::get_type_id() const { return type_id<object>(); }
    std::string_view string_container::get_name() const { return "string_container"; }
    
    //const size_t object_container::expected_types;
    //const size_t object_container::output_type;
    object_container::object_container(const object &value) noexcept : value(value) {}
    object object_container::process(context*) const { return value; }
    void object_container::draw(context*) const { assert(false); }
    size_t object_container::get_type_id() const { return type_id<object>(); }
    std::string_view object_container::get_name() const { return type_name<decltype(*this)>(); }
    
    static bool compare_func(const uint8_t op, const double num1, const double num2) {
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
    
    //const size_t number_comparator::expected_types;
    //const size_t number_comparator::output_type;
    number_comparator::number_comparator(const uint8_t op, const interface* lvalue, const interface* rvalue) noexcept : op(op), lvalue(lvalue), rvalue(rvalue) {}
    number_comparator::~number_comparator() noexcept {
      lvalue->~interface();
      rvalue->~interface();
    }
    
    object number_comparator::process(context* ctx) const {
      const auto &num2 = rvalue->process(ctx);
      const auto &num1 = lvalue->process(ctx);
      const double num1_final = num1.get<double>();
      const double num2_final = num2.get<double>();
      return object(compare_func(op, num1_final, num2_final));
    }
    
    void number_comparator::draw(context* ctx) const {
      const auto &num2 = rvalue->process(ctx);
      change_rvalue crv(ctx, num2, op);
      lvalue->draw(ctx);
    }
    
    size_t number_comparator::get_type_id() const { return type_id<object>(); }
    std::string_view number_comparator::get_name() const { return "number_comparator"; }
    
    //const size_t boolean_comparator::expected_types;
    //const size_t boolean_comparator::output_type;
    boolean_comparator::boolean_comparator(const interface* lvalue, const interface* rvalue) noexcept : lvalue(lvalue), rvalue(rvalue) {}
    boolean_comparator::~boolean_comparator() noexcept {
      lvalue->~interface();
      rvalue->~interface();
    }
    object boolean_comparator::process(context* ctx) const {
      const auto &num2 = rvalue->process(ctx);
      const auto &num1 = lvalue->process(ctx);
      const bool num1_final = num1.get<bool>();
      const bool num2_final = num2.get<bool>();
      return object(num1_final == num2_final);
    }
    
    void boolean_comparator::draw(context* ctx) const {
      const auto &num2 = rvalue->process(ctx);
      change_rvalue cr(ctx, num2, 0);
      lvalue->draw(ctx);
    }
    
    size_t boolean_comparator::get_type_id() const { return type_id<object>(); }
    std::string_view boolean_comparator::get_name() const { return "boolean_comparator"; }
    
    const size_t equals_to::type_index = commands::values::equals_to;
    //const size_t equals_to::expected_types;
    //const size_t equals_to::output_type;
    equals_to::equals_to(const interface* get_obj) noexcept : get_obj(get_obj) {}
    equals_to::~equals_to() noexcept { get_obj->~interface(); }
    object equals_to::process(context* ctx) const {
      const auto obj = get_obj->process(ctx);
      return object(ctx->current == obj);
    }
    
    void equals_to::draw(context* ctx) const {
      const auto obj = get_obj->process(ctx);
      const auto val = object(ctx->current == obj);
      draw_data dd(ctx);
      dd.function_name = get_name();
      dd.value = val;
      dd.original = obj;
      ctx->draw(&dd);
    }
    
    size_t equals_to::get_type_id() const { return type_id<object>(); }
    std::string_view equals_to::get_name() const { return commands::names[type_index]; }
    
    const size_t not_equals_to::type_index = commands::values::not_equals_to;
    //const size_t not_equals_to::expected_types;
    //const size_t not_equals_to::output_type;
    not_equals_to::not_equals_to(const interface* get_obj) noexcept : get_obj(get_obj) {}
    not_equals_to::~not_equals_to() noexcept { get_obj->~interface(); }
    object not_equals_to::process(context* ctx) const {
      const auto obj = get_obj->process(ctx);
      return object(ctx->current != obj);
    }
    
    void not_equals_to::draw(context* ctx) const {
      const auto obj = get_obj->process(ctx);
      const auto val = object(ctx->current != obj);
      draw_data dd(ctx);
      dd.function_name = get_name();
      dd.value = val;
      dd.original = obj;
      ctx->draw(&dd);
    }
    
    size_t not_equals_to::get_type_id() const { return type_id<object>(); }
    std::string_view not_equals_to::get_name() const { return commands::names[type_index]; }
    
    const size_t equality::type_index = commands::values::equality;
    //const size_t equality::expected_types;
    //const size_t equality::output_type;
    equality::equality(const interface* childs) noexcept : childs(childs) {}
    equality::~equality() noexcept { for (auto cur = childs; cur != nullptr; cur = cur->next) { cur->~interface(); } }
    object equality::process(context* ctx) const {
      object obj = ignore_value;
      auto cur = childs;
      for (; cur != nullptr && obj.ignore(); cur = cur->next) {
        obj = cur->process(ctx);
      }
      
      for (; cur != nullptr; cur = cur->next) {
        const auto tmp = cur->process(ctx);
        if (!tmp.ignore() && tmp != obj) return object(false);
        //obj = tmp; // зачем, если мы проверяем равенство?
      }
      
      return object(true);
    }
    
    void equality::draw(context* ctx) const {
      {
        const auto obj = process(ctx);
        draw_data dd(ctx);
        dd.function_name = get_name();
        dd.value = obj;
        if (!ctx->draw(&dd)) return;
      }
      
      // нужно ли тут обходить детей? может и нужно, но как это дело рисовать не ясно
      change_function_name cfn(ctx, get_name());
      change_nesting cn(ctx, ctx->nest_level+1);
      for (auto cur = childs; cur != nullptr; cur = cur->next) { cur->draw(ctx); }
    }
    
    size_t equality::get_type_id() const { return type_id<object>(); }
    std::string_view equality::get_name() const { return commands::names[type_index]; }
    
    const size_t type_equality::type_index = commands::values::type_equality;
    //const size_t type_equality::expected_types;
    //const size_t type_equality::output_type;
    type_equality::type_equality(const interface* childs) noexcept : childs(childs) {}
    type_equality::~type_equality() noexcept { for (auto cur = childs; cur != nullptr; cur = cur->next) { cur->~interface(); } }
    object type_equality::process(context* ctx) const {
      object obj = ignore_value;
      auto cur = childs;
      for (; cur != nullptr && obj.ignore(); cur = cur->next) {
        obj = cur->process(ctx);
      }
      
      for (; cur != nullptr; cur = cur->next) {
        const auto tmp = cur->process(ctx);
        if (!tmp.ignore() && !tmp.lazy_type_compare(obj)) return object(false);
        //obj = tmp; // зачем, если мы проверяем равенство?
      }
      
      return object(true);
    }
    
    void type_equality::draw(context* ctx) const {
      {
        const auto obj = process(ctx);
        draw_data dd(ctx);
        dd.function_name = get_name();
        dd.value = obj;
        if (!ctx->draw(&dd)) return;
      }
      
      // нужно ли тут обходить детей? может и нужно, но как это дело рисовать не ясно
      change_function_name cfn(ctx, get_name());
      change_nesting cn(ctx, ctx->nest_level+1);
      for (auto cur = childs; cur != nullptr; cur = cur->next) { cur->draw(ctx); }
    }
    
    size_t type_equality::get_type_id() const { return type_id<object>(); }
    std::string_view type_equality::get_name() const { return commands::names[type_index]; }
    
    const size_t compare::type_index = commands::values::compare;
    //const size_t compare::expected_types;
    //const size_t compare::output_type;
    compare::compare(const uint8_t op, const interface* childs) noexcept : op(op), childs(childs) {}
    compare::~compare() noexcept { for (auto cur = childs; cur != nullptr; cur = cur->next) { cur->~interface(); } }
    object compare::process(context* ctx) const {
      // если здесь первое число будет неопределено, то что делать? может вылетать? но тогда будет работать иначе по сравнению с функциями sub, divide, mod и проч
      // с другой стороны функция принципиально другая
      // childs не должен быть nullptr
      const auto obj = childs->process(ctx);
      const double first_val = obj.get<double>();
      for (auto cur = childs->next; cur != nullptr; cur = cur->next) {
        const auto obj = cur->process(ctx);
        if (obj.ignore()) continue;
        const double val = obj.get<double>();
        const bool ret = compare_func(op, first_val, val);
        if (!ret) return object(false);
      }
      
      return object(true);
    }
    
    void compare::draw(context* ctx) const {
      {
        const auto obj = childs->process(ctx);
        draw_data dd(ctx);
        dd.function_name = get_name();
        dd.value = process(ctx);
        dd.original = obj;
        if (!ctx->draw(&dd)) return;
      }
      
      change_function_name cfn(ctx, get_name());
      change_nesting cn(ctx, ctx->nest_level+1);
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        cur->draw(ctx);
      }
    }
    
    size_t compare::get_type_id() const { return type_id<object>(); }
    std::string_view compare::get_name() const { return commands::names[type_index]; }
      
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
    
    // тут явно не нужно рисовать детей
    void complex_object::draw(context* ctx) const {
      {
        draw_data dd(ctx);
        dd.function_name = get_name();
        dd.value = process(ctx);
        if (!ctx->draw(&dd)) return;
      }
      
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        cur->draw(ctx);
      }
    }
    
    size_t complex_object::get_type_id() const { return type_id<object>(); }
    std::string_view complex_object::get_name() const { return "complex_value"; }
    
    const size_t root::type_index = commands::values::root;
    struct object root::process(context* ctx) const { return ctx->root; }
    size_t root::get_type_id() const { return type_id<object>(); }
    std::string_view root::get_name() const { return commands::names[type_index]; }
    const size_t prev::type_index = commands::values::prev;
    struct object prev::process(context* ctx) const { return ctx->prev; }
    size_t prev::get_type_id() const { return type_id<object>(); }
    std::string_view prev::get_name() const { return commands::names[type_index]; }
    const size_t current::type_index = commands::values::current;
    struct object current::process(context* ctx) const { return ctx->current; }
    size_t current::get_type_id() const { return type_id<object>(); }
    std::string_view current::get_name() const { return commands::names[type_index]; }
    
    // мы должны что то нарисовать
    void root::draw(context* ctx) const { 
      draw_data dd(ctx);
      dd.function_name = get_name();
      dd.value = process(ctx);
      ctx->draw(&dd);
    }
    
    void prev::draw(context* ctx) const {
      draw_data dd(ctx);
      dd.function_name = get_name();
      dd.value = process(ctx);
      ctx->draw(&dd);
    }
    
    void current::draw(context* ctx) const {
      draw_data dd(ctx);
      dd.function_name = get_name();
      dd.value = process(ctx);
      ctx->draw(&dd);
    }
    
    const size_t index::type_index = commands::index;
    struct object index::process(context* ctx) const { return object(double(ctx->index)); }
    size_t index::get_type_id() const { return type_id<object>(); }
    std::string_view index::get_name() const { return commands::names[type_index]; }
    
    const size_t prev_index::type_index = commands::prev_index;
    struct object prev_index::process(context* ctx) const { return object(double(ctx->prev_index)); }
    size_t prev_index::get_type_id() const { return type_id<object>(); }
    std::string_view prev_index::get_name() const { return commands::names[type_index]; }
    
    void index::draw(context* ctx) const {
      draw_data dd(ctx);
      dd.function_name = get_name();
      dd.value = process(ctx);
      ctx->draw(&dd);
    }
    
    void prev_index::draw(context* ctx) const {
      draw_data dd(ctx);
      dd.function_name = get_name();
      dd.value = process(ctx);
      ctx->draw(&dd);
    }
    
    const size_t value::type_index = commands::value;
    value::value(const interface* compute) noexcept : compute(compute) {}
    value::~value() noexcept { compute->~interface(); }
    struct object value::process(context* ctx) const { return compute->process(ctx); }
    size_t value::get_type_id() const { return type_id<object>(); }
    std::string_view value::get_name() const { return commands::names[type_index]; }
    void value::draw(context* ctx) const { compute->draw(ctx); }
    
    const size_t get_context::type_index = commands::context;
    get_context::get_context(const std::string &str) noexcept : name(str) {}
    get_context::get_context(const std::string_view &str) noexcept : name(str) {}
    get_context::~get_context() noexcept { std::cout << "~get_context()\n"; }
    object get_context::process(context* ctx) const {
      const auto itr = ctx->map.find(name);
      if (itr == ctx->map.end()) throw std::runtime_error("Could not find context object using key " + name);
      return itr->second;
    }
    
    void get_context::draw(context* ctx) const {
      const auto obj = process(ctx);
      // тут нам нужно как то словестно описать что мы получили на основе типа
      draw_data dd(ctx);
      dd.function_name = get_name();
      dd.value = obj;
      dd.original = name;
      ctx->draw(&dd);
    }
    
    size_t get_context::get_type_id() const { return type_id<object>(); }
    std::string_view get_context::get_name() const { return commands::names[type_index]; }
    
#define GET_ENTITY_HELPER_COMMON_PART(name)                              \
    /*const size_t get_##name::expected_types;*/                         \
    /*const size_t get_##name::output_type;*/                            \
    const size_t get_##name::type_index = commands::name;                \
    get_##name::get_##name(const interface* str) noexcept : str(str) {}  \
    get_##name::~get_##name() noexcept { str->~interface(); }            \
    void get_##name::draw(context* ctx) const {                          \
      const auto obj = process(ctx);                                     \
      draw_data dd(ctx);                                                 \
      dd.function_name = get_name();                                     \
      dd.value = obj;                                                    \
      dd.original = str->process(ctx);                                   \
      ctx->draw(&dd);                                                    \
    }                                                                    \
    size_t get_##name::get_type_id() const { return type_id<object>(); } \
    std::string_view get_##name::get_name() const { return commands::names[type_index]; } \
    
#define GET_ENTITY_HELPER_FUNC(name)                               \
    GET_ENTITY_HELPER_COMMON_PART(name)                            \
    object get_##name::process(context* ctx) const {               \
      const auto str_obj = str->process(ctx);                      \
      const auto view = str_obj.get<std::string_view>();           \
      auto core_ctx = global::get<systems::map_t>()->core_context.get(); \
      auto entity = core_ctx->get_entity<core::name>(view);        \
      if (entity == nullptr) throw std::runtime_error("Could not find "#name" " + std::string(view)); \
      return object(entity);                                       \
    }                                                              \
    
#define GET_CONST_ENTITY_HELPER_FUNC(name)                         \
    GET_ENTITY_HELPER_COMMON_PART(name)                            \
    object get_##name::process(context* ctx) const {               \
      const auto str_obj = str->process(ctx);                      \
      const auto view = str_obj.get<std::string_view>();           \
      auto core_ctx = global::get<systems::map_t>()->core_context.get(); \
      const auto entity = core_ctx->get_entity<core::name>(view);  \
      if (entity == nullptr) throw std::runtime_error("Could not find "#name" " + std::string(view)); \
      /*ASSERT(object(entity).is_const());*/                       \
      return object(entity);                                       \
    }                                                              \
    
    // хотя как мы их можем поменять? хороший вопрос, скорее всего никак
    // нам нужно получать из константы
    GET_ENTITY_HELPER_FUNC(culture)
    GET_ENTITY_HELPER_FUNC(religion)
    GET_ENTITY_HELPER_FUNC(titulus)
    GET_ENTITY_HELPER_FUNC(culture_group)
    GET_ENTITY_HELPER_FUNC(religion_group)
    
    GET_CONST_ENTITY_HELPER_FUNC(trait)
    GET_CONST_ENTITY_HELPER_FUNC(modificator)
    GET_CONST_ENTITY_HELPER_FUNC(casus_belli)
    GET_CONST_ENTITY_HELPER_FUNC(building_type)
    GET_CONST_ENTITY_HELPER_FUNC(holding_type)
    GET_CONST_ENTITY_HELPER_FUNC(city_type)
    GET_CONST_ENTITY_HELPER_FUNC(troop_type)
    GET_CONST_ENTITY_HELPER_FUNC(law)
    
#undef GET_ENTITY_HELPER_FUNC
#undef GET_CONST_ENTITY_HELPER_FUNC
#undef GET_ENTITY_HELPER_COMMON_PART
    
    // может быть имеет смысл вообще убрать temporary, оставить только локал? это имеет смысл, как бороться с перезаписью?
    // как быть с отрисовкой (там игрок может в рандомное время захотеть посмотреть вывод по разным методам) 
    // с отрисовкой придется еще проходить другие методы - это наверное лучше чем заводить еще контейнеры
//     const size_t save_temporary::type_index = commands::values::save_temporary;
//     save_temporary::save_temporary(const interface* str, const interface* var) noexcept : str(str), var(var) {}
//     save_temporary::~save_temporary() noexcept { str->~interface(); if (var != nullptr) var->~interface(); }
//     object save_temporary::process(context* ctx) const {
//       // как сделать? должен быть отдельный массив, насколько вообще большая необходимость в хешмапе здесь?
//       // может быть лучше просто завести вектор с хешами строк? поди вектор для локального контекста будет выстрее
//       // у меня редко когда будут больше 10-20 значений, правда я от коллизий не защищен поэтому все равно придется хранить строку =(
//       // 
//       
//       auto obj = ctx->current;
//       if (var != nullptr) {
//         obj = var->process(ctx);
//       }
//       
//       const auto name = str->process(ctx);
//       const auto name_str = name.get<std::string_view>();
//       if (const auto itr = ctx->local_map.find(name_str); itr != ctx->local_map.end()) throw std::runtime_error("Attempt to rewrite local variable " + std::string(name_str));
//       if (const auto itr = ctx->map.find(name_str); itr != ctx->map.end()) throw std::runtime_error("Attempt to rewrite local variable " + std::string(name_str));
//       ctx->local_map.emplace(name_str, obj);
//       return ignore_value;
//     }
//     
//     void save_temporary::draw(context* ctx) const {
//       draw_data dd(ctx);
//       
//       auto obj = ctx->current;
//       if (var != nullptr) {
//         obj = var->process(ctx);
//       }
//       
//       const auto name = str->process(ctx);
//       
//       dd.value = name;
//       dd.original = obj;
//       dd.function_name = commands::names[type_index];
//       ctx->draw(&dd);
//     }
    
    const size_t save_local::type_index = commands::values::save_local;
    save_local::save_local(const std::string &name, const size_t &index, const interface* var) noexcept : name(name), index(index), var(var) { assert(index < context::locals_container_size); }
    save_local::~save_local() noexcept { if (var != nullptr) var->~interface(); }
    object save_local::process(context* ctx) const {
      // тут понятно где запомнить
      auto obj = ctx->current;
      if (var != nullptr) {
        obj = var->process(ctx);
      }
      
      // ошибка? ну было бы неплохо перезапись чекать
      if (ctx->locals[index].valid() && !ctx->locals[index].ignore() && !obj.lazy_type_compare(ctx->locals[index])) 
        throw std::runtime_error("Rewriting local variable " + name + " with different type variable is considered as error"); 
      ctx->locals[index] = obj;
      
      return ignore_value;
    }
    
    void save_local::draw(context* ctx) const {
      auto obj = ctx->current;
      if (var != nullptr) {
        obj = var->process(ctx);
      }
      
      // ошибка? ну было бы неплохо перезапись чекать
      if (ctx->locals[index].valid() && !ctx->locals[index].ignore() && !obj.lazy_type_compare(ctx->locals[index])) 
        throw std::runtime_error("Rewriting local variable " + name + " with different type variable is considered as error"); 
      ctx->locals[index] = obj;
      
      draw_data dd(ctx);
      dd.value = obj;
      dd.original = name;
      dd.function_name = commands::names[type_index];
      ctx->draw(&dd);
    }
    
    size_t save_local::get_type_id() const { return type_id<object>(); }
    std::string_view save_local::get_name() const { return commands::names[type_index]; }
  
    const size_t save::type_index = commands::values::save;
    save::save(const interface* str, const interface* var) noexcept : str(str), var(var) {}
    save::~save() noexcept { str->~interface(); if (var != nullptr) var->~interface(); }
    object save::process(context* ctx) const {
      // сохраняем в дополнительном контейнере, который далее перейдет в контейнер эвента
      // тут понятно где запомнить
      auto obj = ctx->current;
      if (var != nullptr) {
        obj = var->process(ctx);
      }
      
      const auto name = str->process(ctx);
      const auto name_str = name.get<std::string_view>();
//       if (const auto itr = ctx->event_container.find(name_str); itr != ctx->event_container.end()) { 
//         if (!obj.lazy_compare_types(itr->second)) throw std::runtime_error("Rewriting local variable " + std::string(name_str) + " with different type variable is considered as error"); 
//         itr->second = obj;
//       } else ctx->event_container.emplace(name_str, obj);
      
      return ignore_value;
    }
    
    void save::draw(context* ctx) const {
      auto obj = ctx->current;
      if (var != nullptr) {
        obj = var->process(ctx);
      }
      
      const auto name = str->process(ctx);
      
      // тут по идее не нужно записывать никаких данных
      
      draw_data dd(ctx);
      dd.value = obj;
      dd.original = name; // наверное вообще не должно быть
      dd.function_name = commands::names[type_index];
      ctx->draw(&dd);
    }
    
    size_t save::get_type_id() const { return type_id<object>(); }
    std::string_view save::get_name() const { return commands::names[type_index]; }
    
    const size_t save_global::type_index = commands::values::save_global;
    save_global::save_global(const interface* str, const interface* var) noexcept : str(str), var(var) {}
    save_global::~save_global() noexcept { str->~interface(); if (var != nullptr) var->~interface(); }
    object save_global::process(context* ctx) const {
      // запоминать какое то значение для конкретного персонажа - выглядит полезным
      // но мне не хочется вводить еще один относительно неполезный контейнер
      // нужно посмотреть что вообще запонимается и как + имеет смысл придумать как можно взять из глобала уникальную штуку для персонажа
      UNUSED_VARIABLE(ctx);
      assert(false);
      return ignore_value;
    }
    
    void save_global::draw(context* ctx) const {
      UNUSED_VARIABLE(ctx);
      assert(false);
    }
    
    size_t save_global::get_type_id() const { return type_id<object>(); }
    std::string_view save_global::get_name() const { return commands::names[type_index]; }
    
    const size_t has_local::type_index = commands::values::has_local;
    has_local::has_local(const size_t &index) noexcept : index(index) {}
    has_local::~has_local() noexcept {}
    object has_local::process(context* ctx) const {
      ASSERT(index < context::locals_container_size);
      return object(ctx->locals[index].valid() && !ctx->locals[index].ignore());
    }
    
    void has_local::draw(context* ctx) const {
      const auto obj = process(ctx);
      draw_data dd(ctx);
      dd.value = obj;
      //dd.original = name; // наверное надо бы имя оставить
      dd.function_name = commands::names[type_index];
      ctx->draw(&dd);
    }
    
    size_t has_local::get_type_id() const { return type_id<object>(); }
    std::string_view has_local::get_name() const { return commands::names[type_index]; }
    
    const size_t has_global::type_index = commands::values::has_global;
    has_global::has_global(const interface* str) noexcept : str(str) {}
    has_global::~has_global() noexcept { str->~interface(); }
    object has_global::process(context* ctx) const {
      UNUSED_VARIABLE(ctx);
      assert(false);
      return object(false);
    }
    
    void has_global::draw(context* ctx) const {
      UNUSED_VARIABLE(ctx);
      assert(false);
    }
    
    size_t has_global::get_type_id() const { return type_id<object>(); }
    std::string_view has_global::get_name() const { return commands::names[type_index]; }
    
    const size_t remove_local::type_index = commands::values::remove_local;
    remove_local::remove_local(const std::string &name, const size_t &index) noexcept : name(name), index(index) {}
    remove_local::~remove_local() noexcept {}
    object remove_local::process(context* ctx) const {
      ASSERT(index < context::locals_container_size);
      ctx->locals[index] = object();
      return ignore_value;
    }
    
    void remove_local::draw(context* ctx) const {
      draw_data dd(ctx);
      dd.original = object(name);
      dd.function_name = commands::names[type_index];
      ctx->draw(&dd);
    }
    
    size_t remove_local::get_type_id() const { return type_id<object>(); }
    std::string_view remove_local::get_name() const { return commands::names[type_index]; }
    
    const size_t remove_global::type_index = commands::values::remove_global;
    remove_global::remove_global(const interface* str) noexcept : str(str) {}
    remove_global::~remove_global() noexcept { str->~interface(); }
    object remove_global::process(context* ctx) const {
      UNUSED_VARIABLE(ctx);
      assert(false);
      return ignore_value;
    }
    
    void remove_global::draw(context* ctx) const {
      UNUSED_VARIABLE(ctx);
      assert(false);
    }
    
    size_t remove_global::get_type_id() const { return type_id<object>(); }
    std::string_view remove_global::get_name() const { return commands::names[type_index]; }
    
    const size_t get_local::type_index = commands::local;
    get_local::get_local(const std::string &name, const size_t &index) noexcept : name(name), index(index) {}
    get_local::~get_local() noexcept {}
    struct object get_local::process(context* ctx) const {
      const auto obj = ctx->locals[index];
      if (!obj.valid() || obj.ignore()) throw std::runtime_error("Local variable " + name + " is invalid");
      return obj;
    }
    
    void get_local::draw(context* ctx) const {
      const auto obj = process(ctx);
      draw_data dd(ctx);
      dd.value = obj;
      dd.original = object(name);
      dd.function_name = get_name();
      ctx->draw(&dd);
    }
    
    size_t get_local::get_type_id() const { return type_id<object>(); }
    std::string_view get_local::get_name() const { return commands::names[type_index]; }
    
    const size_t add_to_list::type_index = commands::add_to_list;
    add_to_list::add_to_list(const std::string &name) noexcept : name(name) {}
    add_to_list::~add_to_list() noexcept {}
    struct object add_to_list::process(context* ctx) const {
      const auto cur = ctx->current;
      ctx->object_lists[name].push_back(cur);
      return ignore_value;
    }
    
    void add_to_list::draw(context* ctx) const {
      draw_data dd(ctx);
      dd.original = object(name);
      dd.function_name = get_name();
      ctx->draw(&dd);
    }
    
    size_t add_to_list::get_type_id() const { return type_id<object>(); }
    std::string_view add_to_list::get_name() const { return commands::names[type_index]; }
    
    const size_t is_in_list::type_index = commands::is_in_list;
    is_in_list::is_in_list(const std::string &name) noexcept : name(name) {}
    is_in_list::~is_in_list() noexcept {}
    struct object is_in_list::process(context* ctx) const {
      const auto cur = ctx->current;
      const auto itr = ctx->object_lists.find(name);
      if (itr == ctx->object_lists.end()) return object(false);
      
      for (const auto &obj : itr->second) {
        if (cur == obj) return object(true);
      }
      
      return object(false);
    }
    
    void is_in_list::draw(context* ctx) const {
      const auto obj = process(ctx);
      draw_data dd(ctx);
      dd.value = obj;
      dd.original = object(name);
      dd.function_name = get_name();
      ctx->draw(&dd);
    }
    
    size_t is_in_list::get_type_id() const { return type_id<object>(); }
    std::string_view is_in_list::get_name() const { return commands::names[type_index]; }
  
    const size_t has_in_list::type_index = commands::has_in_list;
    has_in_list::has_in_list(const std::string &name, const interface* max_count, const interface* percentage, const interface* childs) noexcept :
      name(name), max_count(max_count), percentage(percentage), childs(childs)
    {}
    
    has_in_list::~has_in_list() noexcept {
      if (max_count != nullptr) max_count->~interface();
      if (percentage != nullptr) percentage->~interface();
      for (auto cur = childs; cur != nullptr; cur = cur->next) { 
        cur->~interface(); 
      }
    }
    
    struct object has_in_list::process(context* ctx) const {
      const auto list_itr = ctx->object_lists.find(name);
      if (list_itr == ctx->object_lists.end()) return object(0.0);
      
      size_t final_max_count = SIZE_MAX;
      if (percentage != nullptr) {
        const auto val = percentage->process(ctx);
        const double final_percent = val.get<double>();
        if (final_percent < 0.0) throw std::runtime_error(std::string(name) + " percentage cannot be less than zero");
        const size_t counter = list_itr->second.size();
        final_max_count = counter * final_percent;
      } else if (max_count != nullptr) {
        const auto val = max_count->process(ctx);
        const double v = val.get<double>();
        if (v < 0.0) throw std::runtime_error(std::string(name) + " count cannot be less than zero");
        final_max_count = v;
      }
      
      if (final_max_count == 0) return object(0.0);
      
      size_t val = 0;
      change_scope cs(ctx, ctx->current, ctx->prev);
      change_indices ci(ctx, 0, ctx->index);
      ctx->prev = ctx->current;
      for (size_t i = 0; i < list_itr->second.size() && i < final_max_count; ++i) {
        const auto obj = list_itr->second[i];
        ctx->current = obj;
        ctx->index = i;
        
        for (auto cur = childs; cur != nullptr; cur = cur->next) {
          const auto &obj = cur->process(ctx);
          const bool ret = obj.ignore() ? true : obj.get<bool>();
          val += size_t(ret);
        }
      }
      
      return object(double(val));
    }
    
    void has_in_list::draw(context* ctx) const {
      const auto list_itr = ctx->object_lists.find(name);
      if (list_itr == ctx->object_lists.end() || list_itr->second.empty()) {
        draw_data dd(ctx);
        dd.function_name = get_name();
        dd.original = object(name);
        ctx->draw(&dd);
        return;
      }
      
      const auto value = process(ctx);
      
      {
        object count;
        object percent;
        if (max_count != nullptr) count = max_count->process(ctx);
        if (percentage != nullptr) percent = percentage->process(ctx);
      
        draw_data dd(ctx);
        dd.function_name = get_name();
        dd.value = value;
        dd.original = object(name);
        if (percentage != nullptr) dd.set_arg(0, "percentage", percent);
        else if (max_count != nullptr) dd.set_arg(0, "count", count);
        if (!ctx->draw(&dd)) return;
      }
      
      const object first = list_itr->second[0];
      if (!first.valid()) return;
      
      change_nesting cn(ctx, ++ctx->nest_level);
      change_scope cs(ctx, first, ctx->current);
      change_function_name cfn(ctx, get_name());
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        cur->draw(ctx);
      }
    }
    
    size_t has_in_list::get_type_id() const { return type_id<object>(); }
    std::string_view has_in_list::get_name() const { return commands::names[type_index]; }
  
    const size_t random_in_list::type_index = commands::random_in_list;
    random_in_list::random_in_list(const std::string &name, const size_t &state, const interface* condition, const interface* weight, const interface* childs) noexcept :
      name(name), state(state), condition(condition), weight(weight), childs(childs)
    {}
    
    random_in_list::~random_in_list() noexcept {
      if (condition != nullptr) condition->~interface();
      if (weight != nullptr) weight->~interface();
      for (auto cur = childs; cur != nullptr; cur = cur->next) { 
        cur->~interface(); 
      }
    }
    
    static struct object get_rand_obj(context* ctx, const utils::array_view<object> &view, const interface* condition, const interface* weight, const size_t state, const std::string_view &func_name) {
      change_indices ci(ctx, 0, ctx->index);
      
      size_t counter = 0;
      double accum_weight = 0.0;
      std::vector<std::pair<struct object, double>> objects;
      objects.reserve(view.size());
      for (size_t i = 0; i < view.size(); ++i) {
        ctx->current = view[i];
        ctx->index = counter;
        ++counter;
        
        if (condition != nullptr) {
          const auto obj = condition->process(ctx);
          if (obj.ignore() || !obj.get<bool>()) continue;
        }
        
        object weight_val(1.0);
        if (weight != nullptr) {
          weight_val = weight->process(ctx);
        }
        
        const double local = weight_val.get<double>();
        if (local < 0.0) throw std::runtime_error(std::string(func_name) + " weights must not be less than zero");
        objects.emplace_back(view[i], local);
        accum_weight += local;
      }
      
      if (objects.size() == 0) return object();
      
      const uint64_t rand_val = ctx->get_random_value(state);
      const double rand = script::context::normalize_value(rand_val) * accum_weight;
      double cumulative = 0.0;
      size_t index = 0;
      for (; index < objects.size() && cumulative <= rand; cumulative += objects[index].second, ++index) {}
      index -= 1;
      
      return objects[index].first;
    }
    
    struct object random_in_list::process(context* ctx) const {
      const auto list_itr = ctx->object_lists.find(name);
      if (list_itr == ctx->object_lists.end()) return object(0.0);
      
      change_scope cs(ctx, object(), ctx->current);
      const auto obj = get_rand_obj(ctx, list_itr->second, condition, weight, state, "random_in_list");
      ctx->current = obj;
      return obj.valid() ? childs->process(ctx) : ignore_value;
    }
    
    void random_in_list::draw(context* ctx) const {
      const auto list_itr = ctx->object_lists.find(name);
      if (list_itr == ctx->object_lists.end() || list_itr->second.empty()) {
        draw_data dd(ctx);
        dd.function_name = get_name();
        dd.original = object(name);
        ctx->draw(&dd);
        return;
      }
      
      auto obj = get_rand_obj(ctx, list_itr->second, condition, weight, state, get_name());
      //if (!obj.valid()) return; // нам бы все равно что то нарисовать желательно
      if (!obj.valid()) obj = list_itr->second[0];
      
      {
        draw_data dd(ctx);
        dd.function_name = get_name();
        dd.value = obj;
        dd.original = object(name);
        if (!ctx->draw(&dd)) return;
      }

      change_scope cs(ctx, obj, ctx->current);
      change_nesting cn(ctx, ctx->nest_level+1);
      change_function_name cfn(ctx, get_name());
//       for (auto cur = childs; cur != nullptr; cur = cur->next) {
//         cur->draw(ctx);
//       }
      childs->draw(ctx);
    }
    
    size_t random_in_list::get_type_id() const { return type_id<object>(); }
    std::string_view random_in_list::get_name() const { return commands::names[type_index]; }
    
    const size_t every_in_list_numeric::type_index = commands::every_in_list;
    every_in_list_numeric::every_in_list_numeric(const std::string &name, const interface* condition, const interface* childs) noexcept :
      name(name), condition(condition), childs(childs)
    {}
    
    every_in_list_numeric::~every_in_list_numeric() noexcept {
      if (condition != nullptr) condition->~interface();
      for (auto cur = childs; cur != nullptr; cur = cur->next) { 
        cur->~interface(); 
      }
    }
    
    struct object every_in_list_numeric::process(context* ctx) const {
      const auto list_itr = ctx->object_lists.find(name);
      if (list_itr == ctx->object_lists.end()) return object(0.0);
      change_scope cs(ctx, object(), ctx->current);
      change_indices ci(ctx, 0, ctx->index);
      
      double val = 0.0;
      for (size_t i = 0; i < list_itr->second.size(); ++i) {
        ctx->current = list_itr->second[i];
        ctx->index = i;
        
        if (condition != nullptr) {
          const auto obj = condition->process(ctx);
          if (obj.ignore() || !obj.get<bool>()) continue;
        }
        
        for (auto cur = childs; cur != nullptr; cur = cur->next) {
          const auto &obj = cur->process(ctx);
          val += obj.ignore() ? 0.0 : obj.get<double>();
        }
      }

      return object(val);
    }
    
    void every_in_list_numeric::draw(context* ctx) const {
      const auto list_itr = ctx->object_lists.find(name);
      if (list_itr == ctx->object_lists.end() || list_itr->second.empty()) {
        draw_data dd(ctx);
        dd.function_name = get_name();
        dd.original = object(name);
        ctx->draw(&dd);
        return;
      }
      
      const auto val = process(ctx);
      
      {
        draw_data dd(ctx);
        dd.function_name = get_name();
        dd.original = object(name);
        dd.value = val;
        if (!ctx->draw(&dd)) return;
      }
      
      const object first = list_itr->second[0];
      assert(first.valid());
      
      change_scope cs(ctx, first, ctx->current);
      change_function_name cfn(ctx, get_name());
      
      if (condition != nullptr) {
        draw_condition dc(ctx);
        change_nesting cn(ctx, ctx->nest_level+1);
        condition->draw(ctx);
      }
      
      change_nesting cn(ctx, ctx->nest_level+1);
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        cur->draw(ctx);
      }
    }
    
    size_t every_in_list_numeric::get_type_id() const { return type_id<object>(); }
    std::string_view every_in_list_numeric::get_name() const { return commands::names[type_index]; }
    
    const size_t every_in_list_logic::type_index = commands::every_in_list;
    every_in_list_logic::every_in_list_logic(const std::string &name, const interface* condition, const interface* childs) noexcept :
      name(name), condition(condition), childs(childs)
    {}
    
    every_in_list_logic::~every_in_list_logic() noexcept {
      if (condition != nullptr) condition->~interface();
      for (auto cur = childs; cur != nullptr; cur = cur->next) { 
        cur->~interface(); 
      }
    }
    
    struct object every_in_list_logic::process(context* ctx) const {
      const auto list_itr = ctx->object_lists.find(name);
      if (list_itr == ctx->object_lists.end()) return object(0.0);
      change_scope cs(ctx, object(), ctx->current);
      change_indices ci(ctx, 0, ctx->index);
      
      bool val = true;
      for (size_t i = 0; i < list_itr->second.size(); ++i) {
        ctx->current = list_itr->second[i];
        ctx->index = i;
        
        if (condition != nullptr) {
          const auto obj = condition->process(ctx);
          if (obj.ignore() || !obj.get<bool>()) continue;
        }
        
        for (auto cur = childs; cur != nullptr; cur = cur->next) {
          const auto &obj = cur->process(ctx);
          const bool ret = obj.ignore() ? true : obj.get<bool>();
          val = val && ret;
        }
      }

      return object(val);
    }
    
    void every_in_list_logic::draw(context* ctx) const {
      const auto list_itr = ctx->object_lists.find(name);
      if (list_itr == ctx->object_lists.end() || list_itr->second.empty()) {
        draw_data dd(ctx);
        dd.function_name = get_name();
        dd.original = object(name);
        ctx->draw(&dd);
        return;
      }
      
      const auto val = process(ctx);
      
      {
        draw_data dd(ctx);
        dd.function_name = get_name();
        dd.original = object(name);
        dd.value = val;
        if (!ctx->draw(&dd)) return;
      }
      
      const object first = list_itr->second[0];
      assert(first.valid());
      
      change_scope cs(ctx, first, ctx->current);
      change_function_name cfn(ctx, get_name());
      
      if (condition != nullptr) {
        draw_condition dc(ctx);
        change_nesting cn(ctx, ++ctx->nest_level);
        condition->draw(ctx);
      }
      
      change_nesting cn(ctx, ++ctx->nest_level);
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        cur->draw(ctx);
      }
    }
    
    size_t every_in_list_logic::get_type_id() const { return type_id<object>(); }
    std::string_view every_in_list_logic::get_name() const { return commands::names[type_index]; }
    
    const size_t every_in_list_effect::type_index = commands::every_in_list;
    every_in_list_effect::every_in_list_effect(const std::string &name, const interface* condition, const interface* childs) noexcept :
      name(name), condition(condition), childs(childs)
    {}
    
    every_in_list_effect::~every_in_list_effect() noexcept {
      if (condition != nullptr) condition->~interface();
      for (auto cur = childs; cur != nullptr; cur = cur->next) { cur->~interface(); }
    }
    
    struct object every_in_list_effect::process(context* ctx) const {
      const auto list_itr = ctx->object_lists.find(name);
      if (list_itr == ctx->object_lists.end()) return object(0.0);
      change_scope cs(ctx, object(), ctx->current);
      change_indices ci(ctx, 0, ctx->index);
      
      for (size_t i = 0; i < list_itr->second.size(); ++i) {
        ctx->current = list_itr->second[i];
        ctx->index = i;
        
        if (condition != nullptr) {
          const auto obj = condition->process(ctx);
          if (obj.ignore() || !obj.get<bool>()) continue;
        }
        
        for (auto cur = childs; cur != nullptr; cur = cur->next) { cur->process(ctx); }
      }

      return object();
    }
    
    void every_in_list_effect::draw(context* ctx) const {
      const auto list_itr = ctx->object_lists.find(name);
      if (list_itr == ctx->object_lists.end() || list_itr->second.empty()) {
        draw_data dd(ctx);
        dd.function_name = name;
        dd.original = object(name);
        ctx->draw(&dd);
        return;
      }
      
      {
        draw_data dd(ctx);
        dd.function_name = get_name();
        dd.original = object(name);
        if (!ctx->draw(&dd)) return;
      }
      
      const object first = list_itr->second[0];
      assert(first.valid());
      
      change_scope cs(ctx, first, ctx->current);
      change_function_name cfn(ctx, get_name());
      
      if (condition != nullptr) {
        draw_condition dc(ctx);
        change_nesting cn(ctx, ++ctx->nest_level);
        condition->draw(ctx);
      }
      
      change_nesting cn(ctx, ++ctx->nest_level);
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        cur->draw(ctx);
      }
    }
    
    size_t every_in_list_effect::get_type_id() const { return type_id<object>(); }
    std::string_view every_in_list_effect::get_name() const { return commands::names[type_index]; }
    
    const size_t list_view::type_index = commands::list_view;
    list_view::list_view(const std::string &name, const interface* default_value, const interface* childs) noexcept : name(name), default_value(default_value), childs(childs) {}
    list_view::~list_view() noexcept {
      default_value->~interface();
      for (auto cur = childs; cur != nullptr; cur = cur->next) { cur->~interface(); }
    }
    
    struct object list_view::process(context* ctx) const {
      const auto list_itr = ctx->object_lists.find(name);
      if (list_itr == ctx->object_lists.end()) return ignore_value;
      
      const auto def_val = default_value != nullptr ? default_value->process(ctx) : object();
      
      change_scope cs(ctx, object(), ctx->current);
      change_reduce_value crv(ctx, def_val);
      change_indices ci(ctx, 0, ctx->index);
      
      size_t counter = 0;
      object cur_ret = ignore_value;
      for (size_t i = 0; i < list_itr->second.size(); ++i) {
        ctx->current = list_itr->second[i];
        ctx->index = counter;
        for (auto child = childs; child != nullptr && !ctx->current.ignore(); child = child->next) {
          const auto ret = child->process(ctx);
          ctx->current = ret;
        }
        
        counter += size_t(!ctx->current.ignore());
        cur_ret = !ctx->current.ignore() ? ctx->current : cur_ret;
      }
      
      return cur_ret;
    }
    
    void list_view::draw(context* ctx) const {
      {
        const auto obj = process(ctx);
        draw_data dd(ctx);
        dd.function_name = get_name();
        dd.value = obj.ignore() ? object() : obj;
        dd.original = name;
        if (!ctx->draw(&dd)) return;
      }
      
      //change_scope cs(ctx, obj, ctx->current);
      change_nesting cn(ctx, ctx->nest_level+1);
      change_function_name cfn(ctx, get_name());
      for (auto child = childs; child != nullptr; child = child->next) { child->draw(ctx); }
    }
    
    size_t list_view::get_type_id() const { return type_id<object>(); }
    std::string_view list_view::get_name() const { return commands::names[type_index]; }
    
    const size_t transform::type_index = commands::transform;
    transform::transform(const interface* changes) noexcept : changes(changes) {}
    transform::~transform() noexcept { changes->~interface(); }
    struct object transform::process(context* ctx) const { return changes->process(ctx); }
    void transform::draw(context* ctx) const {
      {
        draw_data dd(ctx);
        dd.function_name = get_name();
        dd.value = process(ctx);
        if (!ctx->draw(&dd)) return;
      }
      changes->draw(ctx);
    }
    size_t transform::get_type_id() const { return type_id<object>(); }
    std::string_view transform::get_name() const { return commands::names[type_index]; }
    
    const size_t filter::type_index = commands::filter;
    filter::filter(const interface* condition) noexcept : condition(condition) {}
    filter::~filter() noexcept { condition->~interface(); }
    struct object filter::process(context* ctx) const {
      const auto obj = condition->process(ctx);
      return obj.ignore() || !obj.get<bool>() ? ignore_value : ctx->current;
    }
    void filter::draw(context* ctx) const {
      {
        draw_data dd(ctx);
        dd.function_name = get_name();
        dd.value = condition->process(ctx);
        if (!ctx->draw(&dd)) return;
      }
      condition->draw(ctx);
    }
    size_t filter::get_type_id() const { return type_id<object>(); }
    std::string_view filter::get_name() const { return commands::names[type_index]; }
    
    const size_t reduce::type_index = commands::reduce;
    reduce::reduce(const interface* value) noexcept : value(value) {}
    reduce::~reduce() noexcept { value->~interface(); }
    struct object reduce::process(context* ctx) const {
      const auto obj = value->process(ctx);
      ASSERT(!obj.ignore());
      ctx->reduce_value = obj;
      return obj;
    }
    void reduce::draw(context* ctx) const {
      {
        draw_data dd(ctx);
        dd.function_name = get_name();
        dd.value = process(ctx);
        if (!ctx->draw(&dd)) return;
      }
      value->draw(ctx);
    }
    size_t reduce::get_type_id() const { return type_id<object>(); }
    std::string_view reduce::get_name() const { return commands::names[type_index]; }
    
    const size_t take::type_index = commands::take;
    take::take(const size_t count) noexcept : count(count) {}
    take::~take() noexcept {}
    struct object take::process(context* ctx) const { return ctx->index < count ? ctx->current : ignore_value; }
    void take::draw(context* ctx) const {
      draw_data dd(ctx);
      dd.function_name = get_name();
      dd.value = object(double(count));
      ctx->draw(&dd);
    }
    size_t take::get_type_id() const { return type_id<object>(); }
    std::string_view take::get_name() const { return commands::names[type_index]; }
    
    const size_t drop::type_index = commands::drop;
    drop::drop(const size_t count) noexcept : count(count) {}
    drop::~drop() noexcept {}
    struct object drop::process(context* ctx) const { return ctx->index < count ? ignore_value : ctx->current; }
    void drop::draw(context* ctx) const {
      draw_data dd(ctx);
      dd.function_name = get_name();
      dd.value = object(double(count));
      ctx->draw(&dd);
    }
    size_t drop::get_type_id() const { return type_id<object>(); }
    std::string_view drop::get_name() const { return commands::names[type_index]; }
    
    static void draw_nesting(const size_t &nest_level) {
      for (size_t i = 0; i < nest_level; ++i) { std::cout << "  "; }
    }
    
    const size_t assert_condition::type_index = commands::values::assert_condition;
    assert_condition::assert_condition(const interface* condition, const interface* str) noexcept : condition(condition), str(str) {}
    assert_condition::~assert_condition() noexcept { condition->~interface(); if (str != nullptr) str->~interface(); }
    object assert_condition::process(context* ctx) const {
      const auto obj = condition->process(ctx);
      if (obj.is<bool>() && obj.get<bool>()) return ignore_value;
      
      // ошибка, что нужно сделать? вывести кондишен?
      const auto func = [] (const draw_data* data) -> bool {
        draw_nesting(data->nest_level);
        if (data->value.is<bool>() || data->value.is<double>()) {
          std::cout << "func " << data->function_name << 
                       " current type " << data->current.type << 
                       " value " << (data->value.is<bool>() ? data->value.get<bool>() : data->value.get<double>()) << '\n';
        } else {
          std::cout << "func " << data->function_name << 
                       " current type " << data->current.type << 
                       " value type " << data->value.type << '\n';
        }
        
        return true;
      };
      
      ctx->draw_function = func;
      condition->draw(ctx);
      if (str != nullptr) {
        const auto str_obj = str->process(ctx);
        const auto hint = str_obj.get<std::string_view>();
        throw std::runtime_error("Assertion failed in entity " + std::string(ctx->id) + " method " + std::string(ctx->method_name) + " hint: " + std::string(hint));
      } else {
        throw std::runtime_error("Assertion failed in entity " + std::string(ctx->id) + " method " + std::string(ctx->method_name));
      }
      return ignore_value;
    }
    
    void assert_condition::draw(context*) const {
      // надо ли тут рисовать? недумаю
    }
    
    size_t assert_condition::get_type_id() const { return type_id<object>(); }
    std::string_view assert_condition::get_name() const { return commands::names[type_index]; }
  }
}
