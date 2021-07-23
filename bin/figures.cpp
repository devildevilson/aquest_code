#include "figures.h"

#include "utils/utility.h"
#include "utils/logging.h"

#include <iostream>
#include <cassert>

//#define EPSILON 0.00001f

#define PRINT_VEC(name, vec) std::cout << name << " x: " << vec.x << " y: " << vec.y << " z: " << vec.z << "\n";

glm::vec3 closest_point(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &point) {
  const glm::vec3 ab = b - a;
  const float l2 = glm::length2(ab);
  if (l2 <= 0.00001f) return a;
  const glm::vec3 ap = point - a;

  const float t = glm::max(0.0f, glm::min(1.0f, glm::dot(ap, ab) / l2));
  return a + t * ab;
}

void print_vec(const glm::vec3 &vec) {
  std::cout << "x: " << vec.x << " y: " << vec.y << " z: " << vec.z << '\n';
}

float angle(const glm::vec3 &a, const glm::vec3 &b) {
  const float dotV = glm::dot(a, b);
  const float lenSq1 = glm::length2(a);
  const float lenSq2 = glm::length2(b);

  return glm::acos(dotV / glm::sqrt(lenSq1 * lenSq2));
}

namespace devils_engine {
  namespace utils {
    size_t power4(const uint32_t &pow) {
      return 1 << pow*2;
    }
  }
  
  namespace map {
    triangle::triangle() : points(0), current_level(0), upper_level_index(UINT32_MAX) {}
    triangle::triangle(const uint32_t &p1, const uint32_t &p2, const uint32_t &p3) : points(p1, p2, p3), current_level(0), upper_level_index(UINT32_MAX) {}
    triangle::triangle(const uint32_t &p1, const uint32_t &p2, const uint32_t &p3, const uint32_t &current_level, const uint32_t &upper_level_index) : points(p1, p2, p3), current_level(current_level), upper_level_index(upper_level_index) {}
    tile::neighbour::neighbour() : index(UINT32_MAX), points{UINT32_MAX, UINT32_MAX} {}
    tile::tile(const uint32_t &index) : index(index) {}
    // , points{UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX}, neighbours{UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX}
    // void tile::add_neighbour(const uint32_t &index, const uint32_t &point_index) {
    //   for (uint32_t i = 0; i < 6; ++i) {
    //     if (neighbours[i] == index) break;
    //     if (neighbours[i] == UINT32_MAX) {
    //       neighbours[i] = index;
    //       break;
    //     }
    //   }
    //
    //   for (uint32_t i = 0; i < 6; ++i) {
    //     if (points[i] == point_index) return;
    //     if (points[i] == UINT32_MAX) {
    //       points[i] = point_index;
    //       return;
    //     }
    //   }
    //
    //   throw std::runtime_error("full tile");
    // }

    void tile::add_neighbour(const uint32_t &index, const uint32_t &point_a, const uint32_t &point_b) {
//       if (this->index == 0) {
//         PRINT_VAR("n index  ", index)
//         PRINT_VAR("points[0]", point_a)
//         PRINT_VAR("points[1]", point_b)
//       }
      
      for (uint32_t i = 0; i < 6; ++i) {
        if (neighbours[i].index == index) {
          assert(neighbours[i].points[0] == point_a || neighbours[i].points[1] == point_a);
          assert(neighbours[i].points[0] == point_b || neighbours[i].points[1] == point_b);
          return;
        }

        if (neighbours[i].index == UINT32_MAX) {
          neighbours[i].index = index;
          neighbours[i].points[0] = point_a;
          neighbours[i].points[1] = point_b;
          return;
        }
      }

      throw std::runtime_error("full tile");
    }

