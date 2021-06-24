#include "battle_troop_parser.h"

#include "utils/globals.h"
#include "utils/string_container.h"

#include "core/stats.h"

#include "data_parser.h"
#include "battle_context.h"
#include "map_creator.h"
#include "battle_structures.h"

namespace devils_engine {
  namespace utils {
    const check_table_value troop_table[] = {
      {
        "type",
        check_table_value::type::string_t,
        check_table_value::value_required, 0, {}
      }, 
      {
        "tile_index",
        check_table_value::type::int_t,
        check_table_value::value_required, 0, {}
      },
      {
        "current_stats",
        check_table_value::type::array_t,
        check_table_value::value_required, 0, 
        {
          {
            STATS_V2_ARRAY,
            check_table_value::type::string_t,
            0, 0, {}
          }
        }
      }
    };
    
    size_t add_battle_troop(const sol::table &table) {
      return global::get<map::creator::table_container_t>()->add_table(static_cast<size_t>(battle::structure_type::troop), table);
    }
    
    bool validate_battle_troop(const uint32_t &index, const sol::table &table) {
      size_t counter = 0;
      auto id = table["id"];
      std::string check_str;
      if (id.valid()) {
        check_str = id;
      } else {
        check_str = "troop" + std::to_string(index);
      }
      
      const size_t size = sizeof(troop_table) / sizeof(troop_table[0]);
      recursive_check(check_str, "troop", table, nullptr, troop_table, size, counter);
      
      return counter == 0;
    }
    
    void load_battle_troops(battle::context* ctx, const std::vector<sol::table> &tables) {
      auto data_container = global::get<utils::data_string_container>();
      for (size_t i = 0; i < tables.size(); ++i) {
        const auto &table = tables[i];
        battle::troop obj;
        const std::string type_str = table["type"];
        const size_t index = data_container->get(type_str);
        if (index == SIZE_MAX) throw std::runtime_error("Could not find troop type " + type_str);
        obj.type = ctx->get_entity<battle::troop_type>(index);
        
        obj.tile_index = table["tile_index"];
        
        const sol::table t = table["current_stats"];
        for (size_t i = 0; i < core::troop_stats::count; ++i) {
          const auto str = magic_enum::enum_name<core::troop_stats::values>(static_cast<core::troop_stats::values>(i));
          if (auto proxy = t[str]; proxy.valid()) obj.stats[i].ival = proxy.get<double>();
        }
        
        ctx->set_entity_data(i, obj);
      }
    }
  }
}
