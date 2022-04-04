#ifndef DEVILS_ENGINE_CORE_REALM_H
#define DEVILS_ENGINE_CORE_REALM_H

#include <array>
#include "declare_structures.h"
#include "stats.h"
#include "stat_data.h"
#include "realm_mechanics.h"
#include "utils/bit_field.h"
#include "utils/structures_utils.h"
#include "utils/list.h"
#include "utils/handle.h"
#include "diplomacy.h"
#include "script/get_scope_commands_macro.h"
#include "script/condition_commands_macro.h"

namespace devils_engine {
  namespace core {
    struct war;
    
    // эта структура создается однажды для персонажа и удаляется если только наследование происходит к персонажу к оторого уже есть свои владения
    // персонажу которому выдется владение мы должны создать эту структуру, в какой то момент лидер может быть мертв, а наследника еще нет
    // например когда это владение выдается правителем, по идее в этом случае все титулы передаются правителю и структура разрушается
    // то есть эта структура не может существовать без титулов (или может? кочевники? у кочевников наверное будут формальные титулы)
    // возможна еще узурпация элективного государства, по идее как и в случае с наследованием (непосредственно узурпация должна вызвать войну, но это не здесь)
    // по идее тут флаги и модификаторы должны быть еще, эвенты? тоже наверное, но с ними чуть позже тогда
    
