#include "lua_initialization_hidden.h"

#include "utils/globals.h"
#include "utils/systems.h"
#include "utils/game_context.h"
#include "core/character.h"
#include "bin/application.h"
#include "utils/deferred_tasks.h"
#include "bin/logic.h"

namespace devils_engine {
  namespace utils {
    // game::context должен скорее возвращать группу игроков и указатель на состояние хода
    
    // думаю что контекст можно передать с помощью объекта в луа, просто нужно сам контекст немного переделать
    
    void setup_lua_game_context(sol::state_view lua) {
      auto game = lua["game"].get_or_create<sol::table>();
      game.new_usertype<game::context>("game_context", sol::no_constructor,
        "state",      sol::readonly_property([] (const game::context* self) { return TO_LUA_INDEX(self->state); }),
        "turn_state", sol::readonly_property([] (const game::context* self) { return TO_LUA_INDEX(static_cast<size_t>(self->turn_status->state)); }),
        "tile_index", sol::readonly_property([] (const game::context* self) { return TO_LUA_INDEX(self->traced_tile_index); }),
        "tile_index_dist", sol::readonly_property(&game::context::traced_tile_dist),
        "heraldy_tile_index", sol::readonly_property([] (const game::context* self) { return TO_LUA_INDEX(self->traced_heraldy_tile_index); }),
        "heraldy_tile_index_dist", sol::readonly_property(&game::context::traced_heraldy_dist),
        //"player_character", sol::readonly_property(&game::context::player_character),
        "player_character", sol::readonly_property([] (const game::context* self) {
          return self->turn_status->current_player;
        }),
        "is_loading", &game::context::is_loading,
        "main_menu", &game::context::main_menu,
        "quit_game", &game::context::quit_game
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
      
#define ENUM_PAIR(name) std::make_pair(std::string_view(#name), TO_LUA_INDEX(static_cast<size_t>(game::turn_status::state::name)))
      
      game.new_enum("turn_state", {
        ENUM_PAIR(initial),
#define TURN_STATE_FUNC(name) ENUM_PAIR(name),
        TURN_STATES_LIST
#undef TURN_STATE_FUNC
      });
      
#undef ENUM_PAIR
      
//       auto context = game["context"].get_or_create<sol::table>();
//       context.set("state", sol::readonly_property([] () {
//         return TO_LUA_INDEX(global::get<systems::core_t>()->game_ctx->state);
//       }));
//       
//       context.set("tile_index", sol::readonly_property([] () {
//         return TO_LUA_INDEX(global::get<systems::core_t>()->game_ctx->traced_tile_index);
//       }));
//       
//       context.set("tile_dist", sol::readonly_property([] () {
//         return global::get<systems::core_t>()->game_ctx->traced_tile_dist;
//       }));
//       
//       context.set("heraldy_tile_index", sol::readonly_property([] () {
//         return TO_LUA_INDEX(global::get<systems::core_t>()->game_ctx->traced_heraldy_tile_index);
//       }));
//       
//       context.set("heraldy_dist", sol::readonly_property([] () {
//         return global::get<systems::core_t>()->game_ctx->traced_heraldy_dist;
//       }));
//       
//       context.set_function("main_menu", [] () {
//         // меняем состояние через applicaton
//         global::get<utils::deferred_tasks>()->add([] (const size_t &) -> bool {
//           global::get<core::application>()->load_menu();
//           return true;
//         });
//       });
//       
//       context.set_function("quit_game", [] () {
//         global::get<systems::core_t>()->game_ctx->quit_game();
//       });
//       
//       context.set_function("get_players", [] () {
//         // возвращаем список игроков, наверное пока что будет возвращать список персонажей
//       });
//       
//       context.set_function("get_local_players", [] () {
//         // возвращаем список локальных игроков (игроков по хотситу)
//       });
//       
//       // не предполагается что в ходе компа у нас будет какой то конкретный игрок
//       // мы можем вывести это получив список локальных игроков
//       auto turn_status = game["turn_status"].get_or_create<sol::table>();
//       turn_status.set("state", sol::readonly_property([] () {
//         
//       }));
//       
//       turn_status.set("current_player", sol::readonly_property([] () {
//         
//       }));
    }
  }
}
