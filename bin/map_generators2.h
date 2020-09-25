#ifndef MAP_GENERATORS2_H
#define MAP_GENERATORS2_H

#include "utils/sol.h"

#include "generator_context2.h"
#include <string>
#include <mutex>

// с этим все понятно, но как сохраняться во время игры теперь?
// в принципе все эти вещи сохраняемы вполне легко
// государства - это титулы которыми владеет персонаж
// то есть нужно четко обозначить персонажа и титулы
// и тогда можно легко это дело сериализовать

// скорее всего асинхроно запустить генерацию из главного треда у меня не выйдет
// поэтому придется убрать весь мультитрединг отсюда
// может ли он мне вообще пригодиться в игре?

struct biome_data { // чет пока ничего не приходит в голову
  uint32_t color;
  uint32_t image;
  uint32_t object;
  float density;
};

namespace devils_engine {
  namespace map {
    struct container;
    
    namespace debug {
      namespace entities {
        enum values {
          tile,
          plate,
          edge,
          province,
          culture,
          country,
          title,
        };
      }
      
      namespace properties {
        namespace tile {
          enum values {
            plate_index,
            edge_index,
            edge_dist,
            mountain_index,
            mountain_dist,
            ocean_index,
            ocean_dist,
            coastline_index,
            coastline_dist,
            elevation,
            heat,
            moisture,
            biome,
            province_index,
            culture_id,
            country_index,
            test_value_uint1,
            test_value_uint2,
            test_value_uint3,
            
            count
          };
        }
        
        namespace plate {
          enum values {
            drift_axis,
            drift_axis1,
            drift_axis2,
            drift_rate,
            spin_axis,
            spin_axis1,
            spin_axis2,
            spin_rate,
            base_elevation,
            oceanic,
            count
          };
        }
        
        namespace edge {
          enum values {
            plate0_movement,
            plate0_movement1,
            plate0_movement2,
            plate1_movement,
            plate2_movement1,
            plate3_movement2,
            count
          };
        }
        
        namespace culture {
          enum values {
            count
          };
        }
        
        namespace province {
          enum values {
            country_index,
            title_index,
            count
          };
        }
        
        namespace title {
          enum values {
            parent,
            owner,
            count
          };
        }
      }
    }
    
    namespace generator {
      struct context;
    }
    
    using generator_pair = std::pair<std::string, std::function<void(generator::context*, sol::table&)>>;
    extern const generator_pair default_generator_pairs[];
    
    void map_triangle_add2(const map::container* map, const uint32_t &triangle_index, std::mutex &mutex, std::unordered_set<uint32_t> &unique_tiles, std::vector<uint32_t> &tiles_array);
    
    void update_noise_seed(generator::context* ctx);
    
    void begin(generator::context* ctx, sol::table &table); // эта функция всегда должна вызываться
    void setup_generator(generator::context* ctx, sol::table &table);
    void generate_plates(generator::context* ctx, sol::table &table);
    void generate_plate_datas(generator::context* ctx, sol::table &table);
    void compute_boundary_edges(generator::context* ctx, sol::table &table);
    void compute_plate_boundary_stress(generator::context* ctx, sol::table &table);
    void compute_plate_boundary_distances(generator::context* ctx, sol::table &table);
    void calculate_vertex_elevation(generator::context* ctx, sol::table &table);
    void blur_tile_elevation(generator::context* ctx, sol::table &table);
    void normalize_fractional_values(generator::context* ctx, sol::table &table, const uint32_t &entity_id, const uint32_t &property_id);
    void compute_tile_heat(generator::context* ctx, sol::table &table);
    void compute_tile_distances(generator::context* ctx, sol::table &table, const std::function<bool(const generator::context*, const sol::table&, const uint32_t &)> &predicate, const std::string &key);
    void compute_tile_moisture(generator::context* ctx, sol::table &table);
    void create_biomes(generator::context* ctx, sol::table &table);
    void generate_provinces(generator::context* ctx, sol::table &table);
    void province_postprocessing(generator::context* ctx, sol::table &table);
    void calculating_province_neighbours(generator::context* ctx, sol::table &table);
    void generate_cultures(generator::context* ctx, sol::table &table);
    void generate_countries(generator::context* ctx, sol::table &table);
    void generate_titles(generator::context* ctx, sol::table &table);
    void generate_characters(generator::context* ctx, sol::table &table);
    void generate_tech_level(generator::context* ctx, sol::table &table);
    void generate_cities(generator::context* ctx, sol::table &table);
  }
}

#endif
