#include "building_type.h"

namespace devils_engine {
  namespace core {
    const structure building_type::s_type;
    const size_t building_type::maximum_prev_buildings;
    const size_t building_type::maximum_limit_buildings;
    const size_t building_type::maximum_stat_modifiers;
    const size_t building_type::maximum_unit_stat_modifiers;
    building_type::building_type() : 
      name_id(SIZE_MAX), 
      desc_id(SIZE_MAX), 
      prev_buildings{nullptr}, 
      limit_buildings{nullptr}, 
      replaced(nullptr), 
      upgrades_from(nullptr), 
      time(SIZE_MAX), 
      money_cost(0.0f), 
      authority_cost(0.0f), 
      esteem_cost(0.0f), 
      influence_cost(0.0f) 
    {}
  }
}
