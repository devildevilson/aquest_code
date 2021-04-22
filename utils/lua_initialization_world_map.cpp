#include "lua_initialization.h"

#include "bin/map.h"
#include "magic_enum.hpp"
#include "bin/generator_container.h"
#include "globals.h"
#include "battle_map_enum.h"

#define FROM_LUA_INDEX(index) (index-1)
#define TO_LUA_INDEX(index) (index+1)

namespace devils_engine {
  namespace utils {
    void set_tiles_color(core::map* self, const sol::table t) {
      std::unique_lock<std::mutex> lock(self->mutex);
      size_t counter = 0;
      for (const auto &pair : t) {
        if (pair.first.get_type()  != sol::type::number) continue;
        if (pair.second.get_type() != sol::type::number) continue;
        
        auto tile = self->get_tile_ptr(counter);
        const uint32_t data = pair.second.as<uint32_t>();
        tile->color.container = data;
        
        ++counter;
      }
      
      ASSERT(counter == self->tiles_count());
    }
    
    void set_tiles_color(core::map* self, const map::generator::container* cont, const uint32_t &property_index) {
      std::unique_lock<std::mutex> lock(self->mutex);
      assert(self->tiles_count() == cont->entities_count(0));
      for (size_t i = 0; i < self->tiles_count(); ++i) {
        const uint32_t color = cont->get_data<uint32_t>(0, i, FROM_LUA_INDEX(property_index));
        auto tile = self->get_tile_ptr(i);
        tile->color.container = color;
      }
    }
    
    void set_tiles_height(core::map* self, const sol::table t) {
      std::unique_lock<std::mutex> lock(self->mutex);
      size_t counter = 0;
      for (const auto &pair : t) {
        if (pair.first.get_type()  != sol::type::number) continue;
        if (pair.second.get_type() != sol::type::number) continue;

        auto tile = self->get_tile_ptr(counter);
        const float data = pair.second.as<float>();
        tile->height = data;
        
        ++counter;
      }
      
      ASSERT(counter == self->tiles_count());
    }
    
    void set_tiles_height(core::map* self, const map::generator::container* cont, const uint32_t &property_index) {
      std::unique_lock<std::mutex> lock(self->mutex);
      assert(self->tiles_count() == cont->entities_count(0));
      for (size_t i = 0; i < self->tiles_count(); ++i) {
        const float data = cont->get_data<float>(0, i, FROM_LUA_INDEX(property_index));
        auto tile = self->get_tile_ptr(i);
        tile->height = data;
      }
    }
    
    typedef void(*f_table)(core::map*, const sol::table);
    typedef void(*f_cont)(core::map*, const map::generator::container*, const uint32_t&);
    