    container::container(const float &radius, const size_t &detail, const glm::mat3 &rotation) {
      utils::time_log time("tiles generation");

      std::unordered_map<size_t, uint32_t> edges_point;
      std::unordered_map<size_t, std::pair<uint32_t, uint32_t>> edges_hex;
      std::unordered_map<size_t, uint32_t> neighbor_hex;

      size_t triangles_count = 20*utils::power4(0);
      for (size_t i = 1; i < detail+1; ++i) {
        triangles_count += 20*utils::power4(i);
      }

      const size_t last_level_triangles_count = 20*utils::power4(detail);
      const size_t icosahedron_points_count = (last_level_triangles_count*3-12*5)/6+12;
      const size_t triangle_hex_count = last_level_triangles_count;
      const size_t hex_count = triangle_hex_count + icosahedron_points_count;
      const size_t final_points_count = icosahedron_points_count + last_level_triangles_count + last_level_triangles_count*6/2;

      triangles.reserve(triangles_count);
      tiles.reserve(hex_count);
      points.reserve(final_points_count);

//       std::cout << "computed " << '\n';
//       std::cout << "triangles_count " << triangles_count << '\n';
//       std::cout << "last_level_triangles_count " << last_level_triangles_count << '\n';
//       std::cout << "icosahedron_points_count " << icosahedron_points_count << '\n';
//       std::cout << "triangle_hex_count " << triangle_hex_count << '\n';
//       std::cout << "hex_count " << hex_count << '\n';
//       std::cout << "final_points_count " << final_points_count << '\n';

      const float t = (1.0f + std::sqrt(5.0f)) / 2.0f;
      // поворот я решил вычислять в другом месте (ну то есть отдельно)
      (void)rotation;
      add_vertex(radius, glm::vec3(-1,  t,  0)); // rotation*
      add_vertex(radius, glm::vec3( 1,  t,  0));
      add_vertex(radius, glm::vec3(-1, -t,  0));
      add_vertex(radius, glm::vec3( 1, -t,  0));

      add_vertex(radius, glm::vec3( 0, -1,  t));
      add_vertex(radius, glm::vec3( 0,  1,  t));
      add_vertex(radius, glm::vec3( 0, -1, -t));
      add_vertex(radius, glm::vec3( 0,  1, -t));

      add_vertex(radius, glm::vec3( t,  0, -1));
      add_vertex(radius, glm::vec3( t,  0,  1));
      add_vertex(radius, glm::vec3(-t,  0, -1));
      add_vertex(radius, glm::vec3(-t,  0,  1));

      // add_vertex(radius, glm::rotate(glm::vec3(-1,  t,  0), -a, cross));
      // add_vertex(radius, glm::rotate(glm::vec3( 1,  t,  0), -a, cross));
      // add_vertex(radius, glm::rotate(glm::vec3(-1, -t,  0), -a, cross));
      // add_vertex(radius, glm::rotate(glm::vec3( 1, -t,  0), -a, cross));
      //
      // add_vertex(radius, glm::rotate(glm::vec3( 0, -1,  t), -a, cross));
      // add_vertex(radius, glm::rotate(glm::vec3( 0,  1,  t), -a, cross));
      // add_vertex(radius, glm::rotate(glm::vec3( 0, -1, -t), -a, cross));
      // add_vertex(radius, glm::rotate(glm::vec3( 0,  1, -t), -a, cross));
      //
      // add_vertex(radius, glm::rotate(glm::vec3( t,  0, -1), -a, cross));
      // add_vertex(radius, glm::rotate(glm::vec3( t,  0,  1), -a, cross));
      // add_vertex(radius, glm::rotate(glm::vec3(-t,  0, -1), -a, cross));
      // add_vertex(radius, glm::rotate(glm::vec3(-t,  0,  1), -a, cross));

      // 5 triangles around point 0
      triangles.push_back(triangle( 0, 11,  5));
      triangles.push_back(triangle( 0,  5,  1));
      triangles.push_back(triangle( 0,  1,  7));
      triangles.push_back(triangle( 0,  7, 10));
      triangles.push_back(triangle( 0, 10, 11));

      // 5 adjacent triangles
      triangles.push_back(triangle( 1,  5,  9));
      triangles.push_back(triangle( 5, 11,  4));
      triangles.push_back(triangle(11, 10,  2));
      triangles.push_back(triangle(10,  7,  6));
      triangles.push_back(triangle( 7,  1,  8));

      // 5 triangles around point 3
      triangles.push_back(triangle( 3,  9,  4));
      triangles.push_back(triangle( 3,  4,  2));
      triangles.push_back(triangle( 3,  2,  6));
      triangles.push_back(triangle( 3,  6,  8));
      triangles.push_back(triangle( 3,  8,  9));

      // 5 adjacent triangles
      triangles.push_back(triangle( 4,  9,  5));
      triangles.push_back(triangle( 2,  4, 11));
      triangles.push_back(triangle( 6,  2, 10));
      triangles.push_back(triangle( 8,  6,  7));
      triangles.push_back(triangle( 9,  8,  1));

      // refine triangles
      size_t start = 0;
      size_t count = triangles.size();
      for (size_t k = 0; k < detail; k++) {
        for (size_t i = start; i < count; ++i) {
          // replace triangle by 4 triangles
          const uint32_t a = add_middle_point(radius, triangles[i].points.x, triangles[i].points.y, edges_point);
          const uint32_t b = add_middle_point(radius, triangles[i].points.y, triangles[i].points.z, edges_point);
          const uint32_t c = add_middle_point(radius, triangles[i].points.z, triangles[i].points.x, edges_point);

          triangles[i].next_level[0] = triangles.size();
          triangles.push_back(triangle(triangles[i].points.x, a, c, k+1, i));
          triangles[i].next_level[1] = triangles.size();
          triangles.push_back(triangle(triangles[i].points.y, b, a, k+1, i));
          triangles[i].next_level[2] = triangles.size();
          triangles.push_back(triangle(triangles[i].points.z, c, b, k+1, i));
          triangles[i].next_level[3] = triangles.size();
          triangles.push_back(triangle(a, b, c, k+1, i));
        }

        start = count;
        count = triangles.size();
      }

      std::cout << "real " << '\n';
      std::cout << "triangles_count " << triangles.size() << '\n';
      std::cout << "last_level_triangles_count " << (count-start) << '\n';
      std::cout << "icosahedron_points_count " << points.size() << '\n';

      // теперь мы собственно должны собрать гексы
      // наверное сначала создадим по точкам, чтобы удобно было по индексам доступ получать
      for (size_t i = 0; i < points.size(); ++i) {
        tiles.emplace_back(i);
      }

      const size_t point_hexes = points.size();
      
//       std::unordered_map<uint32_t, std::unordered_set<uint32_t>> hex_tri;
//       for (size_t i = start; i < count; ++i) {
//         const triangle &tri = triangles[i];
//         ASSERT(hex_tri[tri.points.x].find(i) == hex_tri[tri.points.x].end());
//         ASSERT(hex_tri[tri.points.y].find(i) == hex_tri[tri.points.y].end());
//         ASSERT(hex_tri[tri.points.z].find(i) == hex_tri[tri.points.z].end());
//         hex_tri[tri.points.x].insert(i);
//         hex_tri[tri.points.y].insert(i);
//         hex_tri[tri.points.z].insert(i);
//       }
//       
//       size_t counter = 0;
//       for (const auto &p : hex_tri) {
//         //ASSERT(p.second.size() == 3);
//         if (p.second.size() == 5) ++counter;
//       }
//       ASSERT(counter == 12);

//       std::unordered_map<uint32_t, std::vector<uint32_t>> point_hex;
      // по идее в start хранится начало последнего уровня
      // некоторые тайлы получают неверные пары точек
      for (size_t i = start; i < count; ++i) {
        triangle &tri = triangles[i];
        const glm::vec3 tri_center = (points[tri.points.x] + points[tri.points.y] + points[tri.points.z]) / 3.0f;
        const float hex_side_length = glm::distance(points[tri.points.x], points[tri.points.y]) / 3.0f;
        const uint32_t index = add_vertex(tri_center);
        tiles.emplace_back(index);
        const uint32_t current_tile_index = tiles.size()-1;
//         ASSERT(current_tile_index == index);

        tile::neighbour n[6];
        n[0].index = tri.points.x;
        n[1].index = tri.points.y;
        n[2].index = tri.points.z;

        {
          const uint32_t point_a = tri.points.x;
          const uint32_t point_b = tri.points.y;
          const glm::vec3 point = closest_point(points[point_a], points[point_b], tri_center);
          const glm::vec3 dir = glm::normalize(points[point_b] - points[point_a]);
          const glm::vec3 edge_point1 = point + hex_side_length/2.0f * dir;
          const glm::vec3 edge_point2 = point - hex_side_length/2.0f * dir;

          const auto points_indices = add_hex_point(point_a, point_b, edge_point1, edge_point2, edges_hex);
          const float dist1 = glm::distance(points[point_a], points[points_indices.first]);
          const float dist2 = glm::distance(points[point_a], points[points_indices.second]);
          
          ASSERT(points_indices.first != points_indices.second);
          
//           // неверно нахожу точки???
//           point_hex[points_indices.first].push_back(current_tile_index);
//           point_hex[points_indices.second].push_back(current_tile_index);
//           
//           const auto &tri_a = hex_tri[point_a];
//           const auto &tri_b = hex_tri[point_b];
//           std::pair<uint32_t, uint32_t> two_tri = std::make_pair(UINT32_MAX, UINT32_MAX);
//           for (const uint32_t id : tri_a) {
//             if (tri_b.find(id) != tri_b.end()) {
//               ASSERT(two_tri.first == UINT32_MAX || two_tri.second == UINT32_MAX);
//               if (two_tri.first != UINT32_MAX) two_tri.second = id;
//               else two_tri.first = id;
//             }
//           }
//           
//           const uint32_t opposite_tri = two_tri.first == i ? two_tri.second : two_tri.first;
//           const uint32_t n_hex_index = point_hexes + (opposite_tri - start); // по идее вот это настоящий индекс соседа

          // tiles.back().add_neighbour(point_a, points_indices.first);
          // tiles.back().add_neighbour(point_b, points_indices.second);
          // tiles[point_a].add_neighbour(current_tile_index, dist1 < dist2 ? points_indices.first : points_indices.second);
          // tiles[point_b].add_neighbour(current_tile_index, dist1 < dist2 ? points_indices.second : points_indices.first);

          n[0].points[0] = dist1 < dist2 ? points_indices.first : points_indices.second;
          n[1].points[0] = dist1 < dist2 ? points_indices.second : points_indices.first;

          const uint32_t side_neighbor = find_neighbor_tile(current_tile_index, points_indices.first, points_indices.second, neighbor_hex);
          if (side_neighbor != UINT32_MAX) { // сосед по ребру найден
//             ASSERT(n_hex_index == side_neighbor); // я видимо верно нахожу соседей
            n[3].index = side_neighbor; // сосед у меня находится по индексу серединной точки
            //n[3].index = n_hex_index;
            n[3].points[0] = points_indices.first;
            n[3].points[1] = points_indices.second;
            // tiles.back().add_neighbour(side_neighbor, points_indices.first);
            // tiles[side_neighbor].add_neighbour(current_tile_index, points_indices.first);
          }
        }

        {
          const uint32_t point_a = tri.points.y;
          const uint32_t point_b = tri.points.z;
          const glm::vec3 point = closest_point(points[point_a], points[point_b], tri_center);
          const glm::vec3 dir = glm::normalize(points[point_b] - points[point_a]);
          const glm::vec3 edge_point1 = point + hex_side_length/2.0f * dir;
          const glm::vec3 edge_point2 = point - hex_side_length/2.0f * dir;

          const auto points_indices = add_hex_point(point_a, point_b, edge_point1, edge_point2, edges_hex);
          const float dist1 = glm::distance(points[point_a], points[points_indices.first]);
          const float dist2 = glm::distance(points[point_a], points[points_indices.second]);
          
          ASSERT(points_indices.first != points_indices.second);
          
//           point_hex[points_indices.first].push_back(current_tile_index);
//           point_hex[points_indices.second].push_back(current_tile_index);
// 
//           const auto &tri_a = hex_tri[point_a];
//           const auto &tri_b = hex_tri[point_b];
//           std::pair<uint32_t, uint32_t> two_tri = std::make_pair(UINT32_MAX, UINT32_MAX);
//           for (const uint32_t id : tri_a) {
//             if (tri_b.find(id) != tri_b.end()) {
//               ASSERT(two_tri.first == UINT32_MAX || two_tri.second == UINT32_MAX);
//               if (two_tri.first != UINT32_MAX) two_tri.second = id;
//               else two_tri.first = id;
//             }
//           }
//           
//           const uint32_t opposite_tri = two_tri.first == i ? two_tri.second : two_tri.first;
//           const uint32_t n_hex_index = point_hexes + (opposite_tri - start); // по идее вот это настоящий индекс соседа

          n[1].points[1] = dist1 < dist2 ? points_indices.first : points_indices.second;
          n[2].points[0] = dist1 < dist2 ? points_indices.second : points_indices.first;

          const uint32_t side_neighbor = find_neighbor_tile(current_tile_index, points_indices.first, points_indices.second, neighbor_hex);
          if (side_neighbor != UINT32_MAX) {
//             ASSERT(n_hex_index == side_neighbor);
            n[4].index = side_neighbor;
            //n[4].index = n_hex_index;
            n[4].points[0] = points_indices.first;
            n[4].points[1] = points_indices.second;
            // tiles.back().add_neighbour(side_neighbor, points_indices.first);
            // tiles[side_neighbor].add_neighbour(current_tile_index, points_indices.first);
          }
        }

        {
          const uint32_t point_a = tri.points.z;
          const uint32_t point_b = tri.points.x;
          const glm::vec3 point = closest_point(points[point_a], points[point_b], tri_center);
          const glm::vec3 dir = glm::normalize(points[point_b] - points[point_a]);
          const glm::vec3 edge_point1 = point + hex_side_length/2.0f * dir;
          const glm::vec3 edge_point2 = point - hex_side_length/2.0f * dir;

          const auto points_indices = add_hex_point(point_a, point_b, edge_point1, edge_point2, edges_hex);
          const float dist1 = glm::distance(points[point_a], points[points_indices.first]);
          const float dist2 = glm::distance(points[point_a], points[points_indices.second]);
          
          ASSERT(points_indices.first != points_indices.second);
          
//           point_hex[points_indices.first].push_back(current_tile_index);
//           point_hex[points_indices.second].push_back(current_tile_index);
// 
//           const auto &tri_a = hex_tri[point_a];
//           const auto &tri_b = hex_tri[point_b];
//           std::pair<uint32_t, uint32_t> two_tri = std::make_pair(UINT32_MAX, UINT32_MAX);
//           for (const uint32_t id : tri_a) {
//             if (tri_b.find(id) != tri_b.end()) {
//               ASSERT(two_tri.first == UINT32_MAX || two_tri.second == UINT32_MAX);
//               if (two_tri.first != UINT32_MAX) two_tri.second = id;
//               else two_tri.first = id;
//             }
//           }
//           
//           const uint32_t opposite_tri = two_tri.first == i ? two_tri.second : two_tri.first;
//           const uint32_t n_hex_index = point_hexes + (opposite_tri - start); // по идее вот это настоящий индекс соседа

          n[2].points[1] = dist1 < dist2 ? points_indices.first : points_indices.second;
          n[0].points[1] = dist1 < dist2 ? points_indices.second : points_indices.first;

          const uint32_t side_neighbor = find_neighbor_tile(current_tile_index, points_indices.first, points_indices.second, neighbor_hex);
          if (side_neighbor != UINT32_MAX) {
//             ASSERT(n_hex_index == side_neighbor);
            n[5].index = side_neighbor;
            //n[5].index = n_hex_index;
            n[5].points[0] = points_indices.first;
            n[5].points[1] = points_indices.second;
            // tiles.back().add_neighbour(side_neighbor, points_indices.first);
            // tiles[side_neighbor].add_neighbour(current_tile_index, points_indices.first);
          }
        }

        for (uint32_t j = 0; j < 6; ++j) {
          if (n[j].index == UINT32_MAX) continue;
          ASSERT(n[j].points[0] != UINT32_MAX);
          ASSERT(n[j].points[1] != UINT32_MAX);

          tiles.back().add_neighbour(n[j].index, n[j].points[0], n[j].points[1]);
          tiles[n[j].index].add_neighbour(current_tile_index, n[j].points[0], n[j].points[1]);
        }

        tri.next_level[0] = current_tile_index;
        tri.next_level[1] = tri.points.x;
        tri.next_level[2] = tri.points.y;
        tri.next_level[3] = tri.points.z;
      }
      
//       for (const auto &p : point_hex) {
//         ASSERT(p.second.size() == 2); // одного соседа мы не затрагиваем
//         
//         
//       }

//       std::cout << "triangle_hex_count " << (tiles.size()-point_hexes) << '\n';
//       std::cout << "hex_count " << tiles.size() << '\n';
//       std::cout << "final_points_count " << points.size() << '\n';

      ASSERT(triangles_count == triangles.size());
      ASSERT(hex_count == tiles.size());
      ASSERT(final_points_count == points.size());

      //fix_triangle_dirs();
    }

