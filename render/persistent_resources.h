#ifndef PERSISTENT_RESOURCES_H
#define PERSISTENT_RESOURCES_H

namespace yavf {
  class Device;
}

namespace devils_engine {
  namespace render {
    void create_persistent_resources(yavf::Device* device);
  }
}

#endif
