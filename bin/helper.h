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
//#include "utils/perlin.h"
#include "FastNoise.h"

#include "render/window.h"
#include "render/render.h"
#include "render/stages.h"
#include "render/targets.h"
#include "render/container.h"
#include "render/shared_structures.h"

#include "figures.h"
#include "camera.h"
#include "map.h"
#include "map_generator.h"
#include "map_generators.h"
#include "generator_system.h"
#include "generator_context.h"
#include "interface_context.h"
#include "overlay.h"

#include <set>
#include <vector>

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
    systems::generator<map::generator_context>* map_generator;
    interface::context* context;

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
  void zoom_input(yacs::entity* ent);
  uint32_t cast_mouse_ray();
  void next_nk_frame();

  void create_render_system(system_container_t &systems);
  void create_render_stages(system_container_t &systems);
  void create_map_container(system_container_t &systems);
  void create_map_generator(system_container_t &systems, dt::thread_pool* pool, map::generator_context* context);

  uint32_t sphere_frustum_test(const glm::vec3 &pos, const float &radius, const utils::frustum &fru);
  void map_frustum_test(const map::container* map, const glm::mat4 &frustum, std::vector<uint32_t> &indices);
  void map_triangle_test(dt::thread_pool* pool, const map::container* map, const utils::frustum &fru, const uint32_t &triangle_index, std::atomic<uint32_t> &counter);
  void map_triangle_test(const map::container* map, const utils::frustum &fru, const uint32_t &triangle_index, std::atomic<uint32_t> &counter);
  void map_triangle_add(const map::container* map, const uint32_t &triangle_index, std::atomic<uint32_t> &counter);
  void map_triangle_add2(const map::container* map, const uint32_t &triangle_index, std::mutex &mutex, std::unordered_set<uint32_t> &unique_tiles, std::vector<uint32_t> &tiles_array);
  void map_triangle_test2(dt::thread_pool* pool, const map::container* map, const utils::frustum &fru, const uint32_t &triangle_index, std::atomic<uint32_t> &counter);

  void sync(utils::frame_time &frame_time, const size_t &time);
  
  void check_tile(const map::container* map, const uint32_t &tile_index);

  void callback(int error, const char* description);
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
