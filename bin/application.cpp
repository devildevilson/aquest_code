#include "application.h"

#include "GLFW/glfw3.h"

#include "interface_context.h"
#include "loading_functions.h"
#include "map_creator.h"

#include "core/context.h"
#include "core/internal_lua_state.h"

#include "utils/progress_container.h"
#include "utils/globals.h"
#include "utils/input.h"
#include "utils/game_context.h"
#include "utils/interface_container2.h"

#include "render/container.h"
#include "render/window.h"
#include "render/stages.h"
#include "render/image_controller.h"
#include "render/pass.h"
#include "render/targets.h"

#include "whereami.h"

#include <filesystem>

// calculating_battle_results, calculating_encounter_results

#define STATES_LIST \
  STATE_FUNC(loading_menu) \
  STATE_FUNC(loading_gen_world) \
  STATE_FUNC(loading_save_world) \
  STATE_FUNC(loading_gen) \
  STATE_FUNC(loading_battle) \
  STATE_FUNC(loading_encounter) \
  STATE_FUNC(generating_world) \
  STATE_FUNC(main_menu) \
  STATE_FUNC(world) \
  STATE_FUNC(generator) \
  STATE_FUNC(battle) \
  STATE_FUNC(encounter) \
  
#define EVENTS_LIST \
  EVENT_FUNC(init) \
  EVENT_FUNC(loading_end) \
  EVENT_FUNC(generator_end) \
  EVENT_FUNC(generator_step) \
  EVENT_FUNC(choosed_gen) \
  EVENT_FUNC(choosed_saved_game) \
  EVENT_FUNC(return_to_menu) \
  EVENT_FUNC(start_battle) \
  EVENT_FUNC(start_encounter) \
  EVENT_FUNC(start_gen) \
  EVENT_FUNC(battle_end) \
  EVENT_FUNC(encounter_end) \

namespace devils_engine {
  namespace core {
    static void callback(int error, const char* description);
    static int lua_panic(lua_State* s);
    static void scrollCallback(GLFWwindow*, double xoffset, double yoffset);
    static void charCallback(GLFWwindow*, unsigned int c);
    static void mouseButtonCallback(GLFWwindow*, int button, int action, int mods);
    static void keyCallback(GLFWwindow*, int key, int scancode, int action, int mods);
    static void window_resize_callback(GLFWwindow*, int w, int h);
    static void iconifyCallback(GLFWwindow*, int iconified);
    static void focusCallback(GLFWwindow*, int focused);
    static const char* getClipboard(void* user_data);
    static void setClipboard(void* user_data, const char* text);
    static void setup_callbacks(render::window* window);
    
    static void sync(utils::frame_time &frame_time, const size_t &time);
    
#define STATE(x) static constexpr dsml::State<struct x##_> x{};
#define STATE_FUNC(name) STATE(name)
    STATES_LIST
#undef STATE_FUNC
#undef STATE

#define EVENT(x) static constexpr dsml::Event<struct x##_> x{};
#define EVENT_FUNC(name) EVENT(name)
    EVENTS_LIST
#undef EVENT_FUNC
#undef EVENT

    static void set_application_path() {
      int32_t dirname = 0;
      const int32_t length = wai_getExecutablePath(nullptr, 0, &dirname);
      ASSERT(length > 0);

      std::vector<char> array(length+1);
      wai_getExecutablePath(array.data(), length, &dirname);

      array[length] = '\0'; // весь путь
      array[dirname+1] = '\0'; // путь до папки

      std::filesystem::path p = std::string(array.data());
      //PRINT(p)           // ...apates_quest/bin/
      p = p.parent_path(); // ...apates_quest/bin
      p = p.parent_path(); // ...apates_quest
      p += "/";            // ...apates_quest/
      p.make_preferred();
      const std::string dir_path = p.string(); // может быть хранить путь в std::filesystem::path? лан пока так оставим

      global g;
      g.set_root_directory(dir_path);
    }
    
