#include "lua_initialization.h"

#include <iostream>
#include "magic_enum.hpp"

namespace devils_engine {
  namespace utils {
//     std::string get_str_table(const sol::state_view &lua, const sol::table &t);
//     
//     std::string get_str_object(const sol::state_view &lua, const sol::object &obj) {
//       switch (obj.get_type()) {
//         case sol::type::boolean: return obj.as<bool>() ? "true" : "false";
//         case sol::type::string: return obj.as<std::string>();
//         case sol::type::number: return std::to_string(obj.as<double>());
//         case sol::type::lua_nil: return "nil";
//         case sol::type::table: return get_str_table(lua, obj.as<sol::table>());
//         case sol::type::lightuserdata:
//         case sol::type::userdata: return lua["type"](obj).get<std::string>();
//         case sol::type::function: return "function";
//         case sol::type::thread: return "thread";
//         default: return "bad_type";
//       }
//       
//       return "";
//     }
//     
//     std::string get_str_table(const sol::state_view &lua, const sol::table &t) {
//       std::string str = "{ ";
//       bool first = true;
//       for (const auto &obj : t) {
//         if (first) first = false;
//         else str += ", ";
//         str += "[" + get_str_object(lua, obj.first) + "] = " + get_str_object(lua, obj.second);
//       }
//       
//       str += " }"
//       return str;
//     }
    
    void setup_lua_loading_functions(sol::state_view lua) {
      auto core = lua[magic_enum::enum_name<reserved_lua::values>(reserved_lua::core)].get_or_create<sol::table>();
      core.set_function("is_loaded", [] (const std::string_view &mod_name) -> bool {
        if (mod_name == "apates_quest") return true;
        return false;
      });
      
//       core.set_function("error", [] (sol::this_state s, const std::string_view &err_string, const sol::variadic_args &args) {
//         sol::state_view lua = s;
//         std::string err = std::string(err_string);
//         for (const auto &obj : args) {
//           err += get_str_object(lua, obj);
//         }
//         
//         throw std::runtime_error(err);
//       });
      
      core.new_enum("image_type", {
        // эти картинки мы выгружаем при изменении состояния приложения, 
        // так что мы можем просто использовать тип биом для всех состояний (мапа, битва, герой)
        std::make_pair(std::string_view("biome"), 0)
        
      });
      
      core.new_enum("sampler_type", {
        // эти картинки мы выгружаем при изменении состояния приложения, 
        // так что мы можем просто использовать тип биом для всех состояний (мапа, битва, герой)
        std::make_pair(std::string_view("nearest"), 0),
        std::make_pair(std::string_view("linear"), 1)
        
      });
    }
  }
}
