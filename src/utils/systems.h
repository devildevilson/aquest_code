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
    struct turn_status;
  }
  
  namespace script {
    class system;
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
    class world_serializator;
//     class localization;
  }
  
  namespace interface {
    struct context;
  }
  
  namespace render {
    class slots;
    class stage_container;
    class image_container;
    class proxy_stage;
    class stage;
    class pass;
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
  
  namespace generator {
    struct system;
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
//       render::slots* render_slots;
      render::proxy_stage* proxy_compute;
      render::proxy_stage* proxy_graphics;
      render::pass* main_pass;
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
      std::unique_ptr<game::turn_status> turn_status;
      
      //utils::interface_container* interface_container;
      std::unique_ptr<utils::interface_container> interface_container;
      std::unique_ptr<localization::container> loc;
      std::unique_ptr<core::internal_lua_state> internal;
      
      std::string load_world;
      
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
      
//       utils::slot_container container; // мы не можем полностью почистить память здесь 
      std::unique_ptr<core::map> map; // я могу все буферы в мап перенести в ГПУ память, нигде больше я их не использую (кроме смены рендер мода, там придется копировать буфер по несколько раз)
      std::unique_ptr<core::context> core_context;
      std::unique_ptr<core::seasons> seasons;
      std::unique_ptr<systems::ai> ai_systems;
      std::unique_ptr<render::mode_container> render_modes;
      std::unique_ptr<map::creator> map_creator;
      std::unique_ptr<script::system> script_system;
      
      std::unique_ptr<render::stage_container> optimizators_container;
      std::unique_ptr<render::stage_container> render_container;
      render::stage* first_compute;
      render::stage* first_graphics;
      
      std::string saved_data; // какие здесь данные? только часть мировая, названия и хеш мы заполняем извне
      // возможно нужно сделать standalone сохранения, нам тогда потребуются все данные ниже
      std::string world_name;
      std::string folder_name; // имя папки в которой хранится world_data
      std::string generator_settings;
      uint8_t hash[hash_size];
      //std::atomic_bool inited; // короче плохая идея, нужно создавать/удалять эти вещи в одном потоке
      
      render::world_map_buffers* world_buffers;
      std::unique_ptr<components::world_map_camera> camera;
      
      map_t();
      ~map_t();
      //inline bool is_init() const { return container.inited(); }
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
      
//       void setup_map_generator(); // как передать сюда данные? по идее мы можем собрать с помощью луа
//       void destroy_map_generator();
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
    
    struct generator_t {
      // генератор карта - примерно как обычная карта, но почти все данные будут в цпу памяти
      // будет немного модифицированный map::creator, его желательно распределить
      // как распределить? ну во первых нужно отказаться от использования прогресс контейнера в пользу
      // того чтобы извне контролировать ход генерации, а для этого нужно хранить шаги где то ближе к этой структуре
      
      std::unique_ptr<core::seasons> seasons;
      std::unique_ptr<core::map> map;
      std::unique_ptr<map::generator::container> temp_container;
      std::unique_ptr<utils::world_serializator> serializator;
      std::unique_ptr<devils_engine::generator::system> gen; // какая то идиотская коллизия имен, исправить!!!!
      std::unique_ptr<utils::random_engine_st> random;
      std::unique_ptr<FastNoiseLite> noise;
      //std::unique_ptr<map::creator> creator;
      std::unique_ptr<render::stage_container> render_container;
      render::world_map_buffers* world_buffers;
      render::stage* first_compute;
      render::stage* first_graphics;
      
      std::string world_name;
      std::string folder_name;
      std::string world_settings;
      
      generator_t();
      ~generator_t();
      
      void create_render_stages();
      void start_rendering();
      void stop_rendering();
      
      void setup_map_generator_functions(utils::interface_container* interface);
      bool gen_end() const;
    };
    
    struct battle_t {
//       utils::slot_container container;
      // нужен баттл контекст
      std::unique_ptr<systems::ai> ai_systems;
      std::unique_ptr<battle::map> map;
      std::unique_ptr<render::stage_container> optimizators_container;
      std::unique_ptr<render::stage_container> render_container;
      render::stage* first_compute;
      render::stage* first_graphics;
      std::unique_ptr<components::battle_camera> camera;
      std::unique_ptr<battle::lua_container> lua_states;
      //battle::unit_states_container* unit_states_container;
      std::unique_ptr<battle::context> context;
      std::unique_ptr<utils::data_string_container> unit_states_map;
      
//       utils::random_engine_st* random;
//       FastNoise* noiser;
//       map::generator::container* generator_container;
//       sol::state* generator_state;
      
      battle_t();
      ~battle_t();
      //inline bool is_init() const { return container.inited(); }
      
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
//       utils::slot_container container;
      //core::map* map; // другая карта скорее всего
      std::unique_ptr<systems::ai> ai_systems;
      std::unique_ptr<render::stage_container> stage_container;
      
      encounter_t();
      ~encounter_t();
//       inline bool is_init() const { return container.inited(); }
    };
    
    
  }
}

#endif
