#include "lua_initialization.h"

#include "utility.h"
#include "progress_container.h"
#include "magic_enum.hpp"

#define MAX_SAFE_INTEGER uint64_t(9007199254740991)

namespace devils_engine {
  namespace utils {
    constexpr uint64_t get_power_of_2(const uint32_t &p) {
      return 1 << p;
    }
    
    static_assert(std::is_same_v<double, LUA_NUMBER>);
    
    void setup_lua_constants(sol::state_view lua) {
      //auto constants = lua["constants"].get_or_create<sol::table>();
//       auto t1 = lua.create_table_with(
//         "creating_map", utils::progress_container::creating_map,
//         "loading_map", utils::progress_container::loading_map,
//         "loading_created_map", utils::progress_container::loading_created_map,
//         "loading_map_save", utils::progress_container::loading_map_save,
//         "loading_battle", utils::progress_container::loading_battle,
//         "loading_encounter", utils::progress_container::loading_encounter,
//         "back_to_menu", utils::progress_container::back_to_menu
//       );

      auto target = lua.create_table_with(
        "time_precision", TIME_PRECISION,
        "one_second", ONE_SECOND,
        "half_second", HALF_SECOND,
        "third_second", THIRD_SECOND,
        "quarter_second", QUARTER_SECOND,
        "fifth_second", FIFTH_SECOND,
        "tenth_second", TENTH_SECOND,
        "time_string", TIME_STRING,
        "app_name", APP_NAME,
        //"app_version_str", APP_VERSION_STR,
        "technical_name", TECHNICAL_NAME,
        "engine_name", ENGINE_NAME,
        "engine_version", ENGINE_VERSION,
        //"engine_version_str", ENGINE_VERSION_STR,
        "pi", PI,
        "pi_2", PI_2,
        "pi_h", PI_H,
        "pi_q", PI_Q,
        "pi_e", PI_E,
        "epsilon", EPSILON,
        "size_max", -1,
        "uint32_max", UINT32_MAX,
        "int32_max", INT32_MAX,
        "int32_min", INT32_MIN,
        "max_safe_integer", MAX_SAFE_INTEGER
//         "loading_type", t1
      );

      target.set_function("deg_to_rad", [] (const double &deg) { return DEG_TO_RAD(deg); });
      target.set_function("rad_to_deg", [] (const double &rad) { return RAD_TO_DEG(rad); });

      sol::table x = lua.create_table_with(sol::meta_function::new_index, sol::detail::fail_on_newindex, sol::meta_function::index, target);
      lua[magic_enum::enum_name<reserved_lua::values>(reserved_lua::constants)] = lua.create_table(0, 0, sol::metatable_key, x);

//       auto progress = lua.new_usertype<utils::progress_container>("progress_container",
//         "current_progress", &utils::progress_container::current_progress,
//         "steps_count", &utils::progress_container::steps_count,
//         "hint", &utils::progress_container::hint,
//         "is_finished", &utils::progress_container::is_finished
//       );
    }
  }
}
