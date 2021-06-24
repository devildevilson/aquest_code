#include "troop.h"

namespace devils_engine {
  namespace core {
    troop::troop() : type(nullptr), character(nullptr) { 
      memset(moded_stats.data(), 0, sizeof(moded_stats[0]) * troop_stats::count);
      memset(current_stats.data(), 0, sizeof(current_stats[0]) * troop_stats::count);
    }
  }
}
