#include "data_parser.h"
#include "utils/globals.h"
#include "core_structures.h"
#include "utils/table_container.h"
#include "utils/string_container.h"
#include "core_context.h"
#include "utils/serializator_helper.h"
#include "map_creator.h"

// наверное чтобы не мучиться лучше какой нибудь список задать

namespace devils_engine {
  namespace utils {
    const check_table_value building_type_table[] = {
      {
        "id",
        check_table_value::type::string_t,
        check_table_value::value_required, 0, {}
      }, 
      {
        "prerequisites",
        check_table_value::type::array_t,
        0, core::building_type::maximum_prev_buildings,
        {
          {
            ID_ARRAY,
            check_table_value::type::string_t,
            0, 0, {}
          }
        }
      },
      {
        "limit_buildings",
        check_table_value::type::array_t,
        0, core::building_type::maximum_limit_buildings,
        {
          {
            ID_ARRAY,
            check_table_value::type::string_t,
            0, 0, {}
          }
        }
      },
      {
        "replaced",
        check_table_value::type::string_t,
        0, 0, {}
      },
      {
        "upgrades_from",
        check_table_value::type::string_t,
        0, 0, {}
      },
      {
        "time",
        check_table_value::type::int_t,
        check_table_value::value_required, 0, {}
      },
      // я задаю тут стринги вместо индексов, зачем? это имеет смысл только в случае с фракциями, но фракционные индексы мы можем задать каким то другим способом
      // я могу с помощью индексов задать все статы в одном месте
      { 
        "modifiers",
        check_table_value::type::array_t,
        0, core::building_type::maximum_stat_modifiers,
        {
          {
            STATS_ARRAY,
            check_table_value::type::int_t,
            0, core::offsets::troop_stats, {}
          }
        }
      },
      {
        "unit_modifiers", // тут нужно указать тип юнита, стат и значение
        check_table_value::type::array_t,
        0, core::building_type::maximum_unit_stat_modifiers,
        {
          {
            "unit_type", 
            check_table_value::type::string_t,
            check_table_value::value_required, 0, {}
          },
          {
            STAT_ID,
            check_table_value::type::int_t,
            core::offsets::troop_stats, core::offsets::troop_stats + core::troop_stats::count, {}
          },
          {
            "value",
            check_table_value::type::float_t,
            check_table_value::value_required, 0, {}
          }
        }
      },
      {
        "hero_modifiers",
        check_table_value::type::array_t,
        0, core::building_type::maximum_stat_modifiers,
        {
          {
            "hero_type",
            check_table_value::type::int_t,
            check_table_value::value_required, 0, {}
          },
          {
            STAT_ID,
            check_table_value::type::int_t,
            core::offsets::hero_stats, core::offsets::hero_stats + core::hero_stats::count, {}
          },
          {
            "value",
            check_table_value::type::float_t,
            check_table_value::value_required, 0, {}
          }
        }
      },
      {
        "money_cost",
        check_table_value::type::float_t,
        0, 0, {}
      },
      {
        "authority_cost",
        check_table_value::type::float_t,
        0, 0, {}
      },
      {
        "esteem_cost",
        check_table_value::type::float_t,
        0, 0, {}
      },
      {
        "influence_cost",
        check_table_value::type::float_t,
        0, 0, {}
      }
    };
    
    size_t add_building(const sol::table &table) {
      return global::get<map::creator::table_container_t>()->add_table(static_cast<size_t>(core::structure::building_type), table);
    }
    
#define SAME_TYPE 1
#define DIFFERENT_TYPE 0
#define DONT_CARE 2
    int check_sol_type(const sol::type &t, const sol::object &obj) {
      switch (t) {
        case sol::type::number: {
          if (obj.is<size_t>()) return SAME_TYPE;
          if (obj.is<double>()) return SAME_TYPE;
          break;
        }
        
        case sol::type::string: {
          if (obj.is<std::string>()) return SAME_TYPE;
          break;
        }
        
        default: return DONT_CARE;
      }
      
      return DIFFERENT_TYPE;
    }
    
