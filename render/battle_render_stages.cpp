#include "battle_render_stages.h"

#include "utils/globals.h"
#include "utils/systems.h"
#include "targets.h"
#include "shared_structures.h"
#include "shared_battle_structures.h"
#include "context.h"
#include "bin/battle_map.h"
#include "image_controller.h"
#include "window.h"

namespace devils_engine {
  namespace render {
    namespace battle {
      tile_optimizer::tile_optimizer(const create_info &info) : device(info.device), set(nullptr), indirect_buffer(nullptr), tiles_indices(nullptr) {
        indirect_buffer = device->create(yavf::BufferCreateInfo::buffer(sizeof(indirect_buffer_data), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
        tiles_indices = device->create(yavf::BufferCreateInfo::buffer(16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT), VMA_MEMORY_USAGE_GPU_ONLY);
        
        yavf::DescriptorSetLayout buffer_layout = VK_NULL_HANDLE;
        {
          yavf::DescriptorLayoutMaker dlm(device);
          buffer_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL)
                             .binding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL)
                             .create("battle_tile_buffer_layout");
        }
        
        yavf::DescriptorPool pool = VK_NULL_HANDLE;
        {
          yavf::DescriptorPoolMaker dpm(device);
          pool = dpm.poolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2).create("battle_tile_buffer_pool");
        }
        
        {
          yavf::DescriptorMaker dm(device);
          set = dm.layout(buffer_layout).create(pool)[0];
        }
        
