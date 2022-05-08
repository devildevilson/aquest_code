#ifndef DEVILS_ENGINE_SCRIPT_TYPE_TRAITS_H
#define DEVILS_ENGINE_SCRIPT_TYPE_TRAITS_H

#include <string_view>
#include <type_traits>
#include <functional>

namespace devils_engine {
  namespace script {
    namespace detail {
      // primary template.
      template<typename T>
      struct function_traits : function_traits<decltype(&T::operator())> {};

      // partial specialization for function type
      template<typename R, typename... Args>
      struct function_traits<R(Args...)> {
        using member_of = void;
        using result_type = R;
        // воид в конце чтобы можно было удобно проверять, но хотя я так подозреваю что это не нужно на самом деле
        // вообще проверкой того что является первым аргументом займется std::invoke...
        // хотя мне в любом случае нужно прикидывать какой аргумент брать у функции 
        using argument_types = std::tuple<Args..., void>;
        static constexpr size_t argument_count = std::tuple_size_v<argument_types> - 1;
        static constexpr bool valid_scripted_argumets = !std::conjunction_v<std::is_reference<Args>...>;
      };

      // partial specialization for function pointer
      template<typename R, typename... Args>
      struct function_traits<R (*)(Args...)> {
        using member_of = void;
        using result_type = R;
        using argument_types = std::tuple<Args..., void>;
        static constexpr size_t argument_count = std::tuple_size_v<argument_types> - 1;
        static constexpr bool valid_scripted_argumets = !std::conjunction_v<std::is_reference<Args>...>;
      };

      // partial specialization for std::function
      template<typename R, typename... Args>
      struct function_traits<std::function<R(Args...)>> {
        using member_of = void;
        using result_type = R;
        using argument_types = std::tuple<Args..., void>;
        static constexpr size_t argument_count = std::tuple_size_v<argument_types> - 1;
        static constexpr bool valid_scripted_argumets = !std::conjunction_v<std::is_reference<Args>...>;
      };

      // partial specialization for pointer-to-member-function (i.e., operator()'s)
      template<typename T, typename R, typename... Args>
      struct function_traits<R (T::*)(Args...)> {
        using member_of = T;
        using result_type = R;
        using argument_types = std::tuple<Args..., void>;
        static constexpr size_t argument_count = std::tuple_size_v<argument_types> - 1;
        static constexpr bool valid_scripted_argumets = !std::conjunction_v<std::is_reference<Args>...>;
      };

      template<typename T, typename R, typename... Args>
      struct function_traits<R (T::*)(Args...) const> {
        using member_of = T;
        using result_type = R;
        using argument_types = std::tuple<Args..., void>;
        static constexpr size_t argument_count = std::tuple_size_v<argument_types> - 1;
        static constexpr bool valid_scripted_argumets = !std::conjunction_v<std::is_reference<Args>...>;
      };
      
      template<typename T, typename R, typename... Args>
      struct function_traits<R (T::*)(Args...) const noexcept> {
        using member_of = T;
        using result_type = R;
        using argument_types = std::tuple<Args..., void>;
        static constexpr size_t argument_count = std::tuple_size_v<argument_types> - 1;
        static constexpr bool valid_scripted_argumets = !std::conjunction_v<std::is_reference<Args>...>;
      };

      // additional cv-qualifier and ref-qualifier combinations omitted
      // sprinkle with C-style variadics if desired
      
      // First, a type trait to check whether a type can be static_casted to another    
      template <typename From, typename To, typename = void>
      struct can_static_cast : public std::false_type {};

      template <typename From, typename To>
      struct can_static_cast<From, To, std::void_t<decltype(static_cast<To>(std::declval<From>()))>> : public std::true_type {};

      // Then, we apply the fact that a virtual base is first and foremost a base,
      // that, however, cannot be static_casted to its derived class.
      template <typename Base, typename Derived>
      struct is_virtual_base_of : public std::conjunction<
          std::is_base_of<Base, Derived>, 
          std::negation<can_static_cast<Base*, Derived*>>
      >{};
      
