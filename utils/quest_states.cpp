#include "quest_states.h"

#include "globals.h"
#include "systems.h"
#include "interface_container2.h"
#include "main_menu.h"
#include "progress_container.h"
#include "thread_pool.h"
#include "bin/map_creator.h"
#include "bin/loading_functions.h"
#include "bin/logic.h"
#include "bin/battle_map.h"
#include "render/image_controller.h"
// #include "render/battle_render_stages.h"
#include "utils/input.h"
#include "utils/game_enums.h"

// посчтитать используемую память
#include "render/container.h"

namespace devils_engine {
  namespace utils {
    void update_interface() {
      auto base = global::get<systems::core_t>();
      auto prog = base->loading_progress;
      auto inter = base->interface_container.get();
      
      // обновляем таблицу
      if (!inter->loading_table_tmp.valid()) {
        inter->loading_table_tmp = inter->lua.create_table(0, 5);
      }
      update_progress_table(prog, inter->loading_table_tmp);
      inter->update_loading_table(inter->loading_table_tmp);
    }
    
    void set_up_input_keys(const uint32_t &state) {
      auto core = global::get<systems::core_t>();
      ASSERT(state < player::states_count);
      auto key_map_ptr = core->keys_mapping[state];
      input::block();
      if (state == player::in_menu) input::set_key_map(nullptr);
      else input::set_key_map(key_map_ptr);
      input::unblock();
    }
    
    main_menu_loading_state::main_menu_loading_state() : quest_state(main_menu_loading) {}
    void main_menu_loading_state::enter(quest_state* prev_state) {
      // эта функция будет запускаться перед загрузкой
      set_up_input_keys(player::in_menu);
      
      if (prev_state != nullptr) prev_state->clean(this);
    }
    
    uint32_t main_menu_loading_state::update(const size_t &time, quest_state* prev_state) {
      (void)time;
      (void)prev_state;
      // тут по идее нужно сделать картинку с компанией короче
      
      // я переделал интерфейс и теперь у меня нет тут главного меню
      // главное меню будет моделироваться в луа, переходы из состояния
      // в состояние будут управляться где то в программе, вызовами функций
      
      //enter();
      return main_menu;
    }
    
    void main_menu_loading_state::clean(quest_state* next_state) { 
      auto base = global::get<systems::core_t>();
      base->interface_container->lua.collect_garbage();
      
      if (next_state->current_state() != main_menu) {
        throw std::runtime_error("wtf");
      }
      
      m_next_state = UINT32_MAX;
    }

    main_menu_state::main_menu_state() : quest_state(main_menu) {}
    void main_menu_state::enter(quest_state* prev_state) {
      // эта функция будет запускаться перед загрузкой
      //set_up_input_keys(player::in_menu);
      if (prev_state != nullptr) prev_state->clean(this);
//       global::get<render::container>()->print_memory_info();
    }

//     bool main_menu_state::load(quest_state* prev_state) {
//       if (prev_state != nullptr) prev_state->clean();
//       
//       auto base = global::get<systems::core_t>();
//       // грузанем здесь новую картинку для заднего фона
//       if (!base->interface_container->is_visible(uint32_t(utils::interface_container::last_layer()))) {
//         base->menu->push("main_menu");
//       }
//       
//       enter();
// 
//       return true;
//     }

    uint32_t main_menu_state::update(const size_t &time, quest_state* prev_state) {
//       (void)time;
//       auto base = global::get<systems::core_t>();
//       auto current_state = game::menu;
//       auto new_state = game::menu;
//       if (base->menu->advance_state(current_state, new_state)) {
//         switch (new_state) {
//           case game::map: m_next_state = quest_state::world_map; break;
//           case game::create_map: m_next_state = quest_state::map_creation; break;
//           case game::menu: break;
//           case game::battle: m_next_state = quest_state::battle; break;
//           case game::encounter: m_next_state = quest_state::encounter; break;
//           case game::loading:
//           case game::count: 
//           default: throw std::runtime_error("What should I do?");
//         }
//       }
      
      // обновление главного меню будет скорее включать обновление инпута
      // например тут нужно обновить кнопки меню
      return UINT32_MAX;
    }

