#ifndef RENDER_MODE_CONTAINER_H
#define RENDER_MODE_CONTAINER_H

#include <functional>
#include <unordered_map>
#include <string>

namespace devils_engine {
  namespace render {
    using mode_func = std::function<void()>;
    using mode_container = std::unordered_map<std::string, mode_func>;
    void mode(const std::string &str);
  }
}

#endif
