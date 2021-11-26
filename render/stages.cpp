#include "stages.h"

#include <array>
#include <iostream>

#include "window.h"
#include "container.h"
#include "targets.h"
#include "image_controller.h"
#include "image_container.h"
#include "makers.h"
#include "map_data.h"

#include "utils/globals.h"
#include "utils/utility.h"
#include "utils/input.h"
#include "utils/systems.h"
#include "utils/frustum.h"
#include "utils/shared_time_constant.h"

#include "bin/interface_context.h"
#include "bin/interface2.h"
#include "bin/map.h"

#include "core/context.h"

#define TILE_RENDER_PIPELINE_LAYOUT_NAME "tile_render_pipeline_layout"
#define TILE_RENDER_PIPELINE_NAME "tile_render_pipeline"

namespace devils_engine {
  namespace render {
    window_next_frame::window_next_frame() {}
    void window_next_frame::begin() {}
    void window_next_frame::proccess(container* ctx) { ctx->next_frame(); }
    void window_next_frame::clear() {}
    void task_begin::begin() {}
    void task_begin::proccess(container* ctx) { 
      auto fence = *ctx->current_frame_fence();
      auto device = ctx->vulkan->device;
      device.resetFences(fence); // имеет смысл резетить прямо перед непосредственным вызовом queue submit
      auto task = ctx->command_buffer();
      // не понимаю что конкретно делает этот флаг, сказано что он освобождает все ресурсы обратно в пул, 
      // это работает как vector clear или как vector clear shrink_to_fit ?
      //task->reset(vk::CommandBufferResetFlagBits::eReleaseResources);
      task->reset(); // попробуем так
      
      const vk::CommandBufferBeginInfo bi(vk::CommandBufferUsageFlagBits::eOneTimeSubmit); 
      task->begin(bi); 
    }
    void task_begin::clear() {}
    void task_end::begin() {}
    void task_end::proccess(container* ctx) { ctx->command_buffer()->end(); }
    void task_end::clear() {}
    render_pass_begin::render_pass_begin(const uint32_t &index) : index(index) {}
    void render_pass_begin::begin() {}
    void render_pass_begin::proccess(container* ctx) {
      
      auto cb = ctx->command_buffer();
      
      cb->setViewport(0, cast(ctx->viewport()));
      cb->setScissor(0, cast(ctx->size()));
      auto render_pass = index == 0 ? *ctx->render_pass() : *ctx->render_pass_objects();
      auto framebuffer = *ctx->current_buffer();
      const auto &values = ctx->clear_values();
      const vk::ClearValue v[] = {cast(std::get<0>(values)), cast(std::get<1>(values))};
      vk::RenderPassBeginInfo bi(
        render_pass,
        framebuffer,
        cast(ctx->size()),
        2, v
      );
      cb->beginRenderPass(bi, vk::SubpassContents::eInline);
    }

    void render_pass_begin::clear() {}

    void render_pass_end::begin() {}
    void render_pass_end::proccess(container* ctx) {
      auto task = ctx->command_buffer();
      task->endRenderPass();
    }

    void render_pass_end::clear() {}
    
    void do_copy_tasks(container* ctx, vk::Event event, const size_t &size, const copy_stage* const* stages) {
      for (size_t i = 0; i < size; ++i) { stages[i]->copy(ctx); }
      auto cb = ctx->command_buffer();
      cb->setEvent(event, vk::PipelineStageFlagBits::eTransfer); // не понимаю когда это вызывается совсем =(
    }
    
    void set_event_cmd(container* ctx, vk::Event event) {
      auto cb = ctx->command_buffer();
      cb->setEvent(event, vk::PipelineStageFlagBits::eTransfer); // не понимаю когда это вызывается совсем =(
//       std::cout << "set_event_cmd" << "\n";
    }
    
    // виндовс не дает использовать базовый offsetof
#define offsetof123(s,m) ((::size_t)&reinterpret_cast<char const volatile&>((((s*)0)->m)))
    
    tile_updater::tile_updater(const create_info &info) : 
      need_copy(false), 
      device(info.device), 
      allocator(info.allocator), 
      copies(new vk::BufferCopy[core::map::hex_count_d(core::map::detail_level)]), 
      map(info.map) 
    {
      const size_t tiles_count = core::map::hex_count_d(core::map::detail_level);
      updates.create(allocator, buffer(align_to(tiles_count * sizeof(update_data_t), 16), vk::BufferUsageFlagBits::eTransferSrc), vma::MemoryUsage::eCpuOnly);
      
#ifdef __linux
      static_assert(sizeof(map_tile_t) - offsetof(render::map_tile_t, texture) == sizeof(update_data_t));
      static_assert(offsetof(render::map_tile_t, texture) - offsetof(render::map_tile_t, texture) == offsetof(update_data_t, texture));
      static_assert(offsetof(render::map_tile_t, color) - offsetof(render::map_tile_t, texture) == offsetof(update_data_t, color));
      static_assert(offsetof(render::map_tile_t, borders_data) - offsetof(render::map_tile_t, texture) == offsetof(update_data_t, borders_data));
      static_assert(offsetof(render::map_tile_t, biome_index) - offsetof(render::map_tile_t, texture) == offsetof(update_data_t, biome_index));
#endif
      
      for (size_t i = 0; i < tiles_count; ++i) {
        const size_t src_offset = i * sizeof(update_data_t);
        const size_t dst_offset = i * sizeof(map_tile_t) + offsetof123(render::map_tile_t, texture);
        const size_t size = sizeof(update_data_t);
        const vk::BufferCopy c{ src_offset, dst_offset, size };
        copies[i] = c;
      }
    }
    
    tile_updater::~tile_updater() {
      updates.destroy(allocator);
      delete [] copies;
    }
    
    void tile_updater::begin() {}
    void tile_updater::proccess(container* ctx) {
      if (!need_copy) return;
      
      const size_t tiles_count = core::map::hex_count_d(core::map::detail_level);
      auto task = ctx->command_buffer();
      task->copyBuffer(updates.handle, map->data->tiles.handle, tiles_count, copies); // это поди будет жутко тяжелое копирование
      need_copy = false;
    }
    
    void tile_updater::clear() {}
    
    // нужно ли тут мьютексами аккуратно закрыть? ну тип надо бы
    
    void tile_updater::update_texture(const uint32_t tile_index, const render::image_t texture) {
      const size_t tiles_count = core::map::hex_count_d(core::map::detail_level);
      assert(tile_index < tiles_count);
      std::unique_lock<std::mutex> lock(map->mutex);
      auto array = reinterpret_cast<update_data_t*>(updates.ptr);
      array[tile_index].texture = texture;
      need_copy = true;
    }
    
    void tile_updater::update_color(const uint32_t tile_index, const render::color_t color) {
      const size_t tiles_count = core::map::hex_count_d(core::map::detail_level);
      assert(tile_index < tiles_count);
      std::unique_lock<std::mutex> lock(map->mutex);
      auto array = reinterpret_cast<update_data_t*>(updates.ptr);
      array[tile_index].color = color;
      need_copy = true;
    }
    
    void tile_updater::update_borders_data(const uint32_t tile_index, const uint32_t borders_data) {
      const size_t tiles_count = core::map::hex_count_d(core::map::detail_level);
      assert(tile_index < tiles_count);
      std::unique_lock<std::mutex> lock(map->mutex);
      auto array = reinterpret_cast<update_data_t*>(updates.ptr);
      array[tile_index].borders_data = borders_data;
      need_copy = true;
    }
    
    void tile_updater::update_biome_index(const uint32_t tile_index, const uint32_t biome_index) {
      const size_t tiles_count = core::map::hex_count_d(core::map::detail_level);
      assert(tile_index < tiles_count);
      std::unique_lock<std::mutex> lock(map->mutex);
      auto array = reinterpret_cast<update_data_t*>(updates.ptr);
      array[tile_index].biome_index = biome_index;
      need_copy = true;
    }
    
    void tile_updater::update_all(const core::context* ctx) {
      std::unique_lock<std::mutex> lock(map->mutex);
      auto array = reinterpret_cast<update_data_t*>(updates.ptr);
      for (size_t i = 0; i < ctx->get_entity_count<core::tile>(); ++i) {
        const auto tile = ctx->get_entity<core::tile>(i);
        array[i].texture = tile->texture;
        array[i].color = tile->color;
        array[i].borders_data = tile->borders_data;
        array[i].biome_index = tile->biome_index;
      }
      
      need_copy = true;
    }
    
