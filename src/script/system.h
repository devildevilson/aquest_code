#ifndef DEVILS_ENGINE_SCRIPT_SYSTEM_H
#define DEVILS_ENGINE_SCRIPT_SYSTEM_H

#include <functional>
#include <string>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "parallel_hashmap/phmap.h"

#include "utils/memory_pool.h"
#include "utils/sol.h"
#include "utils/linear_rng.h"

#include "templates.h"
#include "type_info.h"
#include "interface.h"
#include "container.h"
#include "core.h"
#include "header.h"

#define FUNCTION_ITERATOR (SIZE_MAX-1)
#define UNDEFINED_ARGS_COUNT (SIZE_MAX-2)
#define EVERY_RETURN_TYPE (SIZE_MAX-3)

// короч все это дело требует тестировки серьезной
// + непомешало бы сохранить каунтер у итераторов в локальную переменную (тип "local:index")
// было бы неплохо расширить возможности работы со списками, а именно добавить что то вроде библиотеки
// std ranges (то есть функции типа: transform, filter и проч), эти функции будут преобразовывать
// один список в другой, где будут доступны эти функции? я думаю что валидно ожидать эти функции и
// в every_* и что то отдельное, но непонятно как в every_* отделить от обычных функций
// так что наверное должна быть функция view_* или *_view, в которой мы тем или иным способом будем ожидать
// функции range adaptors
// как определить что мы ожидаем? нам в этом должны помочь функции zip и 'return' (result?)
// для этих функций НЕОБХОДИМА строгая последовательность + в контексте в качестве текущего значения может появиться число

// какие функции: transform (меняет на что то другое), filter (не дает пройти объекту дальше), take/take_while, drop/drop_while, 
// join (вряд ли: соединение нескольких листов, хотя обойти несколько листов как один могло бы пригодиться),
// split (он разделяет массив на несколько, наверное это НЕ предпочтительный способ, в моем случае не особо понятно что делать с этим)
// (вообще часть сплита наверное можно куда то сохранить, но неочевидно как именно, в общем наверное нужно пропустить),
// reverse (в обратную сторону), zip_transform (соединяет несколько листов в один, наверное тоже нужно пропустить),
// reduce (сводит лист к одному значению)
// короче говоря получается только transform, filter, reduce, take/take_while, drop/drop_while + reverse? (может быть еще рандом выбрать в получившимся листе)
// как будет выглядеть скрипт:
// city_view = {
//   { transform = "title.parent.owner" },
//   { filter = { OR = { "is_state", "is_council" } } },
//   { reduce = "money" }, -- как понять какое действие нужно сделать? можно положить текущее вычисленное значение в локал - наверное предпочтительно
//   -- наверное должна быть еще какая то пустая таблица для выполнения эффектов
//   -- от нумериков и кондишенов будет требоваться в конце редьюс (хотя мож составить новый лист)
// }
// еще было бы неплохо обычный цикл зафигачить, хотя это сопряжено с большим количеством проблем (например ошибка бесконечного цикла)

namespace devils_engine {
  namespace script {
#define SAME_TYPE(type1, type2) (type_id<type1>() == type_id<type2>())
    
    size_t get_script_type(const size_t &type_id);
    
    template <typename R>
    constexpr size_t get_script_type() {
      if constexpr (SAME_TYPE(R, void)) { return script_types::effect; }
      else if constexpr (SAME_TYPE(R, bool)) { return script_types::condition; }
      else if constexpr (SAME_TYPE(R, double)) { return script_types::numeric; }
      else if constexpr (SAME_TYPE(R, std::string_view)) { return script_types::string; }
      else if constexpr (SAME_TYPE(R, object)) { return SIZE_MAX; } // any script type
      else return script_types::object;
      return SIZE_MAX;
    }
    
    constexpr bool compare_script_types(const size_t &input, const size_t &expected) {
      if (expected == SIZE_MAX) return true;
      if (input == SIZE_MAX) return true;
      if (input == expected) return true;
      if ((input == script_types::condition || input == script_types::numeric) && 
          (expected == script_types::condition || expected == script_types::numeric)) return true;
      return false;
    }
    
    void check_script_types(const std::string_view &name, const size_t &input, const size_t &expected);
    //static bool is_complex_object(const std::string_view &lvalue);
    
    namespace detail {
      extern const phmap::flat_hash_set<std::string_view> view_allowed_funcs;
      std::string_view get_sol_type_name(const sol::type &t);
    }
    
    class system {
      friend class view;
    public:
      struct init_func_data;
      struct init_context;
      using init_function_t = std::function<interface*(system*, init_context*, const sol::object &, container*)>;
      using get_info_func_t = std::function<std::string()>;
      
      // current_type может быть 0 (void), тогда нужно найти все переопределения функции и пихнуть их в overload
      // когда такие происходит? когда у нас в лвалуе берется значение из контекста, причем больше ничего не происходит
      // в этом случае остается только предполагать
      struct init_context {
        size_t root, current_type, prev_type, script_type, current_size;
        size_t computed_type, compare_operator, current_local_var_id, expected_type;
        const init_func_data* current_block;
        std::string_view script_name;
        std::string_view block_name;
        std::string_view function_name;
        phmap::flat_hash_map<std::string, std::pair<size_t, size_t>> local_var_ids;
        phmap::flat_hash_set<std::string> lists;
        
        // флаги?
        inline init_context() : 
          root(0), current_type(0), prev_type(0), script_type(SIZE_MAX), 
          current_size(0), computed_type(SIZE_MAX), compare_operator(compare_operators::more_eq), 
          current_local_var_id(0), expected_type(SIZE_MAX), current_block(nullptr)
        {}
        
        size_t save_local(const std::string &name, const size_t &type);
        std::tuple<size_t, size_t> get_local(const std::string_view &name) const;
        size_t remove_local(const std::string_view &name);
      };
      
      struct init_func_data {
        size_t script_type;
        size_t input_type;
        size_t return_type;
        size_t arguments_count;
        init_function_t func;
        get_info_func_t info;
      };
      
      struct change_expected_type {
        init_context* ctx;
        size_t expected_type;
        inline change_expected_type(init_context* ctx, const size_t &new_expected_type) : ctx(ctx), expected_type(ctx->expected_type) { ctx->expected_type = new_expected_type; }
        inline ~change_expected_type() { ctx->expected_type = expected_type; }
      };
      
      struct change_computed_type {
        init_context* ctx;
        size_t computed_type;
        inline change_computed_type(init_context* ctx, const size_t &new_computed_type) : ctx(ctx), computed_type(ctx->computed_type) { ctx->computed_type = new_computed_type; }
        inline ~change_computed_type() { ctx->computed_type = computed_type; }
      };
      
      class view {
      public:
        inline view(system* sys, init_context* ctx, container* cont, const bool is_iterator) : sys(sys), ctx(ctx), cont(cont), is_iterator(is_iterator) {}
        
        interface* make_scripted_conditional(const sol::object &obj);
        interface* make_scripted_numeric(const sol::object &obj);
        interface* make_scripted_string(const sol::object &obj);
        interface* make_scripted_effect(const sol::object &obj);
        interface* make_scripted_object(const size_t &id, const sol::object &obj);
        
        template <typename T>
        interface* make_scripted_object(const sol::object &obj) {
          return make_scripted_object(type_id<T>(), obj);
        }
        
        interface* any_scripted_object(const sol::object &obj);
        
        interface* traverse_children(const sol::object &obj);
        interface* traverse_children_numeric(const sol::object &obj);
        interface* traverse_children_condition(const sol::object &obj);
        
        size_t get_random_state();
        
        // это нинужно, в контексте лежит какбы любой тип 
        //size_t get_context_value(const std::string_view &name);
        //void set_context_value(const std::string_view &name, const size_t &type);
        
        size_t save_local(const std::string &name, const size_t &type);
        std::tuple<size_t, size_t> get_local(const std::string_view &name) const;
        size_t remove_local(const std::string_view &name);
        
        void add_list(const std::string &name);
        bool list_exists(const std::string_view &name) const; // вряд ли нужно проверять, можно добавить в несуществующий лист
        
        void set_return_type(const size_t &type); // скорее только для локала
        
        inline const init_context* get_context() const { return ctx; }
      private:
        system* sys;
        init_context* ctx;
        container* cont;
        bool is_iterator;
      };
      
      struct change_block_function {
        init_context* ctx;
        const init_func_data* current_block;
        inline change_block_function(init_context* ctx, const init_func_data* new_block) : ctx(ctx), current_block(ctx->current_block) { ctx->current_block = new_block; }
        inline ~change_block_function() { ctx->current_block = current_block; }
      };
      
      struct change_context_types {
        init_context* ctx;
        size_t current_type;
        size_t prev_type;
        inline change_context_types(init_context* ctx, const size_t new_current_type, const size_t new_prev_type) : ctx(ctx), current_type(ctx->current_type), prev_type(ctx->prev_type) {
          ctx->current_type = new_current_type;
          ctx->prev_type = new_prev_type;
        }
        inline ~change_context_types() { 
          ctx->current_type = current_type;
          ctx->prev_type = prev_type;
        }
      };
      
      struct change_script_type {
        init_context* ctx;
        size_t script_type;
        inline change_script_type(init_context* ctx, const size_t new_script_type) : ctx(ctx), script_type(ctx->script_type) { ctx->script_type = new_script_type; }
        inline ~change_script_type() { ctx->script_type = script_type; }
      };
      
