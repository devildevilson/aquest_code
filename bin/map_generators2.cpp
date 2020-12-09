#include "map_generators2.h"

#include "utils/logging.h"
#include "figures.h"
#include "utils/works_utils.h"
#include "utils/concurrent_vector.h"
#include "generator_context2.h"
#include <atomic>
#include <set>
#include "generator_container.h"
#include "map.h"
#include "FastNoise.h"
#include "data_parser.h"
#include "utils/globals.h"
#include "stats.h"
#include "game_time.h"
#include "seasons.h"
#include "render/shared_structures.h"
#include "image_parser.h"
#include "heraldy_parser.h"

namespace devils_engine {
  namespace map {
    const generator_pair default_generator_pairs[] = {
      generator_pair("begin", begin), // 0
      generator_pair("seting up generator", setup_generator),
      generator_pair("generating plates", generate_plates),
      generator_pair("generating plate datas", generate_plate_datas),
      generator_pair("computing boundary edges", compute_boundary_edges),
      generator_pair("computing plate boundary stress", compute_plate_boundary_stress), // 5
      generator_pair("computing plate boundary distances", compute_plate_boundary_distances),
      generator_pair("calculating vertex elevation", calculate_vertex_elevation),
      generator_pair("bluring tile elevation", blur_tile_elevation),
      generator_pair("normalizing tile elevation", std::bind(normalize_fractional_values, std::placeholders::_1, std::placeholders::_2, debug::entities::tile, debug::properties::tile::elevation)),
      generator_pair("computing tile heat", compute_tile_heat), // 10
      generator_pair("computing water tile distances", std::bind(compute_tile_distances, std::placeholders::_1, std::placeholders::_2, [] (
        const generator::context* ctx,
        const sol::table&,
        const uint32_t &tile_index
      ) -> bool {
        const float h = ctx->container->get_data<float>(debug::entities::tile, tile_index, debug::properties::tile::elevation);
        return h < 0.0f;
      }, "water_distance")),
      generator_pair("computing tile moisture", compute_tile_moisture),
      generator_pair("creating biomes", create_biomes),
      generator_pair("generating provinces", generate_provinces),
      generator_pair("province postprocessing", province_postprocessing),// 15
      generator_pair("calculating province neighbours", calculating_province_neighbours),
      generator_pair("generating cultures", generate_cultures),
      generator_pair("generating countries", generate_countries),
      generator_pair("generating titles", generate_titles),
      generator_pair("generate cities", generate_cities), // 20
      generator_pair("generate characters", generate_characters),
      generator_pair("generate heraldies", generate_heraldy)
    };

    render::color_t make_color_from_index(const uint32_t &index) {
      const uint32_t val1 = render::prng(index);
      const uint32_t val2 = render::prng(val1);
      const uint32_t val3 = render::prng(val2);
      const float f1 = render::prng_normalize(val1);
      const float f2 = render::prng_normalize(val2);
      const float f3 = render::prng_normalize(val3);
      return render::make_color(f1, f2, f3, 1.0f);
    }

    province_neighbour::province_neighbour() : container(UINT32_MAX) {}
    province_neighbour::province_neighbour(const bool across_water, const uint32_t &index) : container(UINT32_MAX) {
      const uint32_t b = uint32_t(across_water) << 31;
      ASSERT(index < INT32_MAX);
      container = b | index;
    }

    province_neighbour::province_neighbour(const province_neighbour &another) : container(another.container) {}
    province_neighbour::province_neighbour(const uint32_t &native) : container(native) {}

    bool province_neighbour::across_water() const {
      const uint32_t mask = uint32_t(1) << 31;
      ASSERT(mask == uint32_t(INT32_MAX)+1);
      return (container & mask) == mask;
    }

    uint32_t province_neighbour::index() const {
      const uint32_t mask = ~(uint32_t(1) << 31);
      ASSERT(mask == INT32_MAX);
      return container & mask;
    }

    bool province_neighbour::operator==(const province_neighbour &another) const { return container == another.container; }
    bool province_neighbour::operator!=(const province_neighbour &another) const { return container != another.container; }
    bool province_neighbour::operator<(const province_neighbour &another) const { return container < another.container; }
    bool province_neighbour::operator>(const province_neighbour &another) const { return container > another.container; }
    bool province_neighbour::operator<=(const province_neighbour &another) const { return container <= another.container; }
    bool province_neighbour::operator>=(const province_neighbour &another) const { return container >= another.container; }

    province_neighbour & province_neighbour::operator=(const province_neighbour &another) {
      container = another.container;
      return *this;
    }

    void update_noise_seed(generator::context* ctx) {
      union trans {
        uint32_t val;
        int32_t val1;
      };

      const uint32_t num = ctx->random->num();
      trans t;
      t.val = num;
      const int32_t final_num = t.val1;
      ctx->noise->SetSeed(final_num);
    }

    void map_triangle_add2(const map::container* map, const uint32_t &triangle_index, std::mutex &mutex, std::unordered_set<uint32_t> &unique_tiles, std::vector<uint32_t> &tiles_array) {
      const auto &tri = map->triangles[triangle_index];

      if (tri.current_level == core::map::detail_level) {
        for (uint32_t i = 0; i < 4; ++i) {
          const uint32_t tile_index = tri.next_level[i];
          {
            std::unique_lock<std::mutex> lock(mutex);
            auto itr = unique_tiles.find(tile_index);
            if (itr != unique_tiles.end()) continue;
            unique_tiles.insert(tile_index);
          }

          tiles_array.push_back(tile_index);
        }

        return;
      }

      for (uint32_t i = 0; i < 4; ++i) {
        const uint32_t next_triangle_index = tri.next_level[i];
        ASSERT(next_triangle_index != UINT32_MAX);
        map_triangle_add2(map, next_triangle_index, mutex, unique_tiles, tiles_array);
      }
    }

    bool push_new_tile_index(
      const core::map* map,
      const size_t &plate_count,
      const size_t &plate_index,
      const size_t &tile_index,
      //std::atomic<uint32_t>* tile_plate_indices,
      uint32_t* tile_plate_indices,
      //utils::concurrent_vector<uint32_t>* plate_tile_indices,
      std::vector<uint32_t>* plate_tile_indices,
      //std::vector<std::pair<uint32_t, uint32_t>> &active_tile_indices
      //active_tile_container_t &active_tile_indices
      //utils::concurrent_vector<std::pair<uint32_t, uint32_t>> &active_tile_indices
      std::vector<std::pair<uint32_t, uint32_t>> &active_tile_indices
    ) {
//       uint32_t data = plate_count;
      //if (!tile_plate_indices[tile_index].compare_exchange_strong(data, plate_index)) return false;
      if (tile_plate_indices[tile_index] != plate_count) return false;
      tile_plate_indices[tile_index] = plate_index;

      const auto &tile = render::unpack_data(map->get_tile(tile_index));
      for (uint32_t i = 0; i < 6; ++i) {
        const uint32_t neighbor_index = tile.neighbours[i];
        if (neighbor_index == UINT32_MAX) continue;
        if (tile_plate_indices[neighbor_index] != plate_count) {
//           uint32_t data = plate_index;
          //tile_plate_indices[tile_index].compare_exchange_strong(data, plate_count);
          return false;
        }
      }

      //std::cout << "current tile_index " << tile_index << "\n";
      //ASSERT(false);

      plate_tile_indices[plate_index].push_back(tile_index);

      for (uint32_t i = 0; i < 6; ++i) {
        const uint32_t neighbor_index = tile.neighbours[i];
        if (neighbor_index == UINT32_MAX) continue;
        active_tile_indices.push_back(std::make_pair(neighbor_index, plate_index));
      }

      // if (tile_plate_indices[tile_index] != plate_count) return false;
      // for (uint32_t i = 0; i < 6; ++i) {
      //   const uint32_t neighbor_index = map->tiles[tile_index].neighbours[i].index;
      //   if (neighbor_index == UINT32_MAX) continue;
      //   if (tile_plate_indices[neighbor_index] != plate_count) return false;
      // }
      //
      // ASSERT(plate_index <= plate_tile_indices.size())
      //
      // if (plate_index >= plate_tile_indices.size()) plate_tile_indices.emplace_back();
      // plate_tile_indices[plate_index].push_back(tile_index);
      // tile_plate_indices[tile_index] = plate_index;
      // for (uint32_t i = 0; i < 6; ++i) {
      //   const uint32_t neighbor_index = map->tiles[tile_index].neighbours[i].index;
      //   if (neighbor_index == UINT32_MAX) continue;
      //   active_tile_indices.emplace_back(neighbor_index, plate_index);
      // }

      return true;
    }

    void begin(generator::context* ctx, sol::table &table) {
      (void)table;
      utils::time_log log("prepare step");
      // создаем базовый каркас карты здесь

      const float angle1 = PI   * ctx->random->norm();
      const float angle2 = PI_2 * ctx->random->norm();

      ASSERT(angle1 != 0.0f);
      ASSERT(angle2 != 0.0f);
//       PRINT_VAR("angle1", angle1)
//       PRINT_VAR("angle2", angle2)

      // нужно создать случайную матрицу поворота
      glm::mat4 mat1 = glm::rotate(glm::mat4(1.0f), angle2, glm::vec3(0.0f, 1.0f, 0.0f));
                mat1 = glm::rotate(mat1,            angle1, glm::vec3(1.0f, 0.0f, 0.0f));
      glm::mat3 mat(mat1);
                mat1 = glm::scale(mat1, glm::vec3(core::map::world_radius, core::map::world_radius, core::map::world_radius));

//       PRINT_VEC3("mat", mat[0])
//       PRINT_VEC3("mat", mat[1])
//       PRINT_VEC3("mat", mat[2])
      map::container generated_core(core::map::world_radius, core::map::detail_level, mat); // возможно нужно как то это ускорить

      auto map = ctx->map;
      ASSERT(generated_core.points.size() == map->points_count());
      ASSERT(generated_core.tiles.size() == map->tiles_count());
      ASSERT(generated_core.triangles.size() == map->triangles_count());

      map->world_matrix = mat1;

      // придется переделать функции и добавить ожидание треду
      utils::submit_works_async(ctx->pool, generated_core.tiles.size(), [&generated_core] (const size_t &start, const size_t &count) {
        for (size_t i = start; i < start+count; ++i) {
          generated_core.fix_tile(i);
        }
      });
      utils::async_wait(ctx->pool);
      
      utils::submit_works_async(ctx->pool, generated_core.tiles.size(), [&generated_core] (const size_t &start, const size_t &count) {
        for (size_t i = start; i < start+count; ++i) {
          generated_core.fix_tile2(i);
        }
      });
      utils::async_wait(ctx->pool);
      
      utils::submit_works_async(ctx->pool, generated_core.points.size(), [&generated_core, mat1] (const size_t &start, const size_t &count) {
        for (size_t i = start; i < start+count; ++i) {
          generated_core.apply_matrix(i, mat1);
        }
      });
      utils::async_wait(ctx->pool);

      utils::submit_works_async(ctx->pool, generated_core.tiles.size(), [&generated_core, map] (const size_t &start, const size_t &count) {
        for (size_t i = start; i < start+count; ++i) {
          map->set_tile_data(&generated_core.tiles[i], i);
        }
      });
      utils::async_wait(ctx->pool);

      utils::submit_works_async(ctx->pool, generated_core.points.size(), [&generated_core, map] (const size_t &start, const size_t &count) {
        for (size_t i = start; i < start+count; ++i) {
          map->set_point_data(generated_core.points[i], i);
        }
      });
      utils::async_wait(ctx->pool);

      const size_t tri_count = core::map::tri_count_d(core::map::accel_struct_detail_level);
      ASSERT(tri_count == map->accel_triangles_count());
//     const size_t hex_count = map::hex_count_d(detail_level);
      std::mutex mutex;
      std::unordered_set<uint32_t> unique_tiles;
      std::atomic<uint32_t> tiles_counter(0);

      utils::submit_works_async(ctx->pool, tri_count, [&mutex, &unique_tiles, &generated_core, &tiles_counter, map] (const size_t &start, const size_t &count) {
        std::vector<uint32_t> tiles_array;
        size_t offset = 0;
        for (size_t i = 0; i < core::map::accel_struct_detail_level; ++i) {
          offset += core::map::tri_count_d(i);
        }

        for (size_t i = start; i < start+count; ++i) {
          const size_t tri_index = i + offset;
          const auto &tri = generated_core.triangles[tri_index];
          ASSERT(tri.current_level == core::map::accel_struct_detail_level);

          map_triangle_add2(&generated_core, tri_index, mutex, unique_tiles, tiles_array);

          uint32_t counter = 0;
          for (int64_t i = tiles_array.size()-1; i > -1 ; --i) {
            const uint32_t tile_index = tiles_array[i];
            if (generated_core.tiles[tile_index].is_pentagon()) {
              ++counter;
              ASSERT(counter < 2);
              std::swap(tiles_array[i], tiles_array.back());
            }
          }

          const uint32_t offset = tiles_counter.fetch_add(tiles_array.size());
          ASSERT(offset + tiles_array.size() <= generated_core.tiles.size());
          map->set_tile_indices(i, tri.points, tiles_array, offset, tiles_array.size(), counter != 0);

          tiles_array.clear();
        }
      });
      utils::async_wait(ctx->pool); // похоже что работает

      ASSERT(ctx->pool->working_count() == 1 && ctx->pool->tasks_count() == 0);

      ASSERT(generated_core.triangles.size() == map->triangles.size());
      static_assert(sizeof(core::map::triangle) == sizeof(map::triangle));
      {
        std::unique_lock<std::mutex> lock(map->mutex);
        memcpy(map->triangles.data(), generated_core.triangles.data(), map->triangles.size()*sizeof(core::map::triangle));
      }

      map->flush_data();

//       ctx->container->set_entity_count(debug::entities::tile, map->tiles_count());
      ctx->map->set_status(core::map::status::valid);
    }

    void setup_generator(generator::context* ctx, sol::table &table) {
      (void)table;
      ctx->container->set_tile_template({
        map::generator::data_type::uint_t,    //       plate_index,
        map::generator::data_type::uint_t,    //       edge_index,
        map::generator::data_type::float_t,   //       edge_dist,
        map::generator::data_type::uint_t,    //       mountain_index,
        map::generator::data_type::float_t,   //       mountain_dist,
        map::generator::data_type::uint_t,    //       ocean_index,
        map::generator::data_type::float_t,   //       ocean_dist,
        map::generator::data_type::uint_t,    //       coastline_index,
        map::generator::data_type::float_t,   //       coastline_dist,
        map::generator::data_type::float_t,   //       elevation,
        map::generator::data_type::float_t,   //       heat,
        map::generator::data_type::float_t,   //       moisture,
        map::generator::data_type::uint_t,    //       biome,
        map::generator::data_type::uint_t,    //       province_index,
        map::generator::data_type::uint_t,    //       culture_id,
        map::generator::data_type::uint_t,    //       country_index
        map::generator::data_type::uint_t,    //       test_value_uint1
        map::generator::data_type::uint_t,    //       test_value_uint2
        map::generator::data_type::uint_t,    //       test_value_uint3
      });

      {
        const size_t index = ctx->container->set_entity_template({
          map::generator::data_type::float_t,  // drift_axis,
          map::generator::data_type::float_t,  // drift_axis1,
          map::generator::data_type::float_t,  // drift_axis2,
          map::generator::data_type::float_t,  // drift_rate,
          map::generator::data_type::float_t,  // spin_axis,
          map::generator::data_type::float_t,  // spin_axis1,
          map::generator::data_type::float_t,  // spin_axis2,
          map::generator::data_type::float_t,  // spin_rate,
          map::generator::data_type::float_t,  // base_elevation,
          map::generator::data_type::uint_t    // oceanic
        });

        ASSERT(index == debug::entities::plate);
        UNUSED_VARIABLE(index);
      }

      {
        const size_t index = ctx->container->set_entity_template({
          map::generator::data_type::float_t,  // plate0_movement,
          map::generator::data_type::float_t,  // plate0_movement1,
          map::generator::data_type::float_t,  // plate0_movement2,
          map::generator::data_type::float_t,  // plate1_movement,
          map::generator::data_type::float_t,  // plate1_movement1,
          map::generator::data_type::float_t,  // plate1_movement2,
        });

        ASSERT(index == debug::entities::edge);
        UNUSED_VARIABLE(index);
      }

      {
        const size_t index = ctx->container->set_entity_template({
          map::generator::data_type::uint_t,  // country_index,
          map::generator::data_type::uint_t   // title_index
        });

        ASSERT(index == debug::entities::province);
        UNUSED_VARIABLE(index);
      }

      {
        const size_t index = ctx->container->set_entity_template({});
        ASSERT(index == debug::entities::culture);
        UNUSED_VARIABLE(index);
      }

      {
        const size_t index = ctx->container->set_entity_template({});
        ASSERT(index == debug::entities::country);
        UNUSED_VARIABLE(index);
      }

      {
        const size_t index = ctx->container->set_entity_template({
          map::generator::data_type::uint_t,  // parent
          map::generator::data_type::uint_t,  // owner
        });

        ASSERT(index == debug::entities::title);
        UNUSED_VARIABLE(index);
      }
    }

    void generate_plates(generator::context* ctx, sol::table &table) {
      utils::time_log log("generate plates");

      // скорее всего это нужно переписать в однопоток

      //auto data = ctx->data;
      auto map = ctx->map;

      const uint32_t plates_count = table["userdata"]["plates_count"];

//       ASSERT(plates_count > 3);
//       ASSERT(plates_count < 200);

      const uint32_t tiles_count = map->tiles_count();
      //std::atomic<uint32_t> tile_plate_atomic[tiles_count];
      std::vector<uint32_t> tile_plate_atomic(tiles_count, UINT32_MAX);
      //utils::concurrent_vector<uint32_t> plate_tiles_concurrent[plates_count];
      std::vector<std::vector<uint32_t>> plate_tiles_concurrent(plates_count);

//       utils::concurrent_vector<std::pair<uint32_t, uint32_t>> active_tile_indices;
//       utils::concurrent_vector<std::pair<uint32_t, uint32_t>>::default_value = std::make_pair(0, UINT32_MAX);
      std::vector<std::pair<uint32_t, uint32_t>> active_tile_indices;

      // for (size_t i = 0; i < tiles_count; ++i) {
      //   tile_plate_atomic[i] = UINT32_MAX;
      // }

      std::unordered_set<uint32_t> unique_tiles;
      auto rand = ctx->random;
      for (size_t i = 0; i < plates_count; ++i) {
        uint32_t attempts = 0;
        bool success = false;
        do {
          ++attempts;
          const uint32_t rand_index = rand->index(map->tiles_count());
          assert(rand_index < map->tiles_count());
          if (tile_plate_atomic[rand_index] != UINT32_MAX) continue;
          const auto &data = render::unpack_data(ctx->map->get_tile(rand_index));
          const uint32_t n_count = render::is_pentagon(data) ? 5 : 6;
          bool continue_b = false;
          for (uint32_t j = 0; j < n_count; ++j) {
            const uint32_t n_index = data.neighbours[j];
            assert(n_index < map->tiles_count());
            if (tile_plate_atomic[n_index] != UINT32_MAX) continue_b = true;
          }

          if (continue_b) continue;

          success = true;
          unique_tiles.insert(rand_index);
          tile_plate_atomic[rand_index] = i;
          plate_tiles_concurrent[i].push_back(rand_index);
          for (uint32_t j = 0; j < n_count; ++j) {
            const uint32_t n_index = data.neighbours[j];
//             unique_tiles.insert(n_index);
//             tile_plate_atomic[n_index] = i;
//             plate_tiles_concurrent[i].push_back(n_index);
            active_tile_indices.push_back(std::make_pair(n_index, i));
          }
        } while (!success && attempts < 100);
      }

//       std::atomic<uint32_t>* tile_plate_atomic_ptr = tile_plate_atomic;
//       utils::concurrent_vector<uint32_t>* plate_tiles_concurrent_ptr = plate_tiles_concurrent;
//       uint32_t* tile_plate_atomic_ptr = tile_plate_atomic;
//       std::vector<uint32_t>* plate_tiles_concurrent_ptr = plate_tiles_concurrent;
//       utils::submit_works(pool, data->plates_count, [] (
//         const size_t &start,
//         const size_t &count,
//         const core::map* map,
//         const uint32_t &plates_count,
//         utils::random_engine* rand,
//         std::atomic<uint32_t>* tile_plate_atomic,
//         utils::concurrent_vector<uint32_t>* plate_tiles_concurrent,
//         utils::concurrent_vector<std::pair<uint32_t, uint32_t>> &active_tile_indices,
//         std::atomic<size_t> &state
//       ) {
//       size_t start = 0;
//       size_t count = plates_count;
//       //const uint32_t &plates_count = plates_count;
//         for (size_t i = start; i < start+count; ++i) {
//           uint32_t attempts = 0;
//           bool success = false;
//           do {
//             success = push_new_tile_index(
//               map,
//               plates_count,
//               i,
//               rand->index(map->tiles_count()),
//               tile_plate_atomic,
//               plate_tiles_concurrent,
//               active_tile_indices
//             );
//           } while (!success && attempts < 100);
//
//           //++state;
//           ASSERT(success);
//         }
//       }, map, data->plates_count, data->engine, tile_plate_atomic_ptr, plate_tiles_concurrent_ptr, std::ref(active_tile_indices), std::ref(state));

      std::atomic<size_t> counter(plates_count);
      while (!active_tile_indices.empty()) {
        const uint32_t random_index = rand->index(active_tile_indices.size());
        const auto pair = active_tile_indices[random_index];
        active_tile_indices[random_index] = active_tile_indices.back();
        active_tile_indices.pop_back();

        const uint32_t tile_index = pair.first;
        const uint32_t plate_index = pair.second;
//         ASSERT(tile_plate_atomic[tile_index] == plate_index);

        if (tile_plate_atomic[tile_index] != UINT32_MAX) continue;
        //ASSERT(tile_plate_atomic[tile_index] == UINT32_MAX);
        tile_plate_atomic[tile_index] = plate_index;
        plate_tiles_concurrent[plate_index].push_back(tile_index);
        unique_tiles.insert(tile_index);

        const auto &data = render::unpack_data(ctx->map->get_tile(tile_index));
        const uint32_t n_count = render::is_pentagon(data) ? 5 : 6;
        for (uint32_t j = 0; j < n_count; ++j) {
          const uint32_t n_index = data.neighbours[j];
          if (unique_tiles.find(n_index) != unique_tiles.end()) continue;

          active_tile_indices.push_back(std::make_pair(n_index, plate_index));
        }
      }

//       utils::submit_works(pool, 8, [] (
//         const size_t &start,
//         const size_t &count,
//         const core::map* map,
//         const uint32_t &plates_count,
//         utils::random_engine* rand,
//         std::atomic<uint32_t>* tile_plate_atomic,
//         utils::concurrent_vector<uint32_t>* plate_tiles_concurrent,
//         utils::concurrent_vector<std::pair<uint32_t, uint32_t>> &active_tile_indices,
//         std::atomic<size_t> &counter,
//         std::atomic<size_t> &state
//       ) {
//         (void)start;
//         (void)count;

//         while (true) {
//           //const auto pair = active_tile_indices.get_random(*rand);
//           if (active_tile_indices.empty()) break;
//           const uint32_t random_index = rand->index(active_tile_indices.size());
//           const auto pair = active_tile_indices[random_index];
//           active_tile_indices[random_index] = active_tile_indices.back();
//           active_tile_indices.pop_back();
//
//           const uint32_t tile_index = pair.first;
//           const uint32_t plate_index = pair.second;
//
//           //if (plate_index == UINT32_MAX) return;
//           if (plate_index == UINT32_MAX) break;
//
// //           if (tile_index == UINT32_MAX) {
// //             active_tile_indices.push_back(std::make_pair(UINT32_MAX, 0));
// //             return;
// //           }
//           if (tile_index == UINT32_MAX) {
//             active_tile_indices.push_back(std::make_pair(UINT32_MAX, 0));
//             break;
//           }
//
//           //uint32_t data = plates_count;
//           //if (!tile_plate_atomic[tile_index].compare_exchange_strong(data, plate_index)) continue;
//           if (tile_plate_atomic[tile_index] != plates_count) continue;
//           ASSERT(plate_index < plates_count);
//           plate_tiles_concurrent[plate_index].push_back(tile_index);
//
//           counter.fetch_add(1);
//
//           // {
//             // std::unique_lock<std::mutex> lock(plate_tile_indices[plate_index].mutex);
//             // plate_tile_indices[plate_index].vector.push_back(tile_index);
//           // }
//
//           const auto &tile = render::unpack_data(map->get_tile(tile_index));
//           for (uint32_t i = 0; i < 6; ++i) {
//             const uint32_t neighbor_index = tile.neighbours[i];
//             if (neighbor_index == UINT32_MAX) continue;
//             if (tile_plate_atomic[neighbor_index] != plates_count) continue;
//             active_tile_indices.push_back(std::make_pair(neighbor_index, plate_index));
//           }
//         }
//       }, map, data->plates_count, data->engine, tile_plate_atomic_ptr, plate_tiles_concurrent_ptr, std::ref(active_tile_indices), std::ref(counter), std::ref(state));

      // странно но это работает
      std::cout << "tile_count " << tiles_count << "\n";
      std::cout << "counter    " << (counter + plates_count) << "\n";

      size_t abc = 0;
      for (size_t i = 0; i < plates_count; ++i) {
        const size_t count = plate_tiles_concurrent[i].size();
        //std::cout << "plate " << i << " count " << count << "\n";
        abc += count;
        // for (size_t j = 0; j < count; ++j) {
        //   std::cout << plate_tile_indices[i].vector[j] << " ";
        // }
        // std::cout << "\n";
      }
      std::cout << "tile_count " << tiles_count << "\n";
      std::cout << "counter    " << abc << '\n';

      // вот так я добавляю данные в контейнер
//       //context->tile_plate_indices.resize(tiles_count);
//       for (size_t i = 0; i < tiles_count; ++i) {
//         //context->tile_plate_indices[i] = tile_plate_atomic[i];
//         ctx->container->set_data(tile_entity, i, plate_index_property, tile_plate_atomic[i]);
//       }
//
//       ctx->container->set_entity_count(plate_entity, plates_count);
//       for (size_t i = 0; i < plates_count; ++i) {
//         //context->plate_tile_indices[i].resize(plate_tiles_concurrent[i].vector.size());
//         for (size_t j = 0; j < plate_tiles_concurrent[i].size(); ++j) {
//           //context->plate_tile_indices[i][j] = plate_tiles_concurrent[i].vector[j];
//           ctx->container->add_child(plate_entity, i, plate_tiles_concurrent[i][j]);
//         }
//       }
//
//       // это нужно организовать как функцию переключения вида карты
//       for (uint32_t i = 0; i < ctx->map->tiles_count(); ++i) {
//         ctx->map->set_tile_tectonic_plate(i, ctx->container->get_data<uint32_t>(tile_entity, i, plate_index_property));
//       }


      // нужно случайно соединить несколько плит друг с другом
      // несколько итераций, на каждой итерации для каждой плиты
      // смотрим нужно ли ей выдать еще плиту если да то выбиваем случайного соседа,
      // должно остаться несколько плит

      const uint32_t min_plates_count = table["userdata"]["plates_connection_limit"];
      const uint32_t max_iterations = table["userdata"]["plates_connection_iteration"];
      uint32_t current_plates_count = plates_count;
      uint32_t current_iter = 0;

      struct next_plates_data {
        std::mutex mutex;
        //std::unordered_set<uint32_t> neighbours; // плох тем, что не гарантирует порядок значений
        std::set<uint32_t> neighbours;             // гарантирует порядок, а значит независим от мультипоточных алгоритмов
      };

      std::vector<std::vector<uint32_t>> plate_tiles_local(plates_count);
      for (size_t i = 0; i < plates_count; ++i) {
        std::swap(plate_tiles_local[i], plate_tiles_concurrent[i]);
      }

      std::vector<uint32_t> tile_plates_local(tile_plate_atomic.size());
      memcpy(tile_plates_local.data(), tile_plate_atomic.data(), tile_plate_atomic.size() * sizeof(tile_plate_atomic[0]));
//       std::vector<uint32_t> plate_new_plate(context->plate_tile_indices.size(), UINT32_MAX);
//       for (size_t i = 0; i < context->plate_tile_indices.size(); ++i) {
//         plate_new_plate[i] = i;
//       }

      while (current_plates_count > min_plates_count && current_iter < max_iterations) {
//         utils::random_engine_st local_random;
//         rand = &local_random;
        std::vector<next_plates_data> next_plates(plates_count);
        utils::submit_works_async(ctx->pool, tiles_count, [&next_plates, &tile_plates_local] (const size_t &start, const size_t &count, const generator::context* ctx) {
          for (size_t i = start; i < start+count; ++i) {
            const auto &tile = render::unpack_data(ctx->map->get_tile(i));
            for (uint32_t j = 0; j < 6; ++j) {
              const uint32_t tile_neighbour_index = tile.neighbours[j];
              if (tile_neighbour_index == UINT32_MAX) continue;

              const uint32_t plate1 = tile_plates_local[i];
              const uint32_t plate2 = tile_plates_local[tile_neighbour_index];

              if (plate1 != plate2) {
                {
                  std::unique_lock<std::mutex> lock(next_plates[plate1].mutex);
                  next_plates[plate1].neighbours.insert(plate2);
                }

                {
                  std::unique_lock<std::mutex> lock(next_plates[plate2].mutex);
                  next_plates[plate2].neighbours.insert(plate1);
                }
              }
            }
          }
        }, ctx);
        utils::async_wait(ctx->pool);
        //ctx->pool->compute();
        //while (ctx->pool->working_count() != 1 || ctx->pool->tasks_count() != 0) { std::this_thread::sleep_for(std::chrono::microseconds(1)); }

        //ASSERT(false);

        uint32_t max_tiles_count = 0;
        uint32_t min_tiles_count = 121523152;
        for (uint32_t i = 0; i < plate_tiles_local.size(); ++i) {
          const uint32_t tiles_count = plate_tiles_local[i].size();
          if (tiles_count < 2) continue;
          //const uint32_t neighbours_count = next_plates[i].neighbours.size();

          max_tiles_count = std::max(max_tiles_count, tiles_count);
          min_tiles_count = std::min(min_tiles_count, tiles_count);
        }

        // ниже код определяет какие плиты соединятся друг с другом на текущей итерации
        std::vector<bool> plates_union(plates_count, false);
        for (uint32_t i = 0; i < plate_tiles_local.size(); ++i) {
          const uint32_t tiles_count = plate_tiles_local[i].size();
          if (tiles_count < 2) continue;

          // я хочу чтобы как можно больше плит соединились на полюсах
          // для этого я беру небольшой коэффициент на основе y компоненты позиции корневого тайла
          const uint32_t root_tile_index = plate_tiles_local[i][0];
          const auto &tile = render::unpack_data(ctx->map->get_tile(root_tile_index));
          const glm::vec4 root_pos = ctx->map->get_point(tile.center);
          const glm::vec4 norm = glm::normalize(root_pos * glm::vec4(1.0f, 1.0f, 1.0f, 0.0f));
          const float poles_k = glm::mix(0.2f, 0.8f, glm::abs(norm.y));
          ASSERT(poles_k >= 0.2f && poles_k <= 0.8f);

          // и так же я хочу чтобы большие плиты не продолжали бесконечно соединятся
          const float k = (float(tiles_count) - float(min_tiles_count)) / (float(max_tiles_count) - float(min_tiles_count));
          ASSERT(k <= 1.0f && k >= 0.0f);
          const float inv_k = 1.0f - k;
          const float sum_inv_k = 0.6f;
          const float sum = inv_k * sum_inv_k + poles_k * (1.0f - sum_inv_k);
          const float final_k = glm::mix(0.1f, 0.8f, sum);
          ASSERT(final_k >= 0.1f && final_k <= 0.8f);

          const bool need_unite = rand->probability(final_k);
          //if (!need_unite) continue;
          plates_union[i] = need_unite;
        }

        // по идее при обходе мы почти все плиты которые нужно соединяем
        for (uint32_t i = 0; i < plate_tiles_local.size(); ++i) {
          if (plate_tiles_local[i].size() < 2) continue;
          if (!plates_union[i]) continue;

          uint32_t plate_index = UINT32_MAX;
          {
//             std::vector<uint32_t> neighbours_vector(next_plates[i].neighbours.begin(), next_plates[i].neighbours.end());
//             while (plate_index == UINT32_MAX) {
// //               ASSERT(!neighbours_vector.empty());
//               const uint32_t rand_index = rand->index(neighbours_vector.size());
//               const uint32_t index = neighbours_vector[rand_index];
//               neighbours_vector[rand_index] = neighbours_vector.back();
//               neighbours_vector.pop_back();
// 
//               if (plates_union[index]) plate_index = index;
//               if (neighbours_vector.empty()) break;
//             }

            for (auto idx : next_plates[i].neighbours) {
              if (!plates_union[idx]) continue;
              // первого соседа?
              plate_index = idx;
              break;
            }
          }

          if (plate_index == UINT32_MAX) continue;
          //ASSERT(plate_tiles_local[plate_index].size() > 1);

          while (plate_tiles_local[plate_index].size() < 2) {
            plate_index = plate_tiles_local[plate_index][0];
          }

          plate_tiles_local[i].insert(plate_tiles_local[i].end(), plate_tiles_local[plate_index].begin(), plate_tiles_local[plate_index].end());
          plate_tiles_local[plate_index].clear();
          plate_tiles_local[plate_index].push_back(i);
          //plate_new_plate[plate_index] = i;
          --current_plates_count;
        }

        // обновляем индексы
        for (size_t i = 0; i < plate_tiles_local.size(); ++i) {
          if (plate_tiles_local[i].size() < 2) continue;

          for (size_t j = 0; j < plate_tiles_local[i].size(); ++j) {
            tile_plates_local[plate_tiles_local[i][j]] = i;
          }
        }

        ++current_iter;
      }

      uint32_t max_tiles_count = 0;
      uint32_t min_tiles_count = 121523152;
      uint32_t plates_counter = 0;
      for (uint32_t i = 0; i < plate_tiles_local.size(); ++i) {
        const uint32_t tiles_count = plate_tiles_local[i].size();
        if (tiles_count < 2) continue;
        //const uint32_t neighbours_count = next_plates[i].neighbours.size();

        max_tiles_count = std::max(max_tiles_count, tiles_count);
        min_tiles_count = std::min(min_tiles_count, tiles_count);
        ++plates_counter;
      }

      for (auto itr = plate_tiles_local.begin(); itr != plate_tiles_local.end();) {
        const uint32_t tiles_count = itr->size();
        if (tiles_count > 1) {
          ++itr;
          continue;
        }

        itr = plate_tiles_local.erase(itr);
      }

      // не могу получить индекс старых плит внутри суперплиты =(

      // так, что то не получается вернуть как было ранее
      // почему то то ли не соединяются, то ли неправильные индексы приходят
      uint32_t tiles_counter = 0;
      for (size_t i = 0; i < plate_tiles_local.size(); ++i) {
        ASSERT(plate_tiles_local[i].size() > 1);

        for (size_t j = 0; j < plate_tiles_local[i].size(); ++j) {
          const uint32_t tile_index = plate_tiles_local[i][j];
          tile_plates_local[tile_index] = i;
          ASSERT(tile_index < ctx->map->tiles_count());
          ++tiles_counter;
        }
      }

      ASSERT(tiles_counter == ctx->map->tiles_count());

      // вот так я добавляю данные в контейнер
      //context->tile_plate_indices.resize(tiles_count);
      for (size_t i = 0; i < tiles_count; ++i) {
        //context->tile_plate_indices[i] = tile_plate_atomic[i];
        ctx->container->set_data<uint32_t>(debug::entities::tile, i, debug::properties::tile::plate_index, tile_plates_local[i]);
      }

      table["final_plates_count"] = plate_tiles_local.size();
      ctx->container->set_entity_count(debug::entities::plate, plate_tiles_local.size());
      for (size_t i = 0; i < plate_tiles_local.size(); ++i) {
        //context->plate_tile_indices[i].resize(plate_tiles_concurrent[i].vector.size());
        for (size_t j = 0; j < plate_tiles_local[i].size(); ++j) {
          //context->plate_tile_indices[i][j] = plate_tiles_concurrent[i].vector[j];
          ctx->container->add_child(debug::entities::plate, i, plate_tiles_local[i][j]);
        }
      }

      // это нужно организовать как функцию переключения вида карты
//       for (uint32_t i = 0; i < ctx->map->tiles_count(); ++i) {
//         //ctx->map->set_tile_tectonic_plate(i, ctx->container->get_data<uint32_t>(debug::entities::tile, i, debug::properties::tile::plate_index));
//         const uint32_t index = ctx->container->get_data<uint32_t>(debug::entities::tile, i, debug::properties::tile::plate_index);
//         const uint32_t rand_num1 = render::lcg(index);
//         const uint32_t rand_num2 = render::lcg(rand_num1);
//         const uint32_t rand_num3 = render::lcg(rand_num2);
//         const float color_r = render::lcg_normalize(rand_num1);
//         const float color_g = render::lcg_normalize(rand_num2);
//         const float color_b = render::lcg_normalize(rand_num3);
//         ctx->map->set_tile_color(i, render::make_color(color_r, color_b, color_g, 1.0f));
//       }

//       std::swap(context->tile_plate_indices, tile_plates_local);
//       std::swap(context->plate_tile_indices, plate_tiles_local);

//       for (uint32_t i = 0; i < context->tile_plate_indices.size(); ++i) {
//         context->map->set_tile_tectonic_plate(i, context->tile_plate_indices[i]);
//       }

      PRINT_VAR("tectonic plates count", plates_counter)
      PRINT_VAR("max tiles count", max_tiles_count)
      PRINT_VAR("min tiles count", min_tiles_count)
    }

