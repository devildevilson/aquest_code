#include "interfaces.h"

#include <climits>
#include "nuklear_header.h"
#include "graphics_component.h"
#include "input.h"
#include "Globals.h"
#include "core_funcs.h"
#include "window.h"
// #include "settings.h"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

enum MenuItems {
  MENU_NEW_GAME,
  MENU_SETTINGS,
  MENU_LOAD_GAME,
  MENU_QUIT,
  MENU_COUNT
};

enum settings_page {
  SETTINGS_GRAPHICS,
  SETTINGS_SOUND,
  SETTINGS_MOUSE,
  SETTINGS_CONTROLS,
  SETTINGS_COUNT
};

enum graphics_page {
  GRAPHICS_FULLSCREEN,
  GRAPHICS_SREEN_RESOLUTION,
  GRAPHICS_FOV,
  GRAPHICS_COUNT
};

enum QuitItem {
  QUIT_GAME_NO,
  QUIT_GAME_YES,
  QUIT_COUNT
};

namespace devils_engine {
  namespace interface {
    struct menu_labels_layout {
      static const size_t labels_count = MENU_COUNT;
      
      struct nk_rect logo;
      struct nk_rect new_game;
      struct nk_rect load_game;
      struct nk_rect options;
      struct nk_rect quit_game;
      struct nk_rect image_pos[labels_count];
      
      menu_labels_layout(context* ctx, const struct nk_rect &bounds);
    };
    
    struct settings_labels_layout {
      static const size_t labels_count = SETTINGS_COUNT;
      
      struct nk_rect label;
      struct nk_rect graphics;
      struct nk_rect sound;
      struct nk_rect mouse;
      struct nk_rect controls;
      struct nk_rect image_pos[labels_count];
      
      settings_labels_layout(context* ctx, const struct nk_rect &bounds);
    };
    
    struct graphics_labels_layout {
      static const size_t labels_count = GRAPHICS_COUNT;
      
      struct nk_rect label;
      struct nk_rect text;
      struct nk_rect resolution;
      struct nk_rect resolution_combo;
      struct nk_rect fov;
      struct nk_rect screen_mode;
      struct nk_rect screen_mode_current;
      struct nk_rect image_pos[labels_count];
      
      graphics_labels_layout(context* ctx, const struct nk_rect &bounds);
    };
    
    struct quit_labels_layout {
      static const size_t labels_count = QUIT_COUNT;
      
      struct nk_rect label;
      struct nk_rect quit_text;
      struct nk_rect opt_no;
      struct nk_rect opt_yes;
      struct nk_rect image_pos[labels_count];
      
      quit_labels_layout(context* ctx, const struct nk_rect &bounds);
    };
    
    container::container(const size_t &containerSize) : default_page(nullptr), current_page(nullptr), focus(0), page_container(containerSize) {} // last_command(data::command::none),
    container::~container() {
      for (auto page : pages) {
        page_container.destroy(page);
      }
    }
    
    void container::proccess(const data::extent &screen_size, const size_t &time) {
      if (!is_opened()) return;
      
      auto page = current_page->proccess(screen_size, command_event, time, focus);
      focus = page == current_page ? focus : 0;
      current_page = page;
      command_event = utils::id();
      command_time = SIZE_MAX;
      Global::get<input::data>()->interface_focus = current_page != nullptr;
    }
    
    void container::set_default_page(page* page_ptr) {
      default_page = page_ptr;
    }
    
    void container::open() {
      current_page = default_page;
    }
    
    bool container::is_opened() const {
      return current_page != nullptr;
    }
    
    void container::escape() {
      current_page = current_page->escape(focus);
    }
    
    void container::send_event(const utils::id &event, const size_t &time, const size_t &frame_time) {
      if (time < command_time) {
        if (event != last_event || time == 0) {
          last_event = event;
          command_event = event;
          delay_time = first_pressed_delay;
        } else {
          if (delay_time <= frame_time) {
            command_event = event;
            delay_time += pressed_delay;
          }
          delay_time -= frame_time;
        }
        command_time = time;
      }
    }
    
    const nk_color background_menu_color = nk_color{150, 150, 0, 64};
    
