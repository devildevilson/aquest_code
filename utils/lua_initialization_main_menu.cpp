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
    void setup_lua_main_menu(sol::state &lua) {
      auto utils = lua["utils"].get_or_create<sol::table>();
      auto main_menu = utils.new_usertype<utils::main_menu>("main_menu",
        sol::no_constructor,
        "push", &utils::main_menu::push,
        "exist", &utils::main_menu::exist,
        "escape", &utils::main_menu::escape,
        "quit_game", &utils::main_menu::quit_game,
        "current_entry", &utils::main_menu::current_entry
      );

      auto demiurge = utils.new_usertype<utils::demiurge>("demiurge",
        sol::no_constructor,
        "create_new_world", &utils::demiurge::create_new_world,
        "refresh",          &utils::demiurge::refresh,
        "worlds_count",     &utils::demiurge::worlds_count,
        "world",            &utils::demiurge::world,
        "choose_world",     &utils::demiurge::choose_world
      );
    }
  }
}
