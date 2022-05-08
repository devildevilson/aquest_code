#ifndef DEVILS_ENGINE_RENDER_PROXY_STAGE_H
#define DEVILS_ENGINE_RENDER_PROXY_STAGE_H

#include "interfaces.h"

namespace devils_engine {
  namespace render {
    class proxy_stage final : public stage {
    public:
      proxy_stage();
      void begin(resource_provider* ctx) override;
      bool process(resource_provider* ctx, vk::CommandBuffer task) override;
      void clear() override;
      void set_childs(stage* childs);
      bool has_childs() const;
      stage* get_childs() const;
    private:
      stage* childs;
    };
  }
}

#endif
