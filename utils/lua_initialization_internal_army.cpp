#include "lua_initialization_internal.h"

#include "core/army.h"

#define TO_LUA_INDEX(index) ((index)+1)
#define FROM_LUA_INDEX(index) ((index)-1)

// ко всем типам добавится работа с эвентами, флагами и модификаторами

namespace devils_engine {
  namespace utils {
    namespace internal {
      void setup_lua_army(sol::state_view lua) {
        auto core = lua["core"].get_or_create<sol::table>();
        core.new_usertype<core::army>("army", sol::no_constructor,
          "troops", sol::readonly_property([] (const core::army* self) { return std::ref(self->troops); }),
          "troops_count", sol::readonly_property(&core::army::troops_count),
          "tile_index", sol::readonly_property(&core::army::tile_index),
          "computed_stats", sol::readonly_property([] (const core::army* self, const uint32_t &index) { return self->computed_stats[FROM_LUA_INDEX(index)].uval; }),
          "current_stats", sol::readonly_property([] (const core::army* self, const uint32_t &index) { return self->current_stats[FROM_LUA_INDEX(index)].uval; }),
          "stats_count", sol::var(core::army_stats::count),
          "find_path", [] (core::army* self, const uint32_t &tile_index) { self->find_path(FROM_LUA_INDEX(tile_index)); },
          "can_advance", &core::army::can_advance,
          "can_full_advance", &core::army::can_full_advance,
          "advance", &core::army::advance,
          "finding_path", &core::army::finding_path,
          "has_path", &core::army::has_path,
          "path_not_found", &core::army::path_not_found,
          "clear_path", &core::army::clear_path
        );
      }
    }
  }
}
