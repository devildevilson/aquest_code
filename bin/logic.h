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
  }
}

#endif
