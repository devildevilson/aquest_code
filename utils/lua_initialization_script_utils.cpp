#include "lua_initialization.h"

#include "script/script_header.h"
#include "magic_enum_header.h"
//#include "bin/core_structures.h"
#include "core/structures_header.h"

namespace devils_engine {
  namespace utils {
    // расходы, как описать сравнение? в имени указать? да, наверное
    void set_stat_function(sol::table &t, const uint32_t &enum_id) {
      const std::string name = std::string(magic_enum::enum_name(static_cast<script::data_source_type::values>(enum_id)));
      t.set_function(name, [enum_id] (const double &num) {
        const auto abc = magic_enum::enum_name(static_cast<script::data_source_type::values>(enum_id));
        return std::string(abc) + ":" + 
          std::string(magic_enum::enum_name<script::data_type::values>(script::data_type::equal)) + ":" + 
          std::to_string(num);
      });
      
      for (uint32_t i = script::data_type::equal; i < script::data_type::index; ++i) {
        const std::string name = std::string(magic_enum::enum_name(static_cast<script::data_type::values>(i))) + "_" + std::string(magic_enum::enum_name(static_cast<script::data_source_type::values>(enum_id)));
        t.set_function(name, [enum_id, i] (const double &num) {
        const auto abc = magic_enum::enum_name(static_cast<script::data_source_type::values>(enum_id));
        const auto type = magic_enum::enum_name(static_cast<script::data_type::values>(i));
        return std::string(abc) + ":" + 
          std::string(type) + ":" + 
          std::to_string(num);
      });
      }
    }
    
    void setup_lua_script_utils(sol::state_view lua) {
      auto script_utils = lua["script_utils"].get_or_create<sol::table>();
      
      //script_utils.new_usertype<script::script_data>("script_data");
      
      script_utils.set_function("more", [] (const double &num) {
        return std::string(magic_enum::enum_name(script::data_source_type::value)) + ":" + 
               std::string(magic_enum::enum_name(script::data_type::more)) + ":" + 
               std::to_string(num);
      });
      
      script_utils.set_function("equal", [] (const double &num) {
        return std::string(magic_enum::enum_name(script::data_source_type::value)) + ":" + 
               std::string(magic_enum::enum_name(script::data_type::equal)) + ":" + 
               std::to_string(num);
      });
      
      script_utils.set_function("less", [] (const double &num) {
        return std::string(magic_enum::enum_name(script::data_source_type::value)) + ":" + 
               std::string(magic_enum::enum_name(script::data_type::less)) + ":" + 
               std::to_string(num);
      });
      
      script_utils.set_function("more_eq", [] (const double &num) {
        return std::string(magic_enum::enum_name(script::data_source_type::value)) + ":" + 
               std::string(magic_enum::enum_name(script::data_type::more_eq)) + ":" + 
               std::to_string(num);
      });
      
      script_utils.set_function("less_eq", [] (const double &num) {
        return std::string(magic_enum::enum_name(script::data_source_type::value)) + ":" + 
               std::string(magic_enum::enum_name(script::data_type::less_eq)) + ":" + 
               std::to_string(num);
      });
      
      script_utils.set_function("not_eq", [] (const double &num) {
        return std::string(magic_enum::enum_name(script::data_source_type::value)) + ":" + 
               std::string(magic_enum::enum_name(script::data_type::not_equal)) + ":" + 
               std::to_string(num);
      });
      
      for (uint32_t i = script::data_source_type::stats_start+1; i < script::data_source_type::count; ++i) {
        set_stat_function(script_utils, i);
      }
      
      script_utils.set_function("context", [] (const sol::object &val) {
        //script::script_data d;
        
        if (val.get_type() == sol::type::number) {
          const double num = val.as<int64_t>();
          return std::string(magic_enum::enum_name(script::data_source_type::context)) + ":" + 
                 std::string(magic_enum::enum_name(script::data_type::index)) + ":" + 
                 std::to_string(num);
        } else if (val.get_type() == sol::type::string) {
          const std::string id = val.as<std::string>();
          if (id == "index") throw std::runtime_error("Invalid context variable id 'index'");
          return std::string(magic_enum::enum_name(script::data_source_type::context)) + ":" + 
                 id + ":" + 
                 std::to_string(0);
        }
        
        throw std::runtime_error("Bad context value type");
        return std::string();
      });
      
//       script_utils.set_function("is_script_data", [] (const sol::object &val) {
//         return val.is<script::script_data>();
//       });
      
      script_utils.new_enum("decision", {
        std::make_pair(magic_enum::enum_name(core::decision::type::diplomatic), core::decision::type::diplomatic),
        std::make_pair(magic_enum::enum_name(core::decision::type::major), core::decision::type::major),
        std::make_pair(magic_enum::enum_name(core::decision::type::minor), core::decision::type::minor)
      });
      
      script_utils.new_enum("target", {
        std::make_pair(magic_enum::enum_name(core::target_type::province), core::target_type::province),
        std::make_pair(magic_enum::enum_name(core::target_type::city), core::target_type::city),
        std::make_pair(magic_enum::enum_name(core::target_type::religion), core::target_type::religion),
        std::make_pair(magic_enum::enum_name(core::target_type::culture), core::target_type::culture),
        std::make_pair(magic_enum::enum_name(core::target_type::title), core::target_type::title),
        std::make_pair(magic_enum::enum_name(core::target_type::character), core::target_type::character),
        std::make_pair(magic_enum::enum_name(core::target_type::dynasty), core::target_type::dynasty),
        std::make_pair(magic_enum::enum_name(core::target_type::realm), core::target_type::realm),
        std::make_pair(magic_enum::enum_name(core::target_type::hero_troop), core::target_type::hero_troop),
        std::make_pair(magic_enum::enum_name(core::target_type::army), core::target_type::army),
        std::make_pair(magic_enum::enum_name(core::target_type::boolean), core::target_type::boolean),
        std::make_pair(magic_enum::enum_name(core::target_type::number), core::target_type::number),
        std::make_pair(magic_enum::enum_name(core::target_type::string), core::target_type::string),
      });
    }
  }
}
