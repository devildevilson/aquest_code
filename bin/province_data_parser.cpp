#include "data_parser.h"
#include "utils/globals.h"
#include "core_structures.h"
#include "utils/table_container.h"
#include "utils/string_container.h"
#include "core_context.h"

namespace devils_engine {
  namespace utils {
    size_t add_province(const sol::table &table) {
      return global::get<utils::table_container>()->add_table(core::structure::province, table);
    }
    
    size_t register_province() {
      return global::get<utils::table_container>()->register_table(core::structure::province);
    }
    
    size_t register_provinces(const size_t &count) {
      return global::get<utils::table_container>()->register_tables(core::structure::province, count);
    }
    
    void set_province(const size_t &index, const sol::table &table) {
      global::get<utils::table_container>()->set_table(core::structure::province, index, table);
    }
    
    bool validate_province(const sol::table &table) {
      return true;
    }
    
    bool validate_province_and_save(sol::this_state lua, const sol::table &table) {
      return true;
    }
    
    void parse_province(core::province* province, const sol::table &table) {
      auto to_data = global::get<utils::data_string_container>();
      auto ctx = global::get<core::context>();
      
//       { // тут можно собрать от тайлов
//         const auto &tiles = table.get<sol::table>("tiles");
//         for (auto itr = tiles.begin(); itr != tiles.end(); ++itr) {
//           if (!(*itr).second.is<uint32_t>()) continue;
//           const uint32_t tile_index = (*itr).second.as<uint32_t>();
//           province->tiles.push_back(tile_index);
//         }
//         
//         province->tiles.shrink_to_fit();
//       }
      
      {
        const auto &tiles = table.get<sol::table>("neighbours");
        for (auto itr = tiles.begin(); itr != tiles.end(); ++itr) {
          if (!(*itr).second.is<uint32_t>()) continue;
          const uint32_t neighbour_index = (*itr).second.as<uint32_t>();
          province->neighbours.push_back(neighbour_index);
        }
        
        province->neighbours.shrink_to_fit();
      }
      
      // наверно проще тут указать, а потом собрать при обработке
      {
        const std::string &str = table["title"];
        const size_t index = to_data->get(str);
        if (index == SIZE_MAX) throw std::runtime_error("Could not find title " + str);
        auto title = ctx->get_entity<core::titulus>(index);
        province->title = title;
      }
      
      if (const auto &count = table["max_cities_count"]; count.valid()) { // мы можем это указать для того чтобы как то ограничить строительство новых городов
        const uint32_t size = count.get<size_t>();
        province->cities_max_count = size;
      }
      
//       { // здесь собираем от городов
//         size_t counter = 0;
//         const auto &tiles = table.get<sol::table>("cities");
//         for (auto itr = tiles.begin(); itr != tiles.end(); ++itr) {
//           if (!(*itr).second.is<uint32_t>()) continue;
//           const uint32_t city_index = (*itr).second.as<uint32_t>();
//           auto city = ctx->get_entity<core::city>(city_index);
//           ASSERT(counter < core::province::cities_max_game_count);
//           province->cities[counter] = city;
//           ++counter;
//         }
//         
//         province->cities_count = counter;
//       }

      UNUSED_VARIABLE(to_data);
      UNUSED_VARIABLE(ctx);
    }
  }
}
