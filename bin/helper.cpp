#include "helper.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <thread>

#include "whereami.h"

static const std::vector<const char*> instanceLayers = {
  "VK_LAYER_LUNARG_standard_validation",
  "VK_LAYER_LUNARG_api_dump",
  "VK_LAYER_LUNARG_assistant_layer"
};

namespace devils_engine {
#define RELEASE_CONTAINER_DATA(var) if (var != nullptr) container.destroy(var); var = nullptr;

  system_container_t::system_container_t() :
    container(
      sizeof(render::container) +
      sizeof(systems::generator<map::generator_context>) + 
      sizeof(core::map) + 
      sizeof(interface::context)
    ),
    graphics_container(nullptr),
    map(nullptr),
    map_generator(nullptr),
    context(nullptr)
  {}

  system_container_t::~system_container_t() {
    RELEASE_CONTAINER_DATA(map)
    RELEASE_CONTAINER_DATA(map_generator)
    RELEASE_CONTAINER_DATA(graphics_container)
    RELEASE_CONTAINER_DATA(context)
  }

  glfw_t::glfw_t() {
    if (glfwInit() != GLFW_TRUE) {
      //Global::console()->printE("Cannot init glfw!");
      throw std::runtime_error("Cannot init glfw!");
    }

    if (glfwVulkanSupported() != GLFW_TRUE) {
      //Global::console()->printE("Vulkan is not supported!");
      throw std::runtime_error("Vulkan is not supported!");
    }

    glfwSetErrorCallback(callback);
  }

  glfw_t::~glfw_t() {
    glfwTerminate();
  }
  
  void set_application_path() {
    int32_t dirname = 0;
    const int32_t length = wai_getExecutablePath(nullptr, 0, &dirname);
    ASSERT(length > 0);
    
    char array[length+1];
    wai_getExecutablePath(array, length, &dirname);
    
    array[length] = '\0'; // весь путь
    
    std::cout << array << "\n";
    
    array[dirname] = '\0'; // путь до папки
    
    std::cout << array << "\n";
    
    const std::string dir_path = std::string(array);
    
    ASSERT(dir_path.size() == size_t(dirname+1));
    
    global g;
    g.set_root_directory(dir_path);
  }

  void poll_events() {
    glfwPollEvents();
  }

  void return_cursor() {
    glfwSetInputMode(global::get<render::window>()->handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  }
  
  void keys_setup() {
    static const utils::id map_move = utils::id::get("map_move");
    input::set_key(GLFW_MOUSE_BUTTON_RIGHT, map_move);
  }
  
  float hit_sphere(const glm::vec4 &center, const float &radius, const utils::ray &r){
    const glm::vec4 oc = r.pos - center;
    const float a = glm::dot(r.dir, r.dir);
    const float b = 2.0f * glm::dot(oc, r.dir);
    const float c = glm::dot(oc, oc) - radius*radius;
    const float discriminant = b*b - 4.0f * a * c;
    
    if (discriminant > 0.0f) {
      float numerator = -b - sqrt(discriminant);
      if (numerator > 0.0f) return numerator / (2.0f * a);

      numerator = -b + sqrt(discriminant);
      if (numerator > 0.0f) return numerator / (2.0f * a);
      
      return -1.0f;
    }
    
    return -1.0f;
  }
  
  void mouse_input(yacs::entity* ent, const size_t &time) {
    auto camera = ent->get<components::camera>();
    auto window = global::get<render::window>();
//     auto buffers = global::get<render::buffers>();
    
    double xpos, ypos;
    static double last_xpos = 0.0, last_ypos = 0.0;
//     int32_t width, height;
    
    glfwGetCursorPos(window->handle, &xpos, &ypos);
    double delta_xpos = last_xpos - xpos, delta_ypos = last_ypos - ypos;
    last_xpos = xpos;
    last_ypos = ypos;
    
    const float sens = 5.0f;
    const float x_sens = 1.0f;
    const float y_sens = 1.0f;
    const float x_move = sens * x_sens * MCS_TO_SEC(time) * delta_xpos;
    const float y_move = sens * y_sens * MCS_TO_SEC(time) * delta_ypos;
    
    static const utils::id map_move = utils::id::get("map_move");
    if (input::is_event_pressed(map_move)) {
      camera->move(x_move, y_move);
    }
  }
  
  void zoom_input(yacs::entity* ent) {
    auto input_data = global::get<input::data>();
    auto camera = ent->get<components::camera>();
    
    // зум нужно ограничить когда мы выделяем хотя бы одно наклир окно
    
    camera->zoom_add(input_data->mouse_wheel);
    input_data->mouse_wheel = 0.0f;
  }
  
  uint32_t cast_mouse_ray() {
    auto window = global::get<render::window>();
    auto buffers = global::get<render::buffers>();
    
    double xpos, ypos;
    glfwGetCursorPos(window->handle, &xpos, &ypos);
    
    if (!window->flags.focused()) return UINT32_MAX;
    if (xpos > window->surface.extent.width) return UINT32_MAX;
    if (ypos > window->surface.extent.height) return UINT32_MAX;
    if (xpos < 0.0) return UINT32_MAX;
    if (ypos < 0.0) return UINT32_MAX;

    const glm::vec4 camera_pos = buffers->get_pos();

    const glm::mat4 inv_proj = buffers->get_inv_proj();
    const glm::mat4 inv_view = buffers->get_inv_view();

    float x = (2.0f * xpos) /  float(window->surface.extent.width) - 1.0f;
    float y = (2.0f * ypos) / float(window->surface.extent.height) - 1.0f; // тут по идее должно быть обратное значение
    float z = 1.0f;
    
    ASSERT(x >= -1.0f && x <= 1.0f);
    ASSERT(y >= -1.0f && y <= 1.0f);
    
    glm::vec3 ray_nds = glm::vec3(x, -y, z);
    glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);
    glm::vec4 ray_eye = inv_proj * ray_clip;
    ray_eye = glm::vec4(glm::vec2(ray_eye), -1.0, 0.0);
    glm::vec3 ray_wor = glm::vec3(inv_view * ray_eye);
    // don't forget to normalise the vector at some point
    const glm::vec4 dir = glm::vec4(glm::normalize(ray_wor), 0.0f);
    const glm::vec4 final_dir = glm::normalize(dir);
    
    // такое чувство, что сществует неверные треугольники
    // которые дают ошибку при вычислении
    
    const utils::ray intersect_sphere{
      camera_pos,
      final_dir
    };
    
    const float hit = hit_sphere(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), 500.0f, intersect_sphere);
    if (hit < 0.0f) return UINT32_MAX;
    
