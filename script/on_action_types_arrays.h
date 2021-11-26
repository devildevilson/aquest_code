#ifndef DEVILS_ENGINE_SCRIPT_ON_ACTION_TYPES_ARRAYS_H
#define DEVILS_ENGINE_SCRIPT_ON_ACTION_TYPES_ARRAYS_H

#include <string_view>
#include "parallel_hashmap/phmap.h"
#include "on_action_types.h"

namespace devils_engine {
  namespace script {
    namespace action_type {
      extern const std::string_view names[];
      extern const phmap::flat_hash_map<std::string_view, values> map;
    }
  }
}

#endif
