#include "lua_initialization_hidden.h"

#include "bin/battle_map.h"
#include "utils/battle_map_enum.h"
#include "utils/globals.h"
#include "magic_enum.hpp"

namespace devils_engine {
  namespace utils {
    void setup_lua_battle_map(sol::state_view lua) {
      auto battle = lua[magic_enum::enum_name<reserved_lua::values>(reserved_lua::battle)].get_or_create<sol::table>();
      battle.new_usertype<battle::map>("map", sol::no_constructor,
        "is_square", &battle::map::is_square,
        "is_flat", &battle::map::is_flat,
        "is_odd", &battle::map::is_odd,
        "set_tile_height", &battle::map::set_tile_height,
        "get_tile_height", &battle::map::get_tile_height,
        "set_tile_biome", &battle::map::set_tile_biome,
        "get_tile_biome", &battle::map::get_tile_biome,
        "tiles_count", sol::readonly_property([] (const battle::map* self) { return self->tiles_count; }),
        "width", sol::readonly_property([] (const battle::map* self) { return self->width; }),
        "height", sol::readonly_property([] (const battle::map* self) { return self->height; }),
        "set_tile_texture_id", [] (const battle::map* self, const uint32_t &index, const std::string &id) -> void { 
          auto container = global::get<utils::battle_map_string_container>();
          const size_t type = static_cast<size_t>(utils::battle_strings::tile_texture_id);
          if (container->get_strings(type).size() < self->tiles_count) {
            const size_t diff = self->tiles_count - container->get_strings(type).size();
            container->register_strings(type, diff);
          }
          
          container->set_string(type, index, id);
        },
        
        "get_tile_texture_id", [] (const battle::map* self, const uint32_t &index) -> std::string_view { 
          auto container = global::get<utils::battle_map_string_container>();
          const size_t type = static_cast<size_t>(utils::battle_strings::tile_texture_id);
          if (container->get_strings(type).size() < self->tiles_count) {
            const size_t diff = self->tiles_count - container->get_strings(type).size();
            container->register_strings(type, diff);
          }
          
          return container->get_strings(type)[index];
        },
        
        "set_tile_wall_texture_id", [] (const battle::map* self, const uint32_t &index, const std::string &id) -> void { 
          auto container = global::get<utils::battle_map_string_container>();
          const size_t type = static_cast<size_t>(utils::battle_strings::tile_walls_texture_id);
          if (container->get_strings(type).size() < self->tiles_count) {
            const size_t diff = self->tiles_count - container->get_strings(type).size();
            container->register_strings(type, diff);
          }
          
          container->set_string(type, index, id);
        },
        
        "get_tile_wall_texture_id", [] (const battle::map* self, const uint32_t &index) -> std::string_view { 
          auto container = global::get<utils::battle_map_string_container>();
          const size_t type = static_cast<size_t>(utils::battle_strings::tile_walls_texture_id);
          if (container->get_strings(type).size() < self->tiles_count) {
            const size_t diff = self->tiles_count - container->get_strings(type).size();
            container->register_strings(type, diff);
          }
          
          return container->get_strings(type)[index];
        }
      );
    }
  }
}
