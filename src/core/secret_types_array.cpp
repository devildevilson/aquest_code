#include "secret_types_array.h"

namespace devils_engine {
  namespace core {
    namespace secret_types {
      const std::string_view names[] = {
#define SECRET_TYPE_FUNC(val) #val,
        SECRET_TYPES_LIST
#undef SECRET_TYPE_FUNC
      };
      
      const phmap::flat_hash_map<std::string_view, secret_types::values> map = {
#define SECRET_TYPE_FUNC(val) std::make_pair(names[values::val], values::val),
        SECRET_TYPES_LIST
#undef SECRET_TYPE_FUNC
      };
      
      static_assert(sizeof(names) / sizeof(names[0]) == count);
    }
  }
}
