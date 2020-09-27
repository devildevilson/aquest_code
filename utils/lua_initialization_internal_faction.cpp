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
      void setup_lua_faction(sol::state &lua) {
        auto core = lua["core"].get_or_create<sol::table>();
        auto faction_type = core.new_usertype<core::faction>("faction",
          sol::no_constructor,
          "leader", sol::readonly(&core::faction::leader),
          "heir", sol::readonly(&core::faction::heir),
          "liege", sol::readonly(&core::faction::liege),
          "state", sol::readonly(&core::faction::state),
          "council", sol::readonly(&core::faction::council),
          "tribunal", sol::readonly(&core::faction::tribunal),
          "vassals", sol::readonly(&core::faction::vassals),
          "next_vassal", sol::readonly(&core::faction::next_vassal),
          "prev_vassal", sol::readonly(&core::faction::prev_vassal),
          "titles", sol::readonly(&core::faction::titles),
          "main_title", sol::readonly(&core::faction::main_title),
          "courtiers", sol::readonly(&core::faction::courtiers),
          "prisoners", sol::readonly(&core::faction::prisoners),
          "stats", sol::readonly_property([] (const core::faction* self) { return std::ref(self->stats); }),
          "realm_mechanics", sol::readonly(&core::faction::mechanics),
          "is_independent", &core::faction::is_independent,
          "is_state", &core::faction::is_state,
          "is_council", &core::faction::is_council,
          "is_tribunal", &core::faction::is_tribunal,
          "is_self", &core::faction::is_self,
          "stats_count", sol::var(core::faction_stats::count),
          "realm_mechanics_size", sol::var(utils::realm_mechanics::count)
        );
      }
    }
  }
}
