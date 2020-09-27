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
    void setup_lua_game_logic(sol::state &lua) {
      {
        auto target = lua.create_table();
        target.set_function("player_end_turn", game::player_end_turn);
        target.set_function("current_player_turn", game::current_player_turn);

        // тут добавятся несколько функций для того чтобы задать клавишу в настройках
        sol::table x = lua.create_table_with(sol::meta_function::new_index, sol::detail::fail_on_newindex, sol::meta_function::index, target);
        lua["game"] = lua.create_table(0, 0, sol::metatable_key, x);
      }
    }
  }
}
