#include "battle_map.h"

#include "render/container.h"
#include "render/vulkan_hpp_header.h"
#include "render/shared_battle_structures.h"
#include "render/makers.h"

namespace devils_engine {
  namespace render {
    struct battle_map_data {
      container* cont;
      
      // нужен наверное какой нибудь юниформ буфер, буфер оффсетов это не юниформ буфер? вряд ли
      // как проверять эти тайлы с фрустумом? (сфера = скорость, но тогда проверять каждый тайл?)
      // в циве 5 на самой большой карте используется 128×80 тайлов (10240 всего), это хорошее число
      // думаю что на этот предел и надо ориентироваться, другое дело что удастся ли уместить все отряды на такую карту
      // или придется ее расширить? с этим проблем быть особенно не должно, но в базовой игре нужно постараться уместить
      // мне нужно будет придумать чем я заполню пустые пространства вне карты + наверное как то ограничить камеру
      // камера по идее легко ограничивается боксом мин макс
      vk_buffer_data tiles_uniform; // пригодится
      vk_buffer_data tiles_buffer;
      vk_buffer_data offsets_buffer;
      vk_buffer_data biomes_buffer;
      // сколько выделять для юнитов
      vk_buffer_data units_buffer;
      // тип нужно последовательно добавить все текстурки
      vk_buffer_data textures_buffer;
      
      vk::DescriptorPool pool;
      vk::DescriptorSetLayout layout;
      vk::DescriptorSet set;
      
      inline battle_map_data(container* cont) : cont(cont) {}
      inline ~battle_map_data() {
        auto allocator = cont->vulkan->buffer_allocator;
        auto device = cont->vulkan->device;
        
        tiles_uniform.destroy(allocator);
        tiles_buffer.destroy(allocator);
        offsets_buffer.destroy(allocator);
        biomes_buffer.destroy(allocator);
        units_buffer.destroy(allocator);
        textures_buffer.destroy(allocator);
        
        device.destroy(pool);
        device.destroy(layout);
      }
    };
  }
  
  namespace battle {
    
    
    map::map(const create_info &info) : 
      width(0), 
      height(0), 
      tiles_count(0), 
      units_count(0),
      textures_count(0),
      data(new render::battle_map_data(info.cont))
    {}
    
    map::~map() {}
    
