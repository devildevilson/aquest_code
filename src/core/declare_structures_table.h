#ifndef DEVILS_ENGINE_CORE_DECLARE_STRUCTURES_TABLE_H
#define DEVILS_ENGINE_CORE_DECLARE_STRUCTURES_TABLE_H

#include <string_view>
#include "parallel_hashmap/phmap.h"
#include "declare_structures.h"

namespace devils_engine {
  namespace core {
    namespace structure_data {
      extern const std::string_view names[];
      extern const phmap::flat_hash_map<std::string_view, structure> map;
    }
  }
}

#endif
