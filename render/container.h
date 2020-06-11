#ifndef CONTAINER_H
#define CONTAINER_H

#include "yavf.h"
#include "render.h"
#include "window.h"
#include "utils/typeless_container.h"
#include "context.h"

namespace devils_engine {
  namespace render {
    struct container : public context {
      utils::typeless_container mem;
      yavf::Instance* instance;
      yavf::Device* device;
      struct window* window;
      systems::render* render;
      std::vector<yavf::CombinedTask*> tasks;

      container();
      ~container();

      yavf::Instance* create_instance(const std::vector<const char*> &extensions, const yavf::Instance::ApplicationInfo* app_info);
      struct window* create_window();
      yavf::Device* create_device();
      systems::render* create_system(const size_t &system_container_size);
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