    void generate_plate_datas(generator::context* ctx, sol::table &table) {
      utils::time_log log("plate datas generator");

      uint32_t max_tiles_count = 0;
      uint32_t min_tiles_count = 1215152;
      const uint32_t plates_count = table["final_plates_count"];
      //const uint32_t plates_count = table["userdata"]["plates_count"];
      std::vector<uint32_t> local_plates_indices(plates_count);
      std::vector<tectonic_plate_data_t> tectonic_plate_props(plates_count);
      for (size_t i = 0; i < local_plates_indices.size(); ++i) {
        local_plates_indices[i] = i;
        const uint32_t tiles_count = ctx->container->get_childs(debug::entities::plate, i).size();
        max_tiles_count = std::max(max_tiles_count, tiles_count);
        min_tiles_count = std::min(min_tiles_count, tiles_count);
      }

      ASSERT(min_tiles_count > 1);
//
// //       std::sort(local_plates_indices.begin(), local_plates_indices.end(), [context] (const uint32_t &first, const uint32_t &second) -> bool {
// //         return context->plate_tile_indices[first].size() > context->plate_tile_indices[second].size();
// //       });
//
//       std::vector<bool> new_plate_oceanic(plate_tiles_local.size());
//       for (size_t i = 0; i < plate_tiles_local.size(); ++i) {
//         // взятие случайных плит: результат выгялит гораздо лучше
//         const uint32_t rand_plate_index = rand->index(local_plates_indices.size());
//         const uint32_t plate_index = local_plates_indices[rand_plate_index];
//         local_plates_indices[rand_plate_index] = local_plates_indices.back();
//         local_plates_indices.pop_back();
//
//         const auto &tile_indices = plate_tiles_local[plate_index];
//         ASSERT(tile_indices.size() > 1);
//
//         const float k = (float(tile_indices.size()) - float(min_tiles_count)) / (float(max_tiles_count) - float(min_tiles_count));
//         ASSERT(k <= 1.0f && k >= 0.0f);
//         const float inv_k = 1.0f - k*k; // обратный квадратичный коэффициент
//         const float final_k = glm::mix(0.2f, 0.8f, inv_k);
//
//         //const bool oceanic = rand->probability(ocean_percentage);
//         bool oceanic = rand->probability(final_k);
//         if (oceanic && oceanic_tiles_count + tile_indices.size() < oceanic_tiles) {
//           oceanic_tiles_count += tile_indices.size();
//         } else if (oceanic && oceanic_tiles_count + tile_indices.size() > oceanic_tiles) {
//           oceanic = false;
//         }
//
//         new_plate_oceanic[i] = oceanic;
//       }

      const float ocean_percentage = table["userdata"]["ocean_percentage"];
      const uint32_t oceanic_tiles = ocean_percentage * ctx->map->tiles_count();
      uint32_t oceanic_tiles_count = 0;
//       const uint32_t ground_tiles = context->map->tiles_count() - oceanic_tiles;

//       auto data = context->data;
      auto map = ctx->map;
      auto rand = ctx->random;
      //context->plate_datas.resize(plates_count);

      // эти вещи по идее можно вынести в данные генерации
      const float min_drift_rate = -PI / 30.0;
      const float max_drift_rate =  PI / 30.0;
      const float min_spin_rate  = -PI / 30.0;
      const float max_spin_rate  =  PI / 30.0;
//       const float rate_k = 2.0f;
//       const float min_drift_rate = -rate_k;
//       const float max_drift_rate =  rate_k;
//       const float min_spin_rate  = -rate_k;
//       const float max_spin_rate  =  rate_k;

      const float min_oceanic_elevation = -0.8f;
      const float max_oceanic_elevation = -0.3f;
      const float min_continental_elevation = 0.15f;
      const float max_continental_elevation = 0.5f;
      ASSERT(ctx->container->entities_count(debug::entities::plate) != 0);

      for (size_t i = 0; i < plates_count; ++i) {
        // взятие случайных плит: результат выгялит гораздо лучше
        const uint32_t rand_plate_index = rand->index(local_plates_indices.size());
        const uint32_t plate_index = local_plates_indices[rand_plate_index];
        local_plates_indices[rand_plate_index] = local_plates_indices.back();
        local_plates_indices.pop_back();

        ASSERT(plate_index < plates_count);

        const auto &tile_indices = ctx->container->get_childs(debug::entities::plate, plate_index);
        ASSERT(tile_indices.size() > 1);

        const float k = (float(tile_indices.size()) - float(min_tiles_count)) / (float(max_tiles_count) - float(min_tiles_count));
        ASSERT(k <= 1.0f && k >= 0.0f);
        const float inv_k = k;
        const float final_k = glm::mix(0.3f, 0.95f, inv_k);

        //const bool oceanic = rand->probability(ocean_percentage);
        bool oceanic = rand->probability(final_k);
        if (oceanic && oceanic_tiles_count + tile_indices.size() <= oceanic_tiles) {
          oceanic_tiles_count += tile_indices.size();
        } else if (oceanic && oceanic_tiles_count + tile_indices.size() > oceanic_tiles) {
          oceanic = false;
        }

//         const uint32_t plate_index = i;
//         const bool oceanic = new_plate_oceanic[plate_new_plate[i]];

//         const auto &tile_indices = plate_tile_indices[plate_index];
//         ASSERT(tile_indices.size() > 1);

        const glm::vec3 drift_axis = glm::normalize(rand->unit3());
        const float drift_rate = rand->closed(min_drift_rate, max_drift_rate);
        // тут берем случайный тайл на плите, но вообще можно любой случайный тайл брать
        const uint32_t idx = rand->index(tile_indices.size());
        ASSERT(idx < tile_indices.size());
        const uint32_t rand_index = tile_indices[idx];
        const auto &tile = map->get_tile(rand_index);
        const glm::vec3 spin_axis = glm::normalize(glm::vec3(map->get_point(tile.tile_indices.x)));
        const float spin_rate = rand->closed(min_spin_rate, max_spin_rate);
        const float base_elevation = oceanic ? rand->closed(min_oceanic_elevation, max_oceanic_elevation) :
                                                rand->closed(min_continental_elevation, max_continental_elevation);

        const tectonic_plate_data_t plate{
          drift_axis,
          drift_rate,
          spin_axis,
          spin_rate,
          base_elevation,
          oceanic
        };
        tectonic_plate_props[plate_index] = plate;
      }

      //std::swap(context->plate_datas, tectonic_plate_props);
      for (size_t i = 0; i < tectonic_plate_props.size(); ++i) {
        ctx->container->set_data<glm::vec3>(debug::entities::plate, i, debug::properties::plate::drift_axis,     tectonic_plate_props[i].drift_axis);
        ctx->container->set_data<float>    (debug::entities::plate, i, debug::properties::plate::drift_rate,     tectonic_plate_props[i].drift_rate);
        ctx->container->set_data<glm::vec3>(debug::entities::plate, i, debug::properties::plate::spin_axis,      tectonic_plate_props[i].spin_axis);
        ctx->container->set_data<float>    (debug::entities::plate, i, debug::properties::plate::spin_rate,      tectonic_plate_props[i].spin_rate);
        ctx->container->set_data<float>    (debug::entities::plate, i, debug::properties::plate::base_elevation, tectonic_plate_props[i].base_elevation);
        ctx->container->set_data<uint32_t> (debug::entities::plate, i, debug::properties::plate::oceanic, uint32_t(tectonic_plate_props[i].oceanic));
      }

      PRINT_VAR("oceanic tiles count", oceanic_tiles_count)
      PRINT_VAR("ground  tiles count", ctx->map->tiles_count() - oceanic_tiles_count)

      uint32_t water_counter = 0;
      for (uint32_t i = 0; i < ctx->map->tiles_count(); ++i) {
        const uint32_t plate_index = ctx->container->get_data<uint32_t>(debug::entities::tile, i, debug::properties::tile::plate_index);
        ASSERT(plate_index < ctx->container->entities_count(debug::entities::plate));
        const bool oceanic = bool(ctx->container->get_data<uint32_t>(debug::entities::plate, plate_index, debug::properties::plate::oceanic));
        const float height = ctx->container->get_data<float>(debug::entities::plate, plate_index, debug::properties::plate::base_elevation);
        ASSERT(plate_index < plates_count);
//         ctx->map->set_tile_tectonic_plate(i, oceanic);
        ctx->map->set_tile_height(i, height);
//         ctx->map->set_tile_biom(i, plate_index);
        const uint32_t rand_num1 = render::prng(plate_index);
        const uint32_t rand_num2 = render::prng(rand_num1);
        const uint32_t rand_num3 = render::prng(rand_num2);
        const float color_r = render::prng_normalize(rand_num1);
        const float color_g = render::prng_normalize(rand_num2);
        const float color_b = render::prng_normalize(rand_num3);
        ctx->map->set_tile_color(i, render::make_color(color_r, color_b, color_g, 1.0f));
        water_counter += uint32_t(oceanic);
      }

      PRINT_VAR("oceanic tiles after recompute", water_counter)
      PRINT_VAR("oceanic tiles k              ", float(water_counter) / float(ctx->map->tiles_count()))

      // для сида 1 мне удалось сгенерировать неплохой земной массив
      // со внутренним озером, несколькими континентами, выглядит неплохо даже без дальнейшей генерации
      // другое дело что нужно будет придумать как сделать горные массивы внутри тектонической плиты
    }

    void compute_boundary_edges(generator::context* ctx, sol::table &table) {
      utils::time_log log("compute boundary edges");
      std::set<size_t> pairs_set;
      std::mutex mutex;

      const uint32_t plates_count = table["final_plates_count"];
      //const uint32_t plates_count = table["userdata"]["plates_count"];

      utils::submit_works_async(ctx->pool, ctx->map->tiles_count(), [&pairs_set, &mutex, plates_count] (const size_t &start, const size_t &count, const generator::context* context) {
        for (size_t i = start; i < start+count; ++i) {
          const uint32_t current_tile_index = i;
          const uint32_t current_plate_index = context->container->get_data<uint32_t>(debug::entities::tile, current_tile_index, debug::properties::tile::plate_index);
          ASSERT(current_plate_index < plates_count);
          const auto &tile = render::unpack_data(context->map->get_tile(current_tile_index));
          const uint32_t n_count = render::is_pentagon(tile) ? 5 : 6;
          for (uint32_t j = 0; j < n_count; ++j) {
            const uint32_t neighbour_tile_index = tile.neighbours[j];
            ASSERT(current_tile_index != neighbour_tile_index);

            const uint32_t neighbour_tile_plate_index = context->container->get_data<uint32_t>(debug::entities::tile, neighbour_tile_index, debug::properties::tile::plate_index);
            ASSERT(neighbour_tile_plate_index < plates_count);
            if (current_plate_index != neighbour_tile_plate_index) {
              const uint32_t min = std::min(current_tile_index, neighbour_tile_index);
              const uint32_t max = std::max(current_tile_index, neighbour_tile_index);
              const size_t key = (size_t(min) << 32) | size_t(max);
              std::unique_lock<std::mutex> lock(mutex);
              pairs_set.insert(key);
            }
          }
        }
      }, ctx);
      ctx->pool->compute();
      while (ctx->pool->working_count() != 1 || ctx->pool->tasks_count() != 0) { std::this_thread::sleep_for(std::chrono::microseconds(1)); }

      PRINT_VAR("edges count", pairs_set.size())

      ctx->container->set_entity_count(debug::entities::edge, pairs_set.size());
      auto itr = pairs_set.begin();
      for (uint32_t i = 0; i < pairs_set.size(); ++i) {
        const size_t key = *itr;
        const uint32_t first  = uint32_t(key & UINT32_MAX);
        const uint32_t second = uint32_t((key >> 32) & UINT32_MAX);

        ctx->container->add_child(debug::entities::edge, i, first);
        ctx->container->add_child(debug::entities::edge, i, second);

        ++itr;
      }
    }

    glm::vec4 project(const glm::vec4 &vector, const glm::vec4 &normal) {
      const float num = glm::dot(normal, normal);
      if (num < EPSILON) {
        return glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
      }

      const float num2 = glm::dot(vector, normal);
      return normal * (num2 / num);
    }

    glm::vec4 calculate_axial_rotation(const glm::vec3 axis, const float &rate, const glm::vec4 &pos) {
//       const glm::vec3 dir1 = glm::vec3(pos) - axis;
      const glm::vec4 dir = glm::vec4(glm::cross(axis, glm::vec3(pos)), 0.0f);
//       const glm::vec4 dir = glm::normalize(glm::vec4(glm::cross(axis, dir1), 0.0f));
//       return dir * rate;
      const float length = glm::length(dir);
      if (glm::abs(length) < EPSILON) return glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
//
      const float position_axis_distance = glm::length(pos - project(pos, glm::vec4(axis, 0.0f)));
      return (dir / length) * (rate * position_axis_distance);
    }

    void compute_plate_boundary_stress(generator::context* ctx, sol::table &table) {
      utils::time_log log("compute plate boundary stresses");
      const size_t size = ctx->container->entities_count(debug::entities::edge);
      std::vector<boundary_stress_t> boundary_stresses(size);
      const uint32_t plates_count = table["final_plates_count"];
      utils::submit_works_async(ctx->pool, size, [&boundary_stresses, plates_count] (const size_t &start, const size_t &count, const generator::context* context) {
        for (size_t i = start; i < start+count; ++i) {
          const auto &childs = context->container->get_childs(debug::entities::edge, i);
          ASSERT(childs.size() == 2);
          const uint32_t first_tile  = childs.front();
          const uint32_t second_tile = childs.back();
          const uint32_t first_plate  = context->container->get_data<uint32_t>(debug::entities::tile, first_tile, debug::properties::tile::plate_index);
          const uint32_t second_plate = context->container->get_data<uint32_t>(debug::entities::tile, second_tile, debug::properties::tile::plate_index);
          ASSERT(first_tile < context->map->tiles_count());
          ASSERT(second_tile < context->map->tiles_count());
          ASSERT(first_plate < plates_count);
          ASSERT(second_plate < plates_count);
          ASSERT(first_plate != second_plate);
          const auto drift_axis1 = context->container->get_data<glm::vec3>(debug::entities::plate, first_plate, debug::properties::plate::drift_axis);
          const auto drift_axis2 = context->container->get_data<glm::vec3>(debug::entities::plate, second_plate, debug::properties::plate::drift_axis);
          const auto spin_axis1 = context->container->get_data<glm::vec3>(debug::entities::plate, first_plate, debug::properties::plate::spin_axis);
          const auto spin_axis2 = context->container->get_data<glm::vec3>(debug::entities::plate, second_plate, debug::properties::plate::spin_axis);
          const float drift_rate1 = context->container->get_data<float>(debug::entities::plate, first_plate, debug::properties::plate::drift_rate);
          const float drift_rate2 = context->container->get_data<float>(debug::entities::plate, second_plate, debug::properties::plate::drift_rate);
          const float spin_rate1 = context->container->get_data<float>(debug::entities::plate, first_plate, debug::properties::plate::spin_rate);
          const float spin_rate2 = context->container->get_data<float>(debug::entities::plate, second_plate, debug::properties::plate::spin_rate);

          const auto &first_tile_data = context->map->get_tile(first_tile);
          const auto &second_tile_data = context->map->get_tile(second_tile);
          const glm::vec4 first_tile_pos  = context->map->get_point(first_tile_data.tile_indices[0]);
          const glm::vec4 second_tile_pos = context->map->get_point(second_tile_data.tile_indices[0]);

          const glm::vec4 boundary_position1 = (first_tile_pos + second_tile_pos) / 2.0f;
          const glm::vec4 boundary_position = glm::normalize(boundary_position1 * glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)) + glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
          //const glm::vec4 boundary_normal = second_tile_pos - first_tile_pos;
          const glm::vec4 boundary_normal = glm::normalize(second_tile_pos - first_tile_pos);
          const glm::vec4 boundary_vector = glm::vec4(glm::normalize(glm::cross(glm::vec3(boundary_normal), glm::vec3(boundary_position))), 0.0f);

          const glm::vec4 plate_movement0 = calculate_axial_rotation(drift_axis1, drift_rate1, boundary_position) + calculate_axial_rotation(spin_axis1, spin_rate1, boundary_position);
          const glm::vec4 plate_movement1 = calculate_axial_rotation(drift_axis2, drift_rate2, boundary_position) + calculate_axial_rotation(spin_axis2, spin_rate2, boundary_position);
//           const glm::vec4 plate_movement0 = glm::vec4(prop1.drift_axis, 0.0f) * prop1.drift_rate + calculate_axial_rotation(prop1.spin_axis, prop1.spin_rate, boundary_position);
//           const glm::vec4 plate_movement1 = glm::vec4(prop2.drift_axis, 0.0f) * prop2.drift_rate + calculate_axial_rotation(prop2.spin_axis, prop2.spin_rate, boundary_position);

          const glm::vec4 relative_movement = plate_movement1 - plate_movement0;
          const glm::vec4 pressure_vector = project(relative_movement, boundary_normal);
          float pressure = glm::length(pressure_vector);
          pressure = glm::dot(pressure_vector, boundary_normal) < 0.0f ? -pressure : pressure;
          const glm::vec4 shear_vector = project(relative_movement, boundary_vector);
          const float shear = glm::length(shear_vector);

          const boundary_stress_t stress{
            2.0f / (1 + static_cast<float>(glm::exp(-pressure * 33.33))) - 1,
            2.0f / (1 + static_cast<float>(glm::exp(-shear * 33.33))) - 1,
            plate_movement0,
            plate_movement1
          };

//           const boundary_stress_t stress{
//             pressure,
//             shear,
//             plate_movement0, //glm::normalize(pressure_vector),
//             plate_movement1  //glm::normalize(shear_vector)
//           };

          boundary_stresses[i] = stress;
        }
      }, ctx);
      ctx->pool->compute();
      while (ctx->pool->working_count() != 1 || ctx->pool->tasks_count() != 0) { std::this_thread::sleep_for(std::chrono::microseconds(1)); }

      for (size_t i = 0; i < boundary_stresses.size(); ++i) {
        ctx->container->set_data<glm::vec3>(debug::entities::edge, i, debug::properties::edge::plate0_movement, glm::vec3(boundary_stresses[i].pressure_vector));
        ctx->container->set_data<glm::vec3>(debug::entities::edge, i, debug::properties::edge::plate1_movement, glm::vec3(boundary_stresses[i].shear_vector));
      }
    }

    void assign_distance_field(
      const generator::context* context,
      const uint32_t &plate_index,
      const std::set<uint32_t> &seeds,
      const std::unordered_set<uint32_t> &stops,
      utils::random_engine_st* rand,
      std::vector<std::pair<uint32_t, float>> &distances
    ) {
//       utils::random_engine_st rand(plate_index);
      std::vector<uint32_t> queue;

      for (auto idx : seeds) {
        ASSERT(idx < context->container->entities_count(debug::entities::edge));
        const auto &childs = context->container->get_childs(debug::entities::edge, idx);
        ASSERT(childs.size() == 2);
        const uint32_t tile_index0 = childs.front();
        const uint32_t tile_index1 = childs.back();
        const uint32_t current_plate_idx0 = context->container->get_data<uint32_t>(debug::entities::tile, tile_index0, debug::properties::tile::plate_index);
        const uint32_t current_plate_idx1 = context->container->get_data<uint32_t>(debug::entities::tile, tile_index1, debug::properties::tile::plate_index);
        if (plate_index == current_plate_idx0) {
          ASSERT(plate_index != current_plate_idx1);
          queue.push_back(tile_index0);
          distances[tile_index0] = std::make_pair(idx, 0);
        }

        if (plate_index == current_plate_idx1) {
          ASSERT(plate_index != current_plate_idx0);
          queue.push_back(tile_index1);
          distances[tile_index1] = std::make_pair(idx, 0);
        }
      }

      // чому то ранд разные значения здесь дает =(
      while (!queue.empty()) {
        // рандом дает более стабильный результат
        // но при этом исчезают острова =(
        const uint32_t index = rand->index(queue.size());
        //const uint32_t index = 0;
        ASSERT(index < queue.size());
        const uint32_t current_tile = queue[index];
        if (queue.size()-1 != index) queue[index] = queue.back();
        queue.pop_back();

        const auto &tile_data = render::unpack_data(context->map->get_tile(current_tile));
        for (uint32_t i = 0; i < 6; ++i) {
          const uint32_t n_index = tile_data.neighbours[i];
          if (n_index == UINT32_MAX) continue;

//           const auto &n_tile_data = render::unpack_data(context->map->get_tile(n_index));
//
//           const glm::vec4 tile_point = context->map->get_point(tile_data.center);
//           const glm::vec4 n_tile_point = context->map->get_point(n_tile_data.center);
//
//           const float test_dist = glm::distance(tile_point, n_tile_point);
//           ASSERT(test_dist < 3.0f);

          const uint32_t current_plate_idx = context->container->get_data<uint32_t>(debug::entities::tile, n_index, debug::properties::tile::plate_index);
          if (plate_index != current_plate_idx) continue;

          if (distances[n_index].second == 100000.0f && stops.find(n_index) == stops.end()) {
            distances[n_index] = std::make_pair(distances[current_tile].first, distances[current_tile].second + 1.0f);
            queue.push_back(n_index);
          }
        }
      }
    }

    void compute_plate_boundary_distances(generator::context* ctx, sol::table &table) {
      utils::time_log log("calculate plate boundary distances");
      // для каждого тайла нужно задать дальность до ближайшей гланицы с плитой
      // по идее это можно сделать по плитам

      // этот алгоритм должен давать тот же результат для тех же входных данных
      // (одинаковый context->boundary_edges, по идее простой сортировкой мы можем добиться чтобы этот массив был одинаковым)

      // попробуем сделать карты дальности для каждого тайла на плите

      std::vector<std::pair<uint32_t, float>> edge_index_dist(ctx->map->tiles_count(), std::make_pair(UINT32_MAX, 100000.0f));
      std::queue<uint32_t> queue;
      std::unordered_set<uint32_t> unique_tiles;
      for (size_t i = 0; i < ctx->container->entities_count(debug::entities::edge); ++i) {
        std::pair<uint32_t, uint32_t> pair;
        {
          const auto &childs = ctx->container->get_childs(debug::entities::edge, i);
          pair.first = childs.front();
          pair.second = childs.back();
        }

        queue.push(pair.first);
        queue.push(pair.second);
        unique_tiles.insert(pair.first);
        unique_tiles.insert(pair.second);

        const auto &tile0 = ctx->map->get_tile(pair.first);
        const auto &tile1 = ctx->map->get_tile(pair.second);

        const auto &point0 = ctx->map->get_point(tile0.tile_indices.x);
        const auto &point1 = ctx->map->get_point(tile1.tile_indices.x);

        const float dist = glm::distance(point0, point1) / 2.0f;

        //std::unique_lock<std::mutex> lock(mutex);
        //edge_index_dist.push_back(std::make_pair(pair.first, dist));
        edge_index_dist[pair.first] = std::make_pair(i, dist);
        edge_index_dist[pair.second] = std::make_pair(i, dist);
      }

      while (!queue.empty()) {
        const uint32_t index = queue.front();
        queue.pop();

        const auto &data = render::unpack_data(ctx->map->get_tile(index));
        const uint32_t n_count = render::is_pentagon(data) ? 5 : 6;
        for (uint32_t i = 0; i < n_count; ++i) {
          const uint32_t n_index = data.neighbours[i];
          if (unique_tiles.find(n_index) != unique_tiles.end()) continue;

          const auto &n_data = render::unpack_data(ctx->map->get_tile(n_index));
          const glm::vec4 point1 = ctx->map->get_point(data.center);
          const glm::vec4 point2 = ctx->map->get_point(n_data.center);

          const auto &prev = edge_index_dist[index];
          const float dist = glm::distance(point1, point2) + prev.second;
          if (edge_index_dist[n_index].second > dist) {
            edge_index_dist[n_index].first = prev.first;
            edge_index_dist[n_index].second = dist;
            queue.push(n_index);
          }
        }
      }

//       //std::mutex mutex;
//       utils::submit_works(ctx->pool, ctx->map->tiles_count(), [&edge_index_dist] (const size_t &start, const size_t &count, const generator::context* ctx) {
//         //std::unordered_set<uint32_t> unique_set;
//         std::queue<uint32_t> queue0;
//         std::queue<uint32_t> queue1;
//         for (size_t i = start; i < start+count; ++i) {
//           for (uint32_t j = 0; j < ctx->container->entities_count(debug::entities::edge); ++j) {
//             std::pair<uint32_t, uint32_t> pair;
//             {
//               const auto &childs = ctx->container->get_childs(debug::entities::edge, j);
//               pair.first = childs.front();
//               pair.second = childs.back();
//             }
//
//             const uint32_t plate_index1 = ctx->container->get_data<uint32_t>(debug::entities::tile, pair.first, debug::properties::tile::plate_index);
//             if (plate_index1 == i) {
// // #ifndef _NDEBUG
// //               auto itr = unique_set.find(pair.first);
// //               ASSERT(itr == unique_set.end());
// // #endif
// //               unique_set.insert(pair.first);
//               queue0.push(pair.first);
//
//               const auto &tile0 = ctx->map->get_tile(pair.first);
//               const auto &tile1 = ctx->map->get_tile(pair.second);
//
//               const auto &point0 = ctx->map->get_point(tile0.tile_indices.x);
//               const auto &point1 = ctx->map->get_point(tile1.tile_indices.x);
//
//               const float dist = glm::distance(point0, point1) / 2.0f;
//
//               //std::unique_lock<std::mutex> lock(mutex);
//               //edge_index_dist.push_back(std::make_pair(pair.first, dist));
//               edge_index_dist[pair.first] = std::make_pair(j, dist);
//             }
//
//             const uint32_t plate_index2 = ctx->container->get_data<uint32_t>(debug::entities::tile, pair.second, debug::properties::tile::plate_index);
//             if (plate_index2 == i) {
// // #ifndef _NDEBUG
// //               auto itr = unique_set.find(pair.second);
// //               ASSERT(itr == unique_set.end());
// // #endif
// //
// //               unique_set.insert(pair.second);
//               queue0.push(pair.second);
//
//               const auto &tile0 = ctx->map->get_tile(pair.first);
//               const auto &tile1 = ctx->map->get_tile(pair.second);
//
//               const auto &point0 = ctx->map->get_point(tile0.tile_indices.x);
//               const auto &point1 = ctx->map->get_point(tile1.tile_indices.x);
//
//               const float dist = glm::distance(point0, point1) / 2.0f;
//
//               //std::unique_lock<std::mutex> lock(mutex);
//               //edge_index_dist.push_back(std::make_pair(pair.second, dist));
//               edge_index_dist[pair.second] = std::make_pair(j, dist);
//             }
//
//             ASSERT(plate_index2 != plate_index1);
//           }
//
//           while (!queue0.empty()) {
//             while (!queue0.empty()) {
//               const uint32_t current_tile_index = queue0.front();
//               queue0.pop();
//
//               const auto &tile = render::unpack_data(ctx->map->get_tile(current_tile_index));
//               for (uint32_t j = 0; j < 6; ++j) {
//                 if (tile.neighbours[j] == UINT32_MAX) continue;
//                 const uint32_t neighbour_tile_index = tile.neighbours[j];
//                 const uint32_t plate_index = ctx->container->get_data<uint32_t>(debug::entities::tile, neighbour_tile_index, debug::properties::tile::plate_index);
//                 if (plate_index != i) continue;
//
//                 const auto &tile1 = ctx->map->get_tile(neighbour_tile_index);
//
//                 const auto &point0 = ctx->map->get_point(tile.center);
//                 const auto &point1 = ctx->map->get_point(tile1.tile_indices.x);
//
//                 //std::unique_lock<std::mutex> lock(mutex);
//                 //edge_index_dist.push_back(std::make_pair(neighbour_tile_index, dist));
//                 const auto &prev = edge_index_dist[current_tile_index];
//                 const float dist = glm::distance(point0, point1) + prev.second;
//                 if (edge_index_dist[neighbour_tile_index].second > dist) {
//                   edge_index_dist[neighbour_tile_index] = std::make_pair(prev.first, dist);
//                   queue1.push(neighbour_tile_index);
//                 }
//               }
//             }
//
//             std::swap(queue0, queue1);
//           }
//
//           //unique_set.clear();
//           {
//             const size_t size = queue0.size();
//             for (size_t i = 0; i < size; ++i) queue0.pop();
//             ASSERT(queue0.empty());
//           }
//
//           {
//             const size_t size = queue1.size();
//             for (size_t i = 0; i < size; ++i) queue1.pop();
//             ASSERT(queue1.empty());
//           }
//         }
//       }, ctx);

      std::set<uint32_t> mountains; // unordered версия нестабильная!
      std::set<uint32_t> oceans;
      std::set<uint32_t> coastlines;
      std::mutex m1;
      std::mutex m2;
      std::mutex m3;
      utils::submit_works_async(ctx->pool, ctx->container->entities_count(debug::entities::edge), [&mountains, &oceans, &coastlines, &m1, &m2, &m3] (const size_t &start, const size_t &count, const generator::context* ctx) {
//         const size_t start = 0;
//         const size_t count = context->boundary_edges.size();
        for (size_t i = start; i < start+count; ++i) {
          std::pair<uint32_t, uint32_t> pair;
            {
              const auto &childs = ctx->container->get_childs(debug::entities::edge, i);
              ASSERT(childs.size() == 2);
              pair.first = childs.front();
              pair.second = childs.back();
            }

          const uint32_t tile_index0 = pair.first;
          const uint32_t tile_index1 = pair.second;

          const uint32_t plate_index0 = ctx->container->get_data<uint32_t>(debug::entities::tile, tile_index0, debug::properties::tile::plate_index);
          const uint32_t plate_index1 = ctx->container->get_data<uint32_t>(debug::entities::tile, tile_index1, debug::properties::tile::plate_index);

          const bool data0_oceanic = bool(ctx->container->get_data<uint32_t>(debug::entities::plate, plate_index0, debug::properties::plate::oceanic));
          const bool data1_oceanic = bool(ctx->container->get_data<uint32_t>(debug::entities::plate, plate_index1, debug::properties::plate::oceanic));

          ASSERT(ctx->container->entities_count(debug::entities::edge) != 0);

          // необходимо какое-то ограничение или коэффициент
          // иначе получается плохо
          // у нас задача: раскидать границы по эффектам столкновения и расхождения плит
          // в зависимости от типа взаимодействия на границе мы получаем разные типы поверхностей
          //

          const auto plate_movement0 = ctx->container->get_data<glm::vec3>(debug::entities::edge, i, debug::properties::edge::plate0_movement);
          const auto plate_movement1 = ctx->container->get_data<glm::vec3>(debug::entities::edge, i, debug::properties::edge::plate1_movement);

          const float length0 = glm::length(plate_movement0);
          const float length1 = glm::length(plate_movement1);

          const auto dir0 = length0 < EPSILON ? glm::vec4(0.0f, 0.0f, 0.0f, 0.0f) : plate_movement0 / length0;
          const auto dir1 = length1 < EPSILON ? glm::vec4(0.0f, 0.0f, 0.0f, 0.0f) : plate_movement1 / length1;

          const uint32_t first_tile  = pair.first;
          const uint32_t second_tile = pair.second;
          const auto &first_tile_data = ctx->map->get_tile(first_tile);
          const auto &second_tile_data = ctx->map->get_tile(second_tile);
          const glm::vec3 first_tile_pos  = glm::vec3(ctx->map->get_point(first_tile_data.tile_indices[0]));
          const glm::vec3 second_tile_pos = glm::vec3(ctx->map->get_point(second_tile_data.tile_indices[0]));

//           const glm::vec4 boundary_position = (second_tile_pos + first_tile_pos) / 2.0f;
          const glm::vec3 boundary_normal = glm::normalize(second_tile_pos - first_tile_pos);

          const float dot0 = glm::dot(dir0,  boundary_normal);
          const float dot1 = glm::dot(dir1, -boundary_normal);

          //const bool collided = boundary_data.pressure > 0.3f;
          const bool collided = dot0 > 0.3f && dot1 > 0.3f;
          const bool opposite_dir = dot0 < -0.3f && dot1 < -0.3f;
          if (data0_oceanic && data1_oceanic) {
//             if (dot0 > 0.6f && dot1 > 0.6f) {
//               std::unique_lock<std::mutex> lock(m1);
//               mountains.insert(i);
//             } else
            if (collided) {
//               std::unique_lock<std::mutex> lock(m3);
//               coastlines.insert(i);
              std::unique_lock<std::mutex> lock(m1);
              mountains.insert(i);
            } else if (opposite_dir) {
              std::unique_lock<std::mutex> lock(m2);
              oceans.insert(i);
            } else {
              std::unique_lock<std::mutex> lock(m3);
              oceans.insert(i); // coastlines
            }
          } else if (!data0_oceanic && !data1_oceanic) {
            if (collided) {
              std::unique_lock<std::mutex> lock(m1);
              mountains.insert(i);
            } else if (opposite_dir) {
              std::unique_lock<std::mutex> lock(m2);
              oceans.insert(i);
            } else {
              std::unique_lock<std::mutex> lock(m3);
              coastlines.insert(i);
            }
          } else {
            if (collided) {
              std::unique_lock<std::mutex> lock(m1);
              mountains.insert(i);
            } else if (opposite_dir) {
              std::unique_lock<std::mutex> lock(m2);
              oceans.insert(i);
            } else {
              std::unique_lock<std::mutex> lock(m3);
              coastlines.insert(i);
            }
          }
        }
      }, ctx);
      ctx->pool->compute();
      while (ctx->pool->working_count() != 1 || ctx->pool->tasks_count() != 0) { std::this_thread::sleep_for(std::chrono::microseconds(1)); }

      std::vector<std::pair<uint32_t, float>> mountain_dist(ctx->map->tiles_count(), std::make_pair(UINT32_MAX, 100000.0f));
      std::vector<std::pair<uint32_t, float>> ocean_dist(ctx->map->tiles_count(), std::make_pair(UINT32_MAX, 100000.0f));
      std::vector<std::pair<uint32_t, float>> coastline_dist(ctx->map->tiles_count(), std::make_pair(UINT32_MAX, 100000.0f));
      std::unordered_set<uint32_t> stops;
      for (auto idx : mountains) {
        const auto &childs = ctx->container->get_childs(debug::entities::edge, idx);
        ASSERT(childs.size() == 2);
        stops.insert(childs.front());
        stops.insert(childs.back());
      }

      for (auto idx : oceans) {
        const auto &childs = ctx->container->get_childs(debug::entities::edge, idx);
        ASSERT(childs.size() == 2);
        stops.insert(childs.front());
        stops.insert(childs.back());
      }

      for (auto idx : coastlines) {
        const auto &childs = ctx->container->get_childs(debug::entities::edge, idx);
        ASSERT(childs.size() == 2);
        stops.insert(childs.front());
        stops.insert(childs.back());
      }

      std::unordered_set<uint32_t> ocean_stops;
      for (auto idx : oceans) {
        const auto &childs = ctx->container->get_childs(debug::entities::edge, idx);
        ASSERT(childs.size() == 2);
        ocean_stops.insert(childs.front());
        ocean_stops.insert(childs.back());
      }

      std::unordered_set<uint32_t> coastline_stops;
      for (auto idx : coastlines) {
        const auto &childs = ctx->container->get_childs(debug::entities::edge, idx);
        ASSERT(childs.size() == 2);
        coastline_stops.insert(childs.front());
        coastline_stops.insert(childs.back());
      }

      const uint32_t plates_count = table["final_plates_count"];
      utils::submit_works_async(ctx->pool, plates_count, [&mountains, &oceans, &coastlines, &ocean_stops, &coastline_stops, &stops, &mountain_dist, &ocean_dist, &coastline_dist] (const size_t &start, const size_t &count, const generator::context* context) {
//         const size_t start = 0;
//         const size_t count = context->plate_tile_indices.size();
        for (size_t i = start; i < start+count; ++i) {
          const uint32_t plate_index = i;

          utils::random_engine_st eng(plate_index);
          assign_distance_field(context, plate_index, mountains, ocean_stops, &eng, mountain_dist);
          assign_distance_field(context, plate_index, oceans, coastline_stops, &eng, ocean_dist);
          assign_distance_field(context, plate_index, coastlines, stops, &eng, coastline_dist);
        }
      }, ctx);
      ctx->pool->compute();
      while (ctx->pool->working_count() != 1 || ctx->pool->tasks_count() != 0) { std::this_thread::sleep_for(std::chrono::microseconds(1)); }

      for (size_t i = 0; i < edge_index_dist.size(); ++i) {
        ctx->container->set_data<uint32_t>(debug::entities::tile, i, debug::properties::tile::edge_index, edge_index_dist[i].first);
        ctx->container->set_data<float>(debug::entities::tile, i, debug::properties::tile::edge_dist, edge_index_dist[i].second);
      }

      for (size_t i = 0; i < mountain_dist.size(); ++i) {
        ctx->container->set_data<uint32_t>(debug::entities::tile, i, debug::properties::tile::mountain_index, mountain_dist[i].first);
        ctx->container->set_data<float>(debug::entities::tile, i, debug::properties::tile::mountain_dist, mountain_dist[i].second);
      }

      for (size_t i = 0; i < ocean_dist.size(); ++i) {
        ctx->container->set_data<uint32_t>(debug::entities::tile, i, debug::properties::tile::ocean_index, ocean_dist[i].first);
        ctx->container->set_data<float>(debug::entities::tile, i, debug::properties::tile::ocean_dist, ocean_dist[i].second);
      }

      for (size_t i = 0; i < coastline_dist.size(); ++i) {
        ctx->container->set_data<uint32_t>(debug::entities::tile, i, debug::properties::tile::coastline_index, coastline_dist[i].first);
        ctx->container->set_data<float>(debug::entities::tile, i, debug::properties::tile::coastline_dist, coastline_dist[i].second);
      }
    }

