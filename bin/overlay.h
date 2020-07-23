#ifndef OVERLAY_H
#define OVERLAY_H

#include "utils/utility.h"
//#include "generator_context.h"
#include "generator_system2.h"
#include "generator_context2.h"

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

namespace dt {
  class thread_pool;
}

namespace devils_engine {
  namespace overlay {
    enum class state {
      waiting,
      constructed_generator,
      end
    };
    
    void debug(const uint32_t &picked_tile_index);
    state debug_generator(systems::generator* gen, map::generator::context* context, sol::table &table);
  }
}

#endif
