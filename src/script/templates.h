#ifndef DEVILS_ENGINE_SCRIPT_TEMPLATES_H
#define DEVILS_ENGINE_SCRIPT_TEMPLATES_H

#include <type_traits>
#include "type_traits.h"
#include "interface.h"
#include "context.h"

// нам в темплейтных функциях недостаточно передать просто тип
// нас скорее интересует тип с модификатором (указатель или handle)
// здесь Т это просто тип (для указания откуда функция)
// видимо нужно добавить еще и тип получаемого объекта

namespace devils_engine {
  namespace script {
    template <typename F, F f, const char* name>
    class basic_function_no_arg final : public interface {
    public:
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    };
    
    template <typename F, F f, const char* name>
    class basic_function_scripted_arg final : public interface {
    public:
      basic_function_scripted_arg(const interface* arg) noexcept;
      ~basic_function_scripted_arg() noexcept;
      
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      const interface* arg;
    };
    
    template <typename F, F f, const char* name>
    class basic_function_scripted_args final : public interface {
    public:
      basic_function_scripted_args(const interface* args) noexcept;
      ~basic_function_scripted_args() noexcept;

      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      //std::array<const interface*, args_num> args;
      const interface* args;
    };
    
    template <typename Th, typename F, F f, const char* name>
    class scripted_function_no_arg final : public interface {
    public:
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    };
    
//     template <typename Th, typename F, F f, const char* name>
//     class scripted_function_handle_arg final : public interface {
//     public:
//       struct object process(context* ctx) const override;
//       void draw(context* ctx) const override;
//       size_t get_type_id() const override;
//       std::string_view get_name() const override;
//     };
    
    template <typename Th, typename F, F f, const char* name>
    class scripted_function_one_arg final : public interface {
    public:
      using input_type = typename std::conditional< !is_member_v<Th, F>, final_arg_type<F, 1>, final_arg_type<F, 0> >::type;

      scripted_function_one_arg(const interface* val) noexcept;
      ~scripted_function_one_arg() noexcept;

      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      const interface* val;
    };
    
//     template <typename T, typename Th, typename F, F f, const char* name>
//     class scripted_function_handle_one_arg final : public interface {
//     public:
//       static constexpr bool is_member = std::is_same_v<T, function_member_of<F>>;
//       using input_type = typename std::conditional< !is_member, final_arg_type<F, 1>, final_arg_type<F, 0> >::type;
// 
//       scripted_function_handle_one_arg(const interface* val) noexcept;
//       ~scripted_function_handle_one_arg() noexcept;
// 
//       struct object process(context* ctx) const override;
//       void draw(context* ctx) const override;
//       size_t get_type_id() const override;
//       std::string_view get_name() const override;
//     private:
//       const interface* val;
//     };
    
    template <typename Th, typename F, F f, const char* name, int64_t... args>
    class scripted_function_const_args final : public interface {
    public:
      static_assert(std::is_invocable_v<F, Th, decltype(args)...>);
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    };
    
//     template <typename T, typename Th, typename F, F f, const char* name, int64_t... args>
//     class scripted_function_handle_const_args final : public interface {
//     public:
//       static_assert(std::is_invocable_v<F, Th, decltype(args)...>);
//       struct object process(context* ctx) const override;
//       void draw(context* ctx) const override;
//       size_t get_type_id() const override;
//       std::string_view get_name() const override;
//     };
    
    template <typename Th, typename F, F f, const char* name, typename... Args>
    class scripted_function_args final : public interface {
    public:
      static_assert(std::is_invocable_v<F, Th, const Args &...>);
      scripted_function_args(Args&&... args) noexcept;
      scripted_function_args(const std::tuple<Args...> &args) noexcept;
      ~scripted_function_args() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      std::tuple<Args...> args;
    };
    
//     template <typename T, typename Th, typename F, F f, const char* name, typename... Args>
//     class scripted_function_handle_args final : public interface {
//     public:
//       static_assert(std::is_invocable_v<F, Th, const Args &...>);
//       scripted_function_handle_args(Args&&... args) noexcept;
//       scripted_function_handle_args(const std::tuple<Args...> &args) noexcept;
//       ~scripted_function_handle_args() noexcept;
//       struct object process(context* ctx) const override;
//       void draw(context* ctx) const override;
//       size_t get_type_id() const override;
//       std::string_view get_name() const override;
//     private:
//       std::tuple<Args...> args;
//     };
    
    template <typename Th, typename F, F f, const char* name>
    class scripted_function_scripted_args final : public interface {
    public:
      //scripted_function_scripted_args(const interface* arg...) noexcept;
      //scripted_function_scripted_args(std::initializer_list<const interface*> args) noexcept;
      scripted_function_scripted_args(const interface* args) noexcept;
      ~scripted_function_scripted_args() noexcept;

      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      //std::array<const interface*, args_num> args;
      const interface* args;
    };
    
    template <typename Th, typename F, F f, const char* name>
    class scripted_iterator_every_numeric final : public interface {
    public:
      scripted_iterator_every_numeric(const interface* condition, const interface* childs) noexcept;
      ~scripted_iterator_every_numeric() noexcept;
      
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      const interface* condition;
      const interface* childs;
    };
    
    template <typename Th, typename F, F f, const char* name>
    class scripted_iterator_every_effect final : public interface {
    public:
      scripted_iterator_every_effect(const interface* condition, const interface* childs) noexcept;
      ~scripted_iterator_every_effect() noexcept;
      
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      const interface* condition;
      const interface* childs;
    };
    
    template <typename Th, typename F, F f, const char* name>
    class scripted_iterator_every_logic final : public interface {
    public:
      scripted_iterator_every_logic(const interface* condition, const interface* childs) noexcept;
      ~scripted_iterator_every_logic() noexcept;
      
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      const interface* condition;
      const interface* childs;
    };
    
