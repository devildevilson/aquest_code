#include "helper.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <thread>
#include <filesystem>

#include "whereami.h"

static const std::vector<const char*> instanceLayers = {
  "VK_LAYER_LUNARG_standard_validation",
  "VK_LAYER_LUNARG_api_dump",
  "VK_LAYER_LUNARG_assistant_layer"
};

namespace devils_engine {
#define RELEASE_CONTAINER_DATA(var) if (var != nullptr) container.destroy(var); var = nullptr;
  
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
    
    std::filesystem::path p = std::string(array);
    p /= "../";
    const std::string dir_path = p;
    std::cout << dir_path << "\n";
    
    //ASSERT(dir_path.size() == size_t(dirname+1));
    
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
    
    const auto biome_render_mode = utils::id::get("biome_render_mode");
    const auto cultures_render_mode = utils::id::get("cultures_render_mode");
    const auto culture_groups_render_mode = utils::id::get("culture_groups_render_mode");
    const auto religions_render_mode = utils::id::get("religions_render_mode");
    const auto religion_groups_render_mode = utils::id::get("religion_groups_render_mode");
    const auto provinces_render_mode = utils::id::get("provinces_render_mode");
    const auto countries_render_mode = utils::id::get("countries_render_mode");
    const auto duchies_render_mode = utils::id::get("duchies_render_mode");
    const auto kingdoms_render_mode = utils::id::get("kingdoms_render_mode");
    const auto empires_render_mode = utils::id::get("empires_render_mode");
    input::set_key(GLFW_KEY_F1, biome_render_mode);
    input::set_key(GLFW_KEY_F2, cultures_render_mode);
    input::set_key(GLFW_KEY_F3, culture_groups_render_mode);
    input::set_key(GLFW_KEY_F4, religions_render_mode);
    input::set_key(GLFW_KEY_F5, religion_groups_render_mode);
    input::set_key(GLFW_KEY_F6, provinces_render_mode);
    input::set_key(GLFW_KEY_F7, countries_render_mode);
    input::set_key(GLFW_KEY_F8, duchies_render_mode);
    input::set_key(GLFW_KEY_F9, kingdoms_render_mode);
    input::set_key(GLFW_KEY_F10, empires_render_mode);
    
