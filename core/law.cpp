#include "law.h"

namespace devils_engine {
  namespace core {
    const structure law::s_type;
    const size_t law::max_stat_modifiers_count;
    const size_t law::max_mechanics_modifiers_count;
    law::law() : 
      name_id(SIZE_MAX), 
      description_id(SIZE_MAX) 
    {}
  }
}
