#include "lua_initialization_hidden.h"

#include "render/shared_render_utility.h"
#include "magic_enum_header.h"
#include "linear_rng.h"
#include "bin/core_structures.h"
#include "lua_environment.h"
#include "globals.h"
#include "systems.h"
#include "localization_container.h"

namespace devils_engine {
  namespace utils {
    static double create_pair_u32f32(const uint32_t &u32, const float &f32) {
      union convert { uint64_t u; double d; };
      const uint32_t &data = glm::floatBitsToUint(f32);
      const uint64_t packed_data = (uint64_t(u32) << 32) | uint64_t(data);
      convert c;
      c.u = packed_data;
      return c.d;
    }
    
    static std::tuple<uint32_t, float> unpack_pair_u32f32(const double &data) {
      union convert { uint64_t u; double d; };
      convert c;
      c.d = data;
      const uint64_t num = c.u;
      const uint32_t data1 = uint32_t(num >> 32);
      const uint32_t data2 = uint32_t(num);
      const float fdata = glm::uintBitsToFloat(data2);
      return std::tie(data1, fdata);
    }
    
    static double make_pair_u32u32(const uint32_t &u1, const uint32_t &u2) {
      union convert { uint64_t u; double d; };
      const uint64_t packed_data = (uint64_t(u1) << 32) | uint64_t(u2);
      convert c;
      c.u = packed_data;
      return c.d;
    }
    
    static std::tuple<uint32_t, uint32_t> unpack_pair_u32u32(const double &d) {
      union convert { uint64_t u; double d; };
      convert c;
      c.d = d;
      const uint64_t num = c.u;
      const uint32_t data1 = uint32_t(num >> 32);
      const uint32_t data2 = uint32_t(num);
      return std::tie(data1, data2);
    }
    
    constexpr bool is_integer(const double num) {
      return std::abs(double(int64_t(num)) - num) < EPSILON;
    }
    
