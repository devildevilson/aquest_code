#include "systems.h"

#include "globals.h"
#include "sol.h"
#include "serializator_helper.h"
#include "input.h"
#include "thread_pool.h"
#include "table_container.h"
#include "string_container.h"
#include "progress_container.h"
//#include "interface_container.h"
#include "interface_container2.h"
#include "main_menu.h"
#include "lua_initialization.h"
#include "settings.h"
//#include "astar_search.h"
#include "astar_search_mt.h"
#include "battle_lua_states.h"
//#include "string_bank.h"
#include "localization_container.h"
#include "game_context.h"

#include "render/window.h"
#include "render/render.h"
#include "render/stages.h"
#include "render/targets.h"
#include "render/container.h"
#include "render/shared_structures.h"
#include "render/persistent_resources.h"
#include "render/slots.h"
#include "render/image_container.h"
#include "render/image_controller.h"
#include "render/battle_render_stages.h"
#include "render/map_data.h"
#include "render/render_mode_container.h"

#include "ai/sub_system.h"
#include "ai/ai_system.h"
#include "ai/build_subsystem.h"
#include "ai/path_container.h"

#include "bin/map.h"
#include "bin/map_generators2.h"
#include "bin/generator_system2.h"
#include "bin/generator_container.h"
#include "bin/generator_context2.h"
#include "bin/interface_context.h"
#include "bin/map_creator.h"
// #include "bin/interface2.h"
#include "bin/seasons.h"
#include "bin/game_time.h"
#include "bin/battle_map.h"
#include "bin/camera.h"
#include "bin/battle_unit_states_container.h"
#include "bin/battle_context.h"
#include "bin/objects_selection.h"

#include "core/context.h"
#include "core/internal_lua_state.h"

#include <filesystem>

#define OPTIMIZATOR_STAGE_SLOT 2
#define RENDER_STAGE_SLOT 7

static const std::vector<const char*> instanceLayers = {
  "VK_LAYER_LUNARG_standard_validation",
  "VK_LAYER_LUNARG_api_dump",
  "VK_LAYER_LUNARG_assistant_layer"
};

namespace devils_engine {
  namespace systems {
#define RELEASE_CONTAINER_DATA(var) if (var != nullptr) container.destroy(var); var = nullptr;
    
    core_t::core_t() :
      container(
        //sizeof(input::data) +
        sizeof(input::keys) * player::states_count +
        sizeof(devils_engine::render::container) +
        sizeof(render::slots) +
        sizeof(render::image_container) +
        sizeof(render::image_controller) +
        sizeof(interface::context) +
//         sizeof(utils::interface) +
        sizeof(utils::main_menu) +
        sizeof(game::context) + 
        sizeof(utils::interface_container) +
//         sizeof(utils::data_string_container) +
        sizeof(utils::sequential_string_container) +
        sizeof(utils::calendar) +
        sizeof(utils::progress_container) +
        sizeof(utils::objects_selection)*2 +
        sizeof(struct path_managment) +
        sizeof(localization::container)
        //sizeof(utils::localization)
        , 8
      ),
      //input_data(nullptr),
      keys_mapping{nullptr},
      graphics_container(nullptr),
      render_slots(nullptr),
      image_container(nullptr),
      image_controller(nullptr),
      context(nullptr),
      interface(nullptr),
      menu(nullptr),
      game_ctx(nullptr),
//       string_container(nullptr),
      sequential_string_container(nullptr),
      game_calendar(nullptr),
      loading_progress(nullptr),
      selection{nullptr, nullptr},
      path_managment(nullptr),
      interface_container(nullptr),
      loc(nullptr)
    {
      ASSERT(keys_mapping[1] == nullptr);
    }
    
    core_t::~core_t() {
      RELEASE_CONTAINER_DATA(context)
      RELEASE_CONTAINER_DATA(render_slots)
      RELEASE_CONTAINER_DATA(image_controller)
      RELEASE_CONTAINER_DATA(image_container)
      //RELEASE_CONTAINER_DATA(input_data)
      RELEASE_CONTAINER_DATA(graphics_container)
//       RELEASE_CONTAINER_DATA(interface)
//       RELEASE_CONTAINER_DATA(menu)
      RELEASE_CONTAINER_DATA(game_ctx)
//       RELEASE_CONTAINER_DATA(string_container)
      RELEASE_CONTAINER_DATA(sequential_string_container)
      RELEASE_CONTAINER_DATA(game_calendar)
      RELEASE_CONTAINER_DATA(loading_progress)
      RELEASE_CONTAINER_DATA(selection.primary)
      RELEASE_CONTAINER_DATA(selection.secondary)
      RELEASE_CONTAINER_DATA(path_managment)
      
      for (size_t i = 0; i < player::states_count; ++i) {
        RELEASE_CONTAINER_DATA(keys_mapping[i])
      }
      
      //RELEASE_CONTAINER_DATA(loc)
      //RELEASE_CONTAINER_DATA(interface_container)
    }
    
    void core_t::create_utility_systems() {
      sequential_string_container = container.create<utils::sequential_string_container>();
      utils::id::set_container(sequential_string_container);
      game_calendar = container.create<utils::calendar>();
      global::get(game_calendar);
      // в идеале, сейчас сделаем иначе
      std::random_device dev;
      const uint32_t v1 = dev();
      const uint32_t v2 = dev();
      const size_t seed = (size_t(v2) << 32) | size_t(v1);
      global g;
      g.initialize_state(seed);
      //input_data = container.create<input::data>();
      //global::get(input_data);
      for (size_t i = 0; i < player::states_count; ++i) {
        keys_mapping[i] = container.create<input::keys>();
      }
//       global g;
//       g.initialize_state(1);

      input::set_menu_key_map(keys_mapping[player::in_menu]);
      
      loading_progress = container.create<utils::progress_container>();
      
      //objects_selector = container.create<utils::objects_selector>();
      path_managment = container.create<struct path_managment>(std::thread::hardware_concurrency());
      //loc = container.create<utils::localization>();
      
      //global::get(objects_selector);
      global::get(path_managment);
      
      game_ctx = container.create<game::context>();
      selection.primary = container.create<utils::objects_selection>();
      selection.secondary = container.create<utils::objects_selection>();
    }
    
