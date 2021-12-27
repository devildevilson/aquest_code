#include "province.h"

#include "bin/data_parser.h"
#include "utils/globals.h"
#include "utils/table_container.h"
#include "utils/string_container.h"
#include "utils/serializator_helper.h"

#include "context.h"
#include "culture.h"
#include "religion.h"

#include "bin/map.h"
// #include "bin/map_creator.h"

namespace devils_engine {
  namespace core {
    const structure province::s_type;
//     const size_t province::modificators_container_size;
//     const size_t province::events_container_size;
//     const size_t province::flags_container_size;
    const size_t province::cities_max_game_count;
    province::province() : 
      title(nullptr), 
      cities_max_count(0), 
      cities_count(0), 
      cities{nullptr},
      capital(nullptr)
    {
//       memset(stats.data(), 0, stats.size() * sizeof(stats[0]));
//       memset(current_stats.data(), 0, current_stats.size() * sizeof(current_stats[0]));
    }
    
    city* province::next_city(const core::city* city) const {
      return utils::ring::list_next<utils::list_type::province_cities>(city, cities);
    }
    
    OUTPUT_TITLE_TYPE province::get_title() const { return title; }
    OUTPUT_ARMY_TYPE province::get_army() const { return offensive_army; }
    OUTPUT_CITY_TYPE2 province::get_capital() const { return capital; }
    
    core::culture* province::get_culture() const { return culture; }
    core::culture_group* province::get_culture_group() const { return culture->group; }
    core::religion* province::get_religion() const { return religion; }
    core::religion_group* province::get_religion_group() const { return religion->group; }
    
    const utils::check_table_value province_table[] = {
      {
        "neighbors",
        utils::check_table_value::type::array_t,
        0, 0, 
        {
          {
            STATS_ARRAY,
            utils::check_table_value::type::int_t,
            0, core::map::hex_count_d(core::map::detail_level), {}
          }
        }
      },
      {
        "tiles",
        utils::check_table_value::type::array_t,
        utils::check_table_value::value_required, 0, 
        {
          {
            STATS_ARRAY,
            utils::check_table_value::type::int_t,
            0, core::map::hex_count_d(core::map::detail_level), {}
          }
        }
      },
      {
        "title",
        utils::check_table_value::type::string_t,
        utils::check_table_value::value_required, 0, {}
      },
      {
        "culture",
        utils::check_table_value::type::string_t,
        utils::check_table_value::value_required, 0, {}
      },
      {
        "religion",
        utils::check_table_value::type::string_t,
        utils::check_table_value::value_required, 0, {}
      },
      {
        "max_cities_count",
        utils::check_table_value::type::int_t,
        0, 0, {}
      }
    };
    
    size_t add_province(const sol::table &table) {
      //return global::get<devils_engine::map::creator::table_container_t>()->add_table(static_cast<size_t>(core::structure::province), table);
    }
    
    size_t register_province() {
      //return global::get<map::creator::table_container_t>()->register_table(static_cast<size_t>(core::structure::province));
    }
    
    size_t register_provinces(const size_t &count) {
      //return global::get<map::creator::table_container_t>()->register_tables(static_cast<size_t>(core::structure::province), count);
    }
    
    void set_province(const size_t &index, const sol::table &table) {
      //global::get<map::creator::table_container_t>()->set_table(static_cast<size_t>(core::structure::province), index, table);
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
      auto str = utils::table_to_string(lua, table, sol::table());
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
        for (const auto &pair : tiles) {
          //if (!pair.second.is<uint32_t>()) continue;
          //const uint32_t tile_index = FROM_LUA_INDEX(pair.second.as<uint32_t>());
          //province->tiles.push_back(tile_index);
          if (!pair.second.is<double>()) continue;
          const int64_t index = FROM_LUA_INDEX(pair.second.as<double>());
          assert(size_t(index) < core::map::hex_count_d(core::map::detail_level));
          province->tiles.push_back(index);
        }
        
        province->tiles.shrink_to_fit();
        ASSERT(!province->tiles.empty());
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
        const auto &str = table["title"].get<std::string_view>();
        auto title = ctx->get_entity<core::titulus>(str);
        if (title == nullptr) throw std::runtime_error("Could not find title " + std::string(str));
        province->title = title;
      }
      
      {
        const auto &str = table["religion"].get<std::string_view>();
        auto religion = ctx->get_entity<core::religion>(str);
        if (religion == nullptr) throw std::runtime_error("Could not find religion " + std::string(str));
        province->religion = religion;
      }
      
      {
        const auto &str = table["culture"].get<std::string_view>();
        auto culture = ctx->get_entity<core::culture>(str);
        if (culture == nullptr) throw std::runtime_error("Could not find culture " + std::string(str));
        province->culture = culture;
      }
      
      if (const auto &count = table["max_cities_count"]; count.valid()) { // мы можем это указать для того чтобы как то ограничить строительство новых городов
        const uint32_t size = count.get<uint32_t>();
        province->cities_max_count = size;
      }
      
      // города добавляются позже

      UNUSED_VARIABLE(to_data);
      UNUSED_VARIABLE(ctx);
    }
  }
}
