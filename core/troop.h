#ifndef TROOP_H
#define TROOP_H

#include "declare_structures.h"
#include "stats.h"
#include "stat_data.h"
#include "utils/structures_utils.h"

namespace devils_engine {
  namespace core {
    // отряды поди должны отдельно существовать (?), мы либо их в городе держим как подкреп
    // либо нанимаем, на мой взгляд и в том и в другом случае нужен указатель
    struct troop {
      static const core::structure s_type = core::structure::troop;
      
      const troop_type* type;
      struct character* character; // это может быть отряд полководца
      // характеристики (здоровье, количество в отряде, атака, урон и прочее)
//       std::array<stat_container, troop_stats::count> moded_stats; // название нужно сменить
//       std::array<stat_container, troop_stats::count> current_stats; //  изменяются только в бою?
      utils::stats_container<troop_stats::values> stats;
      utils::stats_container<troop_stats::values> current_stats;
      
      troop();
    };
  }
}

#endif
