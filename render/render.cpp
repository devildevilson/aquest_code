#include "render.h"

//#include "stage.h"
#include "target.h"

namespace devils_engine {
  namespace render {
    stage_container::stage_container(const size_t &container_size) : container(container_size, 8) {}
    stage_container::~stage_container() {
      for (auto stage : stages) {
        container.destroy(stage);
      }
      
      for (auto target : targets) {
        container.destroy(target);
      }
    }
    
    void stage_container::begin(resource_provider* ctx) {
      for (auto stage : stages) {
        stage->begin(ctx);
      }
    }
    
    bool stage_container::process(resource_provider* ctx, vk::CommandBuffer task) {
      bool has_update = false;
      for (auto stage : stages) {
        has_update = has_update || stage->process(ctx, task);
      }
      
      return has_update;
    }
    
    void stage_container::clear() {
      for (auto stage : stages) {
        stage->clear();
      }
    }
    
    void stage_container::recreate(const uint32_t &width, const uint32_t &height) {
      for (auto target : targets) {
        target->recreate(width, height);
      }
    }
  }
}