    glfw_t::glfw_t() {
      if (glfwInit() != GLFW_TRUE) { throw std::runtime_error("Cannot init glfw!"); }
      if (glfwVulkanSupported() != GLFW_TRUE) { throw std::runtime_error("Vulkan is not supported!"); }
      glfwSetErrorCallback(callback);
    }
    
    glfw_t::~glfw_t() { glfwTerminate(); }
    
    static void poll_events() {
      glfwPollEvents();
    }
    
    block_input_t::block_input_t() { input::block(); }
    block_input_t::~block_input_t() { input::unblock(); }
    
    void set_up_input_keys(const uint32_t &state) {
      auto core = global::get<systems::core_t>();
      ASSERT(state < player::states_count);
      auto key_map_ptr = core->keys_mapping[state];
      
      block_input_t bi;
      if (state == player::in_menu) input::set_key_map(nullptr);
      else input::set_key_map(key_map_ptr);
    }
    
    // в этих функциях должна быть передача смены состояния в приложение, как сделать?
    // лучше чтобы эта функция была бы локальной для application
    static void loading_menu_action() {
      set_up_input_keys(player::in_menu);
      auto base = global::get<systems::core_t>();
      base->internal.reset(nullptr);
      global::get<utils::deffered_tasks>()->add([] (const size_t &) -> bool {
        return true;
      });
    }
    
    struct application::state_machine_decl {
      auto operator()() const noexcept {
        using namespace dsml::literals;
        using namespace dsml::operators;
        
        const auto display = [] (const std::string_view& msg) {
          return [msg] () { std::cout << msg << "\n"; };
        };
        
        const auto set_state = [] (const uint32_t s) {
          return [s] () { global::get<systems::core_t>()->game_ctx->state = s; };
        };
        
        const auto loading_menu_action      = dsml::callee(&application::loading_menu_action);
        const auto loading_generator_action = dsml::callee(&application::loading_generator_action);
        const auto loading_gen_world_action = dsml::callee(&application::loading_gen_world_action);
        const auto loading_sav_world_action = dsml::callee(&application::loading_sav_world_action);
        const auto loading_battle_action    = dsml::callee(&application::loading_battle_action);
        const auto loading_encounter_action = dsml::callee(&application::loading_encounter_action);
        const auto generating_world_action  = dsml::callee(&application::generating_world_action);
        const auto free_world_data          = dsml::callee(&application::free_world_data);
        const auto partial_free_world_data  = dsml::callee(&application::partial_free_world_data);
        const auto restore_world_data       = dsml::callee(&application::restore_world_data);
        
        // не понимаю пока что правда как отправить другие экземпляры эвентов
        // как сделать загрузку? у меня там разделено на несколько шагов создание контейнера, 
        // тут можно сделать примерно тоже самое через вызов несколько эвентов которые приводят
        // к одному и тому же стейту каждый кадр, но с другой стороны нужно ли? не думаю
        // 
        return dsml::make_transition_table(
            dsml::initial_state + init = loading_menu, // инитиал стейт, там мы можем показать лого
            // состояния должны создать работу для другого потока, как дать понять что работа закончена?
            // видимо отправить ивент loading_end
            // тут наверное нужно удалить мир (вообще все удалить)
            loading_menu + dsml::on_entry / (display("loading_menu"), set_state(utils::quest_state::main_menu_loading), loading_menu_action),
            loading_menu + loading_end = main_menu, 

            loading_gen + dsml::on_entry / (display("loading_gen"), set_state(utils::quest_state::world_map_generator_loading), loading_generator_action),
            loading_gen + loading_end = generator, 

            loading_save_world + dsml::on_entry / (display("loading_save_world"), set_state(utils::quest_state::world_map_loading), loading_sav_world_action),
            loading_save_world + loading_end = world, // берем путь до сохранения
            
            loading_gen_world + dsml::on_entry / (display("loading_gen_world"), set_state(utils::quest_state::world_map_loading), loading_gen_world_action),
            loading_gen_world + loading_end = world, // загружаем то что сгенерили

            loading_battle + dsml::on_entry / (display("loading_battle"), set_state(utils::quest_state::battle_map_loading), loading_battle_action),
            loading_battle + loading_end = battle,

            loading_encounter + dsml::on_entry / (display("loading_encounter"), set_state(utils::quest_state::encounter_loading), loading_encounter_action),
            loading_encounter + loading_end = encounter,
            
            // у нас несколько раундов генерации
            generating_world + dsml::on_entry / (display("generating_world"), set_state(utils::quest_state::world_map_generating), generating_world_action),
            generating_world + generator_end = loading_gen_world,
            generating_world + generator_step = generator,

            main_menu + dsml::on_entry / (display("main_menu"), set_state(utils::quest_state::main_menu)), 
            main_menu + choosed_gen = loading_gen, 
            main_menu + choosed_saved_game = loading_save_world, // просто загрузка мира и загрузка сохранения - это по идее разные вещи

            world + dsml::on_entry / (display("world"), set_state(utils::quest_state::world_map)),
            world + dsml::on_exit / (display("exit world")),
            world + start_battle / partial_free_world_data = loading_battle, // тут можно поместить частичную очистку данных мира + при конце битвы их восстановление
            world + start_encounter / partial_free_world_data = loading_encounter,
            world + return_to_menu / free_world_data = loading_menu,

            generator + dsml::on_entry / (display("generator"), set_state(utils::quest_state::world_map_generator)),
            generator + dsml::on_exit / (display("exit generator")),
            //generator + generator_end = world,
            generator + start_gen = generating_world,
            generator + return_to_menu / free_world_data = loading_menu,

            battle + dsml::on_entry / (display("battle"), set_state(utils::quest_state::battle_map)),
            battle + dsml::on_exit / (display("exit battle")), // выход - очистка битвы
            battle + battle_end = world, // еще стейт загрузки мира после битвы добавится
            battle + return_to_menu / free_world_data = loading_menu,

            encounter + dsml::on_entry / (display("encounter"), set_state(utils::quest_state::encounter)),
            encounter + dsml::on_exit / (display("exit encounter")), // выход - очистка столкновения
            encounter + encounter_end = world,
            encounter + return_to_menu / free_world_data = loading_menu
        );
      }
    };
    
