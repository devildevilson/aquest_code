#ifndef DEVILS_ENGINE_CORE_POWER_H
#define DEVILS_ENGINE_CORE_POWER_H

#include <cstdint>
#include <cstddef>
#include <string>
#include "declare_structures.h"

// в чем прикол, глобальная сила - это некая механика привязанная к карте, 
// то есть некая область на карте с крупной нестандартной империей или область на карте с богом (апати), 
// к которой можно обратиться через специальный интерфейс (не через дипломатию), 
// в этом интерфейсе есть какие то услуги предоставляемые за валюту (либо валюту силы либо просто деньги), 
// сила может быть частью религии, то есть религиозный аспект государства может быть подчинен силе, 
// может ли сила принимать участие в обычных взаимодействиях государств? скорее да чем нет, 
// да и потом что есть сила в принципе? отдельное государство (а может и не быть государством)
// + интерфейс с услугами, сила может быть встроена например в религию 
// (например только она может выдавать статус священика), сила может быть встрокена в правила мира
// (например только сила может делать персонажа магом), это не говоря уж о других плюшках
// сила может как то влиять на силу и обновление данжей и монстров в мире
// религии должны скорее всего как то регламентировать отношение к силе (например отношение к апати)
// (собственно молитвы дьяволу в цк2), сила может представлять из себя некое сообщество
// (которое тоже по большому счету предоставляет какие то услуги за плату)

namespace devils_engine {
  namespace core {
    struct power {
      struct service {
        std::string id;
        
        // название и описание будет наверное по скрипту генериться
        // видимость услуги для персонажа
        // стоимость услуги (точнее условие для возможности получения эффекта)
        // эффект
        // ии
        // ???
      };
      
      std::string id;
      std::string name_id;
      std::string description_id;
      
      // картиночка, цвета, оформление интерфейса?
      
      // сила является частью религии (например религиозный глава)
      const struct religion* religion;
      // сила обладает некой территорией
      const struct realm* realm;
      // сила может являться персонажем (апати)
      const struct character* character;
      
      // доступность силы? дипломатическая дальность или какие то другие условия
      // реакция силы на какие то вещи? возможно, но это скорее со стороны эвентов должно быть
      // у силы наверное должна быть какая то память, причем наверное память уникальная для персонажа
      // 
      
      // услуги, количество услуг регламентированно? вряд ли
      // среди услуг должны быть еще и какие то крупные походы
      // все это должно быть частю скрипта, ии тоже должно уметь пользоваться всем этим
      // как будут храниться услуги? просто список? скорее всего
    };
  }
}

#endif