    template <typename Th, typename F, F f, const char* name>
    class scripted_iterator_has final : public interface {
    public:
      scripted_iterator_has(const interface* childs) noexcept;
      scripted_iterator_has(const interface* max_count, const interface* percentage, const interface* childs) noexcept;
      ~scripted_iterator_has() noexcept;
      
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      const interface* max_count;
      const interface* percentage;
      const interface* childs;
    };
    
    template <typename Th, typename F, F f, const char* name>
    class scripted_iterator_random final : public interface {
    public:
      scripted_iterator_random(const size_t &state, const interface* condition, const interface* weight, const interface* childs) noexcept;
      ~scripted_iterator_random() noexcept;
      
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      size_t state;
      const interface* condition;
      const interface* weight;
      const interface* childs;
    };
    
    /* ================================================================================================================================================ */
    // implementation
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    /* ================================================================================================================================================ */
    
    template <typename F, F f, const char* name>
    struct object basic_function_no_arg<F, f, name>::process(context*) const {
      using output_type = function_result_type<F>;
      if constexpr (!std::is_same_v<output_type, void>) return object(std::invoke(f));
      else std::invoke(f);
      return object();
    }
    
    template <typename F, F f, const char* name>
    void basic_function_no_arg<F, f, name>::draw(context* ctx) const {
      draw_data dd(ctx);
      dd.function_name = name;
      using output_type = function_result_type<F>;
      if constexpr (!std::is_same_v<output_type, void>) dd.value = process(ctx);
      ctx->draw(&dd);
    }
    
    template <typename F, F f, const char* name>
    size_t basic_function_no_arg<F, f, name>::get_type_id() const { return type_id<object>(); }
    template <typename F, F f, const char* name>
    std::string_view basic_function_no_arg<F, f, name>::get_name() const { return name; }
    
    template <typename F, F f, const char* name>
    basic_function_scripted_arg<F, f, name>::basic_function_scripted_arg(const interface* arg) noexcept : arg(arg) {}
    template <typename F, F f, const char* name>
    basic_function_scripted_arg<F, f, name>::~basic_function_scripted_arg() noexcept { arg->~interface(); }
    
    template <typename F, F f, const char* name>
    struct object basic_function_scripted_arg<F, f, name>::process(context* ctx) const {
      using output_type = function_result_type<F>;
      using input_type = function_argument_type<F, 0>;
      const auto obj = arg->process(ctx);
      if constexpr (!std::is_same_v<output_type, void>) return object(std::invoke(f, obj.get<input_type>()));
      else std::invoke(f, obj.get<input_type>());
      return object();
    }
    
    template <typename F, F f, const char* name>
    void basic_function_scripted_arg<F, f, name>::draw(context* ctx) const {
      draw_data dd(ctx);
      dd.function_name = name;
      using output_type = function_result_type<F>;
      if constexpr (!std::is_same_v<output_type, void>) dd.value = process(ctx);
      ctx->draw(&dd);
    }
    
    template <typename F, F f, const char* name>
    size_t basic_function_scripted_arg<F, f, name>::get_type_id() const { return type_id<object>(); }
    template <typename F, F f, const char* name>
    std::string_view basic_function_scripted_arg<F, f, name>::get_name() const { return name; }
    
    template <typename F, F f, size_t N, size_t curr, typename... Args>
    static object call_func(context* ctx, const interface* current, Args&&... args) {
      using output_type = function_result_type<F>;
      if constexpr (!std::is_same_v<output_type, void>) {
        if constexpr (curr == N) return object(std::invoke(f, std::forward<Args>(args)...));
        //if constexpr (curr == N) return object((*f)(std::forward<Args>(args)...));
        else {
          using curr_type = std::remove_reference_t<function_argument_type<F, curr>>;
          auto obj = current->process(ctx);
          auto arg = obj.template get<curr_type>();
          return call_func<F, f, N, curr+1>(ctx, current->next, std::forward<Args>(args)..., arg);
        }
      } else {
        if constexpr (curr == N) std::invoke(f, std::forward<Args>(args)...);
        //if constexpr (curr == N) (*f)(std::forward<Args>(args)...);
        else {
          using curr_type = std::remove_reference_t<function_argument_type<F, curr>>;
          auto obj = current->process(ctx);
          auto arg = obj.template get<curr_type>();
          call_func<F, f, N, curr+1>(ctx, current->next, std::forward<Args>(args)..., arg);
        }
      }
      return object();
    }
    
    template <typename F, F f, const char* name>
    basic_function_scripted_args<F, f, name>::basic_function_scripted_args(const interface* args) noexcept : args(args) {}
    template <typename F, F f, const char* name>
    basic_function_scripted_args<F, f, name>::~basic_function_scripted_args() noexcept {
      for (auto cur = args; cur != nullptr; cur = cur->next) { cur->~interface(); } 
    }
    
    template <typename F, F f, const char* name>
    struct object basic_function_scripted_args<F, f, name>::process(context* ctx) const {
      using output_type = function_result_type<F>;
      constexpr size_t func_arg_count = get_function_argument_count<F>();
      if constexpr (!std::is_same_v<output_type, void>) return call_func<F, f, func_arg_count, 0>(ctx, args);
      else call_func<F, f, func_arg_count, 0>(ctx, args);
      return object();
    }
    
    template <typename F, F f, const char* name>
    void basic_function_scripted_args<F, f, name>::draw(context* ctx) const {
      draw_data dd(ctx);
      dd.function_name = name;
      using output_type = function_result_type<F>;
      if constexpr (!std::is_same_v<output_type, void>) dd.value = process(ctx);
      if (!ctx->draw(&dd)) return;
      // нужно ли пройтись по аргументам, скорее всего пригодится в будущем
    }
    
