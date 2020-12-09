#include "frustum.h"

namespace devils_engine {
  namespace utils {
    frustum compute_frustum(const glm::mat4 &matrix) {
      frustum fru;
      const glm::mat4 mat = glm::transpose(matrix);

      fru.planes[0] = mat[3] + mat[0];
      fru.planes[1] = mat[3] - mat[0];
      fru.planes[2] = mat[3] + mat[1];
      fru.planes[3] = mat[3] - mat[1];
      fru.planes[4] = mat[3] + mat[2];
      fru.planes[5] = mat[3] - mat[2];

      for (uint32_t i = 0; i < 6; ++i) {
        const float mag = glm::length(glm::vec4(fru.planes[i][0], fru.planes[i][1], fru.planes[i][2], 0.0f));
        fru.planes[i] /= mag;
      }
      
      return fru;
    }
  }
}
