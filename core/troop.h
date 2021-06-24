#ifndef TROOP_H
#define TROOP_H

#include "declare_structures.h"
#include "stats.h"

namespace devils_engine {
  namespace core {
    // отряды поди должны отдельно существовать (?), мы либо их в городе держим как подкреп
    // либо нанимаем, на мой взгляд и в том и в другом случае нужен указатель
    struct troop {
      const troop_type* type;
      struct character* character; // это может быть отряд полководца
      // характеристики (здоровье, количество в отряде, атака, урон и прочее)
      std::array<stat_container, troop_stats::count> moded_stats; // название нужно сменить
      std::array<stat_container, troop_stats::count> current_stats; //  изменяются только в бою?
      
      troop();
    };
  }
}

#endif
