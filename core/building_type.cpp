#include "building_type.h"

#include "bin/data_parser.h"
#include "utils/globals.h"
#include "utils/table_container.h"
#include "utils/string_container.h"
#include "utils/serializator_helper.h"
#include "core/context.h"
#include "core/stats_table.h"

namespace devils_engine {
  namespace core {
    const structure building_type::s_type;
    const size_t building_type::maximum_prev_buildings;
    const size_t building_type::maximum_limit_buildings;
    const size_t building_type::maximum_stat_modifiers;
    const size_t building_type::maximum_unit_stat_modifiers;
    const size_t building_type::maximum_offensive_units_provided;
    const size_t building_type::maximum_defensive_units_provided;
    building_type::building_type() : 
      prev_buildings{nullptr}, 
      limit_buildings{nullptr}, 
      replaced(nullptr), 
      upgrades_from(nullptr),
      //upgrades_to(nullptr), 
      time(SIZE_MAX), 
      offensive_units{nullptr},
      defensive_units{nullptr},
      money_cost(0.0f), 
      authority_cost(0.0f), 
      esteem_cost(0.0f), 
      influence_cost(0.0f) 
    {}
    
    const utils::check_table_value building_type_table[] = {
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
        "prerequisites",
        utils::check_table_value::type::array_t,
        0, core::building_type::maximum_prev_buildings,
        {
          {
            ID_ARRAY,
            utils::check_table_value::type::string_t,
            0, 0, {}
          }
        }
      },
      {
        "limit_buildings",
        utils::check_table_value::type::array_t,
        0, core::building_type::maximum_limit_buildings,
        {
          {
            ID_ARRAY,
            utils::check_table_value::type::string_t,
            0, 0, {}
          }
        }
      },
      {
        "replaced",
        utils::check_table_value::type::string_t,
        0, 0, {}
      },
      {
        "upgrades_from",
        utils::check_table_value::type::string_t,
        0, 0, {}
      },
      {
        "time",
        utils::check_table_value::type::int_t,
        utils::check_table_value::value_required, 0, {}
      },
      // я задаю тут стринги вместо индексов, зачем? это имеет смысл только в случае с фракциями, но фракционные индексы мы можем задать каким то другим способом
      // я могу с помощью индексов задать все статы в одном месте
      { 
        "modifiers",
        utils::check_table_value::type::array_t,
        0, core::building_type::maximum_stat_modifiers,
        {
          {
            STATS_ARRAY,
            utils::check_table_value::type::int_t,
            0, core::offsets::troop_stats, {}
          }
        }
      },
      {
        "unit_modifiers", // тут нужно указать тип юнита, стат и значение
        utils::check_table_value::type::array_t,
        0, core::building_type::maximum_unit_stat_modifiers, {}
//         {
//           {
//             "unit_type", 
//             utils::check_table_value::type::string_t,
//             utils::check_table_value::value_required, 0, {}
//           },
//           {
//             STAT_ID,
//             utils::check_table_value::type::int_t,
//             core::offsets::troop_stats, core::offsets::troop_stats + core::troop_stats::count, {}
//           },
//           {
//             "value",
//             utils::check_table_value::type::float_t,
//             utils::check_table_value::value_required, 0, {}
//           }
//         }
      },
//       {
//         "hero_modifiers",
//         utils::check_table_value::type::array_t,
//         0, core::building_type::maximum_stat_modifiers,
//         {
//           {
//             "hero_type",
//             utils::check_table_value::type::int_t,
//             utils::check_table_value::value_required, 0, {}
//           },
//           {
//             STAT_ID,
//             utils::check_table_value::type::int_t,
//             core::offsets::hero_stats, core::offsets::hero_stats + core::hero_stats::count, {}
//           },
//           {
//             "value",
//             utils::check_table_value::type::float_t,
//             utils::check_table_value::value_required, 0, {}
//           }
//         }
//       },
      {
        "money_cost",
        utils::check_table_value::type::float_t,
        0, 0, {}
      },
      {
        "authority_cost",
        utils::check_table_value::type::float_t,
        0, 0, {}
      },
      {
        "esteem_cost",
        utils::check_table_value::type::float_t,
        0, 0, {}
      },
      {
        "influence_cost",
        utils::check_table_value::type::float_t,
        0, 0, {}
      },
      {
        "offensive_units",
        utils::check_table_value::type::array_t,
        0, 0, {
          {
            ID_ARRAY,
            utils::check_table_value::type::string_t,
            0, 0, {}
          }
        }
      },
      {
        "defensive_units",
        utils::check_table_value::type::array_t,
        0, 0, {
          {
            ID_ARRAY,
            utils::check_table_value::type::string_t,
            0, 0, {}
          }
        }
      }
    };
    
