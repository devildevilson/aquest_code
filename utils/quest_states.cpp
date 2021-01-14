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
#include "bin/battle_map.h"
#include "render/image_controller.h"
#include "render/battle_render_stages.h"

namespace devils_engine {
  namespace utils {
    void update_interface(const uint32_t &progress_bar_index) {
      auto base = global::get<systems::core_t>();
      auto prog = base->loading_progress;
      auto interface = base->interface_container;
      ASSERT(interface != nullptr);
      sol::table table;
      if (interface->openned_layers[progress_bar_index].args.size() > 0) {
        auto &val = interface->openned_layers[progress_bar_index].args[0];
        ASSERT(val.is<sol::table>());
        table = val.as<sol::table>();
      } else {
        if (interface->open_layers[progress_bar_index].first.size() == 0) return;
        auto &val = interface->open_layers[progress_bar_index].first[0];
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

    main_menu_state::main_menu_state() : quest_state(main_menu) {}
    void main_menu_state::enter() {
      // эта функция будет запускаться перед загрузкой
    }

    bool main_menu_state::load(quest_state* prev_state) {
      if (prev_state != nullptr) prev_state->clean();
      
      auto base = global::get<systems::core_t>();
      // грузанем здесь новую картинку для заднего фона
      if (!base->interface_container->is_visible(uint32_t(utils::interface_container::last_layer()))) {
        base->menu->push("main_menu");
      }
      
      enter();

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
          case game_state::battle: m_next_state = quest_state::battle; break;
          case game_state::encounter: m_next_state = quest_state::encounter; break;
          case game_state::loading:
          case game_state::count: throw std::runtime_error("What should I do?");
        }
      }
    }

    void main_menu_state::clean() {
      auto base = global::get<systems::core_t>();
      base->menu->clear();
      //base->interface_container->close_all();
      //base->interface_container->collect_garbage();
      m_next_state = UINT32_MAX;
    }

//     uint32_t main_menu_state::next_state() const {
//       return m_next_state;
//     }

    void map_creation_state::enter() {
      auto map = global::get<systems::map_t>();
      map->create_map_container();
      map->setup_map_generator();
      systems::setup_map_generator(map);
    }

    map_creation_state::map_creation_state() : quest_state(map_creation) {}
    bool map_creation_state::load(quest_state* prev_state) {
      // теперь тут мне самостоятельно нужно почистить предыдущий стейт
      // то есть запустить clean, запустить enter, что то еще
      
      auto base = global::get<systems::core_t>();
      auto prog = base->loading_progress;
      if (prog->reseted()) {
        base->interface_container->force_open_layer(0, "progress_bar", {base->interface_container->lua.create_table()});
        
        
        
//         auto pool = global::get<dt::thread_pool>();
        prog->set_max_value(4);
        prog->set_hint1(std::string_view("Creating demiurge"));
        prog->set_type(utils::progress_container::creating_map);
//         prog->set_hint2(std::string_view("cleaning"));
//         pool->submitbase([prog] () {
//           systems::from_menu_to_create_map(prog); // похоже что эта функция работает слишком быстро
//         });
      }
      
      if (prog->get_value() == 3) {
        systems::from_menu_to_create_map(prog);
        systems::advance_progress(prog, "end");
        ASSERT(prog->finished());
      }
      
      if (prog->get_value() == 2) {
        enter();
        systems::advance_progress(prog, "finalizing");
      }
      
      if (prog->get_value() == 1) {
        prev_state->clean();
        systems::advance_progress(prog, "creating container");
      }
      
      if (prog->get_value() == 0) {
        systems::advance_progress(prog, "cleaning");
      }

      update_interface(0);

      if (prog->finished()) {
//         auto pool = global::get<dt::thread_pool>();
//         pool->wait();
        base->interface_container->close_all();
        base->interface_container->collect_garbage();
        prog->reset();
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
        base->menu->clear();
//         base->interface_container->close_all();
//         base->interface_container->collect_garbage();
      }

      m_next_state = UINT32_MAX;
    }

