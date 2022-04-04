#include "troop_type_parser.h"

#include "utils/globals.h"
#include "utils/string_container.h"
#include "utils/systems.h"
#include "utils/magic_enum_header.h"

#include "core/stats.h"

#include "context.h"
#include "structures.h"

#include "bin/map_creator.h"
#include "bin/data_parser.h"

namespace devils_engine {
  namespace utils {
    const check_table_value troop_type_table[] = {
      {
        "id",
        check_table_value::type::string_t,
        check_table_value::value_required, 0, {}
      }, 
      {
        "default_unit_state",
        check_table_value::type::string_t,
        check_table_value::value_required, 0, {}
      },
      {
        "units_count",
        check_table_value::type::int_t,
        check_table_value::value_required, 256, {}
      },
      {
        "unit_scale",
        check_table_value::type::float_t,
        check_table_value::value_required, 0, {}
      },
      {
        "stats",
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
    
    size_t add_battle_troop_type(const sol::table &table) {
      return global::get<map::creator::table_container_t>()->add_table(static_cast<size_t>(battle::structure_type::troop_type), table);
    }
    
    bool validate_battle_troop_type(const uint32_t &index, const sol::table &table) {
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
    
    void load_battle_troop_types(battle::context* ctx, const std::vector<sol::table> &tables) {
      auto data_container = global::get<systems::battle_t>()->unit_states_map;
      auto type_container = global::get<utils::data_string_container>();
      for (size_t i = 0; i < tables.size(); ++i) {
        const auto &table = tables[i];
        battle::troop_type obj;
        obj.id = table["id"];
        
        const std::string unit_type_id = table["default_unit_state"];
        const size_t index = data_container->get(unit_type_id);
        if (index == SIZE_MAX) throw std::runtime_error("Could not find unit state " + unit_type_id);
        //obj.units_type = ctx->get_entity<battle::unit_type>(index);
        obj.default_unit_state = ctx->get_entity<core::state>(index);
        
        obj.units_count = table["units_count"];
        ASSERT(obj.units_count < 256);
        obj.unit_scale = table["unit_scale"];
        
        const sol::table t = table["stats"];
        for (size_t i = 0; i < core::troop_stats::count; ++i) {
          const auto str = magic_enum::enum_name<core::troop_stats::values>(static_cast<core::troop_stats::values>(i));
          if (auto proxy = t[str]; proxy.valid()) obj.stats[i].ival = proxy.get<double>();
        }
        
        ctx->set_entity_data(i, obj);
        
        auto ptr = ctx->get_entity<battle::troop_type>(i);
        type_container->insert(ptr->id, i);
      }
    }
  }
}
