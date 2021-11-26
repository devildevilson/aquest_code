#include "change_context_commands.h"

#include "context.h"
#include "core/structures_header.h"
#include "core.h"

#include "utils/shared_mathematical_constant.h"

#include <functional>

namespace devils_engine {
  namespace script {
#define GET_SCOPE_COMMAND_FUNC(func_name, context_types_bits, output_type_bit) \
    const size_t func_name::type_index = commands::values::func_name; \
    void func_name::draw(context* ctx) const {        \
      const auto &obj = process(ctx);                 \
      draw_data dd(ctx);                              \
      dd.function_name = commands::names[type_index]; \
      dd.value = obj;                                 \
      ctx->draw_function(&dd);                        \
    }
    
    GET_SCOPE_COMMANDS_LIST
    
#undef GET_SCOPE_COMMAND_FUNC
    
#define CHANGE_CONTEXT_COMMAND_FUNC(func_name, context_types_bits, expected_types_bits, output_type_bit) \
    const size_t has##func_name::type_index = commands::values::has##func_name;          \
    has##func_name::has##func_name(const interface* childs) noexcept : max_count(nullptr), percentage(nullptr), childs(childs) {} \
    has##func_name::has##func_name(const interface* childs, const interface* max_count, const interface* percentage) noexcept : max_count(max_count), percentage(percentage), childs(childs) {} \
    has##func_name::~has##func_name() noexcept {                                         \
      max_count->~interface();                                                           \
      percentage->~interface();                                                          \
      for (auto cur = childs; cur != nullptr; cur = cur->next) { cur->~interface(); }    \
    }                                                                                    \
    \
    const size_t every##func_name::type_index = commands::values::every##func_name;          \
    every##func_name::every##func_name(const interface* condition, const interface* childs) noexcept : condition(condition), childs(childs) {} \
    every##func_name::~every##func_name() noexcept {                                         \
      condition->~interface();                                                               \
      for (auto cur = childs; cur != nullptr; cur = cur->next) { cur->~interface(); }        \
    }                                                                                        \
    \
    const size_t random##func_name::type_index = commands::values::random##func_name;          \
    random##func_name::random##func_name(const interface* condition, const interface* weight, const interface* childs) noexcept : condition(condition), weight(weight), childs(childs) {} \
    random##func_name::~random##func_name() noexcept {                                         \
      condition->~interface();                                                                 \
      weight->~interface();                                                                    \
      for (auto cur = childs; cur != nullptr; cur = cur->next) { cur->~interface(); }          \
    }                                                                                          \
    
    CHANGE_CONTEXT_COMMANDS_LIST
    
