#include "unit_states_container.h"

namespace devils_engine {
  namespace battle {
    unit_states_container::~unit_states_container() {
      for (const auto pair : states) {
        states_pool.destroy(pair.second);
      }
    }
    
    void unit_states_container::create_state(const core::state &data) {
      if (states.find(data.id) != states.end()) throw std::runtime_error("State " + data.id + " is already exists");
      auto ptr = states_pool.create(data);
      states[ptr->id] = ptr;
    }
    
    const core::state* unit_states_container::get_state(const std::string_view &id) const {
      auto itr = states.find(id);
      return itr != states.end() ? itr->second : nullptr;
    }
  }
}