    void calculate_vertex_elevation(generator::context* ctx, sol::table &table) {
      utils::time_log log("calculate tiles evaliation");
      std::vector<float> tile_elevation(ctx->map->tiles_count());

      //update_noise_seed(ctx);

      // в текущем виде генерирует довольно интересный ландшафт
      // но тем не менее здесь есть еще что изменить

      const float noise_multiplier_local = table["userdata"]["noise_multiplier"];
      utils::submit_works_async(ctx->pool, ctx->map->tiles_count(), [&tile_elevation, noise_multiplier_local] (const size_t &start, const size_t &count, const generator::context* context) {
        for (size_t i = start; i < start+count; ++i) {
          float accum_elevation = 0.0f;
          uint32_t elevations_count = 0;
          const uint32_t plate_index = context->container->get_data<uint32_t>(debug::entities::tile, i, debug::properties::tile::plate_index);
          const uint32_t tile_index = i;
          const auto &tile_data = context->map->get_tile(tile_index);
          const glm::vec4 tile_point = context->map->get_point(tile_data.tile_indices.x);

          const float plate_elevation = context->container->get_data<float>(debug::entities::plate, plate_index, debug::properties::plate::base_elevation);
          ++elevations_count;

          const auto a_pair = std::make_pair(
            context->container->get_data<uint32_t>(debug::entities::tile, i, debug::properties::tile::mountain_index),
            context->container->get_data<float>(debug::entities::tile, i, debug::properties::tile::mountain_dist)
          );

          const auto b_pair = std::make_pair(
            context->container->get_data<uint32_t>(debug::entities::tile, i, debug::properties::tile::ocean_index),
            context->container->get_data<float>(debug::entities::tile, i, debug::properties::tile::ocean_dist)
          );

          const auto c_pair = std::make_pair(
            context->container->get_data<uint32_t>(debug::entities::tile, i, debug::properties::tile::coastline_index),
            context->container->get_data<float>(debug::entities::tile, i, debug::properties::tile::coastline_dist)
          );

          const float a = a_pair.second + EPSILON;
          const float b = b_pair.second + EPSILON;
          const float c = c_pair.second + EPSILON;

          const float max_k =  1.0f;
          const float min_k = -1.0f;
          float a_k = max_k;
          float b_k = min_k;
          {
            // больший коэффициент горы, меньший воды
            if (a_pair.first != UINT32_MAX) {
              const uint32_t edge_index = a_pair.first;
              const auto &childs = context->container->get_childs(debug::entities::edge, edge_index);
              const auto &boundary_pair = std::make_pair(childs.front(), childs.back());
              const uint32_t plate0 = context->container->get_data<uint32_t>(debug::entities::tile, boundary_pair.first, debug::properties::tile::plate_index);
              const uint32_t plate1 = context->container->get_data<uint32_t>(debug::entities::tile, boundary_pair.second, debug::properties::tile::plate_index);
              const uint32_t opposing_plate_index = plate_index == plate0 ? plate1 : plate0;

              const float opposing_plate_elevation = context->container->get_data<float>(debug::entities::plate, opposing_plate_index, debug::properties::plate::base_elevation);
              const float boundary_elevation = std::max(opposing_plate_elevation, plate_elevation);

              // к boundary_elevation нужно прибавить какую то силу
              // максимум который мы можем прибавить это 0.5f
              // как его расчитать, мы можем взять доты как на предыдущих шагах
              // но доты нужно как то компенсировать силой

              // необходимо какое-то ограничение или коэффициент
              // иначе получается плохо
              // у нас задача: раскидать границы по эффектам столкновения и расхождения плит
              // в зависимости от типа взаимодействия на границе мы получаем разные типы поверхностей
              //

              const auto plate_movement0 = context->container->get_data<glm::vec3>(debug::entities::edge, edge_index, debug::properties::edge::plate0_movement);
              const auto plate_movement1 = context->container->get_data<glm::vec3>(debug::entities::edge, edge_index, debug::properties::edge::plate1_movement);

              const float length0 = glm::length(plate_movement0);
              const float length1 = glm::length(plate_movement1);

              const auto dir0 = length0 < EPSILON ? glm::vec3(0.0f, 0.0f, 0.0f) : plate_movement0 / length0;
              const auto dir1 = length1 < EPSILON ? glm::vec3(0.0f, 0.0f, 0.0f) : plate_movement1 / length1;

              const uint32_t first_tile  = boundary_pair.first;
              const uint32_t second_tile = boundary_pair.second;
              const auto &first_tile_data = context->map->get_tile(first_tile);
              const auto &second_tile_data = context->map->get_tile(second_tile);
              const glm::vec3 first_tile_pos  = context->map->get_point(first_tile_data.tile_indices[0]);
              const glm::vec3 second_tile_pos = context->map->get_point(second_tile_data.tile_indices[0]);

    //           const glm::vec4 boundary_position = (second_tile_pos + first_tile_pos) / 2.0f;
              const glm::vec3 boundary_normal = glm::normalize(second_tile_pos - first_tile_pos);

              const float dot0 = glm::dot(dir0,  boundary_normal);
              const float dot1 = glm::dot(dir1, -boundary_normal);

              const float dot_k = (dot0 + dot1) / 2.0f;
              const float final_k = boundary_elevation < 0.0f ? std::max(-boundary_elevation * (1.0f + dot_k) * 0.8f, -boundary_elevation) : glm::mix(-0.2f, 0.4f, dot_k);

              a_k = boundary_elevation + final_k;

              ASSERT(a_k >= 0.0f && a_k <= 1.0f);
            }

            // наоборот
            if (b_pair.first != UINT32_MAX) {
              const uint32_t edge_index = b_pair.first;
              const auto &childs = context->container->get_childs(debug::entities::edge, edge_index);
              const auto &boundary_pair = std::make_pair(childs.front(), childs.back());
              const uint32_t plate0 = context->container->get_data<uint32_t>(debug::entities::tile, boundary_pair.first, debug::properties::tile::plate_index);
              const uint32_t plate1 = context->container->get_data<uint32_t>(debug::entities::tile, boundary_pair.second, debug::properties::tile::plate_index);
              const uint32_t opposing_plate_index = plate_index == plate0 ? plate1 : plate0;

              const float opposing_plate_elevation = context->container->get_data<float>(debug::entities::plate, opposing_plate_index, debug::properties::plate::base_elevation);
              const float boundary_elevation = std::min(opposing_plate_elevation, plate_elevation);

              // к boundary_elevation нужно прибавить какую то силу
              // максимум который мы можем прибавить это 0.5f
              // как его расчитать, мы можем взять доты как на предыдущих шагах
              // но доты нужно как то компенсировать силой

              // необходимо какое-то ограничение или коэффициент
              // иначе получается плохо
              // у нас задача: раскидать границы по эффектам столкновения и расхождения плит
              // в зависимости от типа взаимодействия на границе мы получаем разные типы поверхностей
              //

              const auto plate_movement0 = context->container->get_data<glm::vec3>(debug::entities::edge, edge_index, debug::properties::edge::plate0_movement);
              const auto plate_movement1 = context->container->get_data<glm::vec3>(debug::entities::edge, edge_index, debug::properties::edge::plate1_movement);

              const float length0 = glm::length(plate_movement0);
              const float length1 = glm::length(plate_movement1);

              const auto dir0 = length0 < EPSILON ? glm::vec3(0.0f, 0.0f, 0.0f) : plate_movement0 / length0;
              const auto dir1 = length1 < EPSILON ? glm::vec3(0.0f, 0.0f, 0.0f) : plate_movement1 / length1;

              const uint32_t first_tile  = boundary_pair.first;
              const uint32_t second_tile = boundary_pair.second;
              const auto &first_tile_data = context->map->get_tile(first_tile);
              const auto &second_tile_data = context->map->get_tile(second_tile);
              const glm::vec3 first_tile_pos  = context->map->get_point(first_tile_data.tile_indices[0]);
              const glm::vec3 second_tile_pos = context->map->get_point(second_tile_data.tile_indices[0]);

    //           const glm::vec4 boundary_position = (second_tile_pos + first_tile_pos) / 2.0f;
              const glm::vec3 boundary_normal = glm::normalize(second_tile_pos - first_tile_pos);

              const float dot0 = glm::dot(dir0,  boundary_normal);
              const float dot1 = glm::dot(dir1, -boundary_normal);

              const float dot_k = 1.0f - glm::abs(dot0 + dot1) / 2.0f; // нужно увеличить К воды
              const float final_k = boundary_elevation > 0.0f ?  std::min(-boundary_elevation * (1.0f + dot_k) * 0.8f, -boundary_elevation) : glm::mix(-0.2f, 0.05f, dot_k);

              b_k = boundary_elevation + final_k;

              ASSERT(b_k >= -1.0f && b_k <= 0.0f);
            }
          }

          // тут нужно учесть текущий подъем везде
          if (a_pair.first == UINT32_MAX && b_pair.first == UINT32_MAX) {
            //accum_elevation = plate_elevation;
            //accum_elevation = 0.0f;
            const uint32_t edge_index = c_pair.first;
            const auto &childs = context->container->get_childs(debug::entities::edge, edge_index);
            const auto &boundary_pair = std::make_pair(childs.front(), childs.back());
            const uint32_t plate0 = context->container->get_data<uint32_t>(debug::entities::tile, boundary_pair.first, debug::properties::tile::plate_index);
            const uint32_t plate1 = context->container->get_data<uint32_t>(debug::entities::tile, boundary_pair.second, debug::properties::tile::plate_index);
            const uint32_t opposing_plate_index = plate_index == plate0 ? plate1 : plate0;
            const float opposing_plate_elevation = context->container->get_data<float>(debug::entities::plate, opposing_plate_index, debug::properties::plate::base_elevation);
            accum_elevation = (opposing_plate_elevation + plate_elevation) / 2.0f;
          } else {
            accum_elevation = (a_k/a + b_k/b) / (1.0f/a + 1.0f/b + 1.0f/c);
          }

          // при каких интересно значениях будет больше 1
          accum_elevation += noise_multiplier_local * context->noise->GetNoise(tile_point.x, tile_point.y, tile_point.z);
          //accum_elevation = (accum_elevation - (-0.9f)) / (1.1f - (-0.9f));
          accum_elevation = glm::clamp(accum_elevation, -1.0f, 1.0f);

          ASSERT(accum_elevation == accum_elevation);
          ASSERT(accum_elevation >= -1.0f && accum_elevation <= 1.0f);

          ASSERT(elevations_count != 0);

          tile_elevation[i] = accum_elevation / float(elevations_count);
        }
      }, ctx);
      ctx->pool->compute();
      while (ctx->pool->working_count() != 1 || ctx->pool->tasks_count() != 0) { std::this_thread::sleep_for(std::chrono::microseconds(1)); }

      for (size_t i = 0; i < tile_elevation.size(); ++i) {
        ctx->container->set_data<float>(debug::entities::tile, i, debug::properties::tile::elevation, tile_elevation[i]);
        ctx->map->set_tile_height(i, tile_elevation[i]);
      }
    }

    void blur_tile_elevation(generator::context* ctx, sol::table &table) {
      utils::time_log log("tile elevation bluring");

      std::vector<float> new_elevations(ctx->map->tiles_count());
      for (size_t i = 0; i < ctx->map->tiles_count(); ++i) {
        new_elevations[i] = ctx->container->get_data<float>(debug::entities::tile, i, debug::properties::tile::elevation);
      }

      const float new_old_ratio_local = table["userdata"]["blur_ratio"];
      const float water_ground_ratio_local = table["userdata"]["blur_water_ratio"];
      const uint32_t iterations_count = table["userdata"]["blur_iterations_count"];
      for (uint32_t i = 0; i < iterations_count; ++i) {
        utils::submit_works_async(ctx->pool, ctx->map->tiles_count(), [&new_elevations, new_old_ratio_local, water_ground_ratio_local] (
          const size_t &start,
          const size_t &count,
          const generator::context* context
        ) {
          for (size_t i = start; i < start+count; ++i) {
            const uint32_t tile_index = i;
            const auto &tile_data = render::unpack_data(context->map->get_tile(tile_index));
            const float old_elevation = context->container->get_data<float>(debug::entities::tile, tile_index, debug::properties::tile::elevation);
  //           const uint32_t plate_index = context->tile_plate_indices[tile_index];

            float accum_elevation = 0.0f;
            float accum_water = 0.0f;
            float accum_ground = 0.0f;
            const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
            for (uint32_t j = 0; j < n_count; ++j) {
              const uint32_t n_index = tile_data.neighbours[j];
              if (n_index == UINT32_MAX) continue;

  //             const uint32_t n_plate_index = context->tile_plate_indices[n_index];
              const float n_old_elevation = context->container->get_data<float>(debug::entities::tile, n_index, debug::properties::tile::elevation);
              //accum_elevation += n_old_elevation;
              if (n_old_elevation <= 0.0f) accum_water += n_old_elevation;
              else accum_ground += n_old_elevation;
            }
  //           accum_elevation += old_elevation;
            accum_water = accum_water * water_ground_ratio_local;
            accum_ground = accum_ground * (2.0f - water_ground_ratio_local);
            accum_elevation = (accum_water + accum_ground) / float(n_count);

            const float sum_k = new_old_ratio_local;
            new_elevations[tile_index] = old_elevation * sum_k + accum_elevation * (1.0f - sum_k);
  //           new_elevations[tile_index] = accum_elevation;

          }
        }, ctx);
        ctx->pool->compute();
        while (ctx->pool->working_count() != 1 || ctx->pool->tasks_count() != 0) { std::this_thread::sleep_for(std::chrono::microseconds(1)); }

        for (size_t i = 0; i < ctx->map->tiles_count(); ++i) {
          ctx->container->set_data<float>(debug::entities::tile, i, debug::properties::tile::elevation, new_elevations[i]);
        }
      }

      uint32_t water_counter = 0;
      for (size_t i = 0 ; i < ctx->map->tiles_count(); ++i) {
        const float h = ctx->container->get_data<float>(debug::entities::tile, i, debug::properties::tile::elevation);
        ctx->map->set_tile_height(i, h);
        if (h < 0.0f) ++water_counter;
      }

      PRINT_VAR("oceanic tiles after recompute", water_counter)
      PRINT_VAR("oceanic tiles k              ", float(water_counter) / float(ctx->map->tiles_count()))
    }

    void update_maximum(std::atomic<int32_t> &mem, const int32_t &value) {
      int32_t prev_value = mem;
      while (prev_value < value && !mem.compare_exchange_weak(prev_value, value)) {}
    }

    void update_minimum(std::atomic<int32_t> &mem, const int32_t &value) {
      int32_t prev_value = mem;
      while (prev_value > value && !mem.compare_exchange_weak(prev_value, value)) {}
    }

    void normalize_fractional_values(generator::context* ctx, sol::table &table, const uint32_t &entity_id, const uint32_t &property_id) {
      (void)table;
      utils::time_log log("normalize fractional values");

      std::atomic<int32_t> min_height_mem( INT32_MAX);
      std::atomic<int32_t> max_height_mem(-INT32_MAX);

      std::vector<float> new_data(ctx->container->entities_count(entity_id));
      utils::submit_works_async(ctx->pool, ctx->container->entities_count(entity_id), [&min_height_mem, &max_height_mem, entity_id, property_id] (const size_t &start, const size_t &count, const generator::context* context) {
        for (size_t i = start; i < start+count; ++i) {
          const uint32_t tile_index = i;
          //float data = new_data[tile_index];
          float data = context->container->get_data<float>(entity_id, tile_index, property_id);
          if (data < 0.0f) continue;
          //tile_height = (tile_height + 1.0f) / 2.0f;
          //tile_height = mapper(tile_height, -1.0f, 1.0f, 0.0f, 1.0f);
//           ASSERT(tile_height >= 0.0f && tile_height <= 2.0f);
          update_maximum(max_height_mem, glm::floatBitsToInt(data));
          update_minimum(min_height_mem, glm::floatBitsToInt(data));
        }
      }, ctx);
      ctx->pool->compute();
      while (ctx->pool->working_count() != 1 || ctx->pool->tasks_count() != 0) { std::this_thread::sleep_for(std::chrono::microseconds(1)); }

      utils::submit_works_async(ctx->pool, ctx->container->entities_count(entity_id), [&min_height_mem, &max_height_mem, entity_id, property_id] (const size_t &start, const size_t &count, const generator::context* context) {
        float max_height = glm::intBitsToFloat(max_height_mem);
        float min_height = glm::intBitsToFloat(min_height_mem);
        //max_height = max_height - 1.0f;
        //min_height = min_height - 1.0f;
//         ASSERT(max_height >= -1.0f && max_height <= 1.0f);
//         ASSERT(min_height >= -1.0f && min_height <= 1.0f);
//         max_height = (max_height + 1.0f) / 2.0f;
//         min_height = (min_height + 1.0f) / 2.0f;

//         ASSERT(max_height >= 0.0f && max_height <= 1.0f);
//         ASSERT(min_height >= 0.0f && min_height <= 1.0f);

        for (size_t i = start; i < start+count; ++i) {
          const uint32_t tile_index = i;
          //float tile_data = new_data[tile_index];
          float tile_data = context->container->get_data<float>(entity_id, tile_index, property_id);
          if (tile_data < 0.0f) continue;
          //tile_height = (tile_height + 1.0f) / 2.0f;
          //tile_height = tile_height + 1.0f;
//           ASSERT(tile_height >= 0.0f && tile_height <= 2.0f);
//           ASSERT(min_height >= 0.0f && min_height <= 2.0f);
//           ASSERT(max_height >= 0.0f && max_height <= 2.0f);
//           tile_height = mapper(tile_height, -1.0f, 1.0f, 0.0f, 1.0f);
          float new_tile_height = (tile_data - min_height) / (max_height - min_height);
          //new_tile_height = (new_tile_height - 0.5f) * 2.0f;
//           new_tile_height = mapper(new_tile_height, 0.0f, 1.0f, -1.0f, 1.0f);
          //new_tile_height = new_tile_height - 0.5f;
          ASSERT(new_tile_height >= 0.0f && new_tile_height <= 1.0f);
          context->container->set_data<float>(entity_id, tile_index, property_id, new_tile_height);
          //new_data[tile_index] = new_tile_height;
        }
      }, ctx);
      ctx->pool->compute();
      while (ctx->pool->working_count() != 1 || ctx->pool->tasks_count() != 0) { std::this_thread::sleep_for(std::chrono::microseconds(1)); }

      //std::swap(*data_container, new_data);
    }

    float mapper(const float &value, const float &smin, const float &smax, const float &dmin, const float &dmax) {
      return ((value - smin) / (smax - smin)) * (dmax - dmin) + dmin;
    }

    void compute_tile_heat(generator::context* ctx, sol::table &table) {
      (void)table;
      utils::time_log log("heat generation");

      update_noise_seed(ctx);

      std::vector<float> tile_heat(ctx->map->tiles_count(), 0.0f);
      float min_value =  1.0f;
      float max_value = -1.0f;
      for (uint32_t i = 0; i < tile_heat.size(); ++i) {
        const auto &tile_data = render::unpack_data(ctx->map->get_tile(i));
        const glm::vec4 point = ctx->map->get_point(tile_data.center);
        // скорее всего нужно увеличить вклад шума
        tile_heat[i] = ctx->noise->GetNoise(point.x, point.y, point.z);
//         ASSERT(tile_heat[i] >= -0.1f);
//         ASSERT(tile_heat[i] <=  0.1f);
        min_value = std::min(min_value, tile_heat[i]);
        max_value = std::max(max_value, tile_heat[i]);
      }

      for (uint32_t i = 0; i < tile_heat.size(); ++i) {
        tile_heat[i] = (tile_heat[i] - min_value) / (max_value - min_value);
        tile_heat[i] = 0.1f * tile_heat[i];
      }

      utils::submit_works_async(ctx->pool, ctx->map->tiles_count(), [&tile_heat] (const size_t &start, const size_t &count, const generator::context* context) {
        for (size_t i = start; i < start+count; ++i) {
          const uint32_t current_tile = i;
          const float height = context->container->get_data<float>(debug::entities::tile, current_tile, debug::properties::tile::elevation);
//           if (height >= 0.0f) {
            const auto &tile_data = render::unpack_data(context->map->get_tile(current_tile));
            const glm::vec4 point = glm::normalize(context->map->get_point(tile_data.center));
            const float h_part = glm::mix(-0.9f, 0.2f, 1.0f - height);
            const float h_k = h_part;
            const float y_part = 1.0f - glm::abs(point.y);
            const float final_y = y_part + h_k * y_part;
            float k = glm::clamp(final_y, 0.0f, 1.0f);
//             k = (k - 0.03f) / (0.9f - 0.03f);
//             k = k * 0.9f;
            ASSERT(k >= 0.0f && k <= 1.0f);
            tile_heat[current_tile] += k;
            //tile_heat[current_tile] = std::max(tile_heat[current_tile], 0.0f);
            tile_heat[current_tile] = mapper(tile_heat[current_tile], -0.1f, 1.1f, 0.0f, 1.0f);
//           } else {
//             const auto &tile_data = render::unpack_data(context->map->get_tile(current_tile));
//             const glm::vec4 point = glm::normalize(context->map->get_point(tile_data.center));
//             const uint32_t dist_to_ground = context->ground_distance[current_tile].second;
//             ASSERT(dist_to_ground != 0);
//             const float ground_part = (float(dist_to_ground) / float(9)) * float(dist_to_ground <= 9);
//             const float upper_bound = glm::mix(0.8f, 1.0f, ground_part);
//             const float y_part = 1.0f - glm::abs(point.y);
//             ASSERT(y_part <= 1.0f);
//             ASSERT(y_part >= 0.0f);
//             ASSERT(upper_bound <= 1.0f);
//             ASSERT(upper_bound >= 0.8f);
//             float k = glm::clamp(y_part, 0.03f, upper_bound);
//             k = (k - 0.03f) / (upper_bound - 0.03f);
//             //k = mapper(k, 0.0f, 1.0f, 0.0f, 0.9f);
//             k = k * 0.9f;
//             ASSERT(k >= 0.0f && k <= 1.0f);
//             tile_heat[current_tile] += k;
//             //tile_heat[current_tile] = std::max(tile_heat[current_tile], 0.0f);
//             tile_heat[current_tile] = mapper(tile_heat[current_tile], -0.1f, 1.1f, 0.0f, 1.0f);
//           }
        }
      }, ctx);
      ctx->pool->compute();
      while (ctx->pool->working_count() != 1 || ctx->pool->tasks_count() != 0) { std::this_thread::sleep_for(std::chrono::microseconds(1)); }

      for (size_t i = 0; i < tile_heat.size(); ++i) {
        ctx->container->set_data<float>(debug::entities::tile, i, debug::properties::tile::heat, tile_heat[i]);
      }
    }

    void compute_tile_distances(generator::context* ctx, sol::table &table, const std::function<bool(const generator::context*, const sol::table&, const uint32_t &tile_index)> &predicate, const std::string &key) {
      utils::time_log log("computing tile distances");

      std::vector<std::pair<uint32_t, uint32_t>> ground_distance(ctx->map->tiles_count(), std::make_pair(UINT32_MAX, UINT32_MAX));
      std::queue<uint32_t> queue;

      for (size_t i = 0; i < ctx->map->tiles_count(); ++i) {
        const uint32_t tile_index = i;
        if (predicate(ctx, table, tile_index)) {
          queue.push(tile_index);
          ground_distance[tile_index] = std::make_pair(tile_index, 0);
        }
      }

      while (!queue.empty()) {
        const uint32_t current_tile = queue.front();
        queue.pop();

        const auto &tile_data = render::unpack_data(ctx->map->get_tile(current_tile));
        const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
        for (uint32_t i = 0; i < n_count; ++i) {
          const uint32_t n_index = tile_data.neighbours[i];
          if (ground_distance[n_index].second == UINT32_MAX) {
            ground_distance[n_index] = std::make_pair(ground_distance[current_tile].first, ground_distance[current_tile].second + 1);
            queue.push(n_index);
          }
        }
      }

      // куда сложить ответ?
      table["tiles"].get_or_create<sol::table>();
      for (size_t i = 0; i < ground_distance.size(); ++i) {
        table["tiles"][i].get_or_create<sol::table>();
        table["tiles"][i][key].get_or_create<sol::table>();
        table["tiles"][i][key]["index"] = ground_distance[i].first;
        table["tiles"][i][key]["dist"] = ground_distance[i].second;
      }
    }

    void compute_tile_moisture(generator::context* ctx, sol::table &table) {
      utils::time_log log("computing tile moisture");

      update_noise_seed(ctx);

      // короче шум перлина выглядит подходяще честно говоря
      // возможно его нужно парочкой параметров подкрасить
      std::vector<float> wetness(ctx->map->tiles_count());
      float max_val = -1.0f;
      float min_val =  1.0f;
      for (size_t i = 0; i < ctx->map->tiles_count(); ++i) {
        const uint32_t current_tile = i;
        const auto &tile_data = ctx->map->get_tile(current_tile);
        const glm::vec4 point = ctx->map->get_point(tile_data.tile_indices.x);
        const float k = ctx->noise->GetNoise(point.x, point.y, point.z);
        wetness[current_tile] = k;
        max_val = std::max(max_val, wetness[current_tile]);
        min_val = std::min(min_val, wetness[current_tile]);
      }

      for (size_t i = 0; i < ctx->map->tiles_count(); ++i) {
        const uint32_t current_tile = i;
        wetness[current_tile] = (wetness[current_tile] - min_val) / (max_val - min_val);
        const float height = ctx->container->get_data<float>(debug::entities::tile, current_tile, debug::properties::tile::elevation);
        const float heat = ctx->container->get_data<float>(debug::entities::tile, current_tile, debug::properties::tile::heat);
        wetness[current_tile] = wetness[current_tile] * wetness[current_tile]; //  * heat
        const uint32_t dist_to_water = table["tiles"][i]["water_distance"]["dist"];
        if (dist_to_water > 0) {
          const float dist_k = 1.0f - float(dist_to_water > 3 ? 3 : dist_to_water) / float(3);
          wetness[current_tile] = std::min(wetness[current_tile] + wetness[current_tile] * dist_k, 1.0f);
        }
        // мне нужно сделать так чтобы на берегах была повышенная влажность
        // и чтобы вне экватора влажность распределялась более умерено
        // хотя с другой стороны это можно сделать иначе
      }

      for (size_t i = 0; i < wetness.size(); ++i) {
        ctx->container->set_data<float>(debug::entities::tile, i, debug::properties::tile::moisture, wetness[i]);
      }
    }

    uint32_t calcutate_biome2(const float elevation, const float temperature, const float wetness) {
      if (elevation < 0.0f) {
        if (temperature < 0.3f) return render::biome_ocean_glacier;
        else return render::biome_ocean;
      } else if (elevation < 0.6f) {
        if (temperature > 0.75f) {
//           ASSERT(false);
          if (wetness > 0.75f) return render::biome_rain_forest;
          else if (wetness > 0.5f) return render::biome_rain_forest;
          else if (wetness > 0.25f) return render::biome_grassland;
          else return render::biome_desert;

        } else if (temperature > 0.5f) {
//           ASSERT(false);
          if (wetness > 0.75f) return render::biome_rain_forest;
          else if (wetness > 0.5f) return render::biome_deciduous_forest;
          else if (wetness > 0.25f) return render::biome_grassland;
          else return render::biome_desert;

        } else if (temperature > 0.25f) {

          if (wetness > 0.75f) return render::biome_deciduous_forest;
          else if (wetness > 0.5f) return render::biome_conifer_forest;
          else if (wetness > 0.25f) return render::biome_grassland;
          else return render::biome_plains;

        } else if (temperature > 0.15f) {

//           if (wetness > 0.75f) return render::biome_tundra;
//           else if (wetness > 0.5f) return render::biome_tundra;
//           else if (wetness > 0.25f) return render::biome_land_glacier;
//           else return render::biome_land_glacier;

          return render::biome_tundra;

        } else {
          return render::biome_land_glacier;
        }
      } else {
        if (temperature > 0.5f) return render::biome_mountain;
        else return render::biome_snowy_mountain;
      }

      return UINT32_MAX;
    }

    void create_biomes(generator::context* ctx, sol::table &table) {
      (void)table;
      utils::time_log log("creating biomes");
      
      {
        auto image_table = global::get<sol::state>()->create_table();
        image_table["id"] = "hex_grass1";
        image_table["path"] = "apates_quest/textures/biomes/hex_grass4.png";
        image_table["type"] = 0; // тут индекс
        image_table["sampler"] = 1;
        utils::add_image(image_table);
      }
      
      {
        auto image_table = global::get<sol::state>()->create_table();
        image_table["id"] = "hex_grass2";
        image_table["path"] = "apates_quest/textures/biomes/hex_grass5.png";
        image_table["type"] = 0; // тут индекс
        image_table["sampler"] = 1;
        utils::add_image(image_table);
      }
      
      {
        auto image_table = global::get<sol::state>()->create_table();
        image_table["id"] = "hex_water";
        image_table["path"] = "apates_quest/textures/biomes/hex_water.png";
        image_table["type"] = 0; // тут индекс
        image_table["sampler"] = 1;
        utils::add_image(image_table);
      }
      
      {
        auto image_table = global::get<sol::state>()->create_table();
        image_table["id"] = "hex_snow";
        image_table["path"] = "apates_quest/textures/biomes/hex_snow.png";
        image_table["type"] = 0; // тут индекс
        image_table["sampler"] = 1;
        utils::add_image(image_table);
      }
      
      {
        auto image_table = global::get<sol::state>()->create_table();
        image_table["id"] = "hex_cold_water";
        image_table["path"] = "apates_quest/textures/biomes/hex_cold_water.png";
        image_table["type"] = 0; // тут индекс
        image_table["sampler"] = 1;
        utils::add_image(image_table);
      }
      
      {
        auto image_table = global::get<sol::state>()->create_table();
        image_table["id"] = "hex_desert";
        image_table["path"] = "apates_quest/textures/biomes/hex_desert.png";
        image_table["type"] = 0; // тут индекс
        image_table["sampler"] = 1;
        utils::add_image(image_table);
      }
      
      {
        auto image_table = global::get<sol::state>()->create_table();
        image_table["id"] = "rain_tree";
        image_table["path"] = "apates_quest/textures/biomes/upper_palm.png";
        //auto atlas = image_table["atlas_data"].get_or_create<sol::table>();
        auto scale = image_table["scale"].get_or_create<sol::table>();
        scale["width"] = 32;
        scale["height"] = 32;
        scale["filter"] = 1; // nearest
        image_table["type"] = 0; // тут индекс
        image_table["sampler"] = 1;
        utils::add_image(image_table);
      }
      
      {
        auto image_table = global::get<sol::state>()->create_table();
        image_table["id"] = "deciduous_tree";
        image_table["path"] = "apates_quest/textures/biomes/upper_tree5.png";
        //auto atlas = image_table["atlas_data"].get_or_create<sol::table>();
        auto scale = image_table["scale"].get_or_create<sol::table>();
        scale["width"] = 32;
        scale["height"] = 32;
        scale["filter"] = 1; // nearest
        image_table["type"] = 0; // тут индекс
        image_table["sampler"] = 1;
        utils::add_image(image_table);
      }
      
      {
        auto image_table = global::get<sol::state>()->create_table();
        image_table["id"] = "coniferous_tree";
        image_table["path"] = "apates_quest/textures/biomes/upper_spruce2.png";
        //auto atlas = image_table["atlas_data"].get_or_create<sol::table>();
        auto scale = image_table["scale"].get_or_create<sol::table>();
        scale["width"] = 32;
        scale["height"] = 32;
        scale["filter"] = 1; // nearest
        image_table["type"] = 0; // тут индекс
        image_table["sampler"] = 1;
        utils::add_image(image_table);
      }
      
      {
        auto image_table = global::get<sol::state>()->create_table();
        image_table["id"] = "hero_img";
        image_table["path"] = "apates_quest/textures/armies_and_heroes/KnightHorseback7_128_nearest.png";
        //auto atlas = image_table["atlas_data"].get_or_create<sol::table>();
//         auto scale = image_table["scale"].get_or_create<sol::table>();
//         scale["width"] = 32;
//         scale["height"] = 32;
//         scale["filter"] = 1; // nearest
        image_table["type"] = 0; // тут индекс
        image_table["sampler"] = 1;
        utils::add_image(image_table);
      }
      
//       {
//         auto image_table = global::get<sol::state>()->create_table();
//         image_table["id"] = "test_img2";
//         image_table["path"] = "apates_quest/textures/biomes/upper_tree.png";
//         //auto atlas = image_table["atlas_data"].get_or_create<sol::table>();
//         auto scale = image_table["scale"].get_or_create<sol::table>();
//         scale["width"] = 32;
//         scale["height"] = 32;
//         scale["filter"] = 1; // nearest
//         image_table["type"] = 0; // тут индекс
//         image_table["sampler"] = 0;
//         utils::add_image(image_table);
//       }
//       
//       {
//         auto image_table = global::get<sol::state>()->create_table();
//         image_table["id"] = "test_img3";
//         image_table["path"] = "apates_quest/textures/biomes/upper_tree.png";
//         //auto atlas = image_table["atlas_data"].get_or_create<sol::table>();
//         image_table["type"] = 0; // тут индекс
//         image_table["sampler"] = 0;
//         utils::add_image(image_table);
//       }
//       
//       {
//         auto image_table = global::get<sol::state>()->create_table();
//         image_table["id"] = "test_img4";
//         image_table["path"] = "apates_quest/textures/biomes/upper_tree.png";
//         //auto atlas = image_table["atlas_data"].get_or_create<sol::table>();
//         auto scale = image_table["scale"].get_or_create<sol::table>();
//         scale["width"] = 512;
//         scale["height"] = 512;
//         scale["filter"] = 1; // nearest
//         image_table["type"] = 0; // тут индекс
//         image_table["sampler"] = 0;
//         utils::add_image(image_table);
//       }

      // нужно с цветами немного поработать, слишком яркие, более менее нашел текстурки для биомов
      // почему то некоторые изображения плохо грузятся (например, upper_tree6.png)
      {
        auto biome_table = global::get<sol::state>()->create_table();
//         biome_table["texture"] = "hex_water";
        biome_table["color"] = render::make_color(0.2f, 0.2f, 0.8f, 1.0f).container;
        //biome_table["color"] = render::make_color(0.545f, 0.882f, 0.922f, 1.0f).container;
        biome_table["object1"] = sol::nil;
        biome_table["object2"] = sol::nil;
        biome_table["min_scale1"] = 0.0f;
        biome_table["max_scale1"] = 0.0f;
        biome_table["min_scale2"] = 0.1f;
        biome_table["max_scale2"] = 0.3f;
        biome_table["density"] = 9.0f;
        biome_table["height1"] = 0.0f;
        biome_table["height2"] = 0.0f;
        // может быть добавится множитель uv координат
        // + может быть добавятся флаги
        const uint32_t biome_ocean = ctx->seasons->add_biome(biome_table);
        ASSERT(biome_ocean == render::biome_ocean);
        UNUSED_VARIABLE(biome_ocean);
      }
      
      {
        auto biome_table = global::get<sol::state>()->create_table();
//         biome_table["texture"] = "hex_cold_water";
        biome_table["color"] = render::make_color(0.8f, 0.8f, 1.0f, 1.0f).container;
        biome_table["object1"] = sol::nil;
        biome_table["object2"] = sol::nil;
        biome_table["min_scale1"] = 0.0f;
        biome_table["max_scale1"] = 0.0f;
        biome_table["min_scale2"] = 0.0f;
        biome_table["max_scale2"] = 0.0f;
        biome_table["density"] = 0.0f;
        biome_table["height1"] = 0.0f;
        biome_table["height2"] = 0.0f;
        const uint32_t biome_ocean = ctx->seasons->add_biome(biome_table);
        ASSERT(biome_ocean == render::biome_ocean_glacier);
        UNUSED_VARIABLE(biome_ocean);
      }
      
      {
        auto biome_table = global::get<sol::state>()->create_table();
//         biome_table["texture"] = "hex_desert";
        biome_table["color"] = render::make_color(0.914f, 0.914f, 0.2f, 1.0f).container;
        biome_table["object1"] = sol::nil;
        biome_table["object2"] = sol::nil;
        biome_table["min_scale1"] = 0.0f;
        biome_table["max_scale1"] = 0.0f;
        biome_table["min_scale2"] = 0.0f;
        biome_table["max_scale2"] = 0.0f;
        biome_table["density"] = 0.0f;
        biome_table["height1"] = 0.0f;
        biome_table["height2"] = 0.0f;
        const uint32_t biome_ocean = ctx->seasons->add_biome(biome_table);
        ASSERT(biome_ocean == render::biome_desert);
        UNUSED_VARIABLE(biome_ocean);
      }
      
      {
        auto biome_table = global::get<sol::state>()->create_table();
//         biome_table["texture"] = "hex_grass1";
        biome_table["color"] = render::make_color(0.0f, 0.7f, 0.2f, 1.0f).container;
        biome_table["object1"] = sol::nil;
        biome_table["object2"] = "rain_tree";
        biome_table["min_scale1"] = 0.0f;
        biome_table["max_scale1"] = 0.0f;
        biome_table["min_scale2"] = 0.3f;
        biome_table["max_scale2"] = 1.0f;
        biome_table["density"] = 9.0f;
        biome_table["height1"] = 0.0f;
        biome_table["height2"] = 0.0f;
        const uint32_t biome_ocean = ctx->seasons->add_biome(biome_table);
        ASSERT(biome_ocean == render::biome_rain_forest);
        UNUSED_VARIABLE(biome_ocean);
      }
      
      {
        auto biome_table = global::get<sol::state>()->create_table();
        biome_table["texture"] = sol::nil;
        biome_table["color"] = render::make_color(1.0f, 0.2f, 0.2f, 1.0f).container;
        biome_table["object1"] = sol::nil;
        biome_table["object2"] = sol::nil;
        biome_table["min_scale1"] = 0.0f;
        biome_table["max_scale1"] = 0.0f;
        biome_table["min_scale2"] = 0.0f;
        biome_table["max_scale2"] = 0.0f;
        biome_table["density"] = 0.0f;
        biome_table["height1"] = 0.0f;
        biome_table["height2"] = 0.0f;
        const uint32_t biome_ocean = ctx->seasons->add_biome(biome_table);
        ASSERT(biome_ocean == render::biome_rocky);
        UNUSED_VARIABLE(biome_ocean);
      }
      
      {
        auto biome_table = global::get<sol::state>()->create_table();
//         biome_table["texture"] = "hex_grass2";
        biome_table["color"] = render::make_color(0.553f, 0.769f, 0.208f, 1.0f).container;
        biome_table["object1"] = sol::nil;
        biome_table["object2"] = sol::nil;
        biome_table["min_scale1"] = 0.0f;
        biome_table["max_scale1"] = 0.0f;
        biome_table["min_scale2"] = 0.0f;
        biome_table["max_scale2"] = 0.0f;
        biome_table["density"] = 0.0f;
        biome_table["height1"] = 0.0f;
        biome_table["height2"] = 0.0f;
        const uint32_t biome_ocean = ctx->seasons->add_biome(biome_table);
        ASSERT(biome_ocean == render::biome_plains);
        UNUSED_VARIABLE(biome_ocean);
      }
      
      {
        auto biome_table = global::get<sol::state>()->create_table();
        biome_table["texture"] = sol::nil;
        biome_table["color"] = render::make_color(0.0f, 1.0f, 0.0f, 1.0f).container;
        biome_table["object1"] = sol::nil;
        biome_table["object2"] = sol::nil;
        biome_table["min_scale1"] = 0.0f;
        biome_table["max_scale1"] = 0.0f;
        biome_table["min_scale2"] = 0.0f;
        biome_table["max_scale2"] = 0.0f;
        biome_table["density"] = 0.0f;
        biome_table["height1"] = 0.0f;
        biome_table["height2"] = 0.0f;
        const uint32_t biome_ocean = ctx->seasons->add_biome(biome_table);
        ASSERT(biome_ocean == render::biome_swamp);
        UNUSED_VARIABLE(biome_ocean);
      }
      
      {
        auto biome_table = global::get<sol::state>()->create_table();
//         biome_table["texture"] = "hex_grass2";
//         biome_table["color"] = render::make_color(0.2f, 1.0f, 0.2f, 1.0f).container;
//         biome_table["color"] = render::make_color(0.412f, 0.757f, 0.153f, 1.0f).container;
        biome_table["color"] = render::make_color(0.553f, 0.769f, 0.208f, 1.0f).container;
        biome_table["object1"] = sol::nil;
        biome_table["object2"] = sol::nil;
        biome_table["min_scale1"] = 0.0f;
        biome_table["max_scale1"] = 0.0f;
        biome_table["min_scale2"] = 0.5f;
        biome_table["max_scale2"] = 1.0f;
        biome_table["density"] = 4.0f;
        biome_table["height1"] = 0.0f;
        biome_table["height2"] = 0.0f;
        const uint32_t biome_ocean = ctx->seasons->add_biome(biome_table);
        ASSERT(biome_ocean == render::biome_grassland);
        UNUSED_VARIABLE(biome_ocean);
      }
      
      {
        auto biome_table = global::get<sol::state>()->create_table();
//         biome_table["texture"] = "hex_grass1";
        biome_table["color"] = render::make_color(0.0f, 0.8f, 0.0f, 1.0f).container;
        biome_table["object1"] = sol::nil;
        biome_table["object2"] = "deciduous_tree";
        biome_table["min_scale1"] = 0.0f;
        biome_table["max_scale1"] = 0.0f;
        biome_table["min_scale2"] = 0.4f;
        biome_table["max_scale2"] = 1.0f;
        biome_table["density"] = 6.0f;
        biome_table["height1"] = 0.0f;
        biome_table["height2"] = 0.0f;
        const uint32_t biome_ocean = ctx->seasons->add_biome(biome_table);
        ASSERT(biome_ocean == render::biome_deciduous_forest);
        UNUSED_VARIABLE(biome_ocean);
      }
      
      {
        auto biome_table = global::get<sol::state>()->create_table();
        biome_table["texture"] = sol::nil;
        biome_table["color"] = render::make_color(0.6f, 0.6f, 0.6f, 1.0f).container;
        biome_table["object1"] = sol::nil;
        biome_table["object2"] = sol::nil;
        biome_table["min_scale1"] = 0.0f;
        biome_table["max_scale1"] = 0.0f;
        biome_table["min_scale2"] = 0.0f;
        biome_table["max_scale2"] = 0.0f;
        biome_table["density"] = 0.0f;
        biome_table["height1"] = 0.0f;
        biome_table["height2"] = 0.0f;
        const uint32_t biome_ocean = ctx->seasons->add_biome(biome_table);
        ASSERT(biome_ocean == render::biome_tundra);
        UNUSED_VARIABLE(biome_ocean);
      }
      
      {
        auto biome_table = global::get<sol::state>()->create_table();
//         biome_table["texture"] = "hex_snow";
        biome_table["color"] = render::make_color(0.914f, 0.988f, 1.0f, 1.0f).container;
        biome_table["object1"] = sol::nil;
        biome_table["object2"] = sol::nil;
        biome_table["min_scale1"] = 0.0f;
        biome_table["max_scale1"] = 0.0f;
        biome_table["min_scale2"] = 0.0f;
        biome_table["max_scale2"] = 0.0f;
        biome_table["density"] = 0.0f;
        biome_table["height1"] = 0.0f;
        biome_table["height2"] = 0.0f;
        const uint32_t biome_ocean = ctx->seasons->add_biome(biome_table);
        ASSERT(biome_ocean == render::biome_land_glacier);
        UNUSED_VARIABLE(biome_ocean);
      }
      
{
        auto biome_table = global::get<sol::state>()->create_table();
//         biome_table["texture"] = "hex_grass1";
        biome_table["color"] = render::make_color(0.0f, 0.6f, 0.0f, 1.0f).container;
        biome_table["object1"] = sol::nil;
        biome_table["object2"] = "coniferous_tree";
        biome_table["min_scale1"] = 0.0f;
        biome_table["max_scale1"] = 0.0f;
        biome_table["min_scale2"] = 0.5f;
        biome_table["max_scale2"] = 1.2f;
        biome_table["density"] = 9.0f;
        biome_table["height1"] = 0.0f;
        biome_table["height2"] = 0.0f;
        const uint32_t biome_ocean = ctx->seasons->add_biome(biome_table);
        ASSERT(biome_ocean == render::biome_conifer_forest);
        UNUSED_VARIABLE(biome_ocean);
      }
      
      {
        auto biome_table = global::get<sol::state>()->create_table();
        biome_table["texture"] = sol::nil;
        biome_table["color"] = render::make_color(0.2f, 0.2f, 0.2f, 1.0f).container;
        biome_table["object1"] = sol::nil;
        biome_table["object2"] = sol::nil;
        biome_table["min_scale1"] = 0.0f;
        biome_table["max_scale1"] = 0.0f;
        biome_table["min_scale2"] = 0.0f;
        biome_table["max_scale2"] = 0.0f;
        biome_table["density"] = 0.0f;
        biome_table["height1"] = 0.0f;
        biome_table["height2"] = 0.0f;
        const uint32_t biome_ocean = ctx->seasons->add_biome(biome_table);
        ASSERT(biome_ocean == render::biome_mountain);
        UNUSED_VARIABLE(biome_ocean);
      }
      
      {
        auto biome_table = global::get<sol::state>()->create_table();
//         biome_table["texture"] = "hex_snow";
//         biome_table["color"] = render::make_color(1.0f, 1.0f, 1.0f, 1.0f).container;
        biome_table["color"] = render::make_color(0.914f, 0.988f, 1.0f, 1.0f).container;
        biome_table["object1"] = sol::nil;
        biome_table["object2"] = sol::nil;
        biome_table["min_scale1"] = 0.0f;
        biome_table["max_scale1"] = 0.0f;
        biome_table["min_scale2"] = 0.0f;
        biome_table["max_scale2"] = 0.0f;
        biome_table["density"] = 0.0f;
        biome_table["height1"] = 0.0f;
        biome_table["height2"] = 0.0f;
        const uint32_t biome_ocean = ctx->seasons->add_biome(biome_table);
        ASSERT(biome_ocean == render::biome_snowy_mountain);
        UNUSED_VARIABLE(biome_ocean);
      }

      ctx->seasons->create_biome(render::biome_ocean, {
        { GPU_UINT_MAX }, render::make_color(0.2f, 0.2f, 0.8f, 1.0f), { GPU_UINT_MAX }, { GPU_UINT_MAX },
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
      });

      ctx->seasons->create_biome(render::biome_ocean_glacier, {
        { GPU_UINT_MAX }, render::make_color(0.8f, 0.8f, 1.0f, 1.0f), { GPU_UINT_MAX }, { GPU_UINT_MAX },
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
      });

      ctx->seasons->create_biome(render::biome_desert, {
        { GPU_UINT_MAX }, render::make_color(1.0f, 1.0f, 0.0f, 1.0f), { GPU_UINT_MAX }, { GPU_UINT_MAX },
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
      });

      ctx->seasons->create_biome(render::biome_rain_forest, {
        { GPU_UINT_MAX }, render::make_color(0.0f, 0.7f, 0.2f, 1.0f), { GPU_UINT_MAX }, { GPU_UINT_MAX },
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
      });

      ctx->seasons->create_biome(render::biome_rocky, {
        { GPU_UINT_MAX }, render::make_color(1.0f, 0.2f, 0.2f, 1.0f), { GPU_UINT_MAX }, { GPU_UINT_MAX },
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
      });

      ctx->seasons->create_biome(render::biome_plains, {
        { GPU_UINT_MAX }, render::make_color(0.0f, 1.0f, 0.2f, 1.0f), { GPU_UINT_MAX }, { GPU_UINT_MAX },
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
      });

      ctx->seasons->create_biome(render::biome_swamp, {
        { GPU_UINT_MAX }, render::make_color(0.0f, 1.0f, 0.0f, 1.0f), { GPU_UINT_MAX }, { GPU_UINT_MAX },
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
      });

      ctx->seasons->create_biome(render::biome_grassland, {
        { GPU_UINT_MAX }, render::make_color(0.2f, 1.0f, 0.2f, 1.0f), { GPU_UINT_MAX }, { GPU_UINT_MAX },
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
      });

      ctx->seasons->create_biome(render::biome_deciduous_forest, {
        { GPU_UINT_MAX }, render::make_color(0.0f, 0.8f, 0.0f, 1.0f), { GPU_UINT_MAX }, { GPU_UINT_MAX },
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
      });

      ctx->seasons->create_biome(render::biome_tundra, {
        { GPU_UINT_MAX }, render::make_color(0.6f, 0.6f, 0.6f, 1.0f), { GPU_UINT_MAX }, { GPU_UINT_MAX },
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
      });

      ctx->seasons->create_biome(render::biome_land_glacier, {
        { GPU_UINT_MAX }, render::make_color(0.9f, 0.9f, 0.9f, 1.0f), { GPU_UINT_MAX }, { GPU_UINT_MAX },
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
      });

      ctx->seasons->create_biome(render::biome_conifer_forest, {
        { GPU_UINT_MAX }, render::make_color(0.0f, 0.6f, 0.0f, 1.0f), { GPU_UINT_MAX }, { GPU_UINT_MAX },
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
      });

      ctx->seasons->create_biome(render::biome_mountain, {
        { GPU_UINT_MAX }, render::make_color(0.2f, 0.2f, 0.2f, 1.0f), { GPU_UINT_MAX }, { GPU_UINT_MAX },
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
      });

      ctx->seasons->create_biome(render::biome_snowy_mountain, {
        { GPU_UINT_MAX }, render::make_color(1.0f, 1.0f, 1.0f, 1.0f), { GPU_UINT_MAX }, { GPU_UINT_MAX },
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
      });

      std::vector<uint32_t> tile_biome(ctx->map->tiles_count(), UINT32_MAX);
      utils::submit_works_async(ctx->pool, ctx->map->tiles_count(), [&tile_biome] (const size_t &start, const size_t &count, const generator::context* ctx) {
        for (size_t i = start; i < start+count; ++i) {
          const uint32_t tile_index = i;
          const float elevation = ctx->container->get_data<float>(debug::entities::tile, tile_index, debug::properties::tile::elevation);
          const float temperature = ctx->container->get_data<float>(debug::entities::tile, tile_index, debug::properties::tile::heat);
          const float wetness = ctx->container->get_data<float>(debug::entities::tile, tile_index, debug::properties::tile::moisture);
          const uint32_t biome_id = calcutate_biome2(elevation, temperature, wetness);
          ASSERT(biome_id != UINT32_MAX);
          tile_biome[tile_index] = biome_id;
          ctx->seasons->set_tile_biome(tile_index, biome_id);
        }
      }, ctx);
      utils::async_wait(ctx->pool);

      for (size_t i = 0; i < tile_biome.size(); ++i) {
        ctx->container->set_data<uint32_t>(debug::entities::tile, i, debug::properties::tile::biome, tile_biome[i]);
        const auto &biome = ctx->seasons->biomes[tile_biome[i]];
        ctx->map->set_tile_color(i, biome.color);
      }
    }

