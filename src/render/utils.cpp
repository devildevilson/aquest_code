#include "utils.h"

#include "utils/globals.h"
#include "utils/systems.h"
#include "utils/sol.h"
#include "utils/interface_container2.h"
#include "utils/game_context.h"

#include "stages.h"
#include "targets.h"
#include "window.h"
#include "container.h"

#include "bin/camera.h"
#include "bin/objects_selection.h"
#include "bin/tiles_funcs.h"

#include "core/map.h"
#include "core/context.h"
#include "core/render_stages.h"
#include "battle/render_stages.h"

namespace devils_engine {
  namespace render {
    glm::vec4 find_triangle_normal(const glm::vec4 &p1, const glm::vec4 &p2, const glm::vec4 &p3) {
      const glm::vec4 u_vec = p2 - p1;
      const glm::vec4 v_vec = p3 - p1;
      
      const glm::vec3 norm = glm::normalize(glm::cross(glm::vec3(u_vec), glm::vec3(v_vec)));
      return glm::vec4(norm, 0.0f);
    }
    
    // по идее должно работать но не работает
    glm::vec4 get_cursor_dir2(render::buffers* buffers, render::window* window, const double xpos, const double ypos) {
      const glm::mat4 inv_proj = buffers->get_inv_proj();
      const glm::mat4 inv_view = buffers->get_inv_view();
//       const glm::mat4 inv_view_proj = buffers->get_inv_view_proj();
      const auto [w, h] = window->framebuffer_size();
      const glm::vec2 window_coords = glm::vec2(xpos, ypos) / glm::vec2(w, h);
      const vec4 clip_space_position = vec4(window_coords*2.0f-glm::vec2(1.0), 1.0f, 1.0f);
      vec4 view_space_position = inv_proj * clip_space_position;
      view_space_position /= view_space_position.w;
      const glm::vec4 world_pos = inv_view * view_space_position;
      const glm::vec4 camera_pos = buffers->get_pos();
      //return glm::normalize(view_space_position - camera_pos);
      return glm::normalize(glm::vec4(glm::vec3(world_pos), 1.0f) - camera_pos);
    }
    
    glm::vec4 get_cursor_dir(render::buffers* buffers, render::window* window, const double xpos, const double ypos) {
      const glm::mat4 inv_proj = buffers->get_inv_proj();
      const glm::mat4 inv_view = buffers->get_inv_view();
      
      const auto [w, h] = window->framebuffer_size();
      const float x = (2.0f * xpos) / float(w) - 1.0f;
      const float y = 1.0f - (2.0f * ypos) / float(h); // тут по идее должно быть обратное значение
      const float z = 1.0f; // самая дальняя точка в вулкане это 1.0
      
      const glm::vec3 ray_nds = glm::vec3(x, -y, z);
      const glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, 1.0); // -1 тоже работает ???
      glm::vec4 ray_eye = inv_proj * ray_clip;
      ray_eye = glm::vec4(glm::vec2(ray_eye), -1.0, 0.0);
      const glm::vec3 ray_wor = glm::vec3(inv_view * ray_eye);
      // don't forget to normalise the vector at some point
      const glm::vec4 dir = glm::vec4(glm::normalize(ray_wor), 0.0f);
      return dir;
    }
    
    void set_selection_frustum(const glm::dvec2 &coord_min, const glm::dvec2 &coord_max) {
      auto camera = camera::get_camera();
      auto buffers = global::get<render::buffers>();
      auto opt = global::get<render::tile_optimizer>();
      auto window = global::get<render::window>();
      auto battle_opt = global::get<render::battle::tile_optimizer>();
      
      if (camera == nullptr) return;
      if (battle_opt == nullptr && opt == nullptr) return;
      
      const glm::vec4 dirs[] = {
        get_cursor_dir(buffers, window, coord_min.x, coord_min.y),
        get_cursor_dir(buffers, window, coord_max.x, coord_min.y),
        get_cursor_dir(buffers, window, coord_min.x, coord_max.y),
        get_cursor_dir(buffers, window, coord_max.x, coord_max.y)
      };
      
      const float minimum_dist = MINIMUM_FRUSTUM_DIST;
      const float maximum_dist = MAXIMUM_FRUSTUM_DIST;
      
      const float raw_zoom = camera->zoom();
      const float zoom_norm = (raw_zoom - camera->min_zoom()) / (camera->max_zoom() - camera->min_zoom());
      
      const float final_dist = glm::mix(minimum_dist, maximum_dist, zoom_norm);
      const float near = 1.0f;
      
      const glm::vec4 camera_pos = buffers->get_pos();
      const glm::vec4 camera_dir = buffers->get_dir();
      
      const glm::vec4 points[] = {
        camera_pos + dirs[0] * near,       // 0
        camera_pos + dirs[1] * near,       // 1
        camera_pos + dirs[2] * near,       // 2
        camera_pos + dirs[3] * near,       // 3
        camera_pos + dirs[0] * final_dist, // 4
        camera_pos + dirs[1] * final_dist, // 5
        camera_pos + dirs[2] * final_dist, // 6
        camera_pos + dirs[3] * final_dist  // 7
      };
      
      // находим 6 плоскостей, че за бред со знаками?
      glm::vec4 normals[] = {
        //-find_triangle_normal(points[0], points[1], points[2]), // ближайшая
        camera_dir,
        -find_triangle_normal(points[0], points[1], points[4]), // верх
        -find_triangle_normal(points[0], points[4], points[6]), // лево
        -find_triangle_normal(points[7], points[5], points[3]), // право
        -find_triangle_normal(points[7], points[3], points[6]), // низ
        //-find_triangle_normal(points[7], points[5], points[6]), // дальняя
        -camera_dir
      };
      
      ASSERT(glm::dot(camera_dir, normals[0]) >  EPSILON);
      ASSERT(glm::dot(camera_dir, normals[5]) < -EPSILON);
      ASSERT(glm::dot(normals[0], normals[5]) < -EPSILON);
  //     ASSERT(glm::dot(normals[1], normals[4]) < -EPSILON); // тут может получиться очень большой угол
  //     ASSERT(glm::dot(normals[2], normals[3]) < -EPSILON);
      //ASSERT(glm::dot(normals[4], glm::vec4(0.0f, 1.0f, 0.0f, 0.0f)) < -EPSILON);
      
      //normals[0].w = -glm::dot(normals[0], points[0]);
      normals[0].w = -glm::dot(normals[0], camera_pos + camera_dir * near);
      normals[1].w = -glm::dot(normals[1], points[0]);
      normals[2].w = -glm::dot(normals[2], points[0]);
      normals[3].w = -glm::dot(normals[3], points[7]);
      normals[4].w = -glm::dot(normals[4], points[7]);
      //normals[5].w = -glm::dot(normals[5], points[7]);
      normals[5].w = -glm::dot(normals[5], camera_pos + camera_dir * final_dist);
      
      // работает при малых значениях, не работает при больших
      // считаю неправильно фрустум тест? может быть неправильно направлены нормали
//       const auto diff = coord_max - coord_min;
//       if (std::min(diff.x, diff.y) > 100) assert(false);
      
      const utils::frustum fru{normals[0], normals[1], normals[2], normals[3], normals[4], normals[5]};
      
      if (battle_opt != nullptr) battle_opt->update_selection_frustum(fru);
      if (opt != nullptr) opt->set_selection_frustum(fru);
    }
    
