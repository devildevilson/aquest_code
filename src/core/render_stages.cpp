#include "render_stages.h"

#include "map.h"
#include "context.h"
#include "render/map_data.h"
#include "render/container.h"
#include "render/makers.h"
#include "render/defines.h"
#include "render/image_controller.h"
#include "render/pass.h"
#include "utils/constexpr_funcs.h"
#include "utils/globals.h"
#include "utils/systems.h"

namespace devils_engine {
//   template <typename T, typename M, M* m>
//   constexpr size_t offset_of() {
//     return 0;
//   }
//   
// #define offsetof2(s,m) offset_of<s, decltype(s::m), >()
  
  namespace render {
    // виндовс не дает использовать базовый offsetof, а базовый offsetof не constexpr !!!!!! ААААААААААААААААААААААААААААААААААААААААААААААААААА
#ifndef _WIN32
#define offsetof123(s,m) offsetof(s,m)
#else
#define offsetof123(s,m) ((::size_t)&reinterpret_cast<char const volatile&>((((s*)0)->m)))    
#endif
    
    tile_updater::tile_updater(const create_info &info) :
      need_copy(false),
      device(info.device),
      allocator(info.allocator),
      copies(new vk::BufferCopy[core::map::hex_count_d(core::map::detail_level)]),
      map(info.map)
    {
      const size_t tiles_count = core::map::hex_count_d(core::map::detail_level);
      updates.create(allocator, buffer(align_to(tiles_count * sizeof(update_data_t), 16), vk::BufferUsageFlagBits::eTransferSrc), vma::MemoryUsage::eCpuOnly);

#define offsetof1(s,m) ((::size_t)&reinterpret_cast<char const volatile&>((((s*)0)->m)))
      assert(sizeof(render::map_tile_t) - offsetof1(render::map_tile_t, texture) == sizeof(update_data_t));
      assert(offsetof1(render::map_tile_t, texture) - offsetof1(render::map_tile_t, texture) == offsetof1(update_data_t, texture));
      assert(offsetof1(render::map_tile_t, color) - offsetof1(render::map_tile_t, texture) == offsetof1(update_data_t, color));
      assert(offsetof1(render::map_tile_t, borders_data) - offsetof1(render::map_tile_t, texture) == offsetof1(update_data_t, borders_data));
      assert(offsetof1(render::map_tile_t, biome_index) - offsetof1(render::map_tile_t, texture) == offsetof1(update_data_t, biome_index));
#undef offsetof1

      for (size_t i = 0; i < tiles_count; ++i) {
        const size_t src_offset = i * sizeof(update_data_t);
        const size_t dst_offset = i * sizeof(map_tile_t) + offsetof123(render::map_tile_t, texture);
        const size_t size = sizeof(update_data_t);
        const vk::BufferCopy c{ src_offset, dst_offset, size };
        copies[i] = c;
      }
      
      ev = device.createEvent(vk::EventCreateInfo());
    }

    tile_updater::~tile_updater() {
      updates.destroy(allocator);
      delete [] copies;
      device.destroy(ev);
    }

    void tile_updater::begin(resource_provider*) {}
    bool tile_updater::process(resource_provider*, vk::CommandBuffer task) {
      const bool ret = need_copy.exchange(false); // присваивание потенциально может быть в другом потоке
      if (!ret) return false;

      const size_t tiles_count = core::map::hex_count_d(core::map::detail_level);
      //auto task = ctx->command_buffer();
      task.resetEvent(ev, vk::PipelineStageFlagBits::eTransfer);
      task.copyBuffer(updates.handle, map->data->tiles.handle, tiles_count, copies); // это поди будет жутко тяжелое копирование
      task.setEvent(ev, vk::PipelineStageFlagBits::eTransfer); // а может быть и оба eTransfer

      return true;
    }

    void tile_updater::clear() {}
    
