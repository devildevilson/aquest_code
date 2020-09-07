#include "data_parser.h"
#include "utils/globals.h"
#include "core_structures.h"
#include "utils/table_container.h"
#include "utils/string_container.h"
#include "core_context.h"

namespace devils_engine {
  namespace utils {
    size_t add_city_type(const sol::table &table) {
      return global::get<utils::table_container>()->add_table(core::structure::city_type, table);
    }
    
    bool validate_city_type(const sol::table &table) {
      return true;
    }
    
    bool validate_city_type_and_save(sol::this_state lua, const sol::table &table) {
      return true;
    }
    
    void parse_city_type(core::city_type* city_type, const sol::table &table) {
      auto to_data = global::get<utils::data_string_container>();
      auto ctx = global::get<core::context>();
      
      city_type->id = table.get<std::string>("id");
      
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
        const auto &stats = table.get<sol::table>("base_stats");
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
      
      {
        // графика
      }
    }
  }
}
