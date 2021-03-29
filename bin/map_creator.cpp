#include "map_creator.h"
// #include "interface_context.h"
#include "utils/globals.h"
#include "fmt/format.h"
#include "generator_system2.h"
#include "map_generators2.h"
// #include "utils/random_engine.h"
// #include "FastNoise.h"
#include "generator_container.h"
// #include "render/render_mode_container.h"
#include "utils/thread_pool.h"
#include "utils/interface_container.h"
#include "utils/progress_container.h"
#include "map.h"
#include "utils/lua_initialization.h"
#include "loading_functions.h"

#include <stdexcept>
#include <random>
#include <iostream>
#include <filesystem>
#include <regex>

namespace devils_engine {
  namespace map {
    const char random_seed_chars1[] = {
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };
    
    const char random_seed_chars2[] = {
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
    };
    
    const size_t hex_num_count = sizeof(random_seed_chars1) / sizeof(random_seed_chars1[0]);
    static_assert(hex_num_count == 16);
    static_assert(hex_num_count == sizeof(random_seed_chars2) / sizeof(random_seed_chars2[0]));
    
    constexpr uint64_t get_seed_from_string(const std::string_view &str) {
      uint64_t ret = 0;
      
      // у нас в строке 17 символов, последний символ '/0', он учитываться не должен
      // но размер строки все равно показывает 16
      assert(str.length() <= hex_num_count);
      const size_t last_index = str.length()-1;
      for (size_t s = 0; s < str.length(); ++s) {
        const auto c = str[s];
        bool found = false;
        for (size_t i = 0; i < hex_num_count; ++i) {
          if (c == random_seed_chars1[i] || c == random_seed_chars2[i]) {
            // начинаем с левой стороны числа поэтому (15-s)
            ret = ret | (i << (last_index-s) * 4);
            found = true;
            break;
          }
        }

        assert(found);
      }

      return ret;
    }
    
    std::string get_string_from_seed(const uint64_t &seed) {
      std::string ret(17, '\0');
      const size_t mask = 0xf;
      
      const size_t last_index = ret.length()-2;
      for (size_t s = 0; s < ret.length(); ++s) {
        // начинаем с левой стороны числа поэтому (15-s)
        const size_t num = (seed >> (last_index-s) * 4) & mask;
        ASSERT(num < hex_num_count);

        ret[s] = random_seed_chars1[num];
      }

      return ret;
    }
    
//     property_int::property_int(const create_info &info) :
//       min(info.min),
//       default_val(info.default_val),
//       max(info.max),
//       step(info.step),
//       pixel_step(info.pixel_step),
//       prop_name(info.prop_name),
//       var_name(info.var_name)
//     {}
//
//     void property_int::draw(sol::table &table) {
//       auto interface = global::get<interface::context>();
//       auto ctx = &interface->ctx;
//
//       nk_layout_row_dynamic(ctx, 30.0f, 1);
//       table["userdata"][var_name] = nk_propertyi(ctx, prop_name.c_str(), min, table["userdata"][var_name], max, step, pixel_step);
//     }
//
//     void property_int::set_default_value(sol::table &table) {
//       table["userdata"][var_name] = default_val;
//     }
//
//     property_float::property_float(const create_info &info) :
//       min(info.min),
//       default_val(info.default_val),
//       max(info.max),
//       step(info.step),
//       pixel_step(info.pixel_step),
//       prop_name(info.prop_name),
//       var_name(info.var_name)
//     {}
//
//     void property_float::draw(sol::table &table) {
//       auto interface = global::get<interface::context>();
//       auto ctx = &interface->ctx;
//
//       nk_layout_row_dynamic(ctx, 30.0f, 1);
//       table["userdata"][var_name] = nk_propertyf(ctx, prop_name.c_str(), min, table["userdata"][var_name], max, step, pixel_step);
//     }
//
//     void property_float::set_default_value(sol::table &table) {
//       table["userdata"][var_name] = default_val;
//     }

//     step::step(const bool first, const size_t &container_size, const std::string &name, const std::vector<map::generator_pair> &pairs, const std::string &rendering_mode) :
//       first(first),
//       container(container_size),
//       pairs(pairs),
//       name(name),
//       rendering_mode(rendering_mode)
//     {}

