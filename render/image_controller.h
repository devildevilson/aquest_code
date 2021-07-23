#ifndef IMAGE_CONTROLLER_H
#define IMAGE_CONTROLLER_H

#include <unordered_map>
#include <string>
#include "shared_structures.h"
#include "utils/memory_pool.h"
#include "vulkan_declarations.h"

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

// такой дизайн image_controller вынуждает меня указать максимальный размер изображения каждого типа
// как иначе? иначе у нас может быть много слотов с очень разными текстурками
// но не очень понятна целесообразность текстур супер высокого разрешения в этой игре
// с другой стороны текстурки разрешения побольше могут пригодиться
// в принципе я могу пихнуть в слот текстурки любых разрешений
// скорее я буду вынужден указать максимальное количество слотов

namespace devils_engine {
  namespace render {
    class image_container;
    struct internal;
    
    bool parse_image_id(const std::string_view &str, std::string_view &image_id, uint32_t &layer_index, bool &mirror_u, bool &mirror_v);
    
    struct image_controller {
      enum class image_type {
        system,          // наверное потребуется отделить некоторые системные изображения
        icon,            // иконок в игре поди будет много, нужно ли сократить их количество в разных игровых состояний (скейлятся 32х32)
        heraldy,         // геральдика скорее всего нужна будет везде, но кажется она займет не очень много места       (тоже должна скейлиться, но наверное больше чем иконки)
        face,            // тут аккуратно, может быть много изображений потребуется                                     (было бы неплохо вообще разделить типы + эти изображения скейлятся)
        card,            // нужны видимо всегда (хотя в геройских битвах вряд ли)                                       (скейлить толку не очень много, но размер у карточек скорее всего будет не квадрат)
        common,          // нужно только в меню и при загрузках                                                         (скейлятся по идее по размеру окна)
        state,           // в каких битвах? скорее всего и в тех и других                                               (тут не особо нужны текстурки большого разрешения)
        encounter_biome, // вся информация о биоме будет под этим типом?                                                (с биомами сложно, они составные)
        battle_biome,
        world_biome,     
        architecture,    // здесь я скорее всего смогу понять только уже когда у меня будет большое количество контента
        count
      };
      
      enum registered_slots { // все слоты укажем? могу я предугадать их количество?
        font_atlas_slot,    // system
        distance_font_slot, // system
        style_slot,         // system
        background_slot,    // common
        icon_slot,
        icon_slot_max = icon_slot+4,      // до 256*4 иконок? такое количество хватит скорее всего в любых условиях
        
        registered_slots_count // эти слоты должны быть зарегистрированы по умолчанию
      };
      
      struct container_view {
        image_container* container;
        uint32_t slot;
        uint32_t offset;
        uint32_t count; // count + offset может выходить за пределы UINT8_MAX, это означает что мы берем следующий слот
        image_type type;
        uint32_t sampler;
        
        std::tuple<uint32_t, uint32_t> get_size() const;
        render::image_t get_image(const size_t &index, const bool mirror_u, const bool mirror_v) const;
      };
      
      image_container* container;
      internal* vulkan;
      std::unordered_map<std::string, container_view> image_slots; // строки пихнуть в мемори пул?
      size_t current_slot_offset;
      
      image_controller(vk::Device* device, image_container* container);
      ~image_controller();
      
      const container_view* get_view(const std::string &name) const;
      
      void clear_type(const image_type &type);
      // несколько функций которые создадут несколько слотов и их размер
      // вся эта информация хранится в луа таблицах
      struct input_data {
        std::string name;
        image_type type;   // тип нам поможет найти подходящий слот 
        uint32_t count;  
        uint32_t slot;
        uint32_t offset;
        uint32_t sampler;
//         uint32_t width;
//         uint32_t height;
//         bool mipmap;
      };
      
//       struct output_data {
//         uint32_t slot;
//         uint32_t offset;
//       };
      //output_data create_slot(const input_data &data); // так только мы их создадим
      void register_images(const input_data &data); // пока что просто заполняем массив
      // как заполнить? все что нам нужно сделать это скопировать все изображения
      // по слотам в контейнер, может нужно сначало зарегистрировать пачку изображений
      //void create_slots();
      // грузить наверное будем не здесь, тут лучше просто организовать доступ
      
      void update_set();
      void update_sampler_set();
      size_t get_images_size() const;
      
      vk::DescriptorSet* get_descriptor_set() const;
      vk::DescriptorSetLayout* get_descriptor_set_layout() const;
    };
  }
}

#endif