    void map::create_tiles(const uint32_t &width, const uint32_t &height, const coordinate_system &system, const orientation &orient) {
      type.set(0, system == coordinate_system::square);
      type.set(1, orient == orientation::even_flat || orient == orientation::odd_flat);
      type.set(2, orient == orientation::odd_flat || orient == orientation::odd_pointy);
      auto allocator = data->cont->vulkan->buffer_allocator;
      auto device = data->cont->vulkan->device;
      
      if (is_square()) {
        tiles_count = width * height;
        this->width = width;
        this->height = height;
        // тут по идее просто width*height, просто нужно понять какими системами мы пользуемся
        // в этом случае равенство: y * height + x = tile_index
        data->tiles_uniform.create(allocator, render::buffer(sizeof(uniform_buffer_data), vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst), vma::MemoryUsage::eGpuOnly);
        data->tiles_buffer.create(allocator, render::buffer(tiles_count*sizeof(render::battle_map_tile_data_t), vk::BufferUsageFlagBits::eStorageBuffer), vma::MemoryUsage::eCpuOnly);
        data->offsets_buffer.create(allocator, render::buffer(width*height*sizeof(render::battle_map_tile_data_t), vk::BufferUsageFlagBits::eStorageBuffer), vma::MemoryUsage::eCpuOnly);
        data->biomes_buffer.create(
          allocator,
          render::buffer(
            BATTLE_BIOMES_MAX_COUNT*sizeof(render::battle_biome_data_t), 
            vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst
          ), 
          vma::MemoryUsage::eGpuOnly
        );
        data->units_buffer.create(allocator, render::buffer(1*sizeof(render::unit_t), vk::BufferUsageFlagBits::eStorageBuffer), vma::MemoryUsage::eCpuOnly);
        // короч думаю прост надо сделать сначала квадрат, а потом если захочу гекс (но по идее ничего сложного, но нужно париться в шейдерах с изменением формул)
        
        auto tiles_array_ptr = reinterpret_cast<render::battle_map_tile_data_t*>(data->tiles_buffer.ptr);
        for (size_t i = 0; i < tiles_count; ++i) {
          tiles_array_ptr[i].height = 1.0f;
          tiles_array_ptr[i].ground = render::image_t{GPU_UINT_MAX};
          tiles_array_ptr[i].walls = render::image_t{GPU_UINT_MAX};
          tiles_array_ptr[i].biome_index = GPU_UINT_MAX;
          tiles_array_ptr[i].troop_data = GPU_UINT_MAX;
        }
        
        auto staging = render::create_buffer_unique(allocator, render::buffer(sizeof(uniform_buffer_data), vk::BufferUsageFlagBits::eTransferSrc), vma::MemoryUsage::eCpuOnly);
        auto data_uniform = reinterpret_cast<uniform_buffer_data*>(staging.ptr);
        data_uniform->map_properties.x = tiles_count;
        data_uniform->map_properties.y = width;
        data_uniform->map_properties.z = height;
        data_uniform->map_properties.w = type.container[0];
        
        auto pool = data->cont->vulkan->transfer_command_pool;
        auto fence = data->cont->vulkan->transfer_fence;
        auto queue = data->cont->vulkan->graphics;
        
        render::do_command(
          device, pool, queue, fence, 
          [&] (vk::CommandBuffer task) {
            const vk::BufferCopy c{0, 0, sizeof(uniform_buffer_data)};
            const vk::CommandBufferBeginInfo b(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
            task.begin(b);
            task.copyBuffer(staging.handle, data->tiles_uniform.handle, c);
            task.end();
          }
        );
      } else {
        // наращиваем слои до min(width, height), и дальше наращиваем часть слоя в высоту/ширину
        // тут мы используем аксиальные координаты + буфер оффсетов
        assert(false);
      }
      
      // мне нужен быстрый доступ по координатам к тайлу и в том и в другом случае + позиция тайла
      // по идее для этого нужно сделать формулы перевода из индекса к позиции
      
      {
        render::descriptor_set_layout_maker dlm(&device);
        data->layout = dlm.binding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eAll)
                          .binding(1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eAll)
                          .binding(2, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eAll)
                          .binding(3, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eAll)
                          .binding(4, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eAll)
                          .binding(5, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eAll)
                          .create(BATTLE_MAP_DESCRIPTOR_SET_LAYOUT_NAME);
      }
      
      {
        render::descriptor_pool_maker dpm(&device);
        data->pool = dpm.poolSize(vk::DescriptorType::eStorageBuffer, 6).poolSize(vk::DescriptorType::eUniformBuffer, 1).create(BATTLE_MAP_DESCRIPTOR_POOL_NAME);
      }
      
      {
        render::descriptor_set_maker dm(&device);
        data->set = dm.layout(data->layout).create(data->pool)[0];
        
        render::descriptor_set_updater dsu(&device);
        dsu.currentSet(data->set);
        dsu.begin(0, 0, vk::DescriptorType::eUniformBuffer).buffer(data->tiles_uniform.handle);
        dsu.begin(1, 0, vk::DescriptorType::eStorageBuffer).buffer(data->tiles_buffer.handle);
        dsu.begin(2, 0, vk::DescriptorType::eStorageBuffer).buffer(data->offsets_buffer.handle);
        dsu.begin(3, 0, vk::DescriptorType::eStorageBuffer).buffer(data->biomes_buffer.handle);
        //dsu.begin(4, 0, vk::DescriptorType::eStorageBuffer).buffer(data->textures_buffer.handle); // этот буфер создаем позже
        dsu.begin(5, 0, vk::DescriptorType::eStorageBuffer).buffer(data->units_buffer.handle);
        dsu.update();
      }
    }
    
    // писать в буффер вместе с рендером не рекомендуется 
    // (можно ли писать при добавлении в командный буфер? я бы не стал, но по идее буффер должен быть заблокирован при непосредственной отрисовке)
    // а вот читать по идее можно всегда
    
    void map::set_tile_height(const uint32_t &tile_index, const float &height) {
      if (tile_index >= tiles_count) throw std::runtime_error("Bad tile index");
      std::unique_lock<std::mutex> lock(mutex);
      auto array = reinterpret_cast<render::battle_map_tile_data_t*>(data->tiles_buffer.ptr);
      array[tile_index].height = height;
    }
    
