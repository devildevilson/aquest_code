#ifndef DEVILS_ENGINE_CORE_TRAIT_H
#define DEVILS_ENGINE_CORE_TRAIT_H

#include <string>
#include "stats.h"
#include "stat_modifier.h"
#include "declare_structures.h"
#include "traits_modifier_attribs.h"
#include "utils/bit_field.h"
#include "render/shared_structures.h"

// можно еще указать противоположный треит тип мужественный/трусливый
// группы трейтов можно сделать как религиозные группы
// другое дело что треит без группы имеет нейтральное отношение к другим треитам
// должна быть наверное особая механика для образования чем просто треит
// треиты поди могут иметь уровень? могут, как сделать аккуратно?

namespace devils_engine {
  namespace core {
    struct trait_group {
      std::string id;
      std::string name_id;
      std::string description_id;
      
      // отношение во флоатах? может все таки инт? тут скорее вопрос как обозначить отсутствие какого значение? можно использовать NAN
      // нужно ли добавлять общие опинион модификаторы?
      float same_group_opinion;
      float different_group_opinion;
      float opposite_opinion;
      float nogroup_opinion; // нужно ли?
      
      trait_group();
    };
    
    // треит только для персонажа? треиты и модификаторы должны быть одним и тем же
    struct trait {
      static const structure s_type = structure::trait;
      static const size_t max_stat_modifiers_count = 16;
      static const size_t max_opinion_modifiers_count = 16;
      
      // шанс унаследовать от родителя и шанс унаследовать от обоих родителей
      struct numeric_attribs {
        float birth;
        float inherit_chance;
        float both_parent_inherit_chance;
        float dummy; // тут что?
      };
      
      std::string id;
      std::string name_id;
      std::string description_id;
      // каждые сколько нибудь детей получают этот трейт
      // каста? для некоторых религий
      // религиозная ветвь (по идее лучше бы сделать отдельно религией?) (верование?)
      // еще неплохо было бы сделать какие нибудь констреинты, то есть можно/нельзя получить этот трейт если есть другой
      // 64 типа, 0 игнорируем проверки типов, что делаем если пытаемся добавить по плохому констреинту?
      // мы что то должны заменять, не уверен
      size_t type;
      size_t constraint;
      
      const trait* opposite; // мужественный/трусливый
      const trait_group* group;
      
      struct numeric_attribs numeric_attribs;
      render::image_t icon;
      utils::bit_field_32<core::trait_attributes::bit_container_size> attribs;
      std::array<stat_modifier, max_stat_modifiers_count> bonuses;
      std::array<opinion_modifier, max_opinion_modifiers_count> opinion_mods;
      
      float same_group_opinion;
      float different_group_opinion;
      float opposite_opinion;
      float nogroup_opinion; // нужно ли?
      
      trait();
      inline bool get_attrib(const uint32_t &index) const { return attribs.get(index); }
      inline bool set_attrib(const uint32_t &index, const bool value) { return attribs.set(index, value); }
      
      float get_same_group_opinion() const;
      float get_different_group_opinion() const;
      float get_opposite_opinion() const;
      float get_nogroup_opinion() const;
    };
  }
}

#endif
