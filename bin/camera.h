#ifndef CAMERA_H
#define CAMERA_H

//#include "utils/ecs.h"
#include "utils/utility.h"

// камера должна быть хотя бы побыстрее + она должна быть плавной (!)
// еще немаловажно это менять угол камеры так чтобы мы смотрели не только сверху 
// плавность достигается за счет формулы camera_pos += (end_pos - camera_pos) * CONST
// как настроить поворот камеры? как же она работает в других играх
// позиция, поворот, конечная позиция, и конечный поворот, 
// для анимирования камеры это все может пригодиться
// причем видимо у меня должен быть еще способ зафорсить изменение положение камеры
// у камеры еще должен быть метод апдейт
// в реддите сказали что стандарт камеры это 45 градусов
// мне было бы неплохо сделать до какого то удаления камеру 45 градусов 
// а потом плавный переход к камере сверху
// осталось только по коэффициентам пройтись
// а для этого нужно сделать настройки

namespace yacs {
  class entity;
}

namespace devils_engine {
  namespace components {
    class camera;
  }
  
  namespace camera {
    //void strategic(yacs::entity* ent);
    void strategic(components::camera* camera);
  }
  
  namespace components {
    class camera {
    public:
      camera(const float &m_min_zoom, const float &m_max_zoom) : m_zoom(m_min_zoom), m_min_zoom(m_min_zoom), m_max_zoom(m_max_zoom), m_front(glm::normalize(glm::vec3(-1.0f, 0.0f, 0.0f))) {}
      virtual ~camera() = default;
      virtual void update(const size_t &time) = 0;
      virtual void move(const float &horisontal_angle, const float &vertical_angle) = 0;
      virtual void zoom_add(const float &val) = 0;
      virtual void set_end_pos(const glm::vec3 &end_pos) = 0;
      virtual void set_end_point(const glm::vec3 &end_pos) = 0;
      virtual glm::vec3 current_pos() const = 0;
      virtual float zoom() const = 0;
      
      inline float min_zoom() const { return m_min_zoom; }
      inline float max_zoom() const { return m_max_zoom; }
      
      inline glm::vec3 dir() const { return m_dir; }
      
      inline glm::vec3 front() const { return m_front; }
      inline glm::vec3 right() const { return m_right; }
      inline glm::vec3 up() const { return m_up; }
    protected:
      float m_zoom;
      float m_min_zoom;
      float m_max_zoom;
      
      glm::vec3 m_dir;
      glm::vec3 m_front;
      glm::vec3 m_right;
      glm::vec3 m_up;
    };
    
    class world_map_camera : public camera {
    public:
      // некоторые из этих констант неплохо было бы вынести в defines
      constexpr static const float min_zoom = 0.0f;
      constexpr static const float max_zoom = 100.0f;
      constexpr static const float zoom_k = 5.0f;
      constexpr static const float minimum_camera_height = 20.0f;
      
      world_map_camera(const glm::vec3 &pos);
      
      void update(const size_t &time) override;
      
      void move(const float &horisontal_angle, const float &vertical_angle) override;
      void zoom_add(const float &val) override;
      
      // метод перемещения к какой то точке, причем несколько разными способами
      // мне обязательно нужно сделать так чтобы камера именно смотрела на определенную точку
      // есть по крайней мере 2 метода: прилететь конкретно к точке, максимально близко
      // и подлететь к точке так чтобы направление камеры указывало ровно на точку
      // + еще есть необходимость летать на определенной высоте
      
      void set_end_pos(const glm::vec3 &end_pos) override;   // мы летим максимально близко к точке, причем летим мы по сфере
      void set_end_point(const glm::vec3 &end_pos) override; // мы летим к точке такой чтобы камера была напралена к точке
      
      glm::vec3 current_pos() const override;
      float zoom() const override;
      
//       float zoom() const;
//       glm::vec3 dir() const;
//       
//       glm::vec3 front() const;
//       glm::vec3 right() const;
//       glm::vec3 up() const;
    private:
      glm::vec3 m_spherical_pos;
      glm::vec3 m_spherical_end_pos;
//       glm::vec3 m_dir;
      glm::vec3 m_end_dir;
      
//       glm::vec3 m_front;
//       glm::vec3 m_right;
//       glm::vec3 m_up;
      float m_horisontal_angle;
      float m_vertical_angle;
//       float m_zoom;
      float m_accumulation_zoom;
      
      // нужно еще сделать направление камеры, камера должна менять направление от высоты над картой
      // тут мы должны по идее просто считать от положения
      glm::vec3 compute_dir(const glm::vec3 &normal, const float zoom, float &current_angle);
      void compute_orientation(const glm::vec3 &pos);
    };
    
    class battle_camera : public camera {
    public:
      constexpr static const float min_zoom = 0.0f;
      constexpr static const float max_zoom = 50.0f;
      constexpr static const float minimum_camera_height = 3.0f;
      
      battle_camera(const glm::vec3 &pos);
      
      void update(const size_t &time) override;
      void move(const float &horisontal_angle, const float &vertical_angle) override;
      void zoom_add(const float &val) override;
      void set_end_pos(const glm::vec3 &end_pos) override;
      void set_end_point(const glm::vec3 &end_pos) override;
      glm::vec3 current_pos() const override;
      float zoom() const override;
    private:
      glm::vec3 m_current_pos;
      glm::vec3 m_end_pos;
      glm::vec3 m_end_dir;
      float m_horisontal_angle;
      float m_vertical_angle;
      float m_accumulation_zoom;
      
      glm::vec3 compute_dir(const glm::vec3 &normal, const float zoom, float &current_angle);
    };
    
//     struct transform {
//       glm::vec3 pos;
//       glm::vec3 scale;
//       
//       transform();
//       transform(const glm::vec3 &pos, const glm::vec3 &scale);
//     };
  }
}

#endif