    glm::vec3 compute_normal(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2) {
      return glm::normalize(glm::cross(v1-v0, v2-v0));
    }

    void container::fix_triangle_dirs() {
      for (auto &tri : triangles) {
        const glm::vec3 normal = compute_normal(points[tri.points.x], points[tri.points.y], points[tri.points.z]);
        const float dot = glm::dot(normal, points[tri.points.x]);
        if (dot < 0.0f) std::swap(tri.points.y, tri.points.z);
      }

      for (auto &tile : tiles) {
        const uint32_t points_count = tile.is_pentagon() ? 5 : 6;
        for (uint32_t i = 0; i < points_count; ++i) {
          const glm::vec3 normal = compute_normal(points[tile.index], points[tile.neighbours[i].points[0]], points[tile.neighbours[i].points[1]]);
          const float dot = glm::dot(normal, points[tile.index]);
          if (dot < 0.0f) std::swap(tile.neighbours[i].points[0], tile.neighbours[i].points[1]);
        }
      }
    }

    size_t container::detail_level() const {
      size_t triangle_count = triangles.size();
      size_t level = 0;
      while (triangle_count != 0) {
        const size_t level_count = 20*utils::power4(level);
        triangle_count -= level_count;
        ++level;
      }

      return level-1;
    }

    uint32_t container::add_vertex(const float &radius, const glm::vec3 &v) {
      const glm::vec3 norm = glm::normalize(v);
      points.push_back(norm); // *radius
      // я пока решил не использовать радиус, а потом умножить после построения
      // насколько это вообще мне помогло?
      (void)radius;
      return points.size()-1;
    }

