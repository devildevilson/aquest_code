#include "proxy_stage.h"

namespace devils_engine {
  namespace render {
    proxy_stage::proxy_stage() : childs(nullptr) {}
    void proxy_stage::begin(resource_provider* ctx) {
      if (childs == nullptr) return;
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        cur->begin(ctx);
      }
    }
    
    bool proxy_stage::process(resource_provider* ctx, vk::CommandBuffer task) {
      if (childs == nullptr) return false;
      
      bool has_update = false;
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        const bool ret = cur->process(ctx, task);
        has_update = has_update || ret;
      }
      
      return has_update;
    }
    
    void proxy_stage::clear() {
      if (childs == nullptr) return;
      
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        cur->clear();
      }
    }
    
    void proxy_stage::set_childs(stage* childs) { this->childs = childs; }
    bool proxy_stage::has_childs() const { return childs != nullptr; }
    stage* proxy_stage::get_childs() const { return childs; }
  }
}
