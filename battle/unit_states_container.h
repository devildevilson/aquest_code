#ifndef BATTLE_UNIT_STATES_CONTAINER_H
#define BATTLE_UNIT_STATES_CONTAINER_H

#include <string>
#include <unordered_map>
#include <vector>
#include "utils/memory_pool.h"
#include "state.h"

// зачем?

namespace devils_engine {
  namespace core {
    struct state;
  }
  
  namespace battle {
    class unit_states_container {
    public:
      ~unit_states_container();
      
      void create_state(const core::state &data);
      
      const core::state* get_state(const std::string_view &id) const;
    private:
      std::unordered_map<std::string_view, core::state*> states;
      utils::memory_pool<core::state, sizeof(core::state)*100> states_pool;
    };
  }
}

#endif
