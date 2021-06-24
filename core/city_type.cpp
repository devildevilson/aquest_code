#include "city_type.h"

namespace devils_engine {
  namespace core {
    const structure city_type::s_type;
    const size_t city_type::maximum_buildings;
    city_type::city_type() : 
      buildings{nullptr}, 
      city_image_top{GPU_UINT_MAX}, 
      city_image_face{GPU_UINT_MAX}, 
      city_icon{GPU_UINT_MAX}, 
      scale(1.0f) 
    { 
      memset(stats.data(), 0, sizeof(stats[0]) * city_stats::count);
    }
  }
}
