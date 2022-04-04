#include "targets.h"

#include "shared_structures.h"
#include "bin/core_structures.h"
#include "container.h"
#include "makers.h"
#include "core/map.h"

#include <cstring>
#include <iostream>

namespace devils_engine {
  namespace render {
    buffers::buffers(container* c) : c(c), uniform_camera(nullptr), uniform_matrices(nullptr), uniform_common(nullptr) {
      auto device = c->vulkan->device;
      auto allocator = c->vulkan->buffer_allocator;
      const size_t uniform_buffer_alignment = c->limits()->minUniformBufferOffsetAlignment;
      const size_t storage_buffer_alignment = c->limits()->minStorageBufferOffsetAlignment;
      const size_t buffer1_size = align_to(sizeof(camera_data),   uniform_buffer_alignment);
      const size_t buffer2_size = align_to(sizeof(matrices_data), uniform_buffer_alignment);
      const size_t buffer3_size = align_to(sizeof(common_data),   uniform_buffer_alignment);
       uniform.create(allocator, buffer(buffer1_size + buffer2_size + buffer3_size, vk::BufferUsageFlagBits::eUniformBuffer), vma::MemoryUsage::eCpuOnly, "uniform buffer");
//       matrices.create(allocator, buffer(buffer2_size, vk::BufferUsageFlagBits::eUniformBuffer), vma::MemoryUsage::eCpuOnly, "matrices buffer");
//         common.create(allocator, buffer(buffer3_size, vk::BufferUsageFlagBits::eUniformBuffer), vma::MemoryUsage::eCpuOnly, "common buffer");
       heraldy.create(allocator, buffer(align_to(16, storage_buffer_alignment), vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst), vma::MemoryUsage::eGpuOnly, "heraldy buffer");
      heraldy_indices.create(allocator, buffer(align_to(16, storage_buffer_alignment), vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst), vma::MemoryUsage::eGpuOnly, "heraldy indices buffer");
      
      assert(uniform.ptr != nullptr);
//       assert(matrices.ptr != nullptr);
      memset(uniform.ptr, 0, buffer1_size + buffer2_size + buffer3_size);
      
      auto pool = c->vulkan->descriptor_pool;
//       auto storage_layout = c->vulkan->storage_layout;
      auto uniform_layout = c->vulkan->uniform_layout;
      
      {
        descriptor_set_maker dm(&device);
        auto desc = dm.layout(uniform_layout).create(pool, "uniform layout")[0];
        uniform_set = desc;
        
        descriptor_set_updater dsu(&device);
        dsu.currentSet(uniform_set);
        dsu.begin(0, 0, vk::DescriptorType::eUniformBuffer).buffer(uniform.handle, 0, buffer1_size);
        dsu.begin(1, 0, vk::DescriptorType::eUniformBuffer).buffer(uniform.handle, buffer1_size, buffer2_size);
        dsu.begin(2, 0, vk::DescriptorType::eUniformBuffer).buffer(uniform.handle, buffer1_size + buffer2_size, buffer3_size);
        dsu.update();
        
        auto tmp1 = reinterpret_cast<char*>(uniform.ptr);
        uniform_camera   = &tmp1[0];
        uniform_matrices = &tmp1[buffer1_size];
        uniform_common   = &tmp1[buffer1_size + buffer2_size];
      }
      
      {
        descriptor_set_layout_maker dslm(&device);
        heraldy_layout = dslm.binding(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eFragment)
                             .binding(1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eFragment)
                             .create("heraldy buffers layout");
        
        descriptor_set_maker dm(&device);
        heraldy_set = dm.layout(heraldy_layout).create(pool, "heraldy buffer descriptor")[0];
        
        descriptor_set_updater dsu(&device);
        dsu.currentSet(heraldy_set)
           .begin(0, 0, vk::DescriptorType::eStorageBuffer).buffer(heraldy.handle)
           .begin(1, 0, vk::DescriptorType::eStorageBuffer).buffer(heraldy_indices.handle)
           .update();
           
        // нужно видимо здесь держать буфер
      }
    }