    void recursive_check(const std::string_view &id, const std::string_view &data_type, const sol::table &table, const check_table_value* current_check, const check_table_value* array_check, const size_t &size, size_t &counter) {
      // если current_check != nullptr мы можем понять какой тип массива к нам пришел
      
      ASSERT(size != 0);
      const bool is_id_array = array_check[0].key == ID_ARRAY;
      const bool is_stats_array = array_check[0].key == STATS_ARRAY;
      const bool is_numeric_array = array_check[0].key == NUM_ARRAY;
      const bool is_stats_v2_array = array_check[0].key == STATS_V2_ARRAY;
      const bool basic_check = !is_id_array && !is_stats_array && !is_numeric_array;
      
      if (!basic_check) {
        ASSERT(current_check->value_type == check_table_value::type::array_t);
        sol::type key_type = sol::type::none;
        sol::type value_type = sol::type::none;
        size_t maximum_key_num = array_check[0].max_count == 0 ? SIZE_MAX : array_check[0].max_count;
        size_t minimum_key_num = array_check[0].flags;
        if (is_stats_array) { key_type = sol::type::number; value_type = sol::type::number; }
        if (is_id_array) { value_type = sol::type::string; }
        if (is_numeric_array) { value_type = sol::type::number; }
        if (is_stats_v2_array) { key_type = sol::type::string; value_type = sol::type::number; }
        
        size_t data_counter = 0;
        for (auto itr = table.begin(); itr != table.end(); ++itr) {
          int kret = check_sol_type(key_type, (*itr).first);
          int vret = check_sol_type(value_type, (*itr).second);
          if (kret == DIFFERENT_TYPE) continue;
          if (vret == DIFFERENT_TYPE) continue;
          if (kret == DONT_CARE && vret == DONT_CARE) continue;
          
          if (key_type == sol::type::number) {
            const size_t key = (*itr).first.as<size_t>();
            if ((key < minimum_key_num || key >= maximum_key_num) && is_stats_array) { PRINT("Bad stat key value in container " + std::string(current_check->key) + " in table " + std::string(id)); ++counter; }
          }
          
          ++data_counter;
        }
        
        const size_t max_data_values = current_check->max_count == 0 ? SIZE_MAX : current_check->max_count;
        if (data_counter > max_data_values) {
          PRINT("Too many data in container " + std::string(current_check->key) + " in table " + std::string(id)); ++counter;
          //throw std::runtime_error("fs;amsfs");
        }
      }
      
      if (basic_check) {
        for (size_t i = 0; i < size; ++i) {
          const auto &check = array_check[i];
          const bool stat_key = check.key == STAT_ID;
          const bool req = (check.flags & check_table_value::value_required) == check_table_value::value_required || stat_key;
          auto proxy = table[check.key];
          if (req && !proxy.valid()) { PRINT("Table field " + std::string(check.key) + " is required in " + std::string(data_type) + " data type"); ++counter; }
          if (!proxy.valid()) continue;
          switch (check.value_type) {
            case check_table_value::type::bool_t: {
              const auto t = proxy.get_type();
              if (t != sol::type::boolean) { PRINT("Type of container value " + std::string(check.key) + " is wrong in table " + std::string(id) + ". Must be boolean"); ++counter; }
              break;
            }
            
            case check_table_value::type::int_t: {
              const auto t = proxy.get_type();
              if (t != sol::type::number) { PRINT("Type of container value " + std::string(check.key) + " is wrong in table " + std::string(id) + ". Must be number"); ++counter; }
              break;
            }
            
            case check_table_value::type::float_t: {
              const auto t = proxy.get_type();
              if (t != sol::type::number) { PRINT("Type of container value " + std::string(check.key) + " is wrong in table " + std::string(id) + ". Must be number"); ++counter; }
              break;
            }
            
            case check_table_value::type::string_t: {
              const auto t = proxy.get_type();
              if (t != sol::type::string) { PRINT("Type of container value " + std::string(check.key) + " is wrong in table " + std::string(id) + ". Must be string"); ++counter; }
              break;
            }
            
            case check_table_value::type::array_t: {
              const auto t = proxy.get_type();
              if (t != sol::type::table) { PRINT("Type of container value " + std::string(check.key) + " is wrong in table " + std::string(id) + ". Must be table"); ++counter; }
              recursive_check(id, data_type, proxy.get<sol::table>(), &check, check.nested_array_data.begin(), check.nested_array_data.size(), counter);
              break;
            }
          }
          
          if (stat_key) {
            const size_t stat_id = proxy.get<size_t>();
            const uint32_t start = check.flags;
            const uint32_t end = check.max_count;
            if (stat_id >= start && stat_id < end) continue;
            ASSERT(current_check != nullptr);
            PRINT("Bad stat id in container " + std::string(current_check->key) + " in table " + std::string(id)); ++counter;
          }
        }
      }
    }
    
    bool validate_building(const size_t &index, const sol::table &table) {
      size_t counter = 0;
      auto id = table["id"];
      std::string check_str;
      if (id.valid()) {
        check_str = id;
      } else {
        check_str = "building" + std::to_string(index);
      }
      
      const size_t size = sizeof(building_type_table) / sizeof(building_type_table[0]);
      recursive_check(check_str, "building", table, nullptr, building_type_table, size, counter);
      
      return counter == 0;
    }
    
