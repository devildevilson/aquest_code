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
#include "utils/interface_container2.h"
#include "utils/progress_container.h"
#include "map.h"
#include "utils/lua_initialization.h"
#include "loading_functions.h"
#include "utils/lua_environment.h"
#include "utils/systems.h"

#include "re2/re2.h"

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

    step::step(const std::string &step_name, const std::vector<map::generator_pair> &pairs) : pairs(pairs), m_step_name(step_name) {}
    step::~step() {}

    const std::vector<map::generator_pair> & step::get_functions() const {
      return pairs;
    }

    std::string step::step_name() const {
      return m_step_name;
    }
    
    union convert {
      uint64_t u;
      double d;
    };
    
    //utils::interface_container* interface, 
    creator::creator(core::map* map, core::seasons* seasons) :
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
      scripts_needs_to_update(false),
      advance_gen(false),
      advance_gen_all(false)
    {
      lua.open_libraries(sol::lib::base, sol::lib::table, sol::lib::math, sol::lib::package, sol::lib::string, sol::lib::utf8, sol::lib::coroutine);
      ctx.container = &temp_container;
      ctx.map = map;
      ctx.seasons = seasons;
      ctx.noise = &noise;
      ctx.random = &random;
      ctx.pool = global::get<dt::thread_pool>();

      table["userdata"] = lua.create_table();
      //global::get(&m_table_container);
      global::get(&lua);
      
      // не учитывает к сожалению 0x нотацию, хотя можно сделать
      // не, 0x нотация нинужна
//       const std::string_view &test_str = "0000000000000000";
//       ASSERT(get_seed_from_string(test_str) == 0);
//       const std::string_view &test_str2 = "000000000000000f";
//       ASSERT(get_seed_from_string(test_str2) == 15);
//       ASSERT(get_string_from_seed(get_seed_from_string(test_str2)) == test_str2);

      const std::string path = global::root_directory() + "scripts/";
      {
        auto ret = lua.require_file("serpent", path + "serpent.lua", false);
        if (!ret.valid()) {
          throw std::runtime_error("Could not load serpent.lua");
        }
        
        serpent = ret.as<sol::table>();
        auto proxy = serpent["line"];
        if (!proxy.valid() || proxy.get_type() != sol::type::function) throw std::runtime_error("Bad serpent table");
        serpent_line = proxy.get<sol::function>();
      }
      
      utils::world_map_generation::setup_lua(lua);
      
      utils::make_environment(lua, env_lua);
      utils::add_io_lines(lua, env_lua);
      utils::add_require(lua, env_lua);
      
      global::get(&string_container);
      global::get(&serializator);
      const size_t type = static_cast<size_t>(utils::world_map_strings::tile_biome_id);
      string_container.register_strings(type, map->tiles_count());
    }

    creator::~creator() {
      for (auto p : steps) {
        steps_pool.destroy(p);
      }
      steps.clear();
      steps.shrink_to_fit();

      //global::get<table_container_t>(reinterpret_cast<table_container_t*>(SIZE_MAX));
      global::get<utils::world_serializator>(reinterpret_cast<utils::world_serializator*>(SIZE_MAX));
      global::get<sol::state>(reinterpret_cast<sol::state*>(SIZE_MAX));
      global::get<utils::world_map_string_container>(reinterpret_cast<utils::world_map_string_container*>(SIZE_MAX));
      //ASSERT(global::get<table_container_t>() == nullptr);
      ASSERT(global::get<utils::world_serializator>() == nullptr);
    }

    step* creator::create(const std::string &step_name, const std::vector<map::generator_pair> &pairs) {
//       clearing_sol_state.insert(interface_name);
//       interface->register_function(interface_name, interface_name);
      std::vector<map::generator_pair> final_pairs;
      // это первый степ, мы можем сюда добавить функцию бегин
      if (steps.empty()) final_pairs.push_back(map::default_generator_pairs[0]);

      final_pairs.insert(final_pairs.end(), pairs.begin(), pairs.end());
      auto step = steps_pool.create(step_name, final_pairs);
      steps.push_back(step);
      return step;
    }

    void creator::generate() {      
      ASSERT(size_t(current_step) < steps.size());
      ASSERT(advance_gen || advance_gen_all);
      
      if (scripts_needs_to_update) {
        auto tmp = old_step;
        for (auto p : steps) {
          steps_pool.destroy(p);
        }
        steps.clear();
        
        lua.collect_garbage();
        // перезагрузить скрипты?
        auto map = global::get<systems::map_t>();
        systems::setup_map_generator(map);
        
        scripts_needs_to_update = false;
        old_step = tmp;
      }
      
      gen.clear();
      for (const auto &pair : steps[current_step]->get_functions()) {
        gen.add(pair);
      }
      
      advance_gen = false;
      advance_gen_all = false;
      
      auto prog = global::get<systems::core_t>()->loading_progress;
      prog->set_hint1(steps[current_step]->step_name());
      
      auto generator = &gen;
      auto ctx_ptr = &ctx;
      auto table_ptr = &table;
      global::get<dt::thread_pool>()->submitbase([this, generator, ctx_ptr, table_ptr, prog] () {
        try {
          generator->generate(ctx_ptr, *table_ptr);
          ++current_step;
        } catch (const std::runtime_error &err) {
          // что тут? по идее нужно ждать пока мы не обновим скрипты
          //generator->clear();
          std::cout << err.what() << "\n";
          scripts_needs_to_update = true;
          prog->set_value(1000);
        }
      });
    }

    sol::table & creator::get_table() {
      return table;
    }

    bool creator::finished() const {
      //return size_t(old_step) == steps.size();
      return size_t(current_step) >= steps.size();
    }

    bool creator::back_to_menu() const {
      return current_step < 0;
    }
    
    void creator::prev_step() {
      --current_step;
      if (current_step == 0) current_step = -1;
      advance_gen = true;
    }
    
    void creator::advance() {
      advance_gen = true;
    }
    
    void creator::advance_all() {
      advance_gen_all = true;
    }
    
    bool creator::advancing() const {
      return advance_gen;
    }
    
    bool creator::advancing_all() const {
      return advance_gen_all;
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

    utils::world_serializator* creator::serializator_ptr() {
      return &serializator;
    }

    std::string creator::get_world_name() const { return world_name; }
    std::string creator::get_folder_name() const { return folder_name; }
    std::string creator::get_settings() const { return world_settings; }
    uint64_t creator::get_rand_seed() const { return rand_seed; }
    uint32_t creator::get_noise_seed() const { return noise_seed; }
    
    std::string creator::serialize_table(const sol::table &t) {
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
    
    sol::table creator::deserialize_table(const std::string &str) {
      auto ret = lua.script("return " + str);
      if (!ret.valid()) {
        sol::error err = ret;
        std::cout << err.what();
        throw std::runtime_error("Could not deserialize string");
      }
      
      sol::table t = ret;
      return t;
    }
    
    void creator::set_userdata_table(const sol::table &t) {
      table["userdata"] = t;
    }
    
    sol::object creator::get_post_generation_table() const {
      return table["post_generation"];
    }
    
    sol::function creator::get_serpent_line() const {
      return serpent_line;
    }
    
    static const std::string_view folder_name_regex_str = "^[a-zA-Z0-9_.]+$";
    static const RE2 folder_name_regex(folder_name_regex_str);
    
    void creator::setup_map_generator_functions(utils::interface_container* interface) {
      // мне нужно расшарить несколько функций для интерфейса по которым 
      // я задам сид и название мира, но при этом я не очень хочу
      // функции в таком виде использовать, преждде всего потому что нужно 
      // будет убедиться что this еще существует к этому моменту
      // хотя по идее это не так сложно проверить, как передать эти функции в интерфейс?
      // опять завести таблицу? видимо

      // нужно тут создать интерфейс тоже, только по всей видимости почти без данных
      // все таки придется сериализовать таблицу
      
      auto t = interface->lua.create_table();
      t.set_function("setup_random_seed", [this] (const std::string_view &str_seed) {
        if (global::get<systems::map_t>()->map_creator == nullptr) throw std::runtime_error("Map generator does not exist");
        
        const uint64_t seed = get_seed_from_string(str_seed);
        
        this->rand_seed = seed;
        this->random.set_seed(seed);
      });

      t.set_function("setup_noise_seed", [this] (const std::string_view &str_seed) {
        if (global::get<systems::map_t>()->map_creator == nullptr) throw std::runtime_error("Map generator does not exist");
        
        const uint64_t seed = get_seed_from_string(str_seed);
        
        // может заменить на более мелкое число? хотя если дать нижние регистры заполнять, 
        // можно сделать какую нибудь другие способы задания сида
        //this->noise_seed = uint32_t(seed >> 32);
        this->noise_seed = seed;
        this->noise.SetSeed(*reinterpret_cast<const int*>(&this->noise_seed));
      });

      t.set_function("get_random_number", [] () -> std::string {
        if (global::get<systems::map_t>()->map_creator == nullptr) throw std::runtime_error("Map generator does not exist");
        
        std::random_device dev;
        static_assert(sizeof(std::random_device::result_type) == sizeof(uint32_t));
        const uint64_t tmp = (uint64_t(dev()) << 32) | uint64_t(dev());
        
        // тут нужно бы составить строку размером 16 вида: deadbeafdeadbeaf
        const auto &str = get_string_from_seed(tmp);
        return str;
      });

      t.set_function("set_world_name", [this] (const std::string_view &str) {
        if (global::get<systems::map_t>()->map_creator == nullptr) throw std::runtime_error("Map generator does not exist");
        
        if (str.empty()) throw std::runtime_error("Bad world name string");
        world_name = std::string(str);
      });

      t.set_function("set_folder_name", [this] (const std::string_view &str) {
        if (global::get<systems::map_t>()->map_creator == nullptr) throw std::runtime_error("Map generator does not exist");
        
        if (str.empty()) throw std::runtime_error("Bad world folder name string");
        //const std::regex folder_name_regex("^[a-zA-Z0-9_.]+$", std::regex_constants::icase);
        //if (!std::regex_match(str.begin(), str.end(), folder_name_regex)) throw std::runtime_error("Folder name must match ^[a-zA-Z0-9_.]+$ expression");
        if (!RE2::FullMatch(str, folder_name_regex)) throw std::runtime_error("Folder name must match " + std::string(folder_name_regex_str) + " expression");
        folder_name = std::string(str);
      });

      t.set_function("check_world_existance", [] (const std::string_view &str) {
        if (global::get<systems::map_t>()->map_creator == nullptr) throw std::runtime_error("Map generator does not exist");
        
        if (str.empty()) return 0;
        //const std::regex folder_name_regex("^[a-zA-Z0-9_.]+$", std::regex_constants::icase);
        //if (!std::regex_match(str.begin(), str.end(), folder_name_regex)) return 1;
        if (!RE2::FullMatch(str, folder_name_regex)) return 1;
        const std::filesystem::path p = global::root_directory() + "/saves/" + std::string(str) + "/world_data";
        const std::filesystem::directory_entry e(p);
        if (e.exists()) return 2;
        return 3;
      });
      
      t.set_function("advance", [this] () {
        if (global::get<systems::map_t>()->map_creator == nullptr) throw std::runtime_error("Map generator does not exist");
        advance();
      });
      
      t.set_function("prev_step", [this] () {
        if (global::get<systems::map_t>()->map_creator == nullptr) throw std::runtime_error("Map generator does not exist");
        prev_step();
      });
      
      t.set_function("step", [this] () -> int32_t { // sol::readonly_property(
        if (global::get<systems::map_t>()->map_creator == nullptr) throw std::runtime_error("Map generator does not exist");
        return current_step +1; // приведем к луа индексам
      });
      
      t.set_function("steps_count", [this] () -> uint32_t { // sol::readonly_property(
        if (global::get<systems::map_t>()->map_creator == nullptr) throw std::runtime_error("Map generator does not exist");
        return steps.size();
      });
      
      t.set_function("step_name", [this] () -> std::string {
        if (global::get<systems::map_t>()->map_creator == nullptr) throw std::runtime_error("Map generator does not exist");
        if (current_step < 0 || size_t(current_step) >= steps.size()) return "";
        return steps[current_step]->step_name();
      });
      
      t.set_function("is_finished", [this] () -> bool { // sol::readonly_property(
        if (global::get<systems::map_t>()->map_creator == nullptr) throw std::runtime_error("Map generator does not exist");
        return finished();
      });
      
      interface->setup_map_generator_functions(t);
    }
  }
}
