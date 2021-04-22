#include "generator_system2.h"

#include "utils/globals.h"
#include "utils/systems.h"
#include "utils/progress_container.h"

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
      auto prog = global::get<systems::core_t>()->loading_progress;
      prog->set_max_value(generators.size());
      
      current_step = 0;
      for (const auto &pair : generators) {
        prog->set_value(current_step);
        prog->set_hint2(generators[current_step].first);
        
        pair.second(ctx, table);
        ++current_step;
      }
      
      prog->set_value(current_step);
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
      current_step = 0;
    }
    
    bool generator::finished() const {
      return current_step == generators.size();
    }
  }
}
