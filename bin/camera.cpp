#include "camera.h"

#include "utils/ecs.h"
#include "utils/globals.h"
#include "render/targets.h"
#include "render/window.h"

namespace devils_engine {
  namespace camera {
    void strategic(yacs::entity* ent) {
      auto camera = ent->get<components::camera>();
      auto trans = ent->get<components::transform>();
      auto window = global::get<render::window>();
      auto buffers = global::get<render::buffers>();
      
      const glm::mat4 persp = glm::perspective(glm::radians(75.0f), float(window->surface.extent.width) / float(window->surface.extent.height), 0.1f, 256.0f);
      const glm::mat4 view  = glm::lookAt(trans->pos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
      
      //buffers->update_matrix(persp * view);
      buffers->update_projection_matrix(persp);
      buffers->update_view_matrix(view);
      buffers->update_pos(trans->pos);
      buffers->update_dir(camera->front());
    }
  }
  
  glm::vec3 project_vec_on_plane(const glm::vec3 &normal, const glm::vec3 &origin, const glm::vec3 &vector) {
    float dist = glm::dot(vector, normal);
    glm::vec3 point2 = origin + vector - normal*dist;
    return point2 - origin;
  }
  
  namespace components {
    camera::camera(yacs::entity* ent) : m_ent(ent), m_zoom(min_zoom), m_front(glm::normalize(glm::vec3(-1.0f, 0.0f, 0.0f))) {}
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
      const float zoom_norm = glm::mix(1.0f, 5.0f, (m_zoom - min_zoom) / (max_zoom - min_zoom));
      //const float zoom_add = zoom_norm * 5.0f;
      
      auto trans = m_ent->get<components::transform>();
      const glm::vec3 current_pos = trans->pos + m_right * (horisontal_angle * zoom_norm) + m_up * (vertical_angle * zoom_norm);
      
//       glm::vec3 right;
//       glm::vec3 up;
      const glm::vec3 norm = glm::normalize(current_pos);
      const glm::vec3 new_up = glm::normalize(project_vec_on_plane(norm, current_pos, glm::vec3(0.0f, 1.0f, 0.0f)));
      
//       if (glm::abs(norm[0]) < EPSILON && glm::abs(norm[1]) < EPSILON) {
//         right = glm::vec3(1.0f, 0.0f, 0.0f);
//         up = glm::vec3(0.0f, 1.0f, 0.0f);
//       } else {
//         right = glm::normalize(glm::vec3(-norm[1], norm[0], 0.0f));
//         up = glm::normalize(glm::vec3(-norm[0]*norm[2], -norm[1]*norm[2], norm[0]*norm[0] + norm[1]*norm[1]));
//       }
      
      m_front = -norm;
      m_up = new_up;
      m_right = glm::normalize(glm::cross(m_front, m_up));
      
      trans->pos = -m_front * (520.0f + m_zoom);
      
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
    
    void camera::zoom_add(const float &val) { 
      m_zoom += val; 
      m_zoom = glm::max(m_zoom, min_zoom); 
      m_zoom = glm::min(m_zoom, max_zoom); 
      auto trans = m_ent->get<components::transform>();
      trans->pos = -m_front * (520.0f + m_zoom);
    }
    
    float camera::zoom() const { return m_zoom; }
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
    
    transform::transform() {}
    transform::transform(const glm::vec3 &pos, const glm::vec3 &scale) : pos(pos), scale(scale) {}
  }
}
