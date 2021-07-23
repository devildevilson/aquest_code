#include "targets.h"

#include "shared_structures.h"
#include "bin/core_structures.h"
#include "container.h"
#include "makers.h"

#include <cstring>

namespace devils_engine {
  namespace render {
    buffers::buffers(container* c) : c(c) {
      auto device = c->vulkan->device;
      auto allocator = c->vulkan->buffer_allocator;
      const size_t buffer1_size = align_to(sizeof(camera_data),   16);
      const size_t buffer2_size = align_to(sizeof(matrices_data), 16);
       uniform.create(allocator, buffer(buffer1_size, vk::BufferUsageFlagBits::eUniformBuffer), vma::MemoryUsage::eCpuOnly, "uniform buffer");
      matrices.create(allocator, buffer(buffer2_size, vk::BufferUsageFlagBits::eUniformBuffer), vma::MemoryUsage::eCpuOnly, "matrices buffer");
       heraldy.create(allocator, buffer(16, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst), vma::MemoryUsage::eGpuOnly, "heraldy buffer");
//       heraldy  = device.create(yavf::BufferCreateInfo::buffer(16,   VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT), VMA_MEMORY_USAGE_GPU_ONLY);
      
      assert(uniform.ptr != nullptr);
      assert(matrices.ptr != nullptr);
      
      auto pool = c->vulkan->descriptor_pool;
      auto storage_layout = c->vulkan->storage_layout;
      auto uniform_layout = c->vulkan->uniform_layout;
      
      {
        descriptor_set_maker dm(&device);
        auto desc = dm.layout(uniform_layout).create(pool, "uniform layout")[0];
        uniform_set = desc;
        
        descriptor_set_updater dsu(&device);
        dsu.currentSet(uniform_set);
        dsu.begin(0, 0, vk::DescriptorType::eUniformBuffer).buffer(uniform.handle);
        dsu.begin(1, 0, vk::DescriptorType::eUniformBuffer).buffer(matrices.handle);
        dsu.update();
      }
      
      {
        descriptor_set_maker dm(&device);
        heraldy_set = dm.layout(storage_layout).create(pool, "heraldy buffer descriptor")[0];
        
        descriptor_set_updater dsu(&device);
        dsu.currentSet(heraldy_set).begin(0, 0, vk::DescriptorType::eStorageBuffer).buffer(heraldy.handle).update();
      }
    }

    buffers::~buffers() {
      auto allocator = c->vulkan->buffer_allocator;
      uniform.destroy(allocator);
      matrices.destroy(allocator);
      heraldy.destroy(allocator);
    }

//     void buffers::update_matrix(const glm::mat4 &matrix) {
//       auto camera = reinterpret_cast<camera_data*>(uniform->ptr());
//       camera->viewproj = matrix;
//     }
    
    void buffers::update_projection_matrix(const glm::mat4 &matrix) {
//       auto camera = reinterpret_cast<camera_data*>(uniform->ptr());
      auto mat = reinterpret_cast<matrices_data*>(matrices.ptr);
      
      mat->proj = matrix;
      mat->invProj = glm::inverse(matrix);
    }
    
    void buffers::update_view_matrix(const glm::mat4 &matrix) {
      auto camera = reinterpret_cast<camera_data*>(uniform.ptr);
      auto mat = reinterpret_cast<matrices_data*>(matrices.ptr);
      
      mat->view = matrix;
      mat->invView = glm::inverse(matrix);
      camera->viewproj = mat->proj * mat->view;
      mat->invViewProj = glm::inverse(camera->viewproj);
    }
    
    void buffers::update_pos(const glm::vec3 &pos) {
      auto camera = reinterpret_cast<camera_data*>(uniform.ptr);
      camera->pos = glm::vec4(pos, 1.0f);
    }
    
    void buffers::update_dir(const glm::vec3 &dir) {
      auto camera = reinterpret_cast<camera_data*>(uniform.ptr);
      camera->dir = glm::vec4(dir, 0.0f);
    }
    
    void buffers::update_zoom(const float &zoom) {
      auto camera = reinterpret_cast<camera_data*>(uniform.ptr);
      camera->dim[2] = glm::floatBitsToUint(zoom);
    }
    
    void buffers::update_cursor_dir(const glm::vec4 &cursor_dir) {
      auto camera = reinterpret_cast<camera_data*>(uniform.ptr);
      camera->cursor_dir = cursor_dir;
    }
    
    void buffers::update_dimensions(const uint32_t &width, const uint32_t &height) {
      auto camera = reinterpret_cast<camera_data*>(uniform.ptr);
      camera->dim.x = width;
      camera->dim.y = height;
    }

