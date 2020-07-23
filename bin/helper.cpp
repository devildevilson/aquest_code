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
      //sizeof(systems::generator<map::generator_context>) + 
      sizeof(core::map) + 
      sizeof(interface::context) +
      sizeof(map::generator::container)
    ),
    graphics_container(nullptr),
    map(nullptr),
    //map_generator(nullptr),
    context(nullptr),
    map_container(nullptr)
  {}

  system_container_t::~system_container_t() {
    RELEASE_CONTAINER_DATA(map)
    //RELEASE_CONTAINER_DATA(map_generator)
    RELEASE_CONTAINER_DATA(graphics_container)
    RELEASE_CONTAINER_DATA(context)
    RELEASE_CONTAINER_DATA(map_container)
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
    
//     std::cout << array << "\n";
    
    array[dirname+1] = '\0'; // путь до папки
    
//     std::cout << array << "\n";
    
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
    const utils::id map_move = utils::id::get("map_move");
    input::set_key(GLFW_MOUSE_BUTTON_RIGHT, map_move);
    
    const utils::id plate_render_mode = utils::id::get("plate_render_mode");
    const utils::id elevation_render_mode = utils::id::get("elevation_render_mode");
    const utils::id temperature_render_mode = utils::id::get("temperature_render_mode");
    const utils::id biome_render_mode = utils::id::get("biome_render_mode");
    const utils::id cultures_render_mode = utils::id::get("cultures_render_mode");
    const utils::id provinces_render_mode = utils::id::get("provinces_render_mode");
    const utils::id countries_render_mode = utils::id::get("countries_render_mode");
    input::set_key(GLFW_KEY_F1, plate_render_mode);
    input::set_key(GLFW_KEY_F2, elevation_render_mode);
    input::set_key(GLFW_KEY_F3, temperature_render_mode);
    input::set_key(GLFW_KEY_F4, biome_render_mode);
    input::set_key(GLFW_KEY_F5, cultures_render_mode);
    input::set_key(GLFW_KEY_F6, provinces_render_mode);
    input::set_key(GLFW_KEY_F7, countries_render_mode);
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
  
  void key_input(const size_t &time) {
    (void)time;
    static const utils::id plate_render_mode = utils::id::get("plate_render_mode");
    static const utils::id elevation_render_mode = utils::id::get("elevation_render_mode");
    static const utils::id temperature_render_mode = utils::id::get("temperature_render_mode");
    static const utils::id biome_render_mode = utils::id::get("biome_render_mode");
    static const utils::id cultures_render_mode = utils::id::get("cultures_render_mode");
    static const utils::id provinces_render_mode = utils::id::get("provinces_render_mode");
    static const utils::id countries_render_mode = utils::id::get("countries_render_mode");
    
    auto map = global::get<core::map>();
    auto container = global::get<map::generator::container>();
    
    {
      size_t mem = 0;
      auto change = input::next_input_event(mem, 1);
      while (change.id.valid()) {
        //PRINT_VAR("change id", change.id.name())
        if (change.id == plate_render_mode && change.event != input::release) {
          rendering_mode(container, map, map::debug::properties::tile::plate_index, 1, 0);
        }
        
        if (change.id == elevation_render_mode && change.event != input::release) {
          rendering_mode(container, map, map::debug::properties::tile::elevation, 2, 2);
        }
        
        if (change.id == temperature_render_mode && change.event != input::release) {
          rendering_mode(container, map, map::debug::properties::tile::heat, 2, 2);
        }
        
        if (change.id == biome_render_mode && change.event != input::release) {
          rendering_mode(container, map, map::debug::properties::tile::biome, 0, 0);
        }
        
        if (change.id == cultures_render_mode && change.event != input::release) {
          rendering_mode(container, map, map::debug::properties::tile::culture_id, 1, 1);
        }
        
        if (change.id == provinces_render_mode && change.event != input::release) {
          rendering_mode(container, map, map::debug::properties::tile::province_index, 1, 1);
        }
        
        if (change.id == countries_render_mode && change.event != input::release) {
          rendering_mode(container, map, map::debug::properties::tile::country_index, 1, 1);
        }
        
        change = input::next_input_event(mem, 1);
      }
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
  
  bool long_key(const int &key, const size_t &time) {
    return input::timed_check_key(key, input::state_press, 0, SIZE_MAX, time) || input::timed_check_key(key, input::state_long_press, ONE_SECOND / 2, ONE_SECOND / 15, time);
  }
  
  void next_nk_frame(const size_t &time) {
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
      
//       const int32_t test_in = glfwGetKey(window->handle, GLFW_KEY_DELETE);
//       if (test_in == GLFW_RELEASE) std::cout << "Release" << "\n";
//       else if (test_in == GLFW_PRESS) std::cout << "Press" << "\n";
//       else if (test_in == GLFW_REPEAT) std::cout << "Repeat" << "\n";
//       else std::cout << test_in << "\n";

      const uint32_t rel_event = static_cast<uint32_t>(input::type::release);
      //const bool* keys = Global::data()->keys;
      const auto keys = global::get<input::data>()->key_events.container;
      //nk_input_key(ctx, NK_KEY_DEL, keys[GLFW_KEY_DELETE].event != rel_event);
      //nk_input_key(ctx, NK_KEY_DEL, input::check_key(GLFW_KEY_DELETE, input::state_click | input::state_long_press));
      //nk_input_key(ctx, NK_KEY_DEL, glfwGetKey(window->handle, GLFW_KEY_DELETE) == GLFW_PRESS);
      //nk_input_key(ctx, NK_KEY_DEL, input::timed_check_key(GLFW_KEY_DELETE, input::state_press, 0, SIZE_MAX, time) || input::timed_check_key(GLFW_KEY_DELETE, input::state_long_press, ONE_SECOND / 2, ONE_SECOND / 10, time));
      nk_input_key(ctx, NK_KEY_DEL, long_key(GLFW_KEY_DELETE, time));
      nk_input_key(ctx, NK_KEY_ENTER, input::check_key(GLFW_KEY_ENTER, input::state_click | input::state_double_click | input::state_long_click));
      nk_input_key(ctx, NK_KEY_TAB, long_key(GLFW_KEY_TAB, time)); //keys[GLFW_KEY_TAB].event != rel_event
      nk_input_key(ctx, NK_KEY_BACKSPACE, long_key(GLFW_KEY_BACKSPACE, time)); // keys[GLFW_KEY_BACKSPACE].event != rel_event
      nk_input_key(ctx, NK_KEY_UP, long_key(GLFW_KEY_UP, time)); //keys[GLFW_KEY_UP].event != rel_event
      nk_input_key(ctx, NK_KEY_DOWN, long_key(GLFW_KEY_DOWN, time)); //keys[GLFW_KEY_DOWN].event != rel_event
      nk_input_key(ctx, NK_KEY_SHIFT, input::check_key(GLFW_KEY_LEFT_SHIFT , input::state_press | input::state_double_press | input::state_long_press) ||  //keys[GLFW_KEY_LEFT_SHIFT].event != rel_event ||
                                      input::check_key(GLFW_KEY_RIGHT_SHIFT, input::state_press | input::state_double_press | input::state_long_press));  //keys[GLFW_KEY_RIGHT_SHIFT].event != rel_event
      
      const bool control_press = input::check_key(GLFW_KEY_LEFT_SHIFT , input::state_press | input::state_double_press | input::state_long_press) || 
                                 input::check_key(GLFW_KEY_RIGHT_SHIFT, input::state_press | input::state_double_press | input::state_long_press);
      if (control_press) {
        nk_input_key(ctx, NK_KEY_COPY, input::check_key(GLFW_KEY_C, input::state_click | input::state_double_click | input::state_long_click)); //keys[GLFW_KEY_C].event != rel_event
        nk_input_key(ctx, NK_KEY_PASTE, input::check_key(GLFW_KEY_V, input::state_click | input::state_double_click | input::state_long_click)); //keys[GLFW_KEY_V].event != rel_event
        nk_input_key(ctx, NK_KEY_CUT, input::check_key(GLFW_KEY_X, input::state_click | input::state_double_click | input::state_long_click)); //keys[GLFW_KEY_X].event != rel_event
        nk_input_key(ctx, NK_KEY_TEXT_UNDO, input::check_key(GLFW_KEY_Z, input::state_click | input::state_double_click | input::state_long_click)); //keys[GLFW_KEY_Z].event != rel_event
        nk_input_key(ctx, NK_KEY_TEXT_REDO, input::check_key(GLFW_KEY_R, input::state_click | input::state_double_click | input::state_long_click)); //keys[GLFW_KEY_R].event != rel_event
        nk_input_key(ctx, NK_KEY_TEXT_WORD_LEFT, long_key(GLFW_KEY_LEFT, time)); //keys[GLFW_KEY_LEFT].event != rel_event
        nk_input_key(ctx, NK_KEY_TEXT_WORD_RIGHT, long_key(GLFW_KEY_RIGHT, time)); //keys[GLFW_KEY_RIGHT].event != rel_event
        nk_input_key(ctx, NK_KEY_TEXT_SELECT_ALL, input::check_key(GLFW_KEY_A, input::state_click | input::state_double_click | input::state_long_click)); //keys[GLFW_KEY_A].event != rel_event

        nk_input_key(ctx, NK_KEY_SCROLL_START, input::check_key(GLFW_KEY_PAGE_DOWN, input::state_click | input::state_double_click | input::state_long_click)); //keys[GLFW_KEY_PAGE_DOWN].event != rel_event
        nk_input_key(ctx, NK_KEY_SCROLL_END, input::check_key(GLFW_KEY_PAGE_UP, input::state_click | input::state_double_click | input::state_long_click)); //keys[GLFW_KEY_PAGE_UP].event != rel_event
        nk_input_key(ctx, NK_KEY_TEXT_START, input::check_key(GLFW_KEY_HOME, input::state_click | input::state_double_click | input::state_long_click)); //keys[GLFW_KEY_HOME].event != rel_event
        nk_input_key(ctx, NK_KEY_TEXT_END, input::check_key(GLFW_KEY_END, input::state_click | input::state_double_click | input::state_long_click)); //keys[GLFW_KEY_END].event != rel_event
      } else {
//         nk_input_key(ctx, NK_KEY_LEFT, keys[GLFW_KEY_LEFT].event != rel_event);
//         nk_input_key(ctx, NK_KEY_RIGHT, keys[GLFW_KEY_RIGHT].event != rel_event);
        nk_input_key(ctx, NK_KEY_LEFT, long_key(GLFW_KEY_LEFT, time));
        nk_input_key(ctx, NK_KEY_RIGHT, long_key(GLFW_KEY_RIGHT, time));
        nk_input_key(ctx, NK_KEY_COPY, 0);
        nk_input_key(ctx, NK_KEY_PASTE, 0);
        nk_input_key(ctx, NK_KEY_CUT, 0);
  //       nk_input_key(ctx, NK_KEY_SHIFT, 0);

        nk_input_key(ctx, NK_KEY_SCROLL_DOWN, long_key(GLFW_KEY_PAGE_DOWN, time)); //keys[GLFW_KEY_PAGE_DOWN].event != rel_event
        nk_input_key(ctx, NK_KEY_SCROLL_UP, long_key(GLFW_KEY_PAGE_UP, time)); //keys[GLFW_KEY_PAGE_UP].event != rel_event
        nk_input_key(ctx, NK_KEY_TEXT_LINE_START, input::check_key(GLFW_KEY_HOME, input::state_click | input::state_double_click | input::state_long_click)); //keys[GLFW_KEY_HOME].event != rel_event
        nk_input_key(ctx, NK_KEY_TEXT_LINE_END, input::check_key(GLFW_KEY_END, input::state_click | input::state_double_click | input::state_long_click)); //keys[GLFW_KEY_END].event != rel_event
      }

      glfwGetCursorPos(window->handle, &x, &y);
      if (window->flags.focused()) {
        //  && global::get<input::data>()->interface_focus
        // Mouse position in screen coordinates (set to -1,-1 if no mouse / on another screen, etc.)
        nk_input_motion(ctx, (int)x, (int)y);
      } else {
        nk_input_motion(ctx, -1, -1);
      }

      // тоже заменить, также наклир дает возможность обработать даблклик, как это сделать верно?
//       nk_input_button(ctx, NK_BUTTON_LEFT, (int)x, (int)y, input::check_key(GLFW_MOUSE_BUTTON_LEFT, input::state_click | input::state_long_click));
//       nk_input_button(ctx, NK_BUTTON_MIDDLE,(int)x, (int)y, input::check_key(GLFW_MOUSE_BUTTON_MIDDLE, input::state_click | input::state_long_click));
//       nk_input_button(ctx, NK_BUTTON_RIGHT, (int)x, (int)y, input::check_key(GLFW_MOUSE_BUTTON_RIGHT, input::state_click | input::state_long_click));
      nk_input_button(ctx, NK_BUTTON_LEFT, (int)x, (int)y, keys[GLFW_MOUSE_BUTTON_LEFT].event != rel_event);
      nk_input_button(ctx, NK_BUTTON_MIDDLE,(int)x, (int)y, keys[GLFW_MOUSE_BUTTON_MIDDLE].event != rel_event);
      nk_input_button(ctx, NK_BUTTON_RIGHT, (int)x, (int)y, keys[GLFW_MOUSE_BUTTON_RIGHT].event != rel_event);

//       bool doubleClick = false;
//       if (keys[GLFW_MOUSE_BUTTON_LEFT].event != rel_event) {
//         auto p = std::chrono::steady_clock::now();
//         auto diff = p - global::get<input::data>()->double_click_time_point;
//         auto mcs = std::chrono::duration_cast<std::chrono::microseconds>(diff).count();
//         doubleClick = mcs < DOUBLE_CLICK_TIME;
//         if (!doubleClick) {
//           global::get<input::data>()->double_click_time_point = p;
//         }
//       }

      nk_input_button(ctx, NK_BUTTON_DOUBLE, int(global::get<input::data>()->click_pos.x), int(global::get<input::data>()->click_pos.y), input::check_key(GLFW_MOUSE_BUTTON_LEFT, input::state_double_click));
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
      sizeof(render::tile_borders_optimizer) +

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
                 
    auto opt     = system->add_stage<render::tile_optimizer>(render::tile_optimizer::create_info{device});
    auto opt2    = system->add_stage<render::tile_borders_optimizer>(render::tile_borders_optimizer::create_info{device});
                 
                   system->add_stage<render::render_pass_begin>();
    auto tiles   = system->add_stage<render::tile_render>(render::tile_render::create_info{device, opt});
    auto borders = system->add_stage<render::tile_border_render>(render::tile_border_render::create_info{device, opt2});
                   system->add_stage<render::interface_stage>(render::interface_stage::create_info{device});
                   system->add_stage<render::render_pass_end>();

                   system->add_stage<render::task_end>();
    auto start   = system->add_stage<render::task_start>(device);
                   system->add_stage<render::window_present>(render::window_present::create_info{window});

    global::get(start);
    global::get(tiles);
    global::get(buffers);
    global::get(opt2);
    (void)borders;
    
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
    
    const std::vector<map::generator::data_type> tiles_types = {
      map::generator::data_type::uint_t,   //       plate_index,
      map::generator::data_type::uint_t,   //       edge_index,
      map::generator::data_type::float_t,   //       edge_dist,
      map::generator::data_type::uint_t,   //       mountain_index,
      map::generator::data_type::float_t,   //       mountain_dist,
      map::generator::data_type::uint_t,   //       ocean_index,
      map::generator::data_type::float_t,   //       ocean_dist,
      map::generator::data_type::uint_t,   //       coastline_index,
      map::generator::data_type::float_t,   //       coastline_dist,
      map::generator::data_type::float_t,   //       elevation,
      map::generator::data_type::float_t,   //       heat,
      map::generator::data_type::float_t,   //       moisture,
      map::generator::data_type::uint_t,   //       biome,
      map::generator::data_type::uint_t,   //       province_index,
      map::generator::data_type::uint_t,   //       culture_id,
      map::generator::data_type::uint_t,   //       country_index
    };
    
    const std::vector<std::pair<utils::id, std::vector<map::generator::data_type>>> entities_types = {
      std::make_pair(utils::id::get("plate"), std::vector<map::generator::data_type>{
        map::generator::data_type::float_t,  // drift_axis,
        map::generator::data_type::float_t,  // drift_axis1,
        map::generator::data_type::float_t,  // drift_axis2,
        map::generator::data_type::float_t,  // drift_rate,
        map::generator::data_type::float_t,  // spin_axis,
        map::generator::data_type::float_t,  // spin_axis1,
        map::generator::data_type::float_t,  // spin_axis2,
        map::generator::data_type::float_t,  // spin_rate,
        map::generator::data_type::float_t,  // base_elevation,
        map::generator::data_type::uint_t    // oceanic
      }),
      std::make_pair(utils::id::get("edge"), std::vector<map::generator::data_type>{
        map::generator::data_type::float_t,  // plate0_movement,
        map::generator::data_type::float_t,  // plate0_movement1,
        map::generator::data_type::float_t,  // plate0_movement2,
        map::generator::data_type::float_t,  // plate1_movement,
        map::generator::data_type::float_t,  // plate1_movement1,
        map::generator::data_type::float_t,  // plate1_movement2,
      }),
      std::make_pair(utils::id::get("province"), std::vector<map::generator::data_type>{
        map::generator::data_type::uint_t,  // country_index,
      }),
      std::make_pair(utils::id::get("culture"), std::vector<map::generator::data_type>{}),
      std::make_pair(utils::id::get("country"), std::vector<map::generator::data_type>{}),
    };
    
    systems.map_container = systems.container.create<map::generator::container>(map::generator::container::create_info{tiles_types, entities_types});
    global::get(systems.map_container);
  }
  
//   void create_map_generator(system_container_t &systems, dt::thread_pool* pool, map::generator_context* context) {
//     const size_t map_generator_size = 
//       sizeof(map::beginner) +
//       sizeof(map::plates_generator) + 
//       sizeof(map::plate_datas_generator) + 
//       sizeof(map::compute_boundary_edges) + 
//       sizeof(map::compute_plate_boundary_stress) + 
//       sizeof(map::blur_plate_boundary_stress) + 
//       sizeof(map::calculate_plate_boundary_distances) + 
//       sizeof(map::calculate_plate_root_distances) + 
//       sizeof(map::calculate_vertex_elevation) + 
//       sizeof(map::generate_air_whorls) + 
//       sizeof(map::calculate_vertex_air_current) + 
//       sizeof(map::calculate_air_current_outflows) + 
//       sizeof(map::initialize_circulating_heat) + 
//       sizeof(map::propagate_circulating_heat) + 
//       sizeof(map::calculate_temperatures) + 
//       sizeof(map::initialize_circulating_moisture) + 
//       sizeof(map::propagate_circulating_moisture) + 
//       sizeof(map::calculate_wetness) + 
//       sizeof(map::calculate_biomes) + 
//       sizeof(map::calculate_tile_distance) * 3 + 
//       sizeof(map::modify_tile_elevation) + 
//       sizeof(map::compute_tile_heat) + 
//       sizeof(map::normalize_fractional_values) + 
//       sizeof(map::initialize_circulating_moisture) + 
//       sizeof(map::propagate_circulating_moisture) + 
//       sizeof(map::calculate_wetness) + 
//       sizeof(map::create_biomes) + 
//       sizeof(map::generate_provinces) + 
//       sizeof(map::generate_cultures);
//       
//     systems.map_generator = systems.container.create<systems::generator<map::generator_context>>(map_generator_size);
//     global::get(systems.map_generator);
//     
//     systems.map_generator->add_generator<map::beginner>(map::beginner::create_info{pool});
// //     systems.map_generator->add_generator<map::plates_generator>(map::plates_generator::create_info{pool});
// //     systems.map_generator->add_generator<map::plate_datas_generator>(map::plate_datas_generator::create_info{pool});
// //     systems.map_generator->add_generator<map::compute_boundary_edges>(map::compute_boundary_edges::create_info{pool});
// //     systems.map_generator->add_generator<map::compute_plate_boundary_stress>(map::compute_plate_boundary_stress::create_info{pool});
// //     systems.map_generator->add_generator<map::blur_plate_boundary_stress>(map::blur_plate_boundary_stress::create_info{pool});
// //     systems.map_generator->add_generator<map::calculate_plate_boundary_distances>(map::calculate_plate_boundary_distances::create_info{pool});
// //     systems.map_generator->add_generator<map::calculate_plate_root_distances>(map::calculate_plate_root_distances::create_info{pool});
// //     //systems.map_generator->add_generator<map::modify_plate_datas>(map::modify_plate_datas::create_info{pool});
// //     systems.map_generator->add_generator<map::calculate_vertex_elevation>(map::calculate_vertex_elevation::create_info{pool, 0.1f});
// // //     systems.map_generator->add_generator<map::blur_tile_elevation>(map::blur_tile_elevation::create_info{
// // //       pool,
// // //       2,
// // //       0.75f,
// // //       1.1f
// // //     });
// //     
// // //     systems.map_generator->add_generator<map::normalize_tile_elevation>(map::normalize_tile_elevation::create_info{pool});
// //     systems.map_generator->add_generator<map::calculate_tile_distance>(map::calculate_tile_distance::create_info{
// //       [] (const map::generator_context* context, const uint32_t &tile_index) -> bool {
// //         const float height = context->tile_elevation[tile_index];
// //         return height < 0.0f;
// //       }, &context->water_distance, "calcutating water distances"});
// //     systems.map_generator->add_generator<map::calculate_tile_distance>(map::calculate_tile_distance::create_info{
// //       [] (const map::generator_context* context, const uint32_t &tile_index) -> bool {
// //         const float height = context->tile_elevation[tile_index];
// //         return height >= 0.0f;
// //       }, &context->ground_distance, "calcutating ground distances"});
// //     
// //     systems.map_generator->add_generator<map::tile_postprocessing1>(map::tile_postprocessing1::create_info{pool});
// //     systems.map_generator->add_generator<map::tile_postprocessing2>(map::tile_postprocessing2::create_info{pool});
// // //     
// // //     systems.map_generator->add_generator<map::calculate_tile_distance>(map::calculate_tile_distance::create_info{
// // //       [] (const map::generator_context* context, const uint32_t &tile_index) -> bool {
// // //         const float height = context->tile_elevation[tile_index];
// // //         return height > 0.7f; // что является горой?
// // //       }, &context->mountain_distance, "calcutating mountain distances"});
// // //     
// // //     systems.map_generator->add_generator<map::modify_tile_elevation>(map::modify_tile_elevation::create_info{
// // //       pool,
// // //       [] (const map::generator_context* context, const uint32_t &tile_index) -> float {
// // //         const float height = context->tile_elevation[tile_index];
// // //         if (height < 0.1f) return height - 0.05f;
// // //         if (height > 0.7f) return 1.0f;
// // //         return height + 0.05f;
// // //       }, "calcutating mountain distances"});
// // //     
// // //     systems.map_generator->add_generator<map::blur_tile_elevation>(map::blur_tile_elevation::create_info{
// // //       pool,
// // //       1,
// // //       0.6f,
// // //       0.9f
// // //     });
// // //     
// //     systems.map_generator->add_generator<map::normalize_tile_elevation>(map::normalize_tile_elevation::create_info{pool});
// //     
// //     
// //     
// // //     systems.map_generator->add_generator<map::connect_water_pools>();
// // //     systems.map_generator->add_generator<map::generate_water_pools>();
// //     systems.map_generator->add_generator<map::blur_tile_elevation>(map::blur_tile_elevation::create_info{
// //       pool,
// //       2,
// //       0.75f,
// //       1.1f
// //     });
// //     systems.map_generator->add_generator<map::normalize_tile_elevation>(map::normalize_tile_elevation::create_info{pool});
// //     
// //     systems.map_generator->add_generator<map::calculate_tile_distance>(map::calculate_tile_distance::create_info{
// //       [] (const map::generator_context* context, const uint32_t &tile_index) -> bool {
// //         const float height = context->tile_elevation[tile_index];
// //         return height < 0.0f;
// //       }, &context->water_distance, "calcutating water distances"});
// //     systems.map_generator->add_generator<map::calculate_tile_distance>(map::calculate_tile_distance::create_info{
// //       [] (const map::generator_context* context, const uint32_t &tile_index) -> bool {
// //         const float height = context->tile_elevation[tile_index];
// //         return height >= 0.0f;
// //       }, &context->ground_distance, "calcutating ground distances"});
// //     
// //     systems.map_generator->add_generator<map::compute_tile_heat>(map::compute_tile_heat::create_info{pool});
// //     systems.map_generator->add_generator<map::normalize_fractional_values>(map::normalize_fractional_values::create_info{
// //       pool,
// //       &context->tile_heat,
// //       "normalizing tile heat"
// //     });
// //     
// //     systems.map_generator->add_generator<map::generate_air_whorls>();
// //     systems.map_generator->add_generator<map::calculate_vertex_air_current>(map::calculate_vertex_air_current::create_info{pool});
// //     systems.map_generator->add_generator<map::calculate_air_current_outflows>(map::calculate_air_current_outflows::create_info{pool});
// //     
// // //     systems.map_generator->add_generator<map::initialize_circulating_moisture>(map::initialize_circulating_moisture::create_info{pool});
// // //     systems.map_generator->add_generator<map::propagate_circulating_moisture>(map::propagate_circulating_moisture::create_info{pool});
// // //     systems.map_generator->add_generator<map::calculate_wetness>(map::calculate_wetness::create_info{pool});
// //     systems.map_generator->add_generator<map::compute_moisture>(map::compute_moisture::create_info{pool});
// //     
// //     systems.map_generator->add_generator<map::create_biomes>(map::create_biomes::create_info{pool});
// //     systems.map_generator->add_generator<map::generate_provinces>(map::generate_provinces::create_info{pool});
// //     systems.map_generator->add_generator<map::province_postprocessing>();
// //     systems.map_generator->add_generator<map::calculating_province_neighbours>(map::calculating_province_neighbours::create_info{pool});
// //     systems.map_generator->add_generator<map::generate_cultures>(map::generate_cultures::create_info{pool});
// //     systems.map_generator->add_generator<map::generate_countries>(map::generate_countries::create_info{pool});
//     systems.map_generator->add_generator<map::update_tile_data>(map::update_tile_data::create_info{pool});
//     
//     // что мне нужно для рек? высоты и горы, озера
//     
//     
// //     systems.map_generator->add_generator<map::initialize_circulating_heat>(map::initialize_circulating_heat::create_info{pool});
// //     systems.map_generator->add_generator<map::propagate_circulating_heat>(map::propagate_circulating_heat::create_info{pool});
// //     systems.map_generator->add_generator<map::calculate_temperatures>(map::calculate_temperatures::create_info{pool});
// 
// //     systems.map_generator->add_generator<map::calculate_biomes>(map::calculate_biomes::create_info{pool});
//   }
  
//   std::vector<systems::generator<map::generator_context>*> create_map_generators(system_container_t &systems, dt::thread_pool* pool, map::generator_context* context) {
//     std::vector<systems::generator<map::generator_context>*> generators(3, nullptr);
//     
//     const size_t map_generator_size = 
//       sizeof(map::beginner) +
//       sizeof(map::plates_generator) + 
//       sizeof(map::plate_datas_generator) + 
//       sizeof(map::compute_boundary_edges) + 
//       sizeof(map::compute_plate_boundary_stress) + 
//       sizeof(map::blur_plate_boundary_stress) + 
//       sizeof(map::calculate_plate_boundary_distances) + 
//       sizeof(map::calculate_plate_root_distances) + 
//       sizeof(map::calculate_vertex_elevation) + 
//       sizeof(map::generate_air_whorls) + 
//       sizeof(map::calculate_vertex_air_current) + 
//       sizeof(map::calculate_air_current_outflows) + 
//       sizeof(map::initialize_circulating_heat) + 
//       sizeof(map::propagate_circulating_heat) + 
//       sizeof(map::calculate_temperatures) + 
//       sizeof(map::initialize_circulating_moisture) + 
//       sizeof(map::propagate_circulating_moisture) + 
//       sizeof(map::calculate_wetness) + 
//       sizeof(map::calculate_biomes) + 
//       sizeof(map::calculate_tile_distance) * 3 + 
//       sizeof(map::modify_tile_elevation) + 
//       sizeof(map::compute_tile_heat) + 
//       sizeof(map::normalize_fractional_values) + 
//       sizeof(map::initialize_circulating_moisture) + 
//       sizeof(map::propagate_circulating_moisture) + 
//       sizeof(map::calculate_wetness) + 
//       sizeof(map::create_biomes) + 
//       sizeof(map::generate_provinces) + 
//       sizeof(map::generate_cultures);
//       
//     {
//       generators[0] = new systems::generator<map::generator_context>(map_generator_size);
//       
//     }
//     
//     return generators;
//   }

//   void map_frustum_test(const map::container* map, const glm::mat4 &frustum, std::vector<uint32_t> &indices) {
//     const utils::frustum fru = utils::compute_frustum(frustum);
// 
// 
//   }
  
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
  
//   void map_triangle_test2(dt::thread_pool* pool, const map::container* map, const utils::frustum &fru, const uint32_t &triangle_index, std::atomic<uint32_t> &counter) {
//     // без рекурсии
//   }

  void set_default_values(sol::state &lua, sol::table &table) {
    table["userdata"] = lua.create_table();
    table["userdata"]["plates_count"] = 199;
    table["userdata"]["ocean_percentage"] = 0.7f;
    table["userdata"]["noise_multiplier"] = 0.1f;
    table["userdata"]["blur_ratio"] = 0.3f;
    table["userdata"]["blur_water_ratio"] = 1.0f;
    table["userdata"]["blur_iterations_count"] = 2;
    table["userdata"]["provinces_count"] = 4000;
    table["userdata"]["history_iterations_count"] = 300;
    
    table["tiles"] = lua.create_table();
    const size_t count = core::map::hex_count_d(core::map::detail_level);
    for (size_t i = 0; i < count; ++i) {
      table["tiles"][i] = lua.create_table();
      table["tiles"][i]["water_distance"] = lua.create_table();
    }
  }

  void rendering_mode(const map::generator::container* cont, core::map* map, const uint32_t &property, const uint32_t &render_mode, const uint32_t &water_mode) {
    const auto type = cont->type(map::debug::entities::tile, property);
    
    global::get<render::tile_render>()->change_rendering_mode(render_mode, water_mode, 0, 3, glm::vec3(0.0f, 0.0f, 0.0f));
    
    // по идее у нас может потребоваться не только взять данные из тайла, но и выше по иерархии
    // (например текущий тех государства (или провинции))
    // + может потребоваться дополнительная информация в тултипе
    // тут не обойтись без задаваемого скрипта обновления информации походу
    // вообще нам еще нужны границы, как они должны выглядеть?
    // я думал что достаточно нарисовать толстую линию между двумя точками 
    // у тайла, другое дело что не понятно какие именно границы должны быть отрисованы
    // я так понимаю что границы всегда одни и теже (провинция, герцогство, королество, империя)
    // как то визуально граница объекта выше по иерархии должна быть больше
    // как определить эти самые объекты, после создания карты, мы должны запустить 
    // функцию со списком типов свойств у тайла, по которым будут генерироваться границы
    // короче более менее постоянные границы есть у провинций
    // остальные границы должны генерироваться на основе текущей ситуации
    // фиксированные границы герцогств, королеств и проч нас интересуют когда мы хотим 
    // получить дополнительную информацию об конкретном титуле
    // государство + границы вассалов первого уровня
    // как генерировать эти границы? граница меняется не чаще чем 1 раз за ход
    // а то и реже, значит можно добавить все эти данные в какой-то буфер
    // вообще граница делается по провинциям, как это может мне помочь?
    // да никак особенно, мне нужно придумать как быстро заполнять информацию для границ
    // граница - это всегда два тайла в разных странах (у вассалов)
    // по всей видимости нужно пересобирать каждый раз при изменении границ
    // нжно придумать какой то мультитрединговый алгоритм
    // как рисовать цветную границу? есть алгоритм https://forum.libcinder.org/topic/smooth-thick-lines-using-geometry-shader
    // другое дело что я не могу использовать геометрический шейдер
    // да и он по идее не нужен
    // мне нужно сгенерировать половину границы и покрасить ее в какой то цвет
    // граница - кольцо из точек составляемых как?
    // разные страны, в стране есть вассалы, причем у игрока может не быть независимости
    // тогда нужно рисовать вассалов игрока и вассалов соседних вассалов?
    
//     const std::vector<uint32_t> border_props = {
//       map::debug::properties::tile::province_index,
//       map::debug::properties::tile::gerz_index,
//       map::debug::properties::tile::kingdom_index,
//       map::debug::properties::tile::empire_index,
//     };
    
    // получаем рендер стейдж и меняем ему режим
    switch (type) {
      case map::generator::data_type::float_t: {
        for (size_t i = 0; i < map->tiles_count(); ++i) { // понятное дело легко параллелится
          const float val = cont->get_data<float>(map::debug::entities::tile, i, property);
          map->set_tile_biom(i, glm::floatBitsToUint(val));
        }
        break;
      }
      
      case map::generator::data_type::uint_t: {
        for (size_t i = 0; i < map->tiles_count(); ++i) { // понятное дело легко параллелится
          const uint32_t val = cont->get_data<uint32_t>(map::debug::entities::tile, i, property);
          map->set_tile_biom(i, val);
        }
        break;
      }
      
      // тут может потребоваться как сгенерировать цвет так и взять уже существующий
      case map::generator::data_type::int_t: {
        for (size_t i = 0; i < map->tiles_count(); ++i) {
          union convertor {
            uint32_t uval;
            int32_t ival;
          };
          const int32_t val = cont->get_data<int32_t>(map::debug::entities::tile, i, property);
          convertor c;
          c.ival = val;
          map->set_tile_biom(i, c.uval);
        }
        break;
      }
    }
    
    // по идее всегда флоат значения означают последний режим 
    // нужно только решить вопрос с водой 
    // 
  }
  
  // это мы делаем максимум раз в ход
  void border_points_test(const std::vector<glm::vec4> &array) {
    // точки должны быть замкнутыми
    // точки должны быть последовательными (!)
    // 
    
    std::vector<glm::vec4> out;
    const float thickness = 0.7f;
    for (size_t i = 0; i < array.size(); ++i) {
      const glm::vec4 p0 = array[i+0];
      const glm::vec4 p1 = array[(i+1)%array.size()];
      const glm::vec4 p2 = array[(i+2)%array.size()];
      // все точки должны быть ближайшими к соседям
      
      // некоторые нормализации отсюда можно убрать
      const glm::vec4 p0p1 = glm::normalize(p1-p0);
      const glm::vec4 p1p2 = glm::normalize(p2-p1);
      const glm::vec4 np1 = glm::normalize(p1);
      const glm::vec4 n0 = glm::normalize(glm::vec4(glm::cross(glm::vec3(p0p1), glm::vec3(glm::normalize(p0))), 0.0f));
      const glm::vec4 n1 = glm::normalize(glm::vec4(glm::cross(glm::vec3(p1p2), glm::vec3(np1)), 0.0f));
      const glm::vec4 t = glm::normalize(n1 + n0);
      const glm::vec4 m = glm::normalize(glm::vec4(glm::cross(glm::vec3(t), glm::vec3(np1)), 0.0f));
      
      const float l = thickness / glm::dot(m, p0p1); // возможно в моем случае будет достаточно просто умножить thickness на m
      
      const glm::vec4 point1_near_p1 = p1 + m * l;
      const glm::vec4 point2_near_p1 = p1 - m * l;
      // как то так считается точка границы
      // мне нужно еще взять как то только половину во внутренюю сторону
      // то есть либо point1_near_p1 и p1 либо point2_near_p1 и p1
      // как понять какая сторона внутренняя? по идее сторона будет строго определена 
      // если мы выберем идти ли нам по часовой стрелке или против
      // то есть точки должны быть строго последовательными по часовой стрелке
      out.push_back(p1);
      out.push_back(point1_near_p1);
    }
  }
  
  void find_border_points(const map::generator::container* container, const core::map* map, const sol::table &table) {
    // возможно нужно найти первый граничный тайл, а потом обходить его соседей
    // тут можно поступить несколькими способами
    // 1) попытаться обойти тайлы правильно
    // 2) составить облако точек (ну хотя вряд ли)
    // у меня не всегда провинция более менее верной формы
    // не говоря даже про страны, значит нужно попытаться обойти тайлы верно
    // находится первый случайный граничный тайл, от него, по часовой стрелке
    // обходим граничные тайлы (у всех граничных тайлов, должно быть как минимум два граничных соседа)
    // вот еще вариант, отрисовывать границу с помощью тайловой графики 
    // (то есть несколько спрайтов шестигранников с границами в разном положении)
    // осталось понять как сделать эту тайловую графику
    // самый простой способ - задавать рендер для трех точек (центр + ребро)
    // у этих трех точек всегда одинаковые uv координаты (потому что по сути рисуем только одну одинаковую картинку)
    // осталось только понять как решить некоторые проблемы такого рендеринга
    // есть разрывы в местах перехода от одного тайла к другому
    // разрывы по идее также можно подкорректировать тайловой графикой
    // короче вот что я подумал (точнее вспомнил как это по идее сделано в цк2)
    // границ должно быть две: граница провинций и граница государств (граница вассалов?)
    // граница провинций - статичная, граница государств и вассалов обновляются почти каждый ход
    // вообще я могу сгенерировать границы между провинциями и менять их свойства по ходу игры
    // то есть мне нужно верно определить ребра провинции
    // у ребра две стороны: мы должны рисовать границы разных свойств на каждой стороне
    // все точки ребер мы добавляем в буфер, ребро - треугольник, каждому ребру нужно передать 
    // верную текстурку и какие то дополнительные данные
    struct border_type {
      // особая текстурка для разных типов границ
      // должна быть сплошная прямая и какие то более сложные рисунки
      // пунктир? его нужно правильно задавать, в цк используется
      // двухцветный пунктир, для этого нужно совместить алгоритмы этот и выше
      uint32_t type;
      float size;
      glm::vec4 color;
    };
    
    struct border_data {
      glm::vec4 point;
      glm::vec4 dir;
//       glm::vec4 dir2;
//       uint32_t tile_index;
    };
    
//     struct edge_point {
//       uint32_t container;
//       
//       edge_point(const bool index, const uint32_t &point_index) {
//         
//       }
//     };
    
    struct border_gen_data {
      uint32_t point1; // edge
      uint32_t point2;
      uint32_t tile_index;
      // ??
      uint32_t edge_index;
    };
    
    // тайлы нам сильно помогут сгенерить границы
    // границы делаем по ребрам провинции, нам их нужно верно соединить
    // ребра провинции должны быть по часовой стрелке относительно провинции
    // для каждой провниции я должен сгенерировать ребра
    // между провинциями ребра нужно соединить, 
    // для этого нужно подвести ребро провинции с помощью точки на ребре тайла
    // как получить uv координаты?
    // их можно расчитать для правильного тайла (для неправильного тайла тоже по идее, но нужно проверить)
    // тайл состоит из равносторонних треугольников + у нас имеется размер границы
    // uv для внутренней расчитываются с использованием размера
    // остается только последовательность гарантировать
    
//     std::vector<glm::vec4> triangles;
//     std::unordered_map<size_t, std::vector<uint32_t>> edges_indices;
//     std::unordered_map<size_t, std::vector<border_gen_data>> edges_indices2;
//     //const uint32_t tiles_count = map->tiles_count();
//     const uint32_t provinces_count = container->entities_count(map::debug::entities::province);
//     std::vector<std::vector<std::pair<uint32_t, std::vector<border_data>>>> provinces_borders(provinces_count);
//     for (size_t i = 0; i < provinces_count; ++i) {
//       const uint32_t province_index = i;
//       // нужен более строгий способ получать тайлы у провинций
//       const uint32_t tiles_count = table["provinces"][province_index]["tiles_count"];
//       for (size_t j = 0; j < tiles_count; ++j) {
//         const uint32_t tile_index = table["provinces"][province_index]["tiles"][j];
//         
//         const auto &data = render::unpack_data(map->get_tile(tile_index));
//         const uint32_t n_count = render::is_pentagon(data) ? 5 : 6;
//         for (size_t k = 0; k < n_count; ++k) {
//           const uint32_t n_index = data.neighbours[k];
//           
//           const uint32_t n_province_index = container->get_data<uint32_t>(map::debug::entities::tile, n_index, map::debug::properties::tile::province_index);
//           if (province_index == n_province_index) continue;
//           
//           // ребро, по идее точки это
//           const auto &n_data = render::unpack_data(map->get_tile(n_index));
//           const uint32_t k2 = k == 0 ? n_count-1 : k-1;
//           const uint32_t point1_index = data.points[k];
//           const uint32_t point2_index = data.points[k2];
//           const glm::vec4 point1 = map->get_point(point1_index);
//           const glm::vec4 point2 = map->get_point(point2_index);
//           // тут надо еще както определить соседей для правильного рендеринга
//           // пока так
//           
//           // теперь у нас есть два треугольника, рисовать лучше наверное листом
//           const uint32_t index1 = triangles.size();
//           triangles.push_back(point1);
//           triangles.push_back(point2);
//           triangles.push_back(map->get_point(data.center));
//           const uint32_t index2 = triangles.size();
//           triangles.push_back(point1);
//           triangles.push_back(point2);
//           triangles.push_back(map->get_point(n_data.center));
//           
// //           const uint32_t min_index = std::min(province_index, n_province_index);
// //           const uint32_t max_index = std::max(province_index, n_province_index);
// //           const size_t key = (size_t(min_index) << 32) | max_index;
//           const size_t key = (size_t(n_province_index) << 32) | province_index;
//           edges_indices[key].push_back(index1);
//           edges_indices[key].push_back(index2); 
//           edges_indices2[key].push_back({point2_index, point1_index, tile_index, UINT32_MAX});
//         }
//       }
//     }
//     
//     for (auto &pair : edges_indices2) {
//       ASSERT(!pair.second.empty());
//       std::vector<border_gen_data> sorted_data;
//       sorted_data.push_back(pair.second[0]);
//       while (sorted_data.size() != pair.second.size()) {
//         for (size_t i = 1; i < pair.second.size(); ++i) {
//           const uint32_t current_index = i;
//           const auto &current_data = pair.second[current_index];
//     //         sorted_data.push_back(current_data);
//           for (size_t j = 0; j < sorted_data.size(); ++j) {
//             const auto &data = sorted_data[j];
//             if ((data.point1 == current_data.point1 || data.point1 == current_data.point2) && 
//                 (data.point2 == current_data.point1 || data.point2 == current_data.point2) && 
//                  data.tile_index == current_data.tile_index) continue;
//             
//             if (data.point2 == current_data.point1 || data.point2 == current_data.point2) {
//               if (data.tile_index == current_data.tile_index) {
//                 auto itr = sorted_data.insert(sorted_data.begin()+j+1, current_data);
//                 if (data.point2 == current_data.point2) std::swap(itr->point1, itr->point2);
//               } else {
//                 const auto &tile_data = render::unpack_data(map->get_tile(current_data.tile_index));
//                 const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
//                 for (uint32_t k = 0; k < n_count; ++k) {
//                   const uint32_t n_index = tile_data.neighbours[k];
//                   if (n_index == data.tile_index) {
//                     const uint32_t k2 = k == 0 ? n_count-1 : k-1;
//                     const uint32_t point1 = tile_data.points[k2];
//                     const uint32_t point2 = tile_data.points[k];
//                     // нашли ребро, нужно по особому его добавить
//                     // 
//                     
// #ifndef _NDEBUG
//                     {
//                       const uint32_t test1 = uint32_t(point1 == current_data.point1) + uint32_t(point1 == current_data.point2) + uint32_t(point1 == data.point1) + uint32_t(point1 == data.point2);
//                       const uint32_t test2 = uint32_t(point2 == current_data.point1) + uint32_t(point2 == current_data.point2) + uint32_t(point2 == data.point1) + uint32_t(point2 == data.point2);
//                       ASSERT((test1 == 2 && test2 == 0) || (test1 == 0 && test2 == 2));
//                     }
// #endif
// 
//                     auto itr = sorted_data.insert(sorted_data.begin()+j+1, {current_data.point1, current_data.point2, current_data.tile_index, k});
//                     if (data.point2 == current_data.point2) std::swap(itr->point1, itr->point2);
//                     
//                     break;
//                   }
//                 }
//               }
//             }
//             
//             if (data.point1 == current_data.point1 || data.point1 == current_data.point2) {
//               if (data.tile_index == current_data.tile_index) {
//                 auto itr = sorted_data.insert(sorted_data.begin()+j, current_data);
//                 if (data.point1 == current_data.point1) std::swap(itr->point1, itr->point2);
//               } else {
//                 const auto &tile_data = render::unpack_data(map->get_tile(current_data.tile_index));
//                 const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
//                 for (uint32_t k = 0; k < n_count; ++k) {
//                   const uint32_t n_index = tile_data.neighbours[k];
//                   if (n_index == data.tile_index) {
//                     const uint32_t k2 = k == 0 ? n_count-1 : k-1;
//                     const uint32_t point1 = tile_data.points[k2];
//                     const uint32_t point2 = tile_data.points[k];
//                     // нашли ребро, нужно по особому его добавить
//                     // 
//                     
// #ifndef _NDEBUG
//                     {
//                       const uint32_t test1 = uint32_t(point1 == current_data.point1) + uint32_t(point1 == current_data.point2) + uint32_t(point1 == data.point1) + uint32_t(point1 == data.point2);
//                       const uint32_t test2 = uint32_t(point2 == current_data.point1) + uint32_t(point2 == current_data.point2) + uint32_t(point2 == data.point1) + uint32_t(point2 == data.point2);
//                       ASSERT((test1 == 2 && test2 == 0) || (test1 == 0 && test2 == 2));
//                     }
// #endif
// 
//                     auto itr = sorted_data.insert(sorted_data.begin()+j, {current_data.point1, current_data.point2, current_data.tile_index, k});
//                     if (data.point2 == current_data.point2) std::swap(itr->point1, itr->point2);
//                     
//                     break;
//                   }
//                 }
//               }
//             }
//           }
//         }
//       }
//       
//       std::swap(sorted_data, pair.second);
//     }
//     
//     for (auto &pair : edges_indices2) {
//       const size_t key = pair.first;
//       const uint32_t province_index = uint32_t(key); // & size_t(UINT32_MAX)
//       const uint32_t n_province_index = uint32_t(key >> 32);
//       std::vector<border_data> border;
//       for (size_t i = 0; i < pair.second.size(); ++i) {
//         const auto &border_gen_data = pair.second[i];
//         const auto &data = render::unpack_data(map->get_tile(border_gen_data.tile_index));
//         const glm::vec4 point1 = map->get_point(border_gen_data.point1);
//         const glm::vec4 point2 = map->get_point(border_gen_data.point2);
//         const glm::vec4 center = map->get_point(data.center);
//         const glm::vec4 dir1 = glm::normalize(center - point1);
//         const glm::vec4 dir2 = glm::normalize(center - point2);
//         
//         if (border_gen_data.edge_index != UINT32_MAX) {
//           const uint32_t n_count = render::is_pentagon(data) ? 5 : 6;
//           const uint32_t k = border_gen_data.edge_index;
//           const uint32_t k2 = k == 0 ? n_count-1 : k-1;
//           
//           const uint32_t edge_point1_index = (data.points[k] == border_gen_data.point1) || (data.points[k] == border_gen_data.point2) ? data.points[k] : data.points[k2];
//           const uint32_t edge_point2_index = (data.points[k] == border_gen_data.point1) || (data.points[k] == border_gen_data.point2) ? data.points[k2] : data.points[k];
//           const glm::vec4 point1 = map->get_point(edge_point1_index);
//           const glm::vec4 point2 = map->get_point(edge_point2_index);
//           const glm::vec4 dir = glm::normalize(point2 - point1);
//           const border_data d{
//             glm::vec4(point1.x, point1.y, point1.z, glm::uintBitsToFloat(edge_point1_index)),
//             glm::vec4(dir.x, dir.y, dir.z, glm::uintBitsToFloat(border_gen_data.tile_index))
//           };
//           border.push_back(d);
//         } else {
//           const border_data d1{
//             glm::vec4(point1.x, point1.y, point1.z, glm::uintBitsToFloat(border_gen_data.point1)),
//             glm::vec4(dir1.x, dir1.y, dir1.z, glm::uintBitsToFloat(border_gen_data.tile_index))
//           };
//           border.push_back(d1);
//         }
//         
//         if (i+1 == pair.second.size()) {
//           const border_data d2{
//             glm::vec4(point2.x, point2.y, point2.z, glm::uintBitsToFloat(border_gen_data.point2)),
//             glm::vec4(dir2.x, dir2.y, dir2.z, glm::uintBitsToFloat(border_gen_data.tile_index))
//           };
//           border.push_back(d2);
//         }
//       }
//       provinces_borders[province_index].push_back(std::make_pair(n_province_index, border));
//     }
    
    // теперь где то эту информацию нужно сохранить
    
    // как то так мы составляем ребра, что теперь с ними делать?
    // по идее все, можно рендерить
    // обходим все ребра и выдаем им характеристики на базе текущей ситуации (тип границы, цвет) (1 раз в ход)
    // затем параллельно обходим каждый треугольник и проверяем его с фрустумом
    // (можно на гпу сделать по идее) (каждый кадр)
    // рисуем поверх карты 
    
    // так вообще я тут подумал
    // тайловая графика чем хороша - она предсказуема
    // все что мне нужно сделать это определить 3 типа отрисовки границы:
    // только внутреннее ребро, внутренее и внешняя часть с одной стороны, полное ребро
    // то есть по сути нужно только где-то убрать и где-то добавить внешнюю часть ребра
    // значит нужно просто найти все граничные ребра, с информацией о двух тайлов
    // как определить типы? внешняя часть добавляется только к ребру, смежное ребро которого 
    // приходится на тайл с тем же самым государством (вассалом, провинцей)
    // нужно проверить смежные ребра
    // по идее этого достаточно для того чтобы собрать буфер после фрустум проверки 
    // но это означает что мы каждый кадр обходим этот массив и закидываем данные в буфер
    // с другой стороны как иначе? добавить суда сразу данные о границе (цвет, размер)
    // и считать все на гпу
    struct border_data2 {
      uint32_t tile_index;
      uint32_t opposite_tile_index;
      // по этому индексу мы во первых должны найти opposite_tile_index
      // а во вторых две точки + два смежных тайла
      uint32_t edge_index;
    };
    
    struct border_buffer {
      glm::vec4 points[2];
      glm::vec4 dirs[2];
    };
    
    // кому еще соседи могут пригодиться?
    
    std::vector<border_data2> borders;
    const uint32_t provinces_count = container->entities_count(map::debug::entities::province);
    for (size_t i = 0; i < provinces_count; ++i) {
      const uint32_t province_index = i;
      // нужен более строгий способ получать тайлы у провинций
      const uint32_t tiles_count = container->get_childs(map::debug::entities::province, province_index).size();
      for (size_t j = 0; j < tiles_count; ++j) {
        const uint32_t tile_index = container->get_childs(map::debug::entities::province, province_index)[j]; 
        
        const auto &data = render::unpack_data(map->get_tile(tile_index));
        const uint32_t n_count = render::is_pentagon(data) ? 5 : 6;
        for (uint32_t k = 0; k < n_count; ++k) {
          const uint32_t n_index = data.neighbours[k];
          
          const uint32_t n_province_index = container->get_data<uint32_t>(map::debug::entities::tile, n_index, map::debug::properties::tile::province_index);
          if (province_index == n_province_index) continue;
          
//           const auto &n_data = render::unpack_data(map->get_tile(n_index));
//           const uint32_t k2 = k == 0 ? n_count-1 : k-1;
          const border_data2 d{
            tile_index,
            n_index,
            k
          };
          borders.push_back(d);
        }
      }
    }
    
    struct border_type2 {
      glm::vec3 color1;
      uint32_t image;
      glm::vec3 color2;
      float thickness;
    };
    
    std::vector<border_type2> types(container->entities_count(map::debug::entities::country)+1);
    std::unordered_set<uint32_t> unique_index;
    yavf::Buffer* buffer = global::get<render::buffers>()->border_buffer;
    buffer->resize(borders.size() * sizeof(border_buffer));
    global::get<render::tile_borders_optimizer>()->set_borders_count(borders.size());
    auto* arr = reinterpret_cast<border_buffer*>(buffer->ptr());
    for (size_t i = 0; i < borders.size(); ++i) {
      const auto &current_data = borders[i];
      const auto &tile_data = render::unpack_data(map->get_tile(current_data.tile_index));
      const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
      
      ASSERT(tile_data.neighbours[current_data.edge_index] == current_data.opposite_tile_index);
      
#ifndef _NDEBUG
      {
        const uint32_t k = current_data.edge_index;
        //const uint32_t k2 = k == 0 ? n_count-1 : k-1;
        const uint32_t k2 = (k+1)%n_count;
        const uint32_t point1 = tile_data.points[k];
        const uint32_t point2 = tile_data.points[k2];
        ASSERT(point1 != UINT32_MAX);
        ASSERT(point2 != UINT32_MAX);
        ASSERT(point1 != point2);
        
        const auto &n_tile_data = render::unpack_data(map->get_tile(current_data.opposite_tile_index));
        const uint32_t n_count2 = render::is_pentagon(n_tile_data) ? 5 : 6;
        bool found1 = false;
        bool found2 = false;
        for (uint32_t j = 0; j < n_count2; ++j) {
          const uint32_t n_point_index = n_tile_data.points[j];
          if (n_point_index == point1) found1 = true;
          if (n_point_index == point2) found2 = true;
        }
        
        ASSERT(found1 && found2);
      }
#endif
      
      const uint32_t tmp_index = (current_data.edge_index)%n_count;
      const uint32_t adjacent1 = tile_data.neighbours[(tmp_index+1)%n_count];
      const uint32_t adjacent2 = tile_data.neighbours[tmp_index == 0 ? n_count-1 : tmp_index-1];
      
#ifndef _NDEBUG
      {
        const uint32_t k = (tmp_index+1)%n_count;
        const uint32_t k2 = k == 0 ? n_count-1 : k-1;
        const uint32_t k3 = (tmp_index+2)%n_count;
        const uint32_t k4 = k == 0 ? n_count-2 : (k == 1 ? n_count-1 : k-2);
        const uint32_t point1 = tile_data.points[k];
        const uint32_t point2 = tile_data.points[k2];
        const uint32_t point3 = tile_data.points[k3];
        const uint32_t point4 = tile_data.points[k4];
        ASSERT(point1 != UINT32_MAX);
        ASSERT(point2 != UINT32_MAX);
        ASSERT(point1 != point2);
        
        {
          bool found1 = false;
          bool found2 = false;
          bool found3 = false;
          bool found4 = false;
          
          const auto &n_tile_data = render::unpack_data(map->get_tile(adjacent1));
          const uint32_t n_count2 = render::is_pentagon(n_tile_data) ? 5 : 6;
          for (uint32_t j = 0; j < n_count2; ++j) {
            const uint32_t n_point_index = n_tile_data.points[j];
            if (n_point_index == point1) found1 = true;
            if (n_point_index == point2) found2 = true;
            if (n_point_index == point3) found3 = true;
            if (n_point_index == point4) found4 = true;
          }
          
          ASSERT(found1);
          ASSERT(found3);
          ASSERT(!found2);
          ASSERT(!found4);
        }
        
        {
          bool found1 = false;
          bool found2 = false;
          bool found3 = false;
          bool found4 = false;
          
          const auto &n_tile_data = render::unpack_data(map->get_tile(adjacent2));
          const uint32_t n_count2 = render::is_pentagon(n_tile_data) ? 5 : 6;
          for (uint32_t j = 0; j < n_count2; ++j) {
            const uint32_t n_point_index = n_tile_data.points[j];
            if (n_point_index == point1) found1 = true;
            if (n_point_index == point2) found2 = true;
            if (n_point_index == point3) found3 = true;
            if (n_point_index == point4) found4 = true;
          }
          
          ASSERT(!found1);
          ASSERT(!found3);
          ASSERT(found2);
          ASSERT(found4);
        }
      }
#endif

      // каждый ход нам необходимо произвести небольшие вычисления
      // чтобы обновить границы по ходу игры
      // каждый кадр обновляю то что нужно нарисовать фрустум тест
      
      const uint32_t k = (tmp_index+1)%n_count;
      const uint32_t k2 = k == 0 ? n_count-1 : k-1;
      //const uint32_t k2 = (k+1)%n_count;
      const uint32_t point1_index = tile_data.points[k];
      const uint32_t point2_index = tile_data.points[k2];
      const glm::vec4 point1 = map->get_point(point1_index);
      const glm::vec4 point2 = map->get_point(point2_index);
      arr[i].points[0] = point2;
      arr[i].points[1] = point1;
      
      // нужно заполнить направления
      // нужно проверить принадлежат ли смежные тайлы к тем же государствам 
      // тут же мы определяем тип границы (государственная, вассальная, граница провинции)
      
      const std::vector<uint32_t> prop_arr = {
        map::debug::properties::tile::country_index,
        map::debug::properties::tile::province_index
      };
      
      for (const auto &prop : prop_arr) {
        // или лучше брать эти данные из провинции
        const uint32_t country_index = container->get_data<uint32_t>(map::debug::entities::tile, current_data.tile_index, prop);
        const uint32_t opposing_country_index = container->get_data<uint32_t>(map::debug::entities::tile, current_data.opposite_tile_index, prop);
        
        if (country_index == opposing_country_index) continue;
        
        const uint32_t adjacent1_index = container->get_data<uint32_t>(map::debug::entities::tile, adjacent1, prop);
        const uint32_t adjacent2_index = container->get_data<uint32_t>(map::debug::entities::tile, adjacent2, prop);
        
        if (country_index != adjacent1_index) {
          const glm::vec4 center = map->get_point(tile_data.center);
          arr[i].dirs[1] = glm::normalize(center - arr[i].points[1]);
        } else {
          const uint32_t k3 = (tmp_index+2)%n_count;
          const uint32_t point3_index = tile_data.points[k3];
          const glm::vec4 point3 = map->get_point(point3_index);
          arr[i].dirs[1] = glm::normalize(point3 - arr[i].points[1]);
        }
        
        if (country_index != adjacent2_index) {
          const glm::vec4 center = map->get_point(tile_data.center);
          arr[i].dirs[0] = glm::normalize(center - arr[i].points[0]);
        } else {
          const uint32_t k4 = k == 0 ? n_count-2 : (k == 1 ? n_count-1 : k-2);
          const uint32_t point4_index = tile_data.points[k4];
          const glm::vec4 point4 = map->get_point(point4_index);
          arr[i].dirs[0] = glm::normalize(point4 - arr[i].points[0]);
        }
        
        if (prop == map::debug::properties::tile::country_index) {
          types[country_index+1] = {{0.0f, 0.0f, 0.0f}, UINT32_MAX, {0.0f, 0.0f, 0.0f}, 0.3f};
          arr[i].dirs[1].w = glm::uintBitsToFloat(country_index+1);
        }
        
        if (prop == map::debug::properties::tile::province_index) {
          types[0] = {{0.0f, 0.0f, 0.0f}, UINT32_MAX, {0.0f, 0.0f, 0.0f}, 0.15f};
          arr[i].dirs[1].w = glm::uintBitsToFloat(0);
        }
        
        break;
      }
      
      arr[i].dirs[0].w = glm::uintBitsToFloat(current_data.tile_index);
      //arr[i].dirs[1].w = glm::uintBitsToFloat(border_type);
      
      // по идее все что мне нужно теперь делать каждый ход это:
      // обновить тип границы, обновить направления
      // и все
      // каждый кадр запихиваем три координаты в фрустум тест
    }
    
    yavf::Buffer* types_buffer = global::get<render::buffers>()->border_types;
    types_buffer->resize(types.size() * sizeof(border_type2));
    auto ptr = types_buffer->ptr();
    ASSERT(ptr != nullptr);
    memcpy(ptr, types.data(), types.size() * sizeof(border_type2));
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
    //if (!global::get<input::data>()->interface_focus) return;

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
    
//     if (action == GLFW_RELEASE) std::cout << "Release" << "\n";
//     else if (action == GLFW_PRESS) std::cout << "Press" << "\n";
//     else if (action == GLFW_REPEAT) std::cout << "Repeat" << "\n";

    //global::data()->keys[key] = !(action == GLFW_RELEASE);
    const auto old = global::get<input::data>()->key_events.container[key].event;
//     global::get<input::data>()->key_events.container[key].prev_event = old;
    global::get<input::data>()->key_events.container[key].event = static_cast<input::type>(action);
    auto data = global::get<input::data>()->key_events.container[key].data;
    if (data != nullptr && old != static_cast<input::type>(action)) data->time = 0;
    
    //ASSERT(false);
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
