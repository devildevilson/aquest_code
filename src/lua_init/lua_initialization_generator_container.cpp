#include "lua_initialization_hidden.h"

#include "Cpp/FastNoiseLite.h"

#include "generator/container.h"
#include "generator/context2.h"
#include "bin/map_creator.h"
#include "bin/image_parser.h"

#include "render/shared_render_utility.h"

#include "core/seasons.h"
#include "core/titulus.h"
#include "core/map.h"

#include "utils/globals.h"
#include "utils/serializator_helper.h"
#include "utils/systems.h"
#include "utils/random_engine.h"

#include <iostream>
#include <string>
#include <queue>
#include "utils/shared_time_constant.h"
#include "utils/magic_enum_header.h"

namespace devils_engine {
  typedef size_t(map::generator::container::*container_func)(const sol::table &);
  
  namespace utils {
    static double create_pair_u32f32(const uint32_t &u32, const float &f32) {
      union convert { uint64_t u; double d; };
      const uint32_t &data = glm::floatBitsToUint(f32);
      const uint64_t packed_data = (uint64_t(u32) << 32) | uint64_t(data);
      convert c;
      c.u = packed_data;
      return c.d;
    }
    
    static std::tuple<uint32_t, float> unpack_pair_u32f32(const double &data) {
      union convert { uint64_t u; double d; };
      convert c;
      c.d = data;
      const uint64_t num = c.u;
      const uint32_t data1 = uint32_t(num >> 32);
      const uint32_t data2 = uint32_t(num);
      const float fdata = glm::uintBitsToFloat(data2);
      return std::tie(data1, fdata);
    }
    
    struct timer_t {
      std::string str;
      std::chrono::steady_clock::time_point tp;
      
      timer_t(const std::string &str) : str(str), tp(std::chrono::steady_clock::now()) {}
      ~timer_t() {} // почему я не использую луа 5.4? муннаклир работает с 5.3, но вроде бы компилируется и для 5.4
      
      void checkpoint(const std::string_view &str) {
        auto end = std::chrono::steady_clock::now() - tp;
        auto dur = std::chrono::duration_cast<CHRONO_TIME_TYPE>(end).count();
        std::cout << str << " happen after " << dur << " " << TIME_STRING << " from timer creation\n";
      }
      
      void end() {
        auto end = std::chrono::steady_clock::now() - tp;
        auto dur = std::chrono::duration_cast<CHRONO_TIME_TYPE>(end).count();
        std::cout << str << " took " << dur << " " << TIME_STRING << "\n";
      }
    };
    
