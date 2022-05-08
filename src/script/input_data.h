#ifndef DEVILS_ENGINE_SCRIPT_INPUT_DATA_H
#define DEVILS_ENGINE_SCRIPT_INPUT_DATA_H

#include <cstddef>
#include "utils/bit_field.h"

namespace devils_engine {
  namespace script {
    class interface;
    class container;
    
    
    struct input_data {
      enum class flags {
        dot_is_valid_string_symbol,
        count
      };
      size_t root, prev, current, expected_types, script_type; 
      utils::bit_field<size_t(flags::count)> flags_container;
      inline input_data() : root(0), prev(0), current(0), expected_types(0), script_type(SIZE_MAX) {}
      inline bool get_flag(const flags flag) const { return flags_container.get(size_t(flag)); }
      inline void set_flag(const flags flag, const bool value) { flags_container.set(size_t(flag), value); }
    };
    
    typedef std::tuple<interface*, size_t, size_t> (*init_func_p) (const input_data &input, const sol::object &data, container* cont);
  }
}

#endif
