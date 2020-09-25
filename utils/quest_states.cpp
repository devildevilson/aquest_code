#include "quest_states.h"

#include "globals.h"
#include "systems.h"
#include "interface_container.h"
#include "main_menu.h"
#include "progress_container.h"
#include "thread_pool.h"
#include "bin/map_creator.h"
#include "bin/loading_functions.h"
#include "bin/logic.h"

namespace devils_engine {
  namespace utils {
    void update_interface() {
      auto base = global::get<systems::core_t>();
      auto prog = base->loading_progress;
      auto interface = base->interface_container;
      ASSERT(interface != nullptr);
      sol::table table;
      if (interface->openned_layers[0].args.size() > 0) {
        auto &val = interface->openned_layers[0].args[0];
        ASSERT(val.is<sol::table>());
        table = val.as<sol::table>();
      } else {
        auto &val = interface->open_layers[0].first[0];
        ASSERT(val.is<sol::table>());
        table = val.as<sol::table>();
      }
      table["current_step"] = prog->get_value();
      table["step_count"] = prog->get_max_value();
      table["hint1"] = prog->get_hint1();
      table["hint2"] = prog->get_hint2();
      table["hint3"] = prog->get_hint3();
      table["type"] = prog->get_type();
    }
    
    main_menu_state::main_menu_state() {}
    void main_menu_state::enter() {
      // эта функция будет запускаться перед загрузкой
    }
    
    bool main_menu_state::load() {
      auto base = global::get<systems::core_t>();
      // грузанем здесь новую картинку для заднего фона
      if (!base->interface_container->is_visible(utils::interface_container::last_layer())) {
        base->menu->push("main_menu");
      }
      
      return true;
    }
    
    void main_menu_state::update(const size_t &time) {
      (void)time;
      auto base = global::get<systems::core_t>();
      game_state current_state = game_state::menu;
      game_state new_state = game_state::menu;
      if (base->menu->advance_state(current_state, new_state)) {
        switch (new_state) {
          case game_state::map: m_next_state = quest_state::world_map; break;
          case game_state::create_map: m_next_state = quest_state::map_creation; break;
          case game_state::menu: break;
          case game_state::battle:
          case game_state::encounter:
          case game_state::loading:
          case game_state::count: throw std::runtime_error("What should I do?");
        }
      }
    }
    
    void main_menu_state::clean() {
      auto base = global::get<systems::core_t>();
      base->interface_container->close_all();
      base->menu->clear();
      m_next_state = UINT32_MAX;
    }
    
//     uint32_t main_menu_state::next_state() const {
//       return m_next_state;
//     }
    
    void map_creation_state::enter() {
      auto map = global::get<systems::map_t>();
      map->create_map_container();
//       map->setup_map_generator();
      
    }
    
    map_creation_state::map_creation_state() {}
    bool map_creation_state::load() {
      auto base = global::get<systems::core_t>();
      auto prog = base->loading_progress;
      if (prog->reseted()) {
        base->interface_container->open_layer(0, "progress_bar", {base->interface_container->lua.create_table()});
        auto pool = global::get<dt::thread_pool>();
        prog->set_max_value(1);
        pool->submitbase([prog] () {
          systems::from_menu_to_create_map(prog);
        });
      }
      
      update_interface();
      
      if (base->loading_progress->finished()) {
        base->loading_progress->reset();
        return true;
      }
      
      return false;
    }
    
    void map_creation_state::update(const size_t &time) {
      (void)time;
      auto map_systems = global::get<systems::map_t>();
      map_systems->map_creator->generate();
      
      if (map_systems->map_creator->back_to_menu()) m_next_state = quest_state::main_menu;
      if (map_systems->map_creator->finished()) m_next_state = quest_state::world_map;
    }
    
    void map_creation_state::clean() {
      // чистим ресурсы у map_creator'a мы в загрузке карты (!)
      if (m_next_state == quest_state::main_menu) {
        auto map_systems = global::get<systems::map_t>();
        auto base = global::get<systems::core_t>();
        map_systems->destroy_map_generator();
        map_systems->release_container();
        base->interface_container->close_all();
      }
      
      m_next_state = UINT32_MAX;
    }
    
    world_map_state::world_map_state() {}
    void world_map_state::enter() {
      auto map = global::get<systems::map_t>();
      map->create_map_container();
//       PRINT("world_map_state::enter")
    }
    
    // тут нужно как то определить загружаем ли мы сохранение или грузим карту
    bool world_map_state::load() {
//       auto map = global::get<systems::map_t>();
      auto base = global::get<systems::core_t>();
      auto prog = base->loading_progress;
      if (prog->reseted()) {
        base->interface_container->open_layer(0, "progress_bar", {base->interface_container->lua.create_table()});
        auto pool = global::get<dt::thread_pool>();
        prog->set_max_value(1);
        if (base->menu->loading_path.empty()) {
          // мы здесь оказались после генерации
          pool->submitbase([prog] () {
            systems::from_create_map_to_map(prog);
          });
        } else {
          pool->submitbase([prog] () {
            systems::from_menu_to_map(prog);
          });
        }
      }
      
      update_interface();
      
      if (base->loading_progress->finished()) {
        base->loading_progress->reset();
        return true;
      }
      
      return false;
    }
    
    void world_map_state::update(const size_t &time) {
      (void)time;
      game::advance_state();
      
      auto base = global::get<systems::core_t>();
      game_state current_state = game_state::map;
      game_state new_state = game_state::map;
      if (base->menu->advance_state(current_state, new_state)) {
        switch (new_state) {
          case game_state::map: break;
          case game_state::menu: m_next_state = quest_state::main_menu; break;
          case game_state::create_map:
          case game_state::battle:
          case game_state::encounter:
          case game_state::loading:
          case game_state::count: throw std::runtime_error("What should I do?");
        }
      }
    }
    
    void world_map_state::clean() {
      auto map_systems = global::get<systems::map_t>();
      auto base = global::get<systems::core_t>();
      map_systems->release_container();
      base->interface_container->close_all();
      m_next_state = UINT32_MAX;
    }
    
    battle_state::battle_state() {}
    void battle_state::enter() {
      ASSERT(false);
    }
    
    bool battle_state::load() {}
    void battle_state::update(const size_t &time) {}
    void battle_state::clean() {}
    
    encounter_state::encounter_state() {}
    void encounter_state::enter() {
      ASSERT(false);
    }
    
    bool encounter_state::load() {}
    void encounter_state::update(const size_t &time) {}
    void encounter_state::clean() {}
  }
}
