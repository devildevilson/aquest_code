#include "game_resources.h"

#include <filesystem>

#include "GLFW/glfw3.h"

#include "render/container.h"
#include "render/window.h"
#include "render/render.h"
#include "render/image_controller.h"
#include "render/pass.h"

#include "core/seasons.h"

#include "utils/globals.h"
#include "utils/interface_container2.h"
#include "utils/game_context.h"

#include "interface_context.h"
#include "loading_functions.h"
#include "loading_defines.h"
#include "whereami.h"

namespace devils_engine {
  namespace core {
    static void callback(int error, const char* description);
    static int lua_panic(lua_State* s);
    static void scrollCallback(GLFWwindow*, double xoffset, double yoffset);
    static void charCallback(GLFWwindow*, unsigned int c);
    static void mouseButtonCallback(GLFWwindow*, int button, int action, int mods);
    static void keyCallback(GLFWwindow*, int key, int scancode, int action, int mods);
    static void window_resize_callback(GLFWwindow*, int w, int h);
    static void iconifyCallback(GLFWwindow*, int iconified);
    static void focusCallback(GLFWwindow*, int focused);
    static const char* getClipboard(void* user_data);
    static void setClipboard(void* user_data, const char* text);
    static void setup_callbacks(render::window* window);
    
    static void set_application_path() {
      int32_t dirname = 0;
      const int32_t length = wai_getExecutablePath(nullptr, 0, &dirname);
      ASSERT(length > 0);

      std::vector<char> array(length+1);
      wai_getExecutablePath(array.data(), length, &dirname);

      array[length] = '\0'; // весь путь
      array[dirname+1] = '\0'; // путь до папки

      std::filesystem::path p = std::string(array.data());
      //PRINT(p)           // .../apates_quest/bin/
      p = p.parent_path(); // .../apates_quest/bin
      p = p.parent_path(); // .../apates_quest
      p += "/";            // .../apates_quest/
      p.make_preferred();
      const std::string dir_path = p.string(); // может быть хранить путь в std::filesystem::path? лан пока так оставим

      global g;
      g.set_root_directory(dir_path);
    }
    
    glfw_t::glfw_t() {
      if (glfwInit() != GLFW_TRUE) { throw std::runtime_error("Cannot init glfw!"); }
      if (glfwVulkanSupported() != GLFW_TRUE) { throw std::runtime_error("Vulkan is not supported!"); }
      glfwSetErrorCallback(callback);
    }
    
    glfw_t::~glfw_t() { glfwTerminate(); }
    
//     static void poll_events() {
//       glfwPollEvents();
//     }
    
    block_input_t::block_input_t() { input::block(); }
    block_input_t::~block_input_t() { input::unblock(); }
    
#define KBYTES 1024
#define MBYTES (1024 * 1024)
    const size_t frame_allocator_size = MBYTES * 64;
    
    const size_t threads_count = std::max(std::thread::hardware_concurrency()-1, uint32_t(1));
    
    game_resources_t::game_resources_t(const int argc, const char* argv[]) : pool(threads_count), frame_allocator(frame_allocator_size) {
      UNUSED_VARIABLE(argc);
      UNUSED_VARIABLE(argv);
      
      set_application_path();
      
      global::get(this);
      global::get(&tasks);
      global::get(&base_systems);
//       global::get(&map_systems);
//       global::get(&battle_systems);
//       global::get(&encounter_systems);
      global::get(&settings);
      global::get(&frame_allocator);
      
      base_systems.create_utility_systems();
      settings.load_settings();
      
      block_input_t bi;
      
      uint32_t count;
      const char** ext = glfwGetRequiredInstanceExtensions(&count);
      base_systems.create_render_system(ext, count);
      
      base_systems.create_render_stages();
      base_systems.create_interface();
      settings.graphics.find_video_mode();
      base_systems.load_interface_config(global::root_directory() + "scripts/interface_config.lua");
      
      setup_callbacks(base_systems.graphics_container->window);
      
      //static_assert(test_func());
      //test_func();
      //ASSERT(false);
    }
    
    game_resources_t::~game_resources_t() {
      base_systems.game_ctx->quit_game();
  
      pool.compute();
      pool.wait();
    }
    