    void setup_lua_generator_container(sol::state_view lua) {
      auto generator = lua[magic_enum::enum_name<reserved_lua::values>(reserved_lua::generator)].get_or_create<sol::table>();
      auto container = generator.new_usertype<map::generator::container>(
        "container", sol::no_constructor,
        "add_entity", &map::generator::container::add_entity_lua,
        "set_entity_count", &map::generator::container::set_entity_count_lua,
        "clear_entities", &map::generator::container::clear_entities_lua,
        "get_child", &map::generator::container::get_child_lua,
        "get_childs_count", &map::generator::container::get_childs_count_lua,
        "add_child", &map::generator::container::add_child_lua,
        "clear_childs", &map::generator::container::clear_childs_lua,
        //"get_childs", &map::generator::container::get_childs,
        "get_childs", [] (const map::generator::container* self, const uint32_t &type, const uint32_t &index) { return std::ref(self->get_childs_lua(type, index)); },
        "entities_count", &map::generator::container::entities_count_lua,
        "type", &map::generator::container::type_lua,
        "set_tile_template", &map::generator::container::set_tile_template_lua,
        "set_entity_template", &map::generator::container::set_entity_template_lua,
        "compute_memory_size", &map::generator::container::compute_memory_size,
        "set_data_f32", &map::generator::container::set_data_lua<float>,
        "set_data_i32", &map::generator::container::set_data_lua<int32_t>,
        "set_data_u32", &map::generator::container::set_data_lua<uint32_t>,
        "set_data_vec3", [] (map::generator::container* self, const uint32_t &type, const uint32_t &index, const uint32_t &parameter_type, const float &x, const float &y, const float &z) -> void {
          self->set_data_lua<glm::vec3>(type, index, parameter_type, glm::vec3(x, y, z));
        },
        "get_data_f32", &map::generator::container::get_data_lua<float>,
        "get_data_i32", &map::generator::container::get_data_lua<int32_t>,
        "get_data_u32", &map::generator::container::get_data_lua<uint32_t>,
        "get_data_vec3", [] (map::generator::container* self, const uint32_t &type, const uint32_t &index, const uint32_t &parameter_type) -> std::tuple<float, float, float> {
          const auto data = self->get_data_lua<glm::vec3>(type, index, parameter_type);
          return std::tie(data.x, data.y, data.z);
        }
      );
      
      auto data_type_enum = generator.new_enum("data_type",
        {
          std::make_pair(std::string_view("uint_t"), map::generator::data_type::uint_t),
          std::make_pair(std::string_view("int_t"), map::generator::data_type::int_t),
          std::make_pair(std::string_view("float_t"), map::generator::data_type::float_t)
        }
      );
      
      auto utils = lua[magic_enum::enum_name(reserved_lua::utils)].get_or_create<sol::table>();
      utils.set_function("assign_distance_field", [] (map::generator::context* ctx, const sol::table type_indices, const sol::table seeds, const sol::table stops, sol::table distances) {
        const uint32_t edge_type = type_indices["edge_type"];
        const uint32_t edge_property_first_index = type_indices["edge_property_first_index"];
        const uint32_t edge_property_second_index = type_indices["edge_property_second_index"].valid() ? type_indices["edge_property_second_index"] : UINT32_MAX;
        const uint32_t tile_type = type_indices["tile_type"];
        const uint32_t tile_property_plate_index = type_indices["tile_property_plate_index"];
        
        const size_t tiles_count = ctx->map->tiles_count();
        const size_t edges_count = ctx->container->entities_count_lua(edge_type);
        std::vector<uint32_t> queue(tiles_count);
        queue.clear();
        
        for (const auto &pair : seeds) {
          if (pair.first.get_type() != sol::type::number) continue;
          const size_t index = pair.first.as<size_t>();
          assert(index <= edges_count);
          const uint32_t first_tile  = ctx->container->get_data_lua<uint32_t>(edge_type, index, edge_property_first_index);
          queue.push_back(first_tile);
          distances[first_tile] = create_pair_u32f32(index, 0.0f);
          
          if (edge_property_second_index != UINT32_MAX) {
            const uint32_t second_tile = ctx->container->get_data_lua<uint32_t>(edge_type, index, edge_property_second_index);
            queue.push_back(second_tile);
            distances[second_tile] = create_pair_u32f32(index, 0.0f);
          }
        }
        
        while (!queue.empty()) {
          const uint64_t rand_index = ctx->random->index(queue.size());
          const uint32_t current_tile = queue[rand_index];
          queue[rand_index] = queue.back();
          queue.pop_back();
          
          const auto tile = ctx->map->get_tile_ptr(FROM_LUA_INDEX(current_tile));
          const uint32_t current_plate_idx = ctx->container->get_data_lua<uint32_t>(tile_type, current_tile, tile_property_plate_index);
          auto [current_edge, dist] = unpack_pair_u32f32(distances[current_tile]);
          const uint32_t n_count = tile->neighbors[5] == UINT32_MAX ? 5 : 6;
          for (uint32_t i = 0; i < n_count; ++i) {
            const uint32_t n_index = tile->neighbors[i];
            const uint32_t n_plate_idx = ctx->container->get_data_lua<uint32_t>(tile_type, TO_LUA_INDEX(n_index), tile_property_plate_index);
            if (current_plate_idx != n_plate_idx) continue;
                         
            auto [id, n_dist] = unpack_pair_u32f32(distances[TO_LUA_INDEX(n_index)]);
            if (n_dist == 100000.0 && !stops[TO_LUA_INDEX(n_index)].valid()) {
              distances[TO_LUA_INDEX(n_index)] = create_pair_u32f32(current_edge, dist + 1.0f);
              queue.push_back(TO_LUA_INDEX(n_index));
            }
          }
        }
      });
      
//       utils.set_function("int_random_queue", [] (sol::this_state s, const map::generator::context* ctx, const double &first_count, const sol::function prepare_function, const sol::function queue_function) {
//         sol::state_view lua = s;
//         std::vector<int64_t> queue;
//         queue.reserve(first_count * 2);
//         const auto push_func = [&queue] (const int64_t data) { queue.push_back(data); };
//         sol::object lua_push_func = sol::make_object(lua, push_func);
//         
//         for (size_t i = 0; i < first_count; ++i) {
//           prepare_function(TO_LUA_INDEX(i), lua_push_func);
//         }
//         
//         while (!queue.empty()) {
//           const size_t rand_index = ctx->random->index(queue.size());
//           const int64_t data = queue[rand_index];
//           queue[rand_index] = queue.back();
//           queue.pop_back();
//           
//           queue_function(data, lua_push_func);
//         }
//       });
//       
//       utils.set_function("num_random_queue", [] (sol::this_state s, const map::generator::context* ctx, const double &first_count, const sol::function prepare_function, const sol::function queue_function) {
//         sol::state_view lua = s;
//         std::vector<double> queue;
//         const size_t size = first_count;
//         queue.reserve(size * 2);
//         const auto push_func = [&queue] (const double data) { queue.push_back(data); };
//         sol::object lua_push_func = sol::make_object(lua, push_func);
//         
//         for (size_t i = 0; i < size; ++i) {
//           prepare_function(TO_LUA_INDEX(i), lua_push_func);
//         }
//         
//         while (!queue.empty()) {
//           const size_t rand_index = ctx->random->index(queue.size());
//           const double data = queue[rand_index];
//           queue[rand_index] = queue.back();
//           queue.pop_back();
//           
//           queue_function(data, lua_push_func);
//         }
//       });
//       
//       utils.set_function("random_queue", [] (sol::this_state s, const map::generator::context* ctx, const double &first_count, const sol::function prepare_function, const sol::function queue_function) {
//         sol::state_view lua = s;
//         std::vector<sol::object> queue;
//         const size_t size = first_count;
//         queue.reserve(size * 2);
//         const auto push_func = [&queue] (const sol::object data) { queue.push_back(data); };
//         sol::object lua_push_func = sol::make_object(lua, push_func);
//         
//         for (size_t i = 0; i < size; ++i) {
//           prepare_function(TO_LUA_INDEX(i), lua_push_func);
//         }
//         
//         while (!queue.empty()) {
//           const size_t rand_index = ctx->random->index(queue.size());
//           const sol::object data = queue[rand_index];
//           queue[rand_index] = queue.back();
//           queue.pop_back();
//           
//           queue_function(data, lua_push_func);
//         }
//       });
      
      utils.set_function("each_tile_neighbor", [] (const map::generator::context* ctx, const sol::function activity) {
        const size_t tiles_count = ctx->map->tiles_count();
        for (size_t i = 0; i < tiles_count; ++i) {
          const auto tile = ctx->map->get_tile_ptr(i);
          const uint32_t n_count = tile->neighbors[5] == UINT32_MAX ? 5 : 6;
          for (uint32_t j = 0; j < n_count; ++j) {
            const uint32_t n_index = tile->neighbors[j];
            activity(TO_LUA_INDEX(i), TO_LUA_INDEX(n_index));
          }
        }
      });
      
      auto seasons = generator.new_usertype<core::seasons>("seasons", sol::no_constructor,
        "get_tile_biome", [] (const core::seasons* self, const uint32_t &season_index, const uint32_t &tile_index) {
          return self->get_tile_biome(FROM_LUA_INDEX(season_index), FROM_LUA_INDEX(tile_index));
        },
        "set_tile_biome", [] (core::seasons* self, const uint32_t &season_index, const uint32_t &tile_index, const uint32_t &biome_index) {
          self->set_tile_biome(FROM_LUA_INDEX(season_index), FROM_LUA_INDEX(tile_index), FROM_LUA_INDEX(biome_index));
        },
//         "add_biome", [] (sol::this_state s, core::seasons* self, const sol::table &biome) {
//           sol::state_view lua = s;
//           auto cont = global::get<utils::world_serializator>();
//           
//           const bool ret = utils::validate_biome(cont->get_data_count(utils::world_serializator::biome), biome);
//           if (!ret) throw std::runtime_error("Biome validation error");                                                 
//           auto creator = global::get<systems::map_t>()->map_creator;
//           const std::string str_t = creator->serialize_table(biome);
//           
//           const uint32_t cont_biome_index = cont->add_data(utils::world_serializator::biome, str_t);
//           const uint32_t biome_index = self->allocate_biome();
//           if (cont_biome_index != biome_index) throw std::runtime_error("Something completely wrong with biomes indices");
//           
//           return TO_LUA_INDEX(biome_index);
//         },
        "allocate_season", [] (core::seasons* self) {
          return TO_LUA_INDEX(self->allocate_season());
        },
        "seasons_count", sol::readonly_property(&core::seasons::seasons_count),
//         "biomes_count", sol::readonly_property(&core::seasons::biomes_count),
        "max_seasons_count", sol::var(&core::seasons::maximum_seasons),
        "max_biomes_count", sol::var(&core::seasons::maximum_biomes),
        "invalid_biome", sol::var(&core::seasons::invalid_biome)
      );
      
      auto context = generator.new_usertype<map::generator::context>("context", sol::no_constructor,
        "container", sol::readonly(&map::generator::context::container),
        "random", sol::readonly(&map::generator::context::random),
        "noise", sol::readonly(&map::generator::context::noise),
        "map", sol::readonly(&map::generator::context::map),
        "seasons", sol::readonly(&map::generator::context::seasons)
      );
      
      auto timer = generator.new_usertype<timer_t>("timer_t", sol::constructors<timer_t(const std::string &)>(),
        "checkpoint", &timer_t::checkpoint,
        "finish", &timer_t::end
      );
      
      auto core = lua["core"].get_or_create<sol::table>();
      auto title_type_table = core["titulus"].get_or_create<sol::table>();
      if (!title_type_table["type"].valid()) {
        auto title_type_type = title_type_table.new_enum("type",
          "city", core::titulus::type::city,
          "baron", core::titulus::type::baron,
          "duke", core::titulus::type::duke,
          "king", core::titulus::type::king,
          "imperial", core::titulus::type::imperial
        );
      }
    }
  }
}