    const size_t threads_count = std::max(std::thread::hardware_concurrency()-1, uint32_t(1));
    application::application(const int argc, const char* argv[]) : pool(threads_count), m_sm{*this} {
      UNUSED_VARIABLE(argc);
      UNUSED_VARIABLE(argv);
      
      set_application_path();
      
      global::get(this);
      global::get(&tasks);
      global::get(&base_systems);
      global::get(&map_systems);
      global::get(&battle_systems);
      global::get(&encounter_systems);
      global::get(&settings);
      
      base_systems.create_utility_systems();
      settings.load_settings();
      
      block_input_t bi;
      
      uint32_t count;
      const char** ext = glfwGetRequiredInstanceExtensions(&count);
      base_systems.create_render_system(ext, count);
      
      base_systems.create_render_stages();
      base_systems.create_interface();
      settings.graphics.find_video_mode();
      base_systems.load_interface_config(global::root_directory() + "scripts/interface_config.lua");
      
      setup_callbacks(base_systems.graphics_container->window);
      
      // дальше идет подготовка состояний приложения, их мы вынесем в стейт машину
      m_sm.process_event(init);
      base_systems.graphics_container->window->show();
      m_sm.process_event(loading_end);
    }
    
    application::~application() {
      base_systems.game_ctx->quit_game();
  
      pool.compute();
      pool.wait();
    }
    
    bool application::continue_computation() const {
      return !base_systems.graphics_container->window->close() && !base_systems.game_ctx->quit_state();
    }
    
