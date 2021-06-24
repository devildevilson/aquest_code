#include "trait.h"

namespace devils_engine {
  namespace core {
    const structure trait::s_type;
    const size_t trait::max_stat_modifiers_count;
    trait::trait() : 
      name_str(SIZE_MAX), 
      description_str(SIZE_MAX), 
      numeric_attribs{0,0,0,0}, 
      icon{GPU_UINT_MAX} 
    {}
  }
}