    buffers::~buffers() {
      auto device = c->vulkan->device;
      auto allocator = c->vulkan->buffer_allocator;
      uniform.destroy(allocator);
      //matrices.destroy(allocator);
      heraldy_indices.destroy(allocator);
      heraldy.destroy(allocator);
      device.destroy(heraldy_layout);
    }

//     void buffers::update_matrix(const glm::mat4 &matrix) {
//       auto camera = reinterpret_cast<camera_data*>(uniform->ptr());
//       camera->viewproj = matrix;
//     }
    
    void buffers::update_projection_matrix(const glm::mat4 &matrix) {
//       auto camera = reinterpret_cast<camera_data*>(uniform->ptr());
      auto mat = reinterpret_cast<matrices_data*>(uniform_matrices);
      
      mat->proj = matrix;
      mat->invProj = glm::inverse(matrix);
    }
    
    void buffers::update_view_matrix(const glm::mat4 &matrix) {
      auto camera = reinterpret_cast<camera_data*>(uniform_camera);
      auto mat = reinterpret_cast<matrices_data*>(uniform_matrices);
      
      mat->view = matrix;
      mat->invView = glm::inverse(matrix);
      camera->viewproj = mat->proj * mat->view;
      mat->invViewProj = glm::inverse(camera->viewproj);
    }
    
    void buffers::update_pos(const glm::vec3 &pos) {
      auto camera = reinterpret_cast<camera_data*>(uniform_camera);
      camera->pos = glm::vec4(pos, 1.0f);
    }
    
    void buffers::update_dir(const glm::vec3 &dir) {
      auto camera = reinterpret_cast<camera_data*>(uniform_camera);
      camera->dir = glm::vec4(dir, 0.0f);
    }
    
    void buffers::update_zoom(const float &zoom) {
      auto c = reinterpret_cast<common_data*>(uniform_common);
      c->dim[2] = glm::floatBitsToUint(zoom);
    }
    
    void buffers::update_cursor_dir(const glm::vec4 &cursor_dir) {
//       auto camera = reinterpret_cast<camera_data*>(uniform.ptr);
//       camera->cursor_dir = cursor_dir;
      auto c = reinterpret_cast<common_data*>(uniform_common);
      c->cursor_dir = cursor_dir;
    }
    
    void buffers::update_dimensions(const uint32_t &width, const uint32_t &height) {
      auto c = reinterpret_cast<common_data*>(uniform_common);
      c->dim.x = width;
      c->dim.y = height;
    }
    
    void buffers::update_time(const uint32_t &time) {
      auto c = reinterpret_cast<common_data*>(uniform_common);
      c->dim.w += time;
    }
    
    void buffers::update_persistent_state(const uint32_t &state) {
      auto c = reinterpret_cast<common_data*>(uniform_common);
      c->state.x = state;
    }
    
    void buffers::update_application_state(const uint32_t &state) {
      auto c = reinterpret_cast<common_data*>(uniform_common);
      c->state.y = state;
    }
    
    void buffers::update_turn_state(const uint32_t &state) {
      auto c = reinterpret_cast<common_data*>(uniform_common);
      c->state.z = state;
    }

    void buffers::recreate(const uint32_t &width, const uint32_t &height) {
      const glm::mat4 persp = glm::perspective(glm::radians(75.0f), float(width) / float(height), 0.1f, 256.0f);
      update_projection_matrix(persp);
      auto c = reinterpret_cast<common_data*>(uniform_common);
      c->dim[0] = width;
      c->dim[1] = height;
    }
    
    glm::mat4 buffers::get_matrix() const {
      auto camera = reinterpret_cast<camera_data*>(uniform_camera);
      return camera->viewproj;
    }
    
    glm::mat4 buffers::get_proj() const {
      auto mat = reinterpret_cast<matrices_data*>(uniform_matrices);
      return mat->proj;
    }
    
    glm::mat4 buffers::get_view() const {
      auto mat = reinterpret_cast<matrices_data*>(uniform_matrices);
      return mat->view;
    }
    
