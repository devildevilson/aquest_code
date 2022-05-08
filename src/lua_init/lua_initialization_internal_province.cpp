#include "lua_initialization_internal.h"

#include "core/province.h"
#include "core/titulus.h"
#include "core/city.h"
#include "core/context.h"
#include "utils/systems.h"
#include "utils/globals.h"
#include "utils/magic_enum_header.h"
#include "utils/lua_container_iterators.h"
#include "lua_initialization_handle_types.h"

namespace devils_engine {
  namespace utils {
    namespace internal {
      void setup_lua_province(sol::state_view lua) {
        auto core = lua[magic_enum::enum_name<reserved_lua::core>()].get_or_create<sol::table>();
        const auto province_type = core.new_usertype<core::province>(
          "province", sol::no_constructor,
          "title", sol::readonly(&core::province::title),
          "tiles", [] (const core::province* self) {
            size_t counter = 0;
            return [self, counter] (sol::this_state s) mutable { 
              sol::state_view lua = s;
              const size_t cur = counter; 
              ++counter; 
              if (cur >= self->tiles.size()) return std::make_tuple(sol::object(sol::nil), sol::object(sol::nil));
              return std::make_tuple(sol::make_object(s, counter), sol::make_object(s, TO_LUA_INDEX(self->tiles[cur])));
            };
          },
          "neighbours", [] (const core::province* self) {
            size_t counter = 0;
            return [self, counter] (sol::this_state s) mutable { 
              sol::state_view lua = s;
              const size_t cur = counter;
              ++counter;
              if (cur >= self->neighbours.size()) return std::make_tuple(sol::object(sol::nil), sol::object(sol::nil));
              return std::make_tuple(sol::make_object(s, counter), sol::make_object(s, TO_LUA_INDEX(self->neighbours[cur])));
            };
          },
          "cities_max_count", sol::readonly(&core::province::cities_max_count),
          "cities_count", sol::readonly(&core::province::cities_count),
          "cities", [] (const core::province* self) { 
            auto city = self->cities;
            return [self, city] () mutable -> const core::city* {
              auto cur = city;
              city = self->next_city(city);
              return cur;
            };
          },
          "can_raise_army", &core::province::can_raise_army,
          "raise_army", [] (const core::province* self) { return lua_handle_army(self->raise_army()); },
//           "modificators", sol::readonly(&core::province::modificators),
//           "events", sol::readonly(&core::province::events),
          "has_flag", &core::province::has_flag,
          "flags", &utils::flags_iterator<core::province>,
          "events", &utils::events_iterator<core::province>,
          "modificators", &utils::modificators_iterator<core::province>,
//           "modificators_container_size", sol::var(core::province::modificators_container_size),
//           "events_container_size", sol::var(core::province::events_container_size),
//           "flags_container_size", sol::var(core::province::flags_container_size),
          "cities_max_game_count", sol::var(core::province::cities_max_game_count)
        );
        
        core.set_function("each_tile", [] (const core::province* self, const sol::function &func) {
          if (self == nullptr) throw std::runtime_error("Province is invalid");
          if (!func.valid()) throw std::runtime_error("Function is invalid");
          
          for (size_t i = 0; i < self->tiles.size(); ++i) {
            const auto ret = func(TO_LUA_INDEX(i), TO_LUA_INDEX(self->tiles[i]));
            CHECK_ERROR_THROW(ret)
            
            if (ret.get_type() == sol::type::boolean) {
              const bool val = ret;
              if (val) break;
            }
          }
        });
        
        core.set_function("each_neighbour", [] (const core::province* self, const sol::function &func) {
          if (self == nullptr) throw std::runtime_error("Province is invalid");
          if (!func.valid()) throw std::runtime_error("Function is invalid");
          
          for (size_t i = 0; i < self->neighbours.size(); ++i) {
            const auto ret = func(TO_LUA_INDEX(i), TO_LUA_INDEX(self->neighbours[i]));
            CHECK_ERROR_THROW(ret)
            
            if (ret.get_type() == sol::type::boolean) {
              const bool val = ret;
              if (val) break;
            }
          }
        });
        
        core.set_function("each_city", [] (const core::province* self, const sol::function &func) {
          if (self == nullptr) throw std::runtime_error("Province is invalid");
          if (!func.valid()) throw std::runtime_error("Function is invalid");
          
          for (auto city = self->cities; city != nullptr; city = self->next_city(city)) {
            const auto ret = func(city);
            CHECK_ERROR_THROW(ret)
            
            if (ret.get_type() == sol::type::boolean) {
              const bool val = ret;
              if (val) break;
            }
          }
        });
        
        core.set_function("is_province_index_valid", [] (const uint32_t &index) {
          const uint32_t final_index = FROM_LUA_INDEX(index);
          auto ctx = global::get<systems::map_t>()->core_context.get();
          return final_index < ctx->get_entity_count<core::province>();
        });
        
        core.set_function("get_province", [] (const uint32_t &index) {
          const uint32_t final_index = FROM_LUA_INDEX(index);
          auto ctx = global::get<systems::map_t>()->core_context.get();
          return ctx->get_entity<core::province>(final_index);
        });
      }
    }
  }
}
