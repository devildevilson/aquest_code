#ifndef CAMERA_H
#define CAMERA_H

//#include "utils/ecs.h"
#include "utils/utility.h"

namespace yacs {
  class entity;
}

namespace devils_engine {
  namespace camera {
    void strategic(yacs::entity* ent);
  }
  
  namespace components {
    class camera {
    public:
      constexpr static const float min_zoom = 0.0f;
      constexpr static const float max_zoom = 100.0f;
      constexpr static const float zoom_k = 5.0f;
      
      camera(yacs::entity* ent);
      
      void move(const float &horisontal_angle, const float &vertical_angle);
      void zoom_add(const float &val);
      
      float zoom() const;
      glm::vec3 dir() const;
      
      glm::vec3 front() const;
      glm::vec3 right() const;
      glm::vec3 up() const;
    private:
      yacs::entity* m_ent;
      float m_zoom;
      glm::vec3 m_dir;
      glm::vec3 m_front;
      glm::vec3 m_right;
      glm::vec3 m_up;
      float m_horisontal_angle;
      float m_vertical_angle;
    };
    
    struct transform {
      glm::vec3 pos;
      glm::vec3 scale;
      
      transform();
      transform(const glm::vec3 &pos, const glm::vec3 &scale);
    };
  }
}

#endif