#undef CHANGE_CONTEXT_COMMAND_FUNC
    
    struct object self_realm::process(context* ctx) const {
      const auto c = ctx->current.get<core::character*>();
      return object(c->self);
    }
    
    static core::realm* next_vassal(core::realm* current, core::realm* ref) {
      return utils::ring::list_next<utils::list_type::vassals>(current, ref);
    }
    
    static struct object compute_random_vassal(context* ctx, const interface* condition, const interface* weight) {
      const core::realm* r = nullptr;
      if (ctx->current.is<core::character*>()) {
        const auto c = ctx->current.get<core::character*>();
        r = c->self.get();
        assert(r != nullptr);
      } else if (ctx->current.is<utils::handle<core::realm>>()) {
        const auto h = ctx->current.get<utils::handle<core::realm>>();
        r = h.get();
      }
      
      change_scope cs(ctx, ctx->current, ctx->prev);
      ctx->prev = ctx->current;
      
      double accum_weight = 0.0;
      std::vector<std::pair<core::realm*, double>> objects;
      objects.reserve(50);
      for (auto v = r->vassals; v != nullptr; v = next_vassal(v, r->vassals)) {
        assert(v->object_token != SIZE_MAX);
        const utils::handle<core::realm> h(v, v->object_token);
        ctx->current = object(h);
        if (condition != nullptr) {
          const auto obj = condition->process(ctx);
          if (obj.ignore() || !obj.get<bool>()) continue;
        }
        
        object weight_val(1.0);
        if (weight != nullptr) {
          weight_val = weight->process(ctx);
        }
        
        const double local = weight_val.get<double>();
        objects.emplace_back(v, local);
        accum_weight += local;
      }
      
      if (objects.size() == 0) return object();
      
      const double rand = script::random_state::normalize(ctx->random.next()) * accum_weight;
      core::realm* choosen = nullptr;
      double cumulative = 0.0;
      for (const auto &pair : objects) {
        cumulative += pair.second;
        if (cumulative >= rand) { choosen = pair.first; break; }
      }
      
      assert(choosen != nullptr);
      assert(choosen->object_token != SIZE_MAX);
      const utils::handle<core::realm> h(choosen, choosen->object_token);
      return object(h);
    }
    
    // не знаю почему я решил что такой рандомный вассал - это хорошая идея, для функций типа has_*, random_*, every_* нужно сгенерировать примерно одинаковые функции
    // то есть в каждой функции мне так или иначе придется обойти всех, и возможно запомнить кого то в массиве
    struct object random_vassal::process(context* ctx) const {
      const auto obj = compute_random_vassal(ctx, condition, weight);
      if (!obj.valid()) return object();
      
      change_scope cs(ctx, obj, ctx->current);
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        cur->process(ctx);
      }
      
      return object();
    }
    
    void random_vassal::draw(context* ctx) const {
      const core::realm* r = nullptr;
      if (ctx->current.is<core::character*>()) {
        const auto c = ctx->current.get<core::character*>();
        r = c->self.get();
      } else if (ctx->current.is<utils::handle<core::realm>>()) {
        const auto h = ctx->current.get<utils::handle<core::realm>>();
        r = h.get();
      }
      
      if (r == nullptr || r->vassals == nullptr) return;
      
      auto obj = compute_random_vassal(ctx, condition, weight);
      if (!obj.valid()) {
        const utils::handle<core::realm> h(r->vassals, r->vassals->object_token);
        obj = object(h);
      }
      
      draw_data dd(ctx);
      dd.function_name = commands::names[type_index];
      dd.value = obj;
      if (!ctx->draw(&dd)) return;
      
      change_scope cs(ctx, obj, ctx->current);
      change_nesting cn(ctx, ++ctx->nest_level);
      change_function_name cfn(ctx, commands::names[type_index]);
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        cur->draw(ctx);
      }
    }
    
    struct object every_vassal::process(context* ctx) const {
      const core::realm* r = nullptr;
      if (ctx->current.is<core::character*>()) {
        const auto c = ctx->current.get<core::character*>();
        r = c->self.get();
      } else if (ctx->current.is<utils::handle<core::realm>>()) {
        const auto h = ctx->current.get<utils::handle<core::realm>>();
        r = h.get();
      }
      
      if (r == nullptr) return object();
      
      change_scope cs(ctx, ctx->current, ctx->prev);
      
      ctx->prev = ctx->current;
      for (auto v = r->vassals; v != nullptr; v = next_vassal(v, r->vassals)) {
        assert(v->object_token != SIZE_MAX);
        const utils::handle<core::realm> h(v, v->object_token);
        ctx->current = object(h);
        
        if (condition != nullptr) {
          const auto obj = condition->process(ctx);
          if (obj.ignore() || !obj.get<bool>()) continue;
        }
        
        for (auto cur = childs; cur != nullptr; cur = cur->next) {
          cur->process(ctx);
        }
      }
      
      return object();
    }
    
    void every_vassal::draw(context* ctx) const {
      // сообщаем что для каждого вассала выполнить это
      
      // при этом берем первого вассала? вообще у меня может что то происходить в зависимости от самого вассала
      // возможно имеет смысл в некоторых случаях нарисовать формулу того что происходит
      // например я еще пока не знаю как можно оформить годовой доход словестно
      // хотя может быть и знаю, но нужно обходить формулу полностью для этого
      draw_data dd(ctx);
      dd.function_name = commands::names[type_index];
      if (!ctx->draw(&dd)) return;
      
      const core::realm* r = nullptr;
      if (ctx->current.is<core::character*>()) {
        const auto c = ctx->current.get<core::character*>();
        r = c->self.get();
      } else if (ctx->current.is<utils::handle<core::realm>>()) {
        const auto h = ctx->current.get<utils::handle<core::realm>>();
        r = h.get();
        assert(r != nullptr);
      }
      
      if (r == nullptr || r->vassals == nullptr) return;
      
      const utils::handle<core::realm> h(r->vassals, r->vassals->object_token);
      object vassal(h);
      
      change_scope cs(ctx, vassal, ctx->current);
      change_function_name cfn(ctx, commands::names[type_index]);
      
      if (condition != nullptr) {
        draw_condition dc(ctx);
        change_nesting cn(ctx, ++ctx->nest_level); // на том же нестинге что и эффекты? нужно в любом случае придумать какой то способ пометить что кондишен окончен
        // было бы неплохо еще предыдущую функцию сохранить
        condition->draw(ctx);
      }
      
      change_nesting cn(ctx, ++ctx->nest_level);
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        cur->draw(ctx);
      }
    }
    
    // например чтобы посчитать всех вассалов мы можем поставить единственное значение true в массив
    // кондишен в этой функции возникнуть не может? вряд ли, у нас вот что может тут возникнуть: максимальная метрика
    // как ее передать? числом как, таких функций будет несколько, так что можно завести еще дополнительный список
    // почему не может возникнуть кондишен? возможно пригодится, а тут неясно какой контекст у кондишена, 
    // поэтому лучше оставить без кондишена
    // имеет ли смысл сюда передать одного ребенка с конкретным типом булевой операции? вообще было бы неплохо
    // нужно посмотреть на инициализацию сначала
    struct object has_vassal::process(context* ctx) const {
      const core::realm* r = nullptr;
      if (ctx->current.is<core::character*>()) {
        const auto c = ctx->current.get<core::character*>();
        r = c->self.get();
      } else if (ctx->current.is<utils::handle<core::realm>>()) {
        const auto h = ctx->current.get<utils::handle<core::realm>>();
        r = h.get();
        assert(r != nullptr);
      }
      
      if (r == nullptr) return object(0.0);
      
      change_scope cs(ctx, ctx->current, ctx->prev);
      
      size_t final_max_count = SIZE_MAX;
      if (percentage != nullptr) {
        const auto val = percentage->process(ctx);
        const double final_percent = val.get<double>();
        if (final_percent < 0.0) throw std::runtime_error("has_vassal percentage cannot be less than zero");
        size_t counter = 0;
        for (auto v = r->vassals; v != nullptr; v = utils::ring::list_next<utils::list_type::vassals>(v, r->vassals)) { ++counter; }
        final_max_count = counter * final_percent;
      } else if (max_count != nullptr) {
        const auto val = max_count->process(ctx);
        if (val.get<double>() < 0.0) throw std::runtime_error("has_vassal count cannot be less than zero");
        final_max_count = val.get<double>();
      }
      
      ctx->prev = ctx->current;
      size_t counter = 0;
      for (auto v = r->vassals; v != nullptr && counter < final_max_count; v = utils::ring::list_next<utils::list_type::vassals>(v, r->vassals)) {
        assert(v->object_token != SIZE_MAX);
        const utils::handle<core::realm> h(v, v->object_token);
        ctx->current = object(h);
        
        bool all_ignore = true;
        bool cur_ret = true;
        for (auto cur = childs; cur != nullptr && cur_ret; cur = cur->next) {
          const auto ret = cur->process(ctx);
          all_ignore = all_ignore && ret.ignore();
          cur_ret = ret.ignore() ? cur_ret : cur_ret && ret.get<bool>();
        }
        
        counter += size_t(all_ignore ? 0 : cur_ret);
      }
      
      return object(double(counter));
    }
    
    void has_vassal::draw(context* ctx) const {
      const auto obj = process(ctx);
      // сообщаем что такое количество вассалов подходит под эти условия
      draw_data dd(ctx);
      dd.value = obj;
      dd.function_name = commands::names[type_index];
      if (!ctx->draw(&dd)) return;
      
      const core::realm* r = nullptr;
      if (ctx->current.is<core::character*>()) {
        const auto c = ctx->current.get<core::character*>();
        r = c->self.get();
      } else if (ctx->current.is<utils::handle<core::realm>>()) {
        const auto h = ctx->current.get<utils::handle<core::realm>>();
        r = h.get();
        assert(r != nullptr);
      }
      
      // тут жесткое условие, как же его можно избежать?
      if (r == nullptr || r->vassals == nullptr) return;
      
      // затем что? берем первого вассала? наверное, а его может и не быть
      const utils::handle<core::realm> h(r->vassals, r->vassals->object_token);
      object vassal(h);
      
      change_scope cs(ctx, vassal, ctx->current);
      change_nesting cn(ctx, ++ctx->nest_level);
      change_function_name cfn(ctx, commands::names[type_index]);
      
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        cur->draw(ctx);
      }
    }
    
    static struct object get_random_courtier(context* ctx, const interface* condition, const interface* weight) {
      const core::realm* r = nullptr;
      if (ctx->current.is<core::character*>()) {
        const auto c = ctx->current.get<core::character*>();
        r = c->self.get();
        assert(r != nullptr);
      } else if (ctx->current.is<utils::handle<core::realm>>()) {
        const auto h = ctx->current.get<utils::handle<core::realm>>();
        r = h.get();
      }
      
      if (r == nullptr || r->courtiers == nullptr) return object();
      
      change_scope cs(ctx, ctx->current, ctx->prev);
      ctx->prev = ctx->current;
      
      double accum_weight = 0.0;
      std::vector<std::pair<core::character*, double>> objects;
      objects.reserve(50);
      for (auto v = r->courtiers; v != nullptr; v = utils::ring::list_next<utils::list_type::courtiers>(v, r->courtiers)) {
        ctx->current = object(v);
        if (condition != nullptr) {
          const auto obj = condition->process(ctx);
          if (obj.ignore() || !obj.get<bool>()) continue;
        }
        
        object weight_val(1.0);
        if (weight != nullptr) {
          const auto tmp = weight->process(ctx);
          weight_val = tmp.ignore() ? weight_val : tmp;
        }
        
        const double local = weight_val.get<double>();
        objects.emplace_back(v, local);
        accum_weight += local;
      }
      
      if (objects.size() == 0) return object();
      
      const double rand = script::random_state::normalize(ctx->random.next()) * accum_weight;
      core::character* choosen = nullptr;
      double cumulative = 0.0;
      for (const auto &pair : objects) {
        cumulative += pair.second;
        if (cumulative >= rand) { choosen = pair.first; break; }
      }
      
      assert(choosen != nullptr);
      return object(choosen);
    }
    
    struct object random_courtier::process(context* ctx) const {
      const auto obj = get_random_courtier(ctx, condition, weight);
      if (!obj.valid()) return object();
      
      change_scope cs(ctx, obj, ctx->current);
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        cur->process(ctx);
      }
      
      return object();
    }
    
    void random_courtier::draw(context* ctx) const {
      const core::realm* r = nullptr;
      if (ctx->current.is<core::character*>()) {
        const auto c = ctx->current.get<core::character*>();
        r = c->self.get();
      } else if (ctx->current.is<utils::handle<core::realm>>()) {
        const auto h = ctx->current.get<utils::handle<core::realm>>();
        r = h.get();
      }
      
      if (r == nullptr || r->courtiers == nullptr) return;
      
      auto obj = get_random_courtier(ctx, condition, weight);
      if (!obj.valid()) {
        obj = object(r->courtiers);
      }
      
      draw_data dd(ctx);
      dd.function_name = commands::names[type_index];
      dd.value = obj;
      if (!ctx->draw(&dd)) return;
      
      change_scope cs(ctx, obj, ctx->current);
      change_nesting cn(ctx, ++ctx->nest_level);
      change_function_name cfn(ctx, commands::names[type_index]);
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        cur->draw(ctx);
      }
    }
    
    struct object every_courtier::process(context* ctx) const {
      const core::realm* r = nullptr;
      if (ctx->current.is<core::character*>()) {
        const auto c = ctx->current.get<core::character*>();
        r = c->self.get();
      } else if (ctx->current.is<utils::handle<core::realm>>()) {
        const auto h = ctx->current.get<utils::handle<core::realm>>();
        r = h.get();
      }
      
      if (r == nullptr) return object();
      
      change_scope cs(ctx, ctx->current, ctx->prev);
      ctx->prev = ctx->current;
      
      for (auto v = r->courtiers; v != nullptr; v = utils::ring::list_next<utils::list_type::courtiers>(v, r->courtiers)) {
        ctx->current = object(v);
        
        if (condition != nullptr) {
          const auto obj = condition->process(ctx);
          if (obj.ignore() || !obj.get<bool>()) continue;
        }
        
        for (auto cur = childs; cur != nullptr; cur = cur->next) {
          cur->process(ctx);
        }
      }
      
      return object();
    }
    
    void every_courtier::draw(context* ctx) const {
      draw_data dd(ctx);
      dd.function_name = commands::names[type_index];
      if (!ctx->draw(&dd)) return;
      
      const core::realm* r = nullptr;
      if (ctx->current.is<core::character*>()) {
        const auto c = ctx->current.get<core::character*>();
        r = c->self.get();
      } else if (ctx->current.is<utils::handle<core::realm>>()) {
        const auto h = ctx->current.get<utils::handle<core::realm>>();
        r = h.get();
      }
      
      if (r == nullptr || r->courtiers == nullptr) return;
      
      const object next_obj(r->courtiers);
      change_scope cs(ctx, next_obj, ctx->current);
      change_function_name cfn(ctx, commands::names[type_index]);
      
      if (condition != nullptr) {
        draw_condition dc(ctx);
        change_nesting cn(ctx, ++ctx->nest_level); // на том же нестинге что и эффекты? нужно в любом случае придумать какой то способ пометить что кондишен окончен
        // было бы неплохо еще предыдущую функцию сохранить
        condition->draw(ctx);
      }
      
      change_nesting cn(ctx, ++ctx->nest_level);
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        cur->draw(ctx);
      }
    }
    
    struct object has_courtier::process(context* ctx) const {
      const core::realm* r = nullptr;
      if (ctx->current.is<core::character*>()) {
        const auto c = ctx->current.get<core::character*>();
        r = c->self.get();
        assert(r != nullptr);
      } else if (ctx->current.is<utils::handle<core::realm>>()) {
        const auto h = ctx->current.get<utils::handle<core::realm>>();
        r = h.get();
      }
      
      if (r == nullptr || r->courtiers == nullptr) return object(0.0);
      
      change_scope cs(ctx, ctx->current, ctx->prev);
      
      size_t final_max_count = SIZE_MAX;
      if (percentage != nullptr) {
        const auto val = percentage->process(ctx);
        const double final_percent = val.get<double>();
        if (final_percent < 0.0) throw std::runtime_error("has_courtier percentage cannot be less than zero");
        size_t counter = 0;
        for (auto v = r->courtiers; v != nullptr; v = utils::ring::list_next<utils::list_type::courtiers>(v, r->courtiers)) { ++counter; }
        final_max_count = counter * final_percent;
      } else if (max_count != nullptr) {
        const auto val = max_count->process(ctx);
        if (val.get<double>() < 0.0) throw std::runtime_error("has_courtier count cannot be less than zero");
        final_max_count = val.get<double>();
      }
      
      ctx->prev = ctx->current;
      size_t counter = 0;
      for (auto courtier = r->courtiers; courtier != nullptr && counter < final_max_count; courtier = utils::ring::list_next<utils::list_type::courtiers>(courtier, r->courtiers)) {
        ctx->current = object(courtier);
        
        bool all_ignore = true;
        bool cur_ret = true;
        for (auto cur = childs; cur != nullptr && cur_ret; cur = cur->next) {
          const auto ret = cur->process(ctx);
          all_ignore = all_ignore && ret.ignore();
          cur_ret = ret.ignore() ? cur_ret : cur_ret && ret.get<bool>();
        }
        
        counter += size_t(all_ignore ? 0 : cur_ret);
      }
      
      return object(double(counter));
    }
    
    void has_courtier::draw(context* ctx) const {
      const auto obj = process(ctx);
      draw_data dd(ctx);
      dd.value = obj;
      dd.function_name = commands::names[type_index];
      if (!ctx->draw(&dd)) return;
      
      const core::realm* r = nullptr;
      if (ctx->current.is<core::character*>()) {
        const auto c = ctx->current.get<core::character*>();
        r = c->self.get();
        assert(r != nullptr);
      } else if (ctx->current.is<utils::handle<core::realm>>()) {
        const auto h = ctx->current.get<utils::handle<core::realm>>();
        r = h.get();
      }
      
      if (r == nullptr || r->courtiers == nullptr) return;
      
      // берем первого придворного
      object next_obj(r->courtiers);
      change_scope cs(ctx, next_obj, ctx->current);
      change_nesting cn(ctx, ++ctx->nest_level);
      change_function_name cfn(ctx, commands::names[type_index]);
      
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        cur->draw(ctx);
      }
    }
    
    template <typename T, typename F1>
    static struct object get_random_object(context* ctx, T current, const interface* condition, const interface* weight, const F1 &each_func) {
      change_scope cs(ctx, ctx->current, ctx->prev);
      ctx->prev = ctx->current;
      
      double accum_weight = 0.0;
      std::vector<std::pair<core::character*, double>> objects;
      objects.reserve(50);
      const auto func = [&] (context* ctx, core::character* current) -> size_t {
        ctx->current = object(current);
        if (condition != nullptr) {
          const auto obj = condition->process(ctx);
          if (obj.ignore() || !obj.get<bool>()) return 0;
        }
        
        object weight_val(1.0);
        if (weight != nullptr) {
          weight_val = weight->process(ctx);
        }
        
        const double local = weight_val.get<double>();
        objects.emplace_back(current, local);
        accum_weight += local;
        return 0;
      };
      
      each_func(ctx, SIZE_MAX, current, func);
      
      if (objects.size() == 0) return object();
      
      const double rand = script::random_state::normalize(ctx->random.next()) * accum_weight;
      core::character* choosen = nullptr;
      double cumulative = 0.0;
      for (const auto &pair : objects) {
        cumulative += pair.second;
        if (cumulative >= rand) { choosen = pair.first; break; }
      }
      
      assert(choosen != nullptr);
      return object(choosen);
    }
    
    template <typename T, typename F1, typename F2>
    static struct object has_entity_func(context* ctx, T current, const interface* percentage, const interface* max_count, const interface* childs, const F1 &count_func, const F2 &each_func) {
      change_scope cs(ctx, ctx->current, ctx->prev);
      
      size_t final_max_count = SIZE_MAX;
      if (percentage != nullptr) {
        const auto val = percentage->process(ctx);
        const double final_percent = val.get<double>();
        if (final_percent < 0.0) throw std::runtime_error("has_sibling percentage cannot be less than zero");
        const size_t counter = count_func(current);
        final_max_count = counter * final_percent;
      } else if (max_count != nullptr) {
        const auto val = max_count->process(ctx);
        if (val.get<double>() < 0.0) throw std::runtime_error("has_sibling count cannot be less than zero");
        final_max_count = val.get<double>();
      }
      
      if (final_max_count == 0) return object(0.0);
      
      const auto func = [childs] (context* ctx, core::character* current_char) -> size_t {
        ctx->current = object(current_char);
        bool all_ignore = true;
        bool cur_ret = true;
        for (auto cur = childs; cur != nullptr && cur_ret; cur = cur->next) {
          const auto ret = cur->process(ctx);
          all_ignore = all_ignore && ret.ignore();
          cur_ret = ret.ignore() ? cur_ret : cur_ret && ret.get<bool>();
        }
        return size_t(all_ignore ? 0 : cur_ret);
      };
      
      ctx->prev = ctx->current;
      const size_t counter = each_func(ctx, final_max_count, current, func);
      return object(double(counter));
    }
    
    template <typename T, typename F1>
    static void has_entity_draw_func(context* ctx, T current, const object &value, const interface* percentage, const interface* max_count, const interface* childs, const size_t &type_index, const F1 &first_obj) {
      object count;
      object percent;
      if (max_count != nullptr) count = max_count->process(ctx);
      if (percentage != nullptr) percent = percentage->process(ctx);
      
      draw_data dd(ctx);
      dd.function_name = commands::names[type_index];
      dd.value = value;
      if (percentage != nullptr) dd.set_arg(0, "percentage", percent);
      else if (max_count != nullptr) dd.set_arg(0, "count", count);
      if (!ctx->draw(&dd)) return;
      
      const auto s = first_obj(current);
      if (!s.valid()) return;
      
      change_nesting cn(ctx, ++ctx->nest_level);
      change_scope cs(ctx, s, ctx->current);
      change_function_name cfn(ctx, commands::names[type_index]);
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        cur->draw(ctx);
      }
    }
    
    template <typename T, typename F1>
    static struct object every_entity_func(context* ctx, T current, const interface* condition, const interface* childs, const F1 &each_func) {
      change_scope cs(ctx, ctx->current, ctx->prev);
      ctx->prev = ctx->current;
      
      const auto func = [&] (context* ctx, core::character* current) -> size_t {
        ctx->current = object(current);
        if (condition != nullptr) {
          const auto obj = condition->process(ctx);
          if (obj.ignore() || !obj.get<bool>()) return 0;
        }
        
        for (auto cur = childs; cur != nullptr; cur = cur->next) {
          cur->process(ctx);
        }
        
        return 0;
      };
      
      each_func(ctx, SIZE_MAX, current, func);
      return object();
    }
    
    template <typename T, typename F1>
    static void every_entity_draw_func(context* ctx, T current, const interface* condition, const interface* childs, const size_t &type_index, const F1 &first_obj) {
      draw_data dd(ctx);
      dd.function_name = commands::names[type_index];
      if (!ctx->draw(&dd)) return;
      
      const auto next_obj = first_obj(current);
      if (!next_obj.valid()) return;
      
      change_scope cs(ctx, next_obj, ctx->current);
      change_function_name cfn(ctx, commands::names[type_index]);
      
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
    
#define CHARACTER_HAS_ENTITY_FUNC(name, count_func, each_func)       \
    struct object has##name::process(context* ctx) const { \
      auto c = ctx->current.get<core::character*>();       \
      return has_entity_func(ctx, c, percentage, max_count, childs, count_func, each_func); \
    }
    
