#include "battle_lua_states.h"

#include <thread>

namespace devils_engine {
  namespace battle {
    lua_container::lua_container() : states(new sol::state[std::thread::hardware_concurrency()]) { // registered_functions(std::thread::hardware_concurrency())
      
    }
    
    lua_container::~lua_container() {
      registered_functions.clear();
      delete [] states;
      states = nullptr;
    }
    
    uint32_t lua_container::parse_function(const std::string_view &script) {
      const uint32_t threads_count = std::thread::hardware_concurrency();
      const uint32_t index = registered_functions.size();
      for (uint32_t i = 0; i < threads_count; ++i) {
        const auto ret = states[i].safe_script(script);
        if (!ret.valid()) {
          sol::error err = ret;
          std::cout << err.what() << "\n";
          throw std::runtime_error("Could not parse battle script. Script: \n" + std::string(script));
        }
        
        if (ret.get_type() != sol::type::function) throw std::runtime_error("Script must return function. Script: \n" + std::string(script));
        
        sol::function func = ret;
        if (i == 0) registered_functions.emplace_back(threads_count);
        registered_functions[index][i] = func;
      }
      
      return index;
    }
    
    uint32_t lua_container::parse_function(const std::string &path, const std::string_view &name) {
      const uint32_t threads_count = std::thread::hardware_concurrency();
      const uint32_t index = registered_functions.size();
      for (uint32_t i = 0; i < threads_count; ++i) {
        const auto ret = states[i].safe_script_file(path);
        if (!ret.valid()) {
          sol::error err = ret;
          std::cout << err.what() << "\n";
          throw std::runtime_error("Could not parse battle script. Path: " + path);
        }
        
        auto proxy = states[i][name];
        if (!proxy.valid()) throw std::runtime_error("Bad function name. Path: " + path + " name: " + std::string(name));
        if (proxy.get_type() != sol::type::function) throw std::runtime_error("Bad script function. Path: " + path + " name: " + std::string(name));
        
        sol::function func = proxy;
        if (i == 0) registered_functions.emplace_back(threads_count);
        registered_functions[index][i] = func;
      }
      
      return index;
    }
    
    uint32_t lua_container::parse_existing_function(const std::string_view &name) {
      const uint32_t threads_count = std::thread::hardware_concurrency();
      const uint32_t index = registered_functions.size();
      for (uint32_t i = 0; i < threads_count; ++i) {
        auto proxy = states[i][name];
        if (!proxy.valid()) throw std::runtime_error("Bad function name. Name: " + std::string(name));
        if (proxy.get_type() != sol::type::function) throw std::runtime_error("Bad script function. Name: " + std::string(name));
        
        sol::function func = proxy;
        if (i == 0) registered_functions.emplace_back(threads_count);
        registered_functions[index][i] = func;
      }
      
      return index;
    }
  }
}
