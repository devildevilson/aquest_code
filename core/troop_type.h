#ifndef DEVILS_ENGINE_CORE_TROOP_TYPE_H
#define DEVILS_ENGINE_CORE_TROOP_TYPE_H

#include <string>
#include <cstddef>
#include <cstdint>
#include <array>
#include "declare_structures.h"
#include "stats.h"
#include "utils/structures_utils.h"
#include "render/shared_structures.h"

// в тотал воре у отрядов есть: название, описание (иконки к описанию), сильные/слабые стороны (или дополнительное описание),
// количество людей в отряде (иконка), количество убийств отрядом (иконка), опыт, основные статы, иконки состояния
// есть еще тип формирования (то есть тяжелая пехота, например), на что он влияет кроме описания? можно ли тип формирования задать в enum?
// такое чувство что толком ни на что, то есть особенности формирования все равно задаются в типе разными пассивками,
// характеристиками и проч

namespace devils_engine {
  namespace core {
    struct formation_type {
      std::string id;
      std::string name_id;
      std::string description_id;
      render::image_t icon;
    };
    
    struct troop_type {
      static const structure s_type = structure::troop_type;
      
      std::string id;
      std::string name_id;
      std::string description_id;
      const formation_type* formation;
      // сильные и слабые стороны?
      utils::stats_container<troop_stats::values> stats;
      // нужны иконки и отображение в бою (2д графика с 8 гранями)
      render::image_t card; // карточка для отображения в контейнере интерфейса армии
      // отображение в бою можно сделать через состояния
      // скилы? было бы неплохо что нибудь такое добавить
      // отображение в битве по большому счету нужно только в битве
      // здесь хранить совсем не обязательно
      
      troop_type();
    };
  }
}

#endif
