#include "camera.h"

#include "utils/ecs.h"
#include "utils/globals.h"
#include "render/targets.h"
#include "render/window.h"
#include "render/shared_structures.h"
#include "map.h"
#include "utils/shared_time_constant.h"

glm::vec3 from_decard_to_spherical(const glm::vec3 &vec) {
//   const bool x_less_zero = vec.x < 0.0f;
//   const bool z_less_zero = vec.z < 0.0f;
  const float r = glm::sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
  return glm::vec3( // r, zenit, azimut
    r,
    glm::acos(vec.y / r),
    atan2(vec.z, vec.x) // нужно использовать такое вычисление арктангенса
  ); //  + float(x_less_zero) * PI
}

glm::vec3 from_spherical_to_decard(const glm::vec3 &spherical) {
  return glm::vec3(
    spherical.x * glm::sin(spherical.y) * glm::cos(spherical.z),
    spherical.x * glm::cos(spherical.y),
    spherical.x * glm::sin(spherical.y) * glm::sin(spherical.z)
  );
}

namespace devils_engine {
  namespace camera {
    void strategic(components::camera* camera) {
//       auto camera = ent->get<components::camera>();
//       auto trans = ent->get<components::transform>();
      auto window = global::get<render::window>();
      auto buffers = global::get<render::buffers>();
      
      const glm::mat4 persp = glm::perspective(glm::radians(75.0f), float(window->surface.extent.width) / float(window->surface.extent.height), 0.1f, 256.0f);
      const glm::vec3 pos   = camera->current_pos();
      const glm::vec3 dir   = camera->dir();
      const glm::vec3 up    = camera->up();
      //const glm::vec3 up    = glm::vec3(0.0f, 1.0f, 0.0f);
      const glm::mat4 view  = glm::lookAt(pos, pos+dir, up);
      
      //buffers->update_matrix(persp * view);
      buffers->update_projection_matrix(persp);
      buffers->update_view_matrix(view);
      buffers->update_pos(pos);
      buffers->update_dir(camera->front());
    }
  }
  
  glm::vec3 project_vec_on_plane(const glm::vec3 &normal, const glm::vec3 &origin, const glm::vec3 &vector) {
    float dist = glm::dot(vector, normal);
    glm::vec3 point2 = origin + vector - normal*dist;
    return point2 - origin;
  }
  
  namespace components {
    camera::camera(const glm::vec3 &pos) : m_front(glm::normalize(glm::vec3(-1.0f, 0.0f, 0.0f))), m_zoom(max_zoom) {
      const float lenght = glm::length(pos);
      const glm::vec3 normalize = pos / lenght;
      const float final_lenght = glm::max(lenght, core::map::world_radius + minimum_camera_height);
      const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
      m_front = -normalize;
      m_right = glm::normalize(glm::cross(up, m_front));
      m_up = glm::normalize(glm::cross(m_front, m_right));
      ASSERT(glm::dot(up, m_up) >= 0.0f);
      compute_dir(m_front, final_lenght);
      m_spherical_pos = from_decard_to_spherical(normalize*final_lenght);
      m_spherical_end_pos = m_spherical_pos;
    }
    
    void camera::update(const size_t &time) {
      const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
      const float k = double(time) / double(ONE_SECOND);
      
      const float CAMERA_CONST = 2.0f * k; // как то должно видимо работать со временем
//       PRINT_VEC3("m_spherical_pos", m_spherical_pos)
      const auto tmp = (m_spherical_end_pos - m_spherical_pos);
      const float d = glm::dot(tmp, tmp);
      if (glm::abs(d) < EPSILON) return;
      m_spherical_pos = m_spherical_pos + tmp * CAMERA_CONST; // d < 0.1f ? m_spherical_end_pos : 
      
      const glm::vec3 current_pos = from_spherical_to_decard(m_spherical_pos);
      m_front = -glm::normalize(current_pos);
      m_right = glm::normalize(glm::cross(up, m_front));
      m_up = glm::normalize(glm::cross(m_front, m_right));
      
      const float current_zoom = glm::length(current_pos) - (core::map::world_radius + minimum_camera_height);
      ASSERT(current_zoom >= (min_zoom-1.0f) && current_zoom <= (max_zoom+1.0f));
      m_dir = compute_dir(m_front, current_zoom);
      const float norm_zoom = (current_zoom - min_zoom) / (max_zoom - min_zoom);
      auto u = global::get<render::buffers>()->uniform;
      auto camera_data = reinterpret_cast<render::camera_data*>(u->ptr());
      ASSERT(camera_data != nullptr);
      camera_data->dim.z = glm::floatBitsToUint(norm_zoom);
      
      m_accumulation_zoom += (0.0f - m_accumulation_zoom) * 0.2f;
    }
    
