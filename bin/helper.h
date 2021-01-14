#ifndef HELPER_H
#define HELPER_H

#include "utils/globals.h"
#include "utils/typeless_container.h"
#include "utils/slot_container.h"
#include "utils/input.h"
#include "utils/logging.h"
#include "utils/frame_time.h"
#include "utils/ecs.h"
#include "utils/thread_pool.h"
#include "utils/random_engine.h"
#include "utils/utility.h"
#include "utils/works_utils.h"
#include "utils/table_container.h"
#include "utils/string_container.h"
#include "utils/sol.h"
#include "utils/serializator_helper.h"
#include "utils/progress_container.h"
#include "utils/systems.h"
#include "utils/interface_container.h"
#include "utils/main_menu.h"
#include "utils/quest_states.h"
#include "utils/settings.h"
#include "utils/astar_search.h"
//#include "utils/perlin.h"
#include "FastNoise.h"

#include "render/window.h"
#include "render/render.h"
#include "render/stages.h"
#include "render/targets.h"
#include "render/container.h"
#include "render/shared_structures.h"
#include "render/render_mode_container.h"
#include "render/pipeline_mode_updater.h"
#include "render/slots.h"

#include "ai/sub_system.h"
#include "ai/build_subsystem.h"
#include "ai/ai_system.h"
#include "ai/path_container.h"

#include "figures.h"
#include "camera.h"
#include "map.h"
// #include "map_generator.h"
// #include "map_generators.h"
// #include "generator_system.h"
#include "map_generators2.h"
#include "generator_system2.h"
#include "generator_container.h"
//#include "generator_context.h"
#include "generator_context2.h"
#include "interface_context.h"
#include "overlay.h"
#include "map_creator.h"
#include "core_structures.h"
#include "core_context.h"
#include "interface2.h"
#include "game_time.h"
#include "logic.h"
#include "seasons.h"
#include "tiles_funcs.h"
#include "objects_selector.h"

#include <set>
#include <vector>
#include <atomic>

//#include "render/image_container.h"
//#include "render/particles.h"
//#include "render/decal_system.h"

#define DOUBLE_CLICK_TIME 400000

namespace devils_engine {
  const float radius = 500.0f;
  const uint32_t detail_level = 7;
  const uint32_t detail_level_acc_struct = 4;

  struct glfw_t {
    glfw_t();
    ~glfw_t();
  };
  
  void set_application_path();

  void poll_events();
  void return_cursor();
  
  void keys_setup(); // это не работает !!!!!!!! функционал перенесен в сеттингс
  void mouse_input(components::camera* camera, const size_t &time, const uint32_t &casted_tile_index);
  void key_input(const size_t &time, const uint32_t &current_state, const bool loading);
  void zoom_input(components::camera* camera);
  glm::vec4 get_cursor_dir(render::buffers* buffers, render::window* window, const double xpos, const double ypos);
  uint32_t cast_mouse_ray();
  void draw_tooltip(const uint32_t &index, const sol::function &tile_func);
  void next_nk_frame(const size_t &time);

  void create_render_system(systems::core_t &base_systems);
  void setup_callbacks();
  sol::function basic_interface_functions(systems::core_t &base_systems);

  uint32_t sphere_frustum_test(const glm::vec3 &pos, const float &radius, const utils::frustum &fru);
  
  void set_default_values(sol::state &lua, sol::table &table);
  void border_points_test(const std::vector<glm::vec4> &array);

  void update(const size_t &time);

  void sync(utils::frame_time &frame_time, const size_t &time);
  
  void check_tile(const map::container* map, const uint32_t &tile_index);
  
  void draw_army_path();
  
  void advance_army(core::army* army);
  
  void lock_rendering();
  void unlock_rendering();
  
  components::camera* get_camera();

  void callback(int error, const char* description);
  int lua_panic(lua_State* s);
  void scrollCallback(GLFWwindow*, double xoffset, double yoffset);
  void charCallback(GLFWwindow*, unsigned int c);
  void mouseButtonCallback(GLFWwindow*, int button, int action, int mods);
  void keyCallback(GLFWwindow*, int key, int scancode, int action, int mods);
  void window_resize_callback(GLFWwindow*, int w, int h);
  void iconifyCallback(GLFWwindow*, int iconified);
  void focusCallback(GLFWwindow*, int focused);
  const char* getClipboard(void* user_data);
  void setClipboard(void* user_data, const char* text);
}

#endif
