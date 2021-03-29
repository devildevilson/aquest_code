#include "map_generators.h"
#include "figures.h"
#include "map.h"
#include "utils/works_utils.h"
#include "utils/concurrent_vector.h"
#include "utils/random_engine.h"
#include "utils/logging.h"
#include "FastNoise.h"
#include <unordered_set>
#include <set>
#include <unordered_map>
#include <map>
#include <queue>
#include <cstring>
#include <algorithm>

const float radius_const = 500.0f;

namespace devils_engine {
  namespace map {
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
      std::atomic<uint32_t>* tile_plate_indices,
      utils::concurrent_vector<uint32_t>* plate_tile_indices,
      //std::vector<std::pair<uint32_t, uint32_t>> &active_tile_indices
      //active_tile_container_t &active_tile_indices
      utils::concurrent_vector<std::pair<uint32_t, uint32_t>> &active_tile_indices
    ) {
      uint32_t data = plate_count;
      if (!tile_plate_indices[tile_index].compare_exchange_strong(data, plate_index)) return false;

      const auto &tile = render::unpack_data(map->get_tile(tile_index));
      for (uint32_t i = 0; i < 6; ++i) {
        const uint32_t neighbor_index = tile.neighbors[i];
        if (neighbor_index == UINT32_MAX) continue;
        if (tile_plate_indices[neighbor_index] != plate_count) {
          uint32_t data = plate_index;
          tile_plate_indices[tile_index].compare_exchange_strong(data, plate_count);
          return false;
        }
      }

      //std::cout << "current tile_index " << tile_index << "\n";
      //ASSERT(false);

      plate_tile_indices[plate_index].push_back(tile_index);

      for (uint32_t i = 0; i < 6; ++i) {
        const uint32_t neighbor_index = tile.neighbors[i];
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
    
    beginner::beginner(const create_info &info) : pool(info.pool) {}
    void beginner::process(generator_context* context) {
      utils::time_log log("prepare step");
      // создаем базовый каркас карты здесь
      state = 0;
      
      // нужно создать случайную матрицу поворота
      glm::mat3 mat(1.0f);
      map::container generated_core(radius_const, core::map::detail_level, mat); // возможно нужно как то это ускорить
      
      auto map = context->map;
      ASSERT(generated_core.points.size() == map->points_count());
      ASSERT(generated_core.tiles.size() == map->tiles_count());
      ASSERT(generated_core.triangles.size() == map->triangles_count());
      
      utils::submit_works(pool, generated_core.tiles.size(), [&generated_core] (const size_t &start, const size_t &count, std::atomic<size_t> &state) {
        for (size_t i = start; i < start+count; ++i) {
          generated_core.fix_tile(i);
          ++state;
        }
      }, std::ref(state));
      
      utils::submit_works(pool, generated_core.tiles.size(), [&generated_core, map] (const size_t &start, const size_t &count, std::atomic<size_t> &state) {
        for (size_t i = start; i < start+count; ++i) {
          map->set_tile_data(&generated_core.tiles[i], i);
          ++state;
        }
      }, std::ref(state));
      
      utils::submit_works(pool, generated_core.points.size(), [&generated_core, map] (const size_t &start, const size_t &count, std::atomic<size_t> &state) {
        for (size_t i = start; i < start+count; ++i) {
          map->set_point_data(generated_core.points[i], i);
          ++state;
        }
      }, std::ref(state));
      
      
      const size_t tri_count = core::map::tri_count_d(core::map::accel_struct_detail_level);
      ASSERT(tri_count == map->accel_triangles_count());
//     const size_t hex_count = map::hex_count_d(detail_level);
      std::mutex mutex;
      std::unordered_set<uint32_t> unique_tiles;
      std::atomic<uint32_t> tiles_counter(0);
      
      utils::submit_works(pool, tri_count, [&mutex, &unique_tiles, &generated_core, &tiles_counter, map] (const size_t &start, const size_t &count, std::atomic<size_t> &state) {
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
          
          ++state;
        }
      }, std::ref(state));
      
      ASSERT(generated_core.triangles.size() == map->triangles.size());
      ASSERT(sizeof(core::map::triangle) == sizeof(map::triangle));
      memcpy(map->triangles.data(), generated_core.triangles.data(), map->triangles.size()*sizeof(core::map::triangle));
      
      map->flush_data();
    }
    
    size_t beginner::progress() const {
      return state;
    }
    
    size_t beginner::complete_state(const generator_context* context) const {
      return context->map->tiles_count() * 2 + context->map->points_count() + core::map::tri_count_d(core::map::accel_struct_detail_level);
    }
    
    std::string beginner::hint() const {
      return "preparing map data";
    }
    
    plates_generator::plates_generator(const create_info &info) : pool(info.pool) {}
    void plates_generator::process(generator_context* context) {
      utils::time_log log("plates generator");
      
      state = 0;
      
      // скорее всего это нужно переписать в однопоток
      
      auto data = context->data;
      auto map = context->map;
      ASSERT(data->plates_count > 3);
      ASSERT(data->plates_count < 200);
      
      const uint32_t tiles_count = map->tiles_count();
      std::atomic<uint32_t> tile_plate_atomic[tiles_count];
      utils::concurrent_vector<uint32_t> plate_tiles_concurrent[data->plates_count];
      
      utils::concurrent_vector<std::pair<uint32_t, uint32_t>> active_tile_indices;
      utils::concurrent_vector<std::pair<uint32_t, uint32_t>>::default_value = std::make_pair(0, UINT32_MAX);
      
      for (size_t i = 0; i < tiles_count; ++i) {
        tile_plate_atomic[i] = data->plates_count;
      }
      
      std::atomic<uint32_t>* tile_plate_atomic_ptr = tile_plate_atomic;
      utils::concurrent_vector<uint32_t>* plate_tiles_concurrent_ptr = plate_tiles_concurrent;
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
      size_t start = 0;
      size_t count = data->plates_count;
      const uint32_t &plates_count = data->plates_count;
      auto rand = data->engine;
        for (size_t i = start; i < start+count; ++i) {
          uint32_t attempts = 0;
          bool success = false;
          do {
            success = push_new_tile_index(
              map,
              plates_count,
              i,
              rand->index(map->tiles_count()),
              tile_plate_atomic,
              plate_tiles_concurrent,
              active_tile_indices
            );
          } while (!success && attempts < 100);
          
          ++state;
          ASSERT(success);
        }
//       }, map, data->plates_count, data->engine, tile_plate_atomic_ptr, plate_tiles_concurrent_ptr, std::ref(active_tile_indices), std::ref(state));
      
      std::atomic<size_t> counter(data->plates_count);
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
        
        while (true) {
          const auto pair = active_tile_indices.get_random(*rand);
          const uint32_t tile_index = pair.first;
          const uint32_t plate_index = pair.second;

          //if (plate_index == UINT32_MAX) return;
          if (plate_index == UINT32_MAX) break;

//           if (tile_index == UINT32_MAX) {
//             active_tile_indices.push_back(std::make_pair(UINT32_MAX, 0));
//             return;
//           }
          if (tile_index == UINT32_MAX) {
            active_tile_indices.push_back(std::make_pair(UINT32_MAX, 0));
            break;
          }

          uint32_t data = plates_count;
          if (!tile_plate_atomic[tile_index].compare_exchange_strong(data, plate_index)) continue;
          ASSERT(plate_index < plates_count);
          plate_tiles_concurrent[plate_index].push_back(tile_index);

          counter.fetch_add(1);
          ++state;

          // {
            // std::unique_lock<std::mutex> lock(plate_tile_indices[plate_index].mutex);
            // plate_tile_indices[plate_index].vector.push_back(tile_index);
          // }

          const auto &tile = render::unpack_data(map->get_tile(tile_index));
          for (uint32_t i = 0; i < 6; ++i) {
            const uint32_t neighbor_index = tile.neighbors[i];
            if (neighbor_index == UINT32_MAX) continue;
            if (tile_plate_atomic[neighbor_index] != plates_count) continue;
            active_tile_indices.push_back(std::make_pair(neighbor_index, plate_index));
          }
        }
//       }, map, data->plates_count, data->engine, tile_plate_atomic_ptr, plate_tiles_concurrent_ptr, std::ref(active_tile_indices), std::ref(counter), std::ref(state));
      
      // странно но это работает
      std::cout << "tile_count " << tiles_count << "\n";
      std::cout << "counter    " << (counter + data->plates_count) << "\n";

      size_t abc = 0;
      for (size_t i = 0; i < data->plates_count; ++i) {
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
      
      context->tile_plate_indices.resize(tiles_count);
      for (size_t i = 0; i < tiles_count; ++i) {
        context->tile_plate_indices[i] = tile_plate_atomic[i];
      }
      
      context->plate_tile_indices.resize(data->plates_count);
      for (size_t i = 0; i < data->plates_count; ++i) {
        context->plate_tile_indices[i].resize(plate_tiles_concurrent[i].vector.size());
        for (size_t j = 0; j < plate_tiles_concurrent[i].vector.size(); ++j) {
          context->plate_tile_indices[i][j] = plate_tiles_concurrent[i].vector[j];
        }
      }
      
      for (uint32_t i = 0; i < context->tile_plate_indices.size(); ++i) {
        context->map->set_tile_tectonic_plate(i, context->tile_plate_indices[i]);
      }
      
            
      // нужно случайно соединить несколько плит друг с другом
      // несколько итераций, на каждой итерации для каждой плиты
      // смотрим нужно ли ей выдать еще плиту если да то выбиваем случайного соседа,
      // должно остаться несколько плит
      
      const uint32_t min_plates_count = 75;
      const uint32_t max_iterations = 5;
      uint32_t current_plates_count = context->plate_tile_indices.size();
      uint32_t current_iter = 0;
      
      struct next_plates_data {
        std::mutex mutex;
        //std::unordered_set<uint32_t> neighbours; // плох тем, что не гарантирует порядок значений
        std::set<uint32_t> neighbours;             // гарантирует порядок, а значит независим от мультипоточных алгоритмов
      };
      
      std::vector<std::vector<uint32_t>> plate_tiles_local(context->plate_tile_indices);
      std::vector<uint32_t> tile_plates_local(context->tile_plate_indices);
//       std::vector<uint32_t> plate_new_plate(context->plate_tile_indices.size(), UINT32_MAX);
//       for (size_t i = 0; i < context->plate_tile_indices.size(); ++i) {
//         plate_new_plate[i] = i;
//       }
      
      while (current_plates_count > min_plates_count && current_iter < max_iterations) {
//         utils::random_engine_st local_random;
//         rand = &local_random;
        std::vector<next_plates_data> next_plates(context->plate_tile_indices.size());
        utils::submit_works(pool, context->map->tiles_count(), [&next_plates, &tile_plates_local] (const size_t &start, const size_t &count, const generator_context* context) {
          for (size_t i = start; i < start+count; ++i) {
            const auto &tile = render::unpack_data(context->map->get_tile(i));
            for (uint32_t j = 0; j < 6; ++j) {
              const uint32_t tile_neighbour_index = tile.neighbors[j];
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
        }, context);
        
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
        std::vector<bool> plates_union(context->plate_tile_indices.size(), false);
        for (uint32_t i = 0; i < plate_tiles_local.size(); ++i) {
          const uint32_t tiles_count = plate_tiles_local[i].size();
          if (tiles_count < 2) continue;
          
          // я хочу чтобы как можно больше плит соединились на полюсах
          // для этого я беру небольшой коэффициент на основе y компоненты позиции корневого тайла
          const uint32_t root_tile_index = plate_tiles_local[i][0];
          const auto &tile = render::unpack_data(context->map->get_tile(root_tile_index));
          const glm::vec4 root_pos = context->map->get_point(tile.center);
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
            std::vector<uint32_t> neighbours_vector(next_plates[i].neighbours.begin(), next_plates[i].neighbours.end());
            while (plate_index == UINT32_MAX) {
//               ASSERT(!neighbours_vector.empty());
              const uint32_t rand_index = rand->index(neighbours_vector.size());
              const uint32_t index = neighbours_vector[rand_index];
              neighbours_vector[rand_index] = neighbours_vector.back();
              neighbours_vector.pop_back();
              
              if (plates_union[index]) plate_index = index;
              if (neighbours_vector.empty()) break;
            }
            
//             for (auto idx : next_plates[i].neighbours) {
//               if (!plates_union[idx]) continue;
//               // первого соседа?
//               plate_index = idx;
//               break;
//             }
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
      
      uint32_t tiles_counter = 0;
      for (size_t i = 0; i < plate_tiles_local.size(); ++i) {
        ASSERT(plate_tiles_local[i].size() > 1);
        
        for (size_t j = 0; j < plate_tiles_local[i].size(); ++j) {
          const uint32_t tile_index = plate_tiles_local[i][j];
          tile_plates_local[tile_index] = i;
          ASSERT(tile_index < context->map->tiles_count());
          ++tiles_counter;
        }
      }
      
      ASSERT(tiles_counter == context->map->tiles_count());
      
      std::swap(context->tile_plate_indices, tile_plates_local);
      std::swap(context->plate_tile_indices, plate_tiles_local);
      
      for (uint32_t i = 0; i < context->tile_plate_indices.size(); ++i) {
        context->map->set_tile_tectonic_plate(i, context->tile_plate_indices[i]);
      }
      
      PRINT_VAR("tectonic plates count", plates_counter)
      PRINT_VAR("max tiles count", max_tiles_count)
      PRINT_VAR("min tiles count", min_tiles_count)
      
    }
    
    size_t plates_generator::progress() const {
      return state;
    }
    
    size_t plates_generator::complete_state(const generator_context* context) const {
      return context->map->tiles_count();
    }
    
    std::string plates_generator::hint() const {
      return "generating tectonic plates";
    }
    
    plate_datas_generator::plate_datas_generator(const create_info &info) : pool(info.pool) {}
    void plate_datas_generator::process(generator_context* context) {
      utils::time_log log("plate datas generator");
      const auto &plate_tile_indices = context->plate_tile_indices;
      
      state = 0;

      uint32_t max_tiles_count = 0;
      uint32_t min_tiles_count = 1215152;
      const uint32_t plates_count = context->plate_tile_indices.size();
      std::vector<uint32_t> local_plates_indices(plates_count);
      std::vector<tectonic_plate_data_t> tectonic_plate_props(plates_count);
      for (size_t i = 0; i < local_plates_indices.size(); ++i) {
        local_plates_indices[i] = i;
        const uint32_t tiles_count = context->plate_tile_indices[i].size();
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
      
      const uint32_t oceanic_tiles = context->data->ocean_percentage * context->map->tiles_count();
      uint32_t oceanic_tiles_count = 0;
//       const uint32_t ground_tiles = context->map->tiles_count() - oceanic_tiles;
      
//       auto data = context->data;
      auto map = context->map;
      auto rand = context->data->engine;
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
      ASSERT(plate_tile_indices.size() != 0);
      
      for (size_t i = 0; i < plates_count; ++i) {
        // взятие случайных плит: результат выгялит гораздо лучше
        const uint32_t rand_plate_index = rand->index(local_plates_indices.size());
        const uint32_t plate_index = local_plates_indices[rand_plate_index];
        local_plates_indices[rand_plate_index] = local_plates_indices.back();
        local_plates_indices.pop_back();
        
        ASSERT(plate_index < plates_count);
        
        const auto &tile_indices = plate_tile_indices[plate_index];
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
        ++state;
      }
      
      std::swap(context->plate_datas, tectonic_plate_props);
      
      PRINT_VAR("oceanic tiles count", oceanic_tiles_count)
      PRINT_VAR("ground  tiles count", context->map->tiles_count() - oceanic_tiles_count)
      
      uint32_t water_counter = 0;
      for (uint32_t i = 0; i < context->tile_plate_indices.size(); ++i) {
        const uint32_t plate_index = context->tile_plate_indices[i];
        ASSERT(plate_index < context->plate_datas.size());
        context->map->set_tile_tectonic_plate(i, context->plate_datas[plate_index].oceanic);
        water_counter += uint32_t(context->plate_datas[plate_index].oceanic);
      }
      
      PRINT_VAR("oceanic tiles after recompute", water_counter)
      PRINT_VAR("oceanic tiles k              ", float(water_counter) / float(context->map->tiles_count()))
      
      // для сида 1 мне удалось сгенерировать неплохой земной массив
      // со внутренним озером, несколькими континентами, выглядит неплохо даже без дальнейшей генерации
      // другое дело что нужно будет придумать как сделать горные массивы внутри тектонической плиты
      
//       utils::submit_works(pool, data->plates_count, [&local_plates_indices, oceanic_tiles, ground_tiles] (
//         const size_t &start, 
//         const size_t &count,
//         const core::map* map,
//         const std::vector<std::vector<uint32_t>> &plate_tile_indices,
//         const float &ocean_percentage,
//         std::vector<tectonic_plate_data_t> &tectonic_plate_props,
//         utils::random_engine *rand,
//         std::atomic<size_t> &state
//       ) {
//         // эти вещи по идее можно вынести в данные генерации
//         const float min_drift_rate = -PI / 30.0;
//         const float max_drift_rate =  PI / 30.0;
//         const float min_spin_rate  = -PI / 30.0;
//         const float max_spin_rate  =  PI / 30.0;
// 
//         const float min_oceanic_elevation = -0.8f;
//         const float max_oceanic_elevation = -0.3f;
//         const float min_continental_elevation = 0.1f;
//         const float max_continental_elevation = 0.5f;
//         ASSERT(plate_tile_indices.size() != 0);
//         
//         for (size_t i = start; i < start+count; ++i) {
//           const uint32_t &plate_index = local_plates_indices[i];
//           const auto &tile_indices = plate_tile_indices[plate_index];
//           ASSERT(tile_indices.size() != 0);
//           const glm::vec3 drift_axis = rand->unit3();
//           const float drift_rate = rand->closed(min_drift_rate, max_drift_rate);
//           // тут берем случайный тайл на плите, но вообще можно любой случайный тайл брать
//           const uint32_t idx = rand->index(tile_indices.size());
//           ASSERT(idx < tile_indices.size());
//           const uint32_t rand_index = tile_indices[idx];
//           const auto &tile = map->get_tile(rand_index);
//           const glm::vec3 spin_axis = map->get_point(tile.tile_indices.x);
//           const float spin_rate = rand->closed(min_spin_rate, max_spin_rate);
//           const bool oceanic = rand->probability(ocean_percentage);
//           const float base_elevation = oceanic ? rand->closed(min_oceanic_elevation, max_oceanic_elevation) :
//                                                  rand->closed(min_continental_elevation, max_continental_elevation);
// 
//           const tectonic_plate_data_t plate{
//             drift_axis,
//             drift_rate,
//             spin_axis,
//             spin_rate,
//             base_elevation,
//             oceanic
//           };
//           tectonic_plate_props[i] = plate;
//           ++state;
//         }
//       }, map, plate_tile_indices, data->ocean_percentage, std::ref(context->plate_datas), data->engine, std::ref(state));
    }
    
    size_t plate_datas_generator::progress() const {
      return state;
    }
    
    size_t plate_datas_generator::complete_state(const generator_context* context) const {
      return context->data->plates_count;
    }
    
    std::string plate_datas_generator::hint() const {
      return "generating tectonic plates properties";
    }
    
    compute_boundary_edges::compute_boundary_edges(const create_info &info) : pool(info.pool) {}
    void compute_boundary_edges::process(generator_context* context) {
      utils::time_log log("compute boundary edges");
      std::set<size_t> pairs_set;
      std::mutex mutex;
      state = 0;
      
      utils::submit_works(pool, context->map->tiles_count(), [&pairs_set, &mutex] (const size_t &start, const size_t &count, const generator_context* context, std::atomic<size_t> &state) {
        for (size_t i = start; i < start+count; ++i) {
          const uint32_t current_tile_index = i;
          const uint32_t current_plate_index = context->tile_plate_indices[current_tile_index];
          ASSERT(current_plate_index < context->data->plates_count);
          const auto &tile = render::unpack_data(context->map->get_tile(current_tile_index));
          const uint32_t n_count = render::is_pentagon(tile) ? 5 : 6;
          for (uint32_t j = 0; j < n_count; ++j) {
            const uint32_t neighbour_tile_index = tile.neighbors[j];
            ASSERT(current_tile_index != neighbour_tile_index);
            
            const uint32_t neighbour_tile_plate_index = context->tile_plate_indices[neighbour_tile_index];
            ASSERT(neighbour_tile_plate_index < context->data->plates_count);
            if (current_plate_index != neighbour_tile_plate_index) {
              const uint32_t min = std::min(current_tile_index, neighbour_tile_index);
              const uint32_t max = std::max(current_tile_index, neighbour_tile_index);
              const size_t key = (size_t(min) << 32) | size_t(max);
              std::unique_lock<std::mutex> lock(mutex);
              pairs_set.insert(key);
            }
          }
          
          ++state;
        }
      }, context, std::ref(state));
      
      PRINT_VAR("edges count", pairs_set.size())
      
      context->boundary_edges.resize(pairs_set.size());
      auto itr = pairs_set.begin();
      for (uint32_t i = 0; i < pairs_set.size(); ++i) {
        const size_t key = *itr;
        const uint32_t first  = uint32_t(key & UINT32_MAX);
        const uint32_t second = uint32_t((key >> 32) & UINT32_MAX);
        
        context->boundary_edges[i] = std::make_pair(first, second);
        
        ++itr;
      }
    }
    
    size_t compute_boundary_edges::progress() const {
      return state;
    }
    
    size_t compute_boundary_edges::complete_state(const generator_context* context) const {
      return context->map->tiles_count();
    }
    
    std::string compute_boundary_edges::hint() const {
      return "generating tectonic plates boundary edges";
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
    
    compute_plate_boundary_stress::compute_plate_boundary_stress(const create_info &info) : pool(info.pool) {}
    void compute_plate_boundary_stress::process(generator_context* context) {
      utils::time_log log("compute plate boundary stresses");
      std::vector<boundary_stress_t> boundary_stresses(context->boundary_edges.size());
      state = 0;
      utils::submit_works(pool, context->boundary_edges.size(), [&boundary_stresses] (const size_t &start, const size_t &count, const generator_context* context, std::atomic<size_t> &state) {
        for (size_t i = start; i < start+count; ++i) {
          const uint32_t first_tile  = context->boundary_edges[i].first;
          const uint32_t second_tile = context->boundary_edges[i].second;
          const uint32_t first_plate  = context->tile_plate_indices[first_tile];
          const uint32_t second_plate = context->tile_plate_indices[second_tile];
          ASSERT(first_tile < context->map->tiles_count());
          ASSERT(second_tile < context->map->tiles_count());
          ASSERT(first_plate < context->data->plates_count);
          ASSERT(second_plate < context->data->plates_count);
          ASSERT(first_plate != second_plate);
          const auto &prop1 = context->plate_datas[first_plate];
          const auto &prop2 = context->plate_datas[second_plate];
          
          const auto &first_tile_data = context->map->get_tile(first_tile);
          const auto &second_tile_data = context->map->get_tile(second_tile);
          const glm::vec4 first_tile_pos  = context->map->get_point(first_tile_data.tile_indices[0]);
          const glm::vec4 second_tile_pos = context->map->get_point(second_tile_data.tile_indices[0]);
          
          const glm::vec4 boundary_position1 = (first_tile_pos + second_tile_pos) / 2.0f;
          const glm::vec4 boundary_position = glm::normalize(boundary_position1 * glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)) + glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
          //const glm::vec4 boundary_normal = second_tile_pos - first_tile_pos;
          const glm::vec4 boundary_normal = glm::normalize(second_tile_pos - first_tile_pos);
          const glm::vec4 boundary_vector = glm::vec4(glm::normalize(glm::cross(glm::vec3(boundary_normal), glm::vec3(boundary_position))), 0.0f);
          
          const glm::vec4 plate_movement0 = calculate_axial_rotation(prop1.drift_axis, prop1.drift_rate, boundary_position) + calculate_axial_rotation(prop1.spin_axis, prop1.spin_rate, boundary_position);
          const glm::vec4 plate_movement1 = calculate_axial_rotation(prop2.drift_axis, prop2.drift_rate, boundary_position) + calculate_axial_rotation(prop2.spin_axis, prop2.spin_rate, boundary_position);
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
          
          ++state;
        }
      }, context, std::ref(state));
      
      std::swap(boundary_stresses, context->boundary_stresses);
    }
    
    size_t compute_plate_boundary_stress::progress() const {
      return state;
    }
    
    size_t compute_plate_boundary_stress::complete_state(const generator_context* context) const {
      return context->boundary_edges.size();
    }
    
    std::string compute_plate_boundary_stress::hint() const {
      return "calculating tectonic plates boundary stresses";
    }
    
    uint32_t find_index(const std::vector<std::pair<uint32_t, uint32_t>> &boundary_edges, const uint32_t &index, const uint32_t &start) {
      for (uint32_t i = start; i < boundary_edges.size(); ++i) {
        if (boundary_edges[i].first == index || boundary_edges[i].second == index) return i;
        //if (boundary_edges[i].first == index) return boundary_edges[i].second;
        //if (boundary_edges[i].second == index) return boundary_edges[i].first;
      }
      
      return UINT32_MAX;
    }
    
    void add_boundary_neighbours_indices(const std::vector<std::pair<uint32_t, uint32_t>> &boundary_edges, const uint32_t &index, const uint32_t &pair_index, std::vector<uint32_t> &neighbours) {
      uint32_t edge_index = find_index(boundary_edges, index, 0);
      while (edge_index != UINT32_MAX) {
        if (edge_index != pair_index) {
//           const auto &edge_pair = boundary_edges[edge_index];
//           if (edge_pair.first == index) neighbours.push_back(edge_pair.second);
//           if (edge_pair.second == index) neighbours.push_back(edge_pair.first);
          neighbours.push_back(edge_index);
        }
        edge_index = find_index(boundary_edges, index, edge_index+1);
      }
    }
    
    void add_neighbours_boundary_data(const generator_context* context, const uint32_t &pair_index, float &pressure_sum, float &shear_sum, glm::vec4 &movement0_sum, glm::vec4 &movement1_sum, uint32_t &count) {
      const auto &pair = context->boundary_edges[pair_index];
      const uint32_t plate0 = context->tile_plate_indices[pair.first];
      const uint32_t plate1 = context->tile_plate_indices[pair.second];
      
      for (uint32_t i = 0; i < context->boundary_edges.size(); ++i) {
        if (pair_index == i) continue;
        const auto &pair = context->boundary_edges[i];
        const uint32_t plate0_n = context->tile_plate_indices[pair.first];
        const uint32_t plate1_n = context->tile_plate_indices[pair.second];
        
        if ((plate0 == plate0_n && plate1 == plate1_n) || (plate0 == plate1_n && plate1 == plate0_n)) {
          const auto &data = context->boundary_stresses[i];
          pressure_sum += data.pressure;
          shear_sum += data.shear;
          movement0_sum += data.pressure_vector;
          movement1_sum += data.shear_vector;
          ++count;
        }
      }
    }
    
    blur_plate_boundary_stress::blur_plate_boundary_stress(const create_info &info) : pool(info.pool), state(0) {}
    void blur_plate_boundary_stress::process(generator_context* context) {
      utils::time_log log("blur plate boundary stress");
      state = 0;
      std::vector<boundary_stress_t> new_boundary_stresses(context->boundary_stresses.size());
      const uint32_t iteration_count = context->data->blur_plate_boundaries_iteration_count;
      
      ASSERT(context->boundary_stresses.size() != 0);
//       PRINT_VAR("boundary_stresses.size()", context->boundary_stresses.size())
      
      for (uint32_t i = 0 ; i < iteration_count; ++i) {
//       PRINT_VAR("iteration_count", i)
        
        utils::submit_works(pool, context->boundary_edges.size(), [&new_boundary_stresses] (const size_t &start, const size_t &count, const generator_context* context, std::atomic<size_t> &state) {
          const float center_weighting = context->data->blur_plate_boundaries_center_weighting < 0.0f || context->data->blur_plate_boundaries_center_weighting > 1.0f ? 0.4f : context->data->blur_plate_boundaries_center_weighting;
          const float inverse_center_weighting = 1.0f - center_weighting;
          
          std::vector<uint32_t> neighbours;
          for (size_t i = start; i < start+count; ++i) {
            // найти все эджи рядом, по идее это поиск по массиву
//             const auto &pair = context->boundary_edges[i];
//             add_boundary_neighbours_indices(context->boundary_edges, pair.first, i, neighbours);
//             add_boundary_neighbours_indices(context->boundary_edges, pair.second, i, neighbours);
//             
//             float pressure_sum = 0.0f;
//             float shear_sum = 0.0f;
//             glm::vec4 pressure_vec_sum = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
//             glm::vec4 shear_vec_sum = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
//             uint32_t neighbour_count = 0;
//             for (const auto &neighbor_index : neighbours) {
//               ASSERT(neighbor_index < context->boundary_stresses.size());
//               
//               const auto &data = context->boundary_stresses[neighbor_index];
//               pressure_sum += data.pressure;
//               shear_sum += data.shear;
// //               pressure_vec_sum += data.pressure_vector;
// //               shear_vec_sum += data.shear_vector;
//               ++neighbour_count;
//             }
            
            float pressure_sum = 0.0f;
            float shear_sum = 0.0f;
            uint32_t neighbour_count = 0;
            glm::vec4 pressure_vec_sum = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
            glm::vec4 shear_vec_sum = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
            add_neighbours_boundary_data(context, i, pressure_sum, shear_sum, pressure_vec_sum, shear_vec_sum, neighbour_count);
            
            const auto &old_stress = context->boundary_stresses[i];
            const boundary_stress_t stress{
              neighbour_count == 0 ? old_stress.pressure : old_stress.pressure * center_weighting + (pressure_sum / float(neighbour_count)) * inverse_center_weighting,
              neighbour_count == 0 ? old_stress.shear    : old_stress.shear    * center_weighting + (shear_sum    / float(neighbour_count)) * inverse_center_weighting,
              (neighbour_count == 0 ? old_stress.pressure_vector : old_stress.pressure_vector * center_weighting + (pressure_vec_sum    / float(neighbour_count)) * inverse_center_weighting),
              (neighbour_count == 0 ? old_stress.shear_vector : old_stress.shear_vector    * center_weighting + (shear_vec_sum    / float(neighbour_count)) * inverse_center_weighting)
            };
            
            new_boundary_stresses[i] = stress;
            neighbours.clear();
            ++state;
          }
        }, context, std::ref(state));
        
        std::swap(context->boundary_stresses, new_boundary_stresses);
      }
      
//       for (size_t i = 0; i < context->boundary_stresses.size(); ++i) {
//         const std::string s = "boundary_edge " + std::to_string(i) + " pressure " + std::to_string(context->boundary_stresses[i].pressure) + " shear " + std::to_string(context->boundary_stresses[i].shear) + " \n";
//         std::cout << s;
//       }
    }
    
    size_t blur_plate_boundary_stress::progress() const {
      return state;
    }
    
    size_t blur_plate_boundary_stress::complete_state(const generator_context* context) const {
      return context->data->blur_plate_boundaries_iteration_count * context->boundary_edges.size();
    }
    
    std::string blur_plate_boundary_stress::hint() const {
      return "bluring tectonic plates boundary stresses";
    }
    
    void assign_distance_field(
      const generator_context* context, 
      const uint32_t &plate_index, 
      const std::set<uint32_t> &seeds, 
      const std::unordered_set<uint32_t> &stops, 
      utils::random_engine_st* rand,
      std::vector<std::pair<uint32_t, float>> &distances
    ) {
//       utils::random_engine_st rand(plate_index);
      std::vector<uint32_t> queue;
      
      for (auto idx : seeds) {
        ASSERT(idx < context->boundary_edges.size());
        const uint32_t tile_index0 = context->boundary_edges[idx].first;
        const uint32_t tile_index1 = context->boundary_edges[idx].second;
        const uint32_t current_plate_idx0 = context->tile_plate_indices[tile_index0];
        const uint32_t current_plate_idx1 = context->tile_plate_indices[tile_index1];
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
          const uint32_t n_index = tile_data.neighbors[i];
          if (n_index == UINT32_MAX) continue;
          
//           const auto &n_tile_data = render::unpack_data(context->map->get_tile(n_index));
//           
//           const glm::vec4 tile_point = context->map->get_point(tile_data.center);
//           const glm::vec4 n_tile_point = context->map->get_point(n_tile_data.center);
//           
//           const float test_dist = glm::distance(tile_point, n_tile_point);
//           ASSERT(test_dist < 3.0f);
          
          const uint32_t current_plate_idx = context->tile_plate_indices[n_index];
          if (plate_index != current_plate_idx) continue;
          
          if (distances[n_index].second == 100000.0f && stops.find(n_index) == stops.end()) {
            distances[n_index] = std::make_pair(distances[current_tile].first, distances[current_tile].second + 1.0f);
            queue.push_back(n_index);
          }
        }
      }
    }
    
    calculate_plate_boundary_distances::calculate_plate_boundary_distances(const create_info &info) : pool(info.pool) {}
    void calculate_plate_boundary_distances::process(generator_context* context) {
      utils::time_log log("calculate plate boundary distances");
      // для каждого тайла нужно задать дальность до ближайшей гланицы с плитой
      // по идее это можно сделать по плитам
      
      // этот алгоритм должен давать тот же результат для тех же входных данных
      // (одинаковый context->boundary_edges, по идее простой сортировкой мы можем добиться чтобы этот массив был одинаковым)
      
      // попробуем сделать карты дальности для каждого тайла на плите
      
      state = 0;
      std::vector<std::pair<uint32_t, float>> edge_index_dist(context->map->tiles_count(), std::make_pair(UINT32_MAX, 100000.0f));
      //std::mutex mutex;
      utils::submit_works(pool, context->plate_tile_indices.size(), [&edge_index_dist] (const size_t &start, const size_t &count, const generator_context* context, std::atomic<size_t> &state) {
        //std::unordered_set<uint32_t> unique_set;
        std::queue<uint32_t> queue0;
        std::queue<uint32_t> queue1;
        for (size_t i = start; i < start+count; ++i) {
          for (uint32_t j = 0; j < context->boundary_edges.size(); ++j) {
            const auto &pair = context->boundary_edges[j];
            const uint32_t plate_index1 = context->tile_plate_indices[pair.first];
            if (plate_index1 == i) {
// #ifndef _NDEBUG
//               auto itr = unique_set.find(pair.first);
//               ASSERT(itr == unique_set.end());
// #endif
//               unique_set.insert(pair.first);
              queue0.push(pair.first);
              
              const auto &tile0 = context->map->get_tile(pair.first);
              const auto &tile1 = context->map->get_tile(pair.second);
              
              const auto &point0 = context->map->get_point(tile0.tile_indices.x);
              const auto &point1 = context->map->get_point(tile1.tile_indices.x);
              
              const float dist = glm::distance(point0, point1) / 2.0f;
              
              //std::unique_lock<std::mutex> lock(mutex);
              //edge_index_dist.push_back(std::make_pair(pair.first, dist));
              edge_index_dist[pair.first] = std::make_pair(j, dist);
            }
            
            const uint32_t plate_index2 = context->tile_plate_indices[pair.second];
            if (plate_index2 == i) {
// #ifndef _NDEBUG
//               auto itr = unique_set.find(pair.second);
//               ASSERT(itr == unique_set.end());
// #endif
//               
//               unique_set.insert(pair.second);
              queue0.push(pair.second);
              
              const auto &tile0 = context->map->get_tile(pair.first);
              const auto &tile1 = context->map->get_tile(pair.second);
              
              const auto &point0 = context->map->get_point(tile0.tile_indices.x);
              const auto &point1 = context->map->get_point(tile1.tile_indices.x);
              
              const float dist = glm::distance(point0, point1) / 2.0f;
              
              //std::unique_lock<std::mutex> lock(mutex);
              //edge_index_dist.push_back(std::make_pair(pair.second, dist));
              edge_index_dist[pair.second] = std::make_pair(j, dist);
            }
                          
            ASSERT(plate_index2 != plate_index1);
          }
          
          while (!queue0.empty()) {
            while (!queue0.empty()) {
              const uint32_t current_tile_index = queue0.front();
              queue0.pop();
              
              const auto &tile = render::unpack_data(context->map->get_tile(current_tile_index));
              for (uint32_t j = 0; j < 6; ++j) {
                if (tile.neighbors[j] == UINT32_MAX) continue;
                const uint32_t neighbour_tile_index = tile.neighbors[j];
                const uint32_t plate_index = context->tile_plate_indices[neighbour_tile_index];
                if (plate_index != i) continue;
                
                const auto &tile1 = context->map->get_tile(neighbour_tile_index);
                
                const auto &point0 = context->map->get_point(tile.center);
                const auto &point1 = context->map->get_point(tile1.tile_indices.x);
                
                //std::unique_lock<std::mutex> lock(mutex);
                //edge_index_dist.push_back(std::make_pair(neighbour_tile_index, dist));
                const auto &prev = edge_index_dist[current_tile_index];
                const float dist = glm::distance(point0, point1) + prev.second;
                if (edge_index_dist[neighbour_tile_index].second > dist) {
                  edge_index_dist[neighbour_tile_index] = std::make_pair(prev.first, dist);
                  queue1.push(neighbour_tile_index);
                }
              }
            }
            
            std::swap(queue0, queue1);
          }
          
          //unique_set.clear();
          {
            const size_t size = queue0.size();
            for (size_t i = 0; i < size; ++i) queue0.pop();
            ASSERT(queue0.empty());
          }
          
          {
            const size_t size = queue1.size();
            for (size_t i = 0; i < size; ++i) queue1.pop();
            ASSERT(queue1.empty());
          }
          
          ++state;
        }
      }, context, std::ref(state));
      
      std::set<uint32_t> mountains; // unordered версия нестабильная!
      std::set<uint32_t> oceans;
      std::set<uint32_t> coastlines; 
      std::mutex m1;
      std::mutex m2;
      std::mutex m3;
      utils::submit_works(pool, context->boundary_edges.size(), [&mountains, &oceans, &coastlines, &m1, &m2, &m3] (const size_t &start, const size_t &count, const generator_context* context) {
//         const size_t start = 0;
//         const size_t count = context->boundary_edges.size();
        for (size_t i = start; i < start+count; ++i) {
          const auto &pair = context->boundary_edges[i];
          
          const uint32_t tile_index0 = pair.first;
          const uint32_t tile_index1 = pair.second;
          
          const uint32_t plate_index0 = context->tile_plate_indices[tile_index0];
          const uint32_t plate_index1 = context->tile_plate_indices[tile_index1];
          
          const auto &data0 = context->plate_datas[plate_index0];
          const auto &data1 = context->plate_datas[plate_index1];
          
          ASSERT(context->boundary_stresses.size() != 0);
          const auto &boundary_data = context->boundary_stresses[i];
          
          // необходимо какое-то ограничение или коэффициент
          // иначе получается плохо
          // у нас задача: раскидать границы по эффектам столкновения и расхождения плит
          // в зависимости от типа взаимодействия на границе мы получаем разные типы поверхностей
          // 
          
          const auto &plate_movement0 = boundary_data.pressure_vector;
          const auto &plate_movement1 = boundary_data.shear_vector;
          
          const float length0 = glm::length(plate_movement0);
          const float length1 = glm::length(plate_movement1);
          
          const auto dir0 = length0 < EPSILON ? glm::vec4(0.0f, 0.0f, 0.0f, 0.0f) : plate_movement0 / length0;
          const auto dir1 = length1 < EPSILON ? glm::vec4(0.0f, 0.0f, 0.0f, 0.0f) : plate_movement1 / length1;
          
          const uint32_t first_tile  = pair.first;
          const uint32_t second_tile = pair.second;
          const auto &first_tile_data = context->map->get_tile(first_tile);
          const auto &second_tile_data = context->map->get_tile(second_tile);
          const glm::vec4 first_tile_pos  = context->map->get_point(first_tile_data.tile_indices[0]);
          const glm::vec4 second_tile_pos = context->map->get_point(second_tile_data.tile_indices[0]);
          
//           const glm::vec4 boundary_position = (second_tile_pos + first_tile_pos) / 2.0f;
          const glm::vec4 boundary_normal = glm::normalize(second_tile_pos - first_tile_pos);
          
          const float dot0 = glm::dot(dir0,  boundary_normal);
          const float dot1 = glm::dot(dir1, -boundary_normal);
          
          //const bool collided = boundary_data.pressure > 0.3f;
          const bool collided = dot0 > 0.3f && dot1 > 0.3f;
          const bool opposite_dir = dot0 < -0.3f && dot1 < -0.3f;
          if (data0.oceanic && data1.oceanic) {
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
              coastlines.insert(i);
            }
          } else if (!data0.oceanic && !data1.oceanic) {
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
      }, context);
      
      std::vector<std::pair<uint32_t, float>> mountain_dist(context->map->tiles_count(), std::make_pair(UINT32_MAX, 100000.0f));
      std::vector<std::pair<uint32_t, float>> ocean_dist(context->map->tiles_count(), std::make_pair(UINT32_MAX, 100000.0f));
      std::vector<std::pair<uint32_t, float>> coastline_dist(context->map->tiles_count(), std::make_pair(UINT32_MAX, 100000.0f));
      std::unordered_set<uint32_t> stops;
      for (auto idx : mountains) {
        const auto &pair = context->boundary_edges[idx];
        stops.insert(pair.first);
        stops.insert(pair.second);
      }
      
      for (auto idx : oceans) {
        const auto &pair = context->boundary_edges[idx];
        stops.insert(pair.first);
        stops.insert(pair.second);
      }
      
      for (auto idx : coastlines) {
        const auto &pair = context->boundary_edges[idx];
        stops.insert(pair.first);
        stops.insert(pair.second);
      }
      
      std::unordered_set<uint32_t> ocean_stops;
      for (auto idx : oceans) {
        const auto &pair = context->boundary_edges[idx];
        ocean_stops.insert(pair.first);
        ocean_stops.insert(pair.second);
      }
      
      std::unordered_set<uint32_t> coastline_stops;
      for (auto idx : coastlines) {
        const auto &pair = context->boundary_edges[idx];
        coastline_stops.insert(pair.first);
        coastline_stops.insert(pair.second);
      }
      
      utils::submit_works(pool, context->plate_tile_indices.size(), [&mountains, &oceans, &coastlines, &ocean_stops, &coastline_stops, &stops, &mountain_dist, &ocean_dist, &coastline_dist] (const size_t &start, const size_t &count, const generator_context* context, std::atomic<size_t> &state) {
//         const size_t start = 0;
//         const size_t count = context->plate_tile_indices.size();
        for (size_t i = start; i < start+count; ++i) {
          const uint32_t plate_index = i;
          
          utils::random_engine_st eng(plate_index);
          assign_distance_field(context, plate_index, mountains, ocean_stops, &eng, mountain_dist);
          assign_distance_field(context, plate_index, oceans, coastline_stops, &eng, ocean_dist);
          assign_distance_field(context, plate_index, coastlines, stops, &eng, coastline_dist);
        }
      }, context, std::ref(state));
      
      std::swap(context->edge_index_dist, edge_index_dist);
      std::swap(context->mountain_dist, mountain_dist);
      std::swap(context->ocean_dist, ocean_dist);
      std::swap(context->coastline_dist, coastline_dist);
      
      std::vector<std::pair<uint32_t, uint32_t>> boundary_dist(context->map->tiles_count(), std::make_pair(UINT32_MAX, UINT32_MAX));
      //std::vector<std::pair<uint32_t, uint32_t>> edges_local();
      utils::submit_works(pool, context->plate_tile_indices.size(), [&boundary_dist] (const size_t &start, const size_t &count, const generator_context* context) {
        for (size_t i = start; i < start+count; ++i) {
          const uint32_t plate_index = i;
          //std::vector<std::pair<uint32_t, uint32_t>> edges_local;
          std::queue<uint32_t> queue;
          for (size_t j = 0; j < context->boundary_edges.size(); ++j) {
            const auto &pair = context->boundary_edges[j];
            
            const uint32_t plate_index0 = context->tile_plate_indices[pair.first];
            const uint32_t plate_index1 = context->tile_plate_indices[pair.second];
            
            ASSERT(plate_index0 != plate_index1);
            
            if (plate_index0 == plate_index) {
              //edges_local.push_back(j, 0);
              queue.push(pair.first);
              boundary_dist[pair.first] = std::make_pair(j, 0);
            }
            
            if (plate_index1 == plate_index) {
              queue.push(pair.second);
              boundary_dist[pair.second] = std::make_pair(j, 0);
            }
          }
          
          while (!queue.empty()) {
            const uint32_t tile_index = queue.front();
            queue.pop();
            
            const auto &tile_data = render::unpack_data(context->map->get_tile(tile_index));
            const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
            for (uint32_t j = 0; j < n_count; ++j) {
              const uint32_t n_index = tile_data.neighbors[j];
              
              if (boundary_dist[n_index].first == UINT32_MAX) {
                boundary_dist[n_index] = std::make_pair(boundary_dist[tile_index].first, boundary_dist[tile_index].second + 1);
                queue.push(n_index);
              }
            }
          }
        }
      }, context);
      
      std::vector<uint32_t> max_dist(context->map->tiles_count(), UINT32_MAX);
      utils::submit_works(pool, context->plate_tile_indices.size(), [&max_dist, &boundary_dist] (const size_t &start, const size_t &count, const generator_context* context) {
        for (size_t i = start; i < start+count; ++i) {
          const uint32_t plate_index = i;
          
          std::queue<uint32_t> queue;
          for (size_t j = 0; j < boundary_dist.size(); ++j) {
            const uint32_t plate_idx = context->tile_plate_indices[j];
            if (plate_idx != plate_index) continue;
             
            const uint32_t tile_dist = boundary_dist[j].second;
            if (tile_dist == 0) continue;
            const auto &tile_data = render::unpack_data(context->map->get_tile(j));
            const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
            uint32_t n_counter = 0;
            uint32_t n_count_valid = 0;
            for (uint32_t k = 0; k < n_count; ++k) {
              const uint32_t n_index = tile_data.neighbors[k];
              const uint32_t plate_idx = context->tile_plate_indices[n_index];
              if (plate_idx != plate_index) continue;
              
              ++n_count_valid;
              const uint32_t n_dist = boundary_dist[n_index].second;
              if (n_dist <= tile_dist) ++n_counter;
            }
            
            if (n_counter == n_count_valid) {
              max_dist[j] = 0;
              queue.push(j);
            }
          }
          
          while (!queue.empty()) {
            const uint32_t tile_index = queue.front();
            queue.pop();
            
            const auto &tile_data = render::unpack_data(context->map->get_tile(tile_index));
            const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
            for (uint32_t j = 0; j < n_count; ++j) {
              const uint32_t n_index = tile_data.neighbors[j];
              
              if (max_dist[n_index] == UINT32_MAX) {
                max_dist[n_index] = max_dist[tile_index] + 1;
                queue.push(n_index);
              }
            }
          }
        }
      }, context);
       
      std::swap(context->test_boundary_dist, boundary_dist);
      std::swap(context->test_max_dist, max_dist);
    }
    
    size_t calculate_plate_boundary_distances::progress() const {
      return state;
    }
    
    size_t calculate_plate_boundary_distances::complete_state(const generator_context* context) const {
      return context->plate_tile_indices.size();
    }
    
    std::string calculate_plate_boundary_distances::hint() const {
      return "calculating plate tile boundaries distances";
    }
    
    calculate_plate_root_distances::calculate_plate_root_distances(const create_info &info) : pool(info.pool) {}
    void calculate_plate_root_distances::process(generator_context* context) {
      utils::time_log log("calculate plate root distances");
      // в context->plate_tile_indices первым должен быть рут индекс
      // скорее всего так и есть даже при мультитрединге
      // но проблема в другом, алгоритм при мультитрединге непоследователен
      // а значит может давать разные результаты при одинаковых входных данных
      
      state = 0;
      std::vector<float> root_distances(context->map->tiles_count(), 100000.0f);
//       utils::submit_works(pool, context->plate_tile_indices.size(), [&root_distances] (const size_t &start, const size_t &count, const generator_context* context, std::atomic<size_t> &state) {
//         for (size_t i = start; i < start+count; ++i) {
//           std::queue<uint32_t> queue0;
//           std::queue<uint32_t> queue1;
//           
//           const uint32_t root_tile = context->plate_tile_indices[i][0];
//           root_distances[root_tile] = 0.0f;
//           queue0.push(root_tile);
//           
//           while (!queue0.empty()) {
//             while (!queue0.empty()) {
//               const uint32_t current_tile_index = queue0.front();
//               queue0.pop();
//               
//               const auto &tile = render::unpack_data(context->map->get_tile(current_tile_index));
//               for (uint32_t j = 0; j < 6; ++j) {
//                 if (tile.neighbours[j] == UINT32_MAX) continue;
//                 const uint32_t neighbour_tile_index = tile.neighbours[j];
//                 const uint32_t plate_index = context->tile_plate_indices[neighbour_tile_index];
//                 if (plate_index != i) continue;
//                           
//                 const auto &tile1 = context->map->get_tile(neighbour_tile_index);
//                 
//                 const auto &point0 = context->map->get_point(tile.center);
//                 const auto &point1 = context->map->get_point(tile1.tile_indices.x);
//                 
//                 //std::unique_lock<std::mutex> lock(mutex);
//                 //edge_index_dist.push_back(std::make_pair(neighbour_tile_index, dist));
//                 const auto &prev = root_distances[current_tile_index];
//                 const float dist = glm::distance(point0, point1) + prev;
//                 if (root_distances[neighbour_tile_index] > dist) {
//                   root_distances[neighbour_tile_index] = dist;
//                   queue1.push(neighbour_tile_index);
//                 }
//               }
//             }
//             
//             std::swap(queue0, queue1);
//           }
//           ++state;
//         }
//       }, context, std::ref(state));
      
      utils::submit_works(pool, context->tile_plate_indices.size(), [&root_distances] (const size_t &start, const size_t &count, const generator_context* context, std::atomic<size_t> &state) {
        for (size_t i = start; i < start+count; ++i) {
          const uint32_t &plate_index = context->tile_plate_indices[i];
          const uint32_t &root_index = context->plate_tile_indices[plate_index][0];
          
          const auto &tile = context->map->get_tile(i);
          const auto &root_tile = context->map->get_tile(root_index);
          
          const glm::vec4 tile_center = context->map->get_point(tile.tile_indices.x);
          const glm::vec4 root_center = context->map->get_point(root_tile.tile_indices.x);
          
          root_distances[i] = glm::distance(tile_center, root_center);
          
          ++state;
        }
      }, context, std::ref(state));
      
      std::swap(context->root_distances, root_distances);
    }
    
    size_t calculate_plate_root_distances::progress() const {
      return state;
    }
    
    size_t calculate_plate_root_distances::complete_state(const generator_context* context) const {
      return context->plate_tile_indices.size();
    }
    
    std::string calculate_plate_root_distances::hint() const {
      return "calculating plate tiles root distances";
    }
    
    uint32_t get_neighbours_at_dir(const generator_context* context, const glm::vec4 &dir, const uint32_t &tile_index, uint32_t &start) {
      const auto &tile = render::unpack_data(context->map->get_tile(tile_index));
      const auto base_pos = context->map->get_point(tile.center);
      const uint32_t n_count = render::is_pentagon(tile) ? 5 : 6;
      for (uint32_t i = start; i < n_count; ++i) {
        const uint32_t n_tile_index = tile.neighbors[i];
        const auto &n_tile = (context->map->get_tile(n_tile_index));
        const auto pos = context->map->get_point(n_tile.tile_indices.x);
        
        const auto tile_dir = pos - base_pos;
        if (glm::dot(tile_dir, dir) > 0.0f) return i;
      }
      
      return UINT32_MAX;
    }
    
#define MAX_ELEVATION_IMPACT 0.1f
    
    void plate_superducting_req(
      const generator_context* context, 
      const std::pair<uint32_t, uint32_t> &edge, 
      const glm::vec4 &prev_pos, 
      const float &current_length, 
      const float &accumulated_dist,
      const float &dist_impact_k,
      const uint32_t &current_tile_index, 
      std::vector<std::pair<std::mutex, float>> &elevations
    ) {
      const auto &tile_data = render::unpack_data(context->map->get_tile(current_tile_index));
      const glm::vec4 tile_pos  = context->map->get_point(tile_data.center);
      
      // на сфере по идее неверная дальность
      const float dist = glm::distance(tile_pos, prev_pos);
      const float next_dist = accumulated_dist + dist;
      const float final_dist = next_dist / dist_impact_k;
      //const float max_dist_impact = current_length * dist_impact_k;
      const float dist_k = std::min(final_dist, current_length) / current_length; // возможно должен быть квадратичным
      const float final_dist_k = 1.0f - dist_k;
      
      const uint32_t plate_index = context->tile_plate_indices[current_tile_index];
      const auto &plate_data = context->plate_datas[plate_index];
      
      const float next_length = current_length - dist / dist_impact_k;
      auto& elevation = elevations[current_tile_index];
      
      float elevation_k = 0;
      {
        std::unique_lock<std::mutex> lock(elevation.first);
        const float max_elevation_impact = MAX_ELEVATION_IMPACT;
        const float sub = std::min(std::abs(elevation.second - plate_data.base_elevation), max_elevation_impact);
        elevation_k = 1.0f - sub / max_elevation_impact;
        
        const float sum_k_mul = 0.8f;
        const float sum_k = final_dist_k * sum_k_mul + elevation_k * (1.0f - sum_k_mul);
        
        const float l_modify = max_elevation_impact * (1 / next_length);
        const float final_k = glm::mix(0.0f, l_modify, sum_k);
        const float coef = current_length * final_k;
        elevation.second = elevation.second + coef;
      }
      
      //const float next_length = current_length - dist / dist_impact_k;
      //if (next_length < EPSILON) return;
      if (current_length < final_dist) return;
      
      for (uint32_t i = 0; i < 6; ++i) {
        const uint32_t n_index = tile_data.neighbors[i];
        if (n_index == UINT32_MAX) continue;
        
        const uint32_t n_plate_index = context->tile_plate_indices[n_index];
        if (plate_index != n_plate_index) continue;
        
        plate_superducting_req(context, edge, tile_pos, current_length, next_dist, dist_impact_k, n_index, elevations);
      }
    }
    
    void plate_subducting_req(
      const generator_context* context, 
      const std::pair<uint32_t, uint32_t> &edge, 
      const glm::vec4 &prev_pos, 
      const float &current_length, 
      const float &accumulated_dist,
      const float &dist_impact_k,
      const uint32_t &current_tile_index, 
      std::vector<std::pair<std::mutex, float>> &elevations
    ) {
      const auto &tile_data = render::unpack_data(context->map->get_tile(current_tile_index));
      const glm::vec4 tile_pos  = context->map->get_point(tile_data.center);
      
      // на сфере по идее неверная дальность
      const float dist = glm::distance(tile_pos, prev_pos);
      const float next_dist = accumulated_dist + dist;
      const float final_dist = next_dist / dist_impact_k;
      //const float max_dist_impact = current_length * dist_impact_k;
      const float dist_k = std::min(final_dist, current_length) / current_length; // возможно должен быть квадратичным
      const float final_dist_k = 1.0f - dist_k;
      
      const uint32_t plate_index = context->tile_plate_indices[current_tile_index];
      const auto &plate_data = context->plate_datas[plate_index];
      
      const float next_length = current_length - dist / dist_impact_k;
      auto& elevation = elevations[current_tile_index];
      
      float elevation_k = 0;
      {
        std::unique_lock<std::mutex> lock(elevation.first);
        const float max_elevation_impact = MAX_ELEVATION_IMPACT;
        const float sub = std::min(std::abs(elevation.second - plate_data.base_elevation), max_elevation_impact);
        elevation_k = 1.0f - sub / max_elevation_impact;
        
        const float sum_k_mul = 0.40f;
        const float sum_k = final_dist_k * sum_k_mul + elevation_k * (1.0f - sum_k_mul);
        
        const float l_modify = max_elevation_impact * (1 / next_length);
        const float final_k = glm::mix(0.0f, l_modify, sum_k);
        const float coef = current_length * final_k;
        elevation.second = elevation.second - coef;
      }
      
//       const float next_length = current_length - dist / dist_impact_k;
//       if (next_length < EPSILON) return;
      if (current_length < final_dist) return;
      
      for (uint32_t i = 0; i < 6; ++i) {
        const uint32_t n_index = tile_data.neighbors[i];
        if (n_index == UINT32_MAX) continue;
        
        const uint32_t n_plate_index = context->tile_plate_indices[n_index];
        if (plate_index != n_plate_index) continue;
        
        plate_subducting_req(context, edge, tile_pos, current_length, next_dist, dist_impact_k, n_index, elevations);
      }
    }
    
    // усреднить?
    void plate_friction_req(
      const generator_context* context, 
      const glm::vec4 &prev_pos, 
      const float &current_length, 
      const float &accumulated_dist,
      const float &dist_impact_k,
      const uint32_t &current_tile_index, 
      std::vector<std::pair<std::mutex, float>> &elevations
    ) {
      const auto &tile_data = render::unpack_data(context->map->get_tile(current_tile_index));
      const glm::vec4 tile_pos  = context->map->get_point(tile_data.center);
      
      // на сфере по идее неверная дальность
      const float dist = glm::distance(tile_pos, prev_pos);
      const float next_dist = accumulated_dist + dist;
      const float final_dist = next_dist / dist_impact_k;
      //const float max_dist_impact = current_length * dist_impact_k;
//       const float dist_k = std::min(final_dist, current_length) / current_length; // возможно должен быть квадратичным
//       const float final_dist_k = 1.0f - dist_k * dist_k;
      
//       const uint32_t plate_index = context->tile_plate_indices[current_tile_index];
//       const auto &plate_data = context->plate_datas[plate_index];
      
//       const float next_length = current_length - dist / dist_impact_k;
//       if (next_length < EPSILON) return;
      if (current_length < final_dist) return;
      
      auto &elevation = elevations[current_tile_index];
      float old_elevation = 0.0f;
      {
        std::unique_lock<std::mutex> lock(elevation.first);
        old_elevation = elevation.second;
      }
      
      float accumulated_elevation = 0.0f;
      const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
      for (uint32_t i = 0; i < n_count; ++i) {
        const uint32_t n_index = tile_data.neighbors[i];
        
        auto &elevation = elevations[n_index];
        {
          std::unique_lock<std::mutex> lock(elevation.first);
          accumulated_elevation += elevation.second;
        }
      }
      
      const float sumd_elev_k = 0.6f;
      const float final_elev = old_elevation * sumd_elev_k + accumulated_elevation * (1.0f - sumd_elev_k);
      {
        std::unique_lock<std::mutex> lock(elevation.first);
        elevation.second = final_elev;
      }
      
      for (uint32_t i = 0; i < n_count; ++i) {
        const uint32_t n_index = tile_data.neighbors[i];
        plate_friction_req(context, tile_pos, current_length, next_dist, dist_impact_k, n_index, elevations);
      }
    }
    
    modify_plate_datas::modify_plate_datas(const create_info &info) : pool(info.pool) {}
    void modify_plate_datas::process(generator_context* context) {
      utils::time_log log("modification plate datas");
      
      struct next_plates_data {
        std::mutex mutex;
        std::unordered_set<uint32_t> neighbours;
      };
      
//       std::vector<next_plates_data> next_plates(context->plate_tile_indices.size());
      std::vector<std::pair<std::mutex, float>> elevations(context->map->tiles_count()); // &next_plates
      utils::submit_works(pool, context->map->tiles_count(), [&elevations] (const size_t &start, const size_t &count, const generator_context* context) {
        for (size_t i = start; i < start+count; ++i) {
          const uint32_t plate = context->tile_plate_indices[i];
          const auto &plate_data = context->plate_datas[plate];
          elevations[i].second = plate_data.base_elevation;
          
//           const auto &tile = render::unpack_data(context->map->get_tile(i));
//           for (uint32_t j = 0; j < 6; ++j) {
//             const uint32_t tile_neighbour_index = tile.neighbours[j];
//             if (tile_neighbour_index == UINT32_MAX) continue;
//             
//             const uint32_t plate1 = context->tile_plate_indices[i];
//             const uint32_t plate2 = context->tile_plate_indices[tile_neighbour_index];
//             
// //             if (plate1 != plate2) {
// //               {
// //                 std::unique_lock<std::mutex> lock(next_plates[plate1].mutex);
// //                 next_plates[plate1].neighbours.insert(plate2);
// //               }
// //               
// //               {
// //                 std::unique_lock<std::mutex> lock(next_plates[plate2].mutex);
// //                 next_plates[plate2].neighbours.insert(plate1);
// //               }
// //             }
//           }
        }
      }, context);
      
//       for (size_t i = 0; i < elevations.size(); ++i) elevations[i].second = 0.0f;
      utils::submit_works(pool, context->boundary_stresses.size(), [&elevations] (const size_t &start, const size_t &count, const generator_context* context) {
        for (size_t i = start; i < start+count; ++i) {
          // нужно вычилить вектора скорости 
          
          // нужно по границам вычислить некий вектор
          // и начиная от границы "пропушить" вектор, оставляя какой то вклад каждому из тайлов
          // сумма этих вкладов должна представлять собой высоту
          // выше одна плита по отношению к другой - меньше вклад
          
          const auto &stress = context->boundary_stresses[i];
          const auto &edge = context->boundary_edges[i];
          
          const uint32_t first_tile  = edge.first;
          const uint32_t second_tile = edge.second;
          const auto &first_tile_data = context->map->get_tile(first_tile);
          const auto &second_tile_data = context->map->get_tile(second_tile);
          const glm::vec4 first_tile_pos  = context->map->get_point(first_tile_data.tile_indices[0]);
          const glm::vec4 second_tile_pos = context->map->get_point(second_tile_data.tile_indices[0]);
          
          const auto &plate_movement0 = stress.pressure_vector;
          const auto &plate_movement1 = stress.shear_vector;
          
          const float length0 = glm::length(plate_movement0);
          const float length1 = glm::length(plate_movement1);
          
          const auto dir0 = length0 < EPSILON ? glm::vec4(0.0f, 0.0f, 0.0f, 0.0f) : plate_movement0 / length0;
          const auto dir1 = length1 < EPSILON ? glm::vec4(0.0f, 0.0f, 0.0f, 0.0f) : plate_movement1 / length1;
          
          const glm::vec4 boundary_position = (second_tile_pos + first_tile_pos) / 2.0f;
          const glm::vec4 boundary_normal = glm::normalize(second_tile_pos - first_tile_pos);
          
          // вот у нас есть вектора движения плит
          // несколько ситуаций: разнонаправленные вектора - провал (?)
          // сонаправленные вектора - колизия (или уход одной плиты под другую)
          // в зависимости от того куда направлены по сравнению с эджем может быть еще трение
          
          const float dot0 = glm::dot(dir0,  boundary_normal);
          const float dot1 = glm::dot(dir1, -boundary_normal);
          
          const uint32_t plate_index = context->tile_plate_indices[first_tile];
          const uint32_t opposing_plate_index = context->tile_plate_indices[second_tile];
          
          const auto &plate_data = context->plate_datas[plate_index];
          const auto &opposing_plate_data = context->plate_datas[opposing_plate_index];
          
          if (dot0 > 0.3f && dot1 > 0.3f) {
            // одна плита давит на другую
            
            const float elevation0 = plate_data.base_elevation;
            const float elevation1 = opposing_plate_data.base_elevation;
            const float force = glm::length(plate_movement0 - plate_movement1);
            
            if (plate_data.oceanic == opposing_plate_data.oceanic) { // коллизия 
              //const float elevation = elevation0 + elevation1;
              // на этом месте образуется гора и она должна распространится на соседние тайлы
              // к elevation0 и elevation1 нужно прибавить какие то коэффициенты 
              // (не очень большие, так как я сюда скорее всего попаду из других границ)
              // у нас тут есть по крайней мере 3 переменных: 
              // сила воздействия (берем ее из plate_movement)
              // дальность до воздействия
              // коэффициент высоты
              
              // пусть сила воздействия это length
              // формула должна быть примерно такая current_elevation + length * f(length, dist) * g(current_elevation - base_elevation ?)
              // при высоком текущем подъеме мы должны скорее прибавлять окружающим тайлам вклад
              // чем текущему тайлу
              // таким образом мы должны пройти ближайшие тайлы у плиты
              
              // вообще при сильной разнице base_elevation одна плита должна заходить под другую
              // и в этом случае увеличение/уменьшение высот должно быть более равномерным и дальше распространятся
              if (elevation0 - elevation1 > 0.25f) {
                if (elevation0 > elevation1) {
                  plate_superducting_req(context, edge, boundary_position, force, 0.0f, 3.0f, first_tile, elevations);
                  plate_subducting_req(context, edge, boundary_position, force, 0.0f, 3.0f, second_tile, elevations);
                } else {
                  plate_superducting_req(context, edge, boundary_position, force, 0.0f, 3.0f, second_tile, elevations);
                  plate_subducting_req(context, edge, boundary_position, force, 0.0f, 3.0f, first_tile, elevations);
                }
              } else {
                plate_superducting_req(context, edge, boundary_position, force, 0.0f, 2.0f, first_tile, elevations);
                plate_superducting_req(context, edge, boundary_position, force, 0.0f, 2.0f, second_tile, elevations);
              }
            } else if (plate_data.oceanic) {
              // один из тайлов идет вниз
              // у другого спавнится крутой берег
              plate_superducting_req(context, edge, boundary_position, force, 0.0f, 2.0f, first_tile, elevations);
              plate_subducting_req(context, edge, boundary_position, force, 0.0f, 2.0f, second_tile, elevations);
            } else {
              plate_superducting_req(context, edge, boundary_position, force, 0.0f, 2.0f, second_tile, elevations);
              plate_subducting_req(context, edge, boundary_position, force, 0.0f, 2.0f, first_tile, elevations);
            }
          } else if (dot0 < -0.3f && dot1 < -0.3f) {
            // одна плита отходит от другой
            // тут по идее у всех тайлов должен быть -подъем
            const float force = glm::length(plate_movement0 + plate_movement1);
            plate_subducting_req(context, edge, boundary_position, force, 0.0f, 3.0f, first_tile, elevations);
            plate_subducting_req(context, edge, boundary_position, force, 0.0f, 3.0f, second_tile, elevations);
          } else {
            // плиты трутся друг о друга (землетрясения? )
            // по идее усредняем значение высот
            const float force = glm::length(glm::abs(plate_movement0) + glm::abs(plate_movement1));
            plate_friction_req(context, boundary_position, force, 0.0f, 3.0f, first_tile, elevations);
            plate_friction_req(context, boundary_position, force, 0.0f, 3.0f, second_tile, elevations);
          }
        }
      }, context);
      
      context->tile_elevation.resize(context->map->tiles_count());
      for (size_t i = 0; i < context->map->tiles_count(); ++i) {
        context->tile_elevation[i] = elevations[i].second;
        context->map->set_tile_height(i, context->tile_elevation[i]);
      }
    }
    
    size_t modify_plate_datas::progress() const {
      return state;
    }
    
    size_t modify_plate_datas::complete_state(const generator_context* context) const {
      return context->plate_tile_indices.size();
    }
    
    std::string modify_plate_datas::hint() const {
      return "recalculating plate datas";
    }
    
    float compute_elevation(
      const float &plate_elevation, 
      const float &opposing_plate_elevation, 
      const boundary_stress_t &stress, 
      float t, 
      const uint32_t &plate_index, 
      const uint32_t &opposing_plate_index, 
      const std::pair<uint32_t, uint32_t> &edge,
      const uint32_t &tile_index,
      const generator_context* context
    ) {
//       const auto &plate_movement0 = stress.pressure_vector;
//       const auto &plate_movement1 = stress.shear_vector;
//       
//       const float length0 = glm::length(plate_movement0);
//       const float length1 = glm::length(plate_movement1);
//       
//       const auto dir0 = length0 < EPSILON ? glm::vec4(0.0f, 0.0f, 0.0f, 0.0f) : plate_movement0 / length0;
//       const auto dir1 = length1 < EPSILON ? glm::vec4(0.0f, 0.0f, 0.0f, 0.0f) : plate_movement1 / length1;
//       
//       const uint32_t first_tile  = edge.first;
//       const uint32_t second_tile = edge.second;
//       const auto &first_tile_data = context->map->get_tile(first_tile);
//       const auto &second_tile_data = context->map->get_tile(second_tile);
//       const glm::vec4 first_tile_pos  = context->map->get_point(first_tile_data.tile_indices[0]);
//       const glm::vec4 second_tile_pos = context->map->get_point(second_tile_data.tile_indices[0]);
//       
//       const glm::vec4 boundary_position = (second_tile_pos + first_tile_pos) / 2.0f;
//       const glm::vec4 boundary_normal = glm::normalize(second_tile_pos - first_tile_pos);
//       
//       const auto &tile_data = context->map->get_tile(tile_index);
//       const glm::vec4 tile_pos  = context->map->get_point(tile_data.tile_indices[0]);
//       const float dist = glm::distance(tile_pos, boundary_position);
//       
//       const float dot0 = glm::dot(dir0,  boundary_normal);
//       const float dot1 = glm::dot(dir1, -boundary_normal);
//       
//       const auto &plate_data = context->plate_datas[plate_index];
//       const auto &opposing_plate_data = context->plate_datas[opposing_plate_index];
//       
//       if (dot0 > 0.3f && dot1 > 0.3f) {
//         одна плита давит на другую
//         
//         const float elevation0 = plate_data.base_elevation;
//         const float elevation1 = opposing_plate_data.base_elevation;
//         const float force = glm::length(plate_movement0 - plate_movement1) * 50.0f;
//         const float force_k = 1.0f - std::min(force, dist) / force;
//         const float final_force = glm::mix(0.0f, 0.4f, force_k);
//         
//         ASSERT(false);
//         
//         const float boundary_elevation = std::max(plate_elevation, opposing_plate_elevation) + final_force;
//         
//         if (plate_data.oceanic == opposing_plate_data.oceanic) {
//           if (elevation0 - elevation1 > 0.25f) {
//             if (elevation0 > elevation1) {
//               
//             } else {
//               
//             }
//           } 
//           return plate_elevation + force_k * (boundary_elevation - plate_elevation);
//         } else if (plate_data.oceanic) {
//           return plate_elevation + force_k * force_k * (boundary_elevation - plate_elevation) / 2.0f;
//         } else {
//           return plate_elevation + force_k * force_k * (boundary_elevation - plate_elevation);
//         }
//       } else if (dot0 < -0.3f && dot1 < -0.3f) {
//         
//       } else {
//         
//       }
      
      if (stress.pressure > 0.3f) {
        const float boundary_elevation = std::max(plate_elevation, opposing_plate_elevation) + stress.pressure;
        const bool plate_oceanic = context->plate_datas[plate_index].oceanic;
        const bool opposing_plate_oceanic = context->plate_datas[opposing_plate_index].oceanic;
        if (plate_oceanic == opposing_plate_oceanic) {
          // colliding
          if (t < 0.5f) {
            t = t / 0.5f - 1.0f;
            return plate_elevation + t*t * (boundary_elevation - plate_elevation);
          }
        } else if (plate_oceanic) {
          // subducting
          t = t - 1;
          return plate_elevation + t*t * (boundary_elevation - plate_elevation);
        } else {
          // superducting
          if (t < 0.2f) {
            t = t / 0.2f;
            return boundary_elevation + t * (plate_elevation - boundary_elevation + stress.pressure / 2);
          } else if (t < 0.5f) {
            t = (t - 0.2f) / 0.3f - 1.0f;
            return plate_elevation + t * t * stress.pressure / 2;
          }
        }
        
      } else if (stress.pressure < -0.06f) {
        //ASSERT(false);
        
        // diverging
        const float boundary_elevation = std::max(plate_elevation, opposing_plate_elevation) - stress.pressure / 4.0f;
        if (t < 0.3f) {
          t = t / 0.3f - 1.0f;
          return plate_elevation + t * t * (boundary_elevation - plate_elevation);
        }
      } else if (stress.shear > 0.06f) {
        // shearing
        const float boundary_elevation = std::max(plate_elevation, opposing_plate_elevation) + stress.shear / 8.0f;
        if (t < 0.2f) {
          t = t / 0.2f - 1.0f;
          return plate_elevation + t * t * (boundary_elevation - plate_elevation);
        }
      } else {
        // dormant
        const float boundary_elevation = (plate_elevation + opposing_plate_elevation) / 2.0f;
        const float elevation_difference = boundary_elevation - plate_elevation;
        return t * t * elevation_difference * (2 * t - 3) + boundary_elevation;
      }
      
      return plate_elevation;
    }
    
    calculate_vertex_elevation::calculate_vertex_elevation(const create_info &info) : pool(info.pool), state(0), noise_multiplier(info.noise_multiplier) {
      ASSERT(noise_multiplier >= 0.0f && noise_multiplier <= 1.0f);
    }
    
    void calculate_vertex_elevation::process(generator_context* context) {
      utils::time_log log("calculate tiles evaliation");
      std::vector<float> tile_elevation(context->map->tiles_count());
      
      // здесь херится моя математика, так как меня иной масштаб
      
      // я подумал, может быть стоит подойти иначе с генерации подъемов
      // здесь мы криво учитываем вклад всей границы соприкосновения плит
      // у плиты должна быть некая нормаль, которая показывает как плита накренена по отношению к другой
      // мы должны чутка модифицировать крен + понять насколько граничные тайлы поднялись по отношению 
      // к предыдущим значениям
      
      state = 0;
      const float noise_multiplier_local = noise_multiplier;
      utils::submit_works(pool, context->map->tiles_count(), [&tile_elevation, noise_multiplier_local] (const size_t &start, const size_t &count, const generator_context* context, std::atomic<size_t> &state) {
        for (size_t i = start; i < start+count; ++i) {
          float accum_elevation = 0.0f;
          uint32_t elevations_count = 0;
          const uint32_t plate_index = context->tile_plate_indices[i];
          const uint32_t tile_index = i;
          const auto &tile_data = context->map->get_tile(tile_index);
          const glm::vec4 tile_point = context->map->get_point(tile_data.tile_indices.x);
//           
//           const auto &pair = context->edge_index_dist[i];
//           const uint32_t main_boundary_index = pair.first;
//           const auto &boundary_pair = context->boundary_edges[main_boundary_index];
//           
//           const uint32_t plate0 = context->tile_plate_indices[boundary_pair.first];
//           const uint32_t plate1 = context->tile_plate_indices[boundary_pair.second];
// //           if (plate_index != plate0 && plate_index != plate1) continue;
//                         
//           const uint32_t opposing_plate_index = plate_index == plate0 ? plate1 : plate0;
//           const uint32_t boundary_tile_index0 = plate_index == plate0 ? boundary_pair.first : boundary_pair.second;
// //             const uint32_t boundary_tile_index1 = plate_index == plate0 ? boundary_pair.second : boundary_pair.first;
//           
//           ASSERT(plate_index != opposing_plate_index);
//           ASSERT(i < context->edge_index_dist.size());
//           ASSERT(main_boundary_index < context->boundary_edges.size());
//           ASSERT(plate_index < context->plate_tile_indices.size());
//           
//           const auto &first_tile_data = context->map->get_tile(boundary_pair.first);
//           const auto &second_tile_data = context->map->get_tile(boundary_pair.second);
//           const glm::vec4 first_tile_pos  = context->map->get_point(first_tile_data.tile_indices[0]);
//           const glm::vec4 second_tile_pos = context->map->get_point(second_tile_data.tile_indices[0]);
//           
//           const auto &tile_data = context->map->get_tile(i);
//           const glm::vec4 boundary_position = (second_tile_pos + first_tile_pos) / 2.0f;
//           const float main_boundary_dist = glm::distance(boundary_position, context->map->get_point(tile_data.tile_indices.x));
//           
// //           if (main_boundary_dist > context->root_distances[boundary_tile_index0]) continue;
//           
          const float plate_elevation = context->plate_datas[plate_index].base_elevation;
//           const float opposing_plate_elevation = context->plate_datas[opposing_plate_index].base_elevation;
//           
//           ASSERT(context->root_distances[i] < 1000.0f);
//           
// //           float t = pair.second / (pair.second + context->root_distances[i]);
//           float t = main_boundary_dist / (main_boundary_dist + context->root_distances[i]);
//           
//           ASSERT(t >= 0.0f && t <= 1.0f);
//           
//           const auto &stress = context->boundary_stresses[main_boundary_index];
//           
//           ASSERT(stress.pressure >= -1.0f && stress.pressure <= 1.0f);
//           ASSERT(stress.shear >= -1.0f && stress.shear <= 1.0f);
//           
//           accum_elevation += compute_elevation(plate_elevation, opposing_plate_elevation, stress, t, plate_index, opposing_plate_index, boundary_pair, i, context);
          ++elevations_count;
          
          const auto &a_pair = context->mountain_dist[i];
          const auto &b_pair = context->ocean_dist[i];
          const auto &c_pair = context->coastline_dist[i];
          
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
              const auto &boundary_pair = context->boundary_edges[edge_index];
              const uint32_t plate0 = context->tile_plate_indices[boundary_pair.first];
              const uint32_t plate1 = context->tile_plate_indices[boundary_pair.second];
              const uint32_t opposing_plate_index = plate_index == plate0 ? plate1 : plate0;
              
              const float opposing_plate_elevation = context->plate_datas[opposing_plate_index].base_elevation;
              const float boundary_elevation = std::max(opposing_plate_elevation, plate_elevation);
              
              // к boundary_elevation нужно прибавить какую то силу
              // максимум который мы можем прибавить это 0.5f
              // как его расчитать, мы можем взять доты как на предыдущих шагах
              // но доты нужно как то компенсировать силой 
              
              const auto &boundary_data = context->boundary_stresses[edge_index];
          
              // необходимо какое-то ограничение или коэффициент
              // иначе получается плохо
              // у нас задача: раскидать границы по эффектам столкновения и расхождения плит
              // в зависимости от типа взаимодействия на границе мы получаем разные типы поверхностей
              // 
              
              const auto &plate_movement0 = boundary_data.pressure_vector;
              const auto &plate_movement1 = boundary_data.shear_vector;
              
              const float length0 = glm::length(plate_movement0);
              const float length1 = glm::length(plate_movement1);
              
              const auto dir0 = length0 < EPSILON ? glm::vec4(0.0f, 0.0f, 0.0f, 0.0f) : plate_movement0 / length0;
              const auto dir1 = length1 < EPSILON ? glm::vec4(0.0f, 0.0f, 0.0f, 0.0f) : plate_movement1 / length1;
              
              const uint32_t first_tile  = boundary_pair.first;
              const uint32_t second_tile = boundary_pair.second;
              const auto &first_tile_data = context->map->get_tile(first_tile);
              const auto &second_tile_data = context->map->get_tile(second_tile);
              const glm::vec4 first_tile_pos  = context->map->get_point(first_tile_data.tile_indices[0]);
              const glm::vec4 second_tile_pos = context->map->get_point(second_tile_data.tile_indices[0]);
              
    //           const glm::vec4 boundary_position = (second_tile_pos + first_tile_pos) / 2.0f;
              const glm::vec4 boundary_normal = glm::normalize(second_tile_pos - first_tile_pos);
              
              const float dot0 = glm::dot(dir0,  boundary_normal);
              const float dot1 = glm::dot(dir1, -boundary_normal);
              
              const float dot_k = (dot0 + dot1) / 2.0f;
              const float final_k = boundary_elevation < 0.0f ? std::max(-boundary_elevation * (1.0f + dot_k) * 0.8f, -boundary_elevation) : glm::mix(-0.2f, 0.5f, dot_k);
              
              a_k = boundary_elevation + final_k;
              
              ASSERT(a_k >= 0.0f && a_k <= 1.0f);
            }
            
            // наоборот
            if (b_pair.first != UINT32_MAX) {
              const uint32_t edge_index = b_pair.first;
              const auto &boundary_pair = context->boundary_edges[edge_index];
              const uint32_t plate0 = context->tile_plate_indices[boundary_pair.first];
              const uint32_t plate1 = context->tile_plate_indices[boundary_pair.second];
              const uint32_t opposing_plate_index = plate_index == plate0 ? plate1 : plate0;
              
              const float opposing_plate_elevation = context->plate_datas[opposing_plate_index].base_elevation;
              const float boundary_elevation = std::min(opposing_plate_elevation, plate_elevation);
              
              // к boundary_elevation нужно прибавить какую то силу
              // максимум который мы можем прибавить это 0.5f
              // как его расчитать, мы можем взять доты как на предыдущих шагах
              // но доты нужно как то компенсировать силой 
              
              const auto &boundary_data = context->boundary_stresses[edge_index];
          
              // необходимо какое-то ограничение или коэффициент
              // иначе получается плохо
              // у нас задача: раскидать границы по эффектам столкновения и расхождения плит
              // в зависимости от типа взаимодействия на границе мы получаем разные типы поверхностей
              // 
              
              const auto &plate_movement0 = boundary_data.pressure_vector;
              const auto &plate_movement1 = boundary_data.shear_vector;
              
              const float length0 = glm::length(plate_movement0);
              const float length1 = glm::length(plate_movement1);
              
              const auto dir0 = length0 < EPSILON ? glm::vec4(0.0f, 0.0f, 0.0f, 0.0f) : plate_movement0 / length0;
              const auto dir1 = length1 < EPSILON ? glm::vec4(0.0f, 0.0f, 0.0f, 0.0f) : plate_movement1 / length1;
              
              const uint32_t first_tile  = boundary_pair.first;
              const uint32_t second_tile = boundary_pair.second;
              const auto &first_tile_data = context->map->get_tile(first_tile);
              const auto &second_tile_data = context->map->get_tile(second_tile);
              const glm::vec4 first_tile_pos  = context->map->get_point(first_tile_data.tile_indices[0]);
              const glm::vec4 second_tile_pos = context->map->get_point(second_tile_data.tile_indices[0]);
              
    //           const glm::vec4 boundary_position = (second_tile_pos + first_tile_pos) / 2.0f;
              const glm::vec4 boundary_normal = glm::normalize(second_tile_pos - first_tile_pos);
              
              const float dot0 = glm::dot(dir0,  boundary_normal);
              const float dot1 = glm::dot(dir1, -boundary_normal);
              
              const float dot_k = 1.0f - glm::abs(dot0 + dot1) / 2.0f;
              const float final_k = boundary_elevation > 0.0f ?  std::min(-boundary_elevation * (1.0f + dot_k) * 0.8f, -boundary_elevation) : glm::mix(-0.2f, 0.05f, dot_k);
              
              b_k = boundary_elevation + final_k;
              
              ASSERT(b_k >= -1.0f && b_k <= 0.0f);
            }
          }
          
          // тут нужно учесть текущий подъем везде
          if (a_pair.first == UINT32_MAX && b_pair.first == UINT32_MAX) {
//             const uint32_t edge_index = c_pair.first;
// //             
// //             const auto &boundary_pair = context->boundary_edges[edge_index];
// //             const uint32_t plate0 = context->tile_plate_indices[boundary_pair.first];
// //             const uint32_t plate1 = context->tile_plate_indices[boundary_pair.second];
// //             const uint32_t opposing_plate_index = plate_index == plate0 ? plate1 : plate0;
// //             
// //             const float opposing_plate_elevation = context->plate_datas[opposing_plate_index].base_elevation;
// //             
// //             const float norm_dist = 1.0f/std::max(c, 1.0f);
// //             const float boundary_elevation = (opposing_plate_elevation + plate_elevation) / 2.0f;
// //             const float elev = boundary_elevation - plate_elevation;
// //             accum_elevation = plate_elevation + std::sqrt(norm_dist) * elev;
//             
// //             const uint32_t edge_index = b_pair.first;
//             const auto &boundary_pair = context->boundary_edges[edge_index];
//             const uint32_t plate0 = context->tile_plate_indices[boundary_pair.first];
//             const uint32_t plate1 = context->tile_plate_indices[boundary_pair.second];
//             const uint32_t opposing_plate_index = plate_index == plate0 ? plate1 : plate0;
//             
//             const float opposing_plate_elevation = context->plate_datas[opposing_plate_index].base_elevation;
//             
//             const auto &boundary_data = context->boundary_stresses[edge_index];
//             
//             const auto &plate_movement0 = boundary_data.pressure_vector;
//             const auto &plate_movement1 = boundary_data.shear_vector;
//             
//             const float length0 = glm::length(plate_movement0);
//             const float length1 = glm::length(plate_movement1);
//             
//             const auto dir0 = length0 < EPSILON ? glm::vec4(0.0f, 0.0f, 0.0f, 0.0f) : plate_movement0 / length0;
//             const auto dir1 = length1 < EPSILON ? glm::vec4(0.0f, 0.0f, 0.0f, 0.0f) : plate_movement1 / length1;
//             
//             const uint32_t first_tile  = boundary_pair.first;
//             const uint32_t second_tile = boundary_pair.second;
//             const auto &first_tile_data = context->map->get_tile(first_tile);
//             const auto &second_tile_data = context->map->get_tile(second_tile);
//             const glm::vec4 first_tile_pos  = context->map->get_point(first_tile_data.tile_indices[0]);
//             const glm::vec4 second_tile_pos = context->map->get_point(second_tile_data.tile_indices[0]);
//             
//   //           const glm::vec4 boundary_position = (second_tile_pos + first_tile_pos) / 2.0f;
//             const glm::vec4 boundary_normal = glm::normalize(second_tile_pos - first_tile_pos);
//             
//             const float dot0 = glm::dot(dir0,  boundary_normal);
//             const float dot1 = glm::dot(dir1, -boundary_normal);
//             
//             const uint32_t b_dist = context->test_boundary_dist[tile_index].second;
//             const uint32_t m_dist = context->test_max_dist[tile_index];
//             
//             if (dot0 > 0.3f && dot1 > 0.3f) {
//               // колизия океанов или земли
//               // нужно понять насколько далеко эта граница
//               // значение необходимо нормализовать, вообще это можно сделать
//               // найдя ближайшую точку с максимальной дальностью
//               const float boundary_elevation = std::max(opposing_plate_elevation, plate_elevation);
//               const float dot_average = glm::abs(dot0 + dot1) / 2.0f;
//               
//               float t = float(b_dist) / float(b_dist + m_dist);
//               
//               accum_elevation = plate_elevation + 5.0f;
//             } else if (dot0 < -0.3f && dot1 < -0.3f) {
//               // океаны или земля расходятся
//               const float boundary_elevation = std::min(opposing_plate_elevation, plate_elevation);
//               const float dot_average = glm::abs(dot0 + dot1) / 2.0f;
//               
//               float t = float(b_dist) / float(b_dist + m_dist);
//               
//               //accum_elevation = plate_elevation + t*t * ((plate_elevation - boundary_elevation) - glm::mix(-0.1f, 0.1f, 1.0f - dot_average));
//               accum_elevation = plate_elevation;
//             } else {
//               // трение
//               const float boundary_elevation = (opposing_plate_elevation + plate_elevation) / 2.0f;
//               accum_elevation = boundary_elevation;
//             }
            
            //accum_elevation = plate_elevation;
            //accum_elevation = 0.0f;
            const uint32_t edge_index = c_pair.first;
            const auto &boundary_pair = context->boundary_edges[edge_index];
            const uint32_t plate0 = context->tile_plate_indices[boundary_pair.first];
            const uint32_t plate1 = context->tile_plate_indices[boundary_pair.second];
            const uint32_t opposing_plate_index = plate_index == plate0 ? plate1 : plate0;
            const float opposing_plate_elevation = context->plate_datas[opposing_plate_index].base_elevation;
            accum_elevation = (opposing_plate_elevation + plate_elevation) / 2.0f;
          } else {
            accum_elevation = (a_k/a + b_k/b) / (1.0f/a + 1.0f/b + 1.0f/c);
          }
          
          // при каких интересно значениях будет больше 1
          accum_elevation += noise_multiplier_local * context->data->noise->GetNoise(tile_point.x, tile_point.y, tile_point.z);
          //accum_elevation = glm::clamp(accum_elevation, -1.0f, 1.0f);
          
          ASSERT(accum_elevation == accum_elevation);
          ASSERT(accum_elevation >= -1.0f && accum_elevation <= 1.0f);
          
//           const auto &pair = context->edge_index_dist[i];
//           const auto &boundary_pair = context->boundary_edges[pair.first];
//           const uint32_t plate0 = context->tile_plate_indices[boundary_pair.first];
//           const uint32_t plate1 = context->tile_plate_indices[boundary_pair.second];
//           const uint32_t opposing_plate_index = plate_index == plate0 ? plate1 : plate0;
//           
//           const float opposing_plate_elevation = context->plate_datas[opposing_plate_index].base_elevation;
//           
//           const auto &boundary_data = context->boundary_stresses[pair.first];
//           
//           const auto &plate_movement0 = boundary_data.pressure_vector;
//           const auto &plate_movement1 = boundary_data.shear_vector;
//           
//           const float length0 = glm::length(plate_movement0);
//           const float length1 = glm::length(plate_movement1);
//           
//           const auto dir0 = length0 < EPSILON ? glm::vec4(0.0f, 0.0f, 0.0f, 0.0f) : plate_movement0 / length0;
//           const auto dir1 = length1 < EPSILON ? glm::vec4(0.0f, 0.0f, 0.0f, 0.0f) : plate_movement1 / length1;
//           
//           const uint32_t first_tile  = boundary_pair.first;
//           const uint32_t second_tile = boundary_pair.second;
//           const auto &first_tile_data = context->map->get_tile(first_tile);
//           const auto &second_tile_data = context->map->get_tile(second_tile);
//           const glm::vec4 first_tile_pos  = context->map->get_point(first_tile_data.tile_indices[0]);
//           const glm::vec4 second_tile_pos = context->map->get_point(second_tile_data.tile_indices[0]);
//           
// //           const glm::vec4 boundary_position = (second_tile_pos + first_tile_pos) / 2.0f;
//           const glm::vec4 boundary_normal = glm::normalize(second_tile_pos - first_tile_pos);
//           
//           const float dot0 = glm::dot(dir0,  boundary_normal);
//           const float dot1 = glm::dot(dir1, -boundary_normal);
//           
//           const uint32_t b_dist = context->test_boundary_dist[tile_index].second;
//           const uint32_t m_dist = context->test_max_dist[tile_index];
//           
//           ASSERT(m_dist != UINT32_MAX);
//           ASSERT(b_dist != UINT32_MAX);
//           
//           if (dot0 > 0.3f && dot1 > 0.3f) {
//             // колизия океанов или земли
//             // нужно понять насколько далеко эта граница
//             // значение необходимо нормализовать, вообще это можно сделать
//             // найдя ближайшую точку с максимальной дальностью
//             const float dot_average = glm::abs(dot0 + dot1) / 2.0f;
//             const float boundary_elevation = std::max(opposing_plate_elevation, plate_elevation) + glm::mix(-0.0f, 0.5f, dot_average);
//             
//             float t = float(b_dist) / float(b_dist + m_dist);
//             ASSERT(t >= 0.0f && t <= 1.0f);
//             
//             accum_elevation = plate_elevation + t * ((boundary_elevation - plate_elevation));
//             //accum_elevation = plate_elevation + 5.0f;
//           } else if (dot0 < -0.3f && dot1 < -0.3f) {
//             // океаны или земля расходятся
//             const float boundary_elevation = std::min(opposing_plate_elevation, plate_elevation);
//             const float dot_average = glm::abs(dot0 + dot1) / 2.0f;
//             
//             float t = float(b_dist) / float(b_dist + m_dist);
//             
//             accum_elevation = plate_elevation + t * ((plate_elevation - boundary_elevation) - glm::mix(-0.0f, 0.3f, 1.0f - dot_average));
//             //accum_elevation = plate_elevation;
//           } else {
//             // трение
//             const float boundary_elevation = (opposing_plate_elevation + plate_elevation) / 2.0f;
//             accum_elevation = boundary_elevation;
//           }
          
//           ASSERT(false);
          
          // + шум
          
//           if (main_boundary_dist < context->root_distances[i]) {
//             for (size_t j = 0 ; j < context->boundary_edges.size(); ++j) {
//   //             const auto &pair = context->edge_index_dist[i];
//               //const uint32_t boundary_index = pair.first;
//               const uint32_t boundary_index = j;
//               if (main_boundary_index == boundary_index) continue;
//               const auto &boundary_pair = context->boundary_edges[boundary_index];
//               
//               const uint32_t plate0 = context->tile_plate_indices[boundary_pair.first];
//               const uint32_t plate1 = context->tile_plate_indices[boundary_pair.second];
//               if (plate_index != plate0 && plate_index != plate1) continue;
//                             
//               const uint32_t opposing_plate_index = plate_index == plate0 ? plate1 : plate0;
//   //             const uint32_t boundary_tile_index0 = plate_index == plate0 ? boundary_pair.first : boundary_pair.second;
//   //             const uint32_t boundary_tile_index1 = plate_index == plate0 ? boundary_pair.second : boundary_pair.first;
//               
//               ASSERT(plate_index != opposing_plate_index);
//               ASSERT(i < context->edge_index_dist.size());
//               ASSERT(boundary_index < context->boundary_edges.size());
//               ASSERT(plate_index < context->plate_tile_indices.size());
//               
//               const auto &first_tile_data = context->map->get_tile(boundary_pair.first);
//               const auto &second_tile_data = context->map->get_tile(boundary_pair.second);
//               const glm::vec4 first_tile_pos  = context->map->get_point(first_tile_data.tile_indices[0]);
//               const glm::vec4 second_tile_pos = context->map->get_point(second_tile_data.tile_indices[0]);
//               
//               const auto &tile_data = context->map->get_tile(i);
//               const glm::vec4 boundary_position = (second_tile_pos + first_tile_pos) / 2.0f;
//               const float boundary_dist = glm::distance(boundary_position, context->map->get_point(tile_data.tile_indices.x));
//               
//               //if (boundary_dist > context->root_distances[boundary_tile_index0]) continue;
//               if (boundary_dist * 2.0f > main_boundary_dist) continue;
//               
//               const float plate_elevation = context->plate_datas[plate_index].base_elevation;
//               const float opposing_plate_elevation = context->plate_datas[opposing_plate_index].base_elevation;
//               
//               ASSERT(context->root_distances[i] < 1000.0f);
//               
//     //           float t = pair.second / (pair.second + context->root_distances[i]);
//               float t = boundary_dist / (boundary_dist + context->root_distances[i]);
//               
//               ASSERT(t >= 0.0f && t <= 1.0f);
//               
//               const auto &stress = context->boundary_stresses[boundary_index];
//               
//               ASSERT(stress.pressure >= -1.0f && stress.pressure <= 1.0f);
//               ASSERT(stress.shear >= -1.0f && stress.shear <= 1.0f);
//               
//               accum_elevation += compute_elevation(plate_elevation, opposing_plate_elevation, stress, t, plate_index, opposing_plate_index, boundary_pair, i, context);
//               ++elevations_count;
//             }
//           }
          
          ASSERT(elevations_count != 0);
          
          tile_elevation[i] = accum_elevation / float(elevations_count);
          ++state;
        }
      }, context, std::ref(state));
      
      std::swap(context->tile_elevation, tile_elevation);
    }
    
    size_t calculate_vertex_elevation::progress() const {
      return state;
    }
    
    size_t calculate_vertex_elevation::complete_state(const generator_context* context) const {
      return context->map->tiles_count();
    }
    
    std::string calculate_vertex_elevation::hint() const {
      return "calculating tiles elevation";
    }
    
    blur_tile_elevation::blur_tile_elevation(const create_info &info) : pool(info.pool), iterations_count(info.iterations_count), old_new_ratio(info.old_new_ratio), water_ground_ratio(info.water_ground_ratio) {
      ASSERT(old_new_ratio >= 0.0f && old_new_ratio <= 1.0f);
      ASSERT(water_ground_ratio >= 0.0f && water_ground_ratio <= 2.0f);
    }
    
    void blur_tile_elevation::process(generator_context* context) {
      utils::time_log log("tile elevation bluring");
      
      state = 0;
      std::vector<float> new_elevations(context->tile_elevation);
      const float new_old_ratio_local = old_new_ratio;
      const float water_ground_ratio_local = water_ground_ratio;
      for (uint32_t i = 0; i < iterations_count; ++i) {
        utils::submit_works(pool, context->tile_elevation.size(), [&new_elevations, new_old_ratio_local, water_ground_ratio_local] (
          const size_t &start, 
          const size_t &count, 
          const generator_context* context, 
          std::atomic<size_t> &state
        ) {
          for (size_t i = start; i < start+count; ++i) {
            const uint32_t tile_index = i;
            const auto &tile_data = render::unpack_data(context->map->get_tile(tile_index));
            const float old_elevation = context->tile_elevation[tile_index];
  //           const uint32_t plate_index = context->tile_plate_indices[tile_index];
            
            float accum_elevation = 0.0f;
            float accum_water = 0.0f;
            float accum_ground = 0.0f;
            const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
            for (uint32_t j = 0; j < n_count; ++j) {
              const uint32_t n_index = tile_data.neighbors[j];
              if (n_index == UINT32_MAX) continue;
                            
  //             const uint32_t n_plate_index = context->tile_plate_indices[n_index];
              const float n_old_elevation = context->tile_elevation[n_index];
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
            
            ++state;
          }
        }, context, std::ref(state));
        
        std::swap(context->tile_elevation, new_elevations);
      }
      
      uint32_t water_counter = 0;
      for (size_t i = 0 ; i < context->tile_elevation.size(); ++i) {
        context->map->set_tile_height(i, context->tile_elevation[i]);
        if (context->tile_elevation[i] < 0.0f) ++water_counter;
      }
      
      PRINT_VAR("oceanic tiles after recompute", water_counter)
      PRINT_VAR("oceanic tiles k              ", float(water_counter) / float(context->map->tiles_count()))
    }
    
    size_t blur_tile_elevation::progress() const {
      return state;
    }
    
    size_t blur_tile_elevation::complete_state(const generator_context* context) const {
      
    }
    
    std::string blur_tile_elevation::hint() const {
      return "bluring tile elevation";
    }
    
    void update_maximum(std::atomic<int32_t> &mem, const int32_t &value) {
      int32_t prev_value = mem;
      while (prev_value < value && !mem.compare_exchange_weak(prev_value, value)) {}
    }
    
    void update_minimum(std::atomic<int32_t> &mem, const int32_t &value) {
      int32_t prev_value = mem;
      while (prev_value > value && !mem.compare_exchange_weak(prev_value, value)) {}
    }
    
    float mapper(const float &value, const float &smin, const float &smax, const float &dmin, const float &dmax) {
      return ((value - smin) / (smax - smin)) * (dmax - dmin) + dmin;
    }
    
    normalize_tile_elevation::normalize_tile_elevation(const create_info &info) : pool(info.pool) {}
    void normalize_tile_elevation::process(generator_context* context) {
      utils::time_log log("normalize tile elevation");
      state = 0;
      
      std::atomic<int32_t> min_height_mem( INT32_MAX);
      std::atomic<int32_t> max_height_mem(-INT32_MAX);
      
      utils::submit_works(pool, context->map->tiles_count(), [&min_height_mem, &max_height_mem] (const size_t &start, const size_t &count, const generator_context* context, std::atomic<size_t> &state) {
        for (size_t i = start; i < start+count; ++i) {
          const uint32_t tile_index = i;
          float tile_height = context->tile_elevation[tile_index];
          if (tile_height < 0.0f) continue;
          //tile_height = (tile_height + 1.0f) / 2.0f;
          //tile_height = mapper(tile_height, -1.0f, 1.0f, 0.0f, 1.0f);
//           ASSERT(tile_height >= 0.0f && tile_height <= 2.0f);
          update_maximum(max_height_mem, glm::floatBitsToInt(tile_height));
          update_minimum(min_height_mem, glm::floatBitsToInt(tile_height));
        }
      }, context, std::ref(state));
      
      std::vector<float> new_elevations(context->tile_elevation);
      utils::submit_works(pool, context->map->tiles_count(), [&min_height_mem, &max_height_mem, &new_elevations] (const size_t &start, const size_t &count, const generator_context* context, std::atomic<size_t> &state) {
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
          float tile_height = context->tile_elevation[tile_index];
          if (tile_height < 0.0f) continue;
          //tile_height = (tile_height + 1.0f) / 2.0f;
          //tile_height = tile_height + 1.0f;
//           ASSERT(tile_height >= 0.0f && tile_height <= 2.0f);
//           ASSERT(min_height >= 0.0f && min_height <= 2.0f); 
//           ASSERT(max_height >= 0.0f && max_height <= 2.0f);
//           tile_height = mapper(tile_height, -1.0f, 1.0f, 0.0f, 1.0f);
          float new_tile_height = (tile_height - min_height) / (max_height - min_height);
          //new_tile_height = (new_tile_height - 0.5f) * 2.0f;
//           new_tile_height = mapper(new_tile_height, 0.0f, 1.0f, -1.0f, 1.0f);
          //new_tile_height = new_tile_height - 0.5f;
          ASSERT(new_tile_height >= 0.0f && new_tile_height <= 1.0f);
          new_elevations[tile_index] = new_tile_height;
        }
      }, context, std::ref(state));
      
      std::swap(context->tile_elevation, new_elevations);
      
      for (size_t i = 0 ; i < context->tile_elevation.size(); ++i) {
        context->map->set_tile_height(i, context->tile_elevation[i]);
      }
    }
    
    size_t normalize_tile_elevation::progress() const {
      return state;
    }
    
    size_t normalize_tile_elevation::complete_state(const generator_context* context) const {
      return 0;
    }
    
    std::string normalize_tile_elevation::hint() const {
      return "normalizing tile elevation";
    }
    
    calculate_tile_distance::calculate_tile_distance(const create_info &info) : predicate(info.predicate), data_container(info.data_container), hint_str(info.hint_str) {}
    void calculate_tile_distance::process(generator_context* context) {
      utils::time_log log(hint_str);
      state = 0;
      
      std::vector<std::pair<uint32_t, uint32_t>> ground_distance(context->map->tiles_count(), std::make_pair(UINT32_MAX, UINT32_MAX));
      std::queue<uint32_t> queue;
      
      for (size_t i = 0; i < context->map->tiles_count(); ++i) {
        const uint32_t tile_index = i;
        if (predicate(context, tile_index)) {
          queue.push(tile_index);
          ground_distance[tile_index] = std::make_pair(tile_index, 0);
          ++state;
        }
      }
      
      while (!queue.empty()) {
        const uint32_t current_tile = queue.front();
        queue.pop();
        
        const auto &tile_data = render::unpack_data(context->map->get_tile(current_tile));
        const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
        for (uint32_t i = 0; i < n_count; ++i) {
          const uint32_t n_index = tile_data.neighbors[i];
          if (ground_distance[n_index].second == UINT32_MAX) {
            ground_distance[n_index] = std::make_pair(ground_distance[current_tile].first, ground_distance[current_tile].second + 1);
            queue.push(n_index);
            ++state;
          }
        }
      }
      
      std::swap(*data_container, ground_distance);
    }
    
    size_t calculate_tile_distance::progress() const {
      return state;
    }
    
    size_t calculate_tile_distance::complete_state(const generator_context* context) const {
      return context->map->tiles_count();
    }
    
    std::string calculate_tile_distance::hint() const {
      return hint_str;
    }
    
    modify_tile_elevation::modify_tile_elevation(const create_info &info) : pool(info.pool), elevation_func(info.elevation_func), hint_str(info.hint_str), state(0) {}
    void modify_tile_elevation::process(generator_context* context) {
      utils::time_log log(hint_str);
      state = 0;
      
      std::vector<float> new_elevations(context->map->tiles_count(), 0.0f);
      utils::submit_works(pool, context->map->tiles_count(), [&new_elevations] (
        const size_t &start, 
        const size_t &count, 
        const generator_context* context, 
        std::atomic<size_t> &state, 
        const std::function<float(const generator_context* context, const uint32_t &tile_index)> &func
      ) {
        for (size_t i = start; i < start+count; ++i) {
          const uint32_t tile_index = i;
          new_elevations[tile_index] = func(context, tile_index);
          ++state;
        }
      }, context, std::ref(state), std::ref(elevation_func));
      
      std::swap(context->tile_elevation, new_elevations);
    }
    
    size_t modify_tile_elevation::progress() const {
      return state;
    }
    
    size_t modify_tile_elevation::complete_state(const generator_context* context) const {
      return context->map->tiles_count();
    }
    
    std::string modify_tile_elevation::hint() const {
      return hint_str;
    }
    
    tile_postprocessing1::tile_postprocessing1(const create_info &info) : pool(info.pool) {}
    void tile_postprocessing1::process(generator_context* context) {
      utils::time_log log("tile postprocessing1");
      state = 0;
      
      std::vector<float> new_elevations(context->tile_elevation);
      std::mutex mutex;
      std::unordered_set<uint32_t> islands;
      utils::submit_works(pool, context->map->tiles_count(), [&new_elevations, &mutex, &islands] (const size_t &start, const size_t &count, const generator_context* context, std::atomic<size_t> &state) {
        for (size_t i = start; i < start+count; ++i) {
          const uint32_t tile_index = i;
          const uint32_t dist_to_water = context->water_distance[tile_index].second;
          const float current_height = context->tile_elevation[tile_index];
          if (dist_to_water == 0) continue;
          
          uint32_t n_counter = 0;
          uint32_t n_counter2 = 0;
          float accum_height = 0.0f;
          const auto &tile_data = render::unpack_data(context->map->get_tile(tile_index));
          const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
          for (uint32_t j = 0; j < n_count; ++j) {
            const uint32_t n_index = tile_data.neighbors[j];
            const uint32_t n_dist_to_water = context->water_distance[n_index].second;
            const float n_height = context->tile_elevation[n_index];
            
            n_counter += n_dist_to_water;
            if (n_height >= 0.0f) {
              accum_height += n_height;
              ++n_counter2;
            }
          }
          
          if (dist_to_water == 1 && n_counter <= 4) {
            if (current_height < 0.5f) {
              new_elevations[tile_index] -= 0.6f + EPSILON;
            } else {
              new_elevations[tile_index] = 1.0f;
            }
          }
//           
//           if (dist_to_water == 1 && n_counter > 1) {
//             
//           }
//           
//           if (dist_to_water < 3 && float(n_counter) / float(n_count) <= 12 && ((accum_height + current_height) / float(n_counter2+1) <= 0.15f)) {
//             new_elevations[tile_index] -= 0.2f + EPSILON;
//           }
//           
//           if (n_counter <= 2) {
//             ASSERT(dist_to_water == 1);
//             ASSERT(current_height >= 0.0f);
//             if (current_height < 0.5f) {
//               new_elevations[tile_index] -= 0.6f + EPSILON;
//             } else {
//               new_elevations[tile_index] = 1.0f;
//             }
//           }
          
          ++state;
        }
      }, context, std::ref(state));
      
      std::swap(context->tile_elevation, new_elevations);
      
//       for (size_t i = 0 ; i < context->tile_elevation.size(); ++i) {
//         context->map->set_tile_height(i, context->tile_elevation[i]);
//       }
    }
    
    size_t tile_postprocessing1::progress() const {
      return state;
    }
    
    size_t tile_postprocessing1::complete_state(const generator_context* context) const {
      return context->map->tiles_count();
    }
    
    std::string tile_postprocessing1::hint() const {
      return "postprocessing 1";
    }
    
    // реки... я так понимаю нужно найти горы и озера (или просто ближайшие водоемы)
    // как то правильно у этого всего найти веса, и устремить реки от гор к озерам
    // тут мне потребуется брать соседей по направлению
    // по идее не нужно как то ограничивать спавн рек
    // рек существует два типа: тайловая река и по точкам
    // первые это очень полноводные реки - их должно быть немного
    // вторых можно наспавнить хоть сколько
    
    tile_postprocessing2::tile_postprocessing2(const create_info &info) : pool(info.pool) {}
    void tile_postprocessing2::process(generator_context* context) {
      utils::time_log log("tile postprocessing2");
      state = 0;
      
      std::vector<float> new_elevations(context->tile_elevation);
      utils::submit_works(pool, context->map->tiles_count(), [&new_elevations] (const size_t &start, const size_t &count, const generator_context* context, std::atomic<size_t> &state) {
        for (size_t i = start; i < start+count; ++i) {
          const uint32_t tile_index = i;
          const uint32_t dist_to_ground = context->ground_distance[tile_index].second;
          const float current_height = context->tile_elevation[tile_index];
          if (dist_to_ground == 0) continue;
          
          uint32_t n_counter = 0;
          float n_height_accumulated = 0.0f;
          const auto &tile_data = render::unpack_data(context->map->get_tile(tile_index));
          const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
          for (uint32_t j = 0; j < n_count; ++j) {
            const uint32_t n_index = tile_data.neighbors[j];
            const uint32_t n_dist_to_ground = context->ground_distance[n_index].second;
            const float n_height = context->tile_elevation[n_index];
            
            n_counter += n_dist_to_ground;
            n_height_accumulated += n_height;
          }
          
//           if (n_counter < 1 && n_height_accumulated / float(n_count) < 0.5f) {
//             ASSERT(dist_to_ground == 1);
//             ASSERT(current_height < 0.0f);
// //             if (current_height < 0.5f) {
// //               new_elevations[tile_index] -= 0.5f;
// //             }
//             // это мелкие вкрапления воды, что с ними делать?
//             // возможно как нибудь их расширить
//             // с другой стороны если наспавнить горы вокруг этого водоема
//             // то тогда получится неплохое горное озеро
//           }

          if (n_counter <= 4 && dist_to_ground == 1) {
            new_elevations[tile_index] = n_height_accumulated / float(n_count);
          }
          
          ++state;
        }
      }, context, std::ref(state));
      
      std::swap(context->tile_elevation, new_elevations);
      
//       for (size_t i = 0 ; i < context->tile_elevation.size(); ++i) {
//         context->map->set_tile_height(i, context->tile_elevation[i]);
//       }
    }
    
    size_t tile_postprocessing2::progress() const {
      return state;
    }
    
    size_t tile_postprocessing2::complete_state(const generator_context* context) const {
      return context->map->tiles_count();
    }
    
    std::string tile_postprocessing2::hint() const {
      return "postprocessing 2";
    }
    
    connect_water_pools::connect_water_pools() : state(0) {}
    void connect_water_pools::process(generator_context* context) {
      utils::time_log log("water pools connection");
      state = 0;
      
      std::vector<uint32_t> tile_pool(context->map->tiles_count(), UINT32_MAX);
      
      uint32_t water_tiles = 0;
      uint32_t pool_id = 0;
      for (size_t i = 0; i < context->map->tiles_count(); ++i) {
        const float h = context->tile_elevation[i];
        if (h >= 0.0f) continue;
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
          
          const auto &tile_data = render::unpack_data(context->map->get_tile(current_tile));
          const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
          for (uint32_t j = 0; j < n_count; ++j) {
            const uint32_t n_index = tile_data.neighbors[j];
            const float h = context->tile_elevation[n_index];
            if (h >= 0.0f) continue;
            if (tile_pool[n_index] == UINT32_MAX) {
              tile_pool[n_index] = current_pool_id;
              queue.push(n_index);
            }
          }
        }
      }
      
      std::vector<std::vector<uint32_t>> water_pools(pool_id);
      for (size_t i = 0; i < context->map->tiles_count(); ++i) {
        const uint32_t pool_index = tile_pool[i];
        if (pool_index == UINT32_MAX) continue;
        water_pools[pool_index].push_back(i);
      }
      
      std::vector<uint32_t> sorted_pools(pool_id);
      for (size_t i = 0; i < pool_id; ++i) {
        sorted_pools[i] = i;
      }
      
      std::sort(sorted_pools.begin(), sorted_pools.end(), [&water_pools] (const uint32_t &first, const uint32_t &second) {
        return water_pools[first].size() < water_pools[second].size();
      });
      
      std::vector<float> new_elevations(context->tile_elevation);
      
      uint32_t tile_counter = 0;
      for (size_t i = 0; i < water_pools.size(); ++i) {
        const uint32_t pool_index = sorted_pools[i];
        const uint32_t next_pool_index = sorted_pools[std::min(size_t(i+1), water_pools.size()-1)];
        tile_counter += water_pools[pool_index].size();
        
        ASSERT(water_pools[pool_index].size() <= water_pools[next_pool_index].size());
        
//         uint32_t max_dist = 0;
//         for (size_t j = 0; j < water_pools[pool_index].size(); ++j) {
//           const uint32_t tile_index = water_pools[pool_index][j];
//           
//           ASSERT(context->ground_distance[tile_index].second != 0);
//           if (context->ground_distance[tile_index].second > max_dist) {
//             max_dist = context->ground_distance[tile_index].second;
//           }
//         }
        
//         std::queue<uint32_t> queue0;
//         std::queue<uint32_t> queue1;
        std::map<uint32_t, uint32_t> unique_neighbours;
        
        float min_height = 2.0f;
        uint32_t max_dist = 0;
        for (size_t j = 0; j < water_pools[pool_index].size(); ++j) {
          const uint32_t tile_index = water_pools[pool_index][j];
          ASSERT(context->ground_distance[tile_index].second != 0);
          max_dist = std::max(max_dist, context->ground_distance[tile_index].second);
          const auto &tile_data = render::unpack_data(context->map->get_tile(tile_index));
          const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
          for (uint32_t k = 0; k < n_count; ++k) {
            const uint32_t n_index = tile_data.neighbors[k];
            if (context->water_distance[n_index].second == 0) continue;
            if (tile_pool[n_index] == pool_index) continue;
            
            auto itr = unique_neighbours.find(n_index);
            if (itr == unique_neighbours.end()) unique_neighbours[n_index] = tile_index;
            const float h = context->tile_elevation[n_index];
            ASSERT(h >= 0.0f);
            min_height = std::min(min_height, h);
          }
        }
        
        min_height = std::max(min_height, 0.2f);
        
        std::queue<std::pair<uint32_t, uint32_t>> queue0;
        for (const auto &p : unique_neighbours) {
          queue0.push(p);
        }
        
        uint32_t nearest_water_tile_index = UINT32_MAX;
        while (!queue0.empty() && nearest_water_tile_index == UINT32_MAX) {
//           const auto current_tile = *unique_neighbours.begin();
//           unique_neighbours.erase(unique_neighbours.begin());
          ASSERT(nearest_water_tile_index == UINT32_MAX); 
          const auto current_tile = queue0.front();
          queue0.pop();
          
          const float h = context->tile_elevation[current_tile.first];
          const auto &tile_data = render::unpack_data(context->map->get_tile(current_tile.first));
          const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
          for (uint32_t k = 0; k < n_count; ++k) {
            const uint32_t n_index = tile_data.neighbors[k];
            ASSERT(n_index != UINT32_MAX);
            if (context->tile_elevation[n_index] > min_height) continue;
            if (tile_pool[n_index] == pool_index) continue;
            
            if (context->water_distance[n_index].second == 0) {
              if (water_pools[tile_pool[n_index]].size() < water_pools[pool_index].size()) continue;
              nearest_water_tile_index = n_index;
              unique_neighbours[n_index] = current_tile.first;
              break;
            }
            
            auto itr = unique_neighbours.find(n_index);
            if (itr == unique_neighbours.end()) {
              unique_neighbours[n_index] = current_tile.first;
              queue0.push(std::make_pair(n_index, current_tile.first));
            }
          }
        }
        
        if (nearest_water_tile_index != UINT32_MAX) {
          const uint32_t nearest_water_pool = tile_pool[nearest_water_tile_index];
          ASSERT(nearest_water_pool != pool_index);
          
          std::vector<uint32_t> river_tiles;
          //auto itr = unique_neighbours.find(nearest_water_tile_index);
          uint32_t prev_tile = nearest_water_tile_index;
          river_tiles.push_back(prev_tile);
          while (tile_pool[prev_tile] != pool_index) {
            auto itr = unique_neighbours.find(prev_tile);
            ASSERT(itr != unique_neighbours.end());
            prev_tile = itr->second;
            river_tiles.push_back(prev_tile);
          }
          
          // нужно сделать так чтобы реки спавнились хотя бы змейкой 
          // в river_tiles у нас лежат тайлы по прямой
          // по идее мы просто можем сделать алгоритм на основе nearest_water_tile_index
          // 
          
          const uint32_t end_tile = river_tiles.back();
          const auto &end_tile_data = render::unpack_data(context->map->get_tile(end_tile));
          const glm::vec4 end_point = context->map->get_point(end_tile_data.center);
          uint32_t current_tile = nearest_water_tile_index;
          while (current_tile != river_tiles.back() && current_tile != UINT32_MAX) {
            const float h = context->tile_elevation[current_tile];
            const auto &tile_data = render::unpack_data(context->map->get_tile(current_tile));
            const glm::vec4 point = context->map->get_point(tile_data.center);
            const glm::vec4 end_dir = glm::normalize(end_point - point);
            const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
            uint32_t old_tile_index = current_tile;
            for (uint32_t k = 0; k < n_count; ++k) {
              const uint32_t n_index = tile_data.neighbors[k];
              ASSERT(n_index != UINT32_MAX);
              const float n_h = context->tile_elevation[n_index];
              if (n_h > min_height) continue;
              if (context->water_distance[n_index].second == 0) {
                current_tile = UINT32_MAX;
                break;
              }
              
              const auto &n_tile_data = render::unpack_data(context->map->get_tile(n_index));
              const glm::vec4 n_point = context->map->get_point(n_tile_data.center);
              const glm::vec4 n_dir = glm::normalize(n_point - point);
              const float dot = glm::max(glm::dot(end_dir, n_dir), 0.0f);
              
              ASSERT(dot <= 1.0f);
              const float final_k = glm::mix(0.3f, 0.7f, dot);
              const bool picked = context->data->engine->probability(final_k);
              if (picked) {
                current_tile = n_index;
                new_elevations[n_index] = -0.4f;
                break;
              }
            }
            
            if (old_tile_index == current_tile) break;
          }
          
//           for (size_t j = 0; j < river_tiles.size(); ++j) {
//             new_elevations[river_tiles[j]] = -0.3f;
//           }
        }
      }
      
      std::swap(context->tile_elevation, new_elevations);
    }
    
    size_t connect_water_pools::progress() const {
      return state;
    }
    
    size_t connect_water_pools::complete_state(const generator_context* context) const {
      return 0;
    }
    
    std::string connect_water_pools::hint() const {
      return "connecting water pools";
    }
    
    generate_water_pools::generate_water_pools() : state(0) {}
    void generate_water_pools::process(generator_context* context) {
      utils::time_log log("water pools generation");
      state = 0;
      
      // как определить сколько рек нужно сгенерировать?
      // явно так просто не сказать, у нас есть понимание 
      // где меньше всего водоемов
      
      // короч нужно спавнить реки начиная от водоемов
      // 
      
      std::vector<float> new_elevations(context->tile_elevation);
      
      std::vector<uint32_t> beaches;
      for (uint32_t i = 0; i < context->map->tiles_count(); ++i) {
        const uint32_t tile_index = i;
        if (context->water_distance[tile_index].second == 1) {
          beaches.push_back(tile_index);
        }
      }
      
      PRINT_VAR("beaches.size()", beaches.size())
      while (!beaches.empty()) {
        const uint32_t rand_index = context->data->engine->index(beaches.size());
        const uint32_t tile_index = beaches[rand_index];
        beaches[rand_index] = beaches.back();
        beaches.pop_back();
        
//         const bool chance = context->data->engine->probability(context->data->ocean_percentage);
//         if (!chance) continue;
        
        //if (context->ground_distance[tile_index].second == 1)
        {
          std::unordered_set<uint32_t> unique_tiles;
          unique_tiles.insert(tile_index);
          
          std::vector<uint32_t> river_tiles;
          river_tiles.push_back(tile_index);
          uint32_t current_tile = tile_index;
          float height = context->tile_elevation[current_tile];
          auto tile_data = render::unpack_data(context->map->get_tile(current_tile));
          uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
          glm::vec4 point = context->map->get_point(tile_data.center);
          //glm::vec4 dir = glm::vec4(context->data->engine->unit3(), 0.0f);
          glm::vec4 dir = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
          bool founded = true;
          while (height < 0.75f && founded) {
            founded = false;
            for (uint32_t k = 0; k < n_count; ++k) {
              const uint32_t new_index = tile_data.neighbors[k];
              ASSERT(new_index != UINT32_MAX);
              auto new_tile_data = render::unpack_data(context->map->get_tile(new_index));
              const glm::vec4 new_point = context->map->get_point(new_tile_data.center);
              const glm::vec4 new_dir = glm::normalize(new_point - point);
              const float new_height = context->tile_elevation[new_index];
              
              if (new_height < height) continue;
              if (unique_tiles.find(new_index) != unique_tiles.end()) continue;
              
//               PRINT_VAR("new_index", new_index)
              if (new_height > 0.75f) {
                river_tiles.push_back(new_index);
                current_tile = new_index;
                height = new_height;
                founded = true;
                break;
              }
              
              unique_tiles.insert(new_index);
              const float dot = glm::dot(dir, new_dir);
              const float final_dot = (dot + 0.7f) / 1.7f;
//               PRINT_VAR("dot", dot)
//               PRINT_VAR("new_height", new_height)
              const float final_k = glm::mix(0.0f, 0.99f, final_dot);
//               PRINT_VAR("final_k", final_k)
              const bool choosen = context->data->engine->probability(final_k);
//               const bool choosen = final_k >= 0.3f;
              
              if (choosen) {
                point = new_point;
                dir = new_dir;
                height = new_height;
                current_tile = new_index;
                n_count = render::is_pentagon(new_tile_data) ? 5 : 6;
                founded = true;
                river_tiles.push_back(new_index);
//                 PRINT_VAR("river_tiles size", river_tiles.size())
                break;
              }
            }
          }
          
          if (!founded) continue;
          if (river_tiles.size() < 5) continue;
          
          PRINT_VAR("river_tiles size()", river_tiles.size());
          for (uint32_t k = 0; k < river_tiles.size()-1; ++k) {
            const uint32_t tile_index = river_tiles[k];
            new_elevations[tile_index] = -0.3f;
            //context->tile_elevation[tile_index] = -0.3f;
          }
        }
      }
      
//       std::vector<uint32_t> tile_pool(context->map->tiles_count(), UINT32_MAX);
//       
//       uint32_t water_tiles = 0;
//       uint32_t pool_id = 0;
//       for (size_t i = 0; i < context->map->tiles_count(); ++i) {
//         const float h = context->tile_elevation[i];
//         if (h < 0.0f) continue;
//         ++water_tiles;
//         if (tile_pool[i] != UINT32_MAX) continue;
//         
//         const uint32_t current_pool_id = pool_id;
//         ++pool_id;
//         
//         tile_pool[i] = current_pool_id;
//         std::queue<uint32_t> queue;
//         queue.push(i);
//         
//         while (!queue.empty()) {
//           const uint32_t current_tile = queue.front();
//           queue.pop();
//           
//           const auto &tile_data = render::unpack_data(context->map->get_tile(current_tile));
//           const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
//           for (uint32_t j = 0; j < n_count; ++j) {
//             const uint32_t n_index = tile_data.neighbours[j];
//             const float h = context->tile_elevation[n_index];
//             if (h < 0.0f) continue;
//             if (tile_pool[n_index] == UINT32_MAX) {
//               tile_pool[n_index] = current_pool_id;
//               queue.push(n_index);
//             }
//           }
//         }
//       }
//       
//       std::vector<std::vector<uint32_t>> ground_pools(pool_id);
//       for (size_t i = 0; i < context->map->tiles_count(); ++i) {
//         const uint32_t pool_index = tile_pool[i];
//         if (pool_index == UINT32_MAX) continue;
//         ground_pools[pool_index].push_back(i);
//       }
//       
//       std::vector<uint32_t> sorted_pools(pool_id);
//       for (size_t i = 0; i < pool_id; ++i) {
//         sorted_pools[i] = i;
//       }
//       
//       std::sort(sorted_pools.begin(), sorted_pools.end(), [&ground_pools] (const uint32_t &first, const uint32_t &second) {
//         return ground_pools[first].size() < ground_pools[second].size();
//       });
//       
//       struct ground_pool_data_t {
//         uint32_t size;
//         uint32_t max_dist_to_water;
//         float max_height;
//       };
//       
//       std::vector<ground_pool_data_t> ground_pool_datas(ground_pools.size());
//       for (size_t i = 0; i < ground_pools.size(); ++i) {
//         ground_pool_datas[i].size = ground_pools[i].size();
//         ground_pool_datas[i].max_height = 0.0f;
//         ground_pool_datas[i].max_dist_to_water = 0;
//         
//         for (size_t j = 0; j < ground_pools[i].size(); ++j) {
//           const float height = context->tile_elevation[ground_pools[i][j]];
//           const uint32_t dist = context->water_distance[ground_pools[i][j]].second;
//           ground_pool_datas[i].max_height = std::max(ground_pool_datas[i].max_height, height);
//           ground_pool_datas[i].max_dist_to_water = std::max(ground_pool_datas[i].max_dist_to_water, dist);
//         }
//       }
//       
//       PRINT_VAR("ground_pools size", ground_pools.size())
//       for (size_t i = 0; i < ground_pools.size(); ++i) {
//         if (ground_pool_datas[i].size < 250) continue;
//         if (ground_pool_datas[i].max_height <= 0.5f) continue;
//         if (ground_pool_datas[i].max_dist_to_water < 20) continue;
//         
//         // grand rivers
//         const uint32_t grand_rivers_count = (ground_pool_datas[i].size - 250) / 50 + 1;
//         // TODO: small rivers 
//         
//         std::vector<uint32_t> mountains;
//         for (size_t k = 0; k < ground_pool_datas[i].size; ++k) {
//           const uint32_t tile_index = ground_pools[i][k];
//           const float height = context->tile_elevation[tile_index];
//           if (height < 0.5f) continue;
//           
//           mountains.push_back(tile_index);
//         }
//         
//         PRINT_VAR("grand_rivers_count", grand_rivers_count)
//         PRINT_VAR("mountains size()", mountains.size())
//         for (size_t j = 0; j < grand_rivers_count; ++j) {
//           if (mountains.empty()) break;
//           
//           const uint32_t rand_index = context->data->engine->index(mountains.size());
//           const uint32_t tile_index = mountains[rand_index];
//           mountains[rand_index] = mountains.back();
//           mountains.pop_back();
//           
//           const float tile_height = context->tile_elevation[tile_index];
//           const auto &tile_data = render::unpack_data(context->map->get_tile(tile_index));
//           const glm::vec4 point = context->map->get_point(tile_data.center);
//           uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
//           uint32_t n_index = UINT32_MAX;
//           const uint32_t attempts = 7;
//           uint32_t counter = 0;
//           
//           for (uint32_t n = 0; n < n_count; ++n) {
//             uint32_t n_index = tile_data.neighbours[n];
//             float height = context->tile_elevation[n_index];
//             if (height > tile_height || height < 0.0f) continue;
//             
//             std::vector<uint32_t> river_tiles;
//           
//             auto n_tile_data = render::unpack_data(context->map->get_tile(n_index));
//             n_count = render::is_pentagon(tile_data) ? 5 : 6;
//             glm::vec4 n_point = context->map->get_point(n_tile_data.center);
//             glm::vec4 dir = glm::normalize(point - n_point);
//             std::unordered_set<uint32_t> unique_tiles;
//             unique_tiles.insert(n_index);
//             river_tiles.push_back(n_index);
//             bool founded = true;
//             PRINT_VAR("height", height)
//             while (context->tile_elevation[n_index] >= 0.0f && founded) {
//               founded = false;
//               for (uint32_t k = 0; k < n_count; ++k) {
//                 const uint32_t new_index = n_tile_data.neighbours[k];
//                 auto new_tile_data = render::unpack_data(context->map->get_tile(new_index));
//                 const glm::vec4 new_point = context->map->get_point(new_tile_data.center);
//                 const glm::vec4 new_dir = glm::normalize(n_point - new_point);
//                 const float new_height = context->tile_elevation[new_index];
//                 
//                 if (new_height > (height < 0.4f ? height + 0.1f : height)) continue;
//                 if (unique_tiles.find(new_index) != unique_tiles.end()) continue;
//                 
//                 PRINT_VAR("new_index", new_index)
//                 if (new_height < 0.0f) {
//                   river_tiles.push_back(new_index);
//                   n_index = new_index;
//                   founded = true;
//                   break;
//                 }
//                 
//                 unique_tiles.insert(new_index);
//                 const float dot = std::max(glm::dot(dir, new_dir), 0.0f);
//                 const float final_k = glm::mix(0.1f, 0.7f, dot);
//                 const bool choosen = context->data->engine->probability(final_k);
//                 
//                 if (choosen) {
//                   n_point = new_point;
//                   dir = new_dir;
//                   height = new_height;
//                   n_index = new_index;
//                   founded = true;
//                   river_tiles.push_back(new_index);
//                   break;
//                 }
//               }
//             }
//             
//             if (!founded) continue;
//             
//             PRINT_VAR("river_tiles size()", river_tiles.size());
//             for (uint32_t k = 0; k < river_tiles.size(); ++k) {
//               const uint32_t tile_index = river_tiles[k];
//               new_elevations[tile_index] = -0.3f;
//               //context->tile_elevation[tile_index] = -0.3f;
//             }
//           }
//           
// //           while (n_index == UINT32_MAX && counter < attempts) {
// //             const uint32_t rand_index = context->data->engine->index(n_count);
// //             n_index = tile_data.neighbours[rand_index];
// //             const float height = context->tile_elevation[n_index];
// //             if (height >= tile_height || height < 0.0f) n_index = UINT32_MAX;
// //             
// //             ++counter;
// //           }
//           
// //           if (n_index == UINT32_MAX) continue;
// //           ASSERT(context->tile_elevation[n_index] >= 0.0f);
//           
//           
//         }
//       }
      
      std::swap(context->tile_elevation, new_elevations);
    }
    
    size_t generate_water_pools::progress() const {
      return state;
    }
    
    size_t generate_water_pools::complete_state(const generator_context* context) const {
      return 0;
    }
    
    std::string generate_water_pools::hint() const {
      return "generating rivers and lakes";
    }
    
    compute_tile_heat::compute_tile_heat(const create_info &info) : pool(info.pool), state(0) {}
    void compute_tile_heat::process(generator_context* context) {
      utils::time_log log("heat generation");
      state = 0;
      
      std::vector<float> tile_heat(context->map->tiles_count(), 0.0f);
      float min_value =  1.0f;
      float max_value = -1.0f;
      for (uint32_t i = 0; i < tile_heat.size(); ++i) {
        const auto &tile_data = render::unpack_data(context->map->get_tile(i));
        const glm::vec4 point = context->map->get_point(tile_data.center);
        // скорее всего нужно увеличить вклад шума
        tile_heat[i] = context->data->noise->GetNoise(point.x, point.y, point.z); // 0.1f * 
//         ASSERT(tile_heat[i] >= -0.1f);
//         ASSERT(tile_heat[i] <=  0.1f);
        min_value = std::min(min_value, tile_heat[i]);
        max_value = std::max(max_value, tile_heat[i]);
      }
      
      for (uint32_t i = 0; i < tile_heat.size(); ++i) {
        tile_heat[i] = (tile_heat[i] - min_value) / (max_value - min_value);
        tile_heat[i] = 0.1f * tile_heat[i];
      }
      
      utils::submit_works(pool, context->map->tiles_count(), [&tile_heat] (const size_t &start, const size_t &count, const generator_context* context, std::atomic<size_t> &state) {
        for (size_t i = start; i < start+count; ++i) {
          ++state;
          const uint32_t current_tile = i;
          const float height = context->tile_elevation[current_tile];
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
      }, context, std::ref(state));
      
//       utils::submit_works(pool, context->map->tiles_count(), [&tile_heat] (const size_t &start, const size_t &count, const generator_context* context, std::atomic<size_t> &state) {
//         for (size_t i = start; i < start+count; ++i) {
//           ++state;
//           const uint32_t current_tile = i;
//           const float height = context->tile_elevation[current_tile];
//           if (height < 0.0f) continue;
//                           
//           
//         }
//       }, context, std::ref(state));
      
      std::swap(context->tile_heat, tile_heat);
    }
    
    size_t compute_tile_heat::progress() const {
      return state;
    }
    
    size_t compute_tile_heat::complete_state(const generator_context* context) const {
      return context->map->tiles_count()*2;
    }
    
    std::string compute_tile_heat::hint() const {
      return "generating heat";
    }
    
    normalize_fractional_values::normalize_fractional_values(const create_info &info) : pool(info.pool), state(0), data_container(info.data_container), hint_str(info.hint_str) {}
    void normalize_fractional_values::process(generator_context* context) {
      utils::time_log log(hint_str);
      state = 0;
      
      std::atomic<int32_t> min_height_mem( INT32_MAX);
      std::atomic<int32_t> max_height_mem(-INT32_MAX);
      
      std::vector<float> new_data(*data_container);
      utils::submit_works(pool, data_container->size(), [&min_height_mem, &max_height_mem, &new_data] (const size_t &start, const size_t &count, const generator_context* context, std::atomic<size_t> &state) {
        for (size_t i = start; i < start+count; ++i) {
          ++state;
          const uint32_t tile_index = i;
          float data = new_data[tile_index];
          if (data < 0.0f) continue;
          //tile_height = (tile_height + 1.0f) / 2.0f;
          //tile_height = mapper(tile_height, -1.0f, 1.0f, 0.0f, 1.0f);
//           ASSERT(tile_height >= 0.0f && tile_height <= 2.0f);
          update_maximum(max_height_mem, glm::floatBitsToInt(data));
          update_minimum(min_height_mem, glm::floatBitsToInt(data));
        }
      }, context, std::ref(state));
      
      utils::submit_works(pool, data_container->size(), [&min_height_mem, &max_height_mem, &new_data] (const size_t &start, const size_t &count, const generator_context* context, std::atomic<size_t> &state) {
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
          ++state;
          const uint32_t tile_index = i;
          float tile_data = new_data[tile_index];
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
          new_data[tile_index] = new_tile_height;
        }
      }, context, std::ref(state));
      
      std::swap(*data_container, new_data);
    }
    
    size_t normalize_fractional_values::progress() const {
      return state;
    }
    
    size_t normalize_fractional_values::complete_state(const generator_context* context) const {
      return data_container->size()*2;
    }
    
    std::string normalize_fractional_values::hint() const {
      return hint_str;
    }
    
    compute_moisture::compute_moisture(const create_info &info) : pool(info.pool), state(0) {}
    void compute_moisture::process(generator_context* context) {
      utils::time_log log(hint());
      state = 0;
      
      // короче осадки появляются на земле от испарений из океанов
      // довольно протяженными по Х биомами
      // протяженность по X возникает из-за свойств движения планеты
      // участки биомов появляются из-за циркуляции влаги в природе
      // влага циркулирует по каким то воздушным паттернам 
      // большая часть влаги оседает на берегу
      // огромное количество осадков выпадает сразу после гор и вокруг рек
      // это создает довольно большие однородные участки биомов
      // к сожалению циркуляция воздуха - это очень затратный алгоритм
      // и я бы не хотел его использовать
      // как тогда можно сделать осадки?
      // нам необходимо каким то образом собрать некий условный бассейн осадков
      // который потом проталкивая по земле распределять по тайлам
      // по идее если мы пару раз обойдем все тайлы последовательно начиная от самых удаленных от воды
      // собираем влажность из окенов, движемся примерно по движению ветров, когда встречаем
      // сушу - сбрасываем часть осадков, по каким нибудь признакам
      // как аккумулировать осадки?
      
//       std::vector<uint32_t> highest_dist_to_ground(context->map->tiles_count(), 0);
//       for (uint32_t i = 0; i < context->map->tiles_count(); ++i) {
//         highest_dist_to_ground[i] = i;
//       }
//       
//       std::sort(highest_dist_to_ground.begin(), highest_dist_to_ground.end(), [context] (const uint32_t &first, const uint32_t &second) -> bool {
//         return context->ground_distance[first].second > context->ground_distance[second].second;
//       });
//       
//       // 400 сантиметров осадков при 25 градусах - это тропические джунгли
//       // 25 или 30 градусов - это почти единица
//       const uint32_t max_precipitation = 400;
//       
//       std::vector<size_t> tile_precipitation(context->map->tiles_count(), 0);
//       std::vector<std::pair<uint32_t, uint32_t>> tile_precipitation_rate(context->map->tiles_count(), std::make_pair(0, 0));
//       for (size_t i = 0; i < tile_precipitation.size(); ++i) {
//         const uint32_t current_tile = i;
//         const float height = context->tile_elevation[current_tile];
// //         if (height >= 0.0f) continue;
//         
//         const float heat = context->tile_heat[current_tile];
//         
//         if (height >= 0.0f) {
//           const uint32_t take = heat * max_precipitation * 0.5f;   // забираем осадки исходя из теплоты
//           const uint32_t give = height * max_precipitation; // гора реально много отдает
//           tile_precipitation_rate[i] = std::make_pair(take, give);
//         } else {
//           const uint32_t take = heat*heat * max_precipitation;   // по идее осадков много выпадает на экваторе
//           const uint32_t give = heat * max_precipitation * 10.0f;        // чем меньше температура тем меньше осадков
//           tile_precipitation_rate[i] = std::make_pair(take, give);
//         }
// //         
// //         // я могу предрасчитать сколько влаги испаряется
// //         // и сколько влаги всего может принять тайл
// //         // если мы будем двигаться от самых удаленных частей 
// //         // предположим по 100 осадков за тайл, накапливая в соседях
// //         // рано или поздно мы дойдем до какого нибудь тайла земли 
// //         // но тут проблема заключается в том что если мы не дойдем до земли, 
// //         // то будем циркулировать до бемконечности, либо потеряем целый кусок осадков
// //         // ко всему прочему у нас осадки перебрасывают тайлы друг другу 
// //         // и нужно реально обойти несколько раз все тайлы
// //         
// //         // нужно любыми способами избежать обхода по несколько раз
// //         // как тогда? что у нас есть? у нас есть та странная формула которую я плохо понимаю
// //         // но она прекрасно работает, для нее нужно указать 3 типа тайлов
// //         // в данном случае: пустыни, области высокого количества осадков и умеренные области
// //         // другое дело что переход между областями с осадками и без крайне резкий (в масштабах планеты)
// //         // и нам тут нужно указать скорее не границы, а непосредственно область осадков
// // 
// //         // с другой стороны я могу попытаться расчитать количество осадков на берегу
// //         // и попробовать распределить их, то есть для каждой точки берега посмотреть на температуру,
// //         // на то куда направлен ветер, и условно пропушить вглубь материка коэффициент
// //         // либо мы можем просто рассчитать начиная от самой удаленной точки водоема
// //         // за один раз число осадков на берегу, и это число использовать для расчета 
// //         // осадков для тайлов земли
// //         // другое дело что это скорее всего приведет к очень однобокой генерации
// //         // но пока что этот вариант выглядит наиболее жизнеспособным
// 
//         // лучше просто поиграться с шумами, мне нужно в основном сгруппировать тайлы по типу
//         // так чтобы появились продолжительные области растянутые по X
//         // иначе я не знаю как сделать распределение осадков
//       }
//       
//       std::unordered_set<uint32_t> unique_tiles;
//       std::queue<uint32_t> queue;
//       for (uint32_t i = 0; i < context->map->tiles_count(); ++i) {
//         const uint32_t current_tile = highest_dist_to_ground[i];
//         const float height = context->tile_elevation[current_tile];
//         if (height >= 0.0f) continue;
//         queue.push(current_tile);
//         unique_tiles.insert(current_tile); // это означает, что мы не будем обходить предыдущие тайлы
//       }
//       
//       //tile_precipitation[highest_dist_to_ground[0]] = 1000;
//       
//       while (!queue.empty()) {
//         const uint32_t current_tile = queue.front();
//         queue.pop();
//         
//         const auto &rate = tile_precipitation_rate[current_tile];
//         size_t current_precipitation = tile_precipitation[current_tile];
//         if (rate.first <= rate.second) {
//           current_precipitation += rate.second - rate.first;
//           tile_precipitation[current_tile] = rate.first;
//         } else {
//           const uint32_t final_rate = rate.first - rate.second;
//           if (current_precipitation < final_rate) {
//             tile_precipitation[current_tile] = current_precipitation;
//             current_precipitation = 0;
//           } else {
//             current_precipitation -= final_rate;
//             tile_precipitation[current_tile] = rate.first;
//           }
//         }
//         
//         if (current_precipitation == 0) continue;
//         
//         const auto &tile_data = render::unpack_data(context->map->get_tile(current_tile));
//         const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
//         float max_outflow = 0.0f;
//         for (uint32_t i = 0; i < n_count; ++i) {
//           max_outflow += context->tile_air_outflows[current_tile][i];
//         }
//         
//         for (uint32_t i = 0; i < n_count; ++i) {
//           const float current_outflow = context->tile_air_outflows[current_tile][i];
//           if (current_outflow < EPSILON) continue;
//           const float k = current_outflow / max_outflow;
//           
//           const uint32_t n_index = tile_data.neighbours[i];
//           if (unique_tiles.find(n_index) != unique_tiles.end()) continue;
//           tile_precipitation[n_index] += k * current_precipitation;
//           queue.push(n_index);
//           unique_tiles.insert(n_index);
//         }
//       }
//       
//       context->tile_wetness.resize(context->map->tiles_count(), 0);
//       for (size_t i = 0; i < context->map->tiles_count(); ++i) {
//         context->tile_wetness[i] = std::min(float(tile_precipitation[i]) / float(tile_precipitation_rate[i].first), 1.0f);
//       }
      
      // короче шум перлина выглядит подходяще честно говоря
      // возможно его нужно парочкой параметров подкрасить
      std::vector<float> wetness(context->map->tiles_count());
      float max_val = -1.0f;
      float min_val =  1.0f;
      for (size_t i = 0; i < context->map->tiles_count(); ++i) {
        const uint32_t current_tile = i;
        const auto &tile_data = context->map->get_tile(current_tile);
        const glm::vec4 point = context->map->get_point(tile_data.tile_indices.x);
        const float k = context->data->noise->GetNoise(point.x, point.y, point.z);
        wetness[current_tile] = k;
        max_val = std::max(max_val, wetness[current_tile]);
        min_val = std::min(min_val, wetness[current_tile]);
      }
      
      for (size_t i = 0; i < context->map->tiles_count(); ++i) {
        const uint32_t current_tile = i;
        wetness[current_tile] = (wetness[current_tile] - min_val) / (max_val - min_val);
        const float height = context->tile_elevation[current_tile];
        const float heat = context->tile_heat[current_tile];
        wetness[current_tile] = wetness[current_tile] * wetness[current_tile]; //  * heat
        const uint32_t dist_to_water = context->water_distance[current_tile].second;
        if (dist_to_water > 0) {
          const float dist_k = 1.0f - float(dist_to_water > 3 ? 3 : dist_to_water) / float(3);
          wetness[current_tile] = std::min(wetness[current_tile] + wetness[current_tile] * dist_k, 1.0f);
        }
        // мне нужно сделать так чтобы на берегах была повышенная влажность
        // и чтобы вне экватора влажность распределялась более умерено
        // хотя с другой стороны это можно сделать иначе
      }
      
      std::swap(context->tile_wetness, wetness);
    }
    
    size_t compute_moisture::progress() const {
      return state;
    }
    
    size_t compute_moisture::complete_state(const generator_context* context) const {
      
    }
    
    std::string compute_moisture::hint() const {
      return "computing tile moisture";
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
    
    create_biomes::create_biomes(const create_info &info) : pool(info.pool), state(0) {}
    void create_biomes::process(generator_context* context) {
      utils::time_log log(hint());
      state = 0;
      
      std::vector<uint32_t> tile_biome(context->map->tiles_count(), UINT32_MAX);
      utils::submit_works(pool, context->map->tiles_count(), [&tile_biome] (const size_t &start, const size_t &count, const generator_context* context, std::atomic<size_t> &state) {
        for (size_t i = start; i < start+count; ++i) {
          const uint32_t tile_index = i;
          const float elevation = context->tile_elevation[tile_index];
          const float temperature = context->tile_heat[tile_index];
          const float wetness = context->tile_wetness[tile_index];
          const uint32_t biome_id = calcutate_biome2(elevation, temperature, wetness);
          ASSERT(biome_id != UINT32_MAX);
          tile_biome[tile_index] = biome_id;
        }
      }, context, std::ref(state));
      
      std::swap(context->tile_biom, tile_biome);
    }
    
    size_t create_biomes::progress() const {
      return state;
    }
    
    size_t create_biomes::complete_state(const generator_context* context) const {
      return context->map->tiles_count();
    }
    
    std::string create_biomes::hint() const {
      return "setting up biomes";
    }
    
    // че нужно чтобы сгенерировать остальные вещи на карте? мне нужно:
    // провинции, государства, культуры, религии
    // тут проблема заключается в том, что у всех этих вещей есть История
    // провинции рандом флуд фил? 
    // государства, культуры, религии - это 
    // уже что то друг с другом тесно связано
    // культуры это что? некая совокупность людей, 
    // которые на данной территории считают себя одной культурой (+ говорят на одном языке)
    // государства это что? когда люди собрались в кучу и ассоциировались
    // они пытаются "отстаять свои интересы"
    // что такое религии? изначально совокупность непреднамеренных заблуждений народов
    // затем несколько религий целенаправлено стали себя рекламировать чтобы 
    // получить влияние, и в итоге стали распространяться по миру
    // провинции - диаграма вороного
    // государства по идее начинают с одной провинции с течением времени захватывают соседей
    // через какое то время разваливаются
    // религии? с ними сложнее, чаще всего религии несли именно государства, но иногда
    // происходило что-то что заставляли главу государства сменить религию
    // для этого какие религии должны развиваться параллельно войнам
    // для более менее классного развития нужно сделать итерации
    // тем не менее итераций должно быть много
    // культуры? культуры на картах выглядят гораздо проще
    // большая часть культур просто залиты на карту на свободные места
    // некоторые культуры еще что то получают, какой то бонус
    // ко всему прочему бонусы зависят некоторым образом от истории
    
    uint32_t binary_search(const uint32_t &needle, const std::vector<std::pair<uint32_t, uint32_t>> &array) {
      uint32_t high = array.size() - 1;
      uint32_t low = 0;
      
      uint32_t probe = UINT32_MAX;
      while (low < high) {
        probe = (high + low) / 2;
        
        if (array[probe].second < needle) low = probe + 1;
        else if (array[probe].second > needle) high = probe - 1;
        else return probe;
      }
      
      if (low != high) return probe;
      return (array[low].second >= needle) ? low : low + 1;
    }
    
    generate_provinces::generate_provinces(const create_info &info) : pool(info.pool), state(0) {}
    void generate_provinces::process(generator_context* context) {
      utils::time_log log(hint());
      state = 0;
      
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
      
      std::vector<uint32_t> tile_pool(context->map->tiles_count(), UINT32_MAX);
      
      uint32_t water_tiles = 0;
      uint32_t pool_id = 0;
      for (size_t i = 0; i < context->map->tiles_count(); ++i) {
        const float h = context->tile_elevation[i];
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
          
          const auto &tile_data = render::unpack_data(context->map->get_tile(current_tile));
          const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
          for (uint32_t j = 0; j < n_count; ++j) {
            const uint32_t n_index = tile_data.neighbors[j];
            const float h = context->tile_elevation[n_index];
            if (h < 0.0f) continue;
            if (tile_pool[n_index] == UINT32_MAX) {
              tile_pool[n_index] = current_pool_id;
              queue.push(n_index);
            }
          }
        }
      }
      
      std::vector<std::vector<uint32_t>> ground_pools(pool_id);
      for (size_t i = 0; i < context->map->tiles_count(); ++i) {
        const uint32_t pool_index = tile_pool[i];
        if (pool_index == UINT32_MAX) continue;
        ground_pools[pool_index].push_back(i);
      }
      
      // нужно будет еще парочку раз взглянуть на этот способ (с помощью весов)
      std::vector<std::pair<uint32_t, uint32_t>> tile_weights(context->map->tiles_count(), std::make_pair(0, 0));
      uint32_t good_tiles_count = 0;
      for (size_t i = 0; i < context->map->tiles_count(); ++i) {
        const uint32_t current_tile = i;
        const uint32_t biom_id = context->tile_biom[current_tile];
        const float height = context->tile_elevation[current_tile];
        
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
        
        const float t = context->tile_heat[current_tile];
        // просто умножить? с другой стороны зачем это делать 
        // биом дает довольно много информации
        //biom_points = biom_points * t;
        
        //good_tiles_count += uint32_t(biom_points > 0 && t > 0.15f);
        good_tiles_count += uint32_t(height >= 0.0f && t > 0.15f);
        
        tile_weights[current_tile] = std::make_pair(current_tile, biom_points);
      }
      
      const uint32_t province_tiles_avg = float(good_tiles_count) / float(context->data->provinces_count);
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
      std::vector<std::pair<uint32_t, uint32_t>> tile_province(context->map->tiles_count(), std::make_pair(UINT32_MAX, UINT32_MAX));
      std::vector<uint32_t> pool_prov_count(ground_pools.size(), 0);
      std::vector<std::vector<uint32_t>> province_tiles(context->data->provinces_count);
      for (uint32_t i = 0; i < context->data->provinces_count; ++i) {
        uint32_t tile_index = UINT32_MAX;
        uint32_t attempts = 0;
        
        while (tile_index == UINT32_MAX && attempts < 100) {
//           const uint32_t rand_weight = context->data->engine->index(accum);
//           const uint32_t rand_index = binary_search(rand_weight, tile_weights_accum);
          const uint32_t rand_index = context->data->engine->index(tile_weights_accum.size());
          ASSERT(rand_index < tile_weights_accum.size());
          const auto &pair = tile_weights_accum[rand_index];
          ++attempts;
        
          if (unique_tiles.find(pair.first) != unique_tiles.end()) continue;
          
          const float t = context->tile_heat[pair.first];
          if (t <= 0.15f) continue;
          
          const float h = context->tile_elevation[pair.first];
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
          const auto &tile_data = render::unpack_data(context->map->get_tile(pair.first));
          const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
          bool found = true;
          for (uint32_t j = 0; j < n_count; ++j) {
            if (!found) break;
            
            const uint32_t n_index = tile_data.neighbors[j];
            if (unique_tiles.find(n_index) != unique_tiles.end()) {
              found = false;
              break;
            }
            
            const auto &tile_data = render::unpack_data(context->map->get_tile(n_index));
            const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
            for (uint32_t k = 0; k < n_count; ++k) {
              const uint32_t n_index = tile_data.neighbors[k];
              if (unique_tiles.find(n_index) != unique_tiles.end()) {
                found = false;
                break;
              }
            }
            
            const uint32_t dist_to_water = context->water_distance[n_index].second;
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
        
        const auto &tile_data = render::unpack_data(context->map->get_tile(tile_index));
        const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
        for (uint32_t j = 0; j < n_count; ++j) {
          const uint32_t n_index = tile_data.neighbors[j];
          unique_tiles.insert(n_index);
          const auto &tile_data = render::unpack_data(context->map->get_tile(n_index));
          const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
          for (uint32_t k = 0; k < n_count; ++k) {
            const uint32_t n_index = tile_data.neighbors[k];
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
        const auto &tile_data = render::unpack_data(context->map->get_tile(current_tile));
        const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
        for (uint32_t j = 0; j < n_count; ++j) {
          const uint32_t n_index = tile_data.neighbors[j];
          
          const float t = context->tile_heat[n_index];
          if (t <= 0.15f) continue;
          
          const float height = context->tile_elevation[n_index];
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
      uint32_t current_plates_count = context->plate_tile_indices.size();
      uint32_t current_iter = 0;
      
      struct next_provinces_data {
        std::mutex mutex;
        std::set<uint32_t> neighbours;
      };
      
      std::vector<std::vector<uint32_t>> provinces_tiles_local(province_tiles);
      std::vector<std::pair<uint32_t, uint32_t>> tile_provinces_local(tile_province);
      
      while (current_iter < max_iterations) { // current_plates_count > min_plates_count &&
        std::vector<next_provinces_data> next_provinces(province_tiles.size());
        utils::submit_works(pool, context->map->tiles_count(), [&next_provinces, &tile_provinces_local] (const size_t &start, const size_t &count, const generator_context* context) {
          for (size_t i = start; i < start+count; ++i) {
            const auto &tile = render::unpack_data(context->map->get_tile(i));
            const uint32_t province1 = tile_provinces_local[i].first;
            if (province1 == UINT32_MAX) continue;
            for (uint32_t j = 0; j < 6; ++j) {
              const uint32_t tile_neighbour_index = tile.neighbors[j];
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
        }, context);
        
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
        const uint32_t new_provinces_count = (province_tiles[i].size() / province_tiles_avg);
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
            const uint32_t rand_index = context->data->engine->index(tiles_array.size());
            ASSERT(rand_index < tiles_array.size());
            const uint32_t current_tile_index = tiles_array[rand_index];
            ++attempts;
          
            if (unique_tiles.find(current_tile_index) != unique_tiles.end()) continue;
            
            const float t = context->tile_heat[current_tile_index];
            if (t <= 0.15f) continue;
            
            const float h = context->tile_elevation[current_tile_index];
            if (h < 0.0f) continue;
            
            uint32_t dist_to_water_accum = 0;
            const auto &tile_data = render::unpack_data(context->map->get_tile(current_tile_index));
            const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
            bool found = true;
            for (uint32_t j = 0; j < n_count; ++j) {
              if (!found) break;
              
              const uint32_t n_index = tile_data.neighbors[j];
              if (unique_tiles.find(n_index) != unique_tiles.end()) {
                found = false;
                break;
              }
              
              const auto &tile_data = render::unpack_data(context->map->get_tile(n_index));
              const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
              for (uint32_t k = 0; k < n_count; ++k) {
                const uint32_t n_index = tile_data.neighbors[k];
                if (unique_tiles.find(n_index) != unique_tiles.end()) {
                  found = false;
                  break;
                }
              }
              
              const uint32_t dist_to_water = context->water_distance[n_index].second;
              dist_to_water_accum += dist_to_water;
            }
            
            if (dist_to_water_accum <= n_count) continue;
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
            const uint32_t rand_index = context->data->engine->index(tiles_array.size());
            ASSERT(rand_index < tiles_array.size());
            const uint32_t current_tile_index = tiles_array[rand_index];
            ++attempts;
          
            if (unique_tiles.find(current_tile_index) != unique_tiles.end()) continue;
            
            const float t = context->tile_heat[current_tile_index];
            if (t <= 0.15f) continue;
            
            const float h = context->tile_elevation[current_tile_index];
            if (h < 0.0f) continue;
            
            uint32_t dist_to_water_accum = 0;
            const auto &tile_data = render::unpack_data(context->map->get_tile(current_tile_index));
            const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
            bool found = true;
            for (uint32_t j = 0; j < n_count; ++j) {
              if (!found) break;
              
              const uint32_t n_index = tile_data.neighbors[j];
              if (unique_tiles.find(n_index) != unique_tiles.end()) {
                found = false;
                break;
              }
              
              const auto &tile_data = render::unpack_data(context->map->get_tile(n_index));
              const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
              for (uint32_t k = 0; k < n_count; ++k) {
                const uint32_t n_index = tile_data.neighbors[k];
                if (unique_tiles.find(n_index) != unique_tiles.end()) {
                  found = false;
                  break;
                }
              }
              
              const uint32_t dist_to_water = context->water_distance[n_index].second;
              dist_to_water_accum += dist_to_water;
            }
            
            if (dist_to_water_accum <= n_count) continue;
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
          const auto &tile_data = render::unpack_data(context->map->get_tile(current_tile));
          const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
          for (uint32_t j = 0; j < n_count; ++j) {
            const uint32_t n_index = tile_data.neighbors[j];
            
            const float t = context->tile_heat[n_index];
            if (t <= 0.15f) continue;
            
            const float height = context->tile_elevation[n_index];
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
          const auto &tile = render::unpack_data(context->map->get_tile(current_tile_index));
          const uint32_t n_count = render::is_pentagon(tile) ? 5 : 6;
          for (uint32_t k = 0; k < n_count; ++k) {
            const uint32_t n_index = tile.neighbors[k];
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
          
          const auto &tile_data = render::unpack_data(context->map->get_tile(choosen_tile_index));
          const uint32_t n_count = render::is_pentagon(tile_data);
          for (uint32_t j = 0; j < n_count; ++j) {
            const uint32_t n_index = tile_data.neighbors[j];
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
      
      PRINT_VAR("provinces     ", count)
      for (size_t i = 0; i < province_tiles.size(); ++i) {
        if (province_tiles[i].size() == 0) continue;
        if (province_tiles[i].size() == 1) continue;
        context->province_tile.emplace_back();
        std::swap(context->province_tile.back(), province_tiles[i]);
//         for (size_t j = 0; j < province_tiles[i].size(); ++j) {
//           
//         }
      }
      
      context->tile_province.resize(tile_province.size(), UINT32_MAX);
      for (size_t i = 0; i < context->province_tile.size(); ++i) {
        for (size_t j = 0; j < context->province_tile[i].size(); ++j) {
          const uint32_t tile_index = context->province_tile[i][j];
          context->tile_province[tile_index] = i;
        }
        //context->tile_province[i] = tile_province[i].first;
      }
//       std::swap(context->province_tile, province_tiles);
      
      PRINT_VAR("province_min  ", province_min)
      PRINT_VAR("province_max  ", province_max)
      PRINT_VAR("count_more_max", count_more_max)
      PRINT_VAR("count_less_min", count_less_min)
      PRINT_VAR("more_max_avg  ", float(accum_max) / float(count_more_max))
      PRINT_VAR("less_min_avg  ", float(accum_min) / float(count_less_min))
      PRINT_VAR("final_avg     ", float(accum_tiles_count) / float(count))
      
      ASSERT(count == context->province_tile.size());
    }
    
    size_t generate_provinces::progress() const {
      return state;
    }
    
    size_t generate_provinces::complete_state(const generator_context* context) const {
      // что тут?
      // по идее несколько раз нужно заполнить стейт по количеству тайлов
    }
    
    std::string generate_provinces::hint() const {
      return "generating provinces";
    }
    
    void province_postprocessing::process(generator_context* context) {
      utils::time_log log(hint());
      
      // ищем подходящие неразмеченные области
      std::vector<std::pair<uint32_t, uint32_t>> tile_free_area;
      std::vector<std::vector<uint32_t>> free_area_tile;
      uint32_t current_free_area = 0;
      for (size_t i = 0; i < context->map->tiles_count(); ++i) {
        const uint32_t current_tile_index = i;
        const float t = context->tile_heat[current_tile_index];
        if (t <= 0.15f) continue;
        
        const float height = context->tile_elevation[current_tile_index];
        if (height < 0.0f) continue;
        
        if (context->tile_province[current_tile_index] != UINT32_MAX) continue;
        
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
          const auto &tile_data = render::unpack_data(context->map->get_tile(current_tile_index));
          const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
          for (uint32_t j = 0; j < n_count; ++j) {
            const uint32_t n_index = tile_data.neighbors[j];
            if (unique_tiles.find(n_index) != unique_tiles.end()) continue;
            
            const float t = context->tile_heat[current_tile_index];
            if (t <= 0.15f) continue;
            
            const float height = context->tile_elevation[current_tile_index];
            if (height < 0.0f) continue;
            
            ASSERT(context->tile_province[current_tile_index] == UINT32_MAX);
            
            free_area_tile[current_free_area].push_back(n_index);
            tile_free_area.push_back(std::make_pair(current_free_area, n_index));
            unique_tiles.insert(n_index);
            queue.push(std::make_pair(current_free_area, n_index));
          }
        }
      }
      
      size_t good_tiles_count = 0;
      for (size_t i = 0; i < context->map->tiles_count(); ++i) {
        const uint32_t current_tile_index = i;
        const float t = context->tile_heat[current_tile_index];
        if (t <= 0.15f) continue;
        
        const float height = context->tile_elevation[current_tile_index];
        if (height < 0.0f) continue;
        ++good_tiles_count;
      }
      
      const uint32_t province_tiles_avg = float(good_tiles_count) / float(context->data->provinces_count);
      const float province_tiles_ratio = 0.75f;
      const uint32_t province_tiles_min = province_tiles_ratio * province_tiles_avg;
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
        const uint32_t new_province_index = context->province_tile.size();
        context->province_tile.emplace_back();
        for (size_t j = 0; j < free_area_tile[i].size(); ++j) {
          const uint32_t tile_index = free_area_tile[i][j];
          context->tile_province[tile_index] = new_province_index;
          context->province_tile[new_province_index].push_back(tile_index);
        }
      }
      
      const uint32_t small_islands_iteration_count = 15;
      uint32_t small_islands_iteration = 0;
      while (small_islands_iteration_count > small_islands_iteration) {
        ++small_islands_iteration;
        for (size_t i = 0; i < free_area_tile.size(); ++i) {
          if (free_area_tile[i].size() >= province_tiles_min * 0.75f) continue;
          const uint32_t first_index = free_area_tile[i][0];
          if (context->tile_province[first_index] != UINT32_MAX) continue;
          
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
            const auto &tile_data = render::unpack_data(context->map->get_tile(current_tile_index));
            const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
            for (uint32_t j = 0; j < n_count; ++j) {
              const uint32_t n_index = tile_data.neighbors[j];
              if (unique_tiles.find(n_index) != unique_tiles.end()) continue;
              
              if (context->tile_province[n_index] != UINT32_MAX) {
                neighbor_province_index = context->tile_province[n_index];
                break;
              }
              
              queue.push(std::make_pair(n_index, pair.second+1));
              unique_tiles.insert(n_index);
            }
          }
          
          if (neighbor_province_index == UINT32_MAX) continue;
          
          for (size_t j = 0; j < free_area_tile[i].size(); ++j) {
            const uint32_t tile_index = free_area_tile[i][j];
            context->tile_province[tile_index] = neighbor_province_index;
            context->province_tile[neighbor_province_index].push_back(tile_index);
          }
        }
      }
      
//       std::queue<uint32_t> islands_queue;
//       for (size_t i = 0; i < ground_pools.size(); ++i) {
//         if (ground_pools[i].size() < province_tiles_min / 2) continue;
//         
//         uint32_t first_valid = UINT32_MAX;
//         for (size_t j = 0; j < ground_pools[i].size(); ++j) {
//           const uint32_t current_tile = ground_pools[i][j];
//           const float t = context->tile_heat[current_tile];
//           if (t <= 0.15f) continue;
//           first_valid = current_tile;
//           break;
//         }
//         
//         if (first_valid == UINT32_MAX) continue;
//         
//         // на размеченном острове не должно быть пустых тайлов по идее
// #ifndef _NDEBUG
//         {
//           for (size_t j = 0; j < ground_pools[i].size(); ++j) {
//             const uint32_t current_tile = ground_pools[i][j];
//             const float t = context->tile_heat[current_tile];
//             if (t <= 0.15f) continue;
//             ASSERT((tile_province[first_valid].first != UINT32_MAX) == (tile_province[current_tile].first != UINT32_MAX));
//           }
//         }
// #endif
//         
//         if (tile_province[first_valid].first != UINT32_MAX) continue;
//         
//         // нужно разметить этот остров
//         
//         std::unordered_set<uint32_t> unique_tiles;
//         const uint32_t province_count = (ground_pools[i].size() < province_tiles_avg) ? 1 : ground_pools[i].size() / province_tiles_min;
//         for (size_t j = 0; j < province_count; ++j) {
//           uint32_t tile_index = UINT32_MAX;
//           uint32_t attempts = 0;
//           
//           while (tile_index == UINT32_MAX && attempts < 100) {
//   //           const uint32_t rand_weight = context->data->engine->index(accum);
//   //           const uint32_t rand_index = binary_search(rand_weight, tile_weights_accum);
//             const uint32_t rand_index = context->data->engine->index(ground_pools[i].size());
//             ASSERT(rand_index < ground_pools[i].size());
//             const uint32_t current_tile_index = ground_pools[i][rand_index];
//             ++attempts;
//           
//             if (unique_tiles.find(current_tile_index) != unique_tiles.end()) continue;
//             
//             const float t = context->tile_heat[current_tile_index];
//             if (t <= 0.15f) continue;
//             
//             const float h = context->tile_elevation[current_tile_index];
//             if (h < 0.0f) continue;
//             
//             uint32_t dist_to_water_accum = 0;
//             const auto &tile_data = render::unpack_data(context->map->get_tile(current_tile_index));
//             const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
//             bool found = true;
//             for (uint32_t j = 0; j < n_count; ++j) {
//               if (!found) break;
//               
//               const uint32_t n_index = tile_data.neighbours[j];
//               if (unique_tiles.find(n_index) != unique_tiles.end()) {
//                 found = false;
//                 break;
//               }
//               
//               const auto &tile_data = render::unpack_data(context->map->get_tile(n_index));
//               const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
//               for (uint32_t k = 0; k < n_count; ++k) {
//                 const uint32_t n_index = tile_data.neighbours[k];
//                 if (unique_tiles.find(n_index) != unique_tiles.end()) {
//                   found = false;
//                   break;
//                 }
//               }
//               
//               const uint32_t dist_to_water = context->water_distance[n_index].second;
//               dist_to_water_accum += dist_to_water;
//             }
//             
//             if (dist_to_water_accum <= n_count) continue;
//             if (!found) continue;
//             
//             tile_index = current_tile_index;
//           }
//           
//           //ASSERT(tile_index != UINT32_MAX);
//           if (tile_index == UINT32_MAX) continue;
//           
//           const uint32_t new_province_index = province_tiles.size();
//           province_tiles.emplace_back();
//           province_tiles[new_province_index].push_back(tile_index);
//           tile_province[tile_index] = std::make_pair(new_province_index, 0);
//           islands_queue.push(tile_index);
//           unique_tiles.insert(tile_index);
//         }
//       }
//       
//       while (!islands_queue.empty()) {
//         const uint32_t current_tile = islands_queue.front();
//         islands_queue.pop();
// //         std::pop_heap(queue.begin(), queue.end(), heap_func);
// //         const uint32_t current_tile = queue.back();
// //         queue.pop_back();
// //         const uint32_t rand_index = context->data->engine->index(queue.size());
// //         const uint32_t current_tile = queue[rand_index];
// //         queue[rand_index] = queue.back();
// //         queue.pop_back();
//         
//         const uint32_t province_index = tile_province[current_tile].first;
//         const auto &tile_data = render::unpack_data(context->map->get_tile(current_tile));
//         const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
//         for (uint32_t j = 0; j < n_count; ++j) {
//           const uint32_t n_index = tile_data.neighbours[j];
//           
//           const float t = context->tile_heat[n_index];
//           if (t <= 0.15f) continue;
//           
//           const float height = context->tile_elevation[n_index];
//           if (height < 0.0f) continue;
//           if (tile_province[n_index].first != UINT32_MAX) continue;
//           
//           tile_province[n_index] = std::make_pair(province_index, tile_province[current_tile].second + 1);
//           province_tiles[province_index].push_back(n_index);
//           
//           islands_queue.push(n_index);
// //           std::push_heap(queue.begin(), queue.end(), heap_func);
//         }
//       }
//       
//       const uint32_t small_islands_iteration_count = 15;
//       uint32_t small_islands_iteration = 0;
//       while (small_islands_iteration_count > small_islands_iteration) {
//         ++small_islands_iteration;
//         
//         for (size_t i = 0; i < ground_pools.size(); ++i) {
//           if (ground_pools[i].size() >= province_tiles_min / 2) continue; // возможно чутка больше должен быть размер
//           const uint32_t first_tile = ground_pools[i][0];
//           if (tile_province[first_tile].first != UINT32_MAX) continue;
//           
//           // если у нас остались острова нужно проверить ближайшие провки
//           // причем видимо несколько раз
//           
//           std::unordered_set<uint32_t> unique_tiles;
//           std::queue<std::pair<uint32_t, uint32_t>> queue;
//           for (size_t j = 0; j < ground_pools[i].size(); ++j) {
//             const uint32_t current_tile = ground_pools[i][j];
//             ASSERT(tile_province[current_tile].first == UINT32_MAX);
//             unique_tiles.insert(current_tile);
//             queue.push(std::make_pair(current_tile, 0));
//           }
//           
//           uint32_t province_index = UINT32_MAX;
//           while (!queue.empty() && province_index == UINT32_MAX) {
//             const auto pair = queue.front();
//             queue.pop();
//             if (pair.second > 5) continue;
//             
//             const auto &tile_data = render::unpack_data(context->map->get_tile(pair.first));
//             const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
//             for (uint32_t j = 0; j < n_count; ++j) {
//               const uint32_t n_index = tile_data.neighbours[j];
//               if (unique_tiles.find(n_index) != unique_tiles.end()) continue;
//               
//               if (tile_province[n_index].first != UINT32_MAX) {
//                 province_index = tile_province[n_index].first;
//                 break;
//               }
//               
//               unique_tiles.insert(n_index);
//               queue.push(std::make_pair(n_index, pair.second+1));
//             }
//           }
//           
//           if (province_index == UINT32_MAX) continue;
//           
//           for (size_t j = 0; j < ground_pools[i].size(); ++j) {
//             const uint32_t current_tile = ground_pools[i][j];
//             tile_province[current_tile].first = province_index;
//             province_tiles[province_index].push_back(current_tile);
//           }
//         }
//       }
    }
    
    size_t province_postprocessing::progress() const {
      return 1;
    }
    
    size_t province_postprocessing::complete_state(const generator_context* context) const {
      return 1;
    }
    
    std::string province_postprocessing::hint() const {
      return "province postprocessing";
    }
    
    calculating_province_neighbours::calculating_province_neighbours(const create_info &info) : pool(info.pool), state(0) {}
    void calculating_province_neighbours::process(generator_context* context) {
      utils::time_log log(hint());
      state = 0;
      
      struct neighbours_set {
        std::mutex mutex;
        std::set<province_neighbour> neighbours;
      };
      
      std::vector<neighbours_set> province_n(context->province_tile.size());
      
      utils::submit_works(pool, context->map->tiles_count(), [&province_n] (const size_t &start, const size_t &count, const generator_context* context, std::atomic<size_t> &state) {
        for (size_t i = start; i < start+count; ++i) {
          const uint32_t current_tile_index = i;
          const uint32_t current_province_index = context->tile_province[current_tile_index];
          if (current_province_index == UINT32_MAX) continue;
          const auto &tile_data = render::unpack_data(context->map->get_tile(current_tile_index));
          const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
          for (uint32_t j = 0; j < n_count; ++j) {
            const uint32_t n_index = tile_data.neighbors[j];
            const uint32_t n_province_index = context->tile_province[n_index];
            
            if (n_province_index == UINT32_MAX) continue;
            if (current_province_index == n_province_index) continue;
                          
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
      }, context, std::ref(state));
      
      // вот у меня есть все соседи на земле
      // теперь мне нужно найти соседей по морю так, чтобы я не брал соседей на континенте
      // нет, соседи на континенте нужны
      // то есть просто найти соседей на какой то дальности, какая дальность?
      
      // ее нужно както посчитать
      const uint32_t max_neighbour_dist = 5;
      
      utils::submit_works(pool, context->province_tile.size(), [&province_n] (const size_t &start, const size_t &count, const generator_context* context, std::atomic<size_t> &state) {
        for (size_t i = start; i < start+count; ++i) {
          const uint32_t current_province_index = i;
          
          std::queue<std::tuple<uint32_t, uint32_t, uint32_t>> queue;
          std::unordered_set<uint32_t> unique_tiles;
          
          for (size_t j = 0; j < context->province_tile[current_province_index].size(); ++j) {
            const uint32_t tile_index = context->province_tile[current_province_index][j];
            unique_tiles.insert(tile_index);
            
            const auto &tile_data = render::unpack_data(context->map->get_tile(tile_index));
            const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
            for (uint32_t k = 0; k < n_count; ++k) {
              const uint32_t n_index = tile_data.neighbors[k];
              
              const float h = context->tile_elevation[n_index];
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
              const uint32_t n_index = tile_data.neighbors[k];
              
              if (unique_tiles.find(n_index) != unique_tiles.end()) continue;
                          
              const float h = context->tile_elevation[n_index];
              if (h < 0.0f) {
                queue.push(std::make_tuple(n_index, province_index, std::get<2>(tuple)+1));
                unique_tiles.insert(n_index);
              } else {
                const uint32_t found_province_index = context->tile_province[n_index];
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
      }, context, std::ref(state));
      
      context->province_neighbours.resize(context->province_tile.size());
      for (size_t i = 0; i < province_n.size(); ++i) {
        const uint32_t index = i;
        for (const auto &n_index : province_n[index].neighbours) {
          context->province_neighbours[index].push_back(n_index);
        }
      }
    }
    
    size_t calculating_province_neighbours::progress() const { return state; }
    size_t calculating_province_neighbours::complete_state(const generator_context* context) const {
      
    }
    
    std::string calculating_province_neighbours::hint() const {
      return "calcutating province neighbours";
    }
    
    // как сгенерить теперь культуры, религии, государства?
    // культуры почти везде выглядят как диаграма вороного
    // как только заспавнить культуру на островах?
    // религия и государство довольно тесно сплетены между собой 
    // в средние века, как правильно это можно сделать мне?
    // по идее их нужно генерить одновременно
    // то есть задать правила по которым псевдогосударства воюют друг с другом
    // воюют государства сильнее если различается культура и религия
    // теперь как это в кучу все привести
    
    generate_cultures::generate_cultures(const create_info &info) : pool(info.pool), state(0) {}
    void generate_cultures::process(generator_context* context) {
      utils::time_log log(hint());
      state = 0;
      
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
      std::vector<uint32_t> tiles_culture(context->map->tiles_count(), UINT32_MAX);
      std::vector<std::vector<uint32_t>> culture_tiles(culture_groups_count);
      std::unordered_set<uint32_t> unique_tiles;
      
      for (size_t i = 0; i < culture_tiles.size(); ++i) {
        uint32_t tile_index = UINT32_MAX;
        uint32_t attempts = 0;
        
        while (tile_index == UINT32_MAX && attempts < 100) {
          ++attempts;
          
          const uint32_t rand_index = context->data->engine->index(context->map->tiles_count());
          const uint32_t current_tile_index = rand_index;
          
          const float t = context->tile_heat[current_tile_index];
          if (t <= 0.15f) continue;
          
          if (unique_tiles.find(current_tile_index) != unique_tiles.end()) continue;
          
          bool found = true;
          const auto &tile_data = render::unpack_data(context->map->get_tile(current_tile_index));
          const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
          for (uint32_t j = 0; j < n_count; ++j) {
            const uint32_t n_index = tile_data.neighbors[j];
            const float t = context->tile_heat[n_index];
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
            const uint32_t n_index = tile_data.neighbors[j];
            unique_tiles.insert(n_index);
          }
        }
        
        ASSERT(tile_index != UINT32_MAX);
        ++state;
      }
      
      while (!queue.empty()) {
        const uint32_t rand_index = context->data->engine->index(queue.size());
        const uint32_t current_tile_index = queue[rand_index];
        queue[rand_index] = queue.back();
        queue.pop_back();
//         const uint32_t current_tile_index = queue.front();
//         queue.pop();
        
        const uint32_t current_culture_index = tiles_culture[current_tile_index];
        const auto &tile_data = render::unpack_data(context->map->get_tile(current_tile_index));
        const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
        for (uint32_t j = 0; j < n_count; ++j) {
          const uint32_t n_index = tile_data.neighbors[j];
          if (tiles_culture[n_index] != UINT32_MAX) continue;
          
          tiles_culture[n_index] = current_culture_index;
          culture_tiles[current_culture_index].push_back(n_index);
          queue.push_back(n_index);
          ++state;
        }
      }
      
      std::swap(context->tile_culture, tiles_culture);
      std::swap(context->culture_tiles, culture_tiles);
      
      // может быть нужно четко по провинциям раскидать культуры?
    }
    
    size_t generate_cultures::progress() const {
      return state;
    }
    
    size_t generate_cultures::complete_state(const generator_context* context) const {
      return context->map->tiles_count();
    }
    
    std::string generate_cultures::hint() const {
      return "generating cultures";
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
    
    generate_countries::generate_countries(const create_info &info) : pool(info.pool), state(0) {}
    void generate_countries::process(generator_context* context) {
      utils::time_log log(hint());
      state = 0;
      
      std::vector<history_step> history;
      
      // наверное нужно генерировать заного девелопмент и удачу 
      std::vector<uint32_t> province_country(context->province_tile.size(), UINT32_MAX);
      std::vector<std::vector<uint32_t>> country_province(context->province_tile.size());
      std::vector<float> luckness(context->province_tile.size());
      std::vector<float> development(context->province_tile.size());
      for (size_t i = 0; i < province_country.size(); ++i) {
        province_country[i] = i;
        country_province[i].push_back(i);
        luckness[i] = context->data->engine->norm();
        development[i] = context->data->engine->norm();
//         development[i] = development[i] * development[i];
      }
      
      std::vector<float> avg_temp(context->province_tile.size(), 0.0f);
      for (uint32_t i = 0; i < context->province_tile.size(); ++i) {
        for (uint32_t j = 0; j < context->province_tile[i].size(); ++j) {
          const uint32_t tile_index = context->province_tile[i][j];
          const float t = context->tile_heat[tile_index];
          avg_temp[i] += t;
        }
        
        avg_temp[i] /= float(context->province_tile[i].size());
      }
      
      const uint32_t iteration_count = 300; // итераций должно быть явно больше
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
            
            for (size_t k = 0; k < context->province_neighbours[province_index].size(); ++k) {
              const auto n_index = context->province_neighbours[province_index][k];
              
              const uint32_t opposing_country = province_country[n_index.index()];
              if (country_index == opposing_country) continue;
              
              const float n_country_luck = luckness[opposing_country];
              const float n_country_development = development[opposing_country];
              
              const float final_luck = std::max(country_luck - n_country_luck, 0.1f);
              const float final_development = std::max(country_development - n_country_development, 0.1f);
              const float chance = 0.05f * final_luck * avg_t + 0.05 * final_development;
              //const float chance = 0.05f * country_luck * avg_t + 0.05 * country_development;
              const bool probability = context->data->engine->probability(chance);
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
          
          const bool probability = context->data->engine->probability(final_k);
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
              const uint32_t rand_index = context->data->engine->index(local_country.size());
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
            
            const auto &neighbours = context->province_neighbours[current_index.second];
            for (size_t j = 0; j < neighbours.size(); ++j) {
              const auto n_index = neighbours[j];
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
            
            const auto &neighbors = context->province_neighbours[current_province_index];
            for (size_t k = 0; k < neighbors.size(); ++k) {
              const auto n_index = neighbors[k];
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
          
          const auto &neighbors = context->province_neighbours[current_province_index];
          for (size_t k = 0; k < neighbors.size(); ++k) {
            const auto n_index = neighbors[k];
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
          
          for (size_t k = 0; k < context->province_neighbours[province_index].size(); ++k) {
            const auto n_index = context->province_neighbours[province_index][k];
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
          for (size_t k = 0; k < context->province_tile[province_index].size(); ++k) {
            const uint32_t tile_index = context->province_tile[province_index][k];
            const auto &tile_data = render::unpack_data(context->map->get_tile(tile_index));
            const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
            for (size_t c = 0; c < n_count; ++c) {
              const uint32_t n_index = tile_data.neighbors[c];
              
              const float h = context->tile_elevation[n_index];
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
            for (size_t k = 0; k < context->province_neighbours[province_index].size(); ++k) {
              const auto n_index = context->province_neighbours[province_index][k];
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
              
              const auto &neighbors = context->province_neighbours[province_index];
              for (size_t j = 0; j < neighbors.size(); ++j) {
                const auto n_index = neighbors[j];
                
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
              
              const auto &neighbors = context->province_neighbours[province_index];
              for (size_t j = 0; j < neighbors.size(); ++j) {
                const auto n_index = neighbors[j];
                
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
      
      std::swap(context->province_country, province_country);
      std::swap(context->country_province, country_province);
      
      // меня уже более менее устраивает результат
      // на следующих шагах мне нужно как использовать историю сгененированную здесь
      // причем, мне нужно будет определить какие-то государства как нестабильные
      // и как стабильные, при начале игры нестабильные государства должны как то эпично разваливаться
      // а стабильные большие государства должны представлять угрозу для игрока или легким уровнем сложности
      
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
    }
    
    size_t generate_countries::progress() const { return state; }
    size_t generate_countries::complete_state(const generator_context* context) const {
      
    }
    
    std::string generate_countries::hint() const {
      return "generating countries";
    }
    
    update_tile_data::update_tile_data(const create_info &info) : pool(info.pool), state(0) {}
    void update_tile_data::process(generator_context* context) {
      utils::time_log log("tile data updation");
      state = 0;
      
      std::atomic<size_t> water_tile_count(0);
      std::atomic<size_t> living_tiles_count(0);
      utils::submit_works(pool, context->map->tiles_count(), [&water_tile_count, &living_tiles_count] (const size_t &start, const size_t &count, const generator_context* context, std::atomic<size_t> &state) {
        for (size_t i = start; i < start+count; ++i) {
//           const float h = context->tile_elevation[i];
//           const float t = context->tile_heat[i];
// //           const auto &pair = context->tile_air_currents_speeds[i];
//           const float w = context->tile_wetness[i];
//           const uint32_t biome = context->tile_biom[i];
//           const uint32_t province = context->tile_province[i];
//           const uint32_t culture = context->tile_culture[i];
//           const uint32_t country = province == UINT32_MAX ? UINT32_MAX : context->province_country[province];
//           ASSERT(pair.second >= 0.0f);
//           ASSERT(pair.second <= 1.0f);
//           ASSERT(false);
          //context->map->set_tile_height(i, w);
//           context->map->set_tile_height(i, h);
          //context->map->set_tile_biom(i, biome);
          //context->map->set_tile_biom(i, province);
//           context->map->set_tile_biom(i, culture);
          //context->map->set_tile_biom(i, country);
          context->map->set_tile_biom(i, UINT32_MAX);
//           if (h < 0.0f) ++water_tile_count;
//           if (h >= 0.0f && t > 0.25f) ++living_tiles_count;
          ++state;
        }
      }, context, std::ref(state));
      
      PRINT_VAR("final water tile count  ", water_tile_count)
      PRINT_VAR("final water ground ratio", float(water_tile_count) / float(context->map->tiles_count()))
      PRINT_VAR("apropriate weather tiles count", living_tiles_count)
      PRINT_VAR("apropriate weather tiles ratio", float(living_tiles_count) / float(context->map->tiles_count()))
    }
    
    size_t update_tile_data::progress() const {
      return state;
    }
    
    size_t update_tile_data::complete_state(const generator_context* context) const {
      return context->map->tiles_count();
    }
    
    std::string update_tile_data::hint() const {
      return "updating tile data";
    }
    
//     generate_air_whorls::generate_air_whorls(const create_info &info) : pool(info.pool) {}
    void generate_air_whorls::process(generator_context* context) {
      utils::time_log log("generation air whorls");
      state = 0;
      auto engine = context->data->engine;
      float direction = engine->index(2) == 0 ? 1.0f : -1.0f;
      const uint32_t layer_count = engine->closed(context->data->min_air_whorls_layers_count, context->data->max_air_whorls_layers_count);
      const float base_whorl_radius = PI_2 / (2.0f * float(layer_count - 1.0f));
      
      std::vector<air_whorl_t> air_whorls;
      
      // tilt - поворот по оси X
      // расставляем точки воздушных потоков по планете
      // точка генерится поворотом по оси X и Y
      // (похоже на сферические координаты)
      
      {
        const float tilt = engine->closed(0.0f, PI_2 / (2.0f * float(layer_count + 4)));
        const float rotation = engine->closed(0.0f, PI_2);
        const glm::vec3 p1 = glm::rotate(glm::vec3(0.0f, 1.0f, 0.0f), tilt, glm::vec3(1.0f, 0.0f, 0.0f));
        const glm::vec3 p2 = glm::rotate(p1, rotation, glm::vec3(0.0f, 1.0f, 0.0f));
        const glm::vec3 position = p2;
        const float strength = engine->closed(float(PI_2 / 36.0f), PI_2 / 24.0f) * direction;
        const float radius = engine->closed(base_whorl_radius * 0.8f, base_whorl_radius * 1.2f);
        air_whorls.push_back({position, strength, radius});
        ++state;
      }
      
      float layer_offset = 0.0f;
      for (uint32_t i = 0; i < layer_count-2; ++i) {
        direction = -direction;
        layer_offset = layer_offset == 0.0f ? 0.5f : 0.0f;
        const float base_tilt = float(i+1) / float(layer_count - 1) * PI_2 / 2.0f;
        const uint32_t layer_whorl_count = std::ceil(std::sin(base_tilt) * PI_2 / base_whorl_radius);
        for (uint32_t j = 0; j < layer_whorl_count; ++j) {
          const float tilt = engine->closed(0.0f, PI_2 / (2.0f * float(layer_count + 4)));
          const float rotation = engine->closed(0.0f, PI_2);
          const float extra_rotation = PI_2 * (float(j+1) + layer_offset) / float(layer_whorl_count);
          const glm::vec3 p1 = glm::rotate(glm::vec3(0.0f, 1.0f, 0.0f), tilt, glm::vec3(1.0f, 0.0f, 0.0f));
          const glm::vec3 p2 = glm::rotate(p1, rotation, glm::vec3(0.0f, 1.0f, 0.0f));
          const glm::vec3 p3 = glm::rotate(p2, base_tilt, glm::vec3(1.0f, 0.0f, 0.0f));
          const glm::vec3 p4 = glm::rotate(p3, extra_rotation, glm::vec3(0.0f, 1.0f, 0.0f));
          const glm::vec3 position = p4;
          const float strength = engine->closed(float(PI_2 / 48.0f), PI_2 / 32.0f) * direction;
          const float radius = engine->closed(base_whorl_radius * 0.8f, base_whorl_radius * 1.2f);
          air_whorls.push_back({position, strength, radius});
        }
        ++state;
      }
      
      {
        direction = -direction;
        const float tilt = engine->closed(0.0f, PI_2 / (2.0f * float(layer_count + 4)));
        const float rotation = engine->closed(0.0f, PI_2);
        const glm::vec3 p1 = glm::rotate(glm::vec3(0.0f, 1.0f, 0.0f), tilt, glm::vec3(1.0f, 0.0f, 0.0f));
        const glm::vec3 p2 = glm::rotate(p1, rotation, glm::vec3(0.0f, 1.0f, 0.0f));
        const glm::vec3 position = p2;
        const float strength = engine->closed(float(PI_2 / 36.0f), PI_2 / 24.0f) * direction;
        const float radius = engine->closed(base_whorl_radius * 0.8f, base_whorl_radius * 1.2f);
        air_whorls.push_back({position, strength, radius});
        ++state;
      }
      
      std::swap(context->air_whorls, air_whorls);
    }
    
    size_t generate_air_whorls::progress() const {
      return state;
    }
    
    size_t generate_air_whorls::complete_state(const generator_context* context) const {
      return context->data->max_air_whorls_layers_count;
    }
    
    std::string generate_air_whorls::hint() const {
      return "generating air whorls";
    }
    
    float compute_angle(const glm::vec3 &a, const glm::vec3 &b) {
      const float dot = glm::dot(a, b);
      const float len_sq1 = glm::length2(a);
      const float len_sq2 = glm::length2(b);
      return glm::acos(dot / glm::sqrt(len_sq1 * len_sq2));
    }
    
    std::pair<glm::vec3, float> calculate_air_current(const glm::vec3 &pos, const std::vector<air_whorl_t> &air_whorls) {
      glm::vec3 current_sum = glm::vec3(0.0f, 0.0f, 0.0f);
      float weight_sum = 0.0f;
      for (const auto &whorl : air_whorls) {
        const float angle = compute_angle(whorl.pos, pos);
        if (angle < whorl.radius) {
          const float dist = angle / whorl.radius;
          const float weight = 1.0f - dist;
          const float strength = whorl.strength * weight * dist;
          //glm::vec3 current = glm::cross(whorl.pos, pos); // скорее всего тут требуется нормализация
          glm::vec3 current = glm::cross(whorl.pos, glm::normalize(pos));
          const float length = glm::length(current);
          if (length > EPSILON) {
            current *= float(strength / length);
          } else {
            current = glm::vec3(0.0f, 0.0f, 0.0f);
          }
          
          current_sum += current;
          weight_sum += weight;
        }
      }
      
      // weight_sum иногда 0
      if (glm::length2(weight_sum) < EPSILON) current_sum = glm::vec3(0.0f, 0.0f, 0.0f);
      else current_sum /= weight_sum;
      ASSERT(current_sum.x == current_sum.x);
      return std::make_pair(current_sum, glm::length(current_sum));
    }
    
    calculate_vertex_air_current::calculate_vertex_air_current(const create_info &info) : pool(info.pool) {}
    void calculate_vertex_air_current::process(generator_context* context) {
      utils::time_log log("calculation tile air current");
      state = 0;
      std::vector<std::pair<glm::vec3, float>> tile_air_currents_speeds(context->map->tiles_count());
      
      utils::submit_works(pool, context->map->tiles_count(), [&tile_air_currents_speeds] (const size_t &start, const size_t &count, const generator_context* context, std::atomic<size_t> &state) {
        for (size_t i = start; i < start+count; ++i) {
          const auto &tile = context->map->get_tile(i);
          const glm::vec3 pos = context->map->get_point(tile.tile_indices.x);
          tile_air_currents_speeds[i] = calculate_air_current(pos, context->air_whorls);
          ++state;
        }
      }, context, std::ref(state));
      
      std::swap(context->tile_air_currents_speeds, tile_air_currents_speeds);
    }
    
    size_t calculate_vertex_air_current::progress() const {
      return state;
    }
    
    size_t calculate_vertex_air_current::complete_state(const generator_context* context) const {
      return context->map->tiles_count();
    }
    
    std::string calculate_vertex_air_current::hint() const {
      return "calculating tile air current";
    }
    
    calculate_air_current_outflows::calculate_air_current_outflows(const create_info &info) : pool(info.pool) {}
    void calculate_air_current_outflows::process(generator_context* context) {
      utils::time_log log("calculation air current outflows");
      state = 0;
      std::vector<float[6]> tile_air_outflows(context->map->tiles_count());
      
      utils::submit_works(pool, context->map->tiles_count(), [&tile_air_outflows] (const size_t &start, const size_t &count, const generator_context* context, std::atomic<size_t> &state) {
        for (size_t i = start; i < start+count; ++i) {
          const auto &tile = render::unpack_data(context->map->get_tile(i));
          const uint32_t neighbor_count = render::is_pentagon(tile) ? 5 : 6;
          const float speed = context->tile_air_currents_speeds[i].second;
          if (glm::abs(speed) < EPSILON) {
            memset(&tile_air_outflows[i][0], 0, sizeof(float)*6);
            continue;
          }
          
          ASSERT(speed > 0.0f);
          
          const glm::vec3 pos = context->map->get_point(tile.center);
          const glm::vec3 current = context->tile_air_currents_speeds[i].first;
          const glm::vec3 current_dir = current / speed;
          float outflow_sum = 0.0f;
          for (uint32_t j = 0; j < neighbor_count; ++j) {
            const uint32_t neighbor_index = tile.neighbors[j];
            const auto &tile = context->map->get_tile(neighbor_index);
            const glm::vec3 neighbor_pos = context->map->get_point(tile.tile_indices.x);
            const glm::vec3 neighbor_vector = glm::normalize(neighbor_pos - pos);
            const float outflow = glm::dot(neighbor_vector, current_dir);
            
            if (outflow > 0.0f) {
              tile_air_outflows[i][j] = outflow;
              outflow_sum += outflow;
            } else {
              tile_air_outflows[i][j] = 0.0f;
            }
          }
          
          if (outflow_sum > EPSILON) {
            for (uint32_t j = 0; j < neighbor_count; ++j) {
              tile_air_outflows[i][j] /= outflow_sum;
            }
          }
          
          ++state;
        }
      }, context, std::ref(state));
      
      std::swap(context->tile_air_outflows, tile_air_outflows);
    }
    
    size_t calculate_air_current_outflows::progress() const {
      return state;
    }
    
    size_t calculate_air_current_outflows::complete_state(const generator_context* context) const {
      return context->map->tiles_count();
    }
    
    std::string calculate_air_current_outflows::hint() const {
      return "calculating air current outflows";
    }
    
    initialize_circulating_heat::initialize_circulating_heat(const create_info &info) : pool(info.pool) {}
    void initialize_circulating_heat::process(generator_context* context) {
      utils::time_log log("initialization circulating heat");
      state = 0;
      const float inverse_circulation = 0.05f / std::max(EPSILON, context->data->circulation_modifier);
      const float pole_weight = 0.2f;    // наверное можно немного поизменять и посмотреть что будет
      const float equator_weight = 1.0f; // скорее всего трогать не нужно
      const float latitude_weight_range = equator_weight - pole_weight;
      
      std::vector<heat_properties_t> tile_heat_properties(context->map->tiles_count());
      
      utils::submit_works(pool, context->map->tiles_count(), [
        inverse_circulation, 
        pole_weight, 
//         equator_weight, 
        latitude_weight_range, 
        &tile_heat_properties
      ] (const size_t &start, const size_t &count, const generator_context* context, std::atomic<size_t> &state) {
        for (size_t i = start; i < start+count; ++i) {
          const auto &tile = context->map->get_tile(i);
          const glm::vec3 pos = context->map->get_point(tile.tile_indices.x);
          const glm::vec3 norm = glm::normalize(pos);
          float latitude_effect = std::sqrt(1.0f - glm::abs(norm.y)) * latitude_weight_range + pole_weight;
          latitude_effect = latitude_effect * latitude_effect;
          const float elevation = context->tile_elevation[i];
          
          const float average_area = PI / core::map::hex_count_d(core::map::detail_level);              // это число супер маленькое
          const float average_length = std::sqrt(PI / core::map::hex_count_d(core::map::detail_level)); // это неплохое число (0.0025...)
          
          if (elevation > 0.0f) {
            tile_heat_properties[i].absorption = average_length * 10.0f * inverse_circulation; // нужно поиграться с числами
            tile_heat_properties[i].reflectance = average_length * 2.0f * inverse_circulation * (1.0f - latitude_effect * 0.5f);
            tile_heat_properties[i].circulating_heat = average_area * 1500.0f * latitude_effect * context->data->heat_modifier;
          } else {
            tile_heat_properties[i].absorption = average_length * 9.0f * inverse_circulation; // нужно поиграться с числами
            tile_heat_properties[i].reflectance = average_length * 4.0f * inverse_circulation * (1.0f - latitude_effect * 0.5f);
            tile_heat_properties[i].circulating_heat = average_area * 1666.0f * latitude_effect * context->data->heat_modifier;
          }
          
          ++state;
        }
      }, context, std::ref(state));
      
      std::swap(context->tile_heat_properties, tile_heat_properties);
    }
    
    size_t initialize_circulating_heat::progress() const {
      return state;
    }
    
    size_t initialize_circulating_heat::complete_state(const generator_context* context) const {
      return context->map->tiles_count();
    }
    
    std::string initialize_circulating_heat::hint() const {
      return "initializing circulating heat";
    }
    
    propagate_circulating_heat::propagate_circulating_heat(const create_info &info) : pool(info.pool) {}
    void propagate_circulating_heat::process(generator_context* context) {
      utils::time_log log("circulating heat propagation");
      std::vector<float> tile_absorbed_heat(context->map->tiles_count(), 0.0f);
      std::vector<float> new_tile_circulating_heat(context->map->tiles_count(), 0.0f);
      std::mutex mutex;
      
      std::atomic<bool> still_circulating(false);
      do {
        still_circulating = false;
        state = 0;
        utils::submit_works(pool, context->map->tiles_count(), [&tile_absorbed_heat, &new_tile_circulating_heat, &mutex, &still_circulating] (
          const size_t &start, const size_t &count, const generator_context* context, std::atomic<size_t> &state
        ) {
          for (size_t i = start; i < start+count; ++i) {
            float circulating = context->tile_heat_properties[i].circulating_heat;
            if (circulating > 0.0f) {
              const auto &tile = render::unpack_data(context->map->get_tile(i));
              const uint32_t neighbor_count = render::is_pentagon(tile) ? 5 : 6;
              const float absorption = context->tile_heat_properties[i].absorption;
              const float absorbed = std::min(circulating, absorption);
              
              {
                std::unique_lock<std::mutex> lock(mutex);
                tile_absorbed_heat[i] += absorbed;
              }
              
              if (circulating > absorbed) {
                circulating -= absorbed;
                circulating -= context->tile_heat_properties[i].reflectance;
                if (circulating > 0.0f) {
                  const float circulating_elsewhere = std::min(circulating, context->tile_air_currents_speeds[i].second);
                  if (circulating_elsewhere > 0.0f) {
                    circulating -= circulating_elsewhere;
                    bool has_outflow = false;
                    for (uint32_t j = 0; j < neighbor_count; ++j) {
                      const float outflow = context->tile_air_outflows[i][j];
                      if (outflow > EPSILON) {
                        has_outflow = true;
                        const float circulated = circulating_elsewhere * outflow;
                        if (circulated > 0.0f) {
                          std::unique_lock<std::mutex> lock(mutex);
                          new_tile_circulating_heat[tile.neighbors[j]] += circulated;
                        }
                      }
                    }
                    
                    if (has_outflow || absorption > 0.0f) {
                      still_circulating = true;
                      std::unique_lock<std::mutex> lock(mutex);
                      new_tile_circulating_heat[i] += circulating;
                    }
                  }
                }
              }
            }
            
            ++state;
          }
        }, context, std::ref(state));
        
        for (size_t i = 0 ; i < context->tile_heat_properties.size(); ++i) {
          context->tile_heat_properties[i].circulating_heat = new_tile_circulating_heat[i];
        }
        
        memset(new_tile_circulating_heat.data(), 0, sizeof(float)*new_tile_circulating_heat.size());
      } while (still_circulating);
      
      std::swap(context->tile_absorbed_heat, tile_absorbed_heat);
    }
    
    size_t propagate_circulating_heat::progress() const {
      return state;
    }
    
    size_t propagate_circulating_heat::complete_state(const generator_context* context) const {
      return context->map->tiles_count();
    }
    
    std::string propagate_circulating_heat::hint() const {
      return "propagating circulating heat";
    }
    
    calculate_temperatures::calculate_temperatures(const create_info &info) : pool(info.pool) {}
    void calculate_temperatures::process(generator_context* context) {
      utils::time_log log("temperatures calculation");
      state = 0;
      std::vector<float> tile_temperatures(context->map->tiles_count());
      
      utils::submit_works(pool, context->map->tiles_count(), [&tile_temperatures] (const size_t &start, const size_t &count, const generator_context* context, std::atomic<size_t> &state) {
        for (size_t i = start; i < start+count; ++i) {
          const float elevation = context->tile_elevation[i];
          const float absorbed = context->tile_absorbed_heat[i];
          const float average_area = PI / core::map::hex_count_d(core::map::detail_level);
          const float adjusted_elevation = glm::clamp(elevation * 1.25f, 0.0f, 1.0f); // а как же -elevation? (океан)
          const float elevation_effect = (1 - adjusted_elevation * adjusted_elevation) * 0.7f + 0.3f;
          const float normalized_heat = std::min(absorbed / average_area / 1000.0f, 1.0f);
          tile_temperatures[i] = elevation_effect * normalized_heat * 1.6f - 0.6f;
          ++state;
        }
      }, context, std::ref(state));
      
      std::swap(context->tile_temperatures, tile_temperatures);
    }
    
    size_t calculate_temperatures::progress() const {
      return state;
    }
    
    size_t calculate_temperatures::complete_state(const generator_context* context) const {
      return context->map->tiles_count();
    }
    
    std::string calculate_temperatures::hint() const {
      return "temperatures calculation";
    }
    
    initialize_circulating_moisture::initialize_circulating_moisture(const create_info &info) : pool(info.pool), state(0) {}
    void initialize_circulating_moisture::process(generator_context* context) {
      utils::time_log log("circulating moisture initialization");
      state = 0;
      std::vector<moisture_properties_t> tile_moisture_properties(context->map->tiles_count());
      utils::submit_works(pool, context->map->tiles_count(), [&tile_moisture_properties] (const size_t &start, const size_t &count, const generator_context* context, std::atomic<size_t> &state) {
        const float inverse_circulation = 0.05f / std::max(EPSILON, context->data->circulation_modifier);
        const float average_area = PI / core::map::hex_count_d(core::map::detail_level);              // это число супер маленькое
        const float average_length = std::sqrt(PI / core::map::hex_count_d(core::map::detail_level)); // это неплохое число (0.0025...)
        for (size_t i = start; i < start+count; ++i) {
          //const float temperature = context->tile_temperatures[i];
          const float temperature = context->tile_heat[i];
          const float elevation = context->tile_elevation[i];
          const float temperature_effect = (11.0f - glm::clamp(temperature, 0.0f, 1.0f)) / 10.0f;
          if (elevation >= 0.0f) {
            const float elevation_effect = elevation * 0.5f + 1.0f;
            tile_moisture_properties[i].precipitation_rate = average_length * 1.0f * inverse_circulation * temperature_effect * elevation_effect;
            tile_moisture_properties[i].precipitation_max = average_area * 200.0f;
            tile_moisture_properties[i].circulating_moisture = 0.0f;
          } else {
            tile_moisture_properties[i].precipitation_rate = average_length * 1.0f * inverse_circulation * temperature_effect;
            tile_moisture_properties[i].precipitation_max = average_area * 200.0f;
            tile_moisture_properties[i].circulating_moisture = average_area * 100.0f * context->data->precipitation_modifier;
          }
          
          ++state;
        }
      }, context, std::ref(state));
      
      std::swap(context->tile_moisture_properties, tile_moisture_properties);
    }
    
    size_t initialize_circulating_moisture::progress() const {
      return state;
    }
    
    size_t initialize_circulating_moisture::complete_state(const generator_context* context) const {
      return context->map->tiles_count();
    }
    
    std::string initialize_circulating_moisture::hint() const {
      return "initializing circulating moisture";
    }
    
    propagate_circulating_moisture::propagate_circulating_moisture(const create_info &info) : pool(info.pool), state(0) {}
    void propagate_circulating_moisture::process(generator_context* context) {
      utils::time_log log("circulating moisture propagation");
      std::vector<float> tile_precipitation(context->map->tiles_count(), 0.0f);
      std::vector<float> new_tile_circulating_moisture(context->map->tiles_count(), 0.0f);
      std::mutex mutex;
      
      uint32_t iteration = 0;
      std::atomic<bool> still_circulating(false);
      do {
        still_circulating = false;
        state = 0;
        utils::submit_works(pool, context->map->tiles_count(), [&tile_precipitation, &new_tile_circulating_moisture, &mutex, &still_circulating, &iteration] (
          const size_t &start, const size_t &count, const generator_context* context, std::atomic<size_t> &state
        ) {
          for (size_t i = start; i < start+count; ++i) {
            const uint32_t current_tile = i;
            float circulating = context->tile_moisture_properties[current_tile].circulating_moisture;
            if (circulating > 0.0f) {
              const auto &tile = render::unpack_data(context->map->get_tile(i));
              const uint32_t neighbor_count = render::is_pentagon(tile) ? 5 : 6;
              const float precipitation_rate = context->tile_moisture_properties[current_tile].precipitation_rate;
              const float precipitation_max = context->tile_moisture_properties[current_tile].precipitation_max;
              float current_precipitation = 0.0f;
              {
                std::unique_lock<std::mutex> lock(mutex);
                current_precipitation = tile_precipitation[current_tile];
              }
              
              const float precipitated = std::min(std::min(circulating, precipitation_rate), precipitation_max - current_precipitation);
              
              {
                std::unique_lock<std::mutex> lock(mutex);
                tile_precipitation[i] += precipitated;
              }
              
              if (circulating > precipitated) {
                circulating -= precipitated;
                if (circulating > 0.0f) {
                  const float tile_air_speed = context->tile_air_currents_speeds[i].second;
                  const float circulating_elsewhere = std::min(circulating, tile_air_speed * 4.0f);
                  bool has_outflow = false;
                  if (circulating_elsewhere > 0.0f) {
                    circulating -= circulating_elsewhere;
//                     const auto &tile_data = render::unpack_data(context->map->get_tile(current_tile));
                    for (uint32_t j = 0; j < neighbor_count; ++j) {
                      const uint32_t n_index = tile.neighbors[j];
                      const float outflow = context->tile_air_outflows[i][j];
                      if (outflow > 0.0f) {
                        has_outflow = true;
                        const float circulated = circulating_elsewhere * outflow;
                        if (circulated > 0.0f) {
                          std::unique_lock<std::mutex> lock(mutex);
                          new_tile_circulating_moisture[n_index] += circulated;
                        }
                      }
                    }
                  }
                  
                  if (has_outflow || precipitation_rate > 0.0f) {
                    still_circulating = true;
                    std::unique_lock<std::mutex> lock(mutex);
                    new_tile_circulating_moisture[i] += circulating;
                    
//                     if (iteration >= 15) {
//                       ASSERT(false);
//                     }
                  }
                }
              }
            }
            
            ++state;
          }
        }, context, std::ref(state));
        
        for (size_t i = 0 ; i < context->tile_heat_properties.size(); ++i) {
          context->tile_moisture_properties[i].circulating_moisture = new_tile_circulating_moisture[i];
        }
        
        memset(new_tile_circulating_moisture.data(), 0, sizeof(float)*new_tile_circulating_moisture.size());
        ++iteration;
      } while (still_circulating); // && iteration < 5
      
      std::swap(context->tile_precipitation, tile_precipitation);
    }
    
    size_t propagate_circulating_moisture::progress() const {
      return state;
    }
    
    size_t propagate_circulating_moisture::complete_state(const generator_context* context) const {
      return context->map->tiles_count();
    }
    
    std::string propagate_circulating_moisture::hint() const {
      return "propagating circulating moisture";
    }
    
    calculate_wetness::calculate_wetness(const create_info &info) : pool(info.pool), state(0) {}
    void calculate_wetness::process(generator_context* context) {
      utils::time_log log("wetness calculation");
      state = 0;
      std::vector<float> tile_wetness(context->map->tiles_count());
      
      utils::submit_works(pool, context->map->tiles_count(), [&tile_wetness] (const size_t &start, const size_t &count, const generator_context* context, std::atomic<size_t> &state) {
        const float average_area = PI / core::map::hex_count_d(core::map::detail_level);
        for (size_t i = start; i < start+count; ++i) {
          const float precipitation = context->tile_precipitation[i];
          tile_wetness[i] = std::min(precipitation / average_area / 200.0f, 1.0f);
          ++state;
        }
      }, context, std::ref(state));
      
      std::swap(context->tile_wetness, tile_wetness);
    }
    
    size_t calculate_wetness::progress() const {
      return state;
    }
    
    size_t calculate_wetness::complete_state(const generator_context* context) const {
      return context->map->tiles_count();
    }
    
    std::string calculate_wetness::hint() const {
      return "calculating wetness";
    }
    
    // мне кажется нужно сделать все переменные от 0 до 1
    // а так же генерировать температуры всех биомов в летнем режиме
    // ко всему прочему мне необходимо добавить еще одну переменную 
    // магический ресурс?, чтобы генерировать магические биомы
    uint32_t calculate_biom(const float &elevation, const float &temperature, const float &wetness) {
      if (elevation <= 0.0f) { // вода
        
        if (temperature > 0.0f) return render::biome_ocean;
        else return render::biome_ocean_glacier;
        
      } else if (elevation < 0.6f) { // обычная высота
        
        if (temperature > 0.75f) { // очень жарко (пустыни должны появлятся скорее редко чем часто)
          if (wetness < 0.25f) return render::biome_desert;
          else return render::biome_rain_forest;
        } else if (temperature > 0.5f) { // жарковато (болота тоже должны появляться не очень часто)
          if (wetness < 0.25f) return render::biome_rocky;
          else if (wetness < 0.5f) return render::biome_plains;
          else return render::biome_swamp;
        } else if (temperature > 0.0f) { // обычная температура
          if (wetness < 0.25f) return render::biome_plains;
          else if (wetness < 0.5f) return render::biome_grassland;
          else return render::biome_deciduous_forest;
        } else { // низкая температура
          if (wetness < 0.25f) return render::biome_tundra;
          else return render::biome_land_glacier;
        }
        
      } else if (elevation < 0.8f) { // довольно высоко (плоскогорье?)
        
        if (temperature > 0.0f) {
          if (wetness < 0.25f) return render::biome_tundra;
          else return render::biome_conifer_forest;
        } else return render::biome_tundra;
        
      } else { // горы (вулкан? гейзеры?)
        if (temperature > 0.0f || wetness < 0.25f) return render::biome_mountain;
        else return render::biome_snowy_mountain;
      }
    }
    
    calculate_biomes::calculate_biomes(const create_info &info) : pool(info.pool) {}
    void calculate_biomes::process(generator_context* context) {
      utils::time_log log("biomes calculation");
      state = 0;
      std::vector<uint32_t> tile_biom(context->map->tiles_count());
      
      utils::submit_works(pool, context->map->tiles_count(), [&tile_biom] (const size_t &start, const size_t &count, const generator_context* context, std::atomic<size_t> &state) {
        for (size_t i = start; i < start+count; ++i) {
          const float elevation = context->tile_elevation[i];
          const float temperature = context->tile_temperatures[i];
          const float wetness = context->tile_wetness[i];
          tile_biom[i] = calculate_biom(elevation, temperature, wetness);
          context->map->set_tile_biom(i, tile_biom[i]);
          ++state;
        }
      }, context, std::ref(state));
      
      std::swap(context->tile_biom, tile_biom);
    }
    
    size_t calculate_biomes::progress() const {
      return state;
    }
    
    size_t calculate_biomes::complete_state(const generator_context* context) const {
      return context->map->tiles_count();
    }
    
    std::string calculate_biomes::hint() const {
      return "calculating biomes";
    }
  }
}

