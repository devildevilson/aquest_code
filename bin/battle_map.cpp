#include "battle_map.h"

#include "render/yavf.h"
#include "render/shared_battle_structures.h"

namespace devils_engine {
  namespace battle {
    map::map(const create_info &info) : 
      width(0), 
      height(0), 
      tiles_count(0), 
      units_count(0),
      textures_count(0),
      device(info.device), 
      tiles_buffer(nullptr), 
      offsets_buffer(nullptr), 
      biomes_buffer(nullptr), 
      units_buffer(nullptr), 
      textures_buffer(nullptr),
      set(nullptr) 
    {}
    
    map::~map() {
      device->destroy(tiles_uniform);
      device->destroy(tiles_buffer);
      device->destroy(offsets_buffer);
      device->destroy(biomes_buffer);
      if (units_buffer != nullptr) device->destroy(units_buffer);
      
      device->destroySetLayout(BATTLE_MAP_DESCRIPTOR_SET_LAYOUT_NAME);
      device->destroyDescriptorPool(BATTLE_MAP_DESCRIPTOR_POOL_NAME);
    }
    
    void map::create_tiles(const uint32_t &width, const uint32_t &height, const coordinate_system &system, const orientation &orient) {
      type.set(0, system == coordinate_system::square);
      type.set(1, orient == orientation::even_flat || orient == orientation::odd_flat);
      type.set(2, orient == orientation::odd_flat || orient == orientation::odd_pointy);
      
      if (is_square()) {
        tiles_count = width * height;
        this->width = width;
        this->height = height;
        // тут по идее просто width*height, просто нужно понять какими системами мы пользуемся
        // в этом случае равенство: y * height + x = tile_index
        tiles_uniform = device->create(yavf::BufferCreateInfo::buffer(sizeof(uniform_buffer_data), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT), VMA_MEMORY_USAGE_GPU_ONLY);
        tiles_buffer = device->create(yavf::BufferCreateInfo::buffer(tiles_count*sizeof(render::battle_map_tile_data_t), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
        offsets_buffer = device->create(yavf::BufferCreateInfo::buffer(width*height*sizeof(render::battle_map_tile_data_t), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
        biomes_buffer = device->create(
          yavf::BufferCreateInfo::buffer(
            BATTLE_BIOMES_MAX_COUNT*sizeof(render::battle_biome_data_t), 
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
          ), 
          VMA_MEMORY_USAGE_GPU_ONLY
        );
        // короч думаю прост надо сделать сначала квадрат, а потом если захочу гекс (но по идее ничего сложного, но нужно париться в шейдерах с изменением формул)
        
        yavf::Buffer staging(device, yavf::BufferCreateInfo::buffer(sizeof(uniform_buffer_data), VK_BUFFER_USAGE_TRANSFER_SRC_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
        auto data = reinterpret_cast<uniform_buffer_data*>(staging.ptr());
        data->map_properties.x = tiles_count;
        data->map_properties.y = width;
        data->map_properties.z = height;
        data->map_properties.w = type.container[0];
        
        std::unique_lock<std::mutex> lock(mutex);
        
        auto task = device->allocateTransferTask();
        task->begin();
        task->copy(&staging, tiles_uniform);
        task->end();
        
        task->start();
        task->wait();
        device->deallocate(task);
      } else {
        // наращиваем слои до min(width, height), и дальше наращиваем часть слоя в высоту/ширину
        // тут мы используем аксиальные координаты + буфер оффсетов
      }
      
      // мне нужен быстрый доступ по координатам к тайлу и в том и в другом случае + позиция тайла
      // по идее для этого нужно сделать формулы перевода из индекса к позиции
      
      yavf::DescriptorSetLayout layout = VK_NULL_HANDLE;
      {
        yavf::DescriptorLayoutMaker dlm(device);
        layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL)
                    .binding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL)
                    .binding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL)
                    .binding(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL)
                    .binding(4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL)
                    .create(BATTLE_MAP_DESCRIPTOR_SET_LAYOUT_NAME);
      }
      
      yavf::DescriptorPool pool = VK_NULL_HANDLE;
      {
        yavf::DescriptorPoolMaker dpm(device);
        pool = dpm.poolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 5).poolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1).create(BATTLE_MAP_DESCRIPTOR_POOL_NAME);
      }
      
      {
        yavf::DescriptorMaker dm(device);
        set = dm.layout(layout).create(pool)[0];
      }
      
      set->add({tiles_uniform, 0, tiles_uniform->info().size, 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER});
      set->add({tiles_buffer, 0, tiles_buffer->info().size, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
      set->add({offsets_buffer, 0, offsets_buffer->info().size, 0, 2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
      set->add({biomes_buffer, 0, biomes_buffer->info().size, 0, 3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
      set->update();
    }
    
    // писать в буффер вместе с рендером не рекомендуется 
    // (можно ли писать при добавлении в командный буфер? я бы не стал, но по идее буффер должен быть заблокирован при непосредственной отрисовке)
    // а вот читать по идее можно всегда
    
    void map::set_tile_height(const uint32_t &tile_index, const float &height) {
      if (tile_index >= tiles_count) throw std::runtime_error("Bad tile index");
      std::unique_lock<std::mutex> lock(mutex);
      auto array = reinterpret_cast<render::battle_map_tile_data_t*>(tiles_buffer->ptr());
      array[tile_index].height = height;
    }
    
    float map::get_tile_height(const uint32_t &tile_index) const {
      if (tile_index >= tiles_count) throw std::runtime_error("Bad tile index");
      auto array = reinterpret_cast<render::battle_map_tile_data_t*>(tiles_buffer->ptr());
      return array[tile_index].height;
    }
    
    void map::set_tile_biome(const uint32_t &tile_index, const uint32_t &biome_index) {
      if (tile_index >= tiles_count) throw std::runtime_error("Bad tile index");
      std::unique_lock<std::mutex> lock(mutex);
      auto array = reinterpret_cast<render::battle_map_tile_data_t*>(tiles_buffer->ptr());
      array[tile_index].biome_index = biome_index;
    }
    
    uint32_t map::get_tile_biome(const uint32_t &tile_index) const {
      if (tile_index >= tiles_count) throw std::runtime_error("Bad tile index");
      auto array = reinterpret_cast<render::battle_map_tile_data_t*>(tiles_buffer->ptr());
      return array[tile_index].biome_index;
    }
    
    void map::set_units_count(const uint32_t &count) {
      units_count = count;
      units_buffer = device->create(yavf::BufferCreateInfo::buffer(count*sizeof(render::unit_t), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
    }
    
    render::unit_t map::get_unit_data(const uint32_t &index) const {
      // мьютекс?
      ASSERT(index < units_count);
      auto units = reinterpret_cast<render::unit_t*>(units_buffer->ptr());
      return units[index];
    }
    
    void map::set_unit_data(const uint32_t &index, const render::unit_t &data) {
      ASSERT(index < units_count);
      auto units = reinterpret_cast<render::unit_t*>(units_buffer->ptr());
      units[index] = data;
    }
    
    void map::set_biomes(const std::array<render::battle_biome_data_t, BATTLE_BIOMES_MAX_COUNT> &data) {
      const size_t data_size = BATTLE_BIOMES_MAX_COUNT*sizeof(render::battle_biome_data_t);
      ASSERT(data_size == sizeof(data[0])*data.size());
      ASSERT(data[0].density == 52.0f);
      yavf::Buffer staging(device, yavf::BufferCreateInfo::buffer(data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
      memcpy(staging.ptr(), data.data(), data_size);
      
      std::unique_lock<std::mutex> lock(mutex);
      
      auto task = device->allocateTransferTask();
      task->begin();
      task->copy(&staging, biomes_buffer);
      task->end();
      
      task->start();
      task->wait();
      
      device->deallocate(task);
      
      //biomes_buffer->flush();
    }
    
    void map::add_unit_textures(const std::vector<render::image_t> &array) {
      const size_t size = align_to(array.size() * sizeof(render::image_t), 16);
      yavf::Buffer staging(device, yavf::BufferCreateInfo::buffer(size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
      memcpy(staging.ptr(), array.data(), array.size() * sizeof(render::image_t));
      
      std::unique_lock<std::mutex> lock(mutex);
      textures_buffer = device->create(yavf::BufferCreateInfo::buffer(size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
      
      auto task = device->allocateTransferTask();
      task->begin();
      task->copy(&staging, textures_buffer);
      task->end();
      task->start();
      task->wait();
      device->deallocate(task);
      
      set->add({textures_buffer, 0, textures_buffer->info().size, 0, 4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
      set->update();
      
      // что еще?
    }
  }
}