    void application::update() {
      frame_time.start();
      const size_t time = frame_time.get();
      input::update_time(time);
      poll_events();
      if (!continue_computation()) return;
      
      {
        //utils::time_log log("Interface updation");
        base_systems.interface_container->update(time);
      }
      
      // скорее мы должны обновить состояние игры (то есть перейти от хода игрока к ходу противников)
      // ии будет обновляться наверное в отдельном потоке
      update_ai(time); // обновляем ии
      tasks.update(time);
      
      if (base_systems.game_ctx->reload_interface) {
        base_systems.reload_interface();
        // это не всегда так: интерфейс у меня может поменяться и быть например из модификации какой нибудь
        // тут мне просто нужно путь из него получить, то есть: стандарный путь + текущий (который мы запомним как путь по умолчанию для следующих загрузок)
        base_systems.load_interface_config(global::root_directory() + "scripts/interface_config.lua");
        base_systems.game_ctx->reload_interface = false;
      }
      
      start_render();
      update_world(time); // некоторые вещи мы можем посчитать одновременно с отрисовкой
      wait_for_render();
      
      auto window = base_systems.graphics_container->window;
      const size_t sync_time = window->flags.vsync() ? window->refresh_rate_mcs() : 0;
      sync(frame_time, sync_time);
    }
    
    void application::notify_gen_advance() {
      m_sm.process_event(generator_step);
    }
    
    void application::update_ai(const size_t &time) {
      // если сейчас ход игрока, то не делаем ничего, иначе что?
      // ии обновляется в другом потоке? да ии бы обновить отдельно
      // то есть в основном потоке только интерфейс (возможно даже прогрессия)
      
    }
    
    void application::start_render() {
      auto cont = base_systems.graphics_container;
      // копирование где? должно быть где то повыше, поставил повыше в stages.cpp
      cont->begin();
      cont->next_frame();
      cont->draw();
      cont->present();
    }
    
    void application::loading_menu_action() {
      set_up_input_keys(player::in_menu);
      auto base = global::get<systems::core_t>();
      base->internal.reset(nullptr);
      global::get<utils::deffered_tasks>()->add([this] (const size_t &) -> bool {
        m_sm.process_event(loading_end);
        return true; // не забыть для удаления!
      });
    }
    
    void application::loading_generator_action() {
      auto prog = base_systems.loading_progress;
      
      assert(systems::loading_generator::func_count == 1);
      prog->set_max_value(2 + systems::loading_generator::func_count);
      prog->set_hint1(std::string_view("Creating demiurge"));
      
      systems::advance_progress(prog, "creating container");
      
      global::get<utils::deffered_tasks>()->add([this] (const size_t &) -> bool {
        auto prog = base_systems.loading_progress;
        map_systems.create_map_container();
        map_systems.setup_map_generator();
        systems::setup_map_generator(&map_systems);
        systems::advance_progress(prog, "finalizing");
        auto ctx = systems::create_loading_context();
        systems::loading_generator::advance(ctx, prog);
        systems::destroy_loading_context(ctx);
        
        global::get<utils::deffered_tasks>()->add([this] (const size_t &) -> bool {
          auto prog = base_systems.loading_progress;
          prog->reset();
          set_up_input_keys(player::on_global_map);
          m_sm.process_event(loading_end);
          
          return true;
        });
        
        return true;
      });
    }
    
