#include "lua_initialization_internal.h"

#include "core/city.h"
#include "core/province.h"
#include "core/titulus.h"
#include "core/modificator.h"
#include "core/event.h"
#include "core/stats_table.h"
#include "core/troop.h"
#include "lua_initialization_handle_types.h"
#include "lua_initialization.h"
#include "magic_enum.hpp"
#include "lua_container_iterators.h"

#include <iostream>

namespace devils_engine {
  namespace utils {
    namespace internal {
      static float get_resource(const core::city* self, const sol::object &obj) {
        if (obj.get_type() != sol::type::string && obj.get_type() != sol::type::number) throw std::runtime_error("Bad city stat index type");
        
        if (obj.get_type() == sol::type::number) {
          const size_t stat_index = obj.as<size_t>();
          const size_t final_index = FROM_LUA_INDEX(stat_index);
          if (final_index < core::offsets::city_resources || final_index >= core::offsets::city_resources + core::city_resources::count) {
            throw std::runtime_error("Bad city stat index " + std::to_string(final_index));
          }
          
          const size_t remove_offset = final_index - core::offsets::city_resources;
          return self->resources.get(remove_offset);
        }
        
        const auto str = obj.as<std::string_view>();
        const auto itr = core::city_resources::map.find(str);
        if (itr == core::city_resources::map.end()) throw std::runtime_error("Bad city stat id " + std::string(str));
        const size_t final_index = itr->second;
        return self->resources.get(final_index);
      }
      
      static float get_stat(const core::city* self, const sol::object &obj) {
        if (obj.get_type() != sol::type::string && obj.get_type() != sol::type::number) throw std::runtime_error("Bad city stat index type");
        
        if (obj.get_type() == sol::type::number) {
          const size_t stat_index = obj.as<size_t>();
          const size_t final_index = FROM_LUA_INDEX(stat_index);
          if (final_index < core::offsets::city_stats || final_index >= core::offsets::city_stats + core::city_stats::count) {
            throw std::runtime_error("Bad city stat index " + std::to_string(final_index));
          }
          
          const size_t remove_offset = final_index - core::offsets::city_stats;
          return self->current_stats.get(remove_offset);
        }
        
        const auto str = obj.as<std::string_view>();
        const auto itr = core::city_stats::map.find(str);
        if (itr == core::city_stats::map.end()) throw std::runtime_error("Bad city stat id " + std::string(str));
        const size_t final_index = itr->second;
        return self->current_stats.get(final_index);
      }
      
      static float get_base_stat(const core::city* self, const sol::object &obj) {
        if (obj.get_type() != sol::type::string && obj.get_type() != sol::type::number) throw std::runtime_error("Bad city stat index type");
        
        if (obj.get_type() == sol::type::number) {
          const size_t stat_index = obj.as<size_t>();
          const size_t final_index = FROM_LUA_INDEX(stat_index);
          if (final_index < core::offsets::city_stats || final_index >= core::offsets::city_stats + core::city_stats::count) {
            throw std::runtime_error("Bad city stat index " + std::to_string(final_index));
          }
          
          const size_t remove_offset = final_index - core::offsets::city_stats;
          return self->stats.get(remove_offset);
        }
        
        const auto str = obj.as<std::string_view>();
        const auto itr = core::city_stats::map.find(str);
        if (itr == core::city_stats::map.end()) throw std::runtime_error("Bad city stat id " + std::string(str));
        const size_t final_index = itr->second;
        return self->stats.get(final_index);
      }
      
