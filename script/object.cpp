#include "object.h"

#include <stdexcept>
#include <limits>
#include "utils/shared_mathematical_constant.h"
#include "utils/type_info.h"
#include "core/declare_structures_table.h"
#include "utils/constexpr_funcs.h"
#include "fmt/format.h"

namespace devils_engine {
  namespace script {
    const size_t const_mask = size_t(1) << (SIZE_WIDTH-1);
    const size_t handle_mask = size_t(1) << (SIZE_WIDTH-2);
    const size_t type_mask = make_mask(SIZE_WIDTH-2);
    static_assert(handle_mask > type_mask);
    
    std::string_view get_game_type_name(const size_t &type) noexcept {
#define GAME_TYPE_CASE(name) case script::object::type::name: return core::structure_data::names[static_cast<size_t>(core::structure::name)];
#define GAME_STRUCTURE_FUNC(name) GAME_TYPE_CASE(name)
      switch (static_cast<enum object::type>(type & type_mask)) {
        GAME_STRUCTURES_LIST
        case script::object::type::boolean: return "boolean";
        case script::object::type::number:  return "number";
        case script::object::type::string:  return "string";
        case script::object::type::invalid: return "invalid";
        case script::object::type::count: break;
      }
#undef GAME_STRUCTURE_FUNC
#undef GAME_TYPE_CASE
      return "";
    }
    
    std::string parse_type_bits(const size_t &type_bits) noexcept {
      std::string out;
      for (size_t i = 0; i < size_t(object::type::count); ++i) {
        const size_t mask = size_t(1) << i;
        if ((type_bits & mask) != mask) continue;
        
        if (!out.empty()) out += " or ";
        out += get_game_type_name(i);
      }
      
      return out;
    }
    
    const size_t object::maximum_types_count;
    const size_t object::type_bit::all;
    const size_t object::type_bit::none;
    const size_t object::type_bit::valid_number;
    const size_t object::type_bit::valid_boolean;
    const size_t object::type_bit::all_objects;
    
    size_t object::make_const_type(const size_t type) {
      return type | const_mask;
    }
    
    size_t object::make_handle_type(const size_t type) {
      return type | handle_mask;
    }
    
    object::object() noexcept : type(static_cast<size_t>(type::invalid)), data(nullptr), token(SIZE_MAX) {}
    object::object(const bool val) noexcept : type(static_cast<size_t>(type::boolean)), data(nullptr), value(val) {}
    object::object(const double &val) noexcept : type(static_cast<size_t>(type::number)), data(nullptr), value(val) {}
    object::object(const std::string_view &val) noexcept : type(static_cast<size_t>(type::string)), str(val.data()), token(val.size()) {}
    
#define BOOL_RETURN (!(std::abs(value) < EPSILON))
    
    template<> bool object::is<bool>() const noexcept { return get_type() == type::boolean; }
    template<> bool object::get<bool>() const { 
      if (!is<bool>() && (get_type() != type::number || ignore())) 
        throw_obj_error("boolean");
      return BOOL_RETURN;
    }
    template<> bool object::is<double>() const noexcept { return get_type() == type::number && !std::isnan(value); }
    template<> double object::get<double>() const { 
      if (!is<bool>() && !is<double>()) throw_obj_error("number");
      return is<bool>() ? double(BOOL_RETURN) : value;
    }
    template<> bool object::is<std::string_view>() const noexcept { return get_type() == type::string; }
    template<> std::string_view object::get<std::string_view>() const { if (!is<std::string_view>()) throw_obj_error("string"); return std::string_view(str, token); }
    
#define GAME_STRUCTURE_FUNC(val)                                                                                     \
  template<> bool object::is<core::val*>() const noexcept { return !is_const() && !is_handle() && get_type() == type::val; } \
  template<> core::val* object::get<core::val*>() const {                                                            \
    if (!is<core::val*>())                                                                                           \
      throw_obj_error(get_game_type_name(static_cast<size_t>(script::object::type::val)));                           \
    return reinterpret_cast<core::val*>(data);                                                                       \
  }
  
