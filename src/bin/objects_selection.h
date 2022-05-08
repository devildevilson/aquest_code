#ifndef OBJECTS_SELECTION_H
#define OBJECTS_SELECTION_H

#include <cstddef>
#include <cstdint>
#include <array>
#include "utils/sol.h"

// как добиться здесь уникальности выделения?
// если работать с луа, то тут удобнее сделать сол объекты с указателями

namespace devils_engine {
  namespace core {
    struct army;
    struct hero_troop;
  }
  
  namespace utils {
    struct interface_container;
    
    struct objects_selection {
      static const size_t maximum_selected_objects = 512;
      
      // работаем с 4-мя типами: армия, герой, город, структура
      //interface_container* interface;
      size_t count;
      std::array<sol::object, maximum_selected_objects> objects;
      
      objects_selection(); // interface_container* interface
      ~objects_selection();
      
      int64_t has(const sol::object &obj) const;
      sol::object get(const int64_t &index) const;
      int64_t add(const sol::object &obj);
      int64_t raw_add(const sol::object &obj);
      bool remove(const int64_t &index);
      void clear();
      
      void sort(const sol::function &predicate);
      
      bool has_army() const;
      bool has_hero() const;
      bool has_city() const;
      bool has_structure() const;
      bool has_unit() const;
      bool has_building() const;
      
      void copy(objects_selection* primary);
      void copy_armies(objects_selection* primary);
      void copy_heroes(objects_selection* primary);
      void copy_cities(objects_selection* primary);
      void copy_structures(objects_selection* primary);
      void copy_units(objects_selection* primary);
      void copy_buildings(objects_selection* primary);
      
      void add(objects_selection* primary);
      void add_armies(objects_selection* primary);
      void add_heroes(objects_selection* primary);
      void add_cities(objects_selection* primary);
      void add_structures(objects_selection* primary);
      void add_units(objects_selection* primary);
      void add_buildings(objects_selection* primary);
      
      void remove(objects_selection* primary);
      void remove_armies(objects_selection* primary);
      void remove_heroes(objects_selection* primary);
      void remove_cities(objects_selection* primary);
      void remove_structures(objects_selection* primary);
      void remove_units(objects_selection* primary);
      void remove_buildings(objects_selection* primary);
    };
  }
}

#endif
