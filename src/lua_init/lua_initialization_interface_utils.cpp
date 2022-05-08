#include "lua_initialization_hidden.h"

#include "utils/magic_enum_header.h"
#include "utils/globals.h"
#include "utils/systems.h"
#include "utils/game_context.h"
#include "utils/frame_allocator.h"
#include "utils/constexpr_funcs.h"

#include "bin/interface_context.h"
#include "bin/heraldy_parser.h"

#include "render/utils.h"
#include "render/stages.h"
#include "render/push_constant_data.h"
#include "render/image_controller.h"
#include "render/image_container.h"

#include "core/titulus.h"
#include "core/realm.h"
#include "core/context.h"
#include "core/render_stages.h"

#include "script/context.h"

namespace devils_engine {
  namespace utils {
    struct interface_image {
      render::image_t img;
      uint16_t w, h;
      uint16_t region[4];
      
      inline interface_image() : img{ GPU_INT_MAX }, w(0), h(0), region{ 0,0,0,0 } {}
      inline interface_image(const render::image_t img) : img(img), w(0), h(0), region{ 0,0,0,0 } {}
      inline interface_image(const render::image_t img, const uint16_t w, const uint16_t h) : img(img), w(w), h(h), region{ 0,0,0,0 } {}
      inline interface_image(const render::image_t img, const uint16_t w, const uint16_t h, const uint16_t rx, const uint16_t ry, const uint16_t rw, const uint16_t rh) : 
        img(img), w(w), h(h), region{ rx, ry, rw, rh } {}
    };
    
    static void setup_wh_region(const interface_image &data, struct nk_image &img) {
      static_assert(sizeof(data.region) == sizeof(img.region));
      img.w = data.w;
      img.h = data.h;
      memcpy(img.region, data.region, sizeof(data.region[0]) * 4);
    }
    
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
      //interface.new_usertype<render::image_t>("image_handle", sol::no_constructor);
      interface.new_usertype<interface_image>("interface_image", sol::no_constructor);
      interface.new_usertype<render::heraldy_layer_t>("layer_data", sol::no_constructor);
      
      interface.set_function("get_image_size", [] (const render::image_t &img) {
        const auto cont = global::get<systems::core_t>()->image_container;
        const auto pool = cont->get_pool(render::get_image_index(img));
        if (pool == nullptr) return std::make_tuple(size_t(0), size_t(0));
        const auto &size = pool->image_size();
        return std::make_tuple(size_t(size.width), size_t(size.height));
      });
      
      interface.set_function("get_layer_stencil", [] (const render::heraldy_layer_t &layer) {
        return interface_image(layer.stencil);
      });
      
      interface.set_function("get_layer_image", [] (const render::heraldy_layer_t &layer) {
        return interface_image(layer.image);
      });
      
      // может быть при ошибке возвращать пустое изображение? фиг знает
      // сюда можно добавить характеристики картинки из наклира (то есть поля w, h, region) (нужно вернуть interface_image)
      interface.set_function("get_image", [] (const std::string_view &id, const sol::variadic_args args) {
        std::array<std::string_view, 2> mod_img_name;
        const size_t count = divide_token(id, "/", 2, mod_img_name.data());
        if (count != SIZE_MAX) throw std::runtime_error("Invalid image id " + std::string(id));
        assert(mod_img_name[0] == "apates_quest");
        const auto cont = global::get<systems::core_t>()->image_controller;
        const auto img = utils::find_image(cont, mod_img_name[1]);
        uint16_t w = 0;
        uint16_t h = 0;
        uint16_t reg[4] = { 0, 0, 0, 0 };
        for (const auto &arg : args) {
          if (arg.is<uint16_t>()) {
            if (w == 0) w = arg.as<uint16_t>();
            else h = arg.as<uint16_t>();
          }
          
          if (arg.get_type() == sol::type::table) {
            const auto reg_table = arg.as<sol::table>();
            reg[0] = reg_table[1];
            reg[1] = reg_table[2];
            reg[2] = reg_table[3];
            reg[3] = reg_table[4];
          }
        }
        return interface_image(img, w, h, reg[0], reg[1], reg[2], reg[3]);
      });
      
      interface.set_function("make_subimage", [] (const interface_image &img, const uint16_t w, const uint16_t h, const sol::table &region) {
        return interface_image(img.img, w, h, region[1], region[2], region[3], region[4]);
      });
      
      interface.set_function("is_hovered", [] (const std::string_view &except) {
        auto i = global::get<systems::core_t>()->context;
        return devils_engine::interface::is_interface_hovered(&i->ctx, except);
      });
      
