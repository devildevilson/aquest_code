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
//       sizeof(map::generator::container) + 
      sizeof(utils::interface) +
      sizeof(core::context) +
      sizeof(utils::data_string_container) +
      sizeof(utils::sequential_string_container) +
      sizeof(utils::calendar) +
      sizeof(systems::ai) +
      sizeof(core::seasons)
    ),
    graphics_container(nullptr),
    map(nullptr),
    //map_generator(nullptr),
    context(nullptr),
//     map_container(nullptr),
    interface(nullptr),
    core_context(nullptr),
    string_container(nullptr),
    sequential_string_container(nullptr),
    game_calendar(nullptr),
    ai_systems(nullptr),
    seasons(nullptr)
  {}

  system_container_t::~system_container_t() {
    RELEASE_CONTAINER_DATA(map)
    //RELEASE_CONTAINER_DATA(map_generator)
    RELEASE_CONTAINER_DATA(graphics_container)
    RELEASE_CONTAINER_DATA(context)
//     RELEASE_CONTAINER_DATA(map_container)
    RELEASE_CONTAINER_DATA(interface)
    RELEASE_CONTAINER_DATA(core_context)
    RELEASE_CONTAINER_DATA(string_container)
    RELEASE_CONTAINER_DATA(sequential_string_container)
    RELEASE_CONTAINER_DATA(game_calendar)
    RELEASE_CONTAINER_DATA(ai_systems)
    RELEASE_CONTAINER_DATA(seasons)
  }
  
  game_map_data::game_map_data() : 
    container({
      sizeof(core::map),
      sizeof(systems::render),
      sizeof(core::context),
      sizeof(core::seasons),
      sizeof(systems::ai)
    }),
    map(nullptr),
    render(nullptr),
    core_context(nullptr),
    seasons(nullptr),
    ai_systems(nullptr)
  {}
  
  game_map_data::~game_map_data() {
    RELEASE_CONTAINER_DATA(map)
    RELEASE_CONTAINER_DATA(core_context)
    RELEASE_CONTAINER_DATA(seasons)
    RELEASE_CONTAINER_DATA(ai_systems)
  }
  
  bool game_map_data::is_init() const { return map != nullptr; }
  
  game_battle_data::game_battle_data() :
    container({
      sizeof(core::map),
      sizeof(systems::render),
      sizeof(core::context),
      sizeof(core::seasons),
      sizeof(systems::ai)
    }),
//     map(nullptr),
//     core_context(nullptr),
//     seasons(nullptr),
    ai_systems(nullptr)
  {}
  
  game_battle_data::~game_battle_data() {
//     RELEASE_CONTAINER_DATA(map)
//     RELEASE_CONTAINER_DATA(core_context)
//     RELEASE_CONTAINER_DATA(seasons)
    RELEASE_CONTAINER_DATA(ai_systems)
  }
  
  game_encounter_data::game_encounter_data() : 
    container({
      sizeof(core::map),
      sizeof(systems::render),
      sizeof(core::context),
      sizeof(core::seasons),
      sizeof(systems::ai)
    }),