    world_map_state::world_map_state() : quest_state(world_map) {}
    void world_map_state::enter() {
      auto map = global::get<systems::map_t>();
      map->create_map_container();
//       PRINT("world_map_state::enter")
      map->start_rendering();
    }

    // тут нужно как то определить загружаем ли мы сохранение или грузим карту
    bool world_map_state::load(quest_state* prev_state) {
//       auto map = global::get<systems::map_t>();
      auto base = global::get<systems::core_t>();
      auto prog = base->loading_progress;
      if (prog->reseted()) {
        base->interface_container->open_layer(0, "progress_bar", {base->interface_container->lua.create_table()});
        // нам еще нужно предусмотреть загрузку из битвы (загрузка сохранения + передача данных из битвы в карту + чистка битвы)
        if (base->menu->loading_path.empty()) {
          prog->set_max_value(11);
          prog->set_hint1(std::string_view("Load map"));
          prog->set_type(utils::progress_container::loading_created_map);
//           pool->submitbase([prog] () {
//             systems::from_create_map_to_map(prog);
//           });
        } else {
          prog->set_max_value(14);
          prog->set_hint1(std::string_view("Load world"));
          prog->set_type(utils::progress_container::loading_map);
//           pool->submitbase([prog] () {
//             systems::from_menu_to_map(prog);
//           });
        }
      }
      
      if (prog->get_value() == 2) {
        enter();
        systems::advance_progress(prog, "starting");
        auto pool = global::get<dt::thread_pool>();
        // тут добавим задачу на загрузку
        if (base->menu->loading_path.empty()) {
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
        prev_state->clean();
        systems::advance_progress(prog, "creating container");
      }
      
      if (prog->get_value() == 0) {
        systems::advance_progress(prog, "cleaning");
      }

      update_interface(0);

      if (prog->finished()) {
        auto pool = global::get<dt::thread_pool>();
        pool->wait();
        base->interface_container->close_all();
        base->interface_container->collect_garbage();
        prog->reset();
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
          case game_state::battle: m_next_state = quest_state::battle; break;
          case game_state::encounter: m_next_state = quest_state::encounter; break;
          case game_state::create_map:
          case game_state::loading:
          default: throw std::runtime_error("What should I do?");
        }
      }
    }

    void world_map_state::clean() {
      auto map_systems = global::get<systems::map_t>();
      auto base = global::get<systems::core_t>();
      
      if (m_next_state == quest_state::battle || m_next_state == quest_state::encounter) {
        // где то нужно чето куда то передать, причем после конца битвы нужно собрать данные из битвы с результатами
        // вначале нужно запустить функцию в которую мы передадим вещи тип армий и места в которых они столкнулись
        // все равно лучше бы чистить данные после создания карты
      }
      
      map_systems->stop_rendering();
      map_systems->release_container();
      base->menu->clear();
//       base->interface_container->close_all();
//       base->interface_container->collect_garbage();
      
      // здесь нужно удалять ресурсы используемые картой
      // текущий удалятор довольно долгий (обходим все пачки изображений в мапе)
      base->image_controller->clear_type(render::image_controller::image_type::system); // уберем из удаления позже
      base->image_controller->clear_type(render::image_controller::image_type::world_biome);
      base->image_controller->clear_type(render::image_controller::image_type::architecture); // какие то здания на карте
      base->image_controller->clear_type(render::image_controller::image_type::face); // вряд ли нужен где то еще
      if (m_next_state == quest_state::main_menu) { // выход в главное меню должен сопровождаться чисткой всего 
        base->image_controller->clear_type(render::image_controller::image_type::card);
        base->image_controller->clear_type(render::image_controller::image_type::heraldy);
        base->image_controller->clear_type(render::image_controller::image_type::icon);
      }
      
      m_next_state = UINT32_MAX;
    }
    
    struct battle_state::battle_generator_data {
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

