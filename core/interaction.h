#ifndef CORE_INTERACTION_H
#define CORE_INTERACTION_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
#include "declare_structures.h"
#include "script/script_header.h"

// конечно видимо в будущем я это дело переделаю

// с помощью скриптов мне нужно отправить предложение и обработать отказ или согласие

// по идее это дело тоже нужно компилировать?

namespace devils_engine {
  namespace core {
    struct interaction {
      enum class type {
        
      };
      
      struct option {
        std::string id;
        script::script_data name_script;
        script::script_data description_script;
        script::script_data potential;
        script::script_data condition;
        script::script_data effect;
        script::script_data ai_will_do;
        // флаг?
      };
      
      static const structure s_type = structure::interaction;
      
      std::string id;
      script::script_data name_script;
      script::script_data description_script;
      enum type type;
      
      // как задать входные данные?
      size_t input_count;
      std::array<std::pair<std::string, script::script_data>, 16> input_array;
      // это наверное будет храниться непосредственно в script::script_data
      script::script_data potential; // нужно использовать определенное соглашение по именам для переменных (например root и recipient)
      script::script_data condition;
      script::script_data auto_accept; // это триггер автопринятия, 
      // то есть мы его должны проверить (когда?) и тогда персонаж которому мы отправляем это дело в любом случае выполнит on_accept
      
      // вообще это можно подменить на что то другое, да будет неплохим вариантом например сделать эффект ДО акссепт
      // в этом эффекте например можно быстренько запомнить какие нибудь полезные вещи, 
      // и вообще скрипт вызывается последовательно с "усвоением" новых данных в скоупе
      // как это усвоение правильно сделать? оставить все данные дальше по вызовам
      // запомнить в контекст мы можем только в экшонах? скорее да чем нет
      script::script_data immediate;
      
      // эффекты, нужно добавить проверки что где может использоваться (то есть вещи ниже используют только интеракции)
      // эти эффекты вызываются после send_options
      script::script_data on_accept;
      // форсированное принятие, в каких случаях? например когда мы используем хук, 
      // как понять что мы использовали хук и нужно сделать авто акксепт?
      script::script_data on_auto_accept;
      script::script_data on_decline; // другой персонаж может например отказаться от принятия подарка
      script::script_data pre_auto_accept; // ???
      script::script_data on_blocked_effect; // персонаж может заблокировать интеракцию, например с помощью 
      
      script::script_data ai_accept;    // число с описанием
      script::script_data ai_will_do;   // смотрим будем ли мы это делать
      script::script_data ai_frequency; // насколько часто ии использует это дело
      script::script_data ai_potential; // будет ли ии вообще рассматривать возможность это сделать?
      
      // тут чаще всего нам нужно получить число, которое по условиям игры должно быть хотя бы 1 чтобы кнопку можно было бы нажать
      // некоторые из этих вещей могут быть специальными флажками (например хук как в цк3)
      std::vector<option> send_options; // тип примерно такой же как и опция в эвенте
      
      interaction();
    };
    
    bool validate_interaction(const size_t &index, const sol::table &table);
    void parse_interaction(core::interaction* interaction, const sol::table &table);
  }
}

#endif
