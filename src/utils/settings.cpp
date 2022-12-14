#include "settings.h"

#include <iostream>
#include <fstream>
#include <filesystem>

#include "globals.h"
#include "sol.h"
#include "input.h"
#include "render/window.h"
#include "game_enums.h"
#include "systems.h"

#include <GLFW/glfw3.h>

namespace devils_engine {
  namespace utils {
//     void default_settings(settings* s) {
//       s->graphics.width = 1280;
//       s->graphics.height = 720;
//       s->graphics.fullscreen = false;
//       s->graphics.projection = false;
//       
//       s->game.camera_movement = 5.0f;
//     }
    
    const std::vector<std::string_view> tables_keys = {
      "menu", // по идее не жeлательно здесь что то менять
      "world_map",
      "battle",
      "encounter"
    };
    
    settings::graphics::graphics() : width(1280), height(720), video_mode(UINT32_MAX), fullscreen(false), projection(false) {}
    
    size_t settings::graphics::video_modes_count() const {
      auto w = global::get<render::window>();
      auto mon = w->monitor;
      if (mon == nullptr) {
        mon = glfwGetPrimaryMonitor();
      }
      
      int count = 0;
      glfwGetVideoModes(mon, &count);
      return count;
    }
    
    std::tuple<uint32_t, uint32_t, uint32_t> settings::graphics::get_video_mode(const size_t &index) const {
      auto w = global::get<render::window>();
      auto mon = w->monitor;
      if (mon == nullptr) {
        mon = glfwGetPrimaryMonitor();
      }
      
      int count = 0;
      auto modes = glfwGetVideoModes(mon, &count);
      if (count == 0) throw std::runtime_error("Error getting video modes");
      if (index >= size_t(count)) return std::make_tuple(0, 0, 0);
      return std::make_tuple(modes[index].width, modes[index].height, modes[index].refreshRate);
    }
    
    void settings::graphics::find_video_mode() {
      if (video_mode != UINT32_MAX) return;
      auto w = global::get<render::window>();
      auto mon = w->monitor;
      if (mon == nullptr) {
        mon = glfwGetPrimaryMonitor();
      }
      
      int count = 0;
      auto modes = glfwGetVideoModes(mon, &count);
      auto mode = glfwGetVideoMode(mon);
      
      uint32_t current_mode = UINT32_MAX;
      for (int i = 0; i < count; ++i) {
//         if (uint32_t(modes[i].width) == w->surface.extent.width && uint32_t(modes[i].height) == w->surface.extent.height && uint32_t(modes[i].refreshRate) == w->refresh_rate()) {
//           current_mode = i;
//         }
        
        if (modes[i].width == mode->width && modes[i].height == mode->height && modes[i].refreshRate == mode->refreshRate) {
          current_mode = i;
          break;
        }
        
//         if (&modes[i] == mode) {
//           current_mode = i;
//           break;
//         }
      }
      
      ASSERT(current_mode != UINT32_MAX);
      video_mode = current_mode;
    }
    
    void settings::graphics::apply() {
      auto w = global::get<render::window>();
      if (w->flags.fullscreen() != fullscreen) {
        w->toggle_fullscreen();
//         width = w->surface.extent.width;
//         height = w->surface.extent.height;
//         video_mode = UINT32_MAX;
//         find_video_mode();
        ASSERT(w->flags.fullscreen() == fullscreen);
      }
      
      // а как переключить видео мод?
//       if () {
//         
//       }
      
      const auto [window_width, window_height] = w->size();
      if (width != uint32_t(window_width) && height != uint32_t(window_height)) {
        //w->recreate(width, height);
        glfwSetWindowSize(w->handle, width, height);
//         width = w->surface.extent.width;
//         height = w->surface.extent.height;
      }
    }
    
    settings::game::game() : camera_movement(5.0f), camera_movement_x(1.0f), camera_movement_y(1.0f), sens(1.0f), sens_x(1.0f), sens_y(1.0f), target_fps(500.0f), game_cursor(false) {}
    settings::keys::keys() : container_for_event(0), container_for_iteration(0) {}
    
