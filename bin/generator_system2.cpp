#include "generator_system2.h"

namespace devils_engine {
  namespace systems {
    void generator::add(const map::generator_pair &pair) {
      generators.push_back(pair);
    }
    
    void generator::add(const std::string &hint, const std::function<void(map::generator::context*, sol::table&)> &func) {
      generators.push_back(std::make_pair(hint, func));
    }
    
    void generator::generate(map::generator::context* ctx, sol::table &table) {
      // по идее мы должны засунуть генерацию в отдельный поток
      // и нарисовать во время генерации прогресс бар
      current_step = 0;
      for (const auto &pair : generators) {
        pair.second(ctx, table);
        ++current_step;
      }
    }
    
    size_t generator::size() const {
      return generators.size();
    }
    
    size_t generator::current() const {
      return current_step;
    }
    
    std::string generator::hint() const {
      return generators[current_step].first;
    }
    
    void generator::clear() {
      generators.clear();
    }
    
    bool generator::finished() const {
      return current_step == generators.size();
    }
  }
}