    // обращение к эвенту может быть в другом потоке (хотя мы тут читаем только информацию, изменяет ее гпу)
    void wait_event(vk::Device device, vk::Event ev) {
      while (device.getEventStatus(ev) != vk::Result::eEventSet) { std::this_thread::sleep_for(std::chrono::microseconds(1)); }
    }

    // нужно ли тут мьютексами аккуратно закрыть? ну тип надо бы, зачем? возможно имеет смысл сделать need_copy атомарным, а все мьютексами закрывать смысла не имеет
    // нет, мьютекс нужен только в момент запуска задач в буфере и то до того как тайл апдейт пройдет, возможно тут просто эвентом можно закрыть
    // имеет смысл добавить сюда обновление высоты, но кажется места нехватает, может выкинуть текстуру?

    void tile_updater::update_texture(const uint32_t tile_index, const render::image_t texture) {
      wait_event(device, ev);
      
      const size_t tiles_count = core::map::hex_count_d(core::map::detail_level);
      assert(tile_index < tiles_count);
      auto array = reinterpret_cast<update_data_t*>(updates.ptr);
      array[tile_index].texture = texture;
      need_copy = true;
    }

    void tile_updater::update_color(const uint32_t tile_index, const render::color_t color) {
      wait_event(device, ev);
      
      const size_t tiles_count = core::map::hex_count_d(core::map::detail_level);
      assert(tile_index < tiles_count);
      auto array = reinterpret_cast<update_data_t*>(updates.ptr);
      array[tile_index].color = color;
      need_copy = true;
    }

    void tile_updater::update_borders_data(const uint32_t tile_index, const uint32_t borders_data) {
      wait_event(device, ev);
      
      const size_t tiles_count = core::map::hex_count_d(core::map::detail_level);
      assert(tile_index < tiles_count);
      auto array = reinterpret_cast<update_data_t*>(updates.ptr);
      array[tile_index].borders_data = borders_data;
      need_copy = true;
    }

    void tile_updater::update_biome_index(const uint32_t tile_index, const uint32_t biome_index) {
      wait_event(device, ev);
      
      const size_t tiles_count = core::map::hex_count_d(core::map::detail_level);
      assert(tile_index < tiles_count);
      auto array = reinterpret_cast<update_data_t*>(updates.ptr);
      array[tile_index].biome_index = biome_index;
      need_copy = true;
    }
    