      struct change_block_name {
        init_context* ctx;
        std::string_view block_name;
        inline change_block_name(init_context* ctx, const std::string_view new_block_name) : ctx(ctx), block_name(ctx->block_name) { ctx->block_name = new_block_name; }
        inline ~change_block_name() { ctx->block_name = block_name; }
      };
      
      struct change_current_function_name {
        init_context* ctx;
        std::string_view function_name;
        inline change_current_function_name(init_context* ctx, const std::string_view new_function_name) : ctx(ctx), function_name(ctx->function_name) { ctx->function_name = new_function_name; }
        inline ~change_current_function_name() { ctx->function_name = function_name; }
      };
      
      struct change_compare_op {
        init_context* ctx;
        size_t compare_operator;
        inline change_compare_op(init_context* ctx, const size_t new_compare_operator) : ctx(ctx), compare_operator(ctx->compare_operator) { ctx->compare_operator = new_compare_operator; }
        inline ~change_compare_op() { ctx->compare_operator = compare_operator; }
      };
      
      system(const size_t &seed = 1);
      ~system();
      system(const system &copy) = delete;
      system(system &&move) = default;
      system & operator=(const system &copy) = delete;
      system & operator=(system &&move) = default;
      
      void copy_init_funcs_to(system &another);
      
      size_t get_next_random_state();
      
      // тут по идее что то еще должно быть, например зарегистрировать проверку типа
      // для этого имя нужно задать самостоятельно, потребуется его задавать в темплейте
      // что то еще?
      template <typename T>
      void register_usertype();
      
      // можем ли мы это сделать для произвольного числа входных данных? по крайней мере для нескольких первых
      template <typename Th, typename F, F f, const char* name>
      void register_function();
      
      // к нам тут должна приходить таблица с входными данными
      // из нее по именам мы должны забрать нужные нам скрипты
      // только по именам? нет, может быть еще и по индексам, тогда все более менее просто
      // если по именам то нужно будет передавать список имен куда то
      template <typename F, size_t N, size_t cur>
      interface* make_arguments(
        const std::string_view &name, const std::vector<std::string> &args_name, 
        init_context* ctx, const sol::table &t, container* cont, 
        interface* begin = nullptr, interface* current = nullptr
      );
      
      template <typename F, size_t N, size_t cur>
      interface* make_arguments(const std::string_view &name, init_context* ctx, const sol::table &t, container* cont, interface* begin = nullptr, interface* current = nullptr);
      template <typename Th, typename F, F f, const char* name>
      void register_function_with_args(const std::vector<std::string> &args_name);
      template <typename Th, typename F, F f, const char* name>
      void register_function_with_args();
      
      // тут по идее нужно добавить регистрацию с пользовательской функцией, но это приведет
      // к тому что эту функцию мы будем копировать по миллиону раз, а это глупо
      
      // так мы сделаем общие функции
      template <typename F, F f, const char* name>
      void register_function();
      template <typename F, F f, const char* name>
      void register_function_with_args(const std::vector<std::string> &args_name);
      template <typename F, F f, const char* name>
      void register_function_with_args();
      
      template <typename F, typename I, typename R, const char* name>
      void register_function();
      
      // тут может добавиться некий аргумент (константный? скорее всего)
      // который мы передадим в специальную функцию
      template <typename Th, typename F, F f, const char* name, int64_t head, int64_t... args>
      void register_function();
      template <typename Th, typename F, F f, const char* name, typename... Args>
      void register_function(Args&&... args);
      template <typename T, typename F, typename R, const char* name>
      void register_function(const std::function<interface*(system::view, const sol::object &, container::delayed_initialization<F>)> &user_func);
      template <typename T, typename F, typename R, const char* name>
      void register_iterator();
      template <typename T, typename F, typename R, const char* name>
      void register_iterator(const std::function<interface*(system::view, const sol::object &, container::delayed_initialization<F>)> &user_func);
      template <typename Th, typename F, F f, const char* name>
      void register_every();
      template <typename Th, typename F, F f, const char* name>
      void register_has();
      template <typename Th, typename F, F f, const char* name>
      void register_random();
      template <typename Th, typename F, F f, const char* name>
      void register_view();
      
      // возвращать будем легкие обертки вокруг interface* (тип script::number и проч)
      // + нужно будет возвращать скрипты по названию, скрипты которые вставляются в другие скрипты вызываются иначе
      // + могут пригодится флаги
      template <typename T>
      condition create_condition(const sol::object &obj, const std::string_view &script_name, const std::string_view &save_name = "");
      
      template <typename T>
      number create_number(const sol::object &obj, const std::string_view &script_name, const std::string_view &save_name = "");
      
      template <typename T>
      string create_string(const sol::object &obj, const std::string_view &script_name, const std::string_view &save_name = "");
      
      template <typename T>
      effect create_effect(const sol::object &obj, const std::string_view &script_name, const std::string_view &save_name = "");
      
      condition get_condition(const std::string_view &name);
      number get_number(const std::string_view &name);
      string get_string(const std::string_view &name);
      effect get_effect(const std::string_view &name);
      
      template <typename T>
      const init_func_data* get_init_function(const std::string_view &name) const;
      
      std::string get_function_data_dump() const;
    private:
      bool function_exists(const std::string_view &name, size_t* return_type = nullptr, size_t* arg_count = nullptr);
      const init_func_data* get_init_function(const size_t &type_id, const std::string_view &name) const;
      
      interface* make_raw_script_boolean(init_context* ctx, const sol::object &obj, container* cont);
      interface* make_raw_script_number(init_context* ctx, const sol::object &obj, container* cont);
      interface* make_raw_script_string(init_context* ctx, const sol::object &obj, container* cont);
      interface* make_raw_script_effect(init_context* ctx, const sol::object &obj, container* cont);
      interface* make_raw_script_object(init_context* ctx, const sol::object &obj, container* cont);
      interface* make_raw_script_any(init_context* ctx, const sol::object &obj, container* cont);
      
      interface* find_common_function(init_context* ctx, const std::string_view &func_name, const sol::object &obj, container* cont);
      interface* create_overload(init_context* ctx, const std::string_view &func_name, const sol::object &obj, container* cont);
      
      interface* make_effect_context(init_context* ctx, const std::string_view &lvalue, const sol::object &rvalue, container* cont);
      interface* make_string_context(init_context* ctx, const sol::object &rvalue, container* cont);
      interface* make_object_context(init_context* ctx, const sol::object &rvalue, container* cont);
      interface* make_context_change(init_context* ctx, const std::string_view &lvalue, const sol::object &rvalue, container* cont);
      
      interface* make_raw_number_compare(init_context* ctx, const interface* lvalue, const interface* rvalue, container* cont);
      interface* make_raw_boolean_compare(init_context* ctx, const interface* lvalue, const interface* rvalue, container* cont);
      
      interface* make_number_compare(init_context* ctx, const std::string_view &lvalue, const sol::object &rvalue, container* cont);
      interface* make_boolean_compare(init_context* ctx, const std::string_view &lvalue, const sol::object &rvalue, container* cont);
      interface* make_compare(init_context* ctx, const std::string_view &lvalue, const sol::object &rvalue, container* cont, const size_t &return_type);
      
      interface* make_complex_object(init_context* ctx, const std::string_view &lvalue, const sol::object &rvalue, container* cont);
      interface* make_table_rvalue(init_context* ctx, const sol::object &obj, container* cont);
      interface* make_table_lvalue(init_context* ctx, const std::string_view &lvalue, const sol::object &rvalue, container* cont);
      
      interface* condition_table_traverse(init_context* ctx, const sol::object &obj, container* cont);
      interface* numeric_table_traverse(init_context* ctx, const sol::object &obj, container* cont);
      interface* string_table_traverse(init_context* ctx, const sol::object &obj, container* cont);
      interface* object_table_traverse(init_context* ctx, const sol::object &obj, container* cont);
      interface* effect_table_traverse(init_context* ctx, const sol::object &obj, container* cont);
      interface* table_traverse(init_context* ctx, const sol::object &obj, container* cont);
      
      void register_every_list();
      
      template <typename T, const char* name>
      void register_function(init_func_data&& data);
      
      void register_function(const size_t &id, const std::string_view &type_name, const std::string_view &name, init_func_data&& data);
      
      utils::xoroshiro128starstar::state random_state;
      utils::memory_pool<container, sizeof(container)*100> containers_pool;
      std::vector<std::pair<interface*, container*>> containers;
      phmap::flat_hash_map<std::string, std::pair<size_t, interface*>> scripts; // проверяем чтобы возвращаемые типы совпадали
      phmap::flat_hash_map<size_t, phmap::flat_hash_map<std::string_view, init_func_data>> func_map;
      
      void init(); // default functions init
    };
    
