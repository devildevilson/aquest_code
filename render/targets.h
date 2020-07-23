#ifndef TARGETS_H
#define TARGETS_H

#include "target.h"
#include "yavf.h"
#include "utils/utility.h"
#include "bin/figures.h"
#include <atomic>

namespace devils_engine {
  namespace map {
    struct container;
  }

  namespace render {
    struct buffers : public target {
      yavf::Buffer* uniform;
      yavf::Buffer* matrices;
      yavf::Buffer* border_buffer;
      yavf::Buffer* border_types;

      buffers(yavf::Device* device);
      ~buffers();

      void update_projection_matrix(const glm::mat4 &matrix);
      void update_view_matrix(const glm::mat4 &matrix);
      void update_pos(const glm::vec3 &pos);
      void update_dir(const glm::vec3 &dir);
//       void set_tile_data(const map::tile &tile, const uint32_t &index);
//       void set_point_data(const glm::vec3 &point, const uint32_t &index);
//       void set_tile_indices(const uint32_t &triangle_index, const std::vector<uint32_t> &indices, const uint32_t &offset, const uint32_t &count, const bool has_pentagon);
      void recreate(const uint32_t &width, const uint32_t &height) override;
      glm::mat4 get_matrix() const;
      glm::mat4 get_proj() const;
      glm::mat4 get_view() const;
      glm::mat4 get_inv_proj() const;
      glm::mat4 get_inv_view() const;
      glm::mat4 get_inv_view_proj() const;
      glm::vec4 get_pos() const;
    };
  }
}

#endif
