#ifndef RENDER_H
#define RENDER_H

#include <cstddef>
#include <cstdint>
#include <vector>
#include "stage.h"
#include "target.h"
#include "context.h"
#include "utils/typeless_container.h"

namespace devils_engine {
  namespace systems {
    class render {
    public:
      render(const size_t &container_size);
      ~render();

      template <typename T, typename... Args>
      T* add_stage(Args&&... args) {
        T* ptr = container.create<T>(std::forward<Args>(args)...);
        stages.push_back(ptr);
        return ptr;
      }

      template <typename T, typename... Args>
      T* add_target(Args&&... args) {
        T* ptr = container.create<T>(std::forward<Args>(args)...);
        targets.push_back(ptr);
        return ptr;
      }

      void update(devils_engine::render::context* ctx);
      void clear();
      void recreate(const uint32_t &width, const uint32_t &height);
    private:
      utils::typeless_container container;
      std::vector<devils_engine::render::stage*> stages;
      std::vector<devils_engine::render::target*> targets;
    };
  }
}

#endif
