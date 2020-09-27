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
      template <typename T>
      sol::table create_enum(sol::table &core, const std::string &name) {
        sol::table target = core.create(static_cast<int>(T::count), static_cast<int>(0));
        for (size_t i = 0; i < T::count; ++i) {
          target.set(magic_enum::enum_name<T>(static_cast<T>(i)), i);
        }

        sol::table x = core.create_with(sol::meta_function::new_index, sol::detail::fail_on_newindex, sol::meta_function::index, target);
        sol::table shim = core.create_named(name, sol::metatable_key, x);
        return shim;
      }

      void setup_lua_enums(sol::state &lua) {
        //lua["ctx"] = &global::get<interface::context>()->ctx;

        // тут видимо тоже нужно задать энумы
        auto core = lua["core"].get_or_create<sol::table>();
        core["character_stats"] = create_enum<core::character_stats::values>(core, "character_stats");
        core["troop_stats"] = create_enum<core::troop_stats::values>(core, "troop_stats");
        core["hero_stats"] = create_enum<core::hero_stats::values>(core, "hero_stats");
        core["opinion_stats"] = create_enum<core::opinion_stats::values>(core, "opinion_stats");
        core["faction_stats"] = create_enum<core::faction_stats::values>(core, "faction_stats");
        core["province_stats"] = create_enum<core::province_stats::values>(core, "province_stats");
        core["city_stats"] = create_enum<core::city_stats::values>(core, "city_stats");
        core["army_stats"] = create_enum<core::army_stats::values>(core, "army_stats");
        core["hero_troop_stats"] = create_enum<core::hero_troop_stats::values>(core, "hero_troop_stats");

      }
    }
  }
}