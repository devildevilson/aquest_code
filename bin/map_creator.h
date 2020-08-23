#ifndef MAP_CREATOR_H
#define MAP_CREATOR_H

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include "generator_system2.h"
#include "generator_context2.h"
#include "utils/typeless_container.h"
#include "utils/memory_pool.h"
#include "utils/random_engine.h"
#include "FastNoise.h"

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
      step(const bool first, const size_t &container_size, const std::string &name, const std::vector<map::generator_pair> &pairs, const std::string &rendering_mode);
      ~step();
      int32_t prepare(systems::generator &gen, map::generator::context* context, sol::table &table);
      
      template <typename T, typename... Args>
      T* add(Args&&... args) {
        auto ptr = container.create<T>(std::forward<Args>(args)...);
        variables.push_back(ptr);
        return ptr;
      }
    protected:
      bool first;
      utils::typeless_container container;
      std::vector<variable*> variables;
      // тут еще должен быть массив функций для генератора
      std::vector<map::generator_pair> pairs;
      std::string name;
      std::string rendering_mode;
    };
    
    class creator {
    public:
      creator();
      ~creator();
      step* create(const bool first, const size_t &container_size, const std::string &name, const std::vector<map::generator_pair> &pairs, const std::string &rendering_mode);
      void generate();
      sol::table & get_table();
      bool finished() const;
    private:
      sol::state lua;
      sol::table table;
      systems::generator gen;
      map::generator::context ctx;
      //utils::typeless_container container;
      size_t current_step;
      utils::memory_pool<step, sizeof(step)*10> steps_pool;
      std::vector<step*> steps;
      utils::random_engine_st random;
      FastNoise noise;
    };
  }
}

#endif
