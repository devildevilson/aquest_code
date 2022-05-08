#include "biome.h"

#include "bin/data_parser.h"
#include "utils/globals.h"
#include "core/context.h"
#include "utils/systems.h"
#include "render/image_controller.h"

#include <iostream>

#define MAKE_MAP_PAIR(name) std::make_pair(biome_attributes_names[size_t(biome::attributes::name)], biome::attributes::name)

namespace devils_engine {
  namespace core {
    const size_t biome::maximum_stat_modifiers;
    const size_t biome::maximum_unit_stat_modifiers;
    
    biome::biome() : 
      base_speed(1.0f), 
      data{
        {GPU_UINT_MAX},
        {GPU_UINT_MAX},
        {{GPU_UINT_MAX}, {GPU_UINT_MAX}, {GPU_UINT_MAX}},
        {glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
        {0.0f, 0.0f, 0.0f},
        0.0f, 
        0.0f
      } 
    {}
    
    const std::string_view biome_attributes_names[] = {
#define BIOME_ATTRIBUTE_FUNC(val) #val,
        BIOME_ATTRIBUTES_LIST
#undef BIOME_ATTRIBUTE_FUNC
    };
    
    const phmap::flat_hash_map<std::string_view, biome::attributes> biome_attributes_map = {
#define BIOME_ATTRIBUTE_FUNC(val) MAKE_MAP_PAIR(val),
        BIOME_ATTRIBUTES_LIST
#undef BIOME_ATTRIBUTE_FUNC
    };
    
    const utils::check_table_value biome_table[] = {
      {
        "id",
        utils::check_table_value::type::string_t,
        utils::check_table_value::value_required, 0, {}
      }, 
      {
        "name_id",
        utils::check_table_value::type::string_t,
        utils::check_table_value::value_required, 0, {}
      }, 
      {
        "description_id",
        utils::check_table_value::type::string_t,
        0, 0, {}
      },
      {
        "base_speed",
        utils::check_table_value::type::float_t,
        0, 0, {}
      },
      {
        "attributes",
        utils::check_table_value::type::array_t,
        0, 0,
        {
          {
            ID_ARRAY,
            utils::check_table_value::type::string_t,
            0, 0, {}
          }
        }
      },
      // если у нас будут разные типы даты, то наверное отсюда проверка отвалится
      {
        "data",
        utils::check_table_value::type::array_t,
        0, 0, {
          {
            "color",
            utils::check_table_value::type::int_t,
            utils::check_table_value::value_required, 0, {}
          },
          {
            "density",
            utils::check_table_value::type::float_t,
            0, 0, {}
          },
          {
            "objects",
            utils::check_table_value::type::array_t,
            0, 0, {
              {
                NESTED_ARRAY,
                utils::check_table_value::type::array_t,
                0, 0, {
                  {
                    "texture",
                    utils::check_table_value::type::string_t,
                    utils::check_table_value::value_required, 0, {}
                  },
                  {
                    "min_scale",
                    utils::check_table_value::type::float_t,
                    utils::check_table_value::value_required, 0, {}
                  },
                  {
                    "max_scale",
                    utils::check_table_value::type::float_t,
                    utils::check_table_value::value_required, 0, {}
                  },
                  {
                    "probability",
                    utils::check_table_value::type::float_t,
                    utils::check_table_value::value_required, 0, {}
                  },
                }
              }
            }
          }
        }
      },
    };
    
    bool validate_biome(const size_t &index, const sol::table &table) {
      size_t counter = 0;
      auto id = table["id"];
      std::string check_str;
      if (id.valid()) {
        check_str = id;
      } else {
        check_str = "biome" + std::to_string(index);
      }
      
      const size_t size = sizeof(biome_table) / sizeof(biome_table[0]);
      recursive_check(check_str, "biome", table, nullptr, biome_table, size, counter);
      
      return counter == 0;
    }
    
    bool validate_biome_and_save(const size_t &index, sol::this_state lua, const sol::table &table, utils::world_serializator*) {
      const bool ret = validate_building_type(index, table);
      if (!ret) return false;
      
//       const size_t size = sizeof(building_table) / sizeof(building_table[0]);
      sol::state_view state(lua);
//       auto keyallow = state.create_table(); // похоже что серпент и вложенные таблицы тоже так же проверяет значит придется сохранять все
//       for (size_t i = 0; i < size; ++i) {
//         keyallow.set(building_table[i].key, true);
//       }
//       auto str = table_to_string(lua, table, keyallow);
      auto str = utils::table_to_string(lua, table, sol::table());
      if (str.empty()) throw std::runtime_error("Could not serialize building table");
      ASSERT(false);
      //container->add_data(core::structure::building_type, std::move(str));
      
      return true;
    }
    
    void parse_biome(core::biome* biome, const sol::table &table) {
      {
        biome->id = table["id"];
      }
      
      {
        biome->name_id = table["name_id"];
      }
      
      if (const auto proxy = table["description_id"]; proxy.valid() && proxy.get_type() == sol::type::string) {
        biome->description_id = proxy.get<std::string>();
      }
      
      if (const auto proxy = table["base_speed"]; proxy.valid() && proxy.get_type() == sol::type::number) {
        biome->base_speed = proxy.get<float>();
      }
      
      if (const auto proxy = table["attributes"]; proxy.valid() && proxy.get_type() == sol::type::table) {
        const auto attributes = proxy.get<sol::table>();
        for (const auto &pair : attributes) {
          if (pair.second.get_type() != sol::type::string) continue;
          
          const auto str = pair.second.as<std::string_view>();
          const auto itr = biome_attributes_map.find(str);
          if (itr == biome_attributes_map.end()) throw std::runtime_error("Biome attribute " + std::string(str) + " is not exist");
          
          biome->attribs.set(static_cast<size_t>(itr->second), true);
        }
      }
      
      auto cont = global::get<systems::core_t>()->image_controller;
      if (const auto proxy = table["data"]; proxy.valid() && proxy.get_type() == sol::type::table) {
        const auto data = proxy.get<sol::table>();
        biome->data.color.container = data["color"];
        
        size_t counter = 0;
        if (const auto proxy = data["objects"]; proxy.valid() && proxy.get_type() == sol::type::table) {
          const auto objects = proxy.get<sol::table>();
          for (const auto &pair : objects) {
            if (pair.second.get_type() != sol::type::table) continue;
            if (counter >= render::map_biome_objects_count) throw std::runtime_error("Too many biome " + biome->id + " objects data");
            
            const auto obj_table = pair.second.as<sol::table>();
            {
              const auto str = obj_table["texture"].get<std::string_view>();
              std::string_view img_id;
              uint32_t layer;
              bool mirror_u;
              bool mirror_v;
              const bool ret = render::parse_image_id(str, img_id, layer, mirror_u, mirror_v);
              if (!ret) throw std::runtime_error("Bad texture id " + std::string(str));
              auto view = cont->get_view(img_id);
              if (layer >= view->count) throw std::runtime_error("Image pack " + std::string(img_id) + " does not have " + std::to_string(layer) + " amount of images");
              
              biome->data.texture = view->get_image(layer, mirror_u, mirror_v);
            }
            
            biome->data.scales[counter].x = obj_table["min_scale"].get<float>();
            biome->data.scales[counter].y = obj_table["max_scale"].get<float>();
            biome->data.probabilities[counter] = obj_table["probability"].get<float>();
                        
            ++counter;
          }
        }
        
        const bool has_objects = counter > 0;
        if (const auto proxy = data["density"]; proxy.valid() && proxy.get_type() == sol::type::number) {
          if (!has_objects) std::cout << "Warning: Biome " << biome->id << " has 0 objects, setting density to 0" << "\n";
          biome->data.density = proxy.get<float>() * float(has_objects);
          if (biome->data.density < EPSILON) {
            std::cout << "Warning: Biome " << biome->id << " density is less then EPSILON value (" << EPSILON << "), setting density to 0" << "\n";
            biome->data.density = 0.0f;
          }
        }
      }
    }
  }
}

#undef MAKE_MAP_PAIR