    uint32_t container::add_vertex(const glm::vec3 &v) {
      points.push_back(v);
      return points.size()-1;
    }

    uint32_t container::add_middle_point(const float &radius, const uint32_t &p1, const uint32_t &p2, std::unordered_map<size_t, uint32_t> &cache) {
      //(void)radius;
      const size_t smaller = std::min(p1, p2);
      const size_t greater = std::max(p1, p2);
      const size_t key = (smaller << 32) | greater;

      auto itr = cache.find(key);
      if (itr != cache.end()) return itr->second;

      const glm::vec3 &point1 = points[p1];
      const glm::vec3 &point2 = points[p2];
      const glm::vec3 middle = (point1 + point2) / 2.0f;

      const uint32_t index = add_vertex(radius, middle);
      //const uint32_t index = add_vertex(middle);
      cache.insert(std::make_pair(key, index));
      return index;
    }

    std::pair<uint32_t, uint32_t> container::add_hex_point(const uint32_t &p1, const uint32_t &p2, const glm::vec3 &edge_point1, const glm::vec3 &edge_point2, std::unordered_map<size_t, std::pair<uint32_t, uint32_t>> &cache) {
      const size_t smaller = std::min(p1, p2);
      const size_t greater = std::max(p1, p2);
      const size_t key = (smaller << 32) | greater;

      auto itr = cache.find(key);
      if (itr != cache.end()) return itr->second;

      const uint32_t index1 = add_vertex(edge_point1);
      const uint32_t index2 = add_vertex(edge_point2);
      const auto pair = std::make_pair(index1, index2);
      cache.insert(std::make_pair(key, pair));
      return pair;
    }

