#ifndef DEVILS_ENGINE_CORE_TROOP_H
#define DEVILS_ENGINE_CORE_TROOP_H

#include "declare_structures.h"
#include "stats.h"
#include "stat_data.h"
#include "utils/structures_utils.h"
#include "utils/list.h"
#include "utils/handle.h"

namespace devils_engine {
  namespace core {
    // отряды поди должны отдельно существовать (?), мы либо их в городе держим как подкреп
    // либо нанимаем, на мой взгляд и в том и в другом случае нужен указатель
    // в городе должен быть список отрядов, состояние отрядов может измениться когда армия не в городе
    // эта структура должна быть просто контейнером, который я буду менять по ситуации
    // что делать для героев/полководцев? добавить отряд персонажу? скорее всего
    // не всем нужен этот контейнер
    struct troop : public utils::ring::list<troop, utils::list_type::army_troops> { // , public utils::ring::list<core::troop, utils::list_type::city_troops>
      static const core::structure s_type = core::structure::troop;
      
      size_t object_token;
      
      const troop_type* type;
      const city* origin;
      const building_type* provider;
      utils::handle<army> formation;
      struct character* character; // это может быть отряд полководца
      // характеристики (здоровье, количество в отряде, атака, урон и прочее)
      utils::stats_container<troop_stats::values> stats; // так ли это необходимо? у меня же в типе записаны значения по умолчанию
      utils::stats_container<troop_stats::values> current_stats;
      // добавятся ресурсы: хп, мораль (хотя в тотал варе мораль иначе расчитывалась), ???
      
      troop();
      ~troop();
    };
  }
}

#endif
