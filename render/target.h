#ifndef TARGET_H
#define TARGET_H

#include <cstdint>
//#include "yavf.h"

namespace devils_engine {
  namespace render {
    class target {
    public:
      virtual ~target() {}
      virtual void recreate(const uint32_t &width, const uint32_t &height) = 0;
    };
  }
}

#endif
