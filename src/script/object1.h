#ifndef DEVILS_ENGINE_UNUSED_OBJECT_H
#define DEVILS_ENGINE_UNUSED_OBJECT_H

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <cstring>
#include <type_traits>
#include <cmath>
#include <stdexcept>

#include "utils/shared_mathematical_constant.h"

#define OBJECT_TYPES_LIST \
  GAME_STRUCTURES_LIST \
  GAME_STRUCTURE_FUNC(boolean) \
  GAME_STRUCTURE_FUNC(number) \
  GAME_STRUCTURE_FUNC(string) \
  GAME_STRUCTURE_FUNC(invalid) \
  
//#define STANDART_PTR SIZE_MAX
//#define CONST_PTR (SIZE_MAX-1)

namespace devils_engine {
  namespace unused {
    template <typename T>
    constexpr size_t type_id() { return 1; }
    
//     struct object {
//       static const size_t mem_size = sizeof(size_t) + sizeof(void*);
//       
//       size_t type;
//       union {
//         struct {
//           void* data;
//           double value;
//         };
//         
//         struct {
//           const void* const_data;
//           size_t token;
//         };
//         
//         struct {
//           char mem[mem_size];
//         };
//       };
//       
//       template <typename T>
//       static constexpr void check_type() {
//         static_assert(sizeof(T) <= mem_size);
//         static_assert(std::is_trivially_destructible_v<T>, "Custom destructor is not supported");
//         static_assert(std::is_copy_constuctible_v<T>, "Type must be copyable");
//         static_assert(!(std::is_pointer_v<T> && std::is_fundamental_v<std::remove_reference_t<std::remove_pointer_t<T>>>), "Do not store pointer to fundamental types, array views is supported");
//         static_assert(!std::is_same_v<T, std::string_view*>, "Do not store pointer to string_view");
//       }
//       
//       constexpr inline object() noexcept : type(0), data(nullptr), value(0) {}
//       constexpr inline explicit object(const bool val) noexcept : type(type_id<bool>()), data(nullptr), value(val) {}
//       constexpr inline explicit object(const double val) noexcept : type(type_id<double>()), data(nullptr), value(val) {}
//       constexpr inline object(const std::string_view val) noexcept : type(type_id<std::string_view>()), data(nullptr), value(0) { set_data(val); }
//       template <typename T>
//       constexpr inline object(const array_view<T> val) noexcept : type(type_id<array_view<T>>()), data(nullptr), value(0) { set_data(val); }
//       template <typename T>
//       constexpr inline object(const T val) noexcept : type(type_id<T>()), data(nullptr), value(0) { 
//         check_type<T>();
//         set_data(val); 
//       }
//       
//       object(const object &copy) noexcept = default;
//       object(object &move) noexcept = default;
//       object & operator=(const object &copy) noexcept = default;
//       object & operator=(object &move) noexcept = default;
//       
//       template <typename T>
//       constexpr void set_data(const T data) noexcept {
//         auto ptr = &mem[0];
//         new (ptr) T(data);
//       }
//       
//       template<typename T>
//       constexpr bool is() const noexcept { return type == type_id<T>(); }
//       
//       template<> constexpr bool is<bool>() const noexcept   { return type == type_id<bool>() || type == type_id<double>(); }
//       template<> constexpr bool is<double>() const noexcept { return type == type_id<bool>() || type == type_id<double>(); }
//       
//       template<typename T>
//       T get() const {
//         if constexpr (!is<T>()) throw std::runtime_error("Wrong type");
//         
//         if constexpr (std::is_same_v<T, bool>) return !(std::abs(value) < EPSILON);
//         else if constexpr (std::is_same_v<T, double>) return value;
//         else {
//           check_type<T>();
//           auto ptr = &mem[0];
//           return *reinterpret_cast<T*>(ptr);
//         }
//         return T{};
//       }
//       
//       constexpr bool lazy_type_compare(const object &another) const noexcept {
//         const bool cur_num_or_bool = is<double>() || is<bool>();
//         const bool ano_num_or_bool = another.is<double>() || another.is<bool>();
//         if (cur_num_or_bool && ano_num_or_bool) return true;
//         return type == another.type;
//       }
//       
//       constexpr bool operator==(const object &another) const noexcept {
//         if (is<double>() && another.is<double>()) {
//           const double f = is<bool>() ? get<bool>() : get<double>();
//           const double s = another.is<bool>() ? another.get<bool>() : another.get<double>();
//           return std::abs(f-s) < EPSILON;
//         }
//         
//         if (!lazy_type_compare(another)) return false;
//         
//         if (is<std::string_view>()) return get<std::string_view>() == another.get<std::string_view>();
//         return memcmp(&mem[0], &another.mem[0], sizeof(mem)) == 0;
//       }
//       
//       constexpr bool operator!=(const object &another) const noexcept { return !operator==(another); }
//       
//       constexpr size_t get_type() const noexcept { return type; }
//       constexpr bool valid() const noexcept { return type != 0; }
//       constexpr bool ignore() const noexcept { return is<double>() && std::isnan(value); }
//     };
//     
//     constexpr bool func() {
//       const double d = 54.0;
//       object o(d);
//       const std::string_view str = "abc";
//       object o1(str);
//       size_t* ptr = nullptr;
//       object o2(ptr);
//       
//       static_assert(o.value == 54.0);
//       static_assert(o1.is<std::string_view>());
//       return true;
//     }
//     
//     static_assert(func());

