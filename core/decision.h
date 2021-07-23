#ifndef DECISION_H
#define DECISION_H

#include <string>
#include <cstdint>
#include "core/declare_structures.h"
#include "script/script_header.h"

namespace devils_engine {
  namespace core {
    
    // было бы неплохо иметь возможность проверить наличие данных в контексте и изменить поведение на основе этого
    // как это можно сделать? у меня должны появится специальные функции которые не особо полезные в обычном слоке команд
    // но полезные в кондишоне для блока команд, например проверка есть какие нибудь данные в слоте 
    
    // (например призвать ко двору нового аристократа, по аналогии с цк2)
    // тут какая то полугенерация, есть основные действия которые не изменяются, 
    // и есть действия сгенерированные становятся доступны после каких нибудь событий в игре
    // нужно проверить как в цк2 (реконкиста помоему одно из таких действий)
    // (нет, реконкиста привязана к культуре)
    // решения можно практически полностью описать в json (по аналогии с цк или европой)
    // есть блок контролирующий появление решения, блок условий и блок эффектов
    // соответственно мы можем задать множество функций на проверку всех этих условий
    // это позволит нам сгенерировать классные описания 
    // может быть еще вещью которая появляется по правой кнопке на персонаже или городе (тип decision)
    // в таких решениях целей может быть несколько (2, +1 в некоторых случаях)
    
    // как проверитиь что нам доступно а что нет? мы проверяем potential скрипт
    // какие данные идут в potential? мы делаем ctx и обходим все десижоны для двоих, или для одного
    // вообще у нас сильно отличается ктх для потентиал и для остальных вещей
    // а ктх для потентиал должен примерно работать по тем же правилам что и у всех десижонов
    // у нас может быть ситуация когда мы проверяем кондишон неверного типа, но мы не хотим вылетать при этом
    // что делать? придется оформлять все кондишоны так чтобы они возвращали фалсе если у нас несовпадение типов
    // как же интересно сделано в цк?
    // десижоны которые вызываются как взаимодействие между персонажами, описываются по другому
    // да и вообще я так понимаю взаимодействие с разными типами описывается по разному в цк3
    // взаимодействие между персонажами идет вместе с send_option, on_send, on_auto_accept, on_accept, on_decline
    // мы можем дать игроку возможность выбрать как именно мы сделаем то или иное действие
    // а также может быть согласие и не согласие с взаимодействием, и вот кстати как понять согласен ли чел или нет
    // для модификаторов нужно еще и описание прилепить тогда
    
    // кажется в цк3 они избавились максимально от всех интеракций которые не между персонажами
    // мне в принципе тоже не обязательно какие то другие интеракции придумывать
    // таким образом по сути имеется только взаимодествие персонажей и решения по кнопке
    // это облегчает задачу для potential
    // все равно мне может пригодится взаимодействие с религиями (неточно) и городами (точно)
    // религии поди через персонажей
    // в цк еще можно указать возможных таргетов для ии
    // надо наверное все же разделить десижоны от интеракций
    struct decision {
      enum class type {
        // правая кнопка (дипломатические? интеракция, перенес отсюда), мажорные, минорные, ???
        major,
        minor,
        count
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
      
      static const structure s_type = structure::decision;
      
      std::string id;
      script::script_data name_script;
      script::script_data description_script;
      enum type type;
      
      // было бы неплохо както сократить количество проверяемых decision
      
      // как задать входные данные?
      size_t input_count;
      std::array<std::pair<std::string, script::script_data>, 16> input_array;
      // это наверное будет храниться непосредственно в script::script_data
      script::script_data potential; // нужно использовать определенное соглашение по именам для переменных (например root и recipient)
      script::script_data condition; // используется только для major minor (?)
      // то есть мы его должны проверить (когда?) и тогда персонаж которому мы отправляем это дело в любом случае выполнит on_accept
      
      // тут довольно все просто с этим делом, нажимаем кнопку применить если условия проходят делаем эффект
      script::script_data effect;
      
      // как ии поймет что ему хочется сделать это решение?
      script::script_data ai_will_do;
      
      decision();
      ~decision();
      
      // зачем она мне нужна? я хотел что бы я просто запомнил инпут где нибудь и переиспользовал его
      // но это не особ нужно, можно в функции вычислить все что нужно
      //void setup_input(const uint32_t &count, const script::script_data* &input); // функция для проверки входных данных
      
      // вызов функции происходит по другому, нужно переделать методы
      bool check_shown_condition(script::context* ctx) const; // тут тоже имеет смысл передавать целый контекст
      bool check_condition(script::context* ctx) const;
      // я могу сохранить "скомпилированный" интерфейс, хотя тут не обязательно
      // нужно добавить еще итерацию по потентиал, там нужно указать два объекта 
      // (или больше? например для женитьбы так то нужно 3 объекта)
      void iterate_potential(script::context* ctx) const;
      void iterate_conditions(script::context* ctx) const; // вставляем функцию луа
      void iterate_actions(script::context* ctx) const;
      
      bool run(script::context* ctx) const;
      
      std::string_view get_name(script::context* ctx) const;
      std::string_view get_description(script::context* ctx) const;
      
      // проверка для ии ничем принципиально не отличается
      bool check_ai(script::context* ctx) const;
      // тут мы скорее должны получить число (вес), чтобы рандомно понять будет делать это персонаж
      bool run_ai(script::context* ctx) const;
      
      // возвращаем корневого персонажа
      script::target_t check_input(script::context* ctx) const;
    };
    
    // мне нужно компилировать десижоны, для того чтобы не пересоздавать конткест несколько раз и не портить рандом
    // хотя тут возможен абуз, рандом не должен зависеть от пересоздания, нужно чет придумать с рандомом
    // что у нас есть? у нас есть десижон, у нас есть текущий ход, имеет ли смысл абузить раз в ход
    // да поди неплохо использовать для генерации числа текущий ход, было бы неплохо чтобы 
    // луа контролировал время жизни этой штуки
    struct compiled_decision {
      const decision* d;
      size_t state;
      script::context ctx;
      
      compiled_decision(const decision* d, const sol::table &t);
      
      bool check_shown_condition() const;
      bool check_condition() const;
      void iterate_potential(sol::function func) const;
      void iterate_conditions(sol::function func) const;
      void iterate_actions(sol::function func) const;
      
      bool run() const;
      
      std::string_view get_name() const;
      std::string_view get_description() const;
    };
    
    void init_input_array(const sol::object &obj, core::decision* d);
    // вообще имеет смысл сделать инициализацию прямо здесь
    bool validate_decision(const size_t &index, const sol::table &table);
    void parse_decision(core::decision* decision, const sol::table &table);
  }
}

#endif