    main_menu::main_menu(const create_info &info) : nuklear(info.nuklear), quit(info.quit), settings(info.settings), new_game(info.new_game), current_time(0) {}
    page* main_menu::proccess(const data::extent &screen_size, const utils::id &command, const size_t &time, size_t &focus) {
      const auto in = &nuklear->ctx.input;
      const bool mouse_movement = in->mouse.delta.x != 0.0f || in->mouse.delta.y != 0.0f;
      
      interface::style_borders s1(&nuklear->ctx, 0, 0);
      interface::style_background_color s2(&nuklear->ctx, background_menu_color);
      //const auto window_rect = nk_rect(w, h, page_size.width, page_size.height);
      const auto window_rect = nk_rect(0, 0, screen_size.width, screen_size.height);
      
      current_time += time;
      const bool blink = current_time < blink_time;
      if (current_time >= blink_time*2) {
        current_time -= blink_time*2;
      }
      
      page* next = this;
      if (nk_begin(&nuklear->ctx, "main_menu", window_rect, NK_WINDOW_NO_SCROLLBAR)) {
        nk_layout_space_begin(&nuklear->ctx, NK_STATIC, screen_size.height, 64);
        const auto bounds = nk_layout_space_bounds(&nuklear->ctx);
        menu_labels_layout layout(nuklear, bounds);
        
        nk_style_set_font(&nuklear->ctx, &nuklear->fonts[fonts::menu]->handle);
        
        // лучше использовать заранее заготовленный логотип
        nk_layout_space_push(&nuklear->ctx, layout.logo);
        nk_label_colored(&nuklear->ctx, "Madness returns", NK_TEXT_ALIGN_CENTERED, nk_color{255, 0, 0, 255});
        
        nk_style_set_font(&nuklear->ctx, &nuklear->fonts[fonts::interface]->handle);
        
        nk_layout_space_push(&nuklear->ctx, layout.new_game);
        nk_label_colored(&nuklear->ctx, "New game", NK_TEXT_ALIGN_LEFT, nk_color{255, 0, 0, 255});
//         if (nk_button_label(&nuklear->ctx, "New game")) {
//           next = new_game;
//         }
        
        auto menu_label_bounds = nk_widget_bounds(&nuklear->ctx);
        if (nk_input_is_mouse_hovering_rect(in, menu_label_bounds) && mouse_movement && focus != MENU_NEW_GAME) focus = MENU_NEW_GAME;
        if (nk_input_has_mouse_click_in_rect(in, NK_BUTTON_LEFT, menu_label_bounds)) next = new_game;
        
        nk_layout_space_push(&nuklear->ctx, layout.options);
        nk_label_colored(&nuklear->ctx, "Settings", NK_TEXT_ALIGN_LEFT, nk_color{0, 3, 0, 1});
//         if (nk_button_label(&nuklear->ctx, "Settings")) {
//           next = settings;
//         }
        
        menu_label_bounds = nk_widget_bounds(&nuklear->ctx);
        if (nk_input_is_mouse_hovering_rect(in, menu_label_bounds) && mouse_movement && focus != MENU_SETTINGS) focus = MENU_SETTINGS;
        if (nk_input_has_mouse_click_in_rect(in, NK_BUTTON_LEFT, menu_label_bounds)) next = settings;
        
        nk_layout_space_push(&nuklear->ctx, layout.load_game);
        if (nk_button_label(&nuklear->ctx, "Load game")) {
          //next = new_game;
        }
        
        menu_label_bounds = nk_widget_bounds(&nuklear->ctx);
        if (nk_input_is_mouse_hovering_rect(in, menu_label_bounds) && mouse_movement && focus != MENU_LOAD_GAME) {
          focus = MENU_LOAD_GAME;
        }
        
        nk_layout_space_push(&nuklear->ctx, layout.quit_game);
        if (nk_button_label(&nuklear->ctx, "Quit game")) {
          next = quit;
        }
        
        menu_label_bounds = nk_widget_bounds(&nuklear->ctx);
        if (nk_input_is_mouse_hovering_rect(in, menu_label_bounds) && mouse_movement && focus != MENU_QUIT) {
          focus = MENU_QUIT;
        }
        
//         if (blink) {
          nk_layout_space_push(&nuklear->ctx, layout.image_pos[focus]);
          if (blink) nk_image(&nuklear->ctx, image_to_nk_image(render::create_image(0, 1, 0, false, false)));
          else nk_image(&nuklear->ctx, image_to_nk_image(render::create_image(0, 3, 0, false, false)));
//         }
        
        nk_layout_space_end(&nuklear->ctx);
      }
      nk_end(&nuklear->ctx);
      
      static const utils::id menu_next = utils::id::get("menu_next");
      static const utils::id menu_prev = utils::id::get("menu_prev");
//       static const utils::id menu_increase = utils::id::get("menu_increase");
//       static const utils::id menu_decrease = utils::id::get("menu_decrease");
      static const utils::id menu_choose = utils::id::get("menu_choose");
      
      static const utils::id tmr_item_pickup1 = utils::id::get("tmr_item_pickup1");
      if (command == menu_next) {
        focus = focus == 0 ? MENU_COUNT-1 : (focus-1); // %MENU_COUNT
        core::start_sound(sound::info(nullptr, tmr_item_pickup1));
      } else if (command == menu_prev) {
        focus = (focus+1)%MENU_COUNT;
        core::start_sound(sound::info(nullptr, tmr_item_pickup1));
      } else if (command == menu_choose) {
        switch (focus) {
          case MENU_NEW_GAME: next = new_game; break;
          case MENU_SETTINGS: next = settings; break;
          case MENU_QUIT: next = quit; break;
        }
      }
      
      return next;
    }
    
    page* main_menu::escape(size_t &focus) const {
      focus = 0;
      return nullptr;
    }
    
    void main_menu::set_pointers(page* quit, page* settings, page* new_game) {
      this->quit = quit;
      this->settings = settings;
      this->new_game = new_game;
    }
    