    struct object {
//       enum class type : size_t {
// #define GAME_STRUCTURE_FUNC(val) val,
//       OBJECT_TYPES_LIST
// #undef GAME_STRUCTURE_FUNC
//         
//         count
//       };
//       
//       static const size_t maximum_types_count = SIZE_WIDTH-2;
//       static_assert(static_cast<size_t>(type::count) < maximum_types_count);
//       
//       struct type_bit {
//         enum values : size_t {
// #define GAME_STRUCTURE_FUNC(val) val = size_t(1) << static_cast<size_t>(type::val),
//           OBJECT_TYPES_LIST
// #undef GAME_STRUCTURE_FUNC
//         };
//         
//         static const size_t all = make_mask(static_cast<size_t>(type::invalid));
//         static const size_t none = 0;
//         static const size_t valid_number = boolean | number;
//         static const size_t valid_boolean = boolean | number;
//         static const size_t all_objects = 
// #define GAME_STRUCTURE_FUNC(val) val |
//           GAME_STRUCTURES_LIST
// #undef GAME_STRUCTURE_FUNC
//         0;
//         
//         static_assert((all & invalid) == 0);
//       };
      
      size_t type;
      union {
        void* data;
        const void* const_data; // пригодится, но желательно как нибудь пометить что объект пришел константный
        const char* str;
      };
      union {
        size_t token;
        double value;
      };
      
      static size_t make_const_type(const size_t type);
      static size_t make_handle_type(const size_t type);
      
      // инфу о том является ли объект константным указателем или нет мы может пихнуть в token, 
      // предполагается что значения SIZE_MAX и SIZE_MAX-1 не будут заняты токеном
      object() noexcept;
      template <typename T>
      object(T* obj) noexcept : type(obj == nullptr ? 0 : type_id<T>()), data(obj), token(STANDART_PTR) {}
      template <typename T>
      object(const T* obj) noexcept : type(obj == nullptr ? 0 : type_id<T>()), const_data(obj), token(CONST_PTR) {} // ASSERT(is_const());
      template <typename T>
      object(utils::handle<T> obj) noexcept : 
        type(!obj.valid() ? 0 : type_id<T>()), 
        data(obj.get()), 
        token(!obj.valid() ? SIZE_MAX : obj.get_token()) 
      {} // ASSERT(is_handle());
      explicit object(const bool val) noexcept;
      explicit object(const double &val) noexcept;
      object(const std::string_view &val) noexcept;
      
      // нужно хранить всегда базовый тип T, но проблема тут в том что видимо придется использовать всегда utils::handle
      template <typename T>
      bool is() const noexcept { 
        if constexpr (utils::is_handle_type<T>::value) {
          using handling_type = typename utils::is_handle_type<T>::underlying_type;
          return token != STANDART_PTR && token != CONST_PTR && type == type_id<handling_type>();
        } else if constexpr (!std::is_const_v<T>) return token != CONST_PTR && type == type_id<T>();
        return type == type_id<T>();  // || type == type_id<const T>()
      }
      
      template <typename T>
      T get() const {
        if (!is<T>()) throw std::runtime_error("Expects " + std::string(type_name<T>()) + ", got wrong type");
        
        if constexpr (utils::is_handle_type<T>::value) {
          using handling_type = typename utils::is_handle_type<T>::underlying_type;
          using handle_type = utils::handle<handling_type>;
          return handle_type(reinterpret_cast<handling_type*>(data), token);
        } else {
          if constexpr (std::is_const_v<T>) return reinterpret_cast<T>(const_data);
          else {
            return reinterpret_cast<T>(data);
          }
        }
        return T{};
      }
      
      #define BOOL_RETURN (!(std::abs(value) < EPSILON))
    
      template<> bool is<bool>() const noexcept { return get_type() == type_id<bool>(); }
      template<> bool is<const bool>() const noexcept { return is<bool>(); }
      template<> bool is<double>() const noexcept { return get_type() == type_id<double>() && !std::isnan(value); }
      template<> bool is<const double>() const noexcept { return is<double>(); }
      template<> bool get<bool>() const { 
        if (!is<bool>() && (!is<double>())) throw_obj_error("boolean");
        return BOOL_RETURN;
      }
      template<> const bool get<const bool>() const { return get<bool>(); }
      template<> double get<double>() const { 
        if (!is<bool>() && !is<double>()) throw_obj_error("number");
        return is<bool>() ? double(BOOL_RETURN) : value;
      }
      template<> const double get<const double>() const { return get<double>(); }
      
      template<> bool is<std::string_view>() const noexcept { return get_type() == type_id<std::string_view>(); }
      template<> bool is<const std::string_view>() const noexcept { return is<std::string_view>(); }
      template<> std::string_view get<std::string_view>() const { if (!is<std::string_view>()) throw_obj_error("string"); return std::string_view(str, token); }
      template<> const std::string_view get<const std::string_view>() const { return get<std::string_view>(); }

      bool ignore() const noexcept;
      bool valid() const noexcept;
      size_t get_type() const noexcept;
      bool is_const() const noexcept;
      bool is_handle() const noexcept;
      
      void throw_obj_error(const std::string_view &expected_type_name) const;
//       void throw_obj_error_const(const std::string_view &expected_type_name) const;
//       void throw_obj_error_handle(const std::string_view &expected_type_name) const;
      
      object(const object &copy) noexcept = default;
      object(object &&move) noexcept = default;
      object & operator=(const object &copy) noexcept = default;
      object & operator=(object &&move) noexcept = default;
      
      bool operator==(const object &other) const noexcept;
      bool operator!=(const object &other) const noexcept;
      bool lazy_compare_types(const object &other) const noexcept;
    };
    
    std::string_view get_game_type_name(const size_t &type) noexcept;
    std::string parse_type_bits(const size_t &type_bits) noexcept;
  }
}

#endif
