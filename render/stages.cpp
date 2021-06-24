#include "stages.h"

#include "window.h"
#include "context.h"
#include "utils/globals.h"
#include "utils/utility.h"
#include "targets.h"
#include "bin/interface_context.h"
#include "utils/input.h"
#include "utils/systems.h"
#include "image_controller.h"
#include "utils/frustum.h"
#include "image_container.h"
#include <array>

#define TILE_RENDER_PIPELINE_LAYOUT_NAME "tile_render_pipeline_layout"
#define TILE_RENDER_PIPELINE_NAME "tile_render_pipeline"

namespace devils_engine {
  namespace render {
    window_next_frame::window_next_frame(const create_info &info) : w(info.w) {}
    void window_next_frame::begin() {}
    void window_next_frame::proccess(context* ctx) { w->next_frame(); (void)ctx; }
    void window_next_frame::clear() {}
    void task_begin::begin() {}
    void task_begin::proccess(context* ctx) { ctx->interface()->begin(); }
    void task_begin::clear() {}
    void task_end::begin() {}
    void task_end::proccess(context* ctx) { ctx->interface()->end(); }
    void task_end::clear() {}
    render_pass_begin::render_pass_begin(const uint32_t &index) : index(index) {}
    void render_pass_begin::begin() {}
    void render_pass_begin::proccess(context* ctx) {
      auto w = global::get<window>();
      ctx->graphics()->setRenderTarget(w);
      if (index == 0) {
        ctx->graphics()->beginRenderPass(w->render_pass);
      } else {
        ctx->graphics()->beginRenderPass(w->render_pass_objects);
      }
    }

    void render_pass_begin::clear() {}

    void render_pass_end::begin() {}
    void render_pass_end::proccess(context* ctx) {
      auto task = ctx->graphics();
      task->endRenderPass();
//       auto w = global::get<window>();
//       
//       static const VkImageSubresourceRange range{
//         VK_IMAGE_ASPECT_COLOR_BIT,
//         0, 1, 0, 1
//       };
//       
//       task->setBarrier(w->image(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, range, VK_QUEUE_FAMILY_IGNORED, w->present_family);
      
//       const uint32_t prev_image = w->swapchain.image_index == 0 ? 2 : w->swapchain.image_index-1;
//       const uint32_t next_image = w->swapchain.image_index == 2 ? 0 : w->swapchain.image_index+1;
      
//       task->setBarrier(w->swapchain.images[w->swapchain.image_index]->handle(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
//       task->setBarrier(w->swapchain.images[prev_image]->handle(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
//       task->setBarrier(w->swapchain.images[next_image]->handle(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
//       task->copy(w->swapchain.images[w->swapchain.image_index], w->swapchain.images[prev_image]);
//       task->copy(w->swapchain.images[w->swapchain.image_index], w->swapchain.images[next_image]);
//       task->setBarrier(w->swapchain.images[w->swapchain.image_index]->handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
//       task->setBarrier(w->swapchain.images[prev_image]->handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
//       task->setBarrier(w->swapchain.images[next_image]->handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
    }

    void render_pass_end::clear() {}
    
#define WORLD_MAP_RENDER_DESCRIPTOR_POOL "world_map_render_descriptor_pool"
    
