#ifndef STAT_DATA_H
#define STAT_DATA_H

#include <cstdint>
#include "stats.h"
#include "render/shared_structures.h"
#include "utils/constexpr_funcs.h"
#include "utils/bit_field.h"

namespace devils_engine {
  namespace core {
    union stat_container {
      uint32_t uval;
      int32_t ival;
      float fval;
    };
    
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
      
      size_t name;
      size_t description;
      render::image_t img;
      utils::bit_field_32<bit_container_size> flag_bits;
      uint32_t max_decimals;
      // минимальные максимальные значения?
    };
    
    struct stat_data_container {
      stat_data troop_stats_data[troop_stats::count];
      stat_data hero_stats_data[hero_stats::count];
      stat_data character_stats_data[character_stats::count];
      stat_data opinion_stats_data[opinion_stats::max_count];
      stat_data faction_stats_data[realm_stats::count];
      stat_data province_stats_data[province_stats::count];
      stat_data city_stats_data[city_stats::count];
      stat_data army_stats_data[army_stats::count];
      stat_data hero_troop_stats_data[hero_troop_stats::count];
    };
    
    struct bonus {
      float raw_add;
      float raw_mul;
      float fin_add;
      float fin_mul;
      
      inline bonus() : raw_add(0.0f), raw_mul(0.0f), fin_add(0.0f), fin_mul(0.0f) {}
    };
    
    struct stat_bonus {
      unit_type type;
      uint32_t stat;
      struct bonus bonus;
      
      inline stat_bonus() : type(unit_type::invalid), stat(UINT32_MAX) {}
    };
    
    struct stat_modifier {
      unit_type type;
      uint32_t stat;
      stat_container mod;
      
      inline stat_modifier() : type(unit_type::invalid), stat(UINT32_MAX), mod{0} {}
    };
    
    struct unit_stat_modifier {
      const troop_type* type;
      uint32_t stat;
      stat_container mod;
      
      inline unit_stat_modifier() : type(nullptr), stat(UINT32_MAX), mod{0} {}
    };
  }
}

#endif