    // здесь было бы неплохо залочить спомощью мьютекса
    void tile_updater::update_all(const core::context* ctx) {
      wait_event(device, ev);
      
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

    const size_t half_tiles_count = ceil(double(core::map::hex_count_d(core::map::detail_level)) / 2.0);
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

      //align_to(max_objects_indices_count*sizeof(uint32_t), 16)
      objects_indices.create(
        allocator,
        buffer(sizeof(glm::vec4), vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer),
        vma::MemoryUsage::eGpuOnly,
        "tile_optimizer::objects_indices"
      );

      structures_indices.create(
        allocator, buffer(structures_buffer_size, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eIndexBuffer), vma::MemoryUsage::eCpuOnly,
        "tile_optimizer::structures_indices"
      );

      assert(indirect.ptr != nullptr);
      auto buffer = reinterpret_cast<struct indirect_buffer*>(indirect.ptr);
      memset(indirect.ptr, 0, sizeof(struct indirect_buffer));
      buffer->padding_hex[0] = core::map::tri_count_d(core::map::accel_struct_detail_level);
      buffer->padding_hex[1] = max_indices_count;
      buffer->heraldies_command.vertexCount = 4;

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

      {
        pipeline_layout_maker plm(&device);
        pipe_layout = plm.addDescriptorLayout(info.uniform_layout)
                         .addDescriptorLayout(info.tiles_data_layout)
                         .addDescriptorLayout(layout)
                         .addDescriptorLayout(info.tiles_rendering_data_layout)
                         .create("tiles_optimizer_pipeline_layout");
      }

      {
        const auto mod = create_shader_module(device, global::root_directory() + "shaders/tiles.comp.spv");
        const auto mod1 = create_shader_module(device, global::root_directory() + "shaders/tile_data.comp.spv");
        const auto mod2 = create_shader_module(device, global::root_directory() + "shaders/count_biome_tiles.comp.spv");

        compute_pipeline_maker cpm(&device);
        pipe = cpm.shader(mod.get()).create("tiles_optimizer_pipeline", pipe_layout);
        tile_pipe = cpm.shader(mod1.get()).create("individual_tile_pipeline", pipe_layout);
        counter_pipe = cpm.shader(mod2.get()).create("count_biome_tiles", pipe_layout);
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
      device.destroy(tile_pipe);
    }

    void tile_optimizer::begin(resource_provider*) {
      auto buffer = reinterpret_cast<struct indirect_buffer*>(indirect.ptr);

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

      buffer->borders_command.vertexCount   = 4; // 0
      buffer->borders_command.instanceCount = 0; // 1
      buffer->borders_command.firstVertex   = 0;
      buffer->borders_command.firstInstance = 0;

      buffer->padding2[1] = borders_indices_count;
      buffer->padding2[2] = render_borders ? UINT32_MAX : 0;
      buffer->padding2[3] = UINT32_MAX;

      buffer->walls_command.vertexCount   = 4;
      buffer->walls_command.instanceCount = 0;
      buffer->walls_command.firstVertex   = 0;
      buffer->walls_command.firstInstance = 0;

      buffer->padding3[1] = connections_indices_count;
      buffer->padding3[3] = 1;

      buffer->structures_command.vertexCount   = 4;
      buffer->structures_command.instanceCount = 0;
      buffer->structures_command.firstVertex   = 0;
      buffer->structures_command.firstInstance = 0;

      buffer->padding4[1] = structures_indices_count;
      buffer->padding4[3] = 0;

      buffer->heraldies_command.vertexCount   = 4;
      buffer->heraldies_command.instanceCount = 0;
      buffer->heraldies_command.firstVertex   = 0;
      buffer->heraldies_command.firstInstance = 0;

      buffer->padding5[1] = heraldies_indices_count;

      buffer->dispatch_indirect_command = glm::uvec4(0, 1, 1, 0);

      memcpy(&buffer->frustum, &fru, sizeof(buffer->frustum));

//       for (uint32_t i = 0; i < MAX_BIOMES_COUNT; ++i) {
//         buffer->biome_data[i].objects_indirect[0] = 0; // индексы
//         buffer->biome_data[i].objects_indirect[1] = 0; // инстансы
//         buffer->biome_data[i].objects_indirect[2] = 0; // ферст индекс
//         buffer->biome_data[i].objects_indirect[3] = 0; // вертекс оффсет
//         buffer->biome_data[i].objects_data[0]     = 0; // ферст инстанс
//         buffer->biome_data[i].objects_data[3]     = 0; // нужно ли?
//       }
      
      buffer->biome_data[0].objects_indirect[0] = 4; // вершины
      buffer->biome_data[0].objects_indirect[1] = 0; // инстансы
      buffer->biome_data[0].objects_indirect[2] = 0; // ферст индекс
      buffer->biome_data[0].objects_indirect[3] = 0; // ферст инстанс
      for (uint32_t i = 0; i < MAX_BIOMES_COUNT; ++i) {
        buffer->biome_data[i].objects_data[0] = 0; // количество
      }
    }

    bool tile_optimizer::process(resource_provider* ctx, vk::CommandBuffer task) {
      // пока что не могу придумать кому дать интерфейс resource_provider, можно наверное просто выдать его контейнеру, но там появится куча ненужного мусора
      auto uniform_set = ctx->get_descriptor_set(string_hash(UNIFORM_BUFFERS_DESCRIPTOR_SET_NAME));
      auto tiles = ctx->get_descriptor_set(string_hash(MAP_TILES_BUFFERS_DESCRIPTOR_SET_NAME));
      auto tiles_rendering_data = ctx->get_descriptor_set(string_hash(TILE_RENDERING_DATA_DESCRIPTOR_SET_NAME));

      task.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipe_layout, 0, {
        uniform_set,
        tiles,
        buffers_set,
        tiles_rendering_data
      }, nullptr);

      task.bindPipeline(vk::PipelineBindPoint::eCompute, pipe);
      const uint32_t count = ceil(double(core::map::tri_count_d(core::map::accel_struct_detail_level)) / double(work_group_size));
      task.dispatch(count, 1, 1);
      // тут поди нужно подождать
      const vk::MemoryBarrier2 memory_barrier(
        vk::PipelineStageFlagBits2KHR::eComputeShader,
        vk::AccessFlagBits2KHR::eShaderWrite,
        vk::PipelineStageFlagBits2KHR::eComputeShader,
        vk::AccessFlagBits2KHR::eShaderRead 
      );

      const vk::DependencyInfo dependency_info(vk::DependencyFlagBits::eByRegion, memory_barrier);
      
      task.pipelineBarrier2(dependency_info);
      task.bindPipeline(vk::PipelineBindPoint::eCompute, counter_pipe);
      task.dispatchIndirect(indirect.handle, offsetof123(struct tile_optimizer::indirect_buffer, dispatch_indirect_command));
      
      // пайп для тайлов, в котором мы во первых можем проверить выделение, а во вторых собрать данные биомов и границ
      task.pipelineBarrier2(dependency_info);
      task.bindPipeline(vk::PipelineBindPoint::eCompute, tile_pipe);
      task.dispatchIndirect(indirect.handle, offsetof123(struct tile_optimizer::indirect_buffer, dispatch_indirect_command));

      return true;
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
//       auto buffer = reinterpret_cast<struct indirect_buffer*>(indirect.ptr);
//       for (uint32_t i = 0; i < MAX_BIOMES_COUNT; ++i) {
//         buffer->biome_data[i].objects_data[1] = data[i].first;  // offset
//         buffer->biome_data[i].objects_data[2] = data[i].second; // size
//         buffer->biome_data[i].objects_data[3] = 0;
//       }
      size_t size = 0;
      for (uint32_t i = 0; i < MAX_BIOMES_COUNT; ++i) {
        size += data[i].second * sizeof(uint32_t);
      }
      objects_indices.destroy(allocator);
      objects_indices.create(
        allocator,
        buffer(align_to(size, 16), vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer),
        vma::MemoryUsage::eGpuOnly,
        "tile_optimizer::objects_indices"
      );
    }