      void setup_lua_city(sol::state_view lua) {
        auto core = lua[magic_enum::enum_name<reserved_lua::core>()].get_or_create<sol::table>();
        sol::usertype<core::city> city = core.new_usertype<core::city>("city", sol::no_constructor,
          "province", sol::readonly(&core::city::province),
          "title", sol::readonly(&core::city::title),
          "type", sol::readonly(&core::city::type),
          "buildings_count", sol::readonly_property([] (const core::city* self) { return self->type->buildings_count; }),
          "available_building", [] (const core::city* self, const uint32_t &index) { return self->available(FROM_LUA_INDEX(index)); },
          "completed_building", [] (const core::city* self, const uint32_t &index) { return self->constructed(FROM_LUA_INDEX(index)); },
          "visible_building", [] (const core::city* self, const uint32_t &index) { return self->visible(FROM_LUA_INDEX(index)); },
          "has_building_project", &core::city::has_building_project,
          "can_build", [] (const core::city* self, const core::character* c, const uint32_t &index) { return self->check_build(c, FROM_LUA_INDEX(index)); },
          "start_building_project_turn", sol::readonly(&core::city::start_building),
          "building_project_index", sol::readonly_property([] (const core::city* self) { return TO_LUA_INDEX(self->building_index); }),
          "tile_index", sol::readonly_property([] (const core::city* self) { return TO_LUA_INDEX(self->tile_index); }),
          "get_stat", &get_stat,
          "get_base_stat", &get_base_stat,
          "get_resource", &get_resource,
//           "offensive_army", sol::readonly_property([] (const core::city* self) { return lua_handle_army(self->offensive_army); }),
//           "defensive_army", sol::readonly_property([] (const core::city* self) { return lua_handle_army(self->defensive_army); }),
          // возможно нужно добавить обход юнитов по зданиям?
          "offensive_troops", [] (const core::city* self, const sol::optional<double> &building_index) -> std::function<sol::object(sol::this_state)> {
            size_t buildings_iterator = building_index.has_value() ? building_index.value() : 0;
            size_t troops_iterator = 0;
            
            if (building_index.has_value()) {
              return [self, buildings_iterator, troops_iterator] (sol::this_state s) mutable -> sol::object {
                const auto & view = self->units_view[buildings_iterator];
                if (buildings_iterator >= self->units_view.size() && troops_iterator >= view.off_troops_count) return sol::object(sol::nil);
                
                assert(view.off_troops != nullptr);
                const auto h = view.off_troops[troops_iterator];
                ++troops_iterator;
                return sol::make_object(s, lua_handle_troop(h));
              };
            }
            
            return [self, buildings_iterator, troops_iterator] (sol::this_state s) mutable -> sol::object {
              while (buildings_iterator < self->units_view.size()) {
                const auto & view = self->units_view[buildings_iterator];
                if (troops_iterator >= view.off_troops_count) {
                  ++buildings_iterator;
                  troops_iterator = 0;
                  continue;
                }
                
                assert(view.off_troops != nullptr);
                const auto h = view.off_troops[troops_iterator];
                ++troops_iterator;
                return sol::make_object(s, lua_handle_troop(h));
              }
              
              return sol::object(sol::nil);
            };
          },
          "defensive_troops", [] (const core::city* self, const sol::optional<double> &building_index) -> std::function<sol::object(sol::this_state)> {
            size_t buildings_iterator = building_index.has_value() ? building_index.value() : 0;
            size_t troops_iterator = 0;
            
            if (building_index.has_value()) {
              return [self, buildings_iterator, troops_iterator] (sol::this_state s) mutable -> sol::object {
                const auto & view = self->units_view[buildings_iterator];
                if (buildings_iterator >= self->units_view.size() && troops_iterator >= view.off_troops_count) return sol::object(sol::nil);
                
                assert(view.off_troops != nullptr);
                const auto h = view.off_troops[troops_iterator];
                ++troops_iterator;
                return sol::make_object(s, lua_handle_troop(h));
              };
            }
            
            return [self, buildings_iterator, troops_iterator] (sol::this_state s) mutable {
              //using ptr_type = lua_ha;
              while (buildings_iterator < self->units_view.size()) {
                const auto & view = self->units_view[buildings_iterator];
                if (troops_iterator >= view.def_troops_count) {
                  ++buildings_iterator;
                  troops_iterator = 0;
                  continue;
                }
                
                assert(view.def_troops != nullptr);
                const auto h = view.def_troops[troops_iterator];
                ++troops_iterator;
                return sol::make_object(s, lua_handle_troop(h));
              }
              
              return sol::object(sol::nil);
            };
          },
          "find_building", [] (const core::city* self, const core::building_type* b) {
            const size_t index = self->find_building(b);
            return index == SIZE_MAX ? int64_t(-1) : int64_t(TO_LUA_INDEX(index));
          },
          "find_building_upgrade", [] (const core::city* self, const core::building_type* b, const sol::optional<double> &start) {
            const size_t start_val = start.has_value() ? start.value() : 0;
            const size_t index = self->find_building_upgrade(b, start_val);
            return index == SIZE_MAX ? int64_t(-1) : int64_t(TO_LUA_INDEX(index));
          },
          "flags", &utils::flags_iterator<core::city>,
          "events", &utils::events_iterator<core::city>,
          "modificators", &utils::modificators_iterator<core::city>,
          "maximum_buildings", sol::var(core::city_type::maximum_buildings),
          "stats_start", sol::var(TO_LUA_INDEX(core::offsets::city_stats)),
          "resources_start", sol::var(TO_LUA_INDEX(core::offsets::city_resources)),
          "stats_end", sol::var(core::offsets::city_stats + core::city_stats::count),
          "resources_end", sol::var(core::offsets::city_resources + core::city_resources::count)
        );
        
        core.set_function("each_offensive_troops", [] (const core::city* city, const sol::function &func) {
          if (city == nullptr) throw std::runtime_error("Invalid city");
          if (!func.valid()) throw std::runtime_error("Invalid function");
                          
          bool exit = false;
          for (size_t i = 0; i < city->units_view.size() && !exit; ++i) {
            for (size_t j = 0; j < city->units_view[i].off_troops_count; ++j) {
              const auto h = lua_handle_troop(city->units_view[i].off_troops[j]);
              const auto ret = func(h);
              if (!ret.valid()) {
                sol::error err = ret;
                std::cout << err.what();
                throw std::runtime_error("There is lua errors");
              }
              
              if (ret.get_type() == sol::type::boolean) {
                const bool val = ret;
                if (val) { exit = true; break; }
              }
            }
          }
        });
        
        core.set_function("each_defensive_troops", [] (const core::city* city, const sol::function &func) {
          if (city == nullptr) throw std::runtime_error("Invalid city");
          if (!func.valid()) throw std::runtime_error("Invalid function");
                          
          bool exit = false;
          for (size_t i = 0; i < city->units_view.size() && !exit; ++i) {
            for (size_t j = 0; j < city->units_view[i].def_troops_count; ++j) {
              const auto h = lua_handle_troop(city->units_view[i].def_troops[j]);
              const auto ret = func(h);
              if (!ret.valid()) {
                sol::error err = ret;
                std::cout << err.what();
                throw std::runtime_error("There is lua errors");
              }
              
              if (ret.get_type() == sol::type::boolean) {
                const bool val = ret;
                if (val) { exit = true; break; }
              }
            }
          }
        });
      }
    }
  }
}
