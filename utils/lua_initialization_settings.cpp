#include "lua_initialization_hidden.h"

#include "globals.h"
#include "settings.h"
#include "magic_enum.hpp"

namespace devils_engine {
  namespace utils {
    void setup_lua_settings(sol::state_view lua) {
      auto utils = lua[magic_enum::enum_name<reserved_lua::values>(reserved_lua::utils)].get_or_create<sol::table>();
      
      auto graphics = utils.new_usertype<struct utils::settings::graphics>("graphics_type",
        "width", &utils::settings::graphics::width,
        "height", &utils::settings::graphics::height,
        "video_mode", &utils::settings::graphics::video_mode,
        "fullscreen", &utils::settings::graphics::fullscreen,
        "projection", &utils::settings::graphics::projection,
        "video_modes_count", &utils::settings::graphics::video_modes_count,
        "get_video_mode", &utils::settings::graphics::get_video_mode,
        "apply", &utils::settings::graphics::apply
      );
      
      auto game = utils.new_usertype<struct utils::settings::game>("game_type",
        "camera_movement", &utils::settings::game::camera_movement,
        "camera_movement_x", &utils::settings::game::camera_movement_x,
        "camera_movement_y", &utils::settings::game::camera_movement_y,
        "sens", &utils::settings::game::sens,
        "sens_x", &utils::settings::game::sens_x,
        "sens_y", &utils::settings::game::sens_y,
        "target_fps", &utils::settings::game::target_fps,
        "game_cursor", &utils::settings::game::game_cursor
      );
      
      auto keys = utils.new_usertype<struct utils::settings::keys>("keys_type",
        "is_awaits_key", &utils::settings::keys::is_awaits_key,
        "event_awaiting_key", &utils::settings::keys::event_awaits_key,
        "sey_key_to", &utils::settings::keys::sey_key_to,
        //"update", &utils::settings::keys::update,
        "events_count", &utils::settings::keys::events_count,
        "get_next_event", &utils::settings::keys::get_next_event
      );
      
      auto settings = utils.new_usertype<utils::settings>("settings_type",
        "graphics", &utils::settings::graphics,
        "game", &utils::settings::game,
        "keys", &utils::settings::keys
      );
      
      utils.set("get_settings", [] (const sol::object &self) {
        (void)self;
        return global::get<utils::settings>();
      });
    }
  }
}