    uint32_t container::find_neighbor_tile(const uint32_t &tile, const uint32_t &p1, const uint32_t &p2, std::unordered_map<size_t, uint32_t> &cache) {
      const size_t smaller = std::min(p1, p2);
      const size_t greater = std::max(p1, p2);
      const size_t key = (smaller << 32) | greater;

      auto itr = cache.find(key);
      if (itr != cache.end()) return itr->second;

      cache.insert(std::make_pair(key, tile));
      return UINT32_MAX;
    }

//     bool intersect(const glm::vec3 &point, const utils::ray &ray) {
//       const float radius = 0.00001f;
//       const glm::vec3 oc = ray.pos - point;
//       const float a = dot(ray.dir, ray.dir);
//       const float b = 2.0f * dot(oc, ray.dir);
//       const float c = dot(oc,oc) - radius*radius;
//       const float discriminant = b*b - 4*a*c;
//       return (discriminant > 0.0f);
//     }

//     uint32_t container::cast_ray(const utils::ray &ray) const {
//       const size_t detail = detail_level();
//       size_t current_detail_level = 0;
//       size_t current_tri_index = UINT32_MAX;
// 
//       for (size_t i = 0; i < 20*utils::power4(current_detail_level); ++i) {
//         if (intersect_tri(i, ray)) {
//           ++current_detail_level;
//           current_tri_index = i;
//           break;
//         }
//       }
// 
//       assert(current_tri_index != UINT32_MAX);
//       std::cout << "current_tri_index " << current_tri_index << '\n';
// 
//       //size_t tri_offset = 0;
//       while (current_detail_level <= detail) {
//         const triangle &tri = triangles[current_tri_index];
//         current_tri_index = UINT32_MAX;
//         for (size_t i = 0; i < 4; ++i) {
//           const uint32_t tri_index = tri.next_level[i];
//           if (intersect_tri(tri_index, ray)) {
//             current_tri_index = tri_index;
//             ++current_detail_level;
//             std::cout << "current_tri_index " << current_tri_index << '\n';
//             break;
//           }
//         }
// 
//         assert(current_tri_index != UINT32_MAX);
//       }
// 
//       assert(triangles[current_tri_index].current_level == detail);
// 
//       for (size_t i = 0; i < 4; ++i) {
//         const uint32_t tile_index = triangles[current_tri_index].next_level[i];
//         std::cout << "tile_index " << tile_index << '\n';
//         if (intersect_tile(tile_index, ray)) return tile_index;
//       }
// 
//       return UINT32_MAX;
//     }
// 
//     // void container::frustum_test(const utils::frustum &frustum, std::unordered_set<uint32_t> &tiles_container) const {
//     //   // тут по идее рекурсивная функция
//     //   for (size_t i = 0; i < tri_count(0); ++i) {
//     //     frustum_test_tri(i, frustum, tiles_container);
//     //   }
//     // }
// 
//     bool container::intersect_tri(const uint32_t &triangle_index, const utils::ray &ray) const {
//       const triangle &tri = triangles[triangle_index];
//       return intersect_tri(points[tri.points.x], points[tri.points.y], points[tri.points.z], ray);
//     }
// 
//     bool container::intersect_tile(const uint32_t &tile_index, const utils::ray &ray) const {
//       const uint32_t points_count = tiles[tile_index].is_pentagon() ? 5 : 6;
//       for (uint32_t i = 0; i < points_count; ++i) {
//         const uint32_t point_a = tiles[tile_index].index;
//         const uint32_t point_b = tiles[tile_index].neighbours[i].points[0];
//         const uint32_t point_c = tiles[tile_index].neighbours[i].points[1];
// 
//         const bool ret = intersect_tri(points[point_a], points[point_b], points[point_c], ray);
//         if (ret) return true;
//       }
// 
//       return false;
//     }
// 
//     bool container::intersect_tri(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, const utils::ray &ray) const {
//       // if (intersect(v0, ray)) return true;
//       // if (intersect(v1, ray)) return true;
//       // if (intersect(v2, ray)) return true;
// 
//       const glm::vec3 v0v1 = v1 - v0;
//       const glm::vec3 v0v2 = v2 - v0;
//       const glm::vec3 pvec = glm::cross(ray.dir, v0v2);
//       const float det = glm::dot(v0v1, pvec);
// 
//       //if (det > -EPSILON) return false;
//       if (det < EPSILON) return false;
//       //if (glm::abs(det) < EPSILON) return false;
// 
//       // std::cout << "normal ok" << '\n';
// 
//       const float invDet = 1 / det;
// 
//       float u,v,t;
// 
//       const glm::vec3 tvec = ray.pos - v0;
//       u = glm::dot(tvec, pvec) * invDet;
// 
//       // std::cout << "u " << u << '\n';
//       //if (u < 0 || u > 1) return false;
//       if (u < -EPSILON || u > 1+EPSILON) return false;
// 
//       const glm::vec3 qvec = glm::cross(tvec, v0v1);
//       v = glm::dot(ray.dir, qvec) * invDet;
//       // std::cout << "v " << v << '\n';
//       //if (v < 0 || u + v > 1) return false;
//       if (v < -EPSILON || u + v > 1+EPSILON) return false;
// 
//       t = glm::dot(v0v2, qvec) * invDet;
// 
//       (void)t;
// 
//       return true;
//     }

