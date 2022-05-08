#ifndef RENDER_UTILS_H
#define RENDER_UTILS_H

#include "utils/utility.h"

namespace devils_engine {
  namespace core {
    struct army;
  }
  
  namespace render {
    struct window;
    struct buffers;
    
    glm::vec4 find_triangle_normal(const glm::vec4 &p1, const glm::vec4 &p2, const glm::vec4 &p3);
    glm::vec4 get_cursor_dir(render::buffers* buffers, render::window* window, const double xpos, const double ypos);
    void set_selection_frustum(const glm::dvec2 &coord_min, const glm::dvec2 &coord_max);
    void update_selection();
  }
}

#endif
