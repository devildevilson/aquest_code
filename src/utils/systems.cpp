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
#include "lua_init/lua_initialization.h"
#include "settings.h"
//#include "astar_search.h"
#include "astar_search_mt.h"
//#include "string_bank.h"
#include "localization_container.h"
#include "game_context.h"
#include "bin/loading_functions.h"

#include "generator/system.h"

#include "script/system.h"
#include "script/functions_init.h"

#include "render/window.h"
#include "render/render.h"
#include "render/stages.h"
#include "render/targets.h"
#include "render/container.h"
#include "render/shared_structures.h"
#include "render/persistent_resources.h"
// #include "render/slots.h"
#include "render/image_container.h"
#include "render/image_controller.h"
#include "render/map_data.h"
#include "render/render_mode_container.h"
#include "render/command_buffer.h"
#include "render/pass.h"
#include "render/queue.h"
#include "render/proxy_stage.h"
#include "render/makers.h"
#include "render/defines.h"

#include "ai/sub_system.h"
#include "ai/ai_system.h"
#include "ai/build_subsystem.h"
#include "ai/path_container.h"

//#include "bin/map_generators2.h"
//#include "bin/generator_system2.h"
#include "generator/container.h"
#include "generator/context2.h"
#include "bin/interface_context.h"
#include "bin/map_creator.h"
// #include "bin/interface2.h"
#include "bin/game_time.h"
#include "bin/camera.h"
#include "bin/objects_selection.h"
#include "bin/logic.h"
#include "bin/application.h"
#include "bin/game_resources.h"

#include "core/render_stages.h"
#include "core/map.h"
#include "core/context.h"
#include "core/internal_lua_state.h"
#include "core/seasons.h"

#include "battle/render_stages.h"
#include "battle/lua_states.h"
#include "battle/unit_states_container.h"
#include "battle/context.h"
#include "battle/map.h"

#include "re2/re2.h"

#include <filesystem>

#define OPTIMIZATOR_STAGE_SLOT 2
#define RENDER_STAGE_SLOT 7

static const std::vector<const char*> instanceLayers = {
  "VK_LAYER_LUNARG_standard_validation",
  "VK_LAYER_LUNARG_api_dump",
  "VK_LAYER_LUNARG_assistant_layer"
};

namespace devils_engine {
  static bool valid_num(const int32_t num, const size_t &size) {
    return num > 0 && size_t(num) < size;
  }
  
  core::gen_step_t::gen_step_t(game_resources_t* res) : loading_interface(nullptr), res(res) {}
  core::gen_step_t::~gen_step_t() {}
  void core::gen_step_t::update() {
    if (finished()) return;
    if (!notify.valid()) {
      notify = res->pool.submit([this] () {
        auto gen = global::get<systems::generator_t>()->gen.get();
        gen->steps[gen->cur_step].pairs[counter].second(&gen->ctx, gen->table);
      });
    }
    
    auto status = notify.wait_for(std::chrono::milliseconds(0));
    if (status == std::future_status::ready) { 
      ++counter; 
      if (finished()) {
        auto gen = global::get<systems::generator_t>()->gen.get();
        ++gen->cur_step;
      }
      notify.get();
    }
  }
  bool core::gen_step_t::finished() const { return counter >= count(); }
  size_t core::gen_step_t::current() const { return counter; }
  size_t core::gen_step_t::count() const { 
    auto gen = global::get<systems::generator_t>()->gen.get();
    return valid_num(gen->cur_step, gen->steps.size()) ? gen->steps[gen->cur_step].pairs.size() : 0; 
  }
  const std::string_view end = "end";
  std::string_view core::gen_step_t::hint1() const { 
    //return "generate_map"; 
    auto gen = global::get<systems::generator_t>()->gen.get();
    return valid_num(gen->cur_step, gen->steps.size()) ? gen->steps[gen->cur_step].name : std::string_view();
  }
  std::string_view core::gen_step_t::hint2() const { 
    auto gen = global::get<systems::generator_t>()->gen.get();
    return valid_num(gen->cur_step, gen->steps.size()) ? gen->steps[gen->cur_step].pairs[counter].first : end;
  }
  std::string_view core::gen_step_t::hint3() const { 
//     auto gen = global::get<systems::generator_t>()->gen.get();
//     return valid_num(gen->cur_step, gen->steps.size()) ? gen->steps[gen->cur_step].name : std::string_view();
    return "";
  }
  
  namespace systems {
#define RELEASE_CONTAINER_DATA(var) if (var != nullptr) container.destroy(var); var = nullptr;
    
    core_t::core_t() :
      container(
        sizeof(input::keys) * player::states_count +
        sizeof(devils_engine::render::container) +
        sizeof(render::image_container) +
        sizeof(render::image_controller) +
        sizeof(interface::context) +
        sizeof(utils::main_menu) +
        sizeof(game::context) + 
        sizeof(utils::interface_container) +
        sizeof(utils::sequential_string_container) +
        sizeof(utils::calendar) +
        sizeof(utils::progress_container) +
        sizeof(utils::objects_selection)*2 +
        sizeof(struct path_managment) +
        sizeof(localization::container)
        , 8
      ),
      keys_mapping{nullptr},
      graphics_container(nullptr),
      proxy_compute(nullptr),
      proxy_graphics(nullptr),
      image_container(nullptr),
      image_controller(nullptr),
      context(nullptr),
      interface(nullptr),
      menu(nullptr),
      game_ctx(nullptr),
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
      RELEASE_CONTAINER_DATA(image_controller)
      RELEASE_CONTAINER_DATA(image_container)
      RELEASE_CONTAINER_DATA(graphics_container)
      RELEASE_CONTAINER_DATA(game_ctx)
      RELEASE_CONTAINER_DATA(sequential_string_container)
      RELEASE_CONTAINER_DATA(game_calendar)
      RELEASE_CONTAINER_DATA(loading_progress)
      RELEASE_CONTAINER_DATA(selection.primary)
      RELEASE_CONTAINER_DATA(selection.secondary)
      RELEASE_CONTAINER_DATA(path_managment)
      
