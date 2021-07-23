#ifndef PROVINCE_H
#define PROVINCE_H

#include <vector>
#include <array>
#include "stats.h"
#include "stat_data.h"
#include "declare_structures.h"
#include "utils/structures_utils.h"

namespace devils_engine {
  namespace core {
    // если персонаж владеет титулом баронским этой провинции - он владеет и столицей провинции? (как в цк2)
    // скорее всего да
    struct province : public utils::flags_container, public utils::modificators_container, public utils::events_container {
      static const structure s_type = structure::province;
      static const size_t modificators_container_size = 10;
      static const size_t events_container_size = 15;
      static const size_t flags_container_size = 25;
      static const size_t cities_max_game_count = 10;
      // static attrib* attributes;
      
      // название (или название по столице провинции?)
      // удобно и герб тоже по столице провинции дать (нет, титул для баронства существует)
      titulus* title; // не будет меняться, но нужно заполнить титул (или его заполнять в другом месте?)
      std::vector<uint32_t> tiles;
      std::vector<uint32_t> neighbours;
      // какие техи распространились в этой провинции
      // что то вроде std::vector<bool> ?
      // локальные характеристики (принадлежность провинции?)
      // несколько "городов" (могут быть и замки и епископства, больше типов?) (как минимум должны быть епископства)
      // какой максимум поселений? 7 в цк2, у меня скорее всего меньше (3-4?)
      uint32_t cities_max_count; // в зависимости от размера провинции
      uint32_t cities_count;
      std::array<city*, cities_max_game_count> cities; // тут мы должны заранее указать тайл для будущего города
      
//       std::array<stat_container, province_stats::count> stats;
//       std::array<stat_container, province_stats::count> current_stats;
      utils::stats_container<province_stats::values> stats;
      utils::stats_container<province_stats::values> current_stats;
      
      province();
    };
  }
}

#endif