    float map::get_tile_height(const uint32_t &tile_index) const {
      if (tile_index >= tiles_count) throw std::runtime_error("Bad tile index");
      auto array = reinterpret_cast<render::battle_map_tile_data_t*>(data->tiles_buffer.ptr);
      return array[tile_index].height;
    }
    
    void map::set_tile_biome(const uint32_t &tile_index, const uint32_t &biome_index) {
      if (tile_index >= tiles_count) throw std::runtime_error("Bad tile index");
      std::unique_lock<std::mutex> lock(mutex);
      auto array = reinterpret_cast<render::battle_map_tile_data_t*>(data->tiles_buffer.ptr);
      array[tile_index].biome_index = biome_index;
    }
    
    uint32_t map::get_tile_biome(const uint32_t &tile_index) const {
      if (tile_index >= tiles_count) throw std::runtime_error("Bad tile index");
      auto array = reinterpret_cast<render::battle_map_tile_data_t*>(data->tiles_buffer.ptr);
      return array[tile_index].biome_index;
    }
    
    void map::set_units_count(const uint32_t &count) {
      units_count = count;
      
      std::unique_lock<std::mutex> lock(mutex);
      auto allocator = data->cont->vulkan->buffer_allocator;
      auto device = data->cont->vulkan->device;
      const size_t size = align_to(count*sizeof(render::unit_t), 16);
      data->units_buffer.destroy(allocator);
      data->units_buffer.create(allocator, render::buffer(size, vk::BufferUsageFlagBits::eStorageBuffer), vma::MemoryUsage::eCpuOnly);
      
      render::descriptor_set_updater dsu(&device);
      dsu.currentSet(data->set);
      dsu.begin(5, 0, vk::DescriptorType::eStorageBuffer).buffer(data->units_buffer.handle);
      dsu.update();
    }
    
    render::unit_t map::get_unit_data(const uint32_t &index) const {
      // мьютекс?
      ASSERT(index < units_count);
      auto units = reinterpret_cast<render::unit_t*>(data->units_buffer.ptr);
      return units[index];
    }
    
    void map::set_unit_data(const uint32_t &index, const render::unit_t &data) {
      ASSERT(index < units_count);
      auto units = reinterpret_cast<render::unit_t*>(this->data->units_buffer.ptr);
      units[index] = data;
    }
    
    void map::set_tile_troop_data(const uint32_t &index, const uint32_t &data) {
      if (index >= tiles_count) throw std::runtime_error("Bad tile index");
      std::unique_lock<std::mutex> lock(mutex);
      auto array = reinterpret_cast<render::battle_map_tile_data_t*>(this->data->tiles_buffer.ptr);
      array[index].troop_data = data;
    }
    
    uint32_t map::get_tile_troop_data(const uint32_t &index) const {
      if (index >= tiles_count) throw std::runtime_error("Bad tile index");
      auto array = reinterpret_cast<render::battle_map_tile_data_t*>(data->tiles_buffer.ptr);
      return array[index].troop_data;
    }
    