    void main_menu_state::clean(quest_state* next_state) {
      auto base = global::get<systems::core_t>();
      base->interface_container->lua.collect_garbage();
      m_next_state = UINT32_MAX;
      
      // отсюда потенциально мы можем перейти в любые места, нужно проверить чтобы стейт был именно загрузкой
      const bool check = next_state->current_state() == world_map_loading || 
                         next_state->current_state() == world_map_generator_loading ||
                         next_state->current_state() == world_map_generating ||
                         next_state->current_state() == battle_map_loading || 
                         next_state->current_state() == encounter_loading;
      if (!check) { ASSERT(false); }
      
      //set_up_input_keys(player::in_menu);
    }
    
//     void main_menu_state::mouse_input(const size_t &time, const uint32_t &tile_index) {
//       // тут че? по умолчанию мы прост выходим
//       UNUSED_VARIABLE(time);
//       UNUSED_VARIABLE(tile_index);
//     }
//     
//     void main_menu_state::key_input(const size_t &time, const bool loading) {
//       // тут по идее просто проверка на выход из меню
//       // в общем я чет подумал что это плохая идея
//       // но при этом было бы неплохо разделить все же поведение
//       // у разных состояний игры, чтобы не захламлять функцию
//       
//       auto s = global::get<systems::core_t>();
//       const bool get_menu = current_state != utils::quest_state::map_creation &&
//                             //current_state != game_state::loading &&
//                             input::check_key(GLFW_KEY_ESCAPE, input::state::state_click | input::state::state_double_click | input::state::state_long_click);
//       const bool last_menu = current_state == utils::quest_state::main_menu && s->menu->menu_stack.size() == 1;
//       if (!loading && get_menu) {
//         if (s->interface->escape()) return;
//         if (!s->menu->exist()) {
//           if (current_state == utils::quest_state::main_menu) s->menu->push("main_menu");
//           else s->menu->push("main_menu_map");
//         } else if (!last_menu) s->menu->escape();
//         return;
//       }
//     }

//     uint32_t main_menu_state::next_state() const {
//       return m_next_state;
//     }

    map_creation_loading_state::map_creation_loading_state() : quest_state(world_map_generator_loading) {}
    void map_creation_loading_state::enter(quest_state* prev_state) {
      (void)prev_state;
      auto base = global::get<systems::core_t>();
      auto prog = base->loading_progress;
      prog->reset();
      
      prog->set_max_value(4);
      prog->set_hint1(std::string_view("Creating demiurge"));
      
      update_interface();
    }
    
    uint32_t map_creation_loading_state::update(const size_t &time, quest_state* prev_state) {
      auto base = global::get<systems::core_t>();
      auto prog = base->loading_progress;
      
      if (prog->get_value() == 3) {
        systems::from_menu_to_create_map(prog);
        auto map = global::get<systems::map_t>();
        map->start_rendering();
        systems::advance_progress(prog, "end");
        ASSERT(prog->finished());
      }
      
      if (prog->get_value() == 2) {
        auto map = global::get<systems::map_t>();
        map->create_map_container();
        map->setup_map_generator();
        systems::setup_map_generator(map);
        systems::advance_progress(prog, "finalizing");
      }
      
      if (prog->get_value() == 1) {
        prev_state->clean(this);
        systems::advance_progress(prog, "creating container");
      }
      
      if (prog->get_value() == 0) {
        systems::advance_progress(prog, "cleaning");
      }
      
      update_interface();
      
      if (prog->finished()) {
        prog->reset();
        set_up_input_keys(player::on_global_map);
        return world_map_generator;
      }
      
      UNUSED_VARIABLE(time);
      return UINT32_MAX;
    }
    
    void map_creation_loading_state::clean(quest_state* next_state) {
      auto base = global::get<systems::core_t>();
      auto inter = base->interface_container.get();
      inter->loading_table_tmp = sol::make_object(inter->lua, sol::nil);
      inter->lua.collect_garbage();
      
      if (next_state->current_state() != world_map_generator) {
        auto map_systems = global::get<systems::map_t>();
        map_systems->destroy_map_generator();
        map_systems->release_container();
      }
      m_next_state = UINT32_MAX;
    }

