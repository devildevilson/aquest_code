#ifndef STAT_DATA_H
#define STAT_DATA_H

#include <cstdint>
#include <string>
#include "stats.h"
#include "declare_structures.h"
#include "render/shared_structures.h"
#include "utils/constexpr_funcs.h"
#include "utils/bit_field.h"

namespace devils_engine {
  namespace core {
    struct stat_data {
      enum flags {
        show_as_percent, // стат от нуля до единицы
        is_good,         // положительный стат рисуется зеленым
        is_monthly,      // ежемесячный
        is_hidden,       // скрытый от игрока стат
        show_value,      // используется для того чтобы не показывать значение для какой то переменной
        count
      };
      
      static constexpr const uint32_t bit_container_size = ceil(double(count) / double(UINT32_WIDTH));
      
      std::string name_id;
      std::string description_id;
      render::image_t img;
      utils::bit_field_32<bit_container_size> flag_bits;
      uint32_t max_decimals;
      float min_value;
      float max_value;
    };
    
    struct stat_data_container {
      stat_data troop_stats_data[troop_stats::count];
      stat_data hero_stats_data[hero_stats::count];
      stat_data character_stats_data[character_stats::count];
      stat_data opinion_stats_data[opinion_modifiers::count];
      stat_data faction_stats_data[realm_stats::count];
      stat_data province_stats_data[province_stats::count];
      stat_data city_stats_data[city_stats::count];
      stat_data army_stats_data[army_stats::count];
      stat_data hero_troop_stats_data[hero_troop_stats::count];
    };
  }
}

#endif