    // я чет не подумал что довольно большая часть функций не принадлежит типу, и тип наверное лучше бы указать отдельно
#define REG_FUNC(handle_type, func, name) register_function<handle_type, decltype(&func), &func, decltype(name##_create)::value>
#define REG_BASIC(func, name) register_function<decltype(&func), &func, decltype(name##_create)::value>
#define REG_BASIC_TYPE(type, func, ret, name) register_function<type, func, ret, decltype(name##_create)::value>
#define REG_CONST_ARGS(handle_type, func, name, ...) register_function<handle_type, decltype(&func), &func, decltype(name##_create)::value, __VA_ARGS__>
#define REG_ARGS(handle_type, func, name) register_function_with_args<handle_type, decltype(&func), &func, decltype(name##_create)::value>
#define REG_BASIC_ARGS(func, name) register_function_with_args<decltype(&func), &func, decltype(name##_create)::value>
#define REG_USER(type, func_type, ret_type, name) register_function<type, func_type, ret_type, decltype(name##_create)::value>
#define REG_ITR(type, func_type, ret_type, name) register_iterator<type, func_type, ret_type, decltype(name##_create)::value>
#define REG_EVERY(handle_type, func, name) register_every<handle_type, decltype(&func), &func, decltype(name##_create)::value>
#define REG_HAS(handle_type, func, name) register_has<handle_type, decltype(&func), &func, decltype(name##_create)::value>
#define REG_RANDOM(handle_type, func, name) register_random<handle_type, decltype(&func), &func, decltype(name##_create)::value>
#define REG_VIEW(handle_type, func, name) register_view<handle_type, decltype(&func), &func, decltype(name##_create)::value>
    
    
    /* ====================================================================================================================================================================== */
    /* ====================================================================================================================================================================== */
    /* ====================================================================================================================================================================== */
    /* ====================================================================================================================================================================== */
    /* ====================================================================================================================================================================== */
    
    
    template <typename T>
    void system::register_usertype() { 
      const auto itr = func_map.find(type_id<T>());
      if (itr != func_map.end()) throw std::runtime_error("Type " + std::string(type_name<T>()) + " is already registered");
      func_map[type_id<T>()];
    }
    
    template <typename F, size_t N, size_t cur>
    std::string make_function_args_string(std::string str = "") {
      if constexpr (N == cur) return str;
      else {
        using cur_arg = final_arg_type<F, cur>;
        return make_function_args_string<F, N, cur+1>(str + std::string(type_name<cur_arg>()) + " ");
      }
      return "";
    }
    
    template <typename F>
    std::string make_function_const_args_string() {
      return "";
    }
    
    template <typename F, size_t head, size_t... args>
    std::string make_function_const_args_string() {
      return std::to_string(head) + " " + make_function_const_args_string<F, args...>();
    }
    
    // можем ли мы это сделать для произвольного числа входных данных? по крайней мере для нескольких первых
    template <typename Th, typename F, F f, const char* name>
    void system::register_function() {
      using output_type = final_output_type<F>;
      constexpr size_t script_type = get_script_type<output_type>();
      constexpr size_t arg_count = get_function_argument_count<F>() - size_t(!is_member_v<Th, F> && std::is_same_v<final_arg_type<F, 0>, Th>);
      //std::is_same_v<function_member_of<F>, T> &&
      //constexpr bool no_args = (arg_count == 0) || (!is_member && arg_count == 1 && std::is_invocable_v<F, Th>);
      //static_assert(no_args);
      
      init_func_data ifd{
        script_type, type_id<Th>(), type_id<output_type>(), arg_count,
        [] (system* sys, init_context* ctx, const sol::object &obj, container* cont) -> interface* {
          constexpr size_t arg_count = get_function_argument_count<F>() - size_t(!is_member_v<Th, F> && std::is_same_v<final_arg_type<F, 0>, Th>);
          constexpr size_t script_type = get_script_type<output_type>();
          check_script_types(name, ctx->script_type, script_type);
          change_current_function_name ccfn(ctx, name);
          change_compare_op cco(ctx, compare_operators::more_eq);
          
          if constexpr (arg_count == 0) {
            using function_type = scripted_function_no_arg<Th, F, f, name>;
            
            //using input_type = function_argument_type<F, 0>;
            //static_assert(std::is_same_v<std::remove_reference_t<std::remove_cv_t<input_type>>, Th>, "Registering a handle function is not allowed");
            // тут мы можем создать смену контекста если функция возвращает какой то объект
            
            const size_t size = sizeof(function_type);
            ctx->current_size += size;
            interface* cur = nullptr;
            if (cont != nullptr) { cur = cont->add<function_type>(); }
            if (obj.valid()) {
              if constexpr (std::is_same_v<output_type, bool>) { 
                auto rvalue = sys->make_raw_script_boolean(ctx, obj, cont); 
                return sys->make_raw_boolean_compare(ctx, cur, rvalue, cont);
              } else if constexpr (std::is_same_v<output_type, double>) {
                change_compare_op cco(ctx, compare_operators::more_eq);
                auto rvalue = sys->make_raw_script_number(ctx, obj, cont);
                return sys->make_raw_number_compare(ctx, cur, rvalue, cont);
              } else if constexpr (std::is_same_v<output_type, void>) throw std::runtime_error("Could not compare effect function '" + std::string(name) + "'");
              else throw std::runtime_error("Found compare pattern but return type '" + std::string(type_name<output_type>()) + "' is not supported, for objects and strings use function 'equals_to'");
            }
            
            return cur;
          } else {
            //using function_type = scripted_function_one_arg<T, Th, F, f, name>;
            //using function_type = scripted_function_scripted_args<T, Th, F, f, name>;
            //constexpr bool first_is_handle = std::is_same_v<Th, final_arg_type<F, 0>>;
            using function_type = scripted_function_one_arg<Th, F, f, name>;
            using input_type = typename std::conditional< !is_member_v<Th, F>, final_arg_type<F, 1>, final_arg_type<F, 0> >::type;
            static_assert(arg_count == 1 || (!is_member_v<Th, F> && arg_count == 2), "If function has more then 1 args use another function");
            
            if (!obj.valid()) throw std::runtime_error("Function '" + std::string(name) + "' expects input");
            
            size_t offset = SIZE_MAX;
            if (cont != nullptr) offset = cont->add_delayed<function_type>();
            ctx->current_size += sizeof(function_type);
            
            interface* inter = nullptr;
            switch (type_id<input_type>()) {
              case type_id<void>(): throw std::runtime_error("Bad input type for function '" + std::string(name) + "'");
              case type_id<bool>(): inter = sys->make_raw_script_boolean(ctx, obj, cont);break;
              case type_id<double>(): inter = sys->make_raw_script_number(ctx, obj, cont); break;
              case type_id<std::string_view>(): inter = sys->make_raw_script_string(ctx, obj, cont); break;
              default: {
                // мы по идее должны ожидать только зарегистрированные объекты
                if (sys->func_map.find(type_id<input_type>()) == sys->func_map.end()) throw std::runtime_error("Object " + std::string(type_name<input_type>()) + " was not registrated");
                change_expected_type cet(ctx, type_id<input_type>());
                inter = sys->make_raw_script_object(ctx, obj, cont);
                break;
              }
            }
            
            interface* cur = nullptr;
            if (cont != nullptr) {
              auto init = cont->get_init<function_type>(offset);
              cur = init.init(inter);
            }
            
            return cur;
          }
        }, [] () { 
          constexpr bool is_independent_func = !is_member_v<Th, F>;
          if constexpr (arg_count == 0) return std::string(name) + " I: " + std::string(type_name<Th>()) + " O: " + std::string(type_name<output_type>());
          else return std::string(name) + " I: " + std::string(type_name<Th>()) + 
                                          " O: " + std::string(type_name<output_type>()) + 
                                          " Args: " + make_function_args_string<F, get_function_argument_count<F>(), is_independent_func>();
        }
      };
      
      register_function<Th, name>(std::move(ifd));
    }
    
    // к нам тут должна приходить таблица с входными данными
    // из нее по именам мы должны забрать нужные нам скрипты
    // только по именам? нет, может быть еще и по индексам, тогда все более менее просто
    // если по именам то нужно будет передавать список имен куда то
    template <typename F, size_t N, size_t cur>
    interface* system::make_arguments(const std::string_view &name, const std::vector<std::string> &args_name, init_context* ctx, const sol::table &t, container* cont, interface* begin, interface* current) {
      if constexpr (cur == N) return begin;
      else {
        const auto proxy = t[args_name[cur]];
        if (!proxy.valid()) throw std::runtime_error("Could not find argument " + args_name[cur] + " for function '" + std::string(name) + "'");
        
        interface* local = nullptr;
        using input_type = final_arg_type<F, cur>;
        switch (type_id<input_type>()) {
          case type_id<void>(): throw std::runtime_error("Bad input type for function '" + std::string(name) + "'");
          case type_id<bool>(): local = make_raw_script_boolean(ctx, proxy, cont); break;
          case type_id<double>(): local = make_raw_script_number(ctx, proxy, cont); break;
          case type_id<std::string_view>(): local = make_raw_script_string(ctx, proxy, cont); break;
          default: {
            if (func_map.find(type_id<input_type>()) == func_map.end()) throw std::runtime_error("Object " + std::string(type_name<input_type>()) + " was not registrated");
            change_expected_type cet(ctx, type_id<input_type>());
            local = make_raw_script_object(ctx, proxy, cont); 
            break;
          }
        }
        
        if (current != nullptr) current->next = local;
        return make_arguments<F, N, cur+1>(name, args_name, ctx, t, cont, begin == nullptr ? local : begin, local);
      }
      
      return nullptr;
    }
    
