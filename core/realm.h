#ifndef REALM_H
#define REALM_H

#include <array>
#include "declare_structures.h"
#include "stats.h"
#include "utils/bit_field.h"
#include "utils/realm_mechanics.h"
#include "utils/structures_utils.h"

namespace devils_engine {
  namespace core {
    // эта структура создается однажды для персонажа и удаляется если только наследование происходит к персонажу к оторого уже есть свои владения
    // персонажу которому выдется владение мы должны создать эту структуру, в какой то момент лидер может быть мертв, а наследника еще нет
    // например когда это владение выдается правителем, по идее в этом случае все титулы передаются правителю и структура разрушается
    // то есть эта структура не может существовать без титулов (или может? кочевники? у кочевников наверное будут формальные титулы)
    // возможна еще узурпация элективного государства, по идее как и в случае с наследованием (непосредственно узурпация должна вызвать войну, но это не здесь)
    // по идее тут флаги и модификаторы должны быть еще, эвенты? тоже наверное, но с ними чуть позже тогда
    struct realm : public utils::flags_container, public utils::modificators_container {
      static const structure s_type = structure::realm;
      
      character* leader;    // текущий глава фракции (то есть например король) (его может не быть если это особая фракция)
      character* heir;      // персонаж наследник, к нему отойдут все титулы (это может быть элективный наследник)
      // деньги нужно передать если это личная фракция

      // всякие технические вещи
      // господин и вассалы наследуются
      realm* liege;    // фракция сюзерена
      realm* state;    // фракция государства (либо элективный орган, либо фракция персонажа (то есть указатель на самого себя))
      realm* council;  // фракция парламента (кто входит в парламент? по идее указывается у конкретного персонажа)
      realm* tribunal; // фракция суда (кто входит в суд?)
      
      realm* vassals;
      realm* next_vassal; // персонажи с одним сюзереном
      realm* prev_vassal;
      
      titulus* titles; // если персонаж избран то у персонажа появляются титулы непосредственно государства
      titulus* main_title;
      
      // по идее придворные тоже наследуются
      character* courtiers;
      character* prisoners; // в тюрьму мы сажаем непосредственно персонажей
      // глобальные характеристики
      std::array<stat_container, realm_stats::count> stats; // кому уходят характеристики? по идее если это не личная фракция, то всем
      // статы должны пересчитываться после наследования? я пока не очень понимаю как тут вообще статы будут
      // принятые законы (законы по категориям, причем первые две категории всегда про наследование)
      // права жителей фракции (скорее всего едины с законами) (состоят из прав религии и локальных прав, что то еще?) (более менее сделано)
      utils::bit_field_32<utils::realm_mechanics::bit_container_size> mechanics; // это по сути и будут нашими законами
      // конкретные законы и их группировка задается в структуре
      // механики законов я так понимаю будут раскиданы повсеместно, где в одном месте вряд ли возможно их проработать
      // точнее мы можем вынести основные функции в отдельный файл и там попробовать сгруппировать все что у нас есть
      
      realm();
      
      inline bool get_mechanic(const size_t &index) const { return mechanics.get(index); }
      inline void set_mechanic(const size_t &index, const bool value) { mechanics.set(index, value); }
      
      // нужно выделить 4 состояния: государство независимого персонажа, государство зависимого персонажа, элективное государство и зависимое элективное государство
      inline bool is_independent() const { return liege == nullptr; }
      inline bool is_state() const { return state == this; }      
      inline bool is_council() const { return council == this; }
      inline bool is_tribunal() const { return tribunal == this; }
      inline bool is_self() const { return !is_state() && !is_council() && !is_tribunal(); }
      
      void succession();
      void usurped_by_self();
      void usurped_by_council();
      void create_state();
      void create_council();
      void create_tribunal();
      
      void add_title(titulus* title);
      void remove_title(titulus* title);
      titulus* get_last_title() const;
      void sort_titles(); // титулы нужно сортировать по времени когда они оказались у государства
      
      void add_vassal(realm* vassal);
      void add_vassal_raw(realm* vassal);
      void remove_vassal(realm* vassal);
      realm* get_last_vassal() const;
      
      void add_prisoner(character* prisoner);
      void add_prisoner_raw(character* prisoner);
      void remove_prisoner(character* prisoner);
      character* get_last_prisoner() const;
    };
  }
}

#endif
