#ifndef GENERATOR_SYSTEM_H
#define GENERATOR_SYSTEM_H

#include "map_generator.h"
#include "utils/typeless_container.h"
#include <vector>
#include <atomic>

namespace devils_engine {
  namespace core {
    struct map;
  }
  
  namespace systems {
    template <typename CTX>
    class generator {
    public:
      generator(const size_t &container_size) : container(container_size), current_generator(nullptr) {}
      ~generator() {
        for (auto gen : generators) {
          container.destroy(gen);
        }
      }
      
      template <typename T, typename... Args>
      T* add_generator(Args&&... args) {
        T* ptr = container.create<T>(std::forward<Args>(args)...);
        generators.push_back(ptr);
        return ptr;
      }
      
      void generate(CTX* context) {
        for (auto gen : generators) {
          current_generator = gen;
          gen->process(context);
        }
        
        current_generator = nullptr;
      }
      
      size_t current_state() const {
        if (current_generator == nullptr) return 1;
        return current_generator->state();
      }
      
      size_t complete_state(const CTX* context) const {
        if (current_generator == nullptr) return 1;
        return current_generator->complete_state(context);
      }
      
      size_t overall_state() const {
        size_t counter = 0;
        for (auto gen : generators) {
          counter += gen->state();
        }
        
        return counter;
      }
      
      size_t overall_complete_state(const CTX* context) const {
        size_t counter = 0;
        for (auto gen : generators) {
          counter += gen->complete_state(context);
        }
        
        return counter;
      }
      
      std::string current_hint() const {
        if (current_generator == nullptr) return "";
        return current_generator->hint();
      }
    private:
      utils::typeless_container container;
      std::vector<map::generator<CTX>*> generators;
      std::atomic<map::generator<CTX>*> current_generator;
    };
  }
}

#endif
