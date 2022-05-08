#ifndef LUA_INITIALIZATION_H
#define LUA_INITIALIZATION_H

#include "utils/sol.h"

namespace devils_engine {
  namespace utils {
    namespace game_interface {
      void setup_lua(sol::state_view lua);
    }
    
    namespace world_map_loading {
      void setup_lua(sol::state_view lua);
    }
    
    namespace world_map_generation {
      void setup_lua(sol::state_view lua);
    }
    
    namespace battle_map_generation {
      void setup_lua(sol::state_view lua);
    }
    
    namespace battle_map {
      void setup_lua(sol::state_view lua);
    }
  }
}

#endif