    map_creation_state::map_creation_state() : quest_state(world_map_generator) {}
    void map_creation_state::enter(quest_state* prev_state) {
//       auto map = global::get<systems::map_t>();
//       map->create_map_container();
//       map->setup_map_generator();
//       systems::setup_map_generator(map);
      prev_state->clean(this);
    }

//     bool map_creation_state::load(quest_state* prev_state) {
//       // теперь тут мне самостоятельно нужно почистить предыдущий стейт
//       // то есть запустить clean, запустить enter, что то еще
//       
//       auto base = global::get<systems::core_t>();
//       auto prog = base->loading_progress;
//       if (prog->reseted()) {
//         base->interface_container->force_open_layer(0, "progress_bar", {base->interface_container->lua.create_table()});
//         
//         
//         
// //         auto pool = global::get<dt::thread_pool>();
//         prog->set_max_value(4);
//         prog->set_hint1(std::string_view("Creating demiurge"));
//         prog->set_type(utils::progress_container::creating_map);
// //         prog->set_hint2(std::string_view("cleaning"));
// //         pool->submitbase([prog] () {
// //           systems::from_menu_to_create_map(prog); // похоже что эта функция работает слишком быстро
// //         });
//       }
//       
//       if (prog->get_value() == 3) {
//         systems::from_menu_to_create_map(prog);
//         auto map = global::get<systems::map_t>();
//         map->start_rendering();
//         systems::advance_progress(prog, "end");
//         ASSERT(prog->finished());
//       }
//       
//       if (prog->get_value() == 2) {
//         enter();
//         systems::advance_progress(prog, "finalizing");
//       }
//       
//       if (prog->get_value() == 1) {
//         prev_state->clean();
//         systems::advance_progress(prog, "creating container");
//       }
//       
//       if (prog->get_value() == 0) {
//         systems::advance_progress(prog, "cleaning");
//       }
// 
//       update_interface(0);
// 
//       if (prog->finished()) {
// //         auto pool = global::get<dt::thread_pool>();
// //         pool->wait();
//         base->interface_container->close_all();
//         base->interface_container->collect_garbage();
//         prog->reset();
//         set_up_input_keys(player::on_global_map);
//         return true;
//       }
// 
//       return false;
//     }

    uint32_t map_creation_state::update(const size_t &time, quest_state* prev_state) {
      (void)time;
      (void)prev_state;
      
      auto map_systems = global::get<systems::map_t>();
      ASSERT(map_systems->map_creator != nullptr);
//       map_systems->map_creator->generate();
      
//       auto base = global::get<systems::core_t>();
//       auto prog = base->loading_progress;
//       
//       if (prog->finished()) m_next_state = quest_state::world_map_generating;
      if (map_systems->map_creator->back_to_menu()) {
        return quest_state::main_menu_loading;
      }
      
      if (map_systems->map_creator->advancing()) {
        auto base = global::get<systems::core_t>();
        auto inter = base->interface_container.get();
        auto t = inter->get_generator_table();
        const auto &str = inter->serialize_table(t);
        const auto t1 = map_systems->map_creator->deserialize_table(str);
        map_systems->map_creator->set_userdata_table(t1);
        
        return quest_state::world_map_generating;
      }
      
      if (map_systems->map_creator->finished()) {
        return quest_state::world_map_loading;
      }
      
      // тут должен быть интерфейс для того чтобы понять что делать дальше
      // то есть из луа мы должны вызвать АДВАНС_ГЕНЕРАТОР что то такое
      return UINT32_MAX;
    }

    void map_creation_state::clean(quest_state* next_state) {
      auto base = global::get<systems::core_t>();
      auto inter = base->interface_container.get();
      auto map_systems = global::get<systems::map_t>();
      
      inter->clear_map_generator_functions();
      
      //if (next_state->current_state() != quest_state::world_map_generating) 
      //map_systems->destroy_map_generator();
      
      // чистим ресурсы у map_creator'a мы в загрузке карты (!)
      if (next_state->current_state() == quest_state::main_menu_loading) {
        map_systems->stop_rendering();
        map_systems->destroy_map_generator();
        map_systems->release_container();
//         auto base = global::get<systems::core_t>();
//         base->menu->clear();
//         base->interface_container->close_all();
//         base->interface_container->collect_garbage();
      }
      
      const bool strange_transition = 
        next_state->current_state() != quest_state::main_menu_loading &&
        next_state->current_state() != quest_state::world_map_generating &&
        next_state->current_state() != quest_state::world_map_loading;
        
      if (strange_transition) throw std::runtime_error("Strange transition between states");

      m_next_state = UINT32_MAX;
      //set_up_input_keys(player::in_menu);
    }
    
    map_creation_generation_state::map_creation_generation_state() : quest_state(world_map_generating) {}
    void map_creation_generation_state::enter(quest_state* prev_state) {
      (void)prev_state;
      auto map_systems = global::get<systems::map_t>();
      ASSERT(map_systems->map_creator->advancing() != map_systems->map_creator->advancing_all());
      map_systems->map_creator->generate(); // один раз вызываем для запуска шага генерации
      
      update_interface();
    }
    
