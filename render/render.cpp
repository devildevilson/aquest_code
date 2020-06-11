#include "render.h"

namespace devils_engine {
  namespace systems {
    render::render(const size_t &container_size) : container(container_size) {}
    render::~render() {
      for (auto stage : stages) {
        container.destroy(stage);
      }
      
      for (auto target : targets) {
        container.destroy(target);
      }
    }
    
    void render::update(devils_engine::render::context* ctx) {
      for (auto stage : stages) {
        stage->begin();
      }
      
      for (auto stage : stages) {
        stage->proccess(ctx);
      }
      
      // начинаем рисовать
      // возможно этим займется один из стейджев
    }
    
    void render::clear() {
      for (auto stage : stages) {
        stage->clear();
      }
    }
    
    void render::recreate(const uint32_t &width, const uint32_t &height) {
      for (auto target : targets) {
        target->recreate(width, height);
      }
    }
  }
}
