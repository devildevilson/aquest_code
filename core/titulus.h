#ifndef TITULUS_H
#define TITULUS_H

#include "declare_structures.h"
#include "utils/structures_utils.h"
#include "render/shared_structures.h"

namespace devils_engine {
  namespace core {
    struct titulus : public utils::flags_container, public utils::events_container {
      static const structure s_type = structure::titulus;
      static const size_t events_container_size = 15;
      static const size_t flags_container_size = 25;
      
      enum class type { // скорее всего нужно добавить еще тип владения конкретным городом (для возможности давать этот титул родственникам например)
        city,  // в цк2 существует титул города
        baron, // в цк2 титул города и баронский титул - это разные вещи, но кажется они каким то образом зависимы друг от друга
        duke,
        king,
        imperial,
//         special,
        
        count
      };
      
      // титул может быть формальным или реальным
      // у титулов существует иерархия
      // имперский титул, королевский титул, герцогский титул, титул барона
      // барон - самый нижний титул, остальные составляются из баронов, герцогств и королевств
      // еще нужно учесть что существует де-юре и де-факто владение титулами
      // де-юре - это земли в массиве childs, а де-факто - это владения персонажа для которого этот титул является определяющим
      // если титул не является главным, то де-факто - все земли у владельца титула
      
      std::string id;
      enum type type;
      uint32_t count; // если 0 то это специальный титул
      union {
        titulus** childs; // реальный титул обладает определенным набором титулов нижнего уровня
        titulus* child;
        struct city* city;         // титул города
        struct province* province; // баронский титул
//         struct {
//           uint32_t provinces[2]; // либо это баронский титул
//         };
      };
      titulus* parent; // если мы создем титул, то может ли у текущего быть два титула верхнего уровня?
      realm* owner;  // у титула может быть только один владелец (фракция титулы наследует и передает)
      size_t name_str;
      size_t description_str;
      size_t adjective_str;
      titulus* next;
      titulus* prev;
      render::color_t main_color;    // цвет будет использоваться на глобальной карте для страны
      render::color_t border_color1; // думаю двух цветов достаточно, нужно посмотреть как сделано в цк2
      render::color_t border_color2;
      uint32_t heraldy;
      
      // думаю что в титулах маленькие контейнеры потребуются
      //events_container<events_container_size> events; // должно хватить
      //flags_container<flags_container_size> flags; // флагов довольно много
//       phmap::flat_hash_map<const event*, event_container> events;
//       phmap::flat_hash_map<std::string_view, size_t> flags;
      
      titulus();
      titulus(const enum type &t);
      titulus(const enum type &t, const uint32_t &count);
      ~titulus();
      bool is_formal() const;
      void set_child(const uint32_t &index, titulus* child);
      titulus* get_child(const uint32_t &index) const;
      void set_province(struct province* province);
      struct province* get_province() const;
      void set_city(struct city* city);
      struct city* get_city() const;
      
      void create_children(const uint32_t &count);
      
      std::string_view full_name() const;
      std::string_view base_name() const;
      std::string_view ruler_title() const;
      std::string_view adjective() const;
      std::string_view form_of_address() const; 
    };
  }
}

#endif
