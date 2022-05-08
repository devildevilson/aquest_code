#ifndef MODIFICATOR_H
#define MODIFICATOR_H

#include <string>
#include <array>
#include "declare_structures.h"
#include "stats.h"
#include "stat_modifier.h"
#include "traits_modifier_attribs.h"
#include "utils/bit_field.h"
#include "render/shared_structures.h"

namespace devils_engine {
  namespace core {
    // разделить треиты и модификаторы чтобы удалить модификаторы после смерти?
    // модификаторы поди не имеют бонусов по умолчанию, хотя может и имеют
    struct modificator {
      static const structure s_type = structure::modificator;
      static const size_t max_stat_modifiers_count = 16;
      static const size_t max_opinion_modifiers_count = 16;
      
      std::string id;
      std::string name_id;
      std::string description_id;
      size_t type;
      size_t constraint;
      size_t time;
      render::image_t icon;
      utils::bit_field_32<core::modificator_attributes::count> attribs;
      std::array<stat_modifier, max_stat_modifiers_count> bonuses;
      std::array<opinion_modifier, max_opinion_modifiers_count> opinion_mods;
      
      modificator();
      inline bool get_attrib(const uint32_t &index) const { return attribs.get(index); }
      inline bool set_attrib(const uint32_t &index, const bool value) { return attribs.set(index, value); }
    };
  }
}

#endif