    template <typename F, F f, const char* name>
    size_t basic_function_scripted_args<F, f, name>::get_type_id() const { return type_id<object>(); }
    template <typename F, F f, const char* name>
    std::string_view basic_function_scripted_args<F, f, name>::get_name() const { return name; }
    
    template <typename Th, typename F, F f, const char* name>
    struct object scripted_function_no_arg<Th, F, f, name>::process(context* ctx) const {
      auto cur = ctx->current.get<Th>();
      using output_type = function_result_type<F>;
      if constexpr (!std::is_same_v<output_type, void>) return object(std::invoke(f, cur));
      else std::invoke(f, cur);
      return object();
    }
    
    template <typename Th, typename F, F f, const char* name>
    void scripted_function_no_arg<Th, F, f, name>::draw(context* ctx) const {
      draw_data dd(ctx);
      dd.function_name = name;
      using output_type = function_result_type<F>;
      if constexpr (!std::is_same_v<output_type, void>) dd.value = process(ctx);
      ctx->draw(&dd);
    }
    
    template <typename Th, typename F, F f, const char* name>
    size_t scripted_function_no_arg<Th, F, f, name>::get_type_id() const { return type_id<Th>(); }
    template <typename Th, typename F, F f, const char* name>
    std::string_view scripted_function_no_arg<Th, F, f, name>::get_name() const { return name; }
    
//     template <typename T, typename Th, typename F, F f, const char* name>
//     struct object scripted_function_handle_arg<T, Th, F, f, name>::process(context* ctx) const {
//       auto cur = ctx->current.get<Th>();
//       using ret_t = function_result_type<F>;
//       if constexpr (!std::is_same_v<ret_t, void>) {
//         const ret_t obj = std::invoke(f, cur);
//         return object(obj);
//       } else std::invoke(f, cur);
//       return object();
//     }
//     
//     template <typename T, typename Th, typename F, F f, const char* name>
//     void scripted_function_handle_arg<T, Th, F, f, name>::draw(context* ctx) const {
//       draw_data dd(ctx);
//       dd.function_name = name;
//       using ret_t = function_result_type<F>;
//       if constexpr (!std::is_same_v<ret_t, void>) dd.value = process(ctx);
//       ctx->draw(&dd);
//     }
//     
//     template <typename T, typename Th, typename F, F f, const char* name>
//     size_t scripted_function_handle_arg<T, Th, F, f, name>::get_type_id() const { return type_id<T>(); }
//     template <typename T, typename Th, typename F, F f, const char* name>
//     std::string_view scripted_function_handle_arg<T, Th, F, f, name>::get_name() const { return name; }

    template <typename Th, typename F, F f, const char* name>  
    scripted_function_one_arg<Th, F, f, name>::scripted_function_one_arg(const interface* val) noexcept : val(val) {}
    template <typename Th, typename F, F f, const char* name>  
    scripted_function_one_arg<Th, F, f, name>::~scripted_function_one_arg() noexcept { val->~interface(); }
  
    template <typename Th, typename F, F f, const char* name>
    struct object scripted_function_one_arg<Th, F, f, name>::process(context* ctx) const {
      auto cur = ctx->current.get<Th>();
      const auto input = val->process(ctx);
      using output_type = function_result_type<F>;
      if constexpr (!std::is_same_v<output_type, void>) return object(std::invoke(f, cur, input.get<input_type>()));
      else std::invoke(f, cur, input.get<input_type>());
      return object();
    }
    
    template <typename Th, typename F, F f, const char* name>
    void scripted_function_one_arg<Th, F, f, name>::draw(context* ctx) const {
      draw_data dd(ctx);
      dd.function_name = name;
      using output_type = function_result_type<F>;
      if constexpr (!std::is_same_v<output_type, void>) dd.value = process(ctx);
      dd.original = val->process(ctx);
      ctx->draw(&dd);
    }
    
    template <typename Th, typename F, F f, const char* name>
    size_t scripted_function_one_arg<Th, F, f, name>::get_type_id() const { return type_id<Th>(); }
    template <typename Th, typename F, F f, const char* name>
    std::string_view scripted_function_one_arg<Th, F, f, name>::get_name() const { return name; }
    
//     template <typename Th, typename F, F f, const char* name>  
//     scripted_function_handle_one_arg<Th, F, f, name>::scripted_function_handle_one_arg(const interface* val) noexcept : val(val) {}
//     template <typename Th, typename F, F f, const char* name>  
//     scripted_function_handle_one_arg<Th, F, f, name>::~scripted_function_handle_one_arg() noexcept { val->~interface(); }
// 
//     template <typename T, typename Th, typename F, F f, const char* name>  
//     struct object scripted_function_handle_one_arg<T, Th, F, f, name>::process(context* ctx) const {
//       auto cur = ctx->current.get<Th>();
//       const auto input = val->process(ctx);
//       using ret_t = function_result_type<F>;
//       if constexpr (!std::is_same_v<ret_t, void>) {
//         const ret_t obj = std::invoke(f, cur, input.get<input_type>());
//         return object(obj);
//       } else std::invoke(f, cur, input.get<input_type>());
//       return object();
//     }
//     
//     template <typename T, typename Th, typename F, F f, const char* name>  
//     void scripted_function_handle_one_arg<T, Th, F, f, name>::draw(context* ctx) const {
//       draw_data dd(ctx);
//       dd.function_name = name;
//       using ret_t = function_result_type<F>;
//       if constexpr (!std::is_same_v<ret_t, void>) dd.value = process(ctx);
//       dd.original = val->process(ctx);
//       ctx->draw(&dd);
//     }
//     
//     template <typename T, typename Th, typename F, F f, const char* name>
//     size_t scripted_function_handle_one_arg<T, Th, F, f, name>::get_type_id() const { return type_id<T>(); }
//     template <typename T, typename Th, typename F, F f, const char* name>
//     std::string_view scripted_function_handle_one_arg<T, Th, F, f, name>::get_name() const { return name; }
    