    void tile_updater::setup_default(container* ctx) {
      // тут по идее нужно скопировать в обратную сторону, чтобы заполнить текущими значениями
      const size_t tiles_count = core::map::hex_count_d(core::map::detail_level);
      for (size_t i = 0; i < tiles_count; ++i) { std::swap(copies[i].srcOffset, copies[i].dstOffset); }
      
      auto device = ctx->vulkan->device;
      auto pool = ctx->vulkan->transfer_command_pool;
      auto queue = ctx->vulkan->graphics;
      auto fence = ctx->vulkan->transfer_fence;
      
      render::do_command(device, pool, queue, fence, [&] (vk::CommandBuffer task) {
        const vk::CommandBufferBeginInfo b_info(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        task.begin(b_info);
        task.copyBuffer(map->data->tiles.handle, updates.handle, tiles_count, copies);
        task.end();
      });
      
      for (size_t i = 0; i < tiles_count; ++i) { std::swap(copies[i].srcOffset, copies[i].dstOffset); }
    }
    
#define WORLD_MAP_RENDER_DESCRIPTOR_POOL "world_map_render_descriptor_pool"
    
    const size_t half_tiles_count = (core::map::hex_count_d(core::map::detail_level) / 2 + 1);
    const size_t max_indices_count = half_tiles_count * 7; // тут теперь максимальное количество ИНДЕКСОВ
    const size_t max_objects_indices_count = core::map::hex_count_d(core::map::detail_level) * 5;
    tile_optimizer::tile_optimizer(const create_info &info) : 
      device(info.device),
      allocator(info.allocator),
      pool(nullptr),
      layout(nullptr),
      buffers_set(nullptr),
      borders_indices_count(0),
      connections_indices_count(0),
      structures_indices_count(0),
      heraldies_indices_count(0),
      render_borders(false)
    {
      borders_buffer_size = std::min(align_to(max_indices_count*sizeof(uint32_t), 16), align_to(max_objects_indices_count*sizeof(uint32_t), 16));
      connections_buffer_size = std::min(align_to(max_indices_count*sizeof(uint32_t), 16), align_to(max_objects_indices_count*sizeof(uint32_t), 16));
      structures_buffer_size = 16;
      heraldies_buffer_size = 0;
      
      indirect.create(
        allocator, 
        buffer(sizeof(struct indirect_buffer), vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst), 
        vma::MemoryUsage::eCpuOnly,
        "tile_optimizer::indirect"
      );
      
      tiles_indices.create(
        allocator,
        buffer(align_to(half_tiles_count*sizeof(uint32_t), 16), vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferSrc),
        vma::MemoryUsage::eGpuOnly,
        "tile_optimizer::tiles_indices"
      );
      
      borders_indices.create(
        allocator,
        buffer(borders_buffer_size, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eVertexBuffer),
        vma::MemoryUsage::eGpuOnly,
        "tile_optimizer::borders_indices"
      );
      
      walls_indices.create(
        allocator,
        buffer(connections_buffer_size, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eVertexBuffer),
        vma::MemoryUsage::eGpuOnly,
        "tile_optimizer::walls_indices"
      );
      
      objects_indices.create(
        allocator,
        buffer(align_to(max_objects_indices_count*sizeof(uint32_t), 16), vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer),
        vma::MemoryUsage::eGpuOnly,
        "tile_optimizer::objects_indices"
      );
      
      structures_indices.create(
        allocator, buffer(structures_buffer_size, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eIndexBuffer), vma::MemoryUsage::eCpuOnly,
        "tile_optimizer::structures_indices"
      );
      
//       heraldy_indices = device->create(
//         yavf::BufferCreateInfo::buffer(
//           16,
//           VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT), 
//         VMA_MEMORY_USAGE_GPU_ONLY
//       );
      
      assert(indirect.ptr != nullptr);
      auto buffer = reinterpret_cast<struct indirect_buffer*>(indirect.ptr);
      memset(indirect.ptr, 0, sizeof(struct indirect_buffer));
      buffer->padding_hex[0] = core::map::tri_count_d(core::map::accel_struct_detail_level);
      buffer->padding_hex[1] = max_indices_count;
      buffer->heraldies_command.vertexCount = 4;
      
//       PRINT_VAR("triangle_count", buffer->padding1[0])
//       PRINT_VAR("max_indices_count", buffer->padding1[1])
      
      // начиная с этого стейджа, стейджы создаются уже после создания карты
      // а значил сюда можно передать необходимые лайоуты
      
      {
        descriptor_pool_maker dpm(&device);
        pool = dpm.poolSize(vk::DescriptorType::eStorageBuffer, 30).create(WORLD_MAP_RENDER_DESCRIPTOR_POOL);
      }
      
      {
        descriptor_set_layout_maker dlm(&device);
        
        layout = 
          dlm.binding(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute)
             .binding(1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute)
             .binding(2, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute)
             .binding(3, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute)
             .binding(4, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute)
             .binding(5, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute)
             .binding(6, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute)
             .create("tiles_indices_layout");
      }
      
      {
        descriptor_set_maker dm(&device);
        buffers_set = dm.layout(layout).create(pool)[0];
        
        descriptor_set_updater dsu(&device);
        dsu.currentSet(buffers_set);
        dsu.begin(0, 0, vk::DescriptorType::eStorageBuffer).buffer(indirect.handle);
        dsu.begin(1, 0, vk::DescriptorType::eStorageBuffer).buffer(tiles_indices.handle);
        dsu.begin(2, 0, vk::DescriptorType::eStorageBuffer).buffer(borders_indices.handle);
        dsu.begin(3, 0, vk::DescriptorType::eStorageBuffer).buffer(walls_indices.handle);
        dsu.begin(4, 0, vk::DescriptorType::eStorageBuffer).buffer(objects_indices.handle);
        dsu.begin(5, 0, vk::DescriptorType::eStorageBuffer).buffer(structures_indices.handle);
        dsu.update();
      }
      
      auto world_buffers = global::get<systems::map_t>()->world_buffers;
      
      {
        pipeline_layout_maker plm(&device);
        pipe_layout = plm.addDescriptorLayout(info.uniform_layout)
                         .addDescriptorLayout(info.tiles_data_layout)
                         .addDescriptorLayout(layout)
                         .addDescriptorLayout(world_buffers->tiles_rendering_data_layout)
                         .create("tiles_optimizer_pipeline_layout");
      }
      
      {
        const auto mod = create_shader_module(device, global::root_directory() + "shaders/tiles.comp.spv");
        const auto mod1 = create_shader_module(device, global::root_directory() + "shaders/tile_data.comp.spv");
        
        compute_pipeline_maker cpm(&device);
        pipe = cpm.shader(mod.get()).create("tiles_optimizer_pipeline", pipe_layout);
        tile_pipe = cpm.shader(mod1.get()).create("individual_tile_pipeline", pipe_layout);
      }
    }
    
    tile_optimizer::~tile_optimizer() {
      indirect.destroy(allocator);
      tiles_indices.destroy(allocator);
      borders_indices.destroy(allocator);
      walls_indices.destroy(allocator);
      objects_indices.destroy(allocator);
      structures_indices.destroy(allocator);
      device.destroy(pool);
      device.destroy(layout);
      device.destroy(pipe_layout);
      device.destroy(pipe);
    }
    
    void tile_optimizer::begin() {
      auto buffer = reinterpret_cast<struct indirect_buffer*>(indirect.ptr);
      
//       PRINT_VAR("tiles   indices  ", buffer->tiles_command.indexCount)
//       PRINT_VAR("borders indices  ", buffer->borders_command.indexCount)
//       PRINT_VAR("walls   indices  ", buffer->walls_command.indexCount)
      
      auto buffers = global::get<render::buffers>();
      auto camera_data = reinterpret_cast<render::camera_data*>(buffers->uniform.ptr);
      const auto fru = utils::compute_frustum(camera_data->viewproj);
      
      buffer->pen_tiles_command.indexCount    = 18; // 0
      buffer->pen_tiles_command.instanceCount = 0;  // 1
      buffer->pen_tiles_command.firstIndex    = 0;
      buffer->pen_tiles_command.vertexOffset  = 0;
      buffer->pen_tiles_command.firstInstance = 0;
      
      buffer->hex_tiles_command.indexCount    = 21; // 0
      buffer->hex_tiles_command.instanceCount = 0;  // 1
      buffer->hex_tiles_command.firstIndex    = 0;
      buffer->hex_tiles_command.vertexOffset  = 0;
      buffer->hex_tiles_command.firstInstance = 0;
      
      buffer->padding_hex[2] = glm::floatBitsToUint(10000.0f);
      
//       buffer->borders_command.indexCount    = 4; // 0
//       buffer->borders_command.instanceCount = 0; // 1
//       buffer->borders_command.firstIndex    = 0;
//       buffer->borders_command.vertexOffset  = 0;
//       buffer->borders_command.firstInstance = 0;
//       
//       buffer->padding2[2] = UINT32_MAX;
      
      buffer->borders_command.vertexCount   = 4; // 0
      buffer->borders_command.instanceCount = 0; // 1
      buffer->borders_command.firstVertex   = 0;
      buffer->borders_command.firstInstance = 0;
      
      //ASSERT(borders_indices_count != 0);
      //PRINT_VAR("borders_indices_count", borders_indices_count)
      buffer->padding2[1] = borders_indices_count;
      buffer->padding2[2] = render_borders ? UINT32_MAX : 0;
      buffer->padding2[3] = UINT32_MAX;
      
//       buffer->walls_command.indexCount    = 0;
//       buffer->walls_command.instanceCount = 1;
//       buffer->walls_command.firstIndex    = 0;
//       buffer->walls_command.vertexOffset  = 0;
//       buffer->walls_command.firstInstance = 0;
//       
//       buffer->padding3[2] = 1;
      
      buffer->walls_command.vertexCount   = 4;
      buffer->walls_command.instanceCount = 0;
      buffer->walls_command.firstVertex   = 0;
      buffer->walls_command.firstInstance = 0;
      
      buffer->padding3[1] = connections_indices_count;
      buffer->padding3[3] = 1;
      
//       buffer->structures_command.indexCount    = 0;
//       buffer->structures_command.instanceCount = 1;
//       buffer->structures_command.firstIndex    = 0;
//       buffer->structures_command.vertexOffset  = 0;
//       buffer->structures_command.firstInstance = 0;
      
      buffer->structures_command.vertexCount   = 4;
      buffer->structures_command.instanceCount = 0;
      buffer->structures_command.firstVertex   = 0;
      buffer->structures_command.firstInstance = 0;
      
      buffer->padding4[1] = structures_indices_count;
      buffer->padding4[3] = 0;
      
//       buffer->heraldies_command.indexCount    = 0;
//       buffer->heraldies_command.instanceCount = 1;
//       buffer->heraldies_command.firstIndex    = 0;
//       buffer->heraldies_command.vertexOffset  = 0;
//       buffer->heraldies_command.firstInstance = 0;
      
      buffer->heraldies_command.vertexCount   = 4;
      buffer->heraldies_command.instanceCount = 0;
      buffer->heraldies_command.firstVertex   = 0;
      buffer->heraldies_command.firstInstance = 0;
      
      buffer->padding5[1] = heraldies_indices_count;
      
      buffer->dispatch_indirect_command = glm::uvec4(0, 1, 1, 0);
      
      memcpy(&buffer->frustum, &fru, sizeof(buffer->frustum));
      
      for (uint32_t i = 0; i < MAX_BIOMES_COUNT; ++i) {
        buffer->biome_data[i].objects_indirect[0] = 0; // индексы
        buffer->biome_data[i].objects_indirect[1] = 0; // инстансы
        buffer->biome_data[i].objects_indirect[2] = 0; // ферст индекс
        buffer->biome_data[i].objects_indirect[3] = 0; // вертекс оффсет
        buffer->biome_data[i].objects_data[0]     = 0; // ферст инстанс
        buffer->biome_data[i].objects_data[3]     = 0; // нужно ли?
      }
      
//       global::get<systems::map_t>()->world_buffers->clear_renderable();
    }
    
    void tile_optimizer::proccess(container* ctx) {
      auto map_systems = global::get<systems::map_t>();
      if (!map_systems->is_init()) return;
      auto map = map_systems->map;
      
      auto uniform_set = global::get<render::buffers>()->uniform_set;
      auto tiles = map->data->tiles_set;
      auto tiles_rendering_data = map_systems->world_buffers->tiles_rendering_data;
      
      auto task = ctx->command_buffer();
      task->bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipe_layout, 0, {
        uniform_set, 
        tiles, 
        buffers_set,
        tiles_rendering_data
      }, nullptr);
      
      task->bindPipeline(vk::PipelineBindPoint::eCompute, pipe);
      const uint32_t count = ceil(double(core::map::tri_count_d(core::map::accel_struct_detail_level)) / double(work_group_size));
      task->dispatch(count, 1, 1);
      // пайп для тайлов, в котором мы во первых можем проверить выделение, а во вторых собрать данные биомов и границ
      task->bindPipeline(vk::PipelineBindPoint::eCompute, tile_pipe);
      task->dispatchIndirect(indirect.handle, offsetof123(struct tile_optimizer::indirect_buffer, dispatch_indirect_command));
    }
    