        set->add({indirect_buffer, 0, indirect_buffer->info().size, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
        set->add({tiles_indices, 0, tiles_indices->info().size, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
        
        set->update();
        
        auto map_layout = device->setLayout(BATTLE_MAP_DESCRIPTOR_SET_LAYOUT_NAME);
        auto uniform_layout = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
        yavf::PipelineLayout pipe_layout = VK_NULL_HANDLE;
        {
          yavf::PipelineLayoutMaker plm(device);
          pipe_layout = plm.addDescriptorLayout(uniform_layout)
                           .addDescriptorLayout(map_layout)
                           .addDescriptorLayout(buffer_layout)
                           .create("battle_tile_optimizer_pipeline_layout");
        }
        
        {
          yavf::raii::ShaderModule compute(device, global::root_directory() + "shaders/map_objects.comp.spv");
          
          yavf::ComputePipelineMaker cpm(device);
          pipe = cpm.shader(compute).create("battle_tile_optimizer_pipeline", pipe_layout);
        }
      }
      
      tile_optimizer::~tile_optimizer() {
        device->destroy(indirect_buffer);
        device->destroy(tiles_indices);
        
        device->destroyLayout("battle_tile_buffer_layout");
        device->destroyDescriptorPool("battle_tile_buffer_pool");
        
        device->destroy(pipe.layout());
        device->destroy(pipe);
      }
      
      void tile_optimizer::begin() {
        auto buffers = global::get<render::buffers>();
        
        const auto &mat = buffers->get_matrix();
        const auto &fru = utils::compute_frustum(mat);
        
        auto indirect_data = reinterpret_cast<indirect_buffer_data*>(indirect_buffer->ptr());
        memcpy(&indirect_data->frustum, &fru, sizeof(utils::frustum));
        
        indirect_data->tiles_indirect.x = 6;
        indirect_data->tiles_indirect.y = 0;
        indirect_data->tiles_indirect.z = 0;
        indirect_data->tiles_indirect.w = 0;
        
        auto battle = global::get<systems::battle_t>();
        indirect_data->sizes_data.x = battle->is_init() ? battle->map->tiles_count : 0;
        indirect_data->sizes_data.y = battle->is_init() ? battle->map->width : 0;
        indirect_data->sizes_data.z = battle->is_init() ? battle->map->height : 0;
      }
      
      void tile_optimizer::proccess(context * ctx) {
        auto battle = global::get<systems::battle_t>();
        if (!battle->is_init()) return;
        auto uniform = global::get<render::buffers>()->uniform;
        auto map = battle->map;
        
        auto task = ctx->compute();
        task->setPipeline(pipe);
        task->setDescriptor({
          uniform->descriptorSet()->handle(), 
          map->set->handle(), 
          set->handle()
        }, 0);
        
        const uint32_t count = std::ceil(float(map->tiles_count) / float(work_group_size));
        task->dispatch(count, 1, 1);
      }
      
      void tile_optimizer::clear() {}
      
      yavf::Buffer* tile_optimizer::get_indirect_buffer() const { return indirect_buffer; }
      yavf::Buffer* tile_optimizer::get_tiles_indices() const { return tiles_indices; }
      
      // это при условии что у нас одна текстурка на стенки
      const uint16_t tile_indices[] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1, UINT16_MAX, 12, 13, 14, 15, 16, 17
      };
      
      tile_render::tile_render(const create_info &info) : device(info.device), opt(info.opt), points_indices(nullptr), images_set(nullptr) {
        static_assert(sizeof(tile_indices) == 21 * sizeof(uint16_t));
        const size_t buffer_size = sizeof(tile_indices);
        points_indices = device->create(yavf::BufferCreateInfo::buffer(buffer_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT), VMA_MEMORY_USAGE_GPU_ONLY);
        {
          yavf::Buffer staging(device, yavf::BufferCreateInfo::buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
          memcpy(staging.ptr(), tile_indices, buffer_size);
          
          auto task = device->allocateTransferTask();
          task->begin();
          task->copy(&staging, points_indices);
          task->end();
          
          task->start();
          task->wait();
          device->deallocate(task);
        }
        
        images_set = global::get<systems::core_t>()->image_controller->set;
        
        auto map_layout = device->setLayout(BATTLE_MAP_DESCRIPTOR_SET_LAYOUT_NAME);
        auto uniform_layout = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
        yavf::PipelineLayout pipe_layout = VK_NULL_HANDLE;
        {
          yavf::PipelineLayoutMaker plm(device);
          pipe_layout = plm.addDescriptorLayout(uniform_layout)
                           .addDescriptorLayout(map_layout)
//                            .addDescriptorLayout(buffer_layout)
                           .create("battle_tile_render_pipeline_layout");
        }
        
        {
          yavf::raii::ShaderModule vertex  (device, global::root_directory() + "shaders/map_objects.comp.spv");
          yavf::raii::ShaderModule fragment(device, global::root_directory() + "shaders/map_objects.comp.spv");
          
          yavf::PipelineMaker pm(device);
          pipe = pm.addShader(VK_SHADER_STAGE_VERTEX_BIT, vertex)
                   .addShader(VK_SHADER_STAGE_FRAGMENT_BIT, fragment)
                   .vertexBinding(0, sizeof(uint32_t), VK_VERTEX_INPUT_RATE_INSTANCE) //sizeof(instance_data_t)
                     .vertexAttribute(0, 0, VK_FORMAT_R32_UINT, 0)
  //                  .vertexBinding(1, sizeof(uint32_t))
  //                    .vertexAttribute(1, 1, VK_FORMAT_R32_UINT, 0)
                   .depthTest(VK_TRUE)
                   .depthWrite(VK_TRUE)
                   .frontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE)
                   .cullMode(VK_CULL_MODE_FRONT_BIT)
                   .assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, VK_TRUE)
                   .viewport()
                   .scissor()
                   .dynamicState(VK_DYNAMIC_STATE_VIEWPORT)
                   .dynamicState(VK_DYNAMIC_STATE_SCISSOR)
                   .addDefaultBlending()
                   .create("battle_tile_render_pipeline", pipe_layout, global::get<render::window>()->render_pass);
        }
      }
      
      tile_render::~tile_render() {
        device->destroy(points_indices);
        
        device->destroy(pipe.layout());
        device->destroy(pipe);
      }
      
    // виндовс не дает использовать базовый offsetof
#define offsetof123(s,m) ((::size_t)&reinterpret_cast<char const volatile&>((((s*)0)->m)))
      
      void tile_render::begin() {}
      void tile_render::proccess(context * ctx) {
        auto battle = global::get<systems::battle_t>();
        if (!battle->is_init()) return;
        auto uniform = global::get<render::buffers>()->uniform;
        auto map = battle->map;
        
        auto indirect_buffer = opt->get_indirect_buffer();
        auto indices_buffer = opt->get_tiles_indices();
        
        auto task = ctx->graphics();
        task->setPipeline(pipe);
        ASSERT(uniform->descriptorSet() != nullptr);
        // забыл создать буферу дескриптор
        ASSERT(map->set != nullptr);
        task->setDescriptor({uniform->descriptorSet()->handle(), images_set->handle(), map->set->handle()}, 0);
        
        task->setIndexBuffer(points_indices, VK_INDEX_TYPE_UINT16); // (!)
        task->setVertexBuffer(indices_buffer, 0);
        task->drawIndexedIndirect(indirect_buffer, 1, offsetof123(struct tile_optimizer::indirect_buffer_data, tiles_indirect));
      }
      
      void tile_render::clear() {}
    }
  }
}
