#ifndef RENDER_STAGE_CONTAINER_H
#define RENDER_STAGE_CONTAINER_H

#include <cstddef>
#include <cstdint>
#include <vector>

#include "utils/typeless_container.h"
// #include "stage.h"
#include "interfaces.h"
#include "target.h"

namespace devils_engine {
  namespace render {
    class stage;
    class target;
    struct container;
    
    class stage_container final : public stage, public target {
    public:
      stage_container(const size_t &container_size);
      virtual ~stage_container();

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
      
      void begin(resource_provider* ctx) override;
      bool process(resource_provider* ctx, vk::CommandBuffer task) override;
      void clear() override;
      void recreate(const uint32_t &width, const uint32_t &height) override;
    protected:
      utils::typeless_container container;
      std::vector<stage*> stages;
      std::vector<target*> targets;
    };
  }
}

#endif