    void generate_provinces(generator::context* ctx, sol::table &table) {
      utils::time_log log("generating provinces");

      // провинции - это 100% вороной
      // мне нужно только определить сколько провинций сгенерировать
      // и самое главное как правильно выбрать точки
      // необходимо выбрать точки так что провинции были желательно на некотором расстоянии
      // может быть нужно по одно провинции делать? но так может остаться крайне много пустот
      // если случайно раскидать точки по земле, то наверное получится даже более менее
      // с диаграмой вороного должно получится более менее

      // че с весами у тайлов
      // нужно ли "распространять" людей?
      // честно говоря я не уверен что у меня получится более менее правильно это сделать
      // тут задача скорее не людей распространить
      // а прикинуть примерно где может появиться скопление людей само по себе?
      // благоприятные территории... это какие?
      // этих территорий довольно много и они сильно зависят от текущуего биома
      // продолжительные равнины - кочевники, с другой стороны кочевники не могут появиться
      // среди каких то уже развитых феодальных государств
      // с другой стороны величина провинции не зависит от того кто на ней проживает
      // поэтому будет разумно расчитать величину по благоприятности
      // вот что нужно сделать:
      // найти все острова и континенты
      // (острова - размер суши меньше константы, континенты - размер суши больше константы)
      // расчитать примерные веса всех тайлов по биомам
      // сгененировать много точек на суше, так чтобы на островах максимум одна точка, не генерить на островах меньше там 25
      // построить диаграмму воронного (нужно не забывать о максимальном/минимальном количестве)
      // (хотя если более менее правильно развесовать это не потребуется)
      // оставшиеся бесхозные тайлы пихнуть в ближайшие провинции

      // 4000 провинций означает что провинции будут стоять достаточно плотно
      // сомневаюсь что есть необходимость как то изменять плотность провинций
      // в крусайдер кингсе 1600 провинций (там без учета китая, америки, африки)
      // в европке 3042 провинции
      // в вархаммере больше 300 поселений (я бы сказал около 400)
      // так что возможно нужно будет еще уменьшить количество провинций
      // либо добавить специальные провинции, либо и то и то
      // мне нужно чтобы у меня еще было место на специальные постройки на карте
      // возможно количество провинций нужно как то расчитать
      // например: количество хороших тайлов / 40 (50?)
      // мне нужно задать еще и необитаемые территории, например пустыня и тундра

      std::vector<uint32_t> tile_pool(ctx->map->tiles_count(), UINT32_MAX);

      uint32_t water_tiles = 0;
      uint32_t pool_id = 0;
      for (size_t i = 0; i < ctx->map->tiles_count(); ++i) {
        const float h = ctx->container->get_data<float>(debug::entities::tile, i, debug::properties::tile::elevation);
        if (h < 0.0f) continue;
        ++water_tiles;
        if (tile_pool[i] != UINT32_MAX) continue;

        const uint32_t current_pool_id = pool_id;
        ++pool_id;

        tile_pool[i] = current_pool_id;
        std::queue<uint32_t> queue;
        queue.push(i);

        while (!queue.empty()) {
          const uint32_t current_tile = queue.front();
          queue.pop();

          const auto &tile_data = render::unpack_data(ctx->map->get_tile(current_tile));
          const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
          for (uint32_t j = 0; j < n_count; ++j) {
            const uint32_t n_index = tile_data.neighbours[j];
            const float h = ctx->container->get_data<float>(debug::entities::tile, n_index, debug::properties::tile::elevation);
            if (h < 0.0f) continue;
            if (tile_pool[n_index] == UINT32_MAX) {
              tile_pool[n_index] = current_pool_id;
              queue.push(n_index);
            }
          }
        }
      }

      std::vector<std::vector<uint32_t>> ground_pools(pool_id);
      for (size_t i = 0; i < ctx->map->tiles_count(); ++i) {
        const uint32_t pool_index = tile_pool[i];
        if (pool_index == UINT32_MAX) continue;
        ground_pools[pool_index].push_back(i);
      }

      // нужно будет еще парочку раз взглянуть на этот способ (с помощью весов)
      std::vector<std::pair<uint32_t, uint32_t>> tile_weights(ctx->map->tiles_count(), std::make_pair(0, 0));
      uint32_t good_tiles_count = 0;
      for (size_t i = 0; i < ctx->map->tiles_count(); ++i) {
        const uint32_t current_tile = i;
        const uint32_t biom_id = ctx->container->get_data<uint32_t>(debug::entities::tile, current_tile, debug::properties::tile::biome);
        const float height = ctx->container->get_data<float>(debug::entities::tile, current_tile, debug::properties::tile::elevation);

        uint32_t biom_points = 0;
        switch (biom_id) {
          case render::biome_ocean:            biom_points = 0; break;
          case render::biome_ocean_glacier:    biom_points = 0; break;
          case render::biome_land_glacier:     biom_points = 0; break;
          case render::biome_mountain:         biom_points = 0; break;
          case render::biome_snowy_mountain:   biom_points = 0; break;

          case render::biome_desert:           biom_points = 33; break;
          case render::biome_tundra:           biom_points = 33; break;

          case render::biome_rocky:            biom_points = 66; break;
          case render::biome_plains:           biom_points = 66; break;
          case render::biome_swamp:            biom_points = 66; break;
          case render::biome_grassland:        biom_points = 66; break;

          case render::biome_rain_forest:      biom_points = 100; break;
          case render::biome_deciduous_forest: biom_points = 100; break;
          case render::biome_conifer_forest:   biom_points = 100; break;
          default: throw std::runtime_error("not implemented");
        }

        const float t = ctx->container->get_data<float>(debug::entities::tile, current_tile, debug::properties::tile::heat);
        // просто умножить? с другой стороны зачем это делать
        // биом дает довольно много информации
        //biom_points = biom_points * t;

        //good_tiles_count += uint32_t(biom_points > 0 && t > 0.15f);
        good_tiles_count += uint32_t(height >= 0.0f && t > 0.15f);

        tile_weights[current_tile] = std::make_pair(current_tile, biom_points);
      }

      const uint32_t provinces_count = table["userdata"]["provinces_count"];
      const uint32_t province_tiles_avg = float(good_tiles_count) / float(provinces_count);
      PRINT_VAR("good_tiles_count", good_tiles_count)
      PRINT_VAR("province_tiles_avg", province_tiles_avg)
      const float province_tiles_ratio = 0.75f;
      const uint32_t province_tiles_min = province_tiles_ratio * province_tiles_avg;
      const uint32_t province_tiles_max = (2.0f - province_tiles_ratio) * province_tiles_avg;
      PRINT_VAR("province_tiles_min", province_tiles_min)
      PRINT_VAR("province_tiles_max", province_tiles_max)

      std::vector<std::pair<uint32_t, uint32_t>> tile_weights_accum(tile_weights);
      uint32_t accum = 0;
      for (size_t i = 0; i < tile_weights_accum.size(); ++i) {
        accum += tile_weights_accum[i].second;
        tile_weights_accum[i].second = accum;
      }

      std::sort(tile_weights_accum.begin(), tile_weights_accum.end(), [] (const std::pair<uint32_t, uint32_t> &first, const std::pair<uint32_t, uint32_t> &second) -> bool {
        return first.second < second.second;
      });

      std::queue<uint32_t> voronoi_tiles;
//       std::vector<uint32_t> voronoi_tiles;
      std::unordered_set<uint32_t> unique_tiles;
      std::vector<std::pair<uint32_t, uint32_t>> tile_province(ctx->map->tiles_count(), std::make_pair(UINT32_MAX, UINT32_MAX));
      std::vector<uint32_t> pool_prov_count(ground_pools.size(), 0);
      std::vector<std::vector<uint32_t>> province_tiles(provinces_count);
      for (uint32_t i = 0; i < provinces_count; ++i) {
        uint32_t tile_index = UINT32_MAX;
        uint32_t attempts = 0;

        while (tile_index == UINT32_MAX && attempts < 100) {
//           const uint32_t rand_weight = context->data->engine->index(accum);
//           const uint32_t rand_index = binary_search(rand_weight, tile_weights_accum);
          const uint32_t rand_index = ctx->random->index(tile_weights_accum.size());
          ASSERT(rand_index < tile_weights_accum.size());
          const auto &pair = tile_weights_accum[rand_index];
          ++attempts;

          if (unique_tiles.find(pair.first) != unique_tiles.end()) continue;

          const float t = ctx->container->get_data<float>(debug::entities::tile, pair.first, debug::properties::tile::heat);
          if (t <= 0.15f) continue;

          const float h = ctx->container->get_data<float>(debug::entities::tile, pair.first, debug::properties::tile::elevation);
          if (h < 0.0f) continue;

          const uint32_t pool_index = tile_pool[pair.first];
          if (pool_index == UINT32_MAX) continue;
          if (ground_pools[pool_index].size() < (province_tiles_avg / 2)) continue;
          if (ground_pools[pool_index].size() < province_tiles_avg && pool_prov_count[pool_index] >= 1) continue;
          // если остров меньше чем четверть от минимального (максимального? среднего?) количества тайлов в провке
          // то пропускаем этот остров, если между этим значением и минимального (максимального? среднего?) количества тайлов
          // то на этом острове должно быть только одна провинция
          // иначе любое колиество провинций

          uint32_t dist_to_water_accum = 0;
          const auto &tile_data = render::unpack_data(ctx->map->get_tile(pair.first));
          const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
          bool found = true;
          for (uint32_t j = 0; j < n_count; ++j) {
            if (!found) break;

            const uint32_t n_index = tile_data.neighbours[j];
            if (unique_tiles.find(n_index) != unique_tiles.end()) {
              found = false;
              break;
            }

            const auto &tile_data = render::unpack_data(ctx->map->get_tile(n_index));
            const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
            for (uint32_t k = 0; k < n_count; ++k) {
              const uint32_t n_index = tile_data.neighbours[k];
              if (unique_tiles.find(n_index) != unique_tiles.end()) {
                found = false;
                break;
              }
            }

            const uint32_t dist_to_water = table["tiles"][n_index]["water_distance"]["dist"];
            dist_to_water_accum += dist_to_water;
          }

          if (dist_to_water_accum <= n_count) continue;
          if (!found) continue;

          pool_prov_count[pool_index] += 1;
          tile_index = pair.first;
        }

        //ASSERT(tile_index != UINT32_MAX);
        if (tile_index == UINT32_MAX) continue;
        unique_tiles.insert(tile_index);

        const auto &tile_data = render::unpack_data(ctx->map->get_tile(tile_index));
        const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
        for (uint32_t j = 0; j < n_count; ++j) {
          const uint32_t n_index = tile_data.neighbours[j];
          unique_tiles.insert(n_index);
          const auto &tile_data = render::unpack_data(ctx->map->get_tile(n_index));
          const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
          for (uint32_t k = 0; k < n_count; ++k) {
            const uint32_t n_index = tile_data.neighbours[k];
            unique_tiles.insert(n_index);
          }
        }

        voronoi_tiles.push(tile_index);
        tile_province[tile_index] = std::make_pair(i, 0);
        province_tiles[i].push_back(tile_index);
      }

      // возможно тут нужно постоянно сортировать
      // и сначало обрабатывать мелкие провинции
      // в этом нам должен помочь бинари хип
      // с бинари хипом выглядит не очень
//       const auto heap_func = [&province_tiles, &tile_province] (const uint32_t &index1, const uint32_t &index2) -> bool {
//         const uint32_t province_index1 = tile_province[index1].first;
//         const uint32_t province_index2 = tile_province[index2].first;
//         return province_tiles[province_index1].size() > province_tiles[province_index2].size();
//       };
//
//       std::make_heap(voronoi_tiles.begin(), voronoi_tiles.end(), heap_func);

      while (!voronoi_tiles.empty()) {
        const uint32_t current_tile = voronoi_tiles.front();
        voronoi_tiles.pop();
//         std::pop_heap(voronoi_tiles.begin(), voronoi_tiles.end(), heap_func);
//         const uint32_t current_tile = voronoi_tiles.back();
//         voronoi_tiles.pop_back();
//         const uint32_t rand_index = context->data->engine->index(voronoi_tiles.size());
//         const uint32_t current_tile = voronoi_tiles[rand_index];
//         voronoi_tiles[rand_index] = voronoi_tiles.back();
//         voronoi_tiles.pop_back();

        const uint32_t province_index = tile_province[current_tile].first;
        const auto &tile_data = render::unpack_data(ctx->map->get_tile(current_tile));
        const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
        for (uint32_t j = 0; j < n_count; ++j) {
          const uint32_t n_index = tile_data.neighbours[j];

          const float t = ctx->container->get_data<float>(debug::entities::tile, n_index, debug::properties::tile::heat);
          if (t <= 0.15f) continue;

          const float height = ctx->container->get_data<float>(debug::entities::tile, n_index, debug::properties::tile::elevation);
          if (height < 0.0f) continue;
          if (tile_province[n_index].second != UINT32_MAX) continue;
//           if (province_tiles[province_index].size() >= province_tiles_max) continue;

          tile_province[n_index] = std::make_pair(province_index, tile_province[current_tile].second + 1);
          province_tiles[province_index].push_back(n_index);

          voronoi_tiles.push(n_index);
//           std::push_heap(voronoi_tiles.begin(), voronoi_tiles.end(), heap_func);
        }
      }

//       for (size_t i = 0; i < tile_province.size(); ++i) {
//         const uint32_t province_index = tile_province[i].first;
//         if (province_index == UINT32_MAX) continue;
//
//         ASSERT(province_index < province_tiles.size());
//         province_tiles[province_index].push_back(i);
//       }

      // по идее мы должны избавиться от всех провинций у которых province_tiles[province_index].size() < province_tiles_min
      // все нерасходованные тайлы раздадим соседним провинциям или создадим новые
      // как создать новые? нужно также примерно пройтись по тайлам как в случае с островами, создать пулы свободных тайлов
      // по какой то случайности спавнится 2-3 тайла на островах, нужно будет позже проверить
//       for (size_t i = 0; i < province_tiles.size(); ++i) {
//         if (province_tiles[i].size() >= province_tiles_min) continue;
//
//         for (size_t j = 0; j < province_tiles[i].size(); ++j) {
//           const uint32_t tile_index = province_tiles[i][j];
//           tile_province[tile_index] = std::make_pair(UINT32_MAX-1, UINT32_MAX);
//         }
//
//         province_tiles[i].clear();
//       }

      // пытаемся соединить маленькие провинции
//       const uint32_t min_plates_count = 75;
      const uint32_t max_iterations = 15;
      uint32_t current_plates_count = table["final_plates_count"];
      uint32_t current_iter = 0;

      struct next_provinces_data {
        std::mutex mutex;
        std::set<uint32_t> neighbours;
      };

      std::vector<std::vector<uint32_t>> provinces_tiles_local(province_tiles);
      std::vector<std::pair<uint32_t, uint32_t>> tile_provinces_local(tile_province);

      while (current_iter < max_iterations) { // current_plates_count > min_plates_count &&
        std::vector<next_provinces_data> next_provinces(province_tiles.size());
        utils::submit_works_async(ctx->pool, ctx->map->tiles_count(), [&next_provinces, &tile_provinces_local] (const size_t &start, const size_t &count, const generator::context* context) {
          for (size_t i = start; i < start+count; ++i) {
            const auto &tile = render::unpack_data(context->map->get_tile(i));
            const uint32_t province1 = tile_provinces_local[i].first;
            if (province1 == UINT32_MAX) continue;
            for (uint32_t j = 0; j < 6; ++j) {
              const uint32_t tile_neighbour_index = tile.neighbours[j];
              if (tile_neighbour_index == UINT32_MAX) continue;

              const uint32_t province2 = tile_provinces_local[tile_neighbour_index].first;
              if (province2 == UINT32_MAX) continue;

              if (province1 != province2) {
                {
                  std::unique_lock<std::mutex> lock(next_provinces[province1].mutex);
                  next_provinces[province1].neighbours.insert(province2);
                }

                {
                  std::unique_lock<std::mutex> lock(next_provinces[province2].mutex);
                  next_provinces[province2].neighbours.insert(province1);
                }
              }
            }
          }
        }, ctx);
        ctx->pool->compute();
        while (ctx->pool->working_count() != 1 || ctx->pool->tasks_count() != 0) { std::this_thread::sleep_for(std::chrono::microseconds(1)); }

        uint32_t max_tiles_count = 0;
        uint32_t min_tiles_count = 121523152;
        for (uint32_t i = 0; i < provinces_tiles_local.size(); ++i) {
          const uint32_t tiles_count = provinces_tiles_local[i].size();
          if (tiles_count < 2) continue;
          //const uint32_t neighbours_count = next_plates[i].neighbours.size();

          max_tiles_count = std::max(max_tiles_count, tiles_count);
          min_tiles_count = std::min(min_tiles_count, tiles_count);
        }

        for (uint32_t i = 0; i < provinces_tiles_local.size(); ++i) {
          if (provinces_tiles_local[i].size() < 2) continue;
          if (provinces_tiles_local[i].size() >= province_tiles_min) continue;
//           if (!plates_union[i]) continue;

          // найдем соседей с которыми мы можем соединиться

          uint32_t province_index = UINT32_MAX;
          {
            for (auto idx : next_provinces[i].neighbours) {
              const uint32_t tiles_count = provinces_tiles_local[idx].size();
              if (tiles_count < 2) continue;
              if (tiles_count >= province_tiles_min) continue;
              province_index = idx;
              break;
            }
          }

          if (province_index == UINT32_MAX) continue;

          //ASSERT(false);

          //ASSERT(plate_tiles_local[plate_index].size() > 1);

          ASSERT(provinces_tiles_local[province_index].size() >= 2);
//           while (plate_tiles_local[plate_index].size() < 2) {
//             plate_index = plate_tiles_local[plate_index][0];
//           }

          provinces_tiles_local[i].insert(provinces_tiles_local[i].end(), provinces_tiles_local[province_index].begin(), provinces_tiles_local[province_index].end());
          provinces_tiles_local[province_index].clear();
          provinces_tiles_local[province_index].push_back(i);
          //plate_new_plate[plate_index] = i;
          --current_plates_count;
        }

        // обновляем индексы
        for (size_t i = 0; i < provinces_tiles_local.size(); ++i) {
          if (provinces_tiles_local[i].size() < 2) continue;

          for (size_t j = 0; j < provinces_tiles_local[i].size(); ++j) {
            tile_provinces_local[provinces_tiles_local[i][j]] = std::make_pair(i, UINT32_MAX);
          }
        }

        ++current_iter;
      }

      std::swap(province_tiles, provinces_tiles_local);
      std::swap(tile_province, tile_provinces_local);

      // нам необходимо разделить слишком большие провинции на несколько
      // на сколько? по минимальным значениям? или по средним?
      // лучше по средним, чем больше тайлов в провинции тем лучше
      // в этом случае мы сможем даже захватить какие нибудь вещи не входящие ни в какие провинции
      // (такие есть?)
      for (size_t i = 0; i < province_tiles.size(); ++i) {
        if (province_tiles[i].size() < 2) continue;
        if (province_tiles[i].size() < province_tiles_max) continue;

        //const uint32_t new_provinces_count = (province_tiles[i].size() < province_tiles_avg * 2) ? (province_tiles[i].size() / province_tiles_min) : (province_tiles[i].size() / province_tiles_avg);
        const uint32_t new_provinces_count = std::ceil(float(province_tiles[i].size()) / float(province_tiles_avg));
        ASSERT(new_provinces_count > 0);
        if (new_provinces_count == 1) continue;

        std::vector<uint32_t> tiles_array;
        std::swap(province_tiles[i], tiles_array);
        ASSERT(province_tiles[i].empty());
        for (size_t j = 0; j < tiles_array.size(); ++j) {
          const uint32_t tile_index = tiles_array[j];
          tile_province[tile_index] = std::make_pair(UINT32_MAX, UINT32_MAX);
        }

        std::unordered_set<uint32_t> unique_tiles;
        std::queue<uint32_t> queue;
        {
          uint32_t tile_index = UINT32_MAX;
          uint32_t attempts = 0;

          while (tile_index == UINT32_MAX && attempts < 100) {
            const uint32_t rand_index = ctx->random->index(tiles_array.size());
            ASSERT(rand_index < tiles_array.size());
            const uint32_t current_tile_index = tiles_array[rand_index];
            ++attempts;

            if (unique_tiles.find(current_tile_index) != unique_tiles.end()) continue;

            const float t = ctx->container->get_data<float>(debug::entities::tile, current_tile_index, debug::properties::tile::heat);
            if (t <= 0.15f) continue;

            const float height = ctx->container->get_data<float>(debug::entities::tile, current_tile_index, debug::properties::tile::elevation);
            if (height < 0.0f) continue;

            uint32_t dist_to_water_accum = 0;
            const auto &tile_data = render::unpack_data(ctx->map->get_tile(current_tile_index));
            const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
            bool found = true;
            for (uint32_t j = 0; j < n_count; ++j) {
              if (!found) break;

              const uint32_t n_index = tile_data.neighbours[j];
              if (unique_tiles.find(n_index) != unique_tiles.end()) {
                found = false;
                break;
              }

              const auto &tile_data = render::unpack_data(ctx->map->get_tile(n_index));
              const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
              for (uint32_t k = 0; k < n_count; ++k) {
                const uint32_t n_index = tile_data.neighbours[k];
                if (unique_tiles.find(n_index) != unique_tiles.end()) {
                  found = false;
                  break;
                }
              }

              const uint32_t dist_to_water = table["tiles"][n_index]["water_distance"]["dist"];
              dist_to_water_accum += dist_to_water;
            }

//             if (dist_to_water_accum <= n_count) continue;
            if (!found) continue;

            tile_index = current_tile_index;
          }

          ASSERT(tile_index != UINT32_MAX);

          province_tiles[i].push_back(tile_index);
          tile_province[tile_index] = std::make_pair(i, 0);
          queue.push(tile_index);
          unique_tiles.insert(tile_index);
        }

        for (size_t j = 1; j < new_provinces_count; ++j) {
          uint32_t tile_index = UINT32_MAX;
          uint32_t attempts = 0;

          while (tile_index == UINT32_MAX && attempts < 100) {
  //           const uint32_t rand_weight = context->data->engine->index(accum);
  //           const uint32_t rand_index = binary_search(rand_weight, tile_weights_accum);
            const uint32_t rand_index = ctx->random->index(tiles_array.size());
            ASSERT(rand_index < tiles_array.size());
            const uint32_t current_tile_index = tiles_array[rand_index];
            ++attempts;

            if (unique_tiles.find(current_tile_index) != unique_tiles.end()) continue;

            const float t = ctx->container->get_data<float>(debug::entities::tile, current_tile_index, debug::properties::tile::heat);
            if (t <= 0.15f) continue;

            const float height = ctx->container->get_data<float>(debug::entities::tile, current_tile_index, debug::properties::tile::elevation);
            if (height < 0.0f) continue;

            uint32_t dist_to_water_accum = 0;
            const auto &tile_data = render::unpack_data(ctx->map->get_tile(current_tile_index));
            const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
            bool found = true;
            for (uint32_t j = 0; j < n_count; ++j) {
              if (!found) break;

              const uint32_t n_index = tile_data.neighbours[j];
              if (unique_tiles.find(n_index) != unique_tiles.end()) {
                found = false;
                break;
              }

              const auto &tile_data = render::unpack_data(ctx->map->get_tile(n_index));
              const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
              for (uint32_t k = 0; k < n_count; ++k) {
                const uint32_t n_index = tile_data.neighbours[k];
                if (unique_tiles.find(n_index) != unique_tiles.end()) {
                  found = false;
                  break;
                }
              }

              const uint32_t dist_to_water = table["tiles"][n_index]["water_distance"]["dist"];
              dist_to_water_accum += dist_to_water;
            }

//             if (dist_to_water_accum <= n_count) continue;
            if (!found) continue;

            tile_index = current_tile_index;
          }

          ASSERT(tile_index != UINT32_MAX);

          const uint32_t new_province_index = province_tiles.size();
          province_tiles.emplace_back();
          province_tiles[new_province_index].push_back(tile_index);
          tile_province[tile_index] = std::make_pair(new_province_index, 0);
          queue.push(tile_index);
          unique_tiles.insert(tile_index);
        }

        while (!queue.empty()) {
          const uint32_t current_tile = queue.front();
          queue.pop();
  //         std::pop_heap(queue.begin(), queue.end(), heap_func);
  //         const uint32_t current_tile = queue.back();
  //         queue.pop_back();
  //         const uint32_t rand_index = context->data->engine->index(queue.size());
  //         const uint32_t current_tile = queue[rand_index];
  //         queue[rand_index] = queue.back();
  //         queue.pop_back();

          const uint32_t province_index = tile_province[current_tile].first;
          const auto &tile_data = render::unpack_data(ctx->map->get_tile(current_tile));
          const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
          for (uint32_t j = 0; j < n_count; ++j) {
            const uint32_t n_index = tile_data.neighbours[j];

            const float t = ctx->container->get_data<float>(debug::entities::tile, n_index, debug::properties::tile::heat);
            if (t <= 0.15f) continue;

            const float height = ctx->container->get_data<float>(debug::entities::tile, n_index, debug::properties::tile::elevation);
            if (height < 0.0f) continue;
            if (tile_province[n_index].first != UINT32_MAX) continue;

            tile_province[n_index] = std::make_pair(province_index, tile_province[current_tile].second + 1);
            province_tiles[province_index].push_back(n_index);

            queue.push(n_index);
  //           std::push_heap(queue.begin(), queue.end(), heap_func);
          }
        }
      }

      // если у нас по прежнему остаются провинции с малым количеством тайлов
      // то нужно позаимствовать их у соседа с большим количеством тайлов
      // провинции-коммунисты

      // как заимствовать?
      // нужно найти соседей с количеством тайлов минимум в два раза большим?
      // или соседей с количеством больше avg? или больше макс?
      // третий вариант самый адекватный, но с другой стороны
      // таких соседей может и не быть
      for (size_t i = 0; i < province_tiles.size(); ++i) {
        if (province_tiles[i].size() < 2) continue;
        if (province_tiles[i].size() >= province_tiles_min) continue;

        std::vector<std::pair<uint32_t, uint32_t>> border;
        for (size_t j = 0; j < province_tiles[i].size(); ++j) {
          const uint32_t current_tile_index = province_tiles[i][j];
          const uint32_t province_index1 = tile_province[current_tile_index].first;
          ASSERT(i == province_index1);
          const auto &tile = render::unpack_data(ctx->map->get_tile(current_tile_index));
          const uint32_t n_count = render::is_pentagon(tile) ? 5 : 6;
          for (uint32_t k = 0; k < n_count; ++k) {
            const uint32_t n_index = tile.neighbours[k];
            const uint32_t province_index2 = tile_province[n_index].first;
            if (province_index2 == UINT32_MAX) continue;

            if (province_index1 != province_index2) border.push_back(std::make_pair(current_tile_index, n_index));
          }
        }

        size_t current_index = 0;
        while (province_tiles[i].size() < province_tiles_min && current_index < border.size()) {
          const size_t local_index = current_index;
          ++current_index;

          const auto &pair = border[local_index];
          //const uint32_t opposite_index = pair.first == i ? pair.second : pair.first;
          const uint32_t opposite_index = tile_province[pair.second].first;
          //ASSERT(i != opposite_index);

          if (opposite_index == i) continue;
          if (province_tiles[opposite_index].size() <= province_tiles_avg) continue;

          const uint32_t choosen_tile_index = pair.second;
          for (size_t j = 0; j < province_tiles[opposite_index].size(); ++j) {
            if (province_tiles[opposite_index][j] == choosen_tile_index) {
              province_tiles[opposite_index][j] = province_tiles[opposite_index].back();
              province_tiles[opposite_index].pop_back();
              break;
            }
          }

          province_tiles[i].push_back(choosen_tile_index);
          tile_province[choosen_tile_index].first = i;

          const auto &tile_data = render::unpack_data(ctx->map->get_tile(choosen_tile_index));
          const uint32_t n_count = render::is_pentagon(tile_data);
          for (uint32_t j = 0; j < n_count; ++j) {
            const uint32_t n_index = tile_data.neighbours[j];
            const uint32_t province_index2 = tile_province[n_index].first;
            if (province_index2 == UINT32_MAX) continue;
            if (i != province_index2) {
              border.push_back(std::make_pair(choosen_tile_index, n_index));
            }
          }
        }
      }

      // всякая информация
      uint32_t province_min = 10000;
      uint32_t province_max = 0;
      uint32_t count = 0;
      uint32_t count_more_max = 0;
      uint32_t count_less_min = 0;
      uint32_t accum_max = 0;
      uint32_t accum_min = 0;
      uint32_t accum_tiles_count = 0;
      for (size_t i = 0; i < province_tiles.size(); ++i) {
        if (province_tiles[i].size() == 0) continue;
        if (province_tiles[i].size() == 1) continue;
        province_min = std::min(uint32_t(province_tiles[i].size()), province_min);
        province_max = std::max(uint32_t(province_tiles[i].size()), province_max);
        ++count;
        accum_tiles_count += province_tiles[i].size();

        if (province_tiles[i].size() > province_tiles_max) {
          ++count_more_max;
          accum_max += province_tiles[i].size();
        }

        if (province_tiles[i].size() < province_tiles_min) {
          ++count_less_min;
          accum_min += province_tiles[i].size();
        }
      }

      for (size_t i = 0; i < ctx->map->tiles_count(); ++i) {
        ctx->container->set_data<uint32_t>(debug::entities::tile, i, debug::properties::tile::province_index, UINT32_MAX);
      }

      // в контейнере переход от провинции к тайлу держать совсем не обязательно
      // более важно держать соседей провинции

      PRINT_VAR("provinces     ", count)
//       for (size_t i = 0; i < province_tiles.size(); ++i) {
//         if (province_tiles[i].size() == 0) continue;
//         if (province_tiles[i].size() == 1) continue;
//         context->province_tile.emplace_back();
//         std::swap(context->province_tile.back(), province_tiles[i]);
// //         for (size_t j = 0; j < province_tiles[i].size(); ++j) {
// //
// //         }
//       }

      // нужно придумать что то с этим
      // должен быть более определенный способ получить провинции
      // ну то есть желательно засунуть это в контейнер
      // вообще было бы неплохо наверное строго определить все нужные для игры сущности
//       table["provinces_count"] = count;
//       table["provinces"].get_or_create<sol::table>();
//       size_t counter = 0;
//       for (size_t i = 0; i < province_tiles.size(); ++i) {
//         if (province_tiles[i].size() == 0) continue;
//         if (province_tiles[i].size() == 1) continue;
//
//         table["provinces"][counter].get_or_create<sol::table>();
//         table["provinces"][counter]["tiles_count"] = province_tiles[i].size();
//         table["provinces"][counter]["tiles"].get_or_create<sol::table>();
//         for (size_t j = 0; j < province_tiles[i].size(); ++j) {
//           table["provinces"][counter]["tiles"][j] = province_tiles[i][j];
//         }
//
//         ++counter;
//       }

      ctx->container->set_entity_count(debug::entities::province, count);
      size_t counter = 0;
      for (size_t i = 0; i < province_tiles.size(); ++i) {
        if (province_tiles[i].size() == 0) continue;
        if (province_tiles[i].size() == 1) continue;

        for (size_t j = 0; j < province_tiles[i].size(); ++j) {
          ctx->container->add_child(debug::entities::province, counter, province_tiles[i][j]);
        }
        ++counter;
      }

      //context->tile_province.resize(tile_province.size(), UINT32_MAX);
      counter = 0;
      for (size_t i = 0; i < province_tiles.size(); ++i) {
        if (province_tiles[i].size() == 0) continue;
        if (province_tiles[i].size() == 1) continue;

        for (size_t j = 0; j < province_tiles[i].size(); ++j) {
          const uint32_t tile_index = province_tiles[i][j];
          //context->tile_province[tile_index] = i;
          ctx->container->set_data<uint32_t>(debug::entities::tile, tile_index, debug::properties::tile::province_index, counter);
        }
        //context->tile_province[i] = tile_province[i].first;
        ++counter;
      }
//       std::swap(context->province_tile, province_tiles);

      PRINT_VAR("province_min  ", province_min)
      PRINT_VAR("province_max  ", province_max)
      PRINT_VAR("count_more_max", count_more_max)
      PRINT_VAR("count_less_min", count_less_min)
      PRINT_VAR("accum_tiles_count", accum_tiles_count)
      PRINT_VAR("more_max_avg  ", float(accum_max) / float(count_more_max))
      PRINT_VAR("less_min_avg  ", float(accum_min) / float(count_less_min))
      PRINT_VAR("final_avg     ", float(accum_tiles_count) / float(count))

      //ASSERT(count == province_tiles.size());
    }

