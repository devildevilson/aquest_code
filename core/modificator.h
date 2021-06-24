#ifndef MODIFICATOR_H
#define MODIFICATOR_H

#include "declare_structures.h"
#include "stats.h"

namespace devils_engine {
  namespace core {
    // разделить треиты и модификаторы чтобы удалить модификаторы после смерти?
    struct modificator {
      static const structure s_type = structure::modificator;
      static const size_t max_stat_modifiers_count = 16;
      
      std::string id;
      size_t name_id;
      size_t description_id;
      size_t time;
      render::image_t icon;
      utils::bit_field_32<1> attribs;
      //stat_bonus bonuses[16]; // нужно еще сгенерировать описание для этого
      std::array<stat_modifier, max_stat_modifiers_count> bonuses; // нужно еще сгенерировать описание для этого
      // бонус к отношениям
      
      // надеюсь можно будет передать функцию в луа (но кажется нельзя ссылаться на inline функцию)
      modificator();
      inline bool get_attrib(const uint32_t &index) const { return attribs.get(index); }
      inline void set_attrib(const uint32_t &index, const bool value) { attribs.set(index, value); }
    };
  }
}

#endif