    uint32_t map_creation_generation_state::update(const size_t &time, quest_state* prev_state) {
      (void)time;
      (void)prev_state;
      // слушаем генератор
      
      auto base = global::get<systems::core_t>();
      auto prog = base->loading_progress;
      auto inter = base->interface_container.get();
      
      update_interface();
      
      if (prog->finished()) {
        auto map_systems = global::get<systems::map_t>();
        const auto t = map_systems->map_creator->get_post_generation_table();
        if (t.valid() && t.get_type() == sol::type::table) {
          const auto &str = map_systems->map_creator->serialize_table(t.as<sol::table>());
          const auto t1 = inter->deserialize_table(str);
          inter->update_post_generating_table(t1);
        }
        prog->reset();
        
//         if (map_systems->map_creator->finished()) {
//           // мы наверное можем и так сделать?
//           return quest_state::world_map_loading;
//         }
        
        return quest_state::world_map_generator;
      }
      
      return UINT32_MAX;
    }
    
    void map_creation_generation_state::clean(quest_state* next_state) {
      auto base = global::get<systems::core_t>();
      auto inter = base->interface_container.get();
      inter->loading_table_tmp = sol::make_object(inter->lua, sol::nil);
      inter->lua.collect_garbage();
      
      if (next_state->current_state() != quest_state::world_map_generator) {
        // мы не можем не перейти в меню тогда
        if (next_state->current_state() != quest_state::main_menu_loading) throw std::runtime_error("Strange transition between states");
        
        auto map_systems = global::get<systems::map_t>();
        map_systems->destroy_map_generator();
        map_systems->release_container();
      }
      
      m_next_state = UINT32_MAX;
    }
    
    world_map_loading_state::world_map_loading_state() : quest_state(world_map_loading) {}
    void world_map_loading_state::enter(quest_state* prev_state) {
      (void)prev_state;
      auto base = global::get<systems::core_t>();
      auto map = global::get<systems::map_t>();
      auto prog = base->loading_progress;
      
      // если меню отвалится, то откуда брать путь загрузки? пока пусть так
      //if (base->menu->loading_path.empty()) {
      if (map->load_world.empty()) {
        prog->set_max_value(13);
        prog->set_hint1(std::string_view("Load map"));
      } else {
        prog->set_max_value(13);
        prog->set_hint1(std::string_view("Load world"));
      }
      
      set_up_input_keys(player::on_global_map);
      update_interface();
    }
    
    uint32_t world_map_loading_state::update(const size_t &time, quest_state* prev_state) {
      (void)time;
      auto base = global::get<systems::core_t>();
      auto prog = base->loading_progress;
      
      if (prog->get_value() == 2) {
        auto map = global::get<systems::map_t>();
        map->create_map_container(); // повторно создаваться не будет
        map->start_rendering();
        systems::advance_progress(prog, "starting");
        auto pool = global::get<dt::thread_pool>();
        // тут добавим задачу на загрузку
        if (map->load_world.empty()) {
          pool->submitbase([prog] () {
            systems::from_create_map_to_map(prog);
          });
        } else {
          pool->submitbase([prog] () {
            systems::from_menu_to_map(prog);
          });
        }
      }
      
      if (prog->get_value() == 1) {
        prev_state->clean(this);
        systems::advance_progress(prog, "creating container");
      }
      
      if (prog->get_value() == 0) {
        systems::advance_progress(prog, "cleaning");
      }

      update_interface();

      if (prog->finished()) {
        //base->game_ctx;
        auto pool = global::get<dt::thread_pool>();
        pool->wait();
        base->interface_container->lua.collect_garbage();
        prog->reset();
        
        return world_map;
      }
      
      return UINT32_MAX;
    }
    
    void world_map_loading_state::clean(quest_state* next_state) {
      auto base = global::get<systems::core_t>();
      auto inter = base->interface_container.get();
      inter->loading_table_tmp = sol::make_object(inter->lua, sol::nil);
      inter->lua.collect_garbage();
      
      if (next_state->current_state() != world_map) {
        if (next_state->current_state() == battle_map_loading || next_state->current_state() == encounter_loading) throw std::runtime_error("Strange transition between states");
        
        auto map_systems = global::get<systems::map_t>();
        map_systems->stop_rendering();
        map_systems->release_container();
//         base->menu->clear();
  //       base->interface_container->close_all();
  //       base->interface_container->collect_garbage();
        
        // здесь нужно удалять ресурсы используемые картой
        // текущий удалятор довольно долгий (обходим все пачки изображений в мапе)
        base->image_controller->clear_type(render::image_controller::image_type::system); // уберем из удаления позже
        base->image_controller->clear_type(render::image_controller::image_type::world_biome);
        base->image_controller->clear_type(render::image_controller::image_type::architecture); // какие то здания на карте
        base->image_controller->clear_type(render::image_controller::image_type::face); // вряд ли нужен где то еще
        if (next_state->current_state() == quest_state::main_menu_loading) { // выход в главное меню должен сопровождаться чисткой всего 
          base->image_controller->clear_type(render::image_controller::image_type::card);
          base->image_controller->clear_type(render::image_controller::image_type::heraldy);
          base->image_controller->clear_type(render::image_controller::image_type::icon);
        }
        
        base->image_controller->update_set();
      }
      
      m_next_state = UINT32_MAX;
    }

