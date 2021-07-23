#ifndef EVENT_H
#define EVENT_H

#include <string>
#include <array>
#include "declare_structures.h"
#include "render/shared_structures.h"
#include "script/script_header.h"

// тут в принципе ничего сложного, проверяем триггер, эвент сидит у объекта, проверяем мттх
// эвент может быть скрытым, делаем эффект, даем игроку подумать над опциями
// эвенты полегче чем десижоны

namespace devils_engine {
  namespace core {
    // эвенты можно генерировать даже нужно, изменятся основные данные
    // данные эвента не должны меняться по ходу игры вообще
    // константные эвенты удобнее использовать с std vector
    struct event {
      static const structure s_type = structure::event;
      static const size_t max_options_count = 8;
      
      struct option {
        script::script_data name_script;
        script::script_data description_script;
        script::script_data conditions;
        script::script_data effects;
        script::script_data ai_weight;
        
        option();
      };
      
      std::string id;
      script::script_data name_script;
      script::script_data description_script;
      
      // какой инпут? есть ли тут рут? кажется тут вполне можно использовать те же правила что и в десижоне
      // то есть мы можем запомнить рут в скоуп, но здесь не нужно пушить необходимость присутствия рута
      // а значит нужно аккуратно сделать рвалуе
      // нет, у нас рут - это то объект для которого вызывается эвент
      
      // вообще это тоже нужно пихнуть в скрипт
      render::image_t image;
      script::script_data potential;
      //size_t mtth; // среднее время возникновения, должны быть модификаторы для этого 
      script::script_data mtth_script;
      //immediate
      
      // наверное нужно пихнуть это дело в вектор
      uint32_t options_count;
      std::array<option, max_options_count> options;
      
      event();
      
      void fire(struct character* c);
    };
  }
}

#endif