    void tile_optimizer::set_border_rendering(const bool value) {
      render_borders = value;
    }

    bool tile_optimizer::is_rendering_border() const {
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

    void tile_objects_optimizer::begin(resource_provider*) {}
    bool tile_objects_optimizer::process(resource_provider* ctx, vk::CommandBuffer task) {
      auto uniform_set = ctx->get_descriptor_set(string_hash(UNIFORM_BUFFERS_DESCRIPTOR_SET_NAME));
      auto tiles = ctx->get_descriptor_set(string_hash(MAP_TILES_BUFFERS_DESCRIPTOR_SET_NAME));
      auto set = opt->get_buffers_set();
      auto indirect = opt->indirect_buffer();

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

      task.updateBuffer(indirect, update_data_start, sizeof(update_buffer_data), &udata);

      const vk::MemoryBarrier mem_b(vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead);
      task.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eComputeShader, vk::DependencyFlagBits::eByRegion,
        mem_b, nullptr, nullptr
      );

      task.bindPipeline(vk::PipelineBindPoint::eCompute, pipe);
      task.bindDescriptorSets(vk::PipelineBindPoint::eCompute, p_layout, 0, { uniform_set, tiles, set }, nullptr);
      task.dispatchIndirect(indirect, offsetof123(struct tile_optimizer::indirect_buffer, dispatch_indirect_command));

      return true;
    }