    void buffers::recreate(const uint32_t &width, const uint32_t &height) {
      const glm::mat4 persp = glm::perspective(glm::radians(75.0f), float(width) / float(height), 0.1f, 256.0f);
      update_projection_matrix(persp);
      auto camera = reinterpret_cast<camera_data*>(uniform.ptr);
      camera->dim[0] = width;
      camera->dim[1] = height;
    }
    
    glm::mat4 buffers::get_matrix() const {
      auto camera = reinterpret_cast<camera_data*>(uniform.ptr);
      return camera->viewproj;
    }
    
    glm::mat4 buffers::get_proj() const {
      auto mat = reinterpret_cast<matrices_data*>(matrices.ptr);
      return mat->proj;
    }
    
    glm::mat4 buffers::get_view() const {
      auto mat = reinterpret_cast<matrices_data*>(matrices.ptr);
      return mat->view;
    }
    
    glm::mat4 buffers::get_inv_proj() const {
      auto mat = reinterpret_cast<matrices_data*>(matrices.ptr);
      return mat->invProj;
    }
    
    glm::mat4 buffers::get_inv_view() const {
      auto mat = reinterpret_cast<matrices_data*>(matrices.ptr);
      return mat->invView;
    }
    
    glm::mat4 buffers::get_inv_view_proj() const {
      auto mat = reinterpret_cast<matrices_data*>(matrices.ptr);
      return mat->invViewProj;
    }
    
    glm::vec4 buffers::get_pos() const {
      auto camera = reinterpret_cast<camera_data*>(uniform.ptr);
      return camera->pos;
    }
    
    glm::vec4 buffers::get_dir() const {
      auto camera = reinterpret_cast<camera_data*>(uniform.ptr);
      return camera->dir;
    }
    
    glm::vec4 buffers::get_cursor_dir() const {
      auto camera = reinterpret_cast<camera_data*>(uniform.ptr);
      return camera->cursor_dir;
    }
    
    void buffers::resize_heraldy_buffer(const size_t &size) {
      auto allocator = c->vulkan->buffer_allocator;
      heraldy.destroy(allocator);
      heraldy.create(allocator, buffer(size, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst), vma::MemoryUsage::eGpuOnly, "heraldy buffer");
      
      auto device = c->vulkan->device;
      descriptor_set_updater dsu(&device);
      dsu.currentSet(heraldy_set).begin(0, 0, vk::DescriptorType::eStorageBuffer).buffer(heraldy.handle).update();
    }
    
#define WORLD_MAP_DESCRIPTOR_POOL "world_map_descriptor_pool"
    
    world_map_buffers::world_map_buffers(container* c) : c(c) {
      auto device = c->vulkan->device;
      auto storage_layout = c->vulkan->storage_layout;
      auto allocator = c->vulkan->buffer_allocator;
      border_buffer.create(allocator, buffer(sizeof(glm::vec4)*4, vk::BufferUsageFlagBits::eStorageBuffer), vma::MemoryUsage::eCpuOnly);
      border_types.create(allocator, buffer(sizeof(glm::vec4)*4, vk::BufferUsageFlagBits::eStorageBuffer), vma::MemoryUsage::eCpuOnly);
      
      {
        descriptor_pool_maker dpm(&device);
        pool = dpm.poolSize(vk::DescriptorType::eStorageBuffer, 3).create(WORLD_MAP_DESCRIPTOR_POOL);
      }
      
      {
        descriptor_set_maker dsm(&device);
        border_set = dsm.layout(storage_layout).create(pool, "border buffer set")[0];
        
        descriptor_set_updater dsu(&device);
        dsu.currentSet(border_set).begin(0, 0, vk::DescriptorType::eStorageBuffer).buffer(border_buffer.handle).update();
      }
      
      {
        descriptor_set_maker dsm(&device);
        types_set = dsm.layout(storage_layout).create(pool, "border types set")[0];
        
        descriptor_set_updater dsu(&device);
        dsu.currentSet(types_set).begin(0, 0, vk::DescriptorType::eStorageBuffer).buffer(border_types.handle).update();
      }
    }
    
    world_map_buffers::~world_map_buffers() {
      auto allocator = c->vulkan->buffer_allocator;
      auto device = c->vulkan->device;
      border_buffer.destroy(allocator);
      border_types.destroy(allocator);
      tiles_connections.destroy(allocator);
      structure_buffer.destroy(allocator);
      device.destroy(pool);
    }
    
    void world_map_buffers::recreate(const uint32_t &width, const uint32_t &height) {
      (void)width;
      (void)height;
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
      dsu.currentSet(types_set).begin(0, 0, vk::DescriptorType::eStorageBuffer).buffer(border_types.handle).update();
    }
  }
}
