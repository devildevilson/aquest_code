#include "generator.h"

namespace devils_engine {
  namespace systems {
    generator::generator() : current_part(0) {}
    void generator::add(const std::string &hint, const std::function<void(map::generator::context*, sol::table&)> &func) {
      parts.push_back(std::make_pair(hint, func));
    }
    
    void generator::generate(map::generator::context* context, sol::table &data) {
      current_part = 0;
      for (const auto &part : parts) {
        part.second(context, data);
        ++current_part;
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