    void tile_objects_optimizer::clear() {}

    void barriers::begin(resource_provider*) {}
    bool barriers::process(resource_provider* ctx, vk::CommandBuffer task) {
      (void)ctx;
      (void)task;
//       auto task = ctx->compute();
//       task->setBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
//                        VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_INDIRECT_COMMAND_READ_BIT);
//
//       task->setBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
//                        VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);

//       task->setBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
//                        VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
      return false;
    }

    void barriers::clear() {}

    const uint16_t pen_index_array[] = {
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, UINT16_MAX, 10, 11, 12, 13, 14
    };

    const uint16_t hex_index_array[] = {
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1, UINT16_MAX, 12, 13, 14, 15, 16, 17
    };

#define DEFAULT_COLOR_WRITE_MASK vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    
#define TILE_RENDER_PIPELINE_LAYOUT_NAME "tile_render_pipeline_layout"
#define TILE_RENDER_PIPELINE_NAME "tile_render_pipeline"

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
                 .create(TILE_RENDER_PIPELINE_NAME, p_layout, info.pass->get_handle(), info.subpass_index); // info.cont->vlk_window->render_pass
      }
    }

    tile_render::~tile_render() {
      points_indices.destroy(allocator);
      device.destroy(p_layout);
      device.destroy(pipe);
    }

    void tile_render::begin(resource_provider*) {}
    bool tile_render::process(resource_provider* ctx, vk::CommandBuffer task) {
      auto uniform_set = ctx->get_descriptor_set(string_hash(UNIFORM_BUFFERS_DESCRIPTOR_SET_NAME));
      auto tiles = ctx->get_descriptor_set(string_hash(MAP_TILES_BUFFERS_DESCRIPTOR_SET_NAME));

      auto indirect_buffer = opt->indirect_buffer();
      auto indices_buffer = opt->tiles_index_buffer();

      const auto bind_point = vk::PipelineBindPoint::eGraphics;
      task.bindPipeline(bind_point, pipe);
      task.bindDescriptorSets(bind_point, p_layout, 0, {uniform_set, images_set, tiles}, nullptr);

      task.bindVertexBuffers(0, indices_buffer, {0});
      task.bindIndexBuffer(points_indices.handle, 0, vk::IndexType::eUint16);
      task.drawIndexedIndirect(indirect_buffer, offsetof123(struct render::tile_optimizer::indirect_buffer, pen_tiles_command), 1, sizeof(vk::DrawIndexedIndirectCommand)); // пентагоны отдельно посчитаем

      task.bindVertexBuffers(0, indices_buffer, sizeof(uint32_t)*12);
      task.bindIndexBuffer(points_indices.handle, sizeof(pen_index_array), vk::IndexType::eUint16);
      task.drawIndexedIndirect(indirect_buffer, offsetof123(struct render::tile_optimizer::indirect_buffer, hex_tiles_command), 1, sizeof(vk::DrawIndexedIndirectCommand));

      return true;
    }

    void tile_render::clear() {}

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
      auto border_layout = info.map_buffers->borders_data_layout;

      {
        pipeline_layout_maker plm(&device);
        p_layout = plm.addDescriptorLayout(uniform_layout)
                      .addDescriptorLayout(border_layout)
                      .addDescriptorLayout(tiles_data_layout)
                      .create("tile_borders_render_layout");
      }

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
                 .create(name, p_layout, info.pass->get_handle(), info.subpass_index);
      }
    }

    tile_border_render::~tile_border_render() {
      device.destroy(p_layout);
      device.destroy(pipe);
    }

    void tile_border_render::begin(resource_provider*) {}
    bool tile_border_render::process(resource_provider* ctx, vk::CommandBuffer task) {
      auto indirect_buffer = opt->indirect_buffer();
      auto indices_buffer = opt->borders_index_buffer();
      if (indices_buffer == vk::Buffer(nullptr)) return false;

      auto uniform_set = ctx->get_descriptor_set(string_hash(UNIFORM_BUFFERS_DESCRIPTOR_SET_NAME));
      auto tiles = ctx->get_descriptor_set(string_hash(MAP_TILES_BUFFERS_DESCRIPTOR_SET_NAME));
      auto borders = ctx->get_descriptor_set(string_hash(BORDERS_BUFFERS_DESCRIPTOR_SET_NAME));

      const auto bind_point = vk::PipelineBindPoint::eGraphics;
      task.bindPipeline(bind_point, pipe);
      task.bindDescriptorSets(bind_point, p_layout, 0, {uniform_set, borders, tiles}, nullptr);
      task.bindVertexBuffers(0, indices_buffer, {0});
      task.drawIndirect(indirect_buffer, offsetof123(struct tile_optimizer::indirect_buffer, borders_command), 1, sizeof(vk::DrawIndirectCommand));

      return true;
    }

    void tile_border_render::clear() {}

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
                 .vertexBinding(0, sizeof(uint32_t), vk::VertexInputRate::eInstance)
                   .vertexAttribute(0, 0, vk::Format::eR32Uint, 0)
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
                 .create("object_rendering_pipeline", p_layout, info.pass->get_handle(), info.subpass_index);
      }
    }

    tile_object_render::~tile_object_render() {
      device.destroy(p_layout);
      device.destroy(pipe);
    }

    void tile_object_render::begin(resource_provider*) {}
    bool tile_object_render::process(resource_provider* ctx, vk::CommandBuffer task) {
      auto indirect_buffer = opt->indirect_buffer();
      auto indices_buffer = opt->objects_index_buffer();
      if (indices_buffer == vk::Buffer(nullptr)) return false;

      auto uniform_set = ctx->get_descriptor_set(string_hash(UNIFORM_BUFFERS_DESCRIPTOR_SET_NAME));
      auto tiles = ctx->get_descriptor_set(string_hash(MAP_TILES_BUFFERS_DESCRIPTOR_SET_NAME));

      const auto bind_point = vk::PipelineBindPoint::eGraphics;
      task.bindPipeline(bind_point, pipe);
      task.bindDescriptorSets(bind_point, p_layout, 0, {uniform_set, images_set, tiles}, nullptr);
      //task.bindIndexBuffer(indices_buffer, 0, vk::IndexType::eUint32);
//       if (multi_draw_indirect) {
//         task.drawIndexedIndirect(indirect_buffer, offsetof123(struct tile_optimizer::indirect_buffer, biome_data), MAX_BIOMES_COUNT, sizeof(biome_objects_data_t));
//       } else {
//         for (uint32_t i = 0; i < MAX_BIOMES_COUNT; ++i) {
//           task.drawIndexedIndirect(indirect_buffer, offsetof123(struct tile_optimizer::indirect_buffer, biome_data)+sizeof(biome_objects_data_t)*i, 1, sizeof(vk::DrawIndexedIndirectCommand));
//         }
//       }
      task.bindVertexBuffers(0, indices_buffer, {0});
      task.drawIndexedIndirect(indirect_buffer, offsetof123(struct tile_optimizer::indirect_buffer, biome_data), 1, sizeof(vk::DrawIndirectCommand));

      return true;
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
                 .create("tiling_highlights_rendering_pipeline", p_layout, info.pass->get_handle(), info.subpass_index);
      }

      const size_t size = align_to(max_indices_count * 2, 16);
      tiles_indices.create(allocator, buffer(size, vk::BufferUsageFlagBits::eVertexBuffer), vma::MemoryUsage::eCpuOnly, "tile_highlights_render::tiles_indices");
    }

    tile_highlights_render::~tile_highlights_render() {
      device.destroy(p_layout);
      device.destroy(pipe);
      tiles_indices.destroy(allocator);
    }

    void tile_highlights_render::begin(resource_provider*) {}
    bool tile_highlights_render::process(resource_provider* ctx, vk::CommandBuffer task) {
      if (hex_tiles_count + pen_tiles_count == 0) return false;
      
      auto uniform_set = ctx->get_descriptor_set(string_hash(UNIFORM_BUFFERS_DESCRIPTOR_SET_NAME));
      auto tiles = ctx->get_descriptor_set(string_hash(MAP_TILES_BUFFERS_DESCRIPTOR_SET_NAME));

      const auto bind_point = vk::PipelineBindPoint::eGraphics;
      task.bindPipeline(bind_point, pipe);
      task.bindDescriptorSets(bind_point, p_layout, 0, {uniform_set, tiles}, nullptr);
      task.bindVertexBuffers(0, tiles_indices.handle, {0});
      if (pen_tiles_count != 0) task.draw(5, pen_tiles_count, 0, 0);
      if (hex_tiles_count != 0) task.draw(6, hex_tiles_count, 0, 12);

      return true;
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
                 .create("tile_structure_rendering_pipeline", p_layout, info.pass->get_handle(), info.subpass_index);
      }
    }

    tile_structure_render::~tile_structure_render() {
      device.destroy(p_layout);
      device.destroy(pipe);
      structures_instance.destroy(allocator);
      gpu_structures_instance.destroy(allocator);
    }

    void tile_structure_render::begin(resource_provider*) {
      local_structures_count = structures_count;
      structures_count = 0;
    }

    bool tile_structure_render::process(resource_provider* ctx, vk::CommandBuffer task) {
      if (local_structures_count == 0) return false;
      
      auto uniform_set = ctx->get_descriptor_set(string_hash(UNIFORM_BUFFERS_DESCRIPTOR_SET_NAME));
      auto tiles = ctx->get_descriptor_set(string_hash(MAP_TILES_BUFFERS_DESCRIPTOR_SET_NAME));

      const auto bind_point = vk::PipelineBindPoint::eGraphics;
      task.bindPipeline(bind_point, pipe);
      task.bindDescriptorSets(bind_point, p_layout, 0, {uniform_set, images_set, tiles}, nullptr);

      task.bindVertexBuffers(0, gpu_structures_instance.handle, {0});
      task.draw(4, local_structures_count, 0, 0);

      return true;
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

    void tile_structure_render::copy(resource_provider*, vk::CommandBuffer task) const {
      const size_t instance_buffer_size = align_to(current_inst_size * sizeof(structure_data), 16);
      const vk::BufferCopy c1{ 0, 0, instance_buffer_size };
      task.copyBuffer(structures_instance.handle, gpu_structures_instance.handle, 1, &c1);
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
                 .create("heraldies_rendering_pipeline", p_layout, info.pass->get_handle(), info.subpass_index);
      }
    }

    heraldies_render::~heraldies_render() {
      device.destroy(p_layout);
      device.destroy(pipe);
      layers_chain.destroy(allocator);
      heraldy_instance.destroy(allocator);
      gpu_heraldy_instance.destroy(allocator);

      // нужно ли удалить render::buffers::heraldy_indices?
    }

    void heraldies_render::begin(resource_provider*) {
      local_inst_count = inst_count;
      local_chain_count = chain_count;
      inst_count = 0;
      chain_count = 0;
    }

    bool heraldies_render::process(resource_provider* ctx, vk::CommandBuffer task) {
      if (local_inst_count == 0) return false;

      auto uniform_set = ctx->get_descriptor_set(string_hash(UNIFORM_BUFFERS_DESCRIPTOR_SET_NAME));
      auto tiles = ctx->get_descriptor_set(string_hash(MAP_TILES_BUFFERS_DESCRIPTOR_SET_NAME));
      auto heraldy_set = ctx->get_descriptor_set(string_hash(HERALDY_BUFFERS_DESCRIPTOR_SET_NAME));

      ASSERT(heraldy_set);

      // до рендеринга геральдики имеет смысл почистить буфер глубины
      // для того чтобы геральдика не входила внутрь земли, как это скажется на интерфейсе? фиг знает
      // я же помоему решил так не делать

#define UNUSED_COLOR_ATTACHMENT_INDEX 0
      const vk::ClearAttachment att{vk::ImageAspectFlagBits::eDepth, UNUSED_COLOR_ATTACHMENT_INDEX, vk::ClearValue({1.0f, 0})};
      const vk::ClearRect rect{ctx->get_render_area(0), 0, 1};

      const auto bind_point = vk::PipelineBindPoint::eGraphics;
      task.clearAttachments({att}, {rect});
      task.bindPipeline(bind_point, pipe);

      task.bindDescriptorSets(bind_point, p_layout, 0, {uniform_set, images_set, tiles, heraldy_set}, nullptr);
      task.bindVertexBuffers(0, gpu_heraldy_instance.handle, {0});
      task.draw(4, local_inst_count+1, 0, 1); // наверное начнем со второго инстанса, в первом будет храниться heraldy_highlight_data

      return true;
    }

    void heraldies_render::clear() {
      auto layers_buffer = &global::get<render::buffers>()->heraldy_indices;

      if (local_chain_count > current_buffer_size) {
        layers_chain.destroy(allocator);
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

    void heraldies_render::copy(resource_provider*, vk::CommandBuffer task) const {
      const size_t layers_buffer_size = align_to(current_buffer_size * sizeof(uint32_t), 16);
      const size_t heraldy_instance_buffer_size = align_to(current_inst_size * sizeof(heraldy_instance_data_t), 16);

      auto layers_buffer = &global::get<render::buffers>()->heraldy_indices;
      const vk::BufferCopy c1{ 0, 0, layers_buffer_size };
      const vk::BufferCopy c2{ 0, 0, heraldy_instance_buffer_size };
      task.copyBuffer(layers_chain.handle, layers_buffer->handle, 1, &c1);
      task.copyBuffer(heraldy_instance.handle, gpu_heraldy_instance.handle, 1, &c2);
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
                 .create("armies_rendering_pipeline", p_layout, info.pass->get_handle(), info.subpass_index);
      }
    }

    armies_render::~armies_render() {
      device.destroy(p_layout);
      device.destroy(pipe);
      instance.destroy(allocator);
      gpu_instance.destroy(allocator);
    }

    void armies_render::begin(resource_provider*) {
      local_armies_count = armies_count;
      armies_count = 0;
    }

    bool armies_render::process(resource_provider* ctx, vk::CommandBuffer task) {
      if (local_armies_count == 0) return false;
      
      auto uniform_set = ctx->get_descriptor_set(string_hash(UNIFORM_BUFFERS_DESCRIPTOR_SET_NAME));
      auto tiles = ctx->get_descriptor_set(string_hash(MAP_TILES_BUFFERS_DESCRIPTOR_SET_NAME));

      const auto bind_point = vk::PipelineBindPoint::eGraphics;
      task.bindPipeline(bind_point, pipe);
      task.bindDescriptorSets(bind_point, p_layout, 0, {uniform_set, images_set, tiles}, nullptr); // heraldy->descriptorSet()->handle()

      task.bindVertexBuffers(0, gpu_instance.handle, {0});
      task.draw(4, local_armies_count, 0, 0);

      return true;
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
    void armies_render::copy(resource_provider*, vk::CommandBuffer task) const {
      const size_t current_instance_buffer_size = align_to(current_inst_size * sizeof(army_data), 16);
      const vk::BufferCopy c1{ 0, 0, current_instance_buffer_size };
      task.copyBuffer(instance.handle, gpu_instance.handle, 1, &c1);
    }

    void armies_render::add(const army_data &data) {
      const uint32_t index = armies_count.fetch_add(1);
      if (index < current_inst_size) {
        auto arr = reinterpret_cast<army_data*>(instance.ptr);
        memcpy(&arr[index], &data, sizeof(data));
      }
    }
  }
}
