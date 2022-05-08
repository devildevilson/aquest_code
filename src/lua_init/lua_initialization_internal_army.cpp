#include "lua_initialization_internal.h"

#include "core/army.h"
#include "core/troop.h"
#include "core/stats_table.h"
#include "core/city.h"
#include "core/character.h"
#include "core/province.h"
#include "lua_initialization_handle_types.h"

#include <iostream>

namespace devils_engine {
  namespace utils {
    namespace internal {
      static float get_stat(const core::army* self, const sol::object &obj) {
        const auto type = obj.get_type();
        if (type != sol::type::string && type != sol::type::number) throw std::runtime_error("Bad army stat index type");
        
        if (type == sol::type::number) {
          const size_t stat_index = obj.as<size_t>();
          const size_t final_index = FROM_LUA_INDEX(stat_index);
          if (final_index < core::offsets::army_stats || final_index >= core::offsets::army_stats + core::army_stats::count) {
            throw std::runtime_error("Bad army stat index " + std::to_string(final_index));
          }
          
          const size_t remove_offset = final_index - core::offsets::army_stats;
          return self->current_stats.get(remove_offset);
        }
        
        const auto str = obj.as<std::string_view>();
        const auto itr = core::army_stats::map.find(str);
        if (itr == core::army_stats::map.end()) throw std::runtime_error("Bad army stat id " + std::string(str));
        const size_t final_index = itr->second;
        return self->current_stats.get(final_index);
      }
      
      static float get_base_stat(const core::army* self, const sol::object &obj) {
        const auto type = obj.get_type();
        if (type != sol::type::string && type != sol::type::number) throw std::runtime_error("Bad army stat index type");
        
        if (type == sol::type::number) {
          const size_t stat_index = obj.as<size_t>();
          const size_t final_index = FROM_LUA_INDEX(stat_index);
          if (final_index < core::offsets::army_stats || final_index >= core::offsets::army_stats + core::army_stats::count) {
            throw std::runtime_error("Bad army stat index " + std::to_string(final_index));
          }
          
          const size_t remove_offset = final_index - core::offsets::army_stats;
          return self->stats.get(remove_offset);
        }
        
        const auto str = obj.as<std::string_view>();
        const auto itr = core::army_stats::map.find(str);
        if (itr == core::army_stats::map.end()) throw std::runtime_error("Bad army stat id " + std::string(str));
        const size_t final_index = itr->second;
        return self->stats.get(final_index);
      }
      
      static float get_resource(const core::army* self, const sol::object &obj) {
        const auto type = obj.get_type();
        if (type != sol::type::string && type != sol::type::number) throw std::runtime_error("Bad army resource index type");
        
        if (type == sol::type::number) {
          const size_t stat_index = obj.as<size_t>();
          const size_t final_index = FROM_LUA_INDEX(stat_index);
          if (final_index < core::offsets::army_resources || final_index >= core::offsets::army_resources + core::army_resources::count) {
            throw std::runtime_error("Bad army resource index " + std::to_string(final_index));
          }
          
          const size_t remove_offset = final_index - core::offsets::army_resources;
          return self->resources.get(remove_offset);
        }
        
        const auto str = obj.as<std::string_view>();
        const auto itr = core::army_resources::map.find(str);
        if (itr == core::army_resources::map.end()) throw std::runtime_error("Bad army resource id " + std::string(str));
        const size_t final_index = itr->second;
        return self->resources.get(final_index);
      }
      
      void setup_lua_army(sol::state_view lua) {
        auto core = lua["core"].get_or_create<sol::table>();
        core.new_usertype<core::army>(
          "army", sol::no_constructor,
//           "troops", sol::readonly_property([] (const core::army* self) { return std::ref(self->troops); }),
//           "troops_count", sol::readonly_property(&core::army::troops_count),
          "origin", sol::readonly_property(&core::army::origin),
          "general", sol::readonly_property(&core::army::general),
          "troops", [] (const core::army* self) {
            auto troop = self->troops;
            auto next_troop = self->troops;
            return [troop, next_troop] (sol::this_state s) mutable {
              auto final_troop = next_troop;
              auto t = ring::list_next<list_type::army_troops>(next_troop.get(), troop.get());
              if (t == nullptr) return sol::object(sol::nil);
              next_troop = utils::handle<core::troop>(t, t->object_token);
              return sol::make_object(s, lua_handle_troop(final_troop));
            };
          },
          "tile_index", sol::readonly_property([] (const core::army* self) { return TO_LUA_INDEX(self->tile_index); }),
          "find_path", [] (core::army* self, const uint32_t &tile_index) { self->find_path(FROM_LUA_INDEX(tile_index)); },
          "can_advance", &core::army::can_advance,
          "can_full_advance", &core::army::can_full_advance,
          "advance", &core::army::advance,
          "finding_path", &core::army::finding_path,
          "has_path", &core::army::has_path,
          "path_not_found", &core::army::path_not_found,
          "clear_path", &core::army::clear_path,
          "get_stat", &get_stat,
          "get_base_stat", &get_base_stat,
          "get_resource", &get_resource,
          "can_be_raized", &core::army::can_be_raized,
          "can_be_returned", &core::army::can_be_returned,
          "return_army", &core::army::return_army,
          "stats_start", sol::var(TO_LUA_INDEX(core::offsets::army_stats)),
          "stats_end", sol::var(core::offsets::army_stats + core::army_stats::count),
          "resources_start", sol::var(TO_LUA_INDEX(core::offsets::army_resources)),
          "resources_end", sol::var(core::offsets::army_resources + core::army_resources::count)
        );
        
        core.set_function("each_troop", [] (const lua_handle_army handle, const sol::function &function) {
          if (!handle.valid()) throw std::runtime_error("Invalid army");
          if (!function.valid()) throw std::runtime_error("Invalid function");
          
          auto ptr = handle.get();
          auto troop = ptr->troops.get();
          auto next_troop = ptr->troops.get();
          
          while (next_troop != nullptr) {
            const auto ret = function(next_troop);
            if (!ret.valid()) {
              sol::error err = ret;
              std::cout << err.what();
              throw std::runtime_error("There is lua errors");
            }
            
            if (ret.get_type() == sol::type::boolean) {
              const bool val = ret;
              if (val) break;
            }
            
            next_troop = ring::list_next<list_type::army_troops>(next_troop, troop);
          }
        });
        
        auto handle = core.new_usertype<lua_handle_army>(
          "army_handle", sol::no_constructor,
          "get", &lua_handle_army::get,
          "valid", &lua_handle_army::valid
        );
      }
    }
  }
}
