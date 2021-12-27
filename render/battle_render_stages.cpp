#include "battle_render_stages.h"

#include "bin/battle_map.h"

#include "utils/globals.h"
#include "utils/systems.h"

#include "targets.h"
#include "shared_structures.h"
#include "shared_battle_structures.h"
#include "container.h"
#include "image_controller.h"
#include "window.h"
#include "container.h"
#include "makers.h"
#include "defines.h"
#include "pass.h"

namespace devils_engine {
  namespace render {
    namespace battle {
      tile_optimizer::tile_optimizer(const create_info &info) : 
        device(info.cont->vulkan->device), 
        allocator(info.cont->vulkan->buffer_allocator),
        set(nullptr)
      {
        tiles_indices_size = 16;
        biomes_indices_size = 16;
        units_indices_size = 16;
        
        indirect_buffer.create(allocator, buffer(sizeof(indirect_buffer_data), vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eIndirectBuffer), vma::MemoryUsage::eCpuOnly);
        tiles_indices.create(allocator, buffer(16, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eVertexBuffer), vma::MemoryUsage::eGpuOnly);
        biomes_indices.create(allocator, buffer(16, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eIndexBuffer), vma::MemoryUsage::eGpuOnly);
        units_indices.create(allocator, buffer(16, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eVertexBuffer), vma::MemoryUsage::eGpuOnly);
        selection_indices.create(allocator, buffer(selection_buffer_size, vk::BufferUsageFlagBits::eStorageBuffer), vma::MemoryUsage::eGpuOnly);
        
        {
          descriptor_set_layout_maker dlm(&device);
          buffer_layout = dlm.binding(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute)
                             .binding(1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute)
                             .binding(2, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute)
                             .binding(3, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute)
                             .binding(4, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute)
                             .create("battle_tile_buffer_layout");
        }
        
        {
          descriptor_pool_maker dpm(&device);
          pool = dpm.poolSize(vk::DescriptorType::eStorageBuffer, 5).create("battle_tile_buffer_pool");
        }
        
        {
          descriptor_set_maker dm(&device);
          set = dm.layout(buffer_layout).create(pool, "battle_buffers_descriptor_set")[0];
          
          descriptor_set_updater dsu(&device);
          dsu.currentSet(set);
          dsu.begin(0, 0, vk::DescriptorType::eStorageBuffer).buffer(indirect_buffer.handle);
          dsu.begin(1, 0, vk::DescriptorType::eStorageBuffer).buffer(tiles_indices.handle);
          dsu.begin(2, 0, vk::DescriptorType::eStorageBuffer).buffer(biomes_indices.handle);
          dsu.begin(3, 0, vk::DescriptorType::eStorageBuffer).buffer(units_indices.handle);
          dsu.begin(4, 0, vk::DescriptorType::eStorageBuffer).buffer(selection_indices.handle);
          dsu.update();
        }
        
        auto map_layout = info.map_layout;
        auto uniform_layout = info.cont->vulkan->uniform_layout;
        {
          pipeline_layout_maker plm(&device);
          p_layout = plm.addDescriptorLayout(uniform_layout)
                        .addDescriptorLayout(map_layout)
                        .addDescriptorLayout(buffer_layout)
                        .create("battle_tile_optimizer_pipeline_layout");
        }
        
        {
          const auto compute   = create_shader_module(device, global::root_directory() + "shaders/battle_map_tiles.vert.spv");
          
          compute_pipeline_maker cpm(&device);
          pipe = cpm.shader(compute.get()).create("battle_tile_optimizer_pipeline", p_layout);
        }
      }
      
      tile_optimizer::~tile_optimizer() {
        indirect_buffer.destroy(allocator);
        tiles_indices.destroy(allocator);
        biomes_indices.destroy(allocator);
        units_indices.destroy(allocator);
        selection_indices.destroy(allocator);
        
        device.destroy(buffer_layout);
        device.destroy(pool);
        
        device.destroy(p_layout);
        device.destroy(pipe);
      }
      