    const glm::vec4 point_on_sphere = intersect_sphere.pos + intersect_sphere.dir * hit;
    const glm::vec4 new_dir = glm::normalize(-glm::vec4(glm::vec3(point_on_sphere), 0.0f));
    
    const utils::ray casting_ray{
      point_on_sphere - new_dir * 5.0f,
      new_dir
    };
    
    const uint32_t tile_index_local1 = global::get<core::map>()->cast_ray(casting_ray);
    //PRINT_VEC4("point_on_sphere", point_on_sphere)
    return tile_index_local1;
  }
  
  void next_nk_frame() {
    auto window = global::get<render::window>();
    auto interface = global::get<interface::context>();
    auto ctx = &interface->ctx;
    
    double x, y;
    int widht, height, display_width, display_height;
    glfwGetWindowSize(window->handle, &widht, &height);
    glfwGetFramebufferSize(window->handle, &display_width, &display_height);
    global::get<input::data>()->fb_scale.x = float(display_width / widht);
    global::get<input::data>()->fb_scale.y = float(display_height / height);

//     if (global::get<input::data>()->interface_focus) {
      nk_input_begin(ctx);
      // nk_input_unicode нужен для того чтобы собирать набранный текст
      // можем ли мы воспользоваться им сразу из коллбека?
      // по идее можем, там не оч сложные вычисления
      //nk_input_unicode(ctx, 'f');

      for (uint32_t i = 0; i < global::get<input::data>()->current_text; ++i) {
        nk_input_unicode(ctx, global::get<input::data>()->text[i]);
      }

      const uint32_t rel_event = static_cast<uint32_t>(input::type::release);
      //const bool* keys = Global::data()->keys;
      const auto keys = global::get<input::data>()->key_events.container;
      nk_input_key(ctx, NK_KEY_DEL, keys[GLFW_KEY_DELETE].event != rel_event);
      nk_input_key(ctx, NK_KEY_ENTER, keys[GLFW_KEY_ENTER].event != rel_event);
      nk_input_key(ctx, NK_KEY_TAB, keys[GLFW_KEY_TAB].event != rel_event);
      nk_input_key(ctx, NK_KEY_BACKSPACE, keys[GLFW_KEY_BACKSPACE].event != rel_event);
      nk_input_key(ctx, NK_KEY_UP, keys[GLFW_KEY_UP].event != rel_event);
      nk_input_key(ctx, NK_KEY_DOWN, keys[GLFW_KEY_DOWN].event != rel_event);
      nk_input_key(ctx, NK_KEY_SHIFT, keys[GLFW_KEY_LEFT_SHIFT].event != rel_event ||
                                      keys[GLFW_KEY_RIGHT_SHIFT].event != rel_event);

      if (keys[GLFW_KEY_LEFT_CONTROL].event != rel_event ||
        keys[GLFW_KEY_RIGHT_CONTROL].event != rel_event) {
        nk_input_key(ctx, NK_KEY_COPY, keys[GLFW_KEY_C].event != rel_event);
        nk_input_key(ctx, NK_KEY_PASTE, keys[GLFW_KEY_V].event != rel_event);
        nk_input_key(ctx, NK_KEY_CUT, keys[GLFW_KEY_X].event != rel_event);
        nk_input_key(ctx, NK_KEY_TEXT_UNDO, keys[GLFW_KEY_Z].event != rel_event);
        nk_input_key(ctx, NK_KEY_TEXT_REDO, keys[GLFW_KEY_R].event != rel_event);
        nk_input_key(ctx, NK_KEY_TEXT_WORD_LEFT, keys[GLFW_KEY_LEFT].event != rel_event);
        nk_input_key(ctx, NK_KEY_TEXT_WORD_RIGHT, keys[GLFW_KEY_RIGHT].event != rel_event);
        nk_input_key(ctx, NK_KEY_TEXT_SELECT_ALL, keys[GLFW_KEY_A].event != rel_event);

        nk_input_key(ctx, NK_KEY_SCROLL_START, keys[GLFW_KEY_PAGE_DOWN].event != rel_event);
        nk_input_key(ctx, NK_KEY_SCROLL_END, keys[GLFW_KEY_PAGE_UP].event != rel_event);
        nk_input_key(ctx, NK_KEY_TEXT_START, keys[GLFW_KEY_HOME].event != rel_event);
        nk_input_key(ctx, NK_KEY_TEXT_END, keys[GLFW_KEY_END].event != rel_event);
      } else {
        nk_input_key(ctx, NK_KEY_LEFT, keys[GLFW_KEY_LEFT].event != rel_event);
        nk_input_key(ctx, NK_KEY_RIGHT, keys[GLFW_KEY_RIGHT].event != rel_event);
        nk_input_key(ctx, NK_KEY_COPY, 0);
        nk_input_key(ctx, NK_KEY_PASTE, 0);
        nk_input_key(ctx, NK_KEY_CUT, 0);
  //       nk_input_key(ctx, NK_KEY_SHIFT, 0);

        nk_input_key(ctx, NK_KEY_SCROLL_DOWN, keys[GLFW_KEY_PAGE_DOWN].event != rel_event);
        nk_input_key(ctx, NK_KEY_SCROLL_UP, keys[GLFW_KEY_PAGE_UP].event != rel_event);
        nk_input_key(ctx, NK_KEY_TEXT_LINE_START, keys[GLFW_KEY_HOME].event != rel_event);
        nk_input_key(ctx, NK_KEY_TEXT_LINE_END, keys[GLFW_KEY_END].event != rel_event);
      }

      glfwGetCursorPos(window->handle, &x, &y);
      if (window->flags.focused() && global::get<input::data>()->interface_focus) {
        // Mouse position in screen coordinates (set to -1,-1 if no mouse / on another screen, etc.)
        nk_input_motion(ctx, (int)x, (int)y);
      } else {
        nk_input_motion(ctx, -1, -1);
      }

      // тоже заменить, также наклир дает возможность обработать даблклик, как это сделать верно?
      nk_input_button(ctx, NK_BUTTON_LEFT, (int)x, (int)y, keys[GLFW_MOUSE_BUTTON_LEFT].event != rel_event);
      nk_input_button(ctx, NK_BUTTON_MIDDLE, (int)x, (int)y, keys[GLFW_MOUSE_BUTTON_MIDDLE].event != rel_event);
      nk_input_button(ctx, NK_BUTTON_RIGHT, (int)x, (int)y, keys[GLFW_MOUSE_BUTTON_RIGHT].event != rel_event);

      bool doubleClick = false;
      if (keys[GLFW_MOUSE_BUTTON_LEFT].event != rel_event) {
        auto p = std::chrono::steady_clock::now();
        auto diff = p - global::get<input::data>()->double_click_time_point;
        auto mcs = std::chrono::duration_cast<std::chrono::microseconds>(diff).count();
        doubleClick = mcs < DOUBLE_CLICK_TIME;
        if (!doubleClick) {
          global::get<input::data>()->double_click_time_point = p;
        }
      }

      nk_input_button(ctx, NK_BUTTON_DOUBLE, int(global::get<input::data>()->click_pos.x), int(global::get<input::data>()->click_pos.y), doubleClick);
      nk_input_scroll(ctx, nk_vec2(global::get<input::data>()->mouse_wheel, 0.0f));
      nk_input_end(ctx);
      // обнуляем
      global::get<input::data>()->click_pos = glm::uvec2(x, y);
      global::get<input::data>()->current_text = 0;
      global::get<input::data>()->mouse_wheel = 0.0f;

      glfwSetInputMode(window->handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
//     } else {
//       glfwSetInputMode(window->handle, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
//     }
  }

  void create_render_system(system_container_t &systems) {
    const size_t stageContainerSize =
      sizeof(render::buffers) +
      //sizeof(render::images) +
      //sizeof(render::particles) +
      //sizeof(render::deffered) +

      sizeof(render::window_next_frame) +
      sizeof(render::task_begin) +
      
      sizeof(render::tile_optimizer) +

      sizeof(render::render_pass_begin) +
      sizeof(render::tile_render) +
      sizeof(render::tile_border_render) +
      sizeof(render::interface_stage) +
      sizeof(render::render_pass_end) +

      sizeof(render::task_end) +
      sizeof(render::task_start) +
      sizeof(render::window_present);

    uint32_t count;
    const char** ext = glfwGetRequiredInstanceExtensions(&count);
    if (count == 0) {
      //global::console()->print("Found no extensions\n");
      throw std::runtime_error("Extensions not founded!");
    }

    std::vector<const char*> extensions;
    for (uint32_t i = 0; i < count; ++i) {
      extensions.push_back(ext[i]);
    }

    const yavf::Instance::ApplicationInfo info{
      TECHNICAL_NAME,
      //APP_VERSION,
      MAKE_VERSION(0, 0, 1),
      ENGINE_NAME,
      EGINE_VERSION,
      VK_API_VERSION_1_0
    };

    systems.graphics_container = systems.container.create<render::container>();
    systems.graphics_container->create_instance(extensions, &info);
    auto window = systems.graphics_container->create_window();
    systems.graphics_container->create_device();
    window->create_swapchain(systems.graphics_container->device);
    auto render = systems.graphics_container->create_system(stageContainerSize);
    systems.graphics_container->create_tasks();
    //container.decals_system = container.container.create<systems::decals>();

    global::get(window);
    global::get(render);
    global::get(systems.graphics_container);
    //global::get(systems.decals_system);

    glfwSetKeyCallback(window->handle, keyCallback);
    glfwSetCharCallback(window->handle, charCallback);
    glfwSetMouseButtonCallback(window->handle, mouseButtonCallback);
    glfwSetScrollCallback(window->handle, scrollCallback);
    glfwSetWindowFocusCallback(window->handle, focusCallback);
    glfwSetWindowIconifyCallback(window->handle, iconifyCallback);
    glfwSetWindowSizeCallback(window->handle, window_resize_callback);
    
    systems.context = systems.container.create<interface::context>(systems.graphics_container->device, window);
    global::get(systems.context);
  }

  void create_render_stages(system_container_t &systems) {
    auto system = global::get<systems::render>();
    auto device = systems.graphics_container->device;
    auto window = global::get<render::window>();
    auto buffers = system->add_target<render::buffers>(device);

                 system->add_stage<render::window_next_frame>(render::window_next_frame::create_info{window});
                 system->add_stage<render::task_begin>();
                 
    auto opt   = system->add_stage<render::tile_optimizer>(render::tile_optimizer::create_info{device});
                 
                 system->add_stage<render::render_pass_begin>();
    auto tiles = system->add_stage<render::tile_render>(render::tile_render::create_info{device, opt});
                 system->add_stage<render::tile_border_render>(render::tile_border_render::create_info{device, opt, tiles});
                 system->add_stage<render::interface_stage>(render::interface_stage::create_info{device});
                 system->add_stage<render::render_pass_end>();

                 system->add_stage<render::task_end>();
    auto start = system->add_stage<render::task_start>(device);
                 system->add_stage<render::window_present>(render::window_present::create_info{window});

    global::get(start);
    global::get(tiles);
    global::get(buffers);
    
    const float dist = 550.0f;
    const glm::vec3 default_camera_pos = glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f)) * dist;
    const glm::mat4 persp = glm::perspective(glm::radians(75.0f), float(window->surface.extent.width) / float(window->surface.extent.height), 0.1f, 256.0f);
    const glm::mat4 view  = glm::lookAt(default_camera_pos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
//     buffers->update_matrix(persp * view);
    buffers->update_projection_matrix(persp);
    buffers->update_view_matrix(view);
    buffers->update_dir(glm::normalize(-default_camera_pos));
    buffers->update_pos(default_camera_pos);
    ASSERT(persp[0][0] == persp[0][0]);
    ASSERT(view[0][0] == view[0][0]);
  }
  
  void create_map_container(system_container_t &systems) {
    systems.map = systems.container.create<core::map>(core::map::create_info{systems.graphics_container->device});
    global::get(systems.map);
  }
  
  void create_map_generator(system_container_t &systems, dt::thread_pool* pool, map::generator_context* context) {
    const size_t map_generator_size = 
      sizeof(map::beginner) +
      sizeof(map::plates_generator) + 
      sizeof(map::plate_datas_generator) + 
      sizeof(map::compute_boundary_edges) + 
      sizeof(map::compute_plate_boundary_stress) + 
      sizeof(map::blur_plate_boundary_stress) + 
      sizeof(map::calculate_plate_boundary_distances) + 
      sizeof(map::calculate_plate_root_distances) + 
      sizeof(map::calculate_vertex_elevation) + 
      sizeof(map::generate_air_whorls) + 
      sizeof(map::calculate_vertex_air_current) + 
      sizeof(map::calculate_air_current_outflows) + 
      sizeof(map::initialize_circulating_heat) + 
      sizeof(map::propagate_circulating_heat) + 
      sizeof(map::calculate_temperatures) + 
      sizeof(map::initialize_circulating_moisture) + 
      sizeof(map::propagate_circulating_moisture) + 
      sizeof(map::calculate_wetness) + 
      sizeof(map::calculate_biomes) + 
      sizeof(map::calculate_tile_distance) * 3 + 
      sizeof(map::modify_tile_elevation) + 
      sizeof(map::compute_tile_heat) + 
      sizeof(map::normalize_fractional_values) + 
      sizeof(map::initialize_circulating_moisture) + 
      sizeof(map::propagate_circulating_moisture) + 
      sizeof(map::calculate_wetness) + 
      sizeof(map::create_biomes) + 
      sizeof(map::generate_provinces) + 
      sizeof(map::generate_cultures);
      
    systems.map_generator = systems.container.create<systems::generator<map::generator_context>>(map_generator_size);
    global::get(systems.map_generator);
    
    systems.map_generator->add_generator<map::beginner>(map::beginner::create_info{pool});
//     systems.map_generator->add_generator<map::plates_generator>(map::plates_generator::create_info{pool});
//     systems.map_generator->add_generator<map::plate_datas_generator>(map::plate_datas_generator::create_info{pool});
//     systems.map_generator->add_generator<map::compute_boundary_edges>(map::compute_boundary_edges::create_info{pool});
//     systems.map_generator->add_generator<map::compute_plate_boundary_stress>(map::compute_plate_boundary_stress::create_info{pool});
//     systems.map_generator->add_generator<map::blur_plate_boundary_stress>(map::blur_plate_boundary_stress::create_info{pool});
//     systems.map_generator->add_generator<map::calculate_plate_boundary_distances>(map::calculate_plate_boundary_distances::create_info{pool});
//     systems.map_generator->add_generator<map::calculate_plate_root_distances>(map::calculate_plate_root_distances::create_info{pool});
//     //systems.map_generator->add_generator<map::modify_plate_datas>(map::modify_plate_datas::create_info{pool});
//     systems.map_generator->add_generator<map::calculate_vertex_elevation>(map::calculate_vertex_elevation::create_info{pool, 0.1f});
// //     systems.map_generator->add_generator<map::blur_tile_elevation>(map::blur_tile_elevation::create_info{
// //       pool,
// //       2,
// //       0.75f,
// //       1.1f
// //     });
//     
// //     systems.map_generator->add_generator<map::normalize_tile_elevation>(map::normalize_tile_elevation::create_info{pool});
//     systems.map_generator->add_generator<map::calculate_tile_distance>(map::calculate_tile_distance::create_info{
//       [] (const map::generator_context* context, const uint32_t &tile_index) -> bool {
//         const float height = context->tile_elevation[tile_index];
//         return height < 0.0f;
//       }, &context->water_distance, "calcutating water distances"});
//     systems.map_generator->add_generator<map::calculate_tile_distance>(map::calculate_tile_distance::create_info{
//       [] (const map::generator_context* context, const uint32_t &tile_index) -> bool {
//         const float height = context->tile_elevation[tile_index];
//         return height >= 0.0f;
//       }, &context->ground_distance, "calcutating ground distances"});
//     
//     systems.map_generator->add_generator<map::tile_postprocessing1>(map::tile_postprocessing1::create_info{pool});
//     systems.map_generator->add_generator<map::tile_postprocessing2>(map::tile_postprocessing2::create_info{pool});
// //     
// //     systems.map_generator->add_generator<map::calculate_tile_distance>(map::calculate_tile_distance::create_info{
// //       [] (const map::generator_context* context, const uint32_t &tile_index) -> bool {
// //         const float height = context->tile_elevation[tile_index];
// //         return height > 0.7f; // что является горой?
// //       }, &context->mountain_distance, "calcutating mountain distances"});
// //     
// //     systems.map_generator->add_generator<map::modify_tile_elevation>(map::modify_tile_elevation::create_info{
// //       pool,
// //       [] (const map::generator_context* context, const uint32_t &tile_index) -> float {
// //         const float height = context->tile_elevation[tile_index];
// //         if (height < 0.1f) return height - 0.05f;
// //         if (height > 0.7f) return 1.0f;
// //         return height + 0.05f;
// //       }, "calcutating mountain distances"});
// //     
// //     systems.map_generator->add_generator<map::blur_tile_elevation>(map::blur_tile_elevation::create_info{
// //       pool,
// //       1,
// //       0.6f,
// //       0.9f
// //     });
// //     
//     systems.map_generator->add_generator<map::normalize_tile_elevation>(map::normalize_tile_elevation::create_info{pool});
//     
//     
//     
// //     systems.map_generator->add_generator<map::connect_water_pools>();
// //     systems.map_generator->add_generator<map::generate_water_pools>();
//     systems.map_generator->add_generator<map::blur_tile_elevation>(map::blur_tile_elevation::create_info{
//       pool,
//       2,
//       0.75f,
//       1.1f
//     });
//     systems.map_generator->add_generator<map::normalize_tile_elevation>(map::normalize_tile_elevation::create_info{pool});
//     
//     systems.map_generator->add_generator<map::calculate_tile_distance>(map::calculate_tile_distance::create_info{
//       [] (const map::generator_context* context, const uint32_t &tile_index) -> bool {
//         const float height = context->tile_elevation[tile_index];
//         return height < 0.0f;
//       }, &context->water_distance, "calcutating water distances"});
//     systems.map_generator->add_generator<map::calculate_tile_distance>(map::calculate_tile_distance::create_info{
//       [] (const map::generator_context* context, const uint32_t &tile_index) -> bool {
//         const float height = context->tile_elevation[tile_index];
//         return height >= 0.0f;
//       }, &context->ground_distance, "calcutating ground distances"});
//     
//     systems.map_generator->add_generator<map::compute_tile_heat>(map::compute_tile_heat::create_info{pool});
//     systems.map_generator->add_generator<map::normalize_fractional_values>(map::normalize_fractional_values::create_info{
//       pool,
//       &context->tile_heat,
//       "normalizing tile heat"
//     });
//     
//     systems.map_generator->add_generator<map::generate_air_whorls>();
//     systems.map_generator->add_generator<map::calculate_vertex_air_current>(map::calculate_vertex_air_current::create_info{pool});
//     systems.map_generator->add_generator<map::calculate_air_current_outflows>(map::calculate_air_current_outflows::create_info{pool});
//     
// //     systems.map_generator->add_generator<map::initialize_circulating_moisture>(map::initialize_circulating_moisture::create_info{pool});
// //     systems.map_generator->add_generator<map::propagate_circulating_moisture>(map::propagate_circulating_moisture::create_info{pool});
// //     systems.map_generator->add_generator<map::calculate_wetness>(map::calculate_wetness::create_info{pool});
//     systems.map_generator->add_generator<map::compute_moisture>(map::compute_moisture::create_info{pool});
//     
//     systems.map_generator->add_generator<map::create_biomes>(map::create_biomes::create_info{pool});
//     systems.map_generator->add_generator<map::generate_provinces>(map::generate_provinces::create_info{pool});
//     systems.map_generator->add_generator<map::province_postprocessing>();
//     systems.map_generator->add_generator<map::calculating_province_neighbours>(map::calculating_province_neighbours::create_info{pool});
//     systems.map_generator->add_generator<map::generate_cultures>(map::generate_cultures::create_info{pool});
//     systems.map_generator->add_generator<map::generate_countries>(map::generate_countries::create_info{pool});
    systems.map_generator->add_generator<map::update_tile_data>(map::update_tile_data::create_info{pool});
    
    // что мне нужно для рек? высоты и горы, озера
    
    
//     systems.map_generator->add_generator<map::initialize_circulating_heat>(map::initialize_circulating_heat::create_info{pool});
//     systems.map_generator->add_generator<map::propagate_circulating_heat>(map::propagate_circulating_heat::create_info{pool});
//     systems.map_generator->add_generator<map::calculate_temperatures>(map::calculate_temperatures::create_info{pool});

//     systems.map_generator->add_generator<map::calculate_biomes>(map::calculate_biomes::create_info{pool});
  }

  void map_frustum_test(const map::container* map, const glm::mat4 &frustum, std::vector<uint32_t> &indices) {
    const utils::frustum fru = utils::compute_frustum(frustum);


  }
  
  #define OUTSIDE 0
  #define INSIDE 1
  #define INTERSECT 2
  
  uint32_t sphere_frustum_test(const glm::vec3 &pos, const float &radius, const utils::frustum &fru) {
    const glm::vec4 box_center = glm::vec4(pos, 1.0f);
    const glm::vec4 box_extents = glm::vec4(radius, radius, radius, 0.0f);
    uint32_t result = INSIDE; // Assume that the aabb will be inside the frustum
    for (uint32_t i = 0; i < 6; ++i) {
      const glm::vec4 frustumPlane = glm::vec4(fru.planes[i][0], fru.planes[i][1], fru.planes[i][2], 0.0f); //frustum.planes[i] * simd::vec4(1.0f, 1.0f, 1.0f, 0.0f);
      const float p3 = fru.planes[i][3];

      const float d = glm::dot(box_center,           frustumPlane);

      const float r = glm::dot(box_extents, glm::abs(frustumPlane));

      const float d_p_r = d + r;
      const float d_m_r = d - r;
      
      //frustumPlane.w
      if (d_p_r < -p3) {
        result = OUTSIDE;
        break;
      } else if (d_m_r < -p3) result = INTERSECT;
    }

    return result;
  }
  
  void map_triangle_test(dt::thread_pool* pool, const map::container* map, const utils::frustum &fru, const uint32_t &triangle_index, std::atomic<uint32_t> &counter) {
    const auto &tri = map->triangles[triangle_index];
    glm::vec3 center = map->points[tri.points[0]];
    for (uint32_t i = 1; i < 3; ++i) {
      center += map->points[tri.points[i]];
    }
    center /= float(3);
    
    //std::cout << "Thread id " << pool->thread_index(std::this_thread::get_id()) << "\n";
    
    const float dist = glm::distance(map->points[tri.points[0]], center);
    ++counter;
    const uint32_t ret = sphere_frustum_test(center, dist, fru);
    if (ret == INTERSECT) {
      for (uint32_t i = 0; i < 4; ++i) {
        const uint32_t next_triangle_index = tri.next_level[i];
        if (tri.current_level < 3) {
          pool->submitbase([pool, map, &fru, next_triangle_index, &counter] () {
            map_triangle_test(pool, map, fru, next_triangle_index, counter);
          });
        } else {
          pool->submitbase([map, &fru, next_triangle_index, &counter] () {
            map_triangle_test(map, fru, next_triangle_index, counter);
          });
        }
      }
    } else if (ret == INSIDE) {
      for (uint32_t i = 0; i < 4; ++i) {
        const uint32_t next_triangle_index = tri.next_level[i];
        pool->submitbase([map, next_triangle_index, &counter] () {
          map_triangle_add(map, next_triangle_index, counter);
        });
      }
    }
  }
  
  void map_triangle_test(const map::container* map, const utils::frustum &fru, const uint32_t &triangle_index, std::atomic<uint32_t> &counter) {
    const auto &tri = map->triangles[triangle_index];
    
    glm::vec3 center = map->points[tri.points[0]];
    for (uint32_t i = 1; i < 3; ++i) {
      center += map->points[tri.points[i]];
    }
    center /= float(3);
    
    const float dist = glm::distance(map->points[tri.points[0]], center);
    ++counter;
    const uint32_t ret = sphere_frustum_test(center, dist, fru);
    if (ret == INTERSECT) {
      if (tri.current_level == map->detail_level()) {
        for (uint32_t i = 0; i < 4; ++i) {
          global::get<render::tile_render>()->add(tri.next_level[i]);
        }
        
        return;
      }
      
      for (uint32_t i = 0; i < 4; ++i) {
        const uint32_t next_triangle_index = tri.next_level[i];
        ASSERT(next_triangle_index != UINT32_MAX);
        map_triangle_test(map, fru, next_triangle_index, counter);
      }
    } else if (ret == INSIDE) {
      if (tri.current_level == map->detail_level()) {
        for (uint32_t i = 0; i < 4; ++i) {
          global::get<render::tile_render>()->add(tri.next_level[i]);
        }
        
        return;
      }
      
      for (uint32_t i = 0; i < 4; ++i) {
        const uint32_t next_triangle_index = tri.next_level[i];
        ASSERT(next_triangle_index != UINT32_MAX);
        map_triangle_add(map, next_triangle_index, counter);
      }
    }
  }
  
  void map_triangle_add(const map::container* map, const uint32_t &triangle_index, std::atomic<uint32_t> &counter) {
    const auto &tri = map->triangles[triangle_index];
    
    ++counter;
    
    if (tri.current_level == map->detail_level()) {
      for (uint32_t i = 0; i < 4; ++i) {
        global::get<render::tile_render>()->add(tri.next_level[i]);
      }
      
      return;
    }
    
    for (uint32_t i = 0; i < 4; ++i) {
      const uint32_t next_triangle_index = tri.next_level[i];
      ASSERT(next_triangle_index != UINT32_MAX);
      map_triangle_add(map, next_triangle_index, counter);
    }
  }
  
  void map_triangle_add2(const map::container* map, const uint32_t &triangle_index, std::mutex &mutex, std::unordered_set<uint32_t> &unique_tiles, std::vector<uint32_t> &tiles_array) {
    const auto &tri = map->triangles[triangle_index];
    
    if (tri.current_level == detail_level) {
      for (uint32_t i = 0; i < 4; ++i) {
        const uint32_t tile_index = tri.next_level[i];
        {
          std::unique_lock<std::mutex> lock(mutex);
          auto itr = unique_tiles.find(tile_index);
          if (itr != unique_tiles.end()) continue;
          unique_tiles.insert(tile_index);
        }
        
        tiles_array.push_back(tile_index);
      }
      
      return;
    }
    
    for (uint32_t i = 0; i < 4; ++i) {
      const uint32_t next_triangle_index = tri.next_level[i];
      ASSERT(next_triangle_index != UINT32_MAX);
      map_triangle_add2(map, next_triangle_index, mutex, unique_tiles, tiles_array);
    }
  }
  
  void map_triangle_test2(dt::thread_pool* pool, const map::container* map, const utils::frustum &fru, const uint32_t &triangle_index, std::atomic<uint32_t> &counter) {
    // без рекурсии
  }

  void sync(utils::frame_time &frame_time, const size_t &time) {
    //auto start = std::chrono::steady_clock::now();
    frame_time.end();
    
    {
//       utils::time_log log("task waiting");
      global::get<render::task_start>()->wait();
      global::get<systems::render>()->clear();
    }

    size_t mcs = 0;
    while (mcs < time) {
      std::this_thread::sleep_for(std::chrono::microseconds(1));
      auto end = std::chrono::steady_clock::now() - frame_time.start_point;
      mcs = std::chrono::duration_cast<std::chrono::microseconds>(end).count();
    }
  }
  
  void check_tile(const map::container* map, const uint32_t &tile_index) {
    PRINT_VAR("tile index", tile_index)
    const uint32_t center_index = map->tiles[tile_index].index;
    PRINT_VEC3("center ", map->points[center_index])
    for (uint32_t i = 0; i < 6; ++i) {
      const uint32_t point_index = map->tiles[tile_index].neighbours[i].points[0];
      PRINT_VEC3("point "+std::to_string(i), map->points[point_index])
      //PRINT_VAR("center  index", center_index)
//       for (uint32_t j = 0; j < 2; ++j) {
//         const uint32_t point_index = map->tiles[tile_index].neighbours[i].points[j];
//         PRINT_VEC3("point "+std::to_string(j), map->points[point_index])
//         //PRINT_VAR("point "+std::to_string(j)+" index", point_index)
//       }
      
      if (map->tiles[tile_index].is_pentagon() && i == 4) break;
    }
    
    PRINT("\n")
  }

  void callback(int error, const char* description) {
    std::cout << "Error code: " << error << std::endl;
    std::cout << "Error: " << description << std::endl;
  }

  void scrollCallback(GLFWwindow*, double xoffset, double yoffset) {
    if (!global::get<render::window>()->flags.focused()) return;
//     if (!global::get<input::data>()->interface_focus) return;
//     std::cout << "scroll " << yoffset << "\n";

    (void)xoffset;
    global::get<input::data>()->mouse_wheel += float(yoffset);
  }

  void charCallback(GLFWwindow*, unsigned int c) {
    if (!global::get<render::window>()->flags.focused()) return;
    if (!global::get<input::data>()->interface_focus) return;

    global::get<input::data>()->text[global::get<input::data>()->current_text] = c;
    ++global::get<input::data>()->current_text;
  //   ImGuiIO& io = ImGui::GetIO();
  //   if (c > 0 && c < 0x10000) io.AddInputCharacter((unsigned short)c);
  }

  void mouseButtonCallback(GLFWwindow*, int button, int action, int mods) {
    if (!global::get<render::window>()->flags.focused()) return;

    (void)mods;
  //   if (action == GLFW_PRESS) mousePressed[button] = true;
  //   if (action == GLFW_RELEASE) mousePressed[button] = false;

    //global::data()->keys[button] = !(action == GLFW_RELEASE);
    const auto old = global::get<input::data>()->key_events.container[button].event;
    global::get<input::data>()->key_events.container[button].event = static_cast<input::type>(action);
    auto data = global::get<input::data>()->key_events.container[button].data;
    if (data != nullptr && old != static_cast<input::type>(action)) data->time = 0;
  }

  void keyCallback(GLFWwindow*, int key, int scancode, int action, int mods) {
    (void)mods;

    if (!global::get<render::window>()->flags.focused()) return;
    (void)scancode;

    //global::data()->keys[key] = !(action == GLFW_RELEASE);
    const auto old = global::get<input::data>()->key_events.container[key].event;
    global::get<input::data>()->key_events.container[key].event = static_cast<input::type>(action);
    auto data = global::get<input::data>()->key_events.container[key].data;
    if (data != nullptr && old != static_cast<input::type>(action)) data->time = 0;
  }

  void window_resize_callback(GLFWwindow*, int w, int h) {
    global::get<render::window>()->recreate(w, h);
    //global::get<GBufferStage>()->recreate(w, h);
    global::get<systems::render>()->recreate(w, h);
    global::get<interface::context>()->remake_font_atlas(w, h);
  //   std::cout << "window_resize_callback width " << w << " height " << h << '\n';
  }

  void iconifyCallback(GLFWwindow*, int iconified) {
    global::get<render::window>()->flags.set_iconified(iconified);
  }

  void focusCallback(GLFWwindow*, int focused) {
    global::get<render::window>()->flags.set_focused(focused);
  }

  const char* getClipboard(void* user_data) {
    return glfwGetClipboardString(((render::window*)user_data)->handle);
  }

  void setClipboard(void* user_data, const char* text) {
    glfwSetClipboardString(((render::window*)user_data)->handle, text);
  }
}
