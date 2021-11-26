#ifndef DEVILS_ENGINE_CORE_TITULUS_H
#define DEVILS_ENGINE_CORE_TITULUS_H

#include "declare_structures.h"
#include "utils/structures_utils.h"
#include "render/shared_structures.h"
#include "utils/list.h"
#include "utils/sol.h"
#include "utils/handle.h"

namespace devils_engine {
  namespace utils {
    class world_serializator;
  }
  
  namespace core {
    struct titulus : 
      public utils::flags_container, 
      public utils::events_container, 
      public utils::ring::list<titulus, utils::list_type::titles>,
      public utils::ring::list<titulus, utils::list_type::sibling_titles>
    {
      static const structure s_type = structure::titulus;
      static const size_t events_container_size = 15;
      static const size_t flags_container_size = 25;
      static const size_t heraldy_container_size = 32;
      
      enum class type { // скорее всего нужно добавить еще тип владения конкретным городом (для возможности давать этот титул родственникам например)
        city,  // в цк2 существует титул города
        // в цк2 титул города и баронский титул - это разные вещи, но кажется они каким то образом зависимы друг от друга
        // в цк2 титул столицы провинции == баронский титул
        baron,
        duke,
        king,
        imperial,
        blessing,
        
        count,
        
        top_type = blessing
      };
      
      // титул может быть формальным или реальным
      // у титулов существует иерархия
      // имперский титул, королевский титул, герцогский титул, титул барона
      // барон - самый нижний титул, остальные составляются из баронов, герцогств и королевств
      // еще нужно учесть что существует де-юре и де-факто владение титулами
      // де-юре - это земли в массиве childs, а де-факто - это владения персонажа для которого этот титул является определяющим
      // если титул не является главным, то де-факто - все земли у владельца титула
      
      std::string id;
      enum type t;
      struct city* city;         // титул города
      struct province* province; // баронский титул
      titulus* parent; // если мы создем титул, то может ли у текущего быть два титула верхнего уровня?
      titulus* children; // если нет детей и это не город, то это специальный титул
      utils::handle<realm> owner;  // у титула может быть только один владелец (фракция титулы наследует и передает)
      uint64_t static_state;
      std::string name_id;
      std::string description_id;
      std::string adjective_id;
//       titulus* next;
//       titulus* prev;
      render::color_t main_color;    // цвет будет использоваться на глобальной карте для страны
      render::color_t border_color1; // думаю двух цветов достаточно, нужно посмотреть как сделано в цк2
      render::color_t border_color2;
      uint32_t heraldy_layers_count;
      std::array<uint32_t, heraldy_container_size> heraldy_container;
      
      // думаю что в титулах маленькие контейнеры потребуются
      //events_container<events_container_size> events; // должно хватить
      //flags_container<flags_container_size> flags; // флагов довольно много
//       phmap::flat_hash_map<const event*, event_container> events;
//       phmap::flat_hash_map<std::string_view, size_t> flags;
      
      titulus();
      titulus(const enum type &t);
      ~titulus();
      enum type type() const;
      bool is_formal() const;
      
      std::string_view full_name() const;
      std::string_view base_name() const;
      std::string_view ruler_title() const;
      std::string_view adjective() const;
      std::string_view form_of_address() const; 
    };
    
//     size_t add_title(const sol::table &table);
    bool validate_title(const size_t &index, const sol::table &table);
    bool validate_title_and_save(const size_t &index, sol::this_state lua, const sol::table &table, utils::world_serializator* container);
    void parse_title(core::titulus* title, const sol::table &table);
  }
}

#endif