      interface.set_function("heraldy_image", [] (const sol::object &nk_ctx, const sol::object &obj, const sol::object &stencil) -> void {
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
          
          //const image_handle_data id {IMAGE_TYPE_HERALDY, index == SIZE_MAX ? GPU_UINT_MAX : uint32_t(index)};
          //const auto h = image_data_to_nk_handle(id);
          
          interface_image stencil_img;
          if (stencil.valid() && stencil.is<interface_image>()) {
            stencil_img = stencil.as<interface_image>();
          }
          
          auto ptr = global::get<utils::frame_allocator>()->create<render::push_constant_data>();
          *ptr = { IMAGE_TYPE_HERALDY, index == SIZE_MAX ? GPU_UINT_MAX : uint32_t(index), stencil_img.img.container, GPU_UINT_MAX };
          img.handle.ptr = ptr;
          setup_wh_region(stencil_img, img);
          
          nk_image(ctx, img);
          return;
        }
        
        // геральдика по идее присутствует только у титулов и династий
        throw std::runtime_error("What needs to be done with other objects?");
      });
      
      interface.set_function("heraldy_button", [] (const sol::object &nk_ctx, const sol::object &obj, const sol::object &stencil) -> nk_bool {
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
          
//           image_handle_data id;
//           id.type = IMAGE_TYPE_HERALDY;
//           id.data = index == SIZE_MAX ? GPU_UINT_MAX : uint32_t(index);
//           const auto h = image_data_to_nk_handle(id);
//           img.handle = h;
          
          interface_image stencil_img;
          if (stencil.valid() && stencil.is<interface_image>()) {
            stencil_img = stencil.as<interface_image>();
          }
          
          auto ptr = global::get<utils::frame_allocator>()->create<render::push_constant_data>();
          *ptr = { IMAGE_TYPE_HERALDY, index == SIZE_MAX ? GPU_UINT_MAX : uint32_t(index), stencil_img.img.container, GPU_UINT_MAX };
          img.handle.ptr = ptr;
          setup_wh_region(stencil_img, img);
          
          const nk_bool ret = nk_button_image(ctx, img);
          return ret;
        }
        
        throw std::runtime_error("What needs to be done with other objects?");
        return 0;
      });
      
      interface.set_function("image", [] (const sol::object &nk_ctx, const interface_image &handle) -> void {
        (void)nk_ctx;
        auto ctx = &global::get<systems::core_t>()->context->ctx;
        
        //const render::image_t render_img {handle};
        struct nk_image img;
        memset(&img, 0, sizeof(img));
        
        //const image_handle_data id {IMAGE_TYPE_DEFAULT, handle};
        //const auto h = image_data_to_nk_handle(id);
        auto ptr = global::get<utils::frame_allocator>()->create<render::push_constant_data>();
        *ptr = { IMAGE_TYPE_DEFAULT, handle.img.container, GPU_UINT_MAX, GPU_UINT_MAX };
        img.handle.ptr = ptr;
        setup_wh_region(handle, img);
        
        nk_image(ctx, img);
      });
      
      // можно ли как нибудь ипользовать уже заданные слои геральдики? тогда нужно заводить буфер индексов
      interface.set_function("compile_layers", [] (sol::this_state s, const sol::table &layers) {
        sol::state_view lua = s;
        size_t counter = 0;
        const size_t maximum_image_layers = 16;
        auto layers_table = lua.create_table(maximum_image_layers, 0);
        
        auto controller = global::get<systems::core_t>()->image_controller;
        for (const auto &pair : layers) {
          if (pair.second.get_type() != sol::type::table) continue;
          if (counter >= maximum_image_layers) break; // может вернуть nil?
          
          const auto t = pair.second.as<sol::table>();
          const auto l = parse_heraldy_layer(controller, t);
          layers_table[TO_LUA_INDEX(counter)] = l;
          ++counter;
        }
        
        return layers_table;
      });
      
      interface.set_function("layered_image", [] (const sol::object &nk_ctx, const sol::table &layers, const sol::object &stencil) {
        (void)nk_ctx;
        auto ctx = &global::get<systems::core_t>()->context->ctx;
        
        size_t counter = 0;
        const size_t maximum_image_layers = 16;
        std::array<render::heraldy_layer_t, maximum_image_layers> buffer;
        for (const auto &pair : layers) {
          if (!pair.second.is<render::heraldy_layer_t>()) continue;
          if (counter >= maximum_image_layers) break;
          
          const auto l = pair.second.as<render::heraldy_layer_t>();
          buffer[counter] = l;
          ++counter;
        }
        
        const uint32_t index = global::get<render::interface_stage>()->add_layers(counter, buffer.data());
        
        interface_image stencil_img;
        if (stencil.valid() && stencil.is<interface_image>()) {
          stencil_img = stencil.as<interface_image>();
        }
        
        auto ptr = global::get<utils::frame_allocator>()->create<render::push_constant_data>();
        *ptr = { IMAGE_TYPE_LAYERED_IMAGE, index, stencil_img.img.container, GPU_UINT_MAX };
        struct nk_image img;
        memset(&img, 0, sizeof(img));
        img.handle.ptr = ptr;
        setup_wh_region(stencil_img, img);
        
        nk_image(ctx, img);
      });
      
      interface.set_function("alpha_stencil", [] (const sol::object &nk_ctx, const interface_image &stencil_img, const double &alpha, const uint32_t &color) {
        (void)nk_ctx;
        auto ctx = &global::get<systems::core_t>()->context->ctx;
        
        struct nk_image img;
        memset(&img, 0, sizeof(img));
        
        auto ptr = global::get<utils::frame_allocator>()->create<render::push_constant_data>();
        *ptr = { IMAGE_TYPE_ALPHA_STENCIL, glm::floatBitsToUint(alpha), stencil_img.img.container, color };
        img.handle.ptr = ptr;
        setup_wh_region(stencil_img, img);
        
        nk_image(ctx, img);
      });
      
      auto utils = lua["utils"].get_or_create<sol::table>();
      utils.set_function("set_selection_box", [] (const double &minx, const double &miny, const double &maxx, const double &maxy) {
        const glm::dvec2 min(minx, miny);
        const glm::dvec2 max(maxx, maxy);
        
        render::set_selection_frustum(min, max);
      });
      
