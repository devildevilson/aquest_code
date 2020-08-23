#include "render_mode_container.h"
#include "utils/globals.h"
#include <stdexcept>

namespace devils_engine {
  namespace render {
    void mode(const std::string &str) {
      auto itr = global::get<const mode_container>()->find(str);
      if (itr == global::get<const mode_container>()->end()) throw std::runtime_error("Could not find rendering mode " + str);
      itr->second();
    }
  }
}