    template <typename F, size_t N, size_t cur>
    interface* system::make_arguments(const std::string_view &name, init_context* ctx, const sol::table &t, container* cont, interface* begin, interface* current) {
      if constexpr (cur == N) return begin;
      else {
        const auto proxy = t[TO_LUA_INDEX(cur)];
        if (!proxy.valid()) throw std::runtime_error("Could not find argument " + std::to_string(TO_LUA_INDEX(cur)) + " for function " + std::string(name));
        
        interface* local = nullptr;
        using input_type = final_arg_type<F, cur>;
        switch (type_id<input_type>()) {
          case type_id<void>(): throw std::runtime_error("Bad input type for function '" + std::string(name) + "'");
          case type_id<bool>(): local = make_raw_script_boolean(ctx, proxy, cont); break;
          case type_id<double>(): local = make_raw_script_number(ctx, proxy, cont); break;
          case type_id<std::string_view>(): local = make_raw_script_string(ctx, proxy, cont); break;
          default: {
            if (func_map.find(type_id<input_type>()) == func_map.end()) throw std::runtime_error("Object " + std::string(type_name<input_type>()) + " was not registrated");
            change_expected_type cet(ctx, type_id<input_type>());
            local = make_raw_script_object(ctx, proxy, cont); 
            break;
          }
        }
        
        if (current != nullptr) current->next = local;
        return make_arguments<F, N, cur+1>(name, ctx, t, cont, begin == nullptr ? local : begin, local);
      }
      
      return nullptr;
    }
    
    static double test_func1(void*, double, double) { return 0.0; }
    using test_func_t = decltype(&test_func1);
    static_assert(get_function_argument_count<test_func_t>() == 3);
    constexpr bool is_member = std::is_same_v<system, function_member_of<test_func_t>>;
    constexpr size_t test_func_arg_count = get_function_argument_count<test_func_t>() - size_t(!is_member && std::is_same_v<final_arg_type<test_func_t, 0>, void*>);
    static_assert(test_func_arg_count == 2);
    
    
    template <typename Th, typename F, F f, const char* name>
    void system::register_function_with_args(const std::vector<std::string> &args_name) {
      constexpr size_t func_arg_count = get_function_argument_count<F>() - size_t(!is_member_v<Th, F> && std::is_same_v<final_arg_type<F, 0>, Th>);
      
      if constexpr (func_arg_count == 0) { register_function<Th, F, f, name>(); return; }
      
      if (args_name.size() < func_arg_count) 
        throw std::runtime_error("Function expects " + std::to_string(func_arg_count) + " arguments, "
                                  "but receive only " + std::to_string(args_name.size()) + " arguments names");
      
      using output_type = final_output_type<F>;
      constexpr size_t script_type = get_script_type<output_type>();
      
      init_func_data ifd{
        script_type, type_id<Th>(), type_id<output_type>(), func_arg_count,
        [args_name] (system* sys, init_context* ctx, const sol::object &obj, container* cont) -> interface* {
          constexpr size_t func_arg_count = get_function_argument_count<F>() - size_t(!is_member_v<Th, F> && std::is_same_v<final_arg_type<F, 0>, Th>);
          constexpr size_t script_type = get_script_type<output_type>();
          check_script_types(name, ctx->script_type, script_type);
          using function_type = scripted_function_scripted_args<Th, F, f, name>;
          change_current_function_name ccfn(ctx, name);
          change_compare_op cco(ctx, compare_operators::more_eq);
          
          // если это не метод, то первый слот искать не нужно
          constexpr size_t final_arg_count = get_function_argument_count<F>(); //func_arg_count; //  - std::is_same_v<function_member_of<F>, void>
          constexpr bool is_independent_func = !is_member_v<Th, F>;
          if constexpr (is_independent_func) { static_assert(std::is_same_v<final_arg_type<F, 0>, Th>); }
          
          // всегда тут ожидаем таблицу?
          //if (func_arg_count == 1 && obj.get_type() != sol::type::table) {}
          if (obj.get_type() != sol::type::table) throw std::runtime_error("Function " + std::string(name) + " expects table with arguments");
          
          size_t offset = SIZE_MAX;
          if (cont != nullptr) offset = cont->add_delayed<function_type>();
          ctx->current_size += sizeof(function_type);
          
          const auto table = obj.as<sol::table>();
          auto args = sys->make_arguments<F, final_arg_count, is_independent_func>(name, args_name, ctx, table, cont);
          
#ifndef _NDEBUG
          if (args != nullptr) {
            size_t counter = 0;
            for (auto cur = args; cur != nullptr; cur = cur->next) { counter += 1; } 
            assert(counter == func_arg_count);
          }
#endif
          
          interface* cur = nullptr;
          if (cont != nullptr) {
            auto init = cont->get_init<function_type>(offset);
            cur = init.init(args);
          }
          
          return cur;
        }, [] () { 
          constexpr bool is_independent_func = !is_member_v<Th, F>;
          if constexpr (func_arg_count == 0) return std::string(name) + " I: " + std::string(type_name<Th>()) + " O: " + std::string(type_name<output_type>());
          else return std::string(name) + " I: " + std::string(type_name<Th>()) + 
                                          " O: " + std::string(type_name<output_type>()) + 
                                          " Args: " + make_function_args_string<F, get_function_argument_count<F>(), is_independent_func>();
        }
      };
      
      register_function<Th, name>(std::move(ifd));
    }
    
    template <typename Th, typename F, F f, const char* name>
    void system::register_function_with_args() {
      constexpr size_t func_arg_count = get_function_argument_count<F>() - size_t(!is_member_v<Th, F> && std::is_same_v<final_arg_type<F, 0>, Th>);
      if constexpr (func_arg_count < 2) { register_function<Th, F, f, name>();  return; }
      
      using output_type = final_output_type<F>;
      constexpr size_t script_type = get_script_type<output_type>();
      
      init_func_data ifd{
        script_type, type_id<Th>(), type_id<output_type>(), func_arg_count,
        [] (system* sys, init_context* ctx, const sol::object &obj, container* cont) -> interface* {
          constexpr size_t func_arg_count = get_function_argument_count<F>() - size_t(!is_member_v<Th, F> && std::is_same_v<final_arg_type<F, 0>, Th>);
          constexpr size_t script_type = get_script_type<output_type>();
          check_script_types(name, ctx->script_type, script_type);
          using function_type = scripted_function_scripted_args<Th, F, f, name>;
          change_current_function_name ccfn(ctx, name);
          change_compare_op cco(ctx, compare_operators::more_eq);
          
          constexpr size_t final_arg_count = get_function_argument_count<F>();
          constexpr bool is_independent_func = std::is_same_v<function_member_of<F>, void>;
          if constexpr (is_independent_func) { static_assert(std::is_same_v<final_arg_type<F, 0>, Th>); }
          
          // всегда тут ожидаем таблицу?
          //if (func_arg_count == 1 && obj.get_type() != sol::type::table) {}
          if (obj.get_type() != sol::type::table) throw std::runtime_error("Function " + std::string(name) + " expects table with arguments");
          
          size_t offset = SIZE_MAX;
          if (cont != nullptr) offset = cont->add_delayed<function_type>();
          ctx->current_size += sizeof(function_type);
          
          const auto table = obj.as<sol::table>();
          auto args = sys->make_arguments<F, final_arg_count, is_independent_func>(name, ctx, table, cont);
          
#ifndef _NDEBUG
          if (args != nullptr) {
            size_t counter = 0;
            for (auto cur = args; cur != nullptr; cur = cur->next) { counter += 1; } 
            assert(counter == func_arg_count);
          }
#endif
          
          interface* cur = nullptr;
          if (cont != nullptr) {
            auto init = cont->get_init<function_type>(offset);
            cur = init.init(args);
          }
          
          return cur;
        }, [] () { 
          constexpr bool is_independent_func = !is_member_v<Th, F>;
          if constexpr (func_arg_count == 0) return std::string(name) + " I: " + std::string(type_name<Th>()) + " O: " + std::string(type_name<output_type>());
          else return std::string(name) + " I: " + std::string(type_name<Th>()) + 
                                          " O: " + std::string(type_name<output_type>()) + 
                                          " Args: " + make_function_args_string<F, get_function_argument_count<F>(), is_independent_func>();
        }
      };
      
      register_function<Th, name>(std::move(ifd));
    }
    
    // тут по идее нужно добавить регистрацию с пользовательской функцией, но это приведет
    // к тому что эту функцию мы будем копировать по миллиону раз, а это глупо
    
