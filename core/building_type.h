#ifndef BUILDING_TYPE_H
#define BUILDING_TYPE_H

#include <array>
#include "stats.h"
#include "declare_structures.h"

namespace devils_engine {
  namespace core {
    struct building_type {
      static const structure s_type = structure::building_type;
      static const size_t maximum_prev_buildings = 8;
      static const size_t maximum_limit_buildings = 8;
      static const size_t maximum_stat_modifiers = 16;
      static const size_t maximum_unit_stat_modifiers = 16;
      
      std::string id;
      size_t name_id;
      size_t desc_id;
//       std::vector<utils::functions_container::operation> potential;
//       std::vector<utils::functions_container::operation> conditions;
      // предыдущие постройки (достаточно ли?) (с другой стороны вряд ли имеет смысл ограничивать)
      std::array<const building_type*, maximum_prev_buildings> prev_buildings;
      // нельзя построить если существуют ...
      std::array<const building_type*, maximum_limit_buildings> limit_buildings;
      // то что заменяет (индекс? заменяет предыдущий уровень?)
      const building_type* replaced;
      // улучшение (если построено предыдущее здание, то открывается это)
      const building_type* upgrades_from;
      // стоимость постройки (может использовать разные ресурсы) (нужно наверное денюшку вытащить в характеристики персонажа)
      // скорость строительства (недели)
      size_t time;
      // как ии поймет что нужно строить это здание? (ии все же наверное будет работать по функции луа, нет, нужно добавить описание для ии)
      // при каких условиях здание будет построено перед началом игры (нужно ли?) (должна быть возможность указать что построено а что нет в генераторе)
      // вижен? (дает ли вижен провинции для игрока построившего это) (это для дополнительных построек вроде торговых постов в цк2)
      // во что конвертируется (?) (нужно ли делать переход от одного типа города к другому?)
      // модификаторы (в цк2 могло дать скорость иследований, в моем случае нужно придумать какой то иной способ)
      std::array<stat_modifier, maximum_stat_modifiers> mods;
      std::array<unit_stat_modifier, maximum_unit_stat_modifiers> unit_mods; // модификаторы героев по типу
      // здания наверное могу давать какие то эвенты с какой то периодичностью
      
      float money_cost;
      float authority_cost;
      float esteem_cost;
      float influence_cost;
      
      building_type();
    };
  }
}

#endif