    size_t add_building(const sol::table &table) {
      //return global::get<map::creator::table_container_t>()->add_table(static_cast<size_t>(core::structure::building_type), table);
      assert(false);
      (void)table;
      return SIZE_MAX;
    }
    
    bool validate_building_type(const size_t &index, const sol::table &table) {
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
    
    bool validate_building_and_save(const size_t &index, sol::this_state lua, const sol::table &table, utils::world_serializator*) {
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
    
    void parse_building(core::building_type* building_type, const sol::table &table) {
      // нужно задать предыдущие здания и это строки с id
      // как из строк получать индекс?
      auto ctx = global::get<core::context>();
      
//       building_type->id = table.get<std::string>("id");
      
      {
        building_type->name_id = table.get<std::string>("name_id");
      }
      
      if (const auto proxy = table["description_id"]; proxy.valid() && proxy.get_type() == sol::type::string) {
        building_type->description_id = proxy.get<std::string>();
      }
      
      if (auto proxy = table["prerequisites"]; proxy.valid()) {
        size_t current_index = 0;
        const auto &table = proxy.get<sol::table>();
        for (const auto &pair : table) {
          if (!pair.second.is<std::string>()) continue;
          
          auto view = pair.second.as<std::string_view>();
          if (view == building_type->id) throw std::runtime_error("Prerequisite building can not be current building id " + std::string(view));
          auto t = ctx->get_entity<core::building_type>(view);
          if (t == nullptr) throw std::runtime_error("Could not find previous building type id " + std::string(view) + " for building " + building_type->id);
          if (current_index >= core::building_type::maximum_prev_buildings) throw std::runtime_error("Maximum prerequisite buildings is " + std::to_string(core::building_type::maximum_prev_buildings));
          building_type->prev_buildings[current_index] = t;
          ++current_index;
        }
      }
      
      if (auto proxy = table["limit_buildings"]; proxy.valid()) {
        size_t current_index = 0;
        const auto &table = proxy.get<sol::table>();
        for (const auto &pair : table) {
          if (!pair.second.is<std::string>()) continue;
          
          auto view = pair.second.as<std::string_view>();
          if (view == building_type->id) throw std::runtime_error("Limit building can not be current building id " + std::string(view));
          auto t = ctx->get_entity<core::building_type>(view);
          if (t == nullptr) throw std::runtime_error("Could not find limit building type id " + std::string(view) + " for building " + building_type->id);
          if (current_index >= core::building_type::maximum_limit_buildings) throw std::runtime_error("Maximum limit buildings is " + std::to_string(core::building_type::maximum_limit_buildings));
          building_type->limit_buildings[current_index] = t;
          ++current_index;
        }
      }
      
      if (auto proxy = table["replaced"]; proxy.valid() && proxy.get_type() == sol::type::string) {
        auto view = proxy.get<std::string_view>();
        if (view == building_type->id) throw std::runtime_error("Building to replace can not be current building id " + std::string(view));
        auto b = ctx->get_entity<core::building_type>(view);
        if (b == nullptr) throw std::runtime_error("Could not find replaced building type id " + std::string(view) + " for building " + building_type->id);
        building_type->replaced = b;
      }
      
//       if (auto proxy = table["upgrades_to"]; proxy.valid() && proxy.get_type() == sol::type::string) {
//         auto view = proxy.get<std::string_view>();
//         if (view == building_type->id) throw std::runtime_error("Building upgrades_to can not be current building id " + std::string(view));
//         auto b = ctx->get_entity<core::building_type>(view);
//         if (b == nullptr) throw std::runtime_error("Could not find upgrades to building type id " + std::string(view) + " for building " + building_type->id);
//         building_type->upgrades_to = b;
//       }

      if (auto proxy = table["upgrades_from"]; proxy.valid() && proxy.get_type() == sol::type::string) {
        auto view = proxy.get<std::string_view>();
        if (view == building_type->id) throw std::runtime_error("Building upgrades_from can not be current building id " + std::string(view));
        auto b = ctx->get_entity<core::building_type>(view);
        if (b == nullptr) throw std::runtime_error("Could not find upgrades_from building type id " + std::string(view) + " for building " + building_type->id);
        building_type->upgrades_from = b;
      }
      
      {
        const size_t time = table["time"];
        building_type->time = time;
      }
      
      // еще нужно указать стоимость
      // вообще так то это модификатор персонажа (-деньги), поэтому мы вполне можем для тултипа найти этот модификатор
      // в цк2 указывалось напрямую через кост
      
      if (auto proxy = table["money_cost"]; proxy.valid() && proxy.get_type() == sol::type::number) {
        building_type->money_cost = proxy.get<double>();
      }
      
      if (auto proxy = table["authority_cost"]; proxy.valid() && proxy.get_type() == sol::type::number) {
        building_type->authority_cost = proxy.get<double>();
      }
      
      if (auto proxy = table["esteem_cost"]; proxy.valid() && proxy.get_type() == sol::type::number) {
        building_type->esteem_cost = proxy.get<double>();
      }
      
      if (auto proxy = table["influence_cost"]; proxy.valid() && proxy.get_type() == sol::type::number) {
        building_type->influence_cost = proxy.get<double>();
      }
      
      if (auto proxy = table["stats_modifiers"]; proxy.valid()) {
        uint32_t current_stat = 0;
        const auto &table = proxy.get<sol::table>();
        for (const auto &pair : table) {
          if (pair.first.get_type() != sol::type::string) continue;
          if (pair.second.get_type() != sol::type::number) continue;
          
          auto stat = pair.first.as<std::string_view>();
          auto value = pair.second.as<double>();
          
          if (current_stat >= core::building_type::maximum_stat_modifiers) throw std::runtime_error("Maximum building stat modifiers is " + std::to_string(core::building_type::maximum_stat_modifiers));
          
          // статы персонажа? давать только владельцу? или всем по иерархии?
          if (const auto itr = core::character_stats::map.find(stat); itr != core::character_stats::map.end()) {
            const auto stat_id = itr->second;
            const stat_modifier m(core::stat_type::character_stat, stat_id, value);
            building_type->mods[current_stat] = m;
            ++current_stat;
            continue;
          }
          
          // геройские статы? вряд ли
          if (const auto itr = core::hero_stats::map.find(stat); itr != core::hero_stats::map.end()) {
            const auto stat_id = itr->second;
            const stat_modifier m(core::stat_type::hero_stat, stat_id, value);
            building_type->mods[current_stat] = m;
            ++current_stat;
            continue;
          }
          
          if (const auto itr = core::city_stats::map.find(stat); itr != core::city_stats::map.end()) {
            const auto stat_id = itr->second;
            const stat_modifier m(core::stat_type::city_stat, stat_id, value);
            building_type->mods[current_stat] = m;
            ++current_stat;
            continue;
          }
          
          if (const auto itr = core::province_stats::map.find(stat); itr != core::province_stats::map.end()) {
            const auto stat_id = itr->second;
            const stat_modifier m(core::stat_type::province_stat, stat_id, value);
            building_type->mods[current_stat] = m;
            ++current_stat;
            continue;
          }
          
          if (const auto itr = core::army_stats::map.find(stat); itr != core::army_stats::map.end()) {
            const auto stat_id = itr->second;
            const stat_modifier m(core::stat_type::army_stat, stat_id, value);
            building_type->mods[current_stat] = m;
            ++current_stat;
            continue;
          }
          
          throw std::runtime_error("Could not find stat " + std::string(stat));
        }
      }
      
      // бафы для юнитов во всей провинции?
      if (auto proxy = table["unit_modifiers"]; proxy.valid()) {
        uint32_t current_stat = 0;
        const auto &table = proxy.get<sol::table>();
        
        for (const auto &pair : table) {
          if (pair.first.get_type() != sol::type::string) continue;
          if (pair.second.get_type() != sol::type::table) continue;
          
          const auto unit_type = pair.first.as<std::string_view>();
          auto type = ctx->get_entity<core::troop_type>(unit_type);
          if (type == nullptr) throw std::runtime_error("Could not find troop type " + std::string(unit_type));
          
          const auto &table = pair.second.as<sol::table>();
          for (const auto &pair : table) {
            if (pair.first.get_type() != sol::type::string) continue;
            if (pair.second.get_type() != sol::type::number) continue;
            
            const auto stat = pair.first.as<std::string_view>();
            const auto value = pair.second.as<double>();
            
            const auto itr = core::troop_stats::map.find(stat);
            if (itr == core::troop_stats::map.end()) throw std::runtime_error("Could not find troop stat " + std::string(stat));
            if (current_stat >= core::building_type::maximum_unit_stat_modifiers) throw std::runtime_error("Maximum unit stat modifiers count " + std::to_string(core::building_type::maximum_unit_stat_modifiers));
            
            building_type->unit_mods[current_stat].type = type;
            building_type->unit_mods[current_stat].stat = itr->second;
            building_type->unit_mods[current_stat].mod = value;
            ++current_stat;
          }
        }
      }

      if (const auto proxy = table["offensive_units"]; proxy.valid()) {
        uint32_t counter = 0;
        const auto units = proxy.get<sol::table>();
        for (const auto &pair : units) {
//           if (pair.first.get_type() != sol::type::string) continue;
          if (pair.second.get_type() != sol::type::string) continue;
          if (counter >= core::building_type::maximum_offensive_units_provided) 
            throw std::runtime_error("Maximum offensive units that can provide building is " + std::to_string(core::building_type::maximum_offensive_units_provided));
          
          // в таблице записи вида тип = количество
          // почему я хочу указать количество? можно же просто типы перечислить
          // так должно быть попроще обновлять, да и количество не сказать чтобы что то невероятно нужное
          
          const auto type_id = pair.second.as<std::string_view>();
          const auto type = ctx->get_entity<core::troop_type>(type_id);
          if (type == nullptr) throw std::runtime_error("Could not find troop type " + std::string(type_id));
          
          building_type->offensive_units[counter] = type;
          ++counter;
        }
      }
      
      if (const auto proxy = table["defensive_units"]; proxy.valid()) {
        uint32_t counter = 0;
        const auto units = proxy.get<sol::table>();
        for (const auto &pair : units) {
          if (pair.second.get_type() != sol::type::string) continue;
          if (counter >= core::building_type::maximum_defensive_units_provided) 
            throw std::runtime_error("Maximum defensive units that can provide building is " + std::to_string(core::building_type::maximum_defensive_units_provided));
          
          const auto type_id = pair.second.as<std::string_view>();
          const auto type = ctx->get_entity<core::troop_type>(type_id);
          if (type == nullptr) throw std::runtime_error("Could not find troop type " + std::string(type_id));
          
          building_type->defensive_units[counter] = type;
          ++counter;
        }
      }
    }
  }
}