    void settings::keys::setup_default_mapping() {
      auto core = global::get<systems::core_t>();
      
      {
        auto local_mapping = core->keys_mapping[player::in_menu];
        input::set_key_map(local_mapping);
        
        const utils::id menu_next = utils::id::get("menu_next");
        const utils::id menu_prev = utils::id::get("menu_prev");
        const utils::id menu_increase = utils::id::get("menu_increase");
        const utils::id menu_decrease = utils::id::get("menu_decrease");
        const utils::id menu_choose = utils::id::get("menu_choose");
        const utils::id escape = utils::id::get("escape");
        input::set_key(GLFW_KEY_DOWN, menu_next);
        input::set_key(GLFW_KEY_UP, menu_prev);
        input::set_key(GLFW_KEY_RIGHT, menu_increase);
        input::set_key(GLFW_KEY_LEFT, menu_decrease);
        input::set_key(GLFW_KEY_ENTER, menu_choose);
        input::set_key(GLFW_KEY_ESCAPE, escape);
      }
      
      {
        auto local_mapping = core->keys_mapping[player::on_global_map];
        input::set_key_map(local_mapping);
        
        const utils::id map_move = utils::id::get("map_move");
        input::set_key(GLFW_MOUSE_BUTTON_MIDDLE, map_move);
        const utils::id control_click  = utils::id::get("control_click");
        input::set_key(GLFW_MOUSE_BUTTON_RIGHT, control_click);
        const utils::id activate_click = utils::id::get("activate_click");
        input::set_key(GLFW_MOUSE_BUTTON_LEFT, activate_click);

        const auto biome_render_mode = utils::id::get("biome_render_mode");
        const auto cultures_render_mode = utils::id::get("cultures_render_mode");
        const auto culture_groups_render_mode = utils::id::get("culture_groups_render_mode");
        const auto religions_render_mode = utils::id::get("religions_render_mode");
        const auto religion_groups_render_mode = utils::id::get("religion_groups_render_mode");
        const auto provinces_render_mode = utils::id::get("provinces_render_mode");
        const auto countries_render_mode = utils::id::get("countries_render_mode");
        const auto duchies_render_mode = utils::id::get("duchies_render_mode");
        const auto kingdoms_render_mode = utils::id::get("kingdoms_render_mode");
        const auto empires_render_mode = utils::id::get("empires_render_mode");
        input::set_key(GLFW_KEY_F1, biome_render_mode);
        input::set_key(GLFW_KEY_F2, cultures_render_mode);
        input::set_key(GLFW_KEY_F3, culture_groups_render_mode);
        input::set_key(GLFW_KEY_F4, religions_render_mode);
        input::set_key(GLFW_KEY_F5, religion_groups_render_mode);
        input::set_key(GLFW_KEY_F6, provinces_render_mode);
        input::set_key(GLFW_KEY_F7, countries_render_mode);
        input::set_key(GLFW_KEY_F8, duchies_render_mode);
        input::set_key(GLFW_KEY_F9, kingdoms_render_mode);
        input::set_key(GLFW_KEY_F10, empires_render_mode);
        
        const utils::id border_render = utils::id::get("border_render");
        input::set_key(GLFW_KEY_B, border_render);
        
        const utils::id go_to_capital = utils::id::get("home");
        input::set_key(GLFW_KEY_HOME, go_to_capital);
      }
      
      {
        auto local_mapping = core->keys_mapping[player::on_battle_map];
        input::set_key_map(local_mapping);
        
        const utils::id map_move = utils::id::get("map_move");
        input::set_key(GLFW_MOUSE_BUTTON_MIDDLE, map_move);
        const utils::id control_click  = utils::id::get("control_click");
        input::set_key(GLFW_MOUSE_BUTTON_RIGHT, control_click);
        const utils::id activate_click = utils::id::get("activate_click");
        input::set_key(GLFW_MOUSE_BUTTON_LEFT, activate_click);
        
        const utils::id go_to_capital = utils::id::get("home");
        input::set_key(GLFW_KEY_HOME, go_to_capital);
      }
      
      {
        auto local_mapping = core->keys_mapping[player::on_hero_battle_map];
        input::set_key_map(local_mapping);
        
        // тут пока ничего
      }
    }
    