    // тут нужно пересмотреть то как вычисляется текущий угол
    void camera::move(const float &horisontal_angle, const float &vertical_angle) {
//       m_horisontal_angle += horisontal_angle;
//       m_vertical_angle += vertical_angle;
      
//       if (m_horisontal_angle > 360.0f) m_horisontal_angle = m_horisontal_angle - 360.0f;
//       if (m_horisontal_angle < 0.0f)   m_horisontal_angle = m_horisontal_angle + 360.0f;
// 
//       if (m_vertical_angle > 180.0f - 0.01f) m_vertical_angle = 180.0f - 0.01f;
//       if (m_vertical_angle < 0.0f   + 0.01f) m_vertical_angle = 0.0f   + 0.01f;
// 
//       const float x = -glm::radians(m_horisontal_angle); // азимут
//       const float y = -glm::radians(m_vertical_angle); // зенит
      
      ASSERT(max_zoom > min_zoom);
      const float current_zoom = m_spherical_pos.x - (core::map::world_radius + minimum_camera_height);
      const float zoom_norm = glm::mix(1.0f, 5.0f, (current_zoom - min_zoom) / (max_zoom - min_zoom));
      //const float zoom_add = zoom_norm * 5.0f;
      
//       m_spherical_pos.y += glm::radians(horisontal_angle);
//       m_spherical_pos.z += glm::radians(vertical_angle);
//       if (m_spherical_pos.y < 0.0f + 0.01f) m_spherical_pos.y += PI;
//       if (m_spherical_pos.y > PI   - 0.01f) m_spherical_pos.y -= PI;
//       if (m_spherical_pos.z < 0.0f)         m_spherical_pos.z += PI_2;
//       if (m_spherical_pos.z >= PI_2)        m_spherical_pos.z -= PI_2;
//       m_spherical_end_pos = m_spherical_pos;
      
      // у меня почему то не работает поворот
      
//       auto trans = m_ent->get<components::transform>();
      const glm::vec3 converted = from_spherical_to_decard(m_spherical_pos);
      const glm::vec3 current_pos = converted + (-m_right) * (horisontal_angle * zoom_norm) + m_up * (vertical_angle * zoom_norm);
//       PRINT_VEC3("converted", converted);
      
//       glm::vec3 right;
//       glm::vec3 up;
      const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
      const glm::vec3 norm = glm::normalize(current_pos);
      //const glm::vec3 new_up = glm::normalize(project_vec_on_plane(xnorm, current_pos, glm::vec3(0.0f, 1.0f, 0.0f)));
      
//       if (glm::abs(norm[0]) < EPSILON && glm::abs(norm[1]) < EPSILON) {
//         right = glm::vec3(1.0f, 0.0f, 0.0f);
//         up = glm::vec3(0.0f, 1.0f, 0.0f);
//       } else {
//         right = glm::normalize(glm::vec3(-norm[1], norm[0], 0.0f));
//         up = glm::normalize(glm::vec3(-norm[0]*norm[2], -norm[1]*norm[2], norm[0]*norm[0] + norm[1]*norm[1]));
//       }
      
      m_front = -norm;
      m_right = glm::normalize(glm::cross(up, m_front));
      m_up = glm::normalize(glm::cross(m_front, m_right));
      
      m_dir = compute_dir(m_front, current_zoom);
      const glm::vec3 final_current_pos = -m_front * (core::map::world_radius + minimum_camera_height + current_zoom);
      m_spherical_pos = from_decard_to_spherical(final_current_pos);
      m_spherical_end_pos = m_spherical_pos;
      
//       trans->pos = final_current_pos;
      
//       m_front = glm::vec4(
//         glm::sin(y) * glm::cos(x), // r = 1.0f
//         -glm::cos(y),
//         glm::sin(y) * glm::sin(x),
//         0.0f
//       );
// 
//       m_front = glm::normalize(m_front);
// 
//       m_right = glm::vec4(
//         glm::cos(x - glm::half_pi<float>()),
//         0.0f,
//         glm::sin(x - glm::half_pi<float>()),
//         0.0f
//       );
// 
//       m_right = glm::normalize(m_right);
//       m_up = glm::normalize(glm::cross(m_right, m_front));
      
    }
    
