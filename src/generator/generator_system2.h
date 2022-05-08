#ifndef GENERATOR_SYSTEM2_H
#define GENERATOR_SYSTEM2_H

#include "context2.h"
#include <atomic>

namespace devils_engine {
//   namespace map {
//     struct core;
//   }
  
  namespace systems {
    class generator {
    public:
      void add(const map::generator_pair &pair);
      void add(const std::string &hint, const std::function<void(map::generator::context*, sol::table&)> &func);
      void generate(map::generator::context* ctx, sol::table &table);
      size_t size() const;
      size_t current() const;
      std::string hint() const;
      void clear();
      bool finished() const;
    private:
      std::vector<map::generator_pair> generators;
      std::atomic<size_t> current_step;
    };
  }
}

#endif