      /// Simple type introspection without RTTI.
      template <typename T>
      constexpr std::string_view get_type_name() {
        //std::cout << __PRETTY_FUNCTION__ << "\n";
        //std::cout << __FUNCSIG__ << "\n";
#if defined(_MSC_VER)
        constexpr std::string_view start_char_seq = "get_type_name<";
        constexpr std::string_view end_char_seq = ">(void)";
        constexpr std::string_view function_type_pattern = ")(";
        constexpr std::string_view sig = __FUNCSIG__;
        constexpr size_t str_seq_name_start = sig.find(start_char_seq) + start_char_seq.size();
        constexpr size_t end_of_char_str = sig.rfind(start_char_seq);
        constexpr size_t count = sizeof(__FUNCSIG__) - str_seq_name_start - end_char_seq.size() - 1; // отстается символ '>' в конце
        constexpr std::string_view substr = sig.substr(str_seq_name_start, count);
        if constexpr (substr.find(function_type_pattern) == std::string_view::npos) {
          constexpr std::string_view class_char_seq = "class ";
          constexpr std::string_view struct_char_seq = "struct ";
          const size_t class_seq_start = substr.find(class_char_seq);
          const size_t struct_seq_start = substr.find(struct_char_seq);
          if constexpr (class_seq_start == 0) return substr.substr(class_char_seq.size());
          if constexpr (struct_seq_start == 0) return substr.substr(struct_char_seq.size());;
        }
        return substr;
#elif defined(__clang__)
        constexpr std::string_view sig = __PRETTY_FUNCTION__;
        constexpr std::string_view start_char_seq = "T = ";
        constexpr size_t str_seq_name_start = sig.find(start_char_seq) + start_char_seq.size();
        constexpr size_t end_of_char_str = 2;
        constexpr size_t count = sizeof(__PRETTY_FUNCTION__) - str_seq_name_start - end_of_char_str;
        return sig.substr(str_seq_name_start, count);
#elif defined(__GNUC__)
        constexpr std::string_view sig = __PRETTY_FUNCTION__;
        constexpr std::string_view start_char_seq = "T = ";
        constexpr size_t str_seq_name_start = sig.find(start_char_seq) + start_char_seq.size();
        constexpr size_t end_of_char_str = sizeof(__PRETTY_FUNCTION__) - sig.find(';');
        constexpr size_t count = sizeof(__PRETTY_FUNCTION__) - str_seq_name_start - end_of_char_str;
        return sig.substr(str_seq_name_start, count);
#else
#error Compiler not supported for demangling
#endif
      }
    }
    
    template <char ... C>
    struct string_literal {
      static constexpr const size_t size = sizeof...(C)+1;
      static constexpr const char value[size] = {C..., '\0'};
      constexpr operator const char* (void) const {
        return value;
      }
    };
    //template <char ... C> constexpr const char string_literal<C...>::size;
    template <char ... C> constexpr const char string_literal<C...>::value[string_literal<C...>::size];

    template <typename CharT, CharT... Cs>
    constexpr string_literal<Cs...> operator"" _create() {
      return {};
    }
    
    template<typename T, size_t N>
    using function_argument_type = typename std::tuple_element<N, typename detail::function_traits<T>::argument_types>::type;

    template<typename T>
    using function_result_type = typename detail::function_traits<T>::result_type;
    
    template<typename T>
    using function_member_of = typename detail::function_traits<T>::member_of;
    
    template<typename T>
    constexpr size_t get_function_argument_count() { return detail::function_traits<T>::argument_count; }
    
    template<typename T>
    constexpr std::string_view type_name() {
      using type = std::remove_reference_t<std::remove_cv_t<T>>;
      return detail::get_type_name<type>();
    }
    
    template <typename F>
    using final_output_type = std::remove_cv_t< std::remove_reference_t< function_result_type<F> > >;
    
    template <typename F, size_t N>
    using final_arg_type = std::remove_cv_t< std::remove_reference_t< function_argument_type<F, N> > >;
    
    template <typename T>
    using clear_type_t = std::remove_pointer_t< std::remove_cv_t< std::remove_reference_t< T > > >;
    
    
    
    template <typename T, typename F>
    constexpr bool is_member_v = !std::is_same_v<clear_type_t<T>, void> && 
      (std::is_same_v<clear_type_t<T>, function_member_of<F>> || 
       std::is_same_v<clear_type_t<decltype(*T())>, function_member_of<F>>);
  }
}

#endif