    void application::loading_gen_world_action() {
      // сюда мы перешли уже после того как генератор разрушился... или нет?
      // вообще сейчас первые две функции требуют map_creator, когда он чистится? я не помню уже
      auto prog = base_systems.loading_progress;
      if (!base_systems.internal) base_systems.internal.reset(new core::internal_lua_state);
      
      prog->set_max_value(systems::loading_generated_map::func_count + 1);
      prog->set_hint1(std::string_view("loading_generated_map"));
      
      set_up_input_keys(player::on_global_map);
      
      map_systems.create_map_container(); // повторно создаваться не будет
      map_systems.start_rendering();
      
      // тут как раз начинается загрузка
      pool.submitbase([this] () {
        auto prog = base_systems.loading_progress;
        //systems::from_create_map_to_map(prog);
        auto ctx = systems::create_loading_context();
        systems::loading_generated_map::advance(ctx, prog);
        systems::destroy_loading_context(ctx);
        
        prog->set_hint2(std::string_view("end"));
        prog->advance();
        
        global::get<utils::deffered_tasks>()->add([this] (const size_t &) -> bool {
          auto prog = base_systems.loading_progress;
          // чистим ресурсы, например создателя карты? или может быть его раньше можно почистить?
          // + мы можем почистить при эвенте завершения
          map_systems.destroy_map_generator();
          prog->reset();
          m_sm.process_event(loading_end);
          return true;
        });
      });
    }
    
    void application::loading_sav_world_action() {
      auto prog = base_systems.loading_progress;
      if (!base_systems.internal) base_systems.internal.reset(new core::internal_lua_state);
      
      prog->set_max_value(systems::loading_saved_world::func_count + 1);
      prog->set_hint1(std::string_view("loading_saved_world"));
      
      set_up_input_keys(player::on_global_map);
      
      map_systems.create_map_container(); // повторно создаваться не будет
      map_systems.start_rendering();
      
      // тут как раз начинается загрузка
      pool.submitbase([this] () {
        auto prog = base_systems.loading_progress;
        //systems::from_create_map_to_map(prog);
        auto ctx = systems::create_loading_context();
        systems::loading_saved_world::advance(ctx, prog);
        systems::destroy_loading_context(ctx);
        
        prog->set_hint2(std::string_view("end"));
        prog->advance();
        
        global::get<utils::deffered_tasks>()->add([this] (const size_t &) -> bool {
          auto prog = base_systems.loading_progress;
          // чистим ресурсы? или мы можем почистить при эвенте завершения
          prog->reset();
          m_sm.process_event(loading_end);
          return true;
        });
      });
    }
    
    void application::loading_battle_action() {
      
    }
    
    void application::loading_encounter_action() {
      
    }
    
    void application::generating_world_action() {
      auto inter = base_systems.interface_container.get();
      auto t = inter->get_generator_table();
      const auto &str = inter->serialize_table(t);
      const auto t1 = map_systems.map_creator->deserialize_table(str);
      map_systems.map_creator->set_userdata_table(t1);
      map_systems.map_creator->generate();
      
      global::get<utils::deffered_tasks>()->add([this] (const size_t &) -> bool {
        if (map_systems.map_creator == nullptr) { return true; }
        
        if (map_systems.map_creator->finished()) {
          m_sm.process_event(generator_end);
          return true;
        }
        
        auto inter = base_systems.interface_container.get();
        auto prog = base_systems.loading_progress;
        if (prog->finished()) {
          const auto t = map_systems.map_creator->get_post_generation_table();
          if (t.valid() && t.get_type() == sol::type::table) {
            const auto &str = map_systems.map_creator->serialize_table(t.as<sol::table>());
            const auto t1 = inter->deserialize_table(str);
            inter->update_post_generating_table(t1);
          }
          
          prog->reset();
          m_sm.process_event(generator_step);
          return true;
        }
        
        return false;
      });
    }
    
    void application::free_world_data() {
      // чистим максимально все что можем для того чтобы выйти в меню
      // но причем все равно не все картиночки (иконки, то могут пригодиться в интерфейсе)
      // мы должны будем пропустить уже загруженные иконки + наверное перераспределить то что осталось
      // по другим массивам картинок, хотя наверное не то чтобы пропустить если мы пытаемся что то загрузить заново
      // то над почистить их? или нет?
      // лучше вот как, все же использовать иерархию: 
      // картинки которые существуют во всей программе (иконки интерфейса, причем нам нужно интерфейс перезагружать а значит и картинки тоже поди придется перегрузить или нет?)
      // картинки которые существуют в мире (то есть нужны и в битве и на карте)
      // картинки только для мира, только для битвы и только для энкаунтера
      // их желательно расположить по разным массивам текстур
      
      battle_systems.release_container();
      //encounter_systems.release_container();
      map_systems.destroy_map_generator();
      map_systems.release_container();
    }
    