      static const utils::frustum default_fru{
        glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), 
        glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), 
        glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), 
        glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), 
        glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), 
        glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)
      };
      
      void tile_optimizer::begin(resource_provider*) {
        auto buffers = global::get<render::buffers>();
        
        const auto &mat = buffers->get_matrix();
        const auto &fru = utils::compute_frustum(mat);
        
        auto indirect_data = reinterpret_cast<indirect_buffer_data*>(indirect_buffer.ptr);
        indirect_data->frustum = fru;
        
        indirect_data->tiles_indirect.indexCount = 21;
        indirect_data->tiles_indirect.instanceCount = 0;
        indirect_data->tiles_indirect.firstIndex = 0;
        indirect_data->tiles_indirect.vertexOffset = 0;
        indirect_data->tiles_indirect.firstInstance = 0;
        
        indirect_data->sizes_data.x = 0;
        indirect_data->sizes_data.y = 0;
        indirect_data->sizes_data.z = 0;
        
        indirect_data->units_indirect.vertexCount = 4;
        indirect_data->units_indirect.instanceCount = 0;
        indirect_data->units_indirect.firstVertex = 0;
        indirect_data->units_indirect.firstInstance = 0;
        
//         auto battle = global::get<systems::battle_t>();
//         indirect_data->sizes_data.x = battle->is_init() ? battle->map->tiles_count : 0;
//         indirect_data->sizes_data.y = battle->is_init() ? battle->map->width : 0;
//         indirect_data->sizes_data.z = battle->is_init() ? battle->map->height : 0;
        
        indirect_data->ray_pos = buffers->get_pos();
        indirect_data->ray_dir = buffers->get_cursor_dir();
        
        for (size_t i = 0; i < BATTLE_BIOMES_MAX_COUNT; ++i) {
          indirect_data->biomes_indirect[i].objects_indirect[0] = 0;
          indirect_data->biomes_indirect[i].objects_indirect[1] = 0;
          indirect_data->biomes_indirect[i].objects_indirect[2] = 0;
          indirect_data->biomes_indirect[i].objects_indirect[3] = 0;
          indirect_data->biomes_indirect[i].objects_data[0] = 0;
          indirect_data->biomes_indirect[i].objects_data[3] = 0;
        }
        
        auto ptr = selection_indices.ptr;
        memset(ptr, 0, selection_buffer_size);
      }
      
      bool tile_optimizer::process(resource_provider* ctx, vk::CommandBuffer task) {
        auto battle = global::get<systems::battle_t>();
        if (!battle->is_init()) return false;
        
        auto uniform_set = ctx->get_descriptor_set(string_hash(UNIFORM_BUFFERS_DESCRIPTOR_SET_NAME));
        auto map_set = ctx->get_descriptor_set(string_hash(BATTLE_MAP_TILES_BUFFERS_DESCRIPTOR_SET_NAME));
        
        const auto bind_p = vk::PipelineBindPoint::eCompute;
        task.bindPipeline(bind_p, pipe);
        task.bindDescriptorSets(bind_p, p_layout, 0, { uniform_set, map_set, set }, nullptr);
        const uint32_t count = std::ceil(float(battle->map->tiles_count) / float(work_group_size));
        task.dispatch(count, 1, 1);
        
        return true;
      }
      
      void tile_optimizer::clear() {
        auto indirect_data = reinterpret_cast<indirect_buffer_data*>(indirect_buffer.ptr);
        indirect_data->selection_frustum = default_fru;
      }
      
      vk::Buffer tile_optimizer::get_indirect_buffer() const { return indirect_buffer.handle; }
      vk::Buffer tile_optimizer::get_tiles_indices() const { return tiles_indices.handle; }
      vk::Buffer tile_optimizer::get_biomes_indices() const { return biomes_indices.handle; }
      vk::Buffer tile_optimizer::get_units_indices() const { return units_indices.handle; }
      vk::Buffer tile_optimizer::get_selection_indices() const { return selection_indices.handle; }
      
      vk::DescriptorSetLayout tile_optimizer::get_buffer_layout() const { return buffer_layout; }
      vk::DescriptorSet tile_optimizer::get_set() const { return set; }
      
      void tile_optimizer::update_containers() {
        auto battle = global::get<systems::battle_t>();
        ASSERT(battle->is_init());
        const size_t tiles_indices_size = align_to(battle->map->tiles_count * sizeof(instance_data_t) / 4, 16);
        const size_t biomes_indices_size = align_to(battle->map->tiles_count * 5 * sizeof(uint32_t), 16);
        tiles_indices.destroy(allocator);
        biomes_indices.destroy(allocator);
        
        tiles_indices.create(allocator, buffer(tiles_indices_size, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eVertexBuffer), vma::MemoryUsage::eGpuOnly);
        biomes_indices.create(allocator, buffer(biomes_indices_size, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eIndexBuffer), vma::MemoryUsage::eGpuOnly);
        
        descriptor_set_updater dsu(&device);
        dsu.currentSet(set);
        dsu.begin(1, 0, vk::DescriptorType::eStorageBuffer).buffer(tiles_indices.handle);
        dsu.begin(2, 0, vk::DescriptorType::eStorageBuffer).buffer(biomes_indices.handle);
        // юниты?
        dsu.update();
      }
      
      void tile_optimizer::update_unit_container() {
        auto battle = global::get<systems::battle_t>();
        ASSERT(battle->is_init());
        
        std::unique_lock<std::mutex> lock(battle->map->mutex);
        const size_t units_size = align_to(battle->map->units_count * sizeof(uint32_t), 16);
        units_indices.destroy(allocator);
        units_indices.create(allocator, buffer(units_size, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eVertexBuffer), vma::MemoryUsage::eGpuOnly);
        
        descriptor_set_updater dsu(&device);
        dsu.currentSet(set);
        dsu.begin(3, 0, vk::DescriptorType::eStorageBuffer).buffer(units_indices.handle);
        dsu.update();
      }
      
      void tile_optimizer::update_selection_frustum(const utils::frustum &fru) {
        auto buffer = reinterpret_cast<indirect_buffer_data*>(indirect_buffer.ptr);
        buffer->selection_frustum = fru;
      }
      
      void tile_optimizer::update_biome_data(const std::array<std::pair<uint32_t, uint32_t>, BATTLE_BIOMES_MAX_COUNT> &data) {
        auto buffer = reinterpret_cast<struct indirect_buffer_data*>(indirect_buffer.ptr);
        for (uint32_t i = 0; i < MAX_BIOMES_COUNT; ++i) {
          buffer->biomes_indirect[i].objects_data[1] = data[i].first;  // offset
          buffer->biomes_indirect[i].objects_data[2] = data[i].second; // size
          buffer->biomes_indirect[i].objects_data[3] = 0;
        }
      }
      
      void tile_optimizer::update_mouse_dir_data(const glm::vec4 &pos, const glm::vec4 &dir) {
        auto buffer = reinterpret_cast<struct indirect_buffer_data*>(indirect_buffer.ptr);
        buffer->ray_pos = pos;
        buffer->ray_dir = dir;
      }
      
      uint32_t tile_optimizer::get_selection_count() const {
        auto buffer = reinterpret_cast<struct indirect_buffer_data*>(indirect_buffer.ptr);
        return buffer->sizes_data.z / 2;
      }
      
      const uint32_t* tile_optimizer::get_selection_data() const {
        return reinterpret_cast<uint32_t*>(selection_indices.ptr);
      }
      
      const size_t tile_optimizer::work_group_size;
      const size_t tile_optimizer::selection_slots;
      const size_t tile_optimizer::selection_buffer_size;
      
      // это при условии что у нас одна текстурка на стенки
      const uint16_t tile_indices[] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1, UINT16_MAX, 12, 13, 14, 15, 16, 17
      };
      
      tile_render::tile_render(const create_info &info) : 
        device(info.cont->vulkan->device), 
        allocator(info.cont->vulkan->buffer_allocator),
        opt(info.opt), 
        images_set(nullptr) 
      {
        static_assert(sizeof(tile_indices) == 21 * sizeof(uint16_t));
        const size_t buffer_size = sizeof(tile_indices);
        points_indices.create(allocator, buffer(buffer_size, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst), vma::MemoryUsage::eGpuOnly);
        
        {
          auto staging = render::create_buffer_unique(allocator, render::buffer(buffer_size, vk::BufferUsageFlagBits::eTransferSrc), vma::MemoryUsage::eCpuOnly);
          memcpy(staging.ptr, tile_indices, buffer_size);
          
          auto pool = info.cont->vulkan->transfer_command_pool;
          auto fence = info.cont->vulkan->transfer_fence;
          auto queue = info.cont->vulkan->graphics;
          
          do_command(
            device, pool, queue, fence,
            [&] (vk::CommandBuffer task) {
              const vk::BufferCopy c{0, 0, buffer_size};
              vk::CommandBufferBeginInfo info(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
              task.begin(info);
              task.copyBuffer(staging.handle, points_indices.handle, c);
              task.end();
            }
          );
        }
        
        images_set = *global::get<systems::core_t>()->image_controller->get_descriptor_set();
        auto images_layout = *global::get<systems::core_t>()->image_controller->get_descriptor_set_layout();
        
        auto map_layout = info.map_layout;
        auto uniform_layout = info.cont->vulkan->uniform_layout;
        {
          pipeline_layout_maker plm(&device);
          p_layout = plm.addDescriptorLayout(uniform_layout)
                        .addDescriptorLayout(images_layout)
                        .addDescriptorLayout(map_layout)
                        .create("battle_tile_render_pipeline_layout");
        }
        
        {
          const auto vertex   = create_shader_module(device, global::root_directory() + "shaders/battle_map_tiles.vert.spv");
          const auto fragment = create_shader_module(device, global::root_directory() + "shaders/battle_map_tiles.frag.spv");
          
          pipeline_maker pm(&device);
          pipe = pm.addShader(vk::ShaderStageFlagBits::eVertex, vertex.get())
                   .addShader(vk::ShaderStageFlagBits::eFragment, fragment.get())
                   .vertexBinding(0, sizeof(uint32_t), vk::VertexInputRate::eInstance) //sizeof(instance_data_t)
                     .vertexAttribute(0, 0, vk::Format::eR32Uint, 0)
                   .depthTest(VK_TRUE)
                   .depthWrite(VK_TRUE)
                   .frontFace(vk::FrontFace::eCounterClockwise)
                   .cullMode(vk::CullModeFlagBits::eNone)
                   .assembly(vk::PrimitiveTopology::eTriangleStrip, VK_TRUE)
                   .viewport()
                   .scissor()
                   .dynamicState(vk::DynamicState::eViewport).dynamicState(vk::DynamicState::eScissor)
                   .addDefaultBlending()
                   .create("battle_tile_render_pipeline", p_layout, info.renderpass->get_handle());
        }
      }
      
      tile_render::~tile_render() {
        points_indices.destroy(allocator);
        device.destroy(p_layout);
        device.destroy(pipe);
      }
      
    // виндовс не дает использовать базовый offsetof
#define offsetof123(s,m) ((::size_t)&reinterpret_cast<char const volatile&>((((s*)0)->m)))
      
      void tile_render::begin(resource_provider*) {}
      bool tile_render::process(resource_provider* ctx, vk::CommandBuffer task) {
        auto battle = global::get<systems::battle_t>();
        if (!battle->is_init()) return false;
        
        auto uniform_set = ctx->get_descriptor_set(string_hash(UNIFORM_BUFFERS_DESCRIPTOR_SET_NAME));
        auto map_set = ctx->get_descriptor_set(string_hash(BATTLE_MAP_TILES_BUFFERS_DESCRIPTOR_SET_NAME));
        
        auto indirect_buffer = opt->get_indirect_buffer();
        auto indices_buffer = opt->get_tiles_indices();
        
        const auto bind = vk::PipelineBindPoint::eGraphics;
        task.bindPipeline(bind, pipe);
        task.bindDescriptorSets(bind, p_layout, 0, {uniform_set, images_set, map_set}, nullptr);
        task.bindIndexBuffer(points_indices.handle, 0, vk::IndexType::eUint16); // (!)
        task.bindVertexBuffers(0, indices_buffer, {0});
        task.drawIndexedIndirect(indirect_buffer, offsetof123(struct tile_optimizer::indirect_buffer_data, tiles_indirect), 1, sizeof(vk::DrawIndexedIndirectCommand));
        
        return true;
      }
      
      void tile_render::clear() {}
      
      biome_render::biome_render(const create_info &info) : 
        device(info.cont->vulkan->device), 
        allocator(info.cont->vulkan->buffer_allocator),
        opt(info.opt), 
        multidraw(info.cont->is_properties_presented(render::container::physical_device_multidraw_indirect)) 
      {
        auto uniform_layout = info.cont->vulkan->uniform_layout;
        auto map_layout = info.map_layout;
        auto images_layout = *global::get<systems::core_t>()->image_controller->get_descriptor_set_layout();
        images_set = *global::get<systems::core_t>()->image_controller->get_descriptor_set();

        {
          pipeline_layout_maker plm(&device);
          p_layout = plm.addDescriptorLayout(uniform_layout)
                        .addDescriptorLayout(images_layout)
                        .addDescriptorLayout(map_layout)
                        .create("object_rendering_pipeline_layout");
        }
        
        {
          const auto vertex   = create_shader_module(device, global::root_directory() + "shaders/battle_biome_objects.vert.spv");
          const auto fragment = create_shader_module(device, global::root_directory() + "shaders/battle_biome_objects.frag.spv");

          pipeline_maker pm(&device);
          pm.clearBlending();

          pipe = pm.addShader(vk::ShaderStageFlagBits::eVertex, vertex.get())
                   .addShader(vk::ShaderStageFlagBits::eFragment, fragment.get())
                   .depthTest(VK_TRUE)
                   .depthWrite(VK_TRUE)
                   .frontFace(vk::FrontFace::eClockwise)
                   .cullMode(vk::CullModeFlagBits::eNone)
                   .assembly(vk::PrimitiveTopology::eTriangleStrip, VK_TRUE)
                   .viewport()
                   .scissor()
                   .dynamicState(vk::DynamicState::eViewport).dynamicState(vk::DynamicState::eScissor)
                   .addDefaultBlending()
                   .create("object_rendering_pipeline", p_layout, info.renderpass->get_handle());
        }
      }
      
      biome_render::~biome_render() {
        device.destroy(p_layout);
        device.destroy(pipe);
      }
      
      void biome_render::begin(resource_provider*) {}
      bool biome_render::process(resource_provider* ctx, vk::CommandBuffer task) {
        auto battle = global::get<systems::battle_t>();
        if (!battle->is_init()) return false;
        
        auto uniform_set = ctx->get_descriptor_set(string_hash(UNIFORM_BUFFERS_DESCRIPTOR_SET_NAME));
        auto map_set = ctx->get_descriptor_set(string_hash(BATTLE_MAP_TILES_BUFFERS_DESCRIPTOR_SET_NAME));
        
        // алгоритм похож на ворлд мап биом рендер
        
        auto indirect_buffer = opt->get_indirect_buffer();
        auto biomes_indices = opt->get_biomes_indices();
        
        const auto bind = vk::PipelineBindPoint::eGraphics;
        task.bindPipeline(bind, pipe);
        task.bindDescriptorSets(bind, p_layout, 0, {uniform_set, images_set, map_set}, nullptr);
        task.bindIndexBuffer(biomes_indices, 0, vk::IndexType::eUint32);
        
        if (multidraw) {
          task.drawIndexedIndirect(indirect_buffer, offsetof123(tile_optimizer::indirect_buffer_data, biomes_indirect), BATTLE_BIOMES_MAX_COUNT, sizeof(biome_objects_data_t));
        } else {
          for (size_t i = 0; i < BATTLE_BIOMES_MAX_COUNT; ++i) {
            task.drawIndexedIndirect(indirect_buffer, offsetof123(tile_optimizer::indirect_buffer_data, biomes_indirect)+sizeof(biome_objects_data_t)*i, 1, sizeof(vk::DrawIndexedIndirectCommand));
          }
        }
        
        return true;
      }
      
      void biome_render::clear() {}
      
      units_render::units_render(const create_info &info) : 
        device(info.cont->vulkan->device), 
        allocator(info.cont->vulkan->buffer_allocator),
        opt(info.opt) 
      {
        auto uniform_layout = info.cont->vulkan->uniform_layout;
        auto map_layout = info.map_layout;
        auto images_layout = *global::get<systems::core_t>()->image_controller->get_descriptor_set_layout();
        images_set = *global::get<systems::core_t>()->image_controller->get_descriptor_set();

        {
          pipeline_layout_maker plm(&device);
          p_layout = plm.addDescriptorLayout(uniform_layout)
                        .addDescriptorLayout(images_layout)
                        .addDescriptorLayout(map_layout)
                        .create("map_unit_rendering_pipeline_layout");
        }
        
        {
          const auto vertex   = create_shader_module(device, global::root_directory() + "shaders/battle_map_unit.vert.spv");
          const auto fragment = create_shader_module(device, global::root_directory() + "shaders/battle_map_unit.frag.spv");

          pipeline_maker pm(&device);
          pm.clearBlending();

          pipe = pm.addShader(vk::ShaderStageFlagBits::eVertex, vertex.get())
                   .addShader(vk::ShaderStageFlagBits::eFragment, fragment.get())
                   .vertexBinding(0, sizeof(uint32_t), vk::VertexInputRate::eInstance)
                     .vertexAttribute(0, 0, vk::Format::eR32Uint, 0)
                   .depthTest(VK_TRUE)
                   .depthWrite(VK_TRUE)
                   .frontFace(vk::FrontFace::eClockwise)
                   .cullMode(vk::CullModeFlagBits::eNone)
                   .assembly(vk::PrimitiveTopology::eTriangleStrip)
                   .viewport()
                   .scissor()
                   .dynamicState(vk::DynamicState::eViewport).dynamicState(vk::DynamicState::eScissor)
                   .addDefaultBlending()
                   .create("map_unit_rendering_pipeline", p_layout, info.renderpass->get_handle());
        }
      }
      
      units_render::~units_render() {
        device.destroy(p_layout);
        device.destroy(pipe);
      }
      
      void units_render::begin(resource_provider*) {}
      bool units_render::process(resource_provider* ctx, vk::CommandBuffer task) {
        auto battle = global::get<systems::battle_t>();
        if (!battle->is_init()) return false;
        
        auto uniform_set = ctx->get_descriptor_set(string_hash(UNIFORM_BUFFERS_DESCRIPTOR_SET_NAME));
        auto map_set = ctx->get_descriptor_set(string_hash(BATTLE_MAP_TILES_BUFFERS_DESCRIPTOR_SET_NAME));
        //auto map_set = *battle->map->get_descriptor_set();
        
        // алгоритм похож на ворлд мап биом рендер
        
        auto indirect_buffer = opt->get_indirect_buffer();
        auto units_indices = opt->get_units_indices();
        
        const auto bind = vk::PipelineBindPoint::eGraphics;
        task.bindPipeline(bind, pipe);
        // имеет смысл сделать один лайоут на несколько пайплайнов, это можно сделать через прокси стейдж (он только подключит несколько дескрипторов)
        task.bindDescriptorSets(bind, p_layout, 0, {uniform_set, images_set, map_set}, nullptr);
        task.bindVertexBuffers(0, units_indices, {0});
        task.drawIndirect(indirect_buffer, offsetof123(tile_optimizer::indirect_buffer_data, units_indirect), 1, sizeof(vk::DrawIndirectCommand));
        
        return true;
      }
      
      void units_render::clear() {}
    }
  }
}