      for (size_t i = 0; i < player::states_count; ++i) {
        RELEASE_CONTAINER_DATA(keys_mapping[i])
      }
    }
    
    void core_t::create_utility_systems() {
      sequential_string_container = container.create<utils::sequential_string_container>();
      utils::id::set_container(sequential_string_container);
      game_calendar = container.create<utils::calendar>();
      global::get(game_calendar);

      std::random_device dev;
      const uint32_t v1 = dev();
      const uint32_t v2 = dev();
      const size_t seed = (size_t(v2) << 32) | size_t(v1);
      global g;
      g.initialize_state(seed);
      for (size_t i = 0; i < player::states_count; ++i) {
        keys_mapping[i] = container.create<input::keys>();
      }

      input::set_menu_key_map(keys_mapping[player::in_menu]);
      
      loading_progress = container.create<utils::progress_container>();
      
      path_managment = container.create<struct path_managment>(std::thread::hardware_concurrency());
      
      global::get(path_managment);
      
      game_ctx = container.create<game::context>();
      turn_status.reset(new game::turn_status);
      game_ctx->turn_status = turn_status.get();
      selection.primary = container.create<utils::objects_selection>();
      selection.secondary = container.create<utils::objects_selection>();
    }
    
    void core_t::create_render_system(const char** ext, const uint32_t &count) {
      using namespace devils_engine::render;
      const size_t stage_container_size =
        sizeof(render::buffers) +
        sizeof(render::proxy_stage) +
        sizeof(render::proxy_stage) +
        sizeof(render::pass_container) +
        sizeof(render::pass_container) +
        sizeof(render::interface_stage) +
        sizeof(render::skybox) +
        sizeof(render::secondary_buffer_subpass) +
        0;

      if (count == 0) {
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
      
      graphics_container = container.create<render::container>();
      auto window = graphics_container->create_window(inf);
      graphics_container->create_instance(extensions, &info);
      graphics_container->create_device();
      graphics_container->create_vlk_window();
      graphics_container->create_swapchain();
      graphics_container->create_system(stage_container_size); // auto render = 
      graphics_container->create_tasks();

      global::get(window);
      global::get(graphics_container);
      
      auto device = &graphics_container->vulkan->device;
      auto physical_device = &graphics_container->vulkan->physical_device;
      auto pool = graphics_container->vulkan->transfer_command_pool;
      auto queue = graphics_container->vulkan->graphics;
      auto fence = graphics_container->vulkan->transfer_fence;
      image_container = container.create<render::image_container>(render::image_container::create_info{device, physical_device, &pool, &queue, &fence});
      image_controller = container.create<render::image_controller>(device, image_container);
      
      context = container.create<devils_engine::interface::context>(graphics_container, window, image_container);
      global::get(context);
      
      image_controller->update_set();
    }
    
    void core_t::create_render_stages() {
      auto system = graphics_container->render;
      auto device = graphics_container->vulkan->device;
      auto window = graphics_container->window;
      auto buffers = system->add_target<render::buffers>(graphics_container);
      global::get(buffers);
      
      // вообще в данном случае необязательно городить кучу рендерпассов, достаточно лишь сделать несколько вторичных командных буферов + прокси для вычислительного шейдера
      // добавим сабпассы, в центральном положим собранный вторичный командный буфер, 3 сабпасса с одинаковыми атачментами
      render::render_pass_maker rpm(&device);
      const auto info1 = 
        rpm.attachmentBegin(static_cast<vk::Format>(graphics_container->get_surface_format()))
             // рендерпасс подгрузит инфу что уже есть в изображении, но там могут быть остатки от предыдущих рендеров, как быть?
             // возможно придется чистить в другом месте, а! можно скайбокс рисовать, мне его в любом случае сделать нужно
             .attachmentLoadOp(vk::AttachmentLoadOp::eClear)
             .attachmentStoreOp(vk::AttachmentStoreOp::eStore)
             .attachmentInitialLayout(vk::ImageLayout::eUndefined) // ожидаем что придет колор атачмент либо после скайбокса либо после отрисовки
             .attachmentFinalLayout(vk::ImageLayout::ePresentSrcKHR)
           .attachmentBegin(static_cast<vk::Format>(graphics_container->get_depth_format()))
             .attachmentLoadOp(vk::AttachmentLoadOp::eClear) // по идее буфер глубины не пишется в этом пассе, поэтому мы можем пригнорировать его совсем
             .attachmentStoreOp(vk::AttachmentStoreOp::eDontCare)
             .attachmentInitialLayout(vk::ImageLayout::eUndefined)
             .attachmentFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
           .subpassBegin(vk::PipelineBindPoint::eGraphics)
             .subpassColorAttachment(0, vk::ImageLayout::eColorAttachmentOptimal)
             .subpassDepthStencilAttachment(1, vk::ImageLayout::eDepthStencilAttachmentOptimal)
           .subpassBegin(vk::PipelineBindPoint::eGraphics)
             .subpassColorAttachment(0, vk::ImageLayout::eColorAttachmentOptimal)
             .subpassDepthStencilAttachment(1, vk::ImageLayout::eDepthStencilAttachmentOptimal)
           .subpassBegin(vk::PipelineBindPoint::eGraphics)
             .subpassColorAttachment(0, vk::ImageLayout::eColorAttachmentOptimal)
             .subpassDepthStencilAttachment(1, vk::ImageLayout::eDepthStencilAttachmentOptimal)
              // начинаем то мы не с компут шейдера, видимо придется добавить паузу
           .dependencyBegin(VK_SUBPASS_EXTERNAL, 0)
             .dependencySrcStageMask(vk::PipelineStageFlagBits::eTopOfPipe)
             .dependencyDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
             .dependencySrcAccessMask({})
             .dependencyDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
           .dependencyBegin(0, 1)
             .dependencySrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
             .dependencyDstStageMask(vk::PipelineStageFlagBits::eDrawIndirect)
             .dependencySrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
             .dependencyDstAccessMask(vk::AccessFlagBits::eMemoryRead)
           .dependencyBegin(1, 2)
             .dependencySrcStageMask(vk::PipelineStageFlagBits::eDrawIndirect)
             .dependencyDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
             .dependencySrcAccessMask(vk::AccessFlagBits::eMemoryRead)
             .dependencyDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
           .dependencyBegin(2, VK_SUBPASS_EXTERNAL)
             .dependencySrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
             .dependencyDstStageMask(vk::PipelineStageFlagBits::eBottomOfPipe)
             .dependencySrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
             .dependencyDstAccessMask({}) // это по идее последний пасс
           .get_info();
           
      auto proxy   = system->add_stage<render::proxy_stage>();
      auto proxy2  = system->add_stage<render::proxy_stage>();
      auto intr_pass = system->add_stage<render::pass_framebuffer_container>(graphics_container, info1, "main_game_pass");
      auto sky     = system->add_stage<render::skybox>(graphics_container, intr_pass, 0);
      auto subpass = system->add_stage<render::secondary_buffer_subpass>();
      auto intr    = system->add_stage<render::interface_stage>(render::interface_stage::create_info{graphics_container, *image_controller->get_descriptor_set_layout(), intr_pass, 2});
      
      // теперь немного иначе, intr_pass - это единственный пасс, в котором сначала идет скайбокс, потом загружаем командный буфер, потом рисуем интерфейс
      proxy->next = intr_pass;
      // скайбокс -> вторичный буфер -> интерфейс
      sky->next = subpass;
      subpass->next = intr;
      
      intr_pass->set_childs(sky);
      
      graphics_container->set_command_buffer_childs(proxy);
      graphics_container->set_secondary_command_buffer_childs(proxy2);
      graphics_container->set_secondary_command_buffer_renderpass(intr_pass);
      
      proxy_compute = proxy;
      proxy_graphics = proxy2;
      main_pass = intr_pass;
      
      // как мне запомнить intr_pass для пересоздания при изменении окна?
      
      const float dist = 550.0f;
      const glm::vec3 default_camera_pos = glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f)) * dist;
      const auto [w, h] = window->framebuffer_size();
      const glm::mat4 persp = glm::perspective(glm::radians(75.0f), float(w) / float(h), 0.1f, 256.0f);
      const glm::mat4 view  = glm::lookAt(default_camera_pos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
      buffers->update_projection_matrix(persp);
      buffers->update_view_matrix(view);
      buffers->update_dir(glm::normalize(-default_camera_pos));
      buffers->update_pos(default_camera_pos);
      buffers->update_persistent_state(render::prng(1));
      buffers->update_application_state(uint32_t(global::advance_state()));
      ASSERT(persp[0][0] == persp[0][0]);
      ASSERT(view[0][0] == view[0][0]);
    }
    
