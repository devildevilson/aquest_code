#ifndef IMAGE_CONTROLLER_H
#define IMAGE_CONTROLLER_H

#include <unordered_map>
#include <string>
#include "shared_structures.h"

// тут у нас должны храниться названия изображений и доступ к ним
// как мы загружаем изображения? по идее описываем таблицу 
// тип, путь, id, количество, 

// нам бы как нибудь сохранить данные загрузки картинок, состояний персонажа и проч
// ну как как по таблицам, будем подгружать world_data? можно, где еще их хранить если не на диске?
// у нас в типе героя должно быть описание его состояний, 
// состояние1 {время, следующее, функция, картинка, }

namespace devils_engine {
  namespace render {
    class image_container;
    
    struct image_controller {
      enum registered_slots {
        background_slot,
        font_atlas_slot,
        distance_font_slot,
        style_slot,
        
        count
      };
      
      struct container_view {
        image_container* container;
        size_t offset;
        size_t slot;
        uint32_t type;
        
        std::pair<uint32_t, uint32_t> get_size() const;
        render::image_t get_image(const size_t &index, const bool mirror_u, const bool mirror_v) const;
      };
      
      image_container* container;
      std::unordered_map<std::string, size_t> image_slots; // несколько слотов должны быть техническими
      
      image_controller(image_container* container);
      
      container_view get_view(const std::string &name) const;
    };
  }
}

#endif
