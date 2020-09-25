#ifndef SYSTEMS_H
#define SYSTEMS_H

#include "typeless_container.h"
#include "slot_container.h"
#include "render/render_mode_container.h"
#include <string>
#include <atomic>

namespace devils_engine {
  namespace input {
    struct data;
  }
  
  namespace core {
    struct map;
    class context;
    struct seasons;
  }
  
  namespace utils {
    class interface;
    struct interface_container;
    struct main_menu;
    class data_string_container;
    class sequential_string_container;
    struct calendar;
    class progress_container;
  }
  
  namespace interface {
    struct context;
  }
  
  namespace render {
    struct container;
    class slots;
    class stage_container;
    struct world_map_buffers;
  }
  
  namespace map {
    class creator;
  }
  
  namespace systems {
    class ai;
    
    struct core_t {
      utils::typeless_container container;
      
      input::data* input_data;
      render::container* graphics_container;
      render::slots* render_slots;
      interface::context* context;
      utils::interface* interface;
      utils::main_menu* menu;
      utils::interface_container* interface_container;
//       utils::data_string_container* string_container;
      utils::sequential_string_container* sequential_string_container;
      utils::calendar* game_calendar;
      utils::progress_container* loading_progress;
      
      core_t();
      ~core_t();
      void create_utility_systems();
      void create_render_system(const char** ext, const uint32_t &count);
      void create_render_stages();
      void create_interface();
      
      // нужно создать стейджи общие (интерфейс, буферы)
    };
    
    struct map_t {
      static const size_t hash_size = 64;
      
      utils::slot_container container; // мы не можем полностью почистить память здесь 
      core::map* map; // я могу все буферы в мап перенести в ГПУ память, нигде больше я их не использую (кроме смены рендер мода, там придется копировать буфер по несколько раз)
      core::context* core_context;
      core::seasons* seasons;
      systems::ai* ai_systems;
      render::mode_container* render_modes;
      map::creator* map_creator;
      render::stage_container* optimizators_container;
      render::stage_container* render_container;
      
      std::string saved_data; // какие здесь данные? только часть мировая, названия и хеш мы заполняем извне
      // возможно нужно сделать standalone сохранения, нам тогда потребуются все данные ниже
      std::string world_name;
      std::string folder_name; // имя папки в которой хранится world_data
      std::string generator_settings;
      uint8_t hash[hash_size];
      //std::atomic_bool inited; // короче плохая идея, нужно создавать/удалять эти вещи в одном потоке
      
      render::world_map_buffers* world_buffers;
      
      map_t();
      ~map_t();
      inline bool is_init() const { return container.inited(); }
      //inline bool is_init() const { return inited; }
      void create_map_container();
      void setup_rendering_modes();
      void release_container();
      void create_render_stages();
      
      // нужно создать стейджи только для карты
      
      void lock_map();
      void unlock_map();
      
      void setup_map_generator(); // как передать сюда данные? по идее мы можем собрать с помощью луа
      void destroy_map_generator();
      void save(); // сохраняем данные в saved_data
      std::string get_data() const; // копируем строку (например для записи на диск)
      void set_data(std::string &&data); // заполнили данные
      // мы должны понять какие данные в стринге, создать контейнер, загрузить и удалить строку
      // если я хочу сделать серилизацию битвы, то мне нужно будет отделить данные карты от данных битвы
      void load();
      void clear_saved_data();
      
      // по идее если существует map_creator, то у нас есть все данные
      // но у нас нет таблиц упакованных в строчку
//       void dump_world_to_disk();
    };
    
    struct battle_t {
      utils::slot_container container;
      //core::map* map; // другая карта скорее всего
      systems::ai* ai_systems;
      render::stage_container* stage_container;
      
      battle_t();
      ~battle_t();
      bool is_init() const;
    };
    
    struct encouter_t {
      utils::slot_container container;
      //core::map* map; // другая карта скорее всего
      systems::ai* ai_systems;
      render::stage_container* stage_container;
      
      encouter_t();
      ~encouter_t();
      bool is_init() const;
    };
    
    
  }
}

#endif
