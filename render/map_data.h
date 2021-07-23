#ifndef RENDER_MAP_DATA_H
#define RENDER_MAP_DATA_H

#include "vulkan_hpp_header.h"

namespace devils_engine {
  namespace render {
    struct map_data {
      // не принадлежит
      vk::Device device;
      vk::PhysicalDevice physical_device;
      vma::Allocator allocator;
      
      vk_buffer_data points;
      vk_buffer_data tiles;
      vk_buffer_data accel_triangles;
      vk_buffer_data tile_indices;
      vk_buffer_data biomes;
      vk_buffer_data structures;
      vk_buffer_data tile_object_indices;
      vk_buffer_data army_data_buffer;
      vk_buffer_data provinces;
      vk_buffer_data faiths;
      vk_buffer_data cultures;
      
      vk::DescriptorPool tiles_set_pool;
      vk::DescriptorSetLayout tiles_layout;
      vk::DescriptorSet tiles_set;
      
      inline map_data(vk::Device device, vk::PhysicalDevice physical_device, vma::Allocator allocator) : device(device), physical_device(physical_device), allocator(allocator) {}
      inline ~map_data() {
        points.destroy(allocator);
        tiles.destroy(allocator);
        accel_triangles.destroy(allocator);
        tile_indices.destroy(allocator);
        biomes.destroy(allocator);
        structures.destroy(allocator);
        tile_object_indices.destroy(allocator);
        army_data_buffer.destroy(allocator);
        provinces.destroy(allocator);
        faiths.destroy(allocator);
        cultures.destroy(allocator);
        
        device.destroy(tiles_set_pool);
        device.destroy(tiles_layout);
      }
    };
  }
}

#endif
