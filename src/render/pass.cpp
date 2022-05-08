#include "pass.h"

#include "utils/constexpr_funcs.h"
#include "vulkan_hpp_header.h"

#include "container.h"

#include <iostream>

namespace devils_engine {
  namespace render {
    pass_container::pass_container(vk::Device device, const vk::RenderPassCreateInfo &info, const std::string_view &str_id, const std::string_view &name) : 
      device(device), 
      native(device.createRenderPass(info)),
      framebuffer_id(string_hash(str_id)) 
    {
      set_name(device, native, std::string(name));
    }
    
    pass_container::~pass_container() {
      device.destroy(native);
    }
    
    void pass_container::begin(resource_provider* ctx) {
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        cur->begin(ctx);
      }
    }
    
    bool pass_container::process(resource_provider* ctx, vk::CommandBuffer task) {
      std::array<vk::ClearValue, 16> values;
      const auto f = ctx->get_framebuffer(framebuffer_id);
      const auto render_area = ctx->get_render_area(0);
      const size_t count = ctx->get_clear_values(framebuffer_id, 16, values.data());
      
      const vk::RenderPassBeginInfo info(native, f, render_area, count, values.data());
      task.beginRenderPass(info, vk::SubpassContents::eInline);
      
      vk::Viewport v(render_area.offset.x, render_area.offset.y, render_area.extent.width, render_area.extent.height, 0.0f, 1.0f);
      task.setViewport(0, v);
      task.setScissor(0, render_area);
      
      bool has_update = false;
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        const bool ret = cur->process(ctx, task);
        has_update = has_update || ret;
      }
      
      task.endRenderPass();
      
      return has_update;
    }
    
    void pass_container::clear() {
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        cur->clear();
      }
    }
    
    vk::RenderPass pass_container::get_handle() const {
      return native;
    }
    
    vk::Framebuffer pass_container::get_framebuffer() const {
      return nullptr;
    }
    
    void pass_container::set_childs(stage* childs) { this->childs = childs; }
    
    pass_framebuffer_container::pass_framebuffer_container(container* cont, const vk::RenderPassCreateInfo &info, const std::string_view &name) :
      cont(cont), native(cont->vulkan->device.createRenderPass(info)), count(cont->swapchain_frames_count())
    {
      assert(count < framebuffers.size());
      set_name(cont->vulkan->device, native, std::string(name));
      for (size_t i = 0; i < count; ++i) {
        const std::initializer_list<vk::ImageView> views = {
          cont->vlk_window->swapchain.images[i].view,
          cont->vlk_window->swapchain.images[i].depth_view
        };
        framebuffers[i] = cont->vulkan->device.createFramebuffer(
          vk::FramebufferCreateInfo(
            {},
            native,
            views,
            cont->vlk_window->surface.extent.width,
            cont->vlk_window->surface.extent.height,
            1
          )
        );
        set_name(cont->vulkan->device, framebuffers[i], "window_framebuffer_"+std::to_string(i));
      }
    }
    
    pass_framebuffer_container::~pass_framebuffer_container() {
      auto device = cont->vulkan->device;
      device.destroy(native);
      for (size_t i = 0; i < count; ++i) {
        device.destroy(framebuffers[i]);
      }
    }
    
    void pass_framebuffer_container::begin(resource_provider* ctx) {
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        cur->begin(ctx);
      }
    }
    
    bool pass_framebuffer_container::process(resource_provider* ctx, vk::CommandBuffer task) {
      std::array<vk::ClearValue, 16> values;
      const auto render_area = ctx->get_render_area(0);
      const auto [cv1, cv2] = cont->clear_values();
      values[0] = cast(cv1);
      values[1] = cast(cv2);
      
      vk::Viewport v(render_area.offset.x, render_area.offset.y, render_area.extent.width, render_area.extent.height, 0.0f, 1.0f);
      task.setViewport(0, v);
      task.setScissor(0, render_area);
      
      const vk::RenderPassBeginInfo info(native, get_framebuffer(), render_area, count, values.data());
      task.beginRenderPass(info, vk::SubpassContents::eInline);
      
      bool has_update = false;
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        const bool ret = cur->process(ctx, task);
        has_update = has_update || ret;
      }
      
      task.endRenderPass();
      
      return has_update;
    }
    
    void pass_framebuffer_container::clear() {
      for (auto cur = childs; cur != nullptr; cur = cur->next) {
        cur->clear();
      }
    }
    
    vk::RenderPass pass_framebuffer_container::get_handle() const {
      return native;
    }
    
    vk::Framebuffer pass_framebuffer_container::get_framebuffer() const {
      return framebuffers[cont->swapchain_current_image()];
    }
    
    void pass_framebuffer_container::recreate(const uint32_t &, const uint32_t &) {
      auto device = cont->vulkan->device;
      for (size_t i = 0; i < count; ++i) {
        device.destroy(framebuffers[i]);
      }
      
      for (size_t i = 0; i < count; ++i) {
        const std::initializer_list<vk::ImageView> views = {
          cont->vlk_window->swapchain.images[i].view,
          cont->vlk_window->swapchain.images[i].depth_view
        };
        
        framebuffers[i] = device.createFramebuffer(
          vk::FramebufferCreateInfo(
            {},
            native,
            views,
            cont->vlk_window->surface.extent.width,
            cont->vlk_window->surface.extent.height,
            1
          )
        );
        set_name(device, framebuffers[i], "window_framebuffer_"+std::to_string(i));
      }
    }
    
    void pass_framebuffer_container::set_childs(stage* childs) { this->childs = childs; }
  }
}
