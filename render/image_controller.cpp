#include "image_controller.h"

#include "yavf.h"
#include "image_container.h"
#include "image_container_constants.h"

#include <cctype>

namespace devils_engine {
  namespace render {
    bool parse_image_id(const std::string_view &str, std::string_view &image_id, uint32_t &layer_index, bool &mirror_u, bool &mirror_v) {
      // str - строка вида "image_id.image_index.uv"
      image_id = "";
      layer_index = 0;
      mirror_u = false;
      mirror_v = false;
      
      bool found_mirror = false;
      bool found_index = false;
      
      int64_t str_size = str.size();
      int64_t current_pos = str_size;
      size_t points_found = 0;
      while (str_size > 0 && current_pos > 0 && points_found < 2) {
        --current_pos;
        
        if (str[current_pos] == '.') {
          ++points_found;
          const std::string_view s = str.substr(current_pos, str_size);
          if (s.empty()) {
            str_size = current_pos;
            continue;
          }
          
          const bool is_u  = s == "u";
          const bool is_v  = s == "v";
          const bool is_uv = s == "uv";
          const bool is_vu = s == "vu";
          const bool valid_mirror = is_u || is_v || is_uv || is_vu;
          if (valid_mirror && !found_mirror) {
            mirror_u = is_u || is_uv || is_vu;
            mirror_v = is_v || is_uv || is_vu;
            str_size = current_pos;
            found_mirror = true;
            continue;
          }
          
          bool valid_num = true;
          for (const char c : s) {
            if (!isdigit(c)) {
              valid_num = false;
              break;
            }
          }
          
          if (!valid_num) {
            str_size = current_pos;
            continue;
          }
          
          layer_index = atol(s.data());
          found_index = true;
          str_size = current_pos;
        }
      }
      
      const std::string_view s = str.substr(current_pos, str_size);
      if (s.empty()) return false;
      
      image_id = s;
      return true;
    }
    
    struct internal { // это вытащим наверное в отдельный хедер
      yavf::DescriptorPool pool;
      yavf::DescriptorSetLayout layout;
      yavf::DescriptorSet* image_set;
      yavf::Device* device;
      uint32_t samplers_count;
      uint32_t images_count;
      
      inline internal() : pool(VK_NULL_HANDLE), layout(VK_NULL_HANDLE), image_set(nullptr), samplers_count(0), images_count(0) {}
    };
    
    std::pair<uint32_t, uint32_t> image_controller::container_view::get_size() const {
      const auto pool = container->get_pool(slot);
      return std::make_pair(pool->image_size().width, pool->image_size().height);
    }
    
    render::image_t image_controller::container_view::get_image(const size_t &index, const bool mirror_u, const bool mirror_v) const {
      if (index >= count) throw std::runtime_error("Bad image index");
      uint32_t current_slot = slot;
      uint32_t current_index = offset+index;
      while (current_index > render::image_container::image_pool::max_size) {
        ++current_slot;
        current_index -= render::image_container::image_pool::max_size;
      }
      
      return render::create_image(current_slot, current_index, sampler, mirror_u, mirror_v);
    }
    
    image_controller::image_controller(yavf::Device* device, image_container* container) : container(container), device(device), current_slot_offset(0) {
      container->set_slots(IMAGE_CONTAINER_SLOT_SIZE); // вряд ли будет больше 256
      yavf::DescriptorPool pool = VK_NULL_HANDLE;
      yavf::DescriptorSetLayout layout = VK_NULL_HANDLE;
      
      {
        yavf::DescriptorPoolMaker dpm(device);
        pool = dpm.poolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, IMAGE_CONTAINER_SLOT_SIZE)
                  .poolSize(VK_DESCRIPTOR_TYPE_SAMPLER, IMAGE_SAMPLERS_COUNT)
                  .create(IMAGE_CONTAINER_DESCRIPTOR_POOL_NAME);
      }
      
      {
        yavf::DescriptorLayoutMaker dlm(device);
        layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_ALL, IMAGE_CONTAINER_SLOT_SIZE)
                    .binding(1, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_ALL, IMAGE_SAMPLERS_COUNT)
                    .create(IMAGE_CONTAINER_DESCRIPTOR_LAYOUT_NAME);
      }
      
      {
        yavf::DescriptorMaker dm(device);
        set = dm.layout(layout).create(pool)[0];
      }
      
      {
        yavf::SamplerMaker sm(device);
        yavf::Sampler s1 = sm.addressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT).
                              anisotropy(VK_TRUE, 16.0f).
                              filter(VK_FILTER_LINEAR, VK_FILTER_LINEAR).
                              mipmapMode(VK_SAMPLER_MIPMAP_MODE_LINEAR).
                              lod(0.0f, 1000.0f).
                              create(IMAGE_SAMPLER_LINEAR_NAME);
                              
        yavf::Sampler s2 = sm.addressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT).
                              anisotropy(VK_TRUE, 16.0f).
                              filter(VK_FILTER_NEAREST, VK_FILTER_NEAREST).
                              mipmapMode(VK_SAMPLER_MIPMAP_MODE_NEAREST). // возможно можно скомбинировать с другим типом
                              lod(0.0f, 1000.0f).
                              create(IMAGE_SAMPLER_NEAREST_NAME);
                              
        set->resize(IMAGE_CONTAINER_SLOT_SIZE + IMAGE_SAMPLERS_COUNT);
        set->at(IMAGE_CONTAINER_SLOT_SIZE + 0) = {s1.handle(), VK_NULL_HANDLE, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 1, VK_DESCRIPTOR_TYPE_SAMPLER};
        set->at(IMAGE_CONTAINER_SLOT_SIZE + 1) = {s2.handle(), VK_NULL_HANDLE, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, 1, VK_DESCRIPTOR_TYPE_SAMPLER};
      }
      
      container->update_descriptor_data(set);
      set->update();
    }
    
    image_controller::~image_controller() {
      device->destroyDescriptorPool(IMAGE_CONTAINER_DESCRIPTOR_POOL_NAME);
      device->destroySetLayout(IMAGE_CONTAINER_DESCRIPTOR_LAYOUT_NAME);
      device->destroySampler(IMAGE_SAMPLER_LINEAR_NAME);
      device->destroySampler(IMAGE_SAMPLER_NEAREST_NAME);
    }
    
    const image_controller::container_view* image_controller::get_view(const std::string &name) const {
      auto itr = image_slots.find(name);
      if (itr == image_slots.end()) return nullptr;
      return &itr->second;
    }
    
    void image_controller::clear_type(const image_controller::image_type &type) {
      for (auto itr = image_slots.begin(); itr != image_slots.end();) {
        if (itr->second.type != type) {
          ++itr;
          continue;
        }
        
        container->destroy_pool(itr->second.slot);
        itr = image_slots.erase(itr);
      }
    }
    
    void image_controller::register_images(const image_controller::input_data &data) {
      auto itr = image_slots.find(data.name);
      if (itr != image_slots.end()) throw std::runtime_error("Images with id " + data.name + " are already exists");
      image_slots.insert(std::make_pair(data.name, container_view{container, data.slot, data.offset, data.count, data.type, data.sampler}));
    }
    
//     void image_controller::create_slots() {
//       size_t system_counter = 0;
//       for (const auto &p : image_slots) {
//         if (p.second.type == image_type::system) {
//           ++system_counter;
//           if (system_counter > 4) throw std::runtime_error("Bad system images");
//         }
//         
//         
//       }
//     }

    void image_controller::update_set() {
      container->update_descriptor_data(set);
      set->update();
    }
    
    size_t image_controller::get_images_size() const {
      return container->memory();
    }
  }
}
