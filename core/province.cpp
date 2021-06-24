#include "province.h"

namespace devils_engine {
  namespace core {
    const structure province::s_type;
    const size_t province::modificators_container_size;
    const size_t province::events_container_size;
    const size_t province::flags_container_size;
    const size_t province::cities_max_game_count;
    province::province() : 
      title(nullptr), 
      cities_max_count(0), 
      cities_count(0), 
      cities{nullptr} 
    {}
  }
}