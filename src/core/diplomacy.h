#ifndef DEVILS_ENGINE_CORE_DIPLOMACY_H
#define DEVILS_ENGINE_CORE_DIPLOMACY_H

#include <string_view>
#include "parallel_hashmap/phmap.h"
#include "utils/bit_field.h"

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
  /*DIPLOMACY_TYPE_FUNC(truce_2way)*/ \
  DIPLOMACY_TYPE_FUNC(truce_holder) \
  DIPLOMACY_TYPE_FUNC(truce_receiver) \

#define RELATIONSHIP_TYPES_LIST \
  GOOD_RELATIONSHIP_TYPE_FUNC(potential_friend) \
  GOOD_RELATIONSHIP_TYPE_FUNC(mate) \
  GOOD_RELATIONSHIP_TYPE_FUNC(best_friend) \
  GOOD_RELATIONSHIP_TYPE_FUNC(soldier_friend) \
  LOVE_RELATIONSHIP_TYPE_FUNC(potential_lover) \
  LOVE_RELATIONSHIP_TYPE_FUNC(crush) \
  LOVE_RELATIONSHIP_TYPE_FUNC(lover) \
  LOVE_RELATIONSHIP_TYPE_FUNC(soulmate) \
  BAD_RELATIONSHIP_TYPE_FUNC(potential_rival) \
  BAD_RELATIONSHIP_TYPE_FUNC(oaf) \
  BAD_RELATIONSHIP_TYPE_FUNC(rival) \
  BAD_RELATIONSHIP_TYPE_FUNC(nemesis) \
  BAD_RELATIONSHIP_TYPE_FUNC(bully) \
  /*RELATIONSHIP_TYPE_FUNC(victim)*/ \
  RELATIONSHIP_TYPE_FUNC(ward) \
  RELATIONSHIP_TYPE_FUNC(mentor) \
  RELATIONSHIP_TYPE_FUNC(student) \
  RELATIONSHIP_TYPE_FUNC(court_physician) \
  RELATIONSHIP_TYPE_FUNC(guardian) \
  RELATIONSHIP_TYPE_FUNC(intrigue_mentor) \
  RELATIONSHIP_TYPE_FUNC(intrigue_student) \
  
  
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
    
    namespace relationship {
      enum values {
#define RELATIONSHIP_TYPE_FUNC(name) name,
#define GOOD_RELATIONSHIP_TYPE_FUNC(name) RELATIONSHIP_TYPE_FUNC(name)
#define LOVE_RELATIONSHIP_TYPE_FUNC(name) RELATIONSHIP_TYPE_FUNC(name)
#define BAD_RELATIONSHIP_TYPE_FUNC(name)  RELATIONSHIP_TYPE_FUNC(name)
        RELATIONSHIP_TYPES_LIST
#undef RELATIONSHIP_TYPE_FUNC
#undef GOOD_RELATIONSHIP_TYPE_FUNC
#undef LOVE_RELATIONSHIP_TYPE_FUNC
#undef BAD_RELATIONSHIP_TYPE_FUNC
        count
      };
      
      extern const std::string_view names[];
      extern const phmap::flat_hash_map<std::string_view, values> map;
      
      bool is_good(const values val);
      bool is_love(const values val);
      bool is_bad(const values val);
      bool is_neutral(const values val);
      
      bool has_good(const utils::bit_field<core::relationship::count> &val);
      bool has_love(const utils::bit_field<core::relationship::count> &val);
      bool has_bad(const utils::bit_field<core::relationship::count> &val);
      bool has_neutral(const utils::bit_field<core::relationship::count> &val);
    }
  }
}

#endif