#define CHARACTER_HAS_ENTITY_DRAW_FUNC(name, first_obj)  \
    void has##name::draw(context* ctx) const { \
      auto c = ctx->current.get<core::character*>(); \
      const object value = process(ctx);       \
      has_entity_draw_func(ctx, c, value, percentage, max_count, childs, type_index, first_obj); \
    }
    
#define CHARACTER_RANDOM_ENTITY_FUNC(name, each_func)                      \
    struct object random##name::process(context* ctx) const {    \
      auto c = ctx->current.get<core::character*>();             \
      const auto obj = get_random_object(ctx, c, condition, weight, each_func); \
      if (!obj.valid()) return object();                         \
                                                                 \
      change_scope cs(ctx, obj, ctx->current);                   \
      for (auto cur = childs; cur != nullptr; cur = cur->next) { \
        cur->process(ctx);                                       \
      }                                                          \
                                                                 \
      return object();                                           \
    }
    
#define CHARACTER_RANDOM_ENTITY_DRAW_FUNC(name, each_func, first_obj)   \
    void random##name::draw(context* ctx) const {             \
      auto c = ctx->current.get<core::character*>();          \
      auto obj = get_random_object(ctx, c, condition, weight, each_func); \
      if (!obj.valid()) {                                     \
        obj = object(first_obj(c));                           \
      }                                                       \
                                                              \
      if (!obj.valid()) return;                               \
      draw_data dd(ctx);                                      \
      dd.function_name = commands::names[type_index];         \
      dd.value = obj;                                         \
      if (!ctx->draw(&dd)) return;                            \
                                                              \
      change_scope cs(ctx, obj, ctx->current);                \
      change_nesting cn(ctx, ++ctx->nest_level);              \
      change_function_name cfn(ctx, commands::names[type_index]); \
      for (auto cur = childs; cur != nullptr; cur = cur->next) { \
        cur->draw(ctx);                                       \
      }                                                       \
    }
    
