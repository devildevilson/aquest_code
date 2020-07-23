#include "targets.h"

#include "shared_structures.h"

#include <cstring>

namespace devils_engine {
  namespace render {
    buffers::buffers(yavf::Device* device) {
//       const size_t tri_count = map::tri_count_d(detail_level_acc_struct);
//       biomes  = device->create(yavf::BufferCreateInfo::buffer(sizeof(packed_biom_data_t) * 20,  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
//       tiles   = device->create(yavf::BufferCreateInfo::buffer(sizeof(light_map_tile_t) * map->tiles.size(),  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
//       points  = device->create(yavf::BufferCreateInfo::buffer(sizeof(glm::vec4) * map->points.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
      uniform = device->create(yavf::BufferCreateInfo::buffer(sizeof(camera_data), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
      matrices = device->create(yavf::BufferCreateInfo::buffer(sizeof(matrices_data), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
      border_buffer = device->create(yavf::BufferCreateInfo::buffer(sizeof(glm::vec4)*4, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
      border_types = device->create(yavf::BufferCreateInfo::buffer(sizeof(glm::vec4)*4, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
//       triangles = device->create(yavf::BufferCreateInfo::buffer(sizeof(packed_fast_triangle_t)*tri_count, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
//       tile_indices = device->create(yavf::BufferCreateInfo::buffer(sizeof(uint32_t)*map->tiles.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT), VMA_MEMORY_USAGE_CPU_ONLY);

      //memcpy(tiles->ptr(), map->tiles.data(), sizeof(map::tile) * map->tiles.size());
      //memcpy(points->ptr(), map->points.data(), sizeof(glm::vec3) * map->points.size());
//       memset(tile_indices->ptr(), 0, tile_indices->info().size);
      
//       size_t offset = 0;
//       for (size_t i = 0; i < detail_level_acc_struct; ++i) {
//         offset += map::tri_count_d(i);
//       }
      
//       auto triangles_arr = reinterpret_cast<packed_fast_triangle_t*>(triangles->ptr());
//       for (size_t i = 0; i < tri_count; ++i) {
//         const uint32_t tri_index = i + offset;
//         const auto &tri = map->triangles[tri_index];
//         ASSERT(tri.current_level == detail_level_acc_struct);
//         triangles_arr[i].points[0] = tri.points[0];
//         triangles_arr[i].points[1] = tri.points[1];
//         triangles_arr[i].points[2] = tri.points[2];
//         
//         const glm::vec3 center = (map->points[tri.points[0]] + map->points[tri.points[1]] + map->points[tri.points[2]]) / float(3);
//         const float dist = glm::distance(center, map->points[tri.points[0]]);
//         max_triangle_size = glm::max(max_triangle_size, dist);
//       }

      auto pool = device->descriptorPool(DEFAULT_DESCRIPTOR_POOL_NAME);
      auto storage_layout = device->setLayout(STORAGE_BUFFER_LAYOUT_NAME);
      auto uniform_layout = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
      
//       yavf::DescriptorSetLayout tiles_data_layout = VK_NULL_HANDLE;
//       {
//         yavf::DescriptorLayoutMaker dlm(device);
//         tiles_data_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL)
//                                .binding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL)
//                                .binding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL)
//                                .binding(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL)
//                                .binding(4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL)
//                                .create(TILES_DATA_LAYOUT_NAME);
//       }
//       
//       {
//         yavf::DescriptorMaker dm(device);
//         auto desc = dm.layout(tiles_data_layout).create(pool)[0];
//         size_t index = desc->add({tiles, 0, tiles->info().size, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
//                        desc->add({biomes, 0, biomes->info().size, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
//                        desc->add({points, 0, points->info().size, 0, 2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
//                        desc->add({triangles, 0, triangles->info().size, 0, 3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
//                        desc->add({tile_indices, 0, tile_indices->info().size, 0, 4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
//         desc->update();
//         tiles->setDescriptor(desc, index);
//       }
      
//       {
//         yavf::DescriptorMaker dm(device);
//         auto desc = dm.layout(storage_layout).create(pool)[0];
//         size_t index = desc->add({biomes, 0, biomes->info().size, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
//         biomes->setDescriptor(desc, index);
//       }
// 
//       {
//         yavf::DescriptorMaker dm(device);
//         auto desc = dm.layout(storage_layout).create(pool)[0];
//         size_t index = desc->add({tiles, 0, tiles->info().size, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
//         tiles->setDescriptor(desc, index);
//       }
// 
//       {
//         yavf::DescriptorMaker dm(device);
//         auto desc = dm.layout(storage_layout).create(pool)[0];
//         size_t index = desc->add({points, 0, points->info().size, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
//         points->setDescriptor(desc, index);
//       }
//       
//       {
//         yavf::DescriptorMaker dm(device);
//         auto desc = dm.layout(storage_layout).create(pool)[0];
//         size_t index = desc->add({triangles, 0, triangles->info().size, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
//         triangles->setDescriptor(desc, index);
//       }
//       
//       {
//         yavf::DescriptorMaker dm(device);
//         auto desc = dm.layout(storage_layout).create(pool)[0];
//         size_t index = desc->add({tile_indices, 0, tile_indices->info().size, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
//         tile_indices->setDescriptor(desc, index);
//       }
      
      {
        yavf::DescriptorMaker dm(device);
        auto desc = dm.layout(uniform_layout).create(pool)[0];
        size_t index = desc->add({uniform, 0, uniform->info().size, 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER});
        uniform->setDescriptor(desc, index);
      }
      
      {
        yavf::DescriptorMaker dm(device);
        auto desc = dm.layout(storage_layout).create(pool)[0];
        size_t index = desc->add({border_buffer, 0, border_buffer->info().size, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
        border_buffer->setDescriptor(desc, index);
      }
      
      {
        yavf::DescriptorMaker dm(device);
        auto desc = dm.layout(storage_layout).create(pool)[0];
        size_t index = desc->add({border_types, 0, border_types->info().size, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
        border_types->setDescriptor(desc, index);
      }
      
//       auto tiles_arr = reinterpret_cast<map::tile*>(tiles->ptr());
//       PRINT_VAR("center    index", tiles_arr[0].index)
//       for (uint32_t i = 0; i < 6; ++i) {
//         //ASSERT(tiles_arr[0].neighbours[i].points[0] != tiles_arr[0].neighbours[i].points[1]);
//         PRINT_VAR("neighbour index", tiles_arr[0].neighbours[i].points[0])
//         PRINT_VAR("neighbour index", tiles_arr[0].neighbours[i].points[1])
//       }
    }

    buffers::~buffers() {
      // device->destroy(tiles);
      // device->destroy(points);
      // device->destroy(uniform);
    }

//     void buffers::update_matrix(const glm::mat4 &matrix) {
//       auto camera = reinterpret_cast<camera_data*>(uniform->ptr());
//       camera->viewproj = matrix;
//     }
    
    void buffers::update_projection_matrix(const glm::mat4 &matrix) {
//       auto camera = reinterpret_cast<camera_data*>(uniform->ptr());
      auto mat = reinterpret_cast<matrices_data*>(matrices->ptr());
      
      mat->proj = matrix;
      mat->invProj = glm::inverse(matrix);
    }
    
    void buffers::update_view_matrix(const glm::mat4 &matrix) {
      auto camera = reinterpret_cast<camera_data*>(uniform->ptr());
      auto mat = reinterpret_cast<matrices_data*>(matrices->ptr());
      
      mat->view = matrix;
      mat->invView = glm::inverse(matrix);
      camera->viewproj = mat->proj * mat->view;
      mat->invViewProj = glm::inverse(camera->viewproj);
    }
    
    void buffers::update_pos(const glm::vec3 &pos) {
      auto camera = reinterpret_cast<camera_data*>(uniform->ptr());
      camera->pos = glm::vec4(pos, 1.0f);
    }
    
    void buffers::update_dir(const glm::vec3 &dir) {
      auto camera = reinterpret_cast<camera_data*>(uniform->ptr());
      camera->dir = glm::vec4(dir, 0.0f);
    }
    
//     void buffers::set_tile_data(const map::tile &tile, const uint32_t &index) {
//       auto tiles_arr = reinterpret_cast<light_map_tile_t*>(tiles->ptr());
//       const map_tile_t map_tile{
//         tile.index,
//         0,
//         0,
//         0.0f,
//         {tile.neighbours[0].points[0], tile.neighbours[1].points[0], tile.neighbours[2].points[0], tile.neighbours[3].points[0], tile.neighbours[4].points[0], tile.neighbours[5].points[0]},
//         {tile.neighbours[0].index, tile.neighbours[1].index, tile.neighbours[2].index, tile.neighbours[3].index, tile.neighbours[4].index, tile.neighbours[5].index}
//       };
//       tiles_arr[index] = pack_data(map_tile);
//     }
//     
//     void buffers::set_point_data(const glm::vec3 &point, const uint32_t &index) {
//       auto points_arr = reinterpret_cast<glm::vec4*>(points->ptr());
//       points_arr[index] = glm::vec4(point, 1.0f);
//     }
//     
//     void buffers::set_tile_indices(const uint32_t &triangle_index, const std::vector<uint32_t> &indices, const uint32_t &offset, const uint32_t &count, const bool has_pentagon) {
//       auto triangles_arr = reinterpret_cast<packed_fast_triangle_t*>(triangles->ptr());
//       auto tile_indices_arr = reinterpret_cast<uint32_t*>(tile_indices->ptr());
//       
//       ASSERT(indices.size() == count);
//       
//       triangles_arr[triangle_index].data[0] = offset;
//       triangles_arr[triangle_index].data[1] = count;
//       triangles_arr[triangle_index].data[2] = uint32_t(has_pentagon);
//       
//       memcpy(&tile_indices_arr[offset], indices.data(), sizeof(uint32_t)*count);
//     }

    void buffers::recreate(const uint32_t &width, const uint32_t &height) {
      (void)width;
      (void)height;
    }
    
    glm::mat4 buffers::get_matrix() const {
      auto camera = reinterpret_cast<camera_data*>(uniform->ptr());
      return camera->viewproj;
    }
    
    glm::mat4 buffers::get_proj() const {
      auto mat = reinterpret_cast<matrices_data*>(matrices->ptr());
      return mat->proj;
    }
    
    glm::mat4 buffers::get_view() const {
      auto mat = reinterpret_cast<matrices_data*>(matrices->ptr());
      return mat->view;
    }
    
    glm::mat4 buffers::get_inv_proj() const {
      auto mat = reinterpret_cast<matrices_data*>(matrices->ptr());
      return mat->invProj;
    }
    
    glm::mat4 buffers::get_inv_view() const {
      auto mat = reinterpret_cast<matrices_data*>(matrices->ptr());
      return mat->invView;
    }
    
    glm::mat4 buffers::get_inv_view_proj() const {
      auto mat = reinterpret_cast<matrices_data*>(matrices->ptr());
      return mat->invViewProj;
    }
    
    glm::vec4 buffers::get_pos() const {
      auto camera = reinterpret_cast<camera_data*>(uniform->ptr());
      return camera->pos;
    }
  }
}
