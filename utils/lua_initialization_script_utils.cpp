#include "lua_initialization_hidden.h"

#include "script/header.h"
#include "magic_enum_header.h"
//#include "bin/core_structures.h"
#include "core/structures_header.h"
#include "re2/re2.h"
#include "script/context.h"
#include "script/init_functions.h"
#include "script/core.h"
#include "lua_initialization_handle_types.h"

namespace devils_engine {
  namespace utils {
    static const RE2 regex_dot_matcher(script::dot_matcher);
    static const RE2 regex_colon_matcher(script::colon_matcher);
    static const RE2 regex_number_matcher(script::number_matcher);
    
    static size_t unique_counter = 1;
    
    // расходы, как описать сравнение? в имени указать? да, наверное
    void set_stat_function(sol::table &t, const uint32_t &enum_id) {
//       const std::string name = std::string(magic_enum::enum_name(static_cast<script::data_source_type::values>(enum_id)));
//       t.set_function(name, [enum_id] (const double &num) {
//         const auto abc = magic_enum::enum_name(static_cast<script::data_source_type::values>(enum_id));
//         return std::string(abc) + ":" + 
//           std::string(magic_enum::enum_name<script::data_type::values>(script::data_type::equal)) + ":" + 
//           std::to_string(num);
//       });
      
//       for (uint32_t i = 0; i < script::compare_operators::count; ++i) {
//         const std::string name = std::string(script::compare_operators::names[i]) + "_" + std::string(magic_enum::enum_name(static_cast<script::data_source_type::values>(enum_id)));
//         t.set_function(name, [enum_id, i] (const double &num) {
//           const auto abc = magic_enum::enum_name(static_cast<script::data_source_type::values>(enum_id));
//           const auto type = magic_enum::enum_name(static_cast<script::data_type::values>(i));
//           return std::string(abc) + ":" + 
//             std::string(type) + ":" + 
//             std::to_string(num);
//         });
//       }
    }
    
    template <typename T>
    sol::object make_handle_object(sol::this_state s, const script::object obj) {
      if      constexpr (std::is_same_v<T, core::realm>)      return sol::make_object(s, lua_handle_realm(obj.get<utils::handle<T>>()));
      else if constexpr (std::is_same_v<T, core::army>)       return sol::make_object(s, lua_handle_army(obj.get<utils::handle<T>>()));
      else if constexpr (std::is_same_v<T, core::hero_troop>) return sol::make_object(s, lua_handle_hero_troop(obj.get<utils::handle<T>>()));
      else if constexpr (std::is_same_v<T, core::war>)        return sol::make_object(s, lua_handle_war(obj.get<utils::handle<T>>()));
      else if constexpr (std::is_same_v<T, core::troop>)      return sol::make_object(s, lua_handle_troop(obj.get<utils::handle<T>>()));
      throw std::runtime_error("Bad handle type");
      return sol::nil;
    }
    
#define OBJECT_TYPE_CASE(name) case script::object::type::name: {         \
      if (obj.is_handle()) return make_handle_object<core::name>(s, obj); \
      return sol::make_object(s, obj.get<const core::name*>());           \
    }                                                                     \

    sol::object make_lua_object(sol::this_state s, const script::object &obj) {
      switch (obj.get_type()) {
#define GAME_STRUCTURE_FUNC(name) OBJECT_TYPE_CASE(name)
        GAME_STRUCTURES_LIST
#undef GAME_STRUCTURE_FUNC
        case script::object::type::boolean: return sol::make_object(s, obj.get<bool>());
        case script::object::type::number: return sol::make_object(s, obj.get<double>());
        case script::object::type::string: return sol::make_object(s, obj.get<std::string_view>());
        default: break;
      }
      
      return sol::nil;
    }
    
#undef OBJECT_TYPE_CASE
    
    void setup_lua_script_utils(sol::state_view lua) {
      auto script_utils = lua["script_utils"].get_or_create<sol::table>();
      
      //script_utils.new_usertype<script::script_data>("script_data");
      script_utils.new_usertype<script::draw_data>(
        "draw_data", sol::no_constructor,
        "id", &script::draw_data::id,
        "method_name", &script::draw_data::method_name,
        "function_name", &script::draw_data::function_name,
        "prev_function_name", &script::draw_data::prev_function_name,
        "type", &script::draw_data::type, // пока не понимаю что с типом делать
        "operator_type", &script::draw_data::operator_type,
        "nest_level", &script::draw_data::nest_level,
        "current", sol::readonly_property([] (sol::this_state s, const script::draw_data* self) { return make_lua_object(s, self->current); }),
        "value", sol::readonly_property([] (sol::this_state s, const script::draw_data* self) { return make_lua_object(s, self->value); }),
        "original", sol::readonly_property([] (sol::this_state s, const script::draw_data* self) { return make_lua_object(s, self->original); }),
        // аргументы? составить таблицу нужно из них
        "arguments", sol::readonly_property([] (sol::this_state s, const script::draw_data* self) { 
          sol::state_view lua = s;
          auto table = lua.create_table(0, 16);
          for (size_t i = 0; i < self->arguments.size() && !self->arguments[i].first.empty(); ++i) {
            table.set(self->arguments[i].first, make_lua_object(s, self->arguments[i].second));
          }
          return table;
        })
      );
      
      script_utils.set_function("more", [] (const double &num) {
        return std::to_string(unique_counter++) + ":" + 
               std::string(script::compare_operators::names[script::compare_operators::more]) + ":" + 
               std::to_string(num);
      });
      
      script_utils.set_function("equal", [] (const double &num) {
        return std::to_string(unique_counter++) + ":" + 
               std::string(script::compare_operators::names[script::compare_operators::equal]) + ":" + 
               std::to_string(num);
      });
      
      script_utils.set_function("less", [] (const double &num) {
        return std::to_string(unique_counter++) + ":" + 
               std::string(script::compare_operators::names[script::compare_operators::less]) + ":" + 
               std::to_string(num);
      });
      
      script_utils.set_function("more_eq", [] (const double &num) {
        return std::to_string(unique_counter++) + ":" + 
               std::string(script::compare_operators::names[script::compare_operators::more_eq]) + ":" + 
               std::to_string(num);
      });
      
      script_utils.set_function("less_eq", [] (const double &num) {
        return std::to_string(unique_counter++) + ":" + 
               std::string(script::compare_operators::names[script::compare_operators::less_eq]) + ":" + 
               std::to_string(num);
      });
      
      script_utils.set_function("not_eq", [] (const double &num) {
        return std::to_string(unique_counter++) + ":" + 
               std::string(script::compare_operators::names[script::compare_operators::not_equal]) + ":" + 
               std::to_string(num);
      });
      
//       for (uint32_t i = script::data_source_type::stats_start+1; i < script::data_source_type::count; ++i) {
//         set_stat_function(script_utils, i);
//       }
      
      script_utils.set_function("context", [] (const sol::string_view &id) {
        return std::to_string(unique_counter++) + 
               ":""context"":" + 
               std::string(id);
      });
      
      script_utils.new_enum("decision", {
        //std::make_pair(magic_enum::enum_name(core::decision::type::diplomatic), core::decision::type::diplomatic),
        std::make_pair(magic_enum::enum_name(core::decision::type::major), core::decision::type::major),
        std::make_pair(magic_enum::enum_name(core::decision::type::minor), core::decision::type::minor)
      });
    }
  }
}