    void core_t::create_render_system(const char** ext, const uint32_t &count) {
      using namespace devils_engine::render;
      const size_t stage_container_size =
        sizeof(render::buffers) +
        //sizeof(render::images) +
        //sizeof(render::particles) +
        //sizeof(render::deffered) +

        sizeof(render::window_next_frame) +
        sizeof(render::task_begin) +
        
        sizeof(render::tile_optimizer) +
//         sizeof(render::tile_borders_optimizer) +
//         sizeof(render::tile_walls_optimizer) +
        sizeof(render::barriers) +

        sizeof(render::render_pass_begin) +
        sizeof(render::tile_render) +
        sizeof(render::tile_border_render) +
//         sizeof(render::tile_connections_render) +
        sizeof(render::interface_stage) +
        sizeof(render::render_pass_end) +

        sizeof(render::task_end) +
        sizeof(render::task_start) +
        sizeof(render::window_present);

//       uint32_t count;
//       const char** ext = glfwGetRequiredInstanceExtensions(&count);
      if (count == 0) {
        //global::console()->print("Found no extensions\n");
        throw std::runtime_error("Extensions not founded!");
      }

      std::vector<const char*> extensions;
      for (uint32_t i = 0; i < count; ++i) {
        extensions.push_back(ext[i]);
      }

      const render::container::application_info info{
        TECHNICAL_NAME,
        //APP_VERSION,
        MAKE_VERSION(0, 0, 1),
        ENGINE_NAME,
        ENGINE_VERSION,
        VK_API_VERSION_1_0
      };
      
      auto settings = global::get<utils::settings>();
      
      const render::container::window_info inf{
        settings->graphics.width,
        settings->graphics.height,
        0,
        settings->graphics.fullscreen
      };
      
      // я так понимаю что лучше рендер контекст хранить в одной структуре
      // иначе чет капец душно
      
      graphics_container = container.create<render::container>();
      auto window = graphics_container->create_window(inf);
      graphics_container->create_instance(extensions, &info);
      graphics_container->create_device();
      graphics_container->create_vlk_window();
      graphics_container->create_swapchain();
      graphics_container->create_system(stage_container_size); // auto render = 
      graphics_container->create_tasks();
      //container.decals_system = container.container.create<systems::decals>();
      
      render_slots = container.create<render::slots>();

      global::get(window);
//       global::get(render);
      global::get(graphics_container);
      //global::get(systems.decals_system);
      
      // АККУРАТНО С УКАЗАТЕЛЯМИ! НУЖНО ПЕРЕДЕЛАТЬ
      auto device = &graphics_container->vulkan->device;
      auto physical_device = &graphics_container->vulkan->physical_device;
      auto pool = graphics_container->vulkan->transfer_command_pool;
      auto queue = graphics_container->vulkan->graphics;
      auto fence = graphics_container->vulkan->transfer_fence;
      image_container = container.create<render::image_container>(render::image_container::create_info{device, physical_device, &pool, &queue, &fence});
      image_controller = container.create<render::image_controller>(device, image_container);
      
      context = container.create<interface::context>(graphics_container, window, image_container);
      global::get(context);
      
      image_controller->update_set();
      
//       render::create_persistent_resources(graphics_container->device);
    }
    
    void core_t::create_render_stages() {
//       const size_t world_map_render_size = 
//         sizeof(render::tile_optimizer) + 
//         sizeof(render::tile_borders_optimizer) + 
//         sizeof(render::tile_walls_optimizer) + 
//         sizeof(render::barriers) +
//         sizeof(render::render_pass_begin) +
//         sizeof(render::tile_render) +
//         sizeof(render::tile_border_render) +
//         sizeof(render::tile_connections_render);
      
      auto system = graphics_container->render;
      auto device = graphics_container->vulkan->device;
      auto window = graphics_container->window;
      auto buffers = system->add_target<render::buffers>(graphics_container);
      global::get(buffers);

      auto next_frame = system->add_stage<render::window_next_frame>();
      auto begin   = system->add_stage<render::task_begin>();
                    
//       auto world_r = system->add_stage<render::world_map_render>(render::world_map_render::create_info{world_map_render_size});
                     // тут мы можем добавить особые стартеры, которые будем отключать когда у нас полный рендер пайплайн
                     // хотя лучше наверное сделать несколько структур которые можно проверять на валидность и тем самым отключать часть рендера
                     // так пайплайн не будет расбросан по всей игре не понятно где + можно наверное переиспользовать буферы
//       auto opt     = system->add_stage<render::tile_optimizer>(render::tile_optimizer::create_info{device});
//       auto opt2    = system->add_stage<render::tile_borders_optimizer>(render::tile_borders_optimizer::create_info{device});
//       auto opt3    = system->add_stage<render::tile_walls_optimizer>(render::tile_walls_optimizer::create_info{device});
      auto barriers= system->add_stage<render::barriers>();
                  
      auto rp_begin= system->add_stage<render::render_pass_begin>(0); // TODO: должно быть отдельно!!!
//       auto tiles   = system->add_stage<render::tile_render>(render::tile_render::create_info{device, opt});
//       auto borders = system->add_stage<render::tile_border_render>(render::tile_border_render::create_info{device, opt2});
//       auto walls   = system->add_stage<render::tile_connections_render>(render::tile_connections_render::create_info{device, opt3});
      auto intr    = system->add_stage<render::interface_stage>(render::interface_stage::create_info{graphics_container, *image_controller->get_descriptor_set_layout()});
      auto rp_end  = system->add_stage<render::render_pass_end>();

      auto task_end=  system->add_stage<render::task_end>();
      auto start   =  system->add_stage<render::task_start>(device);
      auto present =  system->add_stage<render::window_present>();
                    
//       auto opt     = world_r->add_stage<render::tile_optimizer>(render::tile_optimizer::create_info{device});
//       auto opt2    = world_r->add_stage<render::tile_borders_optimizer>(render::tile_borders_optimizer::create_info{device});
//       auto opt3    = world_r->add_stage<render::tile_walls_optimizer>(render::tile_walls_optimizer::create_info{device});
//                      world_r->add_stage<render::barriers>();
//                   
//                      world_r->add_stage<render::render_pass_begin>(); // TODO: должно быть отдельно!!!
//       auto tiles   = world_r->add_stage<render::tile_render>(render::tile_render::create_info{device, opt});
//       auto borders = world_r->add_stage<render::tile_border_render>(render::tile_border_render::create_info{device, opt2});
//       auto walls   = world_r->add_stage<render::tile_connections_render>(render::tile_connections_render::create_info{device, opt3});

      global::get(start);
//       global::get(tiles);
//       global::get(walls);
//       global::get(buffers);
//       global::get(opt2);
//       global::get(opt3);
//       (void)borders;
      
      render_slots->set_stage(0, next_frame);
      render_slots->set_stage(1, begin);
      // это слоты для карты, битвы и столкновения, могут ли они пересекаться? неуверен, но оставлю так
      render_slots->set_stage(5, barriers);
      render_slots->set_stage(6, rp_begin);
      // это слоты для карты, битвы и столкновения, могут ли они пересекаться? неуверен, но оставлю так
      render_slots->set_stage(10, intr);
      render_slots->set_stage(11, rp_end);
      render_slots->set_stage(12, task_end);
      render_slots->set_stage(13, start);
      render_slots->set_stage(14, present);
      
      const float dist = 550.0f;
      const glm::vec3 default_camera_pos = glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f)) * dist;
      const auto [w, h] = window->framebuffer_size();
      const glm::mat4 persp = glm::perspective(glm::radians(75.0f), float(w) / float(h), 0.1f, 256.0f);
      const glm::mat4 view  = glm::lookAt(default_camera_pos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
  //     buffers->update_matrix(persp * view);
      buffers->update_projection_matrix(persp);
      buffers->update_view_matrix(view);
      buffers->update_dir(glm::normalize(-default_camera_pos));
      buffers->update_pos(default_camera_pos);
      ASSERT(persp[0][0] == persp[0][0]);
      ASSERT(view[0][0] == view[0][0]);
    }
    