    void world_map_update_selection() {
      const auto opt = global::get<render::tile_optimizer>();
//       const auto map = global::get<systems::map_t>()->map;
      const auto ctx = global::get<systems::map_t>()->core_context.get();
      auto selection_secondary = global::get<systems::core_t>()->selection.secondary;
      auto interface = global::get<systems::core_t>()->interface_container.get();
//       auto game_ctx = global::get<systems::core_t>()->game_ctx;
      ASSERT(opt != nullptr);
//       ASSERT(map != nullptr);
      ASSERT(ctx != nullptr);
      
      selection_secondary->clear();
      const uint32_t selection_count = opt->get_selection_count();
      auto selection_data = opt->get_selection_data();
      
      float min_dist = 10000.0f;
      uint32_t min_dist_heraldy_index = UINT32_MAX;
      
      // эта штука не работает сейчас, было бы неплохо разделить tiles.comp шейдер
      for (uint32_t i = 0 ; i < selection_count; ++i) {
        if (selection_data[i] > INT32_MAX) {
          const float tmp = -glm::uintBitsToFloat(selection_data[i]);
          if (tmp < min_dist) {
            min_dist = tmp;
            min_dist_heraldy_index = selection_data[i+1];
          }
          
          ++i;
          continue;
        }
        
        const uint32_t tile_index = selection_data[i];
//         PRINT_VAR("selection " + std::to_string(i), tile_index)
//         const uint32_t army_gpu_index = selection_data[i];
        // тут по идее нужно использовать unordered_map, в который мы положим индексы армии и указатели на армию
        //global::get<utils::objects_selector>()->add(utils::objects_selector::unit::type::army, selection_data[i]);
        // хотя лучше просто армии по слотам расположить
        // вот у нас селектион, перемещение камеры по карте нужно сделать тогда приближением к краям + нужно сделать рендер курсора?
        // хотя наверное можно обойтись просто ограничением мышки
        
        const auto tile = ctx->get_entity<core::tile>(tile_index);
        const uint32_t city_index = tile->city;
        const size_t army_token = tile->army_token;
        const size_t hero_token = tile->hero_token;
        // по идее у меня будет еще некий индекс в 3 слоте, как понять что это?
        //const uint32_t unit_index = map->get_tile_objects_index(tile_index, 6);
        
//         PRINT_VAR("army_index", army_index)
//         PRINT_VAR("hero_index", hero_index)
        
        if (city_index != GPU_UINT_MAX) {
          auto city = ctx->get_entity<core::city>(city_index);
          //auto city = core::get_tile_city(tile_index);
          ASSERT(city != nullptr);
          selection_secondary->add(sol::make_object(interface->lua, city));
        }
        
        if (army_token != SIZE_MAX) {
          auto army = ctx->get_army(army_token);
          ASSERT(army != nullptr);
          selection_secondary->add(sol::make_object(interface->lua, army));
        }
        
        if (hero_token != SIZE_MAX) {
          auto hero_troop = ctx->get_hero_troop(hero_token);
          ASSERT(hero_troop != nullptr);
          selection_secondary->add(sol::make_object(interface->lua, hero_troop));
        }
      }
      
//       if (min_dist_heraldy_index != UINT32_MAX) {
//         PRINT_VAR("heraldy dist ", min_dist)
//         PRINT_VAR("heraldy index", min_dist_heraldy_index)
//       }
      
//       game_ctx->traced_heraldy_dist = min_dist;
//       game_ctx->traced_heraldy_tile_index = min_dist_heraldy_index;
    }
    
    // че же с тобой делать? надо вот что сделать: проверять указатели на nullptr, а задавать их в конструкторах/деструкторах 
    void update_selection() {
      if (global::get<systems::battle_t>() != nullptr) { ASSERT(false); }
      else if (global::get<systems::encounter_t>() != nullptr) { ASSERT(false); }
      else if (global::get<systems::map_t>() != nullptr) world_map_update_selection();
    }
  }
}