    world_map_state::world_map_state() : quest_state(world_map) {}
    void world_map_state::enter(quest_state* prev_state) {
//       auto map = global::get<systems::map_t>();
//       map->create_map_container();
//       PRINT("world_map_state::enter")
//       map->start_rendering();
      prev_state->clean(this); // ???
//       global::get<render::container>()->print_memory_info();
    }

    // тут нужно как то определить загружаем ли мы сохранение или грузим карту
//     bool world_map_state::load(quest_state* prev_state) {
// //       auto map = global::get<systems::map_t>();
//       auto base = global::get<systems::core_t>();
//       auto prog = base->loading_progress;
//       if (prog->reseted()) {
//         base->interface_container->open_layer(0, "progress_bar", {base->interface_container->lua.create_table()});
//         // нам еще нужно предусмотреть загрузку из битвы (загрузка сохранения + передача данных из битвы в карту + чистка битвы)
//         if (base->menu->loading_path.empty()) {
//           prog->set_max_value(13);
//           prog->set_hint1(std::string_view("Load map"));
//           prog->set_type(utils::progress_container::loading_created_map);
// //           pool->submitbase([prog] () {
// //             systems::from_create_map_to_map(prog);
// //           });
//         } else {
//           prog->set_max_value(13);
//           prog->set_hint1(std::string_view("Load world"));
//           prog->set_type(utils::progress_container::loading_map);
// //           pool->submitbase([prog] () {
// //             systems::from_menu_to_map(prog);
// //           });
//         }
//       }
//       
//       if (prog->get_value() == 2) {
//         enter();
//         systems::advance_progress(prog, "starting");
//         auto pool = global::get<dt::thread_pool>();
//         // тут добавим задачу на загрузку
//         if (base->menu->loading_path.empty()) {
//           pool->submitbase([prog] () {
//             systems::from_create_map_to_map(prog);
//           });
//         } else {
//           pool->submitbase([prog] () {
//             systems::from_menu_to_map(prog);
//           });
//         }
//       }
//       
//       if (prog->get_value() == 1) {
//         prev_state->clean();
//         systems::advance_progress(prog, "creating container");
//       }
//       
//       if (prog->get_value() == 0) {
//         systems::advance_progress(prog, "cleaning");
//       }
// 
//       update_interface(0);
// 
//       if (prog->finished()) {
//         auto pool = global::get<dt::thread_pool>();
//         pool->wait();
//         base->interface_container->close_all();
//         base->interface_container->collect_garbage();
//         prog->reset();
//         set_up_input_keys(player::on_global_map);
//         return true;
//       }
// 
//       return false;
//     }

    uint32_t world_map_state::update(const size_t &time, quest_state* prev_state) {
      (void)time;
      (void)prev_state;
      // пусто, потому что стейт меняется извне, ход меняется извне
//       game::advance_state();
// 
//       auto base = global::get<systems::core_t>();
//       auto current_state = game::map;
//       auto new_state = game::map;
//       // переход от карты к битве или столновению происходит не из меню
//       // хотя там можно это дело запомнить
//       if (base->menu->advance_state(current_state, new_state)) {
//         switch (new_state) {
//           case game::map: break;
//           case game::menu: m_next_state = quest_state::main_menu; break;
//           case game::battle: m_next_state = quest_state::battle; break;
//           case game::encounter: m_next_state = quest_state::encounter; break;
//           case game::create_map:
//           case game::loading:
//           default: throw std::runtime_error("What should I do?");
//         }
//       }
      
      return UINT32_MAX;
    }

