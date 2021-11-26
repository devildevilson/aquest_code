#include "on_action_types_arrays.h"

#define MAKE_MAP_PAIR(name) std::make_pair(names[values::name], values::name)

namespace devils_engine {
  namespace script {
    namespace action_type {
      const std::string_view names[] = {
#define ACTION_TYPE_FUNC(name) #name,
        ACTION_TYPES_LIST
#undef ACTION_TYPE_FUNC
      };
      
      const phmap::flat_hash_map<std::string_view, values> map = {
#define ACTION_TYPE_FUNC(name) MAKE_MAP_PAIR(name),
        ACTION_TYPES_LIST
#undef ACTION_TYPE_FUNC
      };
      
      static_assert(sizeof(names) / sizeof(names[0]) == count);
    }
  }
}