    settings::settings(const create_info &info) : nuklear(info.nuklear), main_menu(info.main_menu), current_time(0) {}
    page* settings::proccess(const data::extent &screen_size, const utils::id &event, const size_t &time, size_t &focus) {
      const auto in = &nuklear->ctx.input;
      const bool mouse_movement = in->mouse.delta.x != 0.0f || in->mouse.delta.y != 0.0f;
      
      interface::style_borders s1(&nuklear->ctx, 0, 0);
      interface::style_background_color s2(&nuklear->ctx, background_menu_color);
      //const auto window_rect = nk_rect(w, h, page_size.width, page_size.height);
      const auto window_rect = nk_rect(0, 0, screen_size.width, screen_size.height);
      
      current_time += time;
      const bool blink = current_time < blink_time;
      if (current_time >= blink_time*2) {
        current_time -= blink_time*2;
      }
      
      page* next = this;
      if (nk_begin(&nuklear->ctx, "settings", window_rect, NK_WINDOW_NO_SCROLLBAR)) {
        nk_layout_space_begin(&nuklear->ctx, NK_STATIC, screen_size.height, 64);
        const auto bounds = nk_layout_space_bounds(&nuklear->ctx);
        settings_labels_layout layout(nuklear, bounds);
        
        nk_style_set_font(&nuklear->ctx, &nuklear->fonts[fonts::menu]->handle);
        
        nk_layout_space_push(&nuklear->ctx, layout.label);
        nk_label_colored(&nuklear->ctx, "Settings", NK_TEXT_ALIGN_CENTERED, nk_color{255, 0, 0, 255});
        
        nk_style_set_font(&nuklear->ctx, &nuklear->fonts[fonts::interface]->handle);
        
        nk_layout_space_push(&nuklear->ctx, layout.graphics);
        nk_label_colored(&nuklear->ctx, "Graphics", NK_TEXT_ALIGN_LEFT, nk_color{255, 0, 0, 255});
        auto menu_label_bounds = nk_widget_bounds(&nuklear->ctx);
        if (nk_input_is_mouse_hovering_rect(in, menu_label_bounds) && mouse_movement && focus != SETTINGS_GRAPHICS) focus = SETTINGS_GRAPHICS;
        if (nk_input_has_mouse_click_in_rect(in, NK_BUTTON_LEFT, menu_label_bounds)) next = graphics;
        
        nk_layout_space_push(&nuklear->ctx, layout.sound);
        nk_label_colored(&nuklear->ctx, "Sound", NK_TEXT_ALIGN_LEFT, nk_color{255, 0, 0, 255});
        menu_label_bounds = nk_widget_bounds(&nuklear->ctx);
        if (nk_input_is_mouse_hovering_rect(in, menu_label_bounds) && mouse_movement && focus != SETTINGS_SOUND) focus = SETTINGS_SOUND;
        if (nk_input_has_mouse_click_in_rect(in, NK_BUTTON_LEFT, menu_label_bounds)) next = sound;
        
        nk_layout_space_push(&nuklear->ctx, layout.mouse);
        nk_label_colored(&nuklear->ctx, "Mouse", NK_TEXT_ALIGN_LEFT, nk_color{255, 0, 0, 255});
        menu_label_bounds = nk_widget_bounds(&nuklear->ctx);
        if (nk_input_is_mouse_hovering_rect(in, menu_label_bounds) && mouse_movement && focus != SETTINGS_MOUSE) focus = SETTINGS_MOUSE;
        if (nk_input_has_mouse_click_in_rect(in, NK_BUTTON_LEFT, menu_label_bounds)) next = mouse;
        
        nk_layout_space_push(&nuklear->ctx, layout.controls);
        nk_label_colored(&nuklear->ctx, "Controls", NK_TEXT_ALIGN_LEFT, nk_color{255, 0, 0, 255});
        menu_label_bounds = nk_widget_bounds(&nuklear->ctx);
        if (nk_input_is_mouse_hovering_rect(in, menu_label_bounds) && mouse_movement && focus != SETTINGS_CONTROLS) focus = SETTINGS_CONTROLS;
        if (nk_input_has_mouse_click_in_rect(in, NK_BUTTON_LEFT, menu_label_bounds)) next = controls;
        
        nk_layout_space_push(&nuklear->ctx, layout.image_pos[focus]);
        if (blink) nk_image(&nuklear->ctx, image_to_nk_image(render::create_image(0, 1, 0, false, false)));
        else nk_image(&nuklear->ctx, image_to_nk_image(render::create_image(0, 3, 0, false, false)));
        
        nk_layout_space_end(&nuklear->ctx);
      }
      nk_end(&nuklear->ctx);
      
      static const utils::id menu_next = utils::id::get("menu_next");
      static const utils::id menu_prev = utils::id::get("menu_prev");
//       static const utils::id menu_increase = utils::id::get("menu_increase");
//       static const utils::id menu_decrease = utils::id::get("menu_decrease");
      static const utils::id menu_choose = utils::id::get("menu_choose");
      
      // на самом деле у настроек будет страница выбора непосредственной настройки
      // и несколько страниц самих настроек
      static const utils::id tmr_item_pickup1 = utils::id::get("tmr_item_pickup1");
      if (event == menu_next) {
        focus = focus == 0 ? SETTINGS_COUNT-1 : (focus-1);
        core::start_sound(sound::info(nullptr, tmr_item_pickup1));
      } else if (event == menu_prev) {
        focus = (focus+1)%SETTINGS_COUNT;
        core::start_sound(sound::info(nullptr, tmr_item_pickup1));
      } else if (event == menu_choose) {
        switch (focus) {
          case SETTINGS_GRAPHICS: next = graphics; break;
          case SETTINGS_SOUND: next = sound; break;
          case SETTINGS_MOUSE: next = mouse; break;
          case SETTINGS_CONTROLS: next = controls; break;
        }
      }
      
      return next;
    }
    
    page* settings::escape(size_t &focus) const {
      focus = 0;
      return main_menu;
    }
    
    void settings::set_pointers(page* main_menu, page* graphics, page* sound, page* mouse, page* controls) {
      this->main_menu = main_menu;
      this->graphics = graphics;
      this->sound = sound;
      this->controls = controls;
      this->mouse = mouse;
    }
    
    size_t init_res_var(GLFWmonitor* mon, const size_t &count, const GLFWvidmode* modes, const GLFWvidmode* current_mode) {
      ASSERT(modes <= current_mode && &modes[count] > current_mode);
      (void)mon;
      return current_mode - modes;
    }
    
