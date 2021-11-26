#include "generator_begin.h"

#include "utils/logging.h"
#include "utils/random_engine.h"
#include "utils/thread_pool.h"
#include "utils/works_utils.h"
#include "utils/constexpr_funcs.h"
#include "bin/generator_context2.h"
#include "bin/map.h"
#include "bin/figures.h"

namespace devils_engine {
  namespace core {
    void map_triangle_add2(const devils_engine::map::container* map, const uint32_t &triangle_index, std::vector<std::atomic<uint32_t>> &unique_tiles, std::vector<uint32_t> &tiles_array) {
      const auto &tri = map->triangles[triangle_index];

      if (tri.current_level == core::map::detail_level) {
        for (uint32_t i = 0; i < 4; ++i) {
          const uint32_t tile_index = tri.next_level[i];
          
          const size_t check = unique_tiles[tile_index].fetch_add(1);
          if (check != 0) continue;
          
          // уникальность проверяем с помощью атомиков, быстрее ли это?
          tiles_array.push_back(tile_index);
        }

        return;
      }

      for (uint32_t i = 0; i < 4; ++i) {
        const uint32_t next_triangle_index = tri.next_level[i];
        ASSERT(next_triangle_index != UINT32_MAX);
        map_triangle_add2(map, next_triangle_index, unique_tiles, tiles_array);
      }
    }
    
