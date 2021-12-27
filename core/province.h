#ifndef DEVILS_ENGINE_CORE_PROVINCE_H
#define DEVILS_ENGINE_CORE_PROVINCE_H

#include <vector>
#include <array>
#include "stats.h"
#include "stat_data.h"
#include "declare_structures.h"
#include "utils/structures_utils.h"
#include "utils/sol.h"
#include "utils/handle.h"
#include "script/get_scope_commands_macro.h"

namespace devils_engine {
  namespace utils {
    class world_serializator;
  }
  
  namespace core {
    // если персонаж владеет титулом баронским этой провинции - он владеет и столицей провинции? (как в цк2)
    // скорее всего да, тут скорее наоборот, если персонаж владеет титулом столицы провинции
    struct province : public utils::flags_container, public utils::modificators_container, public utils::events_container {
      static const structure s_type = structure::province;
      static const size_t cities_max_game_count = 10;
      
      // название (или название по столице провинции?)
      // удобно и герб тоже по столице провинции дать (нет, титул для баронства существует)
      titulus* title; // не будет меняться, но нужно заполнить титул (или его заполнять в другом месте?)
      struct culture* culture;
      struct religion* religion;
      std::vector<uint32_t> tiles;
      std::vector<uint32_t> neighbours;
      // какие техи распространились в этой провинции
      // что то вроде std::vector<bool> ?
      // локальные характеристики (принадлежность провинции?)
      // несколько "городов" (могут быть и замки и епископства, больше типов?) (как минимум должны быть епископства)
      // какой максимум поселений? 7 в цк2, у меня скорее всего меньше (3-4?)
      uint32_t cities_max_count; // в зависимости от размера провинции
      uint32_t cities_count;
      //std::array<city*, cities_max_game_count> cities; // наверное нужно заранее создать все города, город может быть пустым
      city* cities;
      city* capital;
      
      utils::stats_container<province_stats::values> stats;
      utils::stats_container<province_stats::values> current_stats;
      
      // тут было бы неплохо расположить армии провинции
      // как быть с армией защиты? защита должна быть явно у города, но при этом
      // может ли гарнизон замка придти на помощь городу? вряд ли
      // может быть сделать какой нибудь специальной механикой?
      utils::handle<army> offensive_army;
      
      province();
      
      city* next_city(const core::city* city) const;
      
      core::titulus* get_title() const;
      utils::handle<core::army> get_army() const;
      core::city* get_capital() const;
      
      core::culture* get_culture() const;
      core::culture_group* get_culture_group() const;
      core::religion* get_religion() const;
      core::religion_group* get_religion_group() const;
    };
    
//     size_t add_province(const sol::table &table);
//     size_t register_province();
//     size_t register_provinces(const size_t &count);
//     void set_province(const size_t &index, const sol::table &table);
    bool validate_province(const size_t &index, const sol::table &table);
    bool validate_province_and_save(const size_t &index, sol::this_state lua, const sol::table &table, utils::world_serializator* container);
    void parse_province(core::province* province, const sol::table &table);
  }
}

#endif