    // void container::frustum_test_tri(const uint32_t &tri, const utils::frustum &frustum, std::unordered_set<uint32_t> &tiles_container) const {
    //   const size_t max_detail = detail_level();
    //   const triangle &t = triangles[tri];
    //
    //   if (t.current_level == max_detail) {
    //     for (uint32_t i = 0; i < 4; ++i) {
    //       if (frustum_test_tile(i.next_level[i], frustum)) tiles_container.insert(i.next_level[i]);
    //     }
    //   } else {
    //     for (uint32_t i = 0; i < 4; ++i) {
    //
    //       if (frustum_test_tri(i.next_level[i], frustum, tiles_container)) tiles_container.insert(i.next_level[i]);
    //     }
    //   }
    // }
    //
    // bool container::frustum_test_tile(const uint32_t &tile, const utils::frustum &frustum, std::vector<uint32_t> &tiles_container) const {
    //
    // }
    
    glm::vec3 compute_tri_dir(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c) {
      const glm::vec3 ab = b - a;
      const glm::vec3 ac = c - a;
      const glm::vec3 cross = glm::cross(ab, ac);
      return cross;
    }
    
    glm::vec3 compute_tri_normal(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c) {
      return glm::normalize(compute_tri_dir(a, b, c));
    }
    
    void container::fix_tile(const uint32_t &tile_index) {
      ASSERT(tile_index < tiles.size());
      tile &t = tiles[tile_index];
      const glm::vec3 dir = points[t.index];
      
      const uint32_t n_count = t.is_pentagon() ? 5 : 6;
      for (uint32_t i = 0; i < n_count; ++i) {
        const glm::vec3 tri_dir = compute_tri_dir(points[t.index], points[t.neighbours[i].points[0]], points[t.neighbours[i].points[1]]);
        if (glm::dot(dir, tri_dir) < 0.0f) {
          std::swap(t.neighbours[i].points[0], t.neighbours[i].points[1]);
        }
      }
      
      // нужно чтобы пары чисел в соседях шли друг за другом по условию как ниже
      // выше алгортим работает через раз
      
      std::vector<tile::neighbour> new_n;
      std::unordered_set<uint32_t> set;
      new_n.push_back(t.neighbours[0]);
      set.insert(t.neighbours[0].index);
      
      for (uint32_t i = 0; i < new_n.size(); ++i) {
        //const uint32_t current_index = t.neighbours[i].points[1];
        const uint32_t current_index = new_n[i].points[1];
        for (uint32_t j = 0; j < n_count; ++j) {
          //if (i == j) continue;
          if (set.find(t.neighbours[j].index) != set.end()) continue;
          
          if (t.neighbours[j].points[0] == current_index || t.neighbours[j].points[1] == current_index) {
            new_n.push_back(t.neighbours[j]);
            set.insert(t.neighbours[j].index);
            if (t.neighbours[j].points[1] == current_index) std::swap(new_n.back().points[0], new_n.back().points[1]);
          }
        }
      }
      
      ASSERT(new_n.size() == n_count);
      for (uint32_t i = 0; i < n_count; ++i) {
        t.neighbours[i] = new_n[i];
      }
      
//       for (uint32_t i = 0; i < n_count; ++i) {
//         const uint32_t current_index = t.neighbours[i].points[1];
//         for (uint32_t j = i+1; j < n_count-1; ++j) {
//           //if (i == j) continue;
//           
//           if (t.neighbours[j].points[0] == current_index || t.neighbours[j].points[1] == current_index) {
//             //if (j == (i+1) % n_count) break;
//             std::swap(t.neighbours[j], t.neighbours[(i+1) % n_count]);
//           }
//         }
//         
//         if (i+1 == n_count) {
//           ASSERT(current_index == t.neighbours[0].points[0]);
//         }
//       }
      
      for (uint32_t i = 0; i < n_count; ++i) {
        const uint32_t current_index = t.neighbours[i].points[1];
        const uint32_t next_neighbour = (i+1) % n_count;
        ASSERT(current_index == t.neighbours[next_neighbour].points[0]);
        ASSERT(t.neighbours[i].points[1] != t.neighbours[i].points[0]);
        (void)current_index;
        (void)next_neighbour;
      }
    }
    
