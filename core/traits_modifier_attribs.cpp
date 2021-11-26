#include "traits_modifier_attribs.h"

#define MAKE_MAP_PAIR(name) std::make_pair(names[values::name], values::name)

namespace devils_engine {
  namespace core {
    namespace trait_attributes {
      const std::string_view names[] = {
#define TRAIT_ATTRIBUTE_FUNC(val) #val,
        TRAIT_ATTRIBUTES_LIST
#undef TRAIT_ATTRIBUTE_FUNC
      };
      
      const phmap::flat_hash_map<std::string_view, values> map = {
#define TRAIT_ATTRIBUTE_FUNC(val) MAKE_MAP_PAIR(val),
        TRAIT_ATTRIBUTES_LIST
#undef TRAIT_ATTRIBUTE_FUNC
      };
      
      static_assert(sizeof(names) / sizeof(names[0]) == values::count);
    }
    
    namespace modificator_attributes {
      const std::string_view names[] = {
#define MODIFICATOR_ATTRIBUTE_FUNC(val) #val,
        MODIFICATOR_ATTRIBUTES_LIST
#undef MODIFICATOR_ATTRIBUTE_FUNC
      };
      
      const phmap::flat_hash_map<std::string_view, values> map = {
#define MODIFICATOR_ATTRIBUTE_FUNC(val) MAKE_MAP_PAIR(val),
        MODIFICATOR_ATTRIBUTES_LIST
#undef MODIFICATOR_ATTRIBUTE_FUNC
      };
      
      static_assert(sizeof(names) / sizeof(names[0]) == values::count);
    }
  }
}
