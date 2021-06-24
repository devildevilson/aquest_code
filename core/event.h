#ifndef EVENT_H
#define EVENT_H

#include <string>
#include <array>
#include "declare_structures.h"
#include "render/shared_structures.h"

namespace devils_engine {
  namespace core {
    // эвенты можно генерировать даже нужно, изменятся основные данные
    // данные эвента не должны меняться по ходу игры вообще
    // константные эвенты удобнее использовать с std vector
    struct event {
      static const structure s_type = structure::event;
      static const size_t max_options_count = 8;
      
      struct option {
        size_t name_id; // наверное и названия нужно скомпилировать
        size_t desc_id;
//         std::vector<utils::functions_container::operation> conditions; // наверное можно использовать статический массив
//         std::vector<utils::functions_container::operation> effects; // как сгенерить описание?
        
        option();
      };
      
      std::string id;
      size_t name_id; // индекс строки должен состоять из индекса банка и индекса собственно строки
      size_t description_id;
//       enum utils::target_data::type target;
      render::image_t image;
      size_t mtth; // среднее время возникновения, должны быть модификаторы для этого 
      //std::function<bool(const target_data&)> potential;
      uint32_t options_count;
//       std::vector<utils::functions_container::operation> conditions; // средняя длинна скорее всего не превышает 16 (может и меньше) 
      std::array<option, max_options_count> options;
      
      event();
      
      void fire(struct character* c);
    };
  }
}

#endif