    template <typename Th, typename F, F f, const char* name, int64_t... args>
    struct object scripted_function_const_args<Th, F, f, name, args...>::process(context* ctx) const {
      auto cur = ctx->current.get<Th>();
      using output_type = function_result_type<F>;
      if constexpr (!std::is_same_v<output_type, void>) return object(std::invoke(f, cur, args...));
      else std::invoke(f, cur, args...);
      return object();
    }
    
    template <typename Th, typename F, F f, const char* name, int64_t... args>
    void scripted_function_const_args<Th, F, f, name, args...>::draw(context* ctx) const {
      draw_data dd(ctx);
      dd.function_name = name;
      using output_type = function_result_type<F>;
      if constexpr (!std::is_same_v<output_type, void>) dd.value = process(ctx);
      ctx->draw(&dd);
    }
    
    template <typename Th, typename F, F f, const char* name, int64_t... args>
    size_t scripted_function_const_args<Th, F, f, name, args...>::get_type_id() const { return type_id<Th>(); }
    template <typename Th, typename F, F f, const char* name, int64_t... args>
    std::string_view scripted_function_const_args<Th, F, f, name, args...>::get_name() const { return name; }
    
//     template <typename T, typename Th, typename F, F f, const char* name, int64_t... args>
//     struct object scripted_function_handle_const_args<T, Th, F, f, name, args...>::process(context* ctx) const {
//       auto cur = ctx->current.get<Th>();
//       using ret_t = function_result_type<F>;
//       if constexpr (!std::is_same_v<ret_t, void>) {
//         const ret_t obj = (*f)(cur, args...);
//         return object(obj);
//       } else (*f)(cur, args...);
//       
//       return object();
//     }
//     
//     template <typename T, typename Th, typename F, F f, const char* name, int64_t... args>
//     void scripted_function_handle_const_args<T, Th, F, f, name, args...>::draw(context* ctx) const {
//       draw_data dd(ctx);
//       dd.function_name = name;
//       using ret_t = function_result_type<F>;
//       if constexpr (!std::is_same_v<ret_t, void>) dd.value = process(ctx);
//       ctx->draw(&dd);
//     }
//     
//     template <typename T, typename Th, typename F, F f, const char* name, int64_t... args>
//     size_t scripted_function_handle_const_args<T, Th, F, f, name, args...>::get_type_id() const { return type_id<T>(); }
//     template <typename T, typename Th, typename F, F f, const char* name, int64_t... args>
//     std::string_view scripted_function_handle_const_args<T, Th, F, f, name, args...>::get_name() const { return name; }
    
    template <typename F, typename Th, typename Tuple, size_t... I>
    auto apply_tuple_impl(F f, Th cur, const Tuple &t, std::index_sequence<I...>) -> function_result_type<F> {
      return std::invoke(f, cur, std::get<I>(t)...);
    }
    
    template <size_t N, typename F, typename Th, typename Tuple, typename Indices = std::make_index_sequence<N>>
    auto apply_tuple(F f, Th cur, const Tuple &t) -> function_result_type<F> {
      return apply_tuple_impl(f, cur, t, Indices{});
    }
    
    template <typename Th, typename F, F f, const char* name, typename... Args>
    scripted_function_args<Th, F, f, name, Args...>::scripted_function_args(Args&&... args) noexcept : args(std::forward<Args>(args)...) {}
    template <typename Th, typename F, F f, const char* name, typename... Args>
    scripted_function_args<Th, F, f, name, Args...>::scripted_function_args(const std::tuple<Args...> &args) noexcept : args(args) {}
    template <typename Th, typename F, F f, const char* name, typename... Args>
    scripted_function_args<Th, F, f, name, Args...>::~scripted_function_args() noexcept {} // ничего?
    template <typename Th, typename F, F f, const char* name, typename... Args>
    struct object scripted_function_args<Th, F, f, name, Args...>::process(context* ctx) const {
      auto cur = ctx->current.get<Th>();
      using output_type = function_result_type<F>;
      if constexpr (!std::is_same_v<output_type, void>) return object(apply_tuple<std::tuple_size_v<decltype(args)>>(f, cur, args));
      else apply_tuple<std::tuple_size_v<decltype(args)>>(f, cur, args);
      return object();
    }
    
    template <typename Th, typename F, F f, const char* name, typename... Args>
    void scripted_function_args<Th, F, f, name, Args...>::draw(context* ctx) const {
      draw_data dd(ctx);
      dd.function_name = name;
      using output_type = function_result_type<F>;
      if constexpr (!std::is_same_v<output_type, void>) dd.value = process(ctx);
      ctx->draw(&dd);
    }
    
