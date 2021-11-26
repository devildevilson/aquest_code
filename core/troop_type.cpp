#include "troop_type.h"

namespace devils_engine {
  namespace core {
    const structure troop_type::s_type;
    troop_type::troop_type() : 
      formation(nullptr),
      card{GPU_UINT_MAX} 
    {}
  }
}