    uint32_t next_mode(const bool fullscreen, const GLFWvidmode* modes, const GLFWvidmode* current_mode, const uint32_t max_modes, const uint32_t current) {
      if (!fullscreen) {
        for (uint32_t i = current+1; i < max_modes; ++i) {
          if (modes[i].refreshRate != 60) continue;
          return i;
        }
        
        for (uint32_t i = 0; i < current; ++i) {
          if (modes[i].refreshRate != 60) continue;
          return i;
        }
        
        return current;
      }
      
      (void)current_mode;
      return (current+1)%max_modes;
    }
    
    uint32_t prev_mode(const bool fullscreen, const GLFWvidmode* modes, const GLFWvidmode* current_mode, const uint32_t max_modes, const uint32_t current) {
      if (!fullscreen) {
        for (uint32_t i = current-1; i < current; --i) {
          if (modes[i].refreshRate != 60) continue;
          return i;
        }
        
        for (uint32_t i = max_modes-1; i > current; --i) {
          if (modes[i].refreshRate != 60) continue;
          return i;
        }
        
        return current;
      }
      
      (void)current_mode;
      return current == 0 ? max_modes-1 : current-1;
    }
    
#define MAX_RESOLUTION_TEXT 100

    graphics::graphics(const create_info &info) : nuklear(info.nuklear), settings(info.settings), current_time(0) {
      auto settings = Global::get<utils::settings>();
      current_settings = settings->graphics;
    }
    
