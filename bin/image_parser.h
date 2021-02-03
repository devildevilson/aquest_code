#ifndef IMAGE_PARSER_H
#define IMAGE_PARSER_H

#include "utils/sol.h"
#include "seasons.h"

// в таблицах изображения указываются в виде "img_name.index.mirror"
// это позволяет мне не парится по поводу каких то возвращаемых значений из парсера
// изображения будут лежать в папках модов, я конечно могу все картинки сохранить в данных мира
// но делать это не нужно, помимо картинок мне нужно будет грузить луа скрипты
// которые тоже нужно брать из мода
// вообще похоже что у меня сейчас два ресурса которые нужно грузить по каким то путям
// текстурки и скрипты (текстовые файлы? какие то файлы нужно загрузить внутри скрипта) (звуки так то тоже)
// тут нужно еще победить относительные пути, путь всегда должен указывать на файл внутри папки с игрой
// + выход за пределы zip архива тоже нужно ошибкой оформлять
// к ресурсам нужно сделать алиасы если челик хочет потестировать мод до его отправки в воркшоп
// но это еще хрен знает когда
// загрузка в этот раз отличается тем что нам нужно загрузить все вместе
// скорее всего тут тоже неполучится узнать заранее что нам нужно а что нет

namespace devils_engine {
  namespace render {
    struct image_controller;
  }
  
  namespace utils {
    class world_serializator;
    
    void add_image(const sol::table &table); // в другой контейнер пойдет
    bool validate_image(const uint32_t &index, const sol::table &table); // эта функция по идее должна остаться одинакова для всех картинок
    bool validate_image_and_save(const uint32_t &index, sol::this_state lua, const sol::table &table, world_serializator* container);
    void load_images(render::image_controller* controller, const std::vector<sol::table> &image_tables, const uint32_t &current_type);
    
    void load_biomes(render::image_controller* controller, core::seasons* seasons, const std::vector<sol::table> &biome_tables);
    
    size_t add_battle_biome(const sol::table &table);
    bool validate_battle_biome(const uint32_t &index, const sol::table &table);
    // сохранять при генерации битвы ничего не нужно
    //bool validate_battle_biome_and_save(const uint32_t &index, sol::this_state lua, const sol::table &table, world_serializator* container);
    render::image_t parse_image(const std::string_view &str_id, render::image_controller* controller);
    void load_battle_biomes(render::image_controller* controller, const std::vector<sol::table> &biome_tables);
  }
}

#endif