    void application::partial_free_world_data() {
      // чистим какую то часть для битвы или столкновения
      // а это все ресурсы на гпу, кроме некоторых картинок (кроме иконок, геральдик, лиц)
    }
    
    void application::restore_world_data() {
      // обратно грузим рендер + все данные карты на гпу + картинки
    }
    
    static bool check_heraldy_hovering(
      const glm::vec4 &tile_center, 
      const float &tile_height, 
      const float &zoom, 
      const glm::mat4 &matrix, 
      const float &scale, 
      const glm::vec4 &cam_pos, 
      const glm::vec4 &cursor_dir, 
      float &dist
    ) {
      const glm::vec4 default_size = glm::vec4(0.5f, 0.5f, 0.5f, 0.0f);
      
      const glm::vec4 center = render::get_heraldy_pos(tile_center, tile_height, zoom, matrix);
      const glm::vec4 size = default_size * scale;
      
      const bool intersect = render::test_ray_aabb(cam_pos, cursor_dir, center, size, dist);
      return intersect;
    }
    
    void application::update_world(const size_t &time) {
      // эту функцию было бы неплохо получить из состояния стейтмашины
      // она будет различаться для разных состояний
      // 
      
      if (!map_systems.is_init()) return;
      
      // эта функция может попасть на создание объектов в другом потоке
      // при загрузке она нам не нужна
      
      if (base_systems.game_ctx->is_loading()) return;
      
  //     std::cout << "copy wait start" << "\n";
      const auto tp = std::chrono::steady_clock::now();
      auto copy = global::get<render::static_copy_array<16>>();
      while (!copy->end()) { 
        std::this_thread::sleep_for(std::chrono::microseconds(1)); 
        const auto end = std::chrono::steady_clock::now() - tp;
        const size_t mcs = std::chrono::duration_cast<std::chrono::microseconds>(end).count();
        if (mcs > ONE_SECOND) throw std::runtime_error("Long copy");
      }
  //     std::cout << "copy wait end" << "\n";
      
      // рисуются вещи через одно место, исправить
      // исправил, нужно понять че делать с геральдикой
      // я думаю что статическая геральдика это самый нормальный способ
      
      if (map_systems.is_init()) {
        ASSERT(map_systems.core_context != nullptr);
        map_systems.core_context->update_armies(time, [] (const size_t &time, core::army* army) {
          army->update(time);
        });
      }
      
      // обходим все города и армии
      auto ctx = map_systems.core_context;
      auto buffers = map_systems.world_buffers;
      auto map = map_systems.map;
      auto camera = global::get<render::buffers>();
      auto struct_render = global::get<render::tile_structure_render>();
      auto armies_render = global::get<render::armies_render>();
      auto heraldies_render = global::get<render::heraldies_render>();
      float min_dist = 100000.0f;
      uint32_t min_index = UINT32_MAX;
      const uint32_t cities_count = ctx->get_entity_count<core::city>();
      for (size_t i = 0; i < cities_count; ++i) {
        const auto city = ctx->get_entity<core::city>(i);
        const uint32_t tile = city->tile_index;
        const bool rendered = buffers->get_map_renderable(tile);
        //const bool explored = buffers->get_map_exploration(tile);
        const bool explored = true;
        if (!explored || !rendered) continue;
        
        const auto tile_ptr = map->get_tile_ptr(tile);
        const glm::vec4 center = map->get_point(tile_ptr->center);
        const glm::vec4 camera_dir = camera->get_dir();
        const float dot = glm::dot(glm::normalize(glm::vec4(glm::vec3(center), 0.0f)), camera_dir);
        if (dot >= 0.0f) continue; // чет не сильно помогло, причем даже в очевидном случае
        
        const render::tile_structure_render::structure_data data{
          tile,
          city->type->city_image_face,
          1.5f
        };
        struct_render->add(data);
        
        // далее мы проверяем геральдику
        // если этот город столица реалма, то над ним нужно нарисовать геральдику с рамкой
        // помимо этого нужно посчитать пересечение геральдики и луча от мышки
        
        const auto title = city->title;
        // нам нужно взять максимальный титул у владельца, пока так нарисуем
        const auto owner = title->owner;
        // как взять столицу? нужно добавить указатель в реалм 
        if (city != owner->capital) continue;
        
        // по тайлу, зуму и матрице камеры мы по идее можем расчитать позицию геральдики
        // по позиции найдем пересечение с лучем
        
        const render::heraldies_render::heraldy_data hdata{
          tile,
          title->heraldy_container.data(),
          title->heraldy_layers_count,
          GPU_UINT_MAX, // что тут? должен быть глобальный конфиг для этих вещей
          render::image_t{GPU_UINT_MAX},
          1.5f,
          1.5f
        };
        heraldies_render->add(hdata);
        
        // геральдика рисуется плохо, геральдика видна из-за гор и округлости земли
        // нужно выбрать либо статическое расположение над городом, как в цк2,
        // либо как нибудь расположить билборд геральдики поверх города (или рядом)
        // самый простой способ - это конечно статика, билборд наверное не получится из-за шара земли
        
        const glm::mat4 view = camera->get_view();
        const glm::vec4 pos = camera->get_pos();
        const glm::vec4 dir = camera->get_cursor_dir();
        const float zoom = camera->get_zoom();
        float dist = 1000000.0f;
        const bool hover = check_heraldy_hovering(center, tile_ptr->height, zoom, view, 1.0f, pos, dir, dist);
        
        if (hover && dist < min_dist) {
          min_dist = dist;
          min_index = i;
        }
      }
      
      // как обойти армии? хороший вопрос, проблема в том что в массиве армий будут дырки
      // дырки? они возникают потому что у нас хранятся кое какие данные в буфере армий
      // зачем мне теперь этот буфер? теперь данные армии передаются в рендер каждый кадр
      // другое дело что дырки все равно есть потому что составляю токен для армии, как распределены дырки?
      // скорее всего неравномерно, сильно это ударит по производительности? возможно нет
      const size_t armies_count = ctx->get_army_container_size();
      for (size_t i = 0; i < armies_count; ++i) {
        auto army = ctx->get_army_raw(i);
        if (army == nullptr) continue;
        
        const uint32_t tile_index = army->tile_index;
        const bool rendered = buffers->get_map_renderable(tile_index);
  //       const bool explored = buffers->get_map_exploration(tile_index);
  //       const bool visible = buffers->get_map_visibility(tile_index);
        const bool explored = true;
        const bool visible = true;
        
        if (!explored || !rendered || !visible) continue;
        
        // нужно ли это здесь?
  //       const auto tile_ptr = map->get_tile_ptr(tile_index);
  //       const glm::vec4 center = map->get_point(tile_ptr->center);
  //       const glm::vec4 camera_dir = camera->get_dir();
  //       const float dot = glm::dot(glm::vec4(glm::vec3(center), 0.0f), camera_dir);
  //       if (dot >= 0.0f) continue;
        
        const render::armies_render::army_data data{
          army->get_pos(),
          render::image_t{GPU_UINT_MAX},
          1.5f
        };
        armies_render->add(data);
        
        // геральдика 
      }
    }
    
