#ifndef GAME_CONTEXT_H
#define GAME_CONTEXT_H

#include "quest_state.h"
#include <cstddef>
#include <cstdint>
#include <atomic>

namespace devils_engine {
  namespace core {
    struct character;
    struct realm;
  }
  
  namespace game {
    struct turn_status;
    
    struct context {
      struct player_data {
        // вообще должно быть довольно много данных: имя игрока, аватарка, персонаж, информация о том сколько разведано
        // тип присоединения (хотсит или инторнет), сообщения (?), 
      };
      
      uint32_t state; // стейт обновляем в интерфейсе чтобы перейти к загрузке например
      
      struct turn_status* turn_status;
      
      float traced_tile_dist;
      uint32_t traced_tile_index; // тоже +1
      float traced_heraldy_dist;
      uint32_t traced_heraldy_tile_index;
      
      core::character* player_character; // хотя может быть тут даже доступ к фракции
      core::realm* player_faction; // игрок не может без фракции, геймовер или ошибка?
      // быстрый доступ к столице
      // быстрый доступ к войскам
      // быстрый доступ к героям
      // какие то еще данные карты мира, карты битвы и столкновения
      // тут на самом деле не очень много всего
      
      // список игроков и наверное ники игроков + какие то еще данные
      
      std::atomic_bool quit_game_var;
      bool reload_interface;
      
      inline context() : traced_tile_index(UINT32_MAX), player_character(nullptr), player_faction(nullptr), quit_game_var(false), reload_interface(false) {}
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
      
      inline void quit_game() { quit_game_var = true; }
      inline bool quit_state() { return quit_game_var; }
    };
  }
}

#endif
