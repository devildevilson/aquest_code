#include "slots.h"

#include "stage.h"
#include "context.h"
#include <cstring>
#include "utils/assert.h"

namespace devils_engine {
  namespace render {
    slots::slots() { memset(stages.data(), 0, sizeof(stages[0]) * stages.size()); }
    void slots::set_stage(const uint32_t &slot, stage* ptr) {
      ASSERT(slot < stages.size());
      stages[slot] = ptr;
    }
    
    void slots::clear_slot(const uint32_t &slot) {
      ASSERT(slot < stages.size());
      stages[slot] = nullptr;
    }
    
    void slots::update(container* ctx) {
      for (auto p : stages) { 
        if (p == nullptr) continue;
        p->begin();
      }
      
      for (auto p : stages) { 
        if (p == nullptr) continue;
        p->proccess(ctx); 
      }
    }
    
    void slots::clear() {
      for (auto p : stages) { 
        if (p == nullptr) continue;
        p->clear();
      }
    }
    
    const stage* slots::get_stage(const uint32_t &slot) const {
      return slot < stages.size() ? stages[slot] : nullptr;
    }
  }
}