#define CHARACTER_EVERY_ENTITY_FUNC(name, each_func)                   \
    struct object every##name::process(context* ctx) const { \
      auto c = ctx->current.get<core::character*>();         \
      return every_entity_func(ctx, c, condition, childs, each_func); \
    }
    
#define CHARACTER_EVERY_ENTITY_DRAW_FUNC(name, first_obj)              \
    void every##name::draw(context* ctx) const {             \
      auto c = ctx->current.get<core::character*>();         \
      return every_entity_draw_func(ctx, c, condition, childs, type_index, first_obj); \
    }
    
// пытаться обойти эти проблемы by design? а дальше по контексту пускать первого валидного ребенка
// это самый адекватный способ, еще есть вариант, если у нас даже первого невозможно взять то вообще не рисовать дальше
// для отрисовки есть еще один способ: скомпилировать скрипт, все данные расположить в каком нибудь контейнере
// а потом просто обходить контейнер, если нужно вычислить действие скрипта, да, но его нужно перевычислять
// все равно не решает проблем связанных с has_* функциями, тут видимо нужно делать иначе:
// добавлять всех в массив и потом че? тоже бредятина какая то

    static size_t count_siblings(core::character* current) {
      size_t counter = 0;
      for (auto v = current; v != nullptr; v = utils::ring::list_next<utils::list_type::father_line_siblings>(v, current)) { ++counter; }
      for (auto v = current; v != nullptr; v = utils::ring::list_next<utils::list_type::mother_line_siblings>(v, current)) { ++counter; }
      return counter;
    }
    
    static size_t each_sibling(context* ctx, const size_t &max_count, core::character* current, const std::function<size_t(context*, core::character*)> &f) {
      size_t counter = 0;
      for (auto v = current; v != nullptr && counter < max_count; v = utils::ring::list_next<utils::list_type::father_line_siblings>(v, current)) { 
        counter += f(ctx, v);
      }
      for (auto v = current; v != nullptr && counter < max_count; v = utils::ring::list_next<utils::list_type::mother_line_siblings>(v, current)) {
        counter += f(ctx, v);
      }
      return counter;
    }
    
    static object first_sibling(core::character* current) {
      core::character* s = nullptr;
      s = s != nullptr ? s : utils::ring::list_next<utils::list_type::father_line_siblings>(current, current);
      s = s != nullptr ? s : utils::ring::list_next<utils::list_type::mother_line_siblings>(current, current);
      return object(s);
    }
    
