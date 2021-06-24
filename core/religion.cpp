#include "religion.h"

namespace devils_engine {
  namespace core {
    const structure religion_group::s_type;
    const structure religion::s_type;
    religion::religion() : 
      name_str(SIZE_MAX), 
      description_str(SIZE_MAX), 
      group(nullptr), 
      parent(nullptr), 
      reformed(nullptr), 
      aggression(0.0f), 
      crusade_str(SIZE_MAX),  
      scripture_str(SIZE_MAX),
      high_god_str(SIZE_MAX),
      piety_str(SIZE_MAX),
      priest_title_str(SIZE_MAX),
      opinion_stat_index(UINT32_MAX),
      image{GPU_UINT_MAX}
    {}
  }
}
