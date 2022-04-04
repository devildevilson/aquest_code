#ifndef DEVILS_ENGINE_UTILS_TYPE_INFO_H
#define DEVILS_ENGINE_UTILS_TYPE_INFO_H

#include <cstddef>
#include <atomic>
#include <string_view>
#include <cassert>
#include "constexpr_funcs.h"

#if defined __clang__ || defined __GNUC__
  #define DEVILS_ENGINE_UTILS_PRETTY_FUNCTION __PRETTY_FUNCTION__
  #define DEVILS_ENGINE_UTILS_PRETTY_FUNCTION_PREFIX '='
  #define DEVILS_ENGINE_UTILS_PRETTY_FUNCTION_SUFFIX ';'
#elif defined _MSC_VER
  #define DEVILS_ENGINE_UTILS_PRETTY_FUNCTION __FUNCSIG__
  #define DEVILS_ENGINE_UTILS_PRETTY_FUNCTION_SUFFIX '>'
  #define DEVILS_ENGINE_UTILS_PRETTY_FUNCTION_STRUCT_CHARSEQ "struct " // а если класс? обвалится
#endif

namespace devils_engine {
  namespace utils {
    namespace internal {
      struct type_id_counter {
        static std::atomic<size_t> counter;
        template <typename T>
        inline static size_t get() { static const size_t id = counter.fetch_add(1); return id; }
      };
      
//       template <typename T>
//       constexpr std::string_view compute_type_name() {
//         std::string_view pretty_function = DEVILS_ENGINE_UTILS_PRETTY_FUNCTION;
// #if defined __clang__ || defined __GNUC__
//         const size_t first = pretty_function.find_first_not_of(' ', pretty_function.find_first_of(DEVILS_ENGINE_UTILS_PRETTY_FUNCTION_PREFIX)+1);
//         const auto value = pretty_function.substr(first, pretty_function.find_last_of(DEVILS_ENGINE_UTILS_PRETTY_FUNCTION_SUFFIX) - first);
// #elif defined _MSC_VER
//         const std::string_view struct_identifier = DEVILS_ENGINE_UTILS_PRETTY_FUNCTION_STRUCT_CHARSEQ;
//         const size_t last = pretty_function.find_last_of(DEVILS_ENGINE_UTILS_PRETTY_FUNCTION_SUFFIX);
//         const size_t first = pretty_function.rfind(struct_identifier, last) + struct_identifier.size();
//         const auto value = pretty_function.substr(first, last - first);
// #endif
//         return value;
//       }
      
      /// Simple type introspection without RTTI.
      template <typename T>
      constexpr std::string_view get_type_name_pretty() {
#if defined(_MSC_VER)
        assert(false);
        constexpr size_t str_seq_name_start = 33-6; // 33-6
        constexpr size_t end_of_char_str = 8;
        constexpr std::string_view sig = __FUNCSIG__;
        constexpr size_t count = sizeof(__PRETTY_FUNCTION__) - str_seq_name_start - end_of_char_str;
        return sig.substr(str_seq_name_start, count);
#elif defined(__clang__)
        constexpr size_t str_seq_name_start = 38;
        constexpr size_t end_of_char_str = 2;
        constexpr std::string_view sig = __PRETTY_FUNCTION__;
        constexpr size_t count = sizeof(__PRETTY_FUNCTION__) - str_seq_name_start - end_of_char_str;
        return sig.substr(str_seq_name_start, count);
#elif defined(__GNUC__)
        constexpr std::string_view sig = __PRETTY_FUNCTION__;
        constexpr size_t str_seq_name_start = 53;
        constexpr size_t end_of_char_str = sizeof(__PRETTY_FUNCTION__) - sig.find(';');
        constexpr size_t count = sizeof(__PRETTY_FUNCTION__) - str_seq_name_start - end_of_char_str;
        return sig.substr(str_seq_name_start, count);
#endif
      }
      
      // как минимум надо убрать константный модификатор для портируемости
      template <typename T>
      constexpr std::string_view get_type_name() {
        //return compute_type_name<std::remove_cv_t<T>>();
        return get_type_name_pretty<std::remove_cv_t<T>>();
      }
    }
    
    template <typename T>
    inline size_t type_id() { return internal::type_id_counter::get<std::remove_cv_t<std::remove_reference_t<std::remove_pointer_t<T>>>>(); }
    
    // наверное стоит убрать вообще все модификаторы типа
    template <typename T>
    constexpr std::string_view type_name() {
      return internal::get_type_name<std::remove_reference_t<std::remove_pointer_t<T>>>();
    }
    
    constexpr std::string_view remove_namespaces(const std::string_view name) {
      const size_t index = name.rfind(':');
      return name.substr(index+1);
    }
    
    // как можно проверить коллизии?
    template <typename T>
    constexpr uint64_t type_hash() {
      return string_hash(type_name<T>());
    }
  }
}

#endif
