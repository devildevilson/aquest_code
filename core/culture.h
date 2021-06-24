#ifndef CULTURE_H
#define CULTURE_H

#include <array>
#include "declare_structures.h"
#include "stats.h"
#include "utils/realm_mechanics.h"

namespace devils_engine {
  namespace core {
    // возможно будет представлен только в качестве id
    // нет скорее всего нужно сделать как отдельную сущность
    // эвенты и флаги культур?
    struct culture {
      static const structure s_type = structure::culture;
      static const size_t max_stat_modifiers_count = 16;
      
      std::string id;
      size_t name_id;
      // возможно несколько механик (например использование рек кораблями, как в цк2)
      // в цк2 культуры давали отряд, здание, эвенты, решения (decision), 
      // типы правления (китайская администрация), законы наследования, гендерный законы, тактики
      // культурные имена
      //utils::localization::string_bank* name_bank;
      // патронимы (специальные префиксы или постфиксы)
      // династические префиксы
      // шансы что назовут в честь деда
      // родительская культура
      const culture* parent; // культурная группа?
      // модификаторы
      // модификаторы персонажа
      std::array<stat_modifier, max_stat_modifiers_count> attrib; // модификаторы юнитов?
      utils::bit_field_32<utils::culture_mechanics::bit_container_size> mechanics;
      
      culture();
      inline bool get_mechanic(const size_t &index) const { return mechanics.get(index); }
      inline void set_mechanic(const size_t &index, const bool value) { mechanics.set(index, value); }
    };
  }
}

#endif
