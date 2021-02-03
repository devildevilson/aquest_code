#include "lua_initialization.h"

#include "progress_container.h"

namespace devils_engine {
  namespace utils {
    void setup_lua_progress_container(sol::state_view lua) {
      auto utils = lua["utils"].get_or_create<sol::table>();
      utils.new_usertype<utils::progress_container>("progress_container", sol::no_constructor,
        "current_step", &utils::progress_container::get_value,
        "step_count", &utils::progress_container::get_max_value,
        "type", &utils::progress_container::get_type,
        "hint1", &utils::progress_container::get_hint1,
        "hint2", &utils::progress_container::get_hint2,
        "hint3", &utils::progress_container::get_hint3
        
        // нужно ли добавлять функции изменения?
      );
    }
  }
}