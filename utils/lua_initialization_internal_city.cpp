#include "lua_initialization_internal.h"

#include "bin/core_structures.h"

namespace devils_engine {
  namespace utils {
    namespace internal {
      void setup_lua_city(sol::state_view lua) {
        auto core = lua["core"].get_or_create<sol::table>();
        sol::usertype<core::city> city = core.new_usertype<core::city>("city",
          sol::no_constructor,
          "name_id", sol::readonly(&core::city::name_index),
          "province", sol::readonly(&core::city::province),
          "title", sol::readonly(&core::city::title),
          "type", sol::readonly(&core::city::type),
          "available_buildings", sol::readonly(&core::city::available_buildings),
          "complited_buildings", sol::readonly(&core::city::complited_buildings),
          "start_building", sol::readonly(&core::city::start_building),
          "building_index", sol::readonly(&core::city::building_index),
          "tile_index", sol::readonly(&core::city::tile_index),
          "current_stats", sol::readonly_property([] (const core::city* self) { return std::ref(self->current_stats); }),
          "modificators", sol::readonly(&core::city::modificators),
          "modificators_container_size", sol::var(core::city::modificators_container_size),
          "buildings_size", sol::var(core::city_type::maximum_buildings)
        );
      }
    }
  }
}
