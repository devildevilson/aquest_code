#include "heraldy_parser.h"

#include "render/vulkan_hpp_header.h"
#include "render/image_controller.h"
#include "render/targets.h"

#include "utils/globals.h"
#include "utils/table_container.h"
#include "utils/serializator_helper.h"
#include "utils/systems.h"
#include "utils/string_container.h"

#include "core/map.h"

#include "data_parser.h"
#include "map_creator.h"

namespace devils_engine {
  namespace utils {
    const check_table_value heraldy_table[] = {
      {
        "id",
        check_table_value::type::string_t,
        check_table_value::value_required, 0, {}
      }, 
      {
        "stencil",
        check_table_value::type::string_t,
        check_table_value::value_required, 0, {}
      },
      {
        "next_layer",
        check_table_value::type::string_t,
        0, 0, {}
      },
      {
        "colors",
        check_table_value::type::array_t,
        0, 4, 
        {
          {
            NUM_ARRAY,
            check_table_value::type::int_t,
            0, 0, {}
          }
        }
      },
      {
        "coords",
        check_table_value::type::array_t,
        0, 4, 
        {
          {
            NUM_ARRAY,
            check_table_value::type::float_t,
            0, 0, {}
          }
        }
      },
      {
        "tex_coords",
        check_table_value::type::array_t,
        0, 4, 
        {
          {
            NUM_ARRAY,
            check_table_value::type::float_t,
            0, 0, {}
          }
        }
      },
      {
        "discard_layer",
        check_table_value::type::bool_t,
        0, 0, {}
      },
      {
        "continue_layer", // для будущего
        check_table_value::type::bool_t,
        0, 0, {}
      }
    };
    
    void add_heraldy_layer(const sol::table &table) {
      global::get<map::creator::table_container_t>()->add_table(static_cast<size_t>(utils::generator_table_container::additional_data::heraldy), table);
    }
    
    bool validate_heraldy_layer(const uint32_t &index, const sol::table &table) {
      size_t counter = 0;
      auto id = table["id"];
      std::string check_str;
      if (id.valid()) {
        check_str = id;
      } else {
        check_str = "heraldy_layer" + std::to_string(index);
      }
      
      const size_t size = sizeof(heraldy_table) / sizeof(heraldy_table[0]);
      recursive_check(check_str, "heraldy_layer", table, nullptr, heraldy_table, size, counter);
      
      return counter == 0;
    }
    
    bool validate_heraldy_layer_and_save(const uint32_t &index, sol::this_state lua, const sol::table &table, utils::world_serializator* container) {
      const bool ret = validate_heraldy_layer(index, table);
      if (!ret) return false;

      sol::state_view state(lua);
      auto str = table_to_string(lua, table, sol::table());
      if (str.empty()) throw std::runtime_error("Could not serialize image table");
      ASSERT(false);
      //container->add_heraldy_data(std::move(str));
      
      return true;
    }
    
