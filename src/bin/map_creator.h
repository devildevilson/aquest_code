#ifndef DEVILS_ENGINE_BIN_MAP_CREATOR_H
#define DEVILS_ENGINE_BIN_MAP_CREATOR_H

#include "utils/sol.h"

#include "generator/generator_system2.h"
#include "generator/context2.h"
#include "generator/container.h"
#include "utils/typeless_container.h"
#include "utils/memory_pool.h"
#include "utils/random_engine.h"
#include "Cpp/FastNoiseLite.h"
#include "utils/table_container.h"
#include "battle/map_enum.h"
#include "utils/serializator_helper.h"

// нужен класс который соберет в себя несколько шагов создания карты
// примерно тоже самое что и менюинг, но должно задаваться из json
// несколько шагов генерации карты 

namespace devils_engine {
  namespace utils {
    struct interface_container;
  }
  
  namespace localization {
    class container;
  }
  
  namespace map {
    class creator;
    
    class step {
    public:
      step(const std::string &step_name, const std::vector<map::generator_pair> &pairs);
      ~step();

      const std::vector<map::generator_pair> & get_functions() const;
      std::string step_name() const;
    protected:
      std::vector<map::generator_pair> pairs;
      std::string m_step_name;
    };
    
    class creator {
    public:
      using table_container_t = utils::table_container<static_cast<size_t>(utils::generator_table_container::additional_data::count)>;
      
      creator(core::map* map, core::seasons* seasons);
      ~creator();
      
      step* create(const std::string &step_name, const std::vector<map::generator_pair> &pairs);
      void generate(); // теперь запускаем лишь однажды
      sol::table & get_table();
      bool finished() const;
      bool back_to_menu() const;
      
      void prev_step();
      void advance();
      void advance_all();
      bool advancing() const;
      bool advancing_all() const;
      
      void run_script(const std::string_view &path);
      sol::function get_func(const std::string_view &name, const bool remove_global = true);
      
      sol::state & state();
      sol::environment & environment();
      utils::world_serializator* serializator_ptr();
      std::string get_world_name() const;
      std::string get_folder_name() const;
      std::string get_settings() const;
      uint64_t get_rand_seed() const;
      uint32_t get_noise_seed() const;
      
      std::string serialize_table(const sol::table &t);
      sol::table deserialize_table(const std::string &str);
      
      void set_userdata_table(const sol::table &t);
      sol::object get_post_generation_table() const;
      
      sol::function get_serpent_line() const;
      
      void setup_map_generator_functions(utils::interface_container* interface);
    private:
      sol::state lua;
      sol::environment env_lua;
      sol::table table;
      sol::table serpent;
      sol::function serpent_line;
      sol::table serpent_opts;
      
      systems::generator gen;
      
      map::generator::context ctx;
      map::generator::container temp_container; // нужно переделать покраску карты
      
      utils::world_map_string_container string_container; // ???
      uint64_t rand_seed;
      uint32_t noise_seed;
      std::atomic<int32_t> current_step;
      int32_t old_step;
      utils::memory_pool<step, sizeof(step)*10> steps_pool;
      std::vector<step*> steps;
      utils::random_engine_st random;
      FastNoiseLite noise;
      // вместо контейнера нужен сериализатор
      utils::world_serializator serializator;
      sol::table interface_table;
      
      std::string world_name;
      std::string folder_name;
      std::string world_settings;
      
      std::atomic_bool scripts_needs_to_update;
      
      bool advance_gen;
      bool advance_gen_all;
    };
  }
}

#endif
