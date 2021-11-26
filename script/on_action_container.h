#ifndef ON_ACTION_CONTAINER_H
#define ON_ACTION_CONTAINER_H

#include <vector>
#include <cstdint>
#include <cstddef>
#include <functional>
#include "utils/array_proxy.h"
#include "on_action_types.h"
#include "object.h"
#include "header.h"

namespace devils_engine {
  namespace core {
    struct event;
  }
  
  namespace script {
    struct on_action_container {
      struct data {
        struct complex_event {
          const core::event* ev;
          uint32_t delay;
          float probability;
        };
        
        // проверим типы и положим по строкам данные, может быть такое что лайоут отсутствует, 
        // значит это какие то специальные он_экшоны как например пульсирующие (то есть вызываются сами по себе каждое некоторое время)
        std::vector<std::pair<std::string, uint32_t>> layout;
        std::vector<script::effect> effects; // эффекты до эвентов
        std::vector<const core::event*> events;
        std::vector<complex_event> complex_events;
        // тут бы еще наверное функцию какую запустить с тем что есть в контексте он_экшона
        // функция вызывается уже после вызова всех эвентов и эффектов, для того чтобы например сделать наследование
        // для эвентов вызывается потентиал ДО наследования (у эвента лишь один блок условий)
        std::function<void(const utils::array_proxy<object> &)> finalize_func;
      };
      
      struct data data[action_type::count];
      // тут добавится некоторый контейнер в который мы положим данные из fire_on_action
      // вызовем его на специальном этапе обработки on_action
    };
    
    void fire_on_action(const action_type::values &type, const std::initializer_list<object> &targets);
  }
}

#endif