    // функции которые принимают на вход объект и возвращают другой без вызова методов
    template <typename F, F f, const char* name>
    void system::register_function() {
      constexpr size_t func_arg_count = get_function_argument_count<F>();
      using output_type = final_output_type<F>;
      using input_type = final_arg_type<F, 0>;
      constexpr size_t script_type = get_script_type<output_type>();
      using register_type = void; // это тип по которому зарегистрируем функцию
      
      static_assert(type_id<std::string_view>() == type_id<std::basic_string_view<char>>()); // sanity
      
      init_func_data ifd{
        script_type, SIZE_MAX, type_id<output_type>(), func_arg_count,
        [] (system* sys, init_context* ctx, const sol::object &obj, container* cont) -> interface* {
          constexpr size_t func_arg_count = get_function_argument_count<F>();
          constexpr size_t script_type = get_script_type<output_type>();
          check_script_types(name, ctx->script_type, script_type);
          change_current_function_name ccfn(ctx, name);
          change_compare_op cco(ctx, compare_operators::more_eq);
          
          if constexpr (func_arg_count == 0) {
            using function_type = basic_function_no_arg<F, f, name>;
            // создадим сравнение?
            ctx->current_size += sizeof(function_type);
            interface* ret = nullptr;
            if (cont != nullptr) ret = cont->add<function_type>();
            if (obj.valid()) {
              if constexpr (std::is_same_v<output_type, bool>) { 
                auto rvalue = sys->make_raw_script_boolean(ctx, obj, cont); 
                return sys->make_raw_boolean_compare(ctx, ret, rvalue, cont);
              } else if constexpr (std::is_same_v<output_type, double>) {
                change_compare_op cco(ctx, compare_operators::more_eq);
                auto rvalue = sys->make_raw_script_number(ctx, obj, cont);
                return sys->make_raw_number_compare(ctx, ret, rvalue, cont);
              } else if constexpr (std::is_same_v<output_type, void>) throw std::runtime_error("Could not compare effect function '" + std::string(name) + "'");
              else throw std::runtime_error("Found compare pattern but return type '" + std::string(type_name<output_type>()) + "' is not supported, for objects and strings use function 'equals_to'");
            }
            return ret;
          } else {
            using function_type = basic_function_scripted_arg<F, f, name>;
            
            if (!obj.valid()) throw std::runtime_error("Function '" + std::string(name) + "' expects input");
            
            size_t offset = SIZE_MAX;
            if (cont != nullptr) offset = cont->add_delayed<function_type>();
            ctx->current_size += sizeof(function_type);
            
            interface* inter = nullptr;
            switch (type_id<input_type>()) {
              case type_id<void>(): throw std::runtime_error("Bad input type for function '" + std::string(name) + "'");
              case type_id<bool>(): inter = sys->make_raw_script_boolean(ctx, obj, cont);            break;
              case type_id<double>(): inter = sys->make_raw_script_number(ctx, obj, cont);           break;
              case type_id<std::string_view>(): inter = sys->make_raw_script_string(ctx, obj, cont); break;
              default: {
                // мы по идее должны ожидать только зарегистрированные объекты
                change_expected_type cet(ctx, type_id<input_type>());
                if (sys->func_map.find(type_id<input_type>()) == sys->func_map.end()) throw std::runtime_error("Object " + std::string(type_name<input_type>()) + " was not registrated");
                inter = sys->make_raw_script_object(ctx, obj, cont);
                break;
              }
            }
            
            interface* cur = nullptr;
            if (cont != nullptr) {
              auto init = cont->get_init<function_type>(offset);
              cur = init.init(inter);
            }
            
            return cur;
          }
          
          return nullptr;
        }, [] () { 
          if constexpr (func_arg_count == 0) return std::string(name) + " I: " + std::string(type_name<void>()) + " O: " + std::string(type_name<output_type>());
          else return std::string(name) + " I: " + std::string(type_name<void>()) + 
                                          " O: " + std::string(type_name<output_type>()) + 
                                          " Args: " + make_function_args_string<F, func_arg_count, 0>();
        }
      };
      
      register_function<register_type, name>(std::move(ifd));
    }
    
    template <typename F, F f, const char* name>
    void system::register_function_with_args(const std::vector<std::string> &args_name) {
      constexpr size_t func_arg_count = get_function_argument_count<F>();
      using output_type = final_output_type<F>;
      constexpr size_t script_type = get_script_type<output_type>();
      using register_type = void; // это тип по которому зарегистрируем функцию
      
      if constexpr (func_arg_count == 0) { register_function<F, f, name>();  return; }
      
      init_func_data ifd{
        script_type, SIZE_MAX, type_id<output_type>(), func_arg_count,
        [args_name] (system* sys, init_context* ctx, const sol::object &obj, container* cont) -> interface* {
          constexpr size_t func_arg_count = get_function_argument_count<F>();
          constexpr size_t script_type = get_script_type<output_type>();
          check_script_types(name, ctx->script_type, script_type);
          change_current_function_name ccfn(ctx, name);
          change_compare_op cco(ctx, compare_operators::more_eq);
          using function_type = basic_function_scripted_args<F, f, name>;
          
          if (obj.get_type() != sol::type::table) throw std::runtime_error("Function " + std::string(name) + " expects table with arguments");
          
          size_t offset = SIZE_MAX;
          if (cont != nullptr) offset = cont->add_delayed<function_type>();
          ctx->current_size += sizeof(function_type);
          
          const auto table = obj.as<sol::table>();
          auto args = sys->make_arguments<F, func_arg_count, 0>(name, args_name, ctx, table, cont);
          
#ifndef _NDEBUG
          if (args != nullptr) {
            size_t counter = 0;
            for (auto cur = args; cur != nullptr; cur = cur->next) { counter += 1; } 
            assert(counter == func_arg_count);
          }
#endif
          
          interface* cur = nullptr;
          if (cont != nullptr) {
            auto init = cont->get_init<function_type>(offset);
            cur = init.init(args);
          }
          
          return cur;
        }, [] () { 
          if constexpr (func_arg_count == 0) return std::string(name) + " I: " + std::string(type_name<void>()) + " O: " + std::string(type_name<output_type>());
          else return std::string(name) + " I: " + std::string(type_name<void>()) + 
                                          " O: " + std::string(type_name<output_type>()) + 
                                          " Args: " + make_function_args_string<F, func_arg_count, 0>();
        }
      };
      
      register_function<register_type, name>(std::move(ifd));
    }
    
    template <typename F, F f, const char* name>
    void system::register_function_with_args() {
      constexpr size_t func_arg_count = get_function_argument_count<F>();
      using output_type = final_output_type<F>;
      constexpr size_t script_type = get_script_type<output_type>();
      using register_type = void; // это тип по которому зарегистрируем функцию
      
      if constexpr (func_arg_count < 2) { register_function<F, f, name>();  return; }
      
      init_func_data ifd{
        script_type, SIZE_MAX, type_id<output_type>(), func_arg_count,
        [] (system* sys, init_context* ctx, const sol::object &obj, container* cont) -> interface* {
          constexpr size_t func_arg_count = get_function_argument_count<F>();
          constexpr size_t script_type = get_script_type<output_type>();
          check_script_types(name, ctx->script_type, script_type);
          change_current_function_name ccfn(ctx, name);
          change_compare_op cco(ctx, compare_operators::more_eq);
          using function_type = basic_function_scripted_args<F, f, name>;
          
          if (obj.get_type() != sol::type::table) throw std::runtime_error("Function " + std::string(name) + " expects table with arguments");
          
          size_t offset = SIZE_MAX;
          if (cont != nullptr) offset = cont->add_delayed<function_type>();
          ctx->current_size += sizeof(function_type);
          
          const auto table = obj.as<sol::table>();
          auto args = sys->make_arguments<F, func_arg_count, 0>(name, ctx, table, cont);
          
#ifndef _NDEBUG
          if (args != nullptr) {
            size_t counter = 0;
            for (auto cur = args; cur != nullptr; cur = cur->next) { counter += 1; } 
            assert(counter == func_arg_count);
          }
#endif
          
          interface* cur = nullptr;
          if (cont != nullptr) {
            auto init = cont->get_init<function_type>(offset);
            cur = init.init(args);
          }
          
          return cur;
        }, [] () { 
          if constexpr (func_arg_count == 0) return std::string(name) + " I: " + std::string(type_name<void>()) + " O: " + std::string(type_name<output_type>());
          else return std::string(name) + " I: " + std::string(type_name<void>()) + 
                                          " O: " + std::string(type_name<output_type>()) + 
                                          " Args: " + make_function_args_string<F, func_arg_count, 0>();
        }
      };
      
      register_function<register_type, name>(std::move(ifd));
    }
    
    template <typename I, typename F, typename R, const char* name>
    void system::register_function() {
      using output_type = R;
      using input_type = I;
      constexpr size_t script_type = get_script_type<output_type>();
      //using register_type = void; // это тип по которому зарегистрируем функцию
      
      init_func_data ifd{
        script_type, type_id<input_type>(), type_id<output_type>(), UNDEFINED_ARGS_COUNT,
        [] (system*, init_context* ctx, const sol::object &, container* cont) -> interface* {
          constexpr size_t script_type = get_script_type<output_type>();
          check_script_types(name, ctx->script_type, script_type);
          change_current_function_name ccfn(ctx, name);
          using function_type = F;
          
          if (cont != nullptr) return cont->add<function_type>();
          return nullptr;
        }, [] () { 
          return std::string(name) + " I: " + std::string(type_name<input_type>()) + 
                                     " O: " + std::string(type_name<output_type>()) + 
                                     " F: " + std::string(type_name<F>());
        }
      };
      
      register_function<input_type, name>(std::move(ifd));
    }
    
