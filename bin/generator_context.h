#ifndef GENERATOR_CONTEXT_H
#define GENERATOR_CONTEXT_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include <unordered_set>
#include "utils/utility.h"

class FastNoise;

namespace devils_engine {
  namespace utils {
    struct random_engine;
    struct random_engine_st;
  }
  
  namespace core {
    struct map;
  }
  
  namespace map {
    struct tectonic_plate_data_t {
      glm::vec3 drift_axis;
      float drift_rate;
      glm::vec3 spin_axis;
      float spin_rate;
      float base_elevation;
      bool oceanic;
    };
    
    struct boundary_stress_t {
      float pressure;
      float shear;
      glm::vec4 pressure_vector;
      glm::vec4 shear_vector;
    };
    
    struct air_whorl_t {
      glm::vec3 pos;
      float strength;
      float radius;
    };
    
    struct heat_properties_t {
      float absorption;
      float reflectance;
      float circulating_heat;
    };
    
    struct moisture_properties_t {
      float precipitation_rate;
      float precipitation_max;
      float circulating_moisture;
    };
    
    struct generator_data {
      utils::random_engine_st* engine;
      FastNoise* noise;
      uint32_t plates_count;
      float ocean_percentage;
      uint32_t blur_plate_boundaries_iteration_count;
      float blur_plate_boundaries_center_weighting;
      uint32_t min_air_whorls_layers_count;
      uint32_t max_air_whorls_layers_count;
      float circulation_modifier;
      float heat_modifier;
      float precipitation_modifier;
      uint32_t provinces_count;
    };
    
    struct province_neighbour {
      uint32_t container;
      
      province_neighbour();
      province_neighbour(const bool across_water, const uint32_t &index);
      province_neighbour(const province_neighbour &another);
      province_neighbour(const uint32_t &native);
      bool across_water() const;
      uint32_t index() const;
      
      bool operator==(const province_neighbour &another) const;
      bool operator!=(const province_neighbour &another) const;
      bool operator<(const province_neighbour &another) const;
      bool operator>(const province_neighbour &another) const;
      bool operator<=(const province_neighbour &another) const;
      bool operator>=(const province_neighbour &another) const;
      province_neighbour & operator=(const province_neighbour &another);
    };
    
    struct history_step {
      enum class type {
        becoming_empire,
        end_of_empire,
        count
      };
      
      type t;
      uint32_t country;
      uint32_t size;
      uint32_t destroy_size;
      uint32_t empire_iteration;
      uint32_t end_iteration;
      std::unordered_set<uint32_t> country_provinces;
    };
    
    struct generator_context {
      const generator_data* data;
      core::map* map;
      std::vector<uint32_t> tile_plate_indices;
      std::vector<std::vector<uint32_t>> plate_tile_indices;
      std::vector<tectonic_plate_data_t> plate_datas;
      std::vector<std::pair<uint32_t, uint32_t>> boundary_edges;
      std::vector<boundary_stress_t> boundary_stresses;
      std::vector<std::pair<uint32_t, float>> edge_index_dist;
      
      std::vector<std::pair<uint32_t, uint32_t>> test_boundary_dist;
      std::vector<uint32_t> test_max_dist;
      
      std::vector<std::pair<uint32_t, float>> mountain_dist;
      std::vector<std::pair<uint32_t, float>> ocean_dist;
      std::vector<std::pair<uint32_t, float>> coastline_dist;
      
      std::vector<float> root_distances;
      std::vector<float> tile_elevation;
      
      std::vector<std::pair<uint32_t, uint32_t>> water_distance;
      std::vector<std::pair<uint32_t, uint32_t>> ground_distance;
      std::vector<std::pair<uint32_t, uint32_t>> mountain_distance;
      
      std::vector<float> tile_heat;
      
      std::vector<air_whorl_t> air_whorls;
      std::vector<std::pair<glm::vec3, float>> tile_air_currents_speeds;
      std::vector<float[6]> tile_air_outflows;
      std::vector<heat_properties_t> tile_heat_properties;
      std::vector<float> tile_absorbed_heat;
      std::vector<float> tile_temperatures;
      std::vector<moisture_properties_t> tile_moisture_properties;
      std::vector<float> tile_precipitation;
      std::vector<float> tile_wetness;
      std::vector<uint32_t> tile_biom;
      
      std::vector<uint32_t> tile_province;
      std::vector<std::vector<uint32_t>> province_tile;
      
      std::vector<uint32_t> tile_culture;
      std::vector<std::vector<uint32_t>> culture_tiles;
      
      std::vector<std::vector<province_neighbour>> province_neighbours;
      std::vector<uint32_t> province_country;
      std::vector<std::vector<uint32_t>> country_province;
      
      std::vector<history_step> history;
      
      size_t memory() const {
        return sizeof(*data) + 
          sizeof(tile_plate_indices[0]) * tile_plate_indices.size() +
          sizeof(plate_tile_indices[0]) * plate_tile_indices.size() + 
          sizeof(plate_datas[0]) * plate_datas.size() +
          sizeof(boundary_edges[0]) * boundary_edges.size() +
          sizeof(boundary_stresses[0]) * boundary_stresses.size() +
          sizeof(edge_index_dist[0]) * edge_index_dist.size() + 
          sizeof(root_distances[0]) * root_distances.size() +
          sizeof(tile_elevation[0]) * tile_elevation.size() +
          sizeof(air_whorls[0]) * air_whorls.size() +
          sizeof(tile_air_currents_speeds[0]) * tile_air_currents_speeds.size() +
          sizeof(tile_air_outflows[0]) * tile_air_outflows.size() +
          sizeof(tile_heat_properties[0]) * tile_heat_properties.size() +
          sizeof(tile_absorbed_heat[0]) * tile_absorbed_heat.size() +
          sizeof(tile_temperatures[0]) * tile_temperatures.size() +
          sizeof(tile_moisture_properties[0]) * tile_moisture_properties.size() +
          sizeof(tile_precipitation[0]) * tile_precipitation.size() +
          sizeof(tile_wetness[0]) * tile_wetness.size() + 
          sizeof(tile_biom[0]) * tile_biom.size();
      }
    };
  }
}

namespace std {
  template <>
  struct hash<devils_engine::map::province_neighbour> {
    size_t operator() (const devils_engine::map::province_neighbour &obj) const {
      return std::hash<uint32_t>()(obj.container);
    }
  };
}

#endif