    page* graphics::proccess(const data::extent &screen_size, const utils::id &event, const size_t &time, size_t &focus) {
      const auto in = &nuklear->ctx.input;
      const bool mouse_movement = in->mouse.delta.x != 0.0f || in->mouse.delta.y != 0.0f;
      
      const float fov_gain1 = 1.0f;
      const float fov_gain2 = 0.1f;
      
      // монитора нет у игры в окне
      GLFWmonitor* mon = glfwGetWindowMonitor(Global::get<render::window>()->handle);
      int modes_count = 0;
      const GLFWvidmode* modes = nullptr;
      const GLFWvidmode* current_mode = nullptr;
      if (mon != nullptr) {
        modes = glfwGetVideoModes(mon, &modes_count);
        current_mode = glfwGetVideoMode(mon);
      } else {
        mon = glfwGetPrimaryMonitor();
        modes = glfwGetVideoModes(mon, &modes_count);
        current_mode = glfwGetVideoMode(mon);
      }
      
//       size_t resolution_index = init_res_var(mon, modes_count, modes, current_mode);
      ASSERT(modes_count >= 0);
      const size_t resolutions_count = modes_count;
      
      interface::style_borders s1(&nuklear->ctx, 0, 0);
      interface::style_background_color s2(&nuklear->ctx, background_menu_color);
      //const auto window_rect = nk_rect(w, h, page_size.width, page_size.height);
      const auto window_rect = nk_rect(0, 0, screen_size.width, screen_size.height);
      
      current_time += time;
      const bool blink = current_time < blink_time;
      if (current_time >= blink_time*2) {
        current_time -= blink_time*2;
      }
      
//       auto settings = Global::get<utils::settings>();
//       current_settings = settings->graphics;
      char label[MAX_RESOLUTION_TEXT];
      memset(label, 0, MAX_RESOLUTION_TEXT*sizeof(char));
      if (current_settings.fullscreen) {
        ASSERT(current_mode);
        //sprintf(label, "%ix%ix%i", current_mode->width, current_mode->height, current_mode->refreshRate);
        sprintf(label, "%ix%ix%i", modes[current_settings.video_mode].width, modes[current_settings.video_mode].height, modes[current_settings.video_mode].refreshRate);
      } else {
//         int w, h;
//         glfwGetWindowSize(Global::window()->handle(), &w, &h);
        sprintf(label, "%ix%i", current_settings.width, current_settings.height);
      }
      
      page* next = this;
      if (nk_begin(&nuklear->ctx, "graphics", window_rect, NK_WINDOW_NO_SCROLLBAR)) {
        nk_layout_space_begin(&nuklear->ctx, NK_STATIC, screen_size.height, 64);
        const auto bounds = nk_layout_space_bounds(&nuklear->ctx);
        graphics_labels_layout layout(nuklear, bounds);
        
        nk_style_set_font(&nuklear->ctx, &nuklear->fonts[fonts::interface]->handle);
        
        nk_layout_space_push(&nuklear->ctx, layout.label);
        nk_label_colored(&nuklear->ctx, "Settings", NK_TEXT_ALIGN_CENTERED, nk_color{255, 0, 0, 255});
        
        nk_style_set_font(&nuklear->ctx, &nuklear->fonts[fonts::technical]->handle);
        nk_layout_space_push(&nuklear->ctx, layout.text);
        nk_label_colored(&nuklear->ctx, "Graphics settings will be applyed when you leave this page", NK_TEXT_ALIGN_CENTERED, nk_color{255, 0, 0, 255});
        
        nk_layout_space_push(&nuklear->ctx, layout.screen_mode);
        nk_label_colored(&nuklear->ctx, "Screen mode:", NK_TEXT_ALIGN_LEFT, nk_color{255, 0, 0, 255});
        auto menu_label_bounds = nk_widget_bounds(&nuklear->ctx);
        if (nk_input_is_mouse_hovering_rect(in, menu_label_bounds) && mouse_movement) focus = GRAPHICS_FULLSCREEN;
        if (nk_input_has_mouse_click_in_rect(in, NK_BUTTON_LEFT, menu_label_bounds)) current_settings.fullscreen = !current_settings.fullscreen;
        
        nk_layout_space_push(&nuklear->ctx, layout.screen_mode_current);
        if (current_settings.fullscreen) {
          nk_label_colored(&nuklear->ctx, "fullscreen", NK_TEXT_ALIGN_LEFT, nk_color{255, 0, 0, 255});
        } else {
          nk_label_colored(&nuklear->ctx, "windowed", NK_TEXT_ALIGN_LEFT, nk_color{255, 0, 0, 255});
        }
        menu_label_bounds = nk_widget_bounds(&nuklear->ctx);
        if (nk_input_is_mouse_hovering_rect(in, menu_label_bounds) && mouse_movement) focus = GRAPHICS_FULLSCREEN;
        if (nk_input_has_mouse_click_in_rect(in, NK_BUTTON_LEFT, menu_label_bounds)) current_settings.fullscreen = !current_settings.fullscreen;
        
        nk_layout_space_push(&nuklear->ctx, layout.resolution);
        nk_label_colored(&nuklear->ctx, "Resolution:", NK_TEXT_ALIGN_LEFT, nk_color{255, 0, 0, 255});
        menu_label_bounds = nk_widget_bounds(&nuklear->ctx);
        if (nk_input_is_mouse_hovering_rect(in, menu_label_bounds) && mouse_movement) focus = GRAPHICS_SREEN_RESOLUTION;
        nk_layout_space_push(&nuklear->ctx, layout.resolution_combo);
        if (nk_combo_begin_label(&nuklear->ctx, label, nk_vec2(layout.resolution_combo.w, 400))) {
          nk_layout_row_dynamic(&nuklear->ctx, 30, 1);
          
          for (size_t i = 0; i < resolutions_count; ++i) {
            if (!current_settings.fullscreen && modes[i].refreshRate != 60) continue;
            
            if (current_settings.fullscreen) {
              sprintf(label, "%ix%ix%i", modes[i].width, modes[i].height, modes[i].refreshRate);
            } else {
              sprintf(label, "%ix%i", modes[i].width, modes[i].height);
            }
//             nk_label_colored(&nuklear->ctx, label, NK_TEXT_LEFT, nk_color{255, 0, 0, 255});
            nk_combo_item_label(&nuklear->ctx, label, NK_TEXT_LEFT);
            menu_label_bounds = nk_widget_bounds(&nuklear->ctx);
            if (nk_input_is_mouse_hovering_rect(in, menu_label_bounds) && mouse_movement) focus = GRAPHICS_SREEN_RESOLUTION;
            if (nk_input_has_mouse_click_in_rect(in, NK_BUTTON_LEFT, menu_label_bounds)) current_settings.video_mode = i;
          }
          
          nk_combo_end(&nuklear->ctx);
        }
        
        nk_layout_space_push(&nuklear->ctx, layout.fov);
        current_settings.fov = nk_propertyf(&nuklear->ctx, "FOV", 50.0f, current_settings.fov, 70.0f, fov_gain1, fov_gain2);
        menu_label_bounds = nk_widget_bounds(&nuklear->ctx);
        if (nk_input_is_mouse_hovering_rect(in, menu_label_bounds) && mouse_movement) focus = GRAPHICS_FOV;
        
        nk_layout_space_push(&nuklear->ctx, layout.image_pos[focus]);
        if (blink) nk_image(&nuklear->ctx, image_to_nk_image(render::create_image(0, 1, 0, false, false)));
        else nk_image(&nuklear->ctx, image_to_nk_image(render::create_image(0, 3, 0, false, false)));
        
        nk_layout_space_end(&nuklear->ctx);
      }
      nk_end(&nuklear->ctx);
      
      static const utils::id menu_next = utils::id::get("menu_next");
      static const utils::id menu_prev = utils::id::get("menu_prev");
      static const utils::id menu_increase = utils::id::get("menu_increase");
      static const utils::id menu_decrease = utils::id::get("menu_decrease");
      static const utils::id menu_choose = utils::id::get("menu_choose");
      
      // на самом деле у настроек будет страница выбора непосредственной настройки
      // и несколько страниц самих настроек
      static const utils::id tmr_item_pickup1 = utils::id::get("tmr_item_pickup1");
      if (event == menu_next) {
        focus = focus == 0 ? SETTINGS_COUNT-1 : focus-1;
        core::start_sound(sound::info(nullptr, tmr_item_pickup1));
      } else if (event == menu_prev) {
        focus = (focus+1)%SETTINGS_COUNT;
        core::start_sound(sound::info(nullptr, tmr_item_pickup1));
      } else if (event == menu_increase) {
        switch (focus) {
          case GRAPHICS_FULLSCREEN: current_settings.fullscreen = !current_settings.fullscreen; break;
          case GRAPHICS_SREEN_RESOLUTION: current_settings.video_mode = next_mode(current_settings.fullscreen, modes, current_mode, modes_count, current_settings.video_mode); break;
          case GRAPHICS_FOV: current_settings.fov += fov_gain1; break;
        }
        //current_settings.video_mode = (current_settings.video_mode+1)%resolutions_count;
      } else if (event == menu_decrease) {
        switch (focus) {
          case GRAPHICS_FULLSCREEN: current_settings.fullscreen = !current_settings.fullscreen; break;
          case GRAPHICS_SREEN_RESOLUTION: current_settings.video_mode = prev_mode(current_settings.fullscreen, modes, current_mode, modes_count, current_settings.video_mode); break;
          case GRAPHICS_FOV: current_settings.fov -= fov_gain1; break;
        }
        // current_settings.video_mode = current_settings.video_mode == 0 ? resolutions_count-1 : current_settings.video_mode-1;
      } else if (event == menu_choose) {
        switch (focus) {
          case GRAPHICS_FULLSCREEN: current_settings.fullscreen = !current_settings.fullscreen; break;
          case GRAPHICS_SREEN_RESOLUTION: break;
          case GRAPHICS_FOV: break;
        }
      }
      
//       if (!current_settings.fullscreen) {
        current_settings.width = modes[current_settings.video_mode].width;
        current_settings.height = modes[current_settings.video_mode].height;
//       }
      
      auto graphics_settings = &Global::get<utils::settings>()->graphics;
      if (glm::abs(current_settings.fov-graphics_settings->fov) > EPSILON) {
        Global::get<render::window>()->fov = current_settings.fov;
        Global::get<render::window>()->update_buffers();
        graphics_settings->fov = current_settings.fov;
      }
      
      return next;
    }
    
