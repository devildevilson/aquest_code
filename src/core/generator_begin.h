#ifndef DEVILS_ENGINE_CORE_GENERATOR_BEGIN_H
#define DEVILS_ENGINE_CORE_GENERATOR_BEGIN_H

#include "utils/utility.h"
#include "utils/sol.h"

namespace dt {
  class thread_pool;
}

namespace devils_engine {
  namespace map {
    namespace generator {
      struct context;
    }
  }
  
  namespace core {
    struct map;
    
    void make_tiles(const glm::mat4 &mat1, core::map* map, dt::thread_pool* pool);
    void begin(devils_engine::map::generator::context* ctx, sol::table &table);
  }
}

#endif