    void core_t::create_interface() {
      interface_container = std::make_unique<utils::interface_container>(utils::interface_container::create_info{context, game_ctx});
      
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
      
      auto map_creator = global::get<systems::map_t>()->map_creator.get();
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
//       container(
//         {
//           sizeof(core::map),
//           sizeof(core::context),
//           sizeof(core::seasons),
//           sizeof(systems::ai),
//           sizeof(render::mode_container),
//           sizeof(map::creator),
//           sizeof(render::stage_container),
//           sizeof(render::stage_container),
//           sizeof(components::world_map_camera)
//         }
//       ),
      map(nullptr),
      core_context(nullptr),
      seasons(nullptr),
      ai_systems(nullptr),
      render_modes(nullptr),
      map_creator(nullptr),
      optimizators_container(nullptr),
      render_container(nullptr),
      first_compute(nullptr),
      first_graphics(nullptr),
      camera(nullptr)
    {
      memset(hash, 0, sizeof(hash[0]) * hash_size);
      create_map_container();
      ASSERT(global::get<map_t>() == nullptr); // предполагаем что map_t единственный
      global::get(this);
    }
    
    map_t::~map_t() {
      stop_rendering();
      release_container();
      global::get(reinterpret_cast<decltype(this)>(SIZE_MAX));
    }
    
    double vectors_angle(const glm::dvec4 &first, const glm::dvec4 &second) {
      return glm::acos(glm::dot(first, second) / (glm::length(first)*glm::length(second)));
    }
    
    void map_t::create_map_container() {
//       if (is_init()) return;
      
//       container.init();
      auto systems = global::get<core_t>();
      map = std::make_unique<core::map>(core::map::create_info{systems->graphics_container});
      core_context = std::make_unique<core::context>();
      seasons = std::make_unique<core::seasons>();
      const size_t container_size = sizeof(devils_engine::ai::build_subsystem);
      ai_systems = std::make_unique<systems::ai>(container_size);
      ai_systems->add<devils_engine::ai::build_subsystem>();
      render_modes = std::make_unique<render::mode_container>();
      
      const float dist = 550.0f;
      const glm::vec3 default_camera_pos = glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f)) * dist;
      camera = std::make_unique<components::world_map_camera>(default_camera_pos);
      
