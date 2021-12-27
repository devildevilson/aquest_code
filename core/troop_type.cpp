#include "troop_type.h"

#include "bin/data_parser.h"
#include "stats_table.h"

namespace devils_engine {
  namespace core {
    const utils::check_table_value troop_type_table[] = {
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
        "stats",
        utils::check_table_value::type::array_t,
        utils::check_table_value::value_required, 0,
        {
          {
            STATS_V2_ARRAY,
            utils::check_table_value::type::string_t,
            0, 0, {}
          }
        }
      },
      {
        "card",
        utils::check_table_value::type::string_t,
        0, 0, {}
      }
    };
    
    const structure troop_type::s_type;
    troop_type::troop_type() : 
      formation(nullptr),
      card{GPU_UINT_MAX} 
    {}
    
    bool validate_troop_type(const size_t &index, const sol::table &table) {
      size_t counter = 0;
      auto id = table["id"];
      std::string check_str;
      if (id.valid()) {
        check_str = id;
      } else {
        check_str = "troop_type" + std::to_string(index);
      }
      
      const size_t size = sizeof(troop_type_table) / sizeof(troop_type_table[0]);
      recursive_check(check_str, "troop_type", table, nullptr, troop_type_table, size, counter);
      
      return counter == 0;
    }
    
    void parse_troop_type(core::troop_type* troop_type, const sol::table &table) {
      troop_type->id = table["id"];
      troop_type->name_id = table["name_id"];
      if (const auto proxy = table["description_id"]; proxy.valid()) {
        troop_type->description_id = proxy.get<std::string>();
      }
      
      if (const auto proxy = table["stats"]; proxy.valid()) {
        const auto t = proxy.get<sol::table>();
        for (const auto &pair : t) {
          if (pair.first.get_type() != sol::type::string) continue;
          if (pair.second.get_type() != sol::type::number) continue;
          
          const auto str = pair.first.as<std::string_view>();
          const auto value = pair.second.as<double>();
          
          const auto itr = core::stats_list::map.find(str);
          if (itr == core::stats_list::map.end()) throw std::runtime_error("Could not find stat " + std::string(str));
          
          const uint32_t stat_id = core::stats_list::to_troop_stat(itr->second);
          if (stat_id == UINT32_MAX) throw std::runtime_error("Could not find stat " + std::string(str) + " among troop stats");
          
          troop_type->stats.set(stat_id, value);
        }
      }
      
      // формация
      
      // изображение отряда
      if (const auto proxy = table["card"]; proxy.valid()) {
        
      }
    }
  }
}
