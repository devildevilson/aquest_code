#include "diplomacy.h"

#define MAKE_MAP_PAIR(name) std::make_pair(names[values::name], values::name)

namespace devils_engine {
  namespace core {
    namespace diplomacy {
      const std::string_view names[] = {
#define DIPLOMACY_TYPE_FUNC(name) #name,
        DIPLOMACY_TYPES_LIST
#undef DIPLOMACY_TYPE_FUNC
      };
      
      const phmap::flat_hash_map<std::string_view, values> map = {
#define DIPLOMACY_TYPE_FUNC(name) MAKE_MAP_PAIR(name),
        DIPLOMACY_TYPES_LIST
#undef DIPLOMACY_TYPE_FUNC
      };
      
      static_assert(sizeof(names) / sizeof(names[0]) == count);
    }
    
    namespace relationship {
      const std::string_view names[] = {
#define RELATIONSHIP_TYPE_FUNC(name) #name,
#define GOOD_RELATIONSHIP_TYPE_FUNC(name) RELATIONSHIP_TYPE_FUNC(name)
#define LOVE_RELATIONSHIP_TYPE_FUNC(name) RELATIONSHIP_TYPE_FUNC(name)
#define BAD_RELATIONSHIP_TYPE_FUNC(name)  RELATIONSHIP_TYPE_FUNC(name)
        RELATIONSHIP_TYPES_LIST
#undef RELATIONSHIP_TYPE_FUNC
#undef GOOD_RELATIONSHIP_TYPE_FUNC
#undef LOVE_RELATIONSHIP_TYPE_FUNC
#undef BAD_RELATIONSHIP_TYPE_FUNC
      };
      
      const phmap::flat_hash_map<std::string_view, values> map = {
#define RELATIONSHIP_TYPE_FUNC(name) MAKE_MAP_PAIR(name),
#define GOOD_RELATIONSHIP_TYPE_FUNC(name) RELATIONSHIP_TYPE_FUNC(name)
#define LOVE_RELATIONSHIP_TYPE_FUNC(name) RELATIONSHIP_TYPE_FUNC(name)
#define BAD_RELATIONSHIP_TYPE_FUNC(name)  RELATIONSHIP_TYPE_FUNC(name)
        RELATIONSHIP_TYPES_LIST
#undef RELATIONSHIP_TYPE_FUNC
#undef GOOD_RELATIONSHIP_TYPE_FUNC
#undef LOVE_RELATIONSHIP_TYPE_FUNC
#undef BAD_RELATIONSHIP_TYPE_FUNC
      };
      
      static_assert(sizeof(names) / sizeof(names[0]) == count);
      
      bool is_good(const values val) {
        return 
#define RELATIONSHIP_TYPE_FUNC(name)
#define GOOD_RELATIONSHIP_TYPE_FUNC(name) name == val ||
#define LOVE_RELATIONSHIP_TYPE_FUNC(name)
#define BAD_RELATIONSHIP_TYPE_FUNC(name)
        RELATIONSHIP_TYPES_LIST
#undef RELATIONSHIP_TYPE_FUNC
#undef GOOD_RELATIONSHIP_TYPE_FUNC
#undef LOVE_RELATIONSHIP_TYPE_FUNC
#undef BAD_RELATIONSHIP_TYPE_FUNC
        false;
      }
      
      bool is_love(const values val) {
        return 
#define RELATIONSHIP_TYPE_FUNC(name)
#define GOOD_RELATIONSHIP_TYPE_FUNC(name)
#define LOVE_RELATIONSHIP_TYPE_FUNC(name) name == val ||
#define BAD_RELATIONSHIP_TYPE_FUNC(name)
        RELATIONSHIP_TYPES_LIST
#undef RELATIONSHIP_TYPE_FUNC
#undef GOOD_RELATIONSHIP_TYPE_FUNC
#undef LOVE_RELATIONSHIP_TYPE_FUNC
#undef BAD_RELATIONSHIP_TYPE_FUNC
        false;
      }
      