      script_system.reset(new script::system);
      script::register_functions(script_system.get());
      
      create_render_stages();
      
      const auto local_map = map.get();
      const auto local_context = core_context.get();
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
//       ASSERT(is_init());
      auto &render_modes_container = *render_modes;
      render_modes_container[render::modes::biome] = [this] () {
        for (size_t i = 0; i < core::map::hex_count_d(core::map::detail_level); ++i) {
          const uint32_t biome_index = seasons->get_tile_biome(seasons->current_season, i);
          const auto &current_biome = core_context->get_entity<core::biome>(biome_index);
          auto tile = core_context->get_entity<core::tile>(i);
          tile->color = current_biome->data.color;
          tile->texture = current_biome->data.texture;
        }
        
        global::get<render::tile_updater>()->update_all(core_context.get());
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
        
        global::get<render::tile_updater>()->update_all(core_context.get());
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
        
        global::get<render::tile_updater>()->update_all(core_context.get());
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
        
        global::get<render::tile_updater>()->update_all(core_context.get());
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
        
        global::get<render::tile_updater>()->update_all(core_context.get());
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
        
        global::get<render::tile_updater>()->update_all(core_context.get());
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
        
        global::get<render::tile_updater>()->update_all(core_context.get());
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
        
        global::get<render::tile_updater>()->update_all(core_context.get());
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
        
        global::get<render::tile_updater>()->update_all(core_context.get());
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
        
        global::get<render::tile_updater>()->update_all(core_context.get());
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
        
        global::get<render::tile_updater>()->update_all(core_context.get());
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
        
        global::get<render::tile_updater>()->update_all(core_context.get());
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
        
        global::get<render::tile_updater>()->update_all(core_context.get());
      };
    }
    
    void map_t::release_container() {
//       if (!is_init()) return;
      
      global::get(reinterpret_cast<render::tile_render*>(SIZE_MAX));
      global::get(reinterpret_cast<render::world_map_buffers*>(SIZE_MAX));
      
      world_buffers = nullptr;
      first_compute = nullptr;
      first_graphics = nullptr;
      
//       RELEASE_CONTAINER_DATA(core_context)
//       RELEASE_CONTAINER_DATA(map)
//       RELEASE_CONTAINER_DATA(seasons)
//       RELEASE_CONTAINER_DATA(ai_systems)
//       RELEASE_CONTAINER_DATA(map_creator)
//       RELEASE_CONTAINER_DATA(optimizators_container)
//       RELEASE_CONTAINER_DATA(render_container)
//       RELEASE_CONTAINER_DATA(camera)
//       script_system.reset(nullptr);
//       container.clear();
    }
    
    void map_t::create_render_stages() {
      const size_t optimizators_size = sizeof(render::world_map_buffers) +
        sizeof(render::static_copy_array<16>) +
        sizeof(render::tile_optimizer) +
        sizeof(render::tile_updater);
        
      const size_t render_size = optimizators_size +
        sizeof(render::pass_container) +
        sizeof(render::proxy_stage) +
        sizeof(render::tile_render) +
        sizeof(render::tile_border_render) +
        sizeof(render::tile_object_render) +
        sizeof(render::tile_highlights_render) +
        sizeof(render::tile_objects_optimizer) +
        sizeof(render::tile_structure_render) +
        sizeof(render::heraldies_render) +
        sizeof(render::armies_render);
      
      ASSERT(optimizators_container == nullptr);
      ASSERT(render_container == nullptr);
      
      // контейнер нужен только один теперь
//       optimizators_container = container.create<render::stage_container, 6>(optimizators_size);
      render_container = std::make_unique<render::stage_container>(render_size);
      auto systems = global::get<core_t>();
      auto container = systems->graphics_container;
      auto device = container->vulkan->device;
      auto allocator = container->vulkan->buffer_allocator;
      auto uniform_layout = container->vulkan->uniform_layout;
      auto tiles_layout = map->data->tiles_layout;
      auto images_layout = *systems->image_controller->get_descriptor_set_layout();
      //auto buffers = optimizators_container->add_target<render::world_map_buffers>(container);
      auto buffers = render_container->add_target<render::world_map_buffers>(container);
      world_buffers = buffers;
      
      // в таком дизайне может возникнуть путаница что за чем следует, можно тогда вынести отдельно что за чем следует
      // но тогда это дело выглядит неудачно, логично чтобы мы оставили там только создание
      auto copy = render_container->add_stage<render::static_copy_array<16>>(device);
      auto upd  = render_container->add_stage<render::tile_updater>(render::tile_updater::create_info{device, allocator, map.get()});
      auto opt1 = render_container->add_stage<render::tile_optimizer>(render::tile_optimizer::create_info{device, allocator, tiles_layout, uniform_layout, world_buffers->tiles_rendering_data_layout});
      
      copy->next = upd;
      upd->next = opt1;
      
      auto tiles   = render_container->add_stage<render::tile_render>(render::tile_render::create_info{container, tiles_layout, images_layout, opt1, systems->main_pass, 1});
      auto borders = render_container->add_stage<render::tile_border_render>(render::tile_border_render::create_info{container, tiles_layout, images_layout, opt1, buffers, systems->main_pass, 1});
      auto tor     = render_container->add_stage<render::tile_object_render>(render::tile_object_render::create_info{container, tiles_layout, images_layout, opt1, buffers, systems->main_pass, 1});
      auto thl     = render_container->add_stage<render::tile_highlights_render>(render::tile_highlights_render::create_info{container, tiles_layout, buffers, systems->main_pass, 1});
      auto tsr     = render_container->add_stage<render::tile_structure_render>(render::tile_structure_render::create_info{container, tiles_layout, images_layout, opt1, world_buffers, systems->main_pass, 1});
      auto ar      = render_container->add_stage<render::armies_render>(render::armies_render::create_info{container, tiles_layout, images_layout, opt1, world_buffers, systems->main_pass, 1});
      auto hr      = render_container->add_stage<render::heraldies_render>(render::heraldies_render::create_info{container, tiles_layout, images_layout, opt1, world_buffers, systems->main_pass, 1});
      
      // как это можно спрятать? добавить в создание предыдущий стейдж? нет, он просто создает, а зависимости раскидаем отдельно
      tiles->next = borders;
      borders->next = tor;
      tor->next = thl;
      thl->next = tsr;
      tsr->next = ar;
      ar->next = hr;
      
      copy->add(world_buffers);
      copy->add(tsr);
      copy->add(hr);
      copy->add(ar);
      
      first_compute = copy;
      first_graphics = tiles;
      
      global::get(tiles);
      global::get(opt1);
      global::get(thl);
      global::get(tsr);
      global::get(hr);
      global::get(ar);
      global::get(copy);
      global::get(upd);
      global::get(world_buffers);
      UNUSED_VARIABLE(borders);
    }
    
    void map_t::lock_map() {
      while (!map->mutex.try_lock()) { std::this_thread::sleep_for(std::chrono::microseconds(1)); }
    }
    
    void map_t::unlock_map() {
      map->mutex.unlock();
    }
    
    void map_t::start_rendering() {
      std::unique_lock<std::mutex> lock(map->mutex);
      auto systems = global::get<core_t>();
      systems->proxy_compute->set_childs(first_compute);
      systems->proxy_graphics->set_childs(first_graphics);
      std::cout << "start render world map" << "\n";
    }
    
    void map_t::stop_rendering() {
      std::unique_lock<std::mutex> lock(map->mutex);
      auto systems = global::get<core_t>();
      systems->proxy_compute->set_childs(nullptr);
      systems->proxy_graphics->set_childs(nullptr);
      std::cout << "stop render world map" << "\n";
    }
    
    bool map_t::is_rendering() const {
      std::unique_lock<std::mutex> lock(map->mutex); // 
      const auto systems = global::get<core_t>();
      return systems->proxy_compute->get_childs()  == first_compute ||
             systems->proxy_graphics->get_childs() == first_graphics;
    }
    