    template <typename Th, typename F, F f, const char* name, typename... Args>
    size_t scripted_function_args<Th, F, f, name, Args...>::get_type_id() const { return type_id<Th>(); }
    template <typename Th, typename F, F f, const char* name, typename... Args>
    std::string_view scripted_function_args<Th, F, f, name, Args...>::get_name() const { return name; }
    
//     template <typename T, typename Th, typename F, F f, const char* name, typename... Args>
//     scripted_function_handle_args<T, Th, F, f, name, Args...>::scripted_function_handle_args(Args&&... args) noexcept : 
//       args(std::forward<Args>(args)...) {}
//     template <typename T, typename Th, typename F, F f, const char* name, typename... Args>
//     scripted_function_handle_args<T, Th, F, f, name, Args...>::scripted_function_handle_args(const std::tuple<Args...> &args) noexcept : args(args) {}
//     template <typename T, typename Th, typename F, F f, const char* name, typename... Args>
//     scripted_function_handle_args<T, Th, F, f, name, Args...>::~scripted_function_handle_args() noexcept {} // ничего?
//     template <typename T, typename Th, typename F, F f, const char* name, typename... Args>
//     struct object scripted_function_handle_args<T, Th, F, f, name, Args...>::process(context* ctx) const {
//       auto cur = ctx->current.get<Th>();
//       using ret_t = function_result_type<F>;
//       if constexpr (!std::is_same_v<ret_t, void>) {
//         //const ret_t obj = std::apply(f, cur, args);
//         const ret_t obj = apply_tuple<std::tuple_size_v<decltype(args)>>(f, cur, args);
//         return object(obj);
//       } else apply_tuple<std::tuple_size_v<decltype(args)>>(f, cur, args);
//       
//       return object();
//     }
//     
//     template <typename T, typename Th, typename F, F f, const char* name, typename... Args>
//     void scripted_function_handle_args<T, Th, F, f, name, Args...>::draw(context* ctx) const {
//       draw_data dd(ctx);
//       dd.function_name = name;
//       using ret_t = function_result_type<F>;
//       if constexpr (!std::is_same_v<ret_t, void>) dd.value = process(ctx);
//       ctx->draw(&dd);
//     }
//     
//     template <typename T, typename Th, typename F, F f, const char* name, typename... Args>
//     size_t scripted_function_handle_args<T, Th, F, f, name, Args...>::get_type_id() const { return type_id<T>(); }
//     template <typename T, typename Th, typename F, F f, const char* name, typename... Args>
//     std::string_view scripted_function_handle_args<T, Th, F, f, name, Args...>::get_name() const { return name; }
    
    //template <typename T, typename Th, typename F, F f, const char* name, size_t args_num>
    //scripted_function_scripted_args<T, Th, F, f, name, args_num>::scripted_function_scripted_args(const interface* arg...) noexcept : args{arg...} {}
    //template <typename T, typename Th, typename F, F f, const char* name, size_t args_num>
    //scripted_function_scripted_args<T, Th, F, f, name, args_num>::scripted_function_scripted_args(std::initializer_list<const interface*> args) noexcept : args(args) {}
    template <typename Th, typename F, F f, const char* name>
    scripted_function_scripted_args<Th, F, f, name>::scripted_function_scripted_args(const interface* args) noexcept : args(args) {}
    template <typename Th, typename F, F f, const char* name>
    scripted_function_scripted_args<Th, F, f, name>::~scripted_function_scripted_args() noexcept {
      //for (size_t i = 0; i < args.size(); ++i) { args[i]->~interface(); }
      for (auto cur = args; cur != nullptr; cur = cur->next) { cur->~interface(); } 
    }
    
    template <typename Th, typename F, F f, size_t N, size_t curr, typename... Args>
    static object call_func(context* ctx, Th handle, const interface* current, Args&&... args) {
      using output_type = function_result_type<F>;
      if constexpr (!std::is_same_v<output_type, void>) {
        if constexpr (curr == N) { 
//           using arg_t = function_argument_type<F, 0>;
//           if constexpr (std::is_same_v<arg_t, Th> && !is_member_v<Th, F>) return object(std::invoke(f, handle, std::forward<Args>(args)...));
//           else return object(std::invoke(f, *handle, std::forward<Args>(args)...));
          return object(std::invoke(f, handle, std::forward<Args>(args)...));
        } else {
          using curr_type = std::remove_reference_t<function_argument_type<F, curr>>;
          //auto obj = s_args[curr]->process(ctx);
          auto obj = current->process(ctx);
          auto arg = obj.template get<curr_type>();
          return call_func<Th, F, f, N, curr+1>(ctx, handle, current->next, std::forward<Args>(args)..., arg);
        }
      } else {
        if constexpr (curr == N) { 
//           using arg_t = function_argument_type<F, 0>;
//           if constexpr (std::is_same_v<arg_t, Th> && !is_member_v<Th, F>) std::invoke(f, handle, std::forward<Args>(args)...);
//           else std::invoke(f, *handle, std::forward<Args>(args)...);
          std::invoke(f, handle, std::forward<Args>(args)...);
        } else {
          using curr_type = std::remove_reference_t<function_argument_type<F, curr>>;
          //auto obj = s_args[curr]->process(ctx);
          auto obj = current->process(ctx);
          auto arg = obj.template get<curr_type>();
          call_func<Th, F, f, N, curr+1>(ctx, handle, current->next, std::forward<Args>(args)..., arg);
        }
      }
      return object();
    }
    
    template <typename Th, typename F, F f, const char* name>
    struct object scripted_function_scripted_args<Th, F, f, name>::process(context* ctx) const {
      auto cur = ctx->current.get<Th>();
      using output_type = function_result_type<F>;
      constexpr size_t func_arg_count = get_function_argument_count<F>();
      if constexpr (!std::is_same_v<output_type, void>) { return call_func<Th, F, f, func_arg_count, !is_member_v<Th, F>>(ctx, cur, args); }
      else call_func<Th, F, f, func_arg_count, !is_member_v<Th, F>>(ctx, cur, args);
      return object();
    }
    
