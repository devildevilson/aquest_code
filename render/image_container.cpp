#include "image_container.h"

#include "yavf.h"

namespace devils_engine {
  namespace render {
    static yavf::Image* null_image = nullptr;
    
    image_container::image_pool::image_pool(yavf::Device* device, const utils::extent_2d &img_size, const uint32_t &mips, const uint32_t &layers) {
      ASSERT(layers < max_size);
      ASSERT(layers != 0);
      image = device->create(
        yavf::ImageCreateInfo::texture2D(
          {img_size.width, img_size.height}, 
          VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, 
          VK_FORMAT_R8G8B8A8_UNORM, 
          layers, 
          mips
        ), 
        VMA_MEMORY_USAGE_GPU_ONLY
      );
      
      image->createView(layers == 1 ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY, VK_IMAGE_ASPECT_COLOR_BIT);
    }
    
    utils::extent_2d image_container::image_pool::image_size() const {
      return {image->info().extent.width, image->info().extent.height};
    }
    
    uint32_t image_container::image_pool::mip_levels() const {
      return image->info().mipLevels;
    }
    
    size_t image_container::image_pool::used_size() const {
      size_t counter = 0;
      for (size_t i = 0; i < layers_count(); ++i) {
        counter += size_t(is_used(i));
      }
      return counter;
    }
    
    size_t image_container::image_pool::free_size() const {
      size_t counter = 0;
      for (size_t i = 0; i < layers_count(); ++i) {
        counter += size_t(!is_used(i));
      }
      return counter;
    }
    
    bool image_container::image_pool::full() const {
      return layers_count() == used_size();
    }
    
    bool image_container::image_pool::is_used(const size_t &index) const {
      return data.get(index);
    }
    
    void image_container::image_pool::set_used(const size_t &index, const bool value) {
      data.set(index, value);
    }
    
    uint32_t image_container::image_pool::occupy_free_index() {
      for (uint32_t i = 0; i < layers_count(); ++i) {
        if (!is_used(i)) {
          set_used(i, true);
          return i;
        }
      }
      return UINT32_MAX;
    }
    
    void image_container::image_pool::release_index(const uint32_t &index) {
      if (index >= layers_count()) throw std::runtime_error("Releasing bad index");
      set_used(index, false);
    }
    
    uint32_t image_container::image_pool::layers_count() const {
      return image->info().arrayLayers;
    }
    
    image_container::image_container(const create_info &info) : device(info.device) {
      null_image = device->create(yavf::ImageCreateInfo::texture2D({1, 1}, VK_IMAGE_USAGE_SAMPLED_BIT), VMA_MEMORY_USAGE_GPU_ONLY);
      null_image->createView(VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT);
    }
    
    image_container::~image_container() {
      for (auto pool : slots) {
        device->destroy(pool->image);
        image_memory.destroy(pool);
      }
      
      device->destroy(null_image);
    }
    
    void image_container::set_slots(const size_t &slots) {
      if (this->slots.size() > slots) throw std::runtime_error("Removing slots this way is undefined");
      this->slots.resize(slots, nullptr);
    }
    
    size_t image_container::pool_count() const {
      return slots.size();
    }
    
    const image_container::image_pool* image_container::get_pool(const size_t &index) const {
      if (index >= slots.size()) return nullptr;
      return slots[index];
    }
    
    render::image_t image_container::get_image(const size_t &pool_index) {
      if (pool_index >= slots.size()) return {GPU_UINT_MAX};
      const uint32_t free_index = slots[pool_index]->occupy_free_index();
      if (free_index == UINT32_MAX) return {GPU_UINT_MAX};
      return render::create_image(pool_index, free_index, 0, false, false); //  {static_cast<uint32_t>(pool_index), free_index}
    }
    
    void image_container::release_image(const render::image_t &img) {
      const auto index = get_image_index(img);
      if (index >= slots.size()) throw std::runtime_error("Trying to release bad image");
      slots[index]->release_index(get_image_layer(img));
    }
    
    void image_container::create_pool(const uint32_t &slot_index, const utils::extent_2d &img_size, const uint32_t &mips, const uint32_t &layers) {
      if (slot_index > slots.size()) throw std::runtime_error("Bad image slot index");
      if (slot_index == slots.size()) slots.push_back(nullptr);
      if (slots[slot_index] != nullptr) throw std::runtime_error("Replacing an existing slot is not allowed");
      auto ptr = image_memory.create(device, img_size, mips, layers);
      slots[slot_index] = ptr;
    }
    
    void image_container::destroy_pool(const uint32_t &slot_index) {
      if (slot_index >= slots.size()) throw std::runtime_error("Destruction of uncreated slot");
      if (slots[slot_index] == nullptr) throw std::runtime_error("Destruction of uncreated slot");
      
      image_memory.destroy(slots[slot_index]);
      slots[slot_index] = nullptr;
    }
    
    bool image_container::check_image(const render::image_t &img) {
      if (!render::is_image_valid(img)) return false;
      const uint32_t img_index = render::get_image_index(img);
      const uint32_t img_layer = render::get_image_layer(img);
      
      if (img_index >= slots.size()) return false;
      if (slots[img_index] == nullptr) return false;
      if (img_layer >= slots[img_index]->layers_count()) return false;
      if (!slots[img_index]->is_used(img_layer)) return false;
      
      return true;
    }
    
    void image_container::update_descriptor_data(yavf::DescriptorSet* set) {
      if (slots.size() >= set->size()) throw std::runtime_error("slots.size() >= set->size()");
      for (size_t i = 0; i < slots.size(); ++i) {
        // мы можем добавить специальную нулл текстурку 1х1, тем самым моделируя отсутствие данных в этом слоте
        yavf::ImageView* view = slots[i] == nullptr ? null_image->view() : slots[i]->image->view();
        set->at(i) = {VK_NULL_HANDLE, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, static_cast<uint32_t>(i), 0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE};
      }
    }
  }
}
