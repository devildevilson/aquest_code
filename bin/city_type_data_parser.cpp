#include "data_parser.h"
#include "utils/globals.h"
#include "core_structures.h"
#include "utils/table_container.h"
#include "utils/string_container.h"
#include "core_context.h"
#include "utils/serializator_helper.h"
#include "render/image_controller.h"
#include "map_creator.h"

namespace devils_engine {
  namespace utils {
    const check_table_value city_type_table[] = {
      {
        "id",
        check_table_value::type::string_t,
        check_table_value::value_required, 0, {}
      },
      {
        "buildings",
        check_table_value::type::array_t,
        0, core::city_type::maximum_buildings,
        {
          {
            ID_ARRAY,
            check_table_value::type::string_t,
            0, 0, {}
          }
        }
      },
      { 
        "stats",
        check_table_value::type::array_t,
        0, core::city_stats::count,
        {
          {
            STATS_ARRAY,
            check_table_value::type::int_t,
            core::offsets::city_stats, core::offsets::city_stats + core::city_stats::count, {}
          }
        }
      },
      {
        "image_top",
        check_table_value::type::string_t,
        check_table_value::value_required, 0, {}
      },
      {
        "image_face",
        check_table_value::type::string_t,
        check_table_value::value_required, 0, {}
      },
      {
        "scale",
        check_table_value::type::float_t,
        check_table_value::value_required, 0, {}
      },
    };
    
    size_t add_city_type(const sol::table &table) {
      return global::get<map::creator::table_container_t>()->add_table(static_cast<size_t>(core::structure::city_type), table);
    }
    
    bool validate_city_type(const size_t &index, const sol::table &table) {
      size_t counter = 0;
      auto id = table["id"];
      std::string check_str;
      if (id.valid()) {
        check_str = id;
      } else {
        check_str = "city_type" + std::to_string(index);
      }
      
      const size_t size = sizeof(city_type_table) / sizeof(city_type_table[0]);
      recursive_check(check_str, "city type", table, nullptr, city_type_table, size, counter);
      
      return counter == 0;
    }
    
    bool validate_city_type_and_save(const size_t &index, sol::this_state lua, const sol::table &table, utils::world_serializator* container) {
      const bool ret = validate_city_type(index, table);
      if (!ret) return false;
      
      sol::state_view state(lua);
//       auto keyallow = state.create_table();
//       keyallow.add("id", "buildings", "stats");
//       auto str = table_to_string(lua, table, keyallow);
      auto str = table_to_string(lua, table, sol::table());
      if (str.empty()) throw std::runtime_error("Could not serialize city type table");
      container->add_data(core::structure::city_type, std::move(str));
      
      return true;
    }
    
    void parse_city_type(core::city_type* city_type, const sol::table &table) {
      auto to_data = global::get<utils::data_string_container>();
      auto ctx = global::get<core::context>();
      
//       city_type->id = table.get<std::string>("id");
      
      {
        const auto &buildings = table.get<sol::table>("buildings");
        size_t counter = 0;
        for (auto itr = buildings.begin(); itr != buildings.end(); ++itr) {
          if (!(*itr).second.is<std::string>()) continue;
          
          const auto &str = (*itr).second.as<std::string_view>();
          const size_t index = to_data->get(str);
          if (index == SIZE_MAX) throw std::runtime_error("Could not find building type " + std::string(str));
          auto t = ctx->get_entity<core::building_type>(index);
          city_type->buildings[counter] = t;
          ++counter;
        }
      }
      
      {
        // то есть тут у нас только статы города? иначе это были бы модификаторы, наверное да
        const auto &stats = table.get<sol::table>("stats");
        size_t counter = 0;
        for (auto itr = stats.begin(); itr != stats.end(); ++itr) {
          if (!(*itr).first.is<std::string>()) continue;
          if (!(*itr).first.is<double>()) continue;
          
          auto stat = (*itr).first.as<std::string_view>();
          auto value = (*itr).second.as<float>();
          
          if (auto val = magic_enum::enum_cast<core::city_stats::values>(stat); val.has_value()) {
            const uint32_t stat = val.value();
            city_type->stats[stat].fval = value;
            ++counter;
          }
        }
      }
      
      auto controller = global::get<render::image_controller>();
      {
        const std::string str = table.get<std::string>("image_top");
        std::string_view img_id;
        uint32_t layer;
        bool mirror_u;
        bool mirror_v;
        const bool ret = render::parse_image_id(str, img_id, layer, mirror_u, mirror_v);
        if (!ret) throw std::runtime_error("Bad texture id " + std::string(str));
        
        const auto image_id = std::string(img_id);
        auto view = controller->get_view(image_id);
        if (layer >= view->count) throw std::runtime_error("Image pack " + image_id + " does not have " + std::to_string(layer) + " amount of images");
        city_type->city_image_top = view->get_image(layer, mirror_u, mirror_v);
      }
      
      {
        const std::string str = table.get<std::string>("image_face");
        std::string_view img_id;
        uint32_t layer;
        bool mirror_u;
        bool mirror_v;
        const bool ret = render::parse_image_id(str, img_id, layer, mirror_u, mirror_v);
        if (!ret) throw std::runtime_error("Bad texture id " + std::string(str));
        
        const auto image_id = std::string(img_id);
        auto view = controller->get_view(image_id);
        if (layer >= view->count) throw std::runtime_error("Image pack " + image_id + " does not have " + std::to_string(layer) + " amount of images");
        city_type->city_image_face = view->get_image(layer, mirror_u, mirror_v);
      }
      
      {
        const float scale = table.get<float>("scale");
        city_type->scale = scale;
      }
    }
  }
}