    // зум нужно накапливать? 
    void camera::zoom_add(const float &val) { 
      if (m_accumulation_zoom * val < 0.0f) m_accumulation_zoom = 0.0f;
      m_accumulation_zoom += val;
      
      //m_zoom += val; 
      m_zoom += m_accumulation_zoom * 2.0f;
      m_zoom = glm::max(m_zoom, min_zoom);
      m_zoom = glm::min(m_zoom, max_zoom);
      if (m_zoom == min_zoom || m_zoom == max_zoom) m_accumulation_zoom = 0.0f;
      //const glm::vec3 dir = compute_dir(m_front, m_spherical_pos.x - (core::map::world_radius + minimum_camera_height));
      const glm::vec3 final_pos = -m_front * (core::map::world_radius + minimum_camera_height + m_zoom);
      m_spherical_end_pos = from_decard_to_spherical(final_pos);
//       auto trans = m_ent->get<components::transform>();
//       trans->pos = final_pos;
//       const float norm_zoom = (m_zoom - min_zoom) / (max_zoom - min_zoom);
      
//       auto u = global::get<render::buffers>()->uniform;
//       auto camera_data = reinterpret_cast<render::camera_data*>(u->ptr());
//       ASSERT(camera_data != nullptr);
//       camera_data->dim.z = glm::floatBitsToUint(norm_zoom);
    }
    
    void camera::set_end_pos(const glm::vec3 &end_pos) {
      // наверное нужно использовать скорее сферические координаты, а может и нет,
      // обычным методом camera_pos += (end_pos - camera_pos) * CONST кажется не получится
      // то есть мне нужно именно по сфере пройти, хотя вот кажется я могу использовать сферические координаты
      m_spherical_end_pos = from_decard_to_spherical(end_pos);
      // теперь я могу сделать по формуле
      // 
    }
    
    void camera::set_end_point(const glm::vec3 &end_pos) {
      // здесь мне нужно найти точку в которой камера будет отцентрована по end_pos
      // значит вопервых нужно было бы неплохо убедиться что это точка как то отноится к тайлу
      // но в рамках камере по всей видимости это невозможно
      const float length = glm::length(end_pos);
      const glm::vec3 normalize = end_pos / length;
      const float final_length = glm::max(length, core::map::world_radius);
      //glm::vec3 final_pos = end_pos;
      const glm::vec3 final_pos = normalize*final_length;
      //if (lenght < core::map::world_radius) final_pos = normalize*core::map::world_radius;
      // нужно найти вектор 45 градусов
      //const glm::vec3 dir = glm::rotate(normalize, glm::radians(-45.0f), m_right); // не тот вектор, хотя нет, правильный
      const glm::vec3 dir = compute_dir(m_front, final_length - (core::map::world_radius + minimum_camera_height));
      // единственное в чем может быть проблема, это в том что может быть сгенерирован неверная высота
      //const glm::vec3 final_end_pos = final_pos + (-dir) * (core::map::world_radius + minimum_camera_height - final_lenght); 
      const glm::vec3 final_end_pos = final_pos + (-dir) * minimum_camera_height;
      set_end_pos(final_end_pos);
    }
    
    glm::vec3 camera::current_pos() const {
      return from_spherical_to_decard(m_spherical_pos);
    }
    
    float camera::zoom() const { return m_spherical_pos.x - (core::map::world_radius + minimum_camera_height); }
    
    glm::vec3 camera::dir() const {
      return m_dir;
    }
    
    glm::vec3 camera::front() const {
      return m_front;
    }
    
    glm::vec3 camera::right() const {
      return m_right;
    }
    
    glm::vec3 camera::up() const {
      return m_up;
    }
    
    glm::vec3 camera::compute_dir(const glm::vec3 &normal, const float zoom) {
      // тут нужно подобрать несколько коэффициентов 
      // по которым камера плавно переходит от одного угла к другому
      // 3 состояния: камера не меняется 45 градусов, камера плавно переходит к 0 градусам, камера не меняется в 0 градусах
      const float height1 = 5.0f;
      const float height2 = 85.0f;
      
      float angle = glm::radians(30.0f);
      if (zoom > height1 && zoom <= height2) angle = glm::radians(glm::mix(30.0f, 0.0f, (zoom - height1) / (height2 - height1)));
      else if (zoom > height2) angle = 0.0f;
      return glm::normalize(glm::rotate(normal, angle, m_right));
    }
    
    transform::transform() {}
    transform::transform(const glm::vec3 &pos, const glm::vec3 &scale) : pos(pos), scale(scale) {}
  }
}
