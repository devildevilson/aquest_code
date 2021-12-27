#include "change_context_commands.h"

#include "context.h"
#include "core/structures_header.h"
#include "core.h"
#include "utils/globals.h"
#include "utils/systems.h"
#include "core/context.h"

#include "utils/shared_mathematical_constant.h"

#include "core/realm_rights_checker.h"

#include <functional>

namespace devils_engine {
  namespace script {
#define CHANGE_CONTEXT_COMMAND_FUNC(func_name, context_types_bits, expected_types_bits, output_type_bit) \
    const size_t has_##func_name::type_index = commands::values::has_##func_name;        \
    const size_t has_##func_name::context_types;                                         \
    const size_t has_##func_name::expected_types;                                        \
    const size_t has_##func_name::output_type;                                           \
    has_##func_name::has_##func_name(const interface* childs) noexcept : max_count(nullptr), percentage(nullptr), childs(childs) {} \
    has_##func_name::has_##func_name(const interface* childs, const interface* max_count, const interface* percentage) noexcept : max_count(max_count), percentage(percentage), childs(childs) {} \
    has_##func_name::~has_##func_name() noexcept {                                       \
      max_count->~interface();                                                           \
      percentage->~interface();                                                          \
      for (auto cur = childs; cur != nullptr; cur = cur->next) { cur->~interface(); }    \
    }                                                                                    \
    \
    const size_t every_##func_name::type_index = commands::values::every_##func_name;        \
    const size_t every_##func_name::context_types;                                           \
    const size_t every_##func_name::expected_types;                                          \
    const size_t every_##func_name::output_type;                                             \
    every_##func_name::every_##func_name(const size_t &type, const interface* condition, const interface* childs) noexcept : type(type), condition(condition), childs(childs) {} \
    every_##func_name::~every_##func_name() noexcept {                                       \
      condition->~interface();                                                               \
      for (auto cur = childs; cur != nullptr; cur = cur->next) { cur->~interface(); }        \
    }                                                                                        \
    \
    const size_t random_##func_name::type_index = commands::values::random_##func_name;        \
    const size_t random_##func_name::context_types;                                            \
    const size_t random_##func_name::expected_types;                                           \
    const size_t random_##func_name::output_type;                                              \
    random_##func_name::random_##func_name(const size_t &state, const interface* condition, const interface* weight, const interface* childs) noexcept : \
      state(state), condition(condition), weight(weight), childs(childs) {}                    \
    random_##func_name::~random_##func_name() noexcept {                                       \
      condition->~interface();                                                                 \
      weight->~interface();                                                                    \
      for (auto cur = childs; cur != nullptr; cur = cur->next) { cur->~interface(); }          \
    }                                                                                          \
    
    CHANGE_CONTEXT_COMMANDS_FINAL_LIST
    
#undef CHANGE_CONTEXT_COMMAND_FUNC
    
// я мог во всех этих функциях посчитать нужные вещи через референс, а ниже описать только функции обхода объектов !!!!!!!!!!!!!!!
// АРГХААХЫХАХШААШ            переделал сам себя
    
    template <typename NEXT_T, typename CUR_T, typename F1>
    static struct object get_random_object(context* ctx, const size_t &random_state, CUR_T current, const interface* condition, const interface* weight, const F1 &each_func) {
      change_scope cs(ctx, ctx->current, ctx->prev);
      ctx->prev = ctx->current;
      
      double accum_weight = 0.0;
      std::vector<std::pair<NEXT_T, double>> objects;
      objects.reserve(50);
      const auto func = [&] (context* ctx, NEXT_T current) -> double {
        ctx->current = object(current);
        if (condition != nullptr) {
          const auto obj = condition->process(ctx);
          if (obj.ignore() || !obj.get<bool>()) return 0.0;
        }
        
        object weight_val(1.0);
        if (weight != nullptr) {
          weight_val = weight->process(ctx);
        }
        
        const double local = weight_val.get<double>();
        objects.emplace_back(current, local);
        accum_weight += local;
        return 0.0;
      };
      
      each_func(ctx, SIZE_MAX, current, func);
      
      if (objects.size() == 0) return object();
      
      const uint64_t rand_val = ctx->get_random_value(random_state);
      const double rand = script::context::normalize_value(rand_val) * accum_weight;
      NEXT_T choosen = nullptr;
      double cumulative = 0.0;
      for (const auto &pair : objects) {
        cumulative += pair.second;
        if (cumulative >= rand) { choosen = pair.first; break; }
      }
      
      assert(choosen != nullptr);
      return object(choosen);
    }
    
    template <typename NEXT_T, typename CUR_T, typename F1, typename F2>
    static struct object has_entity_func(context* ctx, CUR_T current, const interface* percentage, const interface* max_count, const interface* childs, const F1 &count_func, const F2 &each_func) {
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
      
      const auto func = [&childs] (context* ctx, NEXT_T current) -> double {
        ctx->current = object(current);
        bool all_ignore = true;
        bool cur_ret = true;
        for (auto cur = childs; cur != nullptr && cur_ret; cur = cur->next) {
          const auto ret = cur->process(ctx);
          all_ignore = all_ignore && ret.ignore();
          cur_ret = ret.ignore() ? cur_ret : cur_ret && ret.get<bool>();
        }
        return double(all_ignore ? 0.0 : cur_ret);
      };
      
      ctx->prev = ctx->current;
      const double counter = each_func(ctx, final_max_count, current, func);
      return object(counter);
    }
    
    template <typename CUR_T, typename F1>
    static void has_entity_draw_func(context* ctx, CUR_T current, const object &value, const interface* percentage, const interface* max_count, const interface* childs, const size_t &type_index, const F1 &first_obj) {
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
    
    template <typename NEXT_T, typename CUR_T, typename F1>
    static struct object every_entity_func(context* ctx, CUR_T current, const interface* condition, const interface* childs, const F1 &each_func) {
      change_scope cs(ctx, ctx->current, ctx->prev);
      ctx->prev = ctx->current;
      
      const auto func = [&condition, &childs] (context* ctx, NEXT_T current) -> double {
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
    
    template <typename NEXT_T, typename CUR_T, typename F1>
    static struct object every_entity_numeric_func(context* ctx, CUR_T current, const interface* condition, const interface* childs, const F1 &each_func) {
      change_scope cs(ctx, ctx->current, ctx->prev);
      ctx->prev = ctx->current;
      
      const auto func = [&condition, &childs] (context* ctx, NEXT_T current) -> double {
        ctx->current = object(current);
        if (condition != nullptr) {
          const auto obj = condition->process(ctx);
          if (obj.ignore() || !obj.get<bool>()) return 0.0;
        }
        
        double val = 0.0;
        for (auto cur = childs; cur != nullptr; cur = cur->next) {
          const auto &obj = cur->process(ctx);
          val += obj.ignore() ? 0.0 : obj.get<double>();
        }
        
        return val;
      };
      
      const double obj = each_func(ctx, SIZE_MAX, current, func);
      return object(obj);
    }
    
    template <typename NEXT_T, typename CUR_T, typename F1>
    static struct object every_entity_logic_func(context* ctx, CUR_T current, const interface* condition, const interface* childs, const F1 &each_func) {
      change_scope cs(ctx, ctx->current, ctx->prev);
      ctx->prev = ctx->current;
      
      const auto func = [&condition, &childs] (context* ctx, NEXT_T current) -> double {
        ctx->current = object(current);
        if (condition != nullptr) {
          const auto obj = condition->process(ctx);
          if (obj.ignore() || !obj.get<bool>()) return std::numeric_limits<double>::quiet_NaN();
        }
        
        bool val = true;
        for (auto cur = childs; cur != nullptr; cur = cur->next) {
          const auto &obj = cur->process(ctx);
          val = val && obj.ignore() ? true : obj.get<bool>(); // не должно быть здесь игнора
        }
        
        return double(val);
      };
      
      const bool obj = each_func(ctx, SIZE_MAX, current, func);
      return object(obj);
    }
    
    template <typename CUR_T, typename F1>
    static void every_entity_draw_func(context* ctx, CUR_T current, const interface* condition, const interface* childs, const size_t &type_index, const F1 &first_obj) {
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
    
#define HAS_ENTITY_FUNC(name, count_func, each_func, current_type, next_type) \
    struct object has_##name::process(context* ctx) const {                   \
      auto c = ctx->current.get<current_type>();                              \
      return has_entity_func<next_type>(ctx, c, percentage, max_count, childs, count_func, each_func); \
    }
    
#define HAS_ENTITY_DRAW_FUNC(name, first_obj, current_type)  \
    void has_##name::draw(context* ctx) const {              \
      auto c = ctx->current.get<current_type>();             \
      const object value = process(ctx);                     \
      has_entity_draw_func(ctx, c, value, percentage, max_count, childs, type_index, first_obj); \
    }
    
#define RANDOM_ENTITY_FUNC(name, each_func, current_type, next_type)                      \
    struct object random_##name::process(context* ctx) const {   \
      auto c = ctx->current.get<current_type>();                 \
      const auto obj = get_random_object<next_type>(ctx, state, c, condition, weight, each_func); \
      if (!obj.valid()) return object();                         \
                                                                 \
      change_scope cs(ctx, obj, ctx->current);                   \
      for (auto cur = childs; cur != nullptr; cur = cur->next) { \
        cur->process(ctx);                                       \
      }                                                          \
                                                                 \
      return object();                                           \
    }
    
#define RANDOM_ENTITY_DRAW_FUNC(name, each_func, first_obj, current_type, next_type)   \
    void random_##name::draw(context* ctx) const {             \
      auto c = ctx->current.get<current_type>();               \
      auto obj = get_random_object<next_type>(ctx, state, c, condition, weight, each_func); \
      if (!obj.valid()) {                                      \
        obj = object(first_obj(c));                            \
      }                                                        \
                                                               \
      if (!obj.valid()) return;                                \
      draw_data dd(ctx);                                       \
      dd.function_name = commands::names[type_index];          \
      dd.value = obj;                                          \
      if (!ctx->draw(&dd)) return;                             \
                                                               \
      change_scope cs(ctx, obj, ctx->current);                 \
      change_nesting cn(ctx, ++ctx->nest_level);               \
      change_function_name cfn(ctx, commands::names[type_index]); \
      for (auto cur = childs; cur != nullptr; cur = cur->next) {  \
        cur->draw(ctx);                                        \
      }                                                        \
    }
    
// тут я могу добавить вполне версии для эффекта, для кондишона, для нумерика
// нужно ли для кондишена? вообще может быть полезно
#define EVERY_ENTITY_FUNC(name, each_func, each_logic_func, current_type, next_type) \
    struct object every_##name::process(context* ctx) const { \
      auto c = ctx->current.get<current_type>();              \
      switch (type) {                                         \
        case EVERY_FUNC_EFFECT:  return every_entity_func<next_type>(ctx, c, condition, childs, each_func);             \
        case EVERY_FUNC_NUMERIC: return every_entity_numeric_func<next_type>(ctx, c, condition, childs, each_func);     \
        case EVERY_FUNC_LOGIC:   return every_entity_logic_func<next_type>(ctx, c, condition, childs, each_logic_func); \
        default: throw std::runtime_error("Bad type value");  \
      }                                                       \
      return object();                                        \
    }                                                         \
    
#define EVERY_ENTITY_DRAW_FUNC(name, first_obj, current_type) \
    void every_##name::draw(context* ctx) const {             \
      auto c = ctx->current.get<current_type>();              \
      return every_entity_draw_func(ctx, c, condition, childs, type_index, first_obj); \
    }
    