    glm::mat4 buffers::get_inv_proj() const {
      auto mat = reinterpret_cast<matrices_data*>(uniform_matrices);
      return mat->invProj;
    }
    
    glm::mat4 buffers::get_inv_view() const {
      auto mat = reinterpret_cast<matrices_data*>(uniform_matrices);
      return mat->invView;
    }
    
    glm::mat4 buffers::get_inv_view_proj() const {
      auto mat = reinterpret_cast<matrices_data*>(uniform_matrices);
      return mat->invViewProj;
    }
    
    glm::vec4 buffers::get_pos() const {
      auto camera = reinterpret_cast<camera_data*>(uniform_camera);
      return camera->pos;
    }
    
    glm::vec4 buffers::get_dir() const {
      auto camera = reinterpret_cast<camera_data*>(uniform_camera);
      return camera->dir;
    }
    
    glm::vec4 buffers::get_cursor_dir() const {
      auto camera = reinterpret_cast<common_data*>(uniform_common);
      return camera->cursor_dir;
    }
    
    float buffers::get_zoom() const {
      auto camera = reinterpret_cast<common_data*>(uniform_common);
      return glm::uintBitsToFloat(camera->dim[2]);
    }
    
    // неудачное название, тут приходит размер
    void buffers::resize_heraldy_buffer(const size_t &heraldy_layers_buffer_size) {
      auto allocator = c->vulkan->buffer_allocator;
      heraldy.destroy(allocator);
      const size_t final_size = align_to(heraldy_layers_buffer_size, c->limits()->minStorageBufferOffsetAlignment);
      heraldy.create(allocator, buffer(final_size, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst), vma::MemoryUsage::eGpuOnly, "heraldy buffer");
      
      auto device = c->vulkan->device;
      descriptor_set_updater dsu(&device);
      dsu.currentSet(heraldy_set).begin(0, 0, vk::DescriptorType::eStorageBuffer).buffer(heraldy.handle).update();
    }
    
//     void buffers::resize_heraldy_indices_buffer(const size_t &heraldy_indices_size, const size_t &heraldy_indices_count) {
//       assert(current_indices_size == 0);
//       auto allocator = c->vulkan->buffer_allocator;
//       heraldy_indices.destroy(allocator);
//       heraldy_indices.create(allocator, buffer(heraldy_indices_size, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst), vma::MemoryUsage::eGpuOnly, "heraldy buffer");
//       current_indices_size = heraldy_indices_count;
//       
//       auto device = c->vulkan->device;
//       descriptor_set_updater dsu(&device);
//       dsu.currentSet(heraldy_set).begin(1, 0, vk::DescriptorType::eStorageBuffer).buffer(heraldy_indices.handle).update();
//     }
    
#define WORLD_MAP_DESCRIPTOR_POOL "world_map_descriptor_pool"

    // как раполагается в памяти? рендер, експлор, визибл
#define RENDER_STAT_OFFSET_MULT 0
#define EXPLORED_STAT_OFFSET_MULT 1
#define VISIBILITY_STAT_OFFSET_MULT 2
    
