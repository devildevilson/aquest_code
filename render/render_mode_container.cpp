#include "render_mode_container.h"
#include "utils/globals.h"
#include "utils/assert.h"
#include <stdexcept>
#include "bin/stats.h"
#include <atomic>
#include <iostream>

namespace devils_engine {
  namespace render {
    static std::atomic<modes::values> current_mode = modes::count;
    modes::values get_current_mode() {
      return current_mode;
    }
    
    void mode(const modes::values &mode) {
      ASSERT(mode < modes::count);
      //std::cout << "Current mode " << magic_enum::enum_name<modes::values>(mode) << "\n";
      auto ptr = global::get<const mode_container>();
      if (ptr->at(mode) == nullptr) throw std::runtime_error("Render mode " + std::string(magic_enum::enum_name<modes::values>(mode)) + " is not set");
      ptr->at(mode)();
      current_mode = mode;
    }
  }
}
