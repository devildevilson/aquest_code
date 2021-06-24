#include "interface_container2.h"

#include "bin/interface_context.h"
#include "lua_environment.h"
#include "lua_initialization.h"
#include "game_context.h"
#include "globals.h"

#include <iostream>
#include <tuple>

const std::string_view moonnuklear_functions = 
R"solon(
  
local nk = require("moonnuklear")
local function init_nk_context(raw_context) return nk.init_from_ptr(raw_context) end
local function init_nk_font(raw_font) return nk.font_from_ptr(raw_font) end
local function free_nk_font(moon_font) moon_font:free() end
return init_nk_context, init_nk_font, free_nk_font

)solon";

namespace devils_engine {
  namespace utils {
    interface_container::timer::timer() : m_current_time(0), m_user_time(0), m_accumulated_time(0) {}
    void interface_container::timer::update(const size_t &time) {
      m_current_time = time;
      m_user_time += time;
      m_accumulated_time += time;
    }
    
    void interface_container::timer::reset() { m_user_time = 0; }
    size_t interface_container::timer::current_time() const { return m_current_time; }
    size_t interface_container::timer::user_time() const { return m_user_time; }
    size_t interface_container::timer::accumulated_time() const { return m_accumulated_time; }
    
    interface_container::interface_container(const create_info &info) : ctx(info.ctx), game_ctx(info.game_ctx), env(lua, sol::create), local_table(lua, sol::create) {
      lua.open_libraries(sol::lib::base, sol::lib::table, sol::lib::math, sol::lib::string, sol::lib::utf8, sol::lib::bit32, sol::lib::package, sol::lib::coroutine);
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
      utils::setup_lua_interface_utils(lua);
      utils::setup_lua_types(lua);
      utils::setup_lua_camera(lua);
      utils::setup_lua_selection(lua);
      
      auto utils = lua["utils"].get_or_create<sol::table>();
      utils.new_usertype<timer>("frame_timer", sol::no_constructor,
        "reset", &interface_container::timer::reset,
        "current_time", &interface_container::timer::current_time,
        "user_time", &interface_container::timer::user_time,
        "accumulated_time", &interface_container::timer::accumulated_time
      );
      
      make_environment(lua, env);
      add_interface_require(lua, env);
      
      auto ret = lua.safe_script(moonnuklear_functions, env);
      if (!ret.valid()) {
        sol::error err = ret;
        std::cout << err.what();
        throw std::runtime_error("There is lua errors");
      }
      
      const auto &funcs = ret.get<std::tuple<sol::function, sol::function, sol::function>>();
      
      get_ctx = std::get<0>(funcs);
      get_font = std::get<1>(funcs);
      free_font = std::get<2>(funcs);
      
      if (get_ctx == sol::nil || get_font == sol::nil || free_font == sol::nil) throw std::runtime_error("Interface functions are nil");
      
      const auto nk_ctx = sol::make_light(ctx->ctx);
      moonnuklear_ctx = get_ctx(nk_ctx);
      get_fonts();
      
      {
        auto ret = lua.require_file("serpent", global::root_directory() + "scripts/serpent.lua", false);
        if (!ret.valid()) {
          throw std::runtime_error("Could not load serpent.lua");
        }
        
        serpent = ret.as<sol::table>();
        auto proxy = serpent["line"];
        if (!proxy.valid() || proxy.get_type() != sol::type::function) throw std::runtime_error("Bad serpent table");
        serpent_line = proxy.get<sol::function>();
      }
    }
    
    void interface_container::load_interface_file(const std::string_view &path) {
      auto ret = lua.safe_script_file(std::string(path), env);
      if (!ret.valid()) {
        sol::error err = ret;
        std::cout << err.what();
        throw std::runtime_error("There is lua errors");
      }
      
      // если луа запускает скрипт, то зачем нам отдлельно инит?
      
//       const auto &funcs = ret.get<std::tuple<sol::function, sol::function>>();
//       
//       sol::function init_func = std::get<0>(funcs);
//       update_func = std::get<1>(funcs);
//       
//       if (update_func == sol::nil) throw std::runtime_error("Interface update function is nil");
//       
//       env.set_on(init_func);
//       env.set_on(update_func);
//       
//       if (init_func.valid()) {
//         init_func();
//       }

      const auto &funcs = ret.get<sol::function>();
      update_func = funcs;
      
      if (update_func == sol::nil) throw std::runtime_error("Interface update function is nil");
      
      env.set_on(update_func);
      lua.collect_garbage();
    }
    
    void interface_container::update_loading_table(const sol::table &t) {
      if (t.lua_state() != lua) throw std::runtime_error("Table from another state");
      local_table["loading_table"] = t;
    }
    
    void interface_container::update_post_generating_table(const sol::table &t) {
      if (t.lua_state() != lua) throw std::runtime_error("Table from another state");
      local_table["post_generation_table"] = t;
    }
    
    void interface_container::setup_map_generator_functions(const sol::table &t) {
      if (t.lua_state() != lua) throw std::runtime_error("Table from another state");
      local_table["map_generator"] = t;
    }
    
    void interface_container::clear_map_generator_functions() {
      local_table["map_generator"] = sol::nil;
    }
    
    void interface_container::update(const size_t &time) {
      t.update(time);
      const auto ret = update_func(moonnuklear_ctx, game_ctx, &t, local_table);
      if (!ret.valid()) {
        sol::error err = ret;
        std::cout << err.what();
        throw std::runtime_error("Interface function error");
      }
      
      local_table["loading_table"] = sol::nil;
      local_table["post_generation_table"] = sol::nil;
      //local_table["generator_table"] = sol::nil;
    }
    
    sol::object interface_container::get_serializable_table() const {
      return local_table["serializable_table"];
    }
    
    sol::object interface_container::get_persistent_table() const {
      return local_table["persistent_table"];
    }
    
    void interface_container::set_persistent_table(const sol::table &t) {
      local_table["persistent_table"] = t;
    }
    
    sol::object interface_container::get_generator_table() const {
      return local_table["generator_userdata"];
    }
    
    std::string interface_container::serialize_table(const sol::table &t) {
      if (!t.valid() || t.empty()) return "";
      if (t.lua_state() != lua) throw std::runtime_error("Table from another state");
      
      auto opts = lua.create_table_with(
        "compact", true,
        "fatal",   true, 
        "comment", false
      );
      
      auto ret = serpent_line(t, opts);
      if (!ret.valid()) {
        sol::error err = ret;
        std::cout << err.what();
        throw std::runtime_error("Could not serialize table");
      }
      
      std::string str = ret;
      return str;
    }
    
    sol::table interface_container::deserialize_table(const std::string &str) {
      auto ret = lua.script("return " + str);
      if (!ret.valid()) {
        sol::error err = ret;
        std::cout << err.what();
        throw std::runtime_error("Could not deserialize string");
      }
      
      sol::table t = ret;
      return t;
    }
    
    void interface_container::get_fonts() {
      for (uint32_t i = 0; i < fonts::count; ++i) {
        const auto font = sol::make_light(ctx->fonts[i]);
        fonts[i] = get_font(font);
      }
    }
    
    void interface_container::free_fonts() {
      for (uint32_t i = 0; i < fonts::count; ++i) {
        free_font(fonts[i]);
        fonts[i] = sol::make_object(lua, sol::nil);
      }
    }
  }
}
