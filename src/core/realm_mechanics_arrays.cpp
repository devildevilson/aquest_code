#include "realm_mechanics_arrays.h"

namespace devils_engine {
  namespace core {
    namespace power_rights {
      const std::string_view names[] = {
#define POWER_RIGHT_FUNC(name) #name,
        POWER_RIGHTS_LIST
#undef POWER_RIGHT_FUNC
      };
      
      const phmap::flat_hash_map<std::string_view, values> map = {
#define POWER_RIGHT_FUNC(name) std::make_pair(names[values::name], values::name),
        POWER_RIGHTS_LIST
#undef POWER_RIGHT_FUNC
      };
      
      const size_t array_size = sizeof(names) / sizeof(names[0]);
      static_assert(array_size == values::count);
    }
    
    namespace state_rights {
      const std::string_view names[] = {
#define STATE_RIGHT_FUNC(name) #name,
        STATE_RIGHTS_LIST
#undef STATE_RIGHT_FUNC

#define SECRET_TYPE_FUNC(name) #name"_is_considered_shunned",
        SECRET_TYPES_LIST
#undef SECRET_TYPE_FUNC

#define SECRET_TYPE_FUNC(name) #name"_is_considered_criminal",
        SECRET_TYPES_LIST
#undef SECRET_TYPE_FUNC
      };
      
      const phmap::flat_hash_map<std::string_view, values> map = {
#define STATE_RIGHT_FUNC(name) std::make_pair(names[values::name], values::name),
        STATE_RIGHTS_LIST
#undef STATE_RIGHT_FUNC

#define SECRET_TYPE_FUNC(name) std::make_pair(names[values::name##_is_considered_shunned], values::name##_is_considered_shunned),
        SECRET_TYPES_LIST
#undef SECRET_TYPE_FUNC

#define SECRET_TYPE_FUNC(name) std::make_pair(names[values::name##_is_considered_criminal], values::name##_is_considered_criminal),
        SECRET_TYPES_LIST
#undef SECRET_TYPE_FUNC
      };
      
      const size_t array_size = sizeof(names) / sizeof(names[0]);
      static_assert(array_size == values::count);
    }
    
//     namespace realm_mechanics {
//       const std::string_view names[] = {
// #define REALM_MECHANICS_FUNC(name) #name,
//         REALM_MECHANICS_LIST
// #undef REALM_MECHANICS_FUNC
//       };
//       
//       const phmap::flat_hash_map<std::string_view, values> map = {
// #define REALM_MECHANICS_FUNC(name) std::make_pair(names[values::name], values::name),
//         REALM_MECHANICS_LIST
// #undef REALM_MECHANICS_FUNC
//       };
//       
//       const size_t array_size = sizeof(names) / sizeof(names[0]);
//       static_assert(array_size == values::count);
//     }
    
    namespace religion_mechanics {
      const std::string_view names[] = {
#define RELIGION_MECHANICS_FUNC(name) #name,
        RELIGION_MECHANICS_LIST
#undef RELIGION_MECHANICS_FUNC
      };
      
      const phmap::flat_hash_map<std::string_view, values> map = {
#define RELIGION_MECHANICS_FUNC(name) std::make_pair(names[values::name], values::name),
        RELIGION_MECHANICS_LIST
#undef RELIGION_MECHANICS_FUNC
      };
      
      const size_t array_size = sizeof(names) / sizeof(names[0]);
      static_assert(array_size == values::count);
    }
    
    namespace culture_mechanics {
      const std::string_view names[] = {
#define CULTURE_MECHANICS_FUNC(name) #name,
        CULTURE_MECHANICS_LIST
#undef CULTURE_MECHANICS_FUNC
      };
      
      const phmap::flat_hash_map<std::string_view, values> map = {
#define CULTURE_MECHANICS_FUNC(name) std::make_pair(names[values::name], values::name),
        CULTURE_MECHANICS_LIST
#undef CULTURE_MECHANICS_FUNC
      };
      
      const size_t array_size = sizeof(names) / sizeof(names[0]);
      static_assert(array_size == values::count);
    }
  }
}
