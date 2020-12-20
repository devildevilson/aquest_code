#include "battle_map.h"

#include "render/yavf.h"
#include "render/shared_battle_structures.h"

namespace devils_engine {
  namespace battle {
    map::map(const create_info &info) : width(0), height(0), tiles_count(0), device(info.device), tiles_buffer(nullptr), offsets_buffer(nullptr), set(nullptr) {
      
    }
    
    map::~map() {
      device->destroy(tiles_buffer);
      device->destroy(offsets_buffer);
    }
    
    void map::create_tiles(const uint32_t &width, const uint32_t &height, const coordinate_system &system, const orientation &orient) {
      type.set(0, system == coordinate_system::square);
      type.set(1, orient == orientation::even_flat || orient == orientation::odd_flat);
      type.set(2, orient == orientation::odd_flat || orient == orientation::odd_pointy);
      
      if (is_square()) {
        tiles_count = width * height;
        // тут по идее просто width*height, просто нужно понять какими системами мы пользуемся
        // в этом случае равенство: y * height + x = tile_index
        tiles_buffer = device->create(yavf::BufferCreateInfo::buffer(tiles_count*sizeof(render::battle_map_tile_data_t), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
        offsets_buffer = device->create(yavf::BufferCreateInfo::buffer(width*height*sizeof(render::battle_map_tile_data_t), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
        // короч думаю прост надо сделать сначала квадрат, а потом если захочу гекс (но по идее ничего сложного, но нужно париться в шейдерах с изменением формул)
      } else {
        // наращиваем слои до min(width, height), и дальше наращиваем часть слоя в высоту/ширину
        // тут мы используем аксиальные координаты + буфер оффсетов
      }
      
      // мне нужен быстрый доступ по координатам к тайлу и в том и в другом случае + позиция тайла
      // по идее для этого нужно сделать формулы перевода из индекса к позиции
    }
  }
}
