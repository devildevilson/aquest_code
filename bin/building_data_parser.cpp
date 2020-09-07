#include "data_parser.h"
#include "utils/globals.h"
#include "core_structures.h"
#include "utils/table_container.h"
#include "utils/string_container.h"
#include "core_context.h"

namespace devils_engine {
  namespace utils {
    size_t add_building(const sol::table &table) {
      return global::get<utils::table_container>()->add_table(core::structure::building_type, table);
    }
    
    bool validate_building(const sol::table &table) {
      return true;
    }
    
    bool validate_building_and_save(sol::this_state lua, const sol::table &table) {
      return true;
    }
    
    void parse_building(core::building_type* building_type, const sol::table &table) {
      // нужно задать предыдущие здания и это строки с id
      // как из строк получать индекс?
      auto to_data = global::get<utils::data_string_container>();
      auto ctx = global::get<core::context>();
      
      building_type->id = table.get<std::string>("id");
      
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
          auto value = (*itr).second.as<float>();
          
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
          auto value = (*itr).second.as<float>();
          
          if (auto val = magic_enum::enum_cast<core::province_stats::values>(stat); val.has_value()) {
            building_type->mods[current_stat].type = core::unit_type::province;
            building_type->mods[current_stat].stat = val.value();
            building_type->mods[current_stat].mod.fval = value;
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
          auto value = (*itr).second.as<float>();
          
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
            auto value = (*itr).second.as<float>();
            
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
            auto value = (*itr).second.as<float>();
            
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