    static const utils::frustum default_fru{
      glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), 
      glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), 
      glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), 
      glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), 
      glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), 
      glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)
    };
    
    void tile_optimizer::clear() {
      auto buffer = reinterpret_cast<struct indirect_buffer*>(indirect.ptr);
      buffer->selection_frustum = default_fru;
      buffer->selection_box = {glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)};
    }
    
    vk::Buffer tile_optimizer::indirect_buffer() const {
      return indirect.handle;
    }
    
    vk::Buffer tile_optimizer::tiles_index_buffer() const {
      return tiles_indices.handle;
    }
    
    vk::Buffer tile_optimizer::borders_index_buffer() const {
      return borders_indices.handle;
    }
    
    vk::Buffer tile_optimizer::walls_index_buffer() const {
      return walls_indices.handle;
    }
    
    vk::Buffer tile_optimizer::objects_index_buffer() const {
      return objects_indices.handle;
    }
    
    vk::Buffer tile_optimizer::structures_index_buffer() const {
      return structures_indices.handle;
    }
    
    vk::Buffer tile_optimizer::heraldy_index_buffer() const {
      return heraldy_indices.handle;
    }
    
    vk::DescriptorSet tile_optimizer::get_buffers_set() const {
      return buffers_set;
    }
    
    vk::DescriptorSetLayout tile_optimizer::get_buffers_layout() const {
      return layout;
    }
    
    void tile_optimizer::set_borders_count(const uint32_t &count) {
      const uint32_t indices_count = count;
      const uint32_t final_size = align_to(indices_count*sizeof(uint32_t), 16);
      auto ind_buffer = reinterpret_cast<struct indirect_buffer*>(indirect.ptr);
      borders_indices_count = indices_count;
      ind_buffer->padding2[1] = indices_count;
      if (borders_buffer_size >= final_size) return;
      borders_buffer_size = final_size;
      borders_indices.destroy(allocator);
      borders_indices.create(allocator, buffer(borders_buffer_size, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eVertexBuffer), vma::MemoryUsage::eGpuOnly, "tile_optimizer::borders_indices");
      
      descriptor_set_updater dsu(&device);
      dsu.currentSet(buffers_set).begin(2, 0, vk::DescriptorType::eStorageBuffer).buffer(borders_indices.handle).update();
      
      PRINT_VAR("borders indices_count", indices_count)
    }
    
    void tile_optimizer::set_connections_count(const uint32_t &count) {
      const uint32_t indices_count = count*5;
      const uint32_t final_size = align_to(indices_count*sizeof(uint32_t), 16);
      auto ind_buffer = reinterpret_cast<struct indirect_buffer*>(indirect.ptr);
      connections_indices_count = indices_count;
      ind_buffer->padding3[1] = indices_count;
      if (connections_buffer_size >= final_size) return;
      connections_buffer_size = final_size;
      walls_indices.destroy(allocator);
      walls_indices.create(allocator, buffer(connections_buffer_size, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eVertexBuffer), vma::MemoryUsage::eGpuOnly, "tile_optimizer::walls_indices");
      
      descriptor_set_updater dsu(&device);
      dsu.currentSet(buffers_set).begin(3, 0, vk::DescriptorType::eStorageBuffer).buffer(borders_indices.handle).update();

      PRINT_VAR("walls indices_count", indices_count)
    }
    
    void tile_optimizer::set_max_structures_count(const uint32_t &count) {
      const uint32_t indices_count = count;
      const uint32_t final_size = align_to(indices_count*sizeof(uint32_t), 16);
      auto ind_buffer = reinterpret_cast<struct indirect_buffer*>(indirect.ptr);
      structures_indices_count = indices_count;
      ind_buffer->padding4[1] = indices_count;
      if (structures_buffer_size >= final_size) return;
      structures_buffer_size = final_size;
      structures_indices.destroy(allocator);
      structures_indices.create(allocator, buffer(structures_buffer_size, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eIndexBuffer), vma::MemoryUsage::eCpuOnly, "tile_optimizer::structures_indices");
      
      descriptor_set_updater dsu(&device);
      dsu.currentSet(buffers_set).begin(5, 0, vk::DescriptorType::eStorageBuffer).buffer(borders_indices.handle).update();
    }
    
    void tile_optimizer::set_max_heraldy_count(const uint32_t &count) {
      if (heraldy_indices.handle == vk::Buffer(nullptr)) return; // пока так оставим, индексы геральдики попадают теперь в большой буффер объектов
      const uint32_t indices_count = count;
      const uint32_t final_size = align_to(indices_count*sizeof(uint32_t), 16);
      auto ind_buffer = reinterpret_cast<struct indirect_buffer*>(indirect.ptr);
      heraldies_indices_count = indices_count;
      ind_buffer->padding5[1] = indices_count;
      if (heraldies_buffer_size >= final_size) return;
      heraldies_buffer_size = final_size;
      heraldy_indices.destroy(allocator);
      heraldy_indices.create(allocator, buffer(heraldies_buffer_size, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eIndexBuffer), vma::MemoryUsage::eGpuOnly, "tile_optimizer::heraldy_indices");
      
      descriptor_set_updater dsu(&device);
      dsu.currentSet(buffers_set).begin(6, 0, vk::DescriptorType::eStorageBuffer).buffer(borders_indices.handle).update();
    }
    
    void tile_optimizer::set_biome_tile_count(const std::array<std::pair<uint32_t, uint32_t>, MAX_BIOMES_COUNT> &data) {
      auto buffer = reinterpret_cast<struct indirect_buffer*>(indirect.ptr);
      for (uint32_t i = 0; i < MAX_BIOMES_COUNT; ++i) {
        buffer->biome_data[i].objects_data[1] = data[i].first;  // offset
        buffer->biome_data[i].objects_data[2] = data[i].second; // size
        buffer->biome_data[i].objects_data[3] = 0;
      }
    }
    
    void tile_optimizer::set_border_rendering(const bool value) {
      //auto buffer = reinterpret_cast<struct indirect_buffer*>(indirect->ptr());
      //buffer->padding2[2] = value ? UINT32_MAX : 0;
      render_borders = value;
    }
    
    bool tile_optimizer::is_rendering_border() const {
      //auto buffer = reinterpret_cast<struct indirect_buffer*>(indirect->ptr());
      //return !bool(buffer->padding2[2]);
      return render_borders;
    }
    
    void tile_optimizer::set_selection_box(const aabb_t &box) {
      auto buffer = reinterpret_cast<struct indirect_buffer*>(indirect.ptr);
      buffer->selection_box = box;
    }
    
    void tile_optimizer::set_selection_frustum(const utils::frustum &frustum) {
      auto buffer = reinterpret_cast<struct indirect_buffer*>(indirect.ptr);
      buffer->selection_frustum = frustum;
    }
    
    uint32_t tile_optimizer::get_borders_indices_count() const { return borders_indices_count; }
    uint32_t tile_optimizer::get_connections_indices_count() const { return connections_indices_count; }
    uint32_t tile_optimizer::get_structures_indices_count() const { return structures_indices_count; }
    uint32_t tile_optimizer::get_heraldies_indices_count() const { return heraldies_indices_count; }
    
    uint32_t tile_optimizer::get_selection_count() const {
      auto buffer = reinterpret_cast<struct indirect_buffer*>(indirect.ptr);
      return buffer->heraldies_command.instanceCount;
    }
    
    const uint32_t* tile_optimizer::get_selection_data() const {
      return reinterpret_cast<const uint32_t*>(structures_indices.ptr);
    }
    
    tile_objects_optimizer::tile_objects_optimizer(const create_info &info) : 
      device(info.device), 
      allocator(info.allocator), 
      opt(info.opt) 
    {
      auto tiles_indices_layout = opt->get_buffers_layout();
      
      {
        pipeline_layout_maker plm(&device);
        p_layout = plm.addDescriptorLayout(info.uniform_layout)
                      .addDescriptorLayout(info.tiles_data_layout)
                      .addDescriptorLayout(tiles_indices_layout)
                      .create("tile_objects_optimizer_pipeline_layout");
      }
      
      {
        const auto mod = create_shader_module(device, global::root_directory() + "shaders/map_objects.comp.spv");
        
        compute_pipeline_maker cpm(&device);
        pipe = cpm.shader(mod.get()).create("tile_objects_optimizer_pipeline", p_layout);
      }
    }
    
    tile_objects_optimizer::~tile_objects_optimizer() {
      device.destroy(pipe);
      device.destroy(p_layout);
    }
    
    void tile_objects_optimizer::begin() {
      //auto indirect = opt->indirect_buffer();
      //auto buffer = reinterpret_cast<struct tile_optimizer::indirect_buffer*>(indirect->ptr());
      
//       PRINT_VAR("tiles   indices  ", buffer->tiles_command.indexCount)
//       PRINT_VAR("borders indices  ", buffer->borders_command.indexCount)
//       PRINT_VAR("walls   indices  ", buffer->walls_command.indexCount)
      
//       buffer->tiles_command.indexCount    = 0;
//       buffer->tiles_command.instanceCount = 1;
//       buffer->tiles_command.firstIndex    = 0;
//       buffer->tiles_command.vertexOffset  = 0;
//       buffer->tiles_command.firstInstance = 0;
//       
      //buffer->padding1[2] = glm::floatBitsToUint(10000.0f);
      
//       buffer->borders_command.indexCount    = 0;
//       buffer->borders_command.instanceCount = 1;
//       buffer->borders_command.firstIndex    = 0;
//       buffer->borders_command.vertexOffset  = 0;
//       buffer->borders_command.firstInstance = 0;
      
      //buffer->padding2[2] = UINT32_MAX;
      
//       buffer->walls_command.indexCount    = 0;
//       buffer->walls_command.instanceCount = 1;
//       buffer->walls_command.firstIndex    = 0;
//       buffer->walls_command.vertexOffset  = 0;
//       buffer->walls_command.firstInstance = 0;
      
      //buffer->padding3[2] = 1;
      
//       buffer->structures_command.indexCount    = 0;
//       buffer->structures_command.instanceCount = 1;
//       buffer->structures_command.firstIndex    = 0;
//       buffer->structures_command.vertexOffset  = 0;
//       buffer->structures_command.firstInstance = 0;
      
      //buffer->padding4[2] = 0;
    }
    
    void tile_objects_optimizer::proccess(container* ctx) {
      auto map_systems = global::get<systems::map_t>();
      if (!map_systems->is_init()) return;
      auto map = map_systems->map;
      //if (map->status() != core::map::status::valid) return;
//       if (map->status() == core::map::status::initial) return;
      
      auto uniform = global::get<render::buffers>()->uniform_set;
      auto tiles = map->data->tiles_set;
      auto set = opt->get_buffers_set();
      auto indirect = opt->indirect_buffer();
//       auto points = global::get<render::buffers>()->points;
      
      auto task = ctx->command_buffer();
      
//       task->setBarrier();
      
//       task->setBarrier(VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
//                        VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT);
      
      struct update_buffer_data {
        vk::DrawIndirectCommand borders_command;
        uint32_t padding2[4];
        vk::DrawIndirectCommand walls_command;
        uint32_t padding3[4];
        vk::DrawIndirectCommand structures_command;
        uint32_t padding4[4];
        vk::DrawIndirectCommand heraldies_command;
        uint32_t padding5[4];
      };
      
      static_assert(sizeof(update_buffer_data) % 4 == 0);
      static_assert(std::is_same_v<decltype(tile_optimizer::indirect_buffer::borders_command),    decltype(update_buffer_data::borders_command)>);
      static_assert(std::is_same_v<decltype(tile_optimizer::indirect_buffer::walls_command),      decltype(update_buffer_data::walls_command)>);
      static_assert(std::is_same_v<decltype(tile_optimizer::indirect_buffer::structures_command), decltype(update_buffer_data::structures_command)>);
      static_assert(std::is_same_v<decltype(tile_optimizer::indirect_buffer::heraldies_command),  decltype(update_buffer_data::heraldies_command)>);
      static_assert(sizeof(tile_optimizer::indirect_buffer::padding2) == sizeof(update_buffer_data::padding2));
      static_assert(sizeof(tile_optimizer::indirect_buffer::padding3) == sizeof(update_buffer_data::padding3));
      static_assert(sizeof(tile_optimizer::indirect_buffer::padding4) == sizeof(update_buffer_data::padding4));
      static_assert(sizeof(tile_optimizer::indirect_buffer::padding5) == sizeof(update_buffer_data::padding5));
      
      const size_t update_data_start = offsetof123(struct tile_optimizer::indirect_buffer, borders_command);
      
      const update_buffer_data udata{
        { 4, 0, 0, 0 },
        { 0, opt->get_borders_indices_count(), opt->is_rendering_border() ? UINT32_MAX : 0, UINT32_MAX },
        { 4, 0, 0, 0 },
        { 0, opt->get_connections_indices_count(), 0, 1 },
        { 4, 0, 0, 0 },
        { 0, opt->get_structures_indices_count(), 0, 0 },
        { 4, 0, 0, 0 },
        { 0, opt->get_heraldies_indices_count(), 0, 0 }
      };
      
      task->updateBuffer(indirect, update_data_start, sizeof(update_buffer_data), &udata);
      
      const vk::MemoryBarrier mem_b(vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead);
      task->pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eComputeShader, vk::DependencyFlagBits::eByRegion, 
        mem_b, nullptr, nullptr
      );
      
      task->bindPipeline(vk::PipelineBindPoint::eCompute, pipe);
      task->bindDescriptorSets(vk::PipelineBindPoint::eCompute, p_layout, 0, { uniform, tiles, set }, nullptr);
      task->dispatchIndirect(indirect, offsetof123(struct tile_optimizer::indirect_buffer, dispatch_indirect_command));
    }
    
    void tile_objects_optimizer::clear() {
// #ifndef _NDEBUG
//       auto indirect = opt->indirect_buffer();
//       auto buffer = reinterpret_cast<struct tile_optimizer::indirect_buffer*>(indirect.ptr);
//       ASSERT(buffer->padding3[3] == 1);
// #endif
    }
    
//     tile_borders_optimizer::tile_borders_optimizer(const create_info &info) : 
//       device(info.device), 
//       allocator(info.allocator),
//       map_buffers(info.map_buffers) 
//     {
//       indirect.create(allocator, buffer(sizeof(struct indirect_buffer), vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eStorageBuffer), vma::MemoryUsage::eCpuOnly);
//       
//       yavf::DescriptorPool pool = device->descriptorPool(WORLD_MAP_RENDER_DESCRIPTOR_POOL);
//       if (pool == VK_NULL_HANDLE) {
//         yavf::DescriptorPoolMaker dpm(device);
//         pool = dpm.poolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 30).create(WORLD_MAP_RENDER_DESCRIPTOR_POOL);
//       }
//       
//       yavf::DescriptorSetLayout storage_buffer_layout = device->setLayout(STORAGE_BUFFER_LAYOUT_NAME);
//       yavf::DescriptorSetLayout tiles_indices_layout = VK_NULL_HANDLE;
//       {
//         yavf::DescriptorLayoutMaker dlm(device);
//         tiles_indices_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
//                                   .binding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
//                                   .create("borders_indices_layout");
//       }
//       
//       {
//         yavf::DescriptorMaker dm(device);
//         set = dm.layout(tiles_indices_layout).create(pool)[0];
//         set->resize(2);
// //         size_t index = desc->add({indirect, 0, indirect->info().size, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
// //                        desc->add({borders_indices, 0, borders_indices->info().size, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
// //         desc->update();
// //         indirect->setDescriptor(desc, index);
//       }
//       
//       yavf::PipelineLayout p_layout = VK_NULL_HANDLE;
//       {
//         yavf::PipelineLayoutMaker plm(device);
//         p_layout = plm.addDescriptorLayout(uniform_layout)
//                       .addDescriptorLayout(tiles_data_layout)
//                       .addDescriptorLayout(tiles_indices_layout)
//                       .addDescriptorLayout(storage_buffer_layout)
//                       .create("borders_optimizer_pipeline_layout");
//       }
//       
//       {
//         yavf::raii::ShaderModule compute(device, global::root_directory() + "shaders/borders.comp.spv");
//         
//         yavf::ComputePipelineMaker cpm(device);
//         pipe = cpm.shader(compute).create("borders_optimizer_pipeline", p_layout);
//       }
//     }
//     
//     tile_borders_optimizer::~tile_borders_optimizer() {
//       device->destroySetLayout("borders_indices_layout");
//       device->destroy(indirect);
//       if (borders_indices != nullptr) device->destroy(borders_indices);
//       device->destroy(pipe.layout());
//       device->destroy(pipe);
//     }
//     
//     void tile_borders_optimizer::begin() {
//       if (borders_indices == nullptr) return;
//       auto buffer = reinterpret_cast<struct indirect_buffer*>(indirect->ptr());
//       
// //       PRINT_VAR("pentagon_command.vertexCount  ", buffer->pentagon_command.vertexCount)
// //       PRINT_VAR("pentagon_command.instanceCount", buffer->pentagon_command.instanceCount)
// //       PRINT_VAR("pentagon_command.firstVertex  ", buffer->pentagon_command.firstVertex)
// //       PRINT_VAR("pentagon_command.firstInstance", buffer->pentagon_command.firstInstance)
// //       PRINT_VAR("hexagon_command.instanceCount ", buffer->hexagon_command.instanceCount)
//       
//       auto uniform = global::get<render::buffers>()->uniform;
//       auto camera_data = reinterpret_cast<render::camera_data*>(uniform->ptr());
//       const auto fru = utils::compute_frustum(camera_data->viewproj);
//       
//       buffer->border_command.vertexCount = 0;
//       buffer->border_command.instanceCount = 1;
//       buffer->border_command.firstVertex = 0;
//       buffer->border_command.firstInstance = 0;
//       
//       // как передать количество? 
//       // я заполняю буфер только после того как сделаю полностью карту
//       // в функцию передать
// //       buffer->data.x = core::map::tri_count_d(core::map::accel_struct_detail_level);
// //       buffer->data.y = max_tiles_count;
// //       buffer->data.z = 0;
// //       buffer->data.w = 0;
//       
//       memcpy(&buffer->frustum, &fru, sizeof(utils::frustum));
//     }
//     
//     void tile_borders_optimizer::proccess(container* ctx) {
//       if (borders_indices == nullptr) return;
//       auto map_systems = global::get<systems::map_t>();
//       if (!map_systems->is_init()) return;
//       auto map = map_systems->map;
// //       if (map->status() == core::map::status::initial) return;
//       
//       auto uniform = global::get<render::buffers>()->uniform;
//       auto tiles = map->tiles;
//       auto buffer = reinterpret_cast<struct indirect_buffer*>(indirect->ptr());
//       auto border_buffer = map_buffers->border_buffer;
// //       auto points = global::get<render::buffers>()->points;
//       
//       auto task = ctx->compute();
//       task->setPipeline(pipe);
//       task->setDescriptor({
//         uniform->descriptorSet()->handle(), 
//         tiles->descriptorSet()->handle(), 
// //         points->descriptorSet()->handle(), 
//         indirect->descriptorSet()->handle(),
//         border_buffer->descriptorSet()->handle()
// //         tiles_indices->descriptorSet()->handle()
//       }, 0);
//       
//       const uint32_t count = std::ceil(float(buffer->data.x) / float(work_group_size));
//       task->dispatch(count, 1, 1);
//       
// //       task->setBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
// //                        VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_INDIRECT_COMMAND_READ_BIT);
// //       
// //       task->setBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
// //                        VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
//     }
//     
//     void tile_borders_optimizer::clear() {}
//     
//     yavf::Buffer* tile_borders_optimizer::indirect_buffer() const { return indirect; }
//     yavf::Buffer* tile_borders_optimizer::vertices_buffer() const { return borders_indices; }
//     
//     void tile_borders_optimizer::set_borders_count(const uint32_t &count) {
//       if (borders_indices != nullptr) device->destroy(borders_indices);
//       borders_indices = device->create(yavf::BufferCreateInfo::buffer(sizeof(instance_data_t)*((count*4)/4+1+count), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT), VMA_MEMORY_USAGE_GPU_ONLY);
//       auto buffer = reinterpret_cast<struct indirect_buffer*>(indirect->ptr());
//       buffer->data.x = count;
//       buffer->data.y = 0;
//       buffer->data.z = 0;
//       buffer->data.w = 0;
//       
//       {
//         //size_t index = 0;
//         set->at(0) = {indirect, 0, indirect->info().size, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
//         set->at(1) = {borders_indices, 0, borders_indices->info().size, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
//         set->update();
//         if (indirect->descriptorSet() == nullptr) indirect->setDescriptor(set, 0);
//       }
//     }
//     
//     tile_walls_optimizer::tile_walls_optimizer(const create_info &info) : 
//       device(info.device), 
//       indirect(nullptr), 
//       walls_indices(nullptr), 
//       set(nullptr),
//       map_buffers(info.map_buffers) 
//     {
//       indirect = device->create(yavf::BufferCreateInfo::buffer(sizeof(struct indirect_buffer), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
//       
// //       auto pool = device->descriptorPool(DEFAULT_DESCRIPTOR_POOL_NAME);
// //       auto storage_layout = device->setLayout(STORAGE_BUFFER_LAYOUT_NAME);
//       auto tiles_data_layout = device->setLayout(TILES_DATA_LAYOUT_NAME);
//       auto uniform_layout = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
//       
//       yavf::DescriptorPool pool = device->descriptorPool(WORLD_MAP_RENDER_DESCRIPTOR_POOL);
//       if (pool == VK_NULL_HANDLE) {
//         yavf::DescriptorPoolMaker dpm(device);
//         pool = dpm.poolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 30).create(WORLD_MAP_RENDER_DESCRIPTOR_POOL);
//       }
//       
//       ASSERT(tiles_data_layout != VK_NULL_HANDLE);
//       
//       yavf::DescriptorSetLayout storage_buffer_layout = device->setLayout(STORAGE_BUFFER_LAYOUT_NAME);
//       yavf::DescriptorSetLayout tiles_indices_layout = VK_NULL_HANDLE;
//       {
//         yavf::DescriptorLayoutMaker dlm(device);
//         tiles_indices_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
//                                   .binding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
//                                   .create("walls_indices_layout");
//       }
//       
//       {
//         yavf::DescriptorMaker dm(device);
//         set = dm.layout(tiles_indices_layout).create(pool)[0];
//         set->resize(2);
// //         size_t index = desc->add({indirect, 0, indirect->info().size, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
// //                        desc->add({borders_indices, 0, borders_indices->info().size, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
// //         desc->update();
// //         indirect->setDescriptor(desc, index);
//       }
//       
//       yavf::PipelineLayout p_layout = VK_NULL_HANDLE;
//       {
//         yavf::PipelineLayoutMaker plm(device);
//         p_layout = plm.addDescriptorLayout(uniform_layout)
//                       .addDescriptorLayout(tiles_data_layout)
//                       .addDescriptorLayout(tiles_indices_layout)
//                       .addDescriptorLayout(storage_buffer_layout)
//                       .create("walls_optimizer_pipeline_layout");
//       }
//       
//       {
//         yavf::raii::ShaderModule compute(device, global::root_directory() + "shaders/walls.comp.spv");
//         
//         yavf::ComputePipelineMaker cpm(device);
//         pipe = cpm.shader(compute).create("walls_optimizer_pipeline", p_layout);
//       }
//     }
//     
//     tile_walls_optimizer::~tile_walls_optimizer() {
//       device->destroySetLayout("walls_indices_layout");
//       device->destroy(indirect);
//       if (walls_indices != nullptr) device->destroy(walls_indices);
//       device->destroy(pipe.layout());
//       device->destroy(pipe);
//     }
//     
//     void tile_walls_optimizer::begin() {
//       if (walls_indices == nullptr) return;
//       auto buffer = reinterpret_cast<struct indirect_buffer*>(indirect->ptr());
//       
//       auto uniform = global::get<render::buffers>()->uniform;
//       auto camera_data = reinterpret_cast<render::camera_data*>(uniform->ptr());
//       const auto fru = utils::compute_frustum(camera_data->viewproj);
//       
//       buffer->walls_command.vertexCount = 0;
//       buffer->walls_command.instanceCount = 1;
//       buffer->walls_command.firstVertex = 0;
//       buffer->walls_command.firstInstance = 0;
//       
//       memcpy(&buffer->frustum, &fru, sizeof(utils::frustum));
//     }
//     
//     void tile_walls_optimizer::proccess(container* ctx) {
//       if (walls_indices == nullptr) return;
//       auto map_systems = global::get<systems::map_t>();
//       if (!map_systems->is_init()) return;
//       auto map = map_systems->map;
// //       if (map->status() == core::map::status::initial) return;
//       
//       auto uniform = global::get<render::buffers>()->uniform;
//       auto tiles = map->tiles;
//       auto buffer = reinterpret_cast<struct indirect_buffer*>(indirect->ptr());
//       auto walls_buffer = map_buffers->tiles_connections;
// //       auto points = global::get<render::buffers>()->points;
//       
//       auto task = ctx->compute();
//       task->setPipeline(pipe);
//       task->setDescriptor({
//         uniform->descriptorSet()->handle(), 
//         tiles->descriptorSet()->handle(), 
// //         points->descriptorSet()->handle(), 
//         indirect->descriptorSet()->handle(),
//         walls_buffer->descriptorSet()->handle()
// //         tiles_indices->descriptorSet()->handle()
//       }, 0);
//       
//       const uint32_t count = std::ceil(float(buffer->data.x) / float(work_group_size));
//       task->dispatch(count, 1, 1);
//     }
//     
//     void tile_walls_optimizer::clear() {}
//     yavf::Buffer* tile_walls_optimizer::indirect_buffer() const { return indirect; }
//     yavf::Buffer* tile_walls_optimizer::vertices_buffer() const { return walls_indices; }
//     
//     void tile_walls_optimizer::set_connections_count(const uint32_t &count) {
//       if (walls_indices != nullptr) device->destroy(walls_indices);
//       walls_indices = device->create(yavf::BufferCreateInfo::buffer(sizeof(instance_data_t)*((count*6)/4+1), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT), VMA_MEMORY_USAGE_GPU_ONLY);
//       auto buffer = reinterpret_cast<struct indirect_buffer*>(indirect->ptr());
//       buffer->data.x = count;
//       buffer->data.y = 0;
//       buffer->data.z = 0;
//       buffer->data.w = 0;
//       
//       {
//         size_t index = 0;
//         set->at(0) = {indirect, 0, indirect->info().size, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
//         set->at(1) = {walls_indices, 0, walls_indices->info().size, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
//         set->update();
//         if (indirect->descriptorSet() == nullptr) indirect->setDescriptor(set, index);
//       }
//     }
    
    void barriers::begin() {}
    void barriers::proccess(container* ctx) {
      (void)ctx;
//       auto task = ctx->compute();
//       task->setBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
//                        VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_INDIRECT_COMMAND_READ_BIT);
//       
//       task->setBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
//                        VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
      
//       task->setBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
//                        VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
    }
    
    void barriers::clear() {}
    
    const uint16_t pen_index_array[] = {
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, UINT16_MAX, 10, 11, 12, 13, 14
    };
    
    const uint16_t hex_index_array[] = {
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1, UINT16_MAX, 12, 13, 14, 15, 16, 17
    };
    
