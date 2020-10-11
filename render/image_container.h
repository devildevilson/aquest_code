#ifndef IMAGE_CONTAINER_H
#define IMAGE_CONTAINER_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
#include "shared_structures.h"
#include "utils/memory_pool.h"
#include "utils/bit_field.h"

#define TEXTURE_MAX_LAYER_COUNT 2048

// я тут прикинул, мы вполне можем теперь использовать array
// заполняя слоты нулл текстуркой, другое дело что придется испльно переделать image_pool

namespace yavf {
  class Device;
  class Image;
//   class DescriptorPool;
  class DescriptorSet;
//   class DescriptorSetLayout;
}

namespace devils_engine {
  namespace utils {
    struct extent_2d {
      uint32_t width;
      uint32_t height;
    };
  }
  
  namespace render {
    class image_container {
    public:
      struct image_pool {
        static const size_t max_size = UINT8_MAX+1;
        static const size_t container_size = max_size/SIZE_WIDTH;
        static_assert(max_size % SIZE_WIDTH == 0, "Better if image pool size would be devided by SIZE_WIDTH");
        static_assert(max_size < TEXTURE_MAX_LAYER_COUNT, "Image pool size must be less than TEXTURE_MAX_LAYER_COUNT");
        
        //uint32_t layers_count;
        utils::bit_field<container_size> data;
        yavf::Image* image;

        image_pool(yavf::Device* device, const utils::extent_2d &img_size, const uint32_t &mips, const uint32_t &layers);
        utils::extent_2d image_size() const;
        uint32_t mip_levels() const;
        size_t used_size() const;
        size_t free_size() const;
        bool full() const;
        
        bool is_used(const size_t &index) const;
        void set_used(const size_t &index, const bool value);
        
        uint32_t occupy_free_index();
        void release_index(const uint32_t &index);
        uint32_t layers_count() const;
      };
      
      struct create_info {
        yavf::Device* device;
      };
      image_container(const create_info &info);
      ~image_container();
      
      void set_slots(const size_t &slots);
      size_t pool_count() const;
      size_t first_empty_pool() const;
      bool pool_exists(const size_t &index) const;
      const image_pool* get_pool(const size_t &index) const;
      render::image_t get_image(const size_t &pool_index);
      uint32_t reserve_image(const size_t &pool_index);
      void release_image(const render::image_t &img);
      void create_pool(const uint32_t &slot_index, const utils::extent_2d &img_size, const uint32_t &mips, const uint32_t &layers);
      void destroy_pool(const uint32_t &slot_index);
      bool check_image(const render::image_t &img);
      
      void update_descriptor_data(yavf::DescriptorSet* set);
      size_t memory() const;
    private:
      yavf::Device* device;
      
      utils::memory_pool<image_pool, sizeof(image_pool)*20> image_memory;
      std::vector<image_pool*> slots;
    };
  }
}

#endif
