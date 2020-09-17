#ifndef INTERFACE_CONTEXT_H
#define INTERFACE_CONTEXT_H

#include "render/nuklear_header.h"
#include "render/shared_structures.h"
#include <vector>
#include <cstdint>
// #include "utils/sol.h"

namespace yavf {
  class Device;
  class ImageView;
  class DescriptorSet;
}

namespace devils_engine {
  nk_handle nk_handle_image(const render::image_t &img);
  render::image_t image_nk_handle(const nk_handle &handle);
  struct nk_image image_to_nk_image(const render::image_t &img);
  
  namespace render {
    struct window;
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
      
      struct extent {
        float width;
        float height;
      };
      
      struct offset {
        float x;
        float y;
      };
      
      struct rect {
        data::offset offset;
        data::extent extent;
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
      yavf::Device* device;
      nk_context ctx;
      nk_font* fonts[fonts::count];
      // текстурки шрифтов сложно положить в пул как я раньше это делал
      // но при этом нужно сочетать такие текстурки со всеми остальными
      // нужно создавать пулы из одной картинки
      nk_font_atlas atlas; // атлас единственный?
      yavf::ImageView* view;
      yavf::DescriptorSet* atlas_descriptor;
      size_t descriptor_index;
      nk_draw_null_texture null;
      nk_buffer cmds;
//       sol::object moonnuklear_ctx;
//       sol::object moonnuklear_font;
      
      context(yavf::Device* device, render::window* window);
      ~context();
      void remake_font_atlas(const uint32_t &window_width, const uint32_t &window_height);
    };
    
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
