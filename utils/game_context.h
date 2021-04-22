#ifndef GAME_CONTEXT_H
#define GAME_CONTEXT_H

#include "quest_state.h"
#include <cstddef>
#include <cstdint>

namespace devils_engine {
  namespace core {
    struct character;
    struct faction;
  }
  
  namespace game {
    struct context {
      enum class turn_state {
        
      };
      
      //bool loading; // лодинга нет, вместо него будут стейты тип: лоадинг_баттл -> баттл
//       uint32_t old_state; // опять 3 переменные для стейтов, эта переменная нужна чтобы отметить начало стейта
      uint32_t state; // стейт обновляем в интерфейсе чтобы перейти к загрузке например
//       uint32_t new_state;
      
      // тут еще будет состояние хода
      uint32_t turn_state;
      
      core::character* player_character; // хотя может быть тут даже доступ к фракции
      core::faction* player_faction; // игрок не может без фракции, геймовер или ошибка?
      // быстрый доступ к столице
      // быстрый доступ к войскам
      // быстрый доступ к героям
      // какие то еще данные карты мира, карты битвы и столкновения
      // тут на самом деле не очень много всего
      
      bool quit_game_var;
      
      inline context() : player_character(nullptr), player_faction(nullptr), quit_game_var(false) {}
//       inline bool state_eq() const { return state == new_state; }
      inline bool is_loading() const { 
        return state == utils::quest_state::main_menu_loading || 
               state == utils::quest_state::world_map_generator_loading || 
//                state == utils::quest_state::world_map_generating || // скорее нет чем да
               state == utils::quest_state::world_map_loading || 
               state == utils::quest_state::battle_map_loading || 
               state == utils::quest_state::encounter_loading; 
      }
      
      inline void main_menu() {
        if (state != utils::quest_state::main_menu) state = utils::quest_state::main_menu_loading;
      }
      
      inline void quit_game() {
        quit_game_var = true;
      }
      
//       inline void advance_state() { state = new_state; } // обновляем стейт
    };
  }
}

#endif
