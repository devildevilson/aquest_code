#include "lua_initialization.h"

#include "bin/generator_container.h"
#include "bin/generator_context2.h"
#include "random_engine.h"
#include "FastNoise.h"
#include "bin/map.h"
#include "render/shared_render_utility.h"

#include <iostream>
#include <string>
#include <queue>
#include "shared_time_constant.h"
#include "magic_enum.hpp"

#define FROM_LUA_INDEX(index) (index-1)
#define TO_LUA_INDEX(index) (index+1)

namespace devils_engine {
  typedef size_t(map::generator::container::*container_func)(const sol::table &);
  
  namespace utils {
    double create_pair_u32f32(const uint32_t &u32, const float &f32) {
      union convert { uint64_t u; double d; };
      const uint32_t &data = glm::floatBitsToUint(f32);
      const uint64_t packed_data = (uint64_t(u32) << 32) | uint64_t(data);
      convert c;
      c.u = packed_data;
      return c.d;
    }
    
    std::tuple<uint32_t, float> unpack_pair_u32f32(const double &data) {
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
      ~timer_t() {
        auto end = std::chrono::steady_clock::now() - tp;
        auto dur = std::chrono::duration_cast<CHRONO_TIME_TYPE>(end).count();
        std::cout << str << " took " << dur << " " << TIME_STRING << "\n";
      }
      
      void checkpoint(const std::string_view &str) {
        auto end = std::chrono::steady_clock::now() - tp;
        auto dur = std::chrono::duration_cast<CHRONO_TIME_TYPE>(end).count();
        std::cout << str << " happen after " << dur << " " << TIME_STRING << " from timer creation\n";
      }
    };
    
