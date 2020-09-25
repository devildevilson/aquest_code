#include "render.h"

#include "stage.h"
#include "target.h"
#include "context.h"

namespace devils_engine {
  namespace render {
    stage_container::stage_container(const size_t &container_size) : container(container_size) {}
    stage_container::~stage_container() {
      for (auto stage : stages) {
        container.destroy(stage);
      }
      
      for (auto target : targets) {
        container.destroy(target);
      }
    }
    
    void stage_container::begin() {
      for (auto stage : stages) {
        stage->begin();
      }
    }
    
    void stage_container::proccess(devils_engine::render::context* ctx) {
      for (auto stage : stages) {
        stage->proccess(ctx);
      }
      
      // начинаем рисовать
      // возможно этим займется один из стейджев
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