    void province_postprocessing(generator::context* ctx, sol::table &table) {
      utils::time_log log("province postprocessing");

      // ищем подходящие неразмеченные области
      std::vector<std::pair<uint32_t, uint32_t>> tile_free_area;
      std::vector<std::vector<uint32_t>> free_area_tile;
      uint32_t current_free_area = 0;
      for (size_t i = 0; i < ctx->map->tiles_count(); ++i) {
        const uint32_t current_tile_index = i;
        const float t = ctx->container->get_data<float>(debug::entities::tile, current_tile_index, debug::properties::tile::heat);
        if (t <= 0.15f) continue;

        const float height = ctx->container->get_data<float>(debug::entities::tile, current_tile_index, debug::properties::tile::elevation);
        if (height < 0.0f) continue;

        if (ctx->container->get_data<uint32_t>(debug::entities::tile, current_tile_index, debug::properties::tile::province_index) != UINT32_MAX) continue;

        bool found = false;
        for (size_t j = 0; j < tile_free_area.size(); ++j) {
          if (tile_free_area[j].second == current_tile_index) {
            found = true;
            break;
          }
        }

        if (found) continue;

        std::unordered_set<uint32_t> unique_tiles;
        std::queue<std::pair<uint32_t, uint32_t>> queue;
        unique_tiles.insert(current_tile_index);
        queue.push(std::make_pair(current_free_area, current_tile_index));

        free_area_tile.emplace_back();
        free_area_tile[current_free_area].push_back(current_tile_index);
        tile_free_area.push_back(std::make_pair(current_free_area, current_tile_index));
        ++current_free_area;

        while (!queue.empty()) {
          const auto pair = queue.front();
          queue.pop();

          const uint32_t current_free_area = pair.first;
          const uint32_t current_tile_index = pair.second;
          const auto &tile_data = render::unpack_data(ctx->map->get_tile(current_tile_index));
          const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
          for (uint32_t j = 0; j < n_count; ++j) {
            const uint32_t n_index = tile_data.neighbours[j];
            if (unique_tiles.find(n_index) != unique_tiles.end()) continue;

            const float t = ctx->container->get_data<float>(debug::entities::tile, n_index, debug::properties::tile::heat);
            if (t <= 0.15f) continue;

            const float height = ctx->container->get_data<float>(debug::entities::tile, n_index, debug::properties::tile::elevation);
            if (height < 0.0f) continue;

            ASSERT(ctx->container->get_data<uint32_t>(debug::entities::tile, n_index, debug::properties::tile::province_index) == UINT32_MAX);

            free_area_tile[current_free_area].push_back(n_index);
            tile_free_area.push_back(std::make_pair(current_free_area, n_index));
            unique_tiles.insert(n_index);
            queue.push(std::make_pair(current_free_area, n_index));
          }
        }
      }

      size_t good_tiles_count = 0;
      for (size_t i = 0; i < ctx->map->tiles_count(); ++i) {
        const uint32_t current_tile_index = i;
        const float t = ctx->container->get_data<float>(debug::entities::tile, current_tile_index, debug::properties::tile::heat);
        if (t <= 0.15f) continue;

        const float height = ctx->container->get_data<float>(debug::entities::tile, current_tile_index, debug::properties::tile::elevation);
        if (height < 0.0f) continue;
        ++good_tiles_count;
      }

      const uint32_t province_tiles_avg = float(good_tiles_count) / float(ctx->container->entities_count(debug::entities::province));
      const float province_tiles_ratio = 0.75f;
      const uint32_t province_tiles_min = province_tiles_ratio * province_tiles_avg;
      const uint32_t province_tiles_max = (2.0f - province_tiles_ratio) * province_tiles_avg;
//       const uint32_t province_tiles_max = (2.0f - province_tiles_ratio) * province_tiles_avg;
//       ASSERT(province_tiles_avg == 38);

      // у нас на карте есть еще не все подходящие области раскиданы под провинции
      // остаются еще безхозные однотайловые острова, и довольно крупные острова
      // обходим все граунд пулы, чекаем есть ли среди них крупные сначало

      // тут могут быть кусочки континентов на полюсах, которые чуть чуть пригодны для жизни
      // и че с ними делать? в общем видимо нужно найти неразмеченные области вне зависимости ни от чего
      for (size_t i = 0; i < free_area_tile.size(); ++i) {
        if (free_area_tile[i].size() < province_tiles_min * 0.75f) continue;

        // мы уже нашли по идее все области
        // осталось только их добавить
        const uint32_t new_province_index = ctx->container->add_entity(debug::entities::province);
        for (size_t j = 0; j < free_area_tile[i].size(); ++j) {
          const uint32_t tile_index = free_area_tile[i][j];
          ctx->container->add_child(debug::entities::province, new_province_index, tile_index);
          ctx->container->set_data<uint32_t>(debug::entities::tile, tile_index, debug::properties::tile::province_index, new_province_index);
        }
//         const uint32_t new_province_index = table["provinces_count"];
//         //context->province_tile.emplace_back();
//         table["provinces"][new_province_index].get_or_create<sol::table>();
//         table["provinces"][new_province_index]["tiles_count"] = free_area_tile[i].size();
//         for (size_t j = 0; j < free_area_tile[i].size(); ++j) {
//           const uint32_t tile_index = free_area_tile[i][j];
//           ctx->container->set_data<uint32_t>(debug::entities::tile, tile_index, debug::properties::tile::province_index, new_province_index);
//           table["provinces"][new_province_index]["tiles"].get_or_create<sol::table>();
//           table["provinces"][new_province_index]["tiles"][j] = tile_index;
//         }
//         table["provinces_count"] = new_province_index+1;
      }

      const uint32_t small_islands_iteration_count = 15;
      uint32_t small_islands_iteration = 0;
      while (small_islands_iteration_count > small_islands_iteration) {
        ++small_islands_iteration;
        for (size_t i = 0; i < free_area_tile.size(); ++i) {
          if (free_area_tile[i].size() >= province_tiles_min * 0.75f) continue;
          const uint32_t first_index = free_area_tile[i][0];
          if (ctx->container->get_data<uint32_t>(debug::entities::tile, first_index, debug::properties::tile::province_index) != UINT32_MAX) continue;

          // нужно добавить к ближайшей провинции
          std::queue<std::pair<uint32_t, uint32_t>> queue;
          std::unordered_set<uint32_t> unique_tiles;
          for (size_t j = 0; j < free_area_tile[i].size(); ++j) {
            queue.push(std::make_pair(free_area_tile[i][j], 0));
            unique_tiles.insert(free_area_tile[i][j]);
          }

          uint32_t neighbor_province_index = UINT32_MAX;
          while (!queue.empty() && neighbor_province_index == UINT32_MAX) {
            const auto pair = queue.front();
            queue.pop();

            if (pair.second > 5) continue; // понятное дело нужно будет поменять это число если тайлов будет больше

            const uint32_t current_tile_index = pair.first;
            const auto &tile_data = render::unpack_data(ctx->map->get_tile(current_tile_index));
            const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
            for (uint32_t j = 0; j < n_count; ++j) {
              const uint32_t n_index = tile_data.neighbours[j];
              if (unique_tiles.find(n_index) != unique_tiles.end()) continue;

              const uint32_t temp_index = ctx->container->get_data<uint32_t>(debug::entities::tile, n_index, debug::properties::tile::province_index);
              if (temp_index != UINT32_MAX) {
                neighbor_province_index = temp_index;
                break;
              }

              queue.push(std::make_pair(n_index, pair.second+1));
              unique_tiles.insert(n_index);
            }
          }

          if (neighbor_province_index == UINT32_MAX) continue;

          for (size_t j = 0; j < free_area_tile[i].size(); ++j) {
            const uint32_t tile_index = free_area_tile[i][j];
            ctx->container->set_data<uint32_t>(debug::entities::tile, tile_index, debug::properties::tile::province_index, neighbor_province_index);
            ctx->container->add_child(debug::entities::province, neighbor_province_index, tile_index);
//             const uint32_t index = table["provinces"][neighbor_province_index]["tiles_count"];
//             table["provinces"][neighbor_province_index]["tiles"][index] = tile_index;
//             table["provinces"][neighbor_province_index]["tiles_count"] = index+1;
          }
        }
      }

//       PRINT_VAR("3722 size ", ctx->container->get_childs(debug::entities::province, 3722).size())
//       PRINT_VAR("1171 size ", ctx->container->get_childs(debug::entities::province, 1171).size())

      // всякая информация
      uint32_t province_min = 10000;
      uint32_t province_max = 0;
      uint32_t count = 0;
      uint32_t count_more_max = 0;
      uint32_t count_less_min = 0;
      uint32_t accum_max = 0;
      uint32_t accum_min = 0;
      uint32_t accum_tiles_count = 0;
      for (size_t i = 0; i < ctx->container->entities_count(debug::entities::province); ++i) {
        province_min = std::min(uint32_t(ctx->container->get_childs(debug::entities::province, i).size()), province_min);
        province_max = std::max(uint32_t(ctx->container->get_childs(debug::entities::province, i).size()), province_max);
        ++count;
        accum_tiles_count += ctx->container->get_childs(debug::entities::province, i).size();

        if (ctx->container->get_childs(debug::entities::province, i).size() > province_tiles_max) {
          ++count_more_max;
          accum_max += ctx->container->get_childs(debug::entities::province, i).size();
        }

        if (ctx->container->get_childs(debug::entities::province, i).size() < province_tiles_min) {
          ++count_less_min;
          accum_min += ctx->container->get_childs(debug::entities::province, i).size();
        }
      }

      PRINT_VAR("provinces count ", ctx->container->entities_count(debug::entities::province))
      PRINT_VAR("province_min  ", province_min)
      PRINT_VAR("province_max  ", province_max)
      PRINT_VAR("count_more_max", count_more_max)
      PRINT_VAR("count_less_min", count_less_min)
      PRINT_VAR("accum_tiles_count", accum_tiles_count)
      PRINT_VAR("more_max_avg  ", float(accum_max) / float(count_more_max))
      PRINT_VAR("less_min_avg  ", float(accum_min) / float(count_less_min))
      PRINT_VAR("final_avg     ", float(accum_tiles_count) / float(count))
    }

    void calculating_province_neighbours(generator::context* ctx, sol::table &table) {
      utils::time_log log("calculating province neighbours");

      struct neighbours_set {
        std::mutex mutex;
        std::set<province_neighbour> neighbours;
      };

      const uint32_t provinces_count = ctx->container->entities_count(debug::entities::province);
      std::vector<neighbours_set> province_n(provinces_count);

      utils::submit_works_async(ctx->pool, ctx->map->tiles_count(), [&province_n] (const size_t &start, const size_t &count, const generator::context* context) {
        for (size_t i = start; i < start+count; ++i) {
          const uint32_t current_tile_index = i;
          const uint32_t current_province_index = context->container->get_data<uint32_t>(debug::entities::tile, current_tile_index, debug::properties::tile::province_index);
          if (current_province_index == UINT32_MAX) continue;
          const auto &tile_data = render::unpack_data(context->map->get_tile(current_tile_index));
          const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
          for (uint32_t j = 0; j < n_count; ++j) {
            const uint32_t n_index = tile_data.neighbours[j];
            const uint32_t n_province_index = context->container->get_data<uint32_t>(debug::entities::tile, n_index, debug::properties::tile::province_index);

            if (n_province_index == UINT32_MAX) continue;
            if (current_province_index == n_province_index) continue;

            // эти условия не выполняются
            ASSERT(current_province_index < province_n.size());
            ASSERT(n_province_index < province_n.size());

            {
              std::unique_lock<std::mutex> lock(province_n[current_province_index].mutex);
              const auto n = province_neighbour(false, n_province_index);
              province_n[current_province_index].neighbours.insert(n);
              ASSERT(n.index() == n_province_index);
              ASSERT(!n.across_water());
            }

            {
              std::unique_lock<std::mutex> lock(province_n[n_province_index].mutex);
              const auto n = province_neighbour(false, current_province_index);
              province_n[n_province_index].neighbours.insert(province_neighbour(false, current_province_index));
              ASSERT(n.index() == current_province_index);
              ASSERT(!n.across_water());
            }
          }
        }
      }, ctx);
      ctx->pool->compute();
      while (ctx->pool->working_count() != 1 || ctx->pool->tasks_count() != 0) { std::this_thread::sleep_for(std::chrono::microseconds(1)); }

      // вот у меня есть все соседи на земле
      // теперь мне нужно найти соседей по морю так, чтобы я не брал соседей на континенте
      // нет, соседи на континенте нужны
      // то есть просто найти соседей на какой то дальности, какая дальность?

      // ее нужно както посчитать
      const uint32_t max_neighbour_dist = 5;

      utils::submit_works_async(ctx->pool, provinces_count, [&province_n, &table, max_neighbour_dist] (const size_t &start, const size_t &count, const generator::context* context) {
        for (size_t i = start; i < start+count; ++i) {
          const uint32_t current_province_index = i;

          std::queue<std::tuple<uint32_t, uint32_t, uint32_t>> queue;
          std::unordered_set<uint32_t> unique_tiles;

          // какая то проблема с луа, я думаю что свазано с мультитредингом
          // хотя чтение же... нужно переделать провинции
          const uint32_t tiles_count = context->container->get_childs(debug::entities::province, current_province_index).size();
          for (size_t j = 0; j < tiles_count; ++j) {
            const uint32_t tile_index = context->container->get_childs(debug::entities::province, current_province_index)[j];
            unique_tiles.insert(tile_index);

            const auto &tile_data = render::unpack_data(context->map->get_tile(tile_index));
            const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
            for (uint32_t k = 0; k < n_count; ++k) {
              const uint32_t n_index = tile_data.neighbours[k];

              const float h = context->container->get_data<float>(debug::entities::tile, n_index, debug::properties::tile::elevation);
              if (h < 0.0f) {
                unique_tiles.insert(n_index);
                queue.push(std::make_tuple(n_index, current_province_index, 0));
              }
            }
          }

          while (!queue.empty()) {
            const auto tuple = queue.front();
            queue.pop();

            if (std::get<2>(tuple) > max_neighbour_dist) continue;

            const uint32_t current_tile_index = std::get<0>(tuple);
            const uint32_t province_index = std::get<1>(tuple);
            const auto &tile_data = render::unpack_data(context->map->get_tile(current_tile_index));
            const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
            for (uint32_t k = 0; k < n_count; ++k) {
              const uint32_t n_index = tile_data.neighbours[k];

              if (unique_tiles.find(n_index) != unique_tiles.end()) continue;

              const float h = context->container->get_data<float>(debug::entities::tile, n_index, debug::properties::tile::elevation);
              if (h < 0.0f) {
                queue.push(std::make_tuple(n_index, province_index, std::get<2>(tuple)+1));
                unique_tiles.insert(n_index);
              } else {
                const uint32_t found_province_index = context->container->get_data<uint32_t>(debug::entities::tile, n_index, debug::properties::tile::province_index);
                if (found_province_index == UINT32_MAX) continue;

                ASSERT(current_province_index == province_index);
                const auto n = province_neighbour(true, found_province_index);
                ASSERT(n.index() == found_province_index);
                ASSERT(n.across_water());
                province_n[province_index].neighbours.insert(n);
              }
            }
          }

          // ???
        }
      }, ctx);
      ctx->pool->compute();
      while (ctx->pool->working_count() != 1 || ctx->pool->tasks_count() != 0) { std::this_thread::sleep_for(std::chrono::microseconds(1)); }

      //context->province_neighbours.resize(context->province_tile.size());
      for (size_t i = 0; i < province_n.size(); ++i) {
        const uint32_t index = i;
        auto &province_neighbours = ctx->container->get_province_neighbours(index);
        for (const auto &n_index : province_n[index].neighbours) {
          // теряется в этом случае информация о том заморский ли это сосед
          // а это полезная информация, в с++ коде мы можем типы по приводить
          // в других языках - вряд ли, при этом соседи, хотя бы в каком то виде, нужны
          //province_neighbours.push_back(n_index.index());
          province_neighbours.push_back(n_index.container);
        }
      }

      const uint32_t provinces_creation_count = ctx->container->entities_count(debug::entities::province);
      for (uint32_t i = 0; i < provinces_creation_count; ++i) {
        //auto province = table.create<sol::table>();
        auto province = global::get<sol::state>()->create_table(); // локальный стейт, после генерации нужно заменить
        auto province_ns = province.create("neighbours");
        const auto &province_neighbours = ctx->container->get_province_neighbours(i);
        for (size_t index = 0; index < province_neighbours.size(); ++index) {
          const auto n = province_neighbour(province_neighbours[index]);
          province_ns[index+1] = n.index();
        }

        auto province_t = province.create("tiles");
        const auto &province_tiles = ctx->container->get_childs(debug::entities::province, i);
        for (const uint32_t &tile_index : province_tiles) {
          province_t.add(tile_index);
        }

        province["max_cities_count"] = 1;
        //province["title"] = "province" + std::to_string(i) + "_title";
        province["title"] = "baron" + std::to_string(i) + "_title";
        utils::add_province(province);
      }
    }

    void generate_cultures(generator::context* ctx, sol::table &table) {
      (void)table;
      utils::time_log log("generating cultures");

      // культуры, что по культурам?
      // культуры распространяются почти по форме диаграмы воронного
      // и довольно редко изменяются
      // причем часть культур может выпасть на острова, но при этом
      // попасть и немного на берег
      // поэтому я не ошибусь если сгенерирую диаграму вороного просто по всей поверхности земли
      // и по идее это должно будет выглядеть более менее с точки зрения распространения культур
      // (либо рандом флудфил, по идее тоже неплохо будет выглядеть)

      // следующий вопрос сколько культур генерировать?
      // в еу4 существует 68 культурных груп
      // и сколько-то (68*(от 4 до 7)) отдельных культур
      // в цк2 25 культурных групп и примерно 121 отдельная культура
      // (по 5 в среднем)
      // на весь мир около 340 культур
      // в общем нужно генерировать чуть больше чем заявленное количество культур
      // + нужно удостовериться что культуры не будут вылезать из другого края карты
      // (то есть вылезать за пределы ледяной шапки)

      // если генерировать сразу много культур, то получается лучше
      // возможно после предварительной генерации нужно будет соединить
      // несколько областей (по аналогии с континентами)
      // а затем снова разбить на непосредственно культуры
      // нужно как то учесть водные пространства
      // короче на самом деле культуры поди нужно доделать чуть после того как
      // сделаны будут государства и религии, потому что они важнее

      const uint32_t culture_groups_count = 70*5.0f;

      std::vector<uint32_t> queue;
      std::vector<uint32_t> tiles_culture(ctx->map->tiles_count(), UINT32_MAX);
      std::vector<std::vector<uint32_t>> culture_tiles(culture_groups_count);
      std::unordered_set<uint32_t> unique_tiles;

      for (size_t i = 0; i < culture_tiles.size(); ++i) {
        uint32_t tile_index = UINT32_MAX;
        uint32_t attempts = 0;

        while (tile_index == UINT32_MAX && attempts < 100) {
          ++attempts;

          const uint32_t rand_index = ctx->random->index(ctx->map->tiles_count());
          const uint32_t current_tile_index = rand_index;

          const float t = ctx->container->get_data<float>(debug::entities::tile, current_tile_index, debug::properties::tile::heat);
          if (t <= 0.15f) continue;

          if (unique_tiles.find(current_tile_index) != unique_tiles.end()) continue;

          bool found = true;
          const auto &tile_data = render::unpack_data(ctx->map->get_tile(current_tile_index));
          const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
          for (uint32_t j = 0; j < n_count; ++j) {
            const uint32_t n_index = tile_data.neighbours[j];
            const float t = ctx->container->get_data<float>(debug::entities::tile, n_index, debug::properties::tile::heat);
            if (t <= 0.15f) {
              found = false;
              break;
            }

            if (unique_tiles.find(n_index) != unique_tiles.end()) {
              found = false;
              break;
            }
          }

          if (!found) continue;

          tile_index = current_tile_index;
          queue.push_back(tile_index);
          tiles_culture[tile_index] = i;
          culture_tiles[i].push_back(tile_index);
          unique_tiles.insert(tile_index);
          for (uint32_t j = 0; j < n_count; ++j) {
            const uint32_t n_index = tile_data.neighbours[j];
            unique_tiles.insert(n_index);
          }
        }

        ASSERT(tile_index != UINT32_MAX);
      }

      while (!queue.empty()) {
        const uint32_t rand_index = ctx->random->index(queue.size());
        const uint32_t current_tile_index = queue[rand_index];
        queue[rand_index] = queue.back();
        queue.pop_back();
//         const uint32_t current_tile_index = queue.front();
//         queue.pop();

        const uint32_t current_culture_index = tiles_culture[current_tile_index];
        const auto &tile_data = render::unpack_data(ctx->map->get_tile(current_tile_index));
        const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
        for (uint32_t j = 0; j < n_count; ++j) {
          const uint32_t n_index = tile_data.neighbours[j];
          if (tiles_culture[n_index] != UINT32_MAX) continue;

          tiles_culture[n_index] = current_culture_index;
          culture_tiles[current_culture_index].push_back(n_index);
          queue.push_back(n_index);
        }
      }

      for (size_t i = 0; i < tiles_culture.size(); ++i) {
        ctx->container->set_data<uint32_t>(debug::entities::tile, i, debug::properties::tile::culture_id, tiles_culture[i]);
      }

      ctx->container->set_entity_count(debug::entities::culture, culture_tiles.size());
      for (size_t i = 0; i < culture_tiles.size(); ++i) {
        auto &childs = ctx->container->get_childs(debug::entities::culture, i);
        std::swap(childs, culture_tiles[i]);
      }

      // может быть нужно четко по провинциям раскидать культуры?
    }

    // вот медленно но верно я подобрался к государствам и религиям
    // что тут можно придумать?
    // государства должны иметь какую то историю взаимодействия
    // и между делом должны появляться религии
    // государства должны разваливаться и вновь появляться
    // некоторые государства сильнее других, и с большей вероятностью будут
    // захватывать территории, на каком то этапе в зависимости от величины
    // государства разваливаются на множество мелких государств и все это начинается вновь
    // каким то образом мне необходимо запомнить более менее историю
    // ко всему прочему неплохо было бы сделать де юре империи, королевства, герцогства
    // герцогства точно должны присутствовать с самого начала, а вот королевства и империи
    // они по ходу истории создавались, и каких то объективных позиций у них нет
    // их нужно как то из истории получить
    // для этого нужно посмотреть всех возможных соседей провинции
    // и итерационно делать какие то действия между соседями
    // как найти соседние острова для провинций? ближайшие острова на какой то дальности
    // какая дальность? наверное до 5-ти гексов
    // каждую итерацию мы проверяем соседние провинции захватит ли их сосед
    // так начинают появляться первые государства, случайные государства сложнее захватить и они легче захватывают
    // на каком то этапе в зависимости от величины государства оно разваливается на ряд более мелких государств
    // и по идее начинается все заного
    //

    // нужно четко определить всех соседей провинции, в том числе соседей островных