      bool is_bad(const values val) {
        return 
#define RELATIONSHIP_TYPE_FUNC(name)
#define GOOD_RELATIONSHIP_TYPE_FUNC(name)
#define LOVE_RELATIONSHIP_TYPE_FUNC(name)
#define BAD_RELATIONSHIP_TYPE_FUNC(name) name == val ||
        RELATIONSHIP_TYPES_LIST
#undef RELATIONSHIP_TYPE_FUNC
#undef GOOD_RELATIONSHIP_TYPE_FUNC
#undef LOVE_RELATIONSHIP_TYPE_FUNC
#undef BAD_RELATIONSHIP_TYPE_FUNC
        false;
      }
      
      bool is_neutral(const values val) {
        return 
#define RELATIONSHIP_TYPE_FUNC(name) name == val ||
#define GOOD_RELATIONSHIP_TYPE_FUNC(name)
#define LOVE_RELATIONSHIP_TYPE_FUNC(name)
#define BAD_RELATIONSHIP_TYPE_FUNC(name)
        RELATIONSHIP_TYPES_LIST
#undef RELATIONSHIP_TYPE_FUNC
#undef GOOD_RELATIONSHIP_TYPE_FUNC
#undef LOVE_RELATIONSHIP_TYPE_FUNC
#undef BAD_RELATIONSHIP_TYPE_FUNC
        false;
      }
      
      bool has_good(const utils::bit_field<core::relationship::count> &val) {
        return 
#define RELATIONSHIP_TYPE_FUNC(name) 
#define GOOD_RELATIONSHIP_TYPE_FUNC(name) val.get(name) ||
#define LOVE_RELATIONSHIP_TYPE_FUNC(name)
#define BAD_RELATIONSHIP_TYPE_FUNC(name)
        RELATIONSHIP_TYPES_LIST
#undef RELATIONSHIP_TYPE_FUNC
#undef GOOD_RELATIONSHIP_TYPE_FUNC
#undef LOVE_RELATIONSHIP_TYPE_FUNC
#undef BAD_RELATIONSHIP_TYPE_FUNC
        false;
      }
      
      bool has_love(const utils::bit_field<core::relationship::count> &val) {
        return 
#define RELATIONSHIP_TYPE_FUNC(name) 
#define GOOD_RELATIONSHIP_TYPE_FUNC(name)
#define LOVE_RELATIONSHIP_TYPE_FUNC(name) val.get(name) ||
#define BAD_RELATIONSHIP_TYPE_FUNC(name)
        RELATIONSHIP_TYPES_LIST
#undef RELATIONSHIP_TYPE_FUNC
#undef GOOD_RELATIONSHIP_TYPE_FUNC
#undef LOVE_RELATIONSHIP_TYPE_FUNC
#undef BAD_RELATIONSHIP_TYPE_FUNC
        false;
      }
      
      bool has_bad(const utils::bit_field<core::relationship::count> &val) {
        return 
#define RELATIONSHIP_TYPE_FUNC(name) 
#define GOOD_RELATIONSHIP_TYPE_FUNC(name)
#define LOVE_RELATIONSHIP_TYPE_FUNC(name)
#define BAD_RELATIONSHIP_TYPE_FUNC(name) val.get(name) ||
        RELATIONSHIP_TYPES_LIST
#undef RELATIONSHIP_TYPE_FUNC
#undef GOOD_RELATIONSHIP_TYPE_FUNC
#undef LOVE_RELATIONSHIP_TYPE_FUNC
#undef BAD_RELATIONSHIP_TYPE_FUNC
        false;
      }
      
      bool has_neutral(const utils::bit_field<core::relationship::count> &val) {
        return 
#define RELATIONSHIP_TYPE_FUNC(name) val.get(name) ||
#define GOOD_RELATIONSHIP_TYPE_FUNC(name)
#define LOVE_RELATIONSHIP_TYPE_FUNC(name)
#define BAD_RELATIONSHIP_TYPE_FUNC(name) 
        RELATIONSHIP_TYPES_LIST
#undef RELATIONSHIP_TYPE_FUNC
#undef GOOD_RELATIONSHIP_TYPE_FUNC
#undef LOVE_RELATIONSHIP_TYPE_FUNC
#undef BAD_RELATIONSHIP_TYPE_FUNC
        false;
      }
    }
  }
}
