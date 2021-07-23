#include "lua_initialization_hidden.h"

#include "magic_enum_header.h"
#include "bin/tiles_funcs.h"
#include "core/tile.h"
#include "core/city.h"
#include "core/province.h"
#include "core/army.h"
#include "core/hero_troop.h"
#include "core/realm.h"
#include "core/titulus.h"
#include "bin/map.h"
#include "globals.h"
#include "systems.h"
#include "render/stages.h"
#include "ai/path_container.h"
#include "ai/path_finding_data.h"

#define TO_LUA_INDEX(index) ((index)+1)
#define FROM_LUA_INDEX(index) ((index)-1)

// нужно вытащить тайловые функции отдельно
namespace devils_engine {
  namespace utils {
    constexpr bool valid_tile(const uint32_t &index) {
      return index < core::map::hex_count_d(core::map::detail_level);
    }
    
    void setup_lua_tile(sol::state_view lua) {
      auto core = lua[magic_enum::enum_name(reserved_lua::core)].get_or_create<sol::table>();
//       core.set_function("get_tile_height", [] (const uint32_t &tile_index) { 
//         return core::get_tile_height(FROM_LUA_INDEX(tile_index));
//       });
      
      core.new_usertype<render::map_tile_t>("map_tile", sol::no_constructor,
        "center", [] (const render::map_tile_t* self) -> std::tuple<double, double, double> { 
          const auto map = global::get<systems::map_t>()->map;
          if (map == nullptr) throw std::runtime_error("Wrong game state");
          const auto center = map->get_point(self->center);
          return std::make_tuple(center.x, center.y, center.z);
        },
//         "color", sol::readonly_property([] (const render::map_tile_t* self) { return self->color.container; }),
        "height", sol::readonly_property([] (const render::map_tile_t* self) { return self->height; }),
        "get_point", [] (const render::map_tile_t* self, const uint32_t &lua_index) -> std::tuple<double, double, double> { 
          const uint32_t n_count = self->neighbors[5] == GPU_UINT_MAX ? 5 : 6;
          if (FROM_LUA_INDEX(lua_index) >= n_count) return std::make_tuple(0.0, 0.0, 0.0);
          const auto map = global::get<systems::map_t>()->map;
          if (map == nullptr) throw std::runtime_error("Wrong game state");
          const uint32_t point_index = self->points[FROM_LUA_INDEX(lua_index)];
          const auto point = map->get_point(point_index);
          return std::make_tuple(point.x, point.y, point.z);
        },
        "get_neighbor_index", [] (const render::map_tile_t* self, const uint32_t &lua_index, sol::this_state s) -> sol::object { 
          const uint32_t n_count = self->neighbors[5] == GPU_UINT_MAX ? 5 : 6;
          if (FROM_LUA_INDEX(lua_index) >= n_count) return sol::make_object(s, sol::lua_nil); 
          return sol::make_object(s, TO_LUA_INDEX(self->neighbors[FROM_LUA_INDEX(lua_index)]));
        },
        "n_count", sol::readonly_property([] (const render::map_tile_t* self) -> uint32_t { return self->neighbors[5] == GPU_UINT_MAX ? 5 : 6; }),
//         "p_count", sol::readonly_property([] (const render::map_tile_t* self) -> uint32_t { return self->neighbors[5] == GPU_UINT_MAX ? 5 : 6; }),
        // тут нужно еще добавить индекс биома, ну и возможно сделать другие функции
        "biome_index", sol::readonly_property([] (const render::map_tile_t* self) { return TO_LUA_INDEX(uint32_t(self->biome_index >> 24)); })
      );
      
      core.set_function("get_tile", [] (const uint32_t &tile_index) {
        const uint32_t final_index = FROM_LUA_INDEX(tile_index);
        if (final_index >= core::map::hex_count_d(core::map::detail_level)) throw std::runtime_error("Bad tile index");
        auto map = global::get<systems::map_t>()->map;
        return map->get_tile_ptr(final_index);
      });
      
      core.set_function("get_tile_province", [] (const uint32_t &tile_index) {
        return core::get_tile_province(FROM_LUA_INDEX(tile_index));
      });
      
      core.set_function("get_tile_city", [] (const uint32_t &tile_index) {
        return core::get_tile_city(FROM_LUA_INDEX(tile_index));
      });
      
      core.set_function("is_tile_valid", [] (const uint32_t &tile_index) {
        const uint32_t final_index = FROM_LUA_INDEX(tile_index);
        return valid_tile(final_index);
      });
      
      core.set_function("highlight_tile", [] (const uint32_t &tile_index, const uint32_t &color) {
        const uint32_t final_index = FROM_LUA_INDEX(tile_index);
        if (!valid_tile(final_index)) throw std::runtime_error("Index " + std::to_string(tile_index) + " is not valid tile index");
        global::get<render::tile_highlights_render>()->add(final_index, {color});
      });
      
      core.set_function("highlight_tiles", [] (const sol::table &tile_indices, const uint32_t &color) {
        for (const auto &pair : tile_indices) {
          if (pair.first.get_type() != sol::type::number) continue;
          if (pair.second.get_type() != sol::type::number) continue;
                        
          const uint32_t tile_index = pair.second.as<uint32_t>();
          const uint32_t final_index = FROM_LUA_INDEX(tile_index);
          if (!valid_tile(final_index)) throw std::runtime_error("Index " + std::to_string(tile_index) + " is not valid tile index");
          global::get<render::tile_highlights_render>()->add(final_index, {color});
        }
      });
      
      core.set_function("highlight_unit_path", [] (const sol::object &unit, const sol::table &colors) {
        auto highlighter = global::get<render::tile_highlights_render>();
        std::array<render::color_t, 128> colors_array;
        size_t counter = 0;
        sol::object obj = colors[counter+1];
        while (obj.valid() && obj.get_type() == sol::type::number) { 
          colors_array[counter].container = obj.as<uint32_t>(); 
          ++counter; 
          obj = colors[counter+1];
        }
        
        if (counter < 2) throw std::runtime_error("Colors table is invalid");
        
        const ai::path_finding_data* final_data = nullptr;
        uint32_t current_speed = 0;
        uint32_t maximum_speed = 0;
        if (unit.is<core::army*>()) {
          auto data = unit.as<core::army*>();
          
          if (!data->has_path()) return;
          ASSERT(data->has_path() && data->path_size != 0);
          
          //current_speed = data->current_stats[core::army_stats::speed].uval;
          //maximum_speed = data->computed_stats[core::army_stats::speed].uval;
          current_speed = 500;
          maximum_speed = 500;
          final_data = data;
        } else if (unit.is<core::hero_troop*>()) {
          auto data = unit.as<core::hero_troop*>();
          
          if (!data->has_path()) return;
          ASSERT(data->has_path() && data->path_size != 0);
          
          assert(false);
        }
        
        if (final_data == nullptr) throw std::runtime_error("Unit must be a valid army or hero troop");
        
        size_t current_path = final_data->current_path;
        const size_t available_path = ai::unit_advance(final_data, current_path, current_speed);
        for (size_t i = current_path; i < available_path; ++i) {
          const uint32_t container = i / ai::path_container::container_size;
          const uint32_t index     = i % ai::path_container::container_size;
          const uint32_t next_tile_index = ai::advance_container(final_data->path, container)->tile_path[index].tile;
          highlighter->add(next_tile_index, colors_array[0]);
        }
        
        current_path += available_path;
        size_t color_counter = 1;
        while (current_path < final_data->path_size) {
          const size_t available_path = ai::unit_advance(final_data, current_path, maximum_speed);
          if (available_path == 0) break;
          
          for (size_t i = current_path; i < available_path; ++i) {
            const uint32_t container = i / ai::path_container::container_size;
            const uint32_t index     = i % ai::path_container::container_size;
            const uint32_t next_tile_index = ai::advance_container(final_data->path, container)->tile_path[index].tile;
            highlighter->add(next_tile_index, colors_array[color_counter]);
          }
          
          current_path += available_path;
          color_counter = (color_counter+1) % counter;
          color_counter += uint32_t(color_counter == 0);
        }
      });
      
      // нужно еще вывести какую то инфу для биома
      // тип биома, имя?
    }
  }
}
