#ifndef DEVILS_ENGINE_SCRIPT_INPUT_DATA_H
#define DEVILS_ENGINE_SCRIPT_INPUT_DATA_H

#include <cstddef>

namespace devils_engine {
  namespace script {
    struct input_data { 
      size_t root, prev, current, expected_types, script_type; 
      inline input_data() : root(0), prev(0), current(0), expected_types(0), script_type(SIZE_MAX) {}
    };
  }
}

#endif
