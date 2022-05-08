#ifndef FRUSTUM_H
#define FRUSTUM_H

#include "utility.h"

namespace devils_engine {
  namespace utils {
    struct frustum {
      glm::vec4 planes[6];
    };
    
    frustum compute_frustum(const glm::mat4 &matrix);
  }
}

#endif
