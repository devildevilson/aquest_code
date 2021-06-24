#include "culture.h"

namespace devils_engine {
  namespace core {
    const structure culture::s_type;
    const size_t culture::max_stat_modifiers_count;
    culture::culture() : 
      name_id(SIZE_MAX), 
//       name_bank(nullptr), 
      parent(nullptr) 
    {}
  }
}
