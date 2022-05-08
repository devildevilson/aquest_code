#include "on_action_container.h"

#include "on_action_types_arrays.h"

#include <iostream>

namespace devils_engine {
  namespace script {
    void fire_on_action(const action_type::values &type, const std::initializer_list<object> &targets) {
      (void)targets;
      std::cout << "fire_on_action " << action_type::names[type] << "\n";
    }
  }
}
