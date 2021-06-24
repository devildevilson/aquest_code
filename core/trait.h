#ifndef TRAIT_H
#define TRAIT_H

#include "stats.h"
#include "declare_structures.h"

namespace devils_engine {
  namespace core {
    // треит только для персонажа?
    struct trait {
      static const structure s_type = structure::trait;
      static const size_t max_stat_modifiers_count = 16;
      
      struct numeric_attribs {
        uint8_t birth;
        uint8_t inherit_chance;
        uint8_t both_parent_inherit_chance;
        uint8_t dummy; // тут что?
      };
      
      std::string id;
      size_t name_str;
      size_t description_str;
      // каждые сколько нибудь детей получают этот трейт
      // каста? для некоторых религий
      // шанс унаследовать от родителя и шанс унаследовать от обоих родителей
      // религиозная ветвь (по идее лучше бы сделать отдельно религией?)
      // еще неплохо было бы сделать какие нибудь констреинты, то есть можно/нельзя получить этот трейт если есть другой
      struct numeric_attribs numeric_attribs;
      render::image_t icon;
      utils::bit_field_32<1> attribs;
      //stat_bonus bonuses[16]; 
      std::array<stat_modifier, max_stat_modifiers_count> bonuses; // нужно еще сгенерировать описание для этого
      
      // надеюсь можно будет передать функцию в луа (но кажется нельзя ссылаться на inline функцию)
      trait();
      inline bool get_attrib(const uint32_t &index) const { return attribs.get(index); }
      inline void set_attrib(const uint32_t &index, const bool value) { attribs.set(index, value); }
    };
  }
}

#endif
