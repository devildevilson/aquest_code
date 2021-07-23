#include "image_container.h"

#include "vulkan_hpp_header.h"
#include "makers.h"
#include <iostream>

namespace devils_engine {
  namespace render {
    static vk::DeviceMemory null_memory = nullptr;
    static vk::Image null_image = nullptr;
    static vk::ImageView null_image_view = nullptr;
    
    image_container::image_pool::image_pool(vk::Device* device, vk::PhysicalDevice* physical_device, const extent2d &img_size, const uint32_t &mips, const uint32_t &layers) : 
      device(device), mem(nullptr), image(nullptr), view(nullptr), img_size(img_size), mips(mips), layers(layers) 
    {
      ASSERT(layers < max_size);
      ASSERT(layers != 0);
      const auto usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc;
      const auto info = texture2D(cast(img_size), usage, vk::Format::eR8G8B8A8Unorm, layers, mips);
      const auto [img, memory] = create_image(*device, *physical_device, info, vk::MemoryPropertyFlagBits::eDeviceLocal);
      const auto view_info = make_view_info(img, vk::Format::eR8G8B8A8Unorm, vk::ImageViewType::e2DArray, {vk::ImageAspectFlagBits::eColor, 0, mips, 0, layers});
      const auto v = device->createImageView(view_info);
      image = img;
      mem = memory;
      view = v;
    }
    
    image_container::image_pool::~image_pool() {
//       PRINT_VAR("device_ptr", device)
//       PRINT_VAR("device", &**device)
      device->destroy(view);
      device->destroy(image);
      device->free(mem);
    }
    
    extent2d image_container::image_pool::image_size() const {
      return img_size;
    }
    
    uint32_t image_container::image_pool::mip_levels() const {
      return mips;
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
      return layers;
    }
    
    const size_t image_container::image_pool::max_size;
    
    image_container::image_container(const create_info &info) : device(info.device), physical_device(info.physical_device) {
      const auto null_info = texture2D({1, 1}, vk::ImageUsageFlagBits::eSampled);
      const auto [img, memory] = create_image(*device, *physical_device, null_info, vk::MemoryPropertyFlagBits::eDeviceLocal);
      const auto null_view_info = make_view_info(img, vk::Format::eR8G8B8A8Unorm, vk::ImageViewType::e2DArray);
      null_image = img;
      null_memory = memory;
      null_image_view = device->createImageView(null_view_info);
      
      const vk::ImageSubresourceRange range(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
      change_image_layout(*device, null_image, *info.transfer_command_pool, *info.queue, *info.fence, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, range);
    }
    
    image_container::~image_container() {
      for (auto pool : slots) {
        if (pool == nullptr) continue;
        //device->destroy(pool->image);
        image_memory.destroy(pool);
      }
      
      device->destroy(null_image);
      device->destroy(null_image_view);
      device->free(null_memory);
      null_image = nullptr;
      null_image_view = nullptr;
      null_memory = nullptr;
    }
    
    void image_container::set_slots(const size_t &slots) {
      if (this->slots.size() > slots) throw std::runtime_error("Removing slots this way is undefined");
      this->slots.resize(slots, nullptr);
    }
    
    size_t image_container::pool_count() const {
      return slots.size();
    }
    
    size_t image_container::first_empty_pool() const {
      for (size_t i = 0; i < slots.size(); ++i) {
        if (pool_exists(i)) continue;
        return i;
      }
      
      return SIZE_MAX;
    }
    
    bool image_container::pool_exists(const size_t &index) const {
      if (index >= slots.size()) return false;
      return slots[index] != nullptr;
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
    
    uint32_t image_container::reserve_image(const size_t &pool_index) {
      if (pool_index >= slots.size()) return GPU_UINT_MAX;
      const uint32_t free_index = slots[pool_index]->occupy_free_index();
      return free_index;
    }
    
    void image_container::release_image(const render::image_t &img) {
      const auto index = get_image_index(img);
      if (index >= slots.size()) throw std::runtime_error("Trying to release bad image");
      slots[index]->release_index(get_image_layer(img));
    }
    
    void image_container::create_pool(const uint32_t &slot_index, const extent2d &img_size, const uint32_t &mips, const uint32_t &layers) {
      if (slot_index > slots.size()) throw std::runtime_error("Bad image slot index");
      if (slot_index == slots.size()) slots.push_back(nullptr);
      if (slots[slot_index] != nullptr) throw std::runtime_error("Replacing an existing slot is not allowed");
      auto ptr = image_memory.create(device, physical_device, img_size, mips, layers);
      slots[slot_index] = ptr;
    }
    
    void image_container::destroy_pool(const uint32_t &slot_index) {
      if (slot_index >= slots.size()) throw std::runtime_error("Destruction of uncreated slot");
      //if (slots[slot_index] == nullptr) throw std::runtime_error("Destruction of uncreated slot");
      if (slots[slot_index] == nullptr) return;
      
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
    
    void image_container::update_descriptor_data(vk::DescriptorSet* set) {
      //if (slots.size() >= set->size()) throw std::runtime_error("slots.size() >= set->size()");
      descriptor_set_updater dsu(device);
      
      dsu.currentSet(*set);
      for (size_t i = 0; i < slots.size(); ++i) {
        // мы можем добавить специальную нулл текстурку 1х1, тем самым моделируя отсутствие данных в этом слоте
        auto view = slots[i] == nullptr ? null_image_view : vk::ImageView(slots[i]->view);
        dsu.begin(0, i, vk::DescriptorType::eSampledImage).image(view, vk::ImageLayout::eShaderReadOnlyOptimal);
      }
      
      dsu.update();
    }
    
    size_t image_container::memory() const {
      size_t counter = 0;
      for (auto slot : slots) {
        if (slot == nullptr) continue;
        const uint32_t layers = slot->layers_count();
        const bool mip = slot->mip_levels() != 1;
        const auto size = slot->image_size();
        const size_t one_image_size = size.width * size.height * 4;
        const size_t capacity = (one_image_size + mip * one_image_size / 2) * layers;
        counter += capacity;
      }
      
      return counter;
    }
  }
}