#define DEFAULT_COLOR_WRITE_MASK vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA

//     const uint32_t max_tiles = 500000;
    tile_render::tile_render(const create_info &info) : 
      device(info.cont->vulkan->device), 
      allocator(info.cont->vulkan->buffer_allocator),
      opt(info.opt),
      images_set(nullptr)
    {
      static_assert(sizeof(pen_index_array) == 18 * sizeof(uint16_t));
      static_assert(sizeof(hex_index_array) == 21 * sizeof(uint16_t));
      const size_t pen_buffer_size = sizeof(pen_index_array);
      const size_t hex_buffer_size = sizeof(hex_index_array);
      const size_t final_size = align_to(pen_buffer_size+hex_buffer_size, 4);
      //indices = device->create(yavf::BufferCreateInfo::buffer(sizeof(uint32_t)*max_tiles, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
      points_indices.create(allocator, buffer(final_size, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst), vma::MemoryUsage::eGpuOnly, "tile_render::points_indices");
      
      {
        const auto b_info = buffer(final_size, vk::BufferUsageFlagBits::eTransferSrc);
        const auto buf = create_buffer_unique(allocator, b_info, vma::MemoryUsage::eCpuOnly, "tile_render::points_indices staging");
        auto points_ptr = reinterpret_cast<char*>(buf.ptr);
        memcpy(&points_ptr[0],               pen_index_array, pen_buffer_size);
        memcpy(&points_ptr[pen_buffer_size], hex_index_array, hex_buffer_size);
        
        do_command(device, info.cont->vulkan->transfer_command_pool, info.cont->vulkan->graphics, info.cont->vulkan->transfer_fence, [&] (vk::CommandBuffer task) {
          const vk::BufferCopy c{0, 0, final_size};
          const vk::CommandBufferBeginInfo b_info(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
          task.begin(b_info);
          task.copyBuffer(buf.handle, points_indices.handle, c);
          task.end();
        });
      }
      
      auto uniform_layout = info.cont->vulkan->uniform_layout;
      
      images_set = *global::get<systems::core_t>()->image_controller->get_descriptor_set();

      {
        pipeline_layout_maker plm(&device);
        p_layout = plm.addDescriptorLayout(uniform_layout)
                    .addDescriptorLayout(info.images_layout)
                    .addDescriptorLayout(info.tiles_data_layout)
                    .create(TILE_RENDER_PIPELINE_LAYOUT_NAME);
      }

      {
        // nexus 5x не поддерживает геометрический шейдер
        const auto vertex   = create_shader_module(device, global::root_directory() + "shaders/tiles.vert.spv");
        const auto fragment = create_shader_module(device, global::root_directory() + "shaders/tiles.frag.spv");

        pipeline_maker pm(&device);
        pm.clearBlending();

        pipe = pm.addShader(vk::ShaderStageFlagBits::eVertex,   vertex.get())
                 .addShader(vk::ShaderStageFlagBits::eFragment, fragment.get())
                 .vertexBinding(0, sizeof(uint32_t), vk::VertexInputRate::eInstance) //sizeof(instance_data_t)
                   .vertexAttribute(0, 0, vk::Format::eR32Uint, 0)
                 .depthTest(VK_TRUE)
                 .depthWrite(VK_TRUE)
                 .frontFace(vk::FrontFace::eCounterClockwise)
                 .cullMode(vk::CullModeFlagBits::eBack)
                 .assembly(vk::PrimitiveTopology::eTriangleStrip, VK_TRUE) // VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN
                 .viewport()
                 .scissor()
                 .dynamicState(vk::DynamicState::eViewport)
                 .dynamicState(vk::DynamicState::eScissor)
                 .colorBlendBegin(VK_FALSE)
                   .colorWriteMask(DEFAULT_COLOR_WRITE_MASK)
                 .create(TILE_RENDER_PIPELINE_NAME, p_layout, info.cont->vlk_window->render_pass);
      }
    }

    tile_render::~tile_render() {
      points_indices.destroy(allocator);
      device.destroy(p_layout);
      device.destroy(pipe);
    }

    void tile_render::begin() {}
    void tile_render::proccess(container* ctx) {
      auto map_systems = global::get<systems::map_t>();
      if (!map_systems->is_init()) return;
      auto map = map_systems->map;
      
      auto uniform = global::get<render::buffers>()->uniform_set;
      auto tiles = map->data->tiles_set;
      
      auto indirect_buffer = opt->indirect_buffer();
      auto indices_buffer = opt->tiles_index_buffer();
      
      const auto bind_point = vk::PipelineBindPoint::eGraphics;
      auto task = ctx->command_buffer();
      task->bindPipeline(bind_point, pipe);
      task->bindDescriptorSets(bind_point, p_layout, 0, {uniform, images_set, tiles}, nullptr);
      
      task->bindVertexBuffers(0, indices_buffer, {0});
      task->bindIndexBuffer(points_indices.handle, 0, vk::IndexType::eUint16);
      task->drawIndexedIndirect(indirect_buffer, offsetof123(struct render::tile_optimizer::indirect_buffer, pen_tiles_command), 1, sizeof(vk::DrawIndexedIndirectCommand)); // пентагоны отдельно посчитаем
      
      task->bindVertexBuffers(0, indices_buffer, sizeof(uint32_t)*12);
      task->bindIndexBuffer(points_indices.handle, sizeof(pen_index_array), vk::IndexType::eUint16);
      task->drawIndexedIndirect(indirect_buffer, offsetof123(struct render::tile_optimizer::indirect_buffer, hex_tiles_command), 1, sizeof(vk::DrawIndexedIndirectCommand));
    }

    void tile_render::clear() {}
//     void tile_render::recreate_pipelines(const game::image_resources_t* resource) {
//       (void)resource;
//     }
    
//     void tile_render::change_rendering_mode(const uint32_t &render_mode, const uint32_t &water_mode, const uint32_t &render_slot, const uint32_t &water_slot, const glm::vec3 &color) {
//       if (pipe.handle() != VK_NULL_HANDLE) {
//         device->destroy(pipe);
//       }
//       
//       // нужно ли это пересоздавать?
//       ASSERT(pipe.layout() != VK_NULL_HANDLE);
//       
//       yavf::raii::ShaderModule vertex  (device, global::root_directory() + "shaders/tiles.vert.spv");
//       //yavf::raii::ShaderModule geom    (device, global::root_directory() + "shaders/tiles.geom.spv"); // nexus 5x не поддерживает геометрический шейдер
//       yavf::raii::ShaderModule fragment(device, global::root_directory() + "shaders/tiles.frag.spv");
// 
//       const uint32_t data[] = {glm::floatBitsToUint(color.x), glm::floatBitsToUint(color.y), glm::floatBitsToUint(color.z), render_mode, water_mode, render_slot, water_slot};
//       yavf::PipelineMaker pm(device);
//       pm.clearBlending();
// 
//       pipe = pm.addShader(VK_SHADER_STAGE_VERTEX_BIT, vertex)
//                 .addShader(VK_SHADER_STAGE_FRAGMENT_BIT, fragment)
//                 .addSpecializationEntry(0, 0 * sizeof(uint32_t), sizeof(float))
//                 .addSpecializationEntry(1, 1 * sizeof(uint32_t), sizeof(float))
//                 .addSpecializationEntry(2, 2 * sizeof(uint32_t), sizeof(float))
//                 .addSpecializationEntry(3, 3 * sizeof(uint32_t), sizeof(uint32_t))
//                 .addSpecializationEntry(4, 4 * sizeof(uint32_t), sizeof(uint32_t))
//                 .addSpecializationEntry(5, 5 * sizeof(uint32_t), sizeof(uint32_t))
//                 .addSpecializationEntry(6, 6 * sizeof(uint32_t), sizeof(uint32_t))
//                 .addData(sizeof(data), data)
// //                 .vertexBinding(0, sizeof(uint32_t), VK_VERTEX_INPUT_RATE_INSTANCE)
// //                   .vertexAttribute(0, 0, VK_FORMAT_R32_UINT, 0)
// //                 .vertexBinding(1, sizeof(uint32_t))
// //                   .vertexAttribute(1, 1, VK_FORMAT_R32_UINT, 0)
//                 .depthTest(VK_TRUE)
//                 .depthWrite(VK_TRUE)
//                 .frontFace(VK_FRONT_FACE_CLOCKWISE)
//                 .cullMode(VK_CULL_MODE_BACK_BIT)
//                 .assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN, VK_TRUE)
//                 .viewport()
//                 .scissor()
//                 .dynamicState(VK_DYNAMIC_STATE_VIEWPORT)
//                 .dynamicState(VK_DYNAMIC_STATE_SCISSOR)
//                 .colorBlendBegin(VK_FALSE)
//                   .colorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
//                 .create(TILE_RENDER_PIPELINE_NAME, pipe.layout(), global::get<render::window>()->render_pass);
//     }
    
    vk::Buffer tile_render::vertex_indices() const {
      return points_indices.handle;
    }
    
    tile_border_render::tile_border_render(const create_info &info) :
      device(info.cont->vulkan->device),
      allocator(info.cont->vulkan->buffer_allocator),
      opt(info.opt),
      map_buffers(info.map_buffers)
    {
      
      auto uniform_layout = info.cont->vulkan->uniform_layout;
      auto tiles_data_layout = info.tiles_data_layout;
      auto storage_buffer_layout = info.cont->vulkan->storage_layout;
      
      {
        pipeline_layout_maker plm(&device);
        p_layout = plm.addDescriptorLayout(uniform_layout)
                      .addDescriptorLayout(storage_buffer_layout)
                      .addDescriptorLayout(storage_buffer_layout)
                      .addDescriptorLayout(tiles_data_layout)
                      .create("tile_borders_render_layout");
      }
      
//       glm::vec3 color = glm::vec3(0.0f, 0.0f, 0.0f);
      const std::string name = "tile_borders_render_pipeline";

      {
        const auto vertex   = create_shader_module(device, global::root_directory() + "shaders/borders.vert.spv");
        const auto fragment = create_shader_module(device, global::root_directory() + "shaders/borders.frag.spv");

        pipeline_maker pm(&device);
        pm.clearBlending();

        pipe = pm.addShader(vk::ShaderStageFlagBits::eVertex, vertex.get())
                 .addShader(vk::ShaderStageFlagBits::eFragment, fragment.get())
                 .vertexBinding(0, sizeof(uint32_t), vk::VertexInputRate::eInstance)
                   .vertexAttribute(0, 0, vk::Format::eR32Uint, 0)
                 .depthTest(VK_TRUE)
                 .depthWrite(VK_TRUE)
                 .frontFace(vk::FrontFace::eCounterClockwise)
                 .cullMode(vk::CullModeFlagBits::eBack)
                 .assembly(vk::PrimitiveTopology::eTriangleStrip, VK_TRUE)
                 .viewport()
                 .scissor()
                 .dynamicState(vk::DynamicState::eViewport)
                 .dynamicState(vk::DynamicState::eScissor)
                 .colorBlendBegin(VK_FALSE)
                   .colorWriteMask(DEFAULT_COLOR_WRITE_MASK)
                 .create(name, p_layout, info.cont->vlk_window->render_pass);
      }
    }
    
    tile_border_render::~tile_border_render() {
      device.destroy(p_layout);
      device.destroy(pipe);
    }

    void tile_border_render::begin() {}
    void tile_border_render::proccess(container* ctx) {
      auto map_systems = global::get<systems::map_t>();
      if (!map_systems->is_init()) return;
      auto map = map_systems->map;
      auto uniform = global::get<render::buffers>()->uniform_set;
      auto borders = map_buffers->border_set;
      auto types = map_buffers->types_set;
      auto tiles = map->data->tiles_set;
      
      auto indirect_buffer = opt->indirect_buffer();
      auto indices_buffer = opt->borders_index_buffer();
      if (indices_buffer == vk::Buffer(nullptr)) return;
      
      const auto bind_point = vk::PipelineBindPoint::eGraphics;
      auto task = ctx->command_buffer();
      task->bindPipeline(bind_point, pipe);
      task->bindDescriptorSets(bind_point, p_layout, 0, {uniform, borders, types, tiles}, nullptr);
      task->bindVertexBuffers(0, indices_buffer, {0});
      task->drawIndirect(indirect_buffer, offsetof123(struct tile_optimizer::indirect_buffer, borders_command), 1, sizeof(vk::DrawIndirectCommand));
    }
    
    void tile_border_render::clear() {}
//     void tile_border_render::recreate_pipelines(const game::image_resources_t* resource) { (void)resource; }
    
//     tile_connections_render::tile_connections_render(const create_info &info) :
//       device(info.cont->vulkan->device), 
//       allocator(info.cont->vulkan->buffer_allocator),
//       opt(info.opt),
//       map_buffers(info.map_buffers),
//       images_set(nullptr)
//     {
//       auto uniform_layout = info.cont->vulkan->uniform_layout;
//       
//       auto tiles_data_layout = info.tiles_data_layout;
//       auto storage_layout = info.cont->vulkan->storage_layout;
//       auto images_layout = info.images_layout;
//       images_set = *global::get<systems::core_t>()->image_controller->get_descriptor_set();
// 
//       {
//         pipeline_layout_maker plm(&device);
//         p_layout = plm.addDescriptorLayout(uniform_layout)
//                       .addDescriptorLayout(images_layout)
//                       .addDescriptorLayout(storage_layout)
//                       .addDescriptorLayout(tiles_data_layout)
//                       .create("walls_rendering_pipeline_layout");
//       }
// 
//       {
//         const auto vertex   = create_shader_module(device, global::root_directory() + "shaders/walls.vert.spv");
//         const auto fragment = create_shader_module(device, global::root_directory() + "shaders/walls.frag.spv");
// 
//         pipeline_maker pm(&device);
//         pm.clearBlending();
// 
//         pipe = pm.addShader(vk::ShaderStageFlagBits::eVertex, vertex.get())
//                  .addShader(vk::ShaderStageFlagBits::eFragment, fragment.get())
//                 .vertexBinding(0, sizeof(uint32_t), vk::VertexInputRate::eInstance)
//                   .vertexAttribute(0, 0, vk::Format::eR32Uint, 0)
//                  .depthTest(VK_TRUE)
//                  .depthWrite(VK_TRUE)
//                  .frontFace(vk::FrontFace::eCounterClockwise)
//                  .cullMode(vk::CullModeFlagBits::eBack)
//                  .assembly(vk::PrimitiveTopology::eTriangleStrip, VK_TRUE)
//                  .viewport()
//                  .scissor()
//                  .dynamicState(vk::DynamicState::eViewport)
//                  .dynamicState(vk::DynamicState::eScissor)
//                  .colorBlendBegin(VK_FALSE)
//                    .colorWriteMask(DEFAULT_COLOR_WRITE_MASK)
//                  .create("walls_rendering_pipeline", p_layout, info.cont->vlk_window->render_pass);
//       }
//     }
//     
//     tile_connections_render::~tile_connections_render() {
//       device.destroy(p_layout);
//       device.destroy(pipe);
//     }
// 
//     void tile_connections_render::begin() {}
//     void tile_connections_render::proccess(container* ctx) {
//       auto map_systems = global::get<systems::map_t>();
//       if (!map_systems->is_init()) return;
//       auto map = map_systems->map;
//       auto uniform = global::get<render::buffers>()->uniform;
//       auto walls_buffer = map_buffers->tiles_connections;
//       auto tiles = map->data->tiles_set;
//       
//       auto indirect_buffer = opt->indirect_buffer();
//       auto indices_buffer = opt->walls_index_buffer();
//       if (indices_buffer == nullptr) return;
//       
//       auto task = ctx->graphics();
//       task->setPipeline(pipe);
//       ASSERT(uniform->descriptorSet() != nullptr);
//       ASSERT(walls_buffer->descriptorSet() != nullptr);
//       ASSERT(tiles->descriptorSet() != nullptr);
//       task->setDescriptor({uniform->descriptorSet()->handle(), images_set->handle(), walls_buffer->descriptorSet()->handle(), tiles->descriptorSet()->handle()}, 0);
//       
//       task->setVertexBuffer(indices_buffer, 0);
//       //task->setIndexBuffer(indices_buffer);
//       task->drawIndirect(indirect_buffer, 1, offsetof123(struct tile_optimizer::indirect_buffer, walls_command));
//     }
//     
//     void tile_connections_render::clear() {}
// 
//     void tile_connections_render::recreate_pipelines(const game::image_resources_t* resource) { (void)resource; }
//     void tile_connections_render::change_rendering_mode(const uint32_t &render_mode, const uint32_t &water_mode, const uint32_t &render_slot, const uint32_t &water_slot, const glm::vec3 &color) {
//       if (pipe.handle() != VK_NULL_HANDLE) {
//         device->destroy(pipe);
//       }
//       
//       // нужно ли это пересоздавать?
//       ASSERT(pipe.layout() != VK_NULL_HANDLE);
//       
//       yavf::raii::ShaderModule vertex  (device, global::root_directory() + "shaders/walls.vert.spv");
//       //yavf::raii::ShaderModule geom    (device, global::root_directory() + "shaders/tiles.geom.spv"); // nexus 5x не поддерживает геометрический шейдер
//       yavf::raii::ShaderModule fragment(device, global::root_directory() + "shaders/tiles.frag.spv");
// 
//       const uint32_t data[] = {glm::floatBitsToUint(color.x), glm::floatBitsToUint(color.y), glm::floatBitsToUint(color.z), render_mode, water_mode, render_slot, water_slot};
//       yavf::PipelineMaker pm(device);
//       pm.clearBlending();
// 
//       pipe = pm.addShader(VK_SHADER_STAGE_VERTEX_BIT, vertex)
//                 .addShader(VK_SHADER_STAGE_FRAGMENT_BIT, fragment)
//                 .addSpecializationEntry(0, 0 * sizeof(uint32_t), sizeof(float))
//                 .addSpecializationEntry(1, 1 * sizeof(uint32_t), sizeof(float))
//                 .addSpecializationEntry(2, 2 * sizeof(uint32_t), sizeof(float))
//                 .addSpecializationEntry(3, 3 * sizeof(uint32_t), sizeof(uint32_t))
//                 .addSpecializationEntry(4, 4 * sizeof(uint32_t), sizeof(uint32_t))
//                 .addSpecializationEntry(5, 5 * sizeof(uint32_t), sizeof(uint32_t))
//                 .addSpecializationEntry(6, 6 * sizeof(uint32_t), sizeof(uint32_t))
//                 .addData(sizeof(data), data)
// //                 .vertexBinding(0, sizeof(uint32_t))
// //                   .vertexAttribute(0, 0, VK_FORMAT_R32_UINT, 0)
//                 .depthTest(VK_TRUE)
//                 .depthWrite(VK_TRUE)
//                 .frontFace(VK_FRONT_FACE_CLOCKWISE)
//                 .cullMode(VK_CULL_MODE_FRONT_BIT)
//                 .assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_TRUE)
//                 .viewport()
//                 .scissor()
//                 .dynamicState(VK_DYNAMIC_STATE_VIEWPORT)
//                 .dynamicState(VK_DYNAMIC_STATE_SCISSOR)
//                 .colorBlendBegin(VK_FALSE)
//                   .colorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
//                 .create("walls_rendering_pipeline", pipe.layout(), global::get<render::window>()->render_pass);
//     }
    
    tile_object_render::tile_object_render(const create_info &info) : 
      device(info.cont->vulkan->device), 
      allocator(info.cont->vulkan->buffer_allocator), 
      opt(info.opt), 
      map_buffers(info.map_buffers), 
      images_set(nullptr),
      multi_draw_indirect(info.cont->is_properties_presented(render::container::physical_device_multidraw_indirect))
    {
      auto uniform_layout = info.cont->vulkan->uniform_layout;
      auto tiles_data_layout = info.tiles_data_layout;
      auto images_layout = info.images_layout;
      images_set = *global::get<systems::core_t>()->image_controller->get_descriptor_set();

      {
        pipeline_layout_maker plm(&device);
        p_layout = plm.addDescriptorLayout(uniform_layout)
                      .addDescriptorLayout(images_layout)
                      .addDescriptorLayout(tiles_data_layout)
                      .create("object_rendering_pipeline_layout");
      }
      
      {
        const auto vertex   = create_shader_module(device, global::root_directory() + "shaders/first_object.vert.spv");
        const auto fragment = create_shader_module(device, global::root_directory() + "shaders/first_object.frag.spv");

        pipeline_maker pm(&device);
        pm.clearBlending();

        pipe = pm.addShader(vk::ShaderStageFlagBits::eVertex, vertex.get())
                 .addShader(vk::ShaderStageFlagBits::eFragment, fragment.get())
                 .depthTest(VK_TRUE)
                 .depthWrite(VK_TRUE)
                 .frontFace(vk::FrontFace::eClockwise)
                 .cullMode(vk::CullModeFlagBits::eBack)
                 .assembly(vk::PrimitiveTopology::eTriangleStrip, VK_TRUE)
                 .viewport()
                 .scissor()
                 .dynamicState(vk::DynamicState::eViewport)
                 .dynamicState(vk::DynamicState::eScissor)
                 .colorBlendBegin(VK_FALSE)
                   .colorWriteMask(DEFAULT_COLOR_WRITE_MASK)
                 .create("object_rendering_pipeline", p_layout, info.cont->vlk_window->render_pass);
      }
    }
    
    tile_object_render::~tile_object_render() {
      device.destroy(p_layout);
      device.destroy(pipe);
    }

    void tile_object_render::begin() {}
    void tile_object_render::proccess(container* ctx) {
      auto map_systems = global::get<systems::map_t>();
      if (!map_systems->is_init()) return;
      auto map = map_systems->map;
      auto uniform = global::get<render::buffers>()->uniform_set;
      auto tiles = map->data->tiles_set;
      
      auto indirect_buffer = opt->indirect_buffer();
      auto indices_buffer = opt->objects_index_buffer();
      if (indices_buffer == vk::Buffer(nullptr)) return;
      
      const auto bind_point = vk::PipelineBindPoint::eGraphics;
      auto task = ctx->command_buffer();
      task->bindPipeline(bind_point, pipe);
      task->bindDescriptorSets(bind_point, p_layout, 0, {uniform, images_set, tiles}, nullptr);
      task->bindIndexBuffer(indices_buffer, 0, vk::IndexType::eUint32);
      if (multi_draw_indirect) {
        task->drawIndexedIndirect(indirect_buffer, offsetof123(struct tile_optimizer::indirect_buffer, biome_data), MAX_BIOMES_COUNT, sizeof(biome_objects_data_t));
      } else {
        for (uint32_t i = 0; i < MAX_BIOMES_COUNT; ++i) {
          task->drawIndexedIndirect(indirect_buffer, offsetof123(struct tile_optimizer::indirect_buffer, biome_data)+sizeof(biome_objects_data_t)*i, 1, sizeof(vk::DrawIndexedIndirectCommand));
        }
      }
    }
    
    void tile_object_render::clear() {}
    
#define HIGHLIGHT_DATA_COUNT 2
    tile_highlights_render::tile_highlights_render(const create_info &info) : 
      device(info.cont->vulkan->device), 
      allocator(info.cont->vulkan->buffer_allocator),
      map_buffers(info.map_buffers),  
      hex_tiles_count(0), 
      pen_tiles_count(0) 
    {
      auto uniform_layout = info.cont->vulkan->uniform_layout;
      auto tiles_data_layout = info.tiles_data_layout;
      
      {
        pipeline_layout_maker plm(&device);
        p_layout = plm.addDescriptorLayout(uniform_layout)
                      .addDescriptorLayout(tiles_data_layout)
                      .create("tiling_highlights_rendering_pipeline_layout");
      }
      
      {
        const auto vertex   = create_shader_module(device, global::root_directory() + "shaders/one_tile.vert.spv");
        const auto fragment = create_shader_module(device, global::root_directory() + "shaders/one_tile.frag.spv");

        pipeline_maker pm(&device);

        pipe = pm.addShader(vk::ShaderStageFlagBits::eVertex, vertex.get())
                 .addShader(vk::ShaderStageFlagBits::eFragment, fragment.get())
                 .vertexBinding(0, sizeof(uint32_t)*HIGHLIGHT_DATA_COUNT, vk::VertexInputRate::eInstance)
                   .vertexAttribute(0, 0, vk::Format::eR32Uint, 0)
                   .vertexAttribute(1, 0, vk::Format::eR32Uint, sizeof(uint32_t))
                 .depthTest(VK_TRUE)
                 .depthWrite(VK_TRUE)
                 .frontFace(vk::FrontFace::eCounterClockwise)
                 .cullMode(vk::CullModeFlagBits::eBack)
                 .assembly(vk::PrimitiveTopology::eTriangleFan, VK_TRUE)
                 .viewport()
                 .scissor()
                 .dynamicState(vk::DynamicState::eViewport).dynamicState(vk::DynamicState::eScissor)
                 .colorBlendBegin(VK_TRUE)
                 .create("tiling_highlights_rendering_pipeline", p_layout, info.cont->vlk_window->render_pass);
      }
      
      const size_t size = align_to(max_indices_count * 2, 16);
      tiles_indices.create(allocator, buffer(size, vk::BufferUsageFlagBits::eVertexBuffer), vma::MemoryUsage::eCpuOnly, "tile_highlights_render::tiles_indices");
    }
    
    tile_highlights_render::~tile_highlights_render() {
      device.destroy(p_layout);
      device.destroy(pipe);
      tiles_indices.destroy(allocator);
    }
    
    void tile_highlights_render::begin() {}
    void tile_highlights_render::proccess(container* ctx) {
      if (hex_tiles_count + pen_tiles_count == 0) return;
      
      auto map_systems = global::get<systems::map_t>();
      if (!map_systems->is_init()) return;
      auto map = map_systems->map;
      auto uniform = global::get<render::buffers>()->uniform_set;
      auto tiles = map->data->tiles_set;
      
      const auto bind_point = vk::PipelineBindPoint::eGraphics;
      auto task = ctx->command_buffer();
      task->bindPipeline(bind_point, pipe);
      task->bindDescriptorSets(bind_point, p_layout, 0, {uniform, tiles}, nullptr);
      task->bindVertexBuffers(0, tiles_indices.handle, {0});
      if (pen_tiles_count != 0) task->draw(5, pen_tiles_count, 0, 0);
      if (hex_tiles_count != 0) task->draw(6, hex_tiles_count, 0, 12);
    }
    
    void tile_highlights_render::clear() {
      hex_tiles_count = 0;
      pen_tiles_count = 0;
    }
    
    void tile_highlights_render::add(const uint32_t &tile_index, const color_t &color) {
      const bool pentagon = tile_index < 12;
      auto tiles_indices_ptr = reinterpret_cast<uint32_t*>(tiles_indices.ptr);
      if (pentagon) {
        const uint32_t offset = pen_tiles_count.fetch_add(1);
        ASSERT(offset < 12);
        const uint32_t final_offset = offset * HIGHLIGHT_DATA_COUNT;
        tiles_indices_ptr[final_offset]   = tile_index;
        tiles_indices_ptr[final_offset+1] = color.container;
      } else {
        const uint32_t offset = hex_tiles_count.fetch_add(1);
        const uint32_t final_offset = offset * HIGHLIGHT_DATA_COUNT + 12 * HIGHLIGHT_DATA_COUNT;
        tiles_indices_ptr[final_offset]   = tile_index;
        tiles_indices_ptr[final_offset+1] = color.container;
      }
    }
    
    tile_structure_render::tile_structure_render(const create_info &info) : 
      device(info.cont->vulkan->device), 
      allocator(info.cont->vulkan->buffer_allocator),
      opt(info.opt), 
      map_buffers(info.map_buffers), 
      images_set(nullptr),
      structures_count(0),
      current_inst_size(0),
      local_structures_count(0)
    {
      auto uniform_layout = info.cont->vulkan->uniform_layout;
      auto tiles_data_layout = info.tiles_data_layout;
      auto images_layout = info.images_layout;
      images_set = *global::get<systems::core_t>()->image_controller->get_descriptor_set();
      
      current_inst_size = 2000;
      const size_t size = align_to(current_inst_size * sizeof(structure_data), 16);
      structures_instance.create(allocator, buffer(size, vk::BufferUsageFlagBits::eTransferSrc), vma::MemoryUsage::eCpuOnly, "structures_instance");
      gpu_structures_instance.create(allocator, buffer(size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer), vma::MemoryUsage::eGpuOnly, "gpu_structures_instance");

      {
        pipeline_layout_maker plm(&device);
        p_layout = plm.addDescriptorLayout(uniform_layout)
                      .addDescriptorLayout(images_layout)
                      .addDescriptorLayout(tiles_data_layout)
                      .create("tile_structure_rendering_pipeline_layout");
      }
      
      {
        const auto vertex   = create_shader_module(device, global::root_directory() + "shaders/structure.vert.spv");
        const auto fragment = create_shader_module(device, global::root_directory() + "shaders/first_object.frag.spv");

        pipeline_maker pm(&device);
        pm.clearBlending();

        pipe = pm.addShader(vk::ShaderStageFlagBits::eVertex, vertex.get())
                 .addShader(vk::ShaderStageFlagBits::eFragment, fragment.get())
                 //.vertexBinding(0, sizeof(uint32_t), vk::VertexInputRate::eInstance)
                 //  .vertexAttribute(0, 0, vk::Format::eR32Uint, 0)
                 .vertexBinding(0, sizeof(structure_data), vk::VertexInputRate::eInstance)
                   .vertexAttribute(0, 0, vk::Format::eR32Uint, offsetof123(structure_data, tile_index))
                   .vertexAttribute(1, 0, vk::Format::eR32Uint, offsetof123(structure_data, img))
                   .vertexAttribute(2, 0, vk::Format::eR32Sfloat, offsetof123(structure_data, scale))
                 .depthTest(VK_TRUE)
                 .depthWrite(VK_TRUE)
                 .frontFace(vk::FrontFace::eClockwise)
                 .cullMode(vk::CullModeFlagBits::eNone)
                 .assembly(vk::PrimitiveTopology::eTriangleStrip, VK_TRUE)
                 .viewport()
                 .scissor()
                 .dynamicState(vk::DynamicState::eViewport).dynamicState(vk::DynamicState::eScissor)
                 .colorBlendBegin(VK_FALSE)
                   .colorWriteMask(DEFAULT_COLOR_WRITE_MASK)
                 .create("tile_structure_rendering_pipeline", p_layout, info.cont->vlk_window->render_pass);
      }
    }
    
    tile_structure_render::~tile_structure_render() {
      device.destroy(p_layout);
      device.destroy(pipe);
      structures_instance.destroy(allocator);
      gpu_structures_instance.destroy(allocator);
    }
    
    void tile_structure_render::begin() {
      local_structures_count = structures_count;
      structures_count = 0;
    }
    
    void tile_structure_render::proccess(container* ctx) {
      if (local_structures_count == 0) return;
      
      auto map_systems = global::get<systems::map_t>();
      if (!map_systems->is_init()) return;
      auto map = map_systems->map;
      auto uniform = global::get<render::buffers>()->uniform_set;
      auto tiles = map->data->tiles_set;
      
      //auto indirect_buffer = opt->indirect_buffer();
      //auto indices_buffer = opt->structures_index_buffer();
//       auto indices_buffer = opt->borders_index_buffer();
//       if (indices_buffer == vk::Buffer(nullptr)) return;
      
      const auto bind_point = vk::PipelineBindPoint::eGraphics;
      auto task = ctx->command_buffer();
      task->bindPipeline(bind_point, pipe);
      task->bindDescriptorSets(bind_point, p_layout, 0, {uniform, images_set, tiles}, nullptr);
      //task->bindVertexBuffers(0, indices_buffer, {0});
      //task->drawIndirect(indirect_buffer, offsetof123(struct tile_optimizer::indirect_buffer, borders_command), 1, sizeof(vk::DrawIndirectCommand));
      
      task->bindVertexBuffers(0, gpu_structures_instance.handle, {0});
      task->draw(4, local_structures_count, 0, 0);
    }
    
    void tile_structure_render::clear() {
      if (local_structures_count > current_inst_size) {
        current_inst_size = local_structures_count;
        structures_instance.destroy(allocator);
        gpu_structures_instance.destroy(allocator);
        
        const size_t size = align_to(current_inst_size * sizeof(structure_data), 16);
        structures_instance.create(allocator, buffer(size, vk::BufferUsageFlagBits::eTransferSrc), vma::MemoryUsage::eCpuOnly, "structures_instance");
        gpu_structures_instance.create(allocator, buffer(size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer), vma::MemoryUsage::eGpuOnly, "gpu_structures_instance");
      }
      
      local_structures_count = 0;
    }
    
    void tile_structure_render::copy(container* ctx) const {
      const size_t instance_buffer_size = align_to(current_inst_size * sizeof(structure_data), 16);
      const vk::BufferCopy c1{ 0, 0, instance_buffer_size };
      auto task = ctx->command_buffer();
      task->copyBuffer(structures_instance.handle, gpu_structures_instance.handle, 1, &c1);
    }
    
    void tile_structure_render::add(const structure_data &data) {
      const uint32_t index = structures_count.fetch_add(1);
      if (index < current_inst_size) {
        auto inst = reinterpret_cast<structure_data*>(structures_instance.ptr);
        memcpy(&inst[index], &data, sizeof(data));
      }
    }
    
    struct heraldy_instance_data_t {
      uint32_t tile_index;
      uint32_t offset;
      uint32_t shield_layer;
      float scale;
      image_t frame;
      float frame_scale;
    };
    
    heraldies_render::heraldies_render(const create_info &info) : 
      device(info.cont->vulkan->device), 
      allocator(info.cont->vulkan->buffer_allocator),
      opt(info.opt), 
      map_buffers(info.map_buffers), 
      images_set(nullptr),
      chain_count(0),
      inst_count(0),
      current_buffer_size(0),
      current_inst_size(0),
      local_inst_count(0),
      local_chain_count(0)
    {
      auto uniform_layout = info.cont->vulkan->uniform_layout;
      auto tiles_data_layout = info.tiles_data_layout;
      auto images_layout = info.images_layout;
      auto heraldy_layout = global::get<render::buffers>()->heraldy_layout;
      images_set = *global::get<systems::core_t>()->image_controller->get_descriptor_set();
      
      // какой размер по умолчанию?
      current_inst_size = 2000+1; // попробуем так взять
      current_buffer_size = current_inst_size*5;
      layers_chain.create(allocator, render::buffer(align_to(current_buffer_size * sizeof(uint32_t), 16), vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc), vma::MemoryUsage::eCpuOnly);
      heraldy_instance.create(allocator, render::buffer(align_to(current_inst_size * sizeof(heraldy_instance_data_t), 16), vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferSrc), vma::MemoryUsage::eCpuOnly);
//       gpu_layers_chain.create(allocator, render::buffer(
//         align_to(
//           current_buffer_size * sizeof(uint32_t), 16), 
//           vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst
//         ), 
//         vma::MemoryUsage::eGpuOnly
//       );
      
      auto layers_buffer = &global::get<render::buffers>()->heraldy_indices;
      layers_buffer->destroy(allocator);
      layers_buffer->create(
        allocator, render::buffer(
        align_to(
          current_buffer_size * sizeof(uint32_t), 16), 
          vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst
        ), 
        vma::MemoryUsage::eGpuOnly
      );
      
      gpu_heraldy_instance.create(allocator, render::buffer(
        align_to(
          current_inst_size * sizeof(heraldy_instance_data_t), 16), 
          vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst
        ), 
        vma::MemoryUsage::eGpuOnly
      );
      
      {
        auto set = global::get<render::buffers>()->heraldy_set;
        descriptor_set_updater upd(&device);
        //upd.currentSet(set).begin(1, 0, vk::DescriptorType::eStorageBuffer).buffer(gpu_layers_chain.handle).update();
        upd.currentSet(set).begin(1, 0, vk::DescriptorType::eStorageBuffer).buffer(layers_buffer->handle).update();
      }
      
      // было бы неплохо эти буферы скопировать в гпу буферы в начале рендеринга, как можно это сделать?
      // для этого нужен утилити стейдж, в него мы положим указатели на классы с функцией копирования
      // функцию зададим например здесь

      {
        pipeline_layout_maker plm(&device);
        p_layout = plm.addDescriptorLayout(uniform_layout)
                      .addDescriptorLayout(images_layout)
                      .addDescriptorLayout(tiles_data_layout)
                      .addDescriptorLayout(heraldy_layout)
                      .create("heraldies_rendering_pipeline_layout");
      }
      
      {
        const auto vertex   = create_shader_module(device, global::root_directory() + "shaders/heraldy.vert.spv");
        const auto fragment = create_shader_module(device, global::root_directory() + "shaders/heraldy.frag.spv");

        pipeline_maker pm(&device);
//         pm.clearBlending();

        pipe = pm.addShader(vk::ShaderStageFlagBits::eVertex, vertex.get())
                 .addShader(vk::ShaderStageFlagBits::eFragment, fragment.get())
                 //.vertexBinding(0, sizeof(uint32_t), vk::VertexInputRate::eInstance)
                 //  .vertexAttribute(0, 0, vk::Format::eR32Uint, 0)
                 .vertexBinding(0, sizeof(heraldy_instance_data_t), vk::VertexInputRate::eInstance)
                   .vertexAttribute(0, 0, vk::Format::eR32Uint, offsetof123(heraldy_instance_data_t, tile_index))
                   .vertexAttribute(1, 0, vk::Format::eR32Uint, offsetof123(heraldy_instance_data_t, offset))
                   .vertexAttribute(2, 0, vk::Format::eR32Uint, offsetof123(heraldy_instance_data_t, shield_layer))
                   .vertexAttribute(3, 0, vk::Format::eR32Sfloat, offsetof123(heraldy_instance_data_t, scale))
                 .depthTest(VK_TRUE)
                 .depthWrite(VK_TRUE)
                 .frontFace(vk::FrontFace::eClockwise)
                 .cullMode(vk::CullModeFlagBits::eBack)
                 .assembly(vk::PrimitiveTopology::eTriangleStrip, VK_TRUE)
                 .viewport()
                 .scissor()
                 .dynamicState(vk::DynamicState::eViewport).dynamicState(vk::DynamicState::eScissor)
                 .colorBlendBegin(VK_TRUE)
                 .create("heraldies_rendering_pipeline", p_layout, info.cont->vlk_window->render_pass);
      }
    }
    
    heraldies_render::~heraldies_render() {
      device.destroy(p_layout);
      device.destroy(pipe);
      layers_chain.destroy(allocator);
      heraldy_instance.destroy(allocator);
//       gpu_layers_chain.destroy(allocator);
      gpu_heraldy_instance.destroy(allocator);
      
      // нужно ли удалить render::buffers::heraldy_indices?
    }
    
    void heraldies_render::begin() { 
      local_inst_count = inst_count;
      local_chain_count = chain_count;
      inst_count = 0; 
      chain_count = 0;
    }
    void heraldies_render::proccess(container* ctx) {
      if (local_inst_count == 0) return;
      
      auto map_systems = global::get<systems::map_t>();
      if (!map_systems->is_init()) return;
      auto map = map_systems->map;
      auto uniform = global::get<render::buffers>()->uniform_set;
      auto heraldy = global::get<render::buffers>()->heraldy_set;
      auto tiles = map->data->tiles_set;
      
//       auto indirect_buffer = opt->indirect_buffer();
      //auto indices_buffer = opt->heraldy_index_buffer();
//       auto indices_buffer = opt->walls_index_buffer();
//       if (indices_buffer == vk::Buffer(nullptr)) return;
      
      ASSERT(heraldy);
      
      // до рендеринга геральдики имеет смысл почистить буфер глубины
      // для того чтобы геральдика не входила внутрь земли, как это скажется на интерфейсе? фиг знает
      
      const vk::ClearAttachment att{vk::ImageAspectFlagBits::eDepth, 0, vk::ClearValue({1.0f, 0})};
      const vk::ClearRect rect{cast(ctx->size()), 0, 1};
      
      const auto bind_point = vk::PipelineBindPoint::eGraphics;
      auto task = ctx->command_buffer();
      task->clearAttachments({att}, {rect});
      task->bindPipeline(bind_point, pipe);
      //task->bindDescriptorSets(bind_point, p_layout, 0, {uniform, images_set, tiles, heraldy}, nullptr);
      //task->bindVertexBuffers(0, indices_buffer, {0});
      //task->drawIndirect(indirect_buffer, offsetof123(struct tile_optimizer::indirect_buffer, walls_command), 1, sizeof(vk::DrawIndirectCommand));
      
      task->bindDescriptorSets(bind_point, p_layout, 0, {uniform, images_set, tiles, heraldy}, nullptr);
      task->bindVertexBuffers(0, gpu_heraldy_instance.handle, {0});
      task->draw(4, local_inst_count+1, 0, 1); // наверное начнем со второго инстанса, в первом будет храниться heraldy_highlight_data
    }
    
    void heraldies_render::clear() {
      auto layers_buffer = &global::get<render::buffers>()->heraldy_indices;
      
      if (local_chain_count > current_buffer_size) {
        layers_chain.destroy(allocator);
        //gpu_layers_chain.destroy(allocator);
        layers_buffer->destroy(allocator);
      }
      
      if (local_inst_count+1 > current_inst_size) {
        heraldy_instance.destroy(allocator);
        gpu_heraldy_instance.destroy(allocator);
      }
      
      if (local_chain_count > current_buffer_size) {
        current_buffer_size = local_chain_count;
        
        layers_chain.create(allocator, render::buffer(align_to(current_buffer_size * sizeof(uint32_t), 16), vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst), vma::MemoryUsage::eCpuOnly, "layers_chain");
        layers_buffer->create(allocator, render::buffer( // gpu_layers_chain
          align_to(
            current_buffer_size * sizeof(uint32_t), 16), 
            vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst
          ), 
          vma::MemoryUsage::eGpuOnly,
          "gpu_layers_chain"
        );
        
        {
          auto set = global::get<render::buffers>()->heraldy_set;
          descriptor_set_updater upd(&device);
          //upd.currentSet(set).begin(1, 0, vk::DescriptorType::eStorageBuffer).buffer(gpu_layers_chain.handle).update();
          upd.currentSet(set).begin(1, 0, vk::DescriptorType::eStorageBuffer).buffer(layers_buffer->handle).update();
        }
      }
      
      if (local_inst_count+1 > current_inst_size) {
        current_inst_size = local_inst_count+1;
        
        heraldy_instance.create(allocator, 
          render::buffer(align_to(current_inst_size * sizeof(heraldy_instance_data_t), 16), vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst),
          vma::MemoryUsage::eCpuOnly,
          "heraldy_instance"
        );
        gpu_heraldy_instance.create(allocator, 
          render::buffer(align_to(current_inst_size * sizeof(heraldy_instance_data_t), 16), vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst),
          vma::MemoryUsage::eGpuOnly,
          "gpu_heraldy_instance"
        );
      }
      
      local_inst_count = 0;
      local_chain_count = 0;
    }
    
    void heraldies_render::copy(container* ctx) const {
      const size_t layers_buffer_size = align_to(current_buffer_size * sizeof(uint32_t), 16);
      const size_t heraldy_instance_buffer_size = align_to(current_inst_size * sizeof(heraldy_instance_data_t), 16);
      
      auto layers_buffer = &global::get<render::buffers>()->heraldy_indices;
      const vk::BufferCopy c1{ 0, 0, layers_buffer_size };
      const vk::BufferCopy c2{ 0, 0, heraldy_instance_buffer_size };
      auto task = ctx->command_buffer();
      //task->copyBuffer(layers_chain.handle, gpu_layers_chain.handle, 1, &c1);
      task->copyBuffer(layers_chain.handle, layers_buffer->handle, 1, &c1);
      task->copyBuffer(heraldy_instance.handle, gpu_heraldy_instance.handle, 1, &c2);
    }
    
    // че мне обходить видимые тайлы с предыдущего кадра? их может быть до 250к
    // на мобиле у меня будет 4 ядра (те по 60-70к тайлов на ядро), 
    // желательно потратить на это дело меньше 1мс
    // что нужно сделать? проверить кто на тайле, если город или армия, взять титул у владельца
    // передать в heraldies_render::add, городов и армий будет сильно меньше чем 60к-70к,
    // наверное 2-3к, а значит и все остальные операции беруться только 2-3к раз, 
    // потенциально я могу пробежать по видимым игроком городам и видимым игроком армиям, чтобы 
    // сократить количество, но как проверить видимость тайлов рендером? а нужно ли? 
    // нужно чтобы не рисовать лишний раз, мы можем видимость тайлов задать в буфере для
    // тайл оптимизера (15кб), пробегаемся по всем городам и армиям и смотрим что видим
    // это должно быть быстрее чем обходить все тайлы
    
    // сюда видимо еще придется передать индекс тайла, и еще каким то образом 
    // оформить разницу между типами геральдик (императорская, герцогская, армия и проч)
    // разницу оформим нарисовав рамку (или коронку сверху) вокруг геральдики
    size_t heraldies_render::add(const heraldy_data &data) {
      ASSERT(data.array_size < 256);
      const uint32_t offset = chain_count.fetch_add(data.array_size+1);
      const bool fit_buffer = offset+data.array_size+1 <= current_buffer_size;
      const uint32_t inst_index = inst_count.fetch_add(uint32_t(fit_buffer))+1;
      const bool fit_instance = inst_index < current_inst_size;
      if (fit_buffer && fit_instance) {
        auto ptr = reinterpret_cast<uint32_t*>(layers_chain.ptr);
        ptr[offset] = data.array_size;
        memcpy(&ptr[offset+1], data.array, sizeof(data.array[0])*data.array_size);
        
        // размер буфера?
        auto inst_ptr = reinterpret_cast<heraldy_instance_data_t*>(heraldy_instance.ptr);
        inst_ptr[inst_index].tile_index = data.tile_index;
        inst_ptr[inst_index].offset = offset;
        inst_ptr[inst_index].shield_layer = data.shield_layer;
        inst_ptr[inst_index].frame = data.frame;
        inst_ptr[inst_index].scale = data.scale;
        inst_ptr[inst_index].frame_scale = data.frame_scale;
      }
      
      return offset;
    }
    
    size_t heraldies_render::add(const heraldy_interface_data &data) {
      const uint32_t offset = chain_count.fetch_add(data.array_size+1);
      const bool fit_buffer = offset+data.array_size+1 <= current_buffer_size;
      if (fit_buffer) {
        auto ptr = reinterpret_cast<uint32_t*>(layers_chain.ptr);
        ptr[offset] = data.array_size;
        memcpy(&ptr[offset+1], data.array, sizeof(data.array[0])*data.array_size);
      }
      
      return fit_buffer ? offset : SIZE_MAX;
    }
    
    size_t heraldies_render::add_highlight(const heraldy_highlight_data &data) {
      // хайлайт нужно добавить наверное в первый слот heraldy_instance
      return SIZE_MAX;
    }
    
    armies_render::armies_render(const create_info &info) : 
      device(info.cont->vulkan->device), 
      allocator(info.cont->vulkan->buffer_allocator),
      opt(info.opt), 
      map_buffers(info.map_buffers), 
      images_set(nullptr),
      armies_count(0),
      local_armies_count(0),
      current_inst_size(0)
    {
      auto uniform_layout = info.cont->vulkan->uniform_layout;
      auto tiles_data_layout = info.tiles_data_layout;
      auto images_layout = info.images_layout;
      auto storage_layout = info.cont->vulkan->storage_layout;
      images_set = *global::get<systems::core_t>()->image_controller->get_descriptor_set();
      
      current_inst_size = 1000;
      const size_t size = align_to(current_inst_size * sizeof(army_data), 16);
      instance.create(allocator, buffer(size, vk::BufferUsageFlagBits::eTransferSrc), vma::MemoryUsage::eCpuOnly, "army_instance");
      gpu_instance.create(allocator, buffer(size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer), vma::MemoryUsage::eGpuOnly, "gpu_army_instance");

      {
        pipeline_layout_maker plm(&device);
        p_layout = plm.addDescriptorLayout(uniform_layout)
                      .addDescriptorLayout(images_layout)
                      .addDescriptorLayout(tiles_data_layout)
                      .addDescriptorLayout(storage_layout)
                      .create("armies_rendering_pipeline_layout");
      }
      
      {
        const auto vertex   = create_shader_module(device, global::root_directory() + "shaders/armies.vert.spv");
        const auto fragment = create_shader_module(device, global::root_directory() + "shaders/tiles.frag.spv");

        pipeline_maker pm(&device);
//         pm.clearBlending();

        pipe = pm.addShader(vk::ShaderStageFlagBits::eVertex, vertex.get())
                 .addShader(vk::ShaderStageFlagBits::eFragment, fragment.get())
                 //.vertexBinding(0, sizeof(uint32_t), vk::VertexInputRate::eInstance)
                 //  .vertexAttribute(0, 0, vk::Format::eR32Uint, 0)
                 .vertexBinding(0, sizeof(army_data), vk::VertexInputRate::eInstance)
                   .vertexAttribute(0, 0, vk::Format::eR32G32B32Sfloat, offsetof123(army_data, pos))
                   .vertexAttribute(1, 0, vk::Format::eR32Uint, offsetof123(army_data, img))
                   .vertexAttribute(2, 0, vk::Format::eR32Sfloat, offsetof123(army_data, scale))
                 .depthTest(VK_TRUE)
                 .depthWrite(VK_TRUE)
                 .frontFace(vk::FrontFace::eClockwise)
                 .cullMode(vk::CullModeFlagBits::eNone)
                 .assembly(vk::PrimitiveTopology::eTriangleStrip, VK_TRUE)
                 .viewport()
                 .scissor()
                 .dynamicState(vk::DynamicState::eViewport).dynamicState(vk::DynamicState::eScissor)
                 .colorBlendBegin(VK_TRUE)
                 .create("armies_rendering_pipeline", p_layout, info.cont->vlk_window->render_pass);
      }
    }
    
    armies_render::~armies_render() {
      device.destroy(p_layout);
      device.destroy(pipe);
      instance.destroy(allocator);
      gpu_instance.destroy(allocator);
    }
    
    void armies_render::begin() {
      local_armies_count = armies_count;
      armies_count = 0;
    }
    
    void armies_render::proccess(container* ctx) {
      if (local_armies_count == 0) return;
      
      auto map_systems = global::get<systems::map_t>();
      if (!map_systems->is_init()) return;
      auto map = map_systems->map;
      auto uniform = global::get<render::buffers>()->uniform_set;
//       auto heraldy = global::get<render::buffers>()->heraldy;
      auto tiles = map->data->tiles_set;
      
//       auto indirect_buffer = opt->indirect_buffer();
//       auto indices_buffer = opt->objects_index_buffer();
//       if (indices_buffer == vk::Buffer(nullptr)) return;
      
      const auto bind_point = vk::PipelineBindPoint::eGraphics;
      auto task = ctx->command_buffer();
      task->bindPipeline(bind_point, pipe);
      task->bindDescriptorSets(bind_point, p_layout, 0, {uniform, images_set, tiles}, nullptr); // heraldy->descriptorSet()->handle()
//       task->bindVertexBuffers(0, indices_buffer, {0});
//       task->drawIndirect(indirect_buffer, offsetof123(struct tile_optimizer::indirect_buffer, structures_command), 1, sizeof(vk::DrawIndirectCommand));
      
      task->bindVertexBuffers(0, gpu_instance.handle, {0});
      task->draw(4, local_armies_count, 0, 0);
    }
    
    void armies_render::clear() {
      if (local_armies_count > current_inst_size) {
        current_inst_size = local_armies_count;
        
        instance.destroy(allocator);
        gpu_instance.destroy(allocator);
        
        const size_t size = align_to(current_inst_size * sizeof(army_data), 16);
        instance.create(allocator, buffer(size, vk::BufferUsageFlagBits::eTransferSrc), vma::MemoryUsage::eCpuOnly, "army_instance");
        gpu_instance.create(allocator, buffer(size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer), vma::MemoryUsage::eGpuOnly, "gpu_army_instance");
      }
      
      local_armies_count = 0;
    }
    
    // возможно копировать придется в другом месте (в каком?)
    // запускаем рендер - обходим города/армии/геральдики - ждем окончания рендера - копируем
    void armies_render::copy(container* ctx) const {
      const size_t current_instance_buffer_size = align_to(current_inst_size * sizeof(army_data), 16);
      const vk::BufferCopy c1{ 0, 0, current_instance_buffer_size };
      auto task = ctx->command_buffer();
      task->copyBuffer(instance.handle, gpu_instance.handle, 1, &c1);
    }
      
    void armies_render::add(const army_data &data) {
      const uint32_t index = armies_count.fetch_add(1);
      if (index < current_inst_size) {
        auto arr = reinterpret_cast<army_data*>(instance.ptr);
        memcpy(&arr[index], &data, sizeof(data));
      }
    }
    
    world_map_render::world_map_render(const create_info &info) : stage_container(info.container_size) {}
    void world_map_render::begin() {
      stage_container::begin();
    }
    
    void world_map_render::proccess(struct container* ctx) {
      auto map_systems = global::get<systems::map_t>();
      if (!map_systems->is_init()) return;
      
      stage_container::proccess(ctx);
    }
    
    void world_map_render::clear() {
      stage_container::clear();
    }
    
//     void world_map_render::recreate_pipelines(const game::image_resources_t* resource) { (void)resource; }
    
    struct gui_vertex {
      glm::vec2 pos;
      glm::vec2 uv;
      uint32_t color;
    };

#define MAX_VERTEX_BUFFER (2 * 1024 * 1024)
#define MAX_INDEX_BUFFER (512 * 1024)
    
    interface_stage::interface_stage(const create_info &info) : 
      device(info.cont->vulkan->device),
      allocator(info.cont->vulkan->buffer_allocator)
    {
      vertex_gui.create(allocator, buffer(MAX_VERTEX_BUFFER, vk::BufferUsageFlagBits::eVertexBuffer), vma::MemoryUsage::eCpuOnly, "interface_stage::vertex_gui");
      index_gui.create(allocator, buffer(MAX_INDEX_BUFFER, vk::BufferUsageFlagBits::eIndexBuffer), vma::MemoryUsage::eCpuOnly, "interface_stage::index_gui"); 
      matrix.create(allocator, buffer(sizeof(glm::mat4), vk::BufferUsageFlagBits::eUniformBuffer), vma::MemoryUsage::eCpuOnly, "interface_stage::matrix");
      
      auto uniform_layout = info.cont->vulkan->uniform_layout;
      auto images_layout = info.images_layout;
//       auto storage_layout = info.cont->vulkan->storage_layout;
      ASSERT(global::get<render::buffers>() != nullptr);
      auto heraldy_layout = global::get<render::buffers>()->heraldy_layout;
      images_set = *global::get<systems::core_t>()->image_controller->get_descriptor_set();
      
      {
        // вообще плохо тут использовать общий пул, но с другой стороны эта штука создается и удаляется только один раз за программу
        auto pool = info.cont->vulkan->descriptor_pool;
    
        descriptor_set_maker dm(&device);
        matrix_set = dm.layout(uniform_layout).create(pool, "interface_stage::matrix_set")[0];
        
        descriptor_set_updater dsu(&device);
        dsu.currentSet(matrix_set).begin(0, 0, vk::DescriptorType::eUniformBuffer).buffer(matrix.handle).update();
      }
      
      { 
        pipeline_layout_maker plm(&device);
        p_layout = plm.addDescriptorLayout(uniform_layout)
                      .addDescriptorLayout(images_layout)
                      .addDescriptorLayout(heraldy_layout)
                      .addPushConstRange(0, sizeof(image_handle_data))
                      .create("gui_layout");
      }
      
      {
        pipeline_maker pm(&device);
        pm.clearBlending();
        
        const auto vertex   = create_shader_module(device, global::root_directory() + "shaders/gui.vert.spv");
        const auto fragment = create_shader_module(device, global::root_directory() + "shaders/gui.frag.spv");
        
        pipe = pm.addShader(vk::ShaderStageFlagBits::eVertex, vertex.get())
                 .addShader(vk::ShaderStageFlagBits::eFragment, fragment.get())
                 .vertexBinding(0, sizeof(gui_vertex))
                   .vertexAttribute(0, 0, vk::Format::eR32G32Sfloat, offsetof(gui_vertex, pos))
                   .vertexAttribute(1, 0, vk::Format::eR32G32Sfloat, offsetof(gui_vertex, uv))
                   .vertexAttribute(2, 0, vk::Format::eR8G8B8A8Uint, offsetof(gui_vertex, color))
                 .assembly(vk::PrimitiveTopology::eTriangleList)
                 .depthTest(VK_FALSE)
                 .depthWrite(VK_FALSE)
                 .clearBlending()
                 .colorBlendBegin()
                   .srcColor(vk::BlendFactor::eSrcAlpha)
                   .dstColor(vk::BlendFactor::eOneMinusSrcAlpha)
                   .srcAlpha(vk::BlendFactor::eOneMinusSrcAlpha)
                   .dstAlpha(vk::BlendFactor::eSrcAlpha)
                 .viewport()
                 .scissor()
                 .dynamicState(vk::DynamicState::eViewport).dynamicState(vk::DynamicState::eScissor)
                 .create("gui_pipeline", p_layout, info.cont->vlk_window->render_pass);
      }
    }
    
    interface_stage::~interface_stage() {
      vertex_gui.destroy(allocator);
      index_gui.destroy(allocator);
      matrix.destroy(allocator);
      device.destroy(p_layout);
      device.destroy(pipe);
    }
    
    void interface_stage::begin() {
      auto data = global::get<interface::context>();
      auto w = global::get<render::window>();
  
      {
        void* vertices = vertex_gui.ptr;
        void* elements = index_gui.ptr;
        
        /* fill convert configuration */
        static const struct nk_draw_vertex_layout_element vertex_layout[] = {
          {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, offsetof(gui_vertex, pos)},
          {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, offsetof(gui_vertex, uv)},
          {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, offsetof(gui_vertex, color)},
          {NK_VERTEX_LAYOUT_END}
        };
        
        // нужен ли мне изменяющийся антиаляисинг?
        const nk_convert_config config{
          1.0f,
          NK_ANTI_ALIASING_OFF,
          NK_ANTI_ALIASING_OFF,
          22,
          22,
          22,
          data->null, // нулл текстура
          vertex_layout,
          sizeof(gui_vertex), // размер вершины
          alignof(gui_vertex) // алигн вершины
        };
        
        nk_buffer vbuf, ebuf;
        // мы создаем местные аналоги vkBuffer, то есть мета объект для памяти
        // а затем конвертим вершины в удобный нам формат
        //nk_buffer_init_fixed(&vbuf, vertices, size_t(MAX_VERTEX_BUFFER));
        //nk_buffer_init_fixed(&ebuf, elements, size_t(MAX_INDEX_BUFFER));
        nk_buffer_init_fixed(&vbuf, vertices, MAX_VERTEX_BUFFER);
        nk_buffer_init_fixed(&ebuf, elements, MAX_INDEX_BUFFER);
        nk_convert(&data->ctx, &data->cmds, &vbuf, &ebuf, &config);
      }
      
      const auto [width, height] = w->size();
      glm::mat4* mat = reinterpret_cast<glm::mat4*>(matrix.ptr);
      *mat = glm::mat4(
        2.0f / float(width),  0.0f,  0.0f,  0.0f,
        0.0f,  2.0f / float(height),  0.0f,  0.0f,
        0.0f,  0.0f, -1.0f,  0.0f,
        -1.0f, -1.0f,  0.0f,  1.0f
      );
      
      //commands
      const nk_draw_command *cmd = nullptr;
      nk_draw_foreach(cmd, &data->ctx, &data->cmds) {
        const interface_draw_command command{
          cmd->elem_count,
          { cmd->clip_rect.x, cmd->clip_rect.y, cmd->clip_rect.w, cmd->clip_rect.h },
          cmd->texture.ptr,
          cmd->userdata.ptr
        };
        
        commands.push_back(command);
      }
      
      nk_buffer_clear(&data->cmds);
      nk_clear(&data->ctx);
    }
    
    void interface_stage::proccess(container* ctx) {
      auto task = ctx->command_buffer();
      
      const auto bind_point = vk::PipelineBindPoint::eGraphics;
      task->bindPipeline(bind_point, pipe);
      task->bindVertexBuffers(0, vertex_gui.handle, {0});
      task->bindIndexBuffer(index_gui.handle, 0, vk::IndexType::eUint16);
      
      auto heraldy = global::get<render::buffers>()->heraldy_set;
      task->bindDescriptorSets(bind_point, p_layout, 0, {matrix_set, images_set, heraldy}, nullptr);
      
      uint32_t index_offset = 0;
      for (const auto &cmd : commands) {
        if (cmd.elem_count == 0) continue;
        
        const image_handle_data data = nk_handle_to_image_data(nk_handle{cmd.texture});
        task->pushConstants(p_layout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(data), &data);
        const glm::vec2 fb_scale = input::get_input_data()->fb_scale;
        const vk::Rect2D scissor{
          {
            int32_t(std::max(cmd.clip_rect.x * fb_scale.x, 0.0f)),
            int32_t(std::max(cmd.clip_rect.y * fb_scale.y, 0.0f))
          },
          {
            uint32_t(cmd.clip_rect.w * fb_scale.x),
            uint32_t(cmd.clip_rect.h * fb_scale.y)
          }
        };
        
        task->setScissor(0, scissor);
        task->drawIndexed(cmd.elem_count, 1, index_offset, 0, 0);
        index_offset += cmd.elem_count;
      }
    }
    
    void interface_stage::clear() {
      commands.clear();
    }

    task_start::task_start(vk::Device device) : device(device) {}
    void task_start::begin() {}
    void task_start::proccess(container* ctx) {
      auto task = ctx->command_buffer();
      
      // ctx должен вернуть семафоры и фенс + кью
      rendering_fence = *ctx->current_frame_fence();
      auto img_sem = *ctx->image_wait_semaphore();
      auto render_sem = *ctx->finish_semaphore();
      auto wait_stage = vk::PipelineStageFlags(ctx->wait_pipeline_stage());
      auto queue = *ctx->queue();
      
      const vk::SubmitInfo info(img_sem, wait_stage, *task, render_sem);
      queue.submit(info, rendering_fence);
      ctx->present();
    }

    void task_start::clear() {}
    // если мы wait передадим в другой поток, то тут мы можем разблокировать ресурсы (карту)
    // и так это будет выглядеть наиболее нормально, другое дело что нам нужно ждать этот поток
    // иначе мы не отловим ошибку + мы можем попытаться подождать всю загрузку используя обычное ожидание
    void task_start::wait() {
      const size_t ten_second = size_t(10) * NANO_PRECISION;
      const auto res = device.waitForFences(rendering_fence, VK_TRUE, ten_second);
      if (res != vk::Result::eSuccess) { throw std::runtime_error("drawing takes too long"); }
    }
    
    bool task_start::status() const {
      const auto res = device.getFenceStatus(rendering_fence);
      ASSERT(res != vk::Result::eErrorDeviceLost);
      return res == vk::Result::eSuccess;
    }

    window_present::window_present() {}
    void window_present::begin() {}
    void window_present::proccess(container* ctx) { 
//       w->present(); 
      (void)ctx; 
    }
    void window_present::clear() {}
  }
}