    // нужно добавить методы для обхода мемберов и электората, нужно совместить мапу с войной с мапой с отношениями и добавить туда дольше данных
    struct realm : 
      public utils::flags_container, 
      public utils::modificators_container, 
      public utils::events_container, // должны ли быть эвенты у целого государства? скорее да чем нет
      public utils::ring::list<realm, utils::list_type::vassals> 
    {
      // как обойти эту структуру? можно создать для этого несколько методов типа every_war, every_ally, every_dynasty_ally
      // какие типы: война, союз, данник, протекция, было бы неплохо создать какую нибудь сложную но понятную систему дипломатии
      // может быть экономическая (некий модификатор на доход или на товары, добавится позже видимо) или политическая (один из реалмов - часть другого государства) зависимость
      // + к этому могут быть некоторые гарантии (например независимость (хотя это просто династический брак? а может и нет)), гарантия экономической помощи? коалиция?
//       struct relation {
//         // тут мы должны описать какие то договоренности и видимо скрипты при конце отношений
//         // что такое договоренность? например гарантия независимости, или сбор дани
//         // или договорной альянс, должно быть время договора, 
//         // гарантия независимости дает возможность игроку абузить некоторые тактики, так что тут нужны какие то ограничения
//         // ограничения по теху, по стоимости, по приросту, и наконец количественное ограничение
//         // тут должна быть полная информация о взаимоотношении: война, сторона в войне, альянс, тип альянса, связанные персонажи
//         // по этой информации нам нужно прекращать союз при определенных условиях например если умирает друг с которым мы подписали соглашение
//         // или если распадается династический брак (или он становится неважен (при каких условиях?))
//         uint32_t relation_type;
//         utils::handle<core::war> war;
//         utils::handle<core::realm> related_realm;
//         character* related_characters;
//       };
      
      static const structure s_type = structure::realm;
      
      size_t object_token;
      
      character* leader;    // текущий глава фракции (то есть например король) (его может не быть если это особая фракция)
      character* heir;      // персонаж наследник, к нему отойдут все титулы (это может быть элективный наследник)
      // деньги нужно передать если это личная фракция

      // всякие технические вещи
      // господин и вассалы наследуются
      // эти вещи должны быть по идексу? вообще создаются и удаляются эти вещи не абы как... лан чуть позже посмотрим
      // это по сути отдельные государственные институты, они в общем то одинаковые по механикам
      // у них будут прописаны их права, в этом случае мы смотрим что может институт а что нет в mechanics у каждого
      // вместо того чтобы прописывать все в одном месте, остается вопрос меж институциональном взаимодействии
      // + права определенных групп населения, думаю что институтов должно быть все же конечное количество с некими условными
      // заранее прописынными названиями каждого института, то есть например связка государство, совет, трибунал, религия
      // и еще наверное дополнительный, государство, совет и трибунал фактичеки имеют почти одинаковые механики
      // но должно быть четкое разделение на авторитарный орган правления, коллективный, безинициативный, представительский
      // (то есть орган правления в который входят только определенные челики, например религия)
      utils::handle<core::realm> liege;     // реалм сюзерена
      
      // у меня может быть ситуация когда стейт - это не личный реалм короля, а например парламент
      // стейт может не являться личным реалмом, и может быть одним из основных
      utils::handle<core::realm> state;     // реалм государства (то есть этот реалм отделен от непосредственно игрока и, например, вассалы напрямую не подчиняются игроку)
      utils::handle<core::realm> council;   // реалм парламента (кто входит в парламент? по идее указывается у конкретного персонажа)
      utils::handle<core::realm> tribunal;  // реалм суда (кто входит в суд?)
      utils::handle<core::realm> assembly;  // прямая демократия? как ее сделать в рамках моей игры? придворные? вообще да, как представители всего народа пусть будут придворные
      utils::handle<core::realm> clergy;    // религиозный институт, наполняем только священниками, может быть связан с реалмом из другого государства в зависимости от чтого что за религия
      
      //utils::handle<core::realm> abroad; // для того чтобы задать ситуацию как с папством то можно просто указать liege
      
      religion* dominant_religion;
      
      // вассалам хендл добавить сложно
      realm* vassals;
      
      titulus* titles; // если персонаж избран то у персонажа появляются титулы непосредственно государства
      titulus* main_title;
      city* capital;
      
      // по идее придворные тоже наследуются
      character* courtiers; // придворные как и заключенные могут быть отдельно у государства
      character* prisoners; // в тюрьму мы сажаем непосредственно персонажей
      character* members;
      character* electors;
      // глобальные характеристики
      utils::stats_container<realm_stats::values> stats;
      utils::stats_container<realm_stats::values> current_stats;
      utils::stats_container<realm_resources::values> resources; // ресурсы могут быть отдельно у коллективного государства
      // добавятся чисто государственные статы (налоги, войска и далее)
      // статы должны пересчитываться после наследования? я пока не очень понимаю как тут вообще статы будут
      // принятые законы (законы по категориям, причем первые две категории всегда про наследование)
      // права жителей фракции (скорее всего едины с законами) (состоят из прав религии и локальных прав, что то еще?) (более менее сделано)
      utils::bit_field_32<power_rights::count> power_rights;
      utils::bit_field_32<state_rights::count> state_rights; // имеет смысл если is_state()
      // конкретные законы и их группировка задается в структуре
      // механики законов я так понимаю будут раскиданы повсеместно, где в одном месте вряд ли возможно их проработать
      // точнее мы можем вынести основные функции в отдельный файл и там попробовать сгруппировать все что у нас есть
      
      // где то тут мы должны указать дипломатию, то есть текущие войны + текущие договоренности
      // скорее всего дипломатия уйдет в персонажа
//       phmap::flat_hash_map<utils::handle<core::realm>, utils::handle<core::war>> wars;
//       phmap::flat_hash_map<utils::handle<core::realm>, relation> relations;
      
      realm();
      ~realm();
      
      realm(const realm &copy) = delete;
      realm(realm &&move) = delete;
      realm & operator=(const realm &copy) = delete;
      realm & operator=(realm &&move) = delete;
      
      // изменение механник должно сопровождаться некими проверками на валидность условий, 
      // есть небольшое количество механик которые не могут взаимодействовать друг с другом
      inline bool get_power_mechanic(const size_t &index) const noexcept { return power_rights.get(index); }
      inline bool get_state_mechanic(const size_t &index) const noexcept { return state_rights.get(index); }
      inline bool set_power_mechanic(const size_t &index, const bool value) noexcept { return power_rights.set(index, value); }
      inline bool set_state_mechanic(const size_t &index, const bool value) noexcept { return state_rights.set(index, value); }
      
      // нужно выделить 4 состояния: государство независимого персонажа, государство зависимого персонажа, элективное государство и зависимое элективное государство
//       inline bool is_independent() const noexcept { return liege == nullptr; }
//       inline bool is_state() const noexcept { return state == this; }
//       inline bool is_council() const noexcept { return council == this; }
//       inline bool is_tribunal() const noexcept { return tribunal == this; }
//       inline bool is_assembly() const noexcept { return assembly == this; }
//       inline bool is_clergy() const noexcept { return clergy == this; }
//       inline bool is_state_independent_power() const noexcept { return is_state() && !is_council() && !is_tribunal() && !is_assembly() && !is_clergy(); }
      // собственный реалм может быть стейтом? когда страной владеет некий монарх, кто в таком случае является мембером? глава и наследник?
      // видимо да, возможно придется немного законы поменять, нужен другой способ определить что это собственный реалм
      //inline bool is_self() const noexcept { return !is_council() && !is_tribunal() && !is_assembly() && !is_clergy(); } // !is_state() &&
      // определяется как leader->self == this, если стейт избираемый то это будет другой реалм, стейт может еще являться одним из сил в государстве
//       bool is_self() const noexcept;
      
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
      bool has_landed_title() const;
      
      void add_vassal(realm* vassal);
      void add_vassal_raw(realm* vassal);
      void remove_vassal(realm* vassal);
      realm* get_last_vassal() const;
      
      void add_prisoner(character* prisoner);
      void add_prisoner_raw(character* prisoner);
      void remove_prisoner(character* prisoner);
      character* get_last_prisoner() const;
      
      void add_courtier(character* courtier);
      void add_courtier_raw(character* courtier);
      void remove_courtier(character* courtier);
      character* get_last_courtier() const;
      
      void set_capital(city* c);
      
      bool include(character* c) const;
      
#define GET_SCOPE_COMMAND_FUNC(name, a, b, type) type get_##name() const;
      REALM_GET_SCOPE_COMMANDS_LIST
#undef GET_SCOPE_COMMAND_FUNC

#define CONDITION_COMMAND_FUNC(name) bool name() const noexcept;
      REALM_GET_BOOL_NO_ARGS_COMMANDS_LIST
#undef CONDITION_COMMAND_FUNC

#define CONDITION_ARG_COMMAND_FUNC(name, unused1, unused2, type) bool name(const type &data) const noexcept;
      REALM_GET_BOOL_EXISTED_ARG_COMMANDS_LIST
#undef CONDITION_ARG_COMMAND_FUNC
    };
  }
}

#endif