//     void map_t::setup_map_generator() {
//       if (map_creator != nullptr) return;
//       auto systems = global::get<core_t>();
//       map_creator = std::make_unique<map::creator>(map.get(), seasons.get()); // map::creator занимает 5 кб
//       map_creator->setup_map_generator_functions(systems->interface_container.get());
//     }
//     
//     void map_t::destroy_map_generator() {
//       //RELEASE_CONTAINER_DATA(map_creator)
//       map_creator.reset();
//     }
    
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
    
    const char random_seed_chars1[] = {
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };
    
    const char random_seed_chars2[] = {
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
    };
    
    const size_t hex_num_count = sizeof(random_seed_chars1) / sizeof(random_seed_chars1[0]);
    static_assert(hex_num_count == 16);
    static_assert(hex_num_count == sizeof(random_seed_chars2) / sizeof(random_seed_chars2[0]));
    
    static constexpr uint64_t get_seed_from_string(const std::string_view &str) {
      uint64_t ret = 0;
      
      // у нас в строке 17 символов, последний символ '/0', он учитываться не должен
      // но размер строки все равно показывает 16
      assert(str.length() <= hex_num_count);
      const size_t last_index = str.length()-1;
      for (size_t s = 0; s < str.length(); ++s) {
        const auto c = str[s];
        bool found = false;
        for (size_t i = 0; i < hex_num_count; ++i) {
          if (c == random_seed_chars1[i] || c == random_seed_chars2[i]) {
            // начинаем с левой стороны числа поэтому (15-s)
            ret = ret | (i << (last_index-s) * 4);
            found = true;
            break;
          }
        }

        assert(found);
      }

      return ret;
    }
    
    std::string get_string_from_seed(const uint64_t &seed) {
      std::string ret(17, '\0');
      const size_t mask = 0xf;
      
      const size_t last_index = ret.length()-2;
      for (size_t s = 0; s < ret.length(); ++s) {
        // начинаем с левой стороны числа поэтому (15-s)
        const size_t num = (seed >> (last_index-s) * 4) & mask;
        ASSERT(num < hex_num_count);

        ret[s] = random_seed_chars1[num];
      }

      return ret;
    }
    
    generator_t::generator_t() : 
      seasons(new core::seasons),
      map(new core::map(core::map::create_info{global::get<core_t>()->graphics_container})),
      temp_container(new map::generator::container(core::map::hex_count_d(core::map::detail_level))),
      serializator(new utils::world_serializator),
      gen(new devils_engine::generator::system),
      random(new utils::random_engine_st(gen->rand_seed)),
      noise(new FastNoiseLite(gen->noise_seed))
    {
      ASSERT(global::get<map_t>() == nullptr); // предполагаем что map_t единственный
      global::get(this);
    }
    
    generator_t::~generator_t() { 
      stop_rendering();
      global::get(reinterpret_cast<decltype(this)>(SIZE_MAX));
    }
    
    void generator_t::create_render_stages() {
      const size_t render_size = 
        sizeof(render::world_map_buffers) + 
        sizeof(render::static_copy_array<16>) + 
        sizeof(render::tile_optimizer) + 
        sizeof(render::tile_render) + 
        sizeof(render::tile_border_render) + 
        sizeof(render::tile_highlights_render);
      
      render_container = std::make_unique<render::stage_container>(render_size);
      auto systems = global::get<core_t>();
      auto container = systems->graphics_container;
      auto device = container->vulkan->device;
      auto allocator = container->vulkan->buffer_allocator;
      auto uniform_layout = container->vulkan->uniform_layout;
      auto tiles_layout = map->data->tiles_layout;
      auto images_layout = *systems->image_controller->get_descriptor_set_layout();
      //auto buffers = optimizators_container->add_target<render::world_map_buffers>(container);
      auto buffers = render_container->add_target<render::world_map_buffers>(container);
      world_buffers = buffers;
      
      // в таком дизайне может возникнуть путаница что за чем следует, можно тогда вынести отдельно что за чем следует
      // но тогда это дело выглядит неудачно, логично чтобы мы оставили там только создание
      auto copy = render_container->add_stage<render::static_copy_array<16>>(device);
//       auto upd  = render_container->add_stage<render::tile_updater>(render::tile_updater::create_info{device, allocator, map.get()});
      auto opt1 = render_container->add_stage<render::tile_optimizer>(render::tile_optimizer::create_info{device, allocator, tiles_layout, uniform_layout, world_buffers->tiles_rendering_data_layout});
      
      copy->next = opt1;
      
      auto tiles   = render_container->add_stage<render::tile_render>(render::tile_render::create_info{container, tiles_layout, images_layout, opt1, systems->main_pass, 1});
      auto borders = render_container->add_stage<render::tile_border_render>(render::tile_border_render::create_info{container, tiles_layout, images_layout, opt1, buffers, systems->main_pass, 1});
//       auto tor     = render_container->add_stage<render::tile_object_render>(render::tile_object_render::create_info{container, tiles_layout, images_layout, opt1, buffers, systems->main_pass, 1});
      auto thl     = render_container->add_stage<render::tile_highlights_render>(render::tile_highlights_render::create_info{container, tiles_layout, buffers, systems->main_pass, 1});
//       auto tsr     = render_container->add_stage<render::tile_structure_render>(render::tile_structure_render::create_info{container, tiles_layout, images_layout, opt1, world_buffers, systems->main_pass, 1});
//       auto ar      = render_container->add_stage<render::armies_render>(render::armies_render::create_info{container, tiles_layout, images_layout, opt1, world_buffers, systems->main_pass, 1});
//       auto hr      = render_container->add_stage<render::heraldies_render>(render::heraldies_render::create_info{container, tiles_layout, images_layout, opt1, world_buffers, systems->main_pass, 1});
      
      // как это можно спрятать? добавить в создание предыдущий стейдж? нет, он просто создает, а зависимости раскидаем отдельно
      tiles->next = borders;
      borders->next = thl;
      
      copy->add(world_buffers);
//       copy->add(tsr);
//       copy->add(hr);
//       copy->add(ar);
      
      first_compute = copy;
      first_graphics = tiles;
      
      global::get(tiles);
      global::get(opt1);
      global::get(thl);
      global::get(copy);
      global::get(world_buffers);
      UNUSED_VARIABLE(borders);
    }
    
    void generator_t::start_rendering() {
      // скорее тут требуется мьютекс у рендера в целом
      // но конкретно в случае с генератором, мьютекс нужен чтобы избежать возможного чтения при записи
      std::unique_lock<std::mutex> lock(map->mutex);
      auto systems = global::get<core_t>();
      systems->proxy_compute->set_childs(first_compute);
      systems->proxy_graphics->set_childs(first_graphics);
      std::cout << "start render generator map" << "\n";
    }
    
    void generator_t::stop_rendering() {
      std::unique_lock<std::mutex> lock(map->mutex);
      auto systems = global::get<core_t>();
      systems->proxy_compute->set_childs(nullptr);
      systems->proxy_graphics->set_childs(nullptr);
      std::cout << "stop  render generator map" << "\n";
    }
    
    static const std::string_view folder_name_regex_str = "^[a-zA-Z0-9_.]+$";
    static const RE2 folder_name_regex(folder_name_regex_str);
    
    void generator_t::setup_map_generator_functions(utils::interface_container* interface) {
      // мне нужно расшарить несколько функций для интерфейса по которым 
      // я задам сид и название мира, но при этом я не очень хочу
      // функции в таком виде использовать, преждде всего потому что нужно 
      // будет убедиться что this еще существует к этому моменту
      // хотя по идее это не так сложно проверить, как передать эти функции в интерфейс?
      // опять завести таблицу? видимо

      // нужно тут создать интерфейс тоже, только по всей видимости почти без данных
      // все таки придется сериализовать таблицу
      
      auto t = interface->lua.create_table();
      t.set_function("setup_random_seed", [this] (const std::string_view &str_seed) {
        if (global::get<systems::generator_t>() == nullptr) throw std::runtime_error("Map generator does not exist");
        
        const uint64_t seed = get_seed_from_string(str_seed);
        
        gen->rand_seed = seed;
        random->set_seed(seed);
      });

      t.set_function("setup_noise_seed", [this] (const std::string_view &str_seed) {
        if (global::get<systems::generator_t>() == nullptr) throw std::runtime_error("Map generator does not exist");
        
        const uint64_t seed = get_seed_from_string(str_seed);
        
        // может заменить на более мелкое число? хотя если дать нижние регистры заполнять, 
        // можно сделать какую нибудь другие способы задания сида
        //this->noise_seed = uint32_t(seed >> 32);
        gen->noise_seed = *reinterpret_cast<const uint32_t*>(&seed);
        noise->SetSeed(*reinterpret_cast<const int32_t*>(&seed));
      });

      t.set_function("get_random_number", [] () -> std::string {
        if (global::get<systems::generator_t>() == nullptr) throw std::runtime_error("Map generator does not exist");
        
        std::random_device dev;
        static_assert(sizeof(std::random_device::result_type) == sizeof(uint32_t));
        const uint64_t tmp = (uint64_t(dev()) << 32) | uint64_t(dev());
        
        // тут нужно бы составить строку размером 16 вида: deadbeafdeadbeaf
        const auto &str = get_string_from_seed(tmp);
        return str;
      });

      t.set_function("set_world_name", [this] (const std::string_view &str) {
        if (global::get<systems::generator_t>() == nullptr) throw std::runtime_error("Map generator does not exist");
        
        if (str.empty()) throw std::runtime_error("Bad world name string");
        world_name = std::string(str);
      });

      t.set_function("set_folder_name", [this] (const std::string_view &str) {
        if (global::get<systems::generator_t>() == nullptr) throw std::runtime_error("Map generator does not exist");
        
        if (str.empty()) throw std::runtime_error("Bad world folder name string");
        //const std::regex folder_name_regex("^[a-zA-Z0-9_.]+$", std::regex_constants::icase);
        //if (!std::regex_match(str.begin(), str.end(), folder_name_regex)) throw std::runtime_error("Folder name must match ^[a-zA-Z0-9_.]+$ expression");
        if (!RE2::FullMatch(str, folder_name_regex)) throw std::runtime_error("Folder name must match " + std::string(folder_name_regex_str) + " expression");
        folder_name = std::string(str);
      });

      t.set_function("check_world_existance", [] (const std::string_view &str) {
        if (global::get<systems::generator_t>() == nullptr) throw std::runtime_error("Map generator does not exist");
        
        if (str.empty()) return 0;
        //const std::regex folder_name_regex("^[a-zA-Z0-9_.]+$", std::regex_constants::icase);
        //if (!std::regex_match(str.begin(), str.end(), folder_name_regex)) return 1;
        if (!RE2::FullMatch(str, folder_name_regex)) return 1;
        const std::filesystem::path p = global::root_directory() + "/saves/" + std::string(str) + "/world_data";
        const std::filesystem::directory_entry e(p);
        if (e.exists()) return 2;
        return 3;
      });
      
      // как быть тут? функция advance должна посылать ивент gen_step, а функция prev_step нужна штоб чутка откатить
      // prev_step должет прост индекс менять и тоже отсылать ивент gen_step
      t.set_function("advance", [] () {
        if (global::get<systems::generator_t>() == nullptr) throw std::runtime_error("Map generator does not exist");
        global::get<core::application>()->notify_gen_advance();
      });
      
      t.set_function("prev_step", [this] () {
        if (global::get<systems::generator_t>() == nullptr) throw std::runtime_error("Map generator does not exist");
        --gen->cur_step;
        if (gen->cur_step < 0) global::get<core::application>()->load_menu();
        global::get<core::application>()->notify_gen_advance();
      });
      
      // это наверное должно просто добавиться к таблице загрузки
//       t.set_function("step", [this] () -> int32_t { // sol::readonly_property(
//         if (global::get<systems::generator_t>() == nullptr) throw std::runtime_error("Map generator does not exist");
//         return TO_LUA_INDEX(current_step);
//       });
//       
//       t.set_function("steps_count", [this] () -> uint32_t { // sol::readonly_property(
//         if (global::get<systems::generator_t>() == nullptr) throw std::runtime_error("Map generator does not exist");
//         return gen->steps.size();
//       });
      
//       t.set_function("step_name", [this] () -> std::string {
//         if (global::get<systems::generator_t>() == nullptr) throw std::runtime_error("Map generator does not exist");
//         if (current_step < 0 || size_t(current_step) >= gen->steps.size()) return "";
//         return gen->steps[current_step]->step_name();
//       });
      
//       t.set_function("is_finished", [this] () -> bool { // sol::readonly_property(
//         if (global::get<systems::generator_t>() == nullptr) throw std::runtime_error("Map generator does not exist");
//         return finished();
//       });
      
      interface->setup_map_generator_functions(t);
    }
    
    bool generator_t::gen_end() const {
      return size_t(gen->cur_step) >= gen->steps.size();
    }
    
    battle_t::battle_t() : 