    void core_t::create_interface() {
      //interface_container = container.create<utils::interface_container>(utils::interface_container::create_info{context, game_ctx});
      interface_container = std::make_unique<utils::interface_container>(utils::interface_container::create_info{context, game_ctx});
      
      //loc = container.create<localization::container>(interface_container->lua);
      loc = std::make_unique<localization::container>(interface_container->lua);
      // это мы как то в настройках должны изменять + соответственно менять строчки меню и игры при смене локали
      localization::container::set_current_locale(localization::container::locale("en"));
    }
    
    void core_t::reload_interface() {
      std::string table_container;
      const auto table = interface_container->get_persistent_table();
      if (table.get_type() == sol::type::table) {
        table_container = interface_container->serialize_table(table);
      }
      
      auto inter = interface_container.get();
      const auto loc_data = loc->serialize([inter] (const sol::table &t) {
        return inter->serialize_table(t);
      });
      
      loc.reset(nullptr);
      interface_container.reset(nullptr);
      
      create_interface();
      
      loc->deserialize(loc_data);
      if (!table_container.empty()) {
        const auto t = interface_container->deserialize_table(table_container);
        interface_container->set_persistent_table(t);
      }
      
      auto map_creator = global::get<systems::map_t>()->map_creator;
      if (map_creator != nullptr) {
        map_creator->setup_map_generator_functions(interface_container.get());
      }
    }
    
    std::string mod_path_to_absolute(const std::string &path) {
      std::filesystem::path p(path);
      auto itr = p.begin();
      ASSERT(*itr == "apates_quest");
      std::string final_path = global::root_directory();
      for (++itr; itr != p.end(); ++itr) {
        final_path += "/" + std::string(*itr);
      }
      
      return final_path;
    }
    
    void core_t::load_interface_config(const std::string &config_path) {
      const auto ret = interface_container->lua.safe_script_file(config_path, interface_container->env, sol::load_mode::text);
      if (!ret.valid()) {
        sol::error err = ret;
        std::cout << err.what();
        throw std::runtime_error("There is lua errors");
      }
      
      if (ret.get_type() != sol::type::table) throw std::runtime_error("Bad interface config");
      
      const sol::table config = ret;
      
      if (auto proxy = config["interface_func"]; proxy.valid() && proxy.get_type() == sol::type::string) {
        const std::string path = proxy.get<std::string>();
        
        // функция преобразования пути должна быть глобальной
        // она должна возвращать что? укзатель на мод + путь? 
        // а это значит придется делать контейнер модификаций
        // он нинужен пока что (2021.05.19)
        const auto final_path = mod_path_to_absolute(path);
        interface_container->load_interface_file(final_path);
      }
      
      if (auto proxy = config["string_container"]; proxy.valid() && proxy.get_type() == sol::type::table) {
        const sol::table t = proxy.get<sol::table>();
        for (const auto &pair : t) {
          if (!pair.first.is<std::string>()) continue;
          if (!pair.second.is<std::string>()) continue;
          
          // нужно придумать где будет контейнер
        }
      }
      
      if (auto proxy = config["localization"]; proxy.valid() && proxy.get_type() == sol::type::table) {
        const sol::table t = proxy.get<sol::table>();
        for (const auto &pair : t) {
          if (!pair.second.is<std::string>()) continue;
          
          const std::string path = pair.second.as<std::string>();
          const auto final_path = mod_path_to_absolute(path);
          const auto ret = interface_container->lua.safe_script_file(final_path, interface_container->env, sol::load_mode::text);
          if (!ret.valid()) {
            sol::error err = ret;
            std::cout << err.what();
            throw std::runtime_error("There is lua errors");
          }
          
          if (ret.get_type() != sol::type::table) throw std::runtime_error("Bad localization data");
          
          const sol::table t = ret;
          for (const auto &pair : t) {
            if (!pair.first.is<std::string>()) continue;
            if (!pair.second.is<sol::table>()) continue;
            
            const auto loc_key = pair.first.as<std::string_view>();
            localization::container::locale locale(loc_key);
            const auto t = pair.second.as<sol::table>();
            loc->set_table(locale, t);
          }
        }
      }
      
      if (auto proxy = config["images"]; proxy.valid() && proxy.get_type() == sol::type::table) {
        const sol::table t = proxy.get<sol::table>();
        for (const auto &pair : t) {
          if (!pair.second.is<std::string>()) continue;
          
          // грузим картинки
        }
      }
      
      if (auto proxy = config["sounds"]; proxy.valid() && proxy.get_type() == sol::type::table) {
        const sol::table t = proxy.get<sol::table>();
        for (const auto &pair : t) {
          if (!pair.second.is<std::string>()) continue;
          
          // грузим звуки
        }
      }
    }
    
    map_t::map_t() :
      container(
        {
          sizeof(core::map),
          sizeof(core::context),
          sizeof(core::seasons),
          sizeof(systems::ai),
          sizeof(render::mode_container),
          sizeof(map::creator),
          sizeof(render::stage_container),
          sizeof(render::stage_container),
          sizeof(components::world_map_camera)
        }
      ),
      map(nullptr),
      core_context(nullptr),
      seasons(nullptr),
      ai_systems(nullptr),
      render_modes(nullptr),
      map_creator(nullptr),
      optimizators_container(nullptr),
      render_container(nullptr),
      camera(nullptr)
//       inited(false)
    {
      memset(hash, 0, sizeof(hash[0]) * hash_size);
    }
    
    map_t::~map_t() {
      release_container();
    }
    
