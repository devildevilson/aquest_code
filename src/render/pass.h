#ifndef DEVILS_ENGINE_RENDER_PASS_H
#define DEVILS_ENGINE_RENDER_PASS_H

#include "interfaces.h"
#include "target.h"
#include <string_view>
#include <array>

// чтобы сделать рендертаргет какой я хочу, мне видимо придется либо делать отдельно копирование в изображение свопчеина
// или задавать какие то серьезные констреинты и тащить за собой кучу данных

namespace devils_engine {
  namespace render {
    struct container;
    
    class pass : public stage {
    public:
      virtual ~pass() = default;
      virtual vk::RenderPass get_handle() const = 0;
      virtual vk::Framebuffer get_framebuffer() const = 0;
    };
    
    class pass_container final : public pass {
    public:
      pass_container(vk::Device device, const vk::RenderPassCreateInfo &info, const std::string_view &str_id, const std::string_view &name = "");
      ~pass_container();
      
      void begin(resource_provider* ctx) override;
      bool process(resource_provider* ctx, vk::CommandBuffer task) override;
      void clear() override;
      vk::RenderPass get_handle() const override;
      vk::Framebuffer get_framebuffer() const override;
      
      void set_childs(stage* childs);
    private:
      vk::Device device;
      vk::RenderPass native;
      size_t framebuffer_id;
      stage* childs;
    };
    
    // нужно создать в пассе фреймбуферы, как их передавать потом в другие места?
    // мне нужно передать помоему только в одно место, поэтому нужно просто добавить метод в pass (?)
    
    class pass_framebuffer_container final : public pass, public target {
    public:
      static const size_t max_framebuffers_count = 4;
      
      pass_framebuffer_container(container* cont, const vk::RenderPassCreateInfo &info, const std::string_view &name = "");
      ~pass_framebuffer_container();
      
      void begin(resource_provider* ctx) override;
      bool process(resource_provider* ctx, vk::CommandBuffer task) override;
      void clear() override;
      vk::RenderPass get_handle() const override;
      vk::Framebuffer get_framebuffer() const override;
      void recreate(const uint32_t &width, const uint32_t &height) override;
      
      void set_childs(stage* childs);
    private:
      container* cont;
      vk::RenderPass native;
      size_t count;
      std::array<vk::Framebuffer, max_framebuffers_count> framebuffers;
      stage* childs;
    };
  }
}

#endif
