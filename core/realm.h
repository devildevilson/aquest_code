#ifndef REALM_H
#define REALM_H

#include <array>
#include "declare_structures.h"
#include "stats.h"
#include "stat_data.h"
#include "utils/bit_field.h"
#include "utils/realm_mechanics.h"
#include "utils/structures_utils.h"

namespace devils_engine {
  namespace core {
    struct war;
    
    // эта структура создается однажды для персонажа и удаляется если только наследование происходит к персонажу к оторого уже есть свои владения
    // персонажу которому выдется владение мы должны создать эту структуру, в какой то момент лидер может быть мертв, а наследника еще нет
    // например когда это владение выдается правителем, по идее в этом случае все титулы передаются правителю и структура разрушается
    // то есть эта структура не может существовать без титулов (или может? кочевники? у кочевников наверное будут формальные титулы)
    // возможна еще узурпация элективного государства, по идее как и в случае с наследованием (непосредственно узурпация должна вызвать войну, но это не здесь)
    // по идее тут флаги и модификаторы должны быть еще, эвенты? тоже наверное, но с ними чуть позже тогда
    struct realm : public utils::flags_container, public utils::modificators_container {
      struct relation {
        // тут мы должны описать какие то договоренности и видимо скрипты при конце отношений
        // что такое договоренность? например гарантия независимости, или сбор дани
        // или договорной альянс, должно быть время договора, 
        // гарантия независимости дает возможность игроку абузить некоторые тактики, так что тут нужны какие то ограничения
        // ограничения по теху, по стоимости, по приросту, и наконец количественное ограничение
        uint32_t relation_state;
      };
      
      static const structure s_type = structure::realm;
      
      character* leader;    // текущий глава фракции (то есть например король) (его может не быть если это особая фракция)
      character* heir;      // персонаж наследник, к нему отойдут все титулы (это может быть элективный наследник)
      // деньги нужно передать если это личная фракция

      // всякие технические вещи
      // господин и вассалы наследуются
      // эти вещи должны быть по идексу? вообще создаются и удаляются эти вещи не абы как... лан чуть позже посмотрим
      realm* liege;    // реалм сюзерена
      realm* state;    // реалм государства (то есть этот реалм отделен от непосредственно игрока и, например, вассалы напрямую не подчиняются игроку)
      realm* council;  // реалм парламента (кто входит в парламент? по идее указывается у конкретного персонажа)
      realm* tribunal; // реалм суда (кто входит в суд?)
      
      realm* vassals;
      realm* next_vassal; // реалмы с одним сюзереном
      realm* prev_vassal;
      
      titulus* titles; // если персонаж избран то у персонажа появляются титулы непосредственно государства
      titulus* main_title;
      
      // по идее придворные тоже наследуются
      character* courtiers; // придворные как и заключенные могут быть отдельно у государства
      character* prisoners; // в тюрьму мы сажаем непосредственно персонажей
      // глобальные характеристики
      utils::stats_container<realm_stats::values> stats;
      utils::stats_container<realm_stats::values> current_stats;
      utils::stats_container<realm_resources::values> resources; // ресурсы могут быть отдельно у коллективного государства
      // добавятся чисто государственные статы (налоги, войска и далее)
      // статы должны пересчитываться после наследования? я пока не очень понимаю как тут вообще статы будут
      // принятые законы (законы по категориям, причем первые две категории всегда про наследование)
      // права жителей фракции (скорее всего едины с законами) (состоят из прав религии и локальных прав, что то еще?) (более менее сделано)
      utils::bit_field_32<utils::realm_mechanics::bit_container_size> mechanics; // это по сути и будут нашими законами
      // конкретные законы и их группировка задается в структуре
      // механики законов я так понимаю будут раскиданы повсеместно, где в одном месте вряд ли возможно их проработать
      // точнее мы можем вынести основные функции в отдельный файл и там попробовать сгруппировать все что у нас есть
      
      // где то тут мы должны указать дипломатию, то есть текущие войны + текущие договоренности
      phmap::flat_hash_map<size_t, size_t> wars;
      phmap::flat_hash_map<size_t, relation> relations;
      
      realm();
      
      // изменение механник должно сопровождаться некими проверками на валидность условий, 
      // есть небольшое количество механик которые не могут взаимодействовать друг с другом
      inline bool get_mechanic(const size_t &index) const { return mechanics.get(index); }
      inline void set_mechanic(const size_t &index, const bool value) { mechanics.set(index, value); }
      
      // нужно выделить 4 состояния: государство независимого персонажа, государство зависимого персонажа, элективное государство и зависимое элективное государство
      inline bool is_independent() const { return liege == nullptr; }
      inline bool is_state() const { return state == this; }      
      inline bool is_council() const { return council == this; }
      inline bool is_tribunal() const { return tribunal == this; }
      // тут поди добавится указатель на духовенство
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
