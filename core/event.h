#ifndef EVENT_H
#define EVENT_H

#include <string>
#include <array>
#include "declare_structures.h"
#include "render/shared_structures.h"
#include "script/header.h"
#include "utils/sol.h"

// тут в принципе ничего сложного, проверяем триггер, эвент сидит у объекта, проверяем мттх
// эвент может быть скрытым, делаем эффект, даем игроку подумать над опциями
// эвенты полегче чем десижоны, не, десижоны проще

namespace devils_engine {
  namespace core {
    struct event {
      static const structure s_type = structure::event;
      static const size_t max_options_count = 8;
      
      struct option {
        script::string name_script;
        script::string description_script;
        script::condition condition;
        script::effect effects;
        script::number ai_weight;
        
        option();
      };
      
      std::string id;
      script::string name_script;
      script::string description_script;
      
      // вообще это тоже нужно пихнуть в скрипт, как изображение можно запихнуть в скрипт? пока что неуверен
      // по идее так же как строку - это по сути строковый скрипт, только по полученной стороке мы ищем изображение
      render::image_t image;
      script::string image_id;
      script::condition potential;
      script::number mtth_script; // среднее время возникновения, должны быть модификаторы для этого 
      //immediate effect
      
      // наверное нужно пихнуть это дело в вектор
      uint32_t options_count;
      std::array<option, max_options_count> options;
      
      event();
    };
    
    bool validate_event(const size_t &index, const sol::table &table);
    void parse_event(core::event* event, const sol::table &table);
  }
}

#endif