    page* graphics::escape(size_t &focus) const {
      focus = 0;
      auto graphics_settings = &Global::get<utils::settings>()->graphics;
      
      int w, h;
      glfwGetWindowSize(Global::get<render::window>()->handle, &w, &h);
      
      bool changes = false;
      
      if ((current_settings.fullscreen && !graphics_settings->fullscreen) || (current_settings.fullscreen && current_settings.video_mode != graphics_settings->video_mode)) {
        auto monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = nullptr;
        
        int count;
        auto modes = glfwGetVideoModes(monitor, &count);
        ASSERT(current_settings.video_mode < uint32_t(count));
        mode = &modes[current_settings.video_mode];
        
        glfwSetWindowMonitor(Global::get<render::window>()->handle, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        graphics_settings->fullscreen = current_settings.fullscreen;
        graphics_settings->video_mode = current_settings.video_mode;
        changes = true;
      }
      
      if ((!current_settings.fullscreen && graphics_settings->fullscreen) || (!current_settings.fullscreen && (current_settings.width != uint32_t(w) || current_settings.height != uint32_t(h)))) {
        int x, y;
        glfwGetWindowPos(Global::get<render::window>()->handle, &x, &y);
        glfwSetWindowMonitor(Global::get<render::window>()->handle, nullptr, x, y, current_settings.width, current_settings.height, 0);
        glfwSetWindowSize(Global::get<render::window>()->handle, current_settings.width, current_settings.height);
        graphics_settings->width = current_settings.width;
        graphics_settings->height = current_settings.height;
        graphics_settings->fullscreen = current_settings.fullscreen;
        changes = true;
        //std::cout << "graphics::escape width " << current_settings.width << " height " << current_settings.height << " fullscreen " << current_settings.fullscreen << '\n';
      }
      
      if (glm::abs(current_settings.fov-graphics_settings->fov) > EPSILON) {
        Global::get<render::window>()->fov = current_settings.fov;
        Global::get<render::window>()->update_buffers();
        graphics_settings->fov = current_settings.fov;
        changes = true;
      }
      
//       Global::window()->setFov(game_settings->graphics.fov);
//       if (!Global::window()->isFullscreen() && game_settings->graphics.fullscreen) {
//         Global::window()->fullscreen();
//       } else if (game_settings->graphics.width != uint32_t(w) || game_settings->graphics.height != uint32_t(h)) {
//         int x, y;
//         glfwGetWindowPos(Global::window()->handle(), &x, &y);
//         glfwSetWindowMonitor(Global::window()->handle(), nullptr, x, y, game_settings->graphics.width, game_settings->graphics.height, 0);
//       }
//       game_settings->dump(Global::getGameDir()+"settings.json");

      if (changes) {
//         Global::window()->resize();
//         nuklear->remake_font_atlas();
        Global::get<utils::settings>()->dump(Global::getGameDir()+"settings.json");
      }
      
      return settings;
    }
    
    void graphics::set_pointers(page* settings) {
      this->settings = settings;
    }
    
    save_game::save_game(const create_info &info) : nuklear(info.nuklear), main_menu(info.main_menu), current_time(0) {}
    page* save_game::proccess(const data::extent &screen_size, const utils::id &event, const size_t &time, size_t &focus) {
      // у меня сейчас нет четкого определения когда я с этой страницы выхожу
      // хотя выход я определяю как изменение возвращаемого указателя,
      // я просто не должен как то извне модифицировать этот сценарий
      
      // загрузка и сохранение делаются по одному сценарию
      // когда мы попадем на эту страницу мы должны подгрузить все сохраненки в память
      // + распарсить хедеры, по этой инфе построим интерфейс
      // я хочу чтобы в хедере еще лежал скриншот перед непосредственным сохранением
      // 2 вещи: 
      // как сделать скрин с интерфейсом и без меню? (в думе кастати не парятся и не включают интерфейс в скрин)
      // как расположить скрин в памяти и как его потом загрузить и использовать? 
      // у меня есть структура которая этим занимается, но загрузить одиночную текстурку
      // а потом удалить там довольно проблематично
      
      // лучше конечно как то отдельно загружать скриншот
      // возможно стоит парсить хедер прямо здесь, хотя в думе они как то заранее парсятся
      // наверное лучше парсить всеже один раз все, но не полностью - только хедер
      // название, метку времени, название уровня, скриншот
      
      void* data = nullptr;
      if (data == nullptr) {
        // парсим файлы
      }
      
      page* next = this;
      
      if (next != this) {
        // удаляем?
      }
      
      return next;
    }
    
    page* save_game::escape(size_t &focus) const {
      focus = 0;
      return main_menu;
    }
    
    void save_game::set_pointers(page* main_menu) {
      this->main_menu = main_menu;
    }
    
    quit_game::quit_game(const create_info &info) : nuklear(info.nuklear), main_menu(info.main_menu), quit(info.quit), current_time(0) {}
    page* quit_game::proccess(const data::extent &screen_size, const utils::id &command, const size_t &time, size_t &focus) {
      const auto in = &nuklear->ctx.input;
      const bool mouse_movement = in->mouse.delta.x != 0.0f || in->mouse.delta.y != 0.0f;
      
      interface::style_borders s1(&nuklear->ctx, 0, 0);
      interface::style_background_color s2(&nuklear->ctx, background_menu_color);
      //const auto window_rect = nk_rect(w, h, page_size.width, page_size.height);
      const auto window_rect = nk_rect(0, 0, screen_size.width, screen_size.height);
      
      current_time += time;
      const bool blink = current_time < blink_time;
      if (current_time >= blink_time*2) {
        current_time -= blink_time*2;
      }
      
      page* next = this;
      if (nk_begin(&nuklear->ctx, "quit_game", window_rect, NK_WINDOW_NO_SCROLLBAR)) {
        nk_layout_space_begin(&nuklear->ctx, NK_STATIC, screen_size.height, 64);
        const auto bounds = nk_layout_space_bounds(&nuklear->ctx);
        quit_labels_layout layout(nuklear, bounds);
        
        // в думе на выходе написан небольшой текст + "название страницы" + выбор да/нет
        // причем выбор сделан не с помощью изображения как раньше, а с помощью символа 
        // треугольника, придумать какой нибудь символ или сделать изображение?
        nk_style_set_font(&nuklear->ctx, &nuklear->fonts[fonts::interface]->handle);
        nk_layout_space_push(&nuklear->ctx, layout.label);
        nk_label_colored(&nuklear->ctx, "Quit game", NK_TEXT_ALIGN_CENTERED, nk_color{255, 0, 0, 255});
        
        nk_style_set_font(&nuklear->ctx, &nuklear->fonts[fonts::technical]->handle);
        nk_layout_space_push(&nuklear->ctx, layout.quit_text);
        nk_label_colored(&nuklear->ctx, "Do you really want to quit?", NK_TEXT_ALIGN_CENTERED, nk_color{255, 0, 0, 255});
        
        nk_layout_space_push(&nuklear->ctx, layout.opt_no);
        nk_label_colored(&nuklear->ctx, "No", NK_TEXT_ALIGN_LEFT, nk_color{255, 0, 0, 255});
        auto quit_label_bounds = nk_widget_bounds(&nuklear->ctx);
        if (nk_input_is_mouse_hovering_rect(in, quit_label_bounds) && mouse_movement) focus = QUIT_GAME_NO;
        if (nk_input_has_mouse_click_in_rect(in, NK_BUTTON_LEFT, quit_label_bounds)) next = main_menu;
        
        nk_layout_space_push(&nuklear->ctx, layout.opt_yes);
        nk_label_colored(&nuklear->ctx, "Yes", NK_TEXT_ALIGN_LEFT, nk_color{255, 0, 0, 255});
        quit_label_bounds = nk_widget_bounds(&nuklear->ctx);
        if (nk_input_is_mouse_hovering_rect(in, quit_label_bounds) && mouse_movement) focus = QUIT_GAME_YES;
        if (nk_input_has_mouse_click_in_rect(in, NK_BUTTON_LEFT, quit_label_bounds)) {
          *quit = true;
          next = nullptr;
        }
        
        if (blink) {
          nk_layout_space_push(&nuklear->ctx, layout.image_pos[focus]);
          nk_label_colored(&nuklear->ctx, "#", NK_TEXT_ALIGN_CENTERED, nk_color{255, 0, 0, 255});
        }
        
        nk_layout_space_end(&nuklear->ctx);
      }
      nk_end(&nuklear->ctx);
      
      static const utils::id menu_next = utils::id::get("menu_next");
      static const utils::id menu_prev = utils::id::get("menu_prev");
//       static const utils::id menu_increase = utils::id::get("menu_increase");
//       static const utils::id menu_decrease = utils::id::get("menu_decrease");
      static const utils::id menu_choose = utils::id::get("menu_choose");
      
      if (command == menu_next) focus = focus == 0 ? QUIT_COUNT-1 : focus-1;
      else if (command == menu_prev) focus = (focus+1)%QUIT_COUNT;
      else if (command == menu_choose) {
        if (focus == QUIT_GAME_NO) next = escape(focus);
        else {
          *quit = true;
          next = nullptr;
        }
      }
      
      return next;
    }
    
    page* quit_game::escape(size_t &focus) const {
      focus = 0;
      return main_menu;
    }
    
    void quit_game::set_pointers(page* main_menu) {
      this->main_menu = main_menu;
    }

    menu_labels_layout::menu_labels_layout(context* ctx, const struct nk_rect &bounds) {
      const float font_height = 72.0f;
      const float offset_y = 3;
      const float offset_x = 10;
      const float menu_height = MENU_COUNT * offset_y*2 + MENU_COUNT*font_height;
      const float half_menu_height = menu_height/2.0f;
      const float y = bounds.h/2.0f - half_menu_height;
      const float label_height = offset_y + font_height + offset_y;
      const float menu_width = bounds.w * 0.25f;
      const float x = bounds.w/2.0f - menu_width/2.0f;
      
//       std::cout << "space bounds x " << bounds.x << " y " << bounds.y << " w " << bounds.w << " h " << bounds.h << "\n";
      {
        const float offset_to_menu = y + label_height*0-offset_y;
        logo = nk_rect(0, offset_to_menu/2.0f-50, bounds.w, offset_to_menu);
      }
      
      {
        new_game  = nk_rect(x, y + label_height*0-offset_y, menu_width, font_height);
      }
      
      {
        options   = nk_rect(x, y + label_height*1-offset_y, menu_width, font_height);
      }
      
      {
        load_game = nk_rect(x, y + label_height*2-offset_y, menu_width, font_height);
      }
      
      {
        quit_game = nk_rect(x, y + label_height*3-offset_y, menu_width, font_height);
      }
      
      {
        const float image_width = font_height;
        const float image_x = x - offset_x - image_width;
        for (size_t i = 0; i < MENU_COUNT; ++i) {
          image_pos[i] = nk_rect(image_x, y + label_height*i-offset_y, image_width, font_height);
        }
      }
      
      (void)ctx;
    }
    
    settings_labels_layout::settings_labels_layout(context* ctx, const struct nk_rect &bounds) {
      const float label_font = ctx->fonts[fonts::interface]->handle.height;
      const float text_font = ctx->fonts[fonts::technical]->handle.height;
      const float offset_y = 3;
      const float offset_x = 10;
      const float settings_label_height = SETTINGS_COUNT * offset_y*2 + SETTINGS_COUNT*text_font;
      const float half_quit_height = settings_label_height/2.0f;
      const float y = bounds.h/2.0f - half_quit_height;
      const float label_height = offset_y + text_font + offset_y;
      const float menu_width = 50;
      const float x = bounds.w/2.0f - menu_width/2.0f;
      
      graphics = nk_rect(x, y+label_height*0-offset_y, menu_width, text_font);
      const float offset_to_menu = y + label_height*0-offset_y;
      label = nk_rect(0, offset_to_menu/2.0f-label_font/2.0f, bounds.w, offset_to_menu);
      sound = nk_rect(x, y+label_height*1-offset_y, menu_width, text_font);
      mouse = nk_rect(x, y+label_height*2-offset_y, menu_width, text_font);
      controls = nk_rect(x, y+label_height*3-offset_y, menu_width, text_font);
      const float image_width = 30;
      const float image_x = x - offset_x - image_width;
      for (size_t i = 0; i < QUIT_COUNT; ++i) {
        image_pos[i] = nk_rect(image_x, y + label_height*i-offset_y, image_width, text_font);
      }
    }
    
    graphics_labels_layout::graphics_labels_layout(context* ctx, const struct nk_rect &bounds) {
      const float label_font = ctx->fonts[fonts::interface]->handle.height;
      const float text_font = ctx->fonts[fonts::technical]->handle.height;
      const float offset_y = 3;
      const float offset_x = 10;
      const float settings_label_height = SETTINGS_COUNT * offset_y*2 + SETTINGS_COUNT*text_font;
      const float half_quit_height = settings_label_height/2.0f;
      const float y = bounds.h/2.0f - half_quit_height;
      const float label_height = offset_y + text_font + offset_y;
      const float menu_width = 300;
      const float x = bounds.w/2.0f - menu_width/2.0f;
      
      const float offset_to_menu = y + label_height*0-offset_y;
      label = nk_rect(0, offset_to_menu/2.0f-label_font/2.0f, bounds.w, label_font);
      text = nk_rect(0, screen_mode.y-offset_y-text_font, bounds.w, text_font);
      screen_mode = nk_rect(x, y + label_height*0-offset_y, menu_width, text_font);
      screen_mode_current = nk_rect(x+menu_width, y + label_height*0-offset_y, menu_width, text_font);
      resolution = nk_rect(x, y + label_height*1-offset_y, menu_width, text_font);
      resolution_combo = nk_rect(x+menu_width, y + label_height*1-offset_y, menu_width, text_font);
      fov = nk_rect(x, y + label_height*2-offset_y, menu_width, text_font);
      const float image_width = 30;
      const float image_x = x - offset_x - image_width;
      for (size_t i = 0; i < GRAPHICS_COUNT; ++i) {
        image_pos[i] = nk_rect(image_x, y + label_height*i-offset_y, image_width, text_font);
      }
    }
    
    quit_labels_layout::quit_labels_layout(context* ctx, const struct nk_rect &bounds) {
      // у нас есть 3 секции: название менюшки, текст при выходе, и выбор да/нет
      // название меню, наверное, должно быть написано большим шрифтом, остальные вещи 
      // шрифтом меньшим 
      const float label_font = ctx->fonts[fonts::interface]->handle.height;
      const float text_font = ctx->fonts[fonts::technical]->handle.height;
      const float offset_y = 3;
      const float offset_x = 10;
      const float yesno_height = QUIT_COUNT * offset_y*2 + QUIT_COUNT*text_font;
      const float half_quit_height = yesno_height/2.0f;
      const float y = bounds.h/2.0f - half_quit_height;
      const float label_height = offset_y + text_font + offset_y;
      const float menu_width = 50;
      const float x = bounds.w/2.0f - menu_width/2.0f;
      
      {
        opt_no = nk_rect(x, y+label_height*0-offset_y, menu_width, text_font);
      }
      
      {
        opt_yes = nk_rect(x, y+label_height*1-offset_y, menu_width, text_font);
      }
      
      {
        const float offset_to_menu = y + label_height*0-offset_y;
        label = nk_rect(0, offset_to_menu/2.0f-label_font/2.0f, bounds.w, offset_to_menu);
      }
      
      {
        quit_text = nk_rect(0, opt_no.y-text_font-offset_y, bounds.w, text_font);
      }
      
      {
        const float image_width = 30;
        const float image_x = x - offset_x - image_width;
        for (size_t i = 0; i < QUIT_COUNT; ++i) {
          image_pos[i] = nk_rect(image_x, y + label_height*i-offset_y, image_width, text_font);
        }
      }
    }
  }
}
