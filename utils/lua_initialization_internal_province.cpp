#include "lua_initialization_internal.h"

#include "globals.h"
#include "bin/core_structures.h"
// #include "bin/helper.h"
#include "utility.h"
#include "bin/logic.h"
#include "input.h"
#include "progress_container.h"
#include "main_menu.h"
#include "demiurge.h"

namespace devils_engine {
  namespace utils {
    namespace internal {
      void setup_lua_province(sol::state &lua) {
        auto core = lua["core"].get_or_create<sol::table>();
        sol::usertype<core::province> province_type = core.new_usertype<core::province>("province",
          sol::no_constructor,
          "title", sol::readonly(&core::province::title),
          "tiles", sol::readonly(&core::province::tiles),
          "neighbours", sol::readonly(&core::province::neighbours),
          "cities_max_count", sol::readonly(&core::province::cities_max_count),
          "cities_count", sol::readonly(&core::province::cities_count),
          "cities", sol::readonly_property([] (const core::province* self) { return std::ref(self->cities); }),
          "modificators", sol::readonly(&core::province::modificators),
          "events", sol::readonly(&core::province::events),
          "flags", sol::readonly(&core::province::flags),
          "modificators_container_size", sol::var(core::province::modificators_container_size),
          "events_container_size", sol::var(core::province::events_container_size),
          "flags_container_size", sol::var(core::province::flags_container_size),
          "cities_max_game_count", sol::var(core::province::cities_max_game_count)
        );
      }
    }
  }
}