    // тут может добавиться некий аргумент (константный? скорее всего)
    // который мы передадим в специальную функцию
    template <typename Th, typename F, F f, const char* name, int64_t head, int64_t... args>
    void system::register_function() {
      using output_type = final_output_type<F>;
      constexpr size_t script_type = get_script_type<output_type>();
      constexpr size_t arg_count = 0; // такие функции считаются за 0 аргументов
      
      init_func_data ifd{
        script_type, type_id<Th>(), type_id<output_type>(), arg_count,
        [] (system* sys, init_context* ctx, const sol::object &obj, container* cont) -> interface* {
          using function_type = scripted_function_const_args<Th, F, f, name, head, args...>;
          
          constexpr size_t script_type = get_script_type<output_type>();
          check_script_types(name, ctx->script_type, script_type);
          
          ctx->current_size += sizeof(function_type);
          interface* cur = nullptr;
          if (cont != nullptr) cur = cont->add<function_type>();
          if (obj.valid()) {
            if constexpr (std::is_same_v<output_type, bool>) { 
              auto rvalue = sys->make_raw_script_boolean(ctx, obj, cont); 
              return sys->make_raw_boolean_compare(ctx, cur, rvalue, cont);
            } else if constexpr (std::is_same_v<output_type, double>) {
              change_compare_op cco(ctx, compare_operators::more_eq);
              auto rvalue = sys->make_raw_script_number(ctx, obj, cont);
              return sys->make_raw_number_compare(ctx, cur, rvalue, cont);
            } else if constexpr (std::is_same_v<output_type, void>) throw std::runtime_error("Could not compare effect function '" + std::string(name) + "'");
            else throw std::runtime_error("Found compare pattern but return type '" + std::string(type_name<output_type>()) + "' is not supported, for objects and strings use function 'equals_to'");
          }
          
          return cur;
        }, [] () -> std::string { 
          return std::string(name) + " I: " + std::string(type_name<Th>()) + 
                                     " O: " + std::string(type_name<output_type>()) + 
                                     " CArgs: " + make_function_const_args_string<F, head, args...>();
        }
      };
      
      register_function<Th, name>(std::move(ifd));
    }
    
    template <typename Th, typename F, F f, const char* name, typename... Args>
    void system::register_function(Args&&... args) {
      static_assert(std::conjunction_v<std::is_copy_assignable<Args>...>);
      using output_type = final_output_type<F>;
      constexpr size_t script_type = get_script_type<output_type>();
      constexpr size_t arg_count = 0; // такие функции считаются за 0 аргументов
      
      init_func_data ifd{
        script_type, type_id<Th>(), type_id<output_type>(), arg_count,
        [func_args = std::make_tuple(std::forward<Args>(args) ...)] (system* sys, init_context* ctx, const sol::object &obj, container* cont) {
          using function_type = scripted_function_args<Th, F, f, name, Args...>;
          
          constexpr size_t script_type = get_script_type<output_type>();
          check_script_types(name, ctx->script_type, script_type);
          
          ctx->current_size += sizeof(function_type);
          interface* cur = nullptr;
          if (cont != nullptr) cur = cont->add<function_type>(func_args);
          if (obj.valid()) {
            if constexpr (std::is_same_v<output_type, bool>) { 
              auto rvalue = sys->make_raw_script_boolean(ctx, obj, cont); 
              return sys->make_raw_boolean_compare(ctx, cur, rvalue, cont);
            } else if constexpr (std::is_same_v<output_type, double>) {
              change_compare_op cco(ctx, compare_operators::more_eq);
              auto rvalue = sys->make_raw_script_number(ctx, obj, cont);
              return sys->make_raw_number_compare(ctx, cur, rvalue, cont);
            } else if constexpr (std::is_same_v<output_type, void>) throw std::runtime_error("Could not compare effect function '" + std::string(name) + "'");
            else throw std::runtime_error("Found compare pattern but return type '" + std::string(type_name<output_type>()) + "' is not supported, for objects and strings use function 'equals_to'");
          }
          
          return cur;
        }, [] () { 
          constexpr bool is_independent_func = !is_member_v<Th, F>;
          return std::string(name) + " I: " + std::string(type_name<Th>()) + 
                                     " O: " + std::string(type_name<output_type>()) + 
                                     " CArgs: " + make_function_args_string<F, get_function_argument_count<F>(), is_independent_func>();
        }
      };
      
      register_function<Th, name>(std::move(ifd));
    }
    
    template <typename T, typename F, typename R, const char* name> // тут по идее мы свой тип должны создать
    void system::register_function(const std::function<interface*(system::view, const sol::object &, container::delayed_initialization<F>)> &user_func) {
      static_assert(std::is_base_of_v<interface, F>);
      using output_type = R;
      constexpr size_t script_type = get_script_type<output_type>();
      constexpr bool no_iterator = false;
      
      init_func_data ifd{
        script_type, type_id<T>(), type_id<output_type>(), UNDEFINED_ARGS_COUNT,
        [user_func] (system* sys, init_context* ctx, const sol::object &obj, container* cont) {
          constexpr size_t script_type = get_script_type<output_type>();
          check_script_types(name, ctx->script_type, script_type);
          ctx->current_size += sizeof(F);
          container::delayed_initialization<F> init(nullptr);
          if (cont != nullptr) {
            const size_t offset = cont->add_delayed<F>();
            init = cont->get_init<F>(offset);
          }
          // тут у нас тоже может быть вполне блок с итератором, что если его не будет?
          // функция траверс сломается, надо бы соединить следующую и эту функции
          // а что если мы укажем дополнительную блочную функцию? она вызывается только при смене контекста
          // что должно происходить только в других блочных функциях, блочные функции зависят от приенения *_raw_* функций
          //change_block_function cbf(ctx, sys->get_init_function<T>(name));
          change_current_function_name ccfn(ctx, name);
          change_compare_op cco(ctx, compare_operators::more_eq);
          system::view v(sys, ctx, cont, no_iterator);
          return user_func(v, obj, std::move(init));
        }, [] () { 
          return std::string(name) + " I: " + std::string(type_name<T>()) + 
                                     " O: " + std::string(type_name<output_type>()) + 
                                     " F: " + std::string(type_name<F>());
        }
      };
      
      register_function<T, name>(std::move(ifd));
    }
    
    template <typename T, typename F, typename R, const char* name>
    void system::register_iterator() {
      static_assert(std::is_base_of_v<interface, F>);
      using output_type = R;
      constexpr size_t script_type = get_script_type<output_type>();
      
      init_func_data ifd{
        script_type, type_id<T>(), type_id<output_type>(), FUNCTION_ITERATOR,
        [] (system* sys, init_context* ctx, const sol::object &obj, container* cont) -> interface* {
          constexpr size_t script_type = get_script_type<output_type>();
          check_script_types(name, ctx->script_type, script_type);
          using function_type = F;
          
          const auto sol_type = obj.get_type();
          if (sol_type != sol::type::table) throw std::runtime_error("iterator function " + std::string(name) + " expected table as input");
          
          ctx->current_size += sizeof(function_type);
          container::delayed_initialization<function_type> init(nullptr);
          if (cont != nullptr) {
            const size_t offset = cont->add_delayed<function_type>();
            init = cont->get_init<function_type>(offset);
          }
          
          // указываем текущую блочную функцию (возможно вообще имеет смысл это дело както отдельно от всего сделать)
          // блочная функция имеет смысл только для арифметических и логических итераторов (и то возможно не для всех)
          change_block_function cbf(ctx, sys->get_init_function<T>(name));
          change_block_name cbn(ctx, name);
          change_current_function_name ccfn(ctx, name);
          auto childs = sys->table_traverse(ctx, obj, cont);
          
          interface* cur = nullptr;
          if (init.valid()) cur = init.init(childs);
          return cur;
        }, [] () { 
          return std::string(name) + " I: " + std::string(type_name<T>()) + 
                                     " O: " + std::string(type_name<output_type>()) + 
                                     " F: " + std::string(type_name<F>());
        }
      };
      
      register_function<T, name>(std::move(ifd));
    }
    
    template <typename T, typename F, typename R, const char* name>
    void system::register_iterator(const std::function<interface*(system::view, const sol::object &, container::delayed_initialization<F>)> &user_func) {
      static_assert(std::is_base_of_v<interface, F>);
      using output_type = R;
      constexpr size_t script_type = get_script_type<output_type>();
      constexpr bool iterator = true;
      
      init_func_data ifd{
        script_type, type_id<T>(), type_id<output_type>(), FUNCTION_ITERATOR,
        [user_func] (system* sys, init_context* ctx, const sol::object &obj, container* cont) -> interface* {
          constexpr size_t script_type = get_script_type<output_type>();
          check_script_types(name, ctx->script_type, script_type);
          
          using function_type = F;
          ctx->current_size += sizeof(function_type);
          container::delayed_initialization<function_type> init(nullptr);
          if (cont != nullptr) {
            const size_t offset = cont->add_delayed<function_type>();
            init = cont->get_init<function_type>(offset);
          }
//           change_block_function cbf(ctx, sys->get_init_function<T>(name));
//           change_block_name cbn(ctx, name);
          change_current_function_name ccfn(ctx, name);
          change_compare_op cco(ctx, ctx->compare_operator);
          system::view v(sys, ctx, cont, iterator);
          return user_func(v, obj, std::move(init));
        }, [] () { 
          return std::string(name) + " I: " + std::string(type_name<T>()) + 
                                     " O: " + std::string(type_name<output_type>()) + 
                                     " F: " + std::string(type_name<F>());
        }
      };
      
      register_function<T, name>(std::move(ifd));
    }
    
#define EVERY_FUNC_INIT                              \
  ctx->current_size += sizeof(function_type);        \
  container::delayed_initialization<function_type> init(nullptr); \
  if (cont != nullptr) {                             \
    const size_t offset = cont->add_delayed<function_type>(); \
    init = cont->get_init<function_type>(offset);    \
  }                                                  \
  interface* cond = nullptr;                         \
  const sol::table t = obj.as<sol::table>();         \
  if (const auto proxy = t["condition"]; proxy.valid()) { \
    cond = sys->make_raw_script_boolean(ctx, proxy, cont); \
  }                                                  \
  auto childs = sys->table_traverse(ctx, obj, cont); \
  if (init.valid()) final_int = init.init(cond, childs); \
    