    void generate_countries(generator::context* ctx, sol::table &table) {
      utils::time_log log("generating countries");

      std::vector<history_step> history;

      // наверное нужно генерировать заного девелопмент и удачу
      const uint32_t provinces_count = ctx->container->entities_count(debug::entities::province);
      std::vector<uint32_t> province_country(provinces_count, UINT32_MAX);
      std::vector<std::vector<uint32_t>> country_province(provinces_count);
      std::vector<float> luckness(provinces_count);
      std::vector<float> development(provinces_count);
      for (size_t i = 0; i < province_country.size(); ++i) {
        province_country[i] = i;
        country_province[i].push_back(i);
        luckness[i] = ctx->random->closed(0.1f, 0.8f);
        development[i] = ctx->random->closed(0.1f, 0.8f);
//         development[i] = development[i] * development[i];
      }

      std::vector<float> avg_temp(provinces_count, 0.0f);
      for (uint32_t i = 0; i < provinces_count; ++i) {
        const uint32_t tiles_count = ctx->container->get_childs(debug::entities::province, i).size();
        for (uint32_t j = 0; j < tiles_count; ++j) {
          const uint32_t tile_index = ctx->container->get_childs(debug::entities::province, i)[j];
          const float t = ctx->container->get_data<float>(debug::entities::tile, tile_index, debug::properties::tile::heat);
          avg_temp[i] += t;
        }

        avg_temp[i] /= float(tiles_count);
      }

      const uint32_t iteration_count = table["userdata"]["history_iterations_count"]; // итераций должно быть явно больше
      uint32_t iteration = 0;
      while (iteration < iteration_count) {
        ++iteration;
//         if (iteration % 50 == 0) {
//           PRINT_VAR("iteration", iteration)
//         }

        // обходим каждую страну, смотрим с каким шансом мы можем что захватить
        for (size_t i = 0; i < country_province.size(); ++i) {
          const uint32_t country_index = i;
          if (country_province[country_index].size() == 0) continue;
          const float country_luck = luckness[country_index];
          const float country_development = development[country_index];

          // большая удача и большее развитие - больший шанс что нибудь захватить
          // но все равно при этом он не должен быть слишком большим
          // нужно еще предусмотреть чтобы все это дело не вытягивалось в соплю
          // а было как можно больше похожим на окружность

          // находим соседей, какой шанс захватить провинцию соседа?
          // по идее это должно быть маленькое число, около 0.1
          // то есть примерно 0.05 * country_luck + 0.05 * country_development
          // + какая то географическая состовляющая
          // + должна учитываться удача и развитие противника

          const uint32_t root_province = country_province[country_index][0];
          const float avg_t = avg_temp[root_province];

          //std::unordered_set<uint32_t> unique_provinces;
          const size_t current_size = country_province[country_index].size();
          for (size_t j = 0; j < current_size; ++j) {
            const uint32_t province_index = country_province[country_index][j];

            const auto &neighbors = ctx->container->get_province_neighbours(province_index);
            for (size_t k = 0; k < neighbors.size(); ++k) {
              const auto n_index = province_neighbour(neighbors[k]);

              const uint32_t opposing_country = province_country[n_index.index()];
              if (country_index == opposing_country) continue;

              const float n_country_luck = luckness[opposing_country];
              const float n_country_development = development[opposing_country];

              const float final_luck = std::max(country_luck - n_country_luck, 0.1f);
              const float final_development = std::max(country_development - n_country_development, 0.1f);
              const float chance = 0.05f * final_luck * avg_t + 0.05 * final_development;
              //const float chance = 0.05f * country_luck * avg_t + 0.05 * country_development;
              const bool probability = ctx->random->probability(chance);
              if (!probability) continue;

              bool found = false;
              for (auto itr = country_province[opposing_country].begin(); itr != country_province[opposing_country].end(); ++itr) {
                if (*itr == n_index.index()) {
                  country_province[opposing_country].erase(itr);
                  found = true;
                  break;
                }
              }

              ASSERT(found);

              country_province[country_index].push_back(n_index.index());
              province_country[n_index.index()] = country_index;
            }
          }

          if (country_province[country_index].size() > 80) {
            uint32_t index = UINT32_MAX;
            for (size_t j = 0; j < history.size(); ++j) {
              if (history[j].t == history_step::type::end_of_empire) continue;
              if (country_index == history[j].country) {
                index = j;
                break;
              }
            }

            if (index == UINT32_MAX) {
              index = history.size();
              history.push_back({history_step::type::count, country_index, 0, UINT32_MAX, UINT32_MAX, UINT32_MAX, {}});
            }

            history[index].size = std::max(uint32_t(country_province[country_index].size()), history[index].size);
            if (country_province[country_index].size() > 100 && history[index].t != history_step::type::becoming_empire) {
              history[index].t = history_step::type::becoming_empire;
              history[index].empire_iteration = iteration;
            }

            // ??
            // нужно добавить все страны (провинции?)
            if (history[index].t == history_step::type::becoming_empire) history[index].country_provinces.insert(country_province[country_index].begin(), country_province[country_index].end());
          }
        }

        // в какой то момент мы должны обойти все государства и прикинуть примерно когда они должны развалиться
        // среди государств есть более удачливые: они с большим шансом захватывают соседа и с меньшим шансом разваливаются
        // где то здесь же я должен генерировать распространение религий
//         const std::vector<std::vector<uint32_t>> local_copy(country_province);
        for (size_t i = 0; i < country_province.size(); ++i) {
          const uint32_t country_index = i;
          if (country_province[country_index].size() == 0) continue;
          if (country_province[country_index].size() == 1) continue;
          const float country_luck = 1.0f - luckness[country_index];
          const float country_development = 1.0f - development[country_index];
//           PRINT_VAR("country probably", country_index)

          // как развалить страну? по идее от девелопмента можно вычислить
          // количество частей на которые она разваливается
          // удача контролирует с каким шансом это дело развалится
          // развал страны должен происходить от размера страны
          // некий максимальный развер страны?

          const float avg_t = avg_temp[country_province[country_index][0]];
          const float size_k = std::min(float(country_province[country_index].size()) / 100.0f, 1.0f);
          const float final_k = country_luck * size_k * avg_t * 0.3f; // country_development ?

          const bool probability = ctx->random->probability(final_k);
          if (!probability) continue;

          const uint32_t count = std::min(std::max(uint32_t(country_province[country_index].size() * country_development), uint32_t(2)), uint32_t(30));

          uint32_t history_index = UINT32_MAX;
          for (size_t j = 0; j < history.size(); ++j) {
            if (history[j].t == history_step::type::end_of_empire) continue;
            if (country_index == history[j].country) {
              history_index = j;
              break;
            }
          }

          if (history_index != UINT32_MAX) {
            history[history_index].destroy_size = count;
            history[history_index].t = history_step::type::end_of_empire;
            history[history_index].end_iteration = iteration;
          }

          const std::vector<uint32_t> local_country(country_province[country_index]);
          country_province[country_index].clear();
          for (size_t j = 0; j < local_country.size(); ++j) {
            const uint32_t prov_index = local_country[j];
            province_country[prov_index] = UINT32_MAX;
          }

          // некоторые провинции не получают своего государства и остаются UINT32_MAX

//           PRINT_VAR("country destruction", country_index)

          std::queue<std::pair<uint32_t, uint32_t>> queue;
          std::unordered_set<uint32_t> unique_provinces;
          for (uint32_t j = 0; j < count; ++j) {
            uint32_t attempts = 0;
            uint32_t province_index = UINT32_MAX;
            while (attempts < 100 && province_index == UINT32_MAX) {
              ++attempts;
              const uint32_t rand_index = ctx->random->index(local_country.size());
              const uint32_t country_province_index = local_country[rand_index];
              if (unique_provinces.find(country_province_index) != unique_provinces.end()) continue;

              if (!country_province[country_province_index].empty() && country_province[country_province_index][0] == country_province_index) {
                ASSERT(country_province[country_province_index].size() != 0);
                country_province[country_province_index].clear();
              } else {
                std::vector<uint32_t> tmp;
                std::swap(country_province[country_province_index], tmp);
//                 uint32_t counter = 0;
                while (!tmp.empty()) {
                  const uint32_t first_province = tmp[0];
                  for (size_t k = 0; k < tmp.size(); ++k) {
                    const uint32_t prov = tmp[k];
                    province_country[prov] = first_province;
                  }

                  std::swap(country_province[first_province], tmp);
//                   ASSERT(counter < 25);
                }

                ASSERT(country_province[country_province_index].empty());
              }

              province_index = country_province_index;
            }

            ASSERT(province_index != UINT32_MAX);

            country_province[province_index].push_back(province_index);
            province_country[province_index] = province_index;
            queue.push(std::make_pair(province_index, province_index));
            unique_provinces.insert(province_index);

//             luckness[province_index] = context->data->engine->norm();
//             development[province_index] = context->data->engine->norm();
          }

//           PRINT_VAR("country queue", country_index)

          while (!queue.empty()) {
            const auto current_index = queue.front();
            queue.pop();

            const auto &neighbours = ctx->container->get_province_neighbours(current_index.second);
            for (size_t j = 0; j < neighbours.size(); ++j) {
              const auto n_index = province_neighbour(neighbours[j]);
              //ASSERT(n_index != 2961);
              if (unique_provinces.find(n_index.index()) != unique_provinces.end()) continue;
              if (province_country[n_index.index()] != UINT32_MAX) continue;

              queue.push(std::make_pair(current_index.first, n_index.index()));
              unique_provinces.insert(n_index.index());
              province_country[n_index.index()] = current_index.first;
              country_province[current_index.first].push_back(n_index.index());
            }
          }

          for (size_t j = 0; j < local_country.size(); ++j) {
            const uint32_t prov_index = local_country[j];
            // такое может быть если у страны не осталось прямого выхода на эту провинцию
            // наверное лучше ее возвращать обратно в свое государство
            //ASSERT(province_country[prov_index] != UINT32_MAX);
            if (province_country[prov_index] != UINT32_MAX) continue;

            // такого быть не должно
            if (!country_province[prov_index].empty() && country_province[prov_index][0] == prov_index) {
              province_country[prov_index] = prov_index;
              ASSERT(country_province[prov_index].size() == 1);
              continue;
            }

            std::vector<uint32_t> tmp;
            std::swap(country_province[prov_index], tmp);
//             uint32_t counter = 0;
            while (!tmp.empty()) {
              const uint32_t first_province = tmp[0];
              for (size_t k = 0; k < tmp.size(); ++k) {
                const uint32_t prov = tmp[k];
                province_country[prov] = first_province;
              }

              std::swap(country_province[first_province], tmp);
//               ASSERT(counter < 25);
            }

            ASSERT(country_province[prov_index].empty());
            country_province[prov_index].push_back(prov_index);
            province_country[prov_index] = prov_index;

//             luckness[prov_index] = context->data->engine->norm();
//             development[prov_index] = context->data->engine->norm();
          }
        }

      }

      for (size_t i = 0; i < country_province.size(); ++i) {
        if (country_province[i].size() == 0) continue;
        if (country_province[i][0] == i) continue;

        uint32_t current_index = i;
        const uint32_t root_index = country_province[i][0];
        std::swap(country_province[current_index], country_province[root_index]);
        std::swap(luckness[current_index], luckness[root_index]);
        std::swap(development[current_index], development[root_index]);

        for (size_t j = 0; j < country_province[current_index].size(); ++j) {
          const uint32_t prov_index = country_province[current_index][j];
          province_country[prov_index] = current_index;
        }

        for (size_t j = 0; j < country_province[root_index].size(); ++j) {
          const uint32_t prov_index = country_province[root_index][j];
          province_country[prov_index] = root_index;
        }

        while (current_index != UINT32_MAX) {
          if (country_province[current_index].empty()) break;
          const uint32_t root_index = country_province[current_index][0];
          if (root_index == current_index) break;

          std::swap(country_province[current_index], country_province[root_index]);
          std::swap(luckness[current_index], luckness[root_index]);
          std::swap(development[current_index], development[root_index]);
          for (size_t j = 0; j < country_province[current_index].size(); ++j) {
            const uint32_t prov_index = country_province[current_index][j];
            province_country[prov_index] = current_index;
          }

          for (size_t j = 0; j < country_province[root_index].size(); ++j) {
            const uint32_t prov_index = country_province[root_index][j];
            province_country[prov_index] = root_index;
          }
        }
      }

      // множество провинций получаются оторваны от государств на суше
      // нужно их либо выделить в отдельные государтсва
      // возможно даже дать им дополнительных провинций
      // либо добавить в государство их окружающее
      std::vector<uint32_t> new_countries;
      for (size_t i = 0; i < country_province.size(); ++i) {
        if (country_province[i].size() < 2) continue;
        const uint32_t country_root = country_province[i][0];
        ASSERT(country_root == i);
        for (size_t j = 1; j < country_province[i].size(); ++j) {
          const uint32_t province_index = country_province[i][j];

          std::unordered_set<uint32_t> unique_provinces;
          std::queue<uint32_t> queue;

          unique_provinces.insert(province_index);
          queue.push(province_index);

          bool found = false;
          while (!queue.empty() && !found) {
            const uint32_t current_province_index = queue.front();
            queue.pop();

            const auto &neighbours = ctx->container->get_province_neighbours(current_province_index);
            for (size_t k = 0; k < neighbours.size(); ++k) {
              const auto n_index = province_neighbour(neighbours[k]);
              if (n_index.across_water()) continue;
              if (province_country[n_index.index()] != i) continue;
              if (unique_provinces.find(n_index.index()) != unique_provinces.end()) continue;

              if (n_index.index() == country_root) {
                found = true;
                break;
              }

              unique_provinces.insert(n_index.index());
              queue.push(n_index.index());
            }
          }

          if (found) continue;

          // нет прямого доступа к провинции
          // это значит что эта провинция (и все возможные соседи)
          // должны быть другим государством
          new_countries.push_back(province_index);
        }
      }

      for (size_t i = 0; i < new_countries.size(); ++i) {
        const uint32_t province_index = new_countries[i];
        const uint32_t old_country = province_country[province_index];
        ASSERT(old_country != UINT32_MAX);
        bool found = false;
        for (auto itr = country_province[old_country].begin(); itr != country_province[old_country].end(); ++itr) {
          if (*itr == province_index) {
            country_province[old_country].erase(itr);
            found = true;
            break;
          }
        }

        ASSERT(found);

        province_country[province_index] = UINT32_MAX;
      }

//       {
//         uint32_t province_count = 0;
//         for (size_t i = 0; i < country_province.size(); ++i) {
//           const uint32_t country = i;
//           if (country_province[country].size() == 0) continue;
//           province_count += country_province[country].size();
//         }
//         PRINT_VAR("new_countries  size", new_countries.size())
//         PRINT_VAR("province_count calc", province_count)
//         PRINT_VAR("province_count     ", province_country.size())
//       }

      for (size_t i = 0; i < new_countries.size(); ++i) {
        const uint32_t province_index = new_countries[i];
        if (province_country[province_index] != UINT32_MAX) continue;

        std::unordered_set<uint32_t> unique_provinces;
        std::queue<uint32_t> queue;
        std::vector<uint32_t> new_country_provinces;
        unique_provinces.insert(province_index);
        queue.push(province_index);
        new_country_provinces.push_back(province_index);

        while (!queue.empty()) {
          const uint32_t current_province_index = queue.front();
          queue.pop();

          const auto &neighbours = ctx->container->get_province_neighbours(current_province_index);
          for (size_t k = 0; k < neighbours.size(); ++k) {
            const auto n_index = province_neighbour(neighbours[k]);
            if (province_country[n_index.index()] != UINT32_MAX) continue;
            if (unique_provinces.find(n_index.index()) != unique_provinces.end()) continue;

            unique_provinces.insert(n_index.index());
            queue.push(n_index.index());
            new_country_provinces.push_back(n_index.index());
          }
        }

        for (uint32_t j = 0; j < new_country_provinces.size(); ++j) {
          const uint32_t index = new_country_provinces[j];
          ASSERT(province_country[index] == UINT32_MAX);
          province_country[index] = province_index;
        }

        std::swap(country_province[province_index], new_country_provinces);
        std::unordered_set<uint32_t> loop_root;
        loop_root.insert(province_index);
        ASSERT(new_country_provinces.empty());
//         while (!new_country_provinces.empty()) {
//           const uint32_t root = new_country_provinces[0];
//           ASSERT(loop_root.find(root) == loop_root.end());
//           for (uint32_t j = 0; j < new_country_provinces.size(); ++j) {
//             const uint32_t index = new_country_provinces[j];
//             province_country[index] = root;
//           }
//           std::swap(country_province[root], new_country_provinces);
//           loop_root.insert(root);
//         }
      }

      uint32_t province_count = 0;
      uint32_t country_count = 0;
      uint32_t max_country = 0;
      for (size_t i = 0; i < country_province.size(); ++i) {
        const uint32_t country = i;
        if (country_province[country].size() == 0) continue;

        province_count += country_province[country].size();
        ++country_count;
        max_country = std::max(max_country, uint32_t(country_province[country].size()));
        ASSERT(country_province[country][0] == country);
        for (size_t j = 0; j < country_province[country].size(); ++j) {
          const uint32_t province = country_province[country][j];
          ASSERT(province_country[province] == country);
        }
      }

      // тут у нас примерно 6-7 стран с размерами больше 100
      // + спавнятся однопровинчатые государства по середине более крупных
      // мне нужно как то сильнее распределить провинции + что то сделать с
      // однопровинчатыми государствами
      // возможно эти крупные государства нужно как то развалить?
      // минорки нужно видимо увеличить

      // некоторые из крупных государств только каким то чудом не развалились
      // (ну хотя я может быть напортачил с формулой)

      // нужно сначало минорки определить
      // то государство, которое мало по размеру и у которого только один крупный сосед
      // если два соседа? в этом случае ситуация как у ростова в европке
      //

      // маленький размер - это сколько? может ли заспавнится большая страна внутри другой большой?
      // тут скорее нужно проверить как то есть соседи у государства кроме другого
      // если соседей = 1 и нет выхода к морю, то вот наш кандидат

      for (size_t i = 0; i < country_province.size(); ++i) {
        const uint32_t country_index = i;

        std::unordered_set<uint32_t> neighbors;
        neighbors.insert(country_index);
        for (size_t j = 0; j < country_province[country_index].size(); ++j) {
          const uint32_t province_index = country_province[country_index][j];

          const auto &neighbours = ctx->container->get_province_neighbours(province_index);
          for (size_t k = 0; k < neighbours.size(); ++k) {
            const auto n_index = province_neighbour(neighbours[k]);
            const uint32_t neighbour_country = province_country[n_index.index()];
            neighbors.insert(neighbour_country);
          }
        }

        ASSERT(!neighbors.empty());

        // тут проблема в том что есть несколько провинций которые находятся внутри огромного государства
        // как определить эти провинции? у нас есть несколько проблем,
        // провинция просто может быть сильно внутри, но при этом соединена с другими провинциями
        // типо можно посчитать количество стран
        if (neighbors.size() > 2) continue;

        // нужно проверить выход к морю
        // если он есть, то не все потеряно?
        bool found = false;
        for (size_t j = 0; j < country_province[i].size(); ++j) {
          const uint32_t province_index = country_province[i][j];
          const uint32_t tiles_count = ctx->container->get_childs(debug::entities::province, province_index).size();
          for (size_t k = 0; k < tiles_count; ++k) {
            const uint32_t tile_index = ctx->container->get_childs(debug::entities::province, province_index)[k];
            const auto &tile_data = render::unpack_data(ctx->map->get_tile(tile_index));
            const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
            for (size_t c = 0; c < n_count; ++c) {
              const uint32_t n_index = tile_data.neighbours[c];

              const float h = ctx->container->get_data<float>(debug::entities::tile, n_index, debug::properties::tile::elevation);
              if (h < 0.0f) {
                found = true;
                break;
              }
            }

            if (found) break;
          }

          if (found) break;
        }

        // вообще здесь мы еще можем исключить тех кто имеет выход к озеру
        if (found) continue;

        // теперь у нас есть государства без выхода к морю и только с одним соседом
        // что нужно сделать?
        // если это минорка, нужно посмотреть расстояние до других государств

        if (neighbors.size() == 1) continue; // если вообще нет соседей

        auto itr = neighbors.begin();
        //++itr;
        const uint32_t opposing_country = *itr == country_index ? *(++itr) : *itr;
        ASSERT(opposing_country != country_index);

        // скорее всего в такую ситуацию может попасть не только одна провинция, но и более крупное государство

        if (country_province[country_index].size() < 2) {
          const uint32_t first_province = country_province[country_index][0];
          province_country[first_province] = opposing_country;
          country_province[opposing_country].push_back(first_province);
          country_province[country_index].clear();
        } else {
          // если провинций больше
          // то может быть раздать этому государству провинций?

          std::unordered_set<uint32_t> unique_provinces;
          //std::vector<uint32_t> province_local(country_province[country_index]);
          //std::vector<uint32_t> province_local;
          std::queue<uint32_t> province_local;
          std::queue<uint32_t> province_local2;
          for (size_t j = 0; j < country_province[country_index].size(); ++j) {
            const uint32_t province_index = country_province[country_index][j];
            //unique_provinces.insert(province_index);
            const auto &neighbours = ctx->container->get_province_neighbours(province_index);
            for (size_t k = 0; k < neighbours.size(); ++k) {
              const auto n_index = province_neighbour(neighbours[k]);
              province_local.push(n_index.index());
              unique_provinces.insert(n_index.index());
            }
          }

          uint32_t new_n = UINT32_MAX;
          while (!province_local.empty() && new_n == UINT32_MAX) {
//             while (!province_local2.empty()) {
//               const uint32_t rand_index = context->data->engine->index(province_local.size());
//               const uint32_t province_index = province_local[rand_index];
//               province_local[rand_index] = province_local.back();
//               province_local.pop_back();
              const uint32_t province_index = province_local.front();
              province_local.pop();

              if (province_country[province_index] == country_index) continue;

              const uint32_t index = province_country[province_index];
              ASSERT(index == opposing_country);
              province_country[province_index] = country_index;
              for (auto itr = country_province[index].begin(); itr != country_province[index].end(); ++itr) {
                if (*itr == province_index) {
                  country_province[index].erase(itr);
                  break;
                }
              }

              country_province[country_index].push_back(province_index);

              const auto &neighbours = ctx->container->get_province_neighbours(province_index);
              for (size_t j = 0; j < neighbours.size(); ++j) {
                const auto n_index = province_neighbour(neighbours[j]);

                if (province_country[n_index.index()] == country_index) continue;
                if (unique_provinces.find(n_index.index()) != unique_provinces.end()) continue;

//                 if (province_country[n_index.index()] != opposing_country) {
//                   new_n = province_country[n_index.index()];
//                   break;
//                 }

                province_local2.push(n_index.index());
                unique_provinces.insert(n_index.index());
              }
//             }

            while (!province_local2.empty()) {
              const uint32_t province_index = province_local2.front();
              province_local2.pop();

              const auto &neighbours = ctx->container->get_province_neighbours(province_index);
              for (size_t j = 0; j < neighbours.size(); ++j) {
                const auto n_index = province_neighbour(neighbours[j]);

                if (province_country[n_index.index()] != opposing_country) {
                  new_n = province_country[n_index.index()];
                  break;
                }
              }

              province_local.push(province_index);
            }
          }


        }
      }

      PRINT_VAR("new_countries  size", new_countries.size())
      PRINT_VAR("province_count calc", province_count)
      PRINT_VAR("province_count     ", province_country.size())
      PRINT_VAR("country_count      ", country_count)
      PRINT_VAR("max_country        ", max_country)
      PRINT_VAR("avg provinces      ", float(province_count) / float(country_count))

      for (size_t i = 0; i < country_province.size(); ++i) {
        const uint32_t country = i;
        if (country_province[country].size() < 100) continue;

        PRINT("\n")
        PRINT_VAR("country    ", country)
        PRINT_VAR("provinces  ", country_province[country].size())
        PRINT_VAR("development", development[country])
        PRINT_VAR("luck       ", luckness[country])
        const uint32_t root = country_province[country][0];
        const float avg_t = avg_temp[root];
        PRINT_VAR("avg_t      ", avg_t)
      }

//       PRINT("\n")
//
//       for (size_t i = 0; i < history.size(); ++i) {
//         if (history[i].t == history_step::type::count) continue;
//
//         PRINT_VAR("country", history[i].country);
//         if (history[i].t == history_step::type::becoming_empire) {
//           PRINT_VAR("status", "becoming empire");
//           PRINT_VAR("max size", history[i].size);
//           PRINT_VAR("becoming empire iteration", history[i].empire_iteration);
//         } else {
//           PRINT_VAR("status", "end of empire");
//           PRINT_VAR("max size", history[i].size);
//           PRINT_VAR("becoming empire iteration", history[i].empire_iteration);
//           PRINT_VAR("end of empire iteration", history[i].end_iteration);
//           PRINT_VAR("destroy size", history[i].destroy_size);
//         }
//       }

      for (size_t i = 0; i < ctx->map->tiles_count(); ++i) {
        ctx->container->set_data<uint32_t>(debug::entities::tile, i, debug::properties::tile::country_index, UINT32_MAX);
      }

      for (size_t i = 0; i < province_country.size(); ++i) {
        ctx->container->set_data<uint32_t>(debug::entities::province, i, debug::properties::province::country_index, province_country[i]);
        const auto &childs = ctx->container->get_childs(debug::entities::province, i);
        for (size_t j = 0; j < childs.size(); ++j) {
          const uint32_t tile_index = childs[j];
          ctx->container->set_data<uint32_t>(debug::entities::tile, tile_index, debug::properties::tile::country_index, province_country[i]);
        }
      }

      size_t new_country_counter = 0;
      ctx->container->set_entity_count(debug::entities::country, country_province.size());
      for (size_t i = 0; i < country_province.size(); ++i) {
        auto &childs = ctx->container->get_childs(debug::entities::country, i);
        if (country_province[i].empty()) continue;
        std::swap(childs, country_province[i]);
        ++new_country_counter;
      }

      const uint32_t tiles_count_color = ctx->container->entities_count(debug::entities::tile);
      for (size_t i = 0; i < tiles_count_color; ++i) {
        const uint32_t country_index = ctx->container->get_data<uint32_t>(debug::entities::tile, i, debug::properties::tile::country_index);
        const uint32_t rand_num1 = render::prng(country_index);
        const uint32_t rand_num2 = render::prng(rand_num1);
        const uint32_t rand_num3 = render::prng(rand_num2);
        const float color_r = render::prng_normalize(rand_num1);
        const float color_g = render::prng_normalize(rand_num2);
        const float color_b = render::prng_normalize(rand_num3);
        ctx->map->set_tile_color(i, render::make_color(color_r, color_b, color_g, 1.0f));
      }

      table["country_count"] = new_country_counter;

      // меня уже более менее устраивает результат
      // на следующих шагах мне нужно как использовать историю сгененированную здесь
      // причем, мне нужно будет определить какие-то государства как нестабильные
      // и как стабильные, при начале игры нестабильные государства должны как то эпично разваливаться
      // а стабильные большие государства должны представлять угрозу для игрока или быть легким уровнем сложности

      // какие страны стабильные а какие нет? у меня довольно много государств спавнится в которых
      // очень сложная география (то есть как будто целая страна нарезана на лоскуты)
      // если граница большая? как проверить большую границу? идеальное государство - это такое
      // которое имеет границу максимально подобную окружности
      // периметр такой фигуры будет сильно меньше чем периметр произвольной фигуры
      // помимо периметра есть еще неплохой способ: дальность от границы
      // в окружности центр - это максимальная дальность
      // и что мне нужно в итоге получить? по идее государства - это несколько вассалов
      // (у которых есть свои вассалы и проч), в нестабильном государстве вассалы
      // входят в разные оппозиционные фракции и разрывают страну на части
      // в стабильном государстве вся такая деятельность подавлена
      // и мне нужно сгенерировать по сути стартовый набор вассалов внутри государства
      // со своими амбициями и желаниями
      // у сильного государства вассалы либо подавлены либо юридически у них очень мало прав
      // у слабого государства вассалы чувствуют себя лучше вплоть до игнорирования приказов сюзерена
      // например, довольно сильное государство - византийская империя
      // (имперская администрация как то ограничивала возможности вассалов)
      // слабое государство - это франкская империя, которая после смерти императора развалилась на куски
      //

      // у меня есть какая то небольшая история (возможно нужно ее сделать больше, но это позже)
      // в общем то теперь я плавно перехожу к геймплею
      // для этого мне нужно: починить рендер, сделать несколько типов рендеров, интерфейс
      // как сделать пошаговость?, заспавнить города, мне еще специальные постройки нужно поставить (шахты, данжи)
      //
      // и наверное нужно сразу разделить государства на
      // земли кочевников, государства племен, и феодальные государства
      // неплохо было заспавнить несколько технологических центров (ну то есть сделать условный китай и итальянские республики)
      //

      // нужно создать где то рядом персонажей и раздать им титулы
    }

    void gen_title_color(sol::table &table, const uint32_t &index) {
      const uint32_t val1 = render::prng(index);
      const uint32_t val2 = render::prng(val1);
      const uint32_t val3 = render::prng(val2);
      const uint32_t val4 = render::prng(val3);
      const uint32_t val5 = render::prng(val4);
      const float f1 = render::prng_normalize(val1);
      const float f2 = render::prng_normalize(val2);
      const float f3 = render::prng_normalize(val3);
      const float f4 = render::prng_normalize(val4);
      const float f5 = render::prng_normalize(val5);
      table["main_color"] = render::make_color(f1, f2, f3, 1.0f).container;
      table["border_color1"] = render::make_color(f2, f3, f4, 1.0f).container;
      table["border_color2"] = render::make_color(f3, f4, f5, 1.0f).container;
    }
    
    void generate_heraldy(generator::context* ctx, sol::table &table) {
      // тут нам нужно загрузить во первых картинки геральдик
      // во вторых составить несколько слоев геральдик
      {
        // какое разрешение у геральдики? я планировал использовать 128х128
        // думаю что это максимум, который я могу себе позволить, 
        // геральдик все же будет довольно много (несколько щитов + огромное количество символик)
        auto image_table = global::get<sol::state>()->create_table();
        image_table["id"] = "heraldy_shield1";
        image_table["path"] = "apates_quest/textures/heraldy/shield_straight.png";
        image_table["type"] = 0; // тут индекс
        image_table["sampler"] = 0;
        utils::add_image(image_table);
      }
      
      {
        auto image_table = global::get<sol::state>()->create_table();
        image_table["id"] = "heraldy_quarterly";
        image_table["path"] = "apates_quest/textures/heraldy/quarterly.png";
        image_table["type"] = 0; // тут индекс
        image_table["sampler"] = 1;
        utils::add_image(image_table);
      }
      
      {
        auto image_table = global::get<sol::state>()->create_table();
        image_table["id"] = "heraldy_lion1";
        image_table["path"] = "apates_quest/textures/heraldy/royal_lion.png";
        //auto atlas = image_table["atlas_data"].get_or_create<sol::table>();
        //auto scale = image_table["scale"].get_or_create<sol::table>();
        //scale["width"] = 128;
        //scale["height"] = 128;
        //scale["filter"] = 1; // nearest
        image_table["type"] = 0; // тут индекс
        image_table["sampler"] = 0;
        utils::add_image(image_table);
      }
      
      {
        auto heraldy_table = global::get<sol::state>()->create_table();
        heraldy_table["id"] = "shield_layer";
        heraldy_table["stencil"] = "heraldy_shield1";
        heraldy_table["next_layer"] = "lion_layer";
        auto colors = heraldy_table["colors"].get_or_create<sol::table>();
        colors.add(render::make_color(0.0f, 0.0f, 0.0f, 1.0f).container);
        colors.add(render::make_color(0.0f, 0.0f, 0.0f, 1.0f).container);
        colors.add(render::make_color(0.0f, 0.0f, 0.0f, 1.0f).container);
        colors.add(render::make_color(0.7f, 0.0f, 0.0f, 1.0f).container);
        auto coords = heraldy_table["coords"].get_or_create<sol::table>();
        coords.add(0.0f, 0.0f, 1.0f, 1.0f);
        auto tex_coords = heraldy_table["tex_coords"].get_or_create<sol::table>();
        tex_coords.add(0.0f, 0.0f, 1.0f, 1.0f);
        heraldy_table["discard_layer"] = false;
        heraldy_table["continue_layer"] = false;
        utils::add_heraldy_layer(heraldy_table);
      }
      
      {
        auto heraldy_table = global::get<sol::state>()->create_table();
        heraldy_table["id"] = "shield_player_layer";
        heraldy_table["stencil"] = "heraldy_shield1";
        heraldy_table["next_layer"] = "lion_layer";
        auto colors = heraldy_table["colors"].get_or_create<sol::table>();
        colors.add(render::make_color(0.0f, 0.0f, 0.0f, 1.0f).container);
        colors.add(render::make_color(0.0f, 0.0f, 0.0f, 1.0f).container);
        colors.add(render::make_color(0.0f, 0.0f, 0.0f, 1.0f).container);
        colors.add(render::make_color(0.0f, 0.7f, 0.0f, 1.0f).container);
        auto coords = heraldy_table["coords"].get_or_create<sol::table>();
        coords.add(0.0f, 0.0f, 1.0f, 1.0f);
        auto tex_coords = heraldy_table["tex_coords"].get_or_create<sol::table>();
        tex_coords.add(0.0f, 0.0f, 1.0f, 1.0f);
        heraldy_table["discard_layer"] = false;
        heraldy_table["continue_layer"] = false;
        utils::add_heraldy_layer(heraldy_table);
      }
      
      {
        auto heraldy_table = global::get<sol::state>()->create_table();
        heraldy_table["id"] = "lion_layer";
        heraldy_table["stencil"] = "heraldy_lion1";
        heraldy_table["next_layer"] = "quarterly_layer";
        auto colors = heraldy_table["colors"].get_or_create<sol::table>();
        colors.add(render::make_color(0.0f, 0.0f, 0.0f, 1.0f).container);
        colors.add(render::make_color(0.0f, 0.0f, 0.0f, 1.0f).container);
        colors.add(render::make_color(0.0f, 0.0f, 0.0f, 1.0f).container);
        colors.add(render::make_color(0.0f, 0.7f, 0.0f, 1.0f).container);
        auto coords = heraldy_table["coords"].get_or_create<sol::table>();
        coords.add(0.15f, 0.1f, 0.85f, 0.8f);
        auto tex_coords = heraldy_table["tex_coords"].get_or_create<sol::table>();
        tex_coords.add(0.0f, 0.0f, 1.0f, 1.0f);
        heraldy_table["discard_layer"] = true;
        heraldy_table["continue_layer"] = false;
        utils::add_heraldy_layer(heraldy_table);
      }
      
      {
        auto heraldy_table = global::get<sol::state>()->create_table();
        heraldy_table["id"] = "quarterly_layer";
        heraldy_table["stencil"] = "heraldy_quarterly";
        heraldy_table["next_layer"] = sol::nil;
        auto colors = heraldy_table["colors"].get_or_create<sol::table>();
        colors.add(render::make_color(0.8f, 0.6f, 0.0f, 1.0f).container);
        colors.add(render::make_color(0.9f, 0.9f, 0.9f, 1.0f).container);
        colors.add(render::make_color(0.9f, 0.9f, 0.9f, 1.0f).container);
        colors.add(render::make_color(0.8f, 0.6f, 0.0f, 1.0f).container);
        auto coords = heraldy_table["coords"].get_or_create<sol::table>();
        coords.add(0.15f, 0.1f, 0.85f, 0.8f);
        auto tex_coords = heraldy_table["tex_coords"].get_or_create<sol::table>();
        tex_coords.add(0.0f, 0.0f, 0.2f, 0.2f);
        heraldy_table["discard_layer"] = false;
        heraldy_table["continue_layer"] = false;
        utils::add_heraldy_layer(heraldy_table);
      }
      
      (void)ctx;
      (void)table;
    }