    void map::set_biomes(const std::array<render::battle_biome_data_t, BATTLE_BIOMES_MAX_COUNT> &data) {
      const size_t data_size = BATTLE_BIOMES_MAX_COUNT*sizeof(render::battle_biome_data_t);
      ASSERT(data_size == sizeof(data[0])*data.size());
      ASSERT(data[0].density == 52.0f);
      auto allocator = this->data->cont->vulkan->buffer_allocator;
      auto device = this->data->cont->vulkan->device;
      
      auto staging = render::create_buffer_unique(allocator, render::buffer(data_size, vk::BufferUsageFlagBits::eTransferSrc), vma::MemoryUsage::eCpuOnly);
      memcpy(staging.ptr, data.data(), data_size);
      
      std::unique_lock<std::mutex> lock(mutex);
      
      auto pool = this->data->cont->vulkan->transfer_command_pool;
      auto fence = this->data->cont->vulkan->transfer_fence;
      auto queue = this->data->cont->vulkan->graphics;
      
      render::do_command(
        device, pool, queue, fence, 
        [&] (vk::CommandBuffer task) {
          const vk::BufferCopy c{0, 0, data_size};
          const vk::CommandBufferBeginInfo b(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
          task.begin(b);
          task.copyBuffer(staging.handle, this->data->biomes_buffer.handle, c);
          task.end();
        }
      );
    }
    
    void map::add_unit_textures(const std::vector<render::image_t> &array) {
      auto allocator = this->data->cont->vulkan->buffer_allocator;
      auto device = this->data->cont->vulkan->device;
      const size_t size = align_to(array.size() * sizeof(render::image_t), 16);
      auto staging = render::create_buffer_unique(allocator, render::buffer(size, vk::BufferUsageFlagBits::eTransferSrc), vma::MemoryUsage::eCpuOnly);
      memcpy(staging.ptr, array.data(), array.size() * sizeof(render::image_t));
      
      std::unique_lock<std::mutex> lock(mutex);
      auto pool = this->data->cont->vulkan->transfer_command_pool;
      auto fence = this->data->cont->vulkan->transfer_fence;
      auto queue = this->data->cont->vulkan->graphics;
      data->textures_buffer.create(allocator, render::buffer(size, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst), vma::MemoryUsage::eGpuOnly);
      
      render::do_command(
        device, pool, queue, fence, 
        [&] (vk::CommandBuffer task) {
          const vk::BufferCopy c{0, 0, size};
          const vk::CommandBufferBeginInfo b(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
          task.begin(b);
          task.copyBuffer(staging.handle, this->data->textures_buffer.handle, c);
          task.end();
        }
      );
      
      render::descriptor_set_updater dsu(&device);
      dsu.begin(4, 0, vk::DescriptorType::eStorageBuffer).buffer(data->textures_buffer.handle);
      dsu.update();
    }
    
    glm::vec3 map::get_tile_pos(const uint32_t &tile_index) const {
      static const glm::vec2 hex_map_row_const_offset[] = {
        glm::vec2(-0.5f, 0.0f),
        glm::vec2( 0.5f, 0.0f),
        glm::vec2( 0.0f,-0.5f),
        glm::vec2( 0.0f, 0.5f)
      };
      
      if (tile_index >= tiles_count) throw std::runtime_error("Bad tile index");
      
      const uint32_t row_index    = tile_index / height;
      const uint32_t column_index = tile_index % height;
      const glm::uvec2 tile_coord = glm::uvec2(column_index, row_index);
      //const uvec2 tile_coord = uvec2(0, 0);

      const float hex_size = 1.0f;
      const float hex_width = glm::mix(glm::sqrt(3.0f) * hex_size, 2.0f * hex_size, float(is_flat()));
      const float hex_height = glm::mix(2.0f * hex_size, glm::sqrt(3.0f) * hex_size, float(is_flat()));
      const float hex_width_dist = glm::mix(1.0f * hex_width, (3.0f/4.0f) * hex_width, float(is_flat()));
      const float hex_height_dist = glm::mix((3.0f/4.0f) * hex_height, 1.0f * hex_height, float(is_flat()));

      const uint32_t offset_type_index = uint32_t(is_odd()) + 2 * uint32_t(is_flat());

      // 4 константы зависят от того как мы представляем координатные системы
      // квадратная карта может быть представлена: четный оффсет по строкам, нечетный оффсет по строкам, четный оффсет по столбцам, нечетный оффсет по столбцам
      const uint32_t row_column = uint32_t(is_flat()) * column_index + uint32_t(!is_flat()) * row_index;
      const glm::vec2 const_pos_k = float(row_column % 2 == 1) * hex_map_row_const_offset[offset_type_index] * glm::vec2(hex_width_dist, hex_height_dist);

      const glm::vec2 tile_pos = glm::vec2(tile_coord) * glm::vec2(hex_width_dist, hex_height_dist) + const_pos_k;
      
      const float h = get_tile_height(tile_index);
      return glm::vec3(tile_pos.x, h, tile_pos.y);
    }
    
    vk::DescriptorSet* map::get_descriptor_set() const { return &data->set; }
    vk::DescriptorSetLayout* map::get_descriptor_set_layout() const { return &data->layout; }
    
    void* map::get_tiles_buffer_memory() const {
      return data->tiles_buffer.ptr;
    }
  }
}