    bool settings::keys::is_awaits_key() const {
      return awaiting_key.valid();
    }
    
    utils::id settings::keys::event_awaits_key() const {
      return awaiting_key;
    }
    
    bool settings::keys::sey_key_to(const uint32_t &container_for_event, const utils::id &event_id) {
      if (is_awaits_key()) return false;
      
      if (container_for_event >= player::states_count || container_for_event == 0) throw std::runtime_error("Bad key mapping container");
      
      awaiting_key = event_id;
      this->container_for_event = container_for_event;
      return true;
    }
    
    // мне нужно убрать тогда кнопку у других эвентов
    void settings::keys::update(const int key) {
      if (!is_awaits_key()) return;
      
      auto core = global::get<systems::core_t>();
      auto keys = core->keys_mapping[container_for_event];
      input::set_key_map(keys);
      input::set_key(key, awaiting_key);
      awaiting_key = utils::id();
    }
    
    size_t settings::keys::events_count(const uint32_t &container_for_iteration) const {
      auto core = global::get<systems::core_t>();
      if (container_for_iteration >= player::states_count || container_for_iteration == 0) throw std::runtime_error("Bad key mapping container");
      auto keys = core->keys_mapping[container_for_iteration];
      return keys->events_map.size();
    }
    
    // нужно сделать нормальную итерацию по клавишам, причем мы еще должны учитывать эвенты которые не используем
    // видимо придется завести еще один массив
    std::tuple<utils::id, int, int> settings::keys::get_next_event(const uint32_t &container) {
      auto core = global::get<systems::core_t>();
      if (container >= player::states_count || container == 0) throw std::runtime_error("Bad key mapping container");
      auto keys = core->keys_mapping[container];
      
      if (container_for_iteration != container || current_iterator == keys->events_map.end()) {
        container_for_iteration = container;
        current_iterator = keys->events_map.begin();
      }
      
      const auto data = std::make_tuple(current_iterator->first, current_iterator->second.keys[0], current_iterator->second.keys[1]);
      ++current_iterator;
      return data;
    }
    
    settings::settings() : dumped(false) {}
    settings::~settings() { if (!dumped) dump_settings(); }
    