    template <typename Th, typename F, F f, const char* name>
    void scripted_function_scripted_args<Th, F, f, name>::draw(context* ctx) const {
      draw_data dd(ctx);
      dd.function_name = name;
      using output_type = function_result_type<F>;
      if constexpr (!std::is_same_v<output_type, void>) dd.value = process(ctx);
      ctx->draw(&dd);
    }
    
    template <typename Th, typename F, F f, const char* name>
    size_t scripted_function_scripted_args<Th, F, f, name>::get_type_id() const { return type_id<Th>(); }
    template <typename Th, typename F, F f, const char* name>
    std::string_view scripted_function_scripted_args<Th, F, f, name>::get_name() const { return name; }

    template <typename Th, typename F, F f, const char* name>
    scripted_iterator_every_numeric<Th, F, f, name>::scripted_iterator_every_numeric(const interface* condition, const interface* childs) noexcept : condition(condition), childs(childs) {}
    template <typename Th, typename F, F f, const char* name>
    scripted_iterator_every_numeric<Th, F, f, name>::~scripted_iterator_every_numeric() noexcept { 
      if (condition != nullptr) condition->~interface(); 
      for (auto cur = childs; cur != nullptr; cur = cur->next) { cur->~interface(); } 
    }
    
    template <typename Th, typename F, F f, const char* name>
    struct object scripted_iterator_every_numeric<Th, F, f, name>::process(context* ctx) const {
      // должна быть какая то функция которую мы запустим, а ответ от функции вернем
      // нет, нам скорее нужен итератор, чтобы не писать тонну ненужной фигни для
      // функций типа: сортированный обход, рандом, имеется_ли и проч
      change_scope cs(ctx, object(), ctx->current);
      
      Th cur;
      if constexpr (std::is_same_v<void*, Th>) cur = nullptr;
      else cur = ctx->current.get<Th>();
      
      // передадим текущий объект или контекст? контекст кажется избыточным
      // мы можем передавать сам объект
      double val = 0.0;
      f(cur, [&] (const object &obj) -> bool {
        ctx->current = obj;
        
        if (condition != nullptr) {
          const auto obj = condition->process(ctx);
          if (obj.ignore() || !obj.get<bool>()) return true;
        }
        
        for (auto cur = childs; cur != nullptr; cur = cur->next) {
          const auto &obj = cur->process(ctx);
          val += obj.ignore() ? 0.0 : obj.get<double>();
        }
        
        return true;
      });

      // дальше например сортируем, да зависит от типа, нужно сделать has_*, every_*, ordered_* (?), (обход скриптового массива), ???
      return object(val);
    }
    