    world_map_buffers::world_map_buffers(container* c) : c(c) {
      auto device = c->vulkan->device;
//       auto storage_layout = c->vulkan->storage_layout;
      auto allocator = c->vulkan->buffer_allocator;
      const size_t storage_buffer_alignment = std::max(c->limits()->minStorageBufferOffsetAlignment, size_t(16));
      border_buffer.create(allocator, buffer(align_to(sizeof(glm::vec4)*4, storage_buffer_alignment), vk::BufferUsageFlagBits::eStorageBuffer), vma::MemoryUsage::eCpuOnly);
      border_types.create(allocator, buffer(align_to(sizeof(glm::vec4)*4, storage_buffer_alignment), vk::BufferUsageFlagBits::eStorageBuffer), vma::MemoryUsage::eCpuOnly);
      
      // один буффер? с точки зрения шейдера ничего не изменится
      const size_t tile_stat_buffer_size = align_to(ceil(double(core::map::hex_count_d(core::map::detail_level)) / double(UINT32_WIDTH)), storage_buffer_alignment);
      tiles_renderable.create(
        allocator, 
        buffer(tile_stat_buffer_size * 3, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc), 
        vma::MemoryUsage::eCpuOnly
      );
      gpu_tiles_renderable.create(
        allocator, 
        buffer(tile_stat_buffer_size * 3, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc), 
        vma::MemoryUsage::eGpuOnly
      );
      PRINT_VAR("tile_stat_buffer_size", tile_stat_buffer_size)
      memset(tiles_renderable.ptr, 0, tile_stat_buffer_size * 3);
      
      {
        descriptor_pool_maker dpm(&device);
        pool = dpm.poolSize(vk::DescriptorType::eStorageBuffer, 5).create(WORLD_MAP_DESCRIPTOR_POOL);
      }
      
      {
        descriptor_set_layout_maker dslm(&device);
        tiles_rendering_data_layout = dslm.binding(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eAll)
                                          .binding(1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eAll)
                                          .binding(2, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eAll)
                                          .create("tiles_rendering_data_layout");
      }
      
      {
        descriptor_set_layout_maker dslm(&device);
        borders_data_layout = dslm.binding(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eAll)
                                  .binding(1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eAll)
                                  .create("borders_data_layout");
      }
      
      {
        descriptor_set_maker dsm(&device);
        border_set = dsm.layout(borders_data_layout).create(pool, "border buffer set")[0];
        
        descriptor_set_updater dsu(&device);
        dsu.currentSet(border_set).begin(0, 0, vk::DescriptorType::eStorageBuffer).buffer(border_buffer.handle).update();
        dsu.currentSet(border_set).begin(1, 0, vk::DescriptorType::eStorageBuffer).buffer(border_types.handle).update();
      }
      
//       {
//         descriptor_set_maker dsm(&device);
//         types_set = dsm.layout(storage_layout).create(pool, "border types set")[0];
//         
//         descriptor_set_updater dsu(&device);
//         dsu.currentSet(types_set).begin(0, 0, vk::DescriptorType::eStorageBuffer).buffer(border_types.handle).update();
//       }
      
      {
        descriptor_set_maker dsm(&device);
        tiles_rendering_data = dsm.layout(tiles_rendering_data_layout).create(pool, "tiles_rendering_data")[0];
        descriptor_set_updater dsu(&device);
        dsu.currentSet(tiles_rendering_data)
           .begin(0, 0, vk::DescriptorType::eStorageBuffer).buffer(gpu_tiles_renderable.handle, tile_stat_buffer_size * RENDER_STAT_OFFSET_MULT, tile_stat_buffer_size)
           .begin(1, 0, vk::DescriptorType::eStorageBuffer).buffer(gpu_tiles_renderable.handle, tile_stat_buffer_size * EXPLORED_STAT_OFFSET_MULT, tile_stat_buffer_size)
           .begin(2, 0, vk::DescriptorType::eStorageBuffer).buffer(gpu_tiles_renderable.handle, tile_stat_buffer_size * VISIBILITY_STAT_OFFSET_MULT, tile_stat_buffer_size)
           .update();
      }
    }
    
    world_map_buffers::~world_map_buffers() {
      auto allocator = c->vulkan->buffer_allocator;
      auto device = c->vulkan->device;
      border_buffer.destroy(allocator);
      border_types.destroy(allocator);
      tiles_renderable.destroy(allocator);
      gpu_tiles_renderable.destroy(allocator);
      device.destroy(borders_data_layout);
      device.destroy(tiles_rendering_data_layout);
      device.destroy(pool);
    }
    
    void world_map_buffers::recreate(const uint32_t &width, const uint32_t &height) {
      (void)width;
      (void)height;
    }
    
