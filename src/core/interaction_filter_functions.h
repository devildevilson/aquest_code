#ifndef DEVILS_ENGINE_CORE_INTERACTION_FILTER_FUNCTION_H
#define DEVILS_ENGINE_CORE_INTERACTION_FILTER_FUNCTION_H

#include <functional>
#include <string>
#include "declare_structures.h"
#include "utils/handle.h"
#include "parallel_hashmap/phmap.h"

// тут несколько типов потребуется
// как тут сделать?
// можем ли мы вообще обратиться к реалму с помощью интеракции? не уверен

#define CHARACTERS_FILTERS_LIST \
  CHARACTERS_FILTER_FUNC(known_secrets) \
  CHARACTERS_FILTER_FUNC(hooked_characters) \
  CHARACTERS_FILTER_FUNC(neighboring_rulers) \
  CHARACTERS_FILTER_FUNC(peer_vassals) \
  CHARACTERS_FILTER_FUNC(guests) \
  CHARACTERS_FILTER_FUNC(dynasty) \
  CHARACTERS_FILTER_FUNC(courtiers) \
  CHARACTERS_FILTER_FUNC(prisoners) \
  CHARACTERS_FILTER_FUNC(sub_realm_characters) \
  CHARACTERS_FILTER_FUNC(realm_characters) \
  CHARACTERS_FILTER_FUNC(vassals) \
  CHARACTERS_FILTER_FUNC(liege) \
  CHARACTERS_FILTER_FUNC(self) \
  CHARACTERS_FILTER_FUNC(head_of_faith) \
  CHARACTERS_FILTER_FUNC(spouses) \
  CHARACTERS_FILTER_FUNC(family) \
  CHARACTERS_FILTER_FUNC(children) \
  CHARACTERS_FILTER_FUNC(primary_war_enemies) \
  CHARACTERS_FILTER_FUNC(war_enemies) \
  CHARACTERS_FILTER_FUNC(war_allies) \
  CHARACTERS_FILTER_FUNC(scripted_relations) \
  
#define TITLES_FILTERS_LIST \
  TITLES_FILTER_FUNC(directly_owned_titles) \
  TITLES_FILTER_FUNC(accessible_titles) \
  
#define CITIES_FILTERS_LIST \
  CITIES_FILTER_FUNC(directly_controlled_cities) \
  CITIES_FILTER_FUNC(accessible_cities) \
  CITIES_FILTER_FUNC(cities_in_realm) \
  
#define PROVINCES_FILTERS_LIST \
  PROVINCES_FILTER_FUNC(directly_controlled_provinces) \
  PROVINCES_FILTER_FUNC(accessible_provinces) \
  PROVINCES_FILTER_FUNC(provinces_in_realm) \
  
namespace devils_engine {
  namespace core {
    namespace character_filters {
      typedef bool (*basic_func)(const core::character*, const core::character*);
      typedef bool (*iterator_func)(const core::character*, const std::function<bool(const core::character*, const core::character*)> &);
      
      enum values {
#define CHARACTERS_FILTER_FUNC(name) name,
        CHARACTERS_FILTERS_LIST
#undef CHARACTERS_FILTER_FUNC
        
        count
      };
      
      extern const std::string_view names[];
      extern const phmap::flat_hash_map<std::string_view, values> map;
      extern const basic_func basics[];
      extern const iterator_func iterators[];
    }
    
    namespace title_filters {
      typedef bool (*basic_func)(const core::character*, const core::titulus*);
      typedef bool (*iterator_func)(const core::character*, const std::function<bool(const core::character*, const core::titulus*)> &);
      
      enum values {
#define TITLES_FILTER_FUNC(name) name,
        TITLES_FILTERS_LIST
#undef TITLES_FILTER_FUNC
        
        count
      };
      
      extern const std::string_view names[];
      extern const phmap::flat_hash_map<std::string_view, values> map;
      extern const basic_func basics[];
      extern const iterator_func iterators[];
    }
    
    namespace city_filters {
      typedef bool (*basic_func)(const core::character*, const core::city*);
      typedef bool (*iterator_func)(const core::character*, const std::function<bool(const core::character*, const core::city*)> &);
      
      enum values {
#define CITIES_FILTER_FUNC(name) name,
        CITIES_FILTERS_LIST
#undef CITIES_FILTER_FUNC
        
        count
      };
      
      extern const std::string_view names[];
      extern const phmap::flat_hash_map<std::string_view, values> map;
      extern const basic_func basics[];
      extern const iterator_func iterators[];
    }
    
    namespace province_filters {
      typedef bool (*basic_func)(const core::character*, const core::province*);
      typedef bool (*iterator_func)(const core::character*, const std::function<bool(const core::character*, const core::province*)> &);
      