CHARACTER_HAS_ENTITY_FUNC(_sibling, count_siblings, each_sibling)
CHARACTER_HAS_ENTITY_DRAW_FUNC(_sibling, first_sibling)
CHARACTER_RANDOM_ENTITY_FUNC(_sibling, each_sibling)
CHARACTER_RANDOM_ENTITY_DRAW_FUNC(_sibling, each_sibling, first_sibling)
CHARACTER_EVERY_ENTITY_FUNC(_sibling, each_sibling)
CHARACTER_EVERY_ENTITY_DRAW_FUNC(_sibling, first_sibling)
    
    static size_t count_children(core::character* current) {
      size_t counter = 0;
      if (current->is_male()) for (auto v = current->family.children; v != nullptr; v = utils::ring::list_next<utils::list_type::father_line_siblings>(v, current)) { ++counter; }
      else                    for (auto v = current->family.children; v != nullptr; v = utils::ring::list_next<utils::list_type::mother_line_siblings>(v, current)) { ++counter; }
      return counter;
    }
    
    static size_t each_child(context* ctx, const size_t &max_count, core::character* current, const std::function<size_t(context*, core::character*)> &f) {
      size_t counter = 0;
      if (current->is_male()) {
        for (auto v = current->family.children; v != nullptr && counter < max_count; v = utils::ring::list_next<utils::list_type::father_line_siblings>(v, current)) { 
          counter += f(ctx, v);
        }
      } else {
        for (auto v = current->family.children; v != nullptr && counter < max_count; v = utils::ring::list_next<utils::list_type::mother_line_siblings>(v, current)) {
          counter += f(ctx, v);
        }
      }
      return counter;
    }
    
    static object first_child(core::character* current) {
      return object(current->family.children);
    }
    
CHARACTER_HAS_ENTITY_FUNC(_child, count_children, each_child)
CHARACTER_HAS_ENTITY_DRAW_FUNC(_child, first_child)
CHARACTER_RANDOM_ENTITY_FUNC(_child, each_child)
CHARACTER_RANDOM_ENTITY_DRAW_FUNC(_child, each_child, first_child)
CHARACTER_EVERY_ENTITY_FUNC(_child, each_child)
CHARACTER_EVERY_ENTITY_DRAW_FUNC(_child, first_child)

    static size_t count_brothers(core::character* current) {
      size_t counter = 0;
      for (auto v = current; v != nullptr; v = utils::ring::list_next<utils::list_type::father_line_siblings>(v, current)) { counter += size_t(v->is_male()); }
      for (auto v = current; v != nullptr; v = utils::ring::list_next<utils::list_type::mother_line_siblings>(v, current)) { counter += size_t(v->is_male()); }
      return counter;
    }
    
    static size_t each_brother(context* ctx, const size_t &max_count, core::character* current, const std::function<size_t(context*, core::character*)> &f) {
      size_t counter = 0;
      for (auto v = current; v != nullptr && counter < max_count; v = utils::ring::list_next<utils::list_type::father_line_siblings>(v, current)) { 
        if (!v->is_male()) continue;
        counter += f(ctx, v);
      }
      for (auto v = current; v != nullptr && counter < max_count; v = utils::ring::list_next<utils::list_type::mother_line_siblings>(v, current)) {
        if (!v->is_male()) continue;
        counter += f(ctx, v);
      }
      return counter;
    }
    
    static object first_brother(core::character* current) {
      for (auto v = current; v != nullptr; v = utils::ring::list_next<utils::list_type::father_line_siblings>(v, current)) { if (v->is_male()) return v; }
      for (auto v = current; v != nullptr; v = utils::ring::list_next<utils::list_type::mother_line_siblings>(v, current)) { if (v->is_male()) return v; }
      return object();
    }
    
CHARACTER_HAS_ENTITY_FUNC(_brother, count_brothers, each_brother)
CHARACTER_HAS_ENTITY_DRAW_FUNC(_brother, first_brother)
CHARACTER_RANDOM_ENTITY_FUNC(_brother, each_brother)
CHARACTER_RANDOM_ENTITY_DRAW_FUNC(_brother, each_brother, first_brother)
CHARACTER_EVERY_ENTITY_FUNC(_brother, each_brother)
CHARACTER_EVERY_ENTITY_DRAW_FUNC(_brother, first_brother)

    static size_t count_sisters(core::character* current) {
      size_t counter = 0;
      for (auto v = current; v != nullptr; v = utils::ring::list_next<utils::list_type::father_line_siblings>(v, current)) { counter += size_t(!v->is_male()); }
      for (auto v = current; v != nullptr; v = utils::ring::list_next<utils::list_type::mother_line_siblings>(v, current)) { counter += size_t(!v->is_male()); }
      return counter;
    }
    
    static size_t each_sister(context* ctx, const size_t &max_count, core::character* current, const std::function<size_t(context*, core::character*)> &f) {
      size_t counter = 0;
      for (auto v = current; v != nullptr && counter < max_count; v = utils::ring::list_next<utils::list_type::father_line_siblings>(v, current)) { 
        if (v->is_male()) continue;
        counter += f(ctx, v);
      }
      for (auto v = current; v != nullptr && counter < max_count; v = utils::ring::list_next<utils::list_type::mother_line_siblings>(v, current)) {
        if (v->is_male()) continue;
        counter += f(ctx, v);
      }
      return counter;
    }
    
    static object first_sister(core::character* current) {
      for (auto v = current; v != nullptr; v = utils::ring::list_next<utils::list_type::father_line_siblings>(v, current)) { if (!v->is_male()) return v; }
      for (auto v = current; v != nullptr; v = utils::ring::list_next<utils::list_type::mother_line_siblings>(v, current)) { if (!v->is_male()) return v; }
      return object();
    }
    
