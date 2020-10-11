#ifndef FIGURES_H
#define FIGURES_H

#include "utils/utility.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <iostream>

void print_vec(const glm::vec3 &vec);
float angle(const glm::vec3 &a, const glm::vec3 &b);

// constexpr size_t power4(const uint32_t &pow) {
//   return 1 << pow*2;
// }
// 
// constexpr size_t div4(const size_t &num) {
//   return num >> 2;
// }

namespace devils_engine {
  namespace utils {
//     struct ray {
//       glm::vec3 pos;
//       glm::vec3 dir;
//     };

    struct frustum {
      glm::vec4 planes[6];
    };

//     class timer {
//     public:
//       inline timer(const std::string &str) : tp(std::chrono::steady_clock::now()), str(str) {}
//       inline ~timer() {
//         auto end = std::chrono::steady_clock::now() - tp;
//         auto mcs = std::chrono::duration_cast<std::chrono::microseconds>(end).count();
//         std::cout << str << " took " << mcs << " mcs" << '\n';
//       }
//     private:
//       std::chrono::steady_clock::time_point tp;
//       std::string str;
//     };
    
    utils::frustum compute_frustum(const glm::mat4 &matrix);
  }

  namespace map {
    struct triangle {
      glm::uvec3 points;
      uint32_t current_level;
      uint32_t upper_level_index;
      uint32_t next_level[4]; // на последнем уровне это указатели на гексы

      triangle();
      triangle(const uint32_t &p1, const uint32_t &p2, const uint32_t &p3);
      triangle(const uint32_t &p1, const uint32_t &p2, const uint32_t &p3, const uint32_t &current_level, const uint32_t &upper_level_index);
    };

    struct tile {
      struct neighbour {
        uint32_t index;
        uint32_t points[2];

        neighbour();
      };

      uint32_t index;
      //uint32_t points[6];
      //uint32_t neighbours[6];
      neighbour neighbours[6];

      tile(const uint32_t &index);
      inline bool is_pentagon() const { return neighbours[5].index == UINT32_MAX; }
      //void add_neighbour(const uint32_t &index, const uint32_t &point_index);
      void add_neighbour(const uint32_t &index, const uint32_t &point_a, const uint32_t &point_b);
    };
    
//     constexpr size_t tri_count_d(const size_t &detail_level) {
//       return 20 * power4(detail_level);
//     }
//     
//     constexpr size_t icosahedron_points_count_t(const size_t &tri_count) {
//       return (tri_count*3-12*5)/6+12;
//     }
//     
//     constexpr size_t icosahedron_points_count_d(const size_t &detail_level) {
//       return icosahedron_points_count_t(tri_count_d(detail_level));
//     }
//     
//     constexpr size_t hex_count_t(const size_t &tri_count) {
//       return icosahedron_points_count_t(tri_count) + tri_count;
//     }
//     
//     constexpr size_t hex_count_d(const size_t &detail_level) {
//       return hex_count_t(tri_count_d(detail_level));
//     }
//     
//     constexpr size_t points_count_t(const size_t &tri_count) {
//       return icosahedron_points_count_t(tri_count) + tri_count + tri_count*6/2;
//     }
//     
//     constexpr size_t points_count_d(const size_t &detail_level) {
//       return points_count_t(tri_count_d(detail_level));
//     }

    struct container {
      std::vector<triangle> triangles;
      std::vector<tile> tiles;
      std::vector<glm::vec3> points;

      container(const float &radius, const size_t &detail, const glm::mat3 &rotation);
      void fix_triangle_dirs();
      size_t detail_level() const;

      uint32_t add_vertex(const float &radius, const glm::vec3 &v);
      uint32_t add_vertex(const glm::vec3 &v);
      uint32_t add_middle_point(const float &radius, const uint32_t &p1, const uint32_t &p2, std::unordered_map<size_t, uint32_t> &cache);
      std::pair<uint32_t, uint32_t> add_hex_point(const uint32_t &p1, const uint32_t &p2, const glm::vec3 &edge_point1, const glm::vec3 &edge_point2, std::unordered_map<size_t, std::pair<uint32_t, uint32_t>> &cache);
      uint32_t find_neighbor_tile(const uint32_t &tile, const uint32_t &p1, const uint32_t &p2, std::unordered_map<size_t, uint32_t> &cache);

//       uint32_t cast_ray(const utils::ray &ray) const;
//       // void frustum_test(const utils::frustum &frustum, std::unordered_set<uint32_t> &tiles_container) const;
// 
//       bool intersect_tri(const uint32_t &tri_index, const utils::ray &ray) const;
//       bool intersect_tile(const uint32_t &tile_index, const utils::ray &ray) const;
//       bool intersect_tri(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, const utils::ray &ray) const;
      // void frustum_test_tri(const uint32_t &tri, const utils::frustum &frustum, std::unordered_set<uint32_t> &tiles_container) const;
      // bool frustum_test_tile(const uint32_t &tile, const utils::frustum &frustum) const;
      
      void fix_tile(const uint32_t &tile_index);
      void fix_tile2(const uint32_t &tile_index);
      void apply_matrix(const uint32_t &point_index, const glm::mat4 &matrix);

      size_t memory() const;
      void validate() const;
    };
  }
}

#endif

