#ifndef DEVILS_ENGINE_CORE_DIPLOMACY_H
#define DEVILS_ENGINE_CORE_DIPLOMACY_H

#include <string_view>
#include "parallel_hashmap/phmap.h"

// у меня есть кое какие идеи по продвинутой дипломатии
// прежде всего альянс по династическому браку, нужно указать персонажа
// а если он не один? да хороший вопрос, может ли персонаж стать гарантом нескольких альянсов?
// вряд ли, можно ли добавить тогда персонажей в лист? каких? только тех из другого государства
// что дали гарант? а у другого государства тогда хранятся обратные челики, в этом случае наверное можно
// как сделать тогда остальные соглашения, например фракции, гарантия независимости, 
// политическая/экономическая зависимость? экономическая зависимость это что? только дань?
// политическая зависимость: вассал, один из реалмов - вассал другого государства
// фракции? некоторый набор челиков которые объединяются одной целью, как сделать цель?
// казус белли? возможно, там уже указаны штуки которые произойдут в случае проигрыша
// как задать фракцию лоялистов? да никак король должен заключать соглашения другими способами
// как задать данника? сколько платит данник? фиксированную сумму? скорее всего да
// помимо данника мы можем получить денежную контрибуцию или выплачивать денешную контрибуцию
// перемирие?

#define DIPLOMACY_TYPES_LIST \
  DIPLOMACY_TYPE_FUNC(ally) \
  DIPLOMACY_TYPE_FUNC(war_attacker) \
  DIPLOMACY_TYPE_FUNC(war_defender) \
  DIPLOMACY_TYPE_FUNC(faction_leader) \
  DIPLOMACY_TYPE_FUNC(faction_member) \
  DIPLOMACY_TYPE_FUNC(tributary) \
  DIPLOMACY_TYPE_FUNC(tribute_collector) \
  DIPLOMACY_TYPE_FUNC(truce_2way) \
  DIPLOMACY_TYPE_FUNC(truce_holder) \
  DIPLOMACY_TYPE_FUNC(truce_receiver) \

namespace devils_engine {
  namespace core {
    namespace diplomacy {
      enum values {
#define DIPLOMACY_TYPE_FUNC(name) name,
        DIPLOMACY_TYPES_LIST
#undef DIPLOMACY_TYPE_FUNC
        count
      };
      
      extern const std::string_view names[];
      extern const phmap::flat_hash_map<std::string_view, values> map;
    }
  }
}

#endif