    void make_tiles(const glm::mat4 &mat1, core::map* map, dt::thread_pool* pool) {
      const glm::mat3 mat(mat1);
      devils_engine::map::container generated_core(core::map::world_radius, core::map::detail_level, mat); // возможно нужно как то это ускорить

      ASSERT(generated_core.points.size() == map->points_count());
      ASSERT(generated_core.tiles.size() == map->tiles_count());
      ASSERT(generated_core.triangles.size() == map->triangles_count());
      
      {
        utils::time_log log("fixing tiles");
        utils::submit_works_async(pool, generated_core.tiles.size(), [&generated_core] (const size_t &start, const size_t &count) {
          for (size_t i = start; i < start+count; ++i) {
            generated_core.fix_tile(i);
          }
        });
        utils::async_wait(pool);
      }
      
      {
        utils::time_log log("fixing tiles 2");
        utils::submit_works_async(pool, generated_core.tiles.size(), [&generated_core] (const size_t &start, const size_t &count) {
          for (size_t i = start; i < start+count; ++i) {
            generated_core.fix_tile2(i);
          }
        });
        utils::async_wait(pool);
      }
      
      {
        utils::time_log log("applying matrix");
        utils::submit_works_async(pool, generated_core.points.size(), [&generated_core, mat1] (const size_t &start, const size_t &count) {
          for (size_t i = start; i < start+count; ++i) {
            generated_core.apply_matrix(i, mat1);
          }
        });
        utils::async_wait(pool);
      }
      
      {
        std::vector<render::light_map_tile_t> tiles(map->tiles_count(), render::light_map_tile_t{});
        std::vector<glm::vec4> points(core::map::points_count_d(core::map::detail_level), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        {
          ASSERT(tiles.size() == core::map::hex_count_d(core::map::detail_level));
          ASSERT(generated_core.tiles.size() == core::map::hex_count_d(core::map::detail_level));
          utils::time_log log("seting tile data");
          utils::submit_works_async(pool, generated_core.tiles.size(), [&generated_core, &tiles] (const size_t &start, const size_t &count) {
            for (size_t i = start; i < start+count; ++i) {
  //             map->set_tile_data(&generated_core.tiles[i], i);
              auto tile = &generated_core.tiles[i];
              const render::map_tile_t map_tile{
                {tile->neighbours[0].points[0], tile->neighbours[1].points[0], tile->neighbours[2].points[0], tile->neighbours[3].points[0], tile->neighbours[4].points[0], tile->neighbours[5].points[0]},
                {tile->neighbours[0].index, tile->neighbours[1].index, tile->neighbours[2].index, tile->neighbours[3].index, tile->neighbours[4].index, tile->neighbours[5].index},
                tile->index,
                0.0f,
                GPU_UINT_MAX,
                0,
                {GPU_UINT_MAX},
                {GPU_UINT_MAX},
                GPU_UINT_MAX,
                GPU_UINT_MAX,
              };
              tiles[i] = render::pack_data(map_tile);
            }
          });
          utils::async_wait(pool);
        }

        {
          ASSERT(generated_core.points.size() == core::map::points_count_d(core::map::detail_level));
          ASSERT(points.size() == core::map::points_count_d(core::map::detail_level));
          utils::time_log log("seting point data");
          utils::submit_works_async(pool, generated_core.points.size(), [&generated_core, &points] (const size_t &start, const size_t &count) {
            for (size_t i = start; i < start+count; ++i) {
  //             map->set_point_data(generated_core.points[i], i);
              points[i] = glm::vec4(generated_core.points[i], 1.0f);
            }
          });
          utils::async_wait(pool);
        }
        
        const size_t tri_count = core::map::tri_count_d(core::map::accel_struct_detail_level);
        const size_t tiles_count = core::map::hex_count_d(core::map::detail_level);
        ASSERT(tri_count == map->accel_triangles_count());
  //     const size_t hex_count = map::hex_count_d(detail_level);
//         std::mutex mutex;
        std::vector<std::atomic<uint32_t>> unique_tiles(tiles_count);
        for (auto &val : unique_tiles) { val = 0; }
        std::atomic<uint32_t> tiles_counter(0);
        std::vector<render::packed_fast_triangle_t> fast_triangles(tri_count, render::packed_fast_triangle_t{});
        const size_t indices_size = align_to(tiles_count*sizeof(uint32_t), sizeof(glm::uvec4))/sizeof(uint32_t);
        std::vector<uint32_t> tile_indices(indices_size, UINT32_MAX);
        
        {
          utils::time_log log("making acceleration struct");
          utils::submit_works_async(pool, tri_count, [&unique_tiles, &generated_core, &tiles_counter, &fast_triangles, &tile_indices] (const size_t &start, const size_t &count) {
            std::vector<uint32_t> tiles_array;
            tiles_array.reserve(200);
            size_t tri_offset = 0;
            for (size_t i = 0; i < core::map::accel_struct_detail_level; ++i) {
              tri_offset += core::map::tri_count_d(i);
            }

            for (size_t i = start; i < start+count; ++i) {
              const size_t tri_index = i + tri_offset;
              const auto &tri = generated_core.triangles[tri_index];
              ASSERT(tri.current_level == core::map::accel_struct_detail_level);

              map_triangle_add2(&generated_core, tri_index, unique_tiles, tiles_array);

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
              //map->set_tile_indices(i, tri.points, tiles_array, offset, tiles_array.size(), counter != 0);
              
              fast_triangles[i].points[0] = tri.points[0];
              fast_triangles[i].points[1] = tri.points[1];
              fast_triangles[i].points[2] = tri.points[2];
              fast_triangles[i].data[0] = offset;
              fast_triangles[i].data[1] = tiles_array.size();
              fast_triangles[i].data[2] = uint32_t(counter != 0);
              
              memcpy(&tile_indices.data()[offset], tiles_array.data(), tiles_array.size()*sizeof(tiles_array[0]));

              tiles_array.clear();
            }
          });
          utils::async_wait(pool); // похоже что работает
        }
        
        ASSERT(pool->working_count() == 1 && pool->tasks_count() == 0);
        
        ASSERT(generated_core.triangles.size() == map->triangles.size());
        static_assert(sizeof(core::map::triangle) == sizeof(map::triangle));
        
        utils::time_log log("copying data to map container");
        std::vector<core::map::triangle> tmp_triangles(generated_core.triangles.size());
        memcpy(tmp_triangles.data(), generated_core.triangles.data(), tmp_triangles.size()*sizeof(tmp_triangles[0]));
        map->copy_main_map_data(tiles, points, fast_triangles, tile_indices, tmp_triangles);
      }
    }
    
    void begin(devils_engine::map::generator::context* ctx, sol::table &table) {
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

      auto map = ctx->map;
      map->world_matrix = mat1;
      
      make_tiles(mat1, map, ctx->pool);

      ctx->map->set_status(core::map::status::valid);
    }
  }
}
