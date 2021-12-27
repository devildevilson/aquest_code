#include "lua_initialization_internal.h"

#include "core/troop.h"
#include "core/troop_type.h"
#include "core/army.h"
#include "core/city.h"
#include "core/building_type.h"
#include "core/character.h"
#include "core/stats.h"
#include "core/stats_table.h"

#include "lua_initialization_handle_types.h"

namespace devils_engine {
  namespace utils {
    namespace internal {
      static float get_stat(const core::troop* self, const sol::object &obj) {
        if (obj.get_type() != sol::type::string && obj.get_type() != sol::type::number) throw std::runtime_error("Bad troop stat index type");
        
        if (obj.get_type() == sol::type::number) {
          const size_t stat_index = obj.as<size_t>();
          const size_t final_index = FROM_LUA_INDEX(stat_index);
          if (final_index < core::offsets::troop_stats || final_index >= core::offsets::troop_stats + core::troop_stats::count) {
            throw std::runtime_error("Bad troop stat index " + std::to_string(final_index));
          }
          
          const size_t remove_offset = final_index - core::offsets::troop_stats;
          return self->current_stats.get(remove_offset);
        }
        
        const auto str = obj.as<std::string_view>();
        const auto itr = core::troop_stats::map.find(str);
        if (itr == core::troop_stats::map.end()) throw std::runtime_error("Bad troop stat id " + std::string(str));
        const size_t final_index = itr->second;
        return self->current_stats.get(final_index);
      }
      
      static float get_base_stat(const core::troop* self, const sol::object &obj) {
        if (obj.get_type() != sol::type::string && obj.get_type() != sol::type::number) throw std::runtime_error("Bad troop stat index type");
        
        if (obj.get_type() == sol::type::number) {
          const size_t stat_index = obj.as<size_t>();
          const size_t final_index = FROM_LUA_INDEX(stat_index);
          if (final_index < core::offsets::troop_stats || final_index >= core::offsets::troop_stats + core::troop_stats::count) {
            throw std::runtime_error("Bad troop stat index " + std::to_string(final_index));
          }
          
          const size_t remove_offset = final_index - core::offsets::troop_stats;
          return self->stats.get(remove_offset);
        }
        
        const auto str = obj.as<std::string_view>();
        const auto itr = core::troop_stats::map.find(str);
        if (itr == core::troop_stats::map.end()) throw std::runtime_error("Bad troop stat id " + std::string(str));
        const size_t final_index = itr->second;
        return self->stats.get(final_index);
      }
      
      void setup_lua_troop(sol::state_view lua) {
        auto core = lua["core"].get_or_create<sol::table>();
        core.new_usertype<core::troop>(
          "troop", sol::no_constructor,
          "type", sol::readonly(&core::troop::type),
          "origin", sol::readonly(&core::troop::origin),
          "provider", sol::readonly(&core::troop::provider),
          "formation", sol::readonly_property([] (const core::troop* self) { return lua_handle_army(self->formation); }),
          "character", sol::readonly(&core::troop::character),
          "get_stat", &get_stat,
          "get_base_stat", &get_base_stat,
          "stats_start", sol::var(TO_LUA_INDEX(core::offsets::troop_stats)),
          "stats_end", sol::var(core::offsets::troop_stats + core::troop_stats::count)
        );
      }
    }
  }
}
