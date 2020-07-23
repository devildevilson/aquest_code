#ifndef GENERATOR_SYSTEM_H
#define GENERATOR_SYSTEM_H

#include "map_generator.h"
#include "utils/typeless_container.h"
#include <vector>
#include <atomic>
#include <functional>

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

// короче говоря в контексте должен быть интерфейс вида
// ctx->set_tile_data(utils::id::get("temperature"), 0.03f);
// у всех тайлов, провинций, городов, стран будет набор аттрибутов
// которые мы видимо задаем в конфиге
// сколько у нас есть сущностей которым нужно это задать?
// плита, тайл, провинция, государство, герцогства, королевства, империи,
// города, уникальные объекты, какие-то деревни
// неплохо было бы как-нибудь эти сущности задавать в конфиге
// нужно будет выделить сущности которые существуют только во время генерации
// и те которые будут существовать и дальше

namespace devils_engine {
  namespace core {
    struct map;
  }
  
  namespace systems {
    template <typename CTX>
    class generator {
    public:
      // по идее этого достаточно для генератора
      // должен быть способ задать некоторые данные для отрисовки 
      // и как задать итоговые данные?
      //using part = std::pair<std::string, std::function<void(CTX*, sol::table)>>;
      
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
