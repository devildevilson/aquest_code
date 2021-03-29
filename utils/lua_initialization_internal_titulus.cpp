#include "lua_initialization_internal.h"

#include "bin/core_structures.h"
#include "lua_initialization.h"
#include "magic_enum.hpp"

namespace devils_engine {
  namespace utils {
    namespace internal {
      void setup_lua_titulus(sol::state_view lua) {
        auto t = lua.create_table_with(
          "city", core::titulus::type::city,
          "baron", core::titulus::type::baron,
          "duke", core::titulus::type::duke,
          "king", core::titulus::type::king,
          "imperial", core::titulus::type::imperial
        );
        
        auto core = lua[magic_enum::enum_name<reserved_lua::core>()].get_or_create<sol::table>();
        sol::usertype<core::titulus> title_type = core.new_usertype<core::titulus>("titulus",
          sol::no_constructor,
          "id", sol::readonly(&core::titulus::id),
          "type", sol::readonly(&core::titulus::type),
          "count", sol::readonly(&core::titulus::count),
          "parent", sol::readonly(&core::titulus::parent),
          "owner", sol::readonly(&core::titulus::owner),
          "name_id", sol::readonly(&core::titulus::name_str),
          "desc_id", sol::readonly(&core::titulus::description_str),
          "adj_id", sol::readonly(&core::titulus::adjective_str),
          "next", sol::readonly(&core::titulus::next),
          "prev", sol::readonly(&core::titulus::prev),
          "events", sol::readonly(&core::titulus::events),
          "flags", sol::readonly(&core::titulus::flags),
          "is_formal", &core::titulus::is_formal,
  //           "set_child", &core::titulus::set_child,
          "get_child", &core::titulus::get_child,
          "get_province", &core::titulus::get_province,
          "get_city", &core::titulus::get_city,
          "events_container_size", sol::var(core::titulus::events_container_size),
          "flags_container_size", sol::var(core::titulus::flags_container_size),
          // наименования
          "types", t
        );

        //auto title_type_table = core["titulus"].get_or_create<sol::table>();
//         if (!title_type["type"].valid()) {
//           auto title_type_type = title_type.new_enum("type",
//             "city", core::titulus::type::city,
//             "baron", core::titulus::type::baron,
//             "duke", core::titulus::type::duke,
//             "king", core::titulus::type::king,
//             "imperial", core::titulus::type::imperial
//           );
//         }
      }
    }
  }
}
