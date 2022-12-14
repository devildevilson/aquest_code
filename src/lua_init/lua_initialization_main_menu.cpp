#include "lua_initialization_hidden.h"

//#include "main_menu.h"
#include "utils/demiurge.h"
#include "utils/magic_enum_header.h"
#include "utils/globals.h"
// #include "utils/systems.h"
// #include "utils/game_context.h"
#include "utils/interface_container2.h"
#include "bin/application.h"

namespace devils_engine {
  namespace utils {
    // тут на основе game_ctx происходит выбор следующего состояния, мне нужно начать пользоваться состояниями приложения
    // как же тут сделать? самое простое вынести сюда core::application, что тут нужно вообще?
    // нужно после окончания интерфейс скрипта запустить ивент в стейт машине, может быть имеет смысл 
    // сделать несколько функций с простым интерфейсом, хотя наверное сейчас попробую core::application
    static void choose_world(utils::demiurge* self, const size_t &lua_index) {
      const size_t index = FROM_LUA_INDEX(lua_index);
//       auto ctx = global::get<systems::core_t>()->game_ctx;
//       ctx->state = utils::quest_state::world_map_loading; // возможно нужно отложить изменение состояния?
      const auto t = self->world(index);
      self->choose_world(index);
      
      auto proxy = t["path"];
      if (!proxy.valid()) throw std::runtime_error("Broken world table");
      if (proxy.get_type() != sol::type::string) throw std::runtime_error("Broken world table");
      std::string world_path = proxy.get<std::string>();
      auto base = global::get<systems::core_t>();
      base->load_world = world_path;
      global::get<core::application>()->load_saved_world(); // может быть сюда передать строку с путем?
    }
    
    void setup_lua_main_menu(sol::state_view lua) {
      auto utils = lua[magic_enum::enum_name<reserved_lua::values>(reserved_lua::utils)].get_or_create<sol::table>();
//       auto main_menu = utils.new_usertype<utils::main_menu>("main_menu",
//         sol::no_constructor,
//         "push", &utils::main_menu::push,
//         "exist", &utils::main_menu::exist,
//         "escape", &utils::main_menu::escape,
//         "quit_game", &utils::main_menu::quit_game,
//         "current_entry", &utils::main_menu::current_entry
//       );

      auto demiurge = utils.new_usertype<utils::demiurge>("demiurge", sol::initializers([] (void* mem) {
          auto cont = global::get<systems::core_t>()->interface_container.get();
          new (mem) utils::demiurge(cont->lua);
        }),
        "create_new_world", [] (utils::demiurge* self) {
          // тут мы должны поменять состояние игры на генерацию, как это сделать?
          // нужна видимо какая-то функция которая за нас это сделает
          // сейчас я сделал через геймконтекст, игра слушает эту структуру
          // и состояние у игры меняется в зависимости от нее
          (void)self;
          //auto ctx = global::get<systems::core_t>()->game_ctx;
          //ctx->state = utils::quest_state::world_map_generator_loading;
          global::get<core::application>()->create_new_world();
        },
        "refresh",          &utils::demiurge::refresh,
        "worlds_count",     &utils::demiurge::worlds_count,
        "world",            [] (utils::demiurge* self, const size_t &lua_index) {
          const size_t index = FROM_LUA_INDEX(lua_index);
          return self->world(index);
        },
        //"choose_world",     &utils::demiurge::choose_world
        "choose_world",     &choose_world
      );
    }
  }
}
