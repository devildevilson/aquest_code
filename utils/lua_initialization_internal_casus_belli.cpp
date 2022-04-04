#include "lua_initialization_internal.h"

#include "core/casus_belli.h"

namespace devils_engine {
  namespace utils {
    namespace internal {
      void setup_lua_casus_belli(sol::state_view lua) {
        auto core = lua["core"].get_or_create<sol::table>();
        auto casus_belli = core.new_usertype<core::casus_belli>(
          "casus_belli", sol::no_constructor,
          "id", sol::readonly(&core::casus_belli::id),
          "battle_warscore_mult", sol::readonly(&core::casus_belli::battle_warscore_mult),
          "infamy_modifier", sol::readonly(&core::casus_belli::infamy_modifier),
          "ticking_war_score_multiplier", sol::readonly(&core::casus_belli::ticking_war_score_multiplier),
          "att_ticking_war_score_multiplier", sol::readonly(&core::casus_belli::att_ticking_war_score_multiplier),
          "def_ticking_war_score_multiplier", sol::readonly(&core::casus_belli::def_ticking_war_score_multiplier),
          "max_defender_occupation_score", sol::readonly(&core::casus_belli::max_defender_occupation_score),
          "max_attacker_occupation_score", sol::readonly(&core::casus_belli::max_attacker_occupation_score),
          "max_defender_battle_score", sol::readonly(&core::casus_belli::max_defender_battle_score),
          "max_attacker_battle_score", sol::readonly(&core::casus_belli::max_attacker_battle_score),
          "truce_turns", sol::readonly(&core::casus_belli::truce_turns),
          "can_use",        &core::casus_belli::can_use,
          "authority_cost", &core::casus_belli::authority_cost,
          "esteem_cost",    &core::casus_belli::esteem_cost,
          "influence_cost", &core::casus_belli::influence_cost,
          "money_cost",     &core::casus_belli::money_cost,
          "ai_probability", &core::casus_belli::ai_probability,
          "name",           &core::casus_belli::name,
          "war_name",       &core::casus_belli::war_name
        );
        
#define CASUS_BELLI_FLAG_FUNC(name) casus_belli.set_function(#name, &core::casus_belli::name);
        CASUS_BELLI_GET_BOOL_NO_ARGS_COMMANDS_LIST
#undef CASUS_BELLI_FLAG_FUNC

#define CASUS_BELLI_NUMBER_FUNC(name) casus_belli.set_function(#name, sol::readonly_property(&core::casus_belli::get_##name));
        CASUS_BELLI_GET_NUMBER_NO_ARGS_COMMANDS_LIST
#undef CASUS_BELLI_NUMBER_FUNC
      }
    }
  }
}
