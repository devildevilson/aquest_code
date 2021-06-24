#include "lua_initialization_internal.h"

#include "core/building_type.h"
#include "lua_initialization.h"
#include "magic_enum.hpp"

namespace devils_engine {
  namespace utils {
    namespace internal {
      void setup_lua_building_type(sol::state_view lua) {
        auto core = lua[magic_enum::enum_name<reserved_lua::core>()].get_or_create<sol::table>();
        sol::usertype<core::building_type> building_type = core.new_usertype<core::building_type>("building_type",
          sol::no_constructor,
          "id", sol::readonly(&core::building_type::id),
          "name_id", sol::readonly(&core::building_type::name_id),
          "desc_id", sol::readonly(&core::building_type::desc_id),
//           "potential", sol::readonly(&core::building_type::potential),
//           "conditions", sol::readonly(&core::building_type::conditions),
          "prev_buildings", sol::readonly_property([] (const core::building_type* self) { return std::ref(self->prev_buildings); }),
          "limit_buildings", sol::readonly_property([] (const core::building_type* self) { return std::ref(self->limit_buildings); }),
          "replaced", sol::readonly(&core::building_type::replaced),
          "upgrades_from", sol::readonly(&core::building_type::upgrades_from),
          "time", sol::readonly(&core::building_type::time),
          "mods", sol::readonly_property([] (const core::building_type* self) { return std::ref(self->mods); }),
          "unit_mods", sol::readonly_property([] (const core::building_type* self) { return std::ref(self->unit_mods); }),
          "money_cost", sol::readonly(&core::building_type::money_cost),
          "authority_cost", sol::readonly(&core::building_type::authority_cost),
          "esteem_cost", sol::readonly(&core::building_type::esteem_cost),
          "influence_cost", sol::readonly(&core::building_type::influence_cost),
          "maximum_prev_buildings", sol::var(core::building_type::maximum_prev_buildings),
          "maximum_limit_buildings", sol::var(core::building_type::maximum_limit_buildings),
          "maximum_stat_modifiers", sol::var(core::building_type::maximum_stat_modifiers),
          "maximum_unit_stat_modifiers", sol::var(core::building_type::maximum_unit_stat_modifiers)
        );
      }
    }
  }
}