    template <typename Th, typename F, F f, const char* name>
    void scripted_iterator_every_numeric<Th, F, f, name>::draw(context* ctx) const {
      const auto val = process(ctx);
      
      {
        draw_data dd(ctx);
        dd.function_name = name;
        dd.value = val;
        if (!ctx->draw(&dd)) return;
      }
      
      Th cur;
      if constexpr (std::is_same_v<void*, Th>) cur = nullptr;
      else cur = ctx->current.get<Th>();
      object first;
      f(cur, [&] (const object &obj) -> bool {
        if (condition != nullptr) {
          const auto obj = condition->process(ctx);
          if (obj.ignore() || !obj.get<bool>()) return true;
        }
        
        first = obj;
        return false;
      });
      if (!first.valid()) return;
      
      change_scope cs(ctx, first, ctx->current);
      change_function_name cfn(ctx, name);
      
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
    
    template <typename Th, typename F, F f, const char* name>
    size_t scripted_iterator_every_numeric<Th, F, f, name>::get_type_id() const { return type_id<Th>(); }
    template <typename Th, typename F, F f, const char* name>
    std::string_view scripted_iterator_every_numeric<Th, F, f, name>::get_name() const { return name; }
    
    template <typename Th, typename F, F f, const char* name>
    scripted_iterator_every_effect<Th, F, f, name>::scripted_iterator_every_effect(const interface* condition, const interface* childs) noexcept : condition(condition), childs(childs) {}
    template <typename Th, typename F, F f, const char* name>
    scripted_iterator_every_effect<Th, F, f, name>::~scripted_iterator_every_effect() noexcept { 
      if (condition != nullptr) condition->~interface(); 
      for (auto cur = childs; cur != nullptr; cur = cur->next) { cur->~interface(); } 
    }
    
    template <typename Th, typename F, F f, const char* name>
    struct object scripted_iterator_every_effect<Th, F, f, name>::process(context* ctx) const {
      change_scope cs(ctx, object(), ctx->current);
      
      Th cur;
      if constexpr (std::is_same_v<void*, Th>) cur = nullptr;
      else cur = ctx->current.get<Th>();
      f(cur, [&] (const object &obj) -> bool {
        ctx->current = obj;
        
        if (condition != nullptr) {
          const auto obj = condition->process(ctx);
          if (obj.ignore() || !obj.get<bool>()) return true;
        }
        
        for (auto cur = childs; cur != nullptr; cur = cur->next) { cur->process(ctx); }
        return true;
      });

      return object();
    }
    
    template <typename Th, typename F, F f, const char* name>
    void scripted_iterator_every_effect<Th, F, f, name>::draw(context* ctx) const {       
      {
        draw_data dd(ctx);
        dd.function_name = name;
        if (!ctx->draw(&dd)) return;
      }
      
      Th cur;
      if constexpr (std::is_same_v<void*, Th>) cur = nullptr;
      else cur = ctx->current.get<Th>();
      object first;
      f(cur, [&] (const object &obj) -> bool {
        if (condition != nullptr) {
          const auto obj = condition->process(ctx);
          if (obj.ignore() || !obj.get<bool>()) return true;
        }
        
        first = obj;
        return false;
      });
      if (!first.valid()) return;
      
      change_scope cs(ctx, first, ctx->current);
      change_function_name cfn(ctx, name);
      
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
    
    template <typename Th, typename F, F f, const char* name>
    size_t scripted_iterator_every_effect<Th, F, f, name>::get_type_id() const { return type_id<Th>(); }
    template <typename Th, typename F, F f, const char* name>
    std::string_view scripted_iterator_every_effect<Th, F, f, name>::get_name() const { return name; }
    
    template <typename Th, typename F, F f, const char* name>
    scripted_iterator_every_logic<Th, F, f, name>::scripted_iterator_every_logic(const interface* condition, const interface* childs) noexcept : condition(condition), childs(childs) {}
    template <typename Th, typename F, F f, const char* name>
    scripted_iterator_every_logic<Th, F, f, name>::~scripted_iterator_every_logic() noexcept { 
      if (condition != nullptr) condition->~interface(); 
      for (auto cur = childs; cur != nullptr; cur = cur->next) { cur->~interface(); } 
    }
    
    template <typename Th, typename F, F f, const char* name>
    struct object scripted_iterator_every_logic<Th, F, f, name>::process(context* ctx) const {
      change_scope cs(ctx, object(), ctx->current);
      
      Th cur;
      if constexpr (std::is_same_v<void*, Th>) cur = nullptr;
      else cur = ctx->current.get<Th>();
      
      bool val = true;
      f(cur, [&] (const object &obj) -> bool {
        if (!val) return false;
        ctx->current = obj;
        
        if (condition != nullptr) {
          const auto obj = condition->process(ctx);
          if (obj.ignore() || !obj.get<bool>()) return true;
        }
        
        bool final_r = true;
        for (auto cur = childs; cur != nullptr; cur = cur->next) {
          const auto &obj = cur->process(ctx);
          const bool ret = obj.ignore() ? true : obj.get<bool>();
          final_r = final_r && ret;
        }
        
        val = val && final_r;
        return true;
      });

      return object(val);
    }
    
    template <typename Th, typename F, F f, const char* name>
    void scripted_iterator_every_logic<Th, F, f, name>::draw(context* ctx) const {
      const auto val = process(ctx);
      
      {
        draw_data dd(ctx);
        dd.function_name = name;
        dd.value = val;
        if (!ctx->draw(&dd)) return;
      }
      
      Th cur;
      if constexpr (std::is_same_v<void*, Th>) cur = nullptr;
      else cur = ctx->current.get<Th>();
      object first;
      f(cur, [&] (const object &obj) -> bool {
        if (condition != nullptr) {
          const auto obj = condition->process(ctx);
          if (obj.ignore() || !obj.get<bool>()) return true;
        }
        
        first = obj;
        return false;
      });
      if (!first.valid()) return;
      
      change_scope cs(ctx, first, ctx->current);
      change_function_name cfn(ctx, name);
      
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
    
    template <typename Th, typename F, F f, const char* name>
    size_t scripted_iterator_every_logic<Th, F, f, name>::get_type_id() const { return type_id<Th>(); }
    template <typename Th, typename F, F f, const char* name>
    std::string_view scripted_iterator_every_logic<Th, F, f, name>::get_name() const { return name; }
    
    template <typename Th, typename F, F f, const char* name>
    scripted_iterator_has<Th, F, f, name>::scripted_iterator_has(const interface* childs) noexcept : max_count(nullptr), percentage(nullptr), childs(childs) {}
    template <typename Th, typename F, F f, const char* name>
    scripted_iterator_has<Th, F, f, name>::scripted_iterator_has(const interface* max_count, const interface* percentage, const interface* childs) noexcept : 
      max_count(max_count), percentage(percentage), childs(childs) {}
    template <typename Th, typename F, F f, const char* name>
    scripted_iterator_has<Th, F, f, name>::~scripted_iterator_has() noexcept { 
      if (max_count != nullptr) max_count->~interface(); 
      if (percentage != nullptr) percentage->~interface();
      for (auto cur = childs; cur != nullptr; cur = cur->next) { cur->~interface(); } 
    }
    
    template <typename Th, typename F, F f, const char* name>
    struct object scripted_iterator_has<Th, F, f, name>::process(context* ctx) const {
      change_scope cs(ctx, object(), ctx->current);
      
      Th cur;
      if constexpr (std::is_same_v<void*, Th>) cur = nullptr;
      else cur = ctx->current.get<Th>();
      
      size_t final_max_count = SIZE_MAX;
      if (percentage != nullptr) {
        const auto val = percentage->process(ctx);
        const double final_percent = val.get<double>();
        if (final_percent < 0.0) throw std::runtime_error(std::string(name) + " percentage cannot be less than zero");
        size_t counter = 0;
        f(cur, [&] (const object &) -> bool { ++counter; return true; });
        final_max_count = counter * final_percent;
      } else if (max_count != nullptr) {
        const auto val = max_count->process(ctx);
        const double v = val.get<double>();
        if (v < 0.0) throw std::runtime_error(std::string(name) + " count cannot be less than zero");
        final_max_count = v;
      }
      
      if (final_max_count == 0) return object(0.0);
      
      size_t counter = 0;
      size_t val = 0;
      f(cur, [&] (const object &obj) -> bool {
        if (counter >= final_max_count) return false;
        ++counter;
        ctx->current = obj;
        // задать в контекст текущий val и counter в качестве локальных переменных
        
        bool final_r = true;
        for (auto cur = childs; cur != nullptr; cur = cur->next) {
          const auto &obj = cur->process(ctx);
          const bool ret = obj.ignore() ? true : obj.get<bool>();
          final_r = final_r && ret;
        }
        
        val += size_t(final_r);
        return true;
      });

      return object(double(val));
    }
    
    template <typename Th, typename F, F f, const char* name>
    void scripted_iterator_has<Th, F, f, name>::draw(context* ctx) const {
      const auto value = process(ctx);
      
      {
        object count;
        object percent;
        if (max_count != nullptr) count = max_count->process(ctx);
        if (percentage != nullptr) percent = percentage->process(ctx);
      
        draw_data dd(ctx);
        dd.function_name = name;
        dd.value = value;
        if (percentage != nullptr) dd.set_arg(0, "percentage", percent);
        else if (max_count != nullptr) dd.set_arg(0, "count", count);
        if (!ctx->draw(&dd)) return;
      }
      
      Th cur;
      if constexpr (std::is_same_v<void*, Th>) cur = nullptr;
      else cur = ctx->current.get<Th>();
      
      object first;
      f(cur, [&] (const object &obj) -> bool {
        first = obj;
        return false;
      });
      if (!first.valid()) return;
      
      change_nesting cn(ctx, ++ctx->nest_level);
      change_scope cs(ctx, first, ctx->current);
      change_function_name cfn(ctx, name);
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        cur->draw(ctx);
      }
    }
    
    template <typename Th, typename F, F f, const char* name>
    size_t scripted_iterator_has<Th, F, f, name>::get_type_id() const { return type_id<Th>(); }
    template <typename Th, typename F, F f, const char* name>
    std::string_view scripted_iterator_has<Th, F, f, name>::get_name() const { return name; }
    
    template <typename Th, typename F, F f, const char* name>
    scripted_iterator_random<Th, F, f, name>::scripted_iterator_random(const size_t &state, const interface* condition, const interface* weight, const interface* childs) noexcept : 
      state(state), condition(condition), weight(weight), childs(childs) {}
    template <typename Th, typename F, F f, const char* name>
    scripted_iterator_random<Th, F, f, name>::~scripted_iterator_random() noexcept {
      if (condition != nullptr) condition->~interface();
      if (weight != nullptr) weight->~interface();
      //for (auto cur = childs; cur != nullptr; cur = cur->next) { cur->~interface(); } 
      childs->~interface();
    }
    
    template <typename F, F f, typename Th>
    static struct object get_rand_obj(context* ctx, Th cur, const interface* condition, const interface* weight, const size_t state, const std::string_view &func_name) {
      double accum_weight = 0.0;
      std::vector<std::pair<struct object, double>> objects;
      objects.reserve(50);
      f(cur, [&] (const object &obj) -> bool {
        ctx->current = obj;
        // здесь поди тоже можно задать индексы
        
        if (condition != nullptr) {
          const auto obj = condition->process(ctx);
          if (obj.ignore() || !obj.get<bool>()) return true;
        }
        
        object weight_val(1.0);
        if (weight != nullptr) {
          weight_val = weight->process(ctx);
        }
        
        const double local = weight_val.get<double>();
        if (local < 0.0) throw std::runtime_error(std::string(func_name) + " weights must not be less than zero");
        objects.emplace_back(obj, local);
        accum_weight += local;
        return true;
      });
      
      if (objects.size() == 0) return object();
      
      const uint64_t rand_val = ctx->get_random_value(state);
      const double rand = script::context::normalize_value(rand_val) * accum_weight;
      double cumulative = 0.0;
      size_t index = 0;
      for (; index < objects.size() && cumulative <= rand; cumulative += objects[index].second, ++index) {}
      index -= 1;
      
      return objects[index].first;
    }
    
    template <typename Th, typename F, F f, const char* name>
    struct object scripted_iterator_random<Th, F, f, name>::process(context* ctx) const {
      change_scope cs(ctx, object(), ctx->current);
      
      Th cur;
      if constexpr (std::is_same_v<void*, Th>) cur = nullptr;
      else cur = ctx->current.get<Th>();
        
      const auto obj = get_rand_obj<F, f>(ctx, cur, condition, weight, state, name);
      // рандом можно использовать только в эффекте, или нет? что я получаю если не в эффекте?
      // если передавать сюда собранный скрипт объекта, то понятно становится что возвращать
      ctx->current = obj;
      return childs->process(ctx);
    }
    
    template <typename Th, typename F, F f, const char* name>
    void scripted_iterator_random<Th, F, f, name>::draw(context* ctx) const {
      Th cur;
      if constexpr (std::is_same_v<void*, Th>) cur = nullptr;
      else cur = ctx->current.get<Th>();
      const auto obj = get_rand_obj<F, f>(ctx, cur, condition, weight, state, name);
      // а нужно ли это рисовать вообще если рандом не получился? не думаю, но было бы неплохо для дебага
//       if (!obj.valid()) {
//         object first;
//         f(cur, [&] (const object &obj) -> bool {
//           first = obj;
//           return false;
//         });
//         obj = first;
//       }
      
      if (!obj.valid()) return;
      
      {
        draw_data dd(ctx);
        dd.function_name = name;
        dd.value = obj;
        if (!ctx->draw(&dd)) return;
      }

      change_scope cs(ctx, obj, ctx->current);
      //change_nesting cn(ctx, ++ctx->nest_level);
      change_function_name cfn(ctx, name);
//       for (auto cur = childs; cur != nullptr; cur = cur->next) {
//         cur->draw(ctx);
//       }
      childs->draw(ctx);
    }
    
    template <typename Th, typename F, F f, const char* name>
    size_t scripted_iterator_random<Th, F, f, name>::get_type_id() const { return type_id<Th>(); }
    template <typename Th, typename F, F f, const char* name>
    std::string_view scripted_iterator_random<Th, F, f, name>::get_name() const { return name; }
    
  }
}

#endif