//     map(nullptr),
//     core_context(nullptr),
//     seasons(nullptr),
    ai_systems(nullptr)
  {}
  
  game_encounter_data::~game_encounter_data() {
//     RELEASE_CONTAINER_DATA(map)
//     RELEASE_CONTAINER_DATA(core_context)
//     RELEASE_CONTAINER_DATA(seasons)
    RELEASE_CONTAINER_DATA(ai_systems)
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
  
  void key_input(const size_t &time) {
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
    
//     auto map = global::get<core::map>();
//     auto container = global::get<map::generator::container>();
    
    {
      size_t mem = 0;
      auto change = input::next_input_event(mem, 1);
      while (change.id.valid()) {
        //PRINT_VAR("change id", change.id.name())
        if (change.event != input::release) {
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
    const size_t stage_container_size =
      sizeof(render::buffers) +
      //sizeof(render::images) +
      //sizeof(render::particles) +
      //sizeof(render::deffered) +

      sizeof(render::window_next_frame) +
      sizeof(render::task_begin) +
      
      sizeof(render::tile_optimizer) +
      sizeof(render::tile_borders_optimizer) +
      sizeof(render::tile_walls_optimizer) +
      sizeof(render::barriers) +

      sizeof(render::render_pass_begin) +
      sizeof(render::tile_render) +
      sizeof(render::tile_border_render) +
      sizeof(render::tile_connections_render) +
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
      ENGINE_VERSION,
      VK_API_VERSION_1_0
    };

    systems.graphics_container = systems.container.create<render::container>();
    systems.graphics_container->create_instance(extensions, &info);
    auto window = systems.graphics_container->create_window();
    systems.graphics_container->create_device();
    window->create_swapchain(systems.graphics_container->device);
    auto render = systems.graphics_container->create_system(stage_container_size);
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
    auto opt3    = system->add_stage<render::tile_walls_optimizer>(render::tile_walls_optimizer::create_info{device});
                   system->add_stage<render::barriers>();
                 
                   system->add_stage<render::render_pass_begin>();
    auto tiles   = system->add_stage<render::tile_render>(render::tile_render::create_info{device, opt});
    auto borders = system->add_stage<render::tile_border_render>(render::tile_border_render::create_info{device, opt2});
    auto walls   = system->add_stage<render::tile_connections_render>(render::tile_connections_render::create_info{device, opt3});
                   // тут мы можем добавить особые стартеры, которые будем отключать когда у нас полный рендер пайплайн
                   system->add_stage<render::interface_stage>(render::interface_stage::create_info{device});
                   system->add_stage<render::render_pass_end>();

                   system->add_stage<render::task_end>();
    auto start   = system->add_stage<render::task_start>(device);
                   system->add_stage<render::window_present>(render::window_present::create_info{window});

    global::get(start);
    global::get(tiles);
    global::get(walls);
    global::get(buffers);
    global::get(opt2);
    global::get(opt3);
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
    systems.sequential_string_container = systems.container.create<utils::sequential_string_container>();
    utils::id::set_container(systems.sequential_string_container);
    systems.game_calendar = systems.container.create<utils::calendar>();
    global::get(systems.game_calendar);
//     systems.map_container = systems.container.create<map::generator::container>(); // это мы смело перемещаем в мап креатор
//     global::get(systems.map_container);
    systems.seasons = systems.container.create<core::seasons>();
    global::get(systems.seasons);
    
    // этот интерфейс наверное используем для описания интерфейса создания карты
    // не можем потому что он не предназначен для этого, хотя мы можем хранить вместо указателя объект
    // как записать в объект юзердату? ну по идее как то передаю в функцию юзердату
//     create_interface(systems);
  }
  
  void create_ai_systems(system_container_t &systems) {
    const size_t container_size = sizeof(ai::build_subsystem);
    systems.ai_systems = systems.container.create<systems::ai>(container_size);
    systems.ai_systems->add<ai::build_subsystem>();
    
    global::get(systems.ai_systems);
  }
  
  void create_game_state() {
    // в идеале, сейчас сделаем иначе
//     std::random_device dev;
//     const uint32_t v1 = dev();
//     const uint32_t v2 = dev();
//     const size_t seed = (size_t(v2) << 32) | size_t(v1);
//     global g;
//     g.initialize_state(seed);
    global g;
    g.initialize_state(1);
  }
  
  map::creator* setup_map_generator() {
    auto interface = global::get<utils::interface>();
    ASSERT(interface != nullptr);
    auto ptr = new map::creator(interface);
    
    ptr->run_interface_script(global::root_directory() + "scripts/gen_part1.lua");
    ptr->run_interface_script(global::root_directory() + "scripts/gen_part2.lua");
    ptr->run_interface_script(global::root_directory() + "scripts/gen_part3.lua");
    ptr->run_interface_script(global::root_directory() + "scripts/generator_progress.lua");
    ptr->progress_interface("gen_progress");
    
    {
//       const size_t step1_size = sizeof(map::property_int) + sizeof(map::property_float);
      const std::vector<map::generator_pair> pairs = {
//         map::default_generator_pairs[0],
        map::default_generator_pairs[1],
        map::default_generator_pairs[2],
        map::default_generator_pairs[3]
      };
      ptr->create("gen_part1_fun", pairs);
//       auto step = ptr->create(true, step1_size, "Tectonic plates generator", pairs, "plates_render_mode");
//       auto prop1 = step->add<map::property_int>(map::property_int::create_info{
//         40,
//         199,
//         300,
//         1,
//         0.5f,
//         "Plates count",
//         "plates_count"
//       });
//       prop1->set_default_value(ptr->get_table());
//       
//       auto prop2 = step->add<map::property_float>(map::property_float::create_info{
//         0.0f,
//         0.7f,
//         1.0f,
//         0.01f,
//         0.005f,
//         "Ocean ratio",
//         "ocean_percentage"
//       });
//       prop2->set_default_value(ptr->get_table());
    }
    
    {
//       const size_t step1_size = sizeof(map::property_int) + sizeof(map::property_float) + sizeof(map::property_float) + sizeof(map::property_float);
      const std::vector<map::generator_pair> pairs = {
        map::default_generator_pairs[4],
        map::default_generator_pairs[5],
        map::default_generator_pairs[6],
        map::default_generator_pairs[7],
        map::default_generator_pairs[8],
        map::default_generator_pairs[9],
        map::default_generator_pairs[10],
        map::default_generator_pairs[11],
        map::default_generator_pairs[12],
        map::default_generator_pairs[13]
      };
      
      ptr->create("gen_part2_fun", pairs);
//       auto step = ptr->create(false, step1_size, "Biomes generator", pairs, "biome_render_mode");
//       auto prop1 = step->add<map::property_float>(map::property_float::create_info{
//         0.0f,
//         0.1f,
//         1.0f,
//         0.01f,
//         0.005f,
//         "Noise multiplier",
//         "noise_multiplier"
//       });
//       prop1->set_default_value(ptr->get_table());
//       
//       auto prop2 = step->add<map::property_float>(map::property_float::create_info{
//         0.0f,
//         0.7f,
//         1.0f,
//         0.01f,
//         0.005f,
//         "Blur ratio",
//         "blur_ratio"
//       });
//       prop2->set_default_value(ptr->get_table());
//       
//       auto prop3 = step->add<map::property_float>(map::property_float::create_info{
//         0.0f,
//         1.0f,
//         2.0f,
//         0.01f,
//         0.005f,
//         "Water blur ratio",
//         "blur_water_ratio"
//       });
//       prop3->set_default_value(ptr->get_table());
//       
//       auto prop4 = step->add<map::property_int>(map::property_int::create_info{
//         0,
//         2,
//         5,
//         1,
//         0.05f,
//         "Blur iterations count",
//         "blur_iterations_count"
//       });
//       prop4->set_default_value(ptr->get_table());
    }
    
    {
//       const size_t step1_size = sizeof(map::property_int) + sizeof(map::property_int);
      const std::vector<map::generator_pair> pairs = {
        map::default_generator_pairs[14],
        map::default_generator_pairs[15],
        map::default_generator_pairs[16],
        map::default_generator_pairs[17],
        map::default_generator_pairs[18],
        map::default_generator_pairs[19],
        map::default_generator_pairs[20],
        map::default_generator_pairs[21],
      };
      
      ptr->create("gen_part3_fun", pairs);
//       auto step = ptr->create(false, step1_size, "Countries generator", pairs, "countries_render_mode");
//       auto prop1 = step->add<map::property_int>(map::property_int::create_info{
//         1000,
//         4000,
//         5000,
//         50,
//         float(5000 - 1000) / 400.0f,
//         "Province count",
//         "provinces_count"
//       });
//       prop1->set_default_value(ptr->get_table());
//       
//       auto prop2 = step->add<map::property_int>(map::property_int::create_info{
//         200,
//         300,
//         1000,
//         10,
//         float(1000 - 200) / 400.0f,
//         "History iterations count",
//         "history_iterations_count"
//       });
//       prop2->set_default_value(ptr->get_table());
    }
    
    return ptr;
  }
  
  void destroy_map_generator(map::creator** ptr) {
    auto p = *ptr;
    delete p;
    *ptr = nullptr;
  }
  
  // переделаю данные которые хранятся в тайле (цвет, текстура)
  // эта часть полностью изменится, для некоторых данных нужно будет сгенерировать безье текст
  // 
  void setup_rendering_modes(render::mode_container &container) {
    container[render::modes::biome] = [] () {
      auto ptr = global::get<core::seasons>();
      auto map = global::get<core::map>();
      for (size_t i = 0; i < core::map::hex_count_d(core::map::detail_level); ++i) {
        const auto &current_biome = ptr->biomes[ptr->get_tile_biome(i)];
        map->set_tile_color(i, current_biome.color);
        map->set_tile_texture(i, current_biome.texture);
      }
    };
    
    container[render::modes::cultures] = [] () {
      auto ctx = global::get<core::context>();
      auto map = global::get<core::map>();
      for (size_t i = 0; i < ctx->get_entity_count<core::province>(); ++i) {
        auto province = ctx->get_entity<core::province>(i);
        auto culture = nullptr;
        for (const uint32_t tile_index : province->tiles) {
          const uint32_t rand_num1 = render::lcg(size_t(culture));
          const uint32_t rand_num2 = render::lcg(rand_num1);
          const uint32_t rand_num3 = render::lcg(rand_num2);
          const float color_r = render::lcg_normalize(rand_num1);
          const float color_g = render::lcg_normalize(rand_num2);
          const float color_b = render::lcg_normalize(rand_num3);
          map->set_tile_color(tile_index, render::make_color(color_r, color_b, color_g, 1.0f));
          map->set_tile_texture(tile_index, {GPU_UINT_MAX});
        }
      }
    };
    
    container[render::modes::culture_groups] = [] () {
      auto ctx = global::get<core::context>();
      auto map = global::get<core::map>();
      for (size_t i = 0; i < ctx->get_entity_count<core::province>(); ++i) {
        auto province = ctx->get_entity<core::province>(i);
        auto culture = nullptr;
        for (const uint32_t tile_index : province->tiles) {
          const uint32_t rand_num1 = render::lcg(size_t(culture));
          const uint32_t rand_num2 = render::lcg(rand_num1);
          const uint32_t rand_num3 = render::lcg(rand_num2);
          const float color_r = render::lcg_normalize(rand_num1);
          const float color_g = render::lcg_normalize(rand_num2);
          const float color_b = render::lcg_normalize(rand_num3);
          map->set_tile_color(tile_index, render::make_color(color_r, color_b, color_g, 1.0f));
          map->set_tile_texture(tile_index, {GPU_UINT_MAX});
        }
      }
    };
    
    container[render::modes::religions] = [] () {
      auto ctx = global::get<core::context>();
      auto map = global::get<core::map>();
      for (size_t i = 0; i < ctx->get_entity_count<core::province>(); ++i) {
        auto province = ctx->get_entity<core::province>(i);
        auto religion = nullptr;
        for (const uint32_t tile_index : province->tiles) {
          const uint32_t rand_num1 = render::lcg(size_t(religion));
          const uint32_t rand_num2 = render::lcg(rand_num1);
          const uint32_t rand_num3 = render::lcg(rand_num2);
          const float color_r = render::lcg_normalize(rand_num1);
          const float color_g = render::lcg_normalize(rand_num2);
          const float color_b = render::lcg_normalize(rand_num3);
          map->set_tile_color(tile_index, render::make_color(color_r, color_b, color_g, 1.0f));
          map->set_tile_texture(tile_index, {GPU_UINT_MAX});
        }
      }
    };
    
    container[render::modes::religion_groups] = [] () {
      auto ctx = global::get<core::context>();
      auto map = global::get<core::map>();
      for (size_t i = 0; i < ctx->get_entity_count<core::province>(); ++i) {
        auto province = ctx->get_entity<core::province>(i);
        auto religion = nullptr;
        for (const uint32_t tile_index : province->tiles) {
          const uint32_t rand_num1 = render::lcg(size_t(religion));
          const uint32_t rand_num2 = render::lcg(rand_num1);
          const uint32_t rand_num3 = render::lcg(rand_num2);
          const float color_r = render::lcg_normalize(rand_num1);
          const float color_g = render::lcg_normalize(rand_num2);
          const float color_b = render::lcg_normalize(rand_num3);
          map->set_tile_color(tile_index, render::make_color(color_r, color_b, color_g, 1.0f));
          map->set_tile_texture(tile_index, {GPU_UINT_MAX});
        }
      }
    };
    
    container[render::modes::provinces] = [] () { // не уверен что это вооще нужно
      auto ctx = global::get<core::context>();
      auto map = global::get<core::map>();
      ASSERT(ctx->get_entity_count<core::province>() != 0);
      for (size_t i = 0; i < ctx->get_entity_count<core::province>(); ++i) {
        auto province = ctx->get_entity<core::province>(i);
        ASSERT(!province->tiles.empty());
        for (const uint32_t tile_index : province->tiles) {
          const uint32_t rand_num1 = render::lcg(i);
          const uint32_t rand_num2 = render::lcg(rand_num1);
          const uint32_t rand_num3 = render::lcg(rand_num2);
          const float color_r = render::lcg_normalize(rand_num1);
          const float color_g = render::lcg_normalize(rand_num2);
          const float color_b = render::lcg_normalize(rand_num3);
          map->set_tile_color(tile_index, render::make_color(color_r, color_b, color_g, 1.0f));
          map->set_tile_texture(tile_index, {GPU_UINT_MAX});
        }
      }
    };
    
    container[render::modes::countries] = [] () {
      auto ctx = global::get<core::context>();
      auto map = global::get<core::map>();
      for (size_t i = 0; i < ctx->get_entity_count<core::province>(); ++i) {
        auto province = ctx->get_entity<core::province>(i);
        auto faction = province->title->owner;
        ASSERT(faction != nullptr);
        auto final_faction = faction;
        while (faction != nullptr) {
          final_faction = faction;
          faction = faction->liege;
        }
        ASSERT(final_faction != nullptr);
        
        // какое то число? пока что приведем указатель
        const auto color = final_faction->main_title->main_color;
        
        for (const uint32_t tile_index : province->tiles) {
          map->set_tile_color(tile_index, color);
          map->set_tile_texture(tile_index, {GPU_UINT_MAX});
        }
      }
    };
    
    container[render::modes::duchies] = [] () {
      static bool deure = false;
      if (render::get_current_mode() == render::modes::duchies) deure = !deure;
      if (render::get_current_mode() != render::modes::duchies) deure = false;
      
      auto ctx = global::get<core::context>();
      auto map = global::get<core::map>();
      for (size_t i = 0; i < ctx->get_entity_count<core::province>(); ++i) {
        auto province = ctx->get_entity<core::province>(i);
        core::titulus* duchy_title = nullptr;
        if (deure) {
          auto title = province->title;
          ASSERT(title != nullptr);
          duchy_title = title->parent;
        } else {
          auto owner = province->title->owner;
          while (owner != nullptr) {
            if (owner->main_title->type == core::titulus::type::duke) duchy_title = owner->main_title;
            owner = owner->liege;
          }
        }
        
        if (duchy_title != nullptr && duchy_title->owner != nullptr) {
//           const size_t num = reinterpret_cast<size_t>(duchy_title);
          const render::color_t color = duchy_title->main_color;
          
          for (const uint32_t tile_index : province->tiles) {
//             const uint32_t rand_num1 = render::lcg(num);
//             const uint32_t rand_num2 = render::lcg(rand_num1);
//             const uint32_t rand_num3 = render::lcg(rand_num2);
//             const float color_r = render::lcg_normalize(rand_num1);
//             const float color_g = render::lcg_normalize(rand_num2);
//             const float color_b = render::lcg_normalize(rand_num3);
//             map->set_tile_color(tile_index, render::make_color(color_r, color_b, color_g, 1.0f));
            map->set_tile_color(tile_index, color);
            map->set_tile_texture(tile_index, {GPU_UINT_MAX});
          }
        } else {
          for (const uint32_t tile_index : province->tiles) {
            map->set_tile_color(tile_index, render::make_color(0.3f, 0.3f, 0.3f, 1.0f));
            map->set_tile_texture(tile_index, {GPU_UINT_MAX});
          }
        }
      }
    };
    
    container[render::modes::kingdoms] = [] () {
      static bool deure = false;
      if (render::get_current_mode() == render::modes::kingdoms) deure = !deure;
      if (render::get_current_mode() != render::modes::kingdoms) deure = false;
      
      auto ctx = global::get<core::context>();
      auto map = global::get<core::map>();
      for (size_t i = 0; i < ctx->get_entity_count<core::province>(); ++i) {
        auto province = ctx->get_entity<core::province>(i);
        core::titulus* king_title = nullptr;
        if (deure) {
          auto title = province->title;
          ASSERT(title != nullptr);
          auto duchy_title = title->parent;
          king_title = duchy_title == nullptr ? nullptr : duchy_title->parent;
        } else {
          auto owner = province->title->owner;
          while (owner != nullptr) {
            if (owner->main_title->type == core::titulus::type::king) king_title = owner->main_title;
            owner = owner->liege;
          }
        }
        
        if (king_title != nullptr && king_title->owner != nullptr) {
          const render::color_t color = king_title->main_color;
          
          for (const uint32_t tile_index : province->tiles) {
            map->set_tile_color(tile_index, color);
            map->set_tile_texture(tile_index, {GPU_UINT_MAX});
          }
        } else {
          for (const uint32_t tile_index : province->tiles) {
            map->set_tile_color(tile_index, render::make_color(0.3f, 0.3f, 0.3f, 1.0f));
            map->set_tile_texture(tile_index, {GPU_UINT_MAX});
          }
        }
      }
    };
    
    container[render::modes::empires] = [] () {
      static bool deure = false;
      if (render::get_current_mode() == render::modes::empires) deure = !deure;
      if (render::get_current_mode() != render::modes::empires) deure = false;
      
      auto ctx = global::get<core::context>();
      auto map = global::get<core::map>();
      for (size_t i = 0; i < ctx->get_entity_count<core::province>(); ++i) {
        auto province = ctx->get_entity<core::province>(i);
        core::titulus* emp_title = nullptr;
        if (deure) {
          auto title = province->title;
          ASSERT(title != nullptr);
          auto duchy_title = title->parent;
          auto king_title = duchy_title == nullptr ? nullptr : duchy_title->parent;
          emp_title = king_title == nullptr ? nullptr : king_title->parent;
        } else {
          auto owner = province->title->owner;
          while (owner != nullptr) {
            if (owner->main_title->type == core::titulus::type::imperial) emp_title = owner->main_title;
            owner = owner->liege;
          }
        }
        
        if (emp_title != nullptr && emp_title->owner != nullptr) {
          const render::color_t color = emp_title->main_color;
          
          for (const uint32_t tile_index : province->tiles) {
            map->set_tile_color(tile_index, color);
            map->set_tile_texture(tile_index, {GPU_UINT_MAX});
          }
        } else {
          for (const uint32_t tile_index : province->tiles) {
            map->set_tile_color(tile_index, render::make_color(0.3f, 0.3f, 0.3f, 1.0f));
            map->set_tile_texture(tile_index, {GPU_UINT_MAX});
          }
        }
      }
    };
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
  
//   void map_triangle_test2(dt::thread_pool* pool, const map::container* map, const utils::frustum &fru, const uint32_t &triangle_index, std::atomic<uint32_t> &counter) {
//     // без рекурсии
//   }

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
  
  void load_interface_functions(utils::interface* interface, sol::state &lua) {
    const std::string script_dir = global::root_directory() + "scripts/";
    //lua.script_file(script_dir + "interface1.lua");
    //lua.set_panic(lua_panic);
    {
      auto script_from_file_result = lua.safe_script_file(script_dir + "interface1.lua");
      if (!script_from_file_result.valid()) {
        sol::error err = script_from_file_result;
        throw std::runtime_error("Error in interface1.lua file: " + std::string(err.what()));
      }
      
      {
        sol::protected_function func = lua["test_interface"];
        ASSERT(func != sol::nil);
        interface->register_layer("test_interface", {func, 1, core::structure::character});
      }
      
      interface->open_layer("test_interface", sol::nil);
    }
    
    {
//       auto script_from_file_result = lua.safe_script_file(script_dir + "user_interface.lua");
//       if (!script_from_file_result.valid()) {
//         sol::error err = script_from_file_result;
//         throw std::runtime_error("Error in interface1.lua file: " + std::string(err.what()));
//       }
//       
//       sol::protected_function func = lua["main_menu_window"];
//       ASSERT(func != sol::nil);
//       interface->register_window("main_menu_window", {func, 0, core::structure::character});
//       interface->open_window("main_menu_window", sol::nil);
      
      auto script_from_file_result = lua.safe_script_file(script_dir + "player_layer.lua");
      if (!script_from_file_result.valid()) {
        sol::error err = script_from_file_result;
        throw std::runtime_error("Error in player_layer.lua file: " + std::string(err.what()));
      }
      
      sol::protected_function func = lua["main_interface_layer"];
      ASSERT(func != sol::nil);
      interface->register_layer("main_interface_layer", {func, 0, core::structure::character});
      interface->open_layer("main_interface_layer", sol::nil);
    }
  }
  
  void find_border_points(const core::map* map) {
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
    
    struct border_type2 {
      render::color_t color1; // цвет достаточно хранить в uint32
      uint32_t image;
      render::color_t color2;
      float thickness; // эту штуку не нужно делать здесь - это должны быть константы от титула
    };
    
    // кому еще соседи могут пригодиться?
    
    auto ctx = global::get<core::context>();
    
    std::vector<border_data2> borders;
    const uint32_t provinces_count = ctx->get_entity_count<core::province>();
    for (size_t i = 0; i < provinces_count; ++i) {
      const uint32_t province_index = i;
      auto province = ctx->get_entity<core::province>(province_index);
      // нужен более строгий способ получать тайлы у провинций
      //const auto &childs = container->get_childs(map::debug::entities::province, province_index);
      const auto &childs = province->tiles;
      if (childs.empty()) throw std::runtime_error("Could not find province tiles");
      
      const uint32_t tiles_count = childs.size();
      for (size_t j = 0; j < tiles_count; ++j) {
        const uint32_t tile_index = childs[j]; 
        
        const auto &data = render::unpack_data(map->get_tile(tile_index));
        const uint32_t n_count = render::is_pentagon(data) ? 5 : 6;
        for (uint32_t k = 0; k < n_count; ++k) {
          const uint32_t n_index = data.neighbours[k];
          
          //const uint32_t n_province_index = container->get_data<uint32_t>(map::debug::entities::tile, n_index, map::debug::properties::tile::province_index);
          const uint32_t n_province_index = ctx->get_tile(n_index).province;
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
    
    const float borders_size[] = {0.5f, 0.3f, 0.15f}; // было бы неплохо это убрать в defines.lua (то есть считывать с диска)
    
    std::unordered_map<core::titulus*, uint32_t> type_index;
    std::vector<border_type2> types;
    types.push_back({render::make_color(0.0f, 0.0f, 0.0f, 1.0f), UINT32_MAX, render::make_color(0.0f, 0.0f, 0.0f, 1.0f), 0.0f});
    for (size_t i = 0; i < ctx->get_entity_count<core::titulus>(); ++i) {
      // нужно задать типы границ
      // по владельцам?
      auto title = ctx->get_entity<core::titulus>(i);
      if (title->type == core::titulus::type::baron) continue;
      if (title->type == core::titulus::type::city) continue;
      if (title->owner == nullptr) continue;
      if (title->owner->main_title != title) continue;
      ASSERT(type_index.find(title) == type_index.end());
      type_index.insert(std::make_pair(title, types.size()));
      types.push_back({title->border_color1, GPU_UINT_MAX, title->border_color2, 0.0f}); // толщину границы мы должны в поинт записать
    }
    
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
      
      const uint32_t province_index = ctx->get_tile(current_data.tile_index).province;
      const uint32_t opposite_province_index = ctx->get_tile(current_data.opposite_tile_index).province;
      
      const uint32_t adjacent1_province_index = ctx->get_tile(adjacent1).province;
      const uint32_t adjacent2_province_index = ctx->get_tile(adjacent2).province;
      
      if (province_index != adjacent1_province_index) {
        const glm::vec4 center = map->get_point(tile_data.center);
        arr[i].dirs[1] = glm::normalize(center - arr[i].points[1]);
      } else {
        const uint32_t k3 = (tmp_index+2)%n_count;
        const uint32_t point3_index = tile_data.points[k3];
        const glm::vec4 point3 = map->get_point(point3_index);
        arr[i].dirs[1] = glm::normalize(point3 - arr[i].points[1]);
      }
      
      if (province_index != adjacent2_province_index) {
        const glm::vec4 center = map->get_point(tile_data.center);
        arr[i].dirs[0] = glm::normalize(center - arr[i].points[0]);
      } else {
        const uint32_t k4 = k == 0 ? n_count-2 : (k == 1 ? n_count-1 : k-2);
        const uint32_t point4_index = tile_data.points[k4];
        const glm::vec4 point4 = map->get_point(point4_index);
        arr[i].dirs[0] = glm::normalize(point4 - arr[i].points[0]);
      }
      
      const auto province = ctx->get_entity<core::province>(province_index);
      const auto opposite_province = opposite_province_index == UINT32_MAX ? nullptr : ctx->get_entity<core::province>(opposite_province_index);
      ASSERT(province->title->owner != nullptr);
      
      arr[i].dirs[0].w = glm::uintBitsToFloat(current_data.tile_index);
      if (opposite_province == nullptr) {
        auto title = province->title->owner->main_title;
        const uint32_t type_idx = type_index[title];
        arr[i].points[0].w = borders_size[0];
        arr[i].dirs[1].w = glm::uintBitsToFloat(type_idx); // или здесь другую границу?
        continue;
      }
      
      ASSERT(opposite_province->title->owner != nullptr);
      if (province->title->owner == opposite_province->title->owner) {
        // один владелец, мы должны нарисовать базовую границу
        arr[i].points[0].w = borders_size[2];
        arr[i].dirs[1].w = glm::uintBitsToFloat(0);
        continue;
      }
      
      std::unordered_set<core::faction*> factions;
      auto liege1 = province->title->owner;
      while (liege1 != nullptr) {
        factions.insert(liege1);
        liege1 = liege1->liege;
      }
      
      bool found = false;
      auto liege2 = opposite_province->title->owner;
      while (liege2 != nullptr) {
        found = found || factions.find(liege2) != factions.end();
        liege2 = liege2->liege;
      }
      
      if (found) {
        // эти провинции находятся в одном государстве
        auto title = province->title->owner->main_title;
        const uint32_t type_idx = type_index[title];
        arr[i].points[0].w = borders_size[1];
        arr[i].dirs[1].w = glm::uintBitsToFloat(type_idx);
        
        continue;
      }
      
      // это граница разных государств
      auto title = province->title->owner->main_title;
      const uint32_t type_idx = type_index[title];
      arr[i].points[0].w = borders_size[0];
      arr[i].dirs[1].w = glm::uintBitsToFloat(type_idx);
      
//       const std::vector<uint32_t> prop_arr = {
//         map::debug::properties::tile::country_index,
//         map::debug::properties::tile::province_index
//       };
//       
//       for (const auto &prop : prop_arr) {
//         // или лучше брать эти данные из провинции
//         const uint32_t country_index = container->get_data<uint32_t>(map::debug::entities::tile, current_data.tile_index, prop);
//         const uint32_t opposing_country_index = container->get_data<uint32_t>(map::debug::entities::tile, current_data.opposite_tile_index, prop);
//         
//         if (country_index == opposing_country_index) continue;
//         
//         const uint32_t adjacent1_index = container->get_data<uint32_t>(map::debug::entities::tile, adjacent1, prop);
//         const uint32_t adjacent2_index = container->get_data<uint32_t>(map::debug::entities::tile, adjacent2, prop);
//         
//         if (country_index != adjacent1_index) {
//           const glm::vec4 center = map->get_point(tile_data.center);
//           arr[i].dirs[1] = glm::normalize(center - arr[i].points[1]);
//         } else {
//           const uint32_t k3 = (tmp_index+2)%n_count;
//           const uint32_t point3_index = tile_data.points[k3];
//           const glm::vec4 point3 = map->get_point(point3_index);
//           arr[i].dirs[1] = glm::normalize(point3 - arr[i].points[1]);
//         }
//         
//         if (country_index != adjacent2_index) {
//           const glm::vec4 center = map->get_point(tile_data.center);
//           arr[i].dirs[0] = glm::normalize(center - arr[i].points[0]);
//         } else {
//           const uint32_t k4 = k == 0 ? n_count-2 : (k == 1 ? n_count-1 : k-2);
//           const uint32_t point4_index = tile_data.points[k4];
//           const glm::vec4 point4 = map->get_point(point4_index);
//           arr[i].dirs[0] = glm::normalize(point4 - arr[i].points[0]);
//         }
//         
//         if (prop == map::debug::properties::tile::country_index) {
//           types[country_index+1] = {render::make_color(0.0f, 0.0f, 0.0f, 1.0f), UINT32_MAX, render::make_color(0.0f, 0.0f, 0.0f, 1.0f), 0.3f}; // это мы заполняем для каждого титула
//           arr[i].dirs[1].w = glm::uintBitsToFloat(country_index+1);
//         }
//         
//         if (prop == map::debug::properties::tile::province_index) {
//           types[0] = {render::make_color(0.0f, 0.0f, 0.0f, 1.0f), UINT32_MAX, render::make_color(0.0f, 0.0f, 0.0f, 1.0f), 0.15f};
//           arr[i].dirs[1].w = glm::uintBitsToFloat(0);
//         }
//         
//         break;
//       }
//       
//       arr[i].dirs[0].w = glm::uintBitsToFloat(current_data.tile_index);
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
  
  const uint32_t layers_count = 10;
  const float mountain_height = 0.5f;
  const float render_tile_height = 10.0f;
  const float layer_height = mountain_height / float(layers_count);
  void generate_tile_connections(const core::map* map, dt::thread_pool* pool) {
    struct wall {
      uint32_t tile1;
      uint32_t tile2;
      uint32_t point1;
      uint32_t point2;
    };
    
    std::vector<wall> walls;
    std::mutex mutex;
    
    utils::submit_works(pool, map->tiles_count(), [map, &walls, &mutex] (const size_t &start, const size_t &count) {
      for (size_t i = start; i < start+count; ++i) {
        const uint32_t tile_index = i;
        const auto &tile_data = render::unpack_data(map->get_tile(tile_index));
        const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
        const float tile_height = tile_data.height;
        const uint32_t height_layer = render::compute_height_layer(tile_height);
        if (height_layer == 0) continue;
                        
        for (uint32_t j = 0; j < n_count; ++j) {
          const uint32_t n_index = tile_data.neighbours[j];
          const auto &n_tile_data = render::unpack_data(map->get_tile(n_index));
          const float n_tile_height = n_tile_data.height;
          const uint32_t n_height_layer = render::compute_height_layer(n_tile_height);
          if (height_layer <= n_height_layer) continue;
                        
          // добавляем стенку 
          // нам нужны две точки и индексы тайлов
          
          const uint32_t point1 = tile_data.points[j];
          const uint32_t point2 = tile_data.points[(j+1)%n_count];
          
#ifndef _NDEBUG
          {
            std::unordered_set<uint32_t> tmp;
            tmp.insert(point1);
            tmp.insert(point2);
            
            uint32_t found = 0;
            const uint32_t n_n_count = render::is_pentagon(n_tile_data) ? 5 : 6;
            for (uint32_t k = 0; k < n_n_count; ++k) {
              const uint32_t point_index = n_tile_data.points[k];
              found += uint32_t(tmp.find(point_index) != tmp.end());
              
//               if (point1 == point_index) {
//                 const bool a = (n_tile_data.points[(k+1)%n_n_count] == point2 || n_tile_data.points[k == 0 ? n_n_count-1 : k-1] == point2);
//                 if (a) found = true;
//               }
            }
            
            ASSERT(found == 2);
          }
#endif

          std::unique_lock<std::mutex> lock(mutex);
          walls.push_back({tile_index, n_index, point1, point2});
        }
      }
    });
    
    auto connections = global::get<render::buffers>()->tiles_connections;
    connections->resize(walls.size() * sizeof(walls[0]));
    auto ptr = connections->ptr();
    memcpy(ptr, walls.data(), walls.size() * sizeof(walls[0]));
    global::get<render::tile_walls_optimizer>()->set_connections_count(walls.size());
  }
  
  void connect_game_data(core::map* map, core::context* ctx) {
    {
      const size_t count = ctx->get_entity_count<core::city>();
      for (size_t i = 0; i < count; ++i) {
        auto city = ctx->get_entity<core::city>(i);
        ASSERT(city->title != nullptr);
        ASSERT(city->title->count == 0);
        city->title->create_children(1);
        city->title->set_city(city);
        ASSERT(city->province != nullptr);
        ASSERT(city->province->cities_count < core::province::cities_max_game_count);
        city->province->cities[city->province->cities_count] = city;
        ++city->province->cities_count;
      }
    }
    
    {
      const size_t count = ctx->get_entity_count<core::province>();
      for (size_t i = 0; i < count; ++i) {
        auto province = ctx->get_entity<core::province>(i);
        ASSERT(province->cities_count <= province->cities_max_count);
        ASSERT(province->title != nullptr);
        ASSERT(province->title->count == 0);
        province->title->create_children(1);
        province->title->set_province(province);
        ASSERT(!province->tiles.empty());
        for (const auto &tile_index : province->tiles) {
          core::tile t;
          t.height = map->get_tile_height(tile_index);
          t.province = i;
          ctx->set_tile(tile_index, t);
        }
      }
    }
    
    {
      const size_t count = ctx->get_entity_count<core::titulus>();
      std::unordered_map<core::titulus*, std::vector<core::titulus*>> childs;
      for (size_t i = 0; i < count; ++i) {
        auto title = ctx->get_entity<core::titulus>(i);
        if (title->parent != nullptr) childs[title->parent].push_back(title);
      }
      
      for (size_t i = 0; i < count; ++i) {
        auto title = ctx->get_entity<core::titulus>(i);
        auto itr = childs.find(title);
        if (itr == childs.end()) continue;
        const auto &childs_arr = itr->second;
        ASSERT(title->childs == nullptr);
        title->create_children(childs_arr.size());
        if (childs_arr.size() == 1) {
          title->set_child(0, childs_arr[0]);
          continue;
        }
        
        for (size_t j = 0; j < childs_arr.size(); ++j) {
          title->set_child(j, childs_arr[j]);
        }
      }
    }
    
    {
      const size_t count = ctx->characters_count();
      std::unordered_map<core::faction*, std::vector<core::faction*>> vassals;
      std::unordered_map<core::faction*, std::vector<core::character*>> prisoners;
      std::unordered_map<core::character*, std::vector<core::character*>> court;
      std::unordered_map<core::character*, std::vector<core::character*>> concubines;
      std::unordered_map<core::character*, std::vector<core::character*>> children;
      for (size_t i = 0; i < count; ++i) {
        auto character = ctx->get_character(i);
        if (character->suzerain != nullptr) court[character->suzerain].push_back(character);
        if (character->imprisoner != nullptr) prisoners[character->imprisoner].push_back(character);
        if (character->factions[core::character::self] != nullptr && character->factions[core::character::self]->liege != nullptr) {
          vassals[character->factions[core::character::self]->liege].push_back(character->factions[core::character::self]);
        }
        if (character->family.owner != nullptr) concubines[character->family.owner].push_back(character);
        // братья сестры, нужно выбрать кого то из родителей и положить туда? как быть с полуродственниками?
        // скорее всего несколько супругов может быть только у правителей
        // предыдущих супругов я кажется пока не задаю, тогда нужно найти правителя
        ASSERT(character->family.previous_consorts == nullptr);
        const bool has_parent1 = character->family.parents[0] != nullptr;
        const bool has_parent2 = character->family.parents[1] != nullptr;
        core::character* parent = nullptr;
        if (has_parent1 || has_parent2) {
          if (parent == nullptr) parent = has_parent1 && has_parent2 && character->family.parents[0]->factions[core::character::self] != nullptr ? character->family.parents[0] : nullptr;
          if (parent == nullptr) parent = has_parent1 && has_parent2 && character->family.parents[1]->factions[core::character::self] != nullptr ? character->family.parents[1] : nullptr;
          if (parent == nullptr) parent = has_parent1 ? character->family.parents[0] : nullptr;
          if (parent == nullptr) parent = has_parent2 ? character->family.parents[1] : nullptr;
        }
        
        if (parent != nullptr) children[parent].push_back(character);
        
        for (size_t j = 0; j < core::character::relations::max_game_friends; ++j) {
          auto char_friend = character->relations.friends[j];
          if (char_friend == nullptr) continue;
          size_t counter = 0;
          for (size_t k = 0; k < core::character::relations::max_game_friends; ++k) {
            auto char_char_friend = char_friend->relations.friends[j];
            counter += size_t(char_char_friend != nullptr) + 500 * size_t(char_char_friend == character);
            if (char_char_friend == character) break;
          }
          
          if (counter > 499) continue;
          ASSERT(counter < core::character::relations::max_game_friends);
          char_friend->relations.friends[counter] = character;
        }
        
        for (size_t j = 0; j < core::character::relations::max_game_friends; ++j) {
          auto char_rival = character->relations.rivals[j];
          if (char_rival == nullptr) continue;
          size_t counter = 0;
          for (size_t k = 0; k < core::character::relations::max_game_rivals; ++k) {
            auto char_char_rival = char_rival->relations.rivals[j];
            counter += size_t(char_char_rival != nullptr) + 500 * size_t(char_char_rival == character);
            if (char_char_rival == character) break;
          }
          
          if (counter > 499) continue;
          ASSERT(counter < core::character::relations::max_game_rivals);
          char_rival->relations.rivals[counter] = character;
        }
        
        for (size_t j = 0; j < core::character::relations::max_game_lovers; ++j) {
          auto char_lover = character->relations.lovers[j];
          if (char_lover == nullptr) continue;
          size_t counter = 0;
          for (size_t k = 0; k < core::character::relations::max_game_lovers; ++k) {
            auto char_char_lovar = char_lover->relations.lovers[j];
            counter += size_t(char_char_lovar != nullptr) + 500 * size_t(char_char_lovar == character);
            if (char_char_lovar == character) break;
          }
          
          if (counter > 499) continue;
          ASSERT(counter < core::character::relations::max_game_lovers);
          char_lover->relations.friends[counter] = character;
        }
        
        if (character->family.consort != nullptr) {
          auto consort = character->family.consort;
          ASSERT(consort->family.consort == nullptr || consort->family.consort == character);
          consort->family.consort = character;
        }
      }
      
      for (size_t i = 0; i < count; ++i) {
        auto character = ctx->get_character(i);
        // у персонажа может быть другая форма правления (но наверное в этом случае персонажи не будут считаться за вассалов)
        auto faction = character->factions[core::character::self];
        if (faction == nullptr) continue;
        auto itr = vassals.find(faction);
        if (itr == vassals.end()) continue;
        const auto &vs = itr->second;
        for (size_t j = 0; j < vs.size(); ++j) {
          faction->add_vassal_raw(vs[j]);
        }
      }
      
      for (size_t i = 0; i < count; ++i) {
        auto character = ctx->get_character(i);
        auto faction = character->factions[core::character::self];
        if (faction == nullptr) continue;
        auto itr = prisoners.find(faction);
        if (itr == prisoners.end()) continue;
        const auto &ps = itr->second;
        for (size_t j = 0; j < ps.size(); ++j) {
          faction->add_prisoner_raw(character);
        }
      }
      
      for (size_t i = 0; i < count; ++i) {
        auto character = ctx->get_character(i);
        auto itr = court.find(character);
        if (itr == court.end()) continue;
        const auto &cs = itr->second;
        for (size_t j = 0; j < cs.size(); ++j) {
          character->add_courtier_raw(cs[j]);
        }
      }
      
      for (size_t i = 0; i < count; ++i) {
        auto character = ctx->get_character(i);
        auto itr = concubines.find(character);
        if (itr == concubines.end()) continue;
        const auto &cs = itr->second;
        for (size_t j = 0; j < cs.size(); ++j) {
          ASSERT(character->is_male() != cs[j]->is_male());
          character->add_concubine_raw(cs[j]);
        }
      }
      
      for (size_t i = 0; i < count; ++i) {
        auto character = ctx->get_character(i);
        auto itr = children.find(character);
        if (itr == children.end()) continue;
        const auto &cs = itr->second;
        for (size_t j = 0; j < cs.size(); ++j) {
          character->add_child_raw(cs[j]);
        }
        
        if (character->family.consort != nullptr) { 
          ASSERT(character->family.consort->family.children == nullptr);
          character->family.consort->family.children = character->family.children; 
        }
      }
    }
  }
  
  template <typename T>
  void create_entities_without_id() {
    auto tables = global::get<utils::table_container>();
    auto ctx = global::get<core::context>();
    
    const auto &data = tables->get_tables(T::s_type);
    ctx->create_container<T>(data.size());
    
    PRINT_VAR("cities count", data.size())
  }
  
  template <typename T>
  void create_entities() {
    auto tables = global::get<utils::table_container>();
    auto ctx = global::get<core::context>();
    // это заполнить в валидации не выйдет (потому что string_view)
    // в провинции нет id
    auto to_data = global::get<utils::data_string_container>();
    
    const auto &data = tables->get_tables(T::s_type);
    ctx->create_container<T>(data.size());
    for (size_t i = 0; i < data.size(); ++i) {
      auto ptr = ctx->get_entity<T>(i);
      ptr->id = data[i]["id"];
      const size_t index = to_data->get(ptr->id);
      if (index != SIZE_MAX) throw std::runtime_error("Found duplicate id " + ptr->id + " while parsing " + std::string(magic_enum::enum_name<core::structure>(T::s_type)) + " type data");
      to_data->insert(ptr->id, i);
    }
  }
  
  void create_characters() {
    auto tables = global::get<utils::table_container>();
    auto ctx = global::get<core::context>();
    
    const auto &data = tables->get_tables(core::structure::character);
    ASSERT(!data.empty());
    for (const auto &table : data) {
      bool male = true;
      bool dead = false;
      
      if (const auto &proxy = table["male"]; proxy.valid()) {
        male = proxy.get<bool>();
      }
      
      if (const auto &proxy = table["dead"]; proxy.valid()) {
        dead = proxy.get<bool>();
      }
      
      auto c = ctx->create_character(male, dead);
      
      // нам нужно заполнить стейт рандомайзера
      // как это лучше всего сделать?
      // советуют использовать splitmix64
      // но с чем его использовать не говорят
      // мне нужно придумать как получить сид 
      // по идее нужно задать некий игровой сид и от него отталкиваться
      // с другой стороны у меня локальные сиды на всех персах
      // да но они не будут меняться если я начинаю на тойже карте по нескольку раз
      // поэтому нужно придумать стейт глобальный
      const size_t state1 = global::advance_state();
      const size_t state2 = global::advance_state();
      c->rng_state = {state1, state2};
    }
  }
  
  template <typename T>
  void parse_entities(const std::function<void(T*, const sol::table&)> &parsing_func) {
    auto tables = global::get<utils::table_container>();
    auto ctx = global::get<core::context>();
    const auto &data = tables->get_tables(T::s_type);
    for (size_t i = 0; i < data.size(); ++i) {
      auto ptr = ctx->get_entity<T>(i);
      parsing_func(ptr, data[i]);
    }
  }
  
  void parse_characters(const std::function<void(core::character*, const sol::table&)> &parsing_func) {
    auto tables = global::get<utils::table_container>();
    auto ctx = global::get<core::context>();
    
    const auto &data = tables->get_tables(core::character::s_type);
    for (size_t i = 0; i < data.size(); ++i) {
      auto ptr = ctx->get_character(i);
      parsing_func(ptr, data[i]);
    }
  }
  
  void validate_and_create_data(map::creator* creator, system_container_t &systems) {
    systems.core_context = systems.container.create<core::context>();
    global::get(systems.core_context);
    systems.string_container = systems.container.create<utils::data_string_container>();
    global::get(systems.string_container);
    
    utils::world_serializator cont;
    const std::string_view test_name = "Test world 1\0";
    const std::string_view test_tname = "test_world_1\0";
    const std::string_view test_settings = "{}\0";
    cont.set_name(test_name);
    cont.set_technical_name(test_tname);
    cont.set_settings(test_settings);
    cont.set_seed(1);
    
    const std::function<bool(const size_t &, sol::this_state, const sol::table&, utils::world_serializator*)> validation_funcs[] = {
      nullptr,                   // tile    : нужна ли тайлу валидация? я не уверен что хорошей идеей будет использовать луа таблицы для заполнения тайла
      utils::validate_province_and_save,  // province
      utils::validate_building_and_save,  // building_type,
      utils::validate_city_type_and_save, // city_type,
      utils::validate_city_and_save,      // city,
      nullptr,                   // trait,
      nullptr,                   // modificator,
      nullptr,                   // troop_type,
      nullptr,                   // decision,
      nullptr,                   // religion_group,
      nullptr,                   // religion,
      nullptr,                   // culture,
      nullptr,                   // law,
      nullptr,                   // event,
      utils::validate_title_and_save,     // titulus,
      utils::validate_character_and_save, // character,
      nullptr,                   // dynasty,
      nullptr,                   // faction,    // это и далее делать не нужно по идее
      nullptr,                   // hero_troop, 
      nullptr,                   // army,       
      
    };
    
    global::get<utils::calendar>()->validate();
    
    auto &lua = creator->state();
    //ASSERT(lua != nullptr);
    
    auto &tables = creator->table_container();
    const size_t count = static_cast<size_t>(core::structure::count);
    bool ret = true;
    for (size_t i = 0; i < count; ++i) {
      if (!validation_funcs[i]) continue;
      const auto &data = tables.get_tables(static_cast<core::structure>(i));
      size_t counter = 0;
      for (const auto &table : data) {
        ret = ret && validation_funcs[i](counter, lua.lua_state(), table, &cont);
        ++counter;
      }
    }
    
    if (!ret) throw std::runtime_error("There is validation errors");
    
    // нужно собрать инфу о дубликатах
    create_entities_without_id<core::province>();
    create_entities<core::building_type>();
    create_entities<core::city_type>();
    create_entities_without_id<core::city>();
    create_entities<core::titulus>();
    create_characters();
    
    parse_entities<core::titulus>(utils::parse_title);
    parse_entities<core::province>(utils::parse_province);
    parse_entities<core::building_type>(utils::parse_building);
    parse_entities<core::city_type>(utils::parse_city_type);
    parse_entities<core::city>(utils::parse_city);
    parse_characters(utils::parse_character);
    parse_characters(utils::parse_character_goverment);
    // тут нужно еще соединить все полученные данные друг с другом
    connect_game_data(systems.map, systems.core_context);
    
    // по идее в этой точке все игровые объекты созданы
    // и можно непосредственно переходить к геймплею
    // если валидация и парсинг успешны это повод сохранить мир на диск
    // это означает: сериализация данных карты + записать на диск все таблицы + сериализация персонажей и династий (первых)
    // могу ли я сериализовать конкретные типы? скорее да чем нет, 
    // но при этом мне придется делать отдельный сериализатор для каждого типа
    // понятное дело делать отдельный сериализатор не сруки
    
    for (size_t i = 0; i < core::map::hex_count_d(core::map::detail_level); ++i) {
      const auto &d = systems.core_context->get_tile(i);
      cont.set_tile_data(i, {d.height, d.province});
    }
    
    cont.serialize(); // 
    
    utils::world_serializator test;
    test.deserialize(global::root_directory() + "saves/test_world_1/world_data");
    
    ASSERT(test.get_name() == test_name);
    ASSERT(test.get_technical_name() == test_tname);
    ASSERT(test.get_settings() == test_settings);
    ASSERT(test.get_seed() == 1);
    
    ASSERT(tables.get_tables(core::structure::province).size() == cont.get_data_count(core::structure::province));
    ASSERT(tables.get_tables(core::structure::city_type).size() == cont.get_data_count(core::structure::city_type));
    ASSERT(tables.get_tables(core::structure::city).size() == cont.get_data_count(core::structure::city));
    ASSERT(tables.get_tables(core::structure::building_type).size() == cont.get_data_count(core::structure::building_type));
    ASSERT(tables.get_tables(core::structure::titulus).size() == cont.get_data_count(core::structure::titulus));
    ASSERT(tables.get_tables(core::structure::character).size() == cont.get_data_count(core::structure::character));
    
    ASSERT(cont.get_data_count(core::structure::province) == test.get_data_count(core::structure::province));
    ASSERT(cont.get_data_count(core::structure::city_type) == test.get_data_count(core::structure::city_type));
    ASSERT(cont.get_data_count(core::structure::city) == test.get_data_count(core::structure::city));
    ASSERT(cont.get_data_count(core::structure::building_type) == test.get_data_count(core::structure::building_type));
    ASSERT(cont.get_data_count(core::structure::titulus) == test.get_data_count(core::structure::titulus));
    ASSERT(cont.get_data_count(core::structure::character) == test.get_data_count(core::structure::character));
    
#define CHECK_CONTENT(var) for (uint32_t i = 0; i < cont.get_data_count(var); ++i) { ASSERT(cont.get_data(var, i) == test.get_data(var, i)); }
    CHECK_CONTENT(core::structure::province)
    CHECK_CONTENT(core::structure::city_type)
    CHECK_CONTENT(core::structure::city)
    CHECK_CONTENT(core::structure::building_type)
    CHECK_CONTENT(core::structure::titulus)
    CHECK_CONTENT(core::structure::character)
    
    // кажется все правильно сериализуется и десериализуется
    // сохранения должны храниться в папках с файлом world_data
    // название папки - техническое название мира, 
    // техническое название мира - нужно зафорсить пользователя задать валидное техническое название 
    // (какнибудь бы упростить для пользователя это дело)
    // что самое главное я могу оставить пока так как есть и это покроет почти всю сериализацию
    // остается решить вопрос с климатом, хотя че тут решать: нужно выделить
    // 64*500к или 128*500к памяти чтобы сохранить сезоны, сезонов по идее не имеет смысла делать больше чем размер массива на каждый тайл
    // соответственно 64 сезона на каждый тайл, означает ли это что мы можем сохранить 4 тайла в одном чаре?
    // в сохранениях видимо придется делать протомессадж для каждой сущности которую необходимо сохранить
    // мне нужно еще придумать стендалоне сохранения: то есть сохранения в которых записаны данные мира дополнительно
    // 
  }
  
  void create_interface(system_container_t &systems) {
    systems.interface = systems.container.create<utils::interface>();
    global::get(systems.interface);
    systems.interface->init_constants();
    systems.interface->init_input();
  }
  
  void post_generation_work(map::creator* creator, system_container_t &systems) {
    create_game_state();
    generate_tile_connections(global::get<core::map>(), global::get<dt::thread_pool>());
    validate_and_create_data(creator, systems); // создаем объекты
    find_border_points(global::get<core::map>()); // после генерации нужно сделать много вещей
    // по идее создать границы нужно сейчас, так как в титулах появились данные о цвете
    
    //create_interface(systems);
    systems.interface->init_types();
    systems.interface->init_game_logic();
    auto &lua = systems.interface->get_state();
    load_interface_functions(systems.interface, lua);
    create_ai_systems(systems);
    // нужно выбрать себе какого нибудь персонажа
    // кажется у меня сейчас все персонажи живы, так что можно любого
    auto ctx = global::get<core::context>();
    const size_t chars_count = ctx->characters_count();
    utils::rng::state s = {67586, 987699695};
    s = utils::rng::next(s);
    const double val = utils::rng::normalize(utils::rng::value(s));
    const size_t index = chars_count * val;
    auto c = ctx->get_character(index);
    c->make_player();
    game::update_player(c);
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
    
    game::advance_state();
    
    // здесь рид онли стейт
    global::get<utils::interface>()->draw(time); // весь интерфейс рисуем здесь
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
      global::get<systems::render>()->clear();
      auto s = global::get<core::map>()->status();
      if (s == core::map::status::rendering) global::get<core::map>()->set_status(core::map::status::valid);
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

