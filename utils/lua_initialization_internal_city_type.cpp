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
      void setup_lua_city_type(sol::state &lua) {
        auto core = lua["core"].get_or_create<sol::table>();
        sol::usertype<core::city_type> city_type = core.new_usertype<core::city_type>("city_type",
          sol::no_constructor,
          "id", sol::readonly(&core::city_type::id),
          "buildings", sol::readonly_property([] (const core::city_type* self) { return std::ref(self->buildings); }),
          "stats", sol::readonly_property([] (const core::city_type* self) { return std::ref(self->stats); }),
          "name_id", sol::readonly(&core::city_type::name_id),
          "desc_id", sol::readonly(&core::city_type::desc_id),
          "city_image", sol::readonly(&core::city_type::city_image),
          "city_icon", sol::readonly(&core::city_type::city_icon),
          "maximum_buildings", sol::var(core::city_type::maximum_buildings),
          "stats_count", sol::var(core::city_stats::count)
        );
      }
    }
  }
}
