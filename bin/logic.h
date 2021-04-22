#ifndef LOGIC_H
#define LOGIC_H

#include <atomic>

namespace devils_engine {
  namespace core {
    struct character;
  }
  
  namespace game {
    void update_player(core::character* c);
    core::character* get_player();
    bool current_player_turn();
    bool player_end_turn();
    void advance_state();
    void advance_generator();
  }
}

#endif

// где то тут надо бы расположить все геймплейные функции, для которых нужно подтвержение
// хода + возможно передача каких то данных по сети, какие функции это могут быть:
// перемещение войск, выбор ответа в эвенте, какое то решение, изменение закона,
// женитьба, дипломатия (начало войны например), 