      GAME_STRUCTURES_LIST
      
#undef GAME_STRUCTURE_FUNC

#define GAME_STRUCTURE_FUNC(val)                                                                                     \
  template<> bool object::is<const core::val*>() const noexcept { return !is_handle() && get_type() == type::val; }  \
  template<> const core::val* object::get<const core::val*>() const {                                                \
    if (!is<const core::val*>())                                                                                     \
      throw_obj_error_const(get_game_type_name(static_cast<size_t>(script::object::type::val)));                     \
    return reinterpret_cast<const core::val*>(const_data);                                                           \
  }
  
      GAME_STRUCTURES_LIST
      
#undef GAME_STRUCTURE_FUNC

#define GAME_STRUCTURE_FUNC(val)                                                                                           \
  template<> bool object::is<utils::handle<core::val>>() const noexcept { return is_handle() && get_type() == type::val; } \
  template<> utils::handle<core::val> object::get<utils::handle<core::val>>() const {                                      \
    if (!is<utils::handle<core::val>>())                                                                                   \
      throw_obj_error_handle(get_game_type_name(static_cast<size_t>(script::object::type::val)));                          \
    return utils::handle<core::val>(reinterpret_cast<core::val*>(data), token);                                            \
  }
  
      GAME_STRUCTURES_LIST
      
#undef GAME_STRUCTURE_FUNC

    bool object::ignore() const noexcept { return get_type() == type::number && std::isnan(value); }
    bool object::valid() const noexcept { return get_type() != type::invalid; }
    enum object::type object::get_type() const noexcept { return static_cast<enum object::type>(type & type_mask); }
    bool object::is_const() const noexcept { return (type & const_mask) == const_mask; }
    bool object::is_handle() const noexcept { return (type & handle_mask) == handle_mask; }
    
    // сюда нужно добавить данные контекста и текущей функции
    const std::string_view err_format = "Wrong object type: expected {} {}, stored {} {}";
    const char* get_attrib(const bool is_const, const bool is_handle) { return is_const ? "const " : (is_handle ? "handle " : ""); }
    
    void object::throw_obj_error(const std::string_view &expected_type_name) const {
      const auto str = fmt::format(err_format, "",       expected_type_name, get_attrib(is_const(), is_handle()), ignore() ? "ignore value" : get_game_type_name(type));
      throw std::runtime_error(str);
    }
    
    void object::throw_obj_error_const(const std::string_view &expected_type_name) const {
      const auto str = fmt::format(err_format, "const ", expected_type_name, get_attrib(is_const(), is_handle()), ignore() ? "ignore value" : get_game_type_name(type));
      throw std::runtime_error(str);
    }
    
    void object::throw_obj_error_handle(const std::string_view &expected_type_name) const {
      const auto str = fmt::format(err_format, "handle ", expected_type_name, get_attrib(is_const(), is_handle()), ignore() ? "ignore value" : get_game_type_name(type));
      throw std::runtime_error(str);
    }
    
    bool object::operator==(const object &other) const noexcept {
      if (!lazy_compare_types(other)) return false;
      const bool valid_num = get_type() == type::number || get_type() == type::boolean;
      if (valid_num) return std::abs(value - other.value) < EPSILON;
      if (get_type() == type::string) return std::string_view(str, token) == std::string_view(other.str, other.token);
      return data == other.data && token == other.token;
    }
    
    bool object::operator!=(const object &other) const noexcept {
      return !(*this == other);
    }
    
    bool object::lazy_compare_types(const object &other) const noexcept {
      const bool other_valid_num = other.get_type() == type::number || other.get_type() == type::boolean;
      const bool valid_num = get_type() == type::number || get_type() == type::boolean;
      if (ignore() || other.ignore()) return false;
      if (other_valid_num && valid_num) return true;
      return get_type() == other.get_type();
    }
    
    const object ignore_value(std::numeric_limits<double>::quiet_NaN());
    //static_assert(ignore_value.ignore());
  }
}
