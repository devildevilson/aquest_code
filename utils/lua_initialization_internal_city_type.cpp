#include "lua_initialization_internal.h"

#include "core/city_type.h"
#include "core/building_type.h"
#include "core/stats_table.h"
#include "lua_initialization.h"
#include "magic_enum.hpp"

namespace devils_engine {
  namespace utils {
    namespace internal {
      void setup_lua_city_type(sol::state_view lua) {
        auto core = lua[magic_enum::enum_name<reserved_lua::core>()].get_or_create<sol::table>();
        sol::usertype<core::city_type> city_type = core.new_usertype<core::city_type>("city_type",
          sol::no_constructor,
          "id", sol::readonly(&core::city_type::id),
          "name_id", sol::readonly(&core::city_type::name_id),
          "description_id", sol::readonly(&core::city_type::description_id),
          "buildings", sol::readonly_property([] (const core::city_type* self) { return std::ref(self->buildings); }),
          "each_building", [] (const core::city_type* self) {
            size_t counter = 0;
            return [self, counter] () mutable {
              using ptr_type = const core::building_type*;
              if (counter >= core::city_type::maximum_buildings) return ptr_type(nullptr);
              const size_t current = counter;
              ++counter;
              return self->buildings[current];
            };
          },
          "get_stat", [] (const core::city_type* self, const sol::object &obj) {
            if (obj.get_type() != sol::type::string && obj.get_type() != sol::type::number) throw std::runtime_error("Bad city stat index type");
            
            if (obj.get_type() == sol::type::number) {
              const size_t stat_index = obj.as<size_t>();
              const size_t final_index = FROM_LUA_INDEX(stat_index);
              if (final_index < core::offsets::city_stats || final_index >= core::offsets::city_stats + core::city_stats::count) {
                throw std::runtime_error("Bad city type stat index " + std::to_string(final_index));
              }
              
              const size_t remove_offset = final_index - core::offsets::city_stats;
              return self->stats.get(remove_offset);
            }
            
            const auto str = obj.as<std::string_view>();
            const auto itr = core::city_stats::map.find(str);
            if (itr == core::city_stats::map.end()) throw std::runtime_error("Bad realm stat id " + std::string(str));
            const size_t final_index = itr->second;
            return self->stats.get(final_index);
          },
          "city_image_top", sol::readonly(&core::city_type::city_image_top),
          "city_image_face", sol::readonly(&core::city_type::city_image_face),
          "city_icon", sol::readonly(&core::city_type::city_icon),
          "buildings_count", sol::readonly(&core::city_type::buildings_count),
          "find_building", [] (const core::city_type* self, const core::building_type* b) {
            const size_t index = self->find_building(b);
            return index == SIZE_MAX ? int64_t(-1) : int64_t(TO_LUA_INDEX(index));
          },
          "find_building_upgrade", [] (const core::city_type* self, const core::building_type* b, const sol::optional<double> &start) {
            const size_t start_val = start.has_value() ? start.value() : 0;
            const size_t index = self->find_building_upgrade(b, start_val);
            return index == SIZE_MAX ? int64_t(-1) : int64_t(TO_LUA_INDEX(index));
          },
          "maximum_buildings", sol::var(core::city_type::maximum_buildings),
          "stats_start", sol::var(TO_LUA_INDEX(core::offsets::city_stats)),
          "stats_end", sol::var(core::offsets::city_stats + core::city_stats::count)
        );
        
        core.set_function("each_building", [] (const core::city_type* type, const sol::function &func) {
          if (type == nullptr) throw std::runtime_error("Invalid realm");
          if (!func.valid()) throw std::runtime_error("Invalid function");
                          
          for (size_t i = 0; i < core::city_type::maximum_buildings && type->buildings[i] != nullptr; ++i) {
            auto b = type->buildings[i];
            const auto ret = func(b);
            if (!ret.valid()) {
              sol::error err = ret;
              std::cout << err.what();
              throw std::runtime_error("There is lua errors");
            }
            
            if (ret.get_type() == sol::type::boolean) {
              const bool val = ret;
              if (val) break;
            }
          }
        });
      }
    }
  }
}