    void settings::load_settings(const std::string &path) {
      std::string final_path;
      if (path.empty()) {
        final_path = global::root_directory() + "settings.lua";
      } else {
        final_path = path;
      }
      
      //default_settings(this);
      keys.setup_default_mapping();
      
      std::filesystem::directory_entry e(final_path);
      if (!e.exists() || !e.is_regular_file()) return;
      
      sol::state s;
      auto ret = s.script_file(final_path);
      if (!ret.valid()) {
        std::cout << "Bad settings script, loading default settings" << "\n";
        return;
      }
      
      if (ret.get_type() != sol::type::table) {
        std::cout << "Bad settings script, loading default settings" << "\n";
        return;
      }
      
      sol::table raw_setting = ret;
      if (const auto proxy = raw_setting["graphics"]; proxy.valid() && proxy.get_type() == sol::type::table) {
        const sol::table graphics = proxy.get<sol::table>();
        const auto width_proxy = graphics["width"];
        const auto height_proxy = graphics["height"];
        const auto video_mode_proxy = graphics["video_mode"];
        const auto fullscreen_proxy = graphics["fullscreen"];
        const auto projection_proxy = graphics["projection"];
        this->graphics.width = width_proxy.valid() && width_proxy.get_type() == sol::type::number ? width_proxy.get<uint32_t>() : this->graphics.width;
        this->graphics.height = height_proxy.valid() && height_proxy.get_type() == sol::type::number ? height_proxy.get<uint32_t>() : this->graphics.height;
        this->graphics.video_mode = video_mode_proxy.valid() && video_mode_proxy.get_type() == sol::type::number ? video_mode_proxy.get<uint32_t>() : this->graphics.video_mode;
        this->graphics.fullscreen = fullscreen_proxy.valid() && fullscreen_proxy.get_type() == sol::type::boolean ? fullscreen_proxy.get<bool>() : this->graphics.fullscreen;
        this->graphics.projection = projection_proxy.valid() && projection_proxy.get_type() == sol::type::number ? bool(projection_proxy.get<uint32_t>()) : this->graphics.projection;
      }
      
      if (const auto proxy = raw_setting["game"]; proxy.valid() && proxy.get_type() == sol::type::table) {
        const sol::table game = proxy.get<sol::table>();
        const auto camera_movement   = game["camera_movement"];
        const auto camera_movement_x = game["camera_movement_x"];
        const auto camera_movement_y = game["camera_movement_y"];
        const auto sens   = game["sens"];
        const auto sens_x = game["sens_x"];
        const auto sens_y = game["sens_y"];
        const auto target_fps  = game["target_fps"];
        const auto game_cursor = game["game_cursor"];
        this->game.camera_movement   = camera_movement.valid()   && camera_movement.get_type()   == sol::type::number ? camera_movement.get<float>()   : this->game.camera_movement;
        this->game.camera_movement_x = camera_movement_x.valid() && camera_movement_x.get_type() == sol::type::number ? camera_movement_x.get<float>() : this->game.camera_movement_x;
        this->game.camera_movement_y = camera_movement_y.valid() && camera_movement_y.get_type() == sol::type::number ? camera_movement_y.get<float>() : this->game.camera_movement_y;
        this->game.sens   = sens.valid()   && sens.get_type()   == sol::type::number ? sens.get<float>()   : this->game.sens;
        this->game.sens_x = sens_x.valid() && sens_x.get_type() == sol::type::number ? sens_x.get<float>() : this->game.sens_x;
        this->game.sens_y = sens_y.valid() && sens_y.get_type() == sol::type::number ? sens_y.get<float>() : this->game.sens_y;
        this->game.target_fps  = target_fps.valid()  && target_fps.get_type()  == sol::type::number ? target_fps.get<float>()  : this->game.target_fps;
        this->game.game_cursor = game_cursor.valid() && game_cursor.get_type() == sol::type::number ? game_cursor.get<float>() : this->game.game_cursor;
      }
      
      ASSERT(tables_keys.size() == player::states_count);
      auto core = global::get<systems::core_t>();
      
      if (const auto proxy = raw_setting["keys"]; proxy.valid() && proxy.get_type() == sol::type::table) {
        const sol::table keys = proxy.get<sol::table>();
        // тут у нас будет несколько таблиц, будем пропускать таблицу menu
        for (size_t i = 1; i < tables_keys.size(); ++i) {
          auto proxy = keys[tables_keys[i]];
          if (!proxy.valid() || proxy.get_type() != sol::type::table) continue;
          
          input::set_key_map(core->keys_mapping[i]);
          const sol::table key_table = proxy;
          for (const auto &obj : key_table) {
            if (!obj.second.is<sol::table>()) continue;
            
            const sol::table table = obj.second.as<sol::table>();
            ASSERT(table[0] == sol::nil);
            ASSERT(table[1].get_type() == sol::type::string);
            ASSERT(table[2].get_type() == sol::type::number);
            ASSERT(table[3].get_type() == sol::type::number);
            
            const std::string id = table[1];
            const int key_1 = table[2];
            const int key_2 = table[3];
            
            ASSERT(!id.empty());
            
            const utils::id final_id = utils::id::get(id);
            input::set_key(key_1, final_id, 0); // if (key_1 != INT32_MAX) 
            input::set_key(key_2, final_id, 1); // if (key_2 != INT32_MAX) 
          }
        }
      }
    }
    
