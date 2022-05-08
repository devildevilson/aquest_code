#include "unit_state_parser.h"

#include "utils/globals.h"
#include "utils/string_container.h"
#include "utils/systems.h"

#include "render/image_controller.h"

#include "bin/data_parser.h"
#include "bin/map_creator.h"

#include "lua_states.h"
#include "context.h"
#include "map.h"
#include "structures.h"

namespace devils_engine {
  namespace utils {
    const check_table_value unit_state_table[] = {
      {
        "id",
        check_table_value::type::string_t,
        check_table_value::value_required, 0, {}
      }, 
      {
        "textures",
        check_table_value::type::array_t,
        0, 0, {
          {
            ID_ARRAY,
            check_table_value::type::string_t,
            0, 0, {}
          }
        }
      },
      {
        "next",
        check_table_value::type::string_t,
        0, 0, {}
      },
      {
        "time",
        check_table_value::type::int_t,
        0, 0, {}
      }
    };
    
    size_t add_battle_unit_state(const sol::table &table) {
      return global::get<map::creator::table_container_t>()->add_table(static_cast<size_t>(battle::structure_type::unit_state), table);
    }
    
    bool validate_battle_unit_state(const uint32_t &index, const sol::table &table) {
      size_t counter = 0;
      auto id = table["id"];
      std::string check_str;
      if (id.valid()) {
        check_str = id;
      } else {
        check_str = "unit_state" + std::to_string(index);
      }
      
      const size_t size = sizeof(unit_state_table) / sizeof(unit_state_table[0]);
      recursive_check(check_str, "unit_state", table, nullptr, unit_state_table, size, counter);
      
      if (auto proxy = table["func"]; proxy.valid()) {
        if (proxy.get_type() == sol::type::string) {
          
        } else if (proxy.get_type() == sol::type::table) {
          const auto table = proxy.get<sol::table>();
          if (auto proxy = table["path"]; !proxy.valid()) {PRINT("Could not find path in func table"); ++counter;}
          if (auto proxy = table["name"]; !proxy.valid()) {PRINT("Could not find func name in func table"); ++counter;}
        } else {
          PRINT("Bad func variable type"); ++counter;
        }
      }
      
      return counter == 0;
    }
    
    void load_battle_unit_states(battle::context* ctx, const std::vector<sol::table> &tables, std::unordered_set<std::string> &parsed_scripts) {
      auto data_container = global::get<systems::battle_t>()->unit_states_map.get();
      auto states = global::get<systems::battle_t>()->lua_states.get();
//       auto map = global::get<systems::battle_t>()->map;
      auto controller = global::get<systems::core_t>()->image_controller;
      
      ASSERT(!tables.empty());
      
//       std::vector<render::image_t> textures_array;
      for (size_t i = 0; i < tables.size(); ++i) {
        const auto &table = tables[i];
        core::state state;
        state.id = table["id"];
        
//         const size_t textures_offset = textures_array.size();
//         size_t textures_count = 0;
        const auto textures_table = table["textures"].get<sol::table>();
//         for (const auto &obj : textures_table) {
//           if (!obj.second.is<std::string>()) continue;
//           
//           const std::string image_id_container = obj.second.as<std::string>();
//           std::string_view image_id;
//           uint32_t layer;
//           bool flip_u;
//           bool flip_v;
//           const bool ret = render::parse_image_id(image_id_container, image_id, layer, flip_u, flip_v);
//           if (!ret) throw std::runtime_error("Could not parse image id " + image_id_container);
//           
//           const auto view = controller->get_view(std::string(image_id));
//           if (view == nullptr) throw std::runtime_error("Could not find image " + std::string(image_id));
//           const auto render_id = view->get_image(layer, flip_u, flip_v);
//           textures_array.push_back(render_id);
//           ++textures_count;
//         }
        
        ASSERT(textures_table[0] == sol::nil);
        ASSERT(textures_table[1].get_type() == sol::type::string);
        const auto image_id_container = textures_table[1].get<std::string>();
        std::string_view image_id;
        uint32_t layer;
        bool flip_u;
        bool flip_v;
        const bool ret = render::parse_image_id(image_id_container, image_id, layer, flip_u, flip_v);
        if (!ret) throw std::runtime_error("Could not parse image id " + image_id_container);
        
        const auto view = controller->get_view(std::string(image_id));
        if (view == nullptr) throw std::runtime_error("Could not find image " + std::string(image_id));
        const auto render_id = view->get_image(layer, flip_u, flip_v);
        state.texture = render_id;
        
        // по идее теперь нужно просто добавить textures_array в буфер 
        // как это сделать? точнее куда положить
//         state.texture_offset = textures_offset;
//         state.texture_count  = textures_count;
        
        if (auto proxy = table["next"]; proxy.valid() && proxy.get_type() == sol::type::string) {
          // тут нужно найти указатели на другие состояния
          // что просто означает что нужно заранее задать часть данных
          const std::string str = proxy.get<std::string>();
          const size_t index = data_container->get(str);
          if (index == SIZE_MAX) throw std::runtime_error("Could not find unit state " + str);
          
          state.next = ctx->get_entity<core::state>(index);
        }
        
        const int64_t raw_time = table["time"].get<int64_t>();
        state.time = raw_time < 0 ? SIZE_MAX : raw_time;
        
        if (auto proxy = table["func"]; proxy.valid()) {
          if (proxy.get_type() == sol::type::string) {
            const std::string script = proxy.get<std::string>();
            state.func_index = states->parse_function(script);
          } else if (proxy.get_type() == sol::type::table) {
            const auto table = proxy.get<sol::table>();
            const std::string path = table["path"];
            const std::string name = table["name"];
            
            // проверяем распарсили ли мы скрипт
            if (parsed_scripts.find(path) == parsed_scripts.end()) {
              state.func_index = states->parse_function(path, name);
              parsed_scripts.insert(path);
            } else {
              state.func_index = states->parse_existing_function(name);
            }
          }
        }
        
        ctx->set_entity_data(i, state);
        
        auto ptr = ctx->get_entity<core::state>(i);
        data_container->insert(ptr->id, i);
      }
      
//       map->add_unit_textures(textures_array);
    }
  }
}
