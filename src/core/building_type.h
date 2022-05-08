#ifndef DEVILS_ENGINE_CORE_BUILDING_TYPE_H
#define DEVILS_ENGINE_CORE_BUILDING_TYPE_H

#include <array>
#include <cstddef>
#include <string>
#include "stats.h"
#include "stat_data.h"
#include "stat_modifier.h"
#include "declare_structures.h"
#include "utils/sol.h"

// цепочка зданий видимо делается на основе upgrades_from
// нужно ли делать выбор улучшения? с учетом того что игра не очень аркадная
// и скорее всего выбор разных бонусов это либо слишком сильная механика
// либо слишком душная и бесполезная

// как быть с модификаторами у улучшений? аккумулировать их? или игнорировать у предыдущих зданий?
// будет лучше если я просто буду игнорировать предыдущие здания, 
// а в текущем учитывать бонус с предыдущего by design

// нужно ли добавлять скриптовые условия для появления/строительства здания? потом можно добавить

namespace devils_engine {
  namespace utils {
    class world_serializator;
  }
  
  namespace core {
    struct building_type {
      static const structure s_type = structure::building_type;
      static const size_t maximum_prev_buildings = 8;
      static const size_t maximum_limit_buildings = 8;
      static const size_t maximum_stat_modifiers = 16;
      static const size_t maximum_unit_stat_modifiers = 16;
      static const size_t maximum_offensive_units_provided = 32;
      static const size_t maximum_defensive_units_provided = 32;
      
      struct provide_unit {
        const troop_type* troop;
        size_t count;
        
        inline provide_unit() : troop(nullptr), count(0) {}
      };
      
      std::string id;
      std::string name_id;
      std::string description_id;
//       std::vector<utils::functions_container::operation> potential;
//       std::vector<utils::functions_container::operation> conditions;
      // предыдущие постройки (достаточно ли?) (с другой стороны вряд ли имеет смысл делать этот массив больше)
      std::array<const building_type*, maximum_prev_buildings> prev_buildings;
      // нельзя построить если существуют ...
      std::array<const building_type*, maximum_limit_buildings> limit_buildings;
      // то что заменяет (индекс? заменяет предыдущий уровень?)
      const building_type* replaced;
      // улучшение (если построено предыдущее здание, то открывается это)
      const building_type* upgrades_from;
      // улучшение у здания будет скорее всего одно
      //const building_type* upgrades_to; // неудачное решение
      // скорость строительства (ходы)
      // тоже наверное скрипт (вряд ли)
      size_t time;
      // как ии поймет что нужно строить это здание? (ии все же наверное будет работать по функции луа, нет, нужно добавить описание для ии) (вес?)
      // при каких условиях здание будет построено перед началом игры (нужно ли?) (должна быть возможность указать что построено а что нет в генераторе)
      // вижен? (дает ли вижен провинции для игрока построившего это) (это для дополнительных построек вроде торговых постов в цк2)
      // во что конвертируется (?) (нужно ли делать переход от одного типа города к другому?)
      // модификаторы (в цк2 могло дать скорость иследований, в моем случае нужно придумать какой то иной способ)
      std::array<stat_modifier, maximum_stat_modifiers> mods;
      std::array<unit_stat_modifier, maximum_unit_stat_modifiers> unit_mods; // модификаторы героев по типу
      // здания наверное могу давать какие то эвенты с какой то периодичностью
      // юниты
//       std::array<provide_unit, maximum_units_provided> offensive_units;
//       std::array<provide_unit, maximum_units_provided> defensive_units;
      std::array<const troop_type*, maximum_offensive_units_provided> offensive_units;
      std::array<const troop_type*, maximum_defensive_units_provided> defensive_units;
      
      // стоимость постройки (может использовать разные ресурсы) (нужно наверное денюшку вытащить в характеристики персонажа)
      // скрипт или не скрипт? вообще наверное лучше скрипты  (думаю что нужно сократить количество скриптов)
      float money_cost;
      float authority_cost;
      float esteem_cost;
      float influence_cost;
      
      building_type();
    };
    
//     size_t add_building(const sol::table &table);
    bool validate_building_type(const size_t &index, const sol::table &table);
    bool validate_building_and_save(const size_t &index, sol::this_state lua, const sol::table &table, utils::world_serializator* container);
    void parse_building(core::building_type* building_type, const sol::table &table);
  }
}

#endif
