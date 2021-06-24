#include "lua_initialization.h"

#include "globals.h"
#include "systems.h"
#include "bin/interface_context.h"
#include "render/utils.h"
#include "core/titulus.h"
#include "game_context.h"

namespace devils_engine {
  namespace utils {
    void setup_lua_interface_utils(sol::state_view lua) {
      auto interface = lua["interface"].get_or_create<sol::table>();
      interface.set_function("is_hovered", [] (const std::string_view &except) {
        auto i = global::get<systems::core_t>()->context;
        return devils_engine::interface::is_interface_hovered(&i->ctx, except);
      });
      
      interface.set_function("heraldy_image", [] (const sol::object &nk_ctx, const sol::object &obj) -> void {
        (void)nk_ctx;
        auto ctx = &global::get<systems::core_t>()->context->ctx;
        if (obj.is<core::titulus*>()) {
          auto titulus = obj.as<core::titulus*>();
          const uint32_t heraldy_index = titulus->heraldy;
          struct nk_image img;
          memset(&img, 0, sizeof(img));
          
          image_handle_data id;
          id.type = IMAGE_TYPE_HERALDY;
          id.data = heraldy_index;
          const auto h = image_data_to_nk_handle(id);
          img.handle = h;
          
          nk_image(ctx, img);
          return;
        }
        
        throw std::runtime_error("What needs to be done with other objects?");
      });
      
      interface.set_function("heraldy_button", [] (const sol::object &nk_ctx, const sol::object &obj) -> nk_bool {
        (void)nk_ctx;
        auto ctx = &global::get<systems::core_t>()->context->ctx;
        if (obj.is<core::titulus*>()) {
          auto titulus = obj.as<core::titulus*>();
          const uint32_t heraldy_index = titulus->heraldy;
          struct nk_image img;
          memset(&img, 0, sizeof(img));
          
          image_handle_data id;
          id.type = IMAGE_TYPE_HERALDY;
          id.data = heraldy_index;
          const auto h = image_data_to_nk_handle(id);
          img.handle = h;
          
          const nk_bool ret = nk_button_image(ctx, img);
          return ret;
        }
        
        throw std::runtime_error("What needs to be done with other objects?");
        return 0;
      });
      
      auto utils = lua["utils"].get_or_create<sol::table>();
      utils.set_function("set_selection_box", [] (const double &minx, const double &miny, const double &maxx, const double &maxy) {
        const glm::dvec2 min(minx, miny);
        const glm::dvec2 max(maxx, maxy);
        
        render::set_selection_frustum(min, max);
      });
      
      auto core = lua["core"].get_or_create<sol::table>();
      core.set_function("reload_interface", [] () {
        auto game_ctx = global::get<systems::core_t>()->game_ctx;
        game_ctx->reload_interface = true;
      });
    }
  }
}