//       container(
//         {
//           sizeof(battle::map),
//           sizeof(systems::ai),
//           sizeof(render::stage_container),
//           sizeof(render::stage_container),
//           sizeof(components::battle_camera),
//           sizeof(battle::lua_container),
//           sizeof(battle::context),
//           sizeof(utils::data_string_container)
//         }
//       ),
      ai_systems(nullptr),
      map(nullptr),
      optimizators_container(nullptr),
      render_container(nullptr),
      first_compute(nullptr),
      first_graphics(nullptr),
      camera(nullptr),
      lua_states(nullptr),
      context(nullptr),
      unit_states_map(nullptr)
    {
      create_map_container();
      ASSERT(global::get<map_t>() == nullptr); // предполагаем что map_t единственный
      global::get(this);
    }
    
    battle_t::~battle_t() {
      stop_rendering();
      release_container();
      global::get(reinterpret_cast<decltype(this)>(SIZE_MAX));
    }
    
    void battle_t::create_map_container() {
//       if (!is_init()) container.init();
      
//       auto core = global::get<systems::core_t>();
//       auto cont = core->graphics_container;
//       map = container.create<battle::map, 0>(battle::map::create_info{cont});
//       map->create_tiles(128, 128, battle::map::coordinate_system::square, battle::map::orientation::even_pointy);
//       
//       camera = container.create<components::battle_camera, 4>(glm::vec3(0.0f, 3.0f, 0.0f));
//       lua_states = container.create<battle::lua_container, 5>();
//       context = container.create<battle::context, 6>();
//       unit_states_map = container.create<utils::data_string_container, 7>();
      
      auto core = global::get<systems::core_t>();
      auto cont = core->graphics_container;
      map = std::make_unique<battle::map>(battle::map::create_info{cont});
      map->create_tiles(128, 128, battle::map::coordinate_system::square, battle::map::orientation::even_pointy);
      
      camera = std::make_unique<components::battle_camera>(glm::vec3(0.0f, 3.0f, 0.0f));
      lua_states = std::make_unique<battle::lua_container>();
      context = std::make_unique<battle::context>();
      unit_states_map = std::make_unique<utils::data_string_container>();
    }
    
    void battle_t::create_render_stages() {
      //if (!is_init()) throw std::runtime_error("Container is not inited properly");
      
      const size_t optimizators_size = sizeof(render::battle::tile_optimizer);
        
      const size_t render_size = optimizators_size + sizeof(render::pass_container) + sizeof(render::battle::tile_render) + sizeof(render::battle::biome_render) + sizeof(render::battle::units_render);
      
      ASSERT(optimizators_container == nullptr);
      ASSERT(render_container == nullptr);
      
      //render_container = container.create<render::stage_container, 3>(render_size);
      render_container = std::make_unique<render::stage_container>(render_size);
      auto systems = global::get<core_t>();
      auto cont = systems->graphics_container;
      auto map_layout = *map->get_descriptor_set_layout();
      
      auto opt1 = render_container->add_stage<render::battle::tile_optimizer>(render::battle::tile_optimizer::create_info{cont, map_layout});
      auto tr   = render_container->add_stage<render::battle::tile_render>(render::battle::tile_render::create_info{cont, map_layout, opt1, systems->main_pass});
      auto br   = render_container->add_stage<render::battle::biome_render>(render::battle::biome_render::create_info{cont, map_layout, opt1, systems->main_pass});
      auto ur   = render_container->add_stage<render::battle::units_render>(render::battle::units_render::create_info{cont, map_layout, opt1, systems->main_pass});
      
      // оптимизация -> рендерпасс: тайлы -> биомы -> юниты (рендерпасс во вторичном командном буфере)
      tr->next = br;
      br->next = ur;
      
      first_compute = opt1;
      first_graphics = tr;
      
      global::get(opt1);
      
      opt1->update_containers(); // перенесен из квест стейтов
    }
    
    void battle_t::release_container() {
//       if (!is_init()) return;
      
      global::get(reinterpret_cast<render::battle::tile_optimizer*>(SIZE_MAX));
      
      first_compute = nullptr;
      first_graphics = nullptr;
      
//       RELEASE_CONTAINER_DATA(ai_systems)
//       RELEASE_CONTAINER_DATA(map)
//       RELEASE_CONTAINER_DATA(optimizators_container)
//       RELEASE_CONTAINER_DATA(render_container)
//       RELEASE_CONTAINER_DATA(camera)
//       RELEASE_CONTAINER_DATA(lua_states)
// //       RELEASE_CONTAINER_DATA(unit_states_container)
//       RELEASE_CONTAINER_DATA(context)
//       RELEASE_CONTAINER_DATA(unit_states_map)
//       
//       container.clear();
    }
    
    // нужно избавить нахрен
    void battle_t::lock_map() {
//       if (!is_init()) return;
      while (map->mutex.try_lock()) { std::this_thread::sleep_for(std::chrono::microseconds(1)); }
    }
    
    void battle_t::unlock_map() {
//       if (!is_init()) return;
      map->mutex.unlock();
    }
    
    // по идее start/stop не в потоках
    void battle_t::start_rendering() {
//       if (!is_init()) return;
      std::unique_lock<std::mutex> lock(map->mutex);
      auto systems = global::get<core_t>();
      systems->proxy_compute->set_childs(first_compute);
      systems->proxy_graphics->set_childs(first_graphics);
    }
    
    void battle_t::stop_rendering() {
//       if (!is_init()) return;
      std::unique_lock<std::mutex> lock(map->mutex);
      auto systems = global::get<core_t>();
      systems->proxy_compute->set_childs(nullptr);
      systems->proxy_graphics->set_childs(nullptr);
    }
    
    encounter_t::encounter_t() :
//       container(
//         {
//           sizeof(systems::ai)
//         }
//       ),
      ai_systems(nullptr)
    {
      ASSERT(global::get<map_t>() == nullptr); // предполагаем что map_t единственный
      global::get(this);
    }
    
    encounter_t::~encounter_t() {
//       RELEASE_CONTAINER_DATA(ai_systems)
      global::get(reinterpret_cast<decltype(this)>(SIZE_MAX));
    }
  }
}