    void application::wait_for_render() {
      auto cont = base_systems.graphics_container;
      cont->wait();
      cont->clear();
    }
    
    // надо бы это переделать, как? убрать отсюда pre_sync()
    // там у нас отрисовка и одновременно запуск работы на фоне
    static void sync(utils::frame_time &frame_time, const size_t &time) {
      //pre_sync();
      
      frame_time.end();

      size_t mcs = 0;
      while (mcs < time) {
        std::this_thread::sleep_for(std::chrono::microseconds(1));
        auto end = std::chrono::steady_clock::now() - frame_time.start_point;
        mcs = std::chrono::duration_cast<std::chrono::microseconds>(end).count();
      }
    }
    
    static void setup_callbacks(render::window* window) {
      glfwSetKeyCallback(window->handle, keyCallback);
      glfwSetCharCallback(window->handle, charCallback);
      glfwSetMouseButtonCallback(window->handle, mouseButtonCallback);
      glfwSetScrollCallback(window->handle, scrollCallback);
      glfwSetWindowFocusCallback(window->handle, focusCallback);
      glfwSetWindowIconifyCallback(window->handle, iconifyCallback);
      glfwSetWindowSizeCallback(window->handle, window_resize_callback);
    }
    
    static void callback(int error, const char* description) {
      std::cout << "Error code: " << error << std::endl;
      std::cout << "Error: " << description << std::endl;
    }

