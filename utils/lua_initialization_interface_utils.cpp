#include "lua_initialization_hidden.h"

#include "magic_enum_header.h"
#include "globals.h"
#include "systems.h"
#include "bin/interface_context.h"
#include "render/utils.h"
#include "core/titulus.h"
#include "core/realm.h"
#include "game_context.h"
#include "render/stages.h"

namespace devils_engine {
  namespace utils {
    void setup_lua_interface_core(sol::state_view lua) {
      auto core = lua[magic_enum::enum_name(reserved_lua::core)].get_or_create<sol::table>();
      core.set_function("toggle_border_rendering", [] () {
        auto tile_opt = global::get<render::tile_optimizer>();
        if (tile_opt == nullptr) throw std::runtime_error("Bad game state. Could not get world map");
        tile_opt->set_border_rendering(!tile_opt->is_rendering_border());
      });
      
      core.set_function("each_title", [] (const core::realm* f, const sol::function &function) {
        if (f == nullptr) throw std::runtime_error("each_title: Invalid realm");
                        
        auto title = f->titles;
        while (title != nullptr) {
          const auto ret = function(title);
          if (!ret.valid()) {
            sol::error err = ret;
            std::cout << err.what();
            throw std::runtime_error("There is lua errors");
          }
          
          if (ret.get_type() == sol::type::boolean) {
            const bool val = ret;
            if (val) break;
          }
                        
          title = title->next;
        }
      });
    }
    
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