    std::vector<render::packed_heraldy_layer_t> load_heraldy_layers(render::image_controller* controller, const std::vector<sol::table> &heraldy_tables) {
//       auto to_data = global::get<utils::numeric_string_container>();
      auto buffers_struct = global::get<render::buffers>();
//       utils::numeric_string_container local_container;
//       auto to_data = &local_container;
      
      // нам нужно еще в другом месте получить индекс (в титулах)
      // как это наиболее адекватно сделать? по идее мне это нужно
      // только раз сделать а потом использовать,
      // лучше всего использовать какой то стринг контейнер дополнительный
//       std::unordered_map<std::string, uint32_t> layers_map;
//       for (size_t i = 0; i < heraldy_tables.size(); ++i) {
//         const auto str = heraldy_tables[i]["id"];
//         to_data->insert(str, i);
//       }
      
      // тут нужно получить доступ к буферу
//       const size_t heraldy_buffer_size = heraldy_tables.size() * sizeof(render::packed_heraldy_layer_t);
//       const size_t aligned = align_to(heraldy_buffer_size, 16);
      std::vector<render::packed_heraldy_layer_t> buffer(heraldy_tables.size());
      
      //auto arr = reinterpret_cast<render::packed_heraldy_layer_t*>(buffer->ptr());
      auto arr = buffer.data();
      for (size_t i = 0; i < heraldy_tables.size(); ++i) {
        const auto &table = heraldy_tables[i];
        render::heraldy_layer_t l{
          {GPU_UINT_MAX},
          0,
          GPU_UINT_MAX,
          0,
          {
            render::make_color(1.0f, 0.0f, 0.0f, 1.0f), 
            render::make_color(0.0f, 1.0f, 0.0f, 1.0f), 
            render::make_color(0.0f, 0.0f, 1.0f, 1.0f), 
            render::make_color(0.0f, 0.0f, 0.0f, 1.0f)
          },
          glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
          glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)
        };
        
        {
          const std::string stencil_image_data = table["stencil"];
//           PRINT(str)
          std::string_view img_id;
          uint32_t layer;
          bool mirror_u;
          bool mirror_v;
          const bool ret = render::parse_image_id(stencil_image_data, img_id, layer, mirror_u, mirror_v);
          if (!ret) throw std::runtime_error("Bad texture id " + std::string(stencil_image_data));
          const auto image_id = std::string(img_id);
          auto view = controller->get_view(image_id);
          if (layer >= view->count) throw std::runtime_error("Image pack " + image_id + " does not have " + std::to_string(layer) + " amount of images");
          l.stencil = view->get_image(layer, mirror_u, mirror_v);
        }
        
//         if (auto proxy = table["next_layer"]; proxy.valid()) {
//           const std::string next_id = proxy;
//           const size_t index = to_data->get(next_id);
//           if (index == SIZE_MAX) throw std::runtime_error("Could not find heraldy layer " + next_id);
//           if (index >= UINT32_MAX) throw std::runtime_error("Bad heraldy layer " + next_id);
//           l.next_layer = index;
//         }
        
        if (auto proxy = table["colors"]; proxy.valid()) {
          const sol::table color_table = proxy;
          size_t counter = 0;
          for (auto itr = color_table.begin(); itr != color_table.end(); ++itr) {
            if (!(*itr).second.is<size_t>()) continue;
            const size_t data = (*itr).second.as<size_t>();
            if (data >= UINT32_MAX) throw std::runtime_error("Bad color data");
            l.colors[counter].container = data;
            ++counter;
          }
        }
        
        if (auto proxy = table["coords"]; proxy.valid()) {
          const sol::table vec4_table = proxy;
          size_t counter = 0;
          for (auto itr = vec4_table.begin(); itr != vec4_table.end(); ++itr) {
            if (!(*itr).second.is<float>()) continue;
            const float data = (*itr).second.as<float>();
            l.coords[counter] = data;
            ++counter;
          }
        }
        
        if (auto proxy = table["tex_coords"]; proxy.valid()) {
          const sol::table vec4_table = proxy;
          size_t counter = 0;
          for (auto itr = vec4_table.begin(); itr != vec4_table.end(); ++itr) {
            if (!(*itr).second.is<float>()) continue;
            const float data = (*itr).second.as<float>();
            l.tex_coords[counter] = data;
            ++counter;
          }
        }
        
        if (auto proxy = table["discard_layer"]; proxy.valid()) {
          const bool data = proxy;
          l.stencil_type = l.stencil_type | (DISCARD_LAYER_BIT * uint32_t(data));
        }
        
        if (auto proxy = table["continue_layer"]; proxy.valid()) {
          const bool data = proxy;
          l.stencil_type = l.stencil_type | (CONTINUE_LAYER_BIT * uint32_t(data));
        }
        
        std::string str = table["id"];
        arr[i] = render::pack_heraldy_data(l);
        
        const auto itr = buffers_struct->heraldy_layers_map.find(str);
        if (itr != buffers_struct->heraldy_layers_map.end()) throw std::runtime_error("Heraldy layer with id " + str + " is already exists");
        buffers_struct->heraldy_layers_map.emplace(std::move(str), i);
      }
      
      return buffer;
    }
  }
}
