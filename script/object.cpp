#include "object.h"

#include <stdexcept>
#include <limits>
#include "utils/shared_mathematical_constant.h"

namespace devils_engine {
  namespace script {
    const size_t object::type_bit::all;
    const size_t object::type_bit::none;
    const size_t object::type_bit::valid_number;
    const size_t object::type_bit::valid_boolean;
    const size_t object::type_bit::all_objects;
    
    object::object() noexcept : type(static_cast<size_t>(type::invalid)), data(nullptr), token(SIZE_MAX) {}
    object::object(const bool val) noexcept : type(static_cast<size_t>(type::boolean)), data(nullptr), value(val) {}
    object::object(const double &val) noexcept : type(static_cast<size_t>(type::number)), data(nullptr), value(val) {}
    object::object(const std::string_view &val) noexcept : type(static_cast<size_t>(type::string)), str(val.data()), token(val.size()) {}
    
#define BOOL_RETURN (!(std::abs(value) < EPSILON))
    
    template<> bool object::is<bool>() const noexcept { return type == static_cast<size_t>(type::boolean); }
    template<> bool object::get<bool>() const { 
      if (!is<bool>() && (type != static_cast<size_t>(type::number) || std::isnan(value))) 
        throw std::runtime_error("Wrong object type"); 
      return BOOL_RETURN;
    }
    template<> bool object::is<double>() const noexcept { return type == static_cast<size_t>(type::number) && !std::isnan(value); }
    template<> double object::get<double>() const { 
      if (!is<bool>() && !is<double>()) 
        throw std::runtime_error("Wrong object type");
      return is<bool>() ? double(BOOL_RETURN) : value;
    }
    template<> bool object::is<std::string_view>() const noexcept { return type == static_cast<size_t>(type::string); }
    template<> std::string_view object::get<std::string_view>() const { if (!is<std::string_view>()) throw std::runtime_error("Wrong object type"); return std::string_view(str, token); }
    
#define GAME_STRUCTURE_FUNC(val) \
  template<> bool object::is<core::val*>() const noexcept { return type == static_cast<size_t>(type::val); } \
  template<> core::val* object::get<core::val*>() const { if (!is<core::val*>()) throw std::runtime_error("Wrong object type"); return reinterpret_cast<core::val*>(data); }
  
      GAME_STRUCTURES_LIST
      
#undef GAME_STRUCTURE_FUNC

#define GAME_STRUCTURE_FUNC(val) \
  template<> bool object::is<utils::handle<core::val>>() const noexcept { return type == static_cast<size_t>(type::val); } \
  template<> utils::handle<core::val> object::get<utils::handle<core::val>>() const { \
    if (!is<utils::handle<core::val>>()) throw std::runtime_error("Wrong object type"); \
    return utils::handle<core::val>(reinterpret_cast<core::val*>(data), token); \
  }
  
      GAME_STRUCTURES_LIST
      
#undef GAME_STRUCTURE_FUNC

    bool object::ignore() const noexcept { return type == static_cast<size_t>(type::number) && std::isnan(value); }
    bool object::valid() const noexcept { return type != static_cast<size_t>(type::invalid); }
    enum object::type object::get_type() const noexcept { return static_cast<enum object::type>(type); }
    
    const object ignore_value(std::numeric_limits<double>::signaling_NaN());
  }
}