    //step::step(const sol::function &interface, const std::vector<map::generator_pair> &pairs) : pairs(pairs), interface(interface) {} //const bool first,
    step::step(const std::string &step_name, const std::string &step_function, const std::vector<map::generator_pair> &pairs) : pairs(pairs), m_step_name(step_name), m_step_function(step_function) {}

    step::~step() {
//       for (auto p : variables) {
//         container.destroy(p);
//       }
    }

//     int32_t step::prepare(systems::generator &gen, map::generator::context* context, sol::table &table) {
//       auto interface = global::get<interface::context>();
//       auto ctx = &interface->ctx;
//
//       static uint32_t random_seed = 1;
//       static char buffer[16] = "1";
//       static int32_t buffer_len = 1;
//
//       int32_t code = 0;
//       if (nk_begin(ctx, name.c_str(), nk_rect(5, 5, 400, 400), NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND)) {
//         nk_layout_row_static(ctx, 30.0f, 400, 1);
//         nk_label(ctx, name.c_str(), NK_TEXT_ALIGN_LEFT);
//
//         if (first) {
//           const float ratio[] = {0.6, 0.4};
//           nk_layout_row(ctx, NK_DYNAMIC, 30.0f, 2, ratio);
//           nk_edit_string(ctx, NK_EDIT_SIMPLE, buffer, &buffer_len, 11, nk_filter_decimal);
//           if (nk_button_label(ctx, "Randomize")) {
//             std::random_device dev;
//             random_seed = dev();
//             const std::string str = fmt::format(FMT_STRING("{}"), random_seed);
// //                 std::cout << "Randomized " << str << " size " << str.size() << " length " << str.length() << "\n";
//             memcpy(buffer, str.c_str(), str.size());
//             buffer_len = str.length();
//           }
//
//           buffer[buffer_len] = '\0';
//           const size_t num = atol(buffer);
// //               ASSERT(num != 0);
//
//           if (num > UINT32_MAX) {
//             random_seed = UINT32_MAX;
//           } else {
//             random_seed = num;
//           }
//
//           if (num > UINT32_MAX) {
//             const std::string str = fmt::format(FMT_STRING("{}"), random_seed);
//             memcpy(buffer, str.c_str(), str.size());
//             buffer_len = str.size();
//           }
//         }
//
//         for (auto p : variables) {
//           p->draw(table);
//         }
//
//         nk_layout_row_static(ctx, 30.0f, 199, 2);
//         if (first) nk_spacing(ctx, 1);
//         else {
//           if (nk_button_label(ctx, "Back")) code = -1;
//         }
//
//         if (nk_button_label(ctx, "Generate")) {
//           gen.clear();
//           for (const auto &pair : pairs) {
//             gen.add(pair);
//           }
//
//           if (first) {
//             union transform {
//               uint32_t valu;
//               int32_t vali;
//             };
//
//             transform t;
//             t.valu = random_seed;
//
//             context->random->set_seed(random_seed);
//             context->noise->SetSeed(t.vali);
//           }
//
//           gen.generate(context, table);
//           render::mode(rendering_mode);
//           code = 1;
//         }
//       }
//       nk_end(ctx);
//
//       return code;
//     }

//     const sol::function & step::get_interface() const {
//       return interface;
//     }

    const std::vector<map::generator_pair> & step::get_functions() const {
      return pairs;
    }

    std::string step::step_name() const {
      return m_step_name;
    }

    std::string step::step_function() const {
      return m_step_function;
    }
    
    union convert {
      uint64_t u;
      double d;
    };
    
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
      
//       std::cout << new_path << "\n";
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
    
    void make_environment(sol::state_view lua, sol::environment &env) {
      //env["_G"] = lua.create_table(); 
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
        auto t = lua[name].get<sol::table>();
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
        
        auto file = lua["io"]["open"](p.c_str());
        if (!file.valid()) return sol::make_object(lua, sol::nil);
        auto ret = lua["io"]["lines"](file, sol::as_args(args));
        if (!ret.valid()) return sol::make_object(lua, sol::nil);
        sol::object o = ret;
        return o;
      });
      env["io"] = io;
      
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
        
        sol::state_view lua = s;
        sol::environment &env = e;
        auto proxy = env[module_name];
        if (proxy.valid()) return proxy.get<sol::object>();
                       