    void container::fix_tile2(const uint32_t &tile_index) {
      ASSERT(tile_index < tiles.size());
      tile &t = tiles[tile_index];
      const glm::vec3 center = points[t.index];
      const glm::vec3 dir = glm::normalize(center);
      const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
      const glm::vec3 right = glm::normalize(glm::cross(up, dir));
      const uint32_t n_count = t.is_pentagon() ? 5 : 6;
      float max_dot = 0.0f;
      uint32_t index = 0;
      for (uint32_t i = 0; i < n_count; ++i) {
        const uint32_t point_index = t.neighbours[i].points[0];
        const glm::vec3 point = points[point_index];
        const glm::vec3 point_dir = point - center;
        if (glm::dot(right, point_dir) < 0.0f) continue;
        const float d = glm::dot(point, up);
        if (d > max_dot) {
          max_dot = d;
          index = i;
        }
      }
      
      //ASSERT(index != UINT32_MAX);
      
      std::swap(t.neighbours[0], t.neighbours[index]);
      std::vector<tile::neighbour> new_n;
      std::unordered_set<uint32_t> set;
      new_n.push_back(t.neighbours[0]);
      set.insert(t.neighbours[0].index);
      
      for (uint32_t i = 0; i < new_n.size(); ++i) {
        //const uint32_t current_index = t.neighbours[i].points[1];
        const uint32_t current_index = new_n[i].points[1];
        for (uint32_t j = 0; j < n_count; ++j) {
          //if (i == j) continue;
          if (set.find(t.neighbours[j].index) != set.end()) continue;
          
          if (t.neighbours[j].points[0] == current_index || t.neighbours[j].points[1] == current_index) {
            new_n.push_back(t.neighbours[j]);
            set.insert(t.neighbours[j].index);
            if (t.neighbours[j].points[1] == current_index) std::swap(new_n.back().points[0], new_n.back().points[1]);
          }
        }
      }
      
      ASSERT(new_n.size() == n_count);
      for (uint32_t i = 0; i < n_count; ++i) {
        t.neighbours[i] = new_n[i];
      }
      
      for (uint32_t i = 0; i < n_count; ++i) {
        const uint32_t current_index = t.neighbours[i].points[1];
        const uint32_t next_neighbour = (i+1) % n_count;
        ASSERT(current_index == t.neighbours[next_neighbour].points[0]);
        ASSERT(t.neighbours[i].points[1] != t.neighbours[i].points[0]);
        (void)current_index;
        (void)next_neighbour;
      }
    }
    
    void container::apply_matrix(const uint32_t &point_index, const glm::mat4 &matrix) {
      ASSERT(point_index < points.size());
      const glm::vec4 p = matrix * glm::vec4(points[point_index], 1.0f);
      points[point_index] = p;
    }

    size_t container::memory() const {
      return triangles.size() * sizeof(triangles[0]) +
             tiles.size() * sizeof(tiles[0]) +
             points.size() * sizeof(points[0]);
    }

    void container::validate() const {
      size_t penta_count = 0;
      for (const auto &tile : tiles) {
        assert(tile.index != UINT32_MAX);
        //for (size_t i = 0; i < 5; ++i) assert(tile.points[i] != UINT32_MAX);
        for (size_t i = 0; i < 5; ++i) assert(tile.neighbours[i].index != UINT32_MAX);
        if (tile.is_pentagon()) {
          ++penta_count;
          //assert(tile.points[5] == UINT32_MAX);
          assert(tile.neighbours[5].index == UINT32_MAX);
        } else {
          //assert(tile.points[5] != UINT32_MAX);
          assert(tile.neighbours[5].index != UINT32_MAX);
        }
      }

      assert(penta_count == 12);
    }
  }
}
