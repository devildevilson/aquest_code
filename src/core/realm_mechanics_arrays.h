#ifndef REALM_MECHANICS_ARRAYS_H
#define REALM_MECHANICS_ARRAYS_H

#include <string_view>
#include "realm_mechanics.h"
#include "parallel_hashmap/phmap.h"

namespace devils_engine {
  namespace core {
    namespace power_rights {
      extern const std::string_view names[];
      extern const phmap::flat_hash_map<std::string_view, values> map;
    }
    
    namespace state_rights {
      extern const std::string_view names[];
      extern const phmap::flat_hash_map<std::string_view, values> map;
    }
    
//     namespace realm_mechanics {
//       extern const std::string_view names[];
//       extern const phmap::flat_hash_map<std::string_view, values> map;
//     }
    
    namespace religion_mechanics {
      extern const std::string_view names[];
      extern const phmap::flat_hash_map<std::string_view, values> map;
    }
    
    namespace culture_mechanics {
      extern const std::string_view names[];
      extern const phmap::flat_hash_map<std::string_view, values> map;
    }
  }
}

#endif