    // какое название?
    template <typename Th, typename F, F f, const char* name>
    void system::register_every() {
      using func_t = std::function<bool(const object &obj)>;
      static_assert(std::is_invocable_v<F, Th, func_t>, 
                    "Function must be invocable with arguments: script object and predicate function. "
                    "The function must enterupt execution when predicate function returns 'false'");
      
      using output_type = object;
      constexpr size_t script_type = get_script_type<output_type>();
      static_assert(script_type != script_types::object && script_type != script_types::string);
      
      init_func_data ifd{
        script_type, type_id<Th>(), EVERY_RETURN_TYPE, FUNCTION_ITERATOR,
        [] (system* sys, init_context* ctx, const sol::object &obj, container* cont) {
//           using function_type = typename std::conditional<
//             script_type == script_types::effect, 
//             scripted_iterator_every_effect<T, Th, F, f, name>,
//             typename std::conditional<
//               script_type == script_types::condition, 
//               scripted_iterator_every_logic<T, Th, F, f, name>, 
//               scripted_iterator_every_numeric<T, Th, F, f, name>
//             >::type
//           >::type;
//           
//           constexpr size_t script_type = get_script_type<output_type>();
//           check_script_types(name, ctx->script_type, script_type);
          
          const auto sol_type = obj.get_type();
          if (sol_type != sol::type::table) throw std::runtime_error("Function '" + std::string(name) + "' expected table as input");
          const sol::table t = obj.as<sol::table>();
          
//           change_block_function cbf(ctx, sys->get_init_function<T>(name));
//           change_block_name cbn(ctx, name);
          change_current_function_name ccfn(ctx, name);
          
          interface* final_int = nullptr;
          if (ctx->script_type == script_types::effect) {
            using function_type = scripted_iterator_every_effect<Th, F, f, name>;
            
            EVERY_FUNC_INIT
            
          } else if (ctx->script_type == script_types::condition) {
            using function_type = scripted_iterator_every_logic<Th, F, f, name>;
            
            EVERY_FUNC_INIT
            
          } else if (ctx->script_type == script_types::numeric) {
            using function_type = scripted_iterator_every_numeric<Th, F, f, name>;
            
            EVERY_FUNC_INIT
            
          } else throw std::runtime_error("Cannot use '" + std::string(name) + "' in string or object script");
          
          return final_int;
        }, [] () { 
          return std::string(name) + " I: " + std::string(type_name<Th>()) + 
                                     " O: bool, double or nothing ";
        }
      };
      
      using final_Th = typename std::conditional<std::is_same_v<clear_type_t<Th>, void>, void, Th>::type;
      register_function<final_Th, name>(std::move(ifd));
    }
    
#undef EVERY_FUNC_INIT
    
    template <typename Th, typename F, F f, const char* name>
    void system::register_has() {
      using func_t = std::function<bool(const object &obj)>;
      static_assert(std::is_invocable_v<F, Th, func_t>, 
                    "Function must be invocable with arguments: script object and predicate function. "
                    "The function must enterupt execution when predicate function returns 'false'");
      
      using output_type = double;
      constexpr size_t script_type = get_script_type<output_type>();
      
      init_func_data ifd{
        script_type, type_id<Th>(), type_id<output_type>(), FUNCTION_ITERATOR,
        [] (system* sys, init_context* ctx, const sol::object &obj, container* cont) {
          using function_type = scripted_iterator_has<Th, F, f, name>;
          
          constexpr size_t script_type = get_script_type<output_type>();
          check_script_types(name, ctx->script_type, script_type);
          
          const auto sol_type = obj.get_type();
          if (sol_type != sol::type::table) throw std::runtime_error("Function '" + std::string(name) + "' expected table as input");
          
          ctx->current_size += sizeof(function_type);
          container::delayed_initialization<function_type> init(nullptr);
          if (cont != nullptr) {
            const size_t offset = cont->add_delayed<function_type>();
            init = cont->get_init<function_type>(offset);
          }
          
//           change_block_function cbf(ctx, sys->get_init_function<T>(name));
//           change_block_name cbn(ctx, name);
          change_current_function_name ccfn(ctx, name);
          
          interface* percentage = nullptr;
          interface* max_count = nullptr;
          const sol::table t = obj.as<sol::table>();
          if (const auto proxy = t["percentage"]; proxy.valid()) {
            percentage = sys->make_raw_script_number(ctx, proxy, cont);
          } else if (const auto proxy = t["max_count"]; proxy.valid()) {
            max_count = sys->make_raw_script_number(ctx, proxy, cont);
          }

          auto childs = sys->table_traverse(ctx, obj, cont);
          
          interface* cur = nullptr;
          if (init.valid()) cur = init.init(max_count, percentage, childs);
          return cur;
        }, [] () { 
          return std::string(name) + " I: " + std::string(type_name<Th>()) + 
                                     " O: " + std::string(type_name<output_type>());
        }
      };
      
      using final_Th = typename std::conditional<std::is_same_v<clear_type_t<Th>, void>, void, Th>::type;
      register_function<final_Th, name>(std::move(ifd));
    }
    
    template <typename Th, typename F, F f, const char* name>
    void system::register_random() {
      using func_t = std::function<bool(const object &obj)>;
      static_assert(std::is_invocable_v<F, Th, func_t>, 
                    "Function must be invocable with arguments: script object and predicate function. "
                    "The function must enterupt execution when predicate function returns 'false'");
      
      using output_type = object;
      constexpr size_t script_type = get_script_type<output_type>();
      static_assert(script_type != script_types::object && script_type != script_types::string);
      
      init_func_data ifd{
        script_type, type_id<Th>(), type_id<output_type>(), FUNCTION_ITERATOR,
        [] (system* sys, init_context* ctx, const sol::object &obj, container* cont) {
          using function_type = scripted_iterator_random<Th, F, f, name>;
          
          constexpr size_t script_type = get_script_type<output_type>();
          check_script_types(name, ctx->script_type, script_type);
          
          const auto sol_type = obj.get_type();
          if (sol_type != sol::type::table) throw std::runtime_error("Function '" + std::string(name) + "' expected table as input");
          
          ctx->current_size += sizeof(function_type);
          container::delayed_initialization<function_type> init(nullptr);
          if (cont != nullptr) {
            const size_t offset = cont->add_delayed<function_type>();
            init = cont->get_init<function_type>(offset);
          }
          
//           change_block_function cbf(ctx, sys->get_init_function<T>(name));
//           change_block_name cbn(ctx, name);
          change_current_function_name ccfn(ctx, name);
          
          interface* condition = nullptr;
          interface* weight = nullptr;
          sol::table t = obj.as<sol::table>();
          if (const auto proxy = t["condition"]; proxy.valid()) {
            condition = sys->make_raw_script_boolean(ctx, proxy, cont);
          } 
          
          if (const auto proxy = t["weight"]; proxy.valid()) {
            weight = sys->make_raw_script_number(ctx, proxy, cont);
          }
          
          // костыль, он вообще будет работать?
          const sol::object cond_obj = t["condition"];
          t["condition"] = sol::nil;
          
          //auto childs = sys->table_traverse(ctx, obj, cont);
          // проблема в том что мне нужно отключить в методах проверку condition, иначе он еще раз проверит
          // возможно имеет смысл просто удолить этот кондишен, можно использовать другую функцию
          auto childs = sys->make_raw_script_any(ctx, obj, cont); // возвращает одного ребенка
          
          t["condition"] = cond_obj;
          
          const size_t state = sys->get_next_random_state();
          interface* cur = nullptr;
          if (init.valid()) cur = init.init(state, condition, weight, childs);
          return cur;
        }, [] () { 
          return std::string(name) + " I: " + std::string(type_name<Th>()) + 
                                     " O: " + std::string(type_name<output_type>());
        }
      };
      
      using final_Th = typename std::conditional<std::is_same_v<clear_type_t<Th>, void>, void, Th>::type;
      register_function<final_Th, name>(std::move(ifd));
    }
    
