#ifndef CORE_INTERACTION_H
#define CORE_INTERACTION_H

#include <cstdint>
#include <cstddef>
#include <string>
#include <array>
#include "declare_structures.h"
#include "script/header.h"

// конечно видимо в будущем я это дело переделаю

// с помощью скриптов мне нужно отправить предложение и обработать отказ или согласие

// по идее это дело тоже нужно компилировать?

namespace devils_engine {
  namespace core {
    struct interaction {
      static const structure s_type = structure::interaction;
      static const size_t max_options_count = 8;
      
      enum class type {
        
      };
      
      struct option {
        std::string id;
        script::string name_script;
        script::string description_script;
        script::condition potential;
        script::condition condition;
        script::effect effect;
        script::number ai_will_do;
        // флаг?
      };
      
      std::string id;
      script::string name_script;
      script::string description_script;
      enum type type;
      
      // как задать входные данные?
      size_t input_count;
      //std::array<std::pair<std::string, script::script_data>, 16> input_array;
      // это наверное будет храниться непосредственно в script::script_data
      script::condition potential; // нужно использовать определенное соглашение по именам для переменных (например root и recipient)
      script::condition condition;
      script::condition auto_accept; // это триггер автопринятия, 
      // то есть мы его должны проверить (когда?) и тогда персонаж которому мы отправляем это дело в любом случае выполнит on_accept
      
      // вообще это можно подменить на что то другое, да будет неплохим вариантом например сделать эффект ДО акссепт
      // в этом эффекте например можно быстренько запомнить какие нибудь полезные вещи, 
      // и вообще скрипт вызывается последовательно с "усвоением" новых данных в скоупе
      // как это усвоение правильно сделать? оставить все данные дальше по вызовам
      // запомнить в контекст мы можем только в экшонах? скорее да чем нет
      script::effect immediate;
      
      // эффекты, нужно добавить проверки что где может использоваться (то есть вещи ниже используют только интеракции)
      // эти эффекты вызываются после send_options
      script::effect on_accept;
      // форсированное принятие, в каких случаях? например когда мы используем хук, 
      // как понять что мы использовали хук и нужно сделать авто акксепт?
      script::effect on_auto_accept;
      script::effect on_decline; // другой персонаж может например отказаться от принятия подарка
      script::effect pre_auto_accept; // ???
      script::effect on_blocked_effect; // персонаж может заблокировать интеракцию, например с помощью хука
      
      script::number ai_accept;    // число с описанием
      script::number ai_will_do;   // смотрим будем ли мы это делать
      script::number ai_frequency; // насколько часто ии использует это дело
      script::condition ai_potential; // будет ли ии вообще рассматривать возможность это сделать?
      
      // тут чаще всего нам нужно получить число, которое по условиям игры должно быть хотя бы 1 чтобы кнопку можно было бы нажать
      // некоторые из этих вещей могут быть специальными флажками (например хук как в цк3)
      uint32_t options_count;
      std::array<option, max_options_count> send_options; // тип примерно такой же как и опция в эвенте
      
      interaction();
      
      // какие входные данные? интеракция может быть и с персонажем и с городом каким
      // и с титулом наверное, кстати как титулы создавать? деньги + влияние? где должен быть скрипт на создание?
      // тут можно просто поставить массив какой нибудь, да но при этом мне будет удобно по именам 
      // разсовать все входные данные, таблица в качестве входа?
      //std::string_view name(core::character* root, const uint32_t &input_count, const script::target_t* targets) const;
    };
    
    bool validate_interaction(const size_t &index, const sol::table &table);
    void parse_interaction(core::interaction* interaction, const sol::table &table);
  }
}

#endif