    const size_t half_tiles_count = (core::map::hex_count_d(core::map::detail_level) / 2 + 1);
    const size_t max_indices_count = half_tiles_count * 7; // тут теперь максимальное количество ИНДЕКСОВ
    const size_t max_objects_indices_count = core::map::hex_count_d(core::map::detail_level) * 5;
    //const size_t max_objects_indices_count = half_tiles_count * 5;
    //static_assert(max_tiles_count < 500000);
    tile_optimizer::tile_optimizer(const create_info &info) : 
      device(info.device),
      indirect(nullptr),
      tiles_indices(nullptr),
      borders_indices(nullptr),
      walls_indices(nullptr),
      objects_indices(nullptr),
      structures_indices(nullptr),
      heraldy_indices(nullptr),
      set(nullptr),
      borders_indices_count(0),
      connections_indices_count(0),
      structures_indices_count(0),
      heraldies_indices_count(0),
      render_borders(false)
    {
      indirect = device->create(yavf::BufferCreateInfo::buffer(sizeof(struct indirect_buffer), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
      tiles_indices = device->create(
        yavf::BufferCreateInfo::buffer(
          //(sizeof(instance_data_t)*max_tiles_count)/4+1, 
          //(max_indices_count*4+16-1)/16*16,
          align_to(half_tiles_count*sizeof(uint32_t), 16),
          VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT), 
        VMA_MEMORY_USAGE_GPU_ONLY // возможно нужно сделать буффер в памяти хоста
      );
      
      borders_indices = device->create(
        yavf::BufferCreateInfo::buffer(
          //(sizeof(instance_data_t)*max_tiles_count)/4+1, 
          std::min(align_to(max_indices_count*sizeof(uint32_t), 16), align_to(max_objects_indices_count*sizeof(uint32_t), 16)),
          VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT), 
        VMA_MEMORY_USAGE_GPU_ONLY
      );
      
      walls_indices = device->create(
        yavf::BufferCreateInfo::buffer(
          //(sizeof(instance_data_t)*max_tiles_count)/4+1, 
          std::min(align_to(max_indices_count*sizeof(uint32_t), 16), align_to(max_objects_indices_count*sizeof(uint32_t), 16)),
          VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT),
        VMA_MEMORY_USAGE_GPU_ONLY
      );
      
      objects_indices = device->create(
        yavf::BufferCreateInfo::buffer(
          //(sizeof(instance_data_t)*max_tiles_count)/4+1, 
          align_to(max_objects_indices_count*sizeof(uint32_t), 16),
          VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT), 
        VMA_MEMORY_USAGE_GPU_ONLY
      );
      
      structures_indices = device->create(
        yavf::BufferCreateInfo::buffer(
          16,
          VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT), 
        VMA_MEMORY_USAGE_CPU_ONLY
      );
      
//       heraldy_indices = device->create(
//         yavf::BufferCreateInfo::buffer(
//           16,
//           VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT), 
//         VMA_MEMORY_USAGE_GPU_ONLY
//       );
      
      auto buffer = reinterpret_cast<struct indirect_buffer*>(indirect->ptr());
      memset(indirect->ptr(), 0, sizeof(struct indirect_buffer));
      buffer->padding_hex[0] = core::map::tri_count_d(core::map::accel_struct_detail_level);
      buffer->padding_hex[1] = max_indices_count;
      buffer->heraldies_command.vertexCount = 4;
      
//       PRINT_VAR("triangle_count", buffer->padding1[0])
//       PRINT_VAR("max_indices_count", buffer->padding1[1])
      
      //auto pool = device->descriptorPool(DEFAULT_DESCRIPTOR_POOL_NAME);
//       auto storage_layout = device->setLayout(STORAGE_BUFFER_LAYOUT_NAME);
      auto tiles_data_layout = device->setLayout(TILES_DATA_LAYOUT_NAME);
      auto uniform_layout = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
      
      ASSERT(tiles_data_layout != VK_NULL_HANDLE);
      
      yavf::DescriptorPool pool = device->descriptorPool(WORLD_MAP_RENDER_DESCRIPTOR_POOL);
      if (pool == VK_NULL_HANDLE) {
        yavf::DescriptorPoolMaker dpm(device);
        pool = dpm.poolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 30).create(WORLD_MAP_RENDER_DESCRIPTOR_POOL);
      }
      
      yavf::DescriptorSetLayout tiles_indices_layout = VK_NULL_HANDLE;
      {
        yavf::DescriptorLayoutMaker dlm(device);
        tiles_indices_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
                                  .binding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
                                  .binding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
                                  .binding(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
                                  .binding(4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
                                  .binding(5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
                                  .binding(6, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
                                  .create("tiles_indices_layout");
      }
      
      {
        yavf::DescriptorMaker dm(device);
        set = dm.layout(tiles_indices_layout).create(pool)[0];
        set->add({indirect,           0, indirect->info().size,           0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
        set->add({tiles_indices,      0, tiles_indices->info().size,      0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
        set->add({borders_indices,    0, borders_indices->info().size,    0, 2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
        set->add({walls_indices,      0, walls_indices->info().size,      0, 3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
        set->add({objects_indices,    0, objects_indices->info().size,    0, 4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
        set->add({structures_indices, 0, structures_indices->info().size, 0, 5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
//         set->add({heraldy_indices,    0, heraldy_indices->info().size,    0, 6, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
        set->update();
//         indirect->setDescriptor(desc, index);
      }
      
//       {
//         yavf::DescriptorMaker dm(device);
//         auto desc = dm.layout(storage_layout).create(pool)[0];
//         size_t index = desc->add({tiles_indices, 0, tiles_indices->info().size, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
//         tiles_indices->setDescriptor(desc, index);
//       }
      
      yavf::PipelineLayout p_layout = VK_NULL_HANDLE;
      {
        yavf::PipelineLayoutMaker plm(device);
        p_layout = plm.addDescriptorLayout(uniform_layout)
                      .addDescriptorLayout(tiles_data_layout)
                      .addDescriptorLayout(tiles_indices_layout)
                      .create("tiles_optimizer_pipeline_layout");
      }
      
      {
        yavf::raii::ShaderModule compute(device, global::root_directory() + "shaders/tiles.comp.spv");
        
        yavf::ComputePipelineMaker cpm(device);
        pipe = cpm.shader(compute).create("tiles_optimizer_pipeline", p_layout);
      }
    }
    
    tile_optimizer::~tile_optimizer() {
      device->destroy(indirect);
      device->destroy(tiles_indices);
      device->destroy(borders_indices);
      device->destroy(walls_indices);
      device->destroy(objects_indices);
      device->destroy(structures_indices);
      device->destroyDescriptorPool(WORLD_MAP_RENDER_DESCRIPTOR_POOL);
      device->destroySetLayout("tiles_indices_layout");
      device->destroyLayout("tiles_optimizer_pipeline_layout");
      device->destroy(pipe);
    }
    
    void tile_optimizer::begin() {
      auto buffer = reinterpret_cast<struct indirect_buffer*>(indirect->ptr());
      
//       PRINT_VAR("tiles   indices  ", buffer->tiles_command.indexCount)
//       PRINT_VAR("borders indices  ", buffer->borders_command.indexCount)
//       PRINT_VAR("walls   indices  ", buffer->walls_command.indexCount)
      
      auto uniform = global::get<render::buffers>()->uniform;
      auto camera_data = reinterpret_cast<render::camera_data*>(uniform->ptr());
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
      
      memcpy(&buffer->frustum, &fru, sizeof(utils::frustum));
      
      for (uint32_t i = 0; i < MAX_BIOMES_COUNT; ++i) {
        buffer->biome_data[i].objects_indirect[0] = 0; // индексы
        buffer->biome_data[i].objects_indirect[1] = 0; // инстансы
        buffer->biome_data[i].objects_indirect[2] = 0; // ферст индекс
        buffer->biome_data[i].objects_indirect[3] = 0; // вертекс оффсет
        buffer->biome_data[i].objects_data[0]     = 0; // ферст инстанс
        buffer->biome_data[i].objects_data[3]     = 0; // нужно ли?
      }
    }
    
    void tile_optimizer::proccess(context* ctx) {
      auto map_systems = global::get<systems::map_t>();
      if (!map_systems->is_init()) return;
      auto map = map_systems->map;
      //if (map->status() != core::map::status::valid) return;
//       if (map->status() == core::map::status::initial) return;
      
      auto uniform = global::get<render::buffers>()->uniform;
      auto tiles = map->tiles;
//       auto points = global::get<render::buffers>()->points;
      
      auto task = ctx->compute();
      task->setPipeline(pipe);
      task->setDescriptor({
        uniform->descriptorSet()->handle(), 
        tiles->descriptorSet()->handle(), 
//         points->descriptorSet()->handle(), 
        set->handle()
//         tiles_indices->descriptorSet()->handle()
      }, 0);
      
      const uint32_t count = std::ceil(float(core::map::tri_count_d(core::map::accel_struct_detail_level)) / float(work_group_size));
      task->dispatch(count, 1, 1);
      
//       task->setBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
//                        VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_INDIRECT_COMMAND_READ_BIT);
//       
//       task->setBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
//                        VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
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
      auto buffer = reinterpret_cast<struct indirect_buffer*>(indirect->ptr());
      buffer->selection_frustum = default_fru;
      buffer->selection_box = {glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)};
    }
    
    yavf::Buffer* tile_optimizer::indirect_buffer() const {
      return indirect;
    }
    
    yavf::Buffer* tile_optimizer::tiles_index_buffer() const {
      return tiles_indices;
    }
    
    yavf::Buffer* tile_optimizer::borders_index_buffer() const {
      return borders_indices;
    }
    
    yavf::Buffer* tile_optimizer::walls_index_buffer() const {
      return walls_indices;
    }
    
    yavf::Buffer* tile_optimizer::objects_index_buffer() const {
      return objects_indices;
    }
    
    yavf::Buffer* tile_optimizer::structures_index_buffer() const {
      return structures_indices;
    }
    
    yavf::Buffer* tile_optimizer::heraldy_index_buffer() const {
      return heraldy_indices;
    }
    
    yavf::DescriptorSet* tile_optimizer::buffers_set() const {
      return set;
    }
    
    void tile_optimizer::set_borders_count(const uint32_t &count) {
      const uint32_t indices_count = count;
      const uint32_t final_size = align_to(indices_count*sizeof(uint32_t), 16);
      auto buffer = reinterpret_cast<struct indirect_buffer*>(indirect->ptr());
      borders_indices_count = indices_count;
      buffer->padding2[1] = indices_count;
      if (borders_indices->info().size >= final_size) return;
      borders_indices->resize(final_size);
      
      set->at(2) = {borders_indices, 0, borders_indices->info().size, 0, 2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
      set->update(2);
      
      PRINT_VAR("borders indices_count", indices_count)
    }
    
    void tile_optimizer::set_connections_count(const uint32_t &count) {
      const uint32_t indices_count = count*5;
      const uint32_t final_size = align_to(indices_count*sizeof(uint32_t), 16);
      auto buffer = reinterpret_cast<struct indirect_buffer*>(indirect->ptr());
      connections_indices_count = indices_count;
      buffer->padding3[1] = indices_count;
      if (walls_indices->info().size >= final_size) return;
      walls_indices->resize(final_size);
      
      set->at(3) = {walls_indices, 0, walls_indices->info().size, 0, 3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
      set->update(3);
      PRINT_VAR("walls indices_count", indices_count)
    }
    
    void tile_optimizer::set_max_structures_count(const uint32_t &count) {
      const uint32_t indices_count = count*5;
      const uint32_t final_size = align_to(indices_count*sizeof(uint32_t), 16);
      auto buffer = reinterpret_cast<struct indirect_buffer*>(indirect->ptr());
      structures_indices_count = indices_count;
      buffer->padding4[1] = indices_count;
      if (structures_indices->info().size >= final_size) return;
      structures_indices->resize(final_size);
      
      set->at(5) = {structures_indices, 0, structures_indices->info().size, 0, 5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
      set->update(5);
    }
    
    void tile_optimizer::set_max_heraldy_count(const uint32_t &count) {
      if (heraldy_indices == nullptr) return; // пока так оставим, индексы геральдики попадают теперь в большой буффер объектов
      const uint32_t indices_count = count*5;
      const uint32_t final_size = align_to(indices_count*sizeof(uint32_t), 16);
      auto buffer = reinterpret_cast<struct indirect_buffer*>(indirect->ptr());
      heraldies_indices_count = indices_count;
      buffer->padding5[1] = indices_count;
      if (heraldy_indices->info().size >= final_size) return;
      heraldy_indices->resize(final_size);
      
      set->at(6) = {heraldy_indices, 0, heraldy_indices->info().size, 0, 6, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
      set->update(6);
    }
    
    void tile_optimizer::set_biome_tile_count(const std::array<std::pair<uint32_t, uint32_t>, MAX_BIOMES_COUNT> &data) {
      auto buffer = reinterpret_cast<struct indirect_buffer*>(indirect->ptr());
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
      auto buffer = reinterpret_cast<struct indirect_buffer*>(indirect->ptr());
      buffer->selection_box = box;
    }
    
    void tile_optimizer::set_selection_frustum(const utils::frustum &frustum) {
      auto buffer = reinterpret_cast<struct indirect_buffer*>(indirect->ptr());
      buffer->selection_frustum = frustum;
    }
    
    uint32_t tile_optimizer::get_borders_indices_count() const { return borders_indices_count; }
    uint32_t tile_optimizer::get_connections_indices_count() const { return connections_indices_count; }
    uint32_t tile_optimizer::get_structures_indices_count() const { return structures_indices_count; }
    uint32_t tile_optimizer::get_heraldies_indices_count() const { return heraldies_indices_count; }
    
    tile_objects_optimizer::tile_objects_optimizer(const create_info &info) : device(info.device), opt(info.opt) {
      auto tiles_data_layout = device->setLayout(TILES_DATA_LAYOUT_NAME);
      auto uniform_layout = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
      auto tiles_indices_layout = device->setLayout("tiles_indices_layout");
      
      yavf::PipelineLayout p_layout = VK_NULL_HANDLE;
      {
        yavf::PipelineLayoutMaker plm(device);
        p_layout = plm.addDescriptorLayout(uniform_layout)
                      .addDescriptorLayout(tiles_data_layout)
                      .addDescriptorLayout(tiles_indices_layout)
                      .create("tile_objects_optimizer_pipeline_layout");
      }
      
      {
        yavf::raii::ShaderModule compute(device, global::root_directory() + "shaders/map_objects.comp.spv");
        
        yavf::ComputePipelineMaker cpm(device);
        pipe = cpm.shader(compute).create("tile_objects_optimizer_pipeline", p_layout);
      }
    }
    
    tile_objects_optimizer::~tile_objects_optimizer() {
      device->destroy(pipe);
      device->destroy(pipe.layout());
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
    
    // виндовс не дает использовать базовый offsetof
#define offsetof123(s,m) ((::size_t)&reinterpret_cast<char const volatile&>((((s*)0)->m)))
    
    void tile_objects_optimizer::proccess(context* ctx) {
      auto map_systems = global::get<systems::map_t>();
      if (!map_systems->is_init()) return;
      auto map = map_systems->map;
      //if (map->status() != core::map::status::valid) return;
//       if (map->status() == core::map::status::initial) return;
      
      auto uniform = global::get<render::buffers>()->uniform;
      auto tiles = map->tiles;
      auto set = opt->buffers_set();
      auto indirect = opt->indirect_buffer();
//       auto points = global::get<render::buffers>()->points;
      
      auto task = ctx->compute();
      
//       task->setBarrier();
      
//       task->setBarrier(VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
//                        VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT);
      
      struct update_buffer_data {
        VkDrawIndirectCommand borders_command;
        uint32_t padding2[4];
        VkDrawIndirectCommand walls_command;
        uint32_t padding3[4];
        VkDrawIndirectCommand structures_command;
        uint32_t padding4[4];
        VkDrawIndirectCommand heraldies_command;
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
      
//       const uint32_t data = 0;
//       task->update(
//         indirect, 
//         offsetof123(struct tile_optimizer::indirect_buffer, borders_command) + 
//         offsetof123(decltype(tile_optimizer::indirect_buffer::borders_command), instanceCount), 
//         sizeof(uint32_t), 
//         &data
//       );
//       task->update(indirect, offsetof123(struct tile_optimizer::indirect_buffer, walls_command), sizeof(uint32_t), &data);
//       task->update(indirect, offsetof123(struct tile_optimizer::indirect_buffer, structures_command), sizeof(uint32_t), &data);
//       task->update(indirect, offsetof123(struct tile_optimizer::indirect_buffer, heraldies_command), sizeof(uint32_t), &data);
      task->update(indirect, update_data_start, sizeof(update_buffer_data), &udata);
      
      task->setBarrier(
        VK_PIPELINE_STAGE_TRANSFER_BIT, 
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_ACCESS_SHADER_READ_BIT
      );
      
      task->setPipeline(pipe);
      task->setDescriptor({
        uniform->descriptorSet()->handle(), 
        tiles->descriptorSet()->handle(), 
//         points->descriptorSet()->handle(), 
        set->handle()
//         tiles_indices->descriptorSet()->handle()
      }, 0);
      
      task->dispatchIndirect(indirect, offsetof123(struct tile_optimizer::indirect_buffer, dispatch_indirect_command));
      
//       task->setBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
//                        VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
      
//       task->setBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
//                        VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
    }
    
    void tile_objects_optimizer::clear() {
#ifndef _NDEBUG
      auto indirect = opt->indirect_buffer();
      auto buffer = reinterpret_cast<struct tile_optimizer::indirect_buffer*>(indirect->ptr());
      ASSERT(buffer->padding3[3] == 1);
#endif
    }
    
    tile_borders_optimizer::tile_borders_optimizer(const create_info &info) : 
      device(info.device), 
      indirect(nullptr), 
      borders_indices(nullptr),
      map_buffers(info.map_buffers) 
    {
      indirect = device->create(yavf::BufferCreateInfo::buffer(sizeof(struct indirect_buffer), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
//       borders_indices = device->create(yavf::BufferCreateInfo::buffer(sizeof(instance_data_t)*(max_tiles_count/4), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT), VMA_MEMORY_USAGE_GPU_ONLY);
      
//       auto pool = device->descriptorPool(DEFAULT_DESCRIPTOR_POOL_NAME);
//       auto storage_layout = device->setLayout(STORAGE_BUFFER_LAYOUT_NAME);
      auto tiles_data_layout = device->setLayout(TILES_DATA_LAYOUT_NAME);
      auto uniform_layout = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
      
      ASSERT(tiles_data_layout != VK_NULL_HANDLE);
      
      yavf::DescriptorPool pool = device->descriptorPool(WORLD_MAP_RENDER_DESCRIPTOR_POOL);
      if (pool == VK_NULL_HANDLE) {
        yavf::DescriptorPoolMaker dpm(device);
        pool = dpm.poolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 30).create(WORLD_MAP_RENDER_DESCRIPTOR_POOL);
      }
      
      yavf::DescriptorSetLayout storage_buffer_layout = device->setLayout(STORAGE_BUFFER_LAYOUT_NAME);
      yavf::DescriptorSetLayout tiles_indices_layout = VK_NULL_HANDLE;
      {
        yavf::DescriptorLayoutMaker dlm(device);
        tiles_indices_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
                                  .binding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
                                  .create("borders_indices_layout");
      }
      
      {
        yavf::DescriptorMaker dm(device);
        set = dm.layout(tiles_indices_layout).create(pool)[0];
        set->resize(2);
//         size_t index = desc->add({indirect, 0, indirect->info().size, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
//                        desc->add({borders_indices, 0, borders_indices->info().size, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
//         desc->update();
//         indirect->setDescriptor(desc, index);
      }
      
      yavf::PipelineLayout p_layout = VK_NULL_HANDLE;
      {
        yavf::PipelineLayoutMaker plm(device);
        p_layout = plm.addDescriptorLayout(uniform_layout)
                      .addDescriptorLayout(tiles_data_layout)
                      .addDescriptorLayout(tiles_indices_layout)
                      .addDescriptorLayout(storage_buffer_layout)
                      .create("borders_optimizer_pipeline_layout");
      }
      
      {
        yavf::raii::ShaderModule compute(device, global::root_directory() + "shaders/borders.comp.spv");
        
        yavf::ComputePipelineMaker cpm(device);
        pipe = cpm.shader(compute).create("borders_optimizer_pipeline", p_layout);
      }
    }
    
    tile_borders_optimizer::~tile_borders_optimizer() {
      device->destroySetLayout("borders_indices_layout");
      device->destroy(indirect);
      if (borders_indices != nullptr) device->destroy(borders_indices);
      device->destroy(pipe.layout());
      device->destroy(pipe);
    }
    
    void tile_borders_optimizer::begin() {
      if (borders_indices == nullptr) return;
      auto buffer = reinterpret_cast<struct indirect_buffer*>(indirect->ptr());
      
//       PRINT_VAR("pentagon_command.vertexCount  ", buffer->pentagon_command.vertexCount)
//       PRINT_VAR("pentagon_command.instanceCount", buffer->pentagon_command.instanceCount)
//       PRINT_VAR("pentagon_command.firstVertex  ", buffer->pentagon_command.firstVertex)
//       PRINT_VAR("pentagon_command.firstInstance", buffer->pentagon_command.firstInstance)
//       PRINT_VAR("hexagon_command.instanceCount ", buffer->hexagon_command.instanceCount)
      
      auto uniform = global::get<render::buffers>()->uniform;
      auto camera_data = reinterpret_cast<render::camera_data*>(uniform->ptr());
      const auto fru = utils::compute_frustum(camera_data->viewproj);
      
      buffer->border_command.vertexCount = 0;
      buffer->border_command.instanceCount = 1;
      buffer->border_command.firstVertex = 0;
      buffer->border_command.firstInstance = 0;
      
      // как передать количество? 
      // я заполняю буфер только после того как сделаю полностью карту
      // в функцию передать
//       buffer->data.x = core::map::tri_count_d(core::map::accel_struct_detail_level);
//       buffer->data.y = max_tiles_count;
//       buffer->data.z = 0;
//       buffer->data.w = 0;
      
      memcpy(&buffer->frustum, &fru, sizeof(utils::frustum));
    }
    
    void tile_borders_optimizer::proccess(context* ctx) {
      if (borders_indices == nullptr) return;
      auto map_systems = global::get<systems::map_t>();
      if (!map_systems->is_init()) return;
      auto map = map_systems->map;
//       if (map->status() == core::map::status::initial) return;
      
      auto uniform = global::get<render::buffers>()->uniform;
      auto tiles = map->tiles;
      auto buffer = reinterpret_cast<struct indirect_buffer*>(indirect->ptr());
      auto border_buffer = map_buffers->border_buffer;
//       auto points = global::get<render::buffers>()->points;
      
      auto task = ctx->compute();
      task->setPipeline(pipe);
      task->setDescriptor({
        uniform->descriptorSet()->handle(), 
        tiles->descriptorSet()->handle(), 
//         points->descriptorSet()->handle(), 
        indirect->descriptorSet()->handle(),
        border_buffer->descriptorSet()->handle()
//         tiles_indices->descriptorSet()->handle()
      }, 0);
      
      const uint32_t count = std::ceil(float(buffer->data.x) / float(work_group_size));
      task->dispatch(count, 1, 1);
      
//       task->setBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
//                        VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_INDIRECT_COMMAND_READ_BIT);
//       
//       task->setBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
//                        VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
    }
    
    void tile_borders_optimizer::clear() {}
    
    yavf::Buffer* tile_borders_optimizer::indirect_buffer() const { return indirect; }
    yavf::Buffer* tile_borders_optimizer::vertices_buffer() const { return borders_indices; }
    
    void tile_borders_optimizer::set_borders_count(const uint32_t &count) {
      if (borders_indices != nullptr) device->destroy(borders_indices);
      borders_indices = device->create(yavf::BufferCreateInfo::buffer(sizeof(instance_data_t)*((count*4)/4+1+count), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT), VMA_MEMORY_USAGE_GPU_ONLY);
      auto buffer = reinterpret_cast<struct indirect_buffer*>(indirect->ptr());
      buffer->data.x = count;
      buffer->data.y = 0;
      buffer->data.z = 0;
      buffer->data.w = 0;
      
      {
        //size_t index = 0;
        set->at(0) = {indirect, 0, indirect->info().size, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
        set->at(1) = {borders_indices, 0, borders_indices->info().size, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
        set->update();
        if (indirect->descriptorSet() == nullptr) indirect->setDescriptor(set, 0);
      }
    }
    
    tile_walls_optimizer::tile_walls_optimizer(const create_info &info) : 
      device(info.device), 
      indirect(nullptr), 
      walls_indices(nullptr), 
      set(nullptr),
      map_buffers(info.map_buffers) 
    {
      indirect = device->create(yavf::BufferCreateInfo::buffer(sizeof(struct indirect_buffer), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
      
//       auto pool = device->descriptorPool(DEFAULT_DESCRIPTOR_POOL_NAME);
//       auto storage_layout = device->setLayout(STORAGE_BUFFER_LAYOUT_NAME);
      auto tiles_data_layout = device->setLayout(TILES_DATA_LAYOUT_NAME);
      auto uniform_layout = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
      
      yavf::DescriptorPool pool = device->descriptorPool(WORLD_MAP_RENDER_DESCRIPTOR_POOL);
      if (pool == VK_NULL_HANDLE) {
        yavf::DescriptorPoolMaker dpm(device);
        pool = dpm.poolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 30).create(WORLD_MAP_RENDER_DESCRIPTOR_POOL);
      }
      
      ASSERT(tiles_data_layout != VK_NULL_HANDLE);
      
      yavf::DescriptorSetLayout storage_buffer_layout = device->setLayout(STORAGE_BUFFER_LAYOUT_NAME);
      yavf::DescriptorSetLayout tiles_indices_layout = VK_NULL_HANDLE;
      {
        yavf::DescriptorLayoutMaker dlm(device);
        tiles_indices_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
                                  .binding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
                                  .create("walls_indices_layout");
      }
      
      {
        yavf::DescriptorMaker dm(device);
        set = dm.layout(tiles_indices_layout).create(pool)[0];
        set->resize(2);
//         size_t index = desc->add({indirect, 0, indirect->info().size, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
//                        desc->add({borders_indices, 0, borders_indices->info().size, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
//         desc->update();
//         indirect->setDescriptor(desc, index);
      }
      
      yavf::PipelineLayout p_layout = VK_NULL_HANDLE;
      {
        yavf::PipelineLayoutMaker plm(device);
        p_layout = plm.addDescriptorLayout(uniform_layout)
                      .addDescriptorLayout(tiles_data_layout)
                      .addDescriptorLayout(tiles_indices_layout)
                      .addDescriptorLayout(storage_buffer_layout)
                      .create("walls_optimizer_pipeline_layout");
      }
      
      {
        yavf::raii::ShaderModule compute(device, global::root_directory() + "shaders/walls.comp.spv");
        
        yavf::ComputePipelineMaker cpm(device);
        pipe = cpm.shader(compute).create("walls_optimizer_pipeline", p_layout);
      }
    }
    
    tile_walls_optimizer::~tile_walls_optimizer() {
      device->destroySetLayout("walls_indices_layout");
      device->destroy(indirect);
      if (walls_indices != nullptr) device->destroy(walls_indices);
      device->destroy(pipe.layout());
      device->destroy(pipe);
    }
    
    void tile_walls_optimizer::begin() {
      if (walls_indices == nullptr) return;
      auto buffer = reinterpret_cast<struct indirect_buffer*>(indirect->ptr());
      
      auto uniform = global::get<render::buffers>()->uniform;
      auto camera_data = reinterpret_cast<render::camera_data*>(uniform->ptr());
      const auto fru = utils::compute_frustum(camera_data->viewproj);
      
      buffer->walls_command.vertexCount = 0;
      buffer->walls_command.instanceCount = 1;
      buffer->walls_command.firstVertex = 0;
      buffer->walls_command.firstInstance = 0;
      
      memcpy(&buffer->frustum, &fru, sizeof(utils::frustum));
    }
    
    void tile_walls_optimizer::proccess(context* ctx) {
      if (walls_indices == nullptr) return;
      auto map_systems = global::get<systems::map_t>();
      if (!map_systems->is_init()) return;
      auto map = map_systems->map;
//       if (map->status() == core::map::status::initial) return;
      
      auto uniform = global::get<render::buffers>()->uniform;
      auto tiles = map->tiles;
      auto buffer = reinterpret_cast<struct indirect_buffer*>(indirect->ptr());
      auto walls_buffer = map_buffers->tiles_connections;
//       auto points = global::get<render::buffers>()->points;
      
      auto task = ctx->compute();
      task->setPipeline(pipe);
      task->setDescriptor({
        uniform->descriptorSet()->handle(), 
        tiles->descriptorSet()->handle(), 
//         points->descriptorSet()->handle(), 
        indirect->descriptorSet()->handle(),
        walls_buffer->descriptorSet()->handle()
//         tiles_indices->descriptorSet()->handle()
      }, 0);
      
      const uint32_t count = std::ceil(float(buffer->data.x) / float(work_group_size));
      task->dispatch(count, 1, 1);
    }
    
    void tile_walls_optimizer::clear() {}
    yavf::Buffer* tile_walls_optimizer::indirect_buffer() const { return indirect; }
    yavf::Buffer* tile_walls_optimizer::vertices_buffer() const { return walls_indices; }
    
    void tile_walls_optimizer::set_connections_count(const uint32_t &count) {
      if (walls_indices != nullptr) device->destroy(walls_indices);
      walls_indices = device->create(yavf::BufferCreateInfo::buffer(sizeof(instance_data_t)*((count*6)/4+1), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT), VMA_MEMORY_USAGE_GPU_ONLY);
      auto buffer = reinterpret_cast<struct indirect_buffer*>(indirect->ptr());
      buffer->data.x = count;
      buffer->data.y = 0;
      buffer->data.z = 0;
      buffer->data.w = 0;
      
      {
        size_t index = 0;
        set->at(0) = {indirect, 0, indirect->info().size, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
        set->at(1) = {walls_indices, 0, walls_indices->info().size, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
        set->update();
        if (indirect->descriptorSet() == nullptr) indirect->setDescriptor(set, index);
      }
    }
    
    void barriers::begin() {}
    void barriers::proccess(context* ctx) {
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

//     const uint32_t max_tiles = 500000;
    tile_render::tile_render(const create_info &info) : 
      device(info.device), 
      opt(info.opt),
      points_indices(nullptr),
      images_set(nullptr)
    {
      static_assert(sizeof(pen_index_array) == 18 * sizeof(uint16_t));
      static_assert(sizeof(hex_index_array) == 21 * sizeof(uint16_t));
      const size_t pen_buffer_size = sizeof(pen_index_array);
      const size_t hex_buffer_size = sizeof(hex_index_array);
      const size_t final_size = align_to(pen_buffer_size+hex_buffer_size, 4);
      //indices = device->create(yavf::BufferCreateInfo::buffer(sizeof(uint32_t)*max_tiles, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
      points_indices = device->create(yavf::BufferCreateInfo::buffer(final_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT), VMA_MEMORY_USAGE_GPU_ONLY);
      
      {
        yavf::Buffer staging(device, yavf::BufferCreateInfo::buffer(final_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
        auto points_ptr = reinterpret_cast<char*>(staging.ptr());
        memcpy(&points_ptr[0],               pen_index_array, pen_buffer_size);
        memcpy(&points_ptr[pen_buffer_size], hex_index_array, hex_buffer_size);
        
        auto task = device->allocateTransferTask();
        task->begin();
        task->copy(&staging, points_indices);
        task->end();
        task->start();
        task->wait();
        device->deallocate(task);
      }

      //yavf::DescriptorPool pool = device->descriptorPool(DEFAULT_DESCRIPTOR_POOL_NAME);

//       yavf::DescriptorSetLayout storage_layout = device->setLayout(STORAGE_BUFFER_LAYOUT_NAME);
//       if (storage_layout == VK_NULL_HANDLE) {
//         yavf::DescriptorLayoutMaker dlm(device);
//         storage_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS).create(STORAGE_BUFFER_LAYOUT_NAME);
//       }

      yavf::DescriptorSetLayout uniform_layout = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
      if (uniform_layout == VK_NULL_HANDLE) {
        yavf::DescriptorLayoutMaker dlm(device);
        uniform_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS).create(UNIFORM_BUFFER_LAYOUT_NAME);
      }
      
      auto tiles_data_layout = device->setLayout(TILES_DATA_LAYOUT_NAME);
      auto images_layout = device->setLayout(IMAGE_CONTAINER_DESCRIPTOR_LAYOUT_NAME);
      ASSERT(images_layout != VK_NULL_HANDLE);
      
      images_set = global::get<systems::core_t>()->image_controller->set;

      yavf::PipelineLayout layout = VK_NULL_HANDLE;
//       yavf::PipelineLayout layout2 = VK_NULL_HANDLE;
      {
        yavf::PipelineLayoutMaker plm(device);
        layout = plm.addDescriptorLayout(uniform_layout)
                    .addDescriptorLayout(images_layout)
                    .addDescriptorLayout(tiles_data_layout)
                    .create(TILE_RENDER_PIPELINE_LAYOUT_NAME);
                    
//         layout2 = plm.addDescriptorLayout(uniform_layout)
//                      .addDescriptorLayout(images_layout)
//                      .addDescriptorLayout(tiles_data_layout)
//                      .addPushConstRange(0, sizeof(uint32_t))
//                      .create("one_tile_pipeline_layout");
      }

      {
        yavf::raii::ShaderModule vertex  (device, global::root_directory() + "shaders/tiles.vert.spv");
        //yavf::raii::ShaderModule geom    (device, global::root_directory() + "shaders/tiles.geom.spv"); // nexus 5x не поддерживает геометрический шейдер
        yavf::raii::ShaderModule fragment(device, global::root_directory() + "shaders/tiles.frag.spv");
//         yavf::raii::ShaderModule vertex2  (device, global::root_directory() + "shaders/one_tile.vert.spv");
//         yavf::raii::ShaderModule fragment2(device, global::root_directory() + "shaders/one_tile.frag.spv");

        yavf::PipelineMaker pm(device);
        pm.clearBlending();

        pipe = pm.addShader(VK_SHADER_STAGE_VERTEX_BIT, vertex)
                 .addShader(VK_SHADER_STAGE_FRAGMENT_BIT, fragment)
                 .vertexBinding(0, sizeof(uint32_t), VK_VERTEX_INPUT_RATE_INSTANCE) //sizeof(instance_data_t)
                   .vertexAttribute(0, 0, VK_FORMAT_R32_UINT, 0)
                 .depthTest(VK_TRUE)
                 .depthWrite(VK_TRUE)
                 .frontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE)
                 .cullMode(VK_CULL_MODE_BACK_BIT)
                 .assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, VK_TRUE) // VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN
                 .viewport()
                 .scissor()
                 .dynamicState(VK_DYNAMIC_STATE_VIEWPORT)
                 .dynamicState(VK_DYNAMIC_STATE_SCISSOR)
                 .colorBlendBegin(VK_FALSE)
                   .colorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
                 .create(TILE_RENDER_PIPELINE_NAME, layout, global::get<render::window>()->render_pass);
      }
    }

    tile_render::~tile_render() {
      device->destroy(points_indices);
      device->destroy(pipe.layout());
      device->destroy(pipe);
    }

    void tile_render::begin() {}
    void tile_render::proccess(context* ctx) {
      auto map_systems = global::get<systems::map_t>();
      if (!map_systems->is_init()) return;
      auto map = map_systems->map;
      
      auto uniform = global::get<render::buffers>()->uniform;
      auto tiles = map->tiles;
      
      auto indirect_buffer = opt->indirect_buffer();
      auto indices_buffer = opt->tiles_index_buffer();
      
      auto task = ctx->graphics();
      task->setPipeline(pipe);
      ASSERT(uniform->descriptorSet() != nullptr);
      ASSERT(tiles->descriptorSet() != nullptr);
      task->setDescriptor({uniform->descriptorSet()->handle(), images_set->handle(), tiles->descriptorSet()->handle()}, 0);
      
      task->setVertexBuffer(indices_buffer, 0, 0);
      task->setIndexBuffer(points_indices, VK_INDEX_TYPE_UINT16, 0);
      task->drawIndexedIndirect(indirect_buffer, 1, offsetof123(struct render::tile_optimizer::indirect_buffer, pen_tiles_command)); // пентагоны отдельно посчитаем
      
      task->setVertexBuffer(indices_buffer, 0, sizeof(uint32_t)*12);
      task->setIndexBuffer(points_indices, VK_INDEX_TYPE_UINT16, sizeof(pen_index_array));
      task->drawIndexedIndirect(indirect_buffer, 1, offsetof123(struct render::tile_optimizer::indirect_buffer, hex_tiles_command));
    }

    void tile_render::clear() {}
    void tile_render::recreate_pipelines(const game::image_resources_t* resource) {
      (void)resource;
    }
    
    void tile_render::change_rendering_mode(const uint32_t &render_mode, const uint32_t &water_mode, const uint32_t &render_slot, const uint32_t &water_slot, const glm::vec3 &color) {
      if (pipe.handle() != VK_NULL_HANDLE) {
        device->destroy(pipe);
      }
      
      // нужно ли это пересоздавать?
      ASSERT(pipe.layout() != VK_NULL_HANDLE);
      
      yavf::raii::ShaderModule vertex  (device, global::root_directory() + "shaders/tiles.vert.spv");
      //yavf::raii::ShaderModule geom    (device, global::root_directory() + "shaders/tiles.geom.spv"); // nexus 5x не поддерживает геометрический шейдер
      yavf::raii::ShaderModule fragment(device, global::root_directory() + "shaders/tiles.frag.spv");

      const uint32_t data[] = {glm::floatBitsToUint(color.x), glm::floatBitsToUint(color.y), glm::floatBitsToUint(color.z), render_mode, water_mode, render_slot, water_slot};
      yavf::PipelineMaker pm(device);
      pm.clearBlending();

      pipe = pm.addShader(VK_SHADER_STAGE_VERTEX_BIT, vertex)
                .addShader(VK_SHADER_STAGE_FRAGMENT_BIT, fragment)
                .addSpecializationEntry(0, 0 * sizeof(uint32_t), sizeof(float))
                .addSpecializationEntry(1, 1 * sizeof(uint32_t), sizeof(float))
                .addSpecializationEntry(2, 2 * sizeof(uint32_t), sizeof(float))
                .addSpecializationEntry(3, 3 * sizeof(uint32_t), sizeof(uint32_t))
                .addSpecializationEntry(4, 4 * sizeof(uint32_t), sizeof(uint32_t))
                .addSpecializationEntry(5, 5 * sizeof(uint32_t), sizeof(uint32_t))
                .addSpecializationEntry(6, 6 * sizeof(uint32_t), sizeof(uint32_t))
                .addData(sizeof(data), data)
//                 .vertexBinding(0, sizeof(uint32_t), VK_VERTEX_INPUT_RATE_INSTANCE)
//                   .vertexAttribute(0, 0, VK_FORMAT_R32_UINT, 0)
//                 .vertexBinding(1, sizeof(uint32_t))
//                   .vertexAttribute(1, 1, VK_FORMAT_R32_UINT, 0)
                .depthTest(VK_TRUE)
                .depthWrite(VK_TRUE)
                .frontFace(VK_FRONT_FACE_CLOCKWISE)
                .cullMode(VK_CULL_MODE_BACK_BIT)
                .assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN, VK_TRUE)
                .viewport()
                .scissor()
                .dynamicState(VK_DYNAMIC_STATE_VIEWPORT)
                .dynamicState(VK_DYNAMIC_STATE_SCISSOR)
                .colorBlendBegin(VK_FALSE)
                  .colorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
                .create(TILE_RENDER_PIPELINE_NAME, pipe.layout(), global::get<render::window>()->render_pass);
    }
    
    yavf::Buffer* tile_render::vertex_indices() const {
      return points_indices;
    }
    
    tile_border_render::tile_border_render(const create_info &info) :
      device(info.device),
      opt(info.opt),
      map_buffers(info.map_buffers)
    {
      auto uniform_layout = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
      auto tiles_data_layout = device->setLayout(TILES_DATA_LAYOUT_NAME);
      auto storage_buffer_layout = device->setLayout(STORAGE_BUFFER_LAYOUT_NAME);
      
      yavf::PipelineLayout layout = VK_NULL_HANDLE;
      {
        yavf::PipelineLayoutMaker plm(device);
        layout = plm.addDescriptorLayout(uniform_layout)
                    .addDescriptorLayout(storage_buffer_layout)
                    .addDescriptorLayout(storage_buffer_layout)
                    .addDescriptorLayout(tiles_data_layout)
                    .create("tile_borders_render_layout");
      }
      
//       glm::vec3 color = glm::vec3(0.0f, 0.0f, 0.0f);
      const std::string name = "tile_borders_render_pipeline";

      {
        yavf::raii::ShaderModule vertex  (device, global::root_directory() + "shaders/borders.vert.spv");
        yavf::raii::ShaderModule fragment(device, global::root_directory() + "shaders/borders.frag.spv");

        yavf::PipelineMaker pm(device);
        pm.clearBlending();

        pipe = pm.addShader(VK_SHADER_STAGE_VERTEX_BIT, vertex)
                 .addShader(VK_SHADER_STAGE_FRAGMENT_BIT, fragment)
                 .vertexBinding(0, sizeof(uint32_t), VK_VERTEX_INPUT_RATE_INSTANCE)
                   .vertexAttribute(0, 0, VK_FORMAT_R32_UINT, 0)
                 .depthTest(VK_TRUE)
                 .depthWrite(VK_TRUE)
                 .frontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE)
                 .cullMode(VK_CULL_MODE_BACK_BIT)
                 .assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, VK_TRUE)
                 .viewport()
                 .scissor()
                 .dynamicState(VK_DYNAMIC_STATE_VIEWPORT)
                 .dynamicState(VK_DYNAMIC_STATE_SCISSOR)
                 .colorBlendBegin(VK_FALSE)
                   .colorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
                 .create(name, layout, global::get<render::window>()->render_pass);
      }
    }
    
    tile_border_render::~tile_border_render() {
      device->destroySetLayout("tile_borders_render_layout");
      device->destroy(pipe.layout());
      device->destroy(pipe);
    }

    void tile_border_render::begin() {}
    void tile_border_render::proccess(context* ctx) {
      auto map_systems = global::get<systems::map_t>();
      if (!map_systems->is_init()) return;
      auto map = map_systems->map;
      auto uniform = global::get<render::buffers>()->uniform;
      auto borders = map_buffers->border_buffer;
      auto types = map_buffers->border_types;
      auto tiles = map->tiles;
      
      // должен быть еще буфер с индексами
      // каждый кадр нужно обойти границы и почекать с фрустумом
      // могу ли я это сделать в компут шейдере? 
      // почему нет
      
      auto indirect_buffer = opt->indirect_buffer();
      auto indices_buffer = opt->borders_index_buffer();
      if (indices_buffer == nullptr) return;
      
      auto task = ctx->graphics();
      task->setPipeline(pipe);
      task->setDescriptor({uniform->descriptorSet()->handle(), borders->descriptorSet()->handle(), types->descriptorSet()->handle(), tiles->descriptorSet()->handle()}, 0);
      //task->setIndexBuffer(indices_buffer);
      task->setVertexBuffer(indices_buffer, 0);
      task->drawIndirect(indirect_buffer, 1, offsetof123(struct tile_optimizer::indirect_buffer, borders_command));
    }
    
    void tile_border_render::clear() {}
    void tile_border_render::recreate_pipelines(const game::image_resources_t* resource) { (void)resource; }
    
    tile_connections_render::tile_connections_render(const create_info &info) :
      device(info.device), 
      opt(info.opt),
      map_buffers(info.map_buffers),
      images_set(nullptr)
    {
      yavf::DescriptorSetLayout uniform_layout = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
      ASSERT(uniform_layout != VK_NULL_HANDLE);
      
      auto tiles_data_layout = device->setLayout(TILES_DATA_LAYOUT_NAME);
      auto storage_layout = device->setLayout(STORAGE_BUFFER_LAYOUT_NAME);
      auto images_layout = device->setLayout(IMAGE_CONTAINER_DESCRIPTOR_LAYOUT_NAME);
      ASSERT(images_layout != VK_NULL_HANDLE);
      images_set = global::get<systems::core_t>()->image_controller->set;

      yavf::PipelineLayout layout = VK_NULL_HANDLE;
      {
        yavf::PipelineLayoutMaker plm(device);
        layout = plm.addDescriptorLayout(uniform_layout)
                    .addDescriptorLayout(images_layout)
                    .addDescriptorLayout(storage_layout)
                    .addDescriptorLayout(tiles_data_layout)
                    .create("walls_rendering_pipeline_layout");
      }

      {
        yavf::raii::ShaderModule vertex  (device, global::root_directory() + "shaders/walls.vert.spv");
        //yavf::raii::ShaderModule geom    (device, global::root_directory() + "shaders/tiles.geom.spv"); // nexus 5x не поддерживает геометрический шейдер
        yavf::raii::ShaderModule fragment(device, global::root_directory() + "shaders/tiles.frag.spv");

        yavf::PipelineMaker pm(device);
        pm.clearBlending();

        pipe = pm.addShader(VK_SHADER_STAGE_VERTEX_BIT, vertex)
                 .addShader(VK_SHADER_STAGE_FRAGMENT_BIT, fragment)
                .vertexBinding(0, sizeof(uint32_t), VK_VERTEX_INPUT_RATE_INSTANCE)
                  .vertexAttribute(0, 0, VK_FORMAT_R32_UINT, 0)
                 .depthTest(VK_TRUE)
                 .depthWrite(VK_TRUE)
                 .frontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE)
                 .cullMode(VK_CULL_MODE_BACK_BIT)
                 .assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, VK_TRUE)
                 .viewport()
                 .scissor()
                 .dynamicState(VK_DYNAMIC_STATE_VIEWPORT)
                 .dynamicState(VK_DYNAMIC_STATE_SCISSOR)
                 .colorBlendBegin(VK_FALSE)
                   .colorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
                 .create("walls_rendering_pipeline", layout, global::get<render::window>()->render_pass);
      }
    }
    
    tile_connections_render::~tile_connections_render() {
      device->destroySetLayout("walls_rendering_pipeline_layout");
      device->destroy(pipe.layout());
      device->destroy(pipe);
    }

    void tile_connections_render::begin() {}
    void tile_connections_render::proccess(context* ctx) {
      auto map_systems = global::get<systems::map_t>();
      if (!map_systems->is_init()) return;
      auto map = map_systems->map;
      auto uniform = global::get<render::buffers>()->uniform;
      auto walls_buffer = map_buffers->tiles_connections;
      auto tiles = map->tiles;
      
      auto indirect_buffer = opt->indirect_buffer();
      auto indices_buffer = opt->walls_index_buffer();
      if (indices_buffer == nullptr) return;
      
      auto task = ctx->graphics();
      task->setPipeline(pipe);
      ASSERT(uniform->descriptorSet() != nullptr);
      ASSERT(walls_buffer->descriptorSet() != nullptr);
      ASSERT(tiles->descriptorSet() != nullptr);
      task->setDescriptor({uniform->descriptorSet()->handle(), images_set->handle(), walls_buffer->descriptorSet()->handle(), tiles->descriptorSet()->handle()}, 0);
      
      task->setVertexBuffer(indices_buffer, 0);
      //task->setIndexBuffer(indices_buffer);
      task->drawIndirect(indirect_buffer, 1, offsetof123(struct tile_optimizer::indirect_buffer, walls_command));
    }
    
    void tile_connections_render::clear() {}

    void tile_connections_render::recreate_pipelines(const game::image_resources_t* resource) { (void)resource; }
    void tile_connections_render::change_rendering_mode(const uint32_t &render_mode, const uint32_t &water_mode, const uint32_t &render_slot, const uint32_t &water_slot, const glm::vec3 &color) {
      if (pipe.handle() != VK_NULL_HANDLE) {
        device->destroy(pipe);
      }
      
      // нужно ли это пересоздавать?
      ASSERT(pipe.layout() != VK_NULL_HANDLE);
      
      yavf::raii::ShaderModule vertex  (device, global::root_directory() + "shaders/walls.vert.spv");
      //yavf::raii::ShaderModule geom    (device, global::root_directory() + "shaders/tiles.geom.spv"); // nexus 5x не поддерживает геометрический шейдер
      yavf::raii::ShaderModule fragment(device, global::root_directory() + "shaders/tiles.frag.spv");

      const uint32_t data[] = {glm::floatBitsToUint(color.x), glm::floatBitsToUint(color.y), glm::floatBitsToUint(color.z), render_mode, water_mode, render_slot, water_slot};
      yavf::PipelineMaker pm(device);
      pm.clearBlending();

      pipe = pm.addShader(VK_SHADER_STAGE_VERTEX_BIT, vertex)
                .addShader(VK_SHADER_STAGE_FRAGMENT_BIT, fragment)
                .addSpecializationEntry(0, 0 * sizeof(uint32_t), sizeof(float))
                .addSpecializationEntry(1, 1 * sizeof(uint32_t), sizeof(float))
                .addSpecializationEntry(2, 2 * sizeof(uint32_t), sizeof(float))
                .addSpecializationEntry(3, 3 * sizeof(uint32_t), sizeof(uint32_t))
                .addSpecializationEntry(4, 4 * sizeof(uint32_t), sizeof(uint32_t))
                .addSpecializationEntry(5, 5 * sizeof(uint32_t), sizeof(uint32_t))
                .addSpecializationEntry(6, 6 * sizeof(uint32_t), sizeof(uint32_t))
                .addData(sizeof(data), data)
//                 .vertexBinding(0, sizeof(uint32_t))
//                   .vertexAttribute(0, 0, VK_FORMAT_R32_UINT, 0)
                .depthTest(VK_TRUE)
                .depthWrite(VK_TRUE)
                .frontFace(VK_FRONT_FACE_CLOCKWISE)
                .cullMode(VK_CULL_MODE_FRONT_BIT)
                .assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_TRUE)
                .viewport()
                .scissor()
                .dynamicState(VK_DYNAMIC_STATE_VIEWPORT)
                .dynamicState(VK_DYNAMIC_STATE_SCISSOR)
                .colorBlendBegin(VK_FALSE)
                  .colorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
                .create("walls_rendering_pipeline", pipe.layout(), global::get<render::window>()->render_pass);
    }
    
    tile_object_render::tile_object_render(const create_info &info) : device(info.device), opt(info.opt), map_buffers(info.map_buffers), images_set(nullptr) {
      yavf::DescriptorSetLayout uniform_layout = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
      ASSERT(uniform_layout != VK_NULL_HANDLE);
      
      auto tiles_data_layout = device->setLayout(TILES_DATA_LAYOUT_NAME);
//       auto storage_layout = device->setLayout(STORAGE_BUFFER_LAYOUT_NAME);
      auto images_layout = device->setLayout(IMAGE_CONTAINER_DESCRIPTOR_LAYOUT_NAME);
      ASSERT(images_layout != VK_NULL_HANDLE);
      images_set = global::get<systems::core_t>()->image_controller->set;

      yavf::PipelineLayout layout = VK_NULL_HANDLE;
      {
        yavf::PipelineLayoutMaker plm(device);
        layout = plm.addDescriptorLayout(uniform_layout)
                    .addDescriptorLayout(images_layout)
                    //.addDescriptorLayout(storage_layout)
                    .addDescriptorLayout(tiles_data_layout)
                    .create("object_rendering_pipeline_layout");
      }
      
      {
        yavf::raii::ShaderModule vertex  (device, global::root_directory() + "shaders/first_object.vert.spv");
        //yavf::raii::ShaderModule geom    (device, global::root_directory() + "shaders/tiles.geom.spv"); // nexus 5x не поддерживает геометрический шейдер
        yavf::raii::ShaderModule fragment(device, global::root_directory() + "shaders/first_object.frag.spv");

        yavf::PipelineMaker pm(device);
        pm.clearBlending();

        pipe = pm.addShader(VK_SHADER_STAGE_VERTEX_BIT, vertex)
                 .addShader(VK_SHADER_STAGE_FRAGMENT_BIT, fragment)
//                  .vertexBinding(0, sizeof(uint32_t))
//                    .vertexAttribute(0, 0, VK_FORMAT_R32_UINT, 0)
                 .depthTest(VK_TRUE)
                 .depthWrite(VK_TRUE)
                 .frontFace(VK_FRONT_FACE_CLOCKWISE)
                 .cullMode(VK_CULL_MODE_BACK_BIT)
                 .assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, VK_TRUE)
                 .viewport()
                 .scissor()
                 .dynamicState(VK_DYNAMIC_STATE_VIEWPORT)
                 .dynamicState(VK_DYNAMIC_STATE_SCISSOR)
                 .colorBlendBegin(VK_FALSE)
                   .colorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
                 .create("object_rendering_pipeline", layout, global::get<render::window>()->render_pass);
      }
    }
    
    tile_object_render::~tile_object_render() {
      device->destroy(pipe.layout());
      device->destroy(pipe);
    }

    void tile_object_render::begin() {}
    void tile_object_render::proccess(context* ctx) {
      auto map_systems = global::get<systems::map_t>();
      if (!map_systems->is_init()) return;
      auto map = map_systems->map;
      auto uniform = global::get<render::buffers>()->uniform;
      auto tiles = map->tiles;
      
      auto indirect_buffer = opt->indirect_buffer();
      auto indices_buffer = opt->objects_index_buffer();
      if (indices_buffer == nullptr) return;
      
      auto task = ctx->graphics();
      task->setPipeline(pipe);
      ASSERT(uniform->descriptorSet() != nullptr);
      ASSERT(tiles->descriptorSet() != nullptr);
      task->setDescriptor({uniform->descriptorSet()->handle(), images_set->handle(), tiles->descriptorSet()->handle()}, 0);
      
      //task->setVertexBuffer(vertices_buffer, 0);
      task->setIndexBuffer(indices_buffer);
      for (uint32_t i = 0; i < MAX_BIOMES_COUNT; ++i) {
        task->drawIndexedIndirect(indirect_buffer, 1, offsetof123(struct tile_optimizer::indirect_buffer, biome_data)+sizeof(biome_objects_data_t)*i);
      }
    }
    
    void tile_object_render::clear() {}
    
#define HIGHLIGHT_DATA_COUNT 2
    tile_highlights_render::tile_highlights_render(const create_info &info) : device(info.device), map_buffers(info.map_buffers), tiles_indices(nullptr), hex_tiles_count(0), pen_tiles_count(0) {
      yavf::DescriptorSetLayout uniform_layout = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
      ASSERT(uniform_layout != VK_NULL_HANDLE);
      
      auto tiles_data_layout = device->setLayout(TILES_DATA_LAYOUT_NAME);
      yavf::PipelineLayout layout = VK_NULL_HANDLE;
      {
        yavf::PipelineLayoutMaker plm(device);
        layout = plm.addDescriptorLayout(uniform_layout)
//                     .addDescriptorLayout(images_layout)
                    //.addDescriptorLayout(storage_layout)
                    .addDescriptorLayout(tiles_data_layout)
                    .create("tiling_highlights_rendering_pipeline_layout");
      }
      
      {
        yavf::raii::ShaderModule vertex  (device, global::root_directory() + "shaders/one_tile.vert.spv");
        //yavf::raii::ShaderModule geom    (device, global::root_directory() + "shaders/tiles.geom.spv"); // nexus 5x не поддерживает геометрический шейдер
        yavf::raii::ShaderModule fragment(device, global::root_directory() + "shaders/one_tile.frag.spv");

        yavf::PipelineMaker pm(device);
        //pm.clearBlending();

        pipe = pm.addShader(VK_SHADER_STAGE_VERTEX_BIT, vertex)
                 .addShader(VK_SHADER_STAGE_FRAGMENT_BIT, fragment)
                 .vertexBinding(0, sizeof(uint32_t)*HIGHLIGHT_DATA_COUNT, VK_VERTEX_INPUT_RATE_INSTANCE)
                   .vertexAttribute(0, 0, VK_FORMAT_R32_UINT, 0)
                   .vertexAttribute(1, 0, VK_FORMAT_R32_UINT, sizeof(uint32_t))
                 .depthTest(VK_TRUE)
                 .depthWrite(VK_TRUE)
                 .frontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE)
                 .cullMode(VK_CULL_MODE_BACK_BIT)
                 .assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN, VK_TRUE)
                 .viewport()
                 .scissor()
                 .dynamicState(VK_DYNAMIC_STATE_VIEWPORT)
                 .dynamicState(VK_DYNAMIC_STATE_SCISSOR)
                 .colorBlendBegin(VK_TRUE)
                   //.colorBlending(VK_)
                   //.colorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
                 .create("tiling_highlights_rendering_pipeline", layout, global::get<render::window>()->render_pass);
      }
      
      const size_t size = (max_indices_count * 2 + 16 - 1) / 16 * 16;
      tiles_indices = device->create(yavf::BufferCreateInfo::buffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
    }
    
    tile_highlights_render::~tile_highlights_render() {
      device->destroy(pipe.layout());
      device->destroy(pipe);
      device->destroy(tiles_indices);
    }
    
    void tile_highlights_render::begin() {}
    void tile_highlights_render::proccess(context* ctx) {
      if (hex_tiles_count + pen_tiles_count == 0) return;
      
      auto map_systems = global::get<systems::map_t>();
      if (!map_systems->is_init()) return;
      auto map = map_systems->map;
      auto uniform = global::get<render::buffers>()->uniform;
      auto tiles = map->tiles;
      
      auto task = ctx->graphics();
      task->setPipeline(pipe);
      ASSERT(uniform->descriptorSet() != nullptr);
      ASSERT(tiles->descriptorSet() != nullptr);
      task->setDescriptor({uniform->descriptorSet()->handle(), tiles->descriptorSet()->handle()}, 0);
      //task->setIndexBuffer(tiles_indices);
      task->setVertexBuffer(tiles_indices, 0);
      //task->drawIndexed(tiles_count, 1, 0, 0, 0);
      if (pen_tiles_count != 0) task->draw(5, pen_tiles_count, 0, 0);
      if (hex_tiles_count != 0) task->draw(6, hex_tiles_count, 0, 12);
    }
    
    void tile_highlights_render::clear() {
      hex_tiles_count = 0;
      pen_tiles_count = 0;
    }
    
    void tile_highlights_render::add(const uint32_t &tile_index, const color_t &color) {
      const bool pentagon = tile_index < 12;
      auto tiles_indices_ptr = reinterpret_cast<uint32_t*>(tiles_indices->ptr());
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
      
//       const uint32_t packed_tile_index = tile_index*PACKED_TILE_INDEX_COEF;
//       const uint32_t p_count = tile_index < 12 ? 5 : 6; // кажется так разделяются тайлы
//       const uint32_t offset = tiles_count.fetch_add(p_count+1);
//       
//       for (uint32_t i = 0; i < p_count; ++i) {
//         tiles_indices_ptr[offset+i] = packed_tile_index+i;
//       }
//       
//       tiles_indices_ptr[offset+p_count] = GPU_UINT_MAX;
    }
    
    tile_structure_render::tile_structure_render(const create_info &info) : device(info.device), opt(info.opt), map_buffers(info.map_buffers), images_set(nullptr) {
      yavf::DescriptorSetLayout uniform_layout = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
      ASSERT(uniform_layout != VK_NULL_HANDLE);
      
      auto tiles_data_layout = device->setLayout(TILES_DATA_LAYOUT_NAME);
//       auto storage_layout = device->setLayout(STORAGE_BUFFER_LAYOUT_NAME);
      auto images_layout = device->setLayout(IMAGE_CONTAINER_DESCRIPTOR_LAYOUT_NAME);
      ASSERT(images_layout != VK_NULL_HANDLE);
      images_set = global::get<systems::core_t>()->image_controller->set;

      yavf::PipelineLayout layout = VK_NULL_HANDLE;
      {
        yavf::PipelineLayoutMaker plm(device);
        layout = plm.addDescriptorLayout(uniform_layout)
                    .addDescriptorLayout(images_layout)
                    //.addDescriptorLayout(storage_layout)
                    .addDescriptorLayout(tiles_data_layout)
                    .create("tile_structure_rendering_pipeline_layout");
      }
      
      {
        yavf::raii::ShaderModule vertex  (device, global::root_directory() + "shaders/structure.vert.spv");
        //yavf::raii::ShaderModule geom    (device, global::root_directory() + "shaders/tiles.geom.spv"); // nexus 5x не поддерживает геометрический шейдер
        yavf::raii::ShaderModule fragment(device, global::root_directory() + "shaders/first_object.frag.spv");

        yavf::PipelineMaker pm(device);
        pm.clearBlending();

        pipe = pm.addShader(VK_SHADER_STAGE_VERTEX_BIT, vertex)
                 .addShader(VK_SHADER_STAGE_FRAGMENT_BIT, fragment)
                 .vertexBinding(0, sizeof(uint32_t), VK_VERTEX_INPUT_RATE_INSTANCE)
                   .vertexAttribute(0, 0, VK_FORMAT_R32_UINT, 0)
                 .depthTest(VK_TRUE)
                 .depthWrite(VK_TRUE)
                 .frontFace(VK_FRONT_FACE_CLOCKWISE)
                 .cullMode(VK_CULL_MODE_NONE)
                 .assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, VK_TRUE)
                 .viewport()
                 .scissor()
                 .dynamicState(VK_DYNAMIC_STATE_VIEWPORT)
                 .dynamicState(VK_DYNAMIC_STATE_SCISSOR)
                 .colorBlendBegin(VK_FALSE)
                   .colorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
                 .create("tile_structure_rendering_pipeline", layout, global::get<render::window>()->render_pass);
      }
    }
    
    tile_structure_render::~tile_structure_render() {
      device->destroy(pipe.layout());
      device->destroy(pipe);
    }
    
    void tile_structure_render::begin() {}
    void tile_structure_render::proccess(context* ctx) {
      auto map_systems = global::get<systems::map_t>();
      if (!map_systems->is_init()) return;
      auto map = map_systems->map;
      auto uniform = global::get<render::buffers>()->uniform;
      auto tiles = map->tiles;
      
      auto indirect_buffer = opt->indirect_buffer();
      //auto indices_buffer = opt->structures_index_buffer();
      auto indices_buffer = opt->borders_index_buffer();
      if (indices_buffer == nullptr) return;
      
      auto task = ctx->graphics();
      task->setPipeline(pipe);
      ASSERT(uniform->descriptorSet() != nullptr);
      ASSERT(tiles->descriptorSet() != nullptr);
      task->setDescriptor({uniform->descriptorSet()->handle(), images_set->handle(), tiles->descriptorSet()->handle()}, 0);
      
      task->setVertexBuffer(indices_buffer, 0);
      //task->setIndexBuffer(indices_buffer);
      task->drawIndirect(indirect_buffer, 1, offsetof123(struct tile_optimizer::indirect_buffer, borders_command));
      //task->drawIndexedIndirect(indirect_buffer, 1, offsetof123(struct tile_optimizer::indirect_buffer, borders_command));
    }
    
    void tile_structure_render::clear() {}
    
    heraldies_render::heraldies_render(const create_info &info) : device(info.device), opt(info.opt), map_buffers(info.map_buffers), images_set(nullptr) {
      yavf::DescriptorSetLayout uniform_layout = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
      ASSERT(uniform_layout != VK_NULL_HANDLE);
      
      auto tiles_data_layout = device->setLayout(TILES_DATA_LAYOUT_NAME);
      auto storage_layout = device->setLayout(STORAGE_BUFFER_LAYOUT_NAME);
      auto images_layout = device->setLayout(IMAGE_CONTAINER_DESCRIPTOR_LAYOUT_NAME);
      ASSERT(images_layout != VK_NULL_HANDLE);
      images_set = global::get<systems::core_t>()->image_controller->set;

      yavf::PipelineLayout layout = VK_NULL_HANDLE;
      {
        yavf::PipelineLayoutMaker plm(device);
        layout = plm.addDescriptorLayout(uniform_layout)
                    .addDescriptorLayout(images_layout)
                    //.addDescriptorLayout(storage_layout)
                    .addDescriptorLayout(tiles_data_layout)
                    .addDescriptorLayout(storage_layout)
                    .create("heraldies_rendering_pipeline_layout");
      }
      
      {
        yavf::raii::ShaderModule vertex  (device, global::root_directory() + "shaders/heraldy.vert.spv");
        //yavf::raii::ShaderModule geom    (device, global::root_directory() + "shaders/tiles.geom.spv"); // nexus 5x не поддерживает геометрический шейдер
        yavf::raii::ShaderModule fragment(device, global::root_directory() + "shaders/heraldy.frag.spv");

        yavf::PipelineMaker pm(device);
//         pm.clearBlending();

        pipe = pm.addShader(VK_SHADER_STAGE_VERTEX_BIT, vertex)
                 .addShader(VK_SHADER_STAGE_FRAGMENT_BIT, fragment)
                 .vertexBinding(0, sizeof(uint32_t), VK_VERTEX_INPUT_RATE_INSTANCE)
                   .vertexAttribute(0, 0, VK_FORMAT_R32_UINT, 0)
                 .depthTest(VK_TRUE)
                 .depthWrite(VK_TRUE)
                 .frontFace(VK_FRONT_FACE_CLOCKWISE)
                 .cullMode(VK_CULL_MODE_BACK_BIT)
                 .assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, VK_TRUE)
                 .viewport()
                 .scissor()
                 .dynamicState(VK_DYNAMIC_STATE_VIEWPORT)
                 .dynamicState(VK_DYNAMIC_STATE_SCISSOR)
                 .colorBlendBegin(VK_TRUE)
                   
//                    .colorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
                 .create("heraldies_rendering_pipeline", layout, global::get<render::window>()->render_pass);
      }
    }
    
    heraldies_render::~heraldies_render() {
      device->destroyLayout("heraldies_rendering_pipeline_layout");
      device->destroy(pipe);
    }
    
    void heraldies_render::begin() {}
    void heraldies_render::proccess(context* ctx) {
      auto map_systems = global::get<systems::map_t>();
      if (!map_systems->is_init()) return;
      auto map = map_systems->map;
      auto uniform = global::get<render::buffers>()->uniform;
      auto heraldy = global::get<render::buffers>()->heraldy;
      auto tiles = map->tiles;
      
      auto indirect_buffer = opt->indirect_buffer();
      //auto indices_buffer = opt->heraldy_index_buffer();
      auto indices_buffer = opt->walls_index_buffer();
      if (indices_buffer == nullptr) return;
      
      auto task = ctx->graphics();
      task->setPipeline(pipe);
      ASSERT(uniform->descriptorSet() != nullptr);
      ASSERT(tiles->descriptorSet() != nullptr);
      task->setDescriptor({uniform->descriptorSet()->handle(), images_set->handle(), tiles->descriptorSet()->handle(), heraldy->descriptorSet()->handle()}, 0);
      
      task->setVertexBuffer(indices_buffer, 0);
      //task->setIndexBuffer(indices_buffer);
      task->drawIndirect(indirect_buffer, 1, offsetof123(struct tile_optimizer::indirect_buffer, walls_command));
      //task->drawIndexedIndirect(indirect_buffer, 1, offsetof123(struct tile_optimizer::indirect_buffer, walls_command));
    }
    
    void heraldies_render::clear() {}
    
    armies_render::armies_render(const create_info &info) : device(info.device), opt(info.opt), map_buffers(info.map_buffers), images_set(nullptr) {
      yavf::DescriptorSetLayout uniform_layout = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
      ASSERT(uniform_layout != VK_NULL_HANDLE);
      
      auto tiles_data_layout = device->setLayout(TILES_DATA_LAYOUT_NAME);
      auto storage_layout = device->setLayout(STORAGE_BUFFER_LAYOUT_NAME);
      auto images_layout = device->setLayout(IMAGE_CONTAINER_DESCRIPTOR_LAYOUT_NAME);
      ASSERT(images_layout != VK_NULL_HANDLE);
      images_set = global::get<systems::core_t>()->image_controller->set;

      yavf::PipelineLayout layout = VK_NULL_HANDLE;
      {
        yavf::PipelineLayoutMaker plm(device);
        layout = plm.addDescriptorLayout(uniform_layout)
                    .addDescriptorLayout(images_layout)
                    //.addDescriptorLayout(storage_layout)
                    .addDescriptorLayout(tiles_data_layout)
                    .addDescriptorLayout(storage_layout)
                    .create("armies_rendering_pipeline_layout");
      }
      
      {
        yavf::raii::ShaderModule vertex  (device, global::root_directory() + "shaders/armies.vert.spv");
        //yavf::raii::ShaderModule geom    (device, global::root_directory() + "shaders/tiles.geom.spv"); // nexus 5x не поддерживает геометрический шейдер
        yavf::raii::ShaderModule fragment(device, global::root_directory() + "shaders/tiles.frag.spv");

        yavf::PipelineMaker pm(device);
//         pm.clearBlending();

        pipe = pm.addShader(VK_SHADER_STAGE_VERTEX_BIT, vertex)
                 .addShader(VK_SHADER_STAGE_FRAGMENT_BIT, fragment)
                 .vertexBinding(0, sizeof(uint32_t), VK_VERTEX_INPUT_RATE_INSTANCE)
                   .vertexAttribute(0, 0, VK_FORMAT_R32_UINT, 0)
                 .depthTest(VK_TRUE)
                 .depthWrite(VK_TRUE)
                 .frontFace(VK_FRONT_FACE_CLOCKWISE)
                 .cullMode(VK_CULL_MODE_NONE)
                 .assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, VK_TRUE)
                 .viewport()
                 .scissor()
                 .dynamicState(VK_DYNAMIC_STATE_VIEWPORT)
                 .dynamicState(VK_DYNAMIC_STATE_SCISSOR)
                 .colorBlendBegin(VK_TRUE)
                   
//                    .colorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
                 .create("armies_rendering_pipeline", layout, global::get<render::window>()->render_pass);
      }
    }
    
    armies_render::~armies_render() {
      device->destroyLayout("armies_rendering_pipeline_layout");
      device->destroy(pipe);
    }
    
    void armies_render::begin() {}
    void armies_render::proccess(context* ctx) {
      auto map_systems = global::get<systems::map_t>();
      if (!map_systems->is_init()) return;
      auto map = map_systems->map;
      auto uniform = global::get<render::buffers>()->uniform;
//       auto heraldy = global::get<render::buffers>()->heraldy;
      auto tiles = map->tiles;
      
      auto indirect_buffer = opt->indirect_buffer();
      auto indices_buffer = opt->objects_index_buffer();
      if (indices_buffer == nullptr) return;
      
      auto task = ctx->graphics();
      task->setPipeline(pipe);
      ASSERT(uniform->descriptorSet() != nullptr);
      ASSERT(tiles->descriptorSet() != nullptr);
      task->setDescriptor({uniform->descriptorSet()->handle(), images_set->handle(), tiles->descriptorSet()->handle()}, 0); // heraldy->descriptorSet()->handle()
      
      task->setVertexBuffer(indices_buffer, 0);
      //task->setIndexBuffer(indices_buffer);
      task->drawIndirect(indirect_buffer, 1, offsetof123(struct tile_optimizer::indirect_buffer, structures_command));
      //task->drawIndexedIndirect(indirect_buffer, 1, offsetof123(struct tile_optimizer::indirect_buffer, structures_command));
    }
    
    void armies_render::clear() {}
    
    world_map_render::world_map_render(const create_info &info) : stage_container(info.container_size) {}
    void world_map_render::begin() {
      stage_container::begin();
    }
    
    void world_map_render::proccess(context* ctx) {
      auto map_systems = global::get<systems::map_t>();
//       auto map = map_systems->map;
      if (!map_systems->is_init()) return;
//       if (map->status() != core::map::status::rendering) return;
      
      stage_container::proccess(ctx);
    }
    
    void world_map_render::clear() {
      stage_container::clear();
    }
    
    void world_map_render::recreate_pipelines(const game::image_resources_t* resource) { (void)resource; }
    
    struct gui_vertex {
      glm::vec2 pos;
      glm::vec2 uv;
      uint32_t color;
    };

#define MAX_VERTEX_BUFFER (2 * 1024 * 1024)
#define MAX_INDEX_BUFFER (512 * 1024)
    
    interface_stage::interface_stage(const create_info &info) : 
      device(info.device),
      vertex_gui(device, yavf::BufferCreateInfo::buffer(MAX_VERTEX_BUFFER, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY), 
      index_gui(device, yavf::BufferCreateInfo::buffer(MAX_INDEX_BUFFER, VK_BUFFER_USAGE_INDEX_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY), 
      matrix(device, yavf::BufferCreateInfo::buffer(sizeof(glm::mat4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY)
    {
      auto w = global::get<render::window>();
      images_set = global::get<systems::core_t>()->image_controller->set;
      
      yavf::DescriptorSetLayout sampled_image_layout = device->setLayout(SAMPLED_IMAGE_LAYOUT_NAME);
      yavf::DescriptorSetLayout uniform_layout       = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
      auto storage_layout                            = device->setLayout(STORAGE_BUFFER_LAYOUT_NAME);
      auto images_layout = device->setLayout(IMAGE_CONTAINER_DESCRIPTOR_LAYOUT_NAME);
      {
        yavf::DescriptorLayoutMaker dlm(device);
        
        if (sampled_image_layout == VK_NULL_HANDLE) {
          sampled_image_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT).create(SAMPLED_IMAGE_LAYOUT_NAME);
        }
      }
      
      {
        yavf::DescriptorPool pool = device->descriptorPool(DEFAULT_DESCRIPTOR_POOL_NAME);
    
        yavf::DescriptorMaker dm(device);
        auto d = dm.layout(uniform_layout).create(pool)[0];
        const size_t index = d->add({&matrix, 0, matrix.info().size, 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER});
        matrix.setDescriptor(d, index);
      }
      
//       yavf::DescriptorSetLayout sampled_image_layout = device->setLayout(SAMPLED_IMAGE_LAYOUT_NAME);
//       yavf::DescriptorSetLayout uniform_layout       = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
      
      yavf::PipelineLayout gui_layout = device->layout("gui_layout");
      {
        if (gui_layout != VK_NULL_HANDLE) device->destroy(gui_layout);
        
        yavf::PipelineLayoutMaker plm(device);
        gui_layout = plm.addDescriptorLayout(uniform_layout)
                        .addDescriptorLayout(images_layout)
                        .addDescriptorLayout(storage_layout)
                        .addPushConstRange(0, sizeof(image_handle_data))
                        .create("gui_layout");
      }
      
//       uint32_t constants[2] = {resource->images, resource->samplers};
//       image_set = resource->set;
      
      {
        if (pipe.handle() != VK_NULL_HANDLE) device->destroy(pipe);
        yavf::PipelineMaker pm(device);
        pm.clearBlending();
        
        yavf::raii::ShaderModule vertex(device, (global::root_directory() + "shaders/gui.vert.spv").c_str());
        yavf::raii::ShaderModule fragment(device, (global::root_directory() + "shaders/gui.frag.spv").c_str());
        
        pipe = pm.addShader(VK_SHADER_STAGE_VERTEX_BIT, vertex)
                 .addShader(VK_SHADER_STAGE_FRAGMENT_BIT, fragment)
//                    .addSpecializationEntry(0, 0, sizeof(uint32_t))
//                    .addSpecializationEntry(1, sizeof(uint32_t), sizeof(uint32_t))
//                    .addData(2*sizeof(uint32_t), constants)
                 .vertexBinding(0, sizeof(gui_vertex))
                   .vertexAttribute(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(gui_vertex, pos))
                   .vertexAttribute(1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(gui_vertex, uv))
                   .vertexAttribute(2, 0, VK_FORMAT_R8G8B8A8_UINT, offsetof(gui_vertex, color))
                 .assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
                 .depthTest(VK_FALSE)
                 .depthWrite(VK_FALSE)
                 .clearBlending()
                 .colorBlendBegin()
                   .srcColor(VK_BLEND_FACTOR_SRC_ALPHA)
                   .dstColor(VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
                   .srcAlpha(VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
                   .dstAlpha(VK_BLEND_FACTOR_SRC_ALPHA)
                 .viewport()
                 .scissor()
                 .dynamicState(VK_DYNAMIC_STATE_VIEWPORT)
                 .dynamicState(VK_DYNAMIC_STATE_SCISSOR)
                 .create("gui_pipeline", gui_layout, w->render_pass);
      }
    }
    
    interface_stage::~interface_stage() {
      device->destroyLayout("gui_layout");
      device->destroy(pipe);
    }
    
    void interface_stage::begin() {
      auto data = global::get<interface::context>();
      auto w = global::get<render::window>();
  
      {
        void* vertices = vertex_gui.ptr();
        void* elements = index_gui.ptr();
        
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
        nk_buffer_init_fixed(&vbuf, vertices, vertex_gui.info().size);
        nk_buffer_init_fixed(&ebuf, elements, index_gui.info().size);
        nk_convert(&data->ctx, &data->cmds, &vbuf, &ebuf, &config);
      }
      
      glm::mat4* mat = reinterpret_cast<glm::mat4*>(matrix.ptr());
      *mat = glm::mat4(
        2.0f / w->viewport().width,  0.0f,  0.0f,  0.0f,
        0.0f,  2.0f / w->viewport().height,  0.0f,  0.0f,
        0.0f,  0.0f, -1.0f,  0.0f,
        -1.0f, -1.0f,  0.0f,  1.0f
      );
    }
    
    void interface_stage::proccess(context* ctx) {
      yavf::GraphicTask* task = ctx->graphics();
      auto data = global::get<interface::context>();
      
      task->setPipeline(pipe);

      task->setVertexBuffer(&vertex_gui, 0);
      task->setIndexBuffer(&index_gui, VK_INDEX_TYPE_UINT16);
      
      auto heraldy = global::get<render::buffers>()->heraldy;
      const std::initializer_list<VkDescriptorSet> sets = {matrix.descriptorSet()->handle(), images_set->handle(), heraldy->descriptorSet()->handle()}; // , image_set->handle()
      task->setDescriptor(sets, 0);
      
      uint32_t index_offset = 0;
      const nk_draw_command *cmd = nullptr;
      nk_draw_foreach(cmd, &data->ctx, &data->cmds) {
        if (cmd->elem_count == 0) continue;
        
        //const render::image_t i = image_nk_handle(cmd->texture);
        const image_handle_data data = nk_handle_to_image_data(cmd->texture);
        //ASSERT(i.index == UINT32_MAX && i.layer == UINT32_MAX);
        //auto data = glm::uvec4(i.container, 0, 0, 0);
    //     PRINT_VEC2("image id", data)
        task->setConsts(0, sizeof(data), &data);

        const glm::vec2 fb_scale = input::get_input_data()->fb_scale;
        const VkRect2D scissor{
          {
            static_cast<int32_t>(std::max(cmd->clip_rect.x * fb_scale.x, 0.0f)),
            static_cast<int32_t>(std::max(cmd->clip_rect.y * fb_scale.y, 0.0f)),
          },
          {
            static_cast<uint32_t>(cmd->clip_rect.w * fb_scale.x),
            static_cast<uint32_t>(cmd->clip_rect.h * fb_scale.y),
          }
        };
        
        task->setScissor(scissor);
        task->drawIndexed(cmd->elem_count, 1, index_offset, 0, 0);
        index_offset += cmd->elem_count;
      }
    }
    
    void interface_stage::clear() {
      auto data = global::get<interface::context>();
      nk_buffer_clear(&data->cmds);
      nk_clear(&data->ctx);
    }

    void interface_stage::recreate_pipelines(const game::image_resources_t* resource) {
      (void)resource;
    }

    task_start::task_start(yavf::Device* device) : device(device) {}
    void task_start::begin() {}
    void task_start::proccess(context* ctx) {
      yavf::TaskInterface* task = ctx->interface();
      const VkSubmitInfo &info = task->getSubmitInfo();
      const uint32_t &family = task->getFamily();
//       PRINT_VAR("family", family)

      wait_fence = device->submit(family, 1, &info);
      global::get<window>()->present(wait_fence);
//       vkDeviceWaitIdle(device->handle());
    }

    void task_start::clear() {}
    // если мы wait передадим в другой поток, то тут мы можем разблокировать ресурсы (карту)
    // и так это будет выглядеть наиболее нормально, другое дело что нам нужно ждать этот поток
    // иначе мы не отловим ошибку + мы можем попытаться подождать всю загрузку используя обычное ожидание
    void task_start::wait() {
      const VkResult res = vkWaitForFences(device->handle(), 1, &wait_fence.fence, VK_TRUE, 1000000000);
      if (res != VK_SUCCESS) {
        throw std::runtime_error("drawing takes too long");
      }
    }
    
    bool task_start::status() const {
      const VkResult res = vkGetFenceStatus(device->handle(), wait_fence.fence);
      ASSERT(res != VK_ERROR_DEVICE_LOST);
      return res == VK_SUCCESS;
    }

    window_present::window_present(const create_info &info) : w(info.w) {}
    void window_present::begin() {}
    void window_present::proccess(context* ctx) { 
//       w->present(); 
      (void)ctx; 
    }
    void window_present::clear() {}
  }
}