    void world_map_state::clean(quest_state* next_state) {
      auto map_systems = global::get<systems::map_t>();
      auto base = global::get<systems::core_t>();
      
      if (next_state->current_state() == quest_state::battle_map_loading || next_state->current_state() == quest_state::encounter_loading) {
        // где то нужно чето куда то передать, причем после конца битвы нужно собрать данные из битвы с результатами
        // вначале нужно запустить функцию в которую мы передадим вещи тип армий и места в которых они столкнулись
        // все равно лучше бы чистить данные после создания карты
      }
      
      map_systems->stop_rendering();
      map_systems->release_container();
//       base->menu->clear();
//       base->interface_container->close_all();
//       base->interface_container->collect_garbage();
      
      // здесь нужно удалять ресурсы используемые картой
      // текущий удалятор довольно долгий (обходим все пачки изображений в мапе)
      base->image_controller->clear_type(render::image_controller::image_type::system); // уберем из удаления позже
      base->image_controller->clear_type(render::image_controller::image_type::world_biome);
      base->image_controller->clear_type(render::image_controller::image_type::architecture); // какие то здания на карте
      base->image_controller->clear_type(render::image_controller::image_type::face); // вряд ли нужен где то еще
      if (next_state->current_state() == quest_state::main_menu_loading) { // выход в главное меню должен сопровождаться чисткой всего 
        base->image_controller->clear_type(render::image_controller::image_type::card);
        base->image_controller->clear_type(render::image_controller::image_type::heraldy);
        base->image_controller->clear_type(render::image_controller::image_type::icon);
      }
      
      base->image_controller->update_set();
      
      m_next_state = UINT32_MAX;
      //set_up_input_keys(player::in_menu);
    }
    
    struct battle_loading_state::battle_generator_data {
      sol::state lua;
      utils::random_engine_st random;
      FastNoise noiser;
      
      map::generator::container* container;
      // где то тут же нужно добавить контейнер для таблиц
      map::creator::table_container_t table_container;
      
      battle_generator_data() : container(nullptr) {
        static_assert(sizeof(uint64_t) == sizeof(size_t));
        const uint64_t seed = global::advance_state();
        random.set_seed(seed);
        noiser.SetSeed(reinterpret_cast<const int*>(&seed)[0]);
        global::get(&table_container);
      }
      
      ~battle_generator_data() { 
        global::get<map::creator::table_container_t>(reinterpret_cast<map::creator::table_container_t*>(SIZE_MAX));
        destroy_container();
      }
      
      void create_container(const size_t &tiles_count) { container = new map::generator::container(tiles_count); }
      void destroy_container() { delete container; container = nullptr; }
    };
    
    battle_loading_state::battle_loading_state() : quest_state(battle_map_loading), ctx(nullptr) {}
    battle_loading_state::~battle_loading_state() { destroy_state(); }
    
    // 4 стартовых (вход, сбор данных о карте, чистка ресурсов, подготовка к генерации) + 2 загрузки (валидация + загрузка)
    const size_t additional_step_count = 4 + 2;
    
    void battle_loading_state::enter(quest_state* prev_state) {
      (void)prev_state;
      
      auto base = global::get<systems::core_t>();
      auto prog = base->loading_progress;
      
      //base->interface_container->open_layer(0, "background", {base->interface_container->lua.create_table()}); // картинка
      //base->interface_container->open_layer(1, "progress_bar", {base->interface_container->lua.create_table()});
      //auto obj = sol::make_object(base->interface_container->lua, prog);
      // нам еще нужно предусмотреть загрузку из битвы (загрузка сохранения + передача данных из битвы в карту + чистка битвы)
      prog->set_max_value(additional_step_count);
      prog->set_hint1(std::string_view("Creating battle"));
      prog->set_hint2(std::string_view("getting information from map"));
        
      create_state();
      
      update_interface();
      set_up_input_keys(player::in_menu);
    }
    
