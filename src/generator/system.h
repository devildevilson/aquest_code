#ifndef DEVILS_ENGINE_GENERATOR_SYSTEM_H
#define DEVILS_ENGINE_GENERATOR_SYSTEM_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include <memory>
#include <atomic>
#include "context2.h"
#include "utils/sol.h"

namespace devils_engine {
  namespace utils {
    class world_serializator;
    struct interface_container;
  }
  
  namespace generator {
    struct step {
      std::vector<map::generator_pair> pairs;
      std::string name;
      
      step() noexcept;
      step(std::string name, std::vector<map::generator_pair> pairs) noexcept; // по идее от них можно мув брать
      
      step(const step &copy) noexcept = delete;
      step(step &&move) noexcept = default;
      step & operator=(const step &copy) noexcept = delete;
      step & operator=(step &&move) noexcept = default;
    };
    
    struct system {
      sol::state lua;
      sol::environment env_lua;
      sol::table table;
      sol::function serpent_line;
      sol::table serpent_opts;
      
      std::vector<step> steps;
      map::generator::context ctx;
      
      std::string world_name;
      std::string folder_name;
      std::string world_settings;
      
      std::atomic_bool scripts_needs_to_update;
      
      uint64_t rand_seed;
      uint32_t noise_seed;
      int32_t cur_step;
      
      system();
      
      std::string serialize_table(const sol::table &t);
      sol::table deserialize_table(const std::string &str);
      
      void set_userdata_table(const sol::table &t);
      sol::object get_post_generation_table() const;
      
//       void setup_map_generator_functions(utils::interface_container* interface);
    };
  }
}

#endif
