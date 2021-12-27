#ifndef DEVILS_ENGINE_CORE_CITY_H
#define DEVILS_ENGINE_CORE_CITY_H

#include <array>

#include "city_type.h"
#include "stats.h"
#include "stat_data.h"
#include "declare_structures.h"
#include "utils/bit_field.h"
#include "utils/structures_utils.h"
#include "utils/sol.h"
#include "utils/handle.h"
#include "utils/list.h"

// город должен давать какую то армию (я пока что думаю пусть дает уникальную армию)
// армии без возможности смешивания, как город дает армии? у нас есть построенные здания
// здания дают отряды, из них набирается войско

// у городов должно быть несколько состояний: руины, отсутствие города, собственно город

// возможно хранить армию в провинции? идея неплохая

namespace devils_engine {
  namespace core {
    // на карте еще было бы неплохо разместить что то вроде данжей (или например хижину ведьмы)
    // я полагаю необходимо разделить эти вещи
    // город довольно сильно зависит от титула, в нем будет хранится название, id у города в этом случае не будет видимо
    struct city : public utils::flags_container, public utils::modificators_container, public utils::events_container, public utils::ring::list<core::city, utils::list_type::province_cities> {
      static const structure s_type = structure::city;
      
      enum class state {
        not_exist,
        ruin,
        exist
      };
      
      struct building_view {
        uint32_t off_troops_count;
        uint32_t def_troops_count;
        utils::handle<core::troop>* off_troops;
        utils::handle<core::troop>* def_troops;
        
        building_view();
        ~building_view();
        
        void create(const uint32_t &off_troops_count, const uint32_t &def_troops_count);
        void clear();
      };
      
      struct province* province; // по идее можно смело ставить const
      struct titulus* title; // герб, тут же имя
      const city_type* type;
      utils::bit_field<city_type::maximum_buildings> available_buildings;
      utils::bit_field<city_type::maximum_buildings> completed_buildings;
      utils::bit_field<city_type::maximum_buildings> visible_buildings; // тут быстрый способ проверить что нужно рисовать в интерфейсе а что нет
      size_t start_building;
      uint32_t building_index;
      uint32_t tile_index;
      enum state state;
      // характеристики (текущие характеристики города, база + со всех зданий) (характеристики довольно ограничены, нужно ли дать возможность их модифицировать?)
      utils::stats_container<city_stats::values> stats; // база по идее хранится в типе города
      utils::stats_container<city_stats::values> current_stats;
      utils::stats_container<city_resources::values> resources;
      
      // держать тут сразу армию или держать тут списком отряды? можно и армию в принципе
      // строительство зданий может обновить состав армии, значит пока армия не попадет обратно в город ее свойства не изменятся
      // армия столицы провинции - это провинциальная армия, в нее минорки дают немного войск, 
      // у минорок тоже может быть оффенсив армия, но она скорее всего маленькая
//       utils::handle<army> offensive_army;
//       utils::handle<army> defensive_army;
      
      // здесь скорее нужно хранить отряды (могут ли уменьшится количество отрядов? возможно)
      // как быть? берем контейнер отряда из контекста, если меняется тип отряда, то используем контейнер
      // какой контейнер лучше? вектор? хотя нам было бы неплохо гарантировать будем ли мы убирать отряды
      // было бы неплохо улучшать юнитов (а это удаление старого юнита или нет?)
//       troop* troops;
      std::array<building_view, core::city_type::maximum_buildings> units_view;
      
      city();
      ~city();
      
      // нужно ли тут проверять limit_buildings и prev_buildings? где то это делать нужно в любом случае, и я думаю что это быстрый способ собрать available_buildings 
      // то есть если лимит выполняется или прев не выполняется, то это здание не появляется в постройках, более сложные случаи уходят в conditions
      // более сложные условия делать не нужно, тут же нужно сделать проверку на владельца
      bool check_build(const character* c, const uint32_t &building_index) const;
      // функция непосредственно строительства (отнимаем у игрока деньги)
      bool start_build(character* c, const uint32_t &building_index);
      size_t turns_to_complete() const;
      
      bool has_building_project() const;
      inline bool available(const uint32_t &index) const { return available_buildings.get(index); }
      inline bool constructed(const uint32_t &index) const { return completed_buildings.get(index); }
      inline bool visible(const uint32_t &index) const { return visible_buildings.get(index); }
      
      size_t find_building(const building_type* b) const;
      size_t find_building_upgrade(const building_type* b, const size_t &start = 0) const;
      
      bool check_ownership(const character* c) const;
      bool check_limit(const building_type* b) const;
      bool check_prerequisites(const building_type* b) const;
      bool check_resources(const building_type* b, const character* c) const;
      
      void fill_troops();
      
      void update_turn();
      void update_troops();
      
      core::titulus* get_title() const;
      const core::city_type* get_city_type() const;
    };
    
//     size_t add_city(const sol::table &table);
    bool validate_city(const size_t &index, const sol::table &table);
    bool validate_city_and_save(const size_t &index, sol::this_state lua, const sol::table &table, utils::world_serializator* container);
    void parse_city(core::city* city, const sol::table &table);
  }
}

#endif