    battle_state::battle_state() : quest_state(battle), ctx(nullptr) {} //lua(nullptr),
    battle_state::~battle_state() { destroy_state(); }
    void battle_state::enter() {
      //ASSERT(false);
      // тут создадим карту битв, или не тут? мы вполне можем создать позже
      // загрузку скрыть за картинкой и тогда непосредственно контейнер мы можем создать позже
      
      auto battle = global::get<systems::battle_t>();
      battle->create_map_container();
      battle->create_render_stages();
      
      global::get<render::battle::tile_optimizer>()->update_containers();
      
      battle->start_rendering();
    }

    bool battle_state::load(quest_state* prev_state) { 
      // что тут? во первых мы должны запустить функцию которая наберет нужные данные из карты
      // либо переделать немного систему чтобы удалять все данные кроме контекста
      // с другой стороны можно сделать какую то статичную С++ функцию в которой мы сможем указать че угодно
      // что мне вообще нужно сделать? нужно скопировать состояние из армии, а именно просмотреть отряды,
      // скопировать типы, скопировать характеристики, скопировать некоторые данные персонажей,
      // просмотреть биомы вокруг (тоже оттуда что то скопировать), и все?
      
      // после того как я все скопирую, почищу данные карты, создам каркас карты битвы (?), нужно будет сгенерировать поле битвы
      // то есть у меня будут крупные тайлы, на тайлах какие то красивости, расставленные предварительно отряды армии,
      // ну короч надо бы наверное сначала заняться рендером
      
      // 4 стартовых (вход, сбор данных о карте, чистка ресурсов, подготовка к генерации) + 2 загрузки (валидация + загрузка)
      const size_t additional_step_count = 4 + 2;
      
      // куда убрать sol::state lua ??????????????????????
      // он нужен только при загрузке
      
      auto base = global::get<systems::core_t>();
      auto battle = global::get<systems::battle_t>();
      auto prog = base->loading_progress;
      auto pool = global::get<dt::thread_pool>();
      if (prog->reseted()) {
        //base->interface_container->open_layer(0, "background", {base->interface_container->lua.create_table()}); // картинка
        //base->interface_container->open_layer(1, "progress_bar", {base->interface_container->lua.create_table()});
        auto obj = sol::make_object(base->interface_container->lua, prog);
        base->interface_container->open_layer(1, "progress_bar", {obj});
        // нам еще нужно предусмотреть загрузку из битвы (загрузка сохранения + передача данных из битвы в карту + чистка битвы)
        prog->set_max_value(additional_step_count);
        prog->set_hint1(std::string_view("Creating battle"));
        prog->set_hint2(std::string_view("getting information from map"));
        prog->set_type(utils::progress_container::loading_battle);
        
        create_state();
      }
      
      if (prog->get_value() == 3) {
        enter();
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
        if (prev_state != nullptr) prev_state->clean();
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
      
//       update_interface(1);
      
      if (prog->finished()) {
        pool->wait();
        base->interface_container->close_all();
        base->interface_container->collect_garbage();
        prog->reset();
        //battle->release_generator_data();
        destroy_state();
        return true;
      }
      
      return false;
    }
    
    void battle_state::update(const size_t& time) { UNUSED_VARIABLE(time); }
    void battle_state::clean() {
      auto battle = global::get<systems::battle_t>();
      battle->stop_rendering();
      battle->release_container();
    }
    
    void battle_state::create_state() { ctx = new battle_generator_data; }
    void battle_state::destroy_state() { delete ctx; ctx = nullptr; }

    encounter_state::encounter_state() : quest_state(encounter) {}
    void encounter_state::enter() {
      ASSERT(false);
    }

    bool encounter_state::load(quest_state* prev_state) { UNUSED_VARIABLE(prev_state); return false; }
    void encounter_state::update(const size_t &time) { UNUSED_VARIABLE(time); }
    void encounter_state::clean() {}
  }
}
