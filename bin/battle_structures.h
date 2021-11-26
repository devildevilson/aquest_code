#ifndef BATTLE_STRUCTURES_H
#define BATTLE_STRUCTURES_H

#include <string>
#include <cstdint>
#include <cstddef>
#include <functional>
#include "utils/linear_rng.h"
#include "core/stats.h"
#include "core/stat_modifier.h"
#include "state.h"
#include "battle_structures_enum.h"

namespace devils_engine {
  namespace render {
    struct battle_biome_data_t;
  }
  
  namespace battle {
    // нужно завести массив тайлов с какими то данными для битвы
    struct tile {
      uint32_t troop_index;
    };
    
    struct biome {
      static const structure_type type_id = structure_type::biome;
      
      std::string id;
      //render::battle_biome_data_t* render_data;
      uint32_t index; // индекс данных биома в буфере
    };
    
//     struct unit_type {
//       static const structure_type type_id = structure_type::unit_type;
//       
//       std::string id;
//       const core::state* default_state;
//       // статы (или не будет у юнита статов?)
//       // нам определенно потребуется задать какие юниты скорее всего умрут
//     };
    
    #define MAX_STATE_CHANGES 20
    
    // было бы неплохо оставлять трупы на тайлах где они умирали, для этого нужно сохранять какие то индексы или координаты
    struct unit {
      enum class status {
        idle,
        marching,
        fast_marching,
        onslaught,
        fighting,
        running,
        dying,
        thrown, // нужно как то сделать смешивание (слом) построения 
        
        count
      };
      
      using rng_state = utils::splitmix64::state;
      constexpr static const auto init_func = utils::splitmix64::init;
      constexpr static const auto rng_func = utils::splitmix64::rng;
      constexpr static const auto get_value_func = utils::splitmix64::get_value;
      
      static const structure_type type_id = structure_type::unit;
      
//       const unit_type* type;
      enum status status;
      uint32_t unit_gpu_index;
      const core::state* current_state;
      const core::state* next_state;
      size_t state_time;
      size_t user_time;
      // состояние псевдогенератора (достаточно небольшого, но нам нужно будет использовать его в мультипотоке + в луа)
      rng_state rng;
      // где то тут еще положение и направление
      // данные для гпу (собств поз, дир, текстурки и все?)
      
      float scale;
      uint32_t change_counter;
      
      unit();
      ~unit();
      void set_state(const std::string_view &name); // откуда брать состояния?
      void reset_timer();
      double random(); // тут возвращается число от 0 до 1? да
      double random(const double &upper);
      double random(const double &lower, const double &upper);
      std::string_view state() const;

      void update(const size_t &time); // тут по идее new_state должен быть всегда нулл
      void seed_random(const uint64_t &seed);
      void set_state(const struct core::state* state);
      glm::vec4 get_pos() const;
      void set_pos(const glm::vec4 &pos);
      glm::vec4 get_dir() const;
      void set_dir(const glm::vec4 &dir);
    };
    
    struct troop_type {
      static const structure_type type_id = structure_type::troop_type;
      
      std::string id;
      //const unit_type* units_type;
      uint32_t units_count; // почему я это убрал?
      float unit_scale;
      core::state* default_unit_state;
      core::stat_container stats[core::troop_stats::count];
    };
    
    struct troop {
      static const structure_type type_id = structure_type::troop;
      
      const troop_type* type;
      uint32_t tile_index; // тут информация о количестве хранится?
      core::stat_container stats[core::troop_stats::count];
      // конкретные юниты + статы отряда
      // + данные для гпу (какие? количество и оффсет?)
      // количество - явно не очень большое число (я бы дал до 256)
      // скорее количеств будет сильно меньше чем 200 (20-30 иначе слишком мелкие челики будут)
      // поэтому поместим в одну переменную 8 бит + 24 бит оффсет
      // как быть с мертвыми? их по идее тоже нужно рисовать, но уже относительно тайла
      // было бы неплохо сделать для них рендер вроде частиц
      
      uint32_t unit_count;
      uint32_t unit_offset;
      
      troop();
    };
  }
}

#endif
