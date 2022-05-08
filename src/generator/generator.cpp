#include "generator.h"
#include "render/window.h"
#include "utils/globals.h"
#include "utils/systems.h"
#include "utils/progress_container.h"

namespace devils_engine {
  namespace systems {
    generator::generator() : current_part(0) {}
    void generator::add(const std::string &hint, const std::function<void(map::generator::context*, sol::table&)> &func) {
      parts.push_back(std::make_pair(hint, func));
    }
    
    void generator::generate(map::generator::context* context, sol::table &data) {
      auto prog = global::get<systems::core_t>()->loading_progress;
      prog->set_max_value(parts.size());
      prog->set_value(0);
      prog->set_hint2(parts[0].first);
      
      auto w = global::get<render::window>();
      current_part = 0;
      for (const auto &part : parts) {
        part.second(context, data);
        ++current_part;
        if (w->close()) break;
        
        prog->set_value(current_part);
        prog->set_hint2(parts[current_part].first);
      }
    }
    
    std::string generator::hint() const {
      return parts[current_part].first;
    }
    
    size_t generator::size() const {
      return parts.size();
    }
    
    size_t generator::current() const {
      return current_part;
    }
    
    bool generator::finished() const {
      return current_part == parts.size();
    }
  }
}