    void generate_titles(generator::context* ctx, sol::table &table) {
      utils::time_log log("generating titles");
      // титулы по идее должны как то учитывать сгенерированные государства
      // сначало наверное нужно придумать механизм взаимодействия со строками
      // должен быть некий банк строк либо алгоритм их вывода
      // при совпадении типа и индекса выдается строка
      // тип: строка для провинции, персонажа, титула и проч
      // короче должен быть способ запоминать строки использованные в игре
      // что делать с локализацией? нужны переменные локализации и
      // зарегистрировать особые функции связанные с локализацией
      // при обращении к ним получать строку, а потом ее запихивать в стек

      // нашел хороший код который дал мне идею генератора (https://github.com/skeeto/fantasyname)
      // основная задача генераторов названий: соединить несколько строк с определенными типами друг с другом
      // например алфавит (арит|мерит)(нимир|риар) даст соответственно 4 имени
      // помимо этого нужно добавить возможность делать первый символ большим, ставить символы в обратном порядке,
      // брать символы из таблицы, пустой символ
      // синтаксис !~<table_name|>(name_part|), так ли нужны таблицы?
      // таблицы нужны прежде всего когда нужно повторить какой то паттерн
      // вообще можно использовать % + символ для вызова таблиц
      // (go%b|ro(bu|nu)|)~(%b|%a)!(byu|asd)
      // не понял пока еще что такое Collapse Triples
      // вроде как сделал

      // еще нужно придумать способ задавать переменные в текст
      // то есть: Дженерик текст %character_name% дженерик текст
      // причем еще нужно продумать какую то возможность учитывать падежи
      // вообще идеально как-то помечать какой то участок текста и выводить например тултип
      // я могу так сделать разделив текст на несколько частей
      // но это видимо что-то из разряда фантастики
      // (наверное можно вычислить квадрат где находится этот участок)

      // осталось понять как адекватно сделать базы со строками
      // я думаю что нужно предсгенерировать много имен, положить их в массивы строк по типам
      // и во время игры просто доставать их оттуда
      
      // нужно сгенерировать геральдики: для этого нужно сделать и загрузить большое количество трафаретов
      // трафарет - это, в моем случае, картинка с альфа каналом + зарегестрированы 4 цвета (чистые ргб и черный)
      // мне нужны трафареты щитов, королевских животных, религиозных символик и проч
      // все эти трафареты наверное нужно ужать до 128x128... какой размер использует цк2? 88х88 какой то такой (в атласе)
      // 

      // нужно сгенерировать титулы герцогские (несколько баронств), королевские (несколько герцогств) и имперские (несколько королевств)
      // возможно добавить еще одну иерархию титулов (что-то вроде благословления богини, но ее можно создать только внутриигровым эвентом)
      // я еще не придумал как титул обратно получать, из провинции
      const uint32_t province_count = ctx->container->entities_count(debug::entities::province);
      // нужно посчитать тайлы соприкосновения
      const uint32_t border_size_const = 5;
      std::vector<std::vector<std::pair<uint32_t, size_t>>> neighbours_border_size(province_count);
      {
        for (size_t i = 0; i < neighbours_border_size.size(); ++i) {
          const auto &n = ctx->container->get_province_neighbours(i);
          neighbours_border_size[i] = std::vector<std::pair<uint32_t, size_t>>(n.size());
          for (size_t j = 0; j < neighbours_border_size[i].size(); ++j) {
            neighbours_border_size[i][j] = std::make_pair(n[j], 0);
          }
        }

        std::unordered_set<size_t> unique_border;
        for (uint32_t i = 0; i < ctx->container->entities_count(debug::entities::tile); ++i) {
          const uint32_t province_index = ctx->container->get_data<uint32_t>(debug::entities::tile, i, debug::properties::tile::province_index);
          if (province_index == UINT32_MAX) continue;
          const auto &tile_data = render::unpack_data(ctx->map->get_tile(i));
          const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
          for (uint32_t j = 0; j < n_count; ++j) {
            const uint32_t n_index = tile_data.neighbours[j];
            const uint32_t n_province_index = ctx->container->get_data<uint32_t>(debug::entities::tile, n_index, debug::properties::tile::province_index);
            if (n_province_index == UINT32_MAX) continue;
            if (province_index == n_province_index) continue;

            const uint32_t first_index = std::max(i, n_index);
            const uint32_t second_index = std::min(i, n_index);
            const size_t key = (size_t(second_index) << 32) | size_t(first_index);
            if (unique_border.find(key) != unique_border.end()) continue;

            {
              uint32_t n_prov_index = UINT32_MAX;
              const auto &n = ctx->container->get_province_neighbours(province_index);
              for (size_t k = 0; k < n.size(); ++k) {
                if (n[k] == n_province_index) {
                  n_prov_index = k;
                  break;
                }
              }

              ASSERT(n_prov_index != UINT32_MAX);
              neighbours_border_size[province_index][n_prov_index].second += 1;
            }

            {
              uint32_t n_prov_index = UINT32_MAX;
              const auto &n = ctx->container->get_province_neighbours(n_province_index);
              for (size_t k = 0; k < n.size(); ++k) {
                if (n[k] == province_index) {
                  n_prov_index = k;
                  break;
                }
              }

              ASSERT(n_prov_index != UINT32_MAX);
              neighbours_border_size[n_province_index][n_prov_index].second += 1;
            }

            unique_border.insert(key);
          }
        }

        for (size_t i = 0; i < neighbours_border_size.size(); ++i) {
          std::sort(neighbours_border_size[i].begin(), neighbours_border_size[i].end(), [] (const auto &first, const auto &second) -> bool {
            return first.second > second.second;
          });
        }
      }

      // герцогский титул это что? в цк2 в герцогстве находится до 6 баронств, в цк2 394 гергогств
      const uint32_t duchies_count = province_count / 4; // среднее количество находится в районе 3-4 (в цк2 почти 4)
      std::unordered_set<uint32_t> unique_indices;
      std::queue<std::pair<uint32_t, uint32_t>> queue;
      std::vector<std::vector<uint32_t>> duchies;
      for (size_t i = 0; i < duchies_count; ++i) {
        uint32_t province_index = UINT32_MAX;
        uint32_t attempts = 0;
        while (province_index == UINT32_MAX && attempts < 100) {
          ++attempts;
          const uint32_t rand_index = ctx->random->index(province_count);
          if (unique_indices.find(rand_index) != unique_indices.end()) continue;

          // проверить соседей? соседей больше чем может быть в герцогстве баронств
          province_index = rand_index;
        }

        if (province_index == UINT32_MAX) continue;

        unique_indices.insert(province_index);
        queue.push(std::make_pair(duchies.size(), province_index));
        duchies.emplace_back();
        duchies.back().push_back(province_index);
      }

      while (!queue.empty()) {
        const auto pair = queue.front();
        queue.pop();

        const auto &neighbours = ctx->container->get_province_neighbours(pair.second);
        if (neighbours.size() == 0) continue;
        if (duchies[pair.first].size() > 5) continue;
        ASSERT(duchies[pair.first].size() > 0);
        if (duchies[pair.first].size() == 1) {
          uint32_t province_index = UINT32_MAX;
          uint32_t attempts = 0;
          while (province_index == UINT32_MAX && attempts < 15) {
            ++attempts;
            const uint32_t rand_index = ctx->random->index(neighbours.size());
            const auto border_size = neighbours_border_size[pair.second][rand_index];
            if (border_size.second < border_size_const) continue;
            const auto n_province_index = province_neighbour(neighbours[rand_index]);
            if (n_province_index.across_water()) continue;
            if (unique_indices.find(n_province_index.index()) != unique_indices.end()) continue;

            province_index = n_province_index.index();
          }

          // вообще ситуация возможна при которой мы не сможем взять рандомного соседа
          // это герцогство тогда должно пойти в состав другого, или позаимствовать

          if (province_index == UINT32_MAX) continue;

          unique_indices.insert(province_index);
          //queue.push(std::make_pair(pair.first, province_index));
          // тут наверное нужно опять просмотреть соседей текущего
          // то, что соседи могут "закнчится" и у текущей провинции и у соседней, кажется равновероятным
          queue.push(std::make_pair(pair.first, pair.second));
          duchies[pair.first].push_back(province_index);
        } else {
          for (size_t i = 0; i < neighbours.size(); ++i) {
//             const size_t border_size = neighbours_border_size[pair.second][i];
//             if (border_size < border_size_const) continue;
            //const auto n_province_index = province_neighbour(neighbours[i]);
            const auto n_province_index = province_neighbour(neighbours_border_size[pair.second][i].first);
            const size_t border_size = neighbours_border_size[pair.second][i].second;
            if (border_size < border_size_const) continue;
            if (unique_indices.find(n_province_index.index()) != unique_indices.end()) continue;


            bool found = false;
            const auto &n_neighbours = ctx->container->get_province_neighbours(n_province_index.index());
            for (size_t j = 0; j < n_neighbours.size(); ++j) {
              const auto n_n_province_index = province_neighbour(n_neighbours[j]);
              if (n_n_province_index.index() == pair.second) continue;
              if (n_province_index.across_water()) continue;

              auto itr = std::find(neighbours.begin(), neighbours.end(), n_n_province_index.container);
              if (itr != neighbours.end()) found = true;
            }

            if (!found) continue;

            unique_indices.insert(n_province_index.index());
            queue.push(std::make_pair(pair.first, n_province_index.index()));
            duchies[pair.first].push_back(n_province_index.index());
            break;
          }
        }
      }

      PRINT_VAR("duchies provinces count", unique_indices.size());
      PRINT_VAR("free provinces    count", province_count - unique_indices.size());

      // теперь нужно посмотреть чтобы было примерно равное распределение
      // и наверное добавить свободные провинции

      for (size_t i = 0; i < duchies.size(); ++i) {
        if (duchies[i].size() >= 6) continue;

        const uint32_t count = duchies[i].size();
        for (size_t j = 0; j < count; ++j) {
          const uint32_t province_index = duchies[i][j];
          if (duchies[i].size() >= 6) break;

          const auto &neighbours = ctx->container->get_province_neighbours(province_index);
          for (size_t k = 0; k < neighbours.size(); ++k) {
//             const size_t border_size = neighbours_border_size[province_index][k];
//             if (border_size < border_size_const) continue;
//             const auto n_province_index = province_neighbour(neighbours[k]);
            const auto n_province_index = province_neighbour(neighbours_border_size[province_index][k].first);
            const size_t border_size = neighbours_border_size[province_index][k].second;
            if (border_size < border_size_const) continue;
            ASSERT(province_count > n_province_index.index());
            if (n_province_index.across_water()) continue;
            if (duchies[i].size() >= 6) break;
            if (unique_indices.find(n_province_index.index()) != unique_indices.end()) continue;

            unique_indices.insert(n_province_index.index());
            duchies[i].push_back(n_province_index.index());
          }
        }
      }

      PRINT_VAR("duchies provinces count", unique_indices.size());

      const uint32_t current_duchies_count = duchies.size();
      for (size_t i = 0; i < province_count; ++i) {
        const uint32_t province_index = i;
        if (unique_indices.find(province_index) != unique_indices.end()) continue;

        duchies.emplace_back();
        duchies.back().push_back(province_index);
        unique_indices.insert(province_index);
      }

      ASSERT(unique_indices.size() == province_count);

      std::vector<uint32_t> province_duchy(province_count, UINT32_MAX);
      for (size_t i = 0; i < duchies.size(); ++i) {
        for (size_t j = 0; j < duchies[i].size(); ++j) {
          const uint32_t p_index = duchies[i][j];
          province_duchy[p_index] = i;
        }
      }

      for (size_t i = current_duchies_count; i < duchies.size(); ++i) {
        ASSERT(duchies[i].size() == 1);

        const uint32_t province_index = duchies[i][0];
        const auto &neighbours = ctx->container->get_province_neighbours(province_index);
        std::unordered_set<uint32_t> unique_indices;
        for (size_t k = 0; k < neighbours.size(); ++k) {
//           const size_t border_size = neighbours_border_size[province_index][k];
//           if (border_size < border_size_const) continue;
//           const auto n_province_index = province_neighbour(neighbours[k]);
          const auto n_province_index = province_neighbour(neighbours_border_size[province_index][k].first);
          const size_t border_size = neighbours_border_size[province_index][k].second;
          if (border_size < border_size_const) continue;
          const uint32_t duchy_index = province_duchy[n_province_index.index()];
          ASSERT(duchy_index != UINT32_MAX);

          if (n_province_index.across_water()) continue;
          if (duchies[duchy_index].size() < 5) continue;
          if (unique_indices.find(duchy_index) != unique_indices.end()) continue;

          unique_indices.insert(duchy_index);
          auto itr = std::find(duchies[duchy_index].begin(), duchies[duchy_index].end(), n_province_index.index());
          ASSERT(itr != duchies[duchy_index].end());

          duchies[duchy_index].erase(itr);
          duchies[i].push_back(n_province_index.index());
          province_duchy[n_province_index.index()] = i;
        }
      }

      // по прежнему могут остаться слишком маленькие герцогства
      // в некоторых случаях это остров на одну провинцию
      // в некоторых случайность
      // такие герцогства можно соединить
      const uint32_t duchies_count_stat_size = 7;
      uint32_t duchies_count_stat[duchies_count_stat_size];
      memset(duchies_count_stat, 0, duchies_count_stat_size * sizeof(uint32_t));
      uint32_t max_duchy = 0;
      uint32_t min_duchy = 10;
      size_t duchy_counter = 0;
      for (size_t i = 0; i < duchies.size(); ++i) {
        if (duchies[i].size() == 0) continue;
        if (duchies[i].size() == 1) ++duchies_count_stat[0];
        if (duchies[i].size() == 2) ++duchies_count_stat[1];
        if (duchies[i].size() == 3) ++duchies_count_stat[2];
        if (duchies[i].size() == 4) ++duchies_count_stat[3];
        if (duchies[i].size() == 5) ++duchies_count_stat[4];
        if (duchies[i].size() == 6) ++duchies_count_stat[5];
        if (duchies[i].size() > 6)  ++duchies_count_stat[6];

        ++duchy_counter;
        max_duchy = std::max(max_duchy, uint32_t(duchies[i].size()));
        min_duchy = std::min(min_duchy, uint32_t(duchies[i].size()));
      }

      PRINT_VAR("duchies count ", duchy_counter)
      PRINT_VAR("duchies size 1", duchies_count_stat[0])
      PRINT_VAR("duchies size 2", duchies_count_stat[1])
      PRINT_VAR("duchies size 3", duchies_count_stat[2])
      PRINT_VAR("duchies size 4", duchies_count_stat[3])
      PRINT_VAR("duchies size 5", duchies_count_stat[4])
      PRINT_VAR("duchies size 6", duchies_count_stat[5])
      PRINT_VAR("duchies size more 6", duchies_count_stat[6])
      PRINT_VAR("max duchy     ", max_duchy)
      PRINT_VAR("min duchy     ", min_duchy)

      // однопровинчатые нужно передать минимальному соседу да и все
      for (size_t i = 0; i < duchies.size(); ++i) {
        if (duchies[i].size() == 0) continue;
        if (duchies[i].size() > 2) continue;

        const uint32_t count = duchies[i].size();
        for (size_t j = 0; j < count; ++j) {
          const uint32_t province_index = duchies[i][j];
          const auto &neighbours = ctx->container->get_province_neighbours(province_index);
          bool found = false;
          for (size_t k = 0; k < neighbours.size(); ++k) {
//             const size_t border_size = neighbours_border_size[province_index][k];
//             if (border_size < border_size_const) continue;
//             const auto n_province_index = province_neighbour(neighbours[k]);
            const auto n_province_index = province_neighbour(neighbours_border_size[province_index][k].first);
            const size_t border_size = neighbours_border_size[province_index][k].second;
            if (border_size < border_size_const) continue; // ???
            const uint32_t n_duchy_index = province_duchy[n_province_index.index()];

            if (n_province_index.across_water()) continue;
            if (n_duchy_index == i) continue;

            const uint32_t union_index = std::max(n_duchy_index, uint32_t(i));
            const uint32_t another = union_index == n_duchy_index ? i : n_duchy_index;
            ASSERT(union_index != another);
            if (duchies[union_index].size() + duchies[another].size() > 4) continue;

            duchies[union_index].insert(duchies[union_index].end(), duchies[another].begin(), duchies[another].end());
            for (size_t l = 0; l < duchies[another].size(); ++l) {
              province_duchy[duchies[another][l]] = union_index;
            }

            duchies[another].clear();
            found = true;
            break;
          }

          if (found) break;
        }


      }

      for (size_t i = 0; i < duchies.size(); ++i) {
        if (duchies[i].size() == 0) continue;
        if (duchies[i].size() != 1) continue;

        const uint32_t province_index = duchies[i][0];
        const auto &neighbours = ctx->container->get_province_neighbours(province_index);
        for (size_t k = 0; k < neighbours.size(); ++k) {
//           const size_t border_size = neighbours_border_size[province_index][k];
//           if (border_size < border_size_const) continue;
//           const auto n_province_index = province_neighbour(neighbours[k]);
          const auto n_province_index = province_neighbour(neighbours_border_size[province_index][k].first);
          const uint32_t n_duchy_index = province_duchy[n_province_index.index()];

          if (n_province_index.across_water()) continue;
          if (n_duchy_index == i) continue;

          const uint32_t union_index = std::max(n_duchy_index, uint32_t(i));
          const uint32_t another = union_index == n_duchy_index ? i : n_duchy_index;
          ASSERT(union_index != another);
          if (duchies[union_index].size() + duchies[another].size() > 6) continue;

          duchies[union_index].insert(duchies[union_index].end(), duchies[another].begin(), duchies[another].end());
          for (size_t l = 0; l < duchies[another].size(); ++l) {
            province_duchy[duchies[another][l]] = union_index;
          }

          duchies[another].clear();
          break;
        }
      }

      memset(duchies_count_stat, 0, duchies_count_stat_size * sizeof(uint32_t));
      max_duchy = 0;
      min_duchy = 10;
      duchy_counter = 0;
      uint32_t check_duchies_count = 0;
      for (size_t i = 0; i < duchies.size(); ++i) {
        if (duchies[i].size() == 0) continue;
        if (duchies[i].size() == 1) ++duchies_count_stat[0];
        if (duchies[i].size() == 2) ++duchies_count_stat[1];
        if (duchies[i].size() == 3) ++duchies_count_stat[2];
        if (duchies[i].size() == 4) ++duchies_count_stat[3];
        if (duchies[i].size() == 5) ++duchies_count_stat[4];
        if (duchies[i].size() == 6) ++duchies_count_stat[5];
        if (duchies[i].size() > 6)  ++duchies_count_stat[6];

        check_duchies_count += duchies[i].size();
        for (size_t j = 0; j < duchies[i].size(); ++j) {
          const uint32_t index = duchies[i][j];
          province_duchy[index] = duchy_counter;
        }

        ++duchy_counter;
        max_duchy = std::max(max_duchy, uint32_t(duchies[i].size()));
        min_duchy = std::min(min_duchy, uint32_t(duchies[i].size()));
      }

      PRINT_VAR("duchies count ", duchy_counter)
      PRINT_VAR("duchies size 1", duchies_count_stat[0])
      PRINT_VAR("duchies size 2", duchies_count_stat[1])
      PRINT_VAR("duchies size 3", duchies_count_stat[2])
      PRINT_VAR("duchies size 4", duchies_count_stat[3])
      PRINT_VAR("duchies size 5", duchies_count_stat[4])
      PRINT_VAR("duchies size 6", duchies_count_stat[5])
      PRINT_VAR("duchies size more 6", duchies_count_stat[6])
      PRINT_VAR("max duchy     ", max_duchy)
      PRINT_VAR("min duchy     ", min_duchy)

      const uint32_t prov_count = ctx->container->entities_count(debug::entities::province);
      ASSERT(check_duchies_count == prov_count);

      (void)table;
      for (size_t i = 0; i < ctx->container->entities_count(debug::entities::tile); ++i) {
        ctx->container->set_data<uint32_t>(debug::entities::tile, i, debug::properties::tile::test_value_uint1, UINT32_MAX);
      }

      for (size_t i = 0; i < province_duchy.size(); ++i) {
        const uint32_t duchy_index = province_duchy[i];
        const auto &childs = ctx->container->get_childs(debug::entities::province, i);
        for (size_t j = 0; j < childs.size(); ++j) {
          const uint32_t tile_index = childs[j];
          ctx->container->set_data<uint32_t>(debug::entities::tile, tile_index, debug::properties::tile::test_value_uint1, duchy_index);
        }
      }

      for (auto itr = duchies.begin(); itr != duchies.end();) {
        if (itr->size() == 0) itr = duchies.erase(itr);
        else ++itr;
      }

      ASSERT(duchies.size() == duchy_counter);
      std::vector<std::vector<std::pair<uint32_t, uint32_t>>> duchies_neighbours(duchies.size());
      {
//         for (size_t i = 0; i < duchies_neighbours.size(); ++i) {
//
//         }

        std::unordered_set<size_t> unique_border;
        for (uint32_t i = 0; i < ctx->container->entities_count(debug::entities::tile); ++i) {
          const uint32_t duchy_index = ctx->container->get_data<uint32_t>(debug::entities::tile, i, debug::properties::tile::test_value_uint1);
          if (duchy_index == UINT32_MAX) continue;
          const auto &tile_data = render::unpack_data(ctx->map->get_tile(i));
          const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
          for (uint32_t j = 0; j < n_count; ++j) {
            const uint32_t n_index = tile_data.neighbours[j];
            const uint32_t n_duchy_index = ctx->container->get_data<uint32_t>(debug::entities::tile, n_index, debug::properties::tile::test_value_uint1);
            if (n_duchy_index == UINT32_MAX) continue;
            if (duchy_index == n_duchy_index) continue;

            const uint32_t first_index = std::max(i, n_index);
            const uint32_t second_index = std::min(i, n_index);
            const size_t key = (size_t(second_index) << 32) | size_t(first_index);
            if (unique_border.find(key) != unique_border.end()) continue;

            {
              auto itr = std::find_if(duchies_neighbours[duchy_index].begin(), duchies_neighbours[duchy_index].end(), [n_duchy_index] (const std::pair<uint32_t, uint32_t> &val) -> bool {
                return val.first == n_duchy_index;
              });

              if (itr == duchies_neighbours[duchy_index].end()) duchies_neighbours[duchy_index].push_back(std::make_pair(n_duchy_index, 1));
              else itr->second += 1;
            }

            {
              auto itr = std::find_if(duchies_neighbours[n_duchy_index].begin(), duchies_neighbours[n_duchy_index].end(), [duchy_index] (const std::pair<uint32_t, uint32_t> &val) -> bool {
                return val.first == duchy_index;
              });

              if (itr == duchies_neighbours[n_duchy_index].end()) duchies_neighbours[n_duchy_index].push_back(std::make_pair(duchy_index, 1));
              else itr->second += 1;
            }

            unique_border.insert(key);
          }
        }

        for (size_t i = 0; i < duchies_neighbours.size(); ++i) {
          //ASSERT(duchies_neighbours[i].size() != 0);
          std::sort(duchies_neighbours[i].begin(), duchies_neighbours[i].end(), [] (const auto &first, const auto &second) -> bool {
            return first.second > second.second;
          });
        }

//         std::vector<std::unordered_set<uint32_t>> tmp_duchies_neighbours(duchies.size());
//         for (size_t i = 0 ; i < duchies.size(); ++i) {
//           for (size_t j = 0; j < duchies[i].size(); ++j) {
//             const uint32_t province_index = duchies[i][j];
//             const auto &province_n = ctx->container->get_province_neighbours(province_index);
//             for (size_t k = 0; k < province_n.size(); ++k) {
// //               const size_t border_size = neighbours_border_size[province_index][k];
// //               if (border_size < border_size_const) continue;
//               const auto n_index = province_neighbour(province_n[k]);
//               const uint32_t n_duchy_index =  province_duchy[n_index.index()];
//
//               if (n_duchy_index != i) {
//                 tmp_duchies_neighbours[i].insert(n_duchy_index);
//                 tmp_duchies_neighbours[n_duchy_index].insert(i);
//               }
//             }
//           }
//         }
//
//         for (size_t i = 0; i < tmp_duchies_neighbours.size(); ++i) {
//           for (auto itr = tmp_duchies_neighbours[i].begin(); itr != tmp_duchies_neighbours[i].end(); ++itr) {
//             duchies_neighbours[i].push_back(*itr);
//           }
//         }
      }

      // теперь нужно создать королевства
      // проблема заключается в том что не существует какого то конвенционального числа герцогств в королевстве
      // в цк2 королеских титулов 116, примерно 4 герцогства на каждое
      // но разброс от 2 герцогства до 11, как быть
      // там еще есть множество формальных королевских титулов
      // но с этим еще более менее понятно

      // я так понимаю, нужно сначала заспавнить половину (2/3? 3/4?) из всех королевств
      // их заполнить до какого то предела (например 12 герцогств), а затем попытаться создать
      // оставшееся количество титулов, все королевства с одним герцогством удалить и раздать существующим
      // малые королевства соединить (непохо было бы с учетом длинны границы)
      // герцогства по примерно такому же принципу более менее нормально генерируются

      //const uint32_t king_titles_count = duchy_counter / 4;
      const uint32_t king_titles_count_max = duchy_counter / 4;
      const uint32_t king_titles_count = king_titles_count_max * (2.0f/3.0f);
      std::unordered_set<uint32_t> unique_duchies;
      std::queue<std::tuple<uint32_t, uint32_t, uint32_t>> king_queue;
      std::vector<std::vector<uint32_t>> kingdoms(king_titles_count);
      std::vector<uint32_t> duchy_kingdom(duchy_counter, UINT32_MAX);
      ASSERT(kingdoms.size() != 0);

      const auto func = [&] () {
        ASSERT(!king_queue.empty());
        while (!king_queue.empty()) {
          const auto data = king_queue.front();
          king_queue.pop();

          const uint32_t king_index = std::get<0>(data);
  //         if (kingdoms[king_index].size() > 11) continue;
          const uint32_t kingdom_duchy_index = std::get<1>(data);
          if (kingdom_duchy_index >= kingdoms[king_index].size()) continue;
          const uint32_t current_duchy_index = kingdoms[king_index][kingdom_duchy_index];
          uint32_t current_n_index = std::get<2>(data);

          ASSERT(current_duchy_index < duchies_neighbours.size());

          // нужно еще проверить случайного соседа
          // нужно проверить соседей осортированных по длине границы герцогства (проверяю)
          // по длине границы получаются в основном неплохие королевства
          bool found = false;
          for (; current_n_index < duchies_neighbours[current_duchy_index].size(); ++current_n_index) {
            const auto index = duchies_neighbours[current_duchy_index][current_n_index];
            //if (unique_duchies.find(index.first) != unique_duchies.end()) continue;
            if (index.second < 6) continue;
            if (duchy_kingdom[index.first] != UINT32_MAX) continue;

  //           unique_duchies.insert(index.first);
            if (current_n_index < duchies_neighbours[current_duchy_index].size()-1) {
              king_queue.push(std::make_tuple(king_index, kingdom_duchy_index, current_n_index+1));
            } else {
              king_queue.push(std::make_tuple(king_index, kingdom_duchy_index+1, 0));
            }

            found = true;
            kingdoms[king_index].push_back(index.first);
            ASSERT(duchy_kingdom[index.first] == UINT32_MAX);
            duchy_kingdom[index.first] = king_index;
            break;
          }

          if (!found) king_queue.push(std::make_tuple(king_index, kingdom_duchy_index+1, 0));
        }
      };

      for (size_t i = 0; i < king_titles_count; ++i) {
        uint32_t duchy_index = UINT32_MAX;
        uint32_t attempts = 0;
        while (duchy_index == UINT32_MAX && attempts < 100) {
          ++attempts;
          const uint32_t rand_index = ctx->random->index(duchies.size());
          ASSERT(duchies[rand_index].size() != 0);
          if (unique_duchies.find(rand_index) != unique_duchies.end()) continue;

//           bool found = true;
//           for (size_t j = 0; j < duchies_neighbours[rand_index].size(); ++j) {
//             if (unique_duchies.find(duchies_neighbours[rand_index][j].first) != unique_duchies.end()) {
//               found = false;
//               break;
//             }
//           }
//
//           if (!found) continue;
          duchy_index = rand_index;
        }

        ASSERT(duchy_index != UINT32_MAX);

        unique_duchies.insert(duchy_index);
        king_queue.push(std::make_tuple(i, 0, 0));
        kingdoms[i].push_back(duchy_index);
        duchy_kingdom[duchy_index] = i;

        for (size_t j = 0; j < duchies_neighbours[duchy_index].size(); ++j) {
          unique_duchies.insert(duchies_neighbours[duchy_index][j].first);
        }
      }

      func();

      for (size_t i = 0; i < duchy_kingdom.size(); ++i) {
        if (duchy_kingdom[i] != UINT32_MAX) continue;
        if (duchies[i].size() != 6) continue;

        duchy_kingdom[i] = kingdoms.size();
        kingdoms.emplace_back();
        kingdoms.back().push_back(i);
        king_queue.push(std::make_tuple(duchy_kingdom[i], 0, 0));
      }

      for (size_t i = 0; i < duchy_kingdom.size(); ++i) {
        if (duchy_kingdom[i] != UINT32_MAX) continue;
        if (duchies[i].size() < 4) continue;

        bool found = false;
        for (size_t j = 0; j < duchies[i].size(); ++j) {
          const uint32_t province_index = duchies[i][j];
          ASSERT(province_duchy[province_index] == i);

          const auto &n = ctx->container->get_province_neighbours(province_index);
          for (size_t k = 0; k < n.size(); ++k) {
            const auto n_province_index = province_neighbour(n[k]);
            const uint32_t n_duchy_index = province_duchy[n_province_index.index()];
            if (n_duchy_index == i) continue;
            if (duchy_kingdom[i] != UINT32_MAX) continue;
            found = true;
          }
        }

        if (!found) continue;

        duchy_kingdom[i] = kingdoms.size();
        kingdoms.emplace_back();
        kingdoms.back().push_back(i);
        king_queue.push(std::make_tuple(duchy_kingdom[i], 0, 0));
      }

      if (!king_queue.empty()) func();

      {
        uint32_t max_kingdoms = 0;
        uint32_t min_kingdoms = 235525;
        uint32_t max_kingdoms_provinces = 0;
        uint32_t min_kingdoms_provinces = 34363436;
        uint32_t kingdoms2_count = 0;
        std::vector<uint32_t> province_count(kingdoms.size(), 0);
        size_t counter = 0;
        for (size_t i = 0; i < kingdoms.size(); ++i) {
          max_kingdoms = std::max(max_kingdoms, uint32_t(kingdoms[i].size()));
          min_kingdoms = std::min(min_kingdoms, uint32_t(kingdoms[i].size()));

          for (uint32_t k = 0; k < kingdoms[i].size(); ++k) {
            for (uint32_t c = 0; c < duchies[k].size(); ++c) {
              //const uint32_t baron_index = duchies[k][c];
              ++province_count[i];
            }
          }

          counter += province_count[i];
          max_kingdoms_provinces = std::max(max_kingdoms_provinces, province_count[i]);
          min_kingdoms_provinces = std::min(min_kingdoms_provinces, province_count[i]);
          if (kingdoms[i].size() == 1) ++kingdoms2_count;
        }

        PRINT_VAR("kingdoms_count ", kingdoms.size())
        PRINT_VAR("max_kingdoms   ", max_kingdoms)
        PRINT_VAR("min_kingdoms   ", min_kingdoms)
        PRINT_VAR("kingdoms1_count", kingdoms2_count)
        PRINT_VAR("avg kingdoms count", float(duchy_kingdom.size()) / float(kingdoms.size()))
        PRINT_VAR("max_kingdoms_provinces", max_kingdoms_provinces)
        PRINT_VAR("min_kingdoms_provinces", min_kingdoms_provinces)
        PRINT_VAR("avg kingdoms provinces count", float(counter) / float(kingdoms.size()))
      }

      std::vector<uint32_t> province_kingdom(ctx->container->entities_count(debug::entities::province), UINT32_MAX);
      for (size_t i = 0; i < kingdoms.size(); ++i) {
        for (size_t j = 0; j < kingdoms[i].size(); ++j) {
          const uint32_t duchy_index = kingdoms[i][j];
          for (size_t k = 0; k < duchies[duchy_index].size(); ++k) {
            const uint32_t provice_index = duchies[duchy_index][k];
            province_kingdom[provice_index] = i;
          }
        }
      }

      for (size_t i = 0; i < ctx->container->entities_count(debug::entities::tile); ++i) {
        ctx->container->set_data<uint32_t>(debug::entities::tile, i, debug::properties::tile::test_value_uint2, UINT32_MAX);
      }

      for (size_t i = 0; i < province_kingdom.size(); ++i) {
        const uint32_t kingdom_index = province_kingdom[i];
        const auto &childs = ctx->container->get_childs(debug::entities::province, i);
        for (size_t j = 0; j < childs.size(); ++j) {
          const uint32_t tile_index = childs[j];
          ctx->container->set_data<uint32_t>(debug::entities::tile, tile_index, debug::properties::tile::test_value_uint2, kingdom_index);
        }
      }

      // получается плохо, большая часть королевских титулов имеет слишком случайную форму
      // теперь получается более менее нормально, почти все королевские титулы правильной формы

      // нужно сгенерить имперские титулы (в цк2 26 реальных имперских титулов)
      // тот же принцип, находим соседей с протяженной границей, сортируем
      //

      std::vector<std::vector<std::pair<uint32_t, uint32_t>>> kingdoms_neighbours(kingdoms.size());
      {
//         for (size_t i = 0; i < duchies_neighbours.size(); ++i) {
//
//         }

        std::unordered_set<size_t> unique_border;
        for (uint32_t i = 0; i < ctx->container->entities_count(debug::entities::tile); ++i) {
          const uint32_t kingdom_index = ctx->container->get_data<uint32_t>(debug::entities::tile, i, debug::properties::tile::test_value_uint2);
          if (kingdom_index == UINT32_MAX) continue;
          const auto &tile_data = render::unpack_data(ctx->map->get_tile(i));
          const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
          for (uint32_t j = 0; j < n_count; ++j) {
            const uint32_t n_index = tile_data.neighbours[j];
            const uint32_t n_kingdom_index = ctx->container->get_data<uint32_t>(debug::entities::tile, n_index, debug::properties::tile::test_value_uint2);
            if (n_kingdom_index == UINT32_MAX) continue;
            if (kingdom_index == n_kingdom_index) continue;

            const uint32_t first_index = std::max(i, n_index);
            const uint32_t second_index = std::min(i, n_index);
            const size_t key = (size_t(second_index) << 32) | size_t(first_index);
            if (unique_border.find(key) != unique_border.end()) continue;

            {
              auto itr = std::find_if(kingdoms_neighbours[kingdom_index].begin(), kingdoms_neighbours[kingdom_index].end(), [n_kingdom_index] (const std::pair<uint32_t, uint32_t> &val) -> bool {
                return val.first == n_kingdom_index;
              });

              if (itr == kingdoms_neighbours[kingdom_index].end()) kingdoms_neighbours[kingdom_index].push_back(std::make_pair(n_kingdom_index, 1));
              else itr->second += 1;
            }

            {
              auto itr = std::find_if(kingdoms_neighbours[n_kingdom_index].begin(), kingdoms_neighbours[n_kingdom_index].end(), [kingdom_index] (const std::pair<uint32_t, uint32_t> &val) -> bool {
                return val.first == kingdom_index;
              });

              if (itr == kingdoms_neighbours[n_kingdom_index].end()) kingdoms_neighbours[n_kingdom_index].push_back(std::make_pair(kingdom_index, 1));
              else itr->second += 1;
            }

            unique_border.insert(key);
          }
        }

        for (size_t i = 0; i < kingdoms_neighbours.size(); ++i) {
          //ASSERT(duchies_neighbours[i].size() != 0);
          std::sort(kingdoms_neighbours[i].begin(), kingdoms_neighbours[i].end(), [] (const auto &first, const auto &second) -> bool {
            return first.second > second.second;
          });
        }
      }

      const uint32_t emperor_titles_count_max = kingdoms.size() / 4;
      const uint32_t emperor_titles_count = emperor_titles_count_max; // * (2.0f/3.0f)
      std::unordered_set<uint32_t> unique_kingdoms;
      std::queue<std::tuple<uint32_t, uint32_t, uint32_t>> emperor_queue;
      std::vector<std::vector<uint32_t>> empires(emperor_titles_count);
      std::vector<uint32_t> kingdom_empire(kingdoms.size(), UINT32_MAX);

      const auto empire_func = [&] () {
        ASSERT(!emperor_queue.empty());
        while (!emperor_queue.empty()) {
          const auto data = emperor_queue.front();
          emperor_queue.pop();

          const uint32_t emperor_index = std::get<0>(data);
  //         if (kingdoms[king_index].size() > 11) continue;
          const uint32_t empire_kingdom_index = std::get<1>(data);
          if (empire_kingdom_index >= empires[emperor_index].size()) continue;
          const uint32_t current_kingdom_index = empires[emperor_index][empire_kingdom_index];
          uint32_t current_n_index = std::get<2>(data);

          ASSERT(current_kingdom_index < kingdoms_neighbours.size());

          // нужно еще проверить случайного соседа
          // нужно проверить соседей осортированных по длине границы герцогства (проверяю)
          // по длине границы получаются в основном неплохие королевства
          bool found = false;
          for (; current_n_index < kingdoms_neighbours[current_kingdom_index].size(); ++current_n_index) {
            const auto index = kingdoms_neighbours[current_kingdom_index][current_n_index];
            //if (unique_duchies.find(index.first) != unique_duchies.end()) continue;
            if (index.second < 6) continue;
            if (kingdom_empire[index.first] != UINT32_MAX) continue;

  //           unique_duchies.insert(index.first);
            if (current_n_index < kingdoms_neighbours[current_kingdom_index].size()-1) {
              emperor_queue.push(std::make_tuple(emperor_index, empire_kingdom_index, current_n_index+1));
            } else {
              emperor_queue.push(std::make_tuple(emperor_index, empire_kingdom_index+1, 0));
            }

            found = true;
            empires[emperor_index].push_back(index.first);
            ASSERT(kingdom_empire[index.first] == UINT32_MAX);
            kingdom_empire[index.first] = emperor_index;
            break;
          }

          if (!found) emperor_queue.push(std::make_tuple(emperor_index, empire_kingdom_index+1, 0));
        }
      };

      for (size_t i = 0; i < empires.size(); ++i) {
        uint32_t attempts = 0;
        uint32_t kingdom_index = UINT32_MAX;
        while (kingdom_index == UINT32_MAX && attempts < 100) {
          ++attempts;
          const uint32_t rand_index = ctx->random->index(kingdoms.size());
          if (unique_kingdoms.find(rand_index) != unique_kingdoms.end()) continue;

          kingdom_index = rand_index;
        }

        unique_kingdoms.insert(kingdom_index);
        empires[i].push_back(kingdom_index);
        kingdom_empire[kingdom_index] = i;
        emperor_queue.push(std::make_tuple(i, 0, 0));

        for (size_t j = 0; j < kingdoms_neighbours[kingdom_index].size(); ++j) {
          unique_kingdoms.insert(kingdoms_neighbours[kingdom_index][j].first);
        }
      }

      empire_func();

      // у нас остаются империи с одним королевством, мы их либо добавляем соседям либо удаляем
      // наверное империи должны быть минимум с 3-мя королевствами
      for (size_t i = 0; i < empires.size(); ++i) {
        if (empires[i].size() > 2) continue;
        for (size_t k = 0; k < empires[i].size(); ++k) {
          const uint32_t kingdom_index = empires[i][k];

          bool found = false;
          for (size_t j = 0; j < kingdoms_neighbours[kingdom_index].size(); ++j) {
            const uint32_t index = kingdoms_neighbours[kingdom_index][j].first;
            const uint32_t new_emp = kingdom_empire[index];
            if (new_emp == UINT32_MAX) continue;
            if (empires[new_emp].size() < 3) continue;

            kingdom_empire[kingdom_index] = new_emp;
            empires[new_emp].insert(empires[new_emp].end(), empires[i].begin(), empires[i].end());
            empires[i].clear();
            found = true;
            break;
          }

          if (found) continue;

          // удалить
          empires[i].erase(empires[i].begin()+k);
          kingdom_empire[kingdom_index] = UINT32_MAX;
        }
      }

      for (size_t i = 0; i < kingdom_empire.size(); ++i) {
        if (kingdom_empire[i] != UINT32_MAX) continue;

        const uint32_t kingdom_index = i;
        uint32_t found = 0;
        for (size_t j = 0; j < kingdoms_neighbours[kingdom_index].size(); ++j) {
          const uint32_t n_kingdom_index = kingdoms_neighbours[kingdom_index][j].first;
          if (kingdom_empire[n_kingdom_index] != UINT32_MAX) continue;

          ++found;
        }

        if (found < 2) continue;

        const uint32_t new_emp = empires.size();
        empires.emplace_back();

        empires[new_emp].push_back(kingdom_index);
        kingdom_empire[kingdom_index] = new_emp;
        emperor_queue.push(std::make_tuple(new_emp, 0, 0));
        for (size_t j = 0; j < kingdoms_neighbours[kingdom_index].size(); ++j) {
          const uint32_t n_kingdom_index = kingdoms_neighbours[kingdom_index][j].first;
          if (kingdom_empire[n_kingdom_index] != UINT32_MAX) continue;

          empires[new_emp].push_back(n_kingdom_index);
          kingdom_empire[n_kingdom_index] = new_emp;
        }
      }

      if (!emperor_queue.empty()) empire_func();

      for (auto itr = empires.begin(); itr != empires.end();) {
        if (itr->size() == 0) itr = empires.erase(itr);
        else ++itr;
      }

      {
        uint32_t max_empires = 0;
        uint32_t min_empires = 235525;
        uint32_t max_provinces_empires = 0;
        uint32_t min_provinces_empires = 45366737;
        uint32_t empires1_count = 0;
        uint32_t empires11_count = 0;
        std::vector<uint32_t> province_count(empires.size(), 0);
        size_t counter = 0;
        for (size_t i = 0; i < empires.size(); ++i) {
          if (empires[i].size() == 0) continue;

          for (uint32_t j = 0; j < empires[i].size(); ++j) {
            for (uint32_t k = 0; k < kingdoms[j].size(); ++k) {
              for (uint32_t c = 0; c < duchies[k].size(); ++c) {
                //const uint32_t baron_index = duchies[k][c];
                ++province_count[i];
              }
            }
          }

          counter += province_count[i];
          max_provinces_empires = std::max(max_provinces_empires, province_count[i]);
          min_provinces_empires = std::min(min_provinces_empires, province_count[i]);
          max_empires = std::max(max_empires, uint32_t(empires[i].size()));
          min_empires = std::min(min_empires, uint32_t(empires[i].size()));
          if (empires[i].size() == 3) ++empires1_count;
          if (empires[i].size() == 11) ++empires11_count;
        }

        PRINT_VAR("empires_count  ", empires.size())
        PRINT_VAR("max_empires    ", max_empires)
        PRINT_VAR("min_empires    ", min_empires)
        PRINT_VAR("empires3_count ", empires1_count)
        PRINT_VAR("empires11_count", empires11_count)
        PRINT_VAR("avg empires count", float(kingdom_empire.size()) / float(empires.size()))
        PRINT_VAR("max_provinces_empires", max_provinces_empires)
        PRINT_VAR("min_provinces_empires", min_provinces_empires)
        PRINT_VAR("avg empires provinces count", float(counter) / float(empires.size()))
      }

      std::vector<uint32_t> province_empire(ctx->container->entities_count(debug::entities::province), UINT32_MAX);
      uint32_t counter = 0;
      for (size_t i = 0; i < empires.size(); ++i) {
//         if (empires.size() == 0) continue;
        ASSERT(empires.size() != 0);

        for (size_t j = 0; j < empires[i].size(); ++j) {
          const uint32_t kingdom_index = empires[i][j];
          kingdom_empire[kingdom_index] = counter;

          for (size_t k = 0; k < kingdoms[kingdom_index].size(); ++k) {
            const uint32_t duchy_index = kingdoms[kingdom_index][k];
            for (size_t c = 0; c < duchies[duchy_index].size(); ++c) {
              const uint32_t provice_index = duchies[duchy_index][c];
              province_empire[provice_index] = counter;
            }
          }
        }

        ++counter;
      }

      for (size_t i = 0; i < ctx->container->entities_count(debug::entities::tile); ++i) {
        ctx->container->set_data<uint32_t>(debug::entities::tile, i, debug::properties::tile::test_value_uint3, UINT32_MAX);
      }

      for (size_t i = 0; i < province_empire.size(); ++i) {
        const uint32_t empire_index = province_empire[i];
        const auto &childs = ctx->container->get_childs(debug::entities::province, i);
        for (size_t j = 0; j < childs.size(); ++j) {
          const uint32_t tile_index = childs[j];
          ctx->container->set_data<uint32_t>(debug::entities::tile, tile_index, debug::properties::tile::test_value_uint3, empire_index);
        }
      }

      // выглядит с первого раза все более менее
      // скорее всего история должна генерироваться на основе титулов
      // титулы - это отражение предыдущих и последующих достижений и амбиций

      // как раздать титулы персонажам? нужно раздать покрайней мере несколько баронских титулов главе государства,
      // один-два герцогства в зависимости от размера, королевство (размер > 20?) и империю (размер > 100?),
      // желательно чтобы титулы находились все рядом друг с другом
      // в цк2 на старте игры у всех императоров нет королевских титулов, все существующие королевские титулы независимы,
      // у муслимов спавняться сильные вассалы занимающие несколько провинций с герцогским титулом,
      // у католиков в разнобой сильные вассалы со слабыми, у остальных в основном (!) сильные вассалы
      // нужно начать с раздачи титулов верхнего уровня

      // титулы верхнего уровня не могут привести ко всем титулам нижнего уровня

      const size_t titles_counter = empires.size() + kingdoms.size() + duchies.size() + ctx->container->entities_count(debug::entities::province);
      const size_t emp_offset = 0;
      const size_t king_offset = empires.size();
      const size_t duchy_offset = empires.size() + kingdoms.size();
      const size_t baron_offset = empires.size() + kingdoms.size() + duchies.size();
      ctx->container->set_entity_count(debug::entities::title, titles_counter);

      table["emp_offset"] = emp_offset;
      table["king_offset"] = king_offset;
      table["duchy_offset"] = duchy_offset;
      table["baron_offset"] = baron_offset;

      PRINT_VAR("emp_offset  ", emp_offset)
      PRINT_VAR("king_offset ", king_offset)
      PRINT_VAR("duchy_offset", duchy_offset)
      PRINT_VAR("baron_offset", baron_offset)
      PRINT_VAR("titles_count", titles_counter)

      for (size_t empire_index = 0; empire_index < empires.size(); ++empire_index) {
        ctx->container->set_data<uint32_t>(debug::entities::title, emp_offset + empire_index, debug::properties::title::parent, UINT32_MAX);
        ctx->container->set_data<uint32_t>(debug::entities::title, emp_offset + empire_index, debug::properties::title::owner, UINT32_MAX);
      }

      for (size_t kingdom_index = 0; kingdom_index < kingdoms.size(); ++kingdom_index) {
        ctx->container->set_data<uint32_t>(debug::entities::title, king_offset + kingdom_index, debug::properties::title::parent, UINT32_MAX);
        ctx->container->set_data<uint32_t>(debug::entities::title, king_offset + kingdom_index, debug::properties::title::owner, UINT32_MAX);
      }

      for (size_t duchy_index = 0; duchy_index < duchies.size(); ++duchy_index) {
        ctx->container->set_data<uint32_t>(debug::entities::title, duchy_offset + duchy_index, debug::properties::title::parent, UINT32_MAX);
        ctx->container->set_data<uint32_t>(debug::entities::title, duchy_offset + duchy_index, debug::properties::title::owner, UINT32_MAX);
      }

      for (size_t baron_index = 0; baron_index < ctx->container->entities_count(debug::entities::province); ++baron_index) {
        ctx->container->set_data<uint32_t>(debug::entities::title, baron_offset + baron_index, debug::properties::title::parent, UINT32_MAX);
        ctx->container->set_data<uint32_t>(debug::entities::title, baron_offset + baron_index, debug::properties::title::owner, UINT32_MAX);

      }

      // начинаем создавать титулы
      for (size_t i = 0; i < empires.size(); ++i) {
        const uint32_t empire_index = i;
//         auto emp_t = ctx->container->create_titulus(core::titulus::type::imperial, empires[empire_index].size());
        auto emp_title = global::get<sol::state>()->create_table();
        const std::string emp_id = "imperial" + std::to_string(empire_index) + "_title";
        emp_title["id"] = emp_id;
        emp_title["type"] = core::titulus::type::imperial;
        gen_title_color(emp_title, empire_index);
        emp_title["heraldy"] = "shield_layer";
        const size_t emp_dbg_index = utils::add_title(emp_title);

//         if (empire_index == 14) {
//           PRINT_VAR("emp_dbg_index", emp_dbg_index)
//           PRINT_VAR("emp_id", emp_id)
//         }

//         ASSERT(empire_index != 14);
//         emp_title["count"] = empires[empire_index].size();
//         table[emp_id] = emp_title;

        // по сути мы используем индекс
        // имя нужно будет генерировать на основе культур и местности,
        // я так понимаю нужно сначало сгенерировать какую то информацию о титулах,
        // а потом собственно информацию о местности, возможно нужно придумать регион
        // (например католический мир) в котором имена заспавним какие то католические
        // регион распространения религии? что-то вроде

        for (size_t j = 0; j < empires[empire_index].size(); ++j) {
          const uint32_t kingdom_index = empires[empire_index][j];
//           auto king_t = ctx->container->create_titulus(core::titulus::type::king, kingdoms[kingdom_index].size());
//           emp_t->set_child(j, king_t);
//           king_t->parent = emp_t;
          ctx->container->add_child(debug::entities::title, emp_offset + empire_index, king_offset + kingdom_index);
          auto king_title = global::get<sol::state>()->create_table();
          const std::string king_id = "king" + std::to_string(kingdom_index) + "_title";
          king_title["id"] = king_id;
          king_title["type"] = core::titulus::type::king;
          king_title["parent"] = emp_id;
          king_title["heraldy"] = "shield_layer";
          gen_title_color(king_title, king_offset + kingdom_index);
          utils::add_title(king_title);
//           king_title["count"] = kingdoms[kingdom_index].size();
//           table[king_id] = king_title;

          ctx->container->set_data<uint32_t>(debug::entities::title, king_offset + kingdom_index, debug::properties::title::parent, emp_offset + empire_index);
          ctx->container->set_data<uint32_t>(debug::entities::title, king_offset + kingdom_index, debug::properties::title::owner, UINT32_MAX);

          for (size_t k = 0; k < kingdoms[kingdom_index].size(); ++k) {
            const uint32_t duchy_index = kingdoms[kingdom_index][k];
//             auto duchy_t = ctx->container->create_titulus(core::titulus::type::duke, duchies[duchy_index].size());
//             king_t->set_child(k, duchy_t);
//             duchy_t->parent = king_t;
            ctx->container->add_child(debug::entities::title, king_offset + kingdom_index, duchy_offset + duchy_index);
            auto duchy_title = global::get<sol::state>()->create_table();
            const std::string duchy_id = "duke" + std::to_string(duchy_index) + "_title";
            duchy_title["id"] = duchy_id;
            duchy_title["type"] = core::titulus::type::duke;
            duchy_title["parent"] = king_id;
            duchy_title["heraldy"] = "shield_layer";
            gen_title_color(duchy_title, duchy_offset + duchy_index);
            utils::add_title(duchy_title);
//             duchy_title["count"] = duchies[duchy_index].size();
//             table[duchy_id] = duchy_title;

            ctx->container->set_data<uint32_t>(debug::entities::title, duchy_offset + duchy_index, debug::properties::title::parent, king_offset + kingdom_index);
            ctx->container->set_data<uint32_t>(debug::entities::title, duchy_offset + duchy_index, debug::properties::title::owner, UINT32_MAX);

            for (size_t c = 0; c < duchies[duchy_index].size(); ++c) {
              const uint32_t baron_index = duchies[duchy_index][c]; // по всей видимости я где то теряю провинцию, странно кажется я проверял
//               auto baron_t = ctx->container->create_titulus(core::titulus::type::baron, 1);
//               duchy_t->set_child(c, baron_t);
//               baron_t->parent = duchy_t;
//
//               ASSERT(baron_index < province_count);
//               baron_t->set_province(baron_index);

              ctx->container->add_child(debug::entities::title, duchy_offset + duchy_index, baron_offset + baron_index);
              auto baron_title = global::get<sol::state>()->create_table();
              const std::string baron_id = "baron" + std::to_string(baron_index) + "_title";
              baron_title["id"] = baron_id;
              baron_title["type"] = core::titulus::type::baron;
              baron_title["parent"] = duchy_id;
              baron_title["heraldy"] = "shield_layer";
              baron_title["province"] = baron_index;
              gen_title_color(baron_title, baron_offset + baron_index);
              utils::add_title(baron_title);
//               table[baron_id] = baron_title; // при 5000 провинций получится около 6000 титулов (в общем минус оперативная память)

              ctx->container->set_data<uint32_t>(debug::entities::title, baron_offset + baron_index, debug::properties::title::parent, duchy_offset + duchy_index);
              ctx->container->set_data<uint32_t>(debug::entities::title, baron_offset + baron_index, debug::properties::title::owner, UINT32_MAX);

//               const uint32_t title_index = ctx->container->get_title_index(baron_t);
//               ctx->container->set_data<uint32_t>(debug::entities::province, baron_index, debug::properties::province::title_index, title_index);

//               auto city_title = global::get<sol::state>()->create_table();
//               const std::string city_id = "city" + std::to_string(baron_index) + "_title";
//               city_title["id"] = city_id;
//               city_title["type"] = core::titulus::type::baron;
//               city_title["parent"] = baron_id;
//               city_title["city"] = baron_index;
//               utils::add_title(city_title);
            }
          }
        }
      }

      for (size_t kingdom_index = 0; kingdom_index < kingdoms.size(); ++kingdom_index) {
        const uint32_t parent = ctx->container->get_data<uint32_t>(debug::entities::title, king_offset + kingdom_index, debug::properties::title::parent);
        if (parent != UINT32_MAX) continue;

        auto king_title = global::get<sol::state>()->create_table();
        const std::string king_id = "king" + std::to_string(kingdom_index) + "_title";
        king_title["id"] = king_id;
        king_title["type"] = core::titulus::type::king;
        king_title["heraldy"] = "shield_layer";
        gen_title_color(king_title, king_offset + kingdom_index);
        utils::add_title(king_title);

        for (size_t i = 0; i < kingdoms[kingdom_index].size(); ++i) {
          const uint32_t duchy_index = kingdoms[kingdom_index][i];
          const uint32_t parent = ctx->container->get_data<uint32_t>(debug::entities::title, duchy_offset + duchy_index, debug::properties::title::parent);
          ASSERT(parent == UINT32_MAX);

          ctx->container->add_child(debug::entities::title, king_offset + kingdom_index, duchy_offset + duchy_index);
          auto duchy_title = global::get<sol::state>()->create_table();
          const std::string duchy_id = "duke" + std::to_string(duchy_index) + "_title";
          duchy_title["id"] = duchy_id;
          duchy_title["type"] = core::titulus::type::duke;
          duchy_title["parent"] = king_id;
          duchy_title["heraldy"] = "shield_layer";
          gen_title_color(duchy_title, duchy_offset + duchy_index);
          utils::add_title(duchy_title);

          ctx->container->set_data<uint32_t>(debug::entities::title, duchy_offset + duchy_index, debug::properties::title::parent, king_offset + kingdom_index);

          for (size_t c = 0; c < duchies[duchy_index].size(); ++c) {
            const uint32_t baron_index = duchies[duchy_index][c];
            const uint32_t parent = ctx->container->get_data<uint32_t>(debug::entities::title, baron_offset + baron_index, debug::properties::title::parent);
            ASSERT(parent == UINT32_MAX);

            ctx->container->add_child(debug::entities::title, duchy_offset + duchy_index, baron_offset + baron_index);
            auto baron_title = global::get<sol::state>()->create_table();
            const std::string baron_id = "baron" + std::to_string(baron_index) + "_title";
            baron_title["id"] = baron_id;
            baron_title["type"] = core::titulus::type::baron;
            baron_title["parent"] = duchy_id;
            baron_title["province"] = baron_index;
            baron_title["heraldy"] = "shield_layer";
            gen_title_color(baron_title, baron_offset + baron_index);
            utils::add_title(baron_title);

            ctx->container->set_data<uint32_t>(debug::entities::title, baron_offset + baron_index, debug::properties::title::parent, duchy_offset + duchy_index);
          }
        }
      }

      for (size_t duchy_index = 0; duchy_index < duchies.size(); ++duchy_index) {
        const uint32_t parent = ctx->container->get_data<uint32_t>(debug::entities::title, duchy_offset + duchy_index, debug::properties::title::parent);
        if (parent != UINT32_MAX) continue;

        auto duchy_title = global::get<sol::state>()->create_table();
        const std::string duchy_id = "duke" + std::to_string(duchy_index) + "_title";
        duchy_title["id"] = duchy_id;
        duchy_title["type"] = core::titulus::type::duke;
        duchy_title["heraldy"] = "shield_layer";
        gen_title_color(duchy_title, duchy_offset + duchy_index);
        utils::add_title(duchy_title);

        for (size_t c = 0; c < duchies[duchy_index].size(); ++c) {
          const uint32_t baron_index = duchies[duchy_index][c];
          const uint32_t parent = ctx->container->get_data<uint32_t>(debug::entities::title, baron_offset + baron_index, debug::properties::title::parent);
          ASSERT(parent == UINT32_MAX);

          ctx->container->add_child(debug::entities::title, duchy_offset + duchy_index, baron_offset + baron_index);
          auto baron_title = global::get<sol::state>()->create_table();
          const std::string baron_id = "baron" + std::to_string(baron_index) + "_title";
          baron_title["id"] = baron_id;
          baron_title["type"] = core::titulus::type::baron;
          baron_title["parent"] = duchy_id;
          baron_title["province"] = baron_index;
          baron_title["heraldy"] = "shield_layer";
          gen_title_color(baron_title, baron_offset + baron_index);
          utils::add_title(baron_title);

          ctx->container->set_data<uint32_t>(debug::entities::title, baron_offset + baron_index, debug::properties::title::parent, duchy_offset + duchy_index);
        }
      }

      for (size_t baron_index = 0; baron_index < ctx->container->entities_count(debug::entities::province); ++baron_index) {
        const uint32_t parent = ctx->container->get_data<uint32_t>(debug::entities::title, baron_offset + baron_index, debug::properties::title::parent);
        if (parent != UINT32_MAX) continue;

        auto baron_title = global::get<sol::state>()->create_table();
        const std::string baron_id = "baron" + std::to_string(baron_index) + "_title";
        baron_title["id"] = baron_id;
        baron_title["type"] = core::titulus::type::baron;
        baron_title["province"] = baron_index;
        baron_title["heraldy"] = "shield_layer";
        gen_title_color(baron_title, baron_offset + baron_index);
        utils::add_title(baron_title);
      }

      // нам нужно каким то образом где то еще эти титулы запомнить
      // во первых для того чтобы нарисовать для них интерфейс и выделить их на карте
      // во вторых для того чтобы с ними можно было взаимодействовать
      // где их хранить? лучше всего в провинциях (это логично и меньше места занимает)
      // для всех титулов необходимо еще сгенерировать эмблему
      // отличный механизм сделан уже в цк2 https://ck2.paradoxwikis.com/Coats_of_arms_modding
      // нужно только сильно переделать его в угоду рандомной генерации
      // для того чтобы хранить какие то такие данные не в тайлах
      // нужно немного переделать функцию которая заполняет данными рендер
      // пользователь должен смочь сделать функцию в луа с любыми данными

      // тут есть одна проблема: как правильно выстроить иерархию данных, так чтобы было удобно заполнять данные
      // титулы довольно удобно заполняются сверху вниз, но когда мы переходим к городам удобство заканчивается
      // удобно при генерации городов сгенерировать и титул заодно (причем титул во многом будет "технический")
      // (то есть там будут храниться названия городов)
    }

