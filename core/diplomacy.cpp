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
  }
}
