#include "system.h"

#include "utils/globals.h"
#include "utils/lua_environment.h"
#include "utils/interface_container2.h"

namespace devils_engine {
  namespace generator {
    step::step() noexcept {}
    step::step(std::string name, std::vector<map::generator_pair> pairs) noexcept : pairs(std::move(pairs)), name(std::move(name)) {}
    
    system::system() :
      lua(),
      env_lua(lua, sol::create),
      table(lua.create_table()),
      scripts_needs_to_update(false),
      rand_seed(1),
      noise_seed(1),
      cur_step(0)
    {
      lua.open_libraries(sol::lib::base, sol::lib::table, sol::lib::math, sol::lib::package, sol::lib::string, sol::lib::utf8, sol::lib::coroutine);
      table["userdata"] = lua.create_table();
      
      const std::string path = global::root_directory() + "scripts/";
      {
        auto ret = lua.require_file("serpent", path + "serpent.lua", false);
        if (!ret.valid()) {
          throw std::runtime_error("Could not load serpent.lua");
        }
        
        auto serpent = ret.as<sol::table>();
        auto proxy = serpent["dump"];
        if (!proxy.valid() || proxy.get_type() != sol::type::function) throw std::runtime_error("Bad serpent table");
        serpent_line = proxy.get<sol::function>();
        serpent_opts = lua.create_table_with(
          "compact", true,
          "fatal", true,
          //"sparse", true,
          "nohuge", true,
          "nocode", true,
          "sortkeys", false,
          "comment", false,
          "valtypeignore", lua.create_table_with(
            "function", true,
            "thread", true,
            "userdata", true
          )
        );
      }
      
      utils::make_environment(lua, env_lua);
      utils::add_io_lines(lua, env_lua);
      utils::add_require(lua, env_lua);
      
      
    }
    
    std::string system::serialize_table(const sol::table &t) {
      if (!t.valid() || t.empty()) return "";
      if (t.lua_state() != lua) throw std::runtime_error("Table from another state");
      
      auto ret = serpent_line(t, serpent_opts);
      if (!ret.valid()) {
        sol::error err = ret;
        std::cout << err.what();
        throw std::runtime_error("Could not serialize table");
      }
      
      std::string str = ret;
      return str;
    }
    
    sol::table system::deserialize_table(const std::string &str) {
      auto ret = lua.script(str); // теперь строка хранится в виде скрипта (do local _={};return _;end)
      if (!ret.valid()) {
        sol::error err = ret;
        std::cout << err.what();
        throw std::runtime_error("Could not deserialize string");
      }
      
      sol::table t = ret;
      return t;
    }
    
    void system::set_userdata_table(const sol::table &t) {
      table["userdata"] = t;
    }
    
    sol::object system::get_post_generation_table() const {
      return table["post_generation"];
    }
    
//     void setup_map_generator_functions(utils::interface_container* interface) {
//       
//     }
  }
}
