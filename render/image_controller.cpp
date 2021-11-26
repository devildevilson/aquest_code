#include "image_controller.h"

#include "vulkan_hpp_header.h"
#include "makers.h"
#include "image_container.h"
#include "image_container_constants.h"
#include "utils/globals.h"
#include "container.h"

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
      vk::DescriptorPool pool;
      vk::DescriptorSetLayout layout;
      vk::DescriptorSet image_set;
      vk::Device* device;
      vk::Sampler linear;
      vk::Sampler nearest;
      vk::Sampler nuklear;
      uint32_t samplers_count;
      uint32_t images_count;
      
      internal(vk::Device* device) : pool(nullptr), layout(nullptr), image_set(nullptr), device(device), samplers_count(0), images_count(0) {}
      ~internal() {
        device->destroy(pool);
        device->destroy(layout);
        device->destroy(linear);
        device->destroy(nearest);
        device->destroy(nuklear);
      }
    };
    
//     image_controller::container_view::container_view() : container(nullptr), slot(UINT32_MAX), offset(UINT32_MAX), count(UINT32_MAX), sampler(UINT32_MAX) {}
    
    std::tuple<uint32_t, uint32_t> image_controller::container_view::get_size() const {
      const auto pool = container->get_pool(slot);
      return std::make_tuple(pool->image_size().width, pool->image_size().height);
    }
    
    render::image_t image_controller::container_view::get_image(const size_t &index, const bool mirror_u, const bool mirror_v) const {
      if (index >= count) throw std::runtime_error("Bad image index");
      uint32_t current_slot = slot;
      uint32_t current_index = offset+index;
      while (current_index >= render::image_container::image_pool::max_size) {
        ++current_slot;
        current_index -= render::image_container::image_pool::max_size;
      }
      
      return render::create_image(current_slot, current_index, sampler, mirror_u, mirror_v);
    }
    
    image_controller::image_controller(vk::Device* device, image_container* container) : container(container), vulkan(new internal(device)), current_slot_offset(0) {
      container->set_slots(IMAGE_CONTAINER_SLOT_SIZE); // вряд ли будет больше 256
      
      {
        descriptor_pool_maker dpm(device);
        vulkan->pool = 
          dpm.poolSize(vk::DescriptorType::eSampledImage, IMAGE_CONTAINER_SLOT_SIZE)
             .poolSize(vk::DescriptorType::eSampler, IMAGE_SAMPLERS_COUNT)
             .create(IMAGE_CONTAINER_DESCRIPTOR_POOL_NAME);
      }
      
      {
        descriptor_set_layout_maker dslm(device);
        vulkan->layout = 
          dslm.binding(0, vk::DescriptorType::eSampledImage, vk::ShaderStageFlagBits::eAll, IMAGE_CONTAINER_SLOT_SIZE)
              .binding(1, vk::DescriptorType::eSampler, vk::ShaderStageFlagBits::eAll, IMAGE_SAMPLERS_COUNT)
              .create(IMAGE_CONTAINER_DESCRIPTOR_LAYOUT_NAME);
      }
      
      {
        descriptor_set_maker dsm(device);
        vulkan->image_set = dsm.layout(vulkan->layout).create(vulkan->pool, "image set")[0];
      }
      
      auto render_container = global::get<render::container>();
      int enable = VK_FALSE;
      float level = 1.0f;
      if (render_container->is_properties_presented(render::container::physical_device_sampler_anisotropy)) {
        enable = VK_TRUE;
        level = 16.0f;
      }
      
      {
        sampler_maker sm(device);
        vulkan->linear = sm.addressMode(vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat).
                            anisotropy(enable, level).
                            filter(vk::Filter::eLinear, vk::Filter::eLinear).
                            mipmapMode(vk::SamplerMipmapMode::eLinear).
                            lod(0.0f, 1000.0f).
                            create(IMAGE_SAMPLER_LINEAR_NAME);
                              
        vulkan->nearest = sm.addressMode(vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat).
                             anisotropy(enable, level).
                             filter(vk::Filter::eNearest, vk::Filter::eNearest).
                             mipmapMode(vk::SamplerMipmapMode::eNearest). // возможно можно скомбинировать с другим типом
                             lod(0.0f, 1000.0f).
                             create(IMAGE_SAMPLER_NEAREST_NAME);
                              
        vulkan->nuklear = sm.addressMode(vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat).
                              anisotropy(enable, level).
                              borderColor(vk::BorderColor::eFloatTransparentBlack).
                              compareOp(VK_FALSE, vk::CompareOp::eGreater).
                              filter(vk::Filter::eNearest, vk::Filter::eNearest).
                              mipmapMode(vk::SamplerMipmapMode::eNearest). // возможно можно скомбинировать с другим типом
                              lod(0.0f, 1.0f).
                              unnormalizedCoordinates(VK_FALSE).
                              create("default_nuklear_sampler");
      }
      
      update_set();
    }
    
    image_controller::~image_controller() {
      delete vulkan;
      vulkan = nullptr;
    }
    
    const image_controller::container_view* image_controller::get_view(const std::string_view &name) const {
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

    void image_controller::update_set() {
      container->update_descriptor_data(get_descriptor_set());
      update_sampler_set();
    }
    
    void image_controller::update_sampler_set() {
      descriptor_set_updater dsu(vulkan->device);
      dsu.currentSet(vulkan->image_set);
      dsu.begin(1, 0, vk::DescriptorType::eSampler);
      dsu.sampler(vulkan->linear);
      dsu.begin(1, 1, vk::DescriptorType::eSampler);
      dsu.sampler(vulkan->nearest);
      dsu.begin(1, 2, vk::DescriptorType::eSampler);
      dsu.sampler(vulkan->nuklear);
      dsu.update();
    }
    
    size_t image_controller::get_images_size() const {
      return container->memory();
    }
    
    vk::DescriptorSet* image_controller::get_descriptor_set() const {
      return &vulkan->image_set;
    }
    
    vk::DescriptorSetLayout* image_controller::get_descriptor_set_layout() const {
      return &vulkan->layout;
    }
  }
}
