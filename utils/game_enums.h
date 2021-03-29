#ifndef GAME_ENUMS_H
#define GAME_ENUMS_H

namespace devils_engine {
  namespace player {
    enum values {
      in_menu,
      on_global_map,
      on_battle_map,
      on_hero_battle_map,
      states_count
    };
  }
  
  namespace game {
    enum values {
      loading,
      menu,
      create_map,
      map,
      battle,  
      encounter,
      count
    };
  }
}

#endif
