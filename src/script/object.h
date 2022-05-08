#ifndef DEVILS_ENGINE_SCRIPT_OBJECT_H
#define DEVILS_ENGINE_SCRIPT_OBJECT_H

#include <cstddef>
#include <string>
#include <cmath>
#include <cstring>
//#include "utils/handle.h"
//#include "core/declare_structures.h"
//#include "utils/constexpr_funcs.h"
//#include "utils/assert.h"
#include "type_info.h"
#include "utils/shared_mathematical_constant.h"
#include "utils/array_view.h"
//#include "utils/utility.h"
  
namespace devils_engine {
  namespace script {
    namespace detail {
      template<typename T> 
      constexpr inline bool is(const size_t &type, const double) noexcept { return type == type_id<T>(); }
      template<> constexpr inline bool is<bool>(const size_t &type, const double value) noexcept   { return type == type_id<bool>() || (type == type_id<double>() && !std::isnan(value)); }
      template<> constexpr inline bool is<double>(const size_t &type, const double value) noexcept { return type == type_id<bool>() || (type == type_id<double>() && !std::isnan(value)); }
    }
    
    struct object {
      static const size_t mem_size = sizeof(size_t) + sizeof(void*);
      
      size_t type;
      union {
        struct {
          void* data;
          double value;
        };
        
        struct {
          const void* const_data;
          size_t token;
        };
        
        struct {
          char mem[mem_size];
        };
      };
      
      template <typename T>
      static constexpr void check_type() {
        static_assert(sizeof(T) <= mem_size);
        static_assert(alignof(T) <= alignof(object));
        static_assert(std::is_trivially_destructible_v<T>, "Custom destructor is not supported");
        static_assert(std::is_copy_constructible_v<T>, "Type must be copyable");
        static_assert(!(std::is_pointer_v<T> && std::is_fundamental_v<std::remove_reference_t<std::remove_pointer_t<T>>>), "Do not store pointer to fundamental types, array views is supported");
        static_assert(!std::is_same_v<T, std::string_view*>, "Do not store pointer to string_view");
      }
      
      constexpr inline object() noexcept : type(0), data(nullptr), value(0) {}
      constexpr inline explicit object(const bool val) noexcept : type(type_id<bool>()), data(nullptr), value(val) {}
      constexpr inline explicit object(const double val) noexcept : type(type_id<double>()), data(nullptr), value(val) {}
      inline object(const std::string_view val) noexcept : type(type_id<std::string_view>()), data(nullptr), value(0) { set_data(val); }
      inline object(const std::string &val) noexcept : object(std::string_view(val)) {}
      //constexpr inline object(const glm::vec4 val) noexcept : type(type_id<glm::vec4>()), data(nullptr), value(0) { set_data(val); }
      template <typename T>
      inline object(const utils::array_view<T> val) noexcept : type(type_id<utils::array_view<T>>()), data(nullptr), value(0) { set_data(val); }
      template <typename T>
      inline object(const T val) noexcept : type(type_id<T>()), data(nullptr), value(0) { 
        check_type<T>();
        set_data(val); 
      }
      
      object(const object &copy) noexcept = default;
      object(object &move) noexcept = default;
      object & operator=(const object &copy) noexcept = default;
      object & operator=(object &move) noexcept = default;
      
      constexpr inline object & operator=(const bool val) noexcept { type = type_id<bool>(); data = nullptr; value = double(val); return *this; }
      constexpr inline object & operator=(const double val) noexcept { type = type_id<double>(); data = nullptr; value = val; return *this; }
      constexpr inline object & operator=(const std::string_view val) noexcept { type = type_id<std::string_view>(); data = nullptr; value = 0; set_data(val); return *this; }
      //constexpr inline object(const glm::vec4 val) noexcept : type(type_id<glm::vec4>()), data(nullptr), value(0) { set_data(val); }
      template <typename T>
      constexpr inline object & operator=(const utils::array_view<T> val) noexcept { type = type_id<utils::array_view<T>>(); data = nullptr; value = 0; set_data(val); return *this; }
      template <typename T>
      constexpr inline object & operator=(const T val) noexcept { 
        type = type_id<T>(); 
        data = nullptr; 
        value = 0;
        check_type<T>();
        set_data(val); 
        return *this;
      }
      
      template <typename T>
      constexpr void set_data(const T data) noexcept {
        auto ptr = &mem[0];
        new (ptr) T(data);
      }
      
      template<typename T>
      constexpr inline bool is() const noexcept { return detail::is<T>(type, value); }
      
      template<typename T>
      T get() const {
        if (!is<T>()) throw std::runtime_error("Expected '" + std::string(type_name<T>()) + "' (" + std::to_string(type_id<T>()) + "), but another type stored (" + std::to_string(type) + ")");
        
        if constexpr (std::is_same_v<T, bool>) return !(std::abs(value) < EPSILON);
        else if constexpr (std::is_same_v<T, double>) return value;
        else {
          check_type<T>();
          auto ptr = &mem[0];
          return *reinterpret_cast<const T*>(ptr);
        }
        return T{};
      }
      
      constexpr bool lazy_type_compare(const object &another) const noexcept {
        const bool cur_num_or_bool = is<double>() || is<bool>();
        const bool ano_num_or_bool = another.is<double>() || another.is<bool>();
        if (cur_num_or_bool && ano_num_or_bool) return true;
        return type == another.type;
      }
      
      constexpr bool operator==(const object &another) const noexcept {
        if (is<double>() && another.is<double>()) {
          const double f = value;
          const double s = another.value;
          return std::abs(f-s) < EPSILON;
        }
        
        if (!lazy_type_compare(another)) return false;
        
        if (is<std::string_view>()) return get<std::string_view>() == another.get<std::string_view>();
        return memcmp(&mem[0], &another.mem[0], sizeof(mem)) == 0;
      }
      
      constexpr bool operator!=(const object &another) const noexcept { return !operator==(another); }
      
      constexpr size_t get_type() const noexcept { return type; }
      constexpr bool valid() const noexcept { return type != 0; }
      constexpr bool ignore() const noexcept { return type == type_id<double>() && std::isnan(value); }
    };
    
    extern const object ignore_value;
    
//     constexpr bool test_func() {
//       const double a = 1.0;
//       const bool b = true;
//       object o1(a);
//       object o2(b);
//       return o1 == o2;
//     }
//     
//     static_assert(test_func());
  }
}

#endif
