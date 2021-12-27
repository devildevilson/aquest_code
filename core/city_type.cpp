#include "city_type.h"

#include "bin/data_parser.h"

#include "utils/globals.h"
#include "utils/table_container.h"
#include "utils/string_container.h"
// #include "utils/serializator_helper.h"

#include "render/image_controller.h"

#include "context.h"
#include "stats_table.h"

namespace devils_engine {
  namespace core {
    const structure city_type::s_type;
    const size_t city_type::maximum_buildings;
    city_type::city_type() : 
      holding(nullptr),
      buildings_count(0),
      buildings{nullptr}, 
      default_buildings_count(0),
      default_buildings{UINT32_MAX},
      city_image_top{GPU_UINT_MAX}, 
      city_image_face{GPU_UINT_MAX}, 
      city_icon{GPU_UINT_MAX}, 
      scale(1.0f) 
    {}
    
    size_t city_type::find_building(const building_type* b) const {
      ASSERT(buildings_count != 0);
      size_t index = 0;
      for (; index < buildings_count && buildings[index] != b; ++index);
      index = index >= buildings_count ? SIZE_MAX : index; //  || buildings[index] != b
      return index;
    }
    
    size_t city_type::find_building_upgrade(const building_type* b, const size_t &start) const {
      ASSERT(buildings_count != 0);
      size_t i = start;
      for (; i < buildings_count && buildings[i]->upgrades_from != b; ++i);
      i = i >= buildings_count ? SIZE_MAX : i; //  || buildings[i]->upgrades_from != b
      return i;
    }
    
    const utils::check_table_value city_type_table[] = {
      {
        "id",
        utils::check_table_value::type::string_t,
        utils::check_table_value::value_required, 0, {}
      },
      {
        "buildings",
        utils::check_table_value::type::array_t,
        utils::check_table_value::value_required, core::city_type::maximum_buildings,
        {
          {
            ID_ARRAY,
            utils::check_table_value::type::string_t,
            0, 0, {}
          }
        }
      },
      {
        "default_buildings",
        utils::check_table_value::type::array_t,
        0, core::city_type::maximum_buildings,
        {
          {
            ID_ARRAY,
            utils::check_table_value::type::string_t,
            0, 0, {}
          }
        }
      },
      { 
        "stats",
        utils::check_table_value::type::array_t,
        0, core::city_stats::count,
        {
          {
            STATS_ARRAY,
            utils::check_table_value::type::int_t,
            core::offsets::city_stats, core::offsets::city_stats + core::city_stats::count, {}
          }
        }
      },
      {
        "image_top",
        utils::check_table_value::type::string_t,
        utils::check_table_value::value_required, 0, {}
      },
      {
        "image_face",
        utils::check_table_value::type::string_t,
        utils::check_table_value::value_required, 0, {}
      },
      {
        "scale",
        utils::check_table_value::type::float_t,
        utils::check_table_value::value_required, 0, {}
      },
    };
    
