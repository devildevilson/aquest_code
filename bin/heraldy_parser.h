#ifndef HERALDY_PARSER_H
#define HERALDY_PARSER_H

#include "utils/sol.h"

namespace yavf {
  class Buffer;
}

namespace devils_engine {
  namespace render {
    struct image_controller;
  }
  
  namespace utils {
    class world_serializator;
    
    void add_heraldy_layer(const sol::table &table);
    bool validate_heraldy_layer(const uint32_t &index, const sol::table &table);
    bool validate_heraldy_layer_and_save(const uint32_t &index, sol::this_state lua, const sol::table &table, utils::world_serializator* container);
    // нам нужно сразу пихнуть геральдику в буфер
    // и оставить только мапу строка -> индекс
    void load_heraldy_layers(render::image_controller* controller, const std::vector<sol::table> &heraldy_tables, yavf::Buffer* buffer);
  }
}

#endif