    void setup_lua_world_map(sol::state_view lua) {
      const f_table colorf1 = &set_tiles_color;
      const f_cont  colorf2 = &set_tiles_color;
      const f_table heightf1 = &set_tiles_height;
      const f_cont  heightf2 = &set_tiles_height;
      
      auto core = lua[magic_enum::enum_name<reserved_lua::values>(reserved_lua::core)].get_or_create<sol::table>();
      core.new_usertype<render::map_tile_t>("map_tile", sol::no_constructor,
        "center", sol::readonly_property([] (const render::map_tile_t* self) { return TO_LUA_INDEX(self->center); }),
        "color", sol::readonly_property([] (const render::map_tile_t* self) { return self->color.container; }),
        "height", sol::readonly_property([] (const render::map_tile_t* self) { return self->height; }),
        // тут можно тогда сделать обычную функцию и возвращать по индексу
//         "points", sol::readonly_property([] (const render::map_tile_t* self) { return std::ref(self->points); }),
//         "neighbors", sol::readonly_property([] (const render::map_tile_t* self) { return std::ref(self->neighbors); }),
        "get_point_index", [] (const render::map_tile_t* self, const uint32_t &lua_index, sol::this_state s) -> sol::object { 
          const uint32_t n_count = self->neighbors[5] == GPU_UINT_MAX ? 5 : 6;
          if (FROM_LUA_INDEX(lua_index) >= n_count) return sol::make_object(s, sol::lua_nil); 
          return sol::make_object(s, TO_LUA_INDEX(self->points[FROM_LUA_INDEX(lua_index)]));
        },
        "get_neighbor_index", [] (const render::map_tile_t* self, const uint32_t &lua_index, sol::this_state s) -> sol::object { 
          const uint32_t n_count = self->neighbors[5] == GPU_UINT_MAX ? 5 : 6;
          if (FROM_LUA_INDEX(lua_index) >= n_count) return sol::make_object(s, sol::lua_nil); 
          return sol::make_object(s, TO_LUA_INDEX(self->neighbors[FROM_LUA_INDEX(lua_index)]));
        },
        "n_count", sol::readonly_property([] (const render::map_tile_t* self) -> uint32_t { return self->neighbors[5] == GPU_UINT_MAX ? 5 : 6; }),
        "p_count", sol::readonly_property([] (const render::map_tile_t* self) -> uint32_t { return self->neighbors[5] == GPU_UINT_MAX ? 5 : 6; })
      );
      
      core.new_usertype<core::map>("map", sol::no_constructor,
        "points_count", &core::map::points_count,
        "tiles_count", &core::map::tiles_count,
        "triangles_count", &core::map::triangles_count,
        "set_tile_color", [] (core::map* self, const uint32_t &tile_index, const uint32_t &color) {
          self->set_tile_color(FROM_LUA_INDEX(tile_index), {color});
        },
        "set_tile_height", &core::map::set_tile_height_lua,
        "get_tile_height", &core::map::get_tile_height_lua,
        "get_tile", &core::map::get_tile_ptr_lua,
        "get_point", [] (const core::map* self, const uint32_t &index) -> std::tuple<float, float, float> {
          const auto vec = self->get_point(FROM_LUA_INDEX(index));
          return std::tie(vec.x, vec.y, vec.z);
        },
        "set_plate_color_and_height", [] (core::map* self, sol::object childs, const uint32_t &color, const float &height) {
          std::unique_lock<std::mutex> lock(self->mutex);
          if (childs.is<std::vector<unsigned int>*>()) {
            const auto &array = *childs.as<std::vector<unsigned int>*>();
            for (const auto index : array) {
              auto tile = self->get_tile_ptr(FROM_LUA_INDEX(index));
              ASSERT(tile != nullptr);
              tile->color.container = color;
              tile->height = height;
            }
            
            return;
          } 
          
          if (!childs.is<sol::table>()) throw std::runtime_error("Bad childs type, must be an array");
                                   
          auto t = childs.as<sol::table>();
          for (const auto &obj : t) {
            if (!obj.first.is<uint32_t>()) continue;
            if (!obj.second.is<uint32_t>()) continue;
            
            const uint32_t index = obj.second.as<uint32_t>();
            auto tile = self->get_tile_ptr(FROM_LUA_INDEX(index));
            ASSERT(tile != nullptr);
            tile->color.container = color;
            tile->height = height;
          }
        },
        "set_tiles_color", sol::overload(colorf1, colorf2),
        "set_tiles_height", sol::overload(heightf1, heightf2)
//         "get_tile_biome", [] (const core::map* self, const uint32_t &index) -> std::string_view { 
//           auto container = global::get<utils::world_map_string_container>();
//           const size_t type = static_cast<size_t>(utils::world_map_strings::tile_biome_id);
//           if (container->get_strings(type).size() < self->tiles_count()) {
//             const size_t diff = self->tiles_count() - container->get_strings(type).size();
//             container->register_strings(type, diff);
//           }
//           
//           return container->get_strings(type)[FROM_LUA_INDEX(index)];
//         },
//         
//         "set_tile_biome", [] (const core::map* self, const uint32_t &index, const std::string &id) -> void { 
//           auto container = global::get<utils::world_map_string_container>();
//           const size_t type = static_cast<size_t>(utils::world_map_strings::tile_biome_id);
//           if (container->get_strings(type).size() < self->tiles_count()) {
//             const size_t diff = self->tiles_count() - container->get_strings(type).size();
//             container->register_strings(type, diff);
//           }
//           
//           container->set_string(type, FROM_LUA_INDEX(index), id);
//         }
      );
    }
  }
}
