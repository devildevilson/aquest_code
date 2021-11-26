#ifndef SYSTEMS_H
#define SYSTEMS_H

#include <string>
#include <atomic>
#include <memory>

#include "typeless_container.h"
#include "slot_container.h"
#include "game_enums.h"
#include "render/render_mode_container.h"

class FastNoiseLite;

namespace sol {
  class state;
}

namespace devils_engine {
  namespace input {
    struct data;
    struct keys;
  }
  
  namespace core {
    struct map;
    class context;
    struct seasons;
    struct internal_lua_state;
  }
  
  namespace game {
    struct context;
  }
  
  namespace battle {
    struct map;
    struct lua_container;
    class unit_states_container;
    class context;
  }
  
  namespace utils {
    class interface;
    struct interface_container;
    struct main_menu;
    class data_string_container;
    class sequential_string_container;
    struct calendar;
    class progress_container;
    struct objects_selection;
    struct random_engine_st;
//     class localization;
  }
  
  namespace interface {
    struct context;
  }
  
  namespace render {
    class slots;
    class stage_container;
    class image_container;
    struct container;
    struct world_map_buffers;
    struct image_controller;
  }
  
  namespace map {
    class creator;
    namespace generator {
      class container;
    }
  }
  
  namespace ai {
    struct path_managment;
  }
  
  namespace components {
    class world_map_camera;
    class battle_camera;
  }
  
  namespace localization {
    class container;
  }
  
  namespace systems {
    using devils_engine::ai::path_managment;
    class ai;
    
    struct core_t {
      struct selection_containers {
        utils::objects_selection* primary;
        utils::objects_selection* secondary;
      };
      
      utils::typeless_container container;
      
      // нужно сделать несколько контейнеров с клавишами, где?
      // вообще нам нужно сделать просто 4 указателя?
      //input::data* input_data;
      input::keys* keys_mapping[player::states_count];
      render::container* graphics_container;
      render::slots* render_slots;
      render::image_container* image_container;
      render::image_controller* image_controller;
      interface::context* context;
      utils::interface* interface;
      utils::main_menu* menu;
      game::context* game_ctx;
//       utils::data_string_container* string_container;
      utils::sequential_string_container* sequential_string_container; // ???
      utils::calendar* game_calendar; // не чистится при перезагрузках
      utils::progress_container* loading_progress;
      //utils::objects_selector* objects_selector;
      selection_containers selection;
      struct path_managment* path_managment;
      //localization::container* loc; // не чистится при перезагрузках, но перезаписывается
      
      //utils::interface_container* interface_container;
      std::unique_ptr<utils::interface_container> interface_container;
      std::unique_ptr<localization::container> loc;
      std::unique_ptr<core::internal_lua_state> internal;
      
      core_t();
      ~core_t();
      void create_utility_systems();
      void create_render_system(const char** ext, const uint32_t &count);
      void create_render_stages();
      void create_interface();
      void reload_interface();
      void load_interface_config(const std::string &config_path);
      
      // нужно создать стейджи общие (интерфейс, буферы)
    };
    
    struct map_t {
      static const size_t hash_size = 32;
      
      utils::slot_container container; // мы не можем полностью почистить память здесь 
      core::map* map; // я могу все буферы в мап перенести в ГПУ память, нигде больше я их не использую (кроме смены рендер мода, там придется копировать буфер по несколько раз)
      core::context* core_context;
      core::seasons* seasons;
      systems::ai* ai_systems;
      render::mode_container* render_modes;
      map::creator* map_creator;
      render::stage_container* optimizators_container;
      render::stage_container* render_container;
      
      std::string load_world;
      std::string saved_data; // какие здесь данные? только часть мировая, названия и хеш мы заполняем извне
      // возможно нужно сделать standalone сохранения, нам тогда потребуются все данные ниже
      std::string world_name;
      std::string folder_name; // имя папки в которой хранится world_data
      std::string generator_settings;
      uint8_t hash[hash_size];
      //std::atomic_bool inited; // короче плохая идея, нужно создавать/удалять эти вещи в одном потоке
      
      render::world_map_buffers* world_buffers;
      components::world_map_camera* camera;
      
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
      
      void start_rendering();
      void stop_rendering();
      bool is_rendering() const;
      
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
      // нужен баттл контекст
      systems::ai* ai_systems;
      battle::map* map;
      render::stage_container* optimizators_container;
      render::stage_container* render_container;
      components::battle_camera* camera;
      battle::lua_container* lua_states;
      //battle::unit_states_container* unit_states_container;
      battle::context* context;
      utils::data_string_container* unit_states_map;
      
//       utils::random_engine_st* random;
//       FastNoise* noiser;
//       map::generator::container* generator_container;
//       sol::state* generator_state;
      
      battle_t();
      ~battle_t();
      inline bool is_init() const { return container.inited(); }
      
      void create_map_container();
      void create_render_stages();
      
//       void setup_generator_random();
//       void setup_generator_container(const size_t &tiles_count);
//       void release_generator_data();
      
      void release_container();
      
      void lock_map();
      void unlock_map();
      
      void start_rendering();
      void stop_rendering();
    };
    
    struct encounter_t {
      utils::slot_container container;
      //core::map* map; // другая карта скорее всего
      systems::ai* ai_systems;
      render::stage_container* stage_container;
      
      encounter_t();
      ~encounter_t();
      inline bool is_init() const { return container.inited(); }
    };
    
    
  }
}

#endif