    static void scrollCallback(GLFWwindow*, double xoffset, double yoffset) {
      if (!global::get<render::window>()->flags.focused()) return;

      (void)xoffset;
      auto ptr = input::get_input_data();
      ptr->mouse_wheel += float(yoffset);
    }

    static void charCallback(GLFWwindow*, unsigned int c) {
      if (!global::get<render::window>()->flags.focused()) return;
      
      auto ptr = input::get_input_data();
      ptr->text[ptr->current_text] = c;
      ++ptr->current_text;
    }

    static void mouseButtonCallback(GLFWwindow*, int button, int action, int mods) {
      if (!global::get<render::window>()->flags.focused()) return;

      (void)mods;
      auto data = input::get_input_data();
      const auto old = data->container[button].event;
      data->container[button].event = static_cast<input::type>(action);
      if (old != static_cast<input::type>(action)) data->container[button].event_time = 0;
    }

    static void keyCallback(GLFWwindow*, int key, int scancode, int action, int mods) {
      (void)mods;
      
      if (!global::get<render::window>()->flags.focused()) return;
      if (key == GLFW_KEY_UNKNOWN) return; // нужно сделать ориентацию по скан кодам еще, в этом случае я так понимаю мы должны использовать мапу

      auto data = input::get_input_data();
      const auto old = data->container[key].event;
      data->container[key].event = static_cast<input::type>(action);
      if (old != static_cast<input::type>(action)) data->container[key].event_time = 0;

      (void)scancode;
    }

    static void window_resize_callback(GLFWwindow*, int w, int h) {
      auto system = global::get<systems::core_t>();
      system->graphics_container->window->resize();
      system->interface_container->free_fonts();
      system->context->remake_font_atlas(w, h);
      system->interface_container->get_fonts();
      
      auto settings = global::get<utils::settings>();
      settings->graphics.width = w;
      settings->graphics.height = h;

      system->graphics_container->render->recreate(w, h);
      system->image_controller->update_set();
      
      // TODO: переделать (возможно нужно в контейнере рендера это обновить)
      auto rp = reinterpret_cast<render::pass_framebuffer_container*>(system->main_pass);
      rp->recreate(w, h);
      
      auto map = global::get<systems::map_t>();
      if (map != nullptr && map->optimizators_container != nullptr) {
        map->optimizators_container->recreate(w, h);
        map->render_container->recreate(w, h);
      }
    }

    static void iconifyCallback(GLFWwindow*, int iconified) {
      global::get<render::window>()->flags.set_iconified(iconified);
    }

    static void focusCallback(GLFWwindow*, int focused) {
      global::get<render::window>()->flags.set_focused(focused);
    }

    static const char* getClipboard(void* user_data) {
      return glfwGetClipboardString(((render::window*)user_data)->handle);
    }

    static void setClipboard(void* user_data, const char* text) {
      glfwSetClipboardString(((render::window*)user_data)->handle, text);
    }
  }
}