    void generate_characters(generator::context* ctx, sol::table &table) {
      utils::time_log log("generating characters");

      // для каждого государства мы создаем по персонажу
      // затем внутри каждого государства мы должны распределить провинции на вассалов
      // как раздать провинции? во первых нужно примерно прикинуть сколько будет провинций
      // а это наверное будет зависеть от формы правления, а что с формой правления?
      // это набор нескольких механизмов центральных в средневековой стране:
      // 1. способ передачи власти
      // 2. ответственность органов власти
      // 3. разграничение полномочий
      // первый пункт более менее простой - это очевидно нужно сделать
      // ответственность? зависимость персонажа от чего то, перед советом (ну кстати изменяя набор механик мы можем подкорректировать ответственность и полномочия)
      // разграничение полномочий? тут по идее должна быть механика совета
      // тут есть две силы крупные монарх и совет (который состоит из вассалов)
      // они долны уравновешиваться армиями
      // племя - несколько провинции в децентрализованном государстве (но не всегда)
      // чаще всего мы определяем племя по тех уровню, следующий вопрос что то такое тех уровень?
      // тех уровень я полагаю очень похож на институты в еу4, то есть некоторое знание распространяется
      // по провинциям и обозначает доступ к некоторым зданиям и бонусам (иногда его можно открыть)
      // тех зависит от прошлых империй

      // раздачу титулов нужно совместить с историей похоже

      // можно раздать все титулы сначало одному персонажу, который в свою очередь раздаст
      // титулы своим подельникам, как раздать титулы более высокого уровня?
      // герцогские еще более менее понятно, титулы вообще тоже зависят от тех уровня
      // думаю что нужно сначало тех уровень продумать

      const uint32_t emp_offset = table["emp_offset"];
      const uint32_t king_offset = table["king_offset"];
      const uint32_t duchy_offset = table["duchy_offset"];
      const uint32_t baron_offset = table["baron_offset"];
      const uint32_t real_country_count = table["country_count"];
      PRINT_VAR("real_country_count", real_country_count)
      ASSERT(real_country_count != 0);

      const uint32_t titles_count = ctx->container->entities_count(debug::entities::title);
      const uint32_t country_count = ctx->container->entities_count(debug::entities::country);
      std::vector<std::vector<size_t>> owners(real_country_count, std::vector<size_t>(ctx->container->entities_count(debug::entities::title), 0));
      size_t country_index_counter = 0;
      for (size_t i = 0; i < country_count; ++i) { // некоторые страны пустые, почему то я не очистил их
        const uint32_t country_index = i;
        const auto &childs = ctx->container->get_childs(debug::entities::country, country_index);
        //ASSERT(!childs.empty());
        if (childs.empty()) continue;

        const uint32_t real_country_index = country_index_counter;
        for (const uint32_t province_index : childs) {
          const uint32_t baron_index = baron_offset + province_index;
          ++owners[real_country_index][baron_index];

          const uint32_t duchy_index = ctx->container->get_data<uint32_t>(debug::entities::title, baron_index, debug::properties::title::parent);
          if (duchy_index == UINT32_MAX) continue;
          ++owners[real_country_index][duchy_index];

          const uint32_t king_index = ctx->container->get_data<uint32_t>(debug::entities::title, duchy_index, debug::properties::title::parent);
          if (king_index == UINT32_MAX) continue;
          ++owners[real_country_index][king_index];

          const uint32_t emp_index = ctx->container->get_data<uint32_t>(debug::entities::title, king_index, debug::properties::title::parent);
          if (emp_index == UINT32_MAX) continue;
          ++owners[real_country_index][emp_index];
        }

        ++country_index_counter;
      }

//       ASSERT(counter == 0); // у меня остаются пустые страны после генерации

      // так и что это нам дает? у нас теперь есть количество провинций принадлежащих титулам разных уровней
      // так мы по идее можем определиться с титулом верхнего уровня
      // я так понимаю нужно сделать тип в контейнере

//       size_t computed_province_count = 0;
//       for (size_t i = 0; i < country_count; ++i) {
//         const uint32_t country_index = i;
//         const auto &childs = ctx->container->get_childs(debug::entities::country, country_index);
//         computed_province_count += childs.size();
//       }
//       PRINT(computed_province_count)
//       ASSERT(computed_province_count == ctx->container->entities_count(debug::entities::province));

      country_index_counter = 0;
      std::vector<std::vector<uint32_t>> titles(real_country_count);
      for (size_t i = 0; i < country_count; ++i) {
        const uint32_t country_index = i;
        const auto &childs = ctx->container->get_childs(debug::entities::country, country_index);
        if (childs.empty()) continue;
//         auto character_table = global::get<sol::state>()->create_table();
//         character_table.create("family");
//         character_table.create("stats");
//         character_table.create("hero_stats");
//         auto titles_table = character_table.create("titles");
//         auto character = ctx->container->create_character();

        const uint32_t real_country_index = country_index_counter;
        ++country_index_counter;
        const auto &title_owned = owners[real_country_index];
        for (uint32_t j = baron_offset; j < titles_count; ++j) {
          if (title_owned[j] == 0) continue;
          const uint32_t tmp = ctx->container->get_data<uint32_t>(debug::entities::title, j, debug::properties::title::owner);
          ASSERT(tmp == UINT32_MAX);
          //const uint32_t baron_index = j - baron_offset; // так у меня нет информации о том что это за титул
          titles[real_country_index].push_back(j);
          ctx->container->set_data<uint32_t>(debug::entities::title, j, debug::properties::title::owner, real_country_index);
        }

        // числа 20/70 на 5000 провок выглядят более менее прилично (выглядит прилично и для 1600 на более мелкой карте)
        // нужно ли корректировать эти цифры для меньшего количество провок
        // (2500 = 10/35)? если и нужно то видимо как то иначе, должен быть наверное какой то минимум
        // сколько провок в среднем в титулах? может так?
        const size_t king_start = 20;
        const size_t emp_start = 70;

        if (childs.size() < king_start) {
          size_t max_prov = 0;
          uint32_t index = UINT32_MAX;
          for (uint32_t j = duchy_offset; j < baron_offset; ++j) {
            const uint32_t duchy_index = j - duchy_offset;
            if (ctx->container->get_data<uint32_t>(debug::entities::title, duchy_offset + duchy_index, debug::properties::title::owner) != UINT32_MAX) continue;

            if (max_prov < title_owned[j]) {
              max_prov = title_owned[j];
              index = duchy_index;
            }
          }

          // один титул?
          ctx->container->set_data<uint32_t>(debug::entities::title, duchy_offset + index, debug::properties::title::owner, real_country_index);
          titles[real_country_index].push_back(duchy_offset + index);

          continue;
        }

        if (childs.size() >= king_start && childs.size() < emp_start) { // это королевства? в цк2 византийская империя (2 старт) примерно 65 провинций
          size_t max_prov = 0;
          uint32_t index = UINT32_MAX;
          for (uint32_t j = king_offset; j < duchy_offset; ++j) {
            const uint32_t king_index = j - king_offset;
            if (ctx->container->get_data<uint32_t>(debug::entities::title, king_offset + king_index, debug::properties::title::owner) != UINT32_MAX) continue;

            if (max_prov < title_owned[j]) {
              max_prov = title_owned[j];
              index = king_index;
            }
          }

          // один титул?
          ctx->container->set_data<uint32_t>(debug::entities::title, king_offset + index, debug::properties::title::owner, real_country_index);
          titles[real_country_index].push_back(king_offset + index);
        }

        if (childs.size() >= emp_start) {
          size_t max_prov = 0;
          uint32_t index = UINT32_MAX;
          for (uint32_t j = emp_offset; j < king_offset; ++j) {
            const uint32_t emp_index = j - emp_offset;
            if (ctx->container->get_data<uint32_t>(debug::entities::title, emp_offset + emp_index, debug::properties::title::owner) != UINT32_MAX) continue;

            if (max_prov < title_owned[j]) {
              max_prov = title_owned[j];
              index = emp_index;
            }
          }

          // мы нашли индекс империи с которой у нас больше всего совпадений
          // другое дело что исторически так все радужно с империями не было
          // они часто разваливались и поэтому сильно меняли форму
          // я должен создать титулы до генерации истории, а потом в истории
          // раскидывать титулы (точнее искать максимальный титул для государства)
          // но пока так

          ctx->container->set_data<uint32_t>(debug::entities::title, emp_offset + index, debug::properties::title::owner, real_country_index);
          titles[real_country_index].push_back(emp_offset + index);
        }

        // че с герцогствами? случайно их раскидать? выглядит идея еще ничего
        for (uint32_t j = duchy_offset; j < baron_offset; ++j) {
          if (title_owned[j] == 0) continue;
          if (ctx->container->get_data<uint32_t>(debug::entities::title, j, debug::properties::title::owner) != UINT32_MAX) continue;

//           const uint32_t owned = title_owned[j];                                                    // количество провок которые имеются у государства
//           const uint32_t title_size = ctx->container->get_childs(debug::entities::title, j).size(); // размер титула
//           const uint32_t country_size = childs.size();
          // по идее наличие герцогского титула мало зависит от размера и скорее зависит от поворота истории
          // поэтому нужен какой то случайный коэффициент
          // коэффициент наверное будет зависеть от размера страны (?)
          // в империях наиболее вероятно что будет сделан герцогский титул
          // менее вероятно в королевствах
          // в герцогствах должен один титул
          const float base = 0.3f;
          const float final = base + base * float(childs.size() >= emp_start); // 0.6f норм или нет?
          const bool prob = ctx->random->probability(final);
          if (!prob) continue;

          titles[real_country_index].push_back(j);
          ctx->container->set_data<uint32_t>(debug::entities::title, j, debug::properties::title::owner, real_country_index);
        }

        // хочется все бросить и сделать просто через создание конкретных объектов
        // но я так сделать не могу =( сериализация полных состояний луа невозможна
        // (в смысле я могу сохранить только конкретные данные (и кажется даже не ссылки))
        // я могу попробовать запустить гарбадж коллектор, но скорее всего без особого успеха
        // ко всему прочему мне бы выжать полный максимум производительности от проверки условий
        // в том числе используя мультитрединг, это сложно используя только луа
        // таким образом мне нужно правильно задать связи так чтобы при этом не сломался луа
        // и комп от обработки 20к титулов и столько же персонажей (а может и больше)
        // в луа 176 000 000 чисел (индексы) в таблице занимает 4гб,
        // мне надо чтобы ~5000 провинций, ~12000 городов, ~15000 персонажей, ~15000 титулов,
        // занимало желательно чтобы меньше 512 мб (ну мож гиг максимум)
        // придется тогда некоторые таблицы сохранять (нужны указатели на родителей)
        // мы можем положить индексы просто в ctx->container
        // скорее всего нужно сделать обработку и индексов в парсере: так станет более менее полегче
        // то есть вместо строки мы будем ожидать индекс или сделать и строку и индекс?

        // наверное просто сохраню в таблице все титулы

        ASSERT(childs.size() != 0);
//         if (childs.size() == 1) {
//           const uint32_t title_index = ctx->container->get_data<uint32_t>(debug::entities::province, childs[0], debug::properties::province::title_index);
//           auto title = ctx->container->get_title(title_index);
//           character->add_title(title);
//           // тут просто раздаем баронский титул
//           // возможно какой то герцогский титул тоже нужно раздать
//           continue;
//         }
//
//         std::unordered_map<core::titulus*, std::vector<uint32_t>> unique_titles;
//         for (size_t j = 0; j < childs.size(); ++j) {
//           const uint32_t province_index = childs[j];
//           const uint32_t title_index = ctx->container->get_data<uint32_t>(debug::entities::province, province_index, debug::properties::province::title_index);
//           auto title = ctx->container->get_title(title_index);
//           // как раскинуть провки на персонажей?
//           // нужно сначало догадаться какие максимальные титулы у нас есть
//           unique_titles[title].push_back(province_index);
//           auto p1 = title->parent;
//           while (p1 != nullptr) {
//             unique_titles[p1].push_back(province_index);
//             p1 = p1->parent;
//           }
//         }
//
//         const std::function<void(const core::titulus*, size_t &)> func = [&func] (const core::titulus* title, size_t &counter) {
//           if (title->type == core::titulus::type::baron) {
//             ++counter;
//             return;
//           }
//
//           if (title->type != core::titulus::type::duke) {
//             for (size_t i = 0; i < title->count; ++i) {
//               auto child = title->get_child(i);
//               func(child, counter);
//             }
//
//             return;
//           }
//
//           ASSERT(title->type == core::titulus::type::duke);
//           counter += title->count;
//         };
//
//         for (const auto &pair : unique_titles) {
//           if (pair.first->type == core::titulus::type::imperial && pair.first->owner == nullptr) {
//             const uint32_t owned_provinces = pair.second.size();
//             size_t counter = 0;
//             func(pair.first, counter);
//
//             const float owned_percent = float(owned_provinces) / float(counter);
//             ASSERT(owned_percent >= 0.0f && owned_percent <= 1.0f);
//             if (owned_percent > 0.5f) {
//               character->add_title(pair.first);
//             }
//           }
//         }

//         for (const uint32_t province_index : childs) {
//           titles_table.add("baron" + std::to_string(province_index) + "_title");
//
//           const uint32_t title_index = ctx->container->get_data<uint32_t>(debug::entities::province, province_index, debug::properties::province::title_index);
//           auto t = ctx->container->get_title(title_index);
//           character->add_title(t);
//         }
      }

      // как вообще в принципе раздать сгенерированные титулы сгенерированным персонажам?
      // возможно даже сразу сгенерировать персонажей к титулам лучше чем генерировать сейчас отдельно

      // теперь у нас есть как то раскиданные титулы государству, как раскидывать по персонажам?
      // раскидывать нужно пока не кончатся провинции, отдавать в одни руки числом до герцогского (то есть от 1 до 6)
      // (но 5-6 очень редко)
      for (size_t i = 0; i < real_country_count; ++i) {
        const uint32_t country_index = i;
        // сначало нужно посчитать что имеем
        std::unordered_set<uint32_t> baron_titles;
        std::vector<uint32_t> duchy_titles;
        std::vector<uint32_t> king_titles;
        std::vector<uint32_t> emp_titles;
        const auto &owned_titles = titles[country_index];
        for (const uint32_t title_index : owned_titles) {
          if (title_index >= baron_offset) baron_titles.insert(title_index);
          else if (title_index >= duchy_offset) duchy_titles.push_back(title_index);
          else if (title_index >= king_offset) king_titles.push_back(title_index);
          else if (title_index >= emp_offset) emp_titles.push_back(title_index);
        }

        ASSERT(king_titles.size() + emp_titles.size() < 2);

        // первым делаем господина, у господина должны быть все самые высокие титулы и покрайней мере одно герцогство
        size_t liege_index = SIZE_MAX;
        {
          const uint32_t baron_titles_count = ctx->random->index(5) + 2; // от 2 до 6 (более менее нормальное распределение)
          uint32_t counter = 0;
          uint32_t choosen_baron_index = UINT32_MAX;
          std::vector<uint32_t> final_titles;
          if (!duchy_titles.empty()) {
            final_titles.insert(final_titles.end(), king_titles.begin(), king_titles.end());
            final_titles.insert(final_titles.end(), emp_titles.begin(), emp_titles.end());
            king_titles.clear();
            emp_titles.clear();

            // герцогство выбираем случайно (герцогств может и не быть)
            const uint32_t rand_index = ctx->random->index(duchy_titles.size());
            const uint32_t duchy_index = duchy_titles[rand_index];
            final_titles.push_back(duchy_index);
            duchy_titles[rand_index] = duchy_titles.back();
            duchy_titles.pop_back();

            ASSERT(duchy_index >= duchy_offset && duchy_index < baron_offset);

            // собираем баронские титулы
            const auto &duchy_childs = ctx->container->get_childs(debug::entities::title, duchy_index);
            // нам бы раздать случайные (?) соседние провки,
  //           for (size_t j = 0; j < duchy_childs.size(); ++j) {
  //             const uint32_t baron_index = duchy_childs[j];
  //
  //           }

            while (choosen_baron_index == UINT32_MAX) {
              const uint32_t rand_index = ctx->random->index(duchy_childs.size());
              const uint32_t baron_title_index = duchy_childs[rand_index];
              if (ctx->container->get_data<uint32_t>(debug::entities::title, baron_title_index, debug::properties::title::owner) != country_index) continue;
              auto itr = baron_titles.find(baron_title_index);
              if (itr == baron_titles.end()) continue;

              baron_titles.erase(itr);
              choosen_baron_index = baron_title_index;
            }
          } else {
            while (choosen_baron_index == UINT32_MAX) {
              auto itr = baron_titles.begin();
              const uint32_t baron_title_index = *itr;
              if (ctx->container->get_data<uint32_t>(debug::entities::title, baron_title_index, debug::properties::title::owner) != country_index) continue;
              //auto itr = baron_titles.find(baron_title_index);
              //if (itr == baron_titles.end()) continue;

              baron_titles.erase(itr);
              choosen_baron_index = baron_title_index;
            }
          }

          ASSERT(choosen_baron_index != UINT32_MAX);
          final_titles.push_back(choosen_baron_index);
          ++counter;

          uint32_t last_title = choosen_baron_index;
          while (counter < baron_titles_count) {
            const uint32_t province_index = last_title - baron_offset;
            const auto &neighbors = ctx->container->get_province_neighbours(province_index);
            size_t index = 0;
            while (counter < baron_titles_count && index < neighbors.size()) {
              const auto new_index = province_neighbour(neighbors[index]);
              ++index;
              const uint32_t baron_index = new_index.index() + baron_offset;
              if (ctx->container->get_data<uint32_t>(debug::entities::title, baron_index, debug::properties::title::owner) != country_index) continue;
              auto itr = baron_titles.find(baron_index);
              if (itr == baron_titles.end()) continue;

              final_titles.push_back(baron_index);
              baron_titles.erase(itr);
              ++counter;
            }

            // если не добрали то че делаем
            // у нас может быть ситуация когда у государства очень мало провинций
            // тогда нужно выходить, че делать то
            //ASSERT(last_title != final_titles.back());
            if (last_title == final_titles.back()) break;
            last_title = final_titles.back();
          }

          auto character_table = global::get<sol::state>()->create_table();
          character_table.create("family");
          auto char_stats = character_table.create("stats");
          char_stats[core::character_stats::money] = 400.0f;
          character_table.create("hero_stats");
          character_table["male"] = true;
          character_table["dead"] = false;
          auto titles_table = character_table.create("titles");
          for (const uint32_t title_index : final_titles) {
            if (title_index >= baron_offset) {
              const std::string &str = "baron" + std::to_string(title_index - baron_offset) + "_title";
              titles_table.add(std::move(str));
              const std::string &city_id = "city" + std::to_string(title_index - baron_offset) + "_title";
              titles_table.add(std::move(city_id));
            } else if (title_index >= duchy_offset) {
              const std::string &str = "duke" + std::to_string(title_index - duchy_offset) + "_title";
              titles_table.add(std::move(str));
            } else if (title_index >= king_offset) {
              const std::string &str = "king" + std::to_string(title_index - king_offset) + "_title";
              titles_table.add(std::move(str));
            } else if (title_index >= emp_offset) {
              const std::string &str = "imperial" + std::to_string(title_index - emp_offset) + "_title";
              titles_table.add(std::move(str));
            }
          }

          liege_index = utils::add_character(character_table);
        }

        // тут может быть что соседние вассалы разобрали все баронские титулы у текущего герцогства
        // что делать в этом случае? 2 варианта: либо мы считаем что это герцогство не создано
        // либо мы гарантируем каждому владельцу титула что у него будет баронство

        // теперь делаем вассалов, у них либо герцогство либо несколько просто владений
        // поэтому сначало раздаем герцогства, так то у этих челиков могут быть свои вассалы
        // возможно нужно взять больше баронских титулов
        while (!duchy_titles.empty()) {
          const uint32_t rand_index = ctx->random->index(duchy_titles.size());
          const uint32_t duchy_index = duchy_titles[rand_index];
          duchy_titles[rand_index] = duchy_titles.back();
          duchy_titles.pop_back();

          std::vector<uint32_t> final_titles;
          final_titles.push_back(duchy_index);

          // пока что наверное выберем первый вариант
          size_t baron_counter = 0;
          const auto &duchy_childs = ctx->container->get_childs(debug::entities::title, duchy_index);
          for (size_t i = 0; i < duchy_childs.size(); ++i) {
            const uint32_t index = duchy_childs[i];
            if (ctx->container->get_data<uint32_t>(debug::entities::title, index, debug::properties::title::owner) != country_index) continue;
            auto itr = baron_titles.find(index);
            if (itr == baron_titles.end()) continue;

            ++baron_counter;
          }

          if (baron_counter == 0) continue;

          // собираем баронские титулы (нужно взять больше для того чтобы сделать вассалов вассала)
          const uint32_t baron_titles_count = ctx->random->index(5) + 2; // от 2 до 6 (более менее нормальное распределение)
          uint32_t counter = 0;
          // нам бы раздать случайные (?) соседние провки,
//           for (size_t j = 0; j < duchy_childs.size(); ++j) {
//             const uint32_t baron_index = duchy_childs[j];
//
//           }

          uint32_t choosen_baron_index = UINT32_MAX;
          while (choosen_baron_index == UINT32_MAX) {
            const uint32_t rand_index = ctx->random->index(duchy_childs.size());
            const uint32_t baron_title_index = duchy_childs[rand_index];
            if (ctx->container->get_data<uint32_t>(debug::entities::title, baron_title_index, debug::properties::title::owner) != country_index) continue;
            auto itr = baron_titles.find(baron_title_index);
            if (itr == baron_titles.end()) continue;

            baron_titles.erase(itr);
            choosen_baron_index = baron_title_index;
          }

          ASSERT(choosen_baron_index != UINT32_MAX);
          final_titles.push_back(choosen_baron_index);
          ++counter;

          uint32_t last_title = choosen_baron_index;
//           while (counter < baron_titles_count) {
            const uint32_t province_index = last_title - baron_offset;
            const auto &neighbors = ctx->container->get_province_neighbours(province_index);
            size_t index = 0;
            while (counter < baron_titles_count && index < neighbors.size()) {
              const auto new_index = province_neighbour(neighbors[index]);
              ++index;
              const uint32_t baron_index = new_index.index() + baron_offset;
              if (ctx->container->get_data<uint32_t>(debug::entities::title, baron_index, debug::properties::title::owner) != country_index) continue;
              auto itr = baron_titles.find(baron_index);
              if (itr == baron_titles.end()) continue;

              final_titles.push_back(baron_index);
              baron_titles.erase(itr);
              ++counter;
            }

            // наверное здесь ограничимся тем что есть
//             ASSERT(last_title != final_titles.back());
//             last_title = final_titles.back();
//           }

          auto character_table = global::get<sol::state>()->create_table();
          character_table.create("family");
          auto char_stats = character_table.create("stats");
          char_stats[core::character_stats::money] = 400.0f;
          character_table.create("hero_stats");
          character_table["male"] = true;
          character_table["dead"] = false;
          character_table["liege"] = liege_index;
          auto titles_table = character_table.create("titles");
          for (const uint32_t title_index : final_titles) {
            if (title_index >= baron_offset) {
              const std::string &str = "baron" + std::to_string(title_index - baron_offset) + "_title";
              titles_table.add(std::move(str));
              const std::string &city_id = "city" + std::to_string(title_index - baron_offset) + "_title";
              titles_table.add(std::move(city_id));
            } else if (title_index >= duchy_offset) {
              const std::string &str = "duke" + std::to_string(title_index - duchy_offset) + "_title";
              titles_table.add(std::move(str));
            } else assert(false);
          }

          utils::add_character(character_table);
        }

        // могут остаться баронские титулы еще
        while (!baron_titles.empty()) {
          auto itr = baron_titles.begin();
          const uint32_t choosen_baron_index = *itr;
          baron_titles.erase(itr);

          const uint32_t baron_titles_count = ctx->random->index(6) + 1;
          uint32_t counter = 0;
          std::vector<uint32_t> final_titles;
          final_titles.push_back(choosen_baron_index);
          ++counter;

          uint32_t last_title = choosen_baron_index;
//           while (counter < baron_titles_count) {
            const uint32_t province_index = last_title - baron_offset;
            const auto &neighbors = ctx->container->get_province_neighbours(province_index);
            size_t index = 0;
            while (counter < baron_titles_count && index < neighbors.size()) {
              const auto new_index = province_neighbour(neighbors[index]);
              ++index;
              const uint32_t baron_index = new_index.index() + baron_offset;
              if (ctx->container->get_data<uint32_t>(debug::entities::title, baron_index, debug::properties::title::owner) != country_index) continue;
              auto itr = baron_titles.find(baron_index);
              if (itr == baron_titles.end()) continue;

              final_titles.push_back(baron_index);
              baron_titles.erase(itr);
              ++counter;
            }

            // наверное здесь ограничимся тем что есть
//             ASSERT(last_title != final_titles.back());
//             last_title = final_titles.back();
//           }

          auto character_table = global::get<sol::state>()->create_table();
          character_table.create("family");
          auto char_stats = character_table.create("stats");
          char_stats[core::character_stats::money] = 400.0f;
          character_table.create("hero_stats");
          character_table["male"] = true;
          character_table["dead"] = false;
          character_table["liege"] = liege_index;
          auto titles_table = character_table.create("titles");
          for (const uint32_t title_index : final_titles) {
            if (title_index >= baron_offset) {
              const std::string &str = "baron" + std::to_string(title_index - baron_offset) + "_title";
              titles_table.add(std::move(str));
              const std::string &city_id = "city" + std::to_string(title_index - baron_offset) + "_title";
              titles_table.add(std::move(city_id));
            } else assert(false);
          }

          utils::add_character(character_table);
        }
      }

      // как то раскидали титулы по персонажам
      // по идее мы закончили с генерацией (нужно еще наверное задать какие нибудь константы (например шанс смерти))
      // теперь нужно сделать парсинг
      // мы обходим все таблицы, валидируем их, парсим,
      // после парсинга начинаем собирать все связи между данными в кучу
      // удаляем старый луа стейт, создаем новый (хотя до сих пор не знаю нужен ли он)
      // делаем всю необходимую работу для рендера (границы, стены),
      // переходим непосредственно к игре
      // требуется обойти всех персонажей и посчитать ход, причем часть хода можно сделать в мультитрединге
      // (например посчитать инком, пересчитать статы, строительство и проч)
      // где то здесь же идет ход игрока, отрисовка интерфейса
      // вообще дизайн интерфейса будет той еще задачкой для меня

      // как быть здесь? нужно получить id родительского титула
//       const uint32_t province_count = ctx->container->entities_count(debug::entities::province);
//       for (uint32_t i = 0; i < province_count; ++i) {
//         const uint32_t province_index = i;
//         const std::string &id = "baron" + std::to_string(province_index) + "_title";
//
//
//         auto t = ctx->container->get_title(province_index);
//         auto owner = t->owner;
//         auto next_title = t->parent;
//         if (next_title->owner != nullptr) continue;
//
//         uint32_t counter = 0;
//         for (uint32_t j = 0; j < next_title->count; ++j) {
//           auto child = next_title->get_child(j);
//           if (child->owner == owner) ++counter;
//         }
//
//         if (counter > std::ceil(float(next_title->count) / 2.0f)) {
//           owner->add_title(next_title);
//         }
//       }
//
//       for (size_t i = 0; i < country_count; ++i) {
//         const uint32_t country_index = i;
//         const auto &childs = ctx->container->get_childs(debug::entities::country, country_index);
//         // текущий генератор выдает в среднем 26 провинций на королевство
//         // и 144 провинций на империю, около того что я предполагал (100 провок на империю)
//         if (childs.size() < 20) continue; // минимум 20 королевство?
//
//         const uint32_t titulus_index = ctx->container->get_data<uint32_t>(debug::entities::province, childs[0], debug::properties::province::title_index);
//         auto t = ctx->container->get_title(titulus_index);
//         auto owner = t->owner;
//
//         {
//           std::unordered_map<core::titulus*, uint32_t> unique_kingdoms;
//           for (size_t j = 0; j < childs.size(); ++j) {
//             const uint32_t titulus_index = ctx->container->get_data<uint32_t>(debug::entities::province, childs[j], debug::properties::province::title_index);
//             auto t = ctx->container->get_title(titulus_index);
//             auto duchy = t->parent;
//             auto kingdom = duchy->parent;
//             ASSERT(kingdom->type == core::titulus::type::king);
//             if (kingdom->owner != nullptr) continue;
//             auto itr = unique_kingdoms.find(kingdom);
//             if (itr == unique_kingdoms.end()) itr = unique_kingdoms.insert(std::make_pair(kingdom, 0)).first;
//             ++itr->second;
//           }
//
//           uint32_t max_kingdom_count = 0;
//           core::titulus* kingdom = nullptr;
//           for (const auto &pair : unique_kingdoms) {
//             if (max_kingdom_count < pair.second) {
//               max_kingdom_count = pair.second;
//               kingdom = pair.first;
//             }
//           }
//
//           owner->add_title(kingdom);
//         }
//
//         if (childs.size() < 100) continue; // в этом случае поди нужно несколько королевств найти
//
//         {
//           std::unordered_map<core::titulus*, uint32_t> unique_empires;
//           for (size_t j = 0; j < childs.size(); ++j) {
//             const uint32_t titulus_index = ctx->container->get_data<uint32_t>(debug::entities::province, childs[j], debug::properties::province::title_index);
//             auto t = ctx->container->get_title(titulus_index);
//             auto duchy = t->parent;
//             auto kingdom = duchy->parent;
//             auto empire = kingdom->parent;
//             ASSERT(empire->type == core::titulus::type::imperial);
//             if (empire->owner != nullptr) continue;
//             auto itr = unique_empires.find(empire);
//             if (itr == unique_empires.end()) itr = unique_empires.insert(std::make_pair(empire, 0)).first;
//             ++itr->second;
//           }
//
//
//           core::titulus* empire = nullptr;
//           uint32_t max_kingdom_count = 0;
//           for (const auto &pair : unique_empires) {
//             if (max_kingdom_count < pair.second) {
//               max_kingdom_count = pair.second;
//               empire = pair.first;
//             }
//           }
//
//           owner->add_title(empire);
//         }
//       }

      // как то так раздали все титулы одному персонажу пока что
      // их нужно будет потом распределить по подельникам
      // нужно сделать еще города
    }

    void generate_tech_level(generator::context* ctx, sol::table &table) {
      utils::time_log log("generating tech level");

      // что такое тех уровень?
      // тех в средвековье определялся в основном прошлым
      // то есть примерно в области западной римской империи тех чуть повыше
      // восточная римская империя - высокий тех
      // рядом с китаем высокий тех
      // что такое тех уровень с технической стороны?
      // я думаю что можно сделать что то похожее на институты в еу4
      // хороший вопрос как их распределить?
      // существует несколько техов например обработка железа дает возможность нанимать тяжелые отряды
      // хотя по теху пока еще не понятно как лучше сделать
      // в цк2 тех состоял из формы правления и культурных техов
      // форма правления не давала ниче строить племенам, техи позволяли проводить некоторые законы
      // таким образом техи должны зависить от формы правления, а форма правления должна зависеть от техов
      // техи получить должно быть возможно без формы правления, причем техи можно получить не по порядку
      // какие техи? должны быть техи "ключевые", они позволяют серьезно продвинуться по развитию
      // например тех организованность позволяют получить какую то форму феодализма

      // нужно решить несколько технических проблем
      // как техи будут передавать свои бонусы? особые бонусы делать сложно
      // особые бонусы это что?
    }

    void generate_cities(generator::context* ctx, sol::table &table) {
      utils::time_log log("generating tech level");

      // здания и тип города
      {
        auto building = global::get<sol::state>()->create_table();
        building["id"] = "test_building1";
        building["time"] = 4;
        utils::add_building(building);
      }

      {
        auto building = global::get<sol::state>()->create_table();
        building["id"] = "test_building2";
        building["time"] = 4;
        utils::add_building(building);
      }
      
      {
        auto image_table = global::get<sol::state>()->create_table();
        image_table["id"] = "castle_top";
        image_table["path"] = "apates_quest/textures/structures/castle_top.png";
        image_table["type"] = 0; // тут индекс
        image_table["sampler"] = 1;
        utils::add_image(image_table);
      }
      
      {
        auto image_table = global::get<sol::state>()->create_table();
        image_table["id"] = "castle_face";
        image_table["path"] = "apates_quest/textures/structures/castle_face.png";
        image_table["type"] = 0; // тут индекс
        image_table["sampler"] = 1;
        utils::add_image(image_table);
      }

      {
        auto city_type = global::get<sol::state>()->create_table();
        city_type["id"] = "city_type1";
        auto buildings = city_type.create<std::string>("buildings");
        buildings[0] = "test_building1";
        buildings[1] = "test_building2";
        auto stats = city_type.create<std::string>("stats");
        stats[magic_enum::enum_name<core::city_stats::values>(core::city_stats::tax_income)] = 1.0f;
        city_type["image_top"] = "castle_top";
        city_type["image_face"] = "castle_face";
        city_type["scale"] = 0.5f;
        utils::add_city_type(city_type);
      }

      {
        auto city_type = global::get<sol::state>()->create_table();
        city_type["id"] = "city_type2";
        auto buildings = city_type.create<std::string>("buildings");
        buildings[0] = "test_building1";
        buildings[1] = "test_building2";
        auto stats = city_type.create<std::string>("stats");
        stats[magic_enum::enum_name<core::city_stats::values>(core::city_stats::tax_income)] = 1.0f;
        city_type["image_top"] = "castle_top";
        city_type["image_face"] = "castle_face";
        city_type["scale"] = 0.5f;
        utils::add_city_type(city_type);
      }

      const uint32_t provinces_creation_count = ctx->container->entities_count(debug::entities::province);
      for (uint32_t i = 0; i < provinces_creation_count; ++i) {
        const auto &province_tiles = ctx->container->get_childs(debug::entities::province, i);
        const uint32_t rand_index = ctx->random->index(province_tiles.size());
        const uint32_t rand_tile = province_tiles[rand_index];
        // id получаются совсем не информативными, они будут более информативными если генерировать имена
        // в общем это проблема неединичной генерации, хочется использовать индексы, но они не информативные
        const std::string city_id = "city" + std::to_string(i) + "_title";
        auto city = global::get<sol::state>()->create_table(); // название города видимо будет храниться в титуле
        city["province"] = i;
        city["city_type"] = "city_type1";
        city["tile_index"] = rand_tile;
        city["title"] = city_id; // тут проще использовать индекс
        utils::add_city(city);

        auto city_title = global::get<sol::state>()->create_table();
        city_title["id"] = city_id;
        city_title["type"] = core::titulus::type::city;
//         city_title["parent"] = baron_id; // родителя найдем тогда из провинции
//         city_title["city"] = baron_index;
        gen_title_color(city_title, i);
        utils::add_title(city_title);
      }

      global::get<utils::calendar>()->set_start_date(false, 865, 3, 25);
      global::get<utils::calendar>()->set_current_date(false, 865, 3, 25);
      global::get<utils::calendar>()->add_month_data({SIZE_MAX, 31}); // январь
      global::get<utils::calendar>()->add_month_data({SIZE_MAX, 29});
      global::get<utils::calendar>()->add_month_data({SIZE_MAX, 31});
      global::get<utils::calendar>()->add_month_data({SIZE_MAX, 30});
      global::get<utils::calendar>()->add_month_data({SIZE_MAX, 31}); // май
      global::get<utils::calendar>()->add_month_data({SIZE_MAX, 30});
      global::get<utils::calendar>()->add_month_data({SIZE_MAX, 31});
      global::get<utils::calendar>()->add_month_data({SIZE_MAX, 31});
      global::get<utils::calendar>()->add_month_data({SIZE_MAX, 30}); // сентябрь
      global::get<utils::calendar>()->add_month_data({SIZE_MAX, 31});
      global::get<utils::calendar>()->add_month_data({SIZE_MAX, 30});
      global::get<utils::calendar>()->add_month_data({SIZE_MAX, 31});
    }


  }
}

