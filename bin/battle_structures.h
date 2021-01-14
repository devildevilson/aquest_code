#ifndef BATTLE_STRUCTURES_H
#define BATTLE_STRUCTURES_H

#include <string>
#include <cstdint>
#include <cstddef>
#include <functional>
#include "utils/linear_rng.h"

namespace devils_engine {
  namespace render {
    struct battle_biome_data_t;
  }
  
  namespace battle {
    struct unit;
    
    struct biome {
      std::string id;
      //render::battle_biome_data_t* render_data;
      uint32_t index; // индекс данных биома в буфере
    };
    
    struct state {
      std::string id;
      uint32_t* textures; // оффсет + количество
      size_t time;
      state* next;
      // тут скорее указатель на контейнер с функцией (название или индекс)
      // так как функция будет запускаться параллельно, и нам нужно 
      // функцию распарсить во всех стейтах
      std::function<void(unit*)> func;
    };
    
    struct unit_type {
      std::string id;
      const state* default_state;
      // статы (или не будет у юнита статов?)
      // нам определенно потребуется задать какие юниты скорее всего умрут
    };
    
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
      
      
      using rng_state = utils::xorshift64::state;
      constexpr static const auto rng_func = utils::xorshift64::rng;
      constexpr static const auto get_value_func = utils::xorshift64::get_value;
      
      const unit_type* type;
      enum status status;
      const state* current_state;
      size_t state_time;
      size_t user_time;
      // состояние псевдогенератора (достаточно небольшого, но нам нужно будет использовать его в мультипотоке + в луа)
      rng_state rng;
      // где то тут еще положение и направление
      // данные для гпу (собств поз, дир, текстурки и все?)
      
      void set_state(const std::string_view &name);
      void reset_timer();
      void update(const size_t &time);
      double random(); // тут возвращается число от 0 до 1? да
      double random(const double &upper);
      double random(const double &lower, const double &upper);
    };
    
    struct troop_type {
      std::string id;
      // количество юнитов
    };
    
    struct troop {
      const troop_type* type;
      uint32_t tile_index;
      // конкретные юниты + статы отряда
      // + данные для гпу (какие? количество и оффсет?)
    };
  }
}

#endif
