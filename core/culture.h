#ifndef DEVILS_ENGINE_CORE_CULTURE_H
#define DEVILS_ENGINE_CORE_CULTURE_H

#include <string>
#include <array>
#include "declare_structures.h"
#include "stats.h"
#include "stat_modifier.h"
#include "realm_mechanics.h"
#include "utils/bit_field.h"
#include "utils/structures_utils.h"
#include "utils/list.h"
#include "utils/sol.h"

namespace devils_engine {
  namespace core {
    // как задать отношения между культурами? так же как и отношения между религиями?
    struct culture_opinion_data {
      float character_opinion;
      float popular_opinion;
    };
    
    struct culture_group {
      static const structure s_type = structure::culture_group;
      
      std::string id;
      std::string name_id;
      std::string description_id;
      // в цк еще указывались mercenary_names (id и coat of arms id)
      // культурные группы нужны чтобы немного модифицировать отношение населения
      culture_opinion_data different_groups;
      culture_opinion_data different_cultures;
      culture_opinion_data different_child_cultures;
      
      render::image_t image; // ???
      render::color_t color;
      
      // имеет смысл сюда добавить все культуры одной группы
    };
    
    // сюда нужно добавить касты как в индуизме, было бы неплохо связать как нибудь религию и культуру, 
    // чтобы некоторые культурные особенности отражались и в религии
    struct culture : public utils::flags_container, public utils::events_container, public utils::ring::list<culture, utils::list_type::sibling_cultures> {
      static const structure s_type = structure::culture;
      static const size_t max_stat_modifiers_count = 16;
      
      std::string id;
      std::string name_id;
      std::string description_id;
      // возможно несколько механик (например использование рек кораблями, как в цк2)
      // в цк2 культуры давали отряд, здание, эвенты, решения (decision), 
      // типы правления (китайская администрация), законы наследования, гендерный законы, тактики
      // культурные имена
      std::string names_table_id;
      // патронимы (специальные префиксы или постфиксы, по идее отчества тоже являются патронимами)
      std::string patronims_table_id;
      std::string additional_table_id;
      // тип есть несколько способов по которым строится имя например суффиксы как в русском или добавления специального слова "ибн"
      // как правильно это учесть? нужно задать пару флагов для этого
      // династические префиксы
      float grandparent_name_chance;
      // родительская культура
      culture_group* group;
      culture* parent; // культурная группа?
      culture* children; // дочерняя культура по аналогии с религией?
      render::image_t image; // ???
      render::color_t color;
      std::array<stat_modifier, max_stat_modifiers_count> bonuses; // модификаторы юнитов?
      utils::bit_field_32<core::culture_mechanics::count> mechanics;
      
      culture_opinion_data different_groups;
      culture_opinion_data different_cultures;
      culture_opinion_data different_child_cultures;
      
      mutable core::character* members;
      
      culture();
      inline bool get_mechanic(const size_t &index) const { return mechanics.get(index); }
      inline void set_mechanic(const size_t &index, const bool value) { mechanics.set(index, value); }
      
      void add_member(core::character* member) const;
      void remove_member(core::character* member) const;
      
      core::culture_group* get_culture_group() const;
    };
    
    size_t add_culture_group(const sol::table &table);
    bool validate_culture_group(const size_t &index, const sol::table &table);
    void parse_culture_group(core::culture_group* culture_group, const sol::table &table);
    
    size_t add_culture(const sol::table &table);
    bool validate_culture(const size_t &index, const sol::table &table);
    void parse_culture(core::culture* culture, const sol::table &table);
  }
}

#endif