    double vectors_angle(const glm::dvec4 &first, const glm::dvec4 &second) {
      return glm::acos(glm::dot(first, second) / (glm::length(first)*glm::length(second)));
    }
    
//     bool map_t::is_init() const { return container.inited(); }
    void map_t::create_map_container() {
      if (is_init()) return;
      
      container.init();
      auto systems = global::get<core_t>();
      map = container.create<core::map, 0>(core::map::create_info{systems->graphics_container});
      core_context = container.create<core::context, 1>();
      seasons = container.create<core::seasons, 2>();
      const size_t container_size = sizeof(devils_engine::ai::build_subsystem);
      ai_systems = container.create<systems::ai, 3>(container_size);
      ai_systems->add<devils_engine::ai::build_subsystem>();
      render_modes = container.create<render::mode_container, 4>();
      
      const float dist = 550.0f;
      const glm::vec3 default_camera_pos = glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f)) * dist;
      camera = container.create<components::world_map_camera, 8>(default_camera_pos);
      
      create_render_stages();
      
      const auto local_map = map;
      const auto local_context = core_context;
      utils::astar_search_mt::set_vertex_cost_f([local_context] (const uint32_t &current_tile_index, const uint32_t &neighbour_tile_index, const utils::user_data &) -> utils::astar_search_mt::float_t {
        const auto current_tile_data = local_context->get_entity<core::tile>(current_tile_index);
        const auto neighbour_tile_data = local_context->get_entity<core::tile>(neighbour_tile_index);
        
        if (neighbour_tile_data->height > 0.5f) return 1000.0;
        
        const uint32_t current_height_layer = render::compute_height_layer(current_tile_data->height);
        const uint32_t neighbour_height_layer = render::compute_height_layer(neighbour_tile_data->height);
        
        // какая высота ограничивает передвижение? у меня кажется максимальная высота 20? мне надо прикинуть что 
        // 20 слоев это весь подьем от моря до самой высокой горы, и примерно с 10 слоя (по идее) начинаются горы (проходимые?)
        // то есть мне нужно ориентироваться в рамках 10 слоев?
        
        if (current_height_layer > neighbour_height_layer || current_height_layer == neighbour_height_layer) return 1.0;
        //if (neighbour_height_layer - current_height_layer > 3) return 1000.0;
        
        return (neighbour_height_layer - current_height_layer) + 1.0;
        //return 1.0;
      });
      
      utils::astar_search_mt::set_goal_cost_f([local_context, local_map] (const uint32_t &current_tile_index, const uint32_t &goal_tile_index) -> utils::astar_search_mt::float_t {
        const auto &current_tile_data = local_context->get_entity<core::tile>(current_tile_index);
        const auto current_tile_point_index = current_tile_data->center;
        const auto current_tile_center = local_map->get_point(current_tile_point_index);
//         const auto current_tile_vec = glm::dvec4(glm::dvec3(current_tile_center), 0.0);
        
        const auto &goal_tile_data = local_context->get_entity<core::tile>(goal_tile_index);
        const auto goal_tile_point_index = goal_tile_data->center;
        const auto goal_tile_center = local_map->get_point(goal_tile_point_index);
//         const auto goal_tile_vec = glm::dvec4(glm::dvec3(goal_tile_center), 0.0);
        
        //const double angle = vectors_angle(current_tile_vec, goal_tile_vec);
        
        //return angle * core::map::world_radius;
        return glm::distance(current_tile_center, goal_tile_center); // по углу похоже неверный результат иногда выдает
      });
      
      utils::astar_search_mt::set_fill_successors_f([] (utils::astar_search_mt* searcher, const uint32_t &tile_index, const utils::user_data &) {
        assert(false);
      });
      
      // нужно ли специально обнулять эти функции? врядли
    }
    
    void map_t::setup_rendering_modes() {
      // функции наверное лучше куда нибудь вытащить
      // во всех этих функциях я буду задавать данные в конкретный тайл,
      // функция может быть запущена в другом потоке отличном от потка интерфейса
      // может ли случиться кердык? может, хотя эта вещь нужна только игроку
      // следовательно не имеет смысла тут городить что то серьезное, но возможно
      // нужно сделать отложенное выполнение этой функции
      ASSERT(is_init());
      auto &render_modes_container = *render_modes;
      render_modes_container[render::modes::biome] = [this] () {
        for (size_t i = 0; i < core::map::hex_count_d(core::map::detail_level); ++i) {
          const uint32_t biome_index = seasons->get_tile_biome(seasons->current_season, i);
          const auto &current_biome = core_context->get_entity<core::biome>(biome_index);
          auto tile = core_context->get_entity<core::tile>(i);
          tile->color = current_biome->data.color;
          tile->texture = current_biome->data.texture;
        }
        
        global::get<render::tile_updater>()->update_all(core_context);
      };
      
      render_modes_container[render::modes::cultures] = [this] () {
        for (size_t i = 0; i < core::map::hex_count_d(core::map::detail_level); ++i) {
          auto tile = core_context->get_entity<core::tile>(i);
          tile->texture = {GPU_UINT_MAX};
          
          if (tile->province == UINT32_MAX) {
            const auto &current_biome = core_context->get_entity<core::biome>(tile->biome_index);
            const bool is_water = current_biome->get_attribute(core::biome::attributes::water);
            tile->color = is_water ? current_biome->data.color : render::make_color(0.3f, 0.3f, 0.3f, 1.0f);
            tile->texture = is_water ? current_biome->data.texture : render::image_t{GPU_UINT_MAX};
            //tile->color = current_biome->data.color;
            //tile->texture = current_biome->data.texture;
            continue;
          }
          
          const auto province = core_context->get_entity<core::province>(tile->province);
          auto culture = province->culture;
          tile->color = culture->color;
        }
        
        global::get<render::tile_updater>()->update_all(core_context);
      };
      
      render_modes_container[render::modes::culture_groups] = [this] () {
        for (size_t i = 0; i < core::map::hex_count_d(core::map::detail_level); ++i) {
          auto tile = core_context->get_entity<core::tile>(i);
          tile->texture = {GPU_UINT_MAX};
          
          if (tile->province == UINT32_MAX) {
            const auto &current_biome = core_context->get_entity<core::biome>(tile->biome_index);
            const bool is_water = current_biome->get_attribute(core::biome::attributes::water);
            tile->color = is_water ? current_biome->data.color : render::make_color(0.3f, 0.3f, 0.3f, 1.0f);
            tile->texture = is_water ? current_biome->data.texture : render::image_t{GPU_UINT_MAX};
            //tile->color = current_biome->data.color;
            //tile->texture = current_biome->data.texture;
            continue;
          }
          
          const auto province = core_context->get_entity<core::province>(tile->province);
          auto culture_group = province->culture->group;
          tile->color = culture_group->color;
        }
        
        global::get<render::tile_updater>()->update_all(core_context);
      };
      
      render_modes_container[render::modes::religions] = [this] () {
        for (size_t i = 0; i < core::map::hex_count_d(core::map::detail_level); ++i) {
          auto tile = core_context->get_entity<core::tile>(i);
          tile->texture = {GPU_UINT_MAX};
          
          if (tile->province == UINT32_MAX) {
            const auto &current_biome = core_context->get_entity<core::biome>(tile->biome_index);
            const bool is_water = current_biome->get_attribute(core::biome::attributes::water);
            tile->color = is_water ? current_biome->data.color : render::make_color(0.3f, 0.3f, 0.3f, 1.0f);
            tile->texture = is_water ? current_biome->data.texture : render::image_t{GPU_UINT_MAX};
            //tile->color = current_biome->data.color;
            //tile->texture = current_biome->data.texture;
            continue;
          }
          
          const auto province = core_context->get_entity<core::province>(tile->province);
          auto religion = province->religion;
          tile->color = religion->color;
        }
        
        global::get<render::tile_updater>()->update_all(core_context);
      };
      
      render_modes_container[render::modes::religion_groups] = [this] () {
        for (size_t i = 0; i < core::map::hex_count_d(core::map::detail_level); ++i) {
          auto tile = core_context->get_entity<core::tile>(i);
          tile->texture = {GPU_UINT_MAX};
          
          if (tile->province == UINT32_MAX) {
            const auto &current_biome = core_context->get_entity<core::biome>(tile->biome_index);
            const bool is_water = current_biome->get_attribute(core::biome::attributes::water);
            tile->color = is_water ? current_biome->data.color : render::make_color(0.3f, 0.3f, 0.3f, 1.0f);
            tile->texture = is_water ? current_biome->data.texture : render::image_t{GPU_UINT_MAX};
            //tile->color = current_biome->data.color;
            //tile->texture = current_biome->data.texture;
            continue;
          }
          
          const auto province = core_context->get_entity<core::province>(tile->province);
          auto religion_group = province->religion->group;
          tile->color = religion_group->color;
        }
        
        global::get<render::tile_updater>()->update_all(core_context);
      };
      
      render_modes_container[render::modes::provinces] = [this] () { // не уверен что это вооще нужно
        ASSERT(core_context->get_entity_count<core::province>() != 0);
        for (size_t i = 0; i < core::map::hex_count_d(core::map::detail_level); ++i) {
          auto tile = core_context->get_entity<core::tile>(i);
          tile->texture = {GPU_UINT_MAX};
          
          if (tile->province == UINT32_MAX) {
            const auto &current_biome = core_context->get_entity<core::biome>(tile->biome_index);
            const bool is_water = current_biome->get_attribute(core::biome::attributes::water);
            tile->color = is_water ? current_biome->data.color : render::make_color(0.3f, 0.3f, 0.3f, 1.0f);
            tile->texture = is_water ? current_biome->data.texture : render::image_t{GPU_UINT_MAX};
            //tile->color = current_biome->data.color;
            //tile->texture = current_biome->data.texture;
            continue;
          }
          
          const uint32_t rand_num1 = render::prng(tile->province);
          const uint32_t rand_num2 = render::prng(rand_num1);
          const uint32_t rand_num3 = render::prng(rand_num2);
          const float color_r = render::prng_normalize(rand_num1);
          const float color_g = render::prng_normalize(rand_num2);
          const float color_b = render::prng_normalize(rand_num3);
          tile->color = render::make_color(color_r, color_b, color_g, 1.0f);
        }
        
        global::get<render::tile_updater>()->update_all(core_context);
      };
      
      // возможно имеет смысл для следующих режимов придумать способ как можно сделать то же самое но от провинций
      render_modes_container[render::modes::countries] = [this] () {
        for (size_t i = 0; i < core::map::hex_count_d(core::map::detail_level); ++i) {
          auto tile = core_context->get_entity<core::tile>(i);
          tile->texture = {GPU_UINT_MAX};
          
          if (tile->province == UINT32_MAX) {
            const auto &current_biome = core_context->get_entity<core::biome>(tile->biome_index);
            const bool is_water = current_biome->get_attribute(core::biome::attributes::water);
            tile->color = is_water ? current_biome->data.color : render::make_color(0.3f, 0.3f, 0.3f, 1.0f);
            tile->texture = is_water ? current_biome->data.texture : render::image_t{GPU_UINT_MAX};
            //tile->color = current_biome->data.color;
            //tile->texture = current_biome->data.texture;
            continue;
          }
          
          auto province = core_context->get_entity<core::province>(tile->province);
          auto realm = province->title->owner;
          ASSERT(realm != nullptr);
          auto final_realm = realm;
          while (realm != nullptr) {
            final_realm = realm;
            realm = realm->liege;
          }
          ASSERT(final_realm != nullptr);
          
          // какое то число? пока что приведем указатель
          const auto color = final_realm->main_title->main_color;
          tile->color = color;
        }
        
        global::get<render::tile_updater>()->update_all(core_context);
      };
      
      render_modes_container[render::modes::duchies_de_jure] = [this] () {
        for (size_t i = 0; i < core::map::hex_count_d(core::map::detail_level); ++i) {
          auto tile = core_context->get_entity<core::tile>(i);
          tile->texture = {GPU_UINT_MAX};
          
          // возможно имеет смысл тайлы земли перекрасить в серый
          if (tile->province == UINT32_MAX) {
            const auto &current_biome = core_context->get_entity<core::biome>(tile->biome_index);
            const bool is_water = current_biome->get_attribute(core::biome::attributes::water);
            tile->color = is_water ? current_biome->data.color : render::make_color(0.3f, 0.3f, 0.3f, 1.0f);
            tile->texture = is_water ? current_biome->data.texture : render::image_t{GPU_UINT_MAX};
            continue;
          }
          
          auto province = core_context->get_entity<core::province>(tile->province);
          auto title = province->title;
          ASSERT(title != nullptr);
          const auto duchy_title = title->parent;
          
          if (duchy_title != nullptr) tile->color = duchy_title->main_color;
          else tile->color = render::make_color(0.3f, 0.3f, 0.3f, 1.0f);
        }
        
        global::get<render::tile_updater>()->update_all(core_context);
      };
      
      render_modes_container[render::modes::duchies_de_facto] = [this] () {
        for (size_t i = 0; i < core::map::hex_count_d(core::map::detail_level); ++i) {
          auto tile = core_context->get_entity<core::tile>(i);
          tile->texture = {GPU_UINT_MAX};
          
          // возможно имеет смысл тайлы земли перекрасить в серый
          if (tile->province == UINT32_MAX) {
            const auto &current_biome = core_context->get_entity<core::biome>(tile->biome_index);
            const bool is_water = current_biome->get_attribute(core::biome::attributes::water);
            tile->color = is_water ? current_biome->data.color : render::make_color(0.3f, 0.3f, 0.3f, 1.0f);
            tile->texture = is_water ? current_biome->data.texture : render::image_t{GPU_UINT_MAX};
            continue;
          }
          
          auto province = core_context->get_entity<core::province>(tile->province);
          auto title = province->title;
          ASSERT(title != nullptr);
          auto owner = province->title->owner;
          const core::titulus* duchy_title = nullptr;
          while (owner != nullptr) {
            if (owner->main_title->type() == core::titulus::type::duke) duchy_title = owner->main_title;
            owner = owner->liege;
          }
          
          if (duchy_title != nullptr) tile->color = duchy_title->main_color;
          else tile->color = render::make_color(0.3f, 0.3f, 0.3f, 1.0f);
        }
        
        global::get<render::tile_updater>()->update_all(core_context);
      };
      
      render_modes_container[render::modes::kingdoms_de_jure] = [this] () {
        for (size_t i = 0; i < core::map::hex_count_d(core::map::detail_level); ++i) {
          auto tile = core_context->get_entity<core::tile>(i);
          tile->texture = {GPU_UINT_MAX};
          
          // возможно имеет смысл тайлы земли перекрасить в серый
          if (tile->province == UINT32_MAX) {
            const auto &current_biome = core_context->get_entity<core::biome>(tile->biome_index);
            const bool is_water = current_biome->get_attribute(core::biome::attributes::water);
            tile->color = is_water ? current_biome->data.color : render::make_color(0.3f, 0.3f, 0.3f, 1.0f);
            tile->texture = is_water ? current_biome->data.texture : render::image_t{GPU_UINT_MAX};
            continue;
          }
          
          auto province = core_context->get_entity<core::province>(tile->province);
          auto title = province->title;
          ASSERT(title != nullptr);
          const auto duchy_title = title->parent;
          const auto kingdom_title = duchy_title != nullptr ? duchy_title->parent : nullptr;
          
          if (kingdom_title != nullptr) tile->color = kingdom_title->main_color;
          else tile->color = render::make_color(0.3f, 0.3f, 0.3f, 1.0f);
        }
        
        global::get<render::tile_updater>()->update_all(core_context);
      };
      
      render_modes_container[render::modes::kingdoms_de_facto] = [this] () {
        for (size_t i = 0; i < core::map::hex_count_d(core::map::detail_level); ++i) {
          auto tile = core_context->get_entity<core::tile>(i);
          tile->texture = {GPU_UINT_MAX};
          
          // возможно имеет смысл тайлы земли перекрасить в серый
          if (tile->province == UINT32_MAX) {
            const auto &current_biome = core_context->get_entity<core::biome>(tile->biome_index);
            const bool is_water = current_biome->get_attribute(core::biome::attributes::water);
            tile->color = is_water ? current_biome->data.color : render::make_color(0.3f, 0.3f, 0.3f, 1.0f);
            tile->texture = is_water ? current_biome->data.texture : render::image_t{GPU_UINT_MAX};
            continue;
          }
          
          auto province = core_context->get_entity<core::province>(tile->province);
          auto title = province->title;
          ASSERT(title != nullptr);
          auto owner = province->title->owner;
          const core::titulus* kingdom_title = nullptr;
          while (owner != nullptr) {
            if (owner->main_title->type() == core::titulus::type::king) kingdom_title = owner->main_title;
            owner = owner->liege;
          }
          
          if (kingdom_title != nullptr) tile->color = kingdom_title->main_color;
          else tile->color = render::make_color(0.3f, 0.3f, 0.3f, 1.0f);
        }
        
        global::get<render::tile_updater>()->update_all(core_context);
      };
      
      render_modes_container[render::modes::empires_de_jure] = [this] () {
        for (size_t i = 0; i < core::map::hex_count_d(core::map::detail_level); ++i) {
          auto tile = core_context->get_entity<core::tile>(i);
          tile->texture = {GPU_UINT_MAX};
          
          // возможно имеет смысл тайлы земли перекрасить в серый
          if (tile->province == UINT32_MAX) {
            const auto &current_biome = core_context->get_entity<core::biome>(tile->biome_index);
            const bool is_water = current_biome->get_attribute(core::biome::attributes::water);
            tile->color = is_water ? current_biome->data.color : render::make_color(0.3f, 0.3f, 0.3f, 1.0f);
            tile->texture = is_water ? current_biome->data.texture : render::image_t{GPU_UINT_MAX};
            continue;
          }
          
          auto province = core_context->get_entity<core::province>(tile->province);
          auto title = province->title;
          ASSERT(title != nullptr);
          const auto duchy_title = title->parent;
          const auto kingdom_title = duchy_title != nullptr ? duchy_title->parent : nullptr;
          const auto emp_title = kingdom_title != nullptr ? kingdom_title->parent : nullptr;
          
          if (emp_title != nullptr) tile->color = emp_title->main_color;
          else tile->color = render::make_color(0.3f, 0.3f, 0.3f, 1.0f);
        }
        
        global::get<render::tile_updater>()->update_all(core_context);
      };
      
      render_modes_container[render::modes::empires_de_facto] = [this] () {
        for (size_t i = 0; i < core::map::hex_count_d(core::map::detail_level); ++i) {
          auto tile = core_context->get_entity<core::tile>(i);
          tile->texture = {GPU_UINT_MAX};
          
          // возможно имеет смысл тайлы земли перекрасить в серый
          if (tile->province == UINT32_MAX) {
            const auto &current_biome = core_context->get_entity<core::biome>(tile->biome_index);
            const bool is_water = current_biome->get_attribute(core::biome::attributes::water);
            tile->color = is_water ? current_biome->data.color : render::make_color(0.3f, 0.3f, 0.3f, 1.0f);
            tile->texture = is_water ? current_biome->data.texture : render::image_t{GPU_UINT_MAX};
            continue;
          }
          
          auto province = core_context->get_entity<core::province>(tile->province);
          auto title = province->title;
          ASSERT(title != nullptr);
          auto owner = province->title->owner;
          const core::titulus* emp_title = nullptr;
          while (owner != nullptr) {
            if (owner->main_title->type() == core::titulus::type::imperial) emp_title = owner->main_title;
            owner = owner->liege;
          }
          
          if (emp_title != nullptr) tile->color = emp_title->main_color;
          else tile->color = render::make_color(0.3f, 0.3f, 0.3f, 1.0f);
        }
        
        global::get<render::tile_updater>()->update_all(core_context);
      };
    }
    
    void map_t::release_container() {
//       inited = false;
      if (!is_init()) return;
      
//       auto systems = global::get<core_t>();
//       systems->render_slots->clear_slot(2);
//       systems->render_slots->clear_slot(7);
      
      global::get(reinterpret_cast<render::tile_render*>(SIZE_MAX));
//       global::get(reinterpret_cast<render::tile_connections_render*>(SIZE_MAX));
//       global::get(reinterpret_cast<render::tile_borders_optimizer*>(SIZE_MAX));
//       global::get(reinterpret_cast<render::tile_walls_optimizer*>(SIZE_MAX));
//       global::get(reinterpret_cast<render::world_map_buffers*>(SIZE_MAX));
      
      world_buffers = nullptr;
      
      RELEASE_CONTAINER_DATA(core_context)
      RELEASE_CONTAINER_DATA(map)
      RELEASE_CONTAINER_DATA(seasons)
      RELEASE_CONTAINER_DATA(ai_systems)
      RELEASE_CONTAINER_DATA(map_creator)
      RELEASE_CONTAINER_DATA(optimizators_container)
      RELEASE_CONTAINER_DATA(render_container)
      RELEASE_CONTAINER_DATA(camera)
      container.clear();
    }
    
    void map_t::create_render_stages() {
      const size_t optimizators_size = sizeof(render::world_map_buffers) +
        sizeof(render::static_copy_array<16>) +
        sizeof(render::tile_optimizer) +
        sizeof(render::tile_updater);
//         sizeof(render::tile_borders_optimizer) + 
//         sizeof(render::tile_walls_optimizer);
        
      const size_t render_size = sizeof(render::tile_render) +
        sizeof(render::tile_border_render) +
//         sizeof(render::tile_connections_render) +
        sizeof(render::tile_object_render) +
        sizeof(render::tile_highlights_render) +
        sizeof(render::render_pass_end) +
        sizeof(render::tile_objects_optimizer) +
        sizeof(render::render_pass_begin) +
        sizeof(render::tile_structure_render) +
        sizeof(render::heraldies_render) +
        sizeof(render::armies_render);
      
      ASSERT(optimizators_container == nullptr);
      ASSERT(render_container == nullptr);
        
      optimizators_container = container.create<render::stage_container, 6>(optimizators_size);
      render_container = container.create<render::stage_container, 7>(render_size);
      auto systems = global::get<core_t>();
      auto container = systems->graphics_container;
      auto device = container->vulkan->device;
      auto allocator = container->vulkan->buffer_allocator;
      auto uniform_layout = container->vulkan->uniform_layout;
      auto tiles_layout = map->data->tiles_layout;
      auto images_layout = *systems->image_controller->get_descriptor_set_layout();
      auto buffers = optimizators_container->add_target<render::world_map_buffers>(container);
      world_buffers = buffers;
      
      auto copy = optimizators_container->add_stage<render::static_copy_array<16>>(device);
      auto upd  = optimizators_container->add_stage<render::tile_updater>(render::tile_updater::create_info{device, allocator, map});
      auto opt1 = optimizators_container->add_stage<render::tile_optimizer>(render::tile_optimizer::create_info{device, allocator, tiles_layout, uniform_layout});
//       auto opt2 = optimizators_container->add_stage<render::tile_borders_optimizer>(render::tile_borders_optimizer::create_info{device, buffers});
//       auto opt3 = optimizators_container->add_stage<render::tile_walls_optimizer>(render::tile_walls_optimizer::create_info{device, buffers});
      
      auto tiles   = render_container->add_stage<render::tile_render>(render::tile_render::create_info{container, tiles_layout, images_layout, opt1});
      auto borders = render_container->add_stage<render::tile_border_render>(render::tile_border_render::create_info{container, tiles_layout, images_layout, opt1, buffers});
//       auto walls   = render_container->add_stage<render::tile_connections_render>(render::tile_connections_render::create_info{device, opt1, buffers});
                     render_container->add_stage<render::tile_object_render>(render::tile_object_render::create_info{container, tiles_layout, images_layout, opt1, buffers});
      auto thl     = render_container->add_stage<render::tile_highlights_render>(render::tile_highlights_render::create_info{container, tiles_layout, buffers});
//                      render_container->add_stage<render::render_pass_end>();
//                      render_container->add_stage<render::tile_objects_optimizer>(render::tile_objects_optimizer::create_info{device, allocator, tiles_layout, uniform_layout, opt1});
//                      render_container->add_stage<render::render_pass_begin>(1);
      auto tsr     = render_container->add_stage<render::tile_structure_render>(render::tile_structure_render::create_info{container, tiles_layout, images_layout, opt1, world_buffers});
      auto ar      = render_container->add_stage<render::armies_render>(render::armies_render::create_info{container, tiles_layout, images_layout, opt1, world_buffers});
      auto hr      = render_container->add_stage<render::heraldies_render>(render::heraldies_render::create_info{container, tiles_layout, images_layout, opt1, world_buffers});
      
      copy->add(world_buffers);
      copy->add(tsr);
      copy->add(hr);
      copy->add(ar);
                     
//       systems->render_slots->set_stage(2, optimizators_container);
//       systems->render_slots->set_stage(7, render_container);
      
      global::get(tiles);
//       global::get(walls);
      global::get(opt1);
      global::get(thl);
//       global::get(world_buffers);
//       global::get(opt2);
//       global::get(opt3);
      global::get(tsr);
      global::get(hr);
      global::get(ar);
      global::get(copy);
      global::get(upd);
      UNUSED_VARIABLE(borders);
    }
    
    void map_t::lock_map() {
      if (!is_init()) return;
      //if (!is_rendering()) return;
//       auto s = map->status();
//       if (s == core::map::status::valid) map->set_status(core::map::status::rendering);
      while (!map->mutex.try_lock()) { std::this_thread::sleep_for(std::chrono::microseconds(1)); }
    }
    
    void map_t::unlock_map() {
      if (!is_init()) return;
//       auto s = map->status();
//       if (s == core::map::status::rendering) map->set_status(core::map::status::valid);
      map->mutex.unlock();
    }
    
    void map_t::start_rendering() {
      if (!is_init()) return;
      std::unique_lock<std::mutex> lock(map->mutex);
      auto systems = global::get<core_t>();
      systems->render_slots->set_stage(OPTIMIZATOR_STAGE_SLOT, optimizators_container);
      systems->render_slots->set_stage(RENDER_STAGE_SLOT, render_container);
      std::cout << "start render world map" << "\n";
    }
    
    void map_t::stop_rendering() {
      if (!is_init()) return;
      std::unique_lock<std::mutex> lock(map->mutex);
      auto systems = global::get<core_t>();
      systems->render_slots->clear_slot(RENDER_STAGE_SLOT);
      systems->render_slots->clear_slot(OPTIMIZATOR_STAGE_SLOT);
      std::cout << "stop render world map" << "\n";
    }
    
    bool map_t::is_rendering() const {
      if (!is_init()) return false;
      std::unique_lock<std::mutex> lock(map->mutex); // 
      const auto systems = global::get<core_t>();
      return systems->render_slots->get_stage(RENDER_STAGE_SLOT) == render_container || 
             systems->render_slots->get_stage(OPTIMIZATOR_STAGE_SLOT) == optimizators_container;
    }
    
    void map_t::setup_map_generator() {
      ASSERT(is_init());
      //ASSERT(map_creator == nullptr);
      if (map_creator != nullptr) return;
      auto systems = global::get<core_t>();
      map_creator = container.create<map::creator, 5>(map, seasons); // map::creator занимает 5 кб
      map_creator->setup_map_generator_functions(systems->interface_container.get());
      //map_creator = new map::creator(systems->interface);
    }
    
    void map_t::destroy_map_generator() {
      RELEASE_CONTAINER_DATA(map_creator)
      //delete map_creator;
      //map_creator = nullptr;
    }
    
    void map_t::save() {
      // тут будем сохранять игру перед загрузкой например в битву
      // или просто когда делаем сохранение, сохраняем и отсюда берем строку
    }
    
    std::string map_t::get_data() const {
      return saved_data;
    }
    
    void map_t::set_data(std::string &&data) {
      ASSERT(saved_data.empty());
      saved_data = std::move(data);
    }
    
    void map_t::load() {
      ASSERT(!saved_data.empty());
      // грузим из строки, то есть восстанавливаем стейт в map, core_context и seasons
      // в этом классе должны быть все (!) классы необходимые для глобальной карты
    }
    
    void map_t::clear_saved_data() {
      saved_data.clear();
      saved_data.shrink_to_fit(); // кажется это чистит капасити
      ASSERT(saved_data.capacity() == 0);
    }
    
