#ifndef TROOP_TYPE_H
#define TROOP_TYPE_H

#include <string>
#include <cstddef>
#include <cstdint>
#include "declare_structures.h"
#include "stats.h"

namespace devils_engine {
  namespace core {
    struct troop_type {
      static const structure s_type = structure::troop_type;
      std::string id;
      size_t name_id;
      size_t description_id;
      // сильные и слабые стороны?
      std::array<stat_container, troop_stats::count> stats;
      // нужны иконки и отображение в бою (2д графика с 8 гранями)
      render::image_t card; // карточка для отображения в контейнере интерфейса армии
      // отображение в бою можно сделать через состояния
      // скилы? было бы неплохо что нибудь такое добавить
      
      troop_type();
    };
  }
}

#endif
