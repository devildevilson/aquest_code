#ifndef RELIGION_H
#define RELIGION_H

#include <string>
#include "render/shared_structures.h"
#include "utils/realm_mechanics.h"
#include "utils/bit_field.h"
#include "declare_structures.h"

namespace devils_engine {
  namespace core {  
    struct religion_group {
      static const structure s_type = structure::religion_group;
      
    };
    
    // могут быть как сгенерироваными так и нет
    // эвенты и флаги религий?
    struct religion {
      static const structure s_type = structure::religion;
      std::string id;
      size_t name_str;
      // описание (?) (возможно частично должно быть сгенерировано например на основе прав)
      size_t description_str;
      const religion_group* group;
      const religion* parent;
      const religion* reformed;
      // с кем можно жениться?
      // сколько жен или наложниц? (нужно ли?)
      // мультипликатор к короткому правлению
      // агрессивность ии
      double aggression;
      // элемент одежды главы (головной убор и сама одежда)
      // сколько ресурсов дает за отсутствие войны
      // название священного похода
      size_t crusade_str;
      // название священного текста
      size_t scripture_str;
      // имена богов и злых богов
      // имя главного бога
      size_t high_god_str;
      // название благочестия
      size_t piety_str;
      // титул священников
      size_t priest_title_str;
      uint32_t opinion_stat_index; // все персонажи этой религии, нужно ли делать отношения отдельных групп персонажей
      render::image_t image;
      // цвет?
      // права (ограничения механик последователей данной религии)
      utils::bit_field_32<utils::religion_mechanics::bit_container_size> mechanics;
      
      // модификаторы к персонажу и юнитам
      // модификаторы если на своей земле
      
      religion();
      inline bool get_mechanic(const size_t &index) const { return mechanics.get(index); }
      inline void set_mechanic(const size_t &index, const bool value) { mechanics.set(index, value); }
    };
  }
}

#endif
