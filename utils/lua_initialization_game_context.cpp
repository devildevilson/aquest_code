#include "lua_initialization.h"

#include "globals.h"
#include "systems.h"
#include "game_context.h"
#include "core/character.h"

#define TO_LUA_INDEX(index) ((index)+1)
#define FROM_LUA_INDEX(index) ((index)-1)

namespace devils_engine {
  namespace utils {
    void setup_lua_game_context(sol::state_view lua) {
      auto game = lua["game"].get_or_create<sol::table>();
      game.new_usertype<game::context>("game_context", sol::no_constructor,
        "state", sol::readonly_property([] (const game::context* self) {
          return TO_LUA_INDEX(self->state);
        }),
        "tile_index", sol::readonly_property([] (const game::context* self) {
          return TO_LUA_INDEX(self->traced_tile_index);
        }),
        "tile_index_dist", sol::readonly_property([] (const game::context* self) {
          return self->traced_tile_dist;
        }),
        "heraldy_tile_index", sol::readonly_property([] (const game::context* self) {
          return TO_LUA_INDEX(self->traced_heraldy_tile_index);
        }),
        "heraldy_tile_index_dist", sol::readonly_property([] (const game::context* self) {
          return self->traced_tile_dist;
        }),
//         "new_state", sol::readonly_property(&game::context::new_state),
        "player_character", sol::readonly_property(&game::context::player_character),
        "is_loading", &game::context::is_loading,
        "main_menu", &game::context::main_menu,
        "quit_game", &game::context::quit_game
//         "advance_state", &game::context::advance_state,
//         "state_eq", &game::context::state_eq
      );
      
      game.new_enum("state", {
        std::make_pair(std::string_view("main_menu_loading"),           TO_LUA_INDEX(utils::quest_state::main_menu_loading)),
        std::make_pair(std::string_view("main_menu"),                   TO_LUA_INDEX(utils::quest_state::main_menu)),
        std::make_pair(std::string_view("world_map_generator_loading"), TO_LUA_INDEX(utils::quest_state::world_map_generator_loading)),
        std::make_pair(std::string_view("world_map_generator"),         TO_LUA_INDEX(utils::quest_state::world_map_generator)),
        std::make_pair(std::string_view("world_map_generating"),        TO_LUA_INDEX(utils::quest_state::world_map_generating)),
        std::make_pair(std::string_view("world_map_loading"),           TO_LUA_INDEX(utils::quest_state::world_map_loading)),
        std::make_pair(std::string_view("world_map"),                   TO_LUA_INDEX(utils::quest_state::world_map)),
        std::make_pair(std::string_view("battle_map_loading"),          TO_LUA_INDEX(utils::quest_state::battle_map_loading)),
        std::make_pair(std::string_view("battle_map"),                  TO_LUA_INDEX(utils::quest_state::battle_map)),
        std::make_pair(std::string_view("encounter_loading"),           TO_LUA_INDEX(utils::quest_state::encounter_loading)),
        std::make_pair(std::string_view("encounter"),                   TO_LUA_INDEX(utils::quest_state::encounter))
      });
    }
  }
}