    void world_map_buffers::copy(resource_provider*, vk::CommandBuffer task) const {
      const size_t storage_buffer_alignment = std::max(c->limits()->minStorageBufferOffsetAlignment, size_t(16));
      const size_t tile_stat_buffer_size = align_to(ceil(double(core::map::hex_count_d(core::map::detail_level)) / double(UINT32_WIDTH)), storage_buffer_alignment);
      const size_t whole_size = tile_stat_buffer_size * 3;
      const size_t render_offset = tile_stat_buffer_size * RENDER_STAT_OFFSET_MULT;
      const size_t next_offset = tile_stat_buffer_size * (RENDER_STAT_OFFSET_MULT+1);
      const vk::BufferCopy c1{render_offset, render_offset, tile_stat_buffer_size};
      const vk::BufferCopy c2{next_offset, next_offset, whole_size - next_offset};
//       auto cb = ctx->command_buffer();
      task.copyBuffer(gpu_tiles_renderable.handle, tiles_renderable.handle, c1);
      task.copyBuffer(tiles_renderable.handle, gpu_tiles_renderable.handle, c2);
      task.fillBuffer(gpu_tiles_renderable.handle, render_offset, tile_stat_buffer_size, 0);
    }
    
    void world_map_buffers::resize_border_buffer(const size_t &size) {
      auto device = c->vulkan->device;
      auto allocator = c->vulkan->buffer_allocator;
      border_buffer.destroy(allocator);
      border_buffer.create(allocator, buffer(size, vk::BufferUsageFlagBits::eStorageBuffer), vma::MemoryUsage::eCpuOnly, "map border buffer");
      
      descriptor_set_updater dsu(&device);
      dsu.currentSet(border_set).begin(0, 0, vk::DescriptorType::eStorageBuffer).buffer(border_buffer.handle).update();
    }
    
    void world_map_buffers::resize_border_types(const size_t &size) {
      auto device = c->vulkan->device;
      auto allocator = c->vulkan->buffer_allocator;
      border_types.destroy(allocator);
      border_types.create(allocator, buffer(size, vk::BufferUsageFlagBits::eStorageBuffer), vma::MemoryUsage::eCpuOnly, "map border types");
      
      descriptor_set_updater dsu(&device);
      dsu.currentSet(border_set).begin(1, 0, vk::DescriptorType::eStorageBuffer).buffer(border_types.handle).update();
    }
    
    void world_map_buffers::set_map_exploration(const uint32_t &tile_index, const bool explored) {
      assert(tile_index < core::map::hex_count_d(core::map::detail_level));
      const size_t storage_buffer_alignment = std::max(c->limits()->minStorageBufferOffsetAlignment, size_t(16));
      const size_t tile_stat_buffer_size = align_to(ceil(double(core::map::hex_count_d(core::map::detail_level)) / double(UINT32_WIDTH)), storage_buffer_alignment);
      auto tmp_ptr = reinterpret_cast<char*>(tiles_renderable.ptr);
      auto offset_ptr = reinterpret_cast<uint32_t*>(&tmp_ptr[tile_stat_buffer_size * EXPLORED_STAT_OFFSET_MULT]);
      const uint32_t index_array = tile_index / UINT32_WIDTH;
      const uint32_t index_bit = tile_index % UINT32_WIDTH;
      const uint32_t mask = explored ? (1 << index_bit) : ~(1 << index_bit);
      // атомарность? желательно
      offset_ptr[index_array] = explored ? offset_ptr[index_array] | mask : offset_ptr[index_array] & mask;
    }
    
    void world_map_buffers::set_map_visibility(const uint32_t &tile_index, const bool visible) {
      assert(tile_index < core::map::hex_count_d(core::map::detail_level));
      const size_t storage_buffer_alignment = std::max(c->limits()->minStorageBufferOffsetAlignment, size_t(16));
      const size_t tile_stat_buffer_size = align_to(ceil(double(core::map::hex_count_d(core::map::detail_level)) / double(UINT32_WIDTH)), storage_buffer_alignment);
      auto tmp_ptr = reinterpret_cast<char*>(tiles_renderable.ptr);
      auto offset_ptr = reinterpret_cast<uint32_t*>(&tmp_ptr[tile_stat_buffer_size * VISIBILITY_STAT_OFFSET_MULT]);
      const uint32_t index_array = tile_index / UINT32_WIDTH;
      const uint32_t index_bit = tile_index % UINT32_WIDTH;
      const uint32_t mask = visible ? (1 << index_bit) : ~(1 << index_bit);
      // атомарность? желательно
      offset_ptr[index_array] = visible ? offset_ptr[index_array] | mask : offset_ptr[index_array] & mask;
    }
    
