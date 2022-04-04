#include "lua_initialization_hidden.h"

#include "magic_enum_header.h"
#include "globals.h"
#include "systems.h"
#include "game_context.h"

#include "bin/interface_context.h"

#include "render/utils.h"
#include "render/stages.h"

#include "core/titulus.h"
#include "core/realm.h"
#include "core/context.h"

#include "script/context.h"

namespace devils_engine {
  namespace utils {
    void setup_lua_interface_core(sol::state_view lua) {
      auto core = lua[magic_enum::enum_name(reserved_lua::core)].get_or_create<sol::table>();
      core.set_function("toggle_border_rendering", [] () {
        auto tile_opt = global::get<render::tile_optimizer>();
        if (tile_opt == nullptr) throw std::runtime_error("Bad game state. Could not get world map");
        tile_opt->set_border_rendering(!tile_opt->is_rendering_border());
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
          auto render = global::get<render::heraldies_render>();
          const render::heraldies_render::heraldy_interface_data data{
            titulus->heraldy_container.data(),
            titulus->heraldy_layers_count
          };
          const size_t index = render->add(data);
          //const uint32_t heraldy_index = titulus->heraldy;
          struct nk_image img;
          memset(&img, 0, sizeof(img));
          
          const image_handle_data id {IMAGE_TYPE_HERALDY, index == SIZE_MAX ? GPU_UINT_MAX : uint32_t(index)};
          const auto h = image_data_to_nk_handle(id);
          img.handle = h;
          
          nk_image(ctx, img);
          return;
        }
        
        // геральдика по идее присутствует только у титулов и династий
        throw std::runtime_error("What needs to be done with other objects?");
      });
      
      interface.set_function("heraldy_button", [] (const sol::object &nk_ctx, const sol::object &obj) -> nk_bool {
        (void)nk_ctx;
        auto ctx = &global::get<systems::core_t>()->context->ctx;
        if (obj.is<core::titulus*>()) {
          auto titulus = obj.as<core::titulus*>();
          //const uint32_t heraldy_index = titulus->heraldy;
          auto render = global::get<render::heraldies_render>();
          const render::heraldies_render::heraldy_interface_data data{
            titulus->heraldy_container.data(),
            titulus->heraldy_layers_count
          };
          const size_t index = render->add(data);
          struct nk_image img;
          memset(&img, 0, sizeof(img));
          
          image_handle_data id;
          id.type = IMAGE_TYPE_HERALDY;
          id.data = index == SIZE_MAX ? GPU_UINT_MAX : uint32_t(index);
          const auto h = image_data_to_nk_handle(id);
          img.handle = h;
          
          const nk_bool ret = nk_button_image(ctx, img);
          return ret;
        }
        
        throw std::runtime_error("What needs to be done with other objects?");
        return 0;
      });
      
      interface.set_function("image", [] (const sol::object &nk_ctx, const uint32_t &handle) -> void {
        (void)nk_ctx;
        auto ctx = &global::get<systems::core_t>()->context->ctx;
        
        //const render::image_t render_img {handle};
        struct nk_image img;
        memset(&img, 0, sizeof(img));
        
        const image_handle_data id {IMAGE_TYPE_DEFAULT, handle};
        const auto h = image_data_to_nk_handle(id);
        img.handle = h;
        
        nk_image(ctx, img);
      });
      
      auto utils = lua["utils"].get_or_create<sol::table>();
      utils.set_function("set_selection_box", [] (const double &minx, const double &miny, const double &maxx, const double &maxy) {
        const glm::dvec2 min(minx, miny);
        const glm::dvec2 max(maxx, maxy);
        
        render::set_selection_frustum(min, max);
      });
      
      utils.set_function("get_interactions", [] (sol::this_state s, const sol::object first, const sol::object second) {
        // нужно вернуть список взаимодействий которые потенциально могут быть вызваны игроком
        // как это делается? во первых интеракция всегда исходит от персонажа (даже интеракция апати)
        // интеракция может быть потенциально с любым объектом (то есть тип интеракции определяется вторым объектом)
        // видимо при создании интеракции нужно указать тип получателя
        if (!first.is<core::character*>()) throw std::runtime_error("Expected character in get_interactions function");
        
        sol::state_view lua = s;
        auto table = lua.create_table(100, 0);
        auto ctx = global::get<systems::map_t>()->core_context;
        script::context script_ctx;
        // положим первый объект в рут и в "context:actor", а второй в "context:recipient"
        script_ctx.root = script::object(first.as<core::character*>());
        script_ctx.map.emplace(std::make_pair(std::string_view("actor"), script_ctx.root));
        for (size_t i = 0; i < ctx->get_entity_count<core::interaction>(); ++i) {
          auto inter = ctx->get_entity<core::interaction>(i);
          const bool ret = inter->potential.compute(&script_ctx);
          // нужно ли "компилировать" интеракцию? игроку то наверное вообще все равно
          if (ret) table.add(inter);
        }
        
        return table;
      });
      
      utils.set_function("get_decisions", [] (sol::this_state s, const sol::object first) {
        // все десижоны вызываются от персонажа
        // нужно просто пройтись по тем что мы можем потенциально вызвать
        if (!first.is<core::character*>()) throw std::runtime_error("Expected character in get_decisions function");
        
        sol::state_view lua = s;
        auto table = lua.create_table(100, 0);
        auto ctx = global::get<systems::map_t>()->core_context;
        script::context script_ctx;
        script_ctx.root = script::object(first.as<core::character*>());
        for (size_t i = 0; i < ctx->get_entity_count<core::decision>(); ++i) {
          auto d = ctx->get_entity<core::decision>(i);
          const bool ret = d->potential.compute(&script_ctx);
          // нужно ли "компилировать" решение?
          if (ret) table.add(d);
        }
        
        return table;
      });
      
      auto core = lua["core"].get_or_create<sol::table>();
      core.set_function("reload_interface", [] () {
        auto game_ctx = global::get<systems::core_t>()->game_ctx;
        game_ctx->reload_interface = true;
      });
    }
  }
}
