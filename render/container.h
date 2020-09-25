#ifndef CONTAINER_H
#define CONTAINER_H

#include "utils/typeless_container.h"
#include "context.h"
#include <vector>
#include <string>

namespace yavf {
  class Instance;
  class Device;
  class TaskInterface;
  class CombinedTask;
  class ComputeTask;
  class GraphicTask;
  class TransferTask;
}

namespace devils_engine {
  namespace render {
    class stage_container;
  }
  
  namespace render {
    struct container : public context {
      struct application_info {
        std::string app_name;
        uint32_t app_version;
        std::string engine_name;
        uint32_t engine_version;
        uint32_t api_version;
      };
      
      utils::typeless_container mem;
      yavf::Instance* instance;
      yavf::Device* device;
      struct window* window;
      render::stage_container* render;
      std::vector<yavf::CombinedTask*> tasks;

      container();
      ~container();

      yavf::Instance* create_instance(const std::vector<const char*> &extensions, const application_info* app_info);
      struct window* create_window();
      yavf::Device* create_device();
      render::stage_container* create_system(const size_t &system_container_size);
      void create_tasks();

      yavf::TaskInterface* interface() const override;
      yavf::CombinedTask* combined() const override;
      yavf::ComputeTask* compute() const override;
      yavf::GraphicTask* graphics() const override;
      yavf::TransferTask* transfer() const override;
    };
  }
}

#endif
