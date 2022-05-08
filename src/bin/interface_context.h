#ifndef INTERFACE_CONTEXT_H
#define INTERFACE_CONTEXT_H

#include "render/nuklear_header.h"
#include "render/shared_structures.h"
#include "render/vulkan_declarations.h"
#include "render/push_constant_data.h"
#include <vector>
#include <cstdint>
#include <string>
// #include "utils/sol.h"

#define NUKLEAR_SAMPLER_ID 2

namespace devils_engine {
  struct image_handle_data {
    uint32_t type;
    uint32_t data;
  };
  
  nk_handle nk_handle_image(const render::image_t &img);
  render::image_t image_nk_handle(const nk_handle &handle);
  struct nk_image image_to_nk_image(const render::image_t &img);
  nk_handle image_data_to_nk_handle(const image_handle_data &data);
  image_handle_data nk_handle_to_image_data(const nk_handle &handle);
  
  namespace render {
    struct window;
    struct container;
    class image_container;
  }
  
  namespace fonts {
    enum indices {
      interface,
      technical,
      menu,
      description,
      count
    };
  }
  
  namespace interface {
    namespace data {
      struct PressingData {
        uint32_t button;
        uint32_t modifier;
      };

      struct MousePos {
        float x;
        float y;
      };
      
      // что вообще мне может потребоваться? 
      // на самом деле это определятся строго теми виджетами которые я использую
      // мне бы 
      enum class command {
        next,
        prev,
        increase,
        decrease,
        choose,
        none
      };
    }
    
    struct context {
      struct fonts_settings2 {
        std::string name;
        float size;
      };
      
      render::container* render_container;
      render::image_container* container;
      nk_context ctx;
      nk_font* fonts[fonts::count];
      // текстурки шрифтов сложно положить в пул как я раньше это делал
      // но при этом нужно сочетать такие текстурки со всеми остальными
      // нужно создавать пулы из одной картинки
      nk_font_atlas atlas; // атлас единственный?
      render::image_t font_atlas_image;
      render::push_constant_data font_atlas_image_data;
      vk::DescriptorSet* atlas_descriptor;
      size_t descriptor_index;
      nk_draw_null_texture null;
      nk_buffer cmds;
      std::vector<fonts_settings2> fonts_data;
      
      context(render::container* render_container, render::window* window, render::image_container* container);
      ~context();
      void remake_font_atlas(const uint32_t &window_width, const uint32_t &window_height);
    };
    
    bool is_interface_hovered(nk_context* ctx, const std::string_view &except);
    
    namespace style {
      struct borders {
        nk_context* ctx;
        float old_window_border;
        float old_group_border;
        
        borders(nk_context* ctx, const float &window, const float &group);
        ~borders();
      };
      
      struct background_color {
        nk_context* ctx;
        nk_color old_color;
        nk_style_item old_style_item;
        
        background_color(nk_context* ctx, const nk_color &color);
        ~background_color();
      };
    }
  }
}

#endif
