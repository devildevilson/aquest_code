#include "troop_type.h"

namespace devils_engine {
  namespace core {
    const structure troop_type::s_type;
    troop_type::troop_type() : 
      name_id(SIZE_MAX), 
      description_id(SIZE_MAX), 
      card{GPU_UINT_MAX} 
    { 
      memset(stats.data(), 0, sizeof(stats[0]) * troop_stats::count);
    }
  }
}
