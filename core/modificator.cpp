#include "modificator.h"

namespace devils_engine {
  namespace core {
    const structure modificator::s_type;
    const size_t modificator::max_stat_modifiers_count;
    modificator::modificator() :  
      time(0), 
      icon{GPU_UINT_MAX} 
    {}
  }
}
