#include "lua_initialization_hidden.h"
#include "lua_initialization_internal.h"

#include "core/stats_table.h"
#include "core/realm_mechanics_arrays.h"
#include "utils/magic_enum_header.h"

namespace devils_engine {
  namespace utils {
    namespace internal {
      template <typename T>
      void create_enum(sol::table &core, const std::string_view &name, const size_t &offset, const std::string_view names[], const bool to_lua_index = true) {
        sol::table target = core.create(0, static_cast<int>(T::count));
        for (size_t i = 0; i < T::count; ++i) {
          target.set(names[i], offset + (to_lua_index ? TO_LUA_INDEX(i) : i));
        }
        
        target.set("start", offset + 1);
        target.set("last", offset + T::count);

        const sol::table x = core.create_with(sol::meta_function::new_index, sol::detail::fail_on_newindex, sol::meta_function::index, target);
        core.create_named(name, sol::metatable_key, x);
      }

      void setup_lua_enums(sol::state_view lua) {
        // тут видимо тоже нужно задать энумы
        auto core = lua[magic_enum::enum_name<reserved_lua::core>()].get_or_create<sol::table>();
        // оффсеты нужны чтобы заполнить таблицы статов при создании карты, но нужны ли они в самой игре, можно добавить для дополнительной проверки статов
        create_enum<core::character_stats::values>(core, "character_stats", core::offsets::character_stats, core::character_stats::names);
        create_enum<core::troop_stats::values>(core, "troop_stats", core::offsets::troop_stats, core::troop_stats::names);
        create_enum<core::hero_stats::values>(core, "hero_stats", core::offsets::hero_stats, core::hero_stats::names);
        create_enum<core::realm_stats::values>(core, "faction_stats", core::offsets::realm_stats, core::realm_stats::names);
        create_enum<core::province_stats::values>(core, "province_stats", core::offsets::province_stats, core::province_stats::names);
        create_enum<core::city_stats::values>(core, "city_stats", core::offsets::city_stats, core::city_stats::names);
        create_enum<core::army_stats::values>(core, "army_stats", core::offsets::army_stats, core::army_stats::names);
        create_enum<core::hero_troop_stats::values>(core, "hero_troop_stats", core::offsets::hero_troop_stats, core::hero_troop_stats::names);
        create_enum<core::character_resources::values>(core, "character_resources", core::offsets::character_resources, core::character_resources::names);
        create_enum<core::realm_resources::values>(core, "realm_resources", core::offsets::realm_resources, core::realm_resources::names);
        create_enum<core::city_resources::values>(core, "city_resources", core::offsets::city_resources, core::city_resources::names);
        create_enum<core::army_resources::values>(core, "army_resources", core::offsets::army_resources, core::army_resources::names);
        
        create_enum<core::opinion_modifiers::values>(core, "opinion_modifiers", 0, core::opinion_modifiers::names, false);
        
        create_enum<core::power_rights::values>(core, "power_rights", core::power_rights::offset, core::power_rights::names);
        create_enum<core::state_rights::values>(core, "state_rights", core::state_rights::offset, core::state_rights::names);
//         create_enum<core::realm_mechanics::values, core::realm_mechanics::names>(core, "realm_laws_mechanics", 0);
        create_enum<core::religion_mechanics::values>(core, "religion_mechanics", 0, core::religion_mechanics::names);
        create_enum<core::culture_mechanics::values>(core, "culture_mechanics", 0, core::culture_mechanics::names);
        
      }
    }
  }
}
