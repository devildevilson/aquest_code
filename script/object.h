#ifndef DEVILS_ENGINE_SCRIPT_OBJECT_H
#define DEVILS_ENGINE_SCRIPT_OBJECT_H

#include <cstddef>
#include <string>
#include <cmath>
#include "utils/handle.h"
#include "core/declare_structures.h"
#include "utils/constexpr_funcs.h"

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
      
      static_assert(static_cast<size_t>(type::count) < SIZE_WIDTH);
      
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
        const char* str;
      };
      union {
        size_t token;
        double value;
      };
      
      object() noexcept;
      template <typename T>
      object(T* obj) noexcept : type(obj == nullptr ? static_cast<size_t>(type::invalid) : static_cast<size_t>(T::s_type)), data(obj), token(SIZE_MAX) {}
      template <typename T>
      object(utils::handle<T> obj) noexcept : type(!obj.valid() ? static_cast<size_t>(type::invalid) : static_cast<size_t>(T::s_type)), data(obj.get()), token(!obj.valid() ? SIZE_MAX : obj.get_token()) {}
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
      
      object(const object &copy) noexcept = default;
      object(object &&move) noexcept = default;
      object & operator=(const object &copy) noexcept = default;
      object & operator=(object &&move) noexcept = default;
    };
    
    extern const object ignore_value;
    
    inline bool check_type_overlap(const size_t &type_mask, const size_t &type) { return (type_mask & type) != 0; }
  }
}

#endif
