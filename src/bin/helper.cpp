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

    std::vector<char> array(length+1);
    wai_getExecutablePath(array.data(), length, &dirname);

    array[length] = '\0'; // весь путь

//     std::cout << array << "\n";

    array[dirname+1] = '\0'; // путь до папки

//     std::cout << array << "\n";

    std::filesystem::path p = std::string(array.data());
    p /= "../";
    p.make_preferred();
    const std::string dir_path = p.string();
//     std::cout << dir_path << "\n";

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
    
    const utils::id border_render = utils::id::get("border_render");
    input::set_key(GLFW_KEY_B, border_render);
    
    const utils::id go_to_capital = utils::id::get("home");
    input::set_key(GLFW_KEY_HOME, go_to_capital);
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
  
  // мне нужно сделать ограничение движения мышки за пределы окна
  
  bool camera_movement(components::camera* camera, const size_t &time, const double &xpos, const double &ypos) {
    auto ctx = global::get<interface::context>();
    auto settings = global::get<utils::settings>();
    
    static bool was_pressed_on_map = false;
    static bool was_pressed = false;

    static double last_xpos = 0.0, last_ypos = 0.0;
    double delta_xpos = last_xpos - xpos, delta_ypos = last_ypos - ypos;
    last_xpos = xpos;
    last_ypos = ypos;
    
    const bool changed = !(last_xpos == xpos && last_ypos == ypos);

    const float zoom = camera->zoom();
    const float zoom_k = zoom / camera->max_zoom();
    const float sens = settings->game.camera_movement * (1.0f + zoom_k * 0.5f);
    const float x_sens = settings->game.camera_movement_x;
    const float y_sens = settings->game.camera_movement_x;
    const float x_move = sens * x_sens * MCS_TO_SEC(time) * delta_xpos;
    const float y_move = sens * y_sens * MCS_TO_SEC(time) * delta_ypos;

    static const utils::id map_move = utils::id::get("map_move");
    const bool current_pressed = input::is_event_pressed(map_move);
    if (current_pressed && !was_pressed) {
      was_pressed_on_map = !nk_window_is_any_hovered(&ctx->ctx);
    }

    if (!changed && current_pressed && was_pressed_on_map) {
      camera->move(x_move, y_move);
    }

    was_pressed = current_pressed;
    if (!current_pressed) was_pressed_on_map = false;
    
    return current_pressed;
  }
  
