#ifndef DEVILS_ENGINE_CORE_BIOME_H
#define DEVILS_ENGINE_CORE_BIOME_H

#include <cstdint>
#include <cstddef>
#include <string>
#include "declare_structures.h"
#include "render/shared_structures.h"
#include "utils/constexpr_funcs.h"
#include "utils/bit_field.h"
#include "stat_modifier.h"
#include "parallel_hashmap/phmap.h"
#include "utils/sol.h"

#define BIOME_ATTRIBUTES_LIST \
  BIOME_ATTRIBUTE_FUNC(not_passable) \
  BIOME_ATTRIBUTE_FUNC(water) \
  BIOME_ATTRIBUTE_FUNC(winter) \
  BIOME_ATTRIBUTE_FUNC(ford) /* брод (для визуала) */ \
  BIOME_ATTRIBUTE_FUNC(bridge) /* мост через реку */ \
  /* потом еще нужно придумать механики, хотя возможно это будет в основном визуал */ \
  BIOME_ATTRIBUTE_FUNC(river) \
  BIOME_ATTRIBUTE_FUNC(lava) \
  BIOME_ATTRIBUTE_FUNC(magic) \
  BIOME_ATTRIBUTE_FUNC(wasteland) \
  BIOME_ATTRIBUTE_FUNC(corrupted) \
  BIOME_ATTRIBUTE_FUNC(dead) \
  BIOME_ATTRIBUTE_FUNC(volcanic) \
  BIOME_ATTRIBUTE_FUNC(vortex)
  
// надо бы аттрибутов добавить побольше

namespace devils_engine {
  namespace utils {
    class world_serializator;
  }
  
  namespace core {
    struct biome {
      static const structure s_type = structure::biome;
      
      enum class attributes {
#define BIOME_ATTRIBUTE_FUNC(val) val,
        BIOME_ATTRIBUTES_LIST
#undef BIOME_ATTRIBUTE_FUNC
          
        count
      };
      
      static const size_t bit_field_size = ceil(double(attributes::count) / double(UINT32_WIDTH));
      static const size_t maximum_stat_modifiers = 16;
      static const size_t maximum_unit_stat_modifiers = 16;
      
      std::string id;
      std::string name_id;
      std::string description_id;
      // характеристики: базовая скорость, проходимость, вода (брод?), что то еще? возможно является ли этот биом летним?
      // данные для рендера тоже тут наверное продублируем
      // не очень много, но давно напрашивался
      utils::bit_field_32<bit_field_size> attribs;
      float base_speed; // целое число? скорее всего процент по отношению к базе как в героях
      // в героях есть еще магический терраин, бои на котором дают бонусы к использованию магии
      // например равнины магии дают возмжность кастить любые спеллы на экспертном уровне
      // проклятые равнины - только скиллы первого уровня
      // также есть земли которые дают мораль и удачу
      // возможно нужно добавить здесь список модификаторов
      
      std::array<stat_modifier, maximum_stat_modifiers> mods;
      std::array<unit_stat_modifier, maximum_unit_stat_modifiers> unit_mods; // модификаторы героев по типу
      
      render::biome_data_t data;
      
      biome();
      
      inline bool get_attribute(const uint32_t &index) const { return attribs.get(index); }
      inline bool get_attribute(const attributes &attr) const { return attribs.get(static_cast<uint32_t>(attr)); }
    };
    
    extern const std::string_view biome_attributes_names[];
    extern const phmap::flat_hash_map<std::string_view, biome::attributes> biome_attributes_map;
    
    bool validate_biome(const size_t &index, const sol::table &table);
    bool validate_biome_and_save(const size_t &index, sol::this_state lua, const sol::table &table, utils::world_serializator*);
    void parse_biome(core::biome* biome, const sol::table &table);
  }
}

#endif
