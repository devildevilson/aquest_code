#include "lua_environment.h"

#include <filesystem>
#include <iostream>

#include "magic_enum.hpp"
#include "lua_init/lua_initialization.h"
#include "reserved_lua_table_names.h"
#include "assert.h"
#include "globals.h"

namespace devils_engine {
  namespace utils {
    void make_environment(sol::state_view lua, sol::environment& env) {
      env["_G"] = env; 
      lua["security_env"] = env;
      
      const std::initializer_list<std::string_view> whitelisted = {
        "assert",
        "error",
        "ipairs",
        "next",
        "pairs",
        "pcall",
        "print",
        "select",
        "tonumber",
        "tostring",
        "type",
        "_VERSION",
        "xpcall",

        // эти функции могут обойти метатаблицы, но кажется ничего больше
        "rawequal",
        "rawget",
        "rawset",
        "rawlen",
        "setmetatable",
        "getmetatable",
        
        // по идее тоже безопасный метод
        "collectgarbage"
      };
      
      for (const auto &name : whitelisted) {
        sol::object obj = lua[name]; // прокси добавить не получится
        env[name] = obj;
      }
      
      const std::initializer_list<std::string_view> safe_libraries = {"coroutine", "string", "utf8", "table", "math"}; // "math" - тут нужно отключить матх.рандом
      
      for (const auto &name : safe_libraries) {
        auto copy = lua.create_table(0, 0);
        auto proxy = lua[name];
        if (!proxy.valid()) continue;
        
        auto t = proxy.get<sol::table>();
        for (const auto &pair : t) {
          // first is the name of a function in module, second is the function
          copy[pair.first] = pair.second;
        }
        
        env[name] = copy;
        if (name == "math") {
          env[name]["random"] = sol::nil;
          env[name]["randomseed"] = sol::nil;
        }
      }
      
      // вообще можно получить функции из стейта, но тогда они не удалятся при разрушении стейта
      // нужно ли мне это вообще? скорее всего было бы полезно для интерфейса, и больше ни для чего наверное
      // нужно создать список таблиц в которых я храню юзер типы
      
      for (size_t i = 0; i < utils::reserved_lua::count; ++i) {
        const auto name = magic_enum::enum_name<utils::reserved_lua::values>(static_cast<utils::reserved_lua::values>(i));
        auto proxy = lua[name];
        if (!proxy.valid()) continue;
        
        // копия почему то не работает
//         PRINT(name)
//         auto copy = lua.create_table(0, 0);
        auto t = proxy.get<sol::object>();
//         for (const auto &pair : t) {
//           copy[pair.first] = pair.second;
//         }
        
        //env[name] = copy; // тут еще нужно будет узнать что мне нужно а что нет
        env[name] = t;
      }
    }
    
    std::filesystem::path make_module_path(const std::string_view &module) {
      std::filesystem::path p;
      size_t pos = 0;
      while (pos != std::string::npos) {
        const size_t new_pos = module.find('.', pos);
        const auto str = module.substr(pos, new_pos-pos);
        p /= str;
        pos = new_pos == std::string::npos ? new_pos : new_pos+1;
      }
      
      return p;
    }
    
    std::filesystem::path check_path(const std::filesystem::path &path) {
      if (path.empty()) return std::filesystem::path();
      
      auto first_itr = path.begin();
      ASSERT(*first_itr == "apates_quest");
      std::filesystem::path new_path = global::root_directory();
      for (auto itr = ++first_itr; itr != path.end(); ++itr) {
        new_path /= *itr;
      }
      
      //std::cout << new_path << "\n";
      std::filesystem::directory_entry e(new_path);
      if (!e.exists()) return std::filesystem::path();
      if (!e.is_regular_file()) return std::filesystem::path();
      
      return new_path;
    }
    
    void parse_module_name(const std::string_view &name, std::string_view &mod_name, std::string_view &module_path) {
      const size_t index = name.find_first_of('.');
      if (index == std::string_view::npos) {
        mod_name = "";
        module_path = name;
        return;
      }
      
      mod_name = name.substr(0, index);
      module_path = name.substr(index+1, std::string_view::npos);
    }
    
    void add_io_lines(sol::state_view lua, sol::environment &env) {
      auto io = lua.create_table(0, 0);
      // думаю что инпут не нужен
//       io.set_function("input", [] (sol::this_state s, const std::string_view &path) -> bool {
//         const auto &new_path = check_path(path);
//                       
//         sol::state_view lua = s;
//         auto file = lua["io"]["open"](new_path.c_str());
//         if (!file.valid()) return false;
//         lua["io"]["input"](file);
//         
//         return true;
//       });
      
      io.set_function("lines", [] (sol::this_state s, const std::string_view &path, const sol::variadic_args &args) {
        sol::state_view lua = s;
        
        // проблема в том что у меня потом это дело изменится на zip архивы
        // в этом случае мне чуть ли не самому придется писать функцию лайнс (эта функция возвращает итератор)
        const auto &p = check_path(path);
        if (p.empty()) return sol::make_object(lua, sol::nil);
        
        const auto file = lua["io"]["open"](p.c_str());
        if (!file.valid()) return sol::make_object(lua, sol::nil);
        const auto ret = lua["io"]["lines"](file, sol::as_args(args));
        if (!ret.valid()) return sol::make_object(lua, sol::nil);
        const sol::object o = ret;
        return o;
      });
      env["io"] = io;
    }
    