    uint32_t battle_loading_state::update(const size_t &time, quest_state* prev_state) {
      auto base = global::get<systems::core_t>();
//       auto battle = global::get<systems::battle_t>();
      auto prog = base->loading_progress;
      auto pool = global::get<dt::thread_pool>();
      
      if (prog->get_value() == 3) {
        // тут создадим карту битв, или не тут? мы вполне можем создать позже
        // загрузку скрыть за картинкой и тогда непосредственно контейнер мы можем создать позже
        // нет, контейнер нам нужен для заполнения некоторых значений при генерации
        
        auto battle = global::get<systems::battle_t>();
        battle->create_map_container();
        battle->create_render_stages();
        
        // тут у нас создалась карта, но вот количество юнитов еще недоступно
//         global::get<render::battle::tile_optimizer>()->update_containers();
        
        battle->start_rendering();
        //battle->setup_generator_random();
        sol::state &local_lua = ctx->lua;
        local_lua["usertable"].get_or_create<sol::table>();
        systems::advance_progress(prog, "starting");
        
        //battle->setup_generator_container(128*128);
        ctx->create_container(128*128);
        
        const sol::table t = local_lua["config_table"];
        const sol::table gen = t["generator"];
        const size_t funcs_size = gen.size();
        prog->set_max_value(funcs_size + additional_step_count);
        
        auto ctx_t = local_lua["ctx_table"].get_or_create<sol::table>();
        ctx_t["map"] = battle->map;
        ctx_t["random"] = &ctx->random;
        ctx_t["noiser"] = &ctx->noiser;
        ctx_t["container"] = ctx->container;
        
        // тут добавим задачу на загрузку
        pool->submitbase([&local_lua, prog] () {
          systems::from_map_to_battle_part2(local_lua, prog);
        });
      }
      
      if (prog->get_value() == 2) {
        // прежде чем почистить предыдущий стейт, нужно сохранить ворлд мап
        if (prev_state != nullptr) prev_state->clean(this);
        systems::advance_progress(prog, "creating container");
      }
      
      if (prog->get_value() == 0) {
        //systems::advance_progress(prog, "cleaning");
        systems::advance_progress(prog, "getting information from map");
        sol::state &local_lua = ctx->lua;
        pool->submitbase([&local_lua, prog] () {
          systems::from_map_to_battle_part1(local_lua, prog); // грузим конфиг
        });
      }
      
      // тут мы должны сгенерировать высоты и биомы для боевых тайлов, я вспомнил вот что, тайлы стен в АОВ3
      // это на самом деле два тайла: один собственно площадка где стоит отряд защиты, второй тайл это стены,
      // на который помещается армия атакующая при штурме
      // тут нет необходимости делать отдельный контейнер для всех функций, 
      // нам просто нужно где то хранить положение конфига для создания карт (по идее это должно придти при генерации ворлдмапы)
      // получается что тут мы сначало грузим конфиг, подгружаем функцию которая соберет нужные данные из карты,
      // сохраняем и удалям карту, создаем контейнер, запускаем функции генерации битвы
      
      update_interface();
      
      if (prog->finished()) {
        pool->wait();
        base->interface_container->lua.collect_garbage();
        prog->reset();
        //battle->release_generator_data();
        destroy_state();
        return battle_map;
      }
      
      return UINT32_MAX;
    }
    
    void battle_loading_state::clean(quest_state* next_state) { 
      auto base = global::get<systems::core_t>();
      auto inter = base->interface_container.get();
      inter->loading_table_tmp = sol::make_object(inter->lua, sol::nil);
      inter->lua.collect_garbage();
      
      // неожиданный сценарий
      if (next_state->current_state() != battle_map) {
        auto battle = global::get<systems::battle_t>();
        battle->stop_rendering();
        battle->release_container();
      }
      
      m_next_state = UINT32_MAX; 
    }
    
    void battle_loading_state::create_state() { ctx = new battle_generator_data; }
    void battle_loading_state::destroy_state() { delete ctx; ctx = nullptr; }

