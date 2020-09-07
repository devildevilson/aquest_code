#ifndef HELPER_H
#define HELPER_H

#include "utils/globals.h"
#include "utils/typeless_container.h"
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

#include "ai/sub_system.h"
#include "ai/build_subsystem.h"
#include "ai/ai_system.h"

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
  
  struct system_container_t {
    utils::typeless_container container;
    render::container* graphics_container;
    core::map* map;
//     systems::generator<map::generator_context>* map_generator;
    interface::context* context;
    map::generator::container* map_container;
    utils::interface* interface;
    core::context* core_context;
    utils::data_string_container* string_container;

    system_container_t();
    ~system_container_t();
  };

  struct glfw_t {
    glfw_t();
    ~glfw_t();
  };
  
  void set_application_path();

  void poll_events();
  void return_cursor();
  
  void keys_setup();
  void mouse_input(yacs::entity* ent, const size_t &time);
  void key_input(const size_t &time);
  void zoom_input(yacs::entity* ent);
  uint32_t cast_mouse_ray();
  void next_nk_frame(const size_t &time);

  void create_render_system(system_container_t &systems);
  void create_render_stages(system_container_t &systems);
  void create_map_container(system_container_t &systems);
  map::creator* setup_map_generator();
  void destroy_map_generator(map::creator** ptr);
  void setup_rendering_modes(render::mode_container &container);
//   void create_map_generator(system_container_t &systems, dt::thread_pool* pool, map::generator_context* context);
//   std::vector<systems::generator<map::generator_context>*> create_map_generators(system_container_t &systems, dt::thread_pool* pool, map::generator_context* context);

  uint32_t sphere_frustum_test(const glm::vec3 &pos, const float &radius, const utils::frustum &fru);
//   void map_frustum_test(const map::container* map, const glm::mat4 &frustum, std::vector<uint32_t> &indices);
  void map_triangle_test(dt::thread_pool* pool, const map::container* map, const utils::frustum &fru, const uint32_t &triangle_index, std::atomic<uint32_t> &counter);
  void map_triangle_test(const map::container* map, const utils::frustum &fru, const uint32_t &triangle_index, std::atomic<uint32_t> &counter);
  void map_triangle_add(const map::container* map, const uint32_t &triangle_index, std::atomic<uint32_t> &counter);
  void map_triangle_add2(const map::container* map, const uint32_t &triangle_index, std::mutex &mutex, std::unordered_set<uint32_t> &unique_tiles, std::vector<uint32_t> &tiles_array);
//   void map_triangle_test2(dt::thread_pool* pool, const map::container* map, const utils::frustum &fru, const uint32_t &triangle_index, std::atomic<uint32_t> &counter);
  
  void set_default_values(sol::state &lua, sol::table &table);
  void load_interface_functions(sol::state &lua);
  void rendering_mode(const map::generator::container* cont, core::map* map, const uint32_t &property, const uint32_t &render_mode, const uint32_t &water_mode);
  void border_points_test(const std::vector<glm::vec4> &array);
  void find_border_points(const map::generator::container* container, const core::map* map, const sol::table &table);
  
  void generate_tile_connections(const core::map* map, dt::thread_pool* pool);
  void validate_and_create_data(system_container_t &systems);
  void create_interface(system_container_t &systems);
  void post_generation_work(system_container_t &systems);
  
  bool player_end_turn(core::character* c);
  void update(const size_t &time);

  void sync(utils::frame_time &frame_time, const size_t &time);
  
  void check_tile(const map::container* map, const uint32_t &tile_index);

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