//     void map_t::dump_world_to_disk() {
//       ASSERT(is_init());
//       ASSERT(map_creator != nullptr);
//       
//       // нужно еще проверить есть ли уже такой мир
//       
//       utils::world_serializator dumper;
//       dumper.set_name(world_name);
//       dumper.set_technical_name(folder_name);
//       dumper.set_settings(generator_settings);
//       dumper.set_seed(1); // по идее получаем из map_creator
//       
//       // для дампа мира нужен табле то стринг, который проиходит в другом месте совсем
//     }
    
    battle_t::battle_t() : 
      container(
        {
          sizeof(battle::map),
          sizeof(systems::ai),
          sizeof(render::stage_container),
          sizeof(render::stage_container),
          sizeof(components::battle_camera),
          sizeof(battle::lua_container),
//           sizeof(battle::unit_states_container),
          sizeof(battle::context),
          sizeof(utils::data_string_container)
//           sizeof(utils::random_engine_st),
//           sizeof(FastNoise),
//           sizeof(map::generator::container),
//           sizeof(sol::state)
        }
      ),
      ai_systems(nullptr),
      map(nullptr),
      optimizators_container(nullptr),
      render_container(nullptr),
      camera(nullptr),
      lua_states(nullptr),
//       unit_states_container(nullptr),
      context(nullptr),
      unit_states_map(nullptr)