#define CHANGE_SCOPE_FUNC_IMPLEMENTATION(name, current_type, next_type)              \
  HAS_ENTITY_FUNC(name, count_##name##s, each_##name, current_type, next_type)       \
  HAS_ENTITY_DRAW_FUNC(name, first_##name, current_type)                             \
  RANDOM_ENTITY_FUNC(name, each_##name, current_type, next_type)                     \
  RANDOM_ENTITY_DRAW_FUNC(name, each_##name, first_##name, current_type, next_type)  \
  EVERY_ENTITY_FUNC(name, each_##name, each_##name##_logic, current_type, next_type) \
  EVERY_ENTITY_DRAW_FUNC(name, first_##name, current_type)                           \
    
// пытаться обойти эти проблемы by design? а дальше по контексту пускать первого валидного ребенка
// это самый адекватный способ, еще есть вариант, если у нас даже первого невозможно взять то вообще не рисовать дальше
// для отрисовки есть еще один способ: скомпилировать скрипт, все данные расположить в каком нибудь контейнере
// а потом просто обходить контейнер, если нужно вычислить действие скрипта, да, но его нужно перевычислять
// все равно не решает проблем связанных с has_* функциями, тут видимо нужно делать иначе:
// добавлять всех в массив и потом че? тоже бредятина какая то

#define DEFAULT_MAX_SIBLINGS_COUNT 20
#define DEFAULT_MAX_BROTHERS_COUNT 20
#define DEFAULT_MAX_SISTERS_COUNT 20

    static void fill_brothers_set(core::character* current, phmap::flat_hash_set<core::character*> &set) {
      {
        auto v = utils::ring::list_next<utils::list_type::father_line_siblings>(current, current);
        for (; v != nullptr; v = utils::ring::list_next<utils::list_type::father_line_siblings>(v, current)) { 
          if (v->is_male()) set.insert(v);
        }
      }
      {
        auto v = utils::ring::list_next<utils::list_type::mother_line_siblings>(current, current);
        for (; v != nullptr; v = utils::ring::list_next<utils::list_type::mother_line_siblings>(v, current)) {
          if (v->is_male()) set.insert(v);
        }
      }
    }
    
    static void fill_sisters_set(core::character* current, phmap::flat_hash_set<core::character*> &set) {
      {
        auto v = utils::ring::list_next<utils::list_type::father_line_siblings>(current, current);
        for (; v != nullptr; v = utils::ring::list_next<utils::list_type::father_line_siblings>(v, current)) { 
          if (v->is_female()) set.insert(v);
        }
      }
      {
        auto v = utils::ring::list_next<utils::list_type::mother_line_siblings>(current, current);
        for (; v != nullptr; v = utils::ring::list_next<utils::list_type::mother_line_siblings>(v, current)) {
          if (v->is_female()) set.insert(v);
        }
      }
    }
    
    static void fill_sibling_set(core::character* current, phmap::flat_hash_set<core::character*> &set) {
      fill_brothers_set(current, set);
      fill_sisters_set(current, set);
    }
    
    template <typename T>
    static double each_set_elem(context* ctx, const size_t &max_count, const std::function<double(context*, T)> &f, const phmap::flat_hash_set<T> &set) {
      double counter = 0.0;
      for (auto c : set) {
        if (counter >= max_count) break;
        counter += f(ctx, c);
      }

      return counter;
    }
    
    template <typename T>
    static size_t each_set_elem_logic(context* ctx, const std::function<double(context*, T)> &f, const phmap::flat_hash_set<T> &set) {
      bool counter = true;
      for (auto c : set) {
        const double ret = f(ctx, c);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }

      return counter;
    }

    // сиблингов нужно добавлять в хешсет =(
    static size_t count_siblings(core::character* current) {
      if (current == nullptr) return 0;
      
      phmap::flat_hash_set<core::character*> set;
      set.reserve(DEFAULT_MAX_SIBLINGS_COUNT);
      fill_sibling_set(current, set);
      return set.size();
      //return utils::ring::list_count<utils::list_type::father_line_siblings>(current)-1 + utils::ring::list_count<utils::list_type::mother_line_siblings>(current)-1;
    }
    
    static double each_sibling(context* ctx, const size_t &max_count, core::character* current, const std::function<double(context*, core::character*)> &f) {
      if (current == nullptr) return 0;
      
      phmap::flat_hash_set<core::character*> set;
      set.reserve(DEFAULT_MAX_SIBLINGS_COUNT);
      fill_sibling_set(current, set);
      return each_set_elem(ctx, max_count, f, set);
    }
    
    static size_t each_sibling_logic(context* ctx, const size_t &, core::character* current, const std::function<double(context*, core::character*)> &f) {
      if (current == nullptr) return 0;
      
      phmap::flat_hash_set<core::character*> set;
      set.reserve(DEFAULT_MAX_SIBLINGS_COUNT);
      fill_sibling_set(current, set);
      return each_set_elem_logic(ctx, f, set);
    }
    
    static object first_sibling(core::character* current) {
      core::character* s = nullptr;
      if (current == nullptr) return object(s);
      
      s = s != nullptr ? s : utils::ring::list_next<utils::list_type::father_line_siblings>(current, current);
      s = s != nullptr ? s : utils::ring::list_next<utils::list_type::mother_line_siblings>(current, current);
      return object(s);
    }
    
    static size_t count_childs(core::character* current) {
      size_t counter = 0;
      if (current->is_male()) for (auto v = current->family.children; v != nullptr; v = utils::ring::list_next<utils::list_type::father_line_siblings>(v, current)) { ++counter; }
      else                    for (auto v = current->family.children; v != nullptr; v = utils::ring::list_next<utils::list_type::mother_line_siblings>(v, current)) { ++counter; }
      return counter;
    }
    
    static double each_child(context* ctx, const size_t &max_count, core::character* current, const std::function<double(context*, core::character*)> &f) {
      double counter = 0;
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
    
    static size_t each_child_logic(context* ctx, const size_t &, core::character* current, const std::function<double(context*, core::character*)> &f) {
      bool counter = true;
      if (current->is_male()) {
        for (auto v = current->family.children; v != nullptr; v = utils::ring::list_next<utils::list_type::father_line_siblings>(v, current)) { 
          const double ret = f(ctx, v);
          counter = counter && (std::isnan(ret) ? true : bool(ret));
        }
      } else {
        for (auto v = current->family.children; v != nullptr; v = utils::ring::list_next<utils::list_type::mother_line_siblings>(v, current)) {
          const double ret = f(ctx, v);
          counter = counter && (std::isnan(ret) ? true : bool(ret));
        }
      }
      return counter;
    }
    
    static object first_child(core::character* current) {
      return object(current->family.children);
    }
    
    // а вот братьев тоже нужно в сет добавлять
    static size_t count_brothers(core::character* current) {
      if (current == nullptr) return 0;
      phmap::flat_hash_set<core::character*> set;
      set.reserve(DEFAULT_MAX_BROTHERS_COUNT);
      fill_brothers_set(current, set);
      return set.size();
    }
    
    static double each_brother(context* ctx, const size_t &max_count, core::character* current, const std::function<double(context*, core::character*)> &f) {
      if (current == nullptr) return 0;
      phmap::flat_hash_set<core::character*> set;
      set.reserve(DEFAULT_MAX_BROTHERS_COUNT);
      fill_brothers_set(current, set);
      return each_set_elem(ctx, max_count, f, set);
    }
    
    static size_t each_brother_logic(context* ctx, const size_t &, core::character* current, const std::function<double(context*, core::character*)> &f) {
      if (current == nullptr) return 0;
      phmap::flat_hash_set<core::character*> set;
      set.reserve(DEFAULT_MAX_BROTHERS_COUNT);
      fill_brothers_set(current, set);
      return each_set_elem_logic(ctx, f, set);
    }
    
    static object first_brother(core::character* current) {
      if (current == nullptr) return object();
      for (auto v = current; v != nullptr; v = utils::ring::list_next<utils::list_type::father_line_siblings>(v, current)) { if (v->is_male()) return v; }
      for (auto v = current; v != nullptr; v = utils::ring::list_next<utils::list_type::mother_line_siblings>(v, current)) { if (v->is_male()) return v; }
      return object();
    }

    static size_t count_sisters(core::character* current) {
      if (current == nullptr) return 0;
      phmap::flat_hash_set<core::character*> set;
      set.reserve(DEFAULT_MAX_SISTERS_COUNT);
      fill_sisters_set(current, set);
      return set.size();
    }
    
    static double each_sister(context* ctx, const size_t &max_count, core::character* current, const std::function<double(context*, core::character*)> &f) {
      if (current == nullptr) return 0;
      phmap::flat_hash_set<core::character*> set;
      set.reserve(DEFAULT_MAX_SISTERS_COUNT);
      fill_sisters_set(current, set);
      return each_set_elem(ctx, max_count, f, set);
    }
    
    static size_t each_sister_logic(context* ctx, const size_t &, core::character* current, const std::function<double(context*, core::character*)> &f) {
      if (current == nullptr) return 0;
      phmap::flat_hash_set<core::character*> set;
      set.reserve(DEFAULT_MAX_SISTERS_COUNT);
      fill_sisters_set(current, set);
      return each_set_elem_logic(ctx, f, set);
    }
    
    static object first_sister(core::character* current) {
      if (current == nullptr) return object();
      for (auto v = current; v != nullptr; v = utils::ring::list_next<utils::list_type::father_line_siblings>(v, current)) { if (!v->is_male()) return v; }
      for (auto v = current; v != nullptr; v = utils::ring::list_next<utils::list_type::mother_line_siblings>(v, current)) { if (!v->is_male()) return v; }
      return object();
    }

    static size_t count_concubines(core::character* current) {
      size_t counter = 0;
      for (auto v = current->family.concubines; v != nullptr; v = utils::ring::list_next<utils::list_type::concubines>(v, current)) { ++counter; }
      return counter;
    }
    
    static double each_concubine(context* ctx, const size_t &max_count, core::character* current, const std::function<double(context*, core::character*)> &f) {
      double counter = 0;
      for (auto v = current->family.concubines; v != nullptr && counter < max_count; v = utils::ring::list_next<utils::list_type::concubines>(v, current)) { 
        counter += f(ctx, v);
      }
      return counter;
    }
    
    static size_t each_concubine_logic(context* ctx, const size_t &, core::character* current, const std::function<double(context*, core::character*)> &f) {
      bool counter = true;
      for (auto v = current->family.concubines; v != nullptr; v = utils::ring::list_next<utils::list_type::concubines>(v, current)) { 
        const double ret = f(ctx, v);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      return counter;
    }
    
    static object first_concubine(core::character* current) {
      return object(current->family.concubines);
    }

    static size_t count_acquaintances(core::character* current) {
      size_t counter = 0;
      for (const auto &pair : current->relations.acquaintances) { counter += size_t(pair.first != nullptr); }
      return counter;
    }
    
    static double each_acquaintance(context* ctx, const size_t &max_count, core::character* current, const std::function<double(context*, core::character*)> &f) {
      double counter = 0;
      for (const auto &pair : current->relations.acquaintances) { 
        if (counter >= max_count) break;
        if (pair.first == nullptr) continue;
        counter += f(ctx, pair.first);
      }
      return counter;
    }
    
    static size_t each_acquaintance_logic(context* ctx, const size_t &, core::character* current, const std::function<double(context*, core::character*)> &f) {
      bool counter = true;
      for (const auto &pair : current->relations.acquaintances) { 
        if (pair.first == nullptr) continue;
        const double ret = f(ctx, pair.first);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      return counter;
    }
    
    static object first_acquaintance(core::character* current) {
      for (const auto &pair : current->relations.acquaintances) { if (pair.first != nullptr) return object(pair.first); }
      return object();
    }

    static size_t count_good_acquaintances(core::character* current) {
      size_t counter = 0;
      for (const auto &pair : current->relations.acquaintances) { counter += size_t(pair.first != nullptr && (pair.second.friendship > 0 || pair.second.love > 0)); }
      return counter;
    }
    
    static double each_good_acquaintance(context* ctx, const size_t &max_count, core::character* current, const std::function<double(context*, core::character*)> &f) {
      double counter = 0;
      for (const auto &pair : current->relations.acquaintances) { 
        if (counter >= max_count) break;
        if (pair.first == nullptr || (pair.second.friendship < 0 && pair.second.love < 0)) continue;
        counter += f(ctx, pair.first);
      }
      return counter;
    }
    
    static size_t each_good_acquaintance_logic(context* ctx, const size_t &, core::character* current, const std::function<double(context*, core::character*)> &f) {
      bool counter = true;
      for (const auto &pair : current->relations.acquaintances) { 
        if (pair.first == nullptr || (pair.second.friendship < 0 && pair.second.love < 0)) continue;
        const double ret = f(ctx, pair.first);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      return counter;
    }
    
    static object first_good_acquaintance(core::character* current) {
      for (const auto &pair : current->relations.acquaintances) { if (pair.first != nullptr && (pair.second.friendship > 0 || pair.second.love > 0)) return object(pair.first); }
      return object();
    }
    
    static size_t count_bad_acquaintances(core::character* current) {
      size_t counter = 0;
      for (const auto &pair : current->relations.acquaintances) { counter += size_t(pair.first != nullptr && (pair.second.friendship < 0 || pair.second.love < 0)); }
      return counter;
    }
    
    static double each_bad_acquaintance(context* ctx, const size_t &max_count, core::character* current, const std::function<double(context*, core::character*)> &f) {
      double counter = 0;
      for (const auto &pair : current->relations.acquaintances) { 
        if (counter >= max_count) break;
        if (pair.first == nullptr || (pair.second.friendship > 0 && pair.second.love > 0)) continue;
        counter += f(ctx, pair.first);
      }
      return counter;
    }
    
    static size_t each_bad_acquaintance_logic(context* ctx, const size_t &, core::character* current, const std::function<double(context*, core::character*)> &f) {
      bool counter = true;
      for (const auto &pair : current->relations.acquaintances) { 
        if (pair.first == nullptr || (pair.second.friendship > 0 && pair.second.love > 0)) continue;
        const double ret = f(ctx, pair.first);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      return counter;
    }
    
    static object first_bad_acquaintance(core::character* current) {
      for (const auto &pair : current->relations.acquaintances) { if (pair.first != nullptr && (pair.second.friendship < 0 || pair.second.love < 0)) return object(pair.first); }
      return object();
    }
    
    static size_t count_pals(core::character* current) {
      size_t counter = 0;
      for (const auto &pair : current->relations.acquaintances) { counter += size_t(pair.first != nullptr && pair.second.friendship > 0); }
      return counter;
    }
    
    static double each_pal(context* ctx, const size_t &max_count, core::character* current, const std::function<double(context*, core::character*)> &f) {
      double counter = 0;
      for (const auto &pair : current->relations.acquaintances) { 
        if (counter >= max_count) break;
        if (pair.first == nullptr || pair.second.friendship <= 0) continue;
        counter += f(ctx, pair.first);
      }
      return counter;
    }
    
    static size_t each_pal_logic(context* ctx, const size_t &, core::character* current, const std::function<double(context*, core::character*)> &f) {
      bool counter = true;
      for (const auto &pair : current->relations.acquaintances) {
        if (pair.first == nullptr || pair.second.friendship <= 0) continue;
        const double ret = f(ctx, pair.first);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      return counter;
    }
    
    static object first_pal(core::character* current) {
      for (const auto &pair : current->relations.acquaintances) { if (pair.first != nullptr && pair.second.friendship > 0) return object(pair.first); }
      return object();
    }
    
    static size_t count_foes(core::character* current) {
      size_t counter = 0;
      for (const auto &pair : current->relations.acquaintances) { counter += size_t(pair.first != nullptr && pair.second.friendship < 0); }
      return counter;
    }
    
    static double each_foe(context* ctx, const size_t &max_count, core::character* current, const std::function<double(context*, core::character*)> &f) {
      double counter = 0;
      for (const auto &pair : current->relations.acquaintances) { 
        if (counter >= max_count) break;
        if (pair.first == nullptr && pair.second.friendship >= 0) continue;
        counter += f(ctx, pair.first);
      }
      return counter;
    }
    
    static size_t each_foe_logic(context* ctx, const size_t &, core::character* current, const std::function<double(context*, core::character*)> &f) {
      bool counter = true;
      for (const auto &pair : current->relations.acquaintances) { 
        if (pair.first == nullptr && pair.second.friendship >= 0) continue;
        const double ret = f(ctx, pair.first);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      return counter;
    }
    
    static object first_foe(core::character* current) {
      for (const auto &pair : current->relations.acquaintances) { if (pair.first != nullptr && pair.second.friendship < 0) return object(pair.first); }
      return object();
    }
    
    static size_t count_sympathys(core::character* current) {
      size_t counter = 0;
      for (const auto &pair : current->relations.acquaintances) { counter += size_t(pair.first != nullptr && pair.second.love > 0); }
      return counter;
    }
    
    static double each_sympathy(context* ctx, const size_t &max_count, core::character* current, const std::function<double(context*, core::character*)> &f) {
      double counter = 0;
      for (const auto &pair : current->relations.acquaintances) { 
        if (counter >= max_count) break;
        if (pair.first == nullptr && pair.second.love <= 0) continue;
        counter += f(ctx, pair.first);
      }
      return counter;
    }
    
    static size_t each_sympathy_logic(context* ctx, const size_t &, core::character* current, const std::function<double(context*, core::character*)> &f) {
      bool counter = true;
      for (const auto &pair : current->relations.acquaintances) { 
        if (pair.first == nullptr && pair.second.love <= 0) continue;
        const double ret = f(ctx, pair.first);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      return counter;
    }
    
    static object first_sympathy(core::character* current) {
      for (const auto &pair : current->relations.acquaintances) { if (pair.first != nullptr && pair.second.love > 0) return object(pair.first); }
      return object();
    }
    
    static size_t count_dislikes(core::character* current) {
      size_t counter = 0;
      for (const auto &pair : current->relations.acquaintances) { counter += size_t(pair.first != nullptr && pair.second.love < 0); }
      return counter;
    }
    
    static double each_dislike(context* ctx, const size_t &max_count, core::character* current, const std::function<double(context*, core::character*)> &f) {
      double counter = 0;
      for (const auto &pair : current->relations.acquaintances) { 
        if (counter >= max_count) break;
        if (pair.first == nullptr && pair.second.love >= 0) continue;
        counter += f(ctx, pair.first);
      }
      return counter;
    }
    
    static size_t each_dislike_logic(context* ctx, const size_t &, core::character* current, const std::function<double(context*, core::character*)> &f) {
      bool counter = true;
      for (const auto &pair : current->relations.acquaintances) { 
        if (pair.first == nullptr && pair.second.love >= 0) continue;
        const double ret = f(ctx, pair.first);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      return counter;
    }
    
    static object first_dislike(core::character* current) {
      for (const auto &pair : current->relations.acquaintances) { if (pair.first != nullptr && pair.second.love < 0) return object(pair.first); }
      return object();
    }
    
    static size_t count_parents(core::character*) { return 2; }
    
    static double each_parent(context* ctx, const size_t &max_count, core::character* current, const std::function<double(context*, core::character*)> &f) {
      double counter = 0;
      for (uint8_t i = 0; i < 2; ++i) {
        if (counter >= max_count) break;
        counter += f(ctx, current->family.parents[i]);
      }
      return counter;
    }
    
    static size_t each_parent_logic(context* ctx, const size_t &, core::character* current, const std::function<double(context*, core::character*)> &f) {
      bool counter = true;
      for (uint8_t i = 0; i < 2; ++i) { 
        const double ret = f(ctx, current->family.parents[i]);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      return counter;
    }
    
    static object first_parent(core::character* current) {
      auto c = current->family.parents[0];
      c = c == nullptr ? current->family.parents[1] : c;
      return object(c);
    }
    
    static size_t count_ancestors(core::character*) { assert(false); return 0; }
    static double each_ancestor(context*, const size_t &, core::character*, const std::function<double(context*, core::character*)> &) { assert(false); return 0; }
    static size_t each_ancestor_logic(context*, const size_t &, core::character*, const std::function<double(context*, core::character*)> &) { assert(false); return 0; }
    static object first_ancestor(core::character*) { assert(false); return object(); }
    static size_t count_claims(core::character*) { assert(false); return 0; }
    static double each_claim(context*, const size_t &, core::character*, const std::function<double(context*, core::titulus*)> &) { assert(false); return 0; }
    static size_t each_claim_logic(context*, const size_t &, core::character*, const std::function<double(context*, core::titulus*)> &) { assert(false); return 0; }
    static object first_claim(core::character*) { assert(false); return object(); }
    static size_t count_de_jure_claims(core::character*) { assert(false); return 0; }
    static double each_de_jure_claim(context*, const size_t &, core::character*, const std::function<double(context*, core::titulus*)> &) { assert(false); return 0; }
    static size_t each_de_jure_claim_logic(context*, const size_t &, core::character*, const std::function<double(context*, core::titulus*)> &) { assert(false); return 0; }
    static object first_de_jure_claim(core::character*) { assert(false); return object(); }
    static size_t count_heir_to_titles(core::character*) { assert(false); return 0; }
    static double each_heir_to_title(context*, const size_t &, core::character*, const std::function<double(context*, core::titulus*)> &) { assert(false); return 0; }
    static size_t each_heir_to_title_logic(context*, const size_t &, core::character*, const std::function<double(context*, core::titulus*)> &) { assert(false); return 0; }
    static object first_heir_to_title(core::character*) { assert(false); return object(); }
    static size_t count_election_realms(core::character*) { assert(false); return 0; }
    static double each_election_realm(context*, const size_t &, core::character*, const std::function<double(context*, utils::handle<core::realm>)> &) { assert(false); return 0; }
    static size_t each_election_realm_logic(context*, const size_t &, core::character*, const std::function<double(context*, utils::handle<core::realm>)> &) { assert(false); return 0; }
    static object first_election_realm(core::character*) { assert(false); return object(); }
    
// дурацкие функции боже
    static size_t count_members(utils::handle<core::realm> r) {
      size_t counter = 0;
      if (r->is_state_independent_power()) counter += utils::ring::list_count<utils::list_type::statemans>(r->members);
      if (r->is_council())                 counter += utils::ring::list_count<utils::list_type::councilors>(r->members);
      if (r->is_assembly())                counter += utils::ring::list_count<utils::list_type::assemblers>(r->members);
      if (r->is_tribunal())                counter += utils::ring::list_count<utils::list_type::magistrates>(r->members);
      if (r->is_clergy())                  counter += utils::ring::list_count<utils::list_type::clergymans>(r->members);
      
      return counter;
    }
    
    static size_t count_electors(utils::handle<core::realm> r) {
      size_t counter = 0;
      if (r->is_state_independent_power()) counter += utils::ring::list_count<utils::list_type::state_electors>(r->members);
      if (r->is_council())                 counter += utils::ring::list_count<utils::list_type::council_electors>(r->members);
      if (r->is_assembly())                counter += utils::ring::list_count<utils::list_type::assembly_electors>(r->members);
      if (r->is_tribunal())                counter += utils::ring::list_count<utils::list_type::tribunal_electors>(r->members);
      if (r->is_clergy())                  counter += utils::ring::list_count<utils::list_type::clergy_electors>(r->members);
      
      return counter;
    }
    
    static double each_member(context* ctx, const size_t &max_count, utils::handle<core::realm> r, const std::function<double(context*, core::character*)> &f) {
      double counter = 0;
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
    
    static size_t each_member_logic(context* ctx, const size_t &, utils::handle<core::realm> r, const std::function<double(context*, core::character*)> &f) {
      bool counter = true;
      if (r->is_state_independent_power()) {
        for (auto m = r->members; m != nullptr; m = utils::ring::list_next<utils::list_type::statemans>(m, r->members)) {
          const double ret = f(ctx, m);
          counter = counter && (std::isnan(ret) ? true : bool(ret));
        }
      } else if (r->is_council()) { 
        for (auto m = r->members; m != nullptr; m = utils::ring::list_next<utils::list_type::councilors>(m, r->members)) {
          const double ret = f(ctx, m);
          counter = counter && (std::isnan(ret) ? true : bool(ret));
        }
      } else if (r->is_assembly()) {
        for (auto m = r->members; m != nullptr; m = utils::ring::list_next<utils::list_type::assemblers>(m, r->members)) {
          const double ret = f(ctx, m);
          counter = counter && (std::isnan(ret) ? true : bool(ret));
        }
      } else if (r->is_tribunal()) {
        for (auto m = r->members; m != nullptr; m = utils::ring::list_next<utils::list_type::magistrates>(m, r->members)) {
          const double ret = f(ctx, m);
          counter = counter && (std::isnan(ret) ? true : bool(ret));
        }
      } else if (r->is_clergy()) {
        for (auto m = r->members; m != nullptr; m = utils::ring::list_next<utils::list_type::clergymans>(m, r->members)) {
          const double ret = f(ctx, m);
          counter = counter && (std::isnan(ret) ? true : bool(ret));
        }
      }
      
      return counter;
    }
    
    static double each_elector(context* ctx, const size_t &max_count, utils::handle<core::realm> r, const std::function<double(context*, core::character*)> &f) {
      double counter = 0;
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
    
    static size_t each_elector_logic(context* ctx, const size_t &, utils::handle<core::realm> r, const std::function<double(context*, core::character*)> &f) {
      bool counter = true;
      if (r->is_state_independent_power()) {
        for (auto m = r->electors; m != nullptr; m = utils::ring::list_next<utils::list_type::state_electors>(m, r->electors)) {
          const double ret = f(ctx, m);
          counter = counter && (std::isnan(ret) ? true : bool(ret));
        }
      } else if (r->is_council()) { 
        for (auto m = r->electors; m != nullptr; m = utils::ring::list_next<utils::list_type::council_electors>(m, r->electors)) {
          const double ret = f(ctx, m);
          counter = counter && (std::isnan(ret) ? true : bool(ret));
        }
      } else if (r->is_assembly()) {
        for (auto m = r->electors; m != nullptr; m = utils::ring::list_next<utils::list_type::assembly_electors>(m, r->electors)) {
          const double ret = f(ctx, m);
          counter = counter && (std::isnan(ret) ? true : bool(ret));
        }
      } else if (r->is_tribunal()) {
        for (auto m = r->electors; m != nullptr; m = utils::ring::list_next<utils::list_type::tribunal_electors>(m, r->electors)) {
          const double ret = f(ctx, m);
          counter = counter && (std::isnan(ret) ? true : bool(ret));
        }
      } else if (r->is_clergy()) {
        for (auto m = r->electors; m != nullptr; m = utils::ring::list_next<utils::list_type::clergy_electors>(m, r->electors)) {
          const double ret = f(ctx, m);
          counter = counter && (std::isnan(ret) ? true : bool(ret));
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
    
    static size_t count_wars(utils::handle<core::realm> r) {
      size_t counter = 0;
      for (const auto &pair : r->relations) { counter += size_t(pair.second.relation_type == core::diplomacy::war_attacker || pair.second.relation_type == core::diplomacy::war_defender); }
      return counter;
    }
    
    static double each_war(context* ctx, const size_t &max_count, utils::handle<core::realm> r, const std::function<double(context*, utils::handle<core::war>)> &f) {
      double counter = 0;
      for (const auto &pair : r->relations) {
        if (counter >= max_count) break;
        if (pair.second.relation_type != core::diplomacy::war_attacker && pair.second.relation_type != core::diplomacy::war_defender) continue;
        ASSERT(pair.second.war.valid());
        counter += f(ctx, pair.second.war);
      }
      return counter;
    }
    
    static size_t each_war_logic(context* ctx, const size_t &, utils::handle<core::realm> r, const std::function<double(context*, utils::handle<core::war>)> &f) {
      bool counter = true;
      for (const auto &pair : r->relations) {
        if (pair.second.relation_type != core::diplomacy::war_attacker && pair.second.relation_type != core::diplomacy::war_defender) continue;
        ASSERT(pair.second.war.valid());
        const double ret = f(ctx, pair.second.war);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      return counter;
    }
    
    static object first_war(utils::handle<core::realm> r) {
      for (const auto &pair : r->relations) { 
        if (pair.second.relation_type == core::diplomacy::war_attacker || pair.second.relation_type == core::diplomacy::war_defender) 
          return object(pair.second.war); 
      }
      return object();
    }
    
    static size_t count_war_allys(utils::handle<core::realm> r) {
      // как определить является ли текущий реалм защитником или атакующим? хороший вопрос
      // нужно все отношения между реалмами свести в один контейнер, и там задать параметры по которым я все это узнаю
      size_t counter = 0;
      for (const auto &pair : r->relations) {
        if (pair.second.relation_type == core::diplomacy::war_attacker) {
          ASSERT(pair.second.war.valid());
          const auto &war = pair.second.war;
          counter += war->attackers.size();
        }
        
        if (pair.second.relation_type == core::diplomacy::war_defender) {
          ASSERT(pair.second.war.valid());
          const auto &war = pair.second.war;
          counter += war->defenders.size();
        }
      }
      
      return counter;
    }
    
    static double each_war_ally(context* ctx, const size_t &max_count, utils::handle<core::realm> r, const std::function<double(context*, core::character*)> &f) {
      double counter = 0;
      for (const auto &pair : r->relations) {
        if (counter >= max_count) break;
        
        if (pair.second.relation_type == core::diplomacy::war_attacker) {
          ASSERT(pair.second.war.valid());
          const auto &war = pair.second.war;
          for (const auto &attacker : war->attackers) {
            counter += f(ctx, attacker);
          }
        }
        
        if (pair.second.relation_type == core::diplomacy::war_defender) {
          ASSERT(pair.second.war.valid());
          const auto &war = pair.second.war;
          counter += war->defenders.size();
          for (const auto &defender : war->defenders) {
            counter += f(ctx, defender);
          }
        }
      }
      
      return counter;
    }
    
    static size_t each_war_ally_logic(context* ctx, const size_t &, utils::handle<core::realm> r, const std::function<double(context*, core::character*)> &f) {
      bool counter = true;
      for (const auto &pair : r->relations) { 
        if (pair.second.relation_type == core::diplomacy::war_attacker) {
          ASSERT(pair.second.war.valid());
          const auto &war = pair.second.war;
          for (const auto &attacker : war->attackers) {
            const double ret = f(ctx, attacker);
            counter = counter && (std::isnan(ret) ? true : bool(ret));
          }
        }
        
        if (pair.second.relation_type == core::diplomacy::war_defender) {
          ASSERT(pair.second.war.valid());
          const auto &war = pair.second.war;
          for (const auto &defender : war->defenders) {
            const double ret = f(ctx, defender);
            counter = counter && (std::isnan(ret) ? true : bool(ret));
          }
        }
      }
      
      return counter;
    }
    
    static object first_war_ally(utils::handle<core::realm> r) {
      for (const auto &pair : r->relations) { 
        if (pair.second.relation_type == core::diplomacy::war_attacker) {
          ASSERT(pair.second.war.valid());
          const auto &war = pair.second.war;
          for (const auto &attacker : war->attackers) { return object(attacker); }
        }
        
        if (pair.second.relation_type == core::diplomacy::war_defender) {
          ASSERT(pair.second.war.valid());
          const auto &war = pair.second.war;
          for (const auto &defender : war->defenders) { return object(defender); }
        }
      }
      return object();
    }
    
    static size_t count_war_enemys(utils::handle<core::realm> r) {
      // как определить является ли текущий реалм защитником или атакующим? хороший вопрос
      // нужно все отношения между реалмами свести в один контейнер, и там задать параметры по которым я все это узнаю
      size_t counter = 0;
      for (const auto &pair : r->relations) {
        if (pair.second.relation_type == core::diplomacy::war_attacker) {
          ASSERT(pair.second.war.valid());
          const auto &war = pair.second.war;
          counter += war->defenders.size();
        }
        
        if (pair.second.relation_type == core::diplomacy::war_defender) {
          ASSERT(pair.second.war.valid());
          const auto &war = pair.second.war;
          counter += war->attackers.size();
        }
      }
      
      return counter;
    }
    
    static double each_war_enemy(context* ctx, const size_t &max_count, utils::handle<core::realm> r, const std::function<double(context*, core::character*)> &f) {
      double counter = 0;
      for (const auto &pair : r->relations) {
        if (counter >= max_count) break;
        
        if (pair.second.relation_type == core::diplomacy::war_attacker) {
          ASSERT(pair.second.war.valid());
          const auto &war = pair.second.war;
          for (const auto &defender : war->defenders) {
            counter += f(ctx, defender);
          }
        }
        
        if (pair.second.relation_type == core::diplomacy::war_defender) {
          ASSERT(pair.second.war.valid());
          const auto &war = pair.second.war;
          counter += war->defenders.size();
          for (const auto &attacker : war->attackers) {
            counter += f(ctx, attacker);
          }
        }
      }
      
      return counter;
    }
    
    static size_t each_war_enemy_logic(context* ctx, const size_t &, utils::handle<core::realm> r, const std::function<double(context*, core::character*)> &f) {
      bool counter = true;
      for (const auto &pair : r->relations) { 
        if (pair.second.relation_type == core::diplomacy::war_attacker) {
          ASSERT(pair.second.war.valid());
          const auto &war = pair.second.war;
          for (const auto &defender : war->defenders) {
            const double ret = f(ctx, defender);
            counter = counter && (std::isnan(ret) ? true : bool(ret));
          }
        }
        
        if (pair.second.relation_type == core::diplomacy::war_defender) {
          ASSERT(pair.second.war.valid());
          const auto &war = pair.second.war;
          for (const auto &attacker : war->attackers) {
            const double ret = f(ctx, attacker);
            counter = counter && (std::isnan(ret) ? true : bool(ret));
          }
        }
      }
      
      return counter;
    }
    
    static object first_war_enemy(utils::handle<core::realm> r) {
      for (const auto &pair : r->relations) { 
        if (pair.second.relation_type == core::diplomacy::war_attacker) {
          ASSERT(pair.second.war.valid());
          const auto &war = pair.second.war;
          for (const auto &defender : war->defenders) { return object(defender); }
        }
        
        if (pair.second.relation_type == core::diplomacy::war_defender) {
          ASSERT(pair.second.war.valid());
          const auto &war = pair.second.war;
          for (const auto &attacker : war->attackers) { return object(attacker); }
        }
      }
      return object();
    }
    
    static size_t count_allys(utils::handle<core::realm> r) {
      size_t counter = 0;
      for (const auto &pair : r->relations) { counter += size_t(pair.second.relation_type == core::diplomacy::ally); }
      return counter;
    }
    
    static double each_ally(context* ctx, const size_t &max_count, utils::handle<core::realm> r, const std::function<double(context*, utils::handle<core::realm>)> &f) {
      double counter = 0;
      for (const auto &pair : r->relations) {
        if (counter >= max_count) break;
        if (pair.second.relation_type != core::diplomacy::ally) continue;
        ASSERT(pair.second.related_realm.valid());
        counter += f(ctx, pair.second.related_realm);
      }
      return counter;
    }
    
    static size_t each_ally_logic(context* ctx, const size_t &, utils::handle<core::realm> r, const std::function<double(context*, utils::handle<core::realm>)> &f) {
      bool counter = true;
      for (const auto &pair : r->relations) {
        if (pair.second.relation_type != core::diplomacy::ally) continue;
        ASSERT(pair.second.related_realm.valid());
        const double ret = f(ctx, pair.second.related_realm);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      return counter;
    }
    
    static object first_ally(utils::handle<core::realm> r) {
      for (const auto &pair : r->relations) { 
        if (pair.second.relation_type == core::diplomacy::ally) return object(pair.second.related_realm); 
      }
      return object();
    }
    
    static size_t count_truce_holders(utils::handle<core::realm> r) {
      size_t counter = 0;
      for (const auto &pair : r->relations) { counter += size_t(pair.second.relation_type == core::diplomacy::truce_holder || pair.second.relation_type == core::diplomacy::truce_2way); }
      return counter;
    }
    
    static double each_truce_holder(context* ctx, const size_t &max_count, utils::handle<core::realm> r, const std::function<double(context*, utils::handle<core::realm>)> &f) {
      double counter = 0;
      for (const auto &pair : r->relations) {
        if (counter >= max_count) break;
        if (pair.second.relation_type != core::diplomacy::truce_holder && pair.second.relation_type != core::diplomacy::truce_2way) continue;
        ASSERT(pair.second.related_realm.valid());
        counter += f(ctx, pair.second.related_realm);
      }
      return counter;
    }
    
    static size_t each_truce_holder_logic(context* ctx, const size_t &, utils::handle<core::realm> r, const std::function<double(context*, utils::handle<core::realm>)> &f) {
      bool counter = true;
      for (const auto &pair : r->relations) {
        if (pair.second.relation_type != core::diplomacy::truce_holder && pair.second.relation_type != core::diplomacy::truce_2way) continue;
        ASSERT(pair.second.related_realm.valid());
        const double ret = f(ctx, pair.second.related_realm);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      return counter;
    }
    
    static object first_truce_holder(utils::handle<core::realm> r) {
      for (const auto &pair : r->relations) { 
        if (pair.second.relation_type == core::diplomacy::truce_holder || pair.second.relation_type == core::diplomacy::truce_2way) return object(pair.second.related_realm); 
      }
      return object();
    }
    
    static size_t count_truce_targets(utils::handle<core::realm> r) {
      size_t counter = 0;
      for (const auto &pair : r->relations) { counter += size_t(pair.second.relation_type == core::diplomacy::truce_receiver || pair.second.relation_type == core::diplomacy::truce_2way); }
      return counter;
    }
    
    static double each_truce_target(context* ctx, const size_t &max_count, utils::handle<core::realm> r, const std::function<double(context*, utils::handle<core::realm>)> &f) {
      double counter = 0;
      for (const auto &pair : r->relations) {
        if (counter >= max_count) break;
        if (pair.second.relation_type != core::diplomacy::truce_receiver && pair.second.relation_type != core::diplomacy::truce_2way) continue;
        ASSERT(pair.second.related_realm.valid());
        counter += f(ctx, pair.second.related_realm);
      }
      return counter;
    }
    
    static size_t each_truce_target_logic(context* ctx, const size_t &, utils::handle<core::realm> r, const std::function<double(context*, utils::handle<core::realm>)> &f) {
      bool counter = true;
      for (const auto &pair : r->relations) {
        if (pair.second.relation_type != core::diplomacy::truce_receiver && pair.second.relation_type != core::diplomacy::truce_2way) continue;
        ASSERT(pair.second.related_realm.valid());
        const double ret = f(ctx, pair.second.related_realm);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      return counter;
    }
    
    static object first_truce_target(utils::handle<core::realm> r) {
      for (const auto &pair : r->relations) { 
        if (pair.second.relation_type == core::diplomacy::truce_receiver || pair.second.relation_type == core::diplomacy::truce_2way) return object(pair.second.related_realm); 
      }
      return object();
    }
    
    static size_t count_prisoners(utils::handle<core::realm> r) {
      return utils::ring::list_count<utils::list_type::prisoners>(r->prisoners);
    }
    
    static double each_prisoner(context* ctx, const size_t &max_count, utils::handle<core::realm> r, const std::function<double(context*, core::character*)> &f) {
      double counter = 0;
      for (auto c = r->prisoners; c != nullptr && counter < max_count; c = utils::ring::list_next<utils::list_type::prisoners>(c, r->prisoners)) {
        counter += f(ctx, c);
      }
      return counter;
    }
    
    static size_t each_prisoner_logic(context* ctx, const size_t &, utils::handle<core::realm> r, const std::function<double(context*, core::character*)> &f) {
      bool counter = true;
      for (auto c = r->prisoners; c != nullptr; c = utils::ring::list_next<utils::list_type::prisoners>(c, r->prisoners)) {
        const double ret = f(ctx, c);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      return counter;
    }
    
    static object first_prisoner(utils::handle<core::realm> r) {
      return object(r->prisoners);
    }
    
    static size_t count_courtiers(utils::handle<core::realm> r) {
      return utils::ring::list_count<utils::list_type::courtiers>(r->courtiers);
    }
    
    static double each_courtier(context* ctx, const size_t &max_count, utils::handle<core::realm> r, const std::function<double(context*, core::character*)> &f) {
      double counter = 0;
      for (auto c = r->courtiers; c != nullptr && counter < max_count; c = utils::ring::list_next<utils::list_type::courtiers>(c, r->courtiers)) {
        counter += f(ctx, c);
      }
      return counter;
    }
    
    static size_t each_courtier_logic(context* ctx, const size_t &, utils::handle<core::realm> r, const std::function<double(context*, core::character*)> &f) {
      bool counter = true;
      for (auto c = r->courtiers; c != nullptr; c = utils::ring::list_next<utils::list_type::courtiers>(c, r->courtiers)) {
        const double ret = f(ctx, c);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      return counter;
    }
    
    static object first_courtier(utils::handle<core::realm> r) {
      return object(r->courtiers);
    }
    
    static size_t count_owned_titles(utils::handle<core::realm> r) {
      return utils::ring::list_count<utils::list_type::titles>(r->titles);
    }
    
    static double each_owned_title(context* ctx, const size_t &max_count, utils::handle<core::realm> r, const std::function<double(context*, core::titulus*)> &f) {
      double counter = 0.0;
      for (auto t = r->titles; t != nullptr && counter < max_count; t = utils::ring::list_next<utils::list_type::titles>(t, r->titles)) {
        counter += f(ctx, t);
      }
      return counter;
    }
    
    static size_t each_owned_title_logic(context* ctx, const size_t &max_count, utils::handle<core::realm> r, const std::function<double(context*, core::titulus*)> &f) {
      bool counter = true;
      for (auto t = r->titles; t != nullptr && counter < max_count; t = utils::ring::list_next<utils::list_type::titles>(t, r->titles)) {
        const double ret = f(ctx, t);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      return counter;
    }
    
    static object first_owned_title(utils::handle<core::realm> r) {
      return object(r->titles);
    }
    
    static size_t count_realm_titles(utils::handle<core::realm> r) {
      size_t counter = count_owned_titles(r);
      for (auto v = r->vassals; v != nullptr; v = utils::ring::list_next<utils::list_type::vassals>(v, r->vassals)) {
        const auto h = utils::handle<core::realm>(v, v->object_token);
        counter += count_realm_titles(h);
      }
      return counter;
    }
    
    static double each_realm_title(context* ctx, const size_t &max_count, utils::handle<core::realm> r, const std::function<double(context*, core::titulus*)> &f) {
      double counter = each_owned_title(ctx, max_count, r, f);
      for (auto v = r->vassals; v != nullptr; v = utils::ring::list_next<utils::list_type::vassals>(v, r->vassals)) {
        const auto h = utils::handle<core::realm>(v, v->object_token);
        counter += each_realm_title(ctx, max_count, h, f);
      }
      return std::min(counter, double(max_count));
    }
    
    static size_t each_realm_title_logic(context* ctx, const size_t &max_count, utils::handle<core::realm> r, const std::function<double(context*, core::titulus*)> &f) {
      bool counter = each_owned_title_logic(ctx, max_count, r, f);
      for (auto v = r->vassals; v != nullptr; v = utils::ring::list_next<utils::list_type::vassals>(v, r->vassals)) {
        const auto h = utils::handle<core::realm>(v, v->object_token);
        counter = counter && bool(each_realm_title_logic(ctx, max_count, h, f));
      }
      return counter;
    }
    
    static object first_realm_title(utils::handle<core::realm> r) {
      const auto obj = first_owned_title(r);
      if (obj.valid()) return obj;
      for (auto v = r->vassals; v != nullptr; v = utils::ring::list_next<utils::list_type::vassals>(v, r->vassals)) {
        const auto h = utils::handle<core::realm>(v, v->object_token);
        const auto obj = first_owned_title(h);
        if (obj.valid()) return obj;
      }
      
      return object();
    }
    
    static size_t count_directly_owned_provinces(utils::handle<core::realm> r) {
      size_t counter = 0;
      for (auto t = r->titles; t != nullptr; t = utils::ring::list_next<utils::list_type::titles>(t, r->titles)) {
        counter += size_t(t->type() == core::titulus::type::baron);
      }
      return counter;
    }
    
    static double each_directly_owned_province(context* ctx, const size_t &max_count, utils::handle<core::realm> r, const std::function<double(context*, core::province*)> &f) {
      double counter = 0.0;
      for (auto t = r->titles; t != nullptr && counter < max_count; t = utils::ring::list_next<utils::list_type::titles>(t, r->titles)) {
        if (t->type() != core::titulus::type::baron) continue;
        assert(t->province != nullptr);
        counter += f(ctx, t->province);
      }
      return counter;
    }
    
    static size_t each_directly_owned_province_logic(context* ctx, const size_t &, utils::handle<core::realm> r, const std::function<double(context*, core::province*)> &f) {
      bool counter = true;
      for (auto t = r->titles; t != nullptr; t = utils::ring::list_next<utils::list_type::titles>(t, r->titles)) {
        if (t->type() != core::titulus::type::baron) continue;
        assert(t->province != nullptr);
        const double ret = f(ctx, t->province);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      return counter;
    }
    
    static object first_directly_owned_province(utils::handle<core::realm> r) {
      for (auto t = r->titles; t != nullptr; t = utils::ring::list_next<utils::list_type::titles>(t, r->titles)) {
        if (t->type() == core::titulus::type::baron) return object(t->province);
      }
      return object();
    }
    
    static size_t count_directly_owned_citys(utils::handle<core::realm> r) {
      size_t counter = 0;
      for (auto t = r->titles; t != nullptr; t = utils::ring::list_next<utils::list_type::titles>(t, r->titles)) {
        counter += size_t(t->type() == core::titulus::type::city);
      }
      return counter;
    }
    
    static double each_directly_owned_city(context* ctx, const size_t &max_count, utils::handle<core::realm> r, const std::function<double(context*, core::city*)> &f) {
      double counter = 0.0;
      for (auto t = r->titles; t != nullptr && counter < max_count; t = utils::ring::list_next<utils::list_type::titles>(t, r->titles)) {
        if (t->type() != core::titulus::type::city) continue;
        assert(t->city != nullptr);
        counter += f(ctx, t->city);
      }
      return counter;
    }
    
    static size_t each_directly_owned_city_logic(context* ctx, const size_t &, utils::handle<core::realm> r, const std::function<double(context*, core::city*)> &f) {
      bool counter = true;
      for (auto t = r->titles; t != nullptr; t = utils::ring::list_next<utils::list_type::titles>(t, r->titles)) {
        if (t->type() != core::titulus::type::city) continue;
        assert(t->city != nullptr);
        const double ret = f(ctx, t->city);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      return counter;
    }
    
    static object first_directly_owned_city(utils::handle<core::realm> r) {
      for (auto t = r->titles; t != nullptr; t = utils::ring::list_next<utils::list_type::titles>(t, r->titles)) {
        if (t->type() == core::titulus::type::city) return object(t->province);
      }
      return object();
    }
    
    static size_t count_realm_citys(utils::handle<core::realm> r) {
      size_t counter = count_directly_owned_citys(r);
      for (auto v = r->vassals; v != nullptr; v = utils::ring::list_next<utils::list_type::vassals>(v, r->vassals)) {
        const auto h = utils::handle<core::realm>(v, v->object_token);
        counter += count_realm_citys(h);
      }
      return counter;
    }
    
    static double each_realm_city(context* ctx, const size_t &max_count, utils::handle<core::realm> r, const std::function<double(context*, core::city*)> &f) {
      double counter = each_directly_owned_city(ctx, max_count, r, f);
      for (auto v = r->vassals; v != nullptr && counter < max_count; v = utils::ring::list_next<utils::list_type::vassals>(v, r->vassals)) {
        const auto h = utils::handle<core::realm>(v, v->object_token);
        counter += each_realm_city(ctx, max_count, h, f);
      }
      return std::min(counter, double(max_count));
    }
    
    static size_t each_realm_city_logic(context* ctx, const size_t &, utils::handle<core::realm> r, const std::function<double(context*, core::city*)> &f) {
      bool counter = each_directly_owned_city_logic(ctx, SIZE_MAX, r, f);
      for (auto v = r->vassals; v != nullptr; v = utils::ring::list_next<utils::list_type::vassals>(v, r->vassals)) {
        const auto h = utils::handle<core::realm>(v, v->object_token);
        counter = counter && bool(each_realm_city_logic(ctx, SIZE_MAX, h, f));
      }
      return counter;
    }
    
    static object first_realm_city(utils::handle<core::realm> r) {
      const auto obj = first_directly_owned_city(r);
      if (obj.valid()) return obj;
      
      for (auto v = r->vassals; v != nullptr; v = utils::ring::list_next<utils::list_type::vassals>(v, r->vassals)) {
        const auto h = utils::handle<core::realm>(v, v->object_token);
        const auto obj = first_realm_city(h);
        if (obj.valid()) return obj;
      }
      
      return object();
    }
    
    static size_t count_realm_provinces(utils::handle<core::realm> r) {
      size_t counter = count_directly_owned_provinces(r);
      for (auto v = r->vassals; v != nullptr; v = utils::ring::list_next<utils::list_type::vassals>(v, r->vassals)) {
        const auto h = utils::handle<core::realm>(v, v->object_token);
        counter += count_realm_provinces(h);
      }
      return counter;
    }
    
    static double each_realm_province(context* ctx, const size_t &max_count, utils::handle<core::realm> r, const std::function<double(context*, core::province*)> &f) {
      double counter = each_directly_owned_province(ctx, max_count, r, f);
      for (auto v = r->vassals; v != nullptr && counter < max_count; v = utils::ring::list_next<utils::list_type::vassals>(v, r->vassals)) {
        const auto h = utils::handle<core::realm>(v, v->object_token);
        counter += each_realm_province(ctx, max_count, h, f);
      }
      return std::min(counter, double(max_count));
    }
    
    static size_t each_realm_province_logic(context* ctx, const size_t &, utils::handle<core::realm> r, const std::function<double(context*, core::province*)> &f) {
      bool counter = each_directly_owned_province_logic(ctx, SIZE_MAX, r, f);
      for (auto v = r->vassals; v != nullptr; v = utils::ring::list_next<utils::list_type::vassals>(v, r->vassals)) {
        const auto h = utils::handle<core::realm>(v, v->object_token);
        counter = counter && bool(each_realm_province_logic(ctx, SIZE_MAX, h, f));
      }
      return counter;
    }
    
    static object first_realm_province(utils::handle<core::realm> r) {
      const auto obj = first_directly_owned_province(r);
      if (obj.valid()) return obj;
      
      for (auto v = r->vassals; v != nullptr; v = utils::ring::list_next<utils::list_type::vassals>(v, r->vassals)) {
        const auto h = utils::handle<core::realm>(v, v->object_token);
        const auto obj = first_realm_province(h);
        if (obj.valid()) return obj;
      }
      
      return object();
    }
    
    static size_t count_vassals(utils::handle<core::realm> r) {
      return utils::ring::list_count<utils::list_type::vassals>(r->vassals);
    }
    
    static double each_vassal(context* ctx, const size_t &max_count, utils::handle<core::realm> r, const std::function<double(context*, utils::handle<core::realm>)> &f) {
      double counter = 0;
      for (auto c = r->vassals; c != nullptr && counter < max_count; c = utils::ring::list_next<utils::list_type::vassals>(c, r->vassals)) {
        ASSERT(c->object_token != SIZE_MAX);
        counter += f(ctx, utils::handle<core::realm>(c, c->object_token));
      }
      return counter;
    }
    
    static size_t each_vassal_logic(context* ctx, const size_t &, utils::handle<core::realm> r, const std::function<double(context*, utils::handle<core::realm>)> &f) {
      bool counter = true;
      for (auto c = r->vassals; c != nullptr; c = utils::ring::list_next<utils::list_type::vassals>(c, r->vassals)) {
        ASSERT(c->object_token != SIZE_MAX);
        const double ret = f(ctx, utils::handle<core::realm>(c, c->object_token));
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      return counter;
    }
    
    static object first_vassal(utils::handle<core::realm> r) {
      return object(utils::handle<core::realm>(r->vassals, r->vassals->object_token));
    }
    
    static size_t count_vassal_or_belows(utils::handle<core::realm> r) {
      size_t counter = 0;
      for (auto c = r->vassals; c != nullptr; c = utils::ring::list_next<utils::list_type::vassals>(c, r->vassals)) {
        counter += 1;
        counter += count_vassal_or_belows(utils::handle<core::realm>(c, c->object_token));
      }
      return counter;
    }
    
    static double each_vassal_or_below(context* ctx, const size_t &max_count, utils::handle<core::realm> r, const std::function<double(context*, utils::handle<core::realm>)> &f) {
      double counter = 0;
      // может два раза пройти? зачем?
      for (auto c = r->vassals; c != nullptr && counter < max_count; c = utils::ring::list_next<utils::list_type::vassals>(c, r->vassals)) {
        ASSERT(c->object_token != SIZE_MAX);
        const auto realm = utils::handle<core::realm>(c, c->object_token);
        counter += f(ctx, realm);
        counter += each_vassal_or_below(ctx, max_count, realm, f);
      }
      return std::min(counter, double(max_count));
    }
    
    static size_t each_vassal_or_below_logic(context* ctx, const size_t &, utils::handle<core::realm> r, const std::function<double(context*, utils::handle<core::realm>)> &f) {
      bool counter = true;
      for (auto c = r->vassals; c != nullptr; c = utils::ring::list_next<utils::list_type::vassals>(c, r->vassals)) {
        ASSERT(c->object_token != SIZE_MAX);
        const auto realm = utils::handle<core::realm>(c, c->object_token);
        const double ret = f(ctx, realm);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
        counter = counter && bool(each_vassal_or_below(ctx, SIZE_MAX, realm, f));
      }
      return counter;
    }
    
    static object first_vassal_or_below(utils::handle<core::realm> r) {
      return object(utils::handle<core::realm>(r->vassals, r->vassals->object_token));
    }
    
    static size_t count_liege_or_aboves(utils::handle<core::realm> r) {
      size_t counter = size_t(r.valid() && r->liege.valid());
      if (r.valid()) counter += count_liege_or_aboves(r->liege);
      return counter;
    }
    
    static double each_liege_or_above(context* ctx, const size_t &max_count, utils::handle<core::realm> r, const std::function<double(context*, utils::handle<core::realm>)> &f) {
      if (!r.valid()) return 0.0;
      return std::min(double(max_count), f(ctx, r->liege) + each_liege_or_above(ctx, max_count, r->liege, f));
    }
    
    static size_t each_liege_or_above_logic(context* ctx, const size_t &, utils::handle<core::realm> r, const std::function<double(context*, utils::handle<core::realm>)> &f) {
      if (!r.valid()) return true;
      const double ret = f(ctx, r->liege);
      return (std::isnan(ret) ? true : bool(ret)) && bool(each_liege_or_above(ctx, SIZE_MAX, r->liege, f));
    }
    
    static object first_liege_or_above(utils::handle<core::realm> r) {
      return object(r->liege);
    }
    
    template <typename F>
    static phmap::flat_hash_set<core::titulus*> get_de_jure_titles(utils::handle<core::realm> r, const F &get_func) {
      phmap::flat_hash_set<core::titulus*> duchies;
      duchies.reserve(100);
      const auto func = [&] (context*, core::city* c) -> double {
        auto t = get_func(c->title);
        if (t != nullptr) duchies.insert(t);
        return 0.0;
      };
      each_realm_city(nullptr, SIZE_MAX, r, func);
      return duchies;
    }
    
    
    // эти функции - это обход всех титулов текущих владений: может быть такое что у реалма нет ни одного города в герцогстве, но есть титул, этот титул пропускаем
    template <typename F>
    static size_t count_de_jure_titles(utils::handle<core::realm> r, const F &get_func) {
      const auto &titles = get_de_jure_titles(r, get_func);
      return titles.size();
    }
    
    template <typename F>
    static double each_de_jure_title(context* ctx, const size_t &max_count, utils::handle<core::realm> r, const std::function<double(context*, core::titulus*)> &f, const F &get_func) {
      const auto &titles = get_de_jure_titles(r, get_func);
      double counter = 0.0;
      for (const auto t : titles) {
        if (counter >= max_count) break;
        counter += f(ctx, t);
      }
      return counter;
    }
    
    template <typename F>
    static size_t each_de_jure_title_logic(context* ctx, const size_t &, utils::handle<core::realm> r, const std::function<double(context*, core::titulus*)> &f, const F &get_func) {
      const auto &titles = get_de_jure_titles(r, get_func);
      bool counter = true;
      for (const auto t : titles) {
        const double ret = f(ctx, t);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      return counter;
    }
    
    template <typename F>
    static object first_de_jure_title(utils::handle<core::realm> r, const F &get_func) {
      core::titulus* d_title = nullptr;
      const auto func = [&] (context*, core::city* c) -> double {
        auto t = get_func(c->title);
        d_title = t;
        return t == nullptr ? 0.0 : 1.0;
      };
      each_realm_city(nullptr, 1, r, func);
      return object(d_title);
    }
    
    typedef core::titulus* (*func)(core::titulus*);
    static size_t count_de_jure_duchys(utils::handle<core::realm> r) {
      return count_de_jure_titles(r, func(core::rights::get_duchy));
    }
    
    static double each_de_jure_duchy(context* ctx, const size_t &max_count, utils::handle<core::realm> r, const std::function<double(context*, core::titulus*)> &f) {
      return each_de_jure_title(ctx, max_count, r, f, func(core::rights::get_duchy));
    }
    
    static size_t each_de_jure_duchy_logic(context* ctx, const size_t &max_count, utils::handle<core::realm> r, const std::function<double(context*, core::titulus*)> &f) {
      return each_de_jure_title_logic(ctx, max_count, r, f, func(core::rights::get_duchy));
    }
    
    static object first_de_jure_duchy(utils::handle<core::realm> r) {
      return first_de_jure_title(r, func(core::rights::get_duchy));
    }
    
    static size_t count_de_jure_kingdoms(utils::handle<core::realm> r) {
      return count_de_jure_titles(r, func(core::rights::get_kingdom));
    }
    
    static double each_de_jure_kingdom(context* ctx, const size_t &max_count, utils::handle<core::realm> r, const std::function<double(context*, core::titulus*)> &f) {
      return each_de_jure_title(ctx, max_count, r, f, func(core::rights::get_kingdom));
    }
    
    static size_t each_de_jure_kingdom_logic(context* ctx, const size_t &max_count, utils::handle<core::realm> r, const std::function<double(context*, core::titulus*)> &f) {
      return each_de_jure_title_logic(ctx, max_count, r, f, func(core::rights::get_kingdom));
    }
    
    static object first_de_jure_kingdom(utils::handle<core::realm> r) {
      return first_de_jure_title(r, func(core::rights::get_kingdom));
    }
    
    static size_t count_de_jure_empires(utils::handle<core::realm> r) {
      return count_de_jure_titles(r, func(core::rights::get_empire));
    }
    
    static double each_de_jure_empire(context* ctx, const size_t &max_count, utils::handle<core::realm> r, const std::function<double(context*, core::titulus*)> &f) {
      return each_de_jure_title(ctx, max_count, r, f, func(core::rights::get_empire));
    }
    
    static size_t each_de_jure_empire_logic(context* ctx, const size_t &max_count, utils::handle<core::realm> r, const std::function<double(context*, core::titulus*)> &f) {
      return each_de_jure_title_logic(ctx, max_count, r, f, func(core::rights::get_empire));
    }
    
    static object first_de_jure_empire(utils::handle<core::realm> r) {
      return first_de_jure_title(r, func(core::rights::get_empire));
    }
    
    static size_t count_neighboring_top_lieges(utils::handle<core::realm> r) { UNUSED_VARIABLE(r); assert(false); return 0; }
    static double each_neighboring_top_liege(context* ctx, const size_t &max_count, utils::handle<core::realm> r, const std::function<double(context*, utils::handle<core::realm>)> &f) { 
      UNUSED_VARIABLE(r); UNUSED_VARIABLE(ctx); UNUSED_VARIABLE(max_count); UNUSED_VARIABLE(f); assert(false); return 0;
    }
    static size_t each_neighboring_top_liege_logic(context* ctx, const size_t &max_count, utils::handle<core::realm> r, const std::function<double(context*, utils::handle<core::realm>)> &f) { 
      UNUSED_VARIABLE(r); UNUSED_VARIABLE(ctx); UNUSED_VARIABLE(max_count); UNUSED_VARIABLE(f); assert(false); return 0;
    }
    static object first_neighboring_top_liege(utils::handle<core::realm> r) { UNUSED_VARIABLE(r); assert(false); return object(); }
    static size_t count_neighboring_same_ranks(utils::handle<core::realm> r) { UNUSED_VARIABLE(r); assert(false); return 0; }
    static double each_neighboring_same_rank(context* ctx, const size_t &max_count, utils::handle<core::realm> r, const std::function<double(context*, utils::handle<core::realm>)> &f) { 
      UNUSED_VARIABLE(r); UNUSED_VARIABLE(ctx); UNUSED_VARIABLE(max_count); UNUSED_VARIABLE(f); assert(false); return 0;
    }
    static size_t each_neighboring_same_rank_logic(context* ctx, const size_t &max_count, utils::handle<core::realm> r, const std::function<double(context*, utils::handle<core::realm>)> &f) { 
      UNUSED_VARIABLE(r); UNUSED_VARIABLE(ctx); UNUSED_VARIABLE(max_count); UNUSED_VARIABLE(f); assert(false); return 0;
    }
    static object first_neighboring_same_rank(utils::handle<core::realm> r) { UNUSED_VARIABLE(r); assert(false); return object(); }
    
    static size_t count_armys(utils::handle<core::realm> r) { UNUSED_VARIABLE(r); assert(false); return 0; }
    static double each_army(context* ctx, const size_t &max_count, utils::handle<core::realm> r, const std::function<double(context*, utils::handle<core::army>)> &f) { 
      UNUSED_VARIABLE(r); UNUSED_VARIABLE(ctx); UNUSED_VARIABLE(max_count); UNUSED_VARIABLE(f); assert(false); return 0;
    }
    static size_t each_army_logic(context* ctx, const size_t &max_count, utils::handle<core::realm> r, const std::function<double(context*, utils::handle<core::army>)> &f) { 
      UNUSED_VARIABLE(r); UNUSED_VARIABLE(ctx); UNUSED_VARIABLE(max_count); UNUSED_VARIABLE(f); assert(false); return 0;
    }
    static object first_army(utils::handle<core::realm> r) { UNUSED_VARIABLE(r); assert(false); return object(); }
    static size_t count_election_candidates(utils::handle<core::realm> r) { UNUSED_VARIABLE(r); assert(false); return 0; }
    static double each_election_candidate(context* ctx, const size_t &max_count, utils::handle<core::realm> r, const std::function<double(context*, core::character*)> &f) { 
      UNUSED_VARIABLE(r); UNUSED_VARIABLE(ctx); UNUSED_VARIABLE(max_count); UNUSED_VARIABLE(f); assert(false); return 0;
    }
    static size_t each_election_candidate_logic(context* ctx, const size_t &max_count, utils::handle<core::realm> r, const std::function<double(context*, core::character*)> &f) { 
      UNUSED_VARIABLE(r); UNUSED_VARIABLE(ctx); UNUSED_VARIABLE(max_count); UNUSED_VARIABLE(f); assert(false); return 0;
    }
    static object first_election_candidate(utils::handle<core::realm> r) { UNUSED_VARIABLE(r); assert(false); return object(); }
    
    static size_t count_attackers(utils::handle<core::war> w) {
      return w->attackers.size();
    }
    
    static double each_attacker(context* ctx, const size_t &max_count, utils::handle<core::war> w, const std::function<double(context*, core::character*)> &f) {
      double counter = 0.0;
//       counter += f(ctx, w->opener_realm);
      for (const auto &realms : w->attackers) {
        if (counter >= max_count) break;
        counter += f(ctx, realms);
      }
      
      return counter;
    }
    
    static size_t each_attacker_logic(context* ctx, const size_t &, utils::handle<core::war> w, const std::function<double(context*, core::character*)> &f) {
      bool counter = true;
//       counter += f(ctx, w->opener_realm); // я еще и неправильно делаю
      for (const auto &realms : w->attackers) {
        const double ret = f(ctx, realms);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      
      return counter;
    }
    
    static object first_attacker(utils::handle<core::war> w) {
      return object(w->war_opener);
    }
    
    static size_t count_defenders(utils::handle<core::war> w) {
      return w->defenders.size();
    }
    
    static double each_defender(context* ctx, const size_t &max_count, utils::handle<core::war> w, const std::function<double(context*, core::character*)> &f) {
      double counter = 0.0;
//       counter += f(ctx, w->target_realm);
      for (const auto &realms : w->defenders) {
        if (counter >= max_count) break;
        counter += f(ctx, realms);
      }
      
      return counter;
    }
    
    static size_t each_defender_logic(context* ctx, const size_t &, utils::handle<core::war> w, const std::function<double(context*, core::character*)> &f) {
      bool counter = true;
//       counter += f(ctx, w->target_realm); // я еще и неправильно делаю
      for (const auto &realms : w->defenders) {
        const double ret = f(ctx, realms);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      
      return counter;
    }
    
    static object first_defender(utils::handle<core::war> w) {
      return object(w->target_character);
    }
    
    static size_t count_participants(utils::handle<core::war> w) {
      return count_attackers(w) + count_defenders(w);
    }
    
    static double each_participant(context* ctx, const size_t &max_count, utils::handle<core::war> w, const std::function<double(context*, core::character*)> &f) {
      return each_attacker(ctx, max_count, w, f) + each_defender(ctx, max_count, w, f);
    }
    
    static size_t each_participant_logic(context* ctx, const size_t &, utils::handle<core::war> w, const std::function<double(context*, core::character*)> &f) {
      return bool(each_attacker_logic(ctx, SIZE_MAX, w, f)) && bool(each_defender_logic(ctx, SIZE_MAX, w, f));
    }
    
    static object first_participant(utils::handle<core::war> w) {
      return first_attacker(w);
    }
    
    static size_t count_target_titles(utils::handle<core::war> w) {
      return w->target_titles.size();
    }
    
    static double each_target_title(context* ctx, const size_t &max_count, utils::handle<core::war> w, const std::function<double(context*, core::titulus*)> &f) {
      double counter = 0.0;
      for (const auto &title : w->target_titles) {
        if (counter >= max_count) break;
        counter += f(ctx, title);
      }
      return counter;
    }
    
    static size_t each_target_title_logic(context* ctx, const size_t &, utils::handle<core::war> w, const std::function<double(context*, core::titulus*)> &f) {
      bool counter = true;
      for (const auto &title : w->target_titles) {
        const double ret = f(ctx, title);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      return counter;
    }
    
    static object first_target_title(utils::handle<core::war> w) {
      ASSERT(!w->target_titles.empty());
      return object(w->target_titles[0]);
    }
    
    static size_t count_child_religions(core::religion* r) {
      return utils::ring::list_count<utils::list_type::faiths>(r->children);
    }
    
    static double each_child_religion(context* ctx, const size_t &max_count, core::religion* r, const std::function<double(context*, core::religion*)> &f) {
      double counter = 0.0;
      for (auto faith = r->children; faith != nullptr && counter < max_count; faith = utils::ring::list_next<utils::list_type::faiths>(faith, r->children)) {
        counter += f(ctx, faith);
      }
      return counter;
    }
    
    static size_t each_child_religion_logic(context* ctx, const size_t &, core::religion* r, const std::function<double(context*, core::religion*)> &f) {
      bool counter = true;
      for (auto faith = r->children; faith != nullptr; faith = utils::ring::list_next<utils::list_type::faiths>(faith, r->children)) {
        const double ret = f(ctx, faith);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      return counter;
    }
    
    static object first_child_religion(core::religion* r) {
      return object(r->children);
    }
    
    static size_t count_sibling_religions(core::religion* r) {
      return utils::ring::list_count<utils::list_type::faiths>(r)-1;
    }
    
    static double each_sibling_religion(context* ctx, const size_t &max_count, core::religion* r, const std::function<double(context*, core::religion*)> &f) {
      double counter = 0.0;
      auto faith = utils::ring::list_next<utils::list_type::faiths>(r, r);
      for (; faith != nullptr && counter < max_count; faith = utils::ring::list_next<utils::list_type::faiths>(faith, r)) {
        counter += f(ctx, faith);
      }
      return counter;
    }
    
    static size_t each_sibling_religion_logic(context* ctx, const size_t &, core::religion* r, const std::function<double(context*, core::religion*)> &f) {
      bool counter = true;
      auto faith = utils::ring::list_next<utils::list_type::faiths>(r, r);
      for (; faith != nullptr; faith = utils::ring::list_next<utils::list_type::faiths>(faith, r)) {
        const double ret = f(ctx, faith);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      return counter;
    }
    
    static object first_sibling_religion(core::religion* r) {
      return object(utils::ring::list_next<utils::list_type::faiths>(r, r));
    }
    
    static size_t count_believers(core::religion* r) {
      return utils::ring::list_count<utils::list_type::believer>(r->believers);
    }
    
    static double each_believer(context* ctx, const size_t &max_count, core::religion* r, const std::function<double(context*, core::character*)> &f) {
      double counter = 0.0;
      for (auto faith = r->believers; faith != nullptr && counter < max_count; faith = utils::ring::list_next<utils::list_type::believer>(faith, r->believers)) {
        counter += f(ctx, faith);
      }
      return counter;
    }
    
    static size_t each_believer_logic(context* ctx, const size_t &, core::religion* r, const std::function<double(context*, core::character*)> &f) {
      bool counter = true;
      for (auto faith = r->believers; faith != nullptr; faith = utils::ring::list_next<utils::list_type::believer>(faith, r->believers)) {
        const double ret = f(ctx, faith);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      return counter;
    }
    
    static object first_believer(core::religion* r) {
      return object(r->believers);
    }
    
    static size_t count_secret_believers(core::religion* r) {
      return utils::ring::list_count<utils::list_type::secret_believer>(r->secret_believers);
    }
    
    static double each_secret_believer(context* ctx, const size_t &max_count, core::religion* r, const std::function<double(context*, core::character*)> &f) {
      double counter = 0.0;
      for (auto faith = r->secret_believers; faith != nullptr && counter < max_count; faith = utils::ring::list_next<utils::list_type::secret_believer>(faith, r->secret_believers)) {
        counter += f(ctx, faith);
      }
      return counter;
    }
    
    static size_t each_secret_believer_logic(context* ctx, const size_t &, core::religion* r, const std::function<double(context*, core::character*)> &f) {
      bool counter = true;
      for (auto faith = r->secret_believers; faith != nullptr; faith = utils::ring::list_next<utils::list_type::secret_believer>(faith, r->secret_believers)) {
        const double ret = f(ctx, faith);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      return counter;
    }
    
    static object first_secret_believer(core::religion* r) {
      return object(r->secret_believers);
    }
    
    static size_t count_child_cultures(core::culture* c) {
      return utils::ring::list_count<utils::list_type::sibling_cultures>(c->children);
    }
    
    static double each_child_culture(context* ctx, const size_t &max_count, core::culture* c, const std::function<double(context*, core::culture*)> &f) {
      double counter = 0.0;
      for (auto child = c->children; child != nullptr && counter < max_count; child = utils::ring::list_next<utils::list_type::sibling_cultures>(child, c->children)) {
        counter += f(ctx, child);
      }
      return counter;
    }
    
    static size_t each_child_culture_logic(context* ctx, const size_t &, core::culture* c, const std::function<double(context*, core::culture*)> &f) {
      bool counter = true;
      for (auto child = c->children; child != nullptr; child = utils::ring::list_next<utils::list_type::sibling_cultures>(child, c->children)) {
        const double ret = f(ctx, child);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      return counter;
    }
    
    static object first_child_culture(core::culture* c) {
      return object(c->children);
    }
    
    static size_t count_sibling_cultures(core::culture* c) {
      return utils::ring::list_count<utils::list_type::sibling_cultures>(c)-1;
    }
    
    static double each_sibling_culture(context* ctx, const size_t &max_count, core::culture* c, const std::function<double(context*, core::culture*)> &f) {
      double counter = 0.0;
      auto child = utils::ring::list_next<utils::list_type::sibling_cultures>(c, c);
      for (; child != nullptr && counter < max_count; child = utils::ring::list_next<utils::list_type::sibling_cultures>(child, c)) {
        counter += f(ctx, child);
      }
      return counter;
    }
    
    // мы берем саму культуру? не думаю что это хорошая идея
    static size_t each_sibling_culture_logic(context* ctx, const size_t &, core::culture* c, const std::function<double(context*, core::culture*)> &f) {
      bool counter = true;
      auto child = utils::ring::list_next<utils::list_type::sibling_cultures>(c, c);
      for (; child != nullptr; child = utils::ring::list_next<utils::list_type::sibling_cultures>(child, c)) {
        const double ret = f(ctx, child);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      return counter;
    }
    
    static object first_sibling_culture(core::culture* c) {
      return object(utils::ring::list_next<utils::list_type::sibling_cultures>(c, c));
    }
    
    static size_t count_culture_members(core::culture* c) {
      return utils::ring::list_count<utils::list_type::culture_member>(c->members);
    }
    
    static double each_culture_member(context* ctx, const size_t &max_count, core::culture* r, const std::function<double(context*, core::character*)> &f) {
      double counter = 0.0;
      for (auto m = r->members; m != nullptr && counter < max_count; m = utils::ring::list_next<utils::list_type::culture_member>(m, r->members)) {
        counter += f(ctx, m);
      }
      return counter;
    }
    
    static size_t each_culture_member_logic(context* ctx, const size_t &, core::culture* r, const std::function<double(context*, core::character*)> &f) {
      bool counter = true;
      for (auto m = r->members; m != nullptr; m = utils::ring::list_next<utils::list_type::culture_member>(m, r->members)) {
        const double ret = f(ctx, m);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      return counter;
    }
    
    static object first_culture_member(core::culture* r) {
      return object(r->members);
    }
    
    static size_t count_local_citys(core::province* p) {
      return utils::ring::list_count<utils::list_type::province_cities>(p->cities);
    }
    
    static double each_local_city(context* ctx, const size_t &max_count, core::province* p, const std::function<double(context*, core::city*)> &f) {
      double counter = 0.0;
      for (auto c = p->cities; c != nullptr && counter < max_count; c = utils::ring::list_next<utils::list_type::province_cities>(c, p->cities)) {
        counter += f(ctx, c);
      }
      return counter;
    }
    
    static size_t each_local_city_logic(context* ctx, const size_t &, core::province* p, const std::function<double(context*, core::city*)> &f) {
      bool counter = true;
      for (auto c = p->cities; c != nullptr; c = utils::ring::list_next<utils::list_type::province_cities>(c, p->cities)) {
        const double ret = f(ctx, c);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      return counter;
    }
    
    static object first_local_city(core::province* p) {
      return object(p->cities);
    }
    
    static size_t count_neighbours(core::province* p) {
      return p->neighbours.size();
    }
    
    static double each_neighbour(context* ctx, const size_t &max_count, core::province* p, const std::function<double(context*, core::province*)> &f) {
      auto core_ctx = global::get<systems::map_t>()->core_context;
      double counter = 0.0;
      for (const uint32_t index : p->neighbours) {
        if (counter >= max_count) break;
        auto prov = core_ctx->get_entity<core::province>(index);
        counter += f(ctx, prov);
      }
      return counter;
    }
    
    static size_t each_neighbour_logic(context* ctx, const size_t &, core::province* p, const std::function<double(context*, core::province*)> &f) {
      auto core_ctx = global::get<systems::map_t>()->core_context;
      bool counter = true;
      for (const uint32_t index : p->neighbours) {
        auto prov = core_ctx->get_entity<core::province>(index);
        const double ret = f(ctx, prov);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      return counter;
    }
    
    static object first_neighbour(core::province* p) {
      if (p->neighbours.empty()) return object();
      auto ctx = global::get<systems::map_t>()->core_context;
      const uint32_t index = p->neighbours[0];
      return object(ctx->get_entity<core::province>(index));
    }
    
    static size_t count_sibling_titles(core::titulus* t) {
      return utils::ring::list_count<utils::list_type::sibling_titles>(t)-1;
    }
    
    static double each_sibling_title(context* ctx, const size_t &max_count, core::titulus* t, const std::function<double(context*, core::titulus*)> &f) {
      double counter = 0.0;
      auto s = utils::ring::list_next<utils::list_type::sibling_titles>(t, t);
      for (; s != nullptr && counter < max_count; s = utils::ring::list_next<utils::list_type::sibling_titles>(s, t)) {
        counter += f(ctx, s);
      }
      return counter;
    }
    
    static size_t each_sibling_title_logic(context* ctx, const size_t &, core::titulus* t, const std::function<double(context*, core::titulus*)> &f) {
      bool counter = true;
      auto s = utils::ring::list_next<utils::list_type::sibling_titles>(t, t);
      for (; s != nullptr; s = utils::ring::list_next<utils::list_type::sibling_titles>(s, t)) {
        const double ret = f(ctx, s);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      return counter;
    }
    
    static object first_sibling_title(core::titulus* t) {
      return object(utils::ring::list_next<utils::list_type::sibling_titles>(t, t));
    }
    
    static size_t count_child_titles(core::titulus* t) {
      return utils::ring::list_count<utils::list_type::sibling_titles>(t->children);
    }
    
    static double each_child_title(context* ctx, const size_t &max_count, core::titulus* t, const std::function<double(context*, core::titulus*)> &f) {
      double counter = 0.0;
      for (auto s = t->children; s != nullptr && counter < max_count; s = utils::ring::list_next<utils::list_type::sibling_titles>(s, t->children)) {
        counter += f(ctx, s);
      }
      return counter;
    }
    
    static size_t each_child_title_logic(context* ctx, const size_t &, core::titulus* t, const std::function<double(context*, core::titulus*)> &f) {
      bool counter = true;
      for (auto s = t->children; s != nullptr; s = utils::ring::list_next<utils::list_type::sibling_titles>(s, t->children)) {
        const double ret = f(ctx, s);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      return counter;
    }
    
    static object first_child_title(core::titulus* t) {
      return object(t->children);
    }
    
    static size_t count_in_hierarchys(core::titulus* t) {
      size_t counter = 0;
      for (auto s = t->children; s != nullptr; s = utils::ring::list_next<utils::list_type::sibling_titles>(s, t->children)) {
        counter += 1;
        counter += count_in_hierarchys(s);
      }
      
      return counter;
    }
    
    static double each_in_hierarchy(context* ctx, const size_t &max_count, core::titulus* t, const std::function<double(context*, core::titulus*)> &f) {
      double counter = 0.0;
      // что тут делать с max_count? это значение задается только в has_*, скорее всего нужно просто ограничить этим числом
      for (auto s = t->children; s != nullptr && counter < max_count; s = utils::ring::list_next<utils::list_type::sibling_titles>(s, t->children)) {
        counter += f(ctx, s);
        counter += each_in_hierarchy(ctx, max_count, s, f);
      }
      return std::min(counter, double(max_count));
    }
    
    static size_t each_in_hierarchy_logic(context* ctx, const size_t &, core::titulus* t, const std::function<double(context*, core::titulus*)> &f) {
      bool counter = true;
      for (auto s = t->children; s != nullptr; s = utils::ring::list_next<utils::list_type::sibling_titles>(s, t->children)) {
        const double ret = f(ctx, s);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
        counter = counter && bool(each_in_hierarchy(ctx, SIZE_MAX, s, f));
      }
      return counter;
    }
    
    static object first_in_hierarchy(core::titulus* t) {
      return object(t->children);
    }
    
    // по идее in_de_jure_hierarchy повторяет child_title, нет, тут мы обходим титулы которыми владеют юридические вассалы
    static size_t count_in_de_jure_hierarchys(core::titulus* t) {
      if (!t->owner.valid()) return 0;
      size_t counter = 0;
      for (auto v = t->owner->vassals; v != nullptr; v = utils::ring::list_next<utils::list_type::vassals>(v, t->owner->vassals)) {
        auto m = v->main_title;
        const bool parent = core::rights::is_parent_or_above(t, m);
        counter += parent;
        if (parent) counter += count_in_de_jure_hierarchys(m);
      }
      
      return counter;
    }
    
    static double each_in_de_jure_hierarchy(context* ctx, const size_t &max_count, core::titulus* t, const std::function<double(context*, core::titulus*)> &f) {
      if (!t->owner.valid()) return 0.0;
      double counter = 0.0;
      for (auto v = t->owner->vassals; v != nullptr && counter < max_count; v = utils::ring::list_next<utils::list_type::vassals>(v, t->owner->vassals)) {
        const bool parent = core::rights::is_parent_or_above(t, v->main_title);
        if (!parent) continue;
        counter += f(ctx, v->main_title);
        counter += each_in_de_jure_hierarchy(ctx, max_count, v->main_title, f);
      }
      
      return std::min(counter, double(max_count));
    }
    
    static size_t each_in_de_jure_hierarchy_logic(context* ctx, const size_t &, core::titulus* t, const std::function<double(context*, core::titulus*)> &f) {
      if (!t->owner.valid()) return true;
      bool counter = true;
      for (auto v = t->owner->vassals; v != nullptr; v = utils::ring::list_next<utils::list_type::vassals>(v, t->owner->vassals)) {
        const bool parent = core::rights::is_parent_or_above(t, v->main_title);
        if (!parent) continue;
        const double ret = f(ctx, v->main_title);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
        counter = counter && bool(each_in_de_jure_hierarchy(ctx, SIZE_MAX, v->main_title, f));
      }
      
      return counter;
    }
    
    static object first_in_de_jure_hierarchy(core::titulus* t) {
      if (!t->owner.valid()) return object();
      if (t->owner->vassals == nullptr) return object();
      for (auto v = t->owner->vassals; v != nullptr; v = utils::ring::list_next<utils::list_type::vassals>(v, t->owner->vassals)) {
        const bool parent = core::rights::is_parent_or_above(t, v->main_title);
        if (parent) return object(v->main_title);
      }
      
      return object();
    }
    
    static size_t count_liege_title(const core::realm* owner, const core::titulus* t) {
      size_t counter = 0;
      for (auto owned = t->children; owned != nullptr; owned = utils::ring::list_next<utils::list_type::sibling_titles>(owned, t->children)) {
        if (!owned->owner.valid()) continue;
        const bool is_liege = core::rights::is_liege_or_above(owned->owner.get(), owner);
        counter += is_liege;
        if (is_liege) counter += count_liege_title(owner, owned);
      }
      
      return counter;
    }
    
    static void each_liege_title(context* ctx, const core::realm* owner, const core::titulus* t, const std::function<void(context*, core::titulus*)> &f) {
      for (auto owned = t->children; owned != nullptr; owned = utils::ring::list_next<utils::list_type::sibling_titles>(owned, t->children)) {
        if (!owned->owner.valid()) continue;
        const bool is_liege = core::rights::is_liege_or_above(owned->owner.get(), owner);
        if (!is_liege) continue;
        f(ctx, owned);
        each_liege_title(ctx, owner, owned, f);
      }
    }
    
    // иерархия по факту - это все титулы ниже рангом у вассалов текущего владельца титула (ну и у самого владельца)
    // короч нужно получше понять че происходит, но я так понимаю нужно брать просто владельца титула и у него вассалов обходить
    // брать титул и обходить другие титулы если их владельцы вассалы владельца текущего?
    // нет, я тогда пропущу остальных вассалов... тут основная проблема в том что вассалы хранятся не в титулах а в реалмах
    // пройти иерархию и взять только фактических вассалов? если мы берем короля и у него находим герцога, то это сработает наверное
    // а если мы берем только герцогский титул у короля, то в этом случае сложно что то выдавить из себя, нужно тогда 
    // если титул совпадает по рангу с основным то обходим всех вассалов и их титулы, если нет, то обходим только тех вассалов которые 
    // деюре в составе титула верхнего уровня, сам титул наверное обходить не имеет смысла
    // в цк3 используется в связке с primary_title, не очень понятно как работает в других связках
    // у меня возможно будет работаеть в связке get_state + primary_title + *in_de_facto_hierarchy
    static size_t count_in_de_facto_hierarchys(core::titulus* t) {
      if (!t->owner.valid()) return 0;
      size_t counter = 0;
      const bool same_rank_title = t->type() == t->owner->main_title->type();
      assert(same_rank_title != (t->type() < t->owner->main_title->type()));
      const auto cur_type = t->type();
      // может ли совет владеть каким то титулом? мне представляется что может, например если оно отбирает у преступника, совет отбирает в пользу кого?
      const auto owner = t->owner;
      // титулы государства или титулы все таки овнера? лучше наверное именно овнера, какие титулы? более низкого уровня, одного уровня титулы мы можем пройти отдельно
      for (auto owned = owner->titles; owned != nullptr; owned = utils::ring::list_next<utils::list_type::titles>(owned, owner->titles)) {
        counter += size_t(cur_type > owned->type());
      }
      
      if (same_rank_title) {
        // вассалы государства и у них титулы
        for (auto owned = owner->vassals; owned != nullptr; owned = utils::ring::list_next<utils::list_type::vassals>(owned, owner->vassals)) {
          auto m = owned->main_title;
          counter += count_in_de_facto_hierarchys(m);
        }
      } else {
        // взят титул более низкого ранга
        // нужно пройтись по его иерархии и посмотреть кто есть
        counter += count_liege_title(owner.get(), t);
      }
      
      return counter;
    }
    
    static double each_in_de_facto_hierarchy(context* ctx, const size_t &max_count, core::titulus* t, const std::function<double(context*, core::titulus*)> &f) {
      if (!t->owner.valid()) return 0.0;
      double counter = 0.0;
      const bool same_rank_title = t->type() == t->owner->main_title->type();
      assert(same_rank_title != (t->type() < t->owner->main_title->type()));
      const auto cur_type = t->type();
      const auto owner = t->owner;
      
      for (auto owned = owner->titles; owned != nullptr && counter < max_count; owned = utils::ring::list_next<utils::list_type::titles>(owned, owner->titles)) {
        if (cur_type <= owned->type()) continue;
        counter += f(ctx, owned);
      }
      
      if (same_rank_title) {
        // вассалы государства и у них титулы
        for (auto owned = owner->vassals; owned != nullptr && counter < max_count; owned = utils::ring::list_next<utils::list_type::vassals>(owned, owner->vassals)) {
          auto m = owned->main_title;
          counter += f(ctx, m);
          counter += each_in_de_facto_hierarchy(ctx, max_count, m, f);
        }
      } else {
        // взят титул более низкого ранга
        // нужно пройтись по его иерархии и посмотреть кто есть
        each_liege_title(ctx, owner.get(), t, [&] (context* ctx, core::titulus* t) {
          if (counter >= max_count) return;
          counter += f(ctx, t);
        });
      }
      
      return std::min(counter, double(max_count));
    }
    
    static size_t each_in_de_facto_hierarchy_logic(context* ctx, const size_t &, core::titulus* t, const std::function<double(context*, core::titulus*)> &f) {
      if (!t->owner.valid()) return true;
      bool counter = true;
      const bool same_rank_title = t->type() == t->owner->main_title->type();
      assert(same_rank_title != (t->type() < t->owner->main_title->type()));
      const auto cur_type = t->type();
      const auto owner = t->owner;
      assert(owner.valid());
      for (auto owned = owner->titles; owned != nullptr; owned = utils::ring::list_next<utils::list_type::titles>(owned, owner->titles)) {
        if (cur_type <= owned->type()) continue;
        const double ret = f(ctx, t);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      
      if (same_rank_title) {
        for (auto owned = owner->vassals; owned != nullptr; owned = utils::ring::list_next<utils::list_type::vassals>(owned, owner->vassals)) {
          auto m = owned->main_title;
          const double ret = f(ctx, m);
          counter = counter && (std::isnan(ret) ? true : bool(ret));
          counter = counter && bool(each_in_de_facto_hierarchy_logic(ctx, SIZE_MAX, m, f));
        }
      } else {
        each_liege_title(ctx, owner.get(), t, [&] (context* ctx, core::titulus* t) {
          const double ret = f(ctx, t);
          counter = counter && (std::isnan(ret) ? true : bool(ret));
        });
      }
      
      return counter;
    }
    
    static object first_in_de_facto_hierarchy(core::titulus* t) {
      const auto cur_type = t->type();
      for (auto owned = t->owner->titles; owned != nullptr; owned = utils::ring::list_next<utils::list_type::titles>(owned, t->owner->titles)) {
        if (cur_type > owned->type()) return object(owned);
      }
      return object();
    }

using character_ptr_t = core::character*;
using title_ptr_t = core::titulus*;
using religion_ptr_t = core::religion*;
using culture_ptr_t = core::culture*;
using religion_group_ptr_t = core::religion_group*;
using culture_group_ptr_t = core::culture_group*;
using province_ptr_t = core::province*;
using city_ptr_t = core::city*;
using realm_handle_t = utils::handle<core::realm>;
using army_handle_t = utils::handle<core::army>;
using hero_troop_handle_t = utils::handle<core::hero_troop>;
using war_handle_t = utils::handle<core::war>;

CHANGE_SCOPE_FUNC_IMPLEMENTATION(ancestor,          character_ptr_t, character_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(sibling,           character_ptr_t, character_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(child,             character_ptr_t, character_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(brother,           character_ptr_t, character_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(sister,            character_ptr_t, character_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(concubine,         character_ptr_t, character_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(acquaintance,      character_ptr_t, character_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(good_acquaintance, character_ptr_t, character_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(bad_acquaintance,  character_ptr_t, character_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(pal,               character_ptr_t, character_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(foe,               character_ptr_t, character_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(sympathy,          character_ptr_t, character_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(dislike,           character_ptr_t, character_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(parent,            character_ptr_t, character_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(claim,             character_ptr_t, title_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(de_jure_claim,     character_ptr_t, title_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(heir_to_title,     character_ptr_t, title_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(election_realm,    character_ptr_t, realm_handle_t)

CHANGE_SCOPE_FUNC_IMPLEMENTATION(war,        realm_handle_t, war_handle_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(war_ally,   realm_handle_t, character_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(war_enemy,  realm_handle_t, character_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(ally,       realm_handle_t, realm_handle_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(truce_holder, realm_handle_t, realm_handle_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(truce_target, realm_handle_t, realm_handle_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(member,     realm_handle_t, character_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(elector,    realm_handle_t, character_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(prisoner,   realm_handle_t, character_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(courtier,   realm_handle_t, character_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(vassal,     realm_handle_t, realm_handle_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(vassal_or_below, realm_handle_t, realm_handle_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(liege_or_above,  realm_handle_t, realm_handle_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(owned_title,             realm_handle_t, title_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(realm_title,             realm_handle_t, title_ptr_t) // это по идее обход дефакто титулов, нужно ли нам отдельная функция для скоупа титула?
// сюда мы можем пихнуть обход деюре титулов
CHANGE_SCOPE_FUNC_IMPLEMENTATION(directly_owned_province, realm_handle_t, province_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(directly_owned_city,     realm_handle_t, city_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(realm_province,          realm_handle_t, province_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(realm_city,              realm_handle_t, city_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(de_jure_duchy,           realm_handle_t, title_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(de_jure_kingdom,         realm_handle_t, title_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(de_jure_empire,          realm_handle_t, title_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(neighboring_top_liege, realm_handle_t, realm_handle_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(neighboring_same_rank, realm_handle_t, realm_handle_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(army,                  realm_handle_t, army_handle_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(election_candidate,    realm_handle_t, character_ptr_t)

CHANGE_SCOPE_FUNC_IMPLEMENTATION(attacker,     war_handle_t, character_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(defender,     war_handle_t, character_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(participant,  war_handle_t, character_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(target_title, war_handle_t, title_ptr_t)

CHANGE_SCOPE_FUNC_IMPLEMENTATION(child_religion,   religion_ptr_t, religion_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(sibling_religion, religion_ptr_t, religion_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(believer,         religion_ptr_t, character_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(secret_believer,  religion_ptr_t, character_ptr_t)

CHANGE_SCOPE_FUNC_IMPLEMENTATION(child_culture,   culture_ptr_t, culture_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(sibling_culture, culture_ptr_t, culture_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(culture_member,  culture_ptr_t, character_ptr_t)

CHANGE_SCOPE_FUNC_IMPLEMENTATION(local_city, province_ptr_t, city_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(neighbour,  province_ptr_t, province_ptr_t)

CHANGE_SCOPE_FUNC_IMPLEMENTATION(sibling_title,         title_ptr_t, title_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(child_title,           title_ptr_t, title_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(in_hierarchy,          title_ptr_t, title_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(in_de_facto_hierarchy, title_ptr_t, title_ptr_t)
CHANGE_SCOPE_FUNC_IMPLEMENTATION(in_de_jure_hierarchy,  title_ptr_t, title_ptr_t)

#undef HAS_ENTITY_FUNC
#undef HAS_ENTITY_DRAW_FUNC
#undef RANDOM_ENTITY_FUNC
#undef RANDOM_ENTITY_DRAW_FUNC
#undef EVERY_ENTITY_FUNC
#undef EVERY_ENTITY_DRAW_FUNC
#undef CHANGE_SCOPE_FUNC_IMPLEMENTATION

    static size_t count_city_globals(core::character* c) {
      assert(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      return core_ctx->get_entity_count<core::city>();
    }
    
    static double each_city_global(context* ctx, const size_t &max_count, core::character* c, const std::function<double(context*, core::city*)> &f) {
      assert(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      const size_t count = count_city_globals(nullptr);
      double counter = 0.0;
      for (size_t i = 0; i < count && counter < max_count; ++i) {
        auto ent = core_ctx->get_entity<core::city>(i);
        counter += f(ctx, ent);
      }
      
      return counter;
    }
    
    static size_t each_city_global_logic(context* ctx, const size_t &, core::character* c, const std::function<double(context*, core::city*)> &f) {
      assert(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      const size_t count = count_city_globals(nullptr);
      bool counter = true;
      for (size_t i = 0; i < count; ++i) {
        auto ent = core_ctx->get_entity<core::city>(i);
        const double ret = f(ctx, ent);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      
      return counter;
    }
    
    static object first_city_global(core::character* c) {
      assert(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      return core_ctx->get_entity<core::city>(0);
    }
    
    static size_t count_province_globals(core::character* c) {
      assert(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      return core_ctx->get_entity_count<core::province>();
    }
    
    static double each_province_global(context* ctx, const size_t &max_count, core::character* c, const std::function<double(context*, core::province*)> &f) {
      assert(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      const size_t count = count_city_globals(nullptr);
      double counter = 0.0;
      for (size_t i = 0; i < count && counter < max_count; ++i) {
        auto ent = core_ctx->get_entity<core::province>(i);
        counter += f(ctx, ent);
      }
      
      return counter;
    }
    
    static size_t each_province_global_logic(context* ctx, const size_t &, core::character* c, const std::function<double(context*, core::province*)> &f) {
      assert(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      const size_t count = count_city_globals(nullptr);
      bool counter = true;
      for (size_t i = 0; i < count; ++i) {
        auto ent = core_ctx->get_entity<core::province>(i);
        const double ret = f(ctx, ent);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      
      return counter;
    }
    
    static object first_province_global(core::character* c) {
      assert(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      return core_ctx->get_entity<core::province>(0);
    }
    
    static size_t count_barony_globals(core::character* c) {
      assert(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      const size_t count = core_ctx->get_entity_count<core::titulus>();
      size_t counter = 0;
      for (size_t i = 0; i < count; ++i) {
        auto t = core_ctx->get_entity<core::titulus>(i);
        counter += size_t(t->type() == core::titulus::type::baron);
      }
      return counter;
    }
    
    static double each_barony_global(context* ctx, const size_t &max_count, core::character* c, const std::function<double(context*, core::titulus*)> &f) {
      assert(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      const size_t count = core_ctx->get_entity_count<core::titulus>();
      double counter = 0.0;
      for (size_t i = 0; i < count && counter < max_count; ++i) {
        auto ent = core_ctx->get_entity<core::titulus>(i);
        if (ent->type() != core::titulus::type::baron) continue;
        counter += f(ctx, ent);
      }
      
      return counter;
    }
    
    static size_t each_barony_global_logic(context* ctx, const size_t &, core::character* c, const std::function<double(context*, core::titulus*)> &f) {
      assert(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      const size_t count = core_ctx->get_entity_count<core::titulus>();
      bool counter = true;
      for (size_t i = 0; i < count; ++i) {
        auto ent = core_ctx->get_entity<core::titulus>(i);
        if (ent->type() != core::titulus::type::baron) continue;
        const double ret = f(ctx, ent);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      
      return counter;
    }
    
    static object first_barony_global(core::character* c) {
      assert(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      const size_t count = core_ctx->get_entity_count<core::titulus>();
      for (size_t i = 0; i < count; ++i) {
        auto ent = core_ctx->get_entity<core::titulus>(i);
        if (ent->type() == core::titulus::type::baron) return object(ent);
      }
      
      return object();
    }
    
    static size_t count_duchy_globals(core::character* c) {
      assert(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      const size_t count = core_ctx->get_entity_count<core::titulus>();
      size_t counter = 0;
      for (size_t i = 0; i < count; ++i) {
        auto t = core_ctx->get_entity<core::titulus>(i);
        counter += size_t(t->type() == core::titulus::type::duke);
      }
      return counter;
    }
    
    static double each_duchy_global(context* ctx, const size_t &max_count, core::character* c, const std::function<double(context*, core::titulus*)> &f) {
      assert(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      const size_t count = core_ctx->get_entity_count<core::titulus>();
      double counter = 0.0;
      for (size_t i = 0; i < count && counter < max_count; ++i) {
        auto ent = core_ctx->get_entity<core::titulus>(i);
        if (ent->type() != core::titulus::type::duke) continue;
        counter += f(ctx, ent);
      }
      
      return counter;
    }
    
    static size_t each_duchy_global_logic(context* ctx, const size_t &, core::character* c, const std::function<double(context*, core::titulus*)> &f) {
      assert(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      const size_t count = core_ctx->get_entity_count<core::titulus>();
      bool counter = true;
      for (size_t i = 0; i < count; ++i) {
        auto ent = core_ctx->get_entity<core::titulus>(i);
        if (ent->type() != core::titulus::type::duke) continue;
        const double ret = f(ctx, ent);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      
      return counter;
    }
    
    static object first_duchy_global(core::character* c) {
      assert(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      const size_t count = core_ctx->get_entity_count<core::titulus>();
      for (size_t i = 0; i < count; ++i) {
        auto ent = core_ctx->get_entity<core::titulus>(i);
        if (ent->type() == core::titulus::type::duke) return object(ent);
      }
      
      return object();
    }
    
    static size_t count_kingdom_globals(core::character* c) {
      assert(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      const size_t count = core_ctx->get_entity_count<core::titulus>();
      size_t counter = 0;
      for (size_t i = 0; i < count; ++i) {
        auto t = core_ctx->get_entity<core::titulus>(i);
        counter += size_t(t->type() == core::titulus::type::king);
      }
      return counter;
    }
    
    static double each_kingdom_global(context* ctx, const size_t &max_count, core::character* c, const std::function<double(context*, core::titulus*)> &f) {
      assert(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      const size_t count = core_ctx->get_entity_count<core::titulus>();
      double counter = 0.0;
      for (size_t i = 0; i < count && counter < max_count; ++i) {
        auto ent = core_ctx->get_entity<core::titulus>(i);
        if (ent->type() != core::titulus::type::king) continue;
        counter += f(ctx, ent);
      }
      
      return counter;
    }
    
    static size_t each_kingdom_global_logic(context* ctx, const size_t &, core::character* c, const std::function<double(context*, core::titulus*)> &f) {
      assert(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      const size_t count = core_ctx->get_entity_count<core::titulus>();
      bool counter = true;
      for (size_t i = 0; i < count; ++i) {
        auto ent = core_ctx->get_entity<core::titulus>(i);
        if (ent->type() != core::titulus::type::king) continue;
        const double ret = f(ctx, ent);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      
      return counter;
    }
    
    static object first_kingdom_global(core::character* c) {
      assert(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      const size_t count = core_ctx->get_entity_count<core::titulus>();
      for (size_t i = 0; i < count; ++i) {
        auto ent = core_ctx->get_entity<core::titulus>(i);
        if (ent->type() == core::titulus::type::king) return object(ent);
      }
      
      return object();
    }
    
    static size_t count_empire_globals(core::character* c) {
      assert(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      const size_t count = core_ctx->get_entity_count<core::titulus>();
      size_t counter = 0;
      for (size_t i = 0; i < count; ++i) {
        auto t = core_ctx->get_entity<core::titulus>(i);
        counter += size_t(t->type() == core::titulus::type::imperial);
      }
      return counter;
    }
    
    static double each_empire_global(context* ctx, const size_t &max_count, core::character* c, const std::function<double(context*, core::titulus*)> &f) {
      assert(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      const size_t count = core_ctx->get_entity_count<core::titulus>();
      double counter = 0.0;
      for (size_t i = 0; i < count && counter < max_count; ++i) {
        auto ent = core_ctx->get_entity<core::titulus>(i);
        if (ent->type() != core::titulus::type::imperial) continue;
        counter += f(ctx, ent);
      }
      
      return counter;
    }
    
    static size_t each_empire_global_logic(context* ctx, const size_t &, core::character* c, const std::function<double(context*, core::titulus*)> &f) {
      assert(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      const size_t count = core_ctx->get_entity_count<core::titulus>();
      bool counter = true;
      for (size_t i = 0; i < count; ++i) {
        auto ent = core_ctx->get_entity<core::titulus>(i);
        if (ent->type() != core::titulus::type::imperial) continue;
        const double ret = f(ctx, ent);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      
      return counter;
    }
    
    static object first_empire_global(core::character* c) {
      ASSERT(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      const size_t count = core_ctx->get_entity_count<core::titulus>();
      for (size_t i = 0; i < count; ++i) {
        auto ent = core_ctx->get_entity<core::titulus>(i);
        if (ent->type() == core::titulus::type::imperial) return object(ent);
      }
      
      return object();
    }
    
    static size_t count_religion_group_globals(core::character* c) {
      ASSERT(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      return core_ctx->get_entity_count<core::religion_group>();
    }
    
    static double each_religion_group_global(context* ctx, const size_t &max_count, core::character* c, const std::function<double(context*, core::religion_group*)> &f) {
      ASSERT(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      const size_t count = count_religion_group_globals(nullptr);
      double counter = 0.0;
      for (size_t i = 0; i < count && counter < max_count; ++i) {
        auto ent = core_ctx->get_entity<core::religion_group>(i);
        counter += f(ctx, ent);
      }
      
      return counter;
    }
    
    static size_t each_religion_group_global_logic(context* ctx, const size_t &, core::character* c, const std::function<double(context*, core::religion_group*)> &f) {
      ASSERT(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      const size_t count = count_religion_group_globals(nullptr);
      bool counter = true;
      for (size_t i = 0; i < count; ++i) {
        auto ent = core_ctx->get_entity<core::religion_group>(i);
        const double ret = f(ctx, ent);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      
      return counter;
    }
    
    static object first_religion_group_global(core::character* c) {
      ASSERT(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      return object(core_ctx->get_entity<core::religion_group>(0));
    }
    
    static size_t count_religion_globals(core::character* c) {
      ASSERT(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      return core_ctx->get_entity_count<core::religion>();
    }
    
    static double each_religion_global(context* ctx, const size_t &max_count, core::character* c, const std::function<double(context*, core::religion*)> &f) {
      ASSERT(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      const size_t count = count_religion_globals(nullptr);
      double counter = 0.0;
      for (size_t i = 0; i < count && counter < max_count; ++i) {
        auto ent = core_ctx->get_entity<core::religion>(i);
        counter += f(ctx, ent);
      }
      
      return counter;
    }
    
    static size_t each_religion_global_logic(context* ctx, const size_t &, core::character* c, const std::function<double(context*, core::religion*)> &f) {
      ASSERT(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      const size_t count = count_religion_globals(nullptr);
      bool counter = true;
      for (size_t i = 0; i < count; ++i) {
        auto ent = core_ctx->get_entity<core::religion>(i);
        const double ret = f(ctx, ent);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      
      return counter;
    }
    
    static object first_religion_global(core::character* c) {
      ASSERT(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      return object(core_ctx->get_entity<core::religion>(0));
    }
    
    static size_t count_culture_group_globals(core::character* c) {
      ASSERT(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      return core_ctx->get_entity_count<core::culture_group>();
    }
    
    static double each_culture_group_global(context* ctx, const size_t &max_count, core::character* c, const std::function<double(context*, core::culture_group*)> &f) {
      ASSERT(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      const size_t count = count_culture_group_globals(nullptr);
      double counter = 0.0;
      for (size_t i = 0; i < count && counter < max_count; ++i) {
        auto ent = core_ctx->get_entity<core::culture_group>(i);
        counter += f(ctx, ent);
      }
      
      return counter;
    }
    
    static size_t each_culture_group_global_logic(context* ctx, const size_t &, core::character* c, const std::function<double(context*, core::culture_group*)> &f) {
      ASSERT(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      const size_t count = count_culture_group_globals(nullptr);
      bool counter = true;
      for (size_t i = 0; i < count; ++i) {
        auto ent = core_ctx->get_entity<core::culture_group>(i);
        const double ret = f(ctx, ent);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      
      return counter;
    }
    
    static object first_culture_group_global(core::character* c) {
      ASSERT(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      return object(core_ctx->get_entity<core::culture_group>(0));
    }
    
    static size_t count_culture_globals(core::character* c) {
      ASSERT(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      return core_ctx->get_entity_count<core::culture>();
    }
    
    static double each_culture_global(context* ctx, const size_t &max_count, core::character* c, const std::function<double(context*, core::culture*)> &f) {
      ASSERT(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      const size_t count = count_culture_globals(nullptr);
      double counter = 0.0;
      for (size_t i = 0; i < count && counter < max_count; ++i) {
        auto ent = core_ctx->get_entity<core::culture>(i);
        counter += f(ctx, ent);
      }
      
      return counter;
    }
    
    static size_t each_culture_global_logic(context* ctx, const size_t &, core::character* c, const std::function<double(context*, core::culture*)> &f) {
      ASSERT(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      const size_t count = count_culture_globals(nullptr);
      bool counter = true;
      for (size_t i = 0; i < count; ++i) {
        auto ent = core_ctx->get_entity<core::culture>(i);
        const double ret = f(ctx, ent);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      
      return counter;
    }
    
    static object first_culture_global(core::character* c) {
      ASSERT(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      return object(core_ctx->get_entity<core::culture>(0));
    }
    
    static size_t count_living_character_globals(core::character* c) {
      ASSERT(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      return core_ctx->living_characters_count();
    }
    
    static double each_living_character_global(context* ctx, const size_t &max_count, core::character* c, const std::function<double(context*, core::character*)> &f) {
      ASSERT(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      const size_t count = count_living_character_globals(nullptr);
      double counter = 0.0;
      for (size_t i = 0; i < count && counter < max_count; ++i) {
        auto ent = core_ctx->get_living_character(i);
        counter += f(ctx, ent);
      }
      
      return counter;
    }
    
    static size_t each_living_character_global_logic(context* ctx, const size_t &, core::character* c, const std::function<double(context*, core::character*)> &f) {
      ASSERT(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      const size_t count = count_religion_group_globals(nullptr);
      bool counter = true;
      for (size_t i = 0; i < count; ++i) {
        auto ent = core_ctx->get_living_character(i);
        const double ret = f(ctx, ent);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      
      return counter;
    }
    
    static object first_living_character_global(core::character* c) {
      ASSERT(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      return object(core_ctx->get_living_character(0));
    }
    
    static size_t count_ruler_globals(core::character* c) {
      ASSERT(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      return core_ctx->living_playable_characters_count();
    }
    
    static double each_ruler_global(context* ctx, const size_t &max_count, core::character* c, const std::function<double(context*, core::character*)> &f) {
      ASSERT(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      const size_t count = count_ruler_globals(nullptr);
      double counter = 0.0;
      for (size_t i = 0; i < count && counter < max_count; ++i) {
        auto ent = core_ctx->get_living_playable_character(i);
        counter += f(ctx, ent);
      }
      
      return counter;
    }
    
    static size_t each_ruler_global_logic(context* ctx, const size_t &, core::character* c, const std::function<double(context*, core::character*)> &f) {
      ASSERT(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      const size_t count = count_ruler_globals(nullptr);
      bool counter = true;
      for (size_t i = 0; i < count; ++i) {
        auto ent = core_ctx->get_living_playable_character(i);
        const double ret = f(ctx, ent);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      
      return counter;
    }
    
    static object first_ruler_global(core::character* c) {
      ASSERT(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      return object(core_ctx->get_living_playable_character(0));
    }
    
    static size_t count_independent_ruler_globals(core::character* c) {
      ASSERT(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      size_t counter = 0;
      for (size_t i = 0; i < core_ctx->living_playable_characters_count(); ++i) {
        auto ent = core_ctx->get_living_playable_character(i);
        counter += size_t(ent->is_independent());
      }
      return counter;
    }
    
    static double each_independent_ruler_global(context* ctx, const size_t &max_count, core::character* c, const std::function<double(context*, core::character*)> &f) {
      ASSERT(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      const size_t count = core_ctx->living_playable_characters_count();
      double counter = 0.0;
      for (size_t i = 0; i < count && counter < max_count; ++i) {
        auto ent = core_ctx->get_living_playable_character(i);
        if (!ent->is_independent()) continue;
        counter += f(ctx, ent);
      }
      
      return counter;
    }
    
    static size_t each_independent_ruler_global_logic(context* ctx, const size_t &, core::character* c, const std::function<double(context*, core::character*)> &f) {
      ASSERT(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      const size_t count = core_ctx->living_playable_characters_count();
      bool counter = true;
      for (size_t i = 0; i < count; ++i) {
        auto ent = core_ctx->get_living_playable_character(i);
        if (!ent->is_independent()) continue;
        const double ret = f(ctx, ent);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      
      return counter;
    }
    
    static object first_independent_ruler_global(core::character* c) {
      ASSERT(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      const size_t count = core_ctx->living_playable_characters_count();
      for (size_t i = 0; i < count; ++i) {
        auto ent = core_ctx->get_living_playable_character(i);
        if (ent->is_independent()) return object(ent);
      }
      return object();
    }
    
    // эта информация должна быть в другом месте, где? нужен интерфейс мультиплеера
    static size_t count_player_globals(core::character* c) {
      ASSERT(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      size_t counter = 0;
      for (size_t i = 0; i < core_ctx->living_playable_characters_count(); ++i) {
        auto ent = core_ctx->get_living_playable_character(i);
        counter += size_t(ent->is_player());
      }
      return counter;
    }
    
    static double each_player_global(context* ctx, const size_t &max_count, core::character* c, const std::function<double(context*, core::character*)> &f) {
      ASSERT(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      const size_t count = core_ctx->living_playable_characters_count();
      double counter = 0.0;
      for (size_t i = 0; i < count && counter < max_count; ++i) {
        auto ent = core_ctx->get_living_playable_character(i);
        if (!ent->is_player()) continue;
        counter += f(ctx, ent);
      }
      
      return counter;
    }
    
    static size_t each_player_global_logic(context* ctx, const size_t &, core::character* c, const std::function<double(context*, core::character*)> &f) {
      ASSERT(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      const size_t count = core_ctx->living_playable_characters_count();
      bool counter = true;
      for (size_t i = 0; i < count; ++i) {
        auto ent = core_ctx->get_living_playable_character(i);
        if (!ent->is_player()) continue;
        const double ret = f(ctx, ent);
        counter = counter && (std::isnan(ret) ? true : bool(ret));
      }
      
      return counter;
    }
    
    static object first_player_global(core::character* c) {
      ASSERT(c == nullptr);
      auto core_ctx = global::get<systems::map_t>()->core_context;
      const size_t count = core_ctx->living_playable_characters_count();
      for (size_t i = 0; i < count; ++i) {
        auto ent = core_ctx->get_living_playable_character(i);
        if (ent->is_player()) return object(ent);
      }
      return object();
    }

#define HAS_ENTITY_FUNC(name, count_func, each_func, current_type, next_type) \
    struct object has_##name::process(context* ctx) const {                   \
      return has_entity_func<next_type>(ctx, current_type(nullptr), percentage, max_count, childs, count_func, each_func); \
    }
    
#define HAS_ENTITY_DRAW_FUNC(name, first_obj, current_type)  \
    void has_##name::draw(context* ctx) const {              \
      const object value = process(ctx);                     \
      has_entity_draw_func(ctx, current_type(nullptr), value, percentage, max_count, childs, type_index, first_obj); \
    }
    
#define RANDOM_ENTITY_FUNC(name, each_func, current_type, next_type)                      \
    struct object random_##name::process(context* ctx) const {   \
      const auto obj = get_random_object<next_type>(ctx, state, current_type(nullptr), condition, weight, each_func); \
      if (!obj.valid()) return object();                         \
                                                                 \
      change_scope cs(ctx, obj, ctx->current);                   \
      for (auto cur = childs; cur != nullptr; cur = cur->next) { \
        cur->process(ctx);                                       \
      }                                                          \
                                                                 \
      return object();                                           \
    }
    
#define RANDOM_ENTITY_DRAW_FUNC(name, each_func, first_obj, current_type, next_type)   \
    void random_##name::draw(context* ctx) const {             \
      auto obj = get_random_object<next_type>(ctx, state, current_type(nullptr), condition, weight, each_func); \
      if (!obj.valid()) {                                      \
        obj = object(first_obj(current_type(nullptr)));    \
      }                                                        \
                                                               \
      if (!obj.valid()) return;                                \
      draw_data dd(ctx);                                       \
      dd.function_name = commands::names[type_index];          \
      dd.value = obj;                                          \
      if (!ctx->draw(&dd)) return;                             \
                                                               \
      change_scope cs(ctx, obj, ctx->current);                 \
      change_nesting cn(ctx, ++ctx->nest_level);               \
      change_function_name cfn(ctx, commands::names[type_index]); \
      for (auto cur = childs; cur != nullptr; cur = cur->next) {  \
        cur->draw(ctx);                                        \
      }                                                        \
    }
    
// тут я могу добавить вполне версии для эффекта, для кондишона, для нумерика
// нужно ли для кондишена? вообще может быть полезно
#define EVERY_ENTITY_FUNC(name, each_func, each_logic_func, current_type, next_type) \
    struct object every_##name::process(context* ctx) const { \
      switch (type) {                                         \
        case EVERY_FUNC_EFFECT:  return every_entity_func<next_type>(ctx, current_type(nullptr), condition, childs, each_func);             \
        case EVERY_FUNC_NUMERIC: return every_entity_numeric_func<next_type>(ctx, current_type(nullptr), condition, childs, each_func);     \
        case EVERY_FUNC_LOGIC:   return every_entity_logic_func<next_type>(ctx, current_type(nullptr), condition, childs, each_logic_func); \
        default: throw std::runtime_error("Bad type value");  \
      }                                                       \
      return object();                                        \
    }                                                         \
    
#define EVERY_ENTITY_DRAW_FUNC(name, first_obj, current_type) \
    void every_##name::draw(context* ctx) const {             \
      every_entity_draw_func(ctx, character_ptr_t(nullptr), condition, childs, type_index, first_obj); \
    }
    
#define CHANGE_SCOPE_FUNC_IMPLEMENTATION(name, current_type, next_type)              \
  HAS_ENTITY_FUNC(name, count_##name##s, each_##name, current_type, next_type)       \
  HAS_ENTITY_DRAW_FUNC(name, first_##name, current_type)                             \
  RANDOM_ENTITY_FUNC(name, each_##name, current_type, next_type)                     \
  RANDOM_ENTITY_DRAW_FUNC(name, each_##name, first_##name, current_type, next_type)  \
  EVERY_ENTITY_FUNC(name, each_##name, each_##name##_logic, current_type, next_type) \
  EVERY_ENTITY_DRAW_FUNC(name, first_##name, current_type)                           \
  
  
    CHANGE_SCOPE_FUNC_IMPLEMENTATION(city_global,           character_ptr_t, city_ptr_t)
    CHANGE_SCOPE_FUNC_IMPLEMENTATION(province_global,       character_ptr_t, province_ptr_t)
    CHANGE_SCOPE_FUNC_IMPLEMENTATION(barony_global,         character_ptr_t, title_ptr_t)
    CHANGE_SCOPE_FUNC_IMPLEMENTATION(duchy_global,          character_ptr_t, title_ptr_t)
    CHANGE_SCOPE_FUNC_IMPLEMENTATION(kingdom_global,        character_ptr_t, title_ptr_t)
    CHANGE_SCOPE_FUNC_IMPLEMENTATION(empire_global,         character_ptr_t, title_ptr_t)
    CHANGE_SCOPE_FUNC_IMPLEMENTATION(religion_group_global, character_ptr_t, religion_group_ptr_t)
    CHANGE_SCOPE_FUNC_IMPLEMENTATION(religion_global,       character_ptr_t, religion_ptr_t)
    CHANGE_SCOPE_FUNC_IMPLEMENTATION(culture_group_global,  character_ptr_t, culture_group_ptr_t)
    CHANGE_SCOPE_FUNC_IMPLEMENTATION(culture_global,        character_ptr_t, culture_ptr_t)
    CHANGE_SCOPE_FUNC_IMPLEMENTATION(living_character_global, character_ptr_t, character_ptr_t)
    CHANGE_SCOPE_FUNC_IMPLEMENTATION(ruler_global,          character_ptr_t, character_ptr_t)
    CHANGE_SCOPE_FUNC_IMPLEMENTATION(independent_ruler_global, character_ptr_t, character_ptr_t)
    CHANGE_SCOPE_FUNC_IMPLEMENTATION(player_global,         character_ptr_t, character_ptr_t)
  
#undef HAS_ENTITY_FUNC
#undef HAS_ENTITY_DRAW_FUNC
#undef RANDOM_ENTITY_FUNC
#undef RANDOM_ENTITY_DRAW_FUNC
#undef EVERY_ENTITY_FUNC
#undef EVERY_ENTITY_DRAW_FUNC
#undef CHANGE_SCOPE_FUNC_IMPLEMENTATION
  }
}
