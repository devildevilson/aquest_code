#ifndef DEVILS_ENGINE_CORE_SECRET_TYPES_ARRAY_H
#define DEVILS_ENGINE_CORE_SECRET_TYPES_ARRAY_H

#include <string_view>
#include "parallel_hashmap/phmap.h"
#include "secret_types.h"

namespace devils_engine {
  namespace core {
    namespace secret_types {
      extern const std::string_view names[];
      extern const phmap::flat_hash_map<std::string_view, secret_types::values> map;
    }
  }
}

#endif