    void settings::dump_settings(const std::string &path) {
      sol::state s;
      s.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::table);
      const std::string &root_path = global::root_directory();
      const std::string script_path = root_path + "scripts/";
//       auto serializator = s.require_file("serpent", script_path + "serpent.lua", false);
//       
//       //const auto serpent = s["serpent"];
//       if (!serializator.is<sol::table>()) throw std::runtime_error("Could not loading serializator");
//       const auto serpent = serializator.as<sol::table>();
//       const auto block_proxy = serpent["block"];
//       if (!block_proxy.valid() || block_proxy.get_type() != sol::type::function) throw std::runtime_error("Bad serializator function");
//       const auto block_func = block_proxy.get<sol::function>();
      
//       auto opts = s.create_table();
//       opts["compact"] = false;
//       opts["fatal"] = true;
//       opts["comment"] = false;
      
      sol::table target = s.create_table();
      auto graphics = target["graphics"].get_or_create<sol::table>();
      graphics["width"] = this->graphics.width;
      graphics["height"] = this->graphics.height;
      graphics["video_mode"] = this->graphics.video_mode;
      graphics["fullscreen"] = this->graphics.fullscreen;
      graphics["projection"] = this->graphics.projection;
      auto game = target["game"].get_or_create<sol::table>();
      game["camera_movement"] = this->game.camera_movement;
      game["camera_movement_x"] = this->game.camera_movement_x;
      game["camera_movement_y"] = this->game.camera_movement_y;
      game["sens"] = this->game.sens;
      game["sens_x"] = this->game.sens_x;
      game["sens_y"] = this->game.sens_y;
      game["target_fps"] = this->game.target_fps;
      game["game_cursor"] = this->game.game_cursor;
      
      auto core = global::get<systems::core_t>();
      auto keys = target["keys"].get_or_create<sol::table>();
      for (size_t i = 1; i < tables_keys.size(); ++i) {
        auto current_mapping = keys[tables_keys[i]].get_or_create<sol::table>();
        auto local_mapping = core->keys_mapping[i];
        const auto &event_keys = local_mapping->events_map;
        size_t lua_counter = 1;
        for (const auto &key_data : event_keys) {
          sol::table key_data_table = s.create_table(3, 0);
          //key_data_table.clear();
//           key_data_table.add(key_data.first.name());
//           key_data_table.add(key_data.second.keys[0]);
//           key_data_table.add(key_data.second.keys[1]);
          key_data_table[1] = key_data.first.name();
          key_data_table[2] = key_data.second.keys[0];
          key_data_table[3] = key_data.second.keys[1];
          current_mapping[lua_counter] = key_data_table;
          ++lua_counter;
        }
      }
      
      auto ret = s.require_file("serpent", global::root_directory() + "scripts/serpent.lua", false);
      if (!ret.valid()) {
        throw std::runtime_error("Could not load serpent.lua");
      }
      
      auto serpent = ret.as<sol::table>();
      auto proxy = serpent["dump"];
      if (!proxy.valid() || proxy.get_type() != sol::type::function) throw std::runtime_error("Bad serpent table");
      auto serpent_dump = proxy.get<sol::function>();
      auto serpent_opts = s.create_table_with(
        "compact", true,
        "fatal", true,
        //"sparse", true,
        "nohuge", true,
        "nocode", true,
        "sortkeys", false,
        "comment", false,
        "valtypeignore", s.create_table_with(
          "function", true,
          "thread", true,
          "userdata", true
        )
      );
      
      const auto func_ret = serpent_dump(target, serpent_opts);
      if (!func_ret.valid()) {
        sol::error err = func_ret;
        std::cout << err.what();
        throw std::runtime_error("There is lua errors");
      }
      
      const std::string value = func_ret.get<std::string>(); // "return " + 
      std::string final_path = path;
      if (path.empty()) {
        final_path = root_path + "settings.lua";
      }
      
      const std::string_view comment = R"comment(--[[
  Auto generated settings. sens*, target_fps, game_cursor is not using now. Keys is experimental
]]
)comment";
      std::ofstream file(final_path, std::ios::out | std::ios::trunc);
      file.write(comment.data(), comment.size());
      file.put('\n');
      file.write(value.c_str(), value.size());
      
      dumped = true;
    }
  }
}