CHARACTER_HAS_ENTITY_FUNC(_sister, count_sisters, each_sister)
CHARACTER_HAS_ENTITY_DRAW_FUNC(_sister, first_sister)
CHARACTER_RANDOM_ENTITY_FUNC(_sister, each_sister)
CHARACTER_RANDOM_ENTITY_DRAW_FUNC(_sister, each_sister, first_sister)
CHARACTER_EVERY_ENTITY_FUNC(_sister, each_sister)
CHARACTER_EVERY_ENTITY_DRAW_FUNC(_sister, first_sister)

    static size_t count_concubines(core::character* current) {
      size_t counter = 0;
      for (auto v = current->family.concubines; v != nullptr; v = utils::ring::list_next<utils::list_type::concubines>(v, current)) { ++counter; }
      return counter;
    }
    
    static size_t each_concubine(context* ctx, const size_t &max_count, core::character* current, const std::function<size_t(context*, core::character*)> &f) {
      size_t counter = 0;
      for (auto v = current->family.concubines; v != nullptr && counter < max_count; v = utils::ring::list_next<utils::list_type::concubines>(v, current)) { 
        counter += f(ctx, v);
      }
      return counter;
    }
    
    static object first_concubine(core::character* current) {
      return object(current->family.concubines);
    }
    
CHARACTER_HAS_ENTITY_FUNC(_concubine, count_concubines, each_concubine)
CHARACTER_HAS_ENTITY_DRAW_FUNC(_concubine, first_concubine)
CHARACTER_RANDOM_ENTITY_FUNC(_concubine, each_concubine)
CHARACTER_RANDOM_ENTITY_DRAW_FUNC(_concubine, each_concubine, first_concubine)
CHARACTER_EVERY_ENTITY_FUNC(_concubine, each_concubine)
CHARACTER_EVERY_ENTITY_DRAW_FUNC(_concubine, first_concubine)

    static size_t count_acquaintances(core::character* current) {
      size_t counter = 0;
      for (const auto &pair : current->relations.acquaintances) { counter += size_t(pair.first != nullptr); }
      return counter;
    }
    
    static size_t each_acquaintance(context* ctx, const size_t &max_count, core::character* current, const std::function<size_t(context*, core::character*)> &f) {
      size_t counter = 0;
      for (const auto &pair : current->relations.acquaintances) { 
        if (counter >= max_count) break;
        if (pair.first == nullptr) continue;
        counter += f(ctx, pair.first);
      }
      return counter;
    }
    
    static object first_acquaintance(core::character* current) {
      for (const auto &pair : current->relations.acquaintances) { if (pair.first != nullptr) return object(pair.first); }
      return object();
    }
    
CHARACTER_HAS_ENTITY_FUNC(_acquaintance, count_acquaintances, each_acquaintance)
CHARACTER_HAS_ENTITY_DRAW_FUNC(_acquaintance, first_acquaintance)
CHARACTER_RANDOM_ENTITY_FUNC(_acquaintance, each_acquaintance)
CHARACTER_RANDOM_ENTITY_DRAW_FUNC(_acquaintance, each_acquaintance, first_acquaintance)
CHARACTER_EVERY_ENTITY_FUNC(_acquaintance, each_acquaintance)
CHARACTER_EVERY_ENTITY_DRAW_FUNC(_acquaintance, first_acquaintance)

    static size_t count_good_acquaintances(core::character* current) {
      size_t counter = 0;
      for (const auto &pair : current->relations.acquaintances) { counter += size_t(pair.first != nullptr && (pair.second.friendship > 0 || pair.second.love > 0)); }
      return counter;
    }
    
    static size_t each_good_acquaintance(context* ctx, const size_t &max_count, core::character* current, const std::function<size_t(context*, core::character*)> &f) {
      size_t counter = 0;
      for (const auto &pair : current->relations.acquaintances) { 
        if (counter >= max_count) break;
        if (pair.first == nullptr || (pair.second.friendship < 0 && pair.second.love < 0)) continue;
        counter += f(ctx, pair.first);
      }
      return counter;
    }
    
    static object first_good_acquaintance(core::character* current) {
      for (const auto &pair : current->relations.acquaintances) { if (pair.first != nullptr && (pair.second.friendship > 0 || pair.second.love > 0)) return object(pair.first); }
      return object();
    }
    
CHARACTER_HAS_ENTITY_FUNC(_good_acquaintance, count_good_acquaintances, each_good_acquaintance)
CHARACTER_HAS_ENTITY_DRAW_FUNC(_good_acquaintance, first_good_acquaintance)
CHARACTER_RANDOM_ENTITY_FUNC(_good_acquaintance, each_good_acquaintance)
CHARACTER_RANDOM_ENTITY_DRAW_FUNC(_good_acquaintance, each_good_acquaintance, first_good_acquaintance)
CHARACTER_EVERY_ENTITY_FUNC(_good_acquaintance, each_good_acquaintance)
CHARACTER_EVERY_ENTITY_DRAW_FUNC(_good_acquaintance, first_good_acquaintance)
    
    static size_t count_bad_acquaintances(core::character* current) {
      size_t counter = 0;
      for (const auto &pair : current->relations.acquaintances) { counter += size_t(pair.first != nullptr && (pair.second.friendship < 0 || pair.second.love < 0)); }
      return counter;
    }
    
    static size_t each_bad_acquaintance(context* ctx, const size_t &max_count, core::character* current, const std::function<size_t(context*, core::character*)> &f) {
      size_t counter = 0;
      for (const auto &pair : current->relations.acquaintances) { 
        if (counter >= max_count) break;
        if (pair.first == nullptr || (pair.second.friendship > 0 && pair.second.love > 0)) continue;
        counter += f(ctx, pair.first);
      }
      return counter;
    }
    
    static object first_bad_acquaintance(core::character* current) {
      for (const auto &pair : current->relations.acquaintances) { if (pair.first != nullptr && (pair.second.friendship < 0 || pair.second.love < 0)) return object(pair.first); }
      return object();
    }
    