//       utils.set_function("get_interactions", [] (sol::this_state s, const sol::object first, const sol::object second) {
//         // нужно вернуть список взаимодействий которые потенциально могут быть вызваны игроком
//         // как это делается? во первых интеракция всегда исходит от персонажа (даже интеракция апати)
//         // интеракция может быть потенциально с любым объектом (то есть тип интеракции определяется вторым объектом)
//         // видимо при создании интеракции нужно указать тип получателя
//         if (!first.is<core::character*>()) throw std::runtime_error("Expected character in get_interactions function");
//         
//         sol::state_view lua = s;
//         auto table = lua.create_table(100, 0);
//         auto ctx = global::get<systems::map_t>()->core_context;
//         script::context script_ctx;
//         // положим первый объект в рут и в "context:actor", а второй в "context:recipient"
//         script_ctx.root = script::object(first.as<core::character*>());
//         script_ctx.map.emplace(std::make_pair(std::string_view("actor"), script_ctx.root));
//         for (size_t i = 0; i < ctx->get_entity_count<core::interaction>(); ++i) {
//           auto inter = ctx->get_entity<core::interaction>(i);
//           const bool ret = inter->potential.compute(&script_ctx);
//           // нужно ли "компилировать" интеракцию? игроку то наверное вообще все равно
//           if (ret) table.add(inter);
//         }
//         
//         return table;
//       });
//       
//       utils.set_function("get_decisions", [] (sol::this_state s, const sol::object first) {
//         // все десижоны вызываются от персонажа
//         // нужно просто пройтись по тем что мы можем потенциально вызвать
//         if (!first.is<core::character*>()) throw std::runtime_error("Expected character in get_decisions function");
//         
//         sol::state_view lua = s;
//         auto table = lua.create_table(100, 0);
//         auto ctx = global::get<systems::map_t>()->core_context;
//         script::context script_ctx;
//         script_ctx.root = script::object(first.as<core::character*>());
//         for (size_t i = 0; i < ctx->get_entity_count<core::decision>(); ++i) {
//           auto d = ctx->get_entity<core::decision>(i);
//           const bool ret = d->potential.compute(&script_ctx);
//           // нужно ли "компилировать" решение?
//           if (ret) table.add(d);
//         }
//         
//         return table;
//       });
      
      // наверное должно быть в кор, задано в lua_*_decision.cpp
      //utils.set_function("get_potential_interactions", &core::get_potential_interactions);
      //utils.set_function("get_potential_decisions", &core::get_potential_decisions);
      
      auto core = lua["core"].get_or_create<sol::table>();
      core.set_function("reload_interface", [] () {
        auto game_ctx = global::get<systems::core_t>()->game_ctx;
        game_ctx->reload_interface = true;
      });
    }
  }
}