    static std::string compose_string(const std::string_view &sym, const sol::variadic_args &args) {
      // тут мы должны получать строки из локализации и соединять их в одну через пробел (через пробел ли?)
      // правила соединения строк могут быть разными, но с друго стороны эта функция тупо делает компоновку строк 
      // наиболее быстро и эффективно (тут мы хотя бы память заранее задаем), в локализации у нас может вернуться таблица
      // а не строка, то есть таблица с множеством имен в индекс массиве, из индекс массива нам нужно взять имя по индексу
      // а хотя мы можем скармливать строчку вида: id1.id2.3, но нам соответственно нужно будет определить что перед нами число, 
      // а не символ строки, ну хотя нейминг может быть поразному устроен, хотя лучше наверное индекс все же числом получать
      
      auto loc = global::get<systems::core_t>()->loc.get();
      
      size_t argument_counter = 1;
      size_t string_count = 0;
      std::array<std::pair<std::string_view, double>, 512> strings;
      for (const auto &obj : args) {
        const size_t current_arg_index = argument_counter;
        ++argument_counter;
        
        if (obj.get_type() == sol::type::string) {
          const std::string_view &str = obj.as<std::string_view>();
          const size_t index = string_count;
          ++string_count;
          strings[index] = std::make_pair(str, std::numeric_limits<double>::quiet_NaN());
          continue;
        }
        
        if (obj.get_type() == sol::type::table) {
          const sol::table t = obj.as<sol::table>();
          const auto proxy1 = t[1];
          if (!proxy1.valid() || proxy1.get_type() != sol::type::string) {
            throw std::runtime_error("Bad table value, table must contain localization key string and optionally localization table index");
          }
          
          const std::string_view key = proxy1.get<std::string_view>();
          
          size_t index = SIZE_MAX;
          const auto proxy2 = t[2];
          if (proxy2.valid() && proxy2.get_type() == sol::type::number) {
            index = proxy2.get<size_t>();
          }
          
          const auto loc_obj = loc->get(loc->get_current_locale(), key);
          if (loc_obj.get_type() == sol::type::string) {
            const std::string_view &str = loc_obj.as<std::string_view>();
            const size_t index = string_count;
            ++string_count;
            strings[index] = std::make_pair(str, std::numeric_limits<double>::quiet_NaN());
            continue;
          }
          
          if (loc_obj.get_type() == sol::type::number) {
            const double num = loc_obj.as<double>();
            const size_t index = string_count;
            ++string_count;
            strings[index] = std::make_pair("", num);
            continue;
          }
          
          if (loc_obj.get_type() == sol::type::table) {
            if (index == SIZE_MAX) throw std::runtime_error("Localization container returns a table using key '" + std::string(key) + "', expecting next table value to be localization table index");
            
            const sol::table t = loc_obj.as<sol::table>();
            const auto proxy = t[index];
            if (proxy.valid()) {
              if (proxy.get_type() == sol::type::string) {
                const std::string_view str = proxy.get<std::string_view>();
                const size_t index = string_count;
                ++string_count;
                strings[index] = std::make_pair(str, std::numeric_limits<double>::quiet_NaN());
                continue;
              }
              
              if (proxy.get_type() == sol::type::number) {
                const double num = proxy.get<double>();
                const size_t index = string_count;
                ++string_count;
                strings[index] = std::make_pair("", num);
                continue;
              }
              
              throw std::runtime_error("Localization container returns a table using key '" + std::string(key) + "' and index " + std::to_string(index));
            }
          }
          
          // из контейнера локализации к нам может придти только строка, число или таблица
        }
        
        throw std::runtime_error("Bad argument " + std::to_string(current_arg_index) + " type");
      }
      
      const size_t sym_lenght = sym.length();
      size_t char_counter = 0;
      for (size_t i = 0; i < string_count; ++i) {
        char_counter += (strings[i].first.length() == 0 ? 30 : strings[i].first.length()) + (i != string_count-1)*sym_lenght;
      }
      
      size_t offset = 0;
      std::string container(char_counter, '\0');
      for (size_t i = 0; i < string_count; ++i) {
        if (strings[i].first.length() == 0) {
          const double NaN = std::numeric_limits<double>::quiet_NaN();
          ASSERT(*reinterpret_cast<size_t*>(&strings[i].second) != *reinterpret_cast<const size_t*>(&NaN));
          const std::string str_num = is_integer(strings[i].second) ? std::to_string(int64_t(strings[i].second)) : std::to_string(strings[i].second);
          ASSERT(str_num.length() < 30);
          ASSERT(offset + str_num.length() < container.size());
          memcpy(&container[offset], str_num.data(), str_num.length());
          offset += str_num.length();
        } else {
          ASSERT(offset + strings[i].first.length() < container.size());
          memcpy(&container[offset], strings[i].first.data(), strings[i].first.length());
          offset += strings[i].first.length();
        }
        
        if (i != string_count-1) {
          ASSERT(offset + sym.length() < container.size());
          memcpy(&container[offset], sym.data(), sym.length());
          offset += sym.length();
        }
      }
      
      return container;
    }
    
