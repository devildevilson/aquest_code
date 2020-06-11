#include "stages.h"

#include "window.h"
#include "context.h"
#include "shared_structures.h"
#include "utils/globals.h"
#include "utils/utility.h"
#include "targets.h"
#include "bin/interface_context.h"
#include "utils/input.h"

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
    void render_pass_begin::begin() {}
    void render_pass_begin::proccess(context* ctx) {
      auto w = global::get<window>();
      ctx->graphics()->setRenderTarget(w);
      ctx->graphics()->beginRenderPass();
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
    
    const size_t max_tiles_count = 500000;
    tile_optimizer::tile_optimizer(const create_info &info) : 
      device(info.device)
    {
      indirect = device->create(yavf::BufferCreateInfo::buffer(sizeof(struct indirect_buffer), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
      tiles_indices = device->create(yavf::BufferCreateInfo::buffer(sizeof(instance_data_t)*max_tiles_count, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT), VMA_MEMORY_USAGE_GPU_ONLY);
      
      auto pool = device->descriptorPool(DEFAULT_DESCRIPTOR_POOL_NAME);
//       auto storage_layout = device->setLayout(STORAGE_BUFFER_LAYOUT_NAME);
      auto tiles_data_layout = device->setLayout(TILES_DATA_LAYOUT_NAME);
      auto uniform_layout = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
      
      ASSERT(tiles_data_layout != VK_NULL_HANDLE);
      
      yavf::DescriptorSetLayout tiles_indices_layout = VK_NULL_HANDLE;
      {
        yavf::DescriptorLayoutMaker dlm(device);
        tiles_indices_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
                                  .binding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
                                  .create("tiles_indices_layout");
      }
      
      {
        yavf::DescriptorMaker dm(device);
        auto desc = dm.layout(tiles_indices_layout).create(pool)[0];
        size_t index = desc->add({indirect, 0, indirect->info().size, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
                       desc->add({tiles_indices, 0, tiles_indices->info().size, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
        desc->update();
        indirect->setDescriptor(desc, index);
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
    
    tile_optimizer::~tile_optimizer() {}
    void tile_optimizer::begin() {
      auto buffer = reinterpret_cast<struct indirect_buffer*>(indirect->ptr());
      
//       PRINT_VAR("pentagon_command.vertexCount  ", buffer->pentagon_command.vertexCount)
//       PRINT_VAR("pentagon_command.instanceCount", buffer->pentagon_command.instanceCount)
//       PRINT_VAR("pentagon_command.firstVertex  ", buffer->pentagon_command.firstVertex)
//       PRINT_VAR("pentagon_command.firstInstance", buffer->pentagon_command.firstInstance)
//       PRINT_VAR("hexagon_command.instanceCount ", buffer->hexagon_command.instanceCount)
      
      auto uniform = global::get<render::buffers>()->uniform;
      auto camera_data = reinterpret_cast<render::camera_data*>(uniform->ptr());
      const auto fru = utils::compute_frustum(camera_data->viewproj);
      
      buffer->pentagon_command.vertexCount = 5;
      buffer->pentagon_command.instanceCount = 0;
      buffer->pentagon_command.firstVertex = 0;
      buffer->pentagon_command.firstInstance = 0;
      
      buffer->hexagon_command.vertexCount = 6;
      buffer->hexagon_command.instanceCount = 0;
      buffer->hexagon_command.firstVertex = 0;
      buffer->hexagon_command.firstInstance = 0;
      
      buffer->data.x = core::map::tri_count_d(core::map::accel_struct_detail_level);
      buffer->data.y = max_tiles_count;
      buffer->data.z = 0;
      buffer->data.w = 0;
      
      memcpy(&buffer->frustum, &fru, sizeof(utils::frustum));
    }
    
    void tile_optimizer::proccess(context* ctx) {
      auto uniform = global::get<render::buffers>()->uniform;
      auto tiles = global::get<core::map>()->tiles;
//       auto points = global::get<render::buffers>()->points;
      
      auto task = ctx->compute();
      task->setPipeline(pipe);
      task->setDescriptor({
        uniform->descriptorSet()->handle(), 
        tiles->descriptorSet()->handle(), 
//         points->descriptorSet()->handle(), 
        indirect->descriptorSet()->handle()
//         tiles_indices->descriptorSet()->handle()
      }, 0);
      
      const uint32_t count = std::ceil(float(core::map::tri_count_d(core::map::accel_struct_detail_level)) / float(work_group_size));
      task->dispatch(count, 1, 1);
      
      task->setBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
                       VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_INDIRECT_COMMAND_READ_BIT);
      
      task->setBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                       VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
    }
    
    void tile_optimizer::clear() {}
    
    yavf::Buffer* tile_optimizer::indirect_buffer() const {
      return indirect;
    }
    
    yavf::Buffer* tile_optimizer::instances_buffer() const {
      return tiles_indices;
    }

    const uint32_t max_tiles = 500000;
    tile_render::tile_render(const create_info &info) : 
      device(info.device), 
      opt(info.opt)
//       default_render_pass(VK_NULL_HANDLE), 
//       hexagons_count(0), 
//       pentagons_count(0) 
    {
      //indices = device->create(yavf::BufferCreateInfo::buffer(sizeof(uint32_t)*max_tiles, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
      points_indices = device->create(yavf::BufferCreateInfo::buffer(sizeof(uint32_t)*(5+6), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
      
      {
        auto points_ptr = reinterpret_cast<uint32_t*>(points_indices->ptr());
        for (uint32_t i = 0; i < 5; ++i) {
          points_ptr[i] = i;
        }
        
        for (uint32_t i = 0; i < 6; ++i) {
          points_ptr[i+5] = i;
        }
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

      yavf::PipelineLayout layout = VK_NULL_HANDLE;
      {
        yavf::PipelineLayoutMaker plm(device);
        layout = plm.addDescriptorLayout(uniform_layout)
                    .addDescriptorLayout(tiles_data_layout)
                    .create(TILE_RENDER_PIPELINE_LAYOUT_NAME);
      }

      {
        yavf::raii::ShaderModule vertex  (device, global::root_directory() + "shaders/tiles.vert.spv");
        //yavf::raii::ShaderModule geom    (device, global::root_directory() + "shaders/tiles.geom.spv"); // nexus 5x не поддерживает геометрический шейдер
        yavf::raii::ShaderModule fragment(device, global::root_directory() + "shaders/tiles.frag.spv");

        yavf::PipelineMaker pm(device);
        pm.clearBlending();

        pipe = pm.addShader(VK_SHADER_STAGE_VERTEX_BIT, vertex)
                 .addShader(VK_SHADER_STAGE_FRAGMENT_BIT, fragment)
                 .vertexBinding(0, sizeof(instance_data_t), VK_VERTEX_INPUT_RATE_INSTANCE)
                   .vertexAttribute(0, 0, VK_FORMAT_R32_UINT, 0)
                 .vertexBinding(1, sizeof(uint32_t))
                   .vertexAttribute(1, 1, VK_FORMAT_R32_UINT, 0)
                 .depthTest(VK_TRUE)
                 .depthWrite(VK_TRUE)
                 .frontFace(VK_FRONT_FACE_CLOCKWISE)
                 .cullMode(VK_CULL_MODE_BACK_BIT)
                 .assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN)
                 .viewport()
                 .scissor()
                 .dynamicState(VK_DYNAMIC_STATE_VIEWPORT)
                 .dynamicState(VK_DYNAMIC_STATE_SCISSOR)
                 .colorBlendBegin(VK_FALSE)
                   .colorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
                 .create(TILE_RENDER_PIPELINE_NAME, layout, global::get<render::window>()->render_pass);
      }

      // {
      //   yavf::DescriptorMaker dm(device);
      //   auto desc = dm.layout(storage_layout).create(pool)[0];
      //   size_t i = desc->add({indices, 0, indices->info().size, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
      //   indices->setDescriptor(desc, i);
      // }
    }

    tile_render::~tile_render() {}
    void tile_render::begin() {}
    void tile_render::proccess(context* ctx) {
//       if (hexagons_count + pentagons_count == 0) return;
      
      // все таки мне лучше рисовать треугольниками из центра
      // так как мне нужно нарисовать границы с другими биомами
      // у самого тайла должно быть как можно меньше данных
      // скорее всего только индекс биома
      // не только, нужно крайне аккуратно работать с 2кк тайлами
      
      auto uniform = global::get<render::buffers>()->uniform;
      auto tiles = global::get<core::map>()->tiles;
//       auto points = global::get<render::buffers>()->points;
//       auto biomes = global::get<render::buffers>()->biomes;
      
      auto indirect_buffer = opt->indirect_buffer();
      auto instances_buffer = opt->instances_buffer();
      
      auto task = ctx->graphics();
      task->setPipeline(pipe);
      task->setDescriptor({uniform->descriptorSet()->handle(), tiles->descriptorSet()->handle()}, 0);
      
//       task->setVertexBuffer(indices, 0);
//       task->setVertexBuffer(points_indices, 1);
//       task->draw(5, pentagons_count, 0, 0); // пентагоны отдельно посчитаем
//       
//       task->setVertexBuffer(indices, 0, sizeof(uint32_t)*12);
//       task->setVertexBuffer(points_indices, 1, sizeof(uint32_t)*5);
//       task->draw(6, hexagons_count, 0, 0);
      
      task->setVertexBuffer(instances_buffer, 0);
      task->setVertexBuffer(points_indices, 1);
      task->drawIndirect(indirect_buffer, 1, offsetof(struct tile_optimizer::indirect_buffer, pentagon_command));
//       task->draw(5, 6, 0, 0);
      
      task->setVertexBuffer(instances_buffer, 0, sizeof(uint32_t)*12);
      task->setVertexBuffer(points_indices, 1, sizeof(uint32_t)*5);
      task->drawIndirect(indirect_buffer, 1, offsetof(struct tile_optimizer::indirect_buffer, hexagon_command));
//       task->draw(6, 250000, 0, 0);

//       for (uint32_t i = 0; i < indices_count; ++i) {
//         const uint32_t start = i;
//         task->draw(1, 1, start, 0);
//       }
    }

    void tile_render::clear() {
//       pentagons_count = 0;
//       hexagons_count = 0;
//       unique_indices.clear();
    }

    void tile_render::recreate_pipelines(const game::image_resources_t* resource) {
      (void)resource;
    }
    
    yavf::Buffer* tile_render::vertex_indices() const {
      return points_indices;
    }
    
    void tile_render::add(const uint32_t &tile_index) {
      (void)tile_index;
// //       const uint32_t index = indices_count.fetch_add(1);
// //       ASSERT(index < max_tiles);
// //       auto idx = reinterpret_cast<uint32_t*>(indices->ptr());
// //       idx[index] = tile_index;
//       
//       // мьютекс слишком дорогое удовольствие по всей видимости
// //       {
// //         std::unique_lock<std::mutex> lock(mutex);
// //         auto itr = unique_indices.find(tile_index);
// //         if (itr != unique_indices.end()) return;
// //         unique_indices.insert(tile_index);
// //       }
//       
//       auto tiles = global::get<render::buffers>()->tiles;
//       auto tiles_arr = reinterpret_cast<light_map_tile_t*>(tiles->ptr());
//       auto idx = reinterpret_cast<uint32_t*>(indices->ptr());
//       const auto &current_tile = tiles_arr[tile_index];
//       //const uint32_t n_count = current_tile.is_pentagon() ? 5 : 6;
//       
//       ASSERT(tile_index != UINT32_MAX);
//       
//       if (is_pentagon(current_tile)) {
//         const uint32_t index_offset = pentagons_count.fetch_add(1);
//         idx[index_offset] = tile_index;
//         //ASSERT(pentagons_count <= 12);
//       } else {
//         const uint32_t index_offset = hexagons_count.fetch_add(1);
//         idx[index_offset+12] = tile_index;
//       }
//       
// //       const uint32_t index_offset = indices_count.fetch_add(n_count+1);
// //       ASSERT(index_offset < max_tiles);
// //       idx[index_offset] = current_tile.neighbours[0].points[0];
// //       for (uint32_t i = 1; i < n_count; ++i) {
// //         idx[index_offset+i] = current_tile.neighbours[i].points[0];
// //       }
// //       
// //       idx[index_offset+n_count] = UINT32_MAX;
//       
//       // тут наверное нужно завести буфер для тайлов или текстурок
//       // текстурные координаты мы можем расчитать из мировых позиций
//       // приводим
    }

    void tile_render::create_render_pass() {
//       yavf::RenderPassMaker rpm(device);

      // default_render_pass = rpm.attachmentBegin(surface.format.format)
      //                            .attachmentLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
      //                            .attachmentStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
      //                            .attachmentInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
      //                            .attachmentFinalLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
      //                          .attachmentBegin(swapchain.depths[0]->info().format)
      //                            .attachmentLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
      //                            .attachmentStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE)
      //                            .attachmentInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
      //                            .attachmentFinalLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
      //                          .subpassBegin(VK_PIPELINE_BIND_POINT_GRAPHICS)
      //                            .subpassColorAttachment(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0)
      //                            .subpassDepthStencilAttachment(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1)
      //                          .dependencyBegin(VK_SUBPASS_EXTERNAL, 0)
      //                            .dependencySrcStageMask(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT)
      //                            .dependencyDstStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
      //                            .dependencySrcAccessMask(VK_ACCESS_MEMORY_READ_BIT)
      //                            .dependencyDstAccessMask(VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
      //                          .dependencyBegin(0, VK_SUBPASS_EXTERNAL)
      //                            .dependencySrcStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
      //                            .dependencyDstStageMask(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT)
      //                            .dependencySrcAccessMask(VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
      //                            .dependencyDstAccessMask(VK_ACCESS_MEMORY_READ_BIT)
      //                          .create("tiles_render_pass");
    }
    
    tile_border_render::tile_border_render(const create_info &info) :
      device(info.device),
      opt(info.opt),
      render(info.render)
    {
      auto uniform_layout = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
      auto tiles_data_layout = device->setLayout(TILES_DATA_LAYOUT_NAME);

      yavf::PipelineLayout layout = device->layout(TILE_RENDER_PIPELINE_LAYOUT_NAME);
      if (layout == VK_NULL_HANDLE) {
        yavf::PipelineLayoutMaker plm(device);
        layout = plm.addDescriptorLayout(uniform_layout)
                    .addDescriptorLayout(tiles_data_layout)
                    .create(TILE_RENDER_PIPELINE_LAYOUT_NAME);
      }
      
      glm::vec3 color = glm::vec3(0.0f, 0.0f, 0.0f);
      const std::string name = "tile_borders_render_pipeline";

      {
        yavf::raii::ShaderModule vertex  (device, global::root_directory() + "shaders/tiles.vert.spv");
        yavf::raii::ShaderModule fragment(device, global::root_directory() + "shaders/tiles_borders.frag.spv");

        yavf::PipelineMaker pm(device);
        pm.clearBlending();

        pipe = pm.addShader(VK_SHADER_STAGE_VERTEX_BIT, vertex)
                 .addShader(VK_SHADER_STAGE_FRAGMENT_BIT, fragment)
                 .addSpecializationEntry(0, offsetof(glm::vec3, x), sizeof(color.x))
                 .addSpecializationEntry(1, offsetof(glm::vec3, y), sizeof(color.y))
                 .addSpecializationEntry(2, offsetof(glm::vec3, z), sizeof(color.z))
                   .addData(sizeof(glm::vec3), &color)
                 .vertexBinding(0, sizeof(instance_data_t), VK_VERTEX_INPUT_RATE_INSTANCE)
                   .vertexAttribute(0, 0, VK_FORMAT_R32_UINT, 0)
                 .vertexBinding(1, sizeof(uint32_t))
                   .vertexAttribute(1, 1, VK_FORMAT_R32_UINT, 0)
                 .depthTest(VK_TRUE)
                 .depthWrite(VK_TRUE)
                 .frontFace(VK_FRONT_FACE_CLOCKWISE)
                 .cullMode(VK_CULL_MODE_BACK_BIT)
                 .assembly(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP)
                 .lineWidth(5.0f)
                 .viewport()
                 .scissor()
                 .dynamicState(VK_DYNAMIC_STATE_VIEWPORT)
                 .dynamicState(VK_DYNAMIC_STATE_SCISSOR)
                 .colorBlendBegin(VK_FALSE)
                   .colorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
                 .create(name, layout, global::get<render::window>()->render_pass);
      }
    }
    
    tile_border_render::~tile_border_render() {}

    void tile_border_render::begin() {}
    void tile_border_render::proccess(context* ctx) {
      auto uniform = global::get<render::buffers>()->uniform;
      auto tiles = global::get<core::map>()->tiles;
      
      auto indirect_buffer = opt->indirect_buffer();
      auto instances_buffer = opt->instances_buffer();
      
      auto points_indices = render->vertex_indices();
      
      auto task = ctx->graphics();
      task->setPipeline(pipe);
      task->setDescriptor({uniform->descriptorSet()->handle(), tiles->descriptorSet()->handle()}, 0);
      
      task->setVertexBuffer(instances_buffer, 0);
      task->setVertexBuffer(points_indices, 1);
      task->drawIndirect(indirect_buffer, 1, offsetof(struct tile_optimizer::indirect_buffer, pentagon_command));
      
      task->setVertexBuffer(instances_buffer, 0, sizeof(uint32_t)*12);
      task->setVertexBuffer(points_indices, 1, sizeof(uint32_t)*5);
      task->drawIndirect(indirect_buffer, 1, offsetof(struct tile_optimizer::indirect_buffer, hexagon_command));
    }
    
    void tile_border_render::clear() {}
    void tile_border_render::recreate_pipelines(const game::image_resources_t* resource) { (void)resource; }
    
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
      
      yavf::DescriptorSetLayout sampled_image_layout = device->setLayout(SAMPLED_IMAGE_LAYOUT_NAME);
      yavf::DescriptorSetLayout uniform_layout       = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
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
                        .addDescriptorLayout(sampled_image_layout)
//                         .addDescriptorLayout(resource->layout)
                        .addPushConstRange(0, sizeof(glm::vec2) + sizeof(glm::vec2))
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
          NK_ANTI_ALIASING_ON,
          NK_ANTI_ALIASING_ON,
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
        nk_buffer_init_fixed(&vbuf, vertices, size_t(MAX_VERTEX_BUFFER));
        nk_buffer_init_fixed(&ebuf, elements, size_t(MAX_INDEX_BUFFER));
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
      
      yavf::ImageView* tex = data->view;
      const std::vector<VkDescriptorSet> sets = {matrix.descriptorSet()->handle(), tex->descriptorSet()->handle()}; // , image_set->handle()
      task->setDescriptor(sets, 0);
      
      uint32_t index_offset = 0;
      const nk_draw_command *cmd = nullptr;
      nk_draw_foreach(cmd, &data->ctx, &data->cmds) {
        if (cmd->elem_count == 0) continue;
        
        const render::image_t i = image_nk_handle(cmd->texture);
        //ASSERT(i.index == UINT32_MAX && i.layer == UINT32_MAX);
        auto data = glm::uvec4(i.container, 0, 0, 0);
    //     PRINT_VEC2("image id", data)
        task->setConsts(0, sizeof(data), &data);

        const glm::vec2 fb_scale = global::get<input::data>()->fb_scale;
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
    void task_start::wait() {
      const VkResult res = vkWaitForFences(device->handle(), 1, &wait_fence.fence, VK_TRUE, 1000000000);
      if (res != VK_SUCCESS) {
        throw std::runtime_error("drawing takes too long");
      }
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