      enum values {
#define PROVINCES_FILTER_FUNC(name) name,
        PROVINCES_FILTERS_LIST
#undef PROVINCES_FILTER_FUNC
        
        count
      };
      
      extern const std::string_view names[];
      extern const phmap::flat_hash_map<std::string_view, values> map;
      extern const basic_func basics[];
      extern const iterator_func iterators[];
    }
    
    namespace army_filters {
      typedef bool (*basic_func)(const core::character*, const utils::handle<core::army>);
      typedef bool (*iterator_func)(const core::character*, const std::function<bool(const core::character*, const utils::handle<core::army>)> &);
      
      enum values {
        
        count
      };
      
      extern const std::string_view names[];
      extern const phmap::flat_hash_map<std::string_view, values> map;
      extern const basic_func basics[];
      extern const iterator_func iterators[];
    }
    
    namespace hero_troop_filters {
      typedef bool (*basic_func)(const core::character*, const utils::handle<core::hero_troop>);
      typedef bool (*iterator_func)(const core::character*, const std::function<bool(const core::character*, const utils::handle<core::hero_troop>)> &);
      
      enum values {
        
        count
      };
      
      extern const std::string_view names[];
      extern const phmap::flat_hash_map<std::string_view, values> map;
      extern const basic_func basics[];
      extern const iterator_func iterators[];
    }
    
#define DECLARE_BASIC_FILTER_FUNC(name) bool name##_basic(const core::character*, const core::character*);
#define DECLARE_ITERATOR_FILTER_FUNC(name) bool name##_iterator(const core::character*, const std::function<bool(const core::character*, const core::character*)> &);
    
#define CHARACTERS_FILTER_FUNC(name) DECLARE_BASIC_FILTER_FUNC(name) DECLARE_ITERATOR_FILTER_FUNC(name)
    CHARACTERS_FILTERS_LIST
#undef CHARACTERS_FILTER_FUNC

#undef DECLARE_BASIC_FILTER_FUNC
#undef DECLARE_ITERATOR_FILTER_FUNC

#define DECLARE_BASIC_FILTER_FUNC(name) bool name##_basic(const core::character*, const core::titulus*);
#define DECLARE_ITERATOR_FILTER_FUNC(name) bool name##_iterator(const core::character*, const std::function<bool(const core::character*, const core::titulus*)> &);

#define TITLES_FILTER_FUNC(name) DECLARE_BASIC_FILTER_FUNC(name) DECLARE_ITERATOR_FILTER_FUNC(name)
    TITLES_FILTERS_LIST
#undef TITLES_FILTER_FUNC

#undef DECLARE_BASIC_FILTER_FUNC
#undef DECLARE_ITERATOR_FILTER_FUNC

#define DECLARE_BASIC_FILTER_FUNC(name) bool name##_basic(const core::character*, const core::city*);
#define DECLARE_ITERATOR_FILTER_FUNC(name) bool name##_iterator(const core::character*, const std::function<bool(const core::character*, const core::city*)> &);

#define CITIES_FILTER_FUNC(name) DECLARE_BASIC_FILTER_FUNC(name) DECLARE_ITERATOR_FILTER_FUNC(name)
    CITIES_FILTERS_LIST
#undef CITIES_FILTER_FUNC

#undef DECLARE_BASIC_FILTER_FUNC
#undef DECLARE_ITERATOR_FILTER_FUNC

#define DECLARE_BASIC_FILTER_FUNC(name) bool name##_basic(const core::character*, const core::province*);
#define DECLARE_ITERATOR_FILTER_FUNC(name) bool name##_iterator(const core::character*, const std::function<bool(const core::character*, const core::province*)> &);

#define PROVINCES_FILTER_FUNC(name) DECLARE_BASIC_FILTER_FUNC(name) DECLARE_ITERATOR_FILTER_FUNC(name)
    PROVINCES_FILTERS_LIST
#undef PROVINCES_FILTER_FUNC

#undef DECLARE_BASIC_FILTER_FUNC
#undef DECLARE_ITERATOR_FILTER_FUNC
  }
}

// это для ИИ, к этому наверн добавится еще парочка
// known_secrets - Characters who has a secret you know of
// hooked_characters
// neighboring_rulers
// peer_vassals
// guests
// dynasty
// courtiers
// prisoners
// sub_realm_characters
// realm_characters
// vassals
// liege
// self
// head_of_faith
// spouses
// family
// children
// primary_war_enemies
// war_enemies
// war_allies
// scripted_relations - Any character you have a scripted relation with

// что насчет таргетов типа городов или армий? не особо понятно, 
// но кажется в цк3 ни с кем кроме персонажа нельзя взаимодействовать
// но явно нужно сделать фильры что выше + добавить к ним довольно много других
// нужно ли добавлять фильтр среди всех персонажей или титулов (или прочих)?
// 

#endif