CHARACTER_HAS_ENTITY_FUNC(_bad_acquaintance, count_bad_acquaintances, each_bad_acquaintance)
CHARACTER_HAS_ENTITY_DRAW_FUNC(_bad_acquaintance, first_bad_acquaintance)
CHARACTER_RANDOM_ENTITY_FUNC(_bad_acquaintance, each_bad_acquaintance)
CHARACTER_RANDOM_ENTITY_DRAW_FUNC(_bad_acquaintance, each_bad_acquaintance, first_bad_acquaintance)
CHARACTER_EVERY_ENTITY_FUNC(_bad_acquaintance, each_bad_acquaintance)
CHARACTER_EVERY_ENTITY_DRAW_FUNC(_bad_acquaintance, first_bad_acquaintance)
    
    static size_t count_friends(core::character* current) {
      size_t counter = 0;
      for (const auto &pair : current->relations.acquaintances) { counter += size_t(pair.first != nullptr && pair.second.friendship > 0); }
      return counter;
    }
    
    static size_t each_friend(context* ctx, const size_t &max_count, core::character* current, const std::function<size_t(context*, core::character*)> &f) {
      size_t counter = 0;
      for (const auto &pair : current->relations.acquaintances) { 
        if (counter >= max_count) break;
        if (pair.first == nullptr || pair.second.friendship <= 0) continue;
        counter += f(ctx, pair.first);
      }
      return counter;
    }
    
    static object first_friend(core::character* current) {
      for (const auto &pair : current->relations.acquaintances) { if (pair.first != nullptr && pair.second.friendship > 0) return object(pair.first); }
      return object();
    }
    
CHARACTER_HAS_ENTITY_FUNC(_pal, count_friends, each_friend)
CHARACTER_HAS_ENTITY_DRAW_FUNC(_pal, first_friend)
CHARACTER_RANDOM_ENTITY_FUNC(_pal, each_friend)
CHARACTER_RANDOM_ENTITY_DRAW_FUNC(_pal, each_friend, first_friend)
CHARACTER_EVERY_ENTITY_FUNC(_pal, each_friend)
CHARACTER_EVERY_ENTITY_DRAW_FUNC(_pal, first_friend)
    
    static size_t count_rivals(core::character* current) {
      size_t counter = 0;
      for (const auto &pair : current->relations.acquaintances) { counter += size_t(pair.first != nullptr && pair.second.friendship < 0); }
      return counter;
    }
    
    static size_t each_rival(context* ctx, const size_t &max_count, core::character* current, const std::function<size_t(context*, core::character*)> &f) {
      size_t counter = 0;
      for (const auto &pair : current->relations.acquaintances) { 
        if (counter >= max_count) break;
        if (pair.first == nullptr && pair.second.friendship >= 0) continue;
        counter += f(ctx, pair.first);
      }
      return counter;
    }
    
    static object first_rival(core::character* current) {
      for (const auto &pair : current->relations.acquaintances) { if (pair.first != nullptr && pair.second.friendship < 0) return object(pair.first); }
      return object();
    }
    
CHARACTER_HAS_ENTITY_FUNC(_foe, count_rivals, each_rival)
CHARACTER_HAS_ENTITY_DRAW_FUNC(_foe, first_rival)
CHARACTER_RANDOM_ENTITY_FUNC(_foe, each_rival)
CHARACTER_RANDOM_ENTITY_DRAW_FUNC(_foe, each_rival, first_rival)
CHARACTER_EVERY_ENTITY_FUNC(_foe, each_rival)
CHARACTER_EVERY_ENTITY_DRAW_FUNC(_foe, first_rival)
    
    static size_t count_lovers(core::character* current) {
      size_t counter = 0;
      for (const auto &pair : current->relations.acquaintances) { counter += size_t(pair.first != nullptr && pair.second.love > 0); }
      return counter;
    }
    
    static size_t each_lover(context* ctx, const size_t &max_count, core::character* current, const std::function<size_t(context*, core::character*)> &f) {
      size_t counter = 0;
      for (const auto &pair : current->relations.acquaintances) { 
        if (counter >= max_count) break;
        if (pair.first == nullptr && pair.second.love <= 0) continue;
        counter += f(ctx, pair.first);
      }
      return counter;
    }
    
    static object first_lover(core::character* current) {
      for (const auto &pair : current->relations.acquaintances) { if (pair.first != nullptr && pair.second.love > 0) return object(pair.first); }
      return object();
    }
    
CHARACTER_HAS_ENTITY_FUNC(_sympathy, count_lovers, each_lover)
CHARACTER_HAS_ENTITY_DRAW_FUNC(_sympathy, first_lover)
CHARACTER_RANDOM_ENTITY_FUNC(_sympathy, each_lover)
CHARACTER_RANDOM_ENTITY_DRAW_FUNC(_sympathy, each_lover, first_lover)
CHARACTER_EVERY_ENTITY_FUNC(_sympathy, each_lover)
CHARACTER_EVERY_ENTITY_DRAW_FUNC(_sympathy, first_lover)
    
    static size_t count_haters(core::character* current) {
      size_t counter = 0;
      for (const auto &pair : current->relations.acquaintances) { counter += size_t(pair.first != nullptr && pair.second.love < 0); }
      return counter;
    }
    
    static size_t each_hater(context* ctx, const size_t &max_count, core::character* current, const std::function<size_t(context*, core::character*)> &f) {
      size_t counter = 0;
      for (const auto &pair : current->relations.acquaintances) { 
        if (counter >= max_count) break;
        if (pair.first == nullptr && pair.second.love >= 0) continue;
        counter += f(ctx, pair.first);
      }
      return counter;
    }
    
    static object first_hater(core::character* current) {
      for (const auto &pair : current->relations.acquaintances) { if (pair.first != nullptr && pair.second.love < 0) return object(pair.first); }
      return object();
    }
    
