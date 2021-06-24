#include "utils.h"

#include "utils/globals.h"
#include "utils/systems.h"
#include "stages.h"
#include "battle_render_stages.h"
#include "targets.h"
#include "window.h"
#include "bin/camera.h"

#include "utils/sol.h"
#include "bin/objects_selection.h"
#include "bin/map.h"
#include "core/context.h"
#include "bin/tiles_funcs.h"
#include "utils/interface_container2.h"
#include "utils/game_context.h"

namespace devils_engine {
  namespace render {
    glm::vec4 find_triangle_normal(const glm::vec4 &p1, const glm::vec4 &p2, const glm::vec4 &p3) {
      const glm::vec4 u_vec = p2 - p1;
      const glm::vec4 v_vec = p3 - p1;
      
      const glm::vec3 norm = glm::normalize(glm::cross(glm::vec3(u_vec), glm::vec3(v_vec)));
      return glm::vec4(norm, 0.0f);
    }
    
    glm::vec4 get_cursor_dir(render::buffers* buffers, render::window* window, const double xpos, const double ypos) {
      const glm::mat4 inv_proj = buffers->get_inv_proj();
      const glm::mat4 inv_view = buffers->get_inv_view();

      float x = (2.0f * xpos) /  float(window->surface.extent.width) - 1.0f;
      float y = 1.0f - (2.0f * ypos) / float(window->surface.extent.height); // тут по идее должно быть обратное значение
      float z = 1.0f;

//       ASSERT(x >= -1.0f && x <= 1.0f);
//       ASSERT(y >= -1.0f && y <= 1.0f);

      glm::vec3 ray_nds = glm::vec3(x, -y, z);
      glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);
      glm::vec4 ray_eye = inv_proj * ray_clip;
      ray_eye = glm::vec4(glm::vec2(ray_eye), -1.0, 0.0);
      glm::vec3 ray_wor = glm::vec3(inv_view * ray_eye);
      // don't forget to normalise the vector at some point
      const glm::vec4 dir = glm::vec4(glm::normalize(ray_wor), 0.0f);
      //const glm::vec4 final_dir = camera_dir;
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
      const auto map = global::get<systems::map_t>()->map;
      const auto ctx = global::get<systems::map_t>()->core_context;
      auto selection_secondary = global::get<systems::core_t>()->selection.secondary;
      auto interface = global::get<systems::core_t>()->interface_container.get();
      auto game_ctx = global::get<systems::core_t>()->game_ctx;
      ASSERT(opt != nullptr);
      ASSERT(map != nullptr);
      ASSERT(ctx != nullptr);
      
      selection_secondary->clear();
      
      const auto indirect_buffer = opt->indirect_buffer();
      auto indirect_data = reinterpret_cast<struct render::tile_optimizer::indirect_buffer*>(indirect_buffer->ptr());
      
      ASSERT(indirect_data->heraldies_command.vertexCount == 4);
      const uint32_t selection_count = indirect_data->heraldies_command.instanceCount;
      const auto selection_buffer = opt->structures_index_buffer();
      const auto selection_data = reinterpret_cast<uint32_t*>(selection_buffer->ptr());
      
      float min_dist = 10000.0f;
      uint32_t min_dist_heraldy_index = UINT32_MAX;
      
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
        
        
        const uint32_t structure_index = map->get_tile_objects_index(tile_index, 0);
        const uint32_t army_index = map->get_tile_objects_index(tile_index, 4);
        const uint32_t hero_index = map->get_tile_objects_index(tile_index, 5);
        // по идее у меня будет еще некий индекс в 3 слоте, как понять что это?
        //const uint32_t unit_index = map->get_tile_objects_index(tile_index, 6);
        
//         PRINT_VAR("army_index", army_index)
//         PRINT_VAR("hero_index", hero_index)
        
        if (structure_index != GPU_UINT_MAX) {
          //auto city = ctx->get_entity<core::city>(structure_index);
          auto city = core::get_tile_city(tile_index);
          ASSERT(city != nullptr);
          selection_secondary->add(sol::make_object(interface->lua, city));
        }
        
        if (army_index != GPU_UINT_MAX) {
          auto army = ctx->get_army(army_index);
          ASSERT(army != nullptr);
          selection_secondary->add(sol::make_object(interface->lua, army));
        }
        
        if (hero_index != GPU_UINT_MAX) {
          auto hero_troop = ctx->get_hero_troop(hero_index);
          ASSERT(hero_troop != nullptr);
          selection_secondary->add(sol::make_object(interface->lua, hero_troop));
        }
      }
      
//       if (min_dist_heraldy_index != UINT32_MAX) {
//         PRINT_VAR("heraldy dist ", min_dist)
//         PRINT_VAR("heraldy index", min_dist_heraldy_index)
//       }
      
      game_ctx->traced_heraldy_dist = min_dist;
      game_ctx->traced_heraldy_tile_index = min_dist_heraldy_index;
    }
    
    void update_selection() {
      if (global::get<systems::map_t>()->is_init()) world_map_update_selection();
      else if (global::get<systems::battle_t>()->is_init()) { ASSERT(false); }
      else if (global::get<systems::encounter_t>()->is_init()) { ASSERT(false); }
    }
  }
}