    void setup_lua_safe_utils(sol::state_view lua) {
      auto utils = lua[magic_enum::enum_name<reserved_lua::values>(reserved_lua::utils)].get_or_create<sol::table>();
      utils.set_function("prng32", render::prng);
      utils.set_function("prng32_2", render::prng2);
      utils.set_function("prng_normalize32", render::prng_normalize);
      utils.set_function("prng64", [] (const uint64_t &value) {
        return splitmix64::get_value(splitmix64::rng({value}));
      });
      utils.set_function("prng64_2", [] (const uint64_t &value1, const uint64_t &value2) {
        //if (value1 + value2 == 0) throw std::runtime_error("Summ of prng64_2 args must not be 0"); // ???
        return xoroshiro128starstar::get_value(
          xoroshiro128starstar::rng({
            splitmix64::get_value(splitmix64::rng({value1})), 
            splitmix64::get_value(splitmix64::rng({value2}))
          })
        );
      });
      utils.set_function("prng_normalize64", rng_normalize);
      
      utils.set_function("make_color", [] (const double r, const double g, const double b, const double a) {
        const uint8_t ur = uint8_t(255.0 * glm::clamp(r, 0.0, 1.0));
        const uint8_t ug = uint8_t(255.0 * glm::clamp(g, 0.0, 1.0));
        const uint8_t ub = uint8_t(255.0 * glm::clamp(b, 0.0, 1.0));
        const uint8_t ua = uint8_t(255.0 * glm::clamp(a, 0.0, 1.0));
        const uint32_t c = (uint32_t(ur) << 24) | (uint32_t(ug) << 16) | (uint32_t(ub) << 8) | (uint32_t(ua) << 0);
        return c;
      });
      
      utils.set_function("unpack_color", [] (const uint32_t data) -> std::tuple<double, double, double, double> {
        const uint8_t ur = uint8_t(data >> 24);
        const uint8_t ug = uint8_t(data >> 16);
        const uint8_t ub = uint8_t(data >>  8);
        const uint8_t ua = uint8_t(data >>  0);
        return std::make_tuple(255.0 / double(ur), 255.0 / double(ug), 255.0 / double(ub), 255.0 / double(ua));
      });
      
      utils.set_function("init_array", [] (const size_t &size, sol::object default_value, sol::this_state s) -> sol::table {
        sol::state_view view(s);
        auto t = view.create_table(size, 0);
        if (default_value.is<sol::table>()) {
          for (size_t i = 0; i < size; ++i) {
            t.add(view.create_table(30, 0));
          }
        } else {
          for (size_t i = 0; i < size; ++i) {
            t.add(default_value);
          }
        }
        return t;
      });
      
      utils.set_function("create_table", [] (sol::object arr_size, sol::object hash_size, sol::this_state s) -> sol::table {
        sol::state_view view(s);
        const uint32_t narr = arr_size.is<uint32_t>() ? arr_size.as<uint32_t>() : 100;
        const uint32_t nhash = hash_size.is<uint32_t>() ? hash_size.as<uint32_t>() : 100;
        return view.create_table(narr, nhash);
      });
      
      utils.set_function("create_pair_u32f32", &create_pair_u32f32);
      utils.set_function("unpack_pair_u32f32", &unpack_pair_u32f32);
      
      utils.set_function("create_pair_u32u32", &make_pair_u32u32);
      utils.set_function("unpack_pair_u32u32", &unpack_pair_u32u32);
      
      utils.set_function("compose_string", &compose_string);
      
      // по этой строке я хочу понять какой мне интерфейс делать
      // но думаю что нужно как то иначе это делать, по dpi?
//       utils.set_function("get_platform", [] () -> std::string_view {
// #if defined(__linux__)
//         return "linux";
// #elif defined(WIN32)
//         return "windows";
// #endif
//       });
      
      auto core = lua[magic_enum::enum_name<reserved_lua::values>(reserved_lua::core)].get_or_create<sol::table>();
      core.set_function("type", [] (sol::this_state s, const sol::object &obj) -> std::string {
        if (obj.get_type() != sol::type::userdata) {
          sol::state_view lua = s;
          auto proxy = lua["type"];
          if (!proxy.valid() || proxy.get_type() != sol::type::function) throw std::runtime_error("Could not find function 'type'");
                        
          sol::function f = proxy;
          const auto ret = f(obj);
          if (!ret.valid()) {
            sol::error err = ret;
            std::cout << err.what();
            throw std::runtime_error("There is lua error");
          }
          
          const std::string str = ret;
          return str;
        }
        
        if (obj.is<core::army*>()) return "army";
        else if (obj.is<core::city*>()) return "city";
        else if (obj.is<core::city_type*>()) return "city_type";
        else if (obj.is<core::building_type*>()) return "building_type";
        else if (obj.is<core::hero_troop*>()) return "hero_troop";
        else if (obj.is<core::character*>()) return "character";
        else if (obj.is<core::realm*>()) return "realm";
        else if (obj.is<core::titulus*>()) return "title";
        else if (obj.is<core::province*>()) return "province";
                        
        return "userdata";
      });
      
      core.set_function("clear_module", [] (sol::this_state s, sol::this_environment e, const std::string_view &module_name) -> void {
        sol::state_view lua = s;
        const sol::object nil_obj = sol::make_object(lua, sol::nil);
        sol::environment &env = e;
        env[module_name] = nil_obj;
        
        std::string_view mod_name;
        std::string_view module_path;
        parse_module_name(module_name, mod_name, module_path);
        
        sol::table loaded_table = lua["package"]["loaded"];
        if (!mod_name.empty()) {
          const std::string local_module_name = "module_" + std::string(mod_name) + "_" + std::string(module_path);
          loaded_table[local_module_name] = nil_obj;
        }
      });
    }
  }
}