    bool validate_building_and_save(const size_t &index, sol::this_state lua, const sol::table &table, utils::world_serializator* container) {
      const bool ret = validate_building(index, table);
      if (!ret) return false;
      
//       const size_t size = sizeof(building_table) / sizeof(building_table[0]);
      sol::state_view state(lua);
//       auto keyallow = state.create_table(); // похоже что серпент и вложенные таблицы тоже так же проверяет значит придется сохранять все
//       for (size_t i = 0; i < size; ++i) {
//         keyallow.set(building_table[i].key, true);
//       }
//       auto str = table_to_string(lua, table, keyallow);
      auto str = table_to_string(lua, table, sol::table());
      if (str.empty()) throw std::runtime_error("Could not serialize building table");
      container->add_data(core::structure::building_type, std::move(str));
      
      return true;
    }
    
    void parse_building(core::building_type* building_type, const sol::table &table) {
      // нужно задать предыдущие здания и это строки с id
      // как из строк получать индекс?
      auto to_data = global::get<utils::data_string_container>();
      auto ctx = global::get<core::context>();
      
//       building_type->id = table.get<std::string>("id");
      
      if (auto proxy = table["prerequisites"]; proxy.valid()) {
        size_t current_index = 0;
        const auto &table = proxy.get<sol::table>();
        for (auto itr = table.begin(); itr != table.end(); ++itr) {
          if (!(*itr).second.is<std::string>()) continue;
          
          auto view = (*itr).second.as<std::string_view>();
          const size_t index = to_data->get(view);
          if (index == SIZE_MAX) throw std::runtime_error("Could not find previous buildings type id " + std::string(view) + " for building " + building_type->id);
          auto t = ctx->get_entity<core::building_type>(index);
          building_type->prev_buildings[current_index] = t;
          ++current_index;
        }
      }
      
      if (auto proxy = table["limit_buildings"]; proxy.valid()) {
        size_t current_index = 0;
        const auto &table = proxy.get<sol::table>();
        for (auto itr = table.begin(); itr != table.end(); ++itr) {
          if (!(*itr).second.is<std::string>()) continue;
          
          auto view = (*itr).second.as<std::string_view>();
          const size_t index = to_data->get(view);
          if (index == SIZE_MAX) throw std::runtime_error("Could not find previous buildings type id " + std::string(view) + " for building " + building_type->id);
          auto t = ctx->get_entity<core::building_type>(index);
          building_type->prev_buildings[current_index] = t;
          ++current_index;
        }
      }
      
      if (auto proxy = table["replaced"]; proxy.valid()) {
        auto view = proxy.get<std::string_view>();
        const size_t index = to_data->get(view);
        if (index == SIZE_MAX) throw std::runtime_error("Could not find replaced building type id " + std::string(view) + " for building " + building_type->id);
        building_type->replaced = ctx->get_entity<core::building_type>(index);
      }
      
      if (auto proxy = table["upgrades_from"]; proxy.valid()) {
        auto view = proxy.get<std::string_view>();
        const size_t index = to_data->get(view);
        if (index == SIZE_MAX) throw std::runtime_error("Could not find upgrades from building type id " + std::string(view) + " for building " + building_type->id);
        building_type->upgrades_from = ctx->get_entity<core::building_type>(index);
      }
      
      //if (auto proxy = table["time"]; proxy.valid()) 
      {
        //auto time = proxy.get<size_t>();
        const size_t time = table["time"];
        building_type->time = time;
      }
      
      // еще нужно указать стоимость
      // вообще так то это модификатор персонажа (-деньги), поэтому мы вполне можем для тултипа найти этот модификатор
      // в цк2 указывалось напрямую через кост
      
      if (auto proxy = table["character_modifiers"]; proxy.valid()) {
        uint32_t current_stat = 0;
        const auto &table = proxy.get<sol::table>();
        for (auto itr = table.begin(); itr != table.end(); ++itr) { 
          if (!(*itr).first.is<std::string>()) continue;
          if (!(*itr).second.is<double>()) continue;
          
          auto stat = (*itr).first.as<std::string_view>();
          auto value = (*itr).second.as<double>();
          
          if (auto val = magic_enum::enum_cast<core::character_stats::values>(stat); val.has_value()) {
            building_type->mods[current_stat].type = core::unit_type::character;
            const uint32_t stat = val.value();
            building_type->mods[current_stat].stat = stat;
            switch (core::character_stats::types[stat]) {
              case core::stat_type::float_t: building_type->mods[current_stat].mod.fval = value; break;
              case core::stat_type::uint_t: building_type->mods[current_stat].mod.uval = value;  break;
              case core::stat_type::int_t: building_type->mods[current_stat].mod.ival = value;   break;
            }
            ++current_stat;
            
            continue;
          }
          
          if (auto val = magic_enum::enum_cast<core::faction_stats::values>(stat); val.has_value()) {
            building_type->mods[current_stat].type = core::unit_type::faction;
            building_type->mods[current_stat].stat = val.value();
            building_type->mods[current_stat].mod.fval = value;
            ++current_stat;
            
            continue;
          }
        }
      }
      
      if (auto proxy = table["province_modifiers"]; proxy.valid()) {
        uint32_t current_stat = 0;
        const auto &table = proxy.get<sol::table>();
        
        for (auto itr = table.begin(); itr != table.end(); ++itr) { 
          if (!(*itr).first.is<std::string>()) continue;
          if (!(*itr).second.is<double>()) continue;
          
          auto stat = (*itr).first.as<std::string_view>();
          auto value = (*itr).second.as<double>();
          
          if (auto val = magic_enum::enum_cast<core::province_stats::values>(stat); val.has_value()) {
            building_type->mods[current_stat].type = core::unit_type::province;
            building_type->mods[current_stat].stat = val.value();
            building_type->mods[current_stat].mod.fval = float(value);
            ++current_stat;
          }
        }
      }
      
      if (auto proxy = table["city_modifiers"]; proxy.valid()) {
        uint32_t current_stat = 0;
        const auto &table = proxy.get<sol::table>();
        
        for (auto itr = table.begin(); itr != table.end(); ++itr) { 
          if (!(*itr).first.is<std::string>()) continue;
          if (!(*itr).second.is<double>()) continue;
          
          auto stat = (*itr).first.as<std::string_view>();
          auto value = (*itr).second.as<double>();
          
          if (auto val = magic_enum::enum_cast<core::city_stats::values>(stat); val.has_value()) {
            building_type->mods[current_stat].type = core::unit_type::city;
            building_type->mods[current_stat].stat = val.value();
            building_type->mods[current_stat].mod.fval = value;
            ++current_stat;
          } 
          //else throw std::runtime_error("Could not find stat " + std::string(stat)); // по идее это мы должны проверить при валидации
        }
      }
      
      // бафы для юнитов во всей провинции?
      if (auto proxy = table["unit_modifiers"]; proxy.valid()) {
        uint32_t current_stat = 0;
        const auto &table = proxy.get<sol::table>();
        
        for (auto itr = table.begin(); itr != table.end(); ++itr) {
          if (!(*itr).first.is<std::string>()) continue;
          if (!(*itr).second.is<sol::table>()) continue;
          
          const auto unit_type = (*itr).first.as<std::string_view>();
          const size_t index = to_data->get(unit_type);
          if (index == SIZE_MAX) throw std::runtime_error("Could not find unit type " + std::string(unit_type));
          
          auto type = ctx->get_entity<core::troop_type>(index);
          const auto &table = (*itr).second.as<sol::table>();
          for (auto itr = table.begin(); itr != table.end(); ++itr) {
            if (!(*itr).first.is<std::string>()) continue;
            if (!(*itr).second.is<double>()) continue;
            
            auto stat = (*itr).first.as<std::string_view>();
            auto value = (*itr).second.as<double>();
            
            if (auto val = magic_enum::enum_cast<core::city_stats::values>(stat); val.has_value()) {
              building_type->unit_mods[current_stat].type = type;
              building_type->unit_mods[current_stat].stat = val.value();
              building_type->unit_mods[current_stat].mod.ival = int32_t(value);
            }
          }
        }
      }
      
      if (auto proxy = table["hero_modifiers"]; proxy.valid()) { // для конкретного типа героя
        ASSERT(false);
        uint32_t current_stat = 0;
        const auto &table = proxy.get<sol::table>();
        
        for (auto itr = table.begin(); itr != table.end(); ++itr) {
          if (!(*itr).first.is<std::string>()) continue;
          if (!(*itr).second.is<sol::table>()) continue;
          
          const auto unit_type = (*itr).first.as<std::string_view>();
          const size_t index = to_data->get(unit_type);
          if (index == SIZE_MAX) throw std::runtime_error("Could not find unit type " + std::string(unit_type));
          
//           auto type = ctx->get_entity<core::hero_type>(index);
          const auto &table = (*itr).second.as<sol::table>();
          for (auto itr = table.begin(); itr != table.end(); ++itr) {
            if (!(*itr).first.is<std::string>()) continue;
            if (!(*itr).second.is<double>()) continue;
            
            auto stat = (*itr).first.as<std::string_view>();
            auto value = (*itr).second.as<double>();
            
            if (auto val = magic_enum::enum_cast<core::city_stats::values>(stat); val.has_value()) {
//               building_type->unit_mods[current_stat].type = type;
//               building_type->unit_mods[current_stat].stat = val.value();
//               building_type->unit_mods[current_stat].mod.ival = int32_t(value);
            }
          }
        }
      }
    }
  }
}
