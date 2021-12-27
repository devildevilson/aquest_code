#include "lua_initialization_internal.h"

#include "core/troop_type.h"
#include "core/stats.h"
#include "core/stats_table.h"

#include "lua_initialization_handle_types.h"

namespace devils_engine {
  namespace utils {
    namespace internal {
      static float get_base_stat(const core::troop_type* self, const sol::object &obj) {
        if (obj.get_type() != sol::type::string && obj.get_type() != sol::type::number) throw std::runtime_error("Bad troop type stat index type");
        
        if (obj.get_type() == sol::type::number) {
          const size_t stat_index = obj.as<size_t>();
          const size_t final_index = FROM_LUA_INDEX(stat_index);
          if (final_index < core::offsets::troop_stats || final_index >= core::offsets::troop_stats + core::troop_stats::count) {
            throw std::runtime_error("Bad troop type stat index " + std::to_string(final_index));
          }
          
          const size_t remove_offset = final_index - core::offsets::troop_stats;
          return self->stats.get(remove_offset);
        }
        
        const auto str = obj.as<std::string_view>();
        const auto itr = core::troop_stats::map.find(str);
        if (itr == core::troop_stats::map.end()) throw std::runtime_error("Bad troop type stat id " + std::string(str));
        const size_t final_index = itr->second;
        return self->stats.get(final_index);
      }
      
      void setup_lua_troop_type(sol::state_view lua) {
        auto core = lua["core"].get_or_create<sol::table>();
        core.new_usertype<core::troop_type>(
          "troop_type", sol::no_constructor,
          "id", sol::readonly(&core::troop_type::id),
          "name_id", sol::readonly(&core::troop_type::name_id),
          "description_id", sol::readonly(&core::troop_type::description_id),
          "get_base_stat", &get_base_stat
        );
      }
    }
  }
}
