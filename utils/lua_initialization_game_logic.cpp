#include "lua_initialization_hidden.h"

#include "magic_enum_header.h"
#include "bin/logic.h"

namespace devils_engine {
  namespace utils {
    void setup_lua_game_logic(sol::state_view lua) {
      {
        auto target = lua.create_table();
        target.set_function("player_end_turn", game::player_end_turn);
        target.set_function("current_player_turn", game::current_player_turn);

        // тут добавятся несколько функций для того чтобы задать клавишу в настройках
        sol::table x = lua.create_table_with(sol::meta_function::new_index, sol::detail::fail_on_newindex, sol::meta_function::index, target);
        lua[magic_enum::enum_name<reserved_lua::values>(reserved_lua::game)] = lua.create_table(0, 0, sol::metatable_key, x);
      }
    }
  }
}
