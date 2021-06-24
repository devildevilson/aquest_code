#include "lua_initialization_internal.h"

#include "core/realm.h"
#include "core/character.h"
#include "core/titulus.h"
#include "lua_initialization.h"
#include "magic_enum_header.h"

namespace devils_engine {
  namespace utils {
    namespace internal {
      void setup_lua_faction(sol::state_view lua) {
        auto core = lua[magic_enum::enum_name<reserved_lua::core>()].get_or_create<sol::table>();
        auto faction_type = core.new_usertype<core::realm>("realm",
          sol::no_constructor,
          "leader", sol::readonly(&core::realm::leader),
          "heir", sol::readonly(&core::realm::heir),
          "liege", sol::readonly(&core::realm::liege),
          "state", sol::readonly(&core::realm::state),
          "council", sol::readonly(&core::realm::council),
          "tribunal", sol::readonly(&core::realm::tribunal),
          "vassals", sol::readonly(&core::realm::vassals),
          "next_vassal", sol::readonly(&core::realm::next_vassal),
          "prev_vassal", sol::readonly(&core::realm::prev_vassal),
          "titles", sol::readonly(&core::realm::titles),
          "main_title", sol::readonly(&core::realm::main_title),
          "courtiers", sol::readonly(&core::realm::courtiers),
          "prisoners", sol::readonly(&core::realm::prisoners),
          "stats", sol::readonly_property([] (const core::realm* self) { return std::ref(self->stats); }),
          "realm_mechanics", sol::readonly(&core::realm::mechanics),
          "is_independent", &core::realm::is_independent,
          "is_state", &core::realm::is_state,
          "is_council", &core::realm::is_council,
          "is_tribunal", &core::realm::is_tribunal,
          "is_self", &core::realm::is_self,
          "stats_count", sol::var(core::realm_stats::count),
          "realm_mechanics_size", sol::var(utils::realm_mechanics::count)
        );
      }
    }
  }
}