CHARACTER_HAS_ENTITY_FUNC(_dislike, count_haters, each_hater)
CHARACTER_HAS_ENTITY_DRAW_FUNC(_dislike, first_hater)
CHARACTER_RANDOM_ENTITY_FUNC(_dislike, each_hater)
CHARACTER_RANDOM_ENTITY_DRAW_FUNC(_dislike, each_hater, first_hater)
CHARACTER_EVERY_ENTITY_FUNC(_dislike, each_hater)
CHARACTER_EVERY_ENTITY_DRAW_FUNC(_dislike, first_hater)
    
    static size_t count_members(utils::handle<core::realm> r) {
      size_t counter = 0;
      if (r->is_state_independent_power()) for (auto m = r->members; m != nullptr; m = utils::ring::list_next<utils::list_type::statemans>(m, r->members))   { ++counter; }
      if (r->is_council())  for (auto m = r->members; m != nullptr; m = utils::ring::list_next<utils::list_type::councilors>(m, r->members))  { ++counter; }
      if (r->is_assembly()) for (auto m = r->members; m != nullptr; m = utils::ring::list_next<utils::list_type::assemblers>(m, r->members))  { ++counter; }
      if (r->is_tribunal()) for (auto m = r->members; m != nullptr; m = utils::ring::list_next<utils::list_type::magistrates>(m, r->members)) { ++counter; }
      if (r->is_clergy())   for (auto m = r->members; m != nullptr; m = utils::ring::list_next<utils::list_type::clergymans>(m, r->members))  { ++counter; }
      
      return counter;
    }
    
    static size_t count_electors(utils::handle<core::realm> r) {
      size_t counter = 0;
      if (r->is_state_independent_power()) for (auto m = r->electors; m != nullptr; m = utils::ring::list_next<utils::list_type::state_electors>(m, r->electors))   { ++counter; }
      if (r->is_council())  for (auto m = r->electors; m != nullptr; m = utils::ring::list_next<utils::list_type::council_electors>(m, r->electors))  { ++counter; }
      if (r->is_assembly()) for (auto m = r->electors; m != nullptr; m = utils::ring::list_next<utils::list_type::assembly_electors>(m, r->electors))  { ++counter; }
      if (r->is_tribunal()) for (auto m = r->electors; m != nullptr; m = utils::ring::list_next<utils::list_type::tribunal_electors>(m, r->electors)) { ++counter; }
      if (r->is_clergy())   for (auto m = r->electors; m != nullptr; m = utils::ring::list_next<utils::list_type::clergy_electors>(m, r->electors))  { ++counter; }
      
      return counter;
    }
    
    static size_t each_member(context* ctx, const size_t &max_count, utils::handle<core::realm> r, const std::function<size_t(context*, core::character*)> &f) {
      size_t counter = 0;
      if (r->is_state_independent_power()) {
        for (auto m = r->members; m != nullptr && counter < max_count; m = utils::ring::list_next<utils::list_type::statemans>(m, r->members)) {
          counter += f(ctx, m);
        }
      } else if (r->is_council()) { 
        for (auto m = r->members; m != nullptr && counter < max_count; m = utils::ring::list_next<utils::list_type::councilors>(m, r->members)) {
          counter += f(ctx, m);
        }
      } else if (r->is_assembly()) {
        for (auto m = r->members; m != nullptr && counter < max_count; m = utils::ring::list_next<utils::list_type::assemblers>(m, r->members)) {
          counter += f(ctx, m);
        }
      } else if (r->is_tribunal()) {
        for (auto m = r->members; m != nullptr && counter < max_count; m = utils::ring::list_next<utils::list_type::magistrates>(m, r->members)) {
          counter += f(ctx, m);
        }
      } else if (r->is_clergy()) {
        for (auto m = r->members; m != nullptr && counter < max_count; m = utils::ring::list_next<utils::list_type::clergymans>(m, r->members)) {
          counter += f(ctx, m);
        }
      }
      
      return counter;
    }
    
    static size_t each_elector(context* ctx, const size_t &max_count, utils::handle<core::realm> r, const std::function<size_t(context*, core::character*)> &f) {
      size_t counter = 0;
      if (r->is_state_independent_power()) {
        for (auto m = r->electors; m != nullptr && counter < max_count; m = utils::ring::list_next<utils::list_type::state_electors>(m, r->electors)) {
          counter += f(ctx, m);
        }
      } else if (r->is_council()) { 
        for (auto m = r->electors; m != nullptr && counter < max_count; m = utils::ring::list_next<utils::list_type::council_electors>(m, r->electors)) {
          counter += f(ctx, m);
        }
      } else if (r->is_assembly()) {
        for (auto m = r->electors; m != nullptr && counter < max_count; m = utils::ring::list_next<utils::list_type::assembly_electors>(m, r->electors)) {
          counter += f(ctx, m);
        }
      } else if (r->is_tribunal()) {
        for (auto m = r->electors; m != nullptr && counter < max_count; m = utils::ring::list_next<utils::list_type::tribunal_electors>(m, r->electors)) {
          counter += f(ctx, m);
        }
      } else if (r->is_clergy()) {
        for (auto m = r->electors; m != nullptr && counter < max_count; m = utils::ring::list_next<utils::list_type::clergy_electors>(m, r->electors)) {
          counter += f(ctx, m);
        }
      }
      
      return counter;
    }
    
    // хотя бы один член всегда должен быть иначе это ошибка
    static struct object first_member(utils::handle<core::realm> r) {
      return object(r->members);
    }
    
    // а вот электоров может и не быть
    static struct object first_elector(utils::handle<core::realm> r) {
      return object(r->electors);
    }
    
#define REALM_HAS_ENTITY_FUNC(name, count_func, each_func)     \
    struct object has##name::process(context* ctx) const {     \
      auto c = ctx->current.get<utils::handle<core::realm>>(); \
      return has_entity_func(ctx, c, percentage, max_count, childs, count_func, each_func); \
    }
    
#define REALM_HAS_ENTITY_DRAW_FUNC(name, first_obj)            \
    void has##name::draw(context* ctx) const {                 \
      auto c = ctx->current.get<utils::handle<core::realm>>(); \
      const object value = process(ctx);                       \
      has_entity_draw_func(ctx, c, value, percentage, max_count, childs, type_index, first_obj); \
    }
    
#define REALM_RANDOM_ENTITY_FUNC(name, each_func)                \
    struct object random##name::process(context* ctx) const {    \
      auto c = ctx->current.get<utils::handle<core::realm>>();   \
      const auto obj = get_random_object(ctx, c, condition, weight, each_func); \
      if (!obj.valid()) return object();                         \
                                                                 \
      change_scope cs(ctx, obj, ctx->current);                   \
      for (auto cur = childs; cur != nullptr; cur = cur->next) { \
        cur->process(ctx);                                       \
      }                                                          \
                                                                 \
      return object();                                           \
    }
    
#define REALM_RANDOM_ENTITY_DRAW_FUNC(name, each_func, first_obj) \
    void random##name::draw(context* ctx) const {             \
      auto c = ctx->current.get<utils::handle<core::realm>>(); \
      auto obj = get_random_object(ctx, c, condition, weight, each_func); \
      if (!obj.valid()) {                                     \
        obj = object(first_obj(c));                           \
      }                                                       \
                                                              \
      if (!obj.valid()) return;                               \
      draw_data dd(ctx);                                      \
      dd.function_name = commands::names[type_index];         \
      dd.value = obj;                                         \
      if (!ctx->draw(&dd)) return;                            \
                                                              \
      change_scope cs(ctx, obj, ctx->current);                \
      change_nesting cn(ctx, ++ctx->nest_level);              \
      change_function_name cfn(ctx, commands::names[type_index]); \
      for (auto cur = childs; cur != nullptr; cur = cur->next) { \
        cur->draw(ctx);                                       \
      }                                                       \
    }
    
#define REALM_EVERY_ENTITY_FUNC(name, each_func)             \
    struct object every##name::process(context* ctx) const { \
      auto c = ctx->current.get<utils::handle<core::realm>>();         \
      return every_entity_func(ctx, c, condition, childs, each_func); \
    }
    
#define REALM_EVERY_ENTITY_DRAW_FUNC(name, first_obj)        \
    void every##name::draw(context* ctx) const {             \
      auto c = ctx->current.get<utils::handle<core::realm>>(); \
      return every_entity_draw_func(ctx, c, condition, childs, type_index, first_obj); \
    }
    
REALM_HAS_ENTITY_FUNC(_member, count_members, each_member)
REALM_HAS_ENTITY_DRAW_FUNC(_member, first_member)
REALM_RANDOM_ENTITY_FUNC(_member, each_member)
REALM_RANDOM_ENTITY_DRAW_FUNC(_member, each_member, first_member)
REALM_EVERY_ENTITY_FUNC(_member, each_member)
REALM_EVERY_ENTITY_DRAW_FUNC(_member, first_member)

REALM_HAS_ENTITY_FUNC(_elector, count_electors, each_elector)
REALM_HAS_ENTITY_DRAW_FUNC(_elector, first_elector)
REALM_RANDOM_ENTITY_FUNC(_elector, each_elector)
REALM_RANDOM_ENTITY_DRAW_FUNC(_elector, each_elector, first_elector)
REALM_EVERY_ENTITY_FUNC(_elector, each_elector)
REALM_EVERY_ENTITY_DRAW_FUNC(_elector, first_elector)
  }
}