        std::string_view mod_name;
        std::string_view module_path;
        parse_module_name(module_name, mod_name, module_path);
        
        const std::string local_module_name = "module_" + std::string(mod_name) + "_" + std::string(module_path);
        
        sol::table loaded_table = lua["package"]["loaded"];
        auto lproxy = loaded_table[local_module_name];
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
        
        auto ret = lua.script_file(p.string(), env);
        if (!ret.valid()) {
          sol::error err = ret;
          std::cout << err.what();
          throw std::runtime_error("Could not load module " + std::string(module_name));
        }
        
        sol::object obj = ret;
        loaded_table[local_module_name] = obj;
        
        return obj;
      });
      
      // вообще можно получить функции из стейта, но тогда они не удалятся при разрушении стейта
      // нужно ли мне это вообще? скорее всего было бы полезно для интерфейса, и больше ни для чего наверное
      // нужно создать список таблиц в которых я храню юзер типы
      
      for (size_t i = 0; i < utils::reserved_lua::count; ++i) {
        const auto name = magic_enum::enum_name<utils::reserved_lua::values>(static_cast<utils::reserved_lua::values>(i));
        auto proxy = lua[name];
        if (!proxy.valid()) continue;
        
//         PRINT(name)
//         auto copy = lua.create_table(0, 0);
        auto t = proxy.get<sol::table>();
//         for (const auto &pair : t) {
//           copy[pair.first] = pair.second;
//         }
        
        //env[name] = copy; // тут еще нужно будет узнать что мне нужно а что нет
        env[name] = t;
      }
    }

    creator::creator(utils::interface_container* interface, core::map* map, core::seasons* seasons, utils::localization* loc) :
      lua(),
      env_lua(lua, sol::create),
      table(lua.create_table()),
      temp_container(core::map::hex_count_d(core::map::detail_level)),
      rand_seed(1),
      noise_seed(1),
      current_step(0),
      old_step(0),
      random(rand_seed),
      noise(noise_seed),
      interface(interface),
      scripts_needs_to_update(false)
    {
      lua.open_libraries(sol::lib::base, sol::lib::table, sol::lib::math, sol::lib::package, sol::lib::string, sol::lib::utf8, sol::lib::coroutine);
      ctx.container = &temp_container;
      ctx.map = map;
      ctx.seasons = seasons;
      ctx.loc = loc;
      ctx.noise = &noise;
      ctx.random = &random;
      ctx.pool = global::get<dt::thread_pool>();

      table["userdata"] = lua.create_table();
      global::get(&m_table_container);
      global::get(&lua);
      
      // не учитывает к сожалению 0x нотацию, хотя можно сделать
      // не, 0x нотация нинужна
//       const std::string_view &test_str = "0000000000000000";
//       ASSERT(get_seed_from_string(test_str) == 0);
//       const std::string_view &test_str2 = "000000000000000f";
//       ASSERT(get_seed_from_string(test_str2) == 15);
//       ASSERT(get_string_from_seed(get_seed_from_string(test_str2)) == test_str2);

      const std::string path = global::root_directory() + "scripts/";
      lua.require_file("serpent", path + "serpent.lua", true); //auto obj =
      
      utils::setup_lua_package_path(lua);
      utils::setup_lua_world_map(lua);
      utils::setup_lua_utility_map_generator_functions(lua);
      utils::setup_lua_generator_container(lua);
      utils::setup_lua_constants(lua);
      utils::setup_lua_random_engine(lua);
      utils::setup_lua_noiser(lua);
      utils::setup_lua_calendar(lua);
      
      make_environment(lua, env_lua);

      // нужно тут создать интерфейс тоже, только по всей видимости почти без данных
      // все таки придется сериализовать таблицу
      interface->lua.require_file("serpent", path + "serpent.lua");
      interface_table = interface->lua.create_table();
      auto gen_table = interface->lua["generator"].get_or_create<sol::table>();
      //gen_table["setup_random_seed"] = [this] (const double &seed) {
      gen_table["setup_random_seed"] = [this] (const std::string_view &str_seed) {
        const uint64_t seed = get_seed_from_string(str_seed);
        
        this->rand_seed = seed;
        this->random.set_seed(seed);
      };

      gen_table["setup_noise_seed"] = [this] (const std::string_view &str_seed) {
        const uint64_t seed = get_seed_from_string(str_seed);
        
        // может заменить на более мелкое число? хотя если дать нижние регистры заполнять, 
        // можно сделать какую нибудь другие способы задания сида
        //this->noise_seed = uint32_t(seed >> 32);
        this->noise_seed = seed;
        this->noise.SetSeed(*reinterpret_cast<const int*>(&this->noise_seed));
      };

      gen_table["get_random_number"] = [] () -> std::string {
        std::random_device dev;
        static_assert(sizeof(std::random_device::result_type) == sizeof(uint32_t));
        const uint64_t tmp = (uint64_t(dev()) << 32) | uint64_t(dev());
        
        // тут нужно бы составить строку размером 16 вида: deadbeafdeadbeaf
        const auto &str = get_string_from_seed(tmp);
        
//         convert c;
//         c.u = tmp;
//         return c.d;
        return str;
      };

      gen_table["set_world_name"] = [this] (const std::string_view &str) {
        if (str.empty()) throw std::runtime_error("Bad world name string");
        world_name = std::string(str);
      };

      gen_table["set_folder_name"] = [this] (const std::string_view &str) {
        if (str.empty()) throw std::runtime_error("Bad world folder name string");
        const std::regex folder_name_regex("^[a-zA-Z0-9_.]+$", std::regex_constants::icase);
        if (!std::regex_match(str.begin(), str.end(), folder_name_regex)) throw std::runtime_error("Folder name must match ^[a-zA-Z0-9_.]+$ expression");
        folder_name = std::string(str);
      };

      gen_table["check_world_existance"] = [] (const std::string_view &str) {
        if (str.empty()) return 0;
        const std::regex folder_name_regex("^[a-zA-Z0-9_.]+$", std::regex_constants::icase);
        if (!std::regex_match(str.begin(), str.end(), folder_name_regex)) return 1;
        const std::filesystem::path p = global::root_directory() + "/saves/" + std::string(str) + "/world_data";
        const std::filesystem::directory_entry e(p);
        if (e.exists()) return 2;
        return 3;
      };
      
      global::get(&string_container);
      const size_t type = static_cast<size_t>(utils::world_map_strings::tile_biome_id);
      string_container.register_strings(type, map->tiles_count());
    }

    creator::~creator() {
      for (auto p : steps) {
        steps_pool.destroy(p);
      }
      steps.clear();

      global::get<table_container_t>(reinterpret_cast<table_container_t*>(SIZE_MAX));
      global::get<sol::state>(reinterpret_cast<sol::state*>(SIZE_MAX));
      global::get<utils::world_map_string_container>(reinterpret_cast<utils::world_map_string_container*>(SIZE_MAX));
      ASSERT(global::get<table_container_t>() == nullptr);

      auto gen_table = interface->lua["generator"].get_or_create<sol::table>();
      gen_table["setup_random_seed"] = sol::nil;
      gen_table["setup_noise_seed"] = sol::nil;
      gen_table["get_random_int"] = sol::nil;
      gen_table["set_world_name"] = sol::nil;
      gen_table["set_folder_name"] = sol::nil;

      for (const auto &name : clearing_sol_state) {
        interface->lua[name] = sol::nil;
        interface->layers_table[name] = sol::nil;
      }
    }