    const utils::id menu_next = utils::id::get("menu_next");
    const utils::id menu_prev = utils::id::get("menu_prev");
    const utils::id menu_increase = utils::id::get("menu_increase");
    const utils::id menu_decrease = utils::id::get("menu_decrease");
    const utils::id menu_choose = utils::id::get("menu_choose");
    const utils::id escape = utils::id::get("escape");
    input::set_key(GLFW_KEY_DOWN, menu_next);
    input::set_key(GLFW_KEY_UP, menu_prev);
    input::set_key(GLFW_KEY_RIGHT, menu_increase);
    input::set_key(GLFW_KEY_LEFT, menu_decrease);
    input::set_key(GLFW_KEY_ENTER, menu_choose);
    input::set_key(GLFW_KEY_ESCAPE, escape);
  }
  
  float hit_sphere(const glm::vec4 &center, const float &radius, const utils::ray &r) {
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
    auto ctx = global::get<interface::context>();
//     auto buffers = global::get<render::buffers>();
    
    // если мы изначально нажали на карту то тогда двигаем
    // иначе ничего не делаем
    
    static bool was_pressed_on_map = false;
    static bool was_pressed = false;
    
    double xpos, ypos;
    static double last_xpos = 0.0, last_ypos = 0.0;
//     int32_t width, height;
    
    glfwGetCursorPos(window->handle, &xpos, &ypos);
    double delta_xpos = last_xpos - xpos, delta_ypos = last_ypos - ypos;
    last_xpos = xpos;
    last_ypos = ypos;
    
    const float zoom = camera->zoom();
    const float zoom_k = zoom / components::camera::max_zoom;
    const float sens = 5.0f * (1.0f + zoom_k * 0.5f);
    const float x_sens = 1.0f;
    const float y_sens = 1.0f;
    const float x_move = sens * x_sens * MCS_TO_SEC(time) * delta_xpos;
    const float y_move = sens * y_sens * MCS_TO_SEC(time) * delta_ypos;
    
    static const utils::id map_move = utils::id::get("map_move");
    const bool current_pressed = input::is_event_pressed(map_move);
    if (current_pressed && !was_pressed) {
      was_pressed_on_map = !nk_window_is_any_hovered(&ctx->ctx);
    }
    
    if (current_pressed && was_pressed_on_map) {
      camera->move(x_move, y_move);
    }
    
    was_pressed = current_pressed;
    if (!current_pressed) was_pressed_on_map = false;
  }
  
  void key_input(const size_t &time, const uint32_t &current_state, const bool loading) {
    (void)time;
    static const utils::id modes[] = {
      utils::id::get("biome_render_mode"),
      utils::id::get("cultures_render_mode"),
      utils::id::get("culture_groups_render_mode"),
      utils::id::get("religions_render_mode"),
      utils::id::get("religion_groups_render_mode"),
      utils::id::get("provinces_render_mode"),
      utils::id::get("countries_render_mode"),
      utils::id::get("duchies_render_mode"),
      utils::id::get("kingdoms_render_mode"),
      utils::id::get("empires_render_mode"),
    };
    
    static_assert(sizeof(modes) / sizeof(modes[0]) == render::modes::count);
    
    static const utils::id escape = utils::id::get("escape");
    
//     auto map = global::get<core::map>();
//     auto container = global::get<map::generator::container>();
    
    auto s = global::get<systems::core_t>();
    const bool get_menu = current_state != utils::quest_state::map_creation && 
                          //current_state != game_state::loading &&
                          input::check_event(escape, input::state::state_click | input::state::state_double_click | input::state::state_long_click);
    const bool last_menu = current_state == utils::quest_state::main_menu && s->menu->menu_stack.size() == 1;
    if (!loading && get_menu) {
      if (s->interface->escape()) return;
      if (!s->menu->exist()) {
        if (current_state == utils::quest_state::main_menu) s->menu->push("main_menu");
        else s->menu->push("main_menu_map");
      } else if (!last_menu) s->menu->escape();
      return;
    }
    
    {
      size_t mem = 0;
      auto change = input::next_input_event(mem, 1);
      while (change.id.valid()) {
        //PRINT_VAR("change id", change.id.name())
        if (change.event != input::release) {
//           if (change.id == escape) {
//             
//           }
          
          for (size_t i = 0; i < render::modes::count; ++i) {
            if (change.id != modes[i]) continue;
            render::mode(static_cast<render::modes::values>(i));
          }
        }
        
        change = input::next_input_event(mem, 1);
      }
    }
  }
  
  void zoom_input(yacs::entity* ent) {
    auto input_data = global::get<input::data>();
    auto camera = ent->get<components::camera>();
    auto ctx = global::get<interface::context>();
    const bool window_focus = nk_window_is_any_hovered(&ctx->ctx);
    if (window_focus) return;
    
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
    float y = 1.0f - (2.0f * ypos) / float(window->surface.extent.height); // тут по идее должно быть обратное значение
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
    
    //PRINT_VAR("hit", hit)
    
    const glm::vec4 point_on_sphere = intersect_sphere.pos + intersect_sphere.dir * hit;
    const glm::vec4 new_dir = glm::normalize(-glm::vec4(glm::vec3(point_on_sphere), 0.0f));
    
    const utils::ray casting_ray{
      point_on_sphere - new_dir * 5.0f,
      new_dir
    };
    
    const uint32_t tile_index_local1 = global::get<core::map>()->cast_ray(casting_ray);
    //PRINT_VEC4("point_on_sphere", point_on_sphere)
//     if (tile_index_local1 != UINT32_MAX) {
//       const auto &tile_data = render::unpack_data(global::get<core::map>()->get_tile(tile_index_local1));
//       const glm::vec4 center = global::get<core::map>()->get_point(tile_data.center);
//       PRINT_VEC4("tile center     ", center)
//       PRINT_VEC4("point_on_sphere ", point_on_sphere)
//     }
    return tile_index_local1;
  }
  
  bool long_key(const int &key, const size_t &time) {
    UNUSED_VARIABLE(time);
    return input::timed_check_key(key, input::state_press | input::state_double_press, 0, SIZE_MAX) || 
      input::timed_check_key(key, input::state_long_press | input::state_double_press, ONE_SECOND / 2, ONE_SECOND / 15);
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

  void create_render_system(systems::core_t &base_systems) {
    uint32_t count;
    const char** ext = glfwGetRequiredInstanceExtensions(&count);
    base_systems.create_render_system(ext, count);
  }
  
  void setup_callbacks() {
    auto window = global::get<render::window>();
    glfwSetKeyCallback(window->handle, keyCallback);
    glfwSetCharCallback(window->handle, charCallback);
    glfwSetMouseButtonCallback(window->handle, mouseButtonCallback);
    glfwSetScrollCallback(window->handle, scrollCallback);
    glfwSetWindowFocusCallback(window->handle, focusCallback);
    glfwSetWindowIconifyCallback(window->handle, iconifyCallback);
    glfwSetWindowSizeCallback(window->handle, window_resize_callback);
  }
  
  void basic_interface_functions(systems::core_t &base_systems) {
    const std::string script_folder = global::root_directory() + "scripts/";
    base_systems.interface_container->process_script_file(script_folder + "generator_progress.lua");
    base_systems.interface_container->process_script_file(script_folder + "user_interface.lua");
    base_systems.interface_container->process_script_file(script_folder + "player_layer.lua");
    base_systems.interface_container->register_function("progress_bar", "progress_bar");
    base_systems.interface_container->register_function("main_menu_window", "main_menu");
    base_systems.interface_container->register_function("main_menu_map", "main_menu_map");
    base_systems.interface_container->register_function("worlds_window_func", "worlds_window");
    base_systems.interface_container->register_function("main_interface_layer", "player_interface");
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

  void set_default_values(sol::state &lua, sol::table &table) {
    table["userdata"] = lua.create_table();
    table["userdata"]["plates_count"] = 199;
    table["userdata"]["ocean_percentage"] = 0.7f;
    table["userdata"]["noise_multiplier"] = 0.1f;
    table["userdata"]["blur_ratio"] = 0.7f;
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
  
  int my_exception_handler(lua_State* L, sol::optional<const std::exception&> maybe_exception, sol::string_view description) {
    // L is the lua state, which you can wrap in a state_view if necessary
    // maybe_exception will contain exception, if it exists
    // description will either be the what() of the exception or a description saying that we hit the general-case catch(...)
    std::cout << "Lua throw an exception: ";
    if (maybe_exception) {
      const std::exception& ex = *maybe_exception;
      std::cout << ex.what() << std::endl;
    } else {
      std::cout.write(description.data(), description.size());
      std::cout << std::endl;
    }

    // you must push 1 element onto the stack to be 
    // transported through as the error object in Lua
    // note that Lua -- and 99.5% of all Lua users and libraries -- expects a string
    // so we push a single string (in our case, the description of the error)
    return sol::stack::push(L, description);
  }
  
  void update(const size_t &time) {
    // у нас есть список персонажей 
    // как выглядит непосредственно геймплей? обходим всех персонажей которые не мертвы и играбельны
    // и делаем какие то действия
    // во всех пошаговых стратегий хорошим тоном будет сделать ход игрока первым
    // но мне кажется более хорошим решением будет сделать ход по возрасту (по интеллекту?)
    // то есть ход будет начинаться не от игрока, нужно это для того чтобы подчеркнуть очередность хода
    // ну и на мой взгляд так будет интереснее (правда от этого пострадает мультитрединг)
    // 
    UNUSED_VARIABLE(time);
    game::advance_state();
    
    // как мы теперь интерфейс запускаем? запускаем при загрузке? 
    // именно в загрузке мы переходим от одного состояния к другому, 
    // должен быть список юзер интерфейсов для каждого сотояния
    
    // здесь рид онли стейт
//     global::get<utils::interface>()->draw(time);
    // нужно отключить все окна кроме базового + както ограничить некоторые элементы интерфейса
    // хотя возможно отключить нужно только окна эвентов, решений и еще парочку
    // должно быть примерно так же как это было в эндлесс леджент
    // параллельное вычисление хода может накрыться 
    // потому что я хочу некоторым династиям сделать продвинутый ии
    // с другой стороны здесь можно использовать корутины
    // или делать параллельный луа стейт? корутин из коробки, к сожалению, запускается
    // в том же треде, но при этом можно делать некоторые задачи паралельно
    // нет, корутины не так работают (это просто функции с прерываниями), 
    // нужно сделать отдельный луа стейт для интерфейса
    // 
    
    
    // рисуем интерфейс, ждем пока игрок нажмет какие то кнопки и реагируем на них
    // тут нужно видимо сделать некий класс который это все в себя соберет
    // интерфейс нарисовать бы в луа, чтобы быстро можно было бы менять данные интерфейса
    // нашел библиотечку MoonNuklear - биндинг для наклира (простой пример оказался лучше чем такой же у самого наклира, лол)
    // я так понимаю в примерах наклира где то утечка, эту библиотечку можно использовать для быстрого прототипирования интерфейса
    // что нам нужно для интерфейса? типы окон, нужно ли как то ограничить возможности наклира в луа? 
    // (в интерфейсах не должно быть прямого доступа к данным игры, все данные только для чтения)
    // если мы переключимся на другой стейт, то мы не отрисуем интерфейс
    // то есть интерфейс рисуем во всех стейтах + некую анимацию хода (вращающиеся песочные часы например)
    // следует передать в интерфейс еще время (возможно сделать какую то анимацию)
    
    // интерфейс состоит из нескольких окон, которые мы зададим через луа
    // это означает необходимо последовательно вызвать несколько луа функций
    // по порядку в зависимости от типа (есть базовый интерфейс: кнопки сверху/снизу, ресурсы и проч)
    // (есть окно персонажа: иконки, портреты, статы + тултипы)
    // (есть окно эвента: картинка текст) (снизу база, окно перса, над этим всем эвент)
    // приоритетов не сказать чтобы очень много, возможно стоит просто выделить два типа окна
    // как сделать ориентир окна по клавиатуре? нужно зарегистрировать какие то эвенты для интерфейса
    // и использовать функции для проверки эвентов
    // только теперь нужно делать две функции: инициализация и собственно интерфейс, 
    // нужно ли несколько эвентов прилеплять на одну кнопку? можно
    // нужно ли вешать один эвент на несколько кнопок? фиг знает
    
    // интерфейс главного меню тоже сделать в луа? с этим нужно быть аккуратнее, каким то образом нужно 
    // очень оптимально сделать окно загрузки и сохранения (нужно будет загрузить какую то информацию с диска)
    
    // как мы узнаем что игрок сделал ход? по идее мы должны самостоятельно вызвать функцию адванс тюрн
    // необходимо ограничить адванс тюрн так чтобы мы не могли прожать это дело без того чтобы игрок отреагировал на какие то эвенты
    // на битвы игроку тоже нужно реагировать
    
    // все структуры нужно сделать только для чтения для луа, как тогда делать изменения?
    // во первых какие изменения в луа? в интерфейсе точно не будет изменений
    // изменения будут в битвах, например, но там по идее изменяются другие типы (отряды)
    // 
//     }
  }

  void sync(utils::frame_time &frame_time, const size_t &time) {
    //auto start = std::chrono::steady_clock::now();
    frame_time.end();
    
    {
//       utils::time_log log("task waiting");
      global::get<render::task_start>()->wait();
      //global::get<render::stage_container>()->clear();
      global::get<systems::core_t>()->render_slots->clear();
      global::get<systems::map_t>()->unlock_map();
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
    auto data = global::get<input::data>();
    const auto old = data->key_events.container[button].event;
    data->key_events.container[button].event = static_cast<input::type>(action);
    if (old != static_cast<input::type>(action)) data->key_events.container[button].event_time = 0;
  }

  void keyCallback(GLFWwindow*, int key, int scancode, int action, int mods) {
    (void)mods;

    if (!global::get<render::window>()->flags.focused()) return;
    (void)scancode;
    
//     if (action == GLFW_RELEASE) std::cout << "Release" << "\n";
//     else if (action == GLFW_PRESS) std::cout << "Press" << "\n";
//     else if (action == GLFW_REPEAT) std::cout << "Repeat" << "\n";

    //global::data()->keys[key] = !(action == GLFW_RELEASE);
    auto data = global::get<input::data>();
    const auto old = data->key_events.container[key].event;
    data->key_events.container[key].event = static_cast<input::type>(action);
    if (old != static_cast<input::type>(action)) data->key_events.container[key].event_time = 0;
    
    //ASSERT(false);
  }

  void window_resize_callback(GLFWwindow*, int w, int h) {
    global::get<render::window>()->recreate(w, h);
    //global::get<GBufferStage>()->recreate(w, h);
    global::get<render::stage_container>()->recreate(w, h);
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

