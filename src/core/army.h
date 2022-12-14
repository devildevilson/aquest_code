#ifndef DEVILS_ENGINE_CORE_ARMY_H
#define DEVILS_ENGINE_CORE_ARMY_H

#include <array>
#include "utils/structures_utils.h"
#include "declare_structures.h"
#include "stats.h"
#include "ai/path_container.h"
#include "ai/path_finding_data.h"
#include "utils/handle.h"
#include "script/get_scope_commands_macro.h"
// #include "troop.h"

// у армий наверное будет несколько состояний: армия может быть разбита (в этом состоянии будет зализывать раны в городе?)
// можем ли мы выделить армию которая находится внутри города?

// как сделать поведение атаки чего нибудь? если я пытаюсь перейти на тайл с противником
// мне нужно определить противников

namespace devils_engine {
  namespace core {
    // тип армии нужен по идее только для графики
    struct army_type {
      std::string id;
      std::string name_id;
      std::string description_id;
      
      // графика
    };
    
    // армия создается только когда мы собираем войска
    // при роспуске эту структуру удаляем
    // тут нужно указать еще позицию на карте конкретную, для анимации
    // тут еще должно быть хранилище для пути в армии + манипуляция этим путем
    // отряды можно передать, нападение на другую армию, нападение на город, 
    // зайти в город? засада? быстрый марш? пока сделаем базовые штуки
    // а как сделана принадлежность армий? пока что вообще ни как
    // может ли возникнуть ситуация когда провинция отошла другому государству, а армия с этой провинции все еще участвует в битве?
    // вообще может, что делать? возвращать армию? было бы логично
    // нужно еще обновить отряды армии (их хп и поставить новые выкинуть старые)
    struct army : public ai::path_finding_data, public utils::flags_container, public utils::modificators_container, public utils::events_container {
      static const structure s_type = structure::army;
      static const size_t modificators_container_size = 10;
      static const size_t events_container_size = 15;
      static const size_t flags_container_size = 25;
      static const size_t max_troops_count = 20;
      
      enum class state {
        stationed,      // в провинции
        on_land,        // на земле
        sailing,        // на воде (думаю что погрузка в воду должна быть доступна везде, но ценою очков передвижения)
        pending_siege,  // игроку необходимо выбрать что делать со штурмом города
        pending_battle, // игроку необходимо выбрать что делать при встрече с армией противника
        besieging,      // занята осадой
        fleeing,        // армия должна вернуться после того как была разбита
        
        // на всякий случай
        reserved1,
        reserved2,
        reserved3,
        
        count
      };
      
      enum relationship {
        controlled_unit,
        ally_unit,
        neutral_unit,
        enemy_unit,
        
        count
      };
      
      size_t object_token;
      
      // принадлежность армий, может ли реалм уничтожиться до уничтожения армии? мы можем доедать последнюю провку у противника
      // но тогда должна вызваться чистка, в которой собственно мы должны будем избавиться от армий и прочего
      utils::handle<realm> owner; // овнер по идее всегда владелец провинции
      character* general;
      // отряды + полководцы (главный полководец должен быть всегда первым)
      // количество отрядов в армии может быть больше 20
      //std::array<troop, max_troops_count> troops; // возьмем в качестве отправной точки 20 юнитов (возможно ограничивать не потребуется)
      uint32_t troops_count; //???
      utils::handle<troop> troops;
      // позиция (просто указатель на тайл?)
      uint32_t tile_index;
      // характеристики армии? передвижение, атака, защита (?), все характеристики для отрядов
      utils::stats_container<army_stats::values> stats;
      utils::stats_container<army_stats::values> current_stats;
      utils::stats_container<army_resources::values> resources;
      // какие то спутники? (спутники по идее у персонажа)
      // графика (иконка и отображение на карте)
      //render::image_t map_img; // на карте это нужно отображать с флагом
      
      enum state state;
      province* origin;
      city* target_city;
      utils::handle<army> target_army;
      // наверное еще мы можем провзаимодействовать с героем
      //const city* origin;
      
      //uint32_t army_gpu_slot;
      glm::vec3 pos;
      // картинка по идее должна быть в типе армии (там будут культурные особенности видимо отображены)
      render::image_t image;
      
      size_t current_time;
      float current_pos;
      bool advancing;
      
      army();
      ~army();
      
      void set_pos(const glm::vec3 &pos);
      glm::vec3 get_pos() const;
      void set_img(const render::image_t &img);
      render::image_t get_img() const; // куда пихнуть геральдику? так не хочется еще заводить переменные для армий
      
      // мы не можем искать путь и двигаться пока находимся в определенном состоянии
      // и при этом мы не можем управлять армией когда она убегает
      size_t can_advance() const;
      bool can_full_advance() const;
      void advance();
      void stop();
      void find_path(const uint32_t end_tile_index);
      void clear_path();
      
      void update(const size_t &time);
      
      bool is_home() const;
      
      troop* next_troop(const troop* current) const;
      
      relationship get_relation(const utils::handle<core::army> &another) const;
      relationship get_relation(const utils::handle<core::hero_troop> &another) const;
      relationship get_relation(const core::city* another) const;
      
      bool can_be_raized() const;
      bool can_be_returned() const;
      bool is_in_pending_state() const;
      bool is_in_movement_state() const;
      
      void place_army(const uint32_t &tile_index);
      
      void raize_army(const uint32_t &tile_index);
      // возвращаем армию, армию мы можем вернуть когда? 
      // вообще по идее в любой момент кроме случаев когда
      // армия свободна и не находится рядом с противником
      void return_army();
      // тут мне нужно напасть на город или на армию
      
#define GET_SCOPE_COMMAND_FUNC(name, a, b, type) type get_##name() const;
      ARMY_GET_SCOPE_COMMANDS_LIST
#undef GET_SCOPE_COMMAND_FUNC
    };
  }
}

#endif