    bool world_map_buffers::get_map_exploration(const uint32_t &tile_index) const {
      assert(tile_index < core::map::hex_count_d(core::map::detail_level));
      const size_t storage_buffer_alignment = std::max(c->limits()->minStorageBufferOffsetAlignment, size_t(16));
      const size_t tile_stat_buffer_size = align_to(ceil(double(core::map::hex_count_d(core::map::detail_level)) / double(UINT32_WIDTH)), storage_buffer_alignment);
      auto tmp_ptr = reinterpret_cast<char*>(tiles_renderable.ptr);
      auto offset_ptr = reinterpret_cast<uint32_t*>(&tmp_ptr[tile_stat_buffer_size * EXPLORED_STAT_OFFSET_MULT]);
      const uint32_t index_array = tile_index / UINT32_WIDTH;
      const uint32_t index_bit = tile_index % UINT32_WIDTH;
      const uint32_t mask = 1 << index_bit;
      // атомарность? желательно
      return (offset_ptr[index_array] & mask) == mask;
    }
    
    bool world_map_buffers::get_map_visibility(const uint32_t &tile_index) const {
      assert(tile_index < core::map::hex_count_d(core::map::detail_level));
      const size_t storage_buffer_alignment = std::max(c->limits()->minStorageBufferOffsetAlignment, size_t(16));
      const size_t tile_stat_buffer_size = align_to(ceil(double(core::map::hex_count_d(core::map::detail_level)) / double(UINT32_WIDTH)), storage_buffer_alignment);
      auto tmp_ptr = reinterpret_cast<char*>(tiles_renderable.ptr);
      auto offset_ptr = reinterpret_cast<uint32_t*>(&tmp_ptr[tile_stat_buffer_size * VISIBILITY_STAT_OFFSET_MULT]);
      const uint32_t index_array = tile_index / UINT32_WIDTH;
      const uint32_t index_bit = tile_index % UINT32_WIDTH;
      const uint32_t mask = 1 << index_bit;
      // атомарность? желательно
      return (offset_ptr[index_array] & mask) == mask;
    }
    
    bool world_map_buffers::get_map_renderable(const uint32_t &tile_index) const {
      assert(tile_index < core::map::hex_count_d(core::map::detail_level));
      const size_t storage_buffer_alignment = std::max(c->limits()->minStorageBufferOffsetAlignment, size_t(16));
      const size_t tile_stat_buffer_size = align_to(ceil(double(core::map::hex_count_d(core::map::detail_level)) / double(UINT32_WIDTH)), storage_buffer_alignment);
      auto tmp_ptr = reinterpret_cast<char*>(tiles_renderable.ptr);
      auto offset_ptr = reinterpret_cast<uint32_t*>(&tmp_ptr[tile_stat_buffer_size * RENDER_STAT_OFFSET_MULT]);
      const uint32_t index_array = tile_index / UINT32_WIDTH;
      const uint32_t index_bit = tile_index % UINT32_WIDTH;
      const uint32_t mask = 1 << index_bit;
      // атомарность? желательно
      return (offset_ptr[index_array] & mask) == mask;
    }
    
    void world_map_buffers::clear_renderable() {
      const size_t storage_buffer_alignment = std::max(c->limits()->minStorageBufferOffsetAlignment, size_t(16));
      const size_t tile_stat_buffer_size = align_to(ceil(double(core::map::hex_count_d(core::map::detail_level)) / double(UINT32_WIDTH)), storage_buffer_alignment);
      auto tmp_ptr = reinterpret_cast<char*>(tiles_renderable.ptr);
      auto offset_ptr = reinterpret_cast<uint32_t*>(&tmp_ptr[tile_stat_buffer_size * RENDER_STAT_OFFSET_MULT]);
      memset(offset_ptr, 0, tile_stat_buffer_size);
    }
  }
}
