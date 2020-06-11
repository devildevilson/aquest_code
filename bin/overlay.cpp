#include "overlay.h"
#include "interface_context.h"
#include "utils/globals.h"

namespace devils_engine {
  namespace overlay {
    void debug(const uint32_t &picked_tile_index) {
      if (picked_tile_index == UINT32_MAX) return;
      
      auto interface = global::get<interface::context>();
      auto ctx = &interface->ctx;
      
      auto old_color = ctx->style.window.background;
      interface::style::background_color bg_color(ctx, nk_color{old_color.r, old_color.g, old_color.b, 128});
      
      if (nk_begin(ctx, "debug interface", nk_rect(5, 5, 400, 400), NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND)) {
        nk_style_set_font(ctx, &global::get<interface::context>()->fonts[fonts::technical]->handle);
        nk_layout_row_static(ctx, 30.0f, 400, 1);
        {
          const std::string str = "Tile index " + std::to_string(picked_tile_index);
          nk_label(ctx, str.c_str(), NK_TEXT_LEFT);
        }
      }
      nk_end(ctx);
      
    }
  }
}
