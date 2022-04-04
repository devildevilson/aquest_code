#include "lua_initialization_hidden.h"

#include "globals.h"
#include "systems.h"
#include "input.h"
#include "magic_enum_header.h"

#include <iostream>

namespace devils_engine {
  namespace utils {
    void setup_lua_input(sol::state_view lua) {
      {
        auto utils = lua[magic_enum::enum_name<reserved_lua::values>(reserved_lua::utils)].get_or_create<sol::table>();
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
        target.set_function("timed_check_event", [] (const utils::id &event, const uint32_t &states, const double &wait, const double &period) {
          return input::timed_check_event(event, states, wait, period);
        });
        target.set_function("event_state", input::input_event_state);
        target.set_function("is_event_pressed", input::is_event_pressed);
        target.set_function("is_event_released", input::is_event_released);
        target.set_function("get_event_key_name", [] (const utils::id &id, const uint32_t &slot) -> const char* {
          //if (slot >= input::event_key_slots) throw std::runtime_error();
          auto core = global::get<systems::core_t>();
          const auto* k = core->keys_mapping;
          return input::get_event_key_name(player::states_count, k, id, slot);
        });
        target.set_function("get_event", input::get_event);
        target.set_function("frame_events", [] (const sol::function &func) {
          size_t mem = 0;
          auto ret = input::next_input_event(mem);
          while (ret.id.valid()) {
            auto local_ret = ret;
            ret = input::next_input_event(mem);
            
            const auto ret = func(local_ret.id, local_ret.event);
            if (!ret.valid()) {
              sol::error err = ret;
              std::cout << err.what();
              throw std::runtime_error("There is sol errors");
            }
          }
        });
        target.set_function("frame_states", [] (const sol::function &func) {
          size_t mem = 0;
          auto ret = input::next_input_state(mem);
          while (ret.id.valid()) {
            auto local_ret = ret;
            ret = input::next_input_state(mem);
            
            func(local_ret.id, local_ret.state);
          }
        });
        target.set_function("get_cursor_pos", input::get_cursor_pos);
        target.set_function("get_framebuffer_size", input::get_framebuffer_size);
        target.set_function("get_window_content_scale", input::get_window_content_scale);
        target.set_function("get_monitor_content_scale", input::get_monitor_content_scale);
        target.set_function("get_monitor_physical_size", input::get_monitor_physical_size);
        // думаю что в луа это проверится быстро
//         target.set_function("check_input_state", [] (const uint32_t state, const uint32_t &check_state) {
//           return (state & check_state) != 0;
//         });
        

        // тут добавятся несколько функций для того чтобы задать клавишу в настройках
        sol::table x = lua.create_table_with(sol::meta_function::new_index, sol::detail::fail_on_newindex, sol::meta_function::index, target);
        lua[magic_enum::enum_name<reserved_lua::values>(reserved_lua::input)] = lua.create_table(0, 0, sol::metatable_key, x);
      }
    }
  }
}
