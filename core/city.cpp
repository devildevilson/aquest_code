#include "city.h"

#include "utils/globals.h"
#include "bin/game_time.h"
#include "building_type.h"
#include "character.h"

namespace devils_engine {
  namespace core {
    const structure city::s_type;
    const size_t city::modificators_container_size;
    const size_t city::events_container_size;
    const size_t city::flags_container_size;
    const size_t city::bit_field_size;
    city::city() : 
      province(nullptr), 
      title(nullptr), 
      type(nullptr), 
      start_building(SIZE_MAX), 
      building_index(UINT32_MAX), 
      tile_index(UINT32_MAX) 
    { 
      memset(current_stats.data(), 0, sizeof(current_stats[0]) * city_stats::count);
    }
    
    bool city::check_build(character* c, const uint32_t &building_index) const {
      ASSERT(building_index < core::city_type::maximum_buildings);
      const float build_cost_mod = current_stats[city_stats::build_cost_mod].fval; // по идее к этому моменту статы должны быть все расчитаны
      const float money_cost = type->buildings[building_index]->money_cost * build_cost_mod;
      const float influence_cost = type->buildings[building_index]->influence_cost * build_cost_mod;
      const float esteem_cost = type->buildings[building_index]->esteem_cost * build_cost_mod;
      const float authority_cost = type->buildings[building_index]->authority_cost * build_cost_mod;
      return start_building == SIZE_MAX && !complited_buildings.get(building_index) && available_buildings.get(building_index) && 
        c->current_stats[character_stats::money].fval     >= money_cost && 
        c->current_stats[character_stats::influence].fval >= influence_cost && 
        c->current_stats[character_stats::esteem].fval    >= esteem_cost && 
        c->current_stats[character_stats::authority].fval >= authority_cost;
    }
    
    bool city::start_build(character* c, const uint32_t &building_index) {
      if (!check_build(c, building_index)) return false;
      
      const float build_cost_mod = current_stats[city_stats::build_cost_mod].fval;
      const float money_cost = type->buildings[building_index]->money_cost * build_cost_mod;
      const float influence_cost = type->buildings[building_index]->influence_cost * build_cost_mod;
      const float esteem_cost = type->buildings[building_index]->esteem_cost * build_cost_mod;
      const float authority_cost = type->buildings[building_index]->authority_cost * build_cost_mod;
      
      c->current_stats[character_stats::money].fval -= money_cost;
      c->current_stats[character_stats::influence].fval -= influence_cost;
      c->current_stats[character_stats::esteem].fval -= esteem_cost;
      c->current_stats[character_stats::authority].fval -= authority_cost;
      start_building = global::get<const utils::calendar>()->current_turn();
      this->building_index = building_index;
      
      return true;
    }
    
    void city::advance_building() {
      if (start_building == SIZE_MAX) return;
      const float build_time_mod = current_stats[city_stats::build_time_mod].fval;
      const size_t build_time = type->buildings[building_index]->time * build_time_mod;
      const size_t current_turn = global::get<const utils::calendar>()->current_turn();
      if (current_turn - start_building >= build_time) {
        complited_buildings.set(building_index, true);
        start_building = SIZE_MAX;
        building_index = UINT32_MAX;
      }
    }
  }
}
