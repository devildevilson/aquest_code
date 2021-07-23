#include "lua_initialization_hidden.h"

#include "globals.h"
#include "systems.h"
#include "localization_container.h"

namespace devils_engine {
  namespace utils {
    void setup_lua_localization(sol::state_view lua) {
      auto localization = lua["localization"].get_or_create<sol::table>();
      localization.set_function("get", [] (const std::string_view &key) {
        auto loc = global::get<systems::core_t>()->loc.get();
        auto obj = loc->get(devils_engine::localization::container::get_current_locale(), key);
        if (!obj.valid()) {
          obj = loc->get(devils_engine::localization::container::get_fall_back_locale(), key);
        }
        
        return obj;
      });
    }
  }
}
