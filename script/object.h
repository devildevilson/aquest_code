#ifndef DEVILS_ENGINE_SCRIPT_OBJECT_H
#define DEVILS_ENGINE_SCRIPT_OBJECT_H

#include <cstddef>
#include <string>
#include <cmath>
#include "utils/handle.h"
#include "core/declare_structures.h"
#include "utils/constexpr_funcs.h"
#include "utils/assert.h"

#define OBJECT_TYPES_LIST \
  GAME_STRUCTURES_LIST \
  GAME_STRUCTURE_FUNC(boolean) \
  GAME_STRUCTURE_FUNC(number) \
  GAME_STRUCTURE_FUNC(string) \
  GAME_STRUCTURE_FUNC(invalid) \
  
namespace devils_engine {
  namespace script {
    struct object {
      enum class type : size_t {
#define GAME_STRUCTURE_FUNC(val) val,
      OBJECT_TYPES_LIST
#undef GAME_STRUCTURE_FUNC
        
        count
      };
      
      static const size_t maximum_types_count = SIZE_WIDTH-2;
      static_assert(static_cast<size_t>(type::count) < maximum_types_count);
      
      struct type_bit {
        enum values : size_t {
#define GAME_STRUCTURE_FUNC(val) val = size_t(1) << static_cast<size_t>(type::val),
          OBJECT_TYPES_LIST
#undef GAME_STRUCTURE_FUNC
        };
        
        static const size_t all = make_mask(static_cast<size_t>(type::invalid));
        static const size_t none = 0;
        static const size_t valid_number = boolean | number;
        static const size_t valid_boolean = boolean | number;
        static const size_t all_objects = 
#define GAME_STRUCTURE_FUNC(val) val |
          GAME_STRUCTURES_LIST
#undef GAME_STRUCTURE_FUNC
        0;
        
        static_assert((all & invalid) == 0);
      };
      
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
      
      object() noexcept;
      template <typename T>
      object(T* obj) noexcept : type(obj == nullptr ? static_cast<size_t>(type::invalid) : static_cast<size_t>(T::s_type)), data(obj), token(SIZE_MAX) {}
      template <typename T>
      object(const T* obj) noexcept : type(obj == nullptr ? static_cast<size_t>(type::invalid) : make_const_type(static_cast<size_t>(T::s_type))), const_data(obj), token(SIZE_MAX) { ASSERT(is_const()); }
      template <typename T>
      object(utils::handle<T> obj) noexcept : 
        type(!obj.valid() ? static_cast<size_t>(type::invalid) : make_handle_type(static_cast<size_t>(T::s_type))), 
        data(obj.get()), 
        token(!obj.valid() ? SIZE_MAX : obj.get_token()) 
      { ASSERT(is_handle()); }
      explicit object(const bool val) noexcept;
      explicit object(const double &val) noexcept;
      object(const std::string_view &val) noexcept;
      
      template <typename T>
      bool is() const noexcept;
      template <typename T>
      T get() const;

      bool ignore() const noexcept;
      bool valid() const noexcept;
      enum type get_type() const noexcept;
      bool is_const() const noexcept;
      bool is_handle() const noexcept;
      
      void throw_obj_error(const std::string_view &expected_type_name) const;
      void throw_obj_error_const(const std::string_view &expected_type_name) const;
      void throw_obj_error_handle(const std::string_view &expected_type_name) const;
      
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
    extern const object ignore_value;
  }
}

#endif
