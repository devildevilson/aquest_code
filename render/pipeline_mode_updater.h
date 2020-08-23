#ifndef PIPELINE_MODE_UPDATER_H
#define PIPELINE_MODE_UPDATER_H

#include "utils/utility.h"

namespace devils_engine {
  namespace render {
    class updater {
    public:
      updater();
      
      uint32_t render_mode();
      uint32_t water_mode();
      uint32_t render_slot();
      uint32_t water_slot();
      glm::vec3 water_color();
      
      void set_render_mode(const uint32_t &render_mode);
      void set_water_mode(const uint32_t &water_mode);
      void set_render_slot(const uint32_t &render_slot);
      void set_water_slot(const uint32_t &water_slot);
      void set_water_color(const glm::vec3 &color);
      
      void update();
    private:
      uint32_t m_render_mode; 
      uint32_t m_water_mode; 
      uint32_t m_render_slot; 
      uint32_t m_water_slot; 
      glm::vec3 m_color;
    };
  }
}

#endif
