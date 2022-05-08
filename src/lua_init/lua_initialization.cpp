#include "lua_initialization.h"

#include "lua_initialization_hidden.h"

namespace devils_engine {
  namespace utils {
    namespace game_interface {
      void setup_lua(sol::state_view lua) {
        utils::setup_lua_constants(lua);
        utils::setup_lua_safe_utils(lua);
        utils::setup_lua_input(lua);
        utils::setup_lua_package_path(lua);
        //utils::setup_lua_main_menu(lua); // наверное мы в самом луа сделаем
        utils::setup_lua_settings(lua);
        utils::setup_lua_tile(lua);
        utils::setup_lua_game_context(lua);
        utils::setup_lua_localization(lua);
        utils::setup_lua_main_menu(lua);
        utils::setup_lua_interface_core(lua);
        utils::setup_lua_interface_utils(lua);
        utils::setup_lua_types(lua);
        utils::setup_lua_camera(lua);
        utils::setup_lua_selection(lua);
      }
    }
    
    namespace world_map_loading {
      void setup_lua(sol::state_view lua) {
        utils::setup_lua_safe_utils(lua);
        utils::setup_lua_loading_functions(lua);
        utils::setup_lua_constants(lua);
        
//         utils::setup_lua_script_utils(lua);
      }
    }
    
    namespace world_map_generation {
      void setup_lua(sol::state_view lua) {
        utils::setup_lua_package_path(lua);
        utils::setup_lua_world_map(lua);
        utils::setup_lua_utility_map_generator_functions(lua);
        utils::setup_lua_generator_container(lua);
        utils::setup_lua_constants(lua);
        utils::setup_lua_random_engine(lua);
        utils::setup_lua_noiser(lua);
        utils::setup_lua_calendar(lua);
        utils::setup_lua_safe_utils(lua);
      }
    }
    
    namespace battle_map_generation {
      void setup_lua(sol::state_view lua) {
        utils::setup_lua_generator_container(lua);
        utils::setup_lua_random_engine(lua);
        utils::setup_lua_noiser(lua);
        utils::setup_lua_battle_map(lua);
        utils::setup_lua_utility_battle_generator_functions(lua);
        utils::setup_lua_constants(lua);
      }
    }
    
    namespace battle_map {
      void setup_lua(sol::state_view lua) {
        utils::setup_lua_battle_unit(lua);
        utils::setup_lua_constants(lua);
      }
    }
    
//     void setup_lua_game_logic(sol::state &lua) {
//       {
//         auto target = lua.create_table();
//         target.set_function("player_end_turn", game::player_end_turn);
//         target.set_function("current_player_turn", game::current_player_turn);
// 
//         // тут добавятся несколько функций для того чтобы задать клавишу в настройках
//         sol::table x = lua.create_table_with(sol::meta_function::new_index, sol::detail::fail_on_newindex, sol::meta_function::index, target);
//         lua["game"] = lua.create_table(0, 0, sol::metatable_key, x);
//       }
//     }
  }
}