    void add_require(sol::state_view lua, sol::environment &env) {
      env.set_function("require", [] (sol::this_state s, sol::this_environment e, const std::string_view &module_name) {
        // module_name должен быть вида mod_name/module (+ можно добавить обычные вещи типа: io, table, math и ...)
        // mod_name - указываем архив мода из которого грузим, в архиве что? как архив устроен? 
        // по идее мы можем использовать оригинальный dofile
        // архив устроен как то так
        // std::string lua_lib = arch.get_entry(124).readAsText()
        // lua.script(lua_lib, env)
        // сначала смотрим есть ли название в текущем контексте, парсим название, смотрим если ли валидное название 
        // в lua.packages, грузим как скрипт и добавляем в lua.packages, скрипт должен получить такое название 
        // чтобы не пересекаться с другими скриптами с тем же названием но в других модах,
        // с другой стороны можно просто забить и не запоминать скрипт (хотя почему бы и нет)
        
//         std::cout << "require " << module_name << "\n";
        
        sol::state_view lua = s;
        sol::environment &env = e;
        auto proxy = env[module_name];
        if (proxy.valid()) return proxy.get<sol::object>();
                       
        std::string_view mod_name;
        std::string_view module_path;
        parse_module_name(module_name, mod_name, module_path);
        
        const std::string local_module_name = "module_" + std::string(mod_name) + "_" + std::string(module_path);
        
//         std::cout << "local_module_name " << local_module_name << "\n";
        
        sol::table loaded_table = lua["package"]["loaded"];
        auto lproxy = loaded_table[local_module_name];
//         std::cout << "found " << lproxy.valid() << "\n";
//         std::cout << "table (" << size_t(sol::type::table) << ") type " << size_t(lproxy.get_type()) << "\n";
//         if (local_module_name == "module_apates_quest_scripts.queue" && lproxy.valid()) {
//           auto obj = lproxy.get<sol::object>();
//           auto table = obj.as<sol::table>();
//           auto ret = table["new"]();
//           if (!ret.valid()) {
//             sol::error err = ret;
//             std::cout << err.what();
//             throw std::runtime_error("lua error");
//           }
//         }
        if (lproxy.valid()) return lproxy.get<sol::object>();
        
        // тут проверим путь
        //const auto &p = check_path(module_path);
        //const auto &p = check_path(std::string(module_name) + ".lua");
        auto std_module_path = make_module_path(module_name);
        std_module_path += ".lua";
        //PRINT(std_module_path)
        ASSERT(std_module_path.extension() == ".lua");
        const auto &p = check_path(std_module_path);
        assert(!p.empty());
        if (p.empty()) throw std::runtime_error("Could not find module " + std::string(module_name));
                       
//         std::cout << "path " << p << "\n";
        
        auto ret = lua.script_file(p.string(), env);
        if (!ret.valid()) {
          sol::error err = ret;
          std::cout << err.what();
          throw std::runtime_error("Could not load module " + std::string(module_name));
        }
        
//         std::cout << "loaded " << "\n";
        
        sol::object obj = ret;
        loaded_table[local_module_name] = obj;
        
        return obj;
      });
      
      (void)lua;
    }
    
    const std::initializer_list<std::string_view> registered_lua_module = {
      "moonnuklear",
      "fmt"
    };
    
    size_t check_module_name(const std::string_view &module_name) {
      for (size_t i = 0; i < registered_lua_module.size(); ++i) {
        if (registered_lua_module.begin()[i] == module_name) return i;
      }
      
      return SIZE_MAX;
    }
    
    void add_interface_require(sol::state_view lua, sol::environment &env) {
      env.set_function("require", [] (sol::this_state s, sol::this_environment e, const std::string_view &module_name) {
        sol::state_view lua = s;
        sol::environment &env = e;
        auto proxy = env[module_name];
        if (proxy.valid()) return proxy.get<sol::object>();
        
        std::string_view mod_name;
        std::string_view module_path;
        parse_module_name(module_name, mod_name, module_path);
        
        sol::table loaded_table = lua["package"]["loaded"];
        if (!mod_name.empty()) {
          const std::string local_module_name = "module_" + std::string(mod_name) + "_" + std::string(module_path);
          auto lproxy = loaded_table[local_module_name];
          if (lproxy.valid()) return lproxy.get<sol::object>();
        }
                       
        const size_t id = check_module_name(module_name);
        if (id != SIZE_MAX) {
          // еще мы должны проверить хеш того что загрузим
          
          // загрузили ли мы уже этот модуль?
          //auto proxy = lua["package"]["loaded"][module_name];
          //if (proxy.valid()) return proxy.get<sol::object>();
          
          // по идее это само проверит загружен ли модуль
          auto ret = lua["require"](module_name);
          if (!ret.valid()) {
            sol::error err = ret;
            std::cout << err.what();
            throw std::runtime_error("Could not load module");
          }
          
          sol::object obj = ret;
          return obj;
        }
        
//         std::cout << mod_name << "\n";
        if (mod_name.empty() && id == SIZE_MAX) throw std::runtime_error("Bad module name");
        
        auto std_module_path = make_module_path(module_name);
        std_module_path += ".lua";
        //PRINT(std_module_path)
        ASSERT(std_module_path.extension() == ".lua");
        const auto &p = check_path(std_module_path);
        assert(!p.empty());
        if (p.empty()) throw std::runtime_error("Could not find module " + std::string(module_name));
        
        auto ret = lua.script_file(p.string(), env);
        if (!ret.valid()) {
          sol::error err = ret;
          std::cout << err.what();
          throw std::runtime_error("Could not load module " + std::string(module_name));
        }
        
        const std::string local_module_name = "module_" + std::string(mod_name) + "_" + std::string(module_path);
        sol::object obj = ret;
        loaded_table[local_module_name] = obj;
        
        return obj;
      });
      
      (void)lua;
    }
  }
}
