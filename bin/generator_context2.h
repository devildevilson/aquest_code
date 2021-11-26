#ifndef GENERATOR_CONTEXT_NEW
#define GENERATOR_CONTEXT_NEW

// #include "generator_container.h"
// #include "utils/random_engine.h"
// #include "FastNoise.h"
// #include "map.h"
// #include "utils/thread_pool.h"
#include <cstdint>
#include <unordered_set>
#include "utils/utility.h"

namespace dt {
  class thread_pool;
}

class FastNoiseLite;

namespace devils_engine {
  namespace core {
    struct map;
    struct seasons;
  }
  
  namespace utils {
    struct random_engine_st;
    class localization;
  }
  
  namespace map {
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
    
    namespace generator {
      class container;
      
      struct context {
        map::generator::container* container;
        utils::random_engine_st* random;
        FastNoiseLite* noise;
        core::map* map;
        core::seasons* seasons;
        utils::localization* loc;
        dt::thread_pool* pool;
      };
    }
  }
}
    
#endif
