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

// тут как раз нам нужен дескриптор пул и лайоут
// они будут обновляться при загрузках
// в ворлд дата будут хранится все таблицы, в том числе и с типами войск и героев
// с этим понятно, разделяем картинки по типам со строгим временем жизни
// как быть с картинкой на заднем фоне? было бы неплохо завести и для этого луа функцию
// как чистить? только по типам?

namespace devils_engine {
  namespace render {
    class image_container;
    
    struct image_controller {
      class internal;
      
      enum class image_type {
        system,          // наверное потребуется отделить некоторые системные изображения
        icon,            // иконок в игре поди будет много, нужно ли сократить их количество в разных игровых состояний
        heraldy,         // геральдика скорее всего нужна будет везде, но кажется она займет не очень много места 
        face,            // тут аккуратно, может быть много изображений потребуется 
        card,            // нужны видимо всегда (хотя в геройских битвах вряд ли)
        common,          // нужно только в меню и при загрузках
        state,           // в каких битвах? скорее всего и в тех и других
        encounter_biome, // вся информация о биоме будет под этим типом?
        battle_biome,
        world_biome,     
        architecture,    // здесь я скорее всего смогу понять только уже когда у меня будет большое количество контента
        count
      };
      
      enum registered_slots { // все слоты укажем? могу я предугадать их количество?
        background_slot,
        font_atlas_slot,
        distance_font_slot,
        style_slot,
        
        registered_slots_count
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
      internal* data;
      std::unordered_map<std::string, size_t> image_slots; // несколько слотов должны быть техническими
      
      image_controller(image_container* container);
      ~image_controller();
      
      const container_view* get_view(const std::string &name) const;
      
      void clear_type(const image_type &type);
      // несколько функций которые создадут несколько слотов и их размер
      // вся эта информация хранится в луа таблицах
    };
  }
}

#endif
