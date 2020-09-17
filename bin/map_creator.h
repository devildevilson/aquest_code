#ifndef MAP_CREATOR_H
#define MAP_CREATOR_H

#include "utils/sol.h"

#include "generator_system2.h"
#include "generator_context2.h"
#include "generator_container.h"
#include "utils/typeless_container.h"
#include "utils/memory_pool.h"
#include "utils/random_engine.h"
#include "FastNoise.h"
#include "utils/table_container.h"
#include "interface2.h"

// нужен класс который соберет в себя несколько шагов создания карты
// примерно тоже самое что и менюинг, но должно задаваться из json
// несколько шагов генерации карты 

namespace devils_engine {
  namespace map {
    class creator;
    
    class variable {
    public:
      virtual ~variable() = default;
      virtual void draw(sol::table &table) = 0;
      virtual void set_default_value(sol::table &table) = 0;
    };
    
    class property_int : public variable {
    public:
      struct create_info {
        int32_t min;
        int32_t default_val;
        int32_t max;
        int32_t step;
        float pixel_step;
        std::string prop_name;
        std::string var_name;
      };
      property_int(const create_info &info);
      void draw(sol::table &table) override;
      void set_default_value(sol::table &table) override;
    private:
      int32_t min;
      int32_t default_val;
      int32_t max;
      int32_t step;
      float pixel_step;
      std::string prop_name;
      std::string var_name;
    };
    
    class property_float : public variable {
    public:
      struct create_info {
        float min;
        float default_val;
        float max;
        float step;
        float pixel_step;
        std::string prop_name;
        std::string var_name;
      };
      property_float(const create_info &info);
      void draw(sol::table &table) override;
      void set_default_value(sol::table &table) override;
    private:
      float min;
      float default_val;
      float max;
      float step;
      float pixel_step;
      std::string prop_name;
      std::string var_name;
    };
    
    class step {
    public:
//       step(const bool first, const size_t &container_size, const std::string &name, const std::vector<map::generator_pair> &pairs, const std::string &rendering_mode);
      step(const sol::function &interface, const std::vector<map::generator_pair> &pairs); //const bool first, 
      ~step();
//       int32_t prepare(systems::generator &gen, map::generator::context* context, sol::table &table);
      
//       template <typename T, typename... Args>
//       T* add(Args&&... args) {
//         auto ptr = container.create<T>(std::forward<Args>(args)...);
//         variables.push_back(ptr);
//         return ptr;
//       }
      
      const sol::function & get_interface() const;
      const std::vector<map::generator_pair> & get_functions() const;
    protected:
//       bool first;
//       utils::typeless_container container;
//       std::vector<variable*> variables;
      // тут еще должен быть массив функций для генератора
      std::vector<map::generator_pair> pairs;
//       std::string name;
//       std::string rendering_mode;
      sol::function interface;
    };
    
    class creator {
    public:
      creator(utils::interface* interface);
      ~creator();
//       step* create(const bool first, const size_t &container_size, const std::string &name, const std::vector<map::generator_pair> &pairs, const std::string &rendering_mode);
      step* create(const std::string_view &interface_name, const std::vector<map::generator_pair> &pairs); //const bool first, 
      void generate();
      sol::table & get_table();
      bool finished() const;
      bool back_to_menu() const;
      
      void run_script(const std::string_view &path);
      void run_interface_script(const std::string_view &path);
      void progress_interface(const std::string_view &name);
      
      sol::state & state();
      utils::table_container & table_container();
    private:
      sol::state lua;
      sol::table table;
      systems::generator gen;
      map::generator::context ctx;
      map::generator::container temp_container; // нужно переделать покраску карты
      //utils::typeless_container container;
      int32_t current_step;
      int32_t old_step;
      utils::memory_pool<step, sizeof(step)*10> steps_pool;
      std::vector<step*> steps;
      utils::random_engine_st random;
      FastNoise noise;
      utils::table_container m_table_container;
      utils::interface* interface;
      sol::table interface_table;
      sol::function progress_interface_func;
      std::unordered_set<std::string> clearing_sol_state;
    };
  }
}

#endif