    battle_state::battle_state() : quest_state(battle_map) {} //lua(nullptr),
    battle_state::~battle_state() {}
    void battle_state::enter(quest_state* prev_state) {
      //ASSERT(false);
      prev_state->clean(this);
      
      set_up_input_keys(player::on_battle_map);
    }

//     bool battle_state::load(quest_state* prev_state) { 
//       // что тут? во первых мы должны запустить функцию которая наберет нужные данные из карты
//       // либо переделать немного систему чтобы удалять все данные кроме контекста
//       // с другой стороны можно сделать какую то статичную С++ функцию в которой мы сможем указать че угодно
//       // что мне вообще нужно сделать? нужно скопировать состояние из армии, а именно просмотреть отряды,
//       // скопировать типы, скопировать характеристики, скопировать некоторые данные персонажей,
//       // просмотреть биомы вокруг (тоже оттуда что то скопировать), и все?
//       
//       // после того как я все скопирую, почищу данные карты, создам каркас карты битвы (?), нужно будет сгенерировать поле битвы
//       // то есть у меня будут крупные тайлы, на тайлах какие то красивости, расставленные предварительно отряды армии,
//       // ну короч надо бы наверное сначала заняться рендером
//       
//       // куда убрать sol::state lua ??????????????????????
//       // он нужен только при загрузке
//       
//       auto base = global::get<systems::core_t>();
//       auto battle = global::get<systems::battle_t>();
//       auto prog = base->loading_progress;
//       auto pool = global::get<dt::thread_pool>();
//       if (prog->reseted()) {
//         //base->interface_container->open_layer(0, "background", {base->interface_container->lua.create_table()}); // картинка
//         //base->interface_container->open_layer(1, "progress_bar", {base->interface_container->lua.create_table()});
//         auto obj = sol::make_object(base->interface_container->lua, prog);
//         base->interface_container->open_layer(1, "progress_bar", {obj});
//         // нам еще нужно предусмотреть загрузку из битвы (загрузка сохранения + передача данных из битвы в карту + чистка битвы)
//         prog->set_max_value(additional_step_count);
//         prog->set_hint1(std::string_view("Creating battle"));
//         prog->set_hint2(std::string_view("getting information from map"));
//         prog->set_type(utils::progress_container::loading_battle);
//         
//         create_state();
//       }
//       
//       if (prog->get_value() == 3) {
//         enter();
//         //battle->setup_generator_random();
//         sol::state &local_lua = ctx->lua;
//         local_lua["usertable"].get_or_create<sol::table>();
//         systems::advance_progress(prog, "starting");
//         
//         //battle->setup_generator_container(128*128);
//         ctx->create_container(128*128);
//         
//         const sol::table t = local_lua["config_table"];
//         const sol::table gen = t["generator"];
//         const size_t funcs_size = gen.size();
//         prog->set_max_value(funcs_size + additional_step_count);
//         
//         auto ctx_t = local_lua["ctx_table"].get_or_create<sol::table>();
//         ctx_t["map"] = battle->map;
//         ctx_t["random"] = &ctx->random;
//         ctx_t["noiser"] = &ctx->noiser;
//         ctx_t["container"] = ctx->container;
//         
//         // тут добавим задачу на загрузку
//         pool->submitbase([&local_lua, prog] () {
//           systems::from_map_to_battle_part2(local_lua, prog);
//         });
//       }
//       
//       if (prog->get_value() == 2) {
//         // прежде чем почистить предыдущий стейт, нужно сохранить ворлд мап
//         if (prev_state != nullptr) prev_state->clean();
//         systems::advance_progress(prog, "creating container");
//       }
//       
//       if (prog->get_value() == 0) {
//         //systems::advance_progress(prog, "cleaning");
//         systems::advance_progress(prog, "getting information from map");
//         sol::state &local_lua = ctx->lua;
//         pool->submitbase([&local_lua, prog] () {
//           systems::from_map_to_battle_part1(local_lua, prog); // грузим конфиг
//         });
//       }
//       
//       // тут мы должны сгенерировать высоты и биомы для боевых тайлов, я вспомнил вот что, тайлы стен в АОВ3
//       // это на самом деле два тайла: один собственно площадка где стоит отряд защиты, второй тайл это стены,
//       // на который помещается армия атакующая при штурме
//       // тут нет необходимости делать отдельный контейнер для всех функций, 
//       // нам просто нужно где то хранить положение конфига для создания карт (по идее это должно придти при генерации ворлдмапы)
//       // получается что тут мы сначало грузим конфиг, подгружаем функцию которая соберет нужные данные из карты,
//       // сохраняем и удалям карту, создаем контейнер, запускаем функции генерации битвы
//       
// //       update_interface(1);
//       
//       if (prog->finished()) {
//         pool->wait();
//         base->interface_container->close_all();
//         base->interface_container->collect_garbage();
//         prog->reset();
//         //battle->release_generator_data();
//         destroy_state();
//         set_up_input_keys(player::on_battle_map);
//         return true;
//       }
//       
//       return false;
//     }
    
    uint32_t battle_state::update(const size_t& time, quest_state* prev_state) { UNUSED_VARIABLE(time); UNUSED_VARIABLE(prev_state); return UINT32_MAX; }
    void battle_state::clean(quest_state* next_state) {
      auto battle = global::get<systems::battle_t>();
      battle->stop_rendering();
      battle->release_container();
      
      UNUSED_VARIABLE(next_state);
    }
    
    encounter_loading_state::encounter_loading_state() : quest_state(encounter_loading) {}
    void encounter_loading_state::enter(quest_state* prev_state) { ASSERT(false); UNUSED_VARIABLE(prev_state); update_interface(); set_up_input_keys(player::in_menu); } 
    uint32_t encounter_loading_state::update(const size_t &time, quest_state* prev_state) { UNUSED_VARIABLE(time); UNUSED_VARIABLE(prev_state); return UINT32_MAX; }
    void encounter_loading_state::clean(quest_state* next_state) { UNUSED_VARIABLE(next_state); }

    encounter_state::encounter_state() : quest_state(encounter) {}
    void encounter_state::enter(quest_state* prev_state) {
      UNUSED_VARIABLE(prev_state);
      ASSERT(false);
      set_up_input_keys(player::on_hero_battle_map);
    }

    uint32_t encounter_state::update(const size_t &time, quest_state* prev_state) { UNUSED_VARIABLE(time); UNUSED_VARIABLE(prev_state); return UINT32_MAX; }
    void encounter_state::clean(quest_state* next_state) { UNUSED_VARIABLE(next_state); }
  }
}