    template <typename Th, typename F, F f, const char* name>
    void system::register_view() {
      using func_t = std::function<bool(const object &obj)>;
      static_assert(std::is_invocable_v<F, Th, func_t>, 
                    "Function must be invocable with arguments: script object and predicate function. "
                    "The function must enterupt execution when predicate function returns 'false'");
      
      using output_type = object;
      constexpr size_t script_type = get_script_type<output_type>();
      static_assert(script_type != script_types::string);
      
      init_func_data ifd{
        script_type, type_id<Th>(), type_id<output_type>(), FUNCTION_ITERATOR,
        [] (system* sys, init_context* ctx, const sol::object &obj, container* cont) {
          using function_type = scripted_iterator_view<Th, F, f, name>;
          
          constexpr size_t script_type = get_script_type<output_type>();
          check_script_types(name, ctx->script_type, script_type);
          
          const auto sol_type = obj.get_type();
          if (sol_type != sol::type::table) throw std::runtime_error("Function '" + std::string(name) + "' expected table as input");
          
          ctx->current_size += sizeof(function_type);
          container::delayed_initialization<function_type> init(nullptr);
          if (cont != nullptr) {
            const size_t offset = cont->add_delayed<function_type>();
            init = cont->get_init<function_type>(offset);
          }
          
          // что мы тут ожидаем? ожидаем таблицу с таблицами с одним элементом + наверное "reduce_value" для значения по умолчанию
          // может вообще везде сократить количество ошибок? все таки это не ФАТАЛ эррор, хотя с другой стороны 
          // банальная ошибка которую мы можем не заметить
          change_current_function_name ccfn(ctx, name);
          interface* default_value = nullptr;
          interface* begin_childs = nullptr;
          interface* cur_child = nullptr;
          const auto t = obj.as<sol::table>();
          for (const auto &pair : t) {
            if (pair.first.get_type() == sol::type::string) {
              const auto str = obj.as<std::string_view>();
              if (str != "reduce_value") { throw std::runtime_error("Unexpected lvalue string '" + std::string(str) + "' in function '" + std::string(name) + "', expected only 'reduce_value'"); }
              change_expected_type cet(ctx, SIZE_MAX);
              default_value = sys->make_raw_script_object(ctx, pair.second, cont);
              continue;
            }
            
            if (pair.first.get_type() == sol::type::number) {
              if (pair.second.get_type() != sol::type::table)
                throw std::runtime_error("Unexpected rvalue of type '" + std::string(detail::get_sol_type_name(pair.second.get_type())) + "', function '" + std::string(name) + "' expected table with tables");
              
              // тут ожидаем строго слева название функции
              size_t counter = 0;
              const auto inner_t = pair.second.as<sol::table>();
              for (const auto &pair : inner_t) {
                if (ctx->script_type == script_types::effect) {
                  // если не строка то в эффекте ожидаем просто таблицу с перечислением действий вместо reduce
                }
                
                if (counter >= 1) throw std::runtime_error("Function '" + std::string(name) + "' expects inner table to consist of one function ");
                
                interface* local = nullptr;
                if (ctx->script_type != script_types::effect && pair.first.get_type() != sol::type::string)
                  throw std::runtime_error("Unexpected lvalue of type '" + std::string(detail::get_sol_type_name(pair.first.get_type())) + "' in function '" + std::string(name) + "', expected string ");
                
                if (ctx->script_type == script_types::effect && pair.first.get_type() != sol::type::string) {
                  // ождаем таблицу с эффектами
                  if (pair.second.get_type() != sol::type::table) 
                    throw std::runtime_error("If current script type is effect script, then instead of function 'reduce' the table with effect functions is expected, context '" + std::string(name) + "'");
                  
                  local = sys->make_raw_script_effect(ctx, pair.second, cont);
                } else {
                  const auto str = pair.first.as<std::string_view>();
                  if (const auto itr = detail::view_allowed_funcs.find(str); itr == detail::view_allowed_funcs.end()) 
                    throw std::runtime_error("Function '" + std::string(name) + "' expects only transform, filter, reduce, take and drop functions in inner table, got '" + std::string(str) + "'");
                  
                  if (str == "reduce" && ctx->script_type == script_types::effect) 
                    throw std::runtime_error("Function 'reduce' is not allowed in inner table in effect scripts, context '" + std::string(name) + "'");
                  
                  const auto type_itr = sys->func_map.find(type_id<void>());
                  assert(type_itr != sys->func_map.end());
                  
                  const auto itr = type_itr->second.find(str);
                  if (itr == type_itr->second.end()) throw std::runtime_error("Could not find function '" + std::string(str) + "', context '" + std::string(name) + "'");
                  
                  local = itr->second.func(sys, ctx, pair.second, cont);
                }
                
                ++counter;
                if (begin_childs == nullptr) begin_childs = local;
                if (cur_child != nullptr) cur_child->next = local;
                cur_child = local;
              }
              continue;
            }
            
            throw std::runtime_error("Unexpected lvalue of type '" + std::string(detail::get_sol_type_name(pair.first.get_type())) + "', function '" + std::string(name) + "' expected table or 'reduce_value'");
          }
          
          return init.init(default_value, begin_childs);
        }, [] () { 
          return std::string(name) + " I: " + std::string(type_name<Th>()) + 
                                     " O: " + std::string(type_name<output_type>());
        }
      };
      
      using final_Th = typename std::conditional< std::is_same_v<clear_type_t<Th>, void>, void, Th >::type;
      register_function<final_Th, name>(std::move(ifd));
    }
    
#define DEFAULT_ALIGMENT 8
    
    // возвращать будем легкие обертки вокруг interface* (тип script::number и проч)
    // + нужно будет возвращать скрипты по названию, скрипты которые вставляются в другие скрипты вызываются иначе
    // + могут пригодится флаги
    template <typename T>
    condition system::create_condition(const sol::object &obj, const std::string_view &script_name, const std::string_view &save_name) {
      if (!obj.valid()) return condition();
      
      const size_t id = type_id<T>() == type_id<object>() ? SIZE_MAX : type_id<T>();
      init_context ctx;
      ctx.root = id;
      ctx.current_type = id;
      ctx.script_name = script_name;
      ctx.script_type = script_types::condition;
      
      // мне конечно по прежнему не нравится этот двойной обход, но наверное тут ничего не поделаешь
      make_raw_script_boolean(&ctx, obj, nullptr);
      auto cont = containers_pool.create(ctx.current_size, DEFAULT_ALIGMENT);
      const size_t mem = ctx.current_size;
      ctx.current_size = 0;
      auto i = make_raw_script_boolean(&ctx, obj, cont);
      containers.emplace_back(i, cont);
      assert(mem == ctx.current_size);
      
      if (!save_name.empty()) { scripts.emplace(save_name, std::make_pair(type_id<bool>(), i)); }
      
      return condition(i);
    }
    
    template <typename T>
    number system::create_number(const sol::object &obj, const std::string_view &script_name, const std::string_view &save_name) {
      if (!obj.valid()) return number();
      
      const size_t id = type_id<T>() == type_id<object>() ? SIZE_MAX : type_id<T>();
      init_context ctx;
      ctx.root = id;
      ctx.current_type = id;
      ctx.script_name = script_name;
      ctx.script_type = script_types::numeric;
      
      make_raw_script_number(&ctx, obj, nullptr);
      auto cont = containers_pool.create(ctx.current_size, DEFAULT_ALIGMENT);
      const size_t mem = ctx.current_size;
      ctx.current_size = 0;
      auto i = make_raw_script_number(&ctx, obj, cont);
      containers.emplace_back(i, cont);
      assert(mem == ctx.current_size);
      
      if (!save_name.empty()) { scripts.emplace(save_name, std::make_pair(type_id<double>(), i)); }
      
      return number(i);
    }
    
    template <typename T>
    string system::create_string(const sol::object &obj, const std::string_view &script_name, const std::string_view &save_name) {
      if (!obj.valid()) return string();
      
      const size_t id = type_id<T>() == type_id<object>() ? SIZE_MAX : type_id<T>();
      init_context ctx;
      ctx.root = id;
      ctx.current_type = id;
      ctx.script_name = script_name;
      ctx.script_type = script_types::string;
      
      make_raw_script_string(&ctx, obj, nullptr);
      auto cont = containers_pool.create(ctx.current_size, DEFAULT_ALIGMENT);
      const size_t mem = ctx.current_size;
      ctx.current_size = 0;
      auto i = make_raw_script_string(&ctx, obj, cont);
      containers.emplace_back(i, cont);
      assert(mem == ctx.current_size);
      
      if (!save_name.empty()) { scripts.emplace(save_name, std::make_pair(type_id<std::string_view>(), i)); }
      
      return string(i);
    }
    
    template <typename T>
    effect system::create_effect(const sol::object &obj, const std::string_view &script_name, const std::string_view &save_name) {
      if (!obj.valid()) return effect();
      
      const size_t id = type_id<T>() == type_id<object>() ? SIZE_MAX : type_id<T>();
      init_context ctx;
      ctx.root = id;
      ctx.current_type = id;
      ctx.script_name = script_name;
      ctx.script_type = script_types::effect;
      
      make_raw_script_effect(&ctx, obj, nullptr);
      auto cont = containers_pool.create(ctx.current_size, DEFAULT_ALIGMENT);
      const size_t mem = ctx.current_size;
      ctx.current_size = 0;
      // можем тут создать специальную структуру с данными о скрипте
      auto i = make_raw_script_effect(&ctx, obj, cont);
      containers.emplace_back(i, cont);
      assert(mem == ctx.current_size);
      
      if (!save_name.empty()) { scripts.emplace(save_name, std::make_pair(type_id<void>(), i)); }
      
      return effect(i);
    }
    
    template <typename T>
    const system::init_func_data* system::get_init_function(const std::string_view &name) const {
      return get_init_function(type_id<T>(), name);
    }
    
    template <typename T, const char* name>
    void system::register_function(init_func_data&& data) {
      register_function(type_id<T>(), type_name<T>(), name, std::move(data));
    }
  }
}

#endif

