#include "lua_initialization.h"

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
    void setup_lua_input(sol::state &lua) {
      {
        auto utils = lua["utils"].get_or_create<sol::table>();
        auto id = utils.new_usertype<utils::id>("id",
          "valid", &utils::id::valid,
          "name", &utils::id::name,
          "num", &utils::id::num,
          "get", &utils::id::get
        );
      }

      {
        auto target = lua.create_table_with(
          "release", input::release,
          "press", input::press,
          "repeated", input::repeated,

          "state_initial", input::state_initial,
          "state_press", input::state_press,
          "state_click", input::state_click,
          "state_double_press", input::state_double_press,
          "state_double_click", input::state_double_click,
          "state_long_press", input::state_long_press,
          "state_long_click", input::state_long_click,

          "long_press_time", input::long_press_time,
          "double_press_time", input::double_press_time,

          "event_key_slots", input::event_key_slots
        );

        target.set_function("check_event", input::check_event);
        target.set_function("timed_check_event", input::timed_check_event);
        target.set_function("input_event_state", input::input_event_state);
        target.set_function("is_event_pressed", input::is_event_pressed);
        target.set_function("is_event_released", input::is_event_released);
        target.set_function("get_event_key_name", input::get_event_key_name);
        target.set_function("get_event", input::get_event);
        target.set_function("get_framebuffer_size", input::get_framebuffer_size);
        target.set_function("get_window_content_scale", input::get_window_content_scale);
        target.set_function("get_monitor_content_scale", input::get_monitor_content_scale);
        target.set_function("get_monitor_physical_size", input::get_monitor_physical_size);

        // тут добавятся несколько функций для того чтобы задать клавишу в настройках
        sol::table x = lua.create_table_with(sol::meta_function::new_index, sol::detail::fail_on_newindex, sol::meta_function::index, target);
        lua["input"] = lua.create_table(0, 0, sol::metatable_key, x);
      }
    }
  }
}