//     step* creator::create(const bool first, const size_t &container_size, const std::string &name, const std::vector<map::generator_pair> &pairs, const std::string &rendering_mode) {
//       auto step = steps_pool.create(first, container_size, name, pairs, rendering_mode);
//       steps.push_back(step);
//       return step;
//     }

//     step* creator::create(const std::string_view &interface_name, const std::vector<map::generator_pair> &pairs) { //const bool first,
//       const sol::function &func = interface->get_state()[interface_name]; // теперь более менее удобно передавать именно название функции
//       clearing_sol_state.insert(std::string(interface_name));
//       std::vector<map::generator_pair> final_pairs;
//       // это первый степ, мы можем сюда добавить функцию бегин
//       if (steps.empty()) final_pairs.push_back(map::default_generator_pairs[0]);
//
//       final_pairs.insert(final_pairs.end(), pairs.begin(), pairs.end());
//       auto step = steps_pool.create(func, final_pairs);
//       steps.push_back(step);
//       return step;
//     }

    step* creator::create(const std::string &step_name, const std::string &interface_name, const std::vector<map::generator_pair> &pairs) {
      clearing_sol_state.insert(interface_name);
      interface->register_function(interface_name, interface_name);
      std::vector<map::generator_pair> final_pairs;
      // это первый степ, мы можем сюда добавить функцию бегин
      if (steps.empty()) final_pairs.push_back(map::default_generator_pairs[0]);

      final_pairs.insert(final_pairs.end(), pairs.begin(), pairs.end());
      auto step = steps_pool.create(step_name, interface_name, final_pairs);
      steps.push_back(step);
      return step;
    }

    sol::table pass_to_other_state(sol::state_view first, const sol::table &table, sol::state_view second, std::string &copy) {
      // таблицу в строку и передаем в другую таблицу
      const auto &serializator = first["serpent"];
      if (!serializator.valid()) throw std::runtime_error("Could not load serializator");
      const sol::function &func = serializator["line"];
      if (!func.valid()) throw std::runtime_error("Bad serializator");
      auto opts = first.create_table();
      opts["compact"] = true;
      opts["fatal"] = true;
      opts["comment"] = false;
      auto ret = func(table, opts);
      if (!ret.valid()) {
        sol::error err = ret;
        std::cout << err.what();
        throw std::runtime_error("Could not serialize lua table");
      }

      std::string value = ret;
      copy = value;
//       PRINT_VAR("ser value", value)
      auto res = second.safe_script("return " + value); // value выглядит так: {table_data1=2334,table_data2=1241}
      if (!res.valid()) {
        sol::error err = res;
        std::cout << err.what();
        throw std::runtime_error("Small script error");
      }
      //if (!second["global_tmp_table"].valid()) throw std::runtime_error("Bad context changing");
      return res;
    }

    void creator::generate() {
      if (old_step != current_step) {
        if (gen.finished()) {
          old_step = current_step;
          interface->close_layer(utils::interface_container::last_layer()); // if (!finished())
          
          lua.collect_garbage();
          return;
        }
        
        if (scripts_needs_to_update) {
          PRINT_VAR("old_step", old_step);
          current_step = old_step;
          interface->close_layer(utils::interface_container::last_layer());
          
          return;
        }

        // у нас же существуют значения в луа по ссылкам? тогда можно не обновлять выходные данные через интерфейс верно?
        auto info_table = interface->lua["tmp_table"].get_or_create<sol::table>();
        info_table["current_step"] = gen.current();
        info_table["hint2"] = gen.hint();
        info_table["step_count"] = gen.size();
        info_table["hint1"] = steps[old_step]->step_name();
        info_table["type"] = utils::progress_container::creating_map;
//         auto res = progress_interface_func(interface->get_ctx(), info_table); // контекст и текущая итерация
//         if (!res.valid()) {
//           sol::error err = res;
//           std::cout << err.what();
//           throw std::runtime_error("Progress bar interface function error");
//         }
        return;
      }

      if (!interface->is_visible(utils::interface_container::last_layer())) {
        interface->open_layer(utils::interface_container::last_layer(), steps[current_step]->step_function(), {interface_table});
        return;
      }

      // тут читаем просто выхлоп из интерфейсов
      // или вообще пропускаем если сейчас идет генерация
      auto ret_lua = interface->openned_layers[utils::interface_container::last_layer()].ret;

//         auto ret_lua = steps[current_step]->get_interface()(interface->get_ctx(), interface_table);
//         if (!ret_lua.valid()) {
//           sol::error err = ret_lua;
//           std::cout << err.what();
//           throw std::runtime_error("Bad lua function result. Step " + std::to_string(current_step));
//         }

      ASSERT(!ret_lua.is<sol::nil_t>());
      if (!ret_lua.is<int32_t>()) throw std::runtime_error("Bad return from generator step");
      int32_t ret = ret_lua.as<int32_t>();
      ret = std::min(ret,  1);
      ret = std::max(ret, -1);
      current_step += ret;
      if (current_step != old_step && current_step >= 0) {
        if (scripts_needs_to_update) {
          auto tmp = old_step;
          for (auto p : steps) {
            steps_pool.destroy(p);
          }
          steps.clear();
          
          for (const auto &name : clearing_sol_state) {
            interface->lua[name] = sol::nil;
            interface->layers_table[name] = sol::nil;
          }
          
          clearing_sol_state.clear();
          interface->lua.collect_garbage();
          lua.collect_garbage();
          // перезагрузить скрипты?
          auto map = global::get<systems::map_t>();
          systems::setup_map_generator(map);
          
          scripts_needs_to_update = false;
          old_step = tmp;
        }
        
        table["userdata"] = pass_to_other_state(interface->lua, interface_table, lua, world_settings);
        gen.clear();
        for (const auto &pair : steps[old_step]->get_functions()) {
          gen.add(pair);
        }

        interface->close_layer(utils::interface_container::last_layer());

        auto info_table = interface->lua["tmp_table"].get_or_create<sol::table>();
        info_table["current_step"] = gen.current();
        info_table["hint1"] = gen.hint();
        info_table["step_count"] = gen.size();
        info_table["hint2"] = steps[old_step]->step_name();
        info_table["type"] = utils::progress_container::creating_map;
        interface->open_layer(utils::interface_container::last_layer(), "progress_bar", {info_table});
        
        auto generator = &gen;
        auto ctx_ptr = &ctx;
        auto table_ptr = &table;
        global::get<dt::thread_pool>()->submitbase([this, generator, ctx_ptr, table_ptr] () {
          try {
            generator->generate(ctx_ptr, *table_ptr);
          } catch (const std::runtime_error &err) {
            // что тут? по идее нужно ждать пока мы не обновим скрипты
            //generator->clear();
            std::cout << err.what() << "\n";
            scripts_needs_to_update = true;
//             while (scripts_needs_to_update) {
//               std::this_thread::sleep_for(std::chrono::microseconds(1));
//             }
          }
        });
      }
    }

    sol::table & creator::get_table() {
      return table;
    }

    bool creator::finished() const {
      return size_t(old_step) == steps.size();
    }

    bool creator::back_to_menu() const {
      return current_step < 0;
    }

    void creator::run_script(const std::string_view &path) {
      std::filesystem::path p(path);
      std::filesystem::directory_entry e(p);
      if (!e.exists()) throw std::runtime_error("Script " + std::string(path) + " does not exist");
      if (p.extension() != ".lua") throw std::runtime_error("Bad script extension. " + std::string(path));
      if (!e.is_regular_file()) throw std::runtime_error("Bad script file. " + std::string(path));

      auto res = lua.safe_script_file(p.string(), env_lua);
      if (!res.valid()) {
        sol::error e = res;
        std::cout << e.what() << "\n";
        throw std::runtime_error("Could not load script " + std::string(path));
      }
    }

    void creator::run_interface_script(const std::string_view &path) {
      std::filesystem::path p(path);
      std::filesystem::directory_entry e(p);
      if (!e.exists()) throw std::runtime_error("Script " + std::string(path) + " does not exist");
      if (p.extension() != ".lua") throw std::runtime_error("Bad script extension. " + std::string(path));
      if (!e.is_regular_file()) throw std::runtime_error("Bad script file. " + std::string(path));

//       auto res = interface->lua.safe_script_file(p);
//       if (!res.valid()) {
//         sol::error e = res;
//         std::cout << e.what() << "\n";
//         throw std::runtime_error("Could not load script " + std::string(path));
//       }

      interface->process_script_file(p.string());
    }

//     void creator::progress_interface(const std::string_view &name) {
//       clearing_sol_state.insert(std::string(name));
//       progress_interface_func = interface->get_state()[name];
//     }

    sol::function creator::get_func(const std::string_view &name, const bool remove_global) {
      //auto proxy = lua[name];
      auto proxy = env_lua[name];
      if (!proxy.valid()) throw std::runtime_error("Could not find function " + std::string(name));
      if (proxy.get_type() != sol::type::function) throw std::runtime_error(std::string(name) + " is not a function");
      
      sol::function f = proxy;
      if (remove_global) env_lua[name] = sol::nil;
      return f;
    }

    sol::state & creator::state() {
      return lua;
    }

    creator::table_container_t & creator::table_container() {
      return m_table_container;
    }

    std::string creator::get_world_name() const { return world_name; }
    std::string creator::get_folder_name() const { return folder_name; }
    std::string creator::get_settings() const { return world_settings; }
    uint64_t creator::get_rand_seed() const { return rand_seed; }
    uint32_t creator::get_noise_seed() const { return noise_seed; }
  }
}
