#ifndef DEVILS_ENGINE_CORE_CITY_TYPE_H
#define DEVILS_ENGINE_CORE_CITY_TYPE_H

#include <array>
#include <string>
#include "declare_structures.h"
#include "stats.h"
#include "stat_data.h"
#include "utils/sol.h"
#include "utils/structures_utils.h"

namespace devils_engine {
  namespace utils {
    class world_serializator;
  }
  
  namespace core {
    struct holding_type {
      static const structure s_type = structure::holding_type;
      
      std::string id;
      std::string name_id;
      std::string description_id;
    };
    
    struct city_type {
      static const structure s_type = structure::city_type;
      static const size_t maximum_buildings = 128;
      std::string id;
      std::string name_id;
      std::string description_id;
      
      const holding_type* holding;
      // вообще по идее мы можем здесь не ограничивать количество зданий, нет это нужно для непосредственно города
      // возможно нужно добавить построенные здания по умолчанию, причем имеет смысл наверное указать и в типе и в городе
      size_t buildings_count;
      std::array<const building_type*, maximum_buildings> buildings;
      size_t default_buildings_count;
      std::array<uint32_t, maximum_buildings> default_buildings; // пара-тройка зданий будет существовать по дефолту в новых городах
      // нужно указать что этот город это епископство для механики религии
      // тут я так понимаю может потребоваться несколько флагов
      // дефолтные статы?
      utils::stats_container<city_stats::values> stats;
      
      // графика
      // как перейти от тайла к городу? нужно хранить какую то информацию в тайле, мы можем использовать 
      // 1 32bit int для того чтобы хранить индекс биома и индекс того что стоит на тайле
      // 
      render::image_t city_image_top;
      render::image_t city_image_face;
      // картинка руин? эвент для руин?
      render::image_t city_icon;
      float scale;
      
      city_type();
      
      size_t find_building(const building_type* b) const;
      size_t find_building_upgrade(const building_type* b, const size_t &start = 0) const;
    };
    
    bool validate_city_type(const size_t &index, const sol::table &table);
    bool validate_city_type_and_save(const size_t &index, sol::this_state lua, const sol::table &table, utils::world_serializator* container);
    void parse_city_type(core::city_type* city_type, const sol::table &table);
  }
}

#endif
