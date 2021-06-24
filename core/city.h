#ifndef CITY_H
#define CITY_H

#include <array>
#include "declare_structures.h"
#include "utils/bit_field.h"
#include "city_type.h"
#include "stats.h"
#include "utils/structures_utils.h"

namespace devils_engine {
  namespace core {
    // на карте еще было бы неплохо разместить что то вроде данжей (или например хижину ведьмы)
    // я полагаю необходимо разделить эти вещи
    struct city : public utils::flags_container, public utils::modificators_container, public utils::events_container {
      static const structure s_type = structure::city;
      static const size_t modificators_container_size = 10;
      static const size_t events_container_size = 15;
      static const size_t flags_container_size = 25;
      static const size_t bit_field_size = city_type::maximum_buildings / SIZE_WIDTH;
      static_assert(bit_field_size == 2);
      
      size_t name_index;
      // во время загрузки титул мы можем заполнить по провинции
      struct province* province; // по идее можно смело ставить const
      struct titulus* title; // герб
      const city_type* type;
      utils::bit_field<bit_field_size> available_buildings;
      utils::bit_field<bit_field_size> complited_buildings;
      size_t start_building;
      uint32_t building_index;
      uint32_t tile_index;
      // характеристики (текущие характеристики города, база + со всех зданий) (характеристики довольно ограничены, нужно ли дать возможность их модифицировать?)
      std::array<stat_container, city_stats::count> current_stats;
      
//       modificators_container<modificators_container_size> modificators; // по идее их меньше чем треитов
//       events_container<events_container_size> events; // эвенты и флаги хранятся в титуле
//       flags_container<flags_container_size> flags;
//       phmap::flat_hash_map<const modificator*, size_t> modificators;
      
      city();
      
      // нужно ли тут проверять limit_buildings и prev_buildings? где то это делать нужно в любом случае, и я думаю что это быстрый способ собрать available_buildings 
      // то есть если лимит выполняется или прев не выполняется, то это здание не появляется в постройках, более сложные случаи уходят в conditions
      bool check_build(character* c, const uint32_t &building_index) const;
      // функция непосредственно строительства (отнимаем у игрока деньги)
      bool start_build(character* c, const uint32_t &building_index);
      void advance_building();
    };
  }
}

#endif