    void setup_lua_generator_container(sol::state_view lua) {
      auto generator = lua[magic_enum::enum_name<reserved_lua::values>(reserved_lua::generator)].get_or_create<sol::table>();
      auto container = generator.new_usertype<map::generator::container>("container",
        sol::no_constructor,
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
      
      auto utils = lua[magic_enum::enum_name<reserved_lua::values>(reserved_lua::utils)].get_or_create<sol::table>();
      utils.set_function("prng", render::prng);
      utils.set_function("prng2", render::prng2);
      utils.set_function("prng_normalize", render::prng_normalize);
      utils.set_function("make_color", [] (const float r, const float g, const float b, const float a) {
        const uint32_t ur = uint32_t(255.0f * glm::clamp(r, 0.0f, 1.0f));
        const uint32_t ug = uint32_t(255.0f * glm::clamp(g, 0.0f, 1.0f));
        const uint32_t ub = uint32_t(255.0f * glm::clamp(b, 0.0f, 1.0f));
        const uint32_t ua = uint32_t(255.0f * glm::clamp(a, 0.0f, 1.0f));
        const uint32_t c = (ur << 24) | (ug << 16) | (ub << 8) | (ua << 0);
        return c;
      });
      utils.set_function("init_array", [] (const size_t &size, sol::object default_value, sol::this_state s) -> sol::table {
        sol::state_view view(s);
        auto t = view.create_table(size, 0);
        if (default_value.is<sol::table>()) {
          for (size_t i = 0; i < size; ++i) {
            t.add(view.create_table(30, 0));
          }
        } else {
//           if (default_value.is<bool>()) {
//             const bool val = default_value.as<bool>();
//             for (size_t i = 0; i < size; ++i) {
//               t.add(val);
//             }
//           } else if (default_value.is<double>()) {
//             const double val = default_value.as<double>();
//             for (size_t i = 0; i < size; ++i) {
//               t.add(val);
//             }
//           } else if (default_value.is<std::string>()) {
//             throw std::runtime_error("bad default value");
//           } else throw std::runtime_error("asfasfsfagsgsagg");
          for (size_t i = 0; i < size; ++i) {
            t.add(default_value);
          }
        }
        return t;
      });
      utils.set_function("create_table", [] (sol::object arr_size, sol::object hash_size, sol::this_state s) -> sol::table {
        sol::state_view view(s);
        const uint32_t narr = arr_size.is<uint32_t>() ? arr_size.as<uint32_t>() : 100;
        const uint32_t nhash = hash_size.is<uint32_t>() ? hash_size.as<uint32_t>() : 100;
        return view.create_table(narr, nhash);
      });
      utils.set_function("create_pair_u32f32", &create_pair_u32f32);
      utils.set_function("unpack_pair_u32f32", &unpack_pair_u32f32);
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
          
          const auto tile = ctx->map->get_tile_ptr_lua(current_tile);
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
      utils.set_function("int_random_queue", [] (sol::this_state s, const map::generator::context* ctx, const size_t &first_count, const sol::function prepare_function, const sol::function queue_function) {
        sol::state_view lua = s;
        std::vector<int64_t> queue(first_count * 2);
        queue.clear();
        const auto push_func = [&queue] (const int64_t data) { queue.push_back(data); };
        sol::object lua_push_func = sol::make_object(lua, push_func);
        
        for (size_t i = 0; i < first_count; ++i) {
          prepare_function(TO_LUA_INDEX(i), lua_push_func);
        }
        
        while (!queue.empty()) {
          const size_t rand_index = ctx->random->index(queue.size());
          const int64_t data = queue[rand_index];
          queue[rand_index] = queue.back();
          queue.pop_back();
          
          queue_function(data, lua_push_func);
        }
      });
      utils.set_function("int_queue", [] (sol::this_state s, const size_t &first_count, const sol::function prepare_function, const sol::function queue_function) {
        sol::state_view lua = s;
        std::queue<int64_t> queue;
        const auto push_func = [&queue] (const int64_t data) { queue.push(data); };
        sol::object lua_push_func = sol::make_object(lua, push_func);
        
        for (size_t i = 0; i < first_count; ++i) {
          prepare_function(TO_LUA_INDEX(i), lua_push_func);
        }
        
        while (!queue.empty()) {
          const int64_t data = queue.front();
          queue.pop();
          
          queue_function(data, lua_push_func);
        }
      });
      utils.set_function("random_queue", [] (sol::this_state s, const map::generator::context* ctx, const size_t &first_count, const sol::function prepare_function, const sol::function queue_function) {
        sol::state_view lua = s;
        std::vector<sol::object> queue(first_count * 2);
        queue.clear();
        const auto push_func = [&queue] (const sol::object data) { queue.push_back(data); };
        sol::object lua_push_func = sol::make_object(lua, push_func);
        
        for (size_t i = 0; i < first_count; ++i) {
          prepare_function(TO_LUA_INDEX(i), lua_push_func);
        }
        
        while (!queue.empty()) {
          const size_t rand_index = ctx->random->index(queue.size());
          const sol::object data = queue[rand_index];
          queue[rand_index] = queue.back();
          queue.pop_back();
          
          queue_function(data, lua_push_func);
        }
      });
      utils.set_function("queue", [] (sol::this_state s, const size_t &first_count, const sol::function prepare_function, const sol::function queue_function) {
        sol::state_view lua = s;
        std::queue<sol::object> queue;
        const auto push_func = [&queue] (const sol::object data) { queue.push(data); };
        sol::object lua_push_func = sol::make_object(lua, push_func);
        
        for (size_t i = 0; i < first_count; ++i) {
          prepare_function(TO_LUA_INDEX(i), lua_push_func);
        }
        
        while (!queue.empty()) {
          const sol::object data = queue.front();
          queue.pop();
          
          queue_function(data, lua_push_func);
        }
      });
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
      
      auto context = generator.new_usertype<map::generator::context>("context", sol::no_constructor,
        "container", sol::readonly(&map::generator::context::container),
        "random", sol::readonly(&map::generator::context::random),
        "noise", sol::readonly(&map::generator::context::noise),
        "map", sol::readonly(&map::generator::context::map)
        //"seasons", sol::readonly(&map::generator::context::seasons)
      );
      
      auto timer = generator.new_usertype<timer_t>("timer_t", sol::constructors<timer_t(const std::string &)>(),
        "checkpoint", &timer_t::checkpoint
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