    size_t add_city_type(const sol::table &table) {
//       return global::get<map::creator::table_container_t>()->add_table(static_cast<size_t>(core::structure::city_type), table);
      ASSERT(false);
      (void)table;
      return SIZE_MAX;
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
    
    bool validate_city_type_and_save(const size_t &index, sol::this_state lua, const sol::table &table, utils::world_serializator*) {
      const bool ret = validate_city_type(index, table);
      if (!ret) return false;
      
      sol::state_view state(lua);
//       auto keyallow = state.create_table();
//       keyallow.add("id", "buildings", "stats");
//       auto str = table_to_string(lua, table, keyallow);
      auto str = utils::table_to_string(lua, table, sol::table());
      if (str.empty()) throw std::runtime_error("Could not serialize city type table");
      ASSERT(false);
      //container->add_data(core::structure::city_type, std::move(str));
      
      return true;
    }
    
    void parse_city_type(core::city_type* city_type, const sol::table &table) {
//       auto to_data = global::get<utils::data_string_container>();
      auto ctx = global::get<core::context>();
      
//       city_type->id = table.get<std::string>("id");
      
      {
        const auto &buildings = table.get<sol::table>("buildings");
        size_t counter = 0;
        for (const auto &pair : buildings) {
          if (pair.second.get_type() != sol::type::string) continue;
          
          const auto &str = pair.second.as<std::string_view>();
          auto t = ctx->get_entity<core::building_type>(str);
          if (t == nullptr) throw std::runtime_error("Could not find building type " + std::string(str) + " for city type " + city_type->id);
          if (counter >= core::city_type::maximum_buildings) throw std::runtime_error("Too many buildings for city type " + city_type->id);
          city_type->buildings[counter] = t;
          ++counter;
        }
        
        city_type->buildings_count = counter;
      }
      
      if (const auto &proxy = table["default_buildings"]; proxy.valid() && proxy.get_type() == sol::type::table) {
        const auto &buildings = proxy.get<sol::table>();
        size_t counter = 0;
        for (const auto &pair : buildings) {
          if (pair.second.get_type() != sol::type::string) continue;
          
          const auto &str = pair.second.as<std::string_view>();
          auto t = ctx->get_entity<core::building_type>(str);
          if (t == nullptr) throw std::runtime_error("Could not find building type " + std::string(str) + " for city type " + city_type->id);
          if (counter >= core::city_type::maximum_buildings) throw std::runtime_error("Too many buildings for city type " + city_type->id);
          const size_t index = city_type->find_building(t);
          if (index == SIZE_MAX) throw std::runtime_error("Could not find building type " + std::string(str) + " among city type " + city_type->id + " buildings");
          city_type->default_buildings[counter] = index;
          ++counter;
        }
        
        city_type->default_buildings_count = counter;
      }
      
      // имеет смысл к этому моменту задать тип здания, кажется так сейчас и работает
      // эти проверки не имеют смысла, не все здания вообще имеет смысл указывать
      // у некоторых городов могут быть например ультимативные улучшения
//       for (size_t i = 0; i < city_type->buildings.size() && city_type->buildings[i] != nullptr; ++i) {
//         auto building = city_type->buildings[i];
//         if (building->upgrades_to == nullptr) continue;
//         
//         const size_t index = city_type->find_building(building->upgrades_to);
//         if (index == SIZE_MAX) throw std::runtime_error("Could not find upgrades to building " + building->upgrades_to->id + " in city type " + city_type->id + ". All buildings in chain must be added in city type buildings list");
//       }
      
//       for (size_t i = 0; i < city_type->buildings.size() && city_type->buildings[i] != nullptr; ++i) {
//         auto building = city_type->buildings[i];
//         if (building->replaced == nullptr) continue;
//         
//         const size_t index = city_type->find_building(building->replaced);
//         if (index == SIZE_MAX) throw std::runtime_error("Could not find replaced building " + building->replaced->id + " in city type " + city_type->id + ". Replaced building must be added in city type buildings list");
//       }
      
      {
        // то есть тут у нас только статы города? иначе это были бы модификаторы, наверное да
        const auto &stats = table.get<sol::table>("stats");
        for (const auto &pair : stats) {
          if (pair.first.get_type() != sol::type::string) continue;
          if (pair.second.get_type() != sol::type::number) continue;
          
          auto stat = pair.first.as<std::string_view>();
          auto value = pair.second.as<float>();
          
          const auto itr = core::stats_list::map.find(stat);
          if (itr == core::stats_list::map.end()) throw std::runtime_error("Could not find city stat " + std::string(stat));
          
          if (const uint32_t index = stats_list::to_city_stat(itr->second); index != UINT32_MAX) {
            city_type->stats.set(index, value);
          } else throw std::runtime_error("Stat " + std::string(stat) + " is not belongs to city stats");
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
