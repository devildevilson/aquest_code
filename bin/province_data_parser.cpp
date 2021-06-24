#include "data_parser.h"

#include "utils/globals.h"
#include "utils/table_container.h"
#include "utils/string_container.h"
#include "utils/serializator_helper.h"

#include "core/context.h"

#include "map.h"
#include "map_creator.h"

#define TO_LUA_INDEX(index) ((index)+1)
#define FROM_LUA_INDEX(index) ((index)-1)

namespace devils_engine {
  namespace utils {
    const check_table_value province_table[] = {
      {
        "neighbors",
        check_table_value::type::array_t,
        0, 0, 
        {
          {
            STATS_ARRAY,
            check_table_value::type::int_t,
            0, core::map::hex_count_d(core::map::detail_level), {}
          }
        }
      },
      {
        "tiles",
        check_table_value::type::array_t,
        check_table_value::value_required, 0, 
        {
          {
            STATS_ARRAY,
            check_table_value::type::int_t,
            0, core::map::hex_count_d(core::map::detail_level), {}
          }
        }
      },
      {
        "title",
        check_table_value::type::string_t,
        check_table_value::value_required, 0, {}
      },
      {
        "max_cities_count",
        check_table_value::type::int_t,
        0, 0, {}
      }
    };
    
    size_t add_province(const sol::table &table) {
      return global::get<map::creator::table_container_t>()->add_table(static_cast<size_t>(core::structure::province), table);
    }
    
    size_t register_province() {
      return global::get<map::creator::table_container_t>()->register_table(static_cast<size_t>(core::structure::province));
    }
    
    size_t register_provinces(const size_t &count) {
      return global::get<map::creator::table_container_t>()->register_tables(static_cast<size_t>(core::structure::province), count);
    }
    
    void set_province(const size_t &index, const sol::table &table) {
      global::get<map::creator::table_container_t>()->set_table(static_cast<size_t>(core::structure::province), index, table);
    }
    
    bool validate_province(const size_t &index, const sol::table &table) {
      size_t counter = 0;
      auto id = table["id"];
      std::string check_str;
      if (id.valid()) {
        check_str = id;
      } else {
        check_str = "province" + std::to_string(index);
      }
      
      const size_t size = sizeof(province_table) / sizeof(province_table[0]);
      recursive_check(check_str, "province", table, nullptr, province_table, size, counter);
      
      return counter == 0;
    }
    
    bool validate_province_and_save(const size_t &index, sol::this_state lua, const sol::table &table, utils::world_serializator* container) {
      const bool ret = validate_province(index, table);
      if (!ret) return false;
      
//       const size_t size = sizeof(building_table) / sizeof(building_table[0]);
      sol::state_view state(lua);
//       auto keyallow = state.create_table(); // похоже что серпент и вложенные таблицы тоже так же проверяет значит придется сохранять все
//       for (size_t i = 0; i < size; ++i) {
//         keyallow.set(building_table[i].key, true);
//       }
//       auto str = table_to_string(lua, table, keyallow);
      auto str = table_to_string(lua, table, sol::table());
      if (str.empty()) throw std::runtime_error("Could not serialize province table");
      ASSERT(false);
      //container->add_data(core::structure::province, std::move(str));
      
      return true;
    }
    
    void parse_province(core::province* province, const sol::table &table) {
      auto to_data = global::get<utils::data_string_container>();
      auto ctx = global::get<core::context>();
      
      { // тут можно собрать от тайлов (скорее всего не все данные тайла будем задавать в генераторе)
        const auto proxy = table["tiles"];
        if (!proxy.valid() && proxy.get_type() != sol::type::table) throw std::runtime_error("Province table lacks tiles array");
        const auto &tiles = proxy.get<sol::table>();
        for (auto itr = tiles.begin(); itr != tiles.end(); ++itr) {
          if (!(*itr).second.is<uint32_t>()) continue;
          const uint32_t tile_index = FROM_LUA_INDEX((*itr).second.as<uint32_t>());
          province->tiles.push_back(tile_index);
        }
        
        province->tiles.shrink_to_fit();
      }
      
      if (const auto proxy = table["neighbors"]; proxy.valid() && proxy.get_type() == sol::type::table) {
        const auto &tiles = proxy.get<sol::table>();
        for (auto itr = tiles.begin(); itr != tiles.end(); ++itr) {
          if (!(*itr).second.is<uint32_t>()) continue;
          const uint32_t neighbour_index = FROM_LUA_INDEX((*itr).second.as<uint32_t>());
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
        const uint32_t size = count.get<uint32_t>();
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