//   void find_map_points(const render::buffers* buffers, const components::camera* camera, glm::vec4 &first_point, glm::vec4 &first_point_height) {
//     const auto camera_pos = buffers->get_pos();
//     const auto cursor_dir = buffers->get_cursor_dir();
//     const auto camera_dir = buffers->get_dir();
//     
//     // мы выделяем боксом и на выпуклой сфере есть шанс пройти ниже земли боксом
//     // соответсвенно нужно его увеличить (есть ли в этом случае шанс захватить лишние объекты? наверное)
//     
//     const float minimum_dist = 100.0f;
//     const float maximum_dist = 256.0f;
//     
//     const float raw_zoom = camera->zoom();
//     const float zoom_norm = (raw_zoom - camera->min_zoom()) / (camera->max_zoom() - camera->min_zoom());
//     
//     const float final_dist = glm::mix(minimum_dist, maximum_dist, zoom_norm);
//     
//     float hit = hit_sphere(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), core::map::world_radius, {camera_pos, cursor_dir});
//     const bool ray_hit_sphere = hit >= 0.0f;
//     if (!ray_hit_sphere) hit = final_dist;
//     
//     first_point = camera_pos + cursor_dir * hit;
//     first_point_height = ray_hit_sphere ? 
//       first_point + (-camera_dir) * (components::camera::minimum_camera_height + 1.0f) : 
//       first_point + (-camera_dir) * (final_dist * 0.75f);
//     // че делать с ситуацией когда мы можем выделить два раза области вне сферы? по идее просто умножить на какой то коэффициент
//   }
  
  glm::vec4 find_triangle_normal(const glm::vec4 &p1, const glm::vec4 &p2, const glm::vec4 &p3) {
    const glm::vec4 u_vec = p2 - p1;
    const glm::vec4 v_vec = p3 - p1;
    
    const glm::vec3 norm = glm::normalize(glm::cross(glm::vec3(u_vec), glm::vec3(v_vec)));
    return glm::vec4(norm, 0.0f);
  }
  
  bool frustum_test_123(const render::frustum_t frustum, const glm::vec4 center, const float radius) {
    PRINT("start")
    bool result = true;
    for (uint32_t i = 0; i < 6; ++i) {
      const float d = glm::dot(center, frustum.planes[i]);
      const bool res = !(d <= -radius);
      result = bool(glm::min(uint32_t(result), uint32_t(res)));
      
      PRINT_VEC4("center", center)
      PRINT_VEC4("norm  ", frustum.planes[i])
      PRINT_VAR( "d", d)
      PRINT("\n")
    }
    
    return result;
  }
  
  static const utils::frustum default_fru{
    glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), 
    glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), 
    glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), 
    glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), 
    glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), 
    glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)
  };
  
  bool map_selection(components::camera* camera, const size_t &time, const double &xpos, const double &ypos) {
    auto ctx = global::get<interface::context>();
    auto buffers = global::get<render::buffers>();
    auto opt = global::get<render::tile_optimizer>();
    auto window = global::get<render::window>();
    auto battle_opt = global::get<render::battle::tile_optimizer>();
    if (battle_opt == nullptr && opt == nullptr) return false;
    
    // сначала проверить кликаем ли мы по карте или по интерфейсу
    // (или вообще мимо карты)
    
    static bool was_pressed = false;
    static double prev_xpos = -1.0, prev_ypos = -1.0; // тут поди нужно запомнить все же положение 
//     static glm::vec4 first_point, first_point_height;
    
    static const utils::id activate_click = utils::id::get("activate_click");
    //const bool current_pressed = input::check_event(activate_click, input::state_press | input::state_double_press | input::state_long_press);
    const bool current_pressed = input::is_event_pressed(activate_click);
    
    if (!was_pressed && !current_pressed) return false;
    
    if (!was_pressed && current_pressed) {
      const bool hovered = nk_window_is_any_hovered(&ctx->ctx);
      if (hovered) return true;
      
      // находим первую точку
//       find_map_points(buffers, camera, first_point, first_point_height);
      
      prev_xpos = xpos;
      prev_ypos = ypos;
      was_pressed = current_pressed;
      
//       global::get<utils::objects_selector>()->clear();
      
      return true;
    }
    
    // находим вторую точку и собственно бокс
    // где то еще нужно нарисовать область выделения
    // возможно прямо тут
    
//     glm::vec4 second_point;
//     glm::vec4 second_point_height;
//     find_map_points(buffers, camera, second_point, second_point_height);
// //     const glm::vec4 second_dir = buffers->get_cursor_dir();
//       
//     const std::initializer_list<glm::vec4> point_arr = {
//       first_point, first_point_height, second_point, second_point_height
//     };
//     
//     glm::vec4 min = first_point, max = first_point;
//     for (uint32_t i = 1; i < point_arr.size(); ++i) {
//       min = glm::min(min, point_arr.begin()[i]);
//       max = glm::max(max, point_arr.begin()[i]);
//     }
//     
//     // куда мы это дело спихнем?
//     // можно в принципе в камеру, только это особо нигде больше не нужно
//     // еще бы сделать минимальный бокс (хотя насколько это вообще нужно?)
//     
// //     const auto center =         (max + min) / 2.0f;
// //     const auto extent = glm::abs(max - min) / 2.0f;
// //     const auto final_extent = glm::max(extent, glm::vec4(0.2f, 0.2f, 0.2f, 0.0f));
// //     
// //     min = center - final_extent;
// //     max = center + final_extent;
//     
//     opt->set_selection_box({min, max});
    
    const glm::dvec2 screen_min = glm::min(glm::dvec2(prev_xpos, prev_ypos), glm::dvec2(glm::max(xpos, 0.0), glm::max(ypos, 0.0)));
    const glm::dvec2 screen_max = glm::max(glm::dvec2(prev_xpos, prev_ypos), glm::dvec2(glm::max(xpos, 0.0), glm::max(ypos, 0.0)));
    const glm::dvec2 size = screen_max - screen_min;
    const glm::dvec2 min_size = glm::max(size, glm::dvec2(1, 1));
    const glm::dvec2 final_screen_max = screen_min + min_size;
    
    const glm::vec4 dirs[] = {
      render::get_cursor_dir(buffers, window, screen_min.x, screen_min.y),
      render::get_cursor_dir(buffers, window, final_screen_max.x, screen_min.y),
      render::get_cursor_dir(buffers, window, screen_min.x, final_screen_max.y),
      render::get_cursor_dir(buffers, window, final_screen_max.x, final_screen_max.y)
    };
    
    const float minimum_dist = MINIMUM_FRUSTUM_DIST;
    const float maximum_dist = MAXIMUM_FRUSTUM_DIST;
    
    const float raw_zoom = camera->zoom();
    const float zoom_norm = (raw_zoom - camera->min_zoom()) / (camera->max_zoom() - camera->min_zoom());
    
    const float final_dist = glm::mix(minimum_dist, maximum_dist, zoom_norm);
    const float near = 1.0f;
    
    const glm::vec4 camera_pos = buffers->get_pos();
    const glm::vec4 camera_dir = buffers->get_dir();
    
    const glm::vec4 points[] = {
      camera_pos + dirs[0] * near,       // 0
      camera_pos + dirs[1] * near,       // 1
      camera_pos + dirs[2] * near,       // 2
      camera_pos + dirs[3] * near,       // 3
      camera_pos + dirs[0] * final_dist, // 4
      camera_pos + dirs[1] * final_dist, // 5
      camera_pos + dirs[2] * final_dist, // 6
      camera_pos + dirs[3] * final_dist  // 7
    };
    
    // находим 6 плоскостей
    glm::vec4 normals[] = {
      //-find_triangle_normal(points[0], points[1], points[2]), // ближайшая
      camera_dir,
      find_triangle_normal(points[0], points[1], points[4]), // верх
      find_triangle_normal(points[0], points[4], points[6]), // лево
      find_triangle_normal(points[7], points[5], points[3]), // право
      find_triangle_normal(points[7], points[3], points[6]), // низ
      //-find_triangle_normal(points[7], points[5], points[6]), // дальняя
      -camera_dir
    };
    
    ASSERT(glm::dot(camera_dir, normals[0]) >  EPSILON);
    ASSERT(glm::dot(camera_dir, normals[5]) < -EPSILON);
    ASSERT(glm::dot(normals[0], normals[5]) < -EPSILON);
//     ASSERT(glm::dot(normals[1], normals[4]) < -EPSILON); // тут может получиться очень большой угол
//     ASSERT(glm::dot(normals[2], normals[3]) < -EPSILON);
    //ASSERT(glm::dot(normals[4], glm::vec4(0.0f, 1.0f, 0.0f, 0.0f)) < -EPSILON);
    
    //normals[0].w = -glm::dot(normals[0], points[0]);
    normals[0].w = -glm::dot(normals[0], camera_pos + camera_dir * near);
    normals[1].w = -glm::dot(normals[1], points[0]);
    normals[2].w = -glm::dot(normals[2], points[0]);
    normals[3].w = -glm::dot(normals[3], points[7]);
    normals[4].w = -glm::dot(normals[4], points[7]);
    //normals[5].w = -glm::dot(normals[5], points[7]);
    normals[5].w = -glm::dot(normals[5], camera_pos + camera_dir * final_dist);
    
    //assert(false);
    
    const utils::frustum fru{normals[0], normals[1], normals[2], normals[3], normals[4], normals[5]};
    if (battle_opt != nullptr) battle_opt->update_selection_frustum(fru);
    if (opt != nullptr) opt->set_selection_frustum(fru);
    
//     global::get<utils::objects_selector>()->clear();
    
//     render::frustum_t test_fru;
//     memcpy(test_fru.planes, fru.planes, sizeof(test_fru));
//     
//     const bool ret1 = frustum_test_123(test_fru, glm::vec4(-378.334, -96.6914, -313.912, 1.0f), 1.0f);
// //     const bool ret2 = frustum_test_123(test_fru, glm::vec4(-387.537, -98.2722, -301.987, 1.0f), 1.0f);
//     
//     //army1 x: -378.334 y: -96.6914 z: -313.912
//     //army2 x: -387.537 y: -98.2722 z: -301.987
//     
//     if (ret1) PRINT("Testing obj 1")
//     if (ret2) PRINT("Testing obj 2")
    
    interface::style::background_color color(&ctx->ctx, nk_color{nk_byte(255.0f*0.1f), nk_byte(255.0f*0.1f), nk_byte(255.0f*0.6f), nk_byte(255.0f*0.5f)});
    if (nk_begin(&ctx->ctx, "screen_selection_box", nk_rect(screen_min.x, screen_min.y, min_size.x, min_size.y), NK_WINDOW_BORDER)) {
      // здесь можно добавить какую-нибудь строку, но необязательно
    }
    nk_end(&ctx->ctx);
    
    if (was_pressed && !current_pressed) {
      // здесь заканчиваем выделение
      
      prev_xpos = -1.0;
      prev_ypos = -1.0;
      was_pressed = current_pressed;
    }
    
    (void)time;
    
    // я понял что нужно еще добавить контрол и шифт поведения
    // контрол для исключения выделения, шифт для включения в выделение
    // для этого видимо нужно сделать еще один буффер с выделением
    
    return current_pressed;
  }
  
  bool map_action(const uint32_t &casted_tile_index) {
    return false;
//     // тут нужно получить тайл рейкастингом
//     auto selector = global::get<systems::core_t>()->objects_selector; // селектора поди будет два
//     auto ctx = global::get<systems::map_t>()->core_context;
//     auto map = global::get<systems::map_t>()->map;
//     auto path_seaker = global::get<systems::core_t>()->path_managment;
//     auto pool = global::get<dt::thread_pool>();
//     
//     static bool was_pressed = false;
//     
//     static const utils::id control_click = utils::id::get("control_click");
//     const bool current_pressed = input::is_event_pressed(control_click);
//     const bool is_clicked = was_pressed && !current_pressed;
//     const bool is_not_pressed = !was_pressed && !current_pressed;
//     was_pressed = current_pressed;
//     
//     if (is_not_pressed) return false;
//     
//     static uint32_t end_tile = UINT32_MAX;
//     const bool new_tile = end_tile != casted_tile_index;
//     end_tile = casted_tile_index;
//     
//     // так же нам нужно сделать перемещение армий по второму клику
//     // перемещение армии добавляет некоторых проблем, например несколько армий не могут находиться в одном тайле
//     // для этого видимо нужно проверить все тайлы вокруг? + ко всему выделение не должно поменяться
//     // нужно разделить два клика, первый клик стартует задачу поиска, второй клик по тому же тайлу с тем же выделением
//     // мы должны переместить армии на выбранный тайл 
//     // (причем на мобиле отличается тем что нужно выбрать тайл не отрывая палец от экрана, а затем еще раз кликнуть по тайлу) 
//     // (хотя с другой стороны есть еще несколько способов огранизовать все это дело (например выбрать, по клику выбирать тайл, а отменять выбор по интерфейсу))
//     
//     if (selector->count != 0) {
//       // тут нужно найти тех для кого мы ищем путь
//       if (selector->count == 1 && selector->units[0].type == utils::objects_selector::unit::type::city) {
//         // мы не можем ничего найти для города
//         return false;
//       }
//       
//       if (new_tile) {
//         const auto& tile_data = render::unpack_data(map->get_tile(casted_tile_index));
//         if (tile_data.height < 0.0f) return current_pressed;
//         if (tile_data.height > 0.5f) return current_pressed;
// 
//         for (uint32_t i = 0; i < selector->count; ++i) {
//           const auto &unit = selector->units[i];
//           if (unit.type == utils::objects_selector::unit::type::army) {
//             core::army* army = ctx->get_army(unit.index);
//             path_seaker->find_path(army, army->tile_index, casted_tile_index);
//           } else {
//             
//           }
//         }
//         
//         selector->clicks = 0;
//       }
//       
//       selector->clicks += uint32_t(is_clicked);
//       
//       if (selector->clicks == 2) {
//         // перемещаем войска на дальность хода по пути, но пока у меня нет никаких ходов...
//         // нужно еще как то выделить дальность хода, + сделать так чтобы при кончине хода я обходил армии игрока и двигал их если путь найден
//         // тут бы сделать какой нибудь интерфейс который подойдет для всех, тут нужен какой то интерфейс для анимаций
//         // а пока можно просто сделать функцию
// //         std::vector<utils::objects_selector::unit> copy;
// //         selector->copy(copy);
//         
//         pool->compute();
//         pool->wait();
//         
// //         pool->submitbase([copy = selector->copy(), ctx] () {
//         const auto copy = selector->copy();
//           for (uint32_t i = 0; i < copy.size(); ++i) {
//             const auto &unit = copy[i];
//             if (unit.type == utils::objects_selector::unit::type::army) {
//               core::army* army = ctx->get_army(unit.index);
//               advance_army(army);
//             } else {
//               
//             }
//           }
// //         });
//         
// //         for (uint32_t i = 0; i < selector->count; ++i) {
// //           const auto &unit = selector->units[i];
// //           if (unit.type == utils::objects_selector::unit::type::army) {
// //             core::army* army = ctx->get_army(unit.index);
// //             advance_army(army);
// //           } else {
// //             
// //           }
// //         }
//       }
//       
//       return current_pressed;
//     }
//     
//     // тут нужно вызвать какое то действие, тип например открыть дипломатию или еще что
//     return current_pressed;
  }
  
  void classic_strategic_camera_movement(components::camera* camera, const size_t &time, const double &xpos, const double &ypos) {
    // какой размер рамки перемещения? может ли он отличаться от размеров окна? одинаковые ли размеры по координатам?
    // сейчас пока прикинем рамку
    auto settings = global::get<utils::settings>();
    auto window = global::get<render::window>();
    const auto [w,h] = window->framebuffer_size();
    const glm::uvec2 dim = glm::uvec2(w, h);
    const glm::dvec2 cursor_pos = glm::dvec2(xpos / double(dim.x), ypos / double(dim.y));
    //const glm::dvec2 movement_border = glm::dvec2(20.0 / double(dim.x), 20.0 / double(dim.y));
    const glm::dvec2 movement_border = glm::dvec2(0.1, 0.1);
    
    const bool square1 = cursor_pos.x <  0.5 && cursor_pos.y  < 0.5;
    const bool square2 = cursor_pos.x >= 0.5 && cursor_pos.y  < 0.5;
    const bool square3 = cursor_pos.x <  0.5 && cursor_pos.y >= 0.5;
    const bool square4 = cursor_pos.x >= 0.5 && cursor_pos.y >= 0.5;
    
    uint8_t axis_plus = !square1 ? 0         : (    cursor_pos.x <     cursor_pos.y ? 0 : 1);
            axis_plus = !square2 ? axis_plus : (1.0-cursor_pos.x <     cursor_pos.y ? 2 : 1);
            axis_plus = !square3 ? axis_plus : (    cursor_pos.x < 1.0-cursor_pos.y ? 0 : 3);
            axis_plus = !square4 ? axis_plus : (1.0-cursor_pos.x < 1.0-cursor_pos.y ? 2 : 3);
            
    //const uint8_t axis_plus = xpos < 0.5 ? (xpos < ypos ? 0 : 1) : (xpos < ypos ? 3 : 2);
    const uint8_t axis = axis_plus % 2;
    
    if (cursor_pos[axis] > movement_border[axis] && cursor_pos[axis] < 1.0-movement_border[axis]) return;
    
    // сенсу нужно сильно увеличить, и нужно ограничить угол подъема камеры
    const float zoom = camera->zoom();
    const float zoom_k = (zoom - camera->min_zoom()) / (camera->max_zoom() - camera->min_zoom());
    const float sens = settings->game.camera_movement * (1.0f + zoom_k * 0.5f);
    const float x_sens = settings->game.camera_movement_x;
    const float y_sens = settings->game.camera_movement_x;
    const float x_move = sens * x_sens * MCS_TO_SEC(time) * ((-1.0f)*(axis_plus == 0) + (axis_plus == 2));
    const float y_move = sens * y_sens * MCS_TO_SEC(time) * ((-1.0f)*(axis_plus == 1) + (axis_plus == 3));
    
    camera->move(x_move, y_move);
  }
  
  // сюда нужно передать индекс тайла выделяемого сейчас мышкой
  void mouse_input(components::camera* camera, const size_t &time, const uint32_t &casted_tile_index) {
    if (camera == nullptr) return;
    auto window = global::get<render::window>();
//     auto ctx = global::get<interface::context>();
//     auto buffers = global::get<render::buffers>();
    //static bool window_was_focused = true;
    static bool lock_mouse = true;
    lock_mouse = window->flags.focused() ? lock_mouse : false;
    
    if (!window->flags.focused()) return;

    // если мы изначально нажали на карту то тогда двигаем
    // иначе ничего не делаем
    
    double xpos, ypos;
    glfwGetCursorPos(window->handle, &xpos, &ypos);
    
//     if (!lock_mouse) { // cursor_in_window
      const auto &keys = input::get_input_data()->container;
      for (uint32_t i = GLFW_MOUSE_BUTTON_1; i <= GLFW_MOUSE_BUTTON_LAST; ++i) {
        lock_mouse = lock_mouse || (keys[i].event != input::release);
      }
//     }
    
    const auto [w,h] = window->framebuffer_size();
    // нужно поставить ограничения на выход за пределы окна, но при этом нужно сделать это настраиваемым
    // причем желательно сделать отрисовку курсора в самом рендере
    if (lock_mouse) {
      //ASSERT(false);
      xpos = glm::max(glm::min(xpos, double(w)), 0.0);
      ypos = glm::max(glm::min(ypos, double(h)), 0.0);
      glfwSetCursorPos(window->handle, xpos, ypos);
      //glfwSetWindowAttrib(window->handle, GLFW);
    }
    
    // на пару пикселей выходит за пределы окна
    //const bool cursor_in_window = xpos > 0.0 && ypos > 0.0 && xpos < size.width && ypos < size.height;
    //if (!cursor_in_window) return;
    
    const bool pressed1 = camera_movement(camera, time, xpos, ypos);
    const bool pressed2 = map_selection(camera, time, xpos, ypos);
    // теперь нам нужно выбрать тайл в который мы хотим пойти, либо какой то другое действие если у нас нет выделения (или выделение города)
    const bool pressed3 = map_action(casted_tile_index);
    
    if (pressed1 || pressed2 || pressed3) return;
    classic_strategic_camera_movement(camera, time, xpos, ypos);
    
    // причем на мобиле будет наоборот: если курсор тыкается в края, то не нужно делать селектион и мувмент
    // наверное этот код лучше бы написать на луа (?)
  }

  void key_input(const size_t &time, const uint32_t &current_state, const bool loading) {
    (void)time;
    (void)loading;
    
//     static const utils::id modes[] = {
//       utils::id::get("biome_render_mode"),
//       utils::id::get("cultures_render_mode"),
//       utils::id::get("culture_groups_render_mode"),
//       utils::id::get("religions_render_mode"),
//       utils::id::get("religion_groups_render_mode"),
//       utils::id::get("provinces_render_mode"),
//       utils::id::get("countries_render_mode"),
//       utils::id::get("duchies_render_mode"),
//       utils::id::get("kingdoms_render_mode"),
//       utils::id::get("empires_render_mode"),
//     };
// 
//     static_assert(sizeof(modes) / sizeof(modes[0]) == render::modes::count);

    //static const utils::id escape = utils::id::get("escape");
    static const utils::id border_render = utils::id::get("border_render");
    static const utils::id go_to_capital = utils::id::get("home");

//     auto map = global::get<core::map>();
//     auto container = global::get<map::generator::container>();

    auto s = global::get<systems::core_t>();
//     const bool get_menu = current_state != utils::quest_state::map_creation &&
//                           //current_state != game_state::loading &&
//                           input::check_key(GLFW_KEY_ESCAPE, input::state::state_click | input::state::state_double_click | input::state::state_long_click);
//     const bool last_menu = current_state == utils::quest_state::main_menu && s->menu->menu_stack.size() == 1;
//     if (!loading && get_menu) {
//       if (s->interface->escape()) return;
//       if (!s->menu->exist()) {
//         if (current_state == utils::quest_state::main_menu) s->menu->push("main_menu");
//         else s->menu->push("main_menu_map");
//       } else if (!last_menu) s->menu->escape();
//       return;
//     }

//     if (current_state == utils::quest_state::world_map) {
      size_t mem = 0;
      auto change = input::next_input_event(mem, 1);
      while (change.id.valid()) {
        auto local_copy = change;
        change = input::next_input_event(mem, 1);
        
        //PRINT_VAR("change id", change.id.name())
        if (local_copy.event != input::release) {
          if (local_copy.id == border_render) {
            PRINT(border_render.name())
            auto t = global::get<render::tile_optimizer>();
            t->set_border_rendering(!t->is_rendering_border());
            continue;
          }
          
          //PRINT(change.id.name())
          
          if (local_copy.id == go_to_capital) {
//             PRINT(local_copy.id.name())
            //auto camera = global::get<components::camera>();
            auto camera = camera::get_camera();
            auto p = game::get_player();
            ASSERT(p != nullptr);
            //if (p == nullptr) continue;
            auto title = p->self->titles;
            auto next_title = title;
            while (next_title != nullptr) {
              if (next_title->type() == core::titulus::type::city) break;
              next_title = utils::ring::list_next<utils::list_type::titles>(next_title, title);
            }
            
            ASSERT(title != nullptr);
            
            auto city = title->city;
            auto map = global::get<systems::map_t>()->map;
            auto ctx = global::get<systems::map_t>()->core_context;
            const uint32_t point_index = ctx->get_entity<core::tile>(city->tile_index)->center;
            const auto point = map->get_point(point_index);
            camera->set_end_point(glm::vec3(point));
            continue;
          }

//           for (size_t i = 0; i < render::modes::count; ++i) {
//             if (local_copy.id != modes[i]) continue;
//             render::mode(static_cast<render::modes::values>(i));
//             break;
//           }
        }
      }
//     }
    
    if (current_state == utils::quest_state::battle_map) {
//       size_t mem = 0;
//       auto change = input::next_input_event(mem, 1);
//       while (change.id.valid()) {
//         auto local_copy = change;
//         change = input::next_input_event(mem, 1);
//         
//         if (local_copy.event != input::release) {
//           if (local_copy.id == go_to_capital) {
//             // тут мы переместимся к своей армии
//           }
//         }
//       }
    }
    
    if (current_state == utils::quest_state::encounter) {
      
    }
  }

  void zoom_input(components::camera* camera) {
    if (camera == nullptr) return;
    auto input_data = input::get_input_data();
    auto ctx = global::get<interface::context>();
    const bool window_focus = nk_window_is_any_hovered(&ctx->ctx);
    if (window_focus) return;
    if (input_data->mouse_wheel == 0.0f) return;

    // зум нужно ограничить когда мы выделяем хотя бы одно наклир окно

    camera->zoom_add(input_data->mouse_wheel);
    input_data->mouse_wheel = 0.0f;
  }
  
  glm::vec4 get_cursor_dir(render::buffers* buffers, render::window* window, const double xpos, const double ypos) {
//     const glm::vec4 camera_dir = buffers->get_dir();

    const glm::mat4 inv_proj = buffers->get_inv_proj();
    const glm::mat4 inv_view = buffers->get_inv_view();
    
    const auto [w,h] = window->framebuffer_size();
    const float x = (2.0f * xpos) /  float(w) - 1.0f;
    const float y = 1.0f - (2.0f * ypos) / float(h); // тут по идее должно быть обратное значение
    const float z = 1.0f;

    ASSERT(x >= -1.0f && x <= 1.0f);
    ASSERT(y >= -1.0f && y <= 1.0f);

    glm::vec3 ray_nds = glm::vec3(x, -y, z);
    glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);
    glm::vec4 ray_eye = inv_proj * ray_clip;
    ray_eye = glm::vec4(glm::vec2(ray_eye), -1.0, 0.0);
    glm::vec3 ray_wor = glm::vec3(inv_view * ray_eye);
    // don't forget to normalise the vector at some point
    const glm::vec4 dir = glm::vec4(glm::normalize(ray_wor), 0.0f);
    //const glm::vec4 final_dir = camera_dir;
    return dir;
  }
  
  std::tuple<glm::vec4, glm::vec4> get_up_right_vectors(const glm::vec4 &forw) {
    
  }

  uint32_t cast_mouse_ray(float &ray_dist) {
    auto window = global::get<render::window>();
    auto buffers = global::get<render::buffers>();

    double xpos, ypos;
    glfwGetCursorPos(window->handle, &xpos, &ypos);
    
    buffers->update_cursor_dir(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
    
    const auto [w,h] = window->framebuffer_size();
    if (!window->flags.focused()) return UINT32_MAX;
    if (xpos > w) return UINT32_MAX;
    if (ypos > h) return UINT32_MAX;
    if (xpos < 0.0) return UINT32_MAX;
    if (ypos < 0.0) return UINT32_MAX;

    const glm::vec4 camera_pos = buffers->get_pos();
    const glm::vec4 final_dir = render::get_cursor_dir(buffers, window, xpos, ypos);
    
    buffers->update_cursor_dir(final_dir);

    const utils::ray intersect_sphere{
      camera_pos,
      final_dir
    };
    
    auto map_system = global::get<systems::map_t>();
    if (!map_system->is_init()) return UINT32_MAX;
    if (map_system->map->status() != core::map::status::valid) return UINT32_MAX;
    //const uint32_t tile_index_local1 = map_system->map->cast_ray(intersect_sphere, ray_dist);
    // в контексте не всегда находятся тайлы, как мы можем это определить?
    uint32_t tile_index_local1;
    if (map_system->map->is_tile_data_on_gpu()) {
      tile_index_local1 = map_system->core_context->cast_ray(map_system->map, intersect_sphere, ray_dist);
    } else {
      tile_index_local1 = map_system->map->cast_ray(intersect_sphere, ray_dist);
    }

    return tile_index_local1;
  }
  
  union convert_int_unit_float {
    uint32_t ui;
    int32_t i;
    float f;
  };
  
  // по идее теперь должно работать нормально
  // мне нужно придумать какой то особый фрустум или как то пометить так чтобы не получать каждый кадр выделение 0-го тайла
  uint32_t get_casted_battle_map_tile() {
    auto battle = global::get<systems::battle_t>();
    if (!battle->is_init()) return UINT32_MAX;
    
    float min_dist = 100000.0f;
    uint32_t final_tile_index = UINT32_MAX;
    
//     convert_int_unit_float c;
    //const uint32_t mask = uint32_t(1) << 31;
    const uint32_t mask = 0x1;
    auto opt = global::get<render::battle::tile_optimizer>();
//     auto buffer = opt->get_selection_indices();
//     auto indirect = opt->get_indirect_buffer();
//     auto tiles_data = reinterpret_cast<render::battle::tile_optimizer::indirect_buffer_data*>(indirect->ptr());
    //const size_t selection_slots = render::battle::tile_optimizer::selection_slots;
  
    const size_t selection_slots = opt->get_selection_count();
    auto array = opt->get_selection_data();
    //auto array = reinterpret_cast<uint32_t*>(buffer->ptr());
    for (size_t i = 0; i < selection_slots; ++i) {
      const size_t final_index = i*2;
      const uint32_t val = array[final_index];
      const uint32_t tile_index = array[final_index+1];
      // это по идее результат фрустума
      //const bool frustum_result = ((val >> 31) & mask) == mask;
      // плохая идея, скорее нужно так сравнивать, не применяя маску
      const bool frustum_result = val == mask;
      if (frustum_result) {
        
      } else {
        const float dist = glm::uintBitsToFloat(val);
//         PRINT_VAR("dist", dist)
//         PRINT_VAR("tile_index", tile_index)
        if (min_dist > dist) {
          min_dist = dist;
          final_tile_index = tile_index;
        }
      }
      
      //if (val == 0) continue; // похоже что -0 не равно 0
    }
    
    return final_tile_index;
  }
  
//   void draw_tooltip(const uint32_t &index, const sol::function &tile_func) {
//     if (tile_index != UINT32_MAX) {
//       global::get<render::tile_highlights_render>()->add(tile_index);
//       if (current_game_state == &world_map_state && !loading) tile_func(base_systems.interface_container->moonnuklear_ctx, tile_index);
//     }
//   }

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
    input::get_input_data()->fb_scale.x = float(display_width / widht);
    input::get_input_data()->fb_scale.y = float(display_height / height);
//     global::get<input::data>()->fb_scale.x = float(display_width / widht);
//     global::get<input::data>()->fb_scale.y = float(display_height / height);

//     if (global::get<input::data>()->interface_focus) {
      nk_input_begin(ctx);
      // nk_input_unicode нужен для того чтобы собирать набранный текст
      // можем ли мы воспользоваться им сразу из коллбека?
      // по идее можем, там не оч сложные вычисления
      //nk_input_unicode(ctx, 'f');

      for (uint32_t i = 0; i < input::get_input_data()->current_text; ++i) {
        nk_input_unicode(ctx, input::get_input_data()->text[i]);
      }

//       const int32_t test_in = glfwGetKey(window->handle, GLFW_KEY_DELETE);
//       if (test_in == GLFW_RELEASE) std::cout << "Release" << "\n";
//       else if (test_in == GLFW_PRESS) std::cout << "Press" << "\n";
//       else if (test_in == GLFW_REPEAT) std::cout << "Repeat" << "\n";
//       else std::cout << test_in << "\n";

      const uint32_t rel_event = static_cast<uint32_t>(input::type::release);
      //const bool* keys = Global::data()->keys;
      //const auto keys = global::get<input::data>()->key_events.container;
      const auto &keys = input::get_input_data()->container;
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

      nk_input_button(ctx, NK_BUTTON_DOUBLE, int(input::get_input_data()->click_pos.x), int(input::get_input_data()->click_pos.y), input::check_key(GLFW_MOUSE_BUTTON_LEFT, input::state_double_click));
      nk_input_scroll(ctx, nk_vec2(input::get_input_data()->mouse_wheel, 0.0f));
      nk_input_end(ctx);
      // обнуляем
      input::get_input_data()->click_pos = glm::uvec2(x, y);
      input::get_input_data()->current_text = 0;
      input::get_input_data()->mouse_wheel = 0.0f;

      glfwSetInputMode(window->handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
//     } else {
//       glfwSetInputMode(window->handle, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
//     }
  }
  
  void manage_states(
    const std::array<utils::quest_state*, utils::quest_state::count> &states, 
    utils::quest_state** current_game_state, 
    utils::quest_state** previous_game_state, 
    game::context* game_ctx,
    const size_t &time
  ) {
    auto cur = *current_game_state;
    auto prev = *previous_game_state;
    
    // что-то в программе или current_game_state обновляет новый стейт
    // в интерфейсе мы делаем какую нидуь анимацию конца стейта и обновляем текущий
    // здесь при смене текущего мы должны обновить гейм стейт
    // ... ???
    
    //if (game_ctx->is_loading() || !game_ctx->state_eq()) return;
    
//     if (game_ctx->is_loading()) {
//       if (game_ctx->state_eq()) {if (cur->load(prev)) {
//         game_ctx->new_state = static_cast<enum game::context::state>(static_cast<size_t>(game_ctx->state) + 1);
//       }}
//       
//       return;
//     } 
//     
//     cur->update(time, prev);
//     if (cur->next_state() != UINT32_MAX) {
//       const uint32_t index = cur->next_state();
//       // ождается что здесь мы почистим все ресурсы доконца и обратно вернем next_state к UINT32_MAX
//       // это не работает для перехода от карты к битве и обратно
//       // мне либо делать так же как в героях (то есть не чистить память мира)
//       // либо нужно придумать иной способ взаимодействия между стейтами
//       // переделал немного
//       *previous_game_state = *current_game_state;
//       *current_game_state = states[index];
//       switch (index) {
//         case utils::quest_state::main_menu:    game_ctx->new_state = game::context::state::main_menu_loading; break;
//         case utils::quest_state::map_creation: game_ctx->new_state = game::context::state::world_map_generator_loading; break;
//         case utils::quest_state::world_map:    game_ctx->new_state = game::context::state::world_map_loading; break;
//         case utils::quest_state::battle:       game_ctx->new_state = game::context::state::battle_map_loading; break;
//         case utils::quest_state::encounter:    game_ctx->new_state = game::context::state::encounter_loading; break;
//         default: throw std::runtime_error("Bad state index");
//       }
//     }
    
    // нужно быть предельно аккуратным, переход от лодинга к меню, например, почистит не все ресурсы (!!!)
    // как быть? можно в клин добавить указатель на некст_стейт, и если он не совпадает с ожидаемым, чистить ресурсы как положено
    // похоже что это самое нормальное решение
    if (game_ctx->state != (*current_game_state)->current_state()) {
      *previous_game_state = *current_game_state;
      *current_game_state = states[game_ctx->state];
      (*current_game_state)->enter(*previous_game_state);
    }
    
    // короче нужно сделать квест стейт по количеству геймстейтов
    // у квест стейта оставить только апдейт, ну и собственно его и обновлять
    // энтер, клир, апдейт
    
    // идея с отложенным началом следующего стейта - говно
    // начать вычисления стейта мы должны как можно скорее
    // 
    
    // перестает обновляться когда стейт обновился, хотя не должен
    const uint32_t next_state = cur->update(time, prev);
    if (next_state == UINT32_MAX || next_state == cur->current_state()) return;
    
//     if (next_state != UINT32_MAX || next_state != cur->current_state()) {
      //const uint32_t index = cur->next_state();
      const uint32_t index = next_state;
      *previous_game_state = *current_game_state;
      *current_game_state = states[index];
      (*current_game_state)->enter(*previous_game_state);
      game_ctx->state = index;
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

  sol::function basic_interface_functions(systems::core_t &base_systems) {
//     const std::string script_folder = global::root_directory() + "scripts/";
//     base_systems.interface_container->process_script_file(script_folder + "generator_progress.lua");
//     base_systems.interface_container->process_script_file(script_folder + "user_interface.lua");
//     base_systems.interface_container->process_script_file(script_folder + "player_layer.lua");
//     base_systems.interface_container->process_script_file(script_folder + "settings_menu.lua");
//     base_systems.interface_container->process_script_file(script_folder + "tile_interface.lua");
//     //options_window
//     base_systems.interface_container->register_function("progress_bar", "progress_bar");
//     base_systems.interface_container->register_function("main_menu_window", "main_menu");
//     base_systems.interface_container->register_function("main_menu_map", "main_menu_map");
//     base_systems.interface_container->register_function("worlds_window_func", "worlds_window");
//     base_systems.interface_container->register_function("main_interface_layer", "player_interface");
//     base_systems.interface_container->register_function("options_menu_window", "options_window");
//     base_systems.interface_container->register_function("graphics_options_window", "graphics_window");
//     
//     const auto proxy = base_systems.interface_container->lua["tile_window"];
//     if (proxy.get_type() != sol::type::function) throw std::runtime_error("Bad tile interface function");
//     return proxy.get<sol::function>();
    //base_systems.interface_container->load_interface_file(global::root_directory() + "scripts/interface/main.lua");
    base_systems.load_interface_config(global::root_directory() + "scripts/interface_config.lua");
    return sol::nil;
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
    
    auto map_system = global::get<systems::map_t>();
    if (map_system->is_init()) {
      ASSERT(map_system->core_context != nullptr);
      map_system->core_context->update_armies(time, [] (const size_t &time, core::army* army) {
        army->update(time);
      });
    }
    
    //game::advance_state();

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
  
//   glm::vec4 get_heraldy_pos(const glm::vec4 &tile_center, const float &tile_height, const float &zoom, const glm::mat4 &matrix) {
//     const glm::vec4 cam_right =  glm::vec4(matrix[0][0], matrix[1][0], matrix[2][0], 0.0f);
//     
//     const float offset_k = glm::mix(0.0f, 1.0f, glm::clamp((zoom - 0.2f) * 3.0f, 0.0f, 1.0f));
//     const glm::vec4 normal = glm::vec4(glm::normalize(glm::vec3(tile_center)), 0.0f);
//     return tile_center + normal * tile_height * render::render_tile_height + cam_right * offset_k * 5.0f;
//   }
  
  bool check_heraldy_hovering(
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
  
  void update_map_objects() {
    auto map_systems = global::get<systems::map_t>();
    if (!map_systems->is_init()) return;
    
    // эта функция может попасть на создание объектов в другом потоке
    // при загрузке она нам не нужна
    
    auto game_ctx = global::get<systems::core_t>()->game_ctx;
    if (game_ctx->is_loading()) return;
    
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
    
    // обходим все города и армии
    auto ctx = map_systems->core_context;
    auto buffers = map_systems->world_buffers;
    auto map = map_systems->map;
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
  
  // незнаю все таки как сделать вычисление чего нибудь полезного во время рендера
  // хотелось повычислять города/армии, но чет не могу разобраться с логикой
  // что нужно? нужно пока рисую на экране, пихнуть в буфер данные, 
  // данных может оказаться больше чем размер буфера, поэтому его придется пересоздать
  // может быть что на один кадр пропадут все армии/города/геральдики при пересоздании буфера
  // может это и ладно?
  void update_render() {
    auto cont = global::get<systems::core_t>()->graphics_container;
    // копирование где? должно быть где то повыше, поставил повыше в stages.cpp
    cont->begin();
    cont->next_frame();
    cont->draw();
    cont->present();
    
    //render_slots->update(global::get<render::container>());
    update_map_objects();
    //global::get<render::task_start>()->wait();
    //render_slots->clear();
    cont->wait();
    cont->clear();
  }
  
  // нужно избавиться от мьютекса, сделать это можно если заполнять информацию в буферы через посредника
  // теперь нужно будет только синхронизовать посредника, проблема в том что я обновляю непрерывно посредника
  // 
  void pre_sync() {
    auto m = get_map_mutex();
    if (m != nullptr) {
      std::unique_lock<std::mutex> lock(*m);
      update_render();
    } else {
      update_render();
    }
  }
  
  void sync(utils::frame_time &frame_time, const size_t &time) {
    pre_sync();
    
    frame_time.end();

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
  
  const std::string_view lua_func = R"C0L0N(
return function(command_name, target, value, compare_type, additional_data_type, result)
  print(command_name)
end
)C0L0N";
  
#define INPUT_CASE(type) case core::target_type::type: {      \
    d.input_array[current_index].first = std::string(id);     \
    d.input_array[current_index].second.number_type = script::number_type::object; \
    d.input_array[current_index].second.helper2 = static_cast<uint32_t>(core::type::s_type); \
    break;                                                    \
  }
  
  void draw_nesting(const size_t &nest_level) {
    for (size_t i = 0; i < nest_level; ++i) { std::cout << "  "; }
  }
  
  void test_decision() {
    core::character current(true, false);
    current.make_player();
    current.current_stats.set(core::character_stats::strength, 8);
    current.stats.set(core::character_stats::strength, 8);
    current.resources.set(core::character_resources::money, 50);
    
    auto c = &current;
    
    if (false) {
      sol::state lua;
      lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::package, sol::lib::string, sol::lib::table, sol::lib::utf8, sol::lib::coroutine);
      utils::world_map_loading::setup_lua(lua);
      
      const auto ret = lua.script_file(global::root_directory() + "scripts/decision_thoughts.lua");
      if (!ret.valid()) {
        sol::error err = ret;
        std::cout << err.what() << "\n";
        throw std::runtime_error("There is lua errors");
      }
      
      // короче нужно написать инициализацию десижона + какой то способ обойти десижоны
      const sol::table t = ret;
      core::decision d;
      core::validate_decision(0, t);
      core::parse_decision(&d, t);
      
      // теперь у нас есть десижон d что мы можем попробовать сделать?
      // собрать контекст и что нибудь попробовать запустить
      // для этого нам потребуется персонаж
      
      // вообще нужно наверное разделить данные от запускаемых функций, но с другой стороны
      // это реально не очень много чего освободит
      // вот что правда нужно сделать так это расположить все функции в определенном порядке
      // то бишь нужно завести огромный массив со всеми возможными функциями
      // ну эт несложно если разделить списки на несколько и работать с ними отдельно
      // в принципе нужно будет разделить списки хотя бы на типы таргетов с которыми они работают
      
//       const auto lua_ret = lua.script(lua_func);
//       if (!lua_ret.valid()) {
//         sol::error err = lua_ret;
//         std::cout << err.what();
//         throw std::runtime_error("There is lua errors");
//       }
      
      const auto draw_func = [] (const script::draw_data* dd) -> bool {
//         draw_nesting(dd->nest_level); std::cout << "decision " << dd->id << "\n";
//         draw_nesting(dd->nest_level); std::cout << "method   " << dd->method_name << "\n";
        draw_nesting(dd->nest_level); std::cout << "cur func " << dd->function_name << " nest level " << dd->nest_level << "\n";
        draw_nesting(dd->nest_level); std::cout << "cur type " << core::structure_data::names[size_t(dd->current.get_type())] << "\n";
        draw_nesting(dd->nest_level);
        if (dd->value.get_type() == script::object::type::number) std::cout << "value    type " << "number " << dd->value.get<double>() << "\n";
        else if (dd->value.get_type() == script::object::type::boolean) std::cout << "value    type " << "bool   " << dd->value.get<bool>() << "\n";
        else if (dd->value.valid()) std::cout << "value    type " << core::structure_data::names[size_t(dd->value.get_type())] << "\n";
        draw_nesting(dd->nest_level);
        if (dd->original.valid()) {
          if (dd->original.get_type() == script::object::type::number) std::cout << "original type " << "number " << dd->original.get<double>() << "\n";
          else if (dd->value.get_type() == script::object::type::boolean) std::cout << "original type " << "bool   " << dd->original.get<bool>() << "\n";
          else std::cout << "original type " << core::structure_data::names[size_t(dd->original.get_type())] << "\n";
        }
        std::cout << "\n";
      };
      
      // нужно передать сюда персонажа
      script::context ctx(d.id, "", 0);
      ctx.type = 0;
      ctx.root = ctx.current = script::object(c);
      ctx.draw_function = draw_func;
      
      // прежде чем рисовать нужно наверное сделать какое то обрамление
      ctx.method_name = "condition";
      d.condition.draw(&ctx);
      std::cout << "\n";
      ctx.method_name = "effect";
      d.effect.draw(&ctx);
      std::cout << "\n";
      
      const float before_money = c->resources.get(core::character_resources::money);
      const float before_military = c->stats.get(core::character_stats::military);
      const float before_intellect = c->stats.get(core::character_stats::intellect);
      {
        utils::time_log log("Script compute"); // какие скрипты будут долго бегать? тут скорее интересует скорость не одного скрипта, а скриптов для всех персонажей
        if (d.potential.compute(&ctx) && d.condition.compute(&ctx)) {
          d.effect.compute(&ctx);
        }
      }
      const float after_money = c->resources.get(core::character_resources::money);
      const float after_military = c->stats.get(core::character_stats::military);
      const float after_intellect = c->stats.get(core::character_stats::intellect);
      
      PRINT_VAR("before_money", before_money)
      PRINT_VAR("after_money ", after_money)
      PRINT_VAR("before_military", before_military)
      PRINT_VAR("after_military ", after_military)
      PRINT_VAR("before_intellect", before_intellect)
      PRINT_VAR("after_intellect ", after_intellect)
      
      PRINT_VAR("potential  size", d.potential.size())
      PRINT_VAR("condition  size", d.condition.size())
      PRINT_VAR("effect     size", d.effect.size())
      PRINT_VAR("ai_will_do size", d.ai_will_do.size())
      
      PRINT_VAR("name", d.name_script.compute(&ctx))
      PRINT_VAR("description", d.description_script.compute(&ctx))
    }
    
    {
      sol::state lua;
      lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::package, sol::lib::string, sol::lib::table, sol::lib::utf8, sol::lib::coroutine);
      utils::world_map_loading::setup_lua(lua);
      
      const auto ret = lua.script_file(global::root_directory() + "scripts/wife_test.lua");
      if (!ret.valid()) {
        sol::error err = ret;
        std::cout << err.what() << "\n";
        throw std::runtime_error("There is lua errors");
      }
      
      script::input_data input;
      input.root = input.current = script::object::type_bit::character;
      
      const sol::object script = ret;
      script::condition cond;
      script::create_condition(input, &cond, script);
      PRINT_VAR("cond size", cond.size())
      
      utils::calendar calendar;
      global::get(&calendar);
      
      calendar.set_week_days_count(7);
      calendar.add_month_data({"jan", 31});
      calendar.add_month_data({"feb", 28});
      calendar.add_month_data({"mar", 31});
      calendar.add_month_data({"apr", 30});
      calendar.add_month_data({"may", 31});
      calendar.add_month_data({"jun", 30});
      calendar.add_month_data({"jul", 31});
      calendar.add_month_data({"aug", 31});
      calendar.add_month_data({"sep", 30});
      calendar.add_month_data({"oct", 31});
      calendar.add_month_data({"nov", 30});
      calendar.add_month_data({"dec", 31});
      
      calendar.set_start_date(0, 0, 0);
      // получится не ровно 30 лет, а округление в меньшую сторону в зависимости от текущей недели, потому что текущая дата выводится из текущего хода
      calendar.set_current_date(30, 0, 0);
      calendar.validate();
      
      const auto s_date = calendar.start_date();
      const auto c_date = calendar.current_date();
      const int64_t s_day = calendar.convert_date_to_days(s_date);
      const int64_t c_day = calendar.convert_date_to_days(c_date);
      std::cout << "Start date year " << s_date.year() << " month " << s_date.month() << " day " << s_date.day() << "\n";
      std::cout << "Cur   date year " << c_date.year() << " month " << c_date.month() << " day " << c_date.day() << "\n";
      PRINT_VAR("Start day", s_day)
      PRINT_VAR("Cur   day", c_day)
      
      
      // лучше будет если я разделю текущий ход и календарь
      // календарь пусть возвращает количество дней относительно 0
      const utils::calendar::date last = c_date;
      const int64_t days_count = calendar.convert_date_to_days(last);
      PRINT_VAR("days_count", days_count)
      
      core::religion rel1;
      core::religion rel2;
      
      rel1.id = "rel1";
      rel2.id = "rel2";
      
      using namespace utils::xoshiro256starstar;
      state s = init(1);
      
      core::character main(true, false);
      main.religion = &rel1;
      
      assert(main.is_male());
      
      const size_t count_value = 5000;
      std::vector<core::character> characters;
      characters.reserve(count_value);
      for (size_t i = 0; i < count_value; ++i) {
        bool male = true;
        {
          s = rng(s);
          const uint64_t val = get_value(s);
          const double norm = utils::rng_normalize(val);
          male = norm > 0.5;
        }
        
        core::religion* final_rel = nullptr;
        {
          s = rng(s);
          const uint64_t val = get_value(s);
          const double norm = utils::rng_normalize(val);
          final_rel = norm > 0.5 ? &rel1 : &rel2;
        }
        
        int64_t day_born = SIZE_MAX;
        {
          s = rng(s);
          const uint64_t val = get_value(s);
          const double norm = utils::rng_normalize(val);
          day_born = norm * days_count; // неправильно генерится возраст
        }
        
        characters.emplace_back(male, false);
        characters.back().religion = final_rel;
        characters.back().born_day = day_born;
      }
      
      script::context script_ctx;
      script_ctx.root = script_ctx.prev = script::object(&main);
      script_ctx.draw_function = [] (const script::draw_data* data) -> bool {
        if (data->original.valid()) std::cout << data->function_name << " value " << data->value.get<bool>() << " original " << data->original.get<bool>() << "\n";
        else std::cout << data->function_name << " value " << data->value.get<bool>() << "\n";
        return true;
      };
      
//       for (size_t i = 0; i < 10; ++i) {
//         auto cur = &characters[i];
//         script_ctx.current = script::object(cur);
//         PRINT("Character " + std::to_string(i))
//         cond.draw(&script_ctx);
//         PRINT("")
//       }
//       
//       assert(false);
      
      const auto wife_test = [] (script::context* ctx, const bool print = false) {
        {
          auto c = ctx->current.get<core::character*>();
          if (c->is_married() != false) {
            if (print) std::cout << "Char " << c << " is_married " << "\n";
            return false;
          }
        }
        
        {
          auto c = ctx->current.get<core::character*>();
          if (c->is_adult() != true) {
            if (print) std::cout << "Char " << c << " not is_adult " << "\n";
            return false;
          }
        }
        
        {
          auto c = ctx->current.get<core::character*>();
          const auto male = c->is_male();
          auto root = ctx->root.get<core::character*>();
          const auto r_male = root->is_male();
          const bool final_b = male != r_male;
          if (!final_b) {
            if (print) std::cout << "Char " << c << " is_male == root is_male " << "\n";
            return false;
          }
        }
        
        {
          auto c = ctx->current.get<core::character*>();
          auto root = ctx->root.get<core::character*>();
          const bool final_b = c->has_same_religion_as(root);
          if (!final_b) {
            if (print) std::cout << "Char " << c << " not has_same_religion_as root " << "\n";
            return false;
          }
        }
        
        return true;
      };
      
      const auto draw_func = [] (const script::draw_data* dd) -> bool {
        std::cout << dd->function_name << " value " << dd->value.get<bool>();
        if (dd->original.is<bool>()) std::cout << " original " << dd->original.get<bool>() << "\n";
        else std::cout << "\n";
        //auto c = dd->current.get<core::character*>();
        return true;
      };
      
      script_ctx.draw_function = draw_func;
      
      // простой скрипт для 5000 персонажей получается около 1782 мкс для дебаг билда, это много или мало? нужно конечно потестить оптимизацию
      // в релизном билде 700-900 мкс, это много или мало? с чем сравнить? просто написать функцию самому?
      // просто написанная функция занимает 300-400 мкс, почти в 3 раза быстрее (но это тип с учетом всех оптимизаций, которые тут можно применить без виртуальных функций)
      // скрипт возвращает не то же самое что и функция =( починил два бага: во первых NAND и NOR с одним ребенком работали как AND
      // не идеально починил: NAND и NOR будут создаваться каждый раз как будет вызвана функция инициализации, желательно конечно передавать id родительской функции
      // во вторых в функции инициализации таблицы могут придти последовательно несколько функции из дочерней функции и раньше эта функция перетирала все функции после первой
      // теперь по идее возврат должен быть эквивалентен ручной функции
      {
        std::vector<core::character*> output_characters;
        output_characters.reserve(count_value);
        
        utils::time_log log("Condition for " + std::to_string(count_value) + " characters");
        for (size_t i = 0; i < count_value; ++i) {
          auto cur = &characters[i];
          script_ctx.current = script::object(cur);
          
          if (cond.compute(&script_ctx)) {
            output_characters.push_back(cur);
          }
        }
        
        for (auto c : output_characters) {
          script_ctx.current = script::object(c);
          if (!wife_test(&script_ctx, true)) {
            const bool male = c->is_male();
            const double age = c->get_age();
            std::cout << "character male " << male << '\n';
            std::cout << "character age  " << age << '\n';
            cond.draw(&script_ctx);
            break;
          }
        }
        
        PRINT_VAR("output_characters size", output_characters.size())
      }
      
      {
        std::vector<core::character*> output_characters;
        output_characters.reserve(count_value);
        
        utils::time_log log("Lambda    for " + std::to_string(count_value) + " characters");
        for (size_t i = 0; i < count_value; ++i) {
          auto cur = &characters[i];
          script_ctx.current = script::object(cur);
          
          if (wife_test(&script_ctx)) {
            output_characters.push_back(cur);
          }
        }
        
        PRINT_VAR("output_characters size", output_characters.size())
      }
    }
    
    // я вроде бы более мнее сделал всю логическую чать скриптов (01.07.2021)
    // теперь нужно бы все потестировать и добавить больше функции
    // что нужно потестить: сложные переменные, lvalue, rvalue, взятие скоупа, что то нужно придумать с массивом
    // проблема в том что нам каким то образом нужно получать несколько входных данных, некоторые из этих вещей
    // удобно пихнуть в мапу, а некоторые удобно передать так (например 'потентиал' условия принимают несколько таргетов)
    // (их проще всего их засунуть в массив? хотя зачем? в принципе эти условия должны работать также как и остальные)
    
    // переписал скрипт (29.11.2021), теперь легко вычислять числовые значения со сложными условиями
    // гораздо проще для восприятия, практически отсутствует говнокод (только в функциях инициализации) 
    // (как с этим разделаться при этом не потеряв в гибкости?), поддерживает все тоже что поддерживала предыдущая версия
    // побольшому счету осталось только составить разные скрипты для системных вещей и посмотреть чего не хватает
    // например скрипт расчета отношений двух персонажей или скрипт расчета денежного дохода
    // в отношениях нужно искать наличие противоположного треита, по идее это как то так:
    // every_trait = { { condition = { ["context:other"] = { has_trait = "prev.opposite" } }, -15 } }
    // 
    assert(false);
  }
  
//   void draw_army_path() {
// //     auto selector = global::get<systems::core_t>()->objects_selector;
// //     auto ctx = global::get<systems::map_t>()->core_context;
// //     if (selector->count == 0) return;
// //     if (selector->units[0].type != utils::objects_selector::unit::type::army && selector->units[0].type != utils::objects_selector::unit::type::hero) return;
// //     
// //     auto highlighter = global::get<render::tile_highlights_render>();
// //     
// //     size_t current_path = SIZE_MAX;
// //     size_t current_path_size = 0;
// //     ai::path_container* path = nullptr;
// //     float max_cost = 124414.0f;
// //     for (uint32_t i = 0; i < selector->count; ++i) {
// //       const auto &unit = selector->units[i];
// //       if (unit.type == utils::objects_selector::unit::type::army) {
// //         core::army* army = ctx->get_army(unit.index);
// //         if (army->path == nullptr || army->path == reinterpret_cast<void*>(SIZE_MAX)) continue;
// //         
// //         current_path = army->current_path;
// //         current_path_size = army->path_size;
// //         path = army->path;
// //       } else {
// //         
// //       }
// //       
// //       for (; current_path < current_path_size; ++current_path) {
// //         const uint32_t container = current_path / ai::path_container::container_size;
// //         const uint32_t index     = current_path % ai::path_container::container_size;
// //         const auto current_container = ai::advance_container(path, container);
// //         const uint32_t tile_index = current_container->tile_path[index].tile;
// //         const float tile_cost = current_container->tile_path[index].cost;
// // 
// //         ASSERT(tile_index < core::map::hex_count_d(core::map::detail_level));
// // 
// //         // сравниваем
// //         const auto color = max_cost > tile_cost ? render::make_color(0.0f, 0.7f, 0.0f, 0.5f) : render::make_color(0.7f, 0.0f, 0.0f, 0.5f);
// //         highlighter->add(tile_index, color);
// //       }
// //     }
//   }
  
//   void recompute_army_pos(core::army* army) {
//     auto map = global::get<systems::map_t>()->map;
//     
//     const auto tile_data = map->get_tile(army->tile_index);
//     const uint32_t point_index = tile_data.tile_indices.x;
//     const glm::vec4 center = map->get_point(point_index);
//     const glm::vec4 normal = glm::normalize(glm::vec4(glm::vec3(center), 0.0f));
//     const float height = map->get_tile_height(army->tile_index);
//     const uint32_t height_layer = render::compute_height_layer(height);
//     const float final_height = render::layer_height * height_layer;
//     
//     const glm::vec4 final_point = center + normal * (final_height * render::render_tile_height + 1.0f);
//     army->set_pos(glm::vec3(final_point));
//   }
  
  void advance_army(core::army* army) {
    assert(false);
//     // тут мы продвигаем армию до конца пока не кончится ходы
//     // по идее примерно такую же функцию нужно сделать для игроковых армий
//     // но там нужно обновлять положение каждый кадр
//     // это по идее значит что мы еще не нашли путь, хотя навреное нужно добавить что то еще к этому
//     //if (army->path == nullptr) return;
// //     auto tp = std::chrono::steady_clock::now();
// //     while (army->path == nullptr) {
// //       std::this_thread::sleep_for(CHRONO_TIME_TYPE(1));
// //       auto end = std::chrono::steady_clock::now() - tp;
// //       auto mcs = std::chrono::duration_cast<CHRONO_TIME_TYPE>(end).count();
// //       if (mcs >= ONE_SECOND*5) throw std::runtime_error("finding too long");
// //     }
//     
//     ai::path_container* tmp_ptr = army->path;
//     if (reinterpret_cast<size_t>(tmp_ptr) == SIZE_MAX) return; // неудалось найти путь
//     
// //     ASSERT(army->start_tile != army->path[0].tile_path[0].tile);
//     
//     auto map = global::get<systems::map_t>()->map;
//     
// //     const uint32_t first_tile = army->tile_index;
// //     uint32_t current_tile = army->tile_index;
// //     render::map_tile_t tile_data = render::unpack_data(map->get_tile(current_tile));
// //     uint32_t current_height_layer = render::compute_height_layer(tile_data.height);
//     
//     //ASSERT(army->current_path < army->path_size);
// 
//     if (army->path_size == 0) return;
//     
//     if (army->current_path == army->path_size) {
//       // теперь тут удобнее сделать путь со стартовым тайлом
//       return;
//     }
//     
//     const uint32_t prev_container = (army->current_path) / ai::path_container::container_size;
//     const uint32_t prev_index     = (army->current_path) % ai::path_container::container_size;
//     const float current_cost = army->current_path == 0 ? 0.0f : ai::advance_container(army->path, prev_container)->tile_path[prev_index].cost;
//     // у нас есть представление о стоимости прохода, не нужно дополнительно пересчитывать
//     // тут нужно проверить первый пехеод, если не хватает очков то выходим из функции
//     size_t next_current_path = army->current_path;
//     for (; next_current_path < army->path_size; ++next_current_path) { // тут мы должны получить место куда встанет армия
//       // проверяем не вышли ли мы за пределы передвижения армии
//       const uint32_t container = next_current_path / ai::path_container::container_size;
//       const uint32_t index     = next_current_path % ai::path_container::container_size;
//       
//       //const float next_tile_cost = army->path[container].tile_path[index].cost - current_cost;
//       const float next_tile_cost = ai::advance_container(army->path, container)->tile_path[index].cost - current_cost;
//       //const uint32_t next_tile = army->path[container].tile_path[index].tile;
//       
//       if (next_tile_cost >= 32535) break;
//     }
//     
//     //army->current_path = next_current_path;
//     //const uint32_t new_container = (next_current_path) / ai::path_container::container_size;
//     //const uint32_t new_index     = (next_current_path) % ai::path_container::container_size;
//     //const uint32_t new_tile      = ai::advance_container(army->path, new_container)->tile_path[new_index].tile;
//     
//     // теперь пытаемся поставить армию в next_current_path
//     uint32_t tmp = UINT32_MAX;
//     //bool ret = map->tile_objects_index_comp_swap(new_tile, 4, tmp, army->army_gpu_slot);
//     bool ret = false;
// 
//     while (!ret && next_current_path > army->current_path) {
//       --next_current_path;
//       
//       const uint32_t new_container = (next_current_path) / ai::path_container::container_size;
//       const uint32_t new_index     = (next_current_path) % ai::path_container::container_size;
//       const uint32_t new_tile      = ai::advance_container(army->path, new_container)->tile_path[new_index].tile;
//       
//       tmp = UINT32_MAX;
//       ret = map->tile_objects_index_comp_swap(new_tile, 4, tmp, army->army_gpu_slot);
//     }
//     
//     if (!ret && next_current_path == army->current_path) {
//       // по идее мы ничего не добились, можно выходить
//       return;
//     }
//     
//     const uint32_t final_container = (next_current_path) / ai::path_container::container_size;
//     const uint32_t final_index     = (next_current_path) % ai::path_container::container_size;
//     const uint32_t final_tile      = ai::advance_container(army->path, final_container)->tile_path[final_index].tile;
//     
//     tmp = army->army_gpu_slot;
//     map->tile_objects_index_comp_swap(army->tile_index, 4, tmp, UINT32_MAX);
//     ASSERT(map->get_tile_objects_index(army->tile_index, 4) == UINT32_MAX);
//     army->tile_index = final_tile;
//     // вычитаем стоимость пути
//     army->current_path = next_current_path+1;
//     
// //     while (true) {
// //       const uint32_t last_speed = army->current_stats[core::army_stats::speed].uval; // спид наверное будет масштабом 10 или мож больше
// //       const size_t current_path = army->current_path;
// //       const uint32_t container = current_path / ai::path_container::container_size;
// //       const uint32_t index     = current_path % ai::path_container::container_size;
// //       
// //       const uint32_t next_tile = army->path[container].tile_path[index].tile;
// //       
// //       const auto next_tile_data = render::unpack_data(map->get_tile(next_tile));
// //       
// //       // тут нужно расчитать стоимость перехода с тайла на тайл
// //       // нужно учесть наличие дороги, биом в конечном тайле, подъем
// //       
// //       const uint32_t next_height_layer = render::compute_height_layer(next_tile_data.height);
// //       const uint32_t height_difference = current_height_layer > next_height_layer ? 0 : next_height_layer-current_height_layer;
// //       
// //       // тут нужно выбрать подходящий масштаб, я думаю что это 100 (можно на 1000)
// //       // то есть каждый тайл по умолчанию стоит 100, как добавить веса
// //       // возможно делать по 10 за разницу в высоте, с другой стороны на какую то 
// //       // высоту взобраться мы не можем (разница 3?) а значит наверное нужно умножить на 25 примерно?
// //       const uint32_t move_cost = 100 + height_difference * 25;
// //       
// //       //if (last_speed < move_cost) break;
// //       
// //       //army->current_stats[core::army_stats::speed].uval -= move_cost;
// //       army->tile_index = next_tile;
// //       ++army->current_path;
// //       
// //       current_tile = army->tile_index;
// //       tile_data = render::unpack_data(map->get_tile(current_tile));
// //       current_height_layer = render::compute_height_layer(tile_data.height);
// //       
// //       //if (current_tile == army->end_tile) break;
// //       if (army->current_path == army->path_size) break;
// //     }
// //     
// //     // нужно обновить кто где находится
// //     // путь может закончится на том тайле где уже кто то есть
// //     // здесь мы будем видимо откатывать позицию, нужно ли искать путь в обход текущих позиций?
// //     // было бы неплохо что такое сделать в будущем, но пока ладно
// //     
// //     // в общем нужно делать все вместе поиск пути и перемещение
// //     // или скорее поиск пути 
// //     
// //     // тут надо добавить атомарную версию функции, каст к атомарным - это неизвестное поведение
// //     // че делать, с другой стороны мы не копируем а прост кстатим память
// // //     const uint32_t index = map->get_tile_objects_index(army->tile_index, 4);
// // //     if (index == UINT32_MAX) map->set_tile_objects_index(army->tile_index, 4, army->army_gpu_slot);
// // 
// //     if (army->current_path == 0) return;
// // 
// //     uint32_t tmp = army->army_gpu_slot;
// //     ASSERT(map->tile_objects_index_comp_swap(first_tile, 4, tmp, UINT32_MAX)); // это нужно наверное сделать в конце
// //     tmp = UINT32_MAX;
// //     bool ret = map->tile_objects_index_comp_swap(army->tile_index, 4, tmp, army->army_gpu_slot);
// //     if (army->current_path == 1) {
// //       if (!ret) ASSERT(ret = map->tile_objects_index_comp_swap(first_tile, 4, tmp, army->army_gpu_slot));
// //     } else {
// //       size_t current_path = army->current_path-1; // 
// //       while (!ret && current_path > 0) {
// //         --current_path;
// //         const uint32_t container = current_path / ai::path_container::container_size;
// //         const uint32_t index     = current_path % ai::path_container::container_size;
// //         const uint32_t next_tile = army->path[container].tile_path[index].tile;
// //         tmp = UINT32_MAX;
// //         ret = map->tile_objects_index_comp_swap(next_tile, 4, tmp, army->army_gpu_slot);
// //         army->tile_index = next_tile;
// //       }
// //       
// //       ASSERT(current_path != 0);
// //     }
//     
//     // не знаю на сколько целесообразно вообще передвигать армии в многопотоке
//     // с другой стороны движение армий - это разреженные задачи и пересекаются армии настолько редко
//     // что зачем просиживать это время верно? с третьей стороны эту задачу было бы неплохо сделать в отдельном потоке
//     // а в это время пока сделать другие задачи (там поиск жены и проч) 
//     // прохождение пути еще требует догонять армию, поэтому целесообразнее будет реально сбросить задачу на отдельный поток
//     // и тогда необязательно приводить к другому типу данные
//     
//     recompute_army_pos(army);
  }
  
  std::mutex* get_map_mutex() {
    auto map = global::get<systems::map_t>();
    auto battle = global::get<systems::battle_t>();
    //auto encounter = global::get<systems::encounter_t>();
    
    if (map->is_init()) return &map->map->mutex;
    if (battle->is_init()) return &battle->map->mutex;
    
    return nullptr;
  }
  
  void lock_rendering() {
    global::get<systems::map_t>()->lock_map();
    global::get<systems::battle_t>()->lock_map();
  }
  
  void unlock_rendering() {
    global::get<systems::map_t>()->unlock_map();
    global::get<systems::battle_t>()->unlock_map();
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
    //global::get<input::data>()->mouse_wheel += float(yoffset);
    auto ptr = input::get_input_data();
    ptr->mouse_wheel += float(yoffset);
  }

  void charCallback(GLFWwindow*, unsigned int c) {
    if (!global::get<render::window>()->flags.focused()) return;
    //if (!global::get<input::data>()->interface_focus) return;

//     global::get<input::data>()->text[global::get<input::data>()->current_text] = c;
//     ++global::get<input::data>()->current_text;
  //   ImGuiIO& io = ImGui::GetIO();
  //   if (c > 0 && c < 0x10000) io.AddInputCharacter((unsigned short)c);
    auto ptr = input::get_input_data();
    ptr->text[ptr->current_text] = c;
    ++ptr->current_text;
  }

  void mouseButtonCallback(GLFWwindow*, int button, int action, int mods) {
    if (!global::get<render::window>()->flags.focused()) return;
    
    //PRINT("dsafsf")

    (void)mods;
  //   if (action == GLFW_PRESS) mousePressed[button] = true;
  //   if (action == GLFW_RELEASE) mousePressed[button] = false;

    //global::data()->keys[button] = !(action == GLFW_RELEASE);
    auto data = input::get_input_data();
    const auto old = data->container[button].event;
    data->container[button].event = static_cast<input::type>(action);
    if (old != static_cast<input::type>(action)) data->container[button].event_time = 0;
  }

  void keyCallback(GLFWwindow*, int key, int scancode, int action, int mods) {
    (void)mods;
    
    //PRINT("dsafsf")

    if (!global::get<render::window>()->flags.focused()) return;
    if (key == GLFW_KEY_UNKNOWN) return; // нужно сделать ориентацию по скан кодам еще, в этом случае я так понимаю мы должны использовать мапу

//     if (action == GLFW_RELEASE) std::cout << "Release" << "\n";
//     else if (action == GLFW_PRESS) std::cout << "Press" << "\n";
//     else if (action == GLFW_REPEAT) std::cout << "Repeat" << "\n";
    
    //auto name = glfwGetKeyName(key, scancode);
//     auto name = input::get_key_name(key);
//     PRINT(name)

    //global::data()->keys[key] = !(action == GLFW_RELEASE);
    auto data = input::get_input_data();
    const auto old = data->container[key].event;
    data->container[key].event = static_cast<input::type>(action);
    if (old != static_cast<input::type>(action)) data->container[key].event_time = 0;

    //ASSERT(false);
    (void)scancode;
  }

  void window_resize_callback(GLFWwindow*, int w, int h) {
    auto system = global::get<systems::core_t>();
//     global::get<render::window>()->recreate(w, h);
    global::get<render::window>()->resize();
    //global::get<GBufferStage>()->recreate(w, h);
    //global::get<render::stage_container>()->recreate(w, h);
    system->interface_container->free_fonts();
    global::get<interface::context>()->remake_font_atlas(w, h);
    system->interface_container->get_fonts();
  //   std::cout << "window_resize_callback width " << w << " height " << h << '\n';
    
    auto settings = global::get<utils::settings>();
    settings->graphics.width = w;
    settings->graphics.height = h;

    system->graphics_container->render->recreate(w, h);
    system->image_controller->update_set();
    
    auto rp = reinterpret_cast<render::pass_framebuffer_container*>(system->main_pass);
    rp->recreate(w, h);
    
    auto map = global::get<systems::map_t>();
    if (map != nullptr && map->optimizators_container != nullptr) {
      map->optimizators_container->recreate(w, h);
      map->render_container->recreate(w, h);
    }
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