    void game_resources_t::poll_events() {
      glfwPollEvents();
    }
    
#define LOADING_TYPES_IMPL(type, load_type)                        \
    type::type(game_resources_t* res) : loading_interface(systems::create_loading_context(res)) {} \
    type::~type() { destroy_loading_context(ctx); }                \
    void type::update() {                                          \
      if (finished()) return;                                      \
      if (!notify.valid()) notify = systems::load_type##_async::funcs[counter](ctx);\
      auto status = notify.wait_for(std::chrono::milliseconds(0)); \
      if (status == std::future_status::ready) { ++counter; notify.get(); } \
    }                                                              \
    bool type::finished() const { return counter >= systems::load_type##_async::func_count; } \
    size_t type::current() const { return counter; }               \
    size_t type::count() const { return systems::load_type##_async::func_count; }   \
    std::string_view type::hint1() const { return #load_type; }    \
    std::string_view type::hint2() const { return finished() ? "end" : systems::load_type##_async::names[counter]; } \
    std::string_view type::hint3() const { return ""; }            \
    
    
LOADING_TYPES_IMPL(load_main_menu_t, loading_main_menu)
LOADING_TYPES_IMPL(load_save_game_t, loading_save_game)
LOADING_TYPES_IMPL(load_save_world_t, loading_saved_world)
LOADING_TYPES_IMPL(load_gen_world_t, loading_generated_map)
LOADING_TYPES_IMPL(load_gen_t, loading_generator)
//LOADING_TYPES_IMPL(gen_step_t, loading_save_game)
LOADING_TYPES_IMPL(load_battle_t, loading_battle)
LOADING_TYPES_IMPL(load_encounter_t, loading_encounter)
LOADING_TYPES_IMPL(post_gen_world_t, post_gen)
    
    // это нужно совместить с systems::generator_t
    // определено в loading_functions.cpp
//     gen_step_t::gen_step_t(game_resources_t* res) : loading_interface(systems::create_loading_context(res)) {}
//     gen_step_t::~gen_step_t() { destroy_loading_context(ctx); }
//     void gen_step_t::update() {
//       if (finished()) return;
//       //notify = ctx->pool.submit();
//       if (!notify.valid()) notify = systems::gen_step_async::funcs[counter](ctx);
//       auto status = notify.wait_for(std::chrono::milliseconds(0));
//       if (status == std::future_status::ready) { ++counter; notify.get(); }
//     }
//     bool gen_step_t::finished() const { return counter >= systems::gen_step_async::func_count; }
//     size_t gen_step_t::current() const { return counter; }
//     size_t gen_step_t::count() const { return systems::gen_step_async::func_count; }
//     std::string_view gen_step_t::hint1() const { return "generate_map"; }
//     std::string_view gen_step_t::hint2() const { return finished() ? "end" : systems::gen_step_async::names[counter]; }
//     std::string_view gen_step_t::hint3() const { return ""; }

    static void setup_callbacks(render::window* window) {
      glfwSetKeyCallback(window->handle, keyCallback);
      glfwSetCharCallback(window->handle, charCallback);
      glfwSetMouseButtonCallback(window->handle, mouseButtonCallback);
      glfwSetScrollCallback(window->handle, scrollCallback);
      glfwSetWindowFocusCallback(window->handle, focusCallback);
      glfwSetWindowIconifyCallback(window->handle, iconifyCallback);
      glfwSetWindowSizeCallback(window->handle, window_resize_callback);
    }
    
    static void callback(int error, const char* description) {
      std::cout << "Error code: " << error << std::endl;
      std::cout << "Error: " << description << std::endl;
    }

    static void scrollCallback(GLFWwindow*, double xoffset, double yoffset) {
      if (!global::get<render::window>()->flags.focused()) return;

      (void)xoffset;
      auto ptr = input::get_input_data();
      ptr->mouse_wheel += float(yoffset);
    }

    static void charCallback(GLFWwindow*, unsigned int c) {
      if (!global::get<render::window>()->flags.focused()) return;
      
      auto ptr = input::get_input_data();
      ptr->text[ptr->current_text] = c;
      ++ptr->current_text;
    }

    static void mouseButtonCallback(GLFWwindow*, int button, int action, int mods) {
      if (!global::get<render::window>()->flags.focused()) return;

      (void)mods;
      auto data = input::get_input_data();
      const auto old = data->container[button].event;
      data->container[button].event = static_cast<input::type>(action);
      if (old != static_cast<input::type>(action)) data->container[button].event_time = 0;
    }

    static void keyCallback(GLFWwindow*, int key, int scancode, int action, int mods) {
      (void)mods;
      
      if (!global::get<render::window>()->flags.focused()) return;
      if (key == GLFW_KEY_UNKNOWN) return; // нужно сделать ориентацию по скан кодам еще, в этом случае я так понимаю мы должны использовать мапу

      auto data = input::get_input_data();
      const auto old = data->container[key].event;
      data->container[key].event = static_cast<input::type>(action);
      if (old != static_cast<input::type>(action)) data->container[key].event_time = 0;

      (void)scancode;
    }

    static void window_resize_callback(GLFWwindow*, int w, int h) {
      auto system = global::get<systems::core_t>();
      system->graphics_container->window->resize();
      system->interface_container->free_fonts();
      system->context->remake_font_atlas(w, h);
      system->interface_container->get_fonts();
      
      auto settings = global::get<utils::settings>();
      settings->graphics.width = w;
      settings->graphics.height = h;

      system->graphics_container->render->recreate(w, h);
      system->image_controller->update_set();
      
      // TODO: переделать (возможно нужно в контейнере рендера это обновить)
      auto rp = reinterpret_cast<render::pass_framebuffer_container*>(system->main_pass);
      rp->recreate(w, h);
      
      auto map = global::get<systems::map_t>();
      if (map != nullptr && map->optimizators_container != nullptr) {
        map->optimizators_container->recreate(w, h);
        map->render_container->recreate(w, h);
      }
    }

    static void iconifyCallback(GLFWwindow*, int iconified) {
      global::get<render::window>()->flags.set_iconified(iconified);
    }

    static void focusCallback(GLFWwindow*, int focused) {
      global::get<render::window>()->flags.set_focused(focused);
    }

    static const char* getClipboard(void* user_data) {
      return glfwGetClipboardString(((render::window*)user_data)->handle);
    }

    static void setClipboard(void* user_data, const char* text) {
      glfwSetClipboardString(((render::window*)user_data)->handle, text);
    }
  }
}