//       random(nullptr),
//       noiser(nullptr),
//       generator_container(nullptr)
    {}
    
    battle_t::~battle_t() {
      release_container();
    }
    
    //bool battle_t::is_init() const { return container.inited(); }
    void battle_t::create_map_container() {
      if (!is_init()) container.init();
      
      auto core = global::get<systems::core_t>();
      auto cont = core->graphics_container;
      map = container.create<battle::map, 0>(battle::map::create_info{cont});
      map->create_tiles(128, 128, battle::map::coordinate_system::square, battle::map::orientation::even_pointy);
      
      camera = container.create<components::battle_camera, 4>(glm::vec3(0.0f, 3.0f, 0.0f));
      lua_states = container.create<battle::lua_container, 5>();
//       unit_states_container = container.create<battle::unit_states_container, 6>();
      context = container.create<battle::context, 6>();
      unit_states_map = container.create<utils::data_string_container, 7>();
    }
    
    void battle_t::create_render_stages() {
      if (!is_init()) throw std::runtime_error("Container is not inited properly");
      
      const size_t optimizators_size = sizeof(render::battle::tile_optimizer);
        
      const size_t render_size = sizeof(render::battle::tile_render) + sizeof(render::battle::biome_render) + sizeof(render::battle::units_render);
      
      ASSERT(optimizators_container == nullptr);
      ASSERT(render_container == nullptr);
        
      optimizators_container = container.create<render::stage_container, 2>(optimizators_size);
      render_container = container.create<render::stage_container, 3>(render_size);
      auto systems = global::get<core_t>();
      auto cont = systems->graphics_container;
      auto map_layout = *map->get_descriptor_set_layout();
      
      auto opt1 = optimizators_container->add_stage<render::battle::tile_optimizer>(render::battle::tile_optimizer::create_info{cont, map_layout});
      render_container->add_stage<render::battle::tile_render>(render::battle::tile_render::create_info{cont, map_layout, opt1});
      render_container->add_stage<render::battle::biome_render>(render::battle::biome_render::create_info{cont, map_layout, opt1});
      render_container->add_stage<render::battle::units_render>(render::battle::units_render::create_info{cont, map_layout, opt1});
//       systems->render_slots->set_stage(2, optimizators_container);
//       systems->render_slots->set_stage(7, render_container);
      
      global::get(opt1);
      
      // нужно еще продумать способы синхронизации
      // иногда мне нужно подождать пока другой поток применит изменения 
      // и кажется ничего более адекватного чем мьютекс еще не придумали
      
      opt1->update_containers(); // перенесен из квест стейтов
    }
    
