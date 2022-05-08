#include "container_view.h"

#include "utils/globals.h"
#include "utils/systems.h"
#include "core/map.h"
#include "map_data.h"
#include "container.h"
#include "targets.h"
#include "defines.h"
#include "utils/constexpr_funcs.h"
#include "battle/map.h"
#include "command_buffer.h"

namespace devils_engine {
  namespace render {
    void container_view::recreate(const uint32_t &width, const uint32_t &height) { c->recreate(width, height); }
    vk::Rect2D container_view::get_render_area(const size_t &id) const { assert(id == 0); return cast(c->size()); }
    size_t container_view::get_number(const size_t &id) const { assert(false); (void)id; return 0; }
    std::tuple<vk::Buffer, size_t> container_view::get_buffer(const size_t &id) const { assert(false); (void)id; return std::make_tuple(nullptr, 0); }
    vk::DescriptorSet container_view::get_descriptor_set(const size_t &id) const {
      switch (id) {
        case string_hash(UNIFORM_BUFFERS_DESCRIPTOR_SET_NAME):          return global::get<render::buffers>()->uniform_set;
        case string_hash(MAP_TILES_BUFFERS_DESCRIPTOR_SET_NAME):        return global::get<systems::map_t>()->map->data->tiles_set;
        case string_hash(BATTLE_MAP_TILES_BUFFERS_DESCRIPTOR_SET_NAME): return *global::get<systems::battle_t>()->map->get_descriptor_set();
        case string_hash(HERALDY_BUFFERS_DESCRIPTOR_SET_NAME):          return global::get<render::buffers>()->heraldy_set;
        case string_hash(BORDERS_BUFFERS_DESCRIPTOR_SET_NAME):          return global::get<render::world_map_buffers>()->border_set;
        case string_hash(TILE_RENDERING_DATA_DESCRIPTOR_SET_NAME):      return global::get<render::world_map_buffers>()->tiles_rendering_data;
        default: throw std::runtime_error("Add new resource id");
      }
      
      return nullptr;
    }
    
    vk::Framebuffer container_view::get_framebuffer(const size_t &id) const { 
      assert(id == string_hash(MAIN_FRAMEBUFFER_NAME));
      return *c->current_buffer();
    }
    
    vk::CommandBuffer container_view::get_command_buffer(const size_t &id) const {
      assert(id == string_hash(ENVIRONMENT_COMMAND_BUFFER_NAME));
      auto ptr = reinterpret_cast<secondary_command_buffer*>(c->secondary_command_buffers);
      return ptr->get_current();
    }
    
    vk::ClearValue container_view::get_clear_value(const size_t &id) const { assert(false); (void)id; return {}; }
    size_t container_view::get_numbers(const size_t &id, const size_t &max, size_t* arr) const { assert(false); (void)id; (void)max; (void)arr; return 0; }
    size_t container_view::get_buffers(const size_t &id, const size_t &max, std::tuple<vk::Buffer, size_t>* arr) const { assert(false); (void)id; (void)max; (void)arr; return 0; }
    size_t container_view::get_descriptor_sets(const size_t &id, const size_t &max, vk::DescriptorSet* sets) const { assert(false); (void)id; (void)max; (void)sets; return 0; }
    size_t container_view::get_framebuffers(const size_t &id, const size_t &max, vk::Framebuffer* framebuffers) const { assert(false); (void)id; (void)max; (void)framebuffers; return 0; }
    size_t container_view::get_command_buffers(const size_t &id, const size_t &max, vk::CommandBuffer* command_buffers) const { assert(false); (void)id; (void)max; (void)command_buffers; return 0; }
    size_t container_view::get_clear_values(const size_t &id, const size_t &max, vk::ClearValue* values) const {
      assert(id == string_hash(MAIN_FRAMEBUFFER_NAME));
      if (max < 2) return SIZE_MAX;
      values[0] = vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f});
      values[1] = vk::ClearDepthStencilValue(1.0f, 0);
      return 2;
    }
    
    size_t container_view::get_signaling(const size_t &max, vk::Semaphore* semaphores_arr, vk::PipelineStageFlags* flags_arr) const {
      if (max == 0) return SIZE_MAX;
      const uint32_t cur_index = c->vlk_window->swapchain.current_frame;
      const auto &frames = c->vlk_window->swapchain.frames;
      semaphores_arr[0] = frames[cur_index].image_available;
      if (flags_arr != nullptr) flags_arr[0] = vk::PipelineStageFlagBits::eTopOfPipe;
      return 1;
    }
  }
}
