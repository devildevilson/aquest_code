#include "render_mode_container.h"

#include "utils/globals.h"
#include "utils/assert.h"
#include <stdexcept>
#include "core/stats.h"
#include "utils/systems.h"
#include <atomic>
#include <iostream>

#define MAKE_MAP_PAIR(name) std::make_pair(names[values::name], values::name)

namespace devils_engine {
  namespace render {
    namespace modes {
      const std::string_view names[] = {
#define RENDER_MODE_FUNC(val) #val,
          RENDER_MODES_LIST
#undef RENDER_MODE_FUNC
      };
      
      const phmap::flat_hash_map<std::string_view, values> map = {
#define RENDER_MODE_FUNC(val) MAKE_MAP_PAIR(val),
          RENDER_MODES_LIST
#undef RENDER_MODE_FUNC
      };
      
      const size_t size = sizeof(names) / sizeof(names[0]);
      static_assert(size == values::count);
    }
    
    static std::atomic<modes::values> current_mode = modes::count;
    modes::values get_current_mode() {
      return current_mode;
    }
    
    void mode(const modes::values &mode) {
      if (mode >= modes::count) throw std::runtime_error("Bad render mode index " + std::to_string(mode));
      //std::cout << "Current mode " << magic_enum::enum_name<modes::values>(mode) << "\n";
      auto ptr = global::get<systems::map_t>()->render_modes;
      if (ptr->at(mode) == nullptr) throw std::runtime_error("Render mode " + std::string(modes::names[mode]) + " is not set");
      current_mode = mode;
      ptr->at(mode)();
    }
  }
}
