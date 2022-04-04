#ifndef DECISION_H
#define DECISION_H

#include <string>
#include <cstdint>
#include "core/declare_structures.h"
#include "script/header.h"
#include "script/context.h"

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
//       enum class type {
//         // правая кнопка (дипломатические? интеракция, перенес отсюда), мажорные, минорные, ???
//         major,
//         minor,
//         count
//       };
      
//       struct option {
//         std::string id;
//         script::string name_script;
//         script::string description_script;
//         script::condition potential;
//         script::condition condition;
//         script::effect effect;
//         script::number ai_will_do;
//         // флаг?
//       };
      
      static const structure s_type = structure::decision;
      
      std::string id;
//       enum type type;
      
      script::string name_script;
      script::string description_script;
      script::string confirm_text;
      
      script::condition ai_potential;
      script::condition potential;
      script::condition condition;
      script::effect effect;
      script::number ai_will_do;
      script::number ai_check_frequency;
      
      script::number money_cost;
      script::number authority_cost;
      script::number esteem_cost;
      script::number influence_cost;
      
      script::number cooldown;
      
      bool ai_goal;
      bool major;
      
      decision();
      ~decision();
      
      // зачем она мне нужна? я хотел что бы я просто запомнил инпут где нибудь и переиспользовал его
      // но это не особ нужно, можно в функции вычислить все что нужно
      //void setup_input(const uint32_t &count, const script::script_data* &input); // функция для проверки входных данных
      
      // вызов функции происходит по другому, нужно переделать методы
//       bool check_shown_condition(script::context* ctx) const; // тут тоже имеет смысл передавать целый контекст
//       bool check_condition(script::context* ctx) const;
//       // я могу сохранить "скомпилированный" интерфейс, хотя тут не обязательно
//       // нужно добавить еще итерацию по потентиал, там нужно указать два объекта 
//       // (или больше? например для женитьбы так то нужно 3 объекта)
//       void iterate_potential(script::context* ctx) const;
//       void iterate_conditions(script::context* ctx) const; // вставляем функцию луа
//       void iterate_actions(script::context* ctx) const;
//       
//       bool run(script::context* ctx) const;
//       
//       std::string_view get_name(script::context* ctx) const;
//       std::string_view get_description(script::context* ctx) const;
//       
//       // проверка для ии ничем принципиально не отличается
//       bool check_ai(script::context* ctx) const;
//       // тут мы скорее должны получить число (вес), чтобы рандомно понять будет делать это персонаж
//       bool run_ai(script::context* ctx) const;
//       
//       // возвращаем корневого персонажа
//       script::target_t check_input(script::context* ctx) const;
    };
    
    // pending_state ?
    struct compiled_decision {
      const decision* d;
      script::context ctx;
      bool used;
      
      compiled_decision(const decision* d, const script::context &ctx);
      
      std::string_view get_name();
      std::string_view get_description();
      std::string_view get_confirm_text();
      
      bool ai_potential();
      bool potential();
      bool condition();
      double ai_will_do();
      double ai_check_frequency();
      bool run();
      
      double money_cost();
      double authority_cost();
      double esteem_cost();
      double influence_cost();
      
      void draw_name(const sol::function &func);
      void draw_description(const sol::function &func);
      void draw_confirm_text(const sol::function &func);
      
      void draw_ai_potential(const sol::function &func);
      void draw_potential(const sol::function &func);
      void draw_condition(const sol::function &func);
      void draw_ai_will_do(const sol::function &func);
      void draw_ai_check_frequency(const sol::function &func);
      void draw_effect(const sol::function &func);
      
      void draw_money_cost(const sol::function &func);
      void draw_authority_cost(const sol::function &func);
      void draw_esteem_cost(const sol::function &func);
      void draw_influence_cost(const sol::function &func);
    };
    
    // мне нужно компилировать десижоны, для того чтобы не пересоздавать контекст несколько раз и не портить рандом
    // хотя тут возможен абуз, рандом не должен зависеть от пересоздания, нужно чет придумать с рандомом
    // что у нас есть? у нас есть десижон, у нас есть текущий ход, имеет ли смысл абузить раз в ход
    // да поди неплохо использовать для генерации числа текущий ход, было бы неплохо чтобы 
    // луа контролировал время жизни этой штуки
//     struct compiled_decision {
//       const decision* d;
//       size_t state;
//       script::context ctx;
//       
//       compiled_decision(const decision* d, const sol::table &t);
//       
//       bool check_shown_condition() const;
//       bool check_condition() const;
//       void iterate_potential(sol::function func) const;
//       void iterate_conditions(sol::function func) const;
//       void iterate_actions(sol::function func) const;
//       
//       bool run() const;
//       
//       std::string_view get_name() const;
//       std::string_view get_description() const;
//     };
    
    void init_input_array(const sol::object &obj, core::decision* d);
    // вообще имеет смысл сделать инициализацию прямо здесь
    bool validate_decision(const size_t &index, const sol::table &table);
    void parse_decision(core::decision* decision, const sol::table &table);
  }
}

#endif
