#ifndef CITY_TYPE_H
#define CITY_TYPE_H

#include <array>
#include <string>
#include "declare_structures.h"
#include "stats.h"
#include "stat_data.h"

namespace devils_engine {
  namespace core {
    struct city_type {
      static const structure s_type = structure::city_type;
      static const size_t maximum_buildings = 128;
      std::string id;
      // вообще по идее мы можем здесь не ограничивать количество зданий, нет это нужно для непосредственно города
      std::array<const building_type*, maximum_buildings> buildings;
      // нужно указать что этот город это епископство для механики религии
      // тут я так понимаю может потребоваться несколько флагов
      // дефолтные статы?
      std::array<stat_container, city_stats::count> stats;
      // локализация (название типа)
      size_t name_id;
      size_t desc_id;
      // графика
      // как перейти от тайла к городу? нужно хранить какую то информацию в тайле, мы можем использовать 
      // 1 32bit int для того чтобы хранить индекс биома и индекс того что стоит на тайле
      // 
      render::image_t city_image_top;
      render::image_t city_image_face;
      render::image_t city_icon;
      float scale;
      
      city_type();
    };
  }
}

#endif
