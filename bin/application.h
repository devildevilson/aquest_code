#ifndef DEVILS_ENGINE_CORE_APPLICATION_H
#define DEVILS_ENGINE_CORE_APPLICATION_H

#include "utils/thread_pool.h"
#include "utils/systems.h"
#include "utils/deferred_tasks.h"
#include "utils/settings.h"
#include "utils/frame_time.h"
#include "dsml.hpp" // dsml/dsml.hpp

// покрутив приложение по всякому я понял что чистить совсем ресурсы карты не нужно
// чистить нужно только рендер (а нужно ли чистить картиночки? вообще было бы неплохо)
// контекст оказался гораздо легче чем я думал, его чистить не имеет смысла
// + нам могут потребоваться эти данные
// вообще надо бы проверить что конкретно занимает так много памяти
// что делать с картиночками? по идее я буду использовать массив картиночек очень маленького разрешения
// картиночка 32х32х4 - 4кб: это основной размер который я буду использовать
// картиночки лиц уже скорее всего будут крупнее (512х512х4 - 1мб) нужны ли они в битвах?
// могут пригодиться, к этому у меня добавится изображения в самой битве
// можно удалить текстурки городов и армий, нужно тогда сделать частичное восстановление
// мирового стейта, 

// как же все таки организовать доступ к создаваемым данным? 
// разделить их на несколько структур идея хорошая, мож оставить все как есть, просто убрать в этот класс?
// эт конечно что первое в голову приходит

namespace devils_engine {
  namespace core {
    struct glfw_t {
      glfw_t();
      ~glfw_t();
    };
    
    struct block_input_t {
      block_input_t();
      ~block_input_t();
    };
    
    class application {
    public:
      application(const int argc, const char* argv[]);
      ~application();
      application(const application &copy) = delete;
      application(application &&move) = delete;
      application & operator=(const application &copy) = delete;
      application & operator=(application &&move) = delete;
      
      bool continue_computation() const;
      void update(); // передавать ли время? или время считать внутри?
      
      void notify_gen_advance();
    private:
      struct state_machine_decl;
      friend struct state_machine_decl;
      
      glfw_t glfw;
      dt::thread_pool pool;

      systems::core_t base_systems;
      systems::map_t map_systems;
      systems::battle_t battle_systems;
      systems::encounter_t encounter_systems;
      
      utils::deffered_tasks tasks;
      
      utils::settings settings;
      
      utils::frame_time frame_time;
      
      dsml::Sm<state_machine_decl, application> m_sm; // , observer*
      
      void update_ai(const size_t &time);
      void start_render();
      void update_world(const size_t &time);
      void wait_for_render();
      
      // вот это дело наверное нужно вынести в отдельный файл
      void loading_menu_action();
      void loading_generator_action();
      void loading_gen_world_action();
      void loading_sav_world_action();
      void loading_battle_action();
      void loading_encounter_action();
      void generating_world_action();
      void free_world_data();
      void partial_free_world_data(); // для битв и столкновений
      void restore_world_data(); // + к этому нужно добавить расчет последствий битвы
    };
  }
}

#endif
