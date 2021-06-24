#include "modificator.h"

namespace devils_engine {
  namespace core {
    const structure modificator::s_type;
    const size_t modificator::max_stat_modifiers_count;
    modificator::modificator() : 
      name_id(SIZE_MAX), 
      description_id(SIZE_MAX), 
      time(0), 
      icon{GPU_UINT_MAX} 
    {}
  }
}
