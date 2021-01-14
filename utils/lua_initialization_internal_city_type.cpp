#include "lua_initialization_internal.h"

#include "bin/core_structures.h"

namespace devils_engine {
  namespace utils {
    namespace internal {
      void setup_lua_city_type(sol::state_view lua) {
        auto core = lua["core"].get_or_create<sol::table>();
        sol::usertype<core::city_type> city_type = core.new_usertype<core::city_type>("city_type",
          sol::no_constructor,
          "id", sol::readonly(&core::city_type::id),
          "buildings", sol::readonly_property([] (const core::city_type* self) { return std::ref(self->buildings); }),
          "stats", sol::readonly_property([] (const core::city_type* self) { return std::ref(self->stats); }),
          "name_id", sol::readonly(&core::city_type::name_id),
          "desc_id", sol::readonly(&core::city_type::desc_id),
          "city_image_top", sol::readonly(&core::city_type::city_image_top),
          "city_image_face", sol::readonly(&core::city_type::city_image_face),
          "city_icon", sol::readonly(&core::city_type::city_icon),
          "maximum_buildings", sol::var(core::city_type::maximum_buildings),
          "stats_count", sol::var(core::city_stats::count)
        );
      }
    }
  }
}