//     void battle_t::setup_generator_random() {
//       const size_t seed = global::advance_state();
//       random = container.create<utils::random_engine_st, 5>(seed);
//       noiser = container.create<FastNoise, 6>(reinterpret_cast<const int*>(&seed)[0]);
// //       generator_state = container.create<sol::state, 8>();
//     }
//     
//     void battle_t::setup_generator_container(const size_t &tiles_count) {
//       generator_container = container.create<map::generator::container, 7>(tiles_count);
//     }
//     
//     void battle_t::release_generator_data() {
//       RELEASE_CONTAINER_DATA(random)
//       RELEASE_CONTAINER_DATA(noiser)
//       RELEASE_CONTAINER_DATA(generator_container)
// //       RELEASE_CONTAINER_DATA(generator_state)
//     }
    
    void battle_t::release_container() {
      if (!is_init()) return;
//       auto systems = global::get<core_t>();
//       systems->render_slots->clear_slot(2);
//       systems->render_slots->clear_slot(7);
      
      global::get(reinterpret_cast<render::battle::tile_optimizer*>(SIZE_MAX));
      
      RELEASE_CONTAINER_DATA(ai_systems)
      RELEASE_CONTAINER_DATA(map)
      RELEASE_CONTAINER_DATA(optimizators_container)
      RELEASE_CONTAINER_DATA(render_container)
      RELEASE_CONTAINER_DATA(camera)
      RELEASE_CONTAINER_DATA(lua_states)
//       RELEASE_CONTAINER_DATA(unit_states_container)
      RELEASE_CONTAINER_DATA(context)
      RELEASE_CONTAINER_DATA(unit_states_map)
      
      container.clear();
    }
    
    void battle_t::lock_map() {
      if (!is_init()) return;
      while (map->mutex.try_lock()) { std::this_thread::sleep_for(std::chrono::microseconds(1)); }
    }
    
    void battle_t::unlock_map() {
      if (!is_init()) return;
      map->mutex.unlock();
    }
    
    void battle_t::start_rendering() {
      if (!is_init()) return;
      std::unique_lock<std::mutex> lock(map->mutex);
      auto systems = global::get<core_t>();
      systems->render_slots->set_stage(2, optimizators_container);
      systems->render_slots->set_stage(7, render_container);
    }
    
    void battle_t::stop_rendering() {
      if (!is_init()) return;
      std::unique_lock<std::mutex> lock(map->mutex);
      auto systems = global::get<core_t>();
      systems->render_slots->clear_slot(7);
      systems->render_slots->clear_slot(2);
    }
    
    encounter_t::encounter_t() :
      container(
        {
          sizeof(systems::ai)
        }
      ),
      ai_systems(nullptr)
    {}
    
    encounter_t::~encounter_t() {
      RELEASE_CONTAINER_DATA(ai_systems)
    }
    
    //bool encouter_t::is_init() const { return container.inited(); }
  }
}
