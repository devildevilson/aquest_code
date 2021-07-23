#ifndef LUA_INITIALIZATION_HIDDEN_H
#define LUA_INITIALIZATION_HIDDEN_H

#include "sol.h"
#include "reserved_lua_table_names.h"

namespace devils_engine {
  namespace utils {
    /* =================== COMMON =================== */
    void setup_lua_safe_utils(sol::state_view lua);
    void setup_lua_constants(sol::state_view lua);
    void setup_lua_package_path(sol::state_view lua);
    void setup_lua_loading_functions(sol::state_view lua);
    
    /* =================== INTERFACE =================== */
    void setup_lua_interface_core(sol::state_view lua);
    void setup_lua_interface_utils(sol::state_view lua);
    void setup_lua_main_menu(sol::state_view lua);
    void setup_lua_types(sol::state_view lua);
    void setup_lua_input(sol::state_view lua);
    void setup_lua_game_logic(sol::state_view lua);
    void setup_lua_settings(sol::state_view lua);
    void setup_lua_tile(sol::state_view lua);
    void setup_lua_selection(sol::state_view lua);
    void setup_lua_calendar(sol::state_view lua);
    void setup_lua_localization(sol::state_view lua);
    void setup_lua_game_context(sol::state_view lua);
    void setup_lua_camera(sol::state_view lua);
    void setup_lua_progress_container(sol::state_view lua);
    
    /* =================== WORLD MAP LOADING =================== */
    void setup_lua_script_utils(sol::state_view lua);
    
    /* =================== WORLD MAP GENERATION =================== */
    void setup_lua_generator_container(sol::state_view lua);
    void setup_lua_world_map(sol::state_view lua);
    void setup_lua_random_engine(sol::state_view lua);
    void setup_lua_noiser(sol::state_view lua);
    void setup_lua_utility_map_generator_functions(sol::state_view lua);
    
    /* =================== BATTLE MAP GENERATION =================== */
    void setup_lua_battle_map(sol::state_view lua);
    void setup_lua_utility_battle_generator_functions(sol::state_view lua);
    void setup_lua_battle_unit(sol::state_view lua);
    
    /* =================== ENCOUNTER MAP GENERATION =================== */
    void setup_lua_utility_encounter_generator_functions(sol::state_view lua);
  }
}

#endif
